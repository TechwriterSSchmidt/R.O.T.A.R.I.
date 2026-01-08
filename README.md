# Rotary Phone Agent

An ESPHome-based firmware that transforms a vintage rotary phone into a modern, portable voice assistant for Home Assistant.

## Features

*   **Push-to-Talk:** Picking up the handset instantly starts the voice assistant (no wake word required).
*   **Visual Feedback:** Keys light up Green when listening, Blue when processing, and Red on error.
*   **Acoustic Feedback:** Authentic click sounds and ringtones via DY-SV17F MP3 module.
*   **Rotary Dial:** sends events to Home Assistant for automation.
*   **Speed Dial Buttons:** 4 configurable buttons for custom actions.
    *   **Button 1 (Long Press):** Toggles Microphone Mute. (LED 1 pulses purple). Hanging up resets mute.
*   **Sensors:** supports BME280 (Temp/Hum/Press) and LD2410 (Radar Presence).
*   **Room Tracking:** Bluetooth proxy enabled for room presence via Bermuda/ESPresense.
*   **Find My Phone:** Visual alarm mode if device is misplaced.
*   **Battery Power:** Voltage monitoring with configurable low-battery thresholds.
*   **Watchdog:** Auto-reboot on connection loss.

## Hardware Support

*   **Controller:** LilyGO T7-S3 (ESP32-S3)
*   **Audio:** Dual I2S Bus (Handset + Base Speaker)
*   **Feedback:** DRV2605 LRA Haptic Driver (for silent ringing)
*   **MP3:** DY-SV17F UART Module

## Project Structure

This project has been refactored for maintainability:

*   **`rotary_phone_agent.yaml`**: Main configuration file.
    *   **Pin Definitions**: All GPIO assignments are defined in the `substitutions` block at the top.
    *   **Settings**: Timings and thresholds are available as variables/globals.
*   **`src/rotary_helpers.h`**: Custom C++ helper functions (Battery logic, UART commands).
*   **`components/drv2605`**: Custom or patched components.

## Pinout Configuration

You can easily change the pin assignments in `rotary_phone_agent.yaml` under `substitutions`.

| Component | Function | Variable | Default GPIO |
| :--- | :--- | :--- | :--- |
| **I2S Handset** | LRCLK | `pin_i2s_handset_lrclk` | 5 |
| | BCLK | `pin_i2s_handset_bclk` | 6 |
| | DIN (Mic) | `pin_i2s_handset_din` | 7 |
| | DOUT (Spk) | `pin_i2s_handset_dout` | 15 |
| **I2S Base** | LRCLK | `pin_i2s_base_lrclk` | 16 |
| | BCLK | `pin_i2s_base_bclk` | 18 |
| | DOUT (Spk) | `pin_i2s_base_dout` | 19 |
| **Mechanics** | Hook | `pin_hook_switch` | 1 |
| | Dial Pulse | `pin_dial_pulse` | 14 |
| | Dial Active | `pin_dial_active` | 13 |
| **Peripherals** | LEDs | `pin_led_strip` | 2 |
| | Battery | `pin_battery_voltage` | 4 |
| | MP3 TX | `pin_uart_tx` | 43 |
| | MP3 RX | `pin_uart_rx` | 44 |

## Getting Started

1.  **Secrets:** Ensure you have a `secrets.yaml` with your `wifi_ssid` and `wifi_password`.
2.  **Build:** Run `esphome run rotary_phone_agent.yaml` to compile and upload.
3.  **Adjust:** Use the `globals` in the YAML to tweak Ringtone Duration or Vibration intensity.

## Maintenance

*   **Battery Thresholds:** Adjusted via "Low Battery Threshold" number entity in HA/Web UI.
*   **C++ Logic:** Core logic for battery and UART commands resides in `src/rotary_helpers.h`.
*   **Updates:** To update the ESPHome version, simply run the build command again.
