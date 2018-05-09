// Copyright 2018 Takashi Matsuura.
// Copyright 2015 Matthias Puech.
// Copyright 2013 Olivier Gillet.
//
// Original author: Olivier Gillet (ol.gillet@gmail.com)
// Modified by: Matthias Puech (matthias.puech@gmail.com)
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
// User interface.

#include "ui.h"
#include "stmlib/system/storage.h"

#include <algorithm>

namespace batumi {

using namespace std;
using namespace stmlib;

const int32_t kLongPressDuration = 500;
const int32_t kVeryLongPressDuration = 2000;
const int32_t kClearSettingsLongPressDuration = 4000;

// These threshold should be as large as possible
// to prevent the setting values to jump to the current slider position.
// It depends on required value resolution on each mode.
const int32_t kPotMoveThreshold = 1 << (16 - 8);  // 8 bits
const int32_t kPotMoveThresholdOnZoomMode = 1 << (16 - 6);  // 6 bits
const int32_t kPotMoveThresholdOnRandomWaveformSelectMode = 1 << (16 - 5);  // 5 bits

const uint16_t kCatchupThreshold = 1 << 10;

stmlib::Storage<0x8020000, 4> storage;

void Ui::Init(Adc *adc) {
  mode_ = UI_MODE_SPLASH;
  adc_ = adc;
  leds_.Init();
  switches_.Init(adc_);
  animation_counter_ = 0;

  if (!storage.ParsimoniousLoad(&feat_mode_, SETTINGS_SIZE, &version_token_)) {
    feat_mode_ = FEAT_MODE_FREE;
    clearAllHiddenSettings();
  }

  // synchronize pots at startup
  for (uint8_t i=0; i<4; i++) {
    uint16_t adc_value = adc_->pot(i);
    pot_value_[i] = pot_filtered_value_[i] = pot_coarse_value_[i] = adc_value;
    catchup_state_[i] = false;
  }
}

void Ui::Poll() {
  switches_.Debounce();

  // we begin the iteration after the internal switches (or jumpers),
  // which are polled manually
  for (uint8_t i = SWITCH_SELECT; i < kNumSwitches; ++i) {
    if (switches_.just_pressed(i)) {
      queue_.AddEvent(CONTROL_SWITCH, i, 0);
      press_time_[i] = system_clock.milliseconds();
      detect_very_long_press_[i] = false;
      detect_clear_settings_long_press_[i] = false;
    }
    if (switches_.pressed(i) && press_time_[i]) {
      int32_t pressed_time = system_clock.milliseconds() - press_time_[i];

      if (!detect_clear_settings_long_press_[i]) {
        if (!detect_very_long_press_[i]) {
          if (pressed_time > kLongPressDuration) {
            queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
            detect_very_long_press_[i] = true;
          }
        } else {
          if (pressed_time > kVeryLongPressDuration) {
            queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
            detect_very_long_press_[i] = false;
            detect_clear_settings_long_press_[i] = true;
          }
        }
      } else {
        if (pressed_time > kClearSettingsLongPressDuration) {
          queue_.AddEvent(CONTROL_SWITCH, i, pressed_time);
          detect_very_long_press_[i] = false;
          detect_clear_settings_long_press_[i] = false;
          press_time_[i] = 0;
        }
      }
    }

    if (switches_.released(i) &&
        press_time_[i] != 0 &&
        !detect_very_long_press_[i] &&
        !detect_clear_settings_long_press_[i]) {
      queue_.AddEvent(
          CONTROL_SWITCH,
          i,
          system_clock.milliseconds() - press_time_[i] + 1);
      press_time_[i] = 0;
    }
  }

  // filter the pot values and emit events when changed

  // Decide pot move threshold
  int32_t potMoveThreshold = kPotMoveThreshold;
  if (mode_ == UI_MODE_ZOOM)
    potMoveThreshold = kPotMoveThresholdOnZoomMode;
  else if (mode_ == UI_MODE_RANDOM_WAVEFORM_SELECT)
    potMoveThreshold = kPotMoveThresholdOnRandomWaveformSelectMode;

  for (uint8_t i = 0; i < 4; ++i) {
    uint16_t adc_value = adc_->pot(i);
    int32_t value = (31 * pot_filtered_value_[i] + adc_value) >> 5;
    pot_filtered_value_[i] = value;
    int32_t current_value = static_cast<int32_t>(pot_value_[i]);

    if (value >= current_value + potMoveThreshold ||
	value <= current_value - potMoveThreshold) {
      queue_.AddEvent(CONTROL_POT, i, value);
      pot_value_[i] = value;
    }
  }
  
  // paint the interface
  switch (mode_) {
  case UI_MODE_SPLASH:
    if (animation_counter_ % 64 == 0) {
      for (int i=0; i<kNumLeds; i++)
	leds_.set(i, ((animation_counter_ / 64) % 4) == i);
      if (animation_counter_ / 64 > 3)
	mode_ = UI_MODE_NORMAL;
    }
    animation_counter_++;
    break;

  case UI_MODE_ZOOM:
    animation_counter_++;
    for (uint8_t i=0; i<kNumLeds; i++)
      leds_.set(i, false);
    leds_.set(last_touched_pot_, animation_counter_ & 128);
    break;

  case UI_MODE_NORMAL:
    {
      animation_counter_++;
      bool flash = (animation_counter_ & 64) &&
        (animation_counter_ & 32) &&
        (animation_counter_ & 16);
      for (uint8_t i=0; i<kNumLeds; i++) {
        if (catchup_state_[i])
	  leds_.set(i, i==feat_mode_ ? !flash : flash);
        else
	  leds_.set(i, i == feat_mode_);
      }
    }
    break;

  case UI_MODE_RANDOM_WAVEFORM_SELECT:
    {
      const uint16_t led_on_period = 0x80;
      bool flash_base = !(animation_counter_ & led_on_period);
      uint8_t setting_val_count = (animation_counter_ / (led_on_period * 2)) % 4;

      for (uint8_t i=0; i<kNumLeds; i++) {
	if (flash_base &&
	    bank_[i] == BANK_RANDOM &&
	    (random_waveform_index_[i] >= setting_val_count)) {
	  leds_.set(i, true);
	}
	else {
	  leds_.set(i, false);
	}
      }
    }
    animation_counter_++;
    break;
  case UI_MODE_SPLASH_FOR_RANDOM_WAVEFORM_SELECT:
    {
      const int num_blink = 2;
      if (animation_counter_ % 100 == 0) {
        for (int i=0; i<kNumLeds; i++)
	  leds_.set(i, ((animation_counter_ / 100) % 2) == 0);
      }
      animation_counter_++;

      if (animation_counter_ >= (num_blink * 200) - 1) {
	mode_ = UI_MODE_RANDOM_WAVEFORM_SELECT;
	animation_counter_ = 0;
      }
    }
    break;
  }

  leds_.Write();
}

void Ui::FlushEvents() {
  queue_.Flush();
}

void Ui::OnSwitchPressed(const Event& e) {
}

void Ui::OnSwitchReleased(const Event& e) {
  switch (e.control_id) {
  case SWITCH_SYNC:
  case SWITCH_WAV1:
  case SWITCH_WAV2:
    break;
  case SWITCH_SELECT:
    if (e.data > kClearSettingsLongPressDuration) {
      // Clear all hidden settings and save to ROM
      clearAllHiddenSettings();
      animation_counter_ = 0;
      mode_ = UI_MODE_SPLASH;
      storage.ParsimoniousSave(&feat_mode_, SETTINGS_SIZE, &version_token_);
    } else if (e.data > kVeryLongPressDuration) {
      if (mode_ != UI_MODE_RANDOM_WAVEFORM_SELECT) {
        mode_ = UI_MODE_SPLASH_FOR_RANDOM_WAVEFORM_SELECT;
        animation_counter_ = 0;
      }
    } else if (e.data > kLongPressDuration) {
      if (mode_ == UI_MODE_NORMAL) {
	mode_ = UI_MODE_ZOOM;
      } else if (mode_ == UI_MODE_ZOOM) {
	gotoNormalModeWithCatchupAndSaving();
      }
    } else {
      switch (mode_) {
      case UI_MODE_SPLASH:
      case UI_MODE_SPLASH_FOR_RANDOM_WAVEFORM_SELECT:
	break;
      case UI_MODE_ZOOM:
      case UI_MODE_RANDOM_WAVEFORM_SELECT:
	gotoNormalModeWithCatchupAndSaving();
	break;

      case UI_MODE_NORMAL:
	feat_mode_ = static_cast<FeatureMode>((feat_mode_ + 1) % FEAT_MODE_LAST);
	// reset all alternate values
	clearZoomSettings();
	storage.ParsimoniousSave(&feat_mode_, SETTINGS_SIZE, &version_token_);
	break;
      }
    }
    break;
  }
}

void Ui::OnPotChanged(const Event& e) {
  switch (mode_) {
  case UI_MODE_SPLASH:
  case UI_MODE_SPLASH_FOR_RANDOM_WAVEFORM_SELECT:
    break;
  case UI_MODE_ZOOM:
    switch (e.control_id) {
    case 0:
      pot_fine_value_[last_touched_pot_] = e.data;
      break;
    case 1:
      pot_level_value_[last_touched_pot_] = e.data;
      break;
    case 2:
      pot_atten_value_[last_touched_pot_] = e.data;
      break;
    case 3:
      pot_phase_value_[last_touched_pot_] = e.data;
      break;
    }
    break;
  case UI_MODE_NORMAL:
    last_touched_pot_ = e.control_id;
    if (!catchup_state_[e.control_id]) {
      pot_coarse_value_[e.control_id] = e.data;
    } else if (abs(e.data - pot_coarse_value_[e.control_id]) < kCatchupThreshold) {
      pot_coarse_value_[e.control_id] = e.data;
      catchup_state_[e.control_id] = false;
    }
    break;

  case UI_MODE_RANDOM_WAVEFORM_SELECT:
    selectRandomWaveformFromPot(e.control_id, e.data);
    break;
  }
}

void Ui::selectRandomWaveformFromPot(uint16_t id, int32_t potVal)
{
  static const uint16_t tbl_waveformSelectionThreshold[] = {
    9500, 26214, 42312, 60000
  };
  static const int size_table = sizeof(tbl_waveformSelectionThreshold) / sizeof(tbl_waveformSelectionThreshold[0]);

  uint8_t pos = 4;
  for (uint8_t i=0; i<size_table; i++) {
    if (potVal < tbl_waveformSelectionThreshold[i]) {
      pos = i;
      break;
    }
  }

  CONSTRAIN(pos, 0, 4);

  if (pos <= 0) {
    random_waveform_index_[id] = 0;
    bank_[id] = BANK_CLASSIC;
  }
  else {
    random_waveform_index_[id] = pos - 1;
    bank_[id] = BANK_RANDOM;
  }
}

void Ui::clearZoomSettings()
{
  for (int i=0; i<4; i++) {
    pot_fine_value_[i] = UINT16_MAX / 2;
    pot_phase_value_[i] = UINT16_MAX;
    pot_level_value_[i] = UINT16_MAX;
    pot_atten_value_[i] = UINT16_MAX;
  }
}

void Ui::clearAllHiddenSettings()
{
  for (int i=0; i<4; i++) {
    random_waveform_index_[i] = 0;
    bank_[i] = BANK_CLASSIC;
  }

  clearZoomSettings();
}

void Ui::gotoNormalModeWithCatchupAndSaving()
{
  for (int i=0; i<4; i++)
    if (abs(pot_value_[i] - pot_coarse_value_[i]) > kCatchupThreshold) {
      catchup_state_[i] = true;
    }

  mode_ = UI_MODE_NORMAL;
  storage.ParsimoniousSave(&feat_mode_, SETTINGS_SIZE, &version_token_);
}

void Ui::DoEvents() {
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    if (e.control_type == CONTROL_SWITCH) {
      if (e.data == 0) {
        OnSwitchPressed(e);
      } else {
        OnSwitchReleased(e);
      }
    } else if (e.control_type == CONTROL_POT) {
      OnPotChanged(e);
    }
  }
  if (queue_.idle_time() > 500) {
    queue_.Touch();
  }
}

}  // namespace batumi
