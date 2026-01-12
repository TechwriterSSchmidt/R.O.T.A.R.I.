#pragma once

#include "esphome.h"
#include "esphome/components/i2s_audio/i2s_audio.h"
#include "esphome/components/speaker/speaker.h"

namespace mux_speaker {

class MuxSpeaker : public esphome::speaker::Speaker, public esphome::Component {
 public:
  esphome::i2s_audio::I2SAudioComponent *bus_handset_{nullptr};
  esphome::i2s_audio::I2SAudioComponent *bus_base_{nullptr};
  esphome::switch_::Switch *mode_switch_{nullptr};
  
  i2s_port_t port_handset_ = I2S_NUM_0;
  i2s_port_t port_base_ = I2S_NUM_1;

  void setup() override {
    ESP_LOGCONFIG("mux_speaker", "Setting up Mux Speaker...");
  }
  
  bool has_buffered_data() const override {
      return false;
  }

  void set_handset_bus(esphome::i2s_audio::I2SAudioComponent *bus) { bus_handset_ = bus; }
  void set_base_bus(esphome::i2s_audio::I2SAudioComponent *bus) { bus_base_ = bus; }
  void set_mode_switch(esphome::switch_::Switch *sw) { mode_switch_ = sw; }

  void start() override {
    this->state_ = esphome::speaker::STATE_RUNNING;
  }

  void stop() override {
    this->state_ = esphome::speaker::STATE_STOPPED;
  }

  size_t play(const uint8_t *data, size_t length) override {
    if (this->is_failed()) return 0;
    
    bool use_base = false;
    if (mode_switch_ != nullptr) {
        if (mode_switch_->state) {
            use_base = true;
        }
    }

    i2s_port_t target_port = use_base ? port_base_ : port_handset_;
    
    size_t bytes_written = 0;
    esp_err_t err = i2s_write(target_port, data, length, &bytes_written, 100 / portTICK_PERIOD_MS);
    
    if (err != ESP_OK) {
         return 0;
    }
    
    return bytes_written;
  }
};

} // namespace mux_speaker
