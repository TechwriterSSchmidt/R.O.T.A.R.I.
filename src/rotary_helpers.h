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

} // namespace rotary
