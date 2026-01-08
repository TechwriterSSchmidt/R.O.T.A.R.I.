# R.O.T.A.R.I.
The Robotic Operative Telecommunication & Artificial Response Interface. An ESPHome-based firmware that transforms a vintage rotary phone into a modern, portable voice assistant for Home Assistant.

## Features

*   **Authentic "Vintage" Experience:**
    *   **Dial Tone:** Generates a 425Hz sine wave (plus authentic wobbling and background noise) when the handset is lifted.
    *   **Electronic Clicking:** Generates click sounds directly in the handset while dialing, simulating mechanical contacts.
    *   **Operator Mode:** Dialing **'0'** simulates calling the operator. Depending on how long since the last call, you get a "Standard" or "Annoyed" response before the AI connects.
    *   **Busy Signal:** Plays a busy tone after the call ends logic.
    *   **Comfort Noise:** Injects subtle static noise during TTS playback to simulate an analog line.
*   **Push-to-Talk / Dial-to-Talk:** Picking up plays the dial tone. Dialing '0' connects to the Voice Assistant.
*   **Visual Feedback:** Keys light up Green when listening, Blue when processing, and Red on error.
*   **Acoustic Feedback:** Ringtones via DY-SV17F MP3 module (Base) and synthesized tones via I2S (Handset).
*   **Rotary Dial:** sends events to Home Assistant for automation. Includes robust software debouncing (15ms) for reliable digit detection.
*   **Speed Dial Buttons:** 4 configurable buttons.
    *   **Button 1:** System controls (Battery Check & Do Not Disturb).
    *   **Buttons 2-4:** Send events to Home Assistant for custom actions.
*   **Sensors:** supports BME280 (Temp/Hum/Press) and LD2410 (Radar Presence).
*   **Room Tracking:** Bluetooth proxy enabled for room presence via Bermuda/ESPresense.
*   **Find My Phone:** Visual alarm mode if device is misplaced.
*   **Battery Power:** Voltage monitoring with configurable low-battery thresholds.
*   **Watchdog:** Auto-reboot on connection loss.

## Hardware Support

*   **Controller:** LilyGO T7-S3 (ESP32-S3)
*   **Audio Base:** I2S Bus B -> DY-SV17F / Amplifier for loud Ringing.
*   **Audio Handset:** I2S Bus A -> MAX98357A (Speaker) + INMP441 (Mic).
*   **Feedback:** DRV2605 LRA Haptic Driver (for silent ringing).
*   **MP3:** DY-SV17F UART Module (Ringtones).

## Project Structure

This project has been refactored for maintainability:

*   **`rotary_phone_agent.yaml`**: Main configuration file.
    *   **Pin Definitions**: All GPIO assignments are defined in the `substitutions` block at the top.
    *   **Settings**: Timings and thresholds are available as variables/globals.
*   **`src/rotary_helpers.h`**: Custom C++ helper functions (Battery logic, UART commands, Tone colors).
*   **`components/vintage_tone_generator`**: Custom External Component. Handles the generation of synths (Dial/Busy tones), clicks, and noise injection into the I2S stream.
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
| **Buttons** | Button 1 | `pin_button_1` | 10 |
| | Button 2 | `pin_button_2` | 11 |
| | Button 3 | `pin_button_3` | 12 |
| | Button 4 | `pin_button_4` | 21 |

## LED Signaling & Button Functions

The 4 buttons on the device provide local control and visual feedback via the 4-LED strip.

| Button | Function | LED Feedback | Description |
| :--- | :--- | :--- | :--- |
| **1** | **System** | **Bar Graph** (Long Press) | **Short Press:** Toggle "Do Not Disturb" (DND).<br>**Long Press (2s):** Show Battery Level. |
| **2** | **Custom** | **None** | Sends event `button: "2"` to HA. |
| **3** | **Custom** | **None** | Sends event `button: "3"` to HA. |
| **4** | **Custom** | **None** | Sends event `button: "4"` to HA. |

### Voice Assistant Feedback

| State | LED Color | Animation |
| :--- | :--- | :--- |
| **Listening** | 🟢 Green | Constant |
| **Thinking** | 🔵 Blue | Blinking |
| **Speaking** | ⚫ Off | (Off to allow conversation) |
| **Error** | 🔴 Red | Blinking |
| **Muted** | 🟣 Purple | Pulsing (on LED 2) |

## Required Sound Files

The firmware controls a DY-SV17F MP3 module which requires specific filenames on the internal 4mb flash (FAT32 formatted).

| Filename | Description | Usage |
| :--- | :--- | :--- |
| **`00001.mp3`** | **Ringtone** | Plays through the **Base Speaker** when the "Ring Phone" script is triggered. |

> **Note:** The "Click/Tick" sounds for dialing and the "Hook Flash" signal are generated electronically in the handset.

## Getting Started

1.  **Secrets:** Ensure you have a `secrets.yaml` with your `wifi_ssid` and `wifi_password`.
2.  **Build:** Run `esphome run rotary_phone_agent.yaml` to compile and upload.
3.  **Adjust:** Use the `globals` in the YAML to tweak Ringtone Duration or Vibration intensity.

## Maintenance

*   **Battery Thresholds:** Adjusted via "Low Battery Threshold" number entity in HA/Web UI.
*   **Updates:** To update the ESPHome version, simply run the build command again.



