#pragma once

#include "esphome.h"
#include <string>
#include <cmath>
#include <vector>

namespace rotary {



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
