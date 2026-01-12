#pragma once

#include "esphome.h"
#include "esphome/components/i2s_audio/i2s_audio.h"
#include <driver/i2s.h>
#include <cmath>

namespace rotary {

class VintageToneGenerator : public esphome::Component {
 public:
  esphome::i2s_audio::I2SAudioComponent *bus_;
  bool active = false; 
  float frequency = 425.0f;
  float amplitude = 0.5f;   
  float phase = 0.0f;
  
  // Pulsing Logic (Tone)
  bool is_pulsing = false; 
  uint32_t pattern_on_ms = 1000;
  uint32_t pattern_off_ms = 4000;
  uint32_t pattern_start = 0;

  // Vintage Effects
  float noise_amplitude = 0.01f; 
  bool wobble_enabled = true;
  float wobble_phase = 0.0f;
  
  // Hard Click / Impulse
  int click_samples_remaining = 0;
  float click_amplitude = 0.8f;

  VintageToneGenerator(esphome::i2s_audio::I2SAudioComponent *bus) : bus_(bus) {}

  void setup() override {
      if (this->bus_ == nullptr) {
          ESP_LOGE("vintage_tone", "I2S Bus pointer is NULL!");
          this->mark_failed();
      } else {
          ESP_LOGI("vintage_tone", "Vintage Tone Generator Setup Complete. Bus: %p", this->bus_);
      }
  }

  void start_tone(float freq, bool pulse = false) {
    this->frequency = freq;
    this->active = true;
    this->is_pulsing = pulse;
    this->pattern_start = millis();
    this->phase = 0.0f;
    this->amplitude = 0.5f; // Reset standard amplitude
    this->noise_amplitude = 0.01f; // Reset standard background noise
    trigger_click();
  }

  void start_white_noise(float noise_amp) {
      this->active = true;
      this->is_pulsing = false;
      this->amplitude = 0.0f; // Silence tone
      this->noise_amplitude = noise_amp; // Set requested noise level
  }

  void stop_tone() {
    this->active = false;
  }
  
  void set_pulse_timing(uint32_t on, uint32_t off) {
      pattern_on_ms = on;
      pattern_off_ms = off;
  }

  void trigger_click() {
      click_samples_remaining = 160; 
  }

  void loop() override {
      if (this->active || click_samples_remaining > 0) {
          // Generate a small chunk (10ms = 160 samples)
          int16_t buffer[160];
          size_t samples_to_gen = 160;
          
          bool output_enabled = true;
          if (is_pulsing) {
             uint32_t elapsed = (millis() - pattern_start) % (pattern_on_ms + pattern_off_ms);
             if (elapsed > pattern_on_ms) output_enabled = false;
          }
          
          bool has_content = false;
          
          for (size_t i = 0; i < samples_to_gen; i++) {
              float val = 0.0f;
              
              if (active && output_enabled) {
                  has_content = true;
                  float current_freq = frequency;
                  if (wobble_enabled) {
                      current_freq += sin(wobble_phase * 2.0f * M_PI) * 2.0f;
                      wobble_phase += 0.5f/16000.0f;
                      if (wobble_phase >= 1.0f) wobble_phase -= 1.0f;
                  }
                  
                  val += sin(phase * 2.0f * M_PI) * amplitude;
                  phase += current_freq / 16000.0f;
                  if (phase >= 1.0f) phase -= 1.0f;
                  
                  float noise = ((float)(rand() % 100) / 50.0f) - 1.0f;
                  val += noise * noise_amplitude;
              }
              
              if (click_samples_remaining > 0) {
                  has_content = true;
                  float click_val = ((float)(rand() % 100) / 50.0f) - 1.0f;
                  val += click_val * click_amplitude;
                  click_samples_remaining--;
              }
              
              if (val > 1.0f) val = 1.0f;
              if (val < -1.0f) val = -1.0f;
              buffer[i] = (int16_t)(val * 32767.0f);
          }
          
          if (has_content) {
             if (this->bus_ != nullptr) { // Re-enabled I2S WRITE
                size_t bytes_written = 0;
                // Try writing to I2S Port 0 (Handset) with non-blocking timeout first to prevent UI freezing
                // Usage of port 0 matches the order of definition in YAML usually. 
                // Using 10 ticks timeout to avoid dropping too many samples but not block.
                
                // Note: On ESP32-S3 with Arduino 2.0.x / 3.0.x (IDF 4.4/5.1), i2s_write is available via driver/i2s.h
                // We use I2S_NUM_0 as a best guess since i2s_bus_handset is the first one.
                esp_err_t err = i2s_write(I2S_NUM_0, buffer, samples_to_gen * sizeof(int16_t), &bytes_written, 10 / portTICK_PERIOD_MS);
                
                if (err != ESP_OK) {
                     // Check for common errors to debug "repair" process
                     // ESP_LOGW("vintage_tone", "I2S Write Error: %d", err);
                }
             }
          }
      }
  }
};

} // namespace rotary
