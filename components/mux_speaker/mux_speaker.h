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
  
  // Volumes (0.0 to 1.0+)
  float volume_handset_ = 1.0f;
  float volume_base_ = 1.0f;

  void setup() override {
    ESP_LOGCONFIG("mux_speaker", "Setting up Mux Speaker...");
  }
  
  bool has_buffered_data() const override {
      return false;
  }

  void set_handset_bus(esphome::i2s_audio::I2SAudioComponent *bus) { bus_handset_ = bus; }
  void set_base_bus(esphome::i2s_audio::I2SAudioComponent *bus) { bus_base_ = bus; }
  void set_mode_switch(esphome::switch_::Switch *sw) { mode_switch_ = sw; }
  
  void set_handset_volume(float v) { volume_handset_ = v; }
  void set_base_volume(float v) { volume_base_ = v; }

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
    float volume = use_base ? volume_base_ : volume_handset_;

    // Volume Scaling (Soft-Volume)
    // We assume 16-bit signed PCM data (standard for ESPHome voice)
    // We need to copy data to apply volume, or modify in place?
    // 'data' is const, so we must copy. 
    // Usually chunks are small (e.g. 512 bytes), so stack allocation might be risky if large.
    // Length is typically variable.
    
    // Create a temporary buffer on heap/stack
    int16_t *samples = (int16_t *)data;
    size_t num_samples = length / 2;
    
    // We use a static buffer to avoid heap fragmentation, assuming max chunk size
    // or just allocate. Allocation is safer for varying sizes.
    int16_t *buffer = new int16_t[num_samples];
    
    for (size_t i = 0; i < num_samples; i++) {
        int32_t val = (int32_t)(samples[i] * volume);
        // Clip
        if (val > 32767) val = 32767;
        if (val < -32768) val = -32768;
        buffer[i] = (int16_t)val;
    }
    
    size_t bytes_written = 0;
    esp_err_t err = i2s_write(target_port, (const char*)buffer, length, &bytes_written, 100 / portTICK_PERIOD_MS);
    
    delete[] buffer;
    
    if (err != ESP_OK) {
         return 0;
    }
    
    return bytes_written;
  }
};

} // namespace mux_speaker
