#pragma once

#include "esphome.h"
#include <string>
#include <cmath>
#include <vector>

namespace rotary {

/**
 * Sends a command to the DY-SV17F MP3 Player via UART.
 * Protocol: AA <CMD> <LEN> <DATA...> <CHECKSUM>
 */
void play_mp3_file(esphome::uart::UARTComponent *uart, int file_num) {
    uint8_t high = (file_num >> 8) & 0xFF;
    uint8_t low = file_num & 0xFF;
    // AA 07 02 High Low
    std::vector<uint8_t> cmd = {0xAA, 0x07, 0x02, high, low};
    uint8_t sum = 0;
    for (uint8_t b : cmd) sum += b;
    
    uart->write_array(cmd);
    uart->write_byte(sum);
}

void set_mp3_volume(esphome::uart::UARTComponent *uart, float volume_val) {
    uint8_t vol = (uint8_t)volume_val;
    // AA 13 01 Vol
    std::vector<uint8_t> cmd = {0xAA, 0x13, 0x01, vol};
    uint8_t sum = 0;
    for (uint8_t b : cmd) sum += b;

    uart->write_array(cmd);
    uart->write_byte(sum);
}

/**
 * Calculates battery level percentage from voltage.
 * Linear mapping: 3.0V = 0%, 4.2V = 100%
 */
float get_battery_level(float voltage) {
    if (std::isnan(voltage)) return NAN;
    float level = (voltage - 3.0) / 1.2 * 100.0;
    if (level < 0) return 0.0f;
    if (level > 100) return 100.0f;
    return level;
}

/**
 * Checks if battery is below threshold.
 */
bool is_battery_low(float voltage, float threshold_percent) {
    if (std::isnan(voltage)) return false;
    float threshold_volts = 3.0 + (threshold_percent / 100.0) * 1.2;
    return voltage < threshold_volts;
}

// =========================================================================
//  Vintage Speaker (I2S Speaker + Tone Generator + Noise Injection)
// =========================================================================
#include "esphome/components/i2s_audio/speaker/i2s_audio_speaker.h"

class VintageSpeaker;
VintageSpeaker *global_tone_source = nullptr;

class VintageSpeaker : public esphome::i2s_audio::I2SAudioSpeaker {
 public:
  bool active = false; // Tone Generator Active?
  float frequency = 425.0f;
  float amplitude = 0.5f;   
  float phase = 0.0f;
  
  // Pulsing Logic (Tone)
  bool is_pulsing = false; 
  uint32_t pattern_on_ms = 1000;
  uint32_t pattern_off_ms = 4000;
  uint32_t pattern_start = 0;

  // Vintage Effects
  float noise_amplitude = 0.01f; // Comfort Noise (Simulates Line Noise)
  bool wobble_enabled = true;
  float wobble_phase = 0.0f;
  
  // Hard Click / Impulse
  int click_samples_remaining = 0;
  float click_amplitude = 0.8f;

  VintageSpeaker() {
      global_tone_source = this;
  }

  void start_tone(float freq, bool pulse = false) {
    this->frequency = freq;
    this->active = true;
    this->is_pulsing = pulse;
    this->pattern_start = millis();
    this->phase = 0.0f;
    trigger_click();
  }

  void stop_tone() {
    this->active = false;
  }
  
  void set_pulse_timing(uint32_t on, uint32_t off) {
      pattern_on_ms = on;
      pattern_off_ms = off;
  }

  void trigger_click() {
      click_samples_remaining = 160; // 10ms
  }

  // Override the play method to intercept TTS audio and add noise
  // Note: This method is called by the Voice Assistant to push audio chunks.
  // The buffer duration depends on the chunk size.
  size_t play(const void *data, size_t length) override {
      int16_t *pcm_data = (int16_t *)data;
      size_t samples = length / sizeof(int16_t);
      
      // We modify the buffer in-place before sending it to I2S hardware.
      // NOTE: This modifies the source definition data if it's not a copy, 
      // but usually 'play' receives a buffer we can read. 
      // Safest is to modify a temp buffer if const, but usually we can const_cast if needed,
      // OR we just write our own modified data to the FIFO.
      // I2SAudioSpeaker::play writes to a RingBuffer usually or directly calls write.
      // Let's assume we can intercept by NOT calling super::play directly but doing what it does?
      // No, too risky.
      // Easier: The data passed in here is from the source (Microphone/Media).
      // If we want to ADD logic on top, we should process 'data'.
      // Since it is 'const void*', we should copy it to a local buffer, modify it, and pass it to super.
      
      std::vector<int16_t> temp_buffer(samples);
      memcpy(temp_buffer.data(), data, length);
      
      for (size_t i = 0; i < samples; i++) {
          float sample_val = (float)temp_buffer[i] / 32768.0f;
          
          // Add Comfort Noise
          if (noise_amplitude > 0) {
            float noise = ((float)(rand() % 100) / 50.0f) - 1.0f; 
            sample_val += noise * noise_amplitude;
          }
          
          if (sample_val > 1.0f) sample_val = 1.0f;
          if (sample_val < -1.0f) sample_val = -1.0f;
          
          temp_buffer[i] = (int16_t)(sample_val * 32767.0f);
      }
      
      return esphome::i2s_audio::I2SAudioSpeaker::play(temp_buffer.data(), length);
  }
  
  // Create a loop hook to generate Tones when NO streaming audio is playing
  void loop() override {
      // Logic: If Tone is Active AND the Speaker is NOT currently playing a Stream, 
      // we need to feed the I2S bus.
      // Problem: component::loop runs on main thread. Audio needs to be fed fast.
      // I2S Component usually handles feeding from RingBuffer.
      // If we want to generate Tone, we should push to the RingBuffer.
      
      // Simplification: We rely on the fact that Voice Assistant stops feeding when silent.
      // If we are 'active' (Dial Tone / Ringback), we inject frames.
      // CAUTION: Conflict if 'play' is called while we act?
      // We'll prioritize 'play'. If 'has_buffered_data()', we don't inject tone.
      
      if (this->active || click_samples_remaining > 0) {
          // Check if I2S is busy
          // Note: has_buffered_data() might not be exposed publicly on all implementations.
          // Let's blindly push small chunks if we are supposed to make noise.
          
          // Generate a small chunk (e.g. 10ms = 160 samples)
          int16_t buffer[160];
          size_t samples_to_gen = 160;
          
          // Fill logic (copied from previous SineToneSource)
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
                      wobble_phase += 0.5f / 16000.0f; // Approx update
                      if (wobble_phase >= 1.0f) wobble_phase -= 1.0f;
                  }
                  
                  val += sin(phase * 2.0f * M_PI) * amplitude;
                  phase += current_freq / 16000.0f;
                  if (phase >= 1.0f) phase -= 1.0f;
                  
                  // Noise in Tone
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
          
          // Push to I2S if we generated something relevant OR if we just want to keep the line alive?
          // If we write '0's, we silence the line.
          // If we write nothing, I2S might starve/repeat.
          // If we are Active, we MUST write.
          if (has_content) {
             esphome::i2s_audio::I2SAudioSpeaker::play(buffer, samples_to_gen * sizeof(int16_t));
          }
      }
      
      esphome::Component::loop();
  }
};


} // namespace rotary
