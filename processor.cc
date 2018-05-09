// Copyright 2018 Takashi Matsuura.
// Copyright 2015 Matthias Puech
//
// Original author: Matthias Puech (matthias.puech@gmail.com)
// Modified by: Takashi Matsuura (fwthesteelleg@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Processor. Orchestrates the four LFOs.

#include "drivers/adc.h"
#include "drivers/dac.h"

#include "processor.h"
#include "ui.h"
#include "resources.h"
#include "stmlib/utils/dsp.h"

namespace batumi {

using namespace stmlib;

const int16_t kUnsyncPotThreshold = INT16_MAX / 20;
const int16_t kResetThresholdLow = 10000;
const int16_t kResetThresholdHigh = 20000;

void Processor::Init(Ui *ui, Adc *adc, Dac *dac) {
  ui_ = ui;
  adc_ = adc;
  dac_ = dac;
  previous_feat_mode_ = FEAT_MODE_LAST;
  // no need to Init the LFOs, it'll be done in Process on first run
  for (uint8_t i=0; i<kNumChannels; i++) {
    reset_trigger_armed_[i]= false;
    last_reset_[i] = 0;
  }
  waveform_offset_ = 0;
}

inline int16_t AdcValuesToPitch(uint16_t coarse, int16_t fine, int16_t cv) {
  coarse = Interpolate88(lut_scale_pitch, coarse) - 32768;
  fine = (1 * kOctave * static_cast<int32_t>(fine)) >> 16;
  cv = cv * 5 * kOctave >> 15;
  return coarse + fine + cv;
}

inline uint8_t AdcValuesToDivider(uint16_t pot, int16_t fine, int16_t cv) {
  int32_t ctrl = pot + cv;
  CONSTRAIN(ctrl, 0, UINT16_MAX);
  fine = (5 * static_cast<int32_t>(fine + INT16_MAX / 5)) >> 16;
  int8_t div = lut_scale_divide[ctrl >> 8];
  div -= fine;
  CONSTRAIN(div, 1, 64);
  return div;
}

inline uint16_t AdcValuesToPhase(uint16_t pot, int16_t fine, int16_t cv) {
  pot = 65536 - pot;
  int32_t ctrl = pot + cv + fine / 8;
  CONSTRAIN(ctrl, 0, UINT16_MAX);
  return Interpolate88(lut_scale_phase, ctrl);
}

inline uint16_t AdcValuesToLevel(uint16_t pot, int16_t fine, int16_t cv) {
  int32_t ctrl = pot + cv - 256;
  ctrl += fine / 4;
  CONSTRAIN(ctrl, 0, UINT16_MAX);
  // lut_scale_phase is completely linear, so we can use it for levels
  return Interpolate88(lut_scale_phase, ctrl);
}

void Processor::SetFrequency(int8_t lfo_no) {

  int16_t cv = (filtered_cv_[lfo_no] * ui_->atten(lfo_no)) >> 16;

  // In sync mode, CV multiplies or divides period
  if (ui_->sync_mode()) {
    if (cv > 0) {
      lfo_[lfo_no].set_multiplier((cv * 8 / 32767) + 1);
      lfo_[lfo_no].set_divider(1);
    } else {
      lfo_[lfo_no].set_multiplier(1);
      lfo_[lfo_no].set_divider((-cv * 8 / 32767) + 1);
    }
  }

  // sync or reset
  if (reset_triggered_[lfo_no]) {
    if (ui_->sync_mode()) {
      lfo_[lfo_no].set_period(last_reset_[lfo_no]);
      lfo_[lfo_no].align();
      synced_[lfo_no] = true;
    } else {
      lfo_[lfo_no].Reset(reset_subsample_[lfo_no]);
    }
    reset_trigger_armed_[lfo_no] = false;
    last_reset_[lfo_no] = 0;
  } else {
    last_reset_[lfo_no]++;
  }

  int16_t pitch = AdcValuesToPitch(ui_->coarse(lfo_no),
				   ui_->fine(lfo_no),
				   cv);

  // on individual-wavebank firmware, this pitch shift should be deactivated because it can be confusing.
  //if (ui_->bank(lfo_no) == BANK_RANDOM)
  //  pitch += 1 * kOctave;

  // set pitch
  if (!synced_[lfo_no] ||
      ui_->coarse(lfo_no) >= last_coarse_[lfo_no] + kUnsyncPotThreshold ||
      ui_->coarse(lfo_no) <= last_coarse_[lfo_no] - kUnsyncPotThreshold) {
    lfo_[lfo_no].set_pitch(pitch);
    last_coarse_[lfo_no] = ui_->coarse(lfo_no);
    synced_[lfo_no] = false;
  }
}

void Processor::Process() {

  // do not run during the splash animation
  if (ui_->mode() == UI_MODE_SPLASH)
    return;

  // reset the LFOs if mode changed
  if (ui_->feat_mode() != previous_feat_mode_) {
    for (int i=0; i<kNumChannels; i++)
      lfo_[i].Init();
    previous_feat_mode_ = ui_->feat_mode();
    waveform_offset_ = 0;
  }

  for (int i=0; i<kNumChannels; i++) {
    // set level
    if (ui_->feat_mode() != FEAT_MODE_QUAD)
      lfo_[i].set_level(AdcValuesToLevel(ui_->level(i), 0, 0));
    // filter CV
    filtered_cv_[i] += (adc_->cv(i) - filtered_cv_[i]) >> 6;

    // detect triggers on the reset input
    int16_t reset = adc_->reset(i);

    if (reset < kResetThresholdLow)
      reset_trigger_armed_[i] = true;

    if (reset > kResetThresholdHigh &&
	reset_trigger_armed_[i]) {
      reset_triggered_[i] = true;
      int32_t dist_to_trig = kResetThresholdHigh - previous_reset_[i];
      int32_t dist_to_next = reset - previous_reset_[i];
      reset_subsample_[i] = dist_to_trig * 32L / dist_to_next;
    } else {
      reset_triggered_[i] = false;
    }

    previous_reset_[i] = reset;
  }

  switch (ui_->feat_mode()) {

  case FEAT_MODE_FREE:
  {
    for (uint8_t i=0; i<kNumChannels; i++) {
      SetFrequency(i);
      lfo_[i].set_initial_phase(ui_->phase(i));
    }
  }
  break;

  case FEAT_MODE_QUAD:
  {
    // 1st channel sets frequency as usual
    SetFrequency(0);
    lfo_[0].set_initial_phase(ui_->phase(0));

    // reset 2 holds the LFOs
    lfo_[0].set_hold(reset_triggered_[1]);
    // reset 3 changes direction
    lfo_[0].set_direction(!reset_triggered_[2]);
    // reset 4 changes waveform
    if (reset_triggered_[3]) {
      waveform_offset_++;
      reset_trigger_armed_[3] = false;
    }

    lfo_[0].set_level(AdcValuesToLevel(ui_->level(0), 0, 0));

    // the others are special cases
    for (int i=1; i<kNumChannels; i++) {

      // main pot and CV sets level
      int32_t cv = (filtered_cv_[i] * ui_->atten(i)) >> 16;
      lfo_[i].set_level(AdcValuesToLevel(ui_->coarse(i), ui_->fine(i), cv));

      // channel i is divided by i+1; second parameter adjusts divider
      lfo_[i].link_to(&lfo_[0]);
      int16_t div = (7 * static_cast<int32_t>(65535 - ui_->level(i))) >> 16;
      div += i + 1;
      CONSTRAIN(div, 1, 16);
      lfo_[i].set_divider(div);

      // last parameter controls phase
      lfo_[i].set_initial_phase(ui_->phase(i));
    }
  }
  break;

  case FEAT_MODE_PHASE:
  {
    SetFrequency(0);

    // reset 2 holds the LFOs
    lfo_[0].set_hold(reset_triggered_[1]);
    // reset 3 changes direction
    lfo_[0].set_direction(!reset_triggered_[2]);
    // reset 4 changes waveform
    if (reset_triggered_[3]) {
      waveform_offset_++;
      reset_trigger_armed_[3] = false;
    }

    // if all the pots are maxed out, quadrature mode
    if (ui_->coarse(1) > UINT16_MAX - 256 &&
	ui_->coarse(2) > UINT16_MAX - 256 &&
	ui_->coarse(3) > UINT16_MAX - 256)
      for (int i=1; i<kNumChannels; i++) {
	lfo_[i].link_to(&lfo_[0]);
	lfo_[i].set_initial_phase((kNumChannels - i) * (UINT16_MAX >> 2));
      }
    else // normal phase mode
      for (int i=1; i<kNumChannels; i++) {
	lfo_[i].link_to(&lfo_[0]);
	int16_t cv = (filtered_cv_[i] * ui_->atten(i)) >> 16;
	lfo_[i].set_initial_phase(AdcValuesToPhase(ui_->coarse(i),
						   ui_->fine(i),
						   cv));
	int16_t div = (7 * static_cast<int32_t>(65535 - ui_->phase(i))) >> 16;
	CONSTRAIN(div, 1, 16);
	lfo_[i].set_divider(div);
      }
  }
  break;

  case FEAT_MODE_DIVIDE:
  {
    SetFrequency(0);
    lfo_[0].set_initial_phase(ui_->phase(0));

    // reset 2 holds the LFOs
    lfo_[0].set_hold(reset_triggered_[1]);
    // reset 3 changes direction
    lfo_[0].set_direction(!reset_triggered_[2]);
    // reset 4 changes waveform
    if (reset_triggered_[3]) {
      waveform_offset_++;
      reset_trigger_armed_[3] = false;
    }

    for (int i=1; i<kNumChannels; i++) {
      lfo_[i].link_to(&lfo_[0]);
      int16_t cv = (filtered_cv_[i] * ui_->atten(i)) >> 16;
      lfo_[i].set_divider(AdcValuesToDivider(ui_->coarse(i),
					     ui_->fine(i),
					     cv));
      lfo_[i].set_initial_phase(ui_->phase(i));
      // when 1st channel resets, all other channels reset
      if (!ui_->sync_mode() && reset_triggered_[0]) {
	lfo_[i].Reset(reset_subsample_[0]);
      }
    }
  }
  break;

  case FEAT_MODE_LAST: break;	// to please the compiler
  }

  LfoShape shape[4];
  for (int i=0; i<4; i++) {
    int ui_shape = ui_->shape();
    uint8_t offset = 0;
    switch (ui_->bank(i)) {
    case BANK_CLASSIC:
      offset = SHAPE_TRAPEZOID;
      break;

    case BANK_RANDOM:
      // RANDOM is not affected by panel switches, but by random waveform setting
      ui_shape = ui_->random_waveform_index(i);
      offset = SHAPE_RANDOM_STEP;
      break;

    default:
      offset = 42;
      break;
    }

    int s = ((ui_shape + waveform_offset_) % 4) + offset;
    shape[i] = static_cast<LfoShape>(s);

    // exception: in quad mode, trapezoid becomes square
    if (ui_->feat_mode() == FEAT_MODE_QUAD &&
        shape[i] == SHAPE_TRAPEZOID)
      shape[i] = SHAPE_SQUARE;
  }

  int32_t sample1 = 0;
  int32_t sample2 = 0;
  int32_t gain = 0;

  // send to DAC and step
  for (int i=kNumChannels-1; i>=0; i--) {
    lfo_[i].Step();

    if (ui_->feat_mode() != FEAT_MODE_QUAD) {
      sample1 = sample2 = gain = 0;
    }

    sample1 += lfo_[i].ComputeSampleShape(SHAPE_SINE);
    sample2 += lfo_[i].ComputeSampleShape(shape[i]);
    gain += lfo_[i].level();

    if (ui_->feat_mode() == FEAT_MODE_QUAD) {
      // normalized
      int32_t g = gain < UINT16_MAX ? UINT16_MAX : gain;
      dac_->set_sine(i, ((sample1 << 13) / g) << 3);
      dac_->set_asgn(i, ((sample2 << 13) / g) << 3);
    } else {
      dac_->set_sine(i, sample1);
      dac_->set_asgn(i, sample2);
    }
  }
}
}
