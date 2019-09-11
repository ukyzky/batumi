// Copyright 2018 Takashi Matsuura.
// Copyright 2015 Matthias Puech.
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
// User Interface

#ifndef BATUMI_UI_H_
#define BATUMI_UI_H_

#include "stmlib/stmlib.h"

#include "stmlib/ui/event_queue.h"

#include "drivers/adc.h"
#include "drivers/leds.h"
#include "drivers/switches.h"

#include "lfo.h"

namespace batumi {

const uint8_t kFinePotDivider = 8;

enum FeatureMode {
  FEAT_MODE_FREE,
  FEAT_MODE_QUAD,
  FEAT_MODE_PHASE,
  FEAT_MODE_DIVIDE,
  FEAT_MODE_LAST
};

enum UiMode {
  UI_MODE_SPLASH,
  UI_MODE_NORMAL,
  UI_MODE_ZOOM,
  UI_MODE_WAVEBANK_SELECT,
  UI_MODE_SPLASH_FOR_WAVEBANK_SELECT
};

enum WaveBank {
  BANK_CLASSIC,
  BANK_RANDOM,
  BANK_LAST
};

class Ui {
 public:
  Ui() {
    classic_waveform_index_[0] = -1;
    classic_waveform_index_[1] = -1;
    classic_waveform_index_[2] = -1;
    classic_waveform_index_[3] = -1;
  }
  ~Ui() { }
  
  void Init(Adc *adc);
  void Poll();
  void DoEvents();
  void FlushEvents();

  inline uint16_t coarse(uint8_t channel) {
    return pot_coarse_value_[channel];
  }

  inline int16_t fine(uint8_t channel) {
    return pot_fine_value_[channel] - 32768;
  }

  inline uint16_t phase(uint8_t channel) {
    return pot_phase_value_[channel];
  }

  inline uint16_t level(uint8_t channel) {
    return pot_level_value_[channel];
  }

  inline uint16_t atten(uint8_t channel) {
    return pot_atten_value_[channel];
  }

  inline FeatureMode feat_mode() const { return feat_mode_; }
  inline UiMode mode() const { return mode_; }
  inline WaveBank bank(uint8_t channel) const { return bank_[channel]; }
  inline uint8_t shape() const {
    return (switches_.pressed(2) << 1) | switches_.pressed(1);
  }
  inline uint8_t shape(uint8_t channel) const {
    return (classic_waveform_index_[channel] == -1)
      ? shape()
      : classic_waveform_index_[channel];
  }
  inline uint8_t random_waveform_index(uint8_t channel) const
  {
    return random_waveform_index_[channel];
  }
  inline bool sync_mode() const {
    return switches_.pressed(0);
  }

 private:
  void OnSwitchPressed(const stmlib::Event& e);
  void OnSwitchReleased(const stmlib::Event& e);
  void OnPotChanged(const stmlib::Event& e);

  void selectRandomWaveformFromPot(uint16_t id, int32_t val);
  void selectWaveformFromPot(uint16_t id, int32_t val);
  void clearZoomSettings();
  void clearAllHiddenSettings();
  void gotoNormalModeWithCatchupAndSaving();

  uint16_t pot_value_[4];
  uint16_t pot_filtered_value_[4];
  uint16_t pot_coarse_value_[4];
  uint8_t last_touched_pot_;
  uint32_t press_time_[kNumSwitches];
  bool detect_very_long_press_[kNumSwitches];
  bool detect_clear_settings_long_press_[kNumSwitches];
  bool catchup_state_[4];

  int32_t animation_counter_;

  stmlib::EventQueue<32> queue_;

  Leds leds_;
  Switches switches_;
  Adc *adc_;
  UiMode mode_;

  FeatureMode feat_mode_;
  uint8_t padding[3];
  WaveBank bank_[4];
  int8_t classic_waveform_index_[4];
  uint8_t random_waveform_index_[4];
  uint16_t pot_fine_value_[4];
  uint16_t pot_level_value_[4];
  uint16_t pot_atten_value_[4];
  uint16_t pot_phase_value_[4];

  enum SettingsSize {
    SETTINGS_SIZE = sizeof(feat_mode_) +
    sizeof(bank_) +
    sizeof(random_waveform_index_) +
    sizeof(pot_fine_value_) +
    sizeof(pot_phase_value_) +
    sizeof(pot_level_value_) +
    sizeof(pot_atten_value_) +
    sizeof(padding)
  };

  uint16_t version_token_;

  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace batumi

#endif  // BATUMI_UI_H_
