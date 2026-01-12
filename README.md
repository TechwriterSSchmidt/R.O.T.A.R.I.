# R.O.T.A.R.I.
The **R**obotic **O**perative **T**elecommunication & **A**rtificial **R**esponse **I**nterface. An ESPHome-based firmware that transforms a vintage rotary phone into a modern, portable voice assistant for Home Assistant.

## Support my projects

If you like this project, consider a tip. Your tip motivates me to continue developing useful stuff for the DIY community. Thank you very much for your support!

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/D1D01HVT9A)

## Table of Contents
*   [Features](#features)
*   [Operational Logic](#operational-logic)
*   [Hardware Support & Pinout](#hardware-support--pinout)
*   [Project Structure](#project-structure)
*   [Audio Configuration](#audio-configuration)
*   [LED Signaling & Button Functions](#led-signaling--button-functions)
*   [Getting Started](#getting-started)
*   [Home Assistant Integration](#home-assistant-integration)
*   [Maintenance](#maintenance)
*   [Release Notes](RELEASE_NOTES.md)

## Features

| Category | Feature | Description |
| :--- | :--- | :--- |
| **Vintage Experience** | **Dial Tone** | Generates a 425Hz sine wave (plus authentic wobbling and background noise) when the handset is lifted. |
| | **Electronic Clicking** | Generates click sounds directly in the handset while dialing, simulating mechanical contacts. |
| | **Operator Mode** | Dialing **'0'** simulates calling the operator. Context-aware reactions ("Standard" vs "Annoyed"). |
| | **Busy Signal** | Plays a busy tone after the call ends. |
| | **Comfort Noise** | Injects subtle static noise during TTS playback to simulate an analog line. |
| **Interaction** | **Push-to-Talk** | Picking up plays the dial tone. Dialing '0' connects to the Voice Assistant. |
| | **Visual Feedback** | Keys light up Green (Listening), Blue (Processing), and Red (Error). |
| | **Acoustic Feedback** | Ringtones via DY-SV17F MP3 module (Base) and synthesized tones via I2S (Handset). |
| **Input** | **Rotary Dial** | Sends events to Home Assistant. Robust software debouncing (15ms) for reliable digit detection. |
| | **Speed Dial Buttons** | **Button 1:** System (Battery/DND). **Buttons 2-4:** Custom HA events. |
| **Smart Helpers** | **Sensors** | Supports BME280 (Temp/Hum/Press). |
| | **Room Tracking** | Best practice: Place a physical Bluetooth Beacon (AirTag, Tile) inside the case for stable room tracking via [Bermuda Integration](https://community.home-assistant.io/t/bermuda-bluetooth-ble-room-presence-and-tracking-custom-integration/625780). |
| | **Find My Phone** | Visual alarm mode if device is misplaced. |
| | **Battery Power** | Voltage monitoring with configurable low-battery thresholds. |
| | **Watchdog** | Auto-reboot on connection loss. `Safe Mode` enabled for crash recovery. |
| **Advanced Audio** | **Smart Routing** | Dynamic switching between Handset (Private) and Base Speaker (Public) based on mode. |
| | **Auto Gain** | Microphone automagically adjusts volume level (0dBFS) for whispering or distance talking. |

## Operational Logic

The device differentiates between two main usage modes based on the handset state:

### 1. Call Mode (Handset Lifted)
*   **Trigger:** Lift the handset (Off-Hook) or activate Speakerphone.
*   **Behavior:** Full vintage simulation. You hear dial tones, dialing clicks, and connection noise.
*   **Actions:** All dialed numbers (including '0') generate an `esphome.rotary_dial` event in Home Assistant. 
    *   **Logic is now fully controlled by Home Assistant.** 
    *   Create an automation in HA to trigger the voice assistant service `esphome.rotary_phone_start_listening` when specific numbers are dialed.

### 2. Command Mode (Handset on Cradle)
*   **Trigger:** Rotate the dial while the phone is On-Hook.
*   **Behavior:** Silent operation. No tones are played.
*   **Actions:**
    *   Dial `1..9`: Sends instant events to Home Assistant. Ideal for scenarios like "Dial 1 to toggle lights" or "Dial 5 for Cinema Mode".

### 3. Incoming Call Mode (Ringing)
*   **Accept:** Lift handset. Ringing stops immediately.
*   **Reject:** Lift handset and immediately place it back on the cradle (hang up).

## Hardware Support & Pinout

**Target Board:** WEMOS S3 Pro (ESP32-S3)

Since the physical PCB schematic is separate, use this reference for wiring the components to the ESP32-S3.

| Component | Pin Function | GPIO | Notes |
| :--- | :--- | :--- | :--- |
| **I2S Handset** | DIN (Mic) | `GPIO 7` | INMP441 / similar |
| | DOUT (Speaker) | `GPIO 15` | MAX98357A / DAC |
| | BCLK | `GPIO 6` | Shared clocks possible |
| | LRCLK | `GPIO 5` | |
| **I2S Base** | DOUT (Speaker) | `GPIO 19` | Amplifier for separate base speaker for authentic Ringing |
| | BCLK | `GPIO 18` | |
| | LRCLK | `GPIO 16` | |
| **Rotary Dial** | Pulse Switch | `GPIO 14` | Connect to GND, Input Pullup |
| | Active Switch | `GPIO 13` | Connect to GND, Input Pullup |
| **Phone Hook** | Hook Switch | `GPIO 1` | Open = Off-Hook |
| **Battery** | Voltage | `GPIO 4` | On-board divider (Factor 2.0) |
| **LEDs** | LED 1 (Status) | `GPIO 2` | WS2812B / Neopixel |
| | LED 2 | `GPIO 3` | |
| | LED 3 | `GPIO 20` | |
| | LED 4 | `GPIO 17` | |
| **I2C Bus** | SDA | `GPIO 8` | For DRV2605 / Sensors |
| | SCL | `GPIO 9` | |
| **Accessory** | Buttons 1-3 | `GPIO 10-12` | Speed dials |
| | Button 4 | `GPIO 21` | |

## Project Structure

This project has been refactored for maintainability:

*   **`rotary_phone_agent.yaml`**: Main configuration file.
    *   **Pin Definitions**: All GPIO assignments are defined in the `substitutions` block.
*   **`components/mux_speaker`**: **[NEW]** Custom component. handles logic to route audio streams and maintain separate volume levels for Handset vs Base.
*   **`components/vintage_tone_generator`**: Custom Component. Generates low-latency synths (Dial/Busy tones), clicks, and noise injection.
*   **`src/rotary_helpers.h`**: Lightweight helper functions (Battery logic, tone colors).

## Audio Configuration

R.O.T.A.R.I. features advanced audio routing that allows it to function as both a private handset and a room-filling speakerphone.

### Volume & Sensitivity Control
You can fine-tune the audio experience directly from Home Assistant. All settings are **persistent** and saved to the ESP's flash memory.

| Setting | Function | Recommended Value |
| :--- | :--- | :--- |
| **Volume Handset** | Playback volume for the ear speaker. | 40-80% |
| **Volume Base** | Playback volume for the speakerphone/ringtones. | 50-100% |
| **Auto Gain** | Automatic microphone gain control (target 0dBFS). | Enabled |

*Note: The system creates `number` entities in Home Assistant for these values.*

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
| **Listening** | 🟢 Green | Constant (100% Green) |
| **Thinking** | 🟠 Amber | Pulsing |
| **Speaking** | 🔵 Blue | Constant |
| **Error** | 🔴 Red | Pulsing |
| **DND** | 🟣 Purple | Constant |
| **Find Phone** | 🟡 Yellow | Pulsing |
| **Hands-Free** | ⚪ Warm White | Constant |
| **Bootup** | ⚪ Warm White | All LEDs Pulsing |


## Acoustic Signals

The device generates synthetic vintage call progress tones directly in the handset earpiece.

| Signal | Sound Profile | Trigger Condition |
| :--- | :--- | :--- |
| **Dial Tone** | 425Hz Sine + Analog Hiss + Pitch Wobble | Handset lifted (Off-Hook). Stops when dialing begins. |
| **Dialing Noise** | Low-volume White Noise (Static) | While the rotary dial is in motion (Active). |
| **Impulse Click** | Sharp Mechanical Click | Per dial pulse (1-10 clicks per digit). |
| **Pickup Click** | Single "Pop" | After dialing is complete, before Voice Assistant (TTS) responds. |
| **Busy Tone** | 425Hz Pulse (480ms ON / 480ms OFF) | After the Voice Assistant finishes speaking (Call ended). |
| **Ringing** | Mechanical Bell or Haptic Vibration | Incoming call script triggered. |

## Getting Started

1.  **Secrets:** Ensure you have a `secrets.yaml` with your `wifi_ssid` and `wifi_password`.
2.  **Build:** Run `esphome run rotary_phone_agent.yaml` to compile and upload.
3.  **Adjust:** Use the `globals` in the YAML to tweak Ringtone Duration or Vibration intensity.

## Home Assistant Integration

The device is designed to be a "dumb input, smart output" terminal. It acts as a physical interface for your Home Assistant logic.

### Event Reference

Trigger your automations using these Events (available in HA under `Developer Tools -> Events` to test):

| Trigger Source | Event Name (`event_type`) | Data Payload (`trigger.event.data`) | Description |
| :--- | :--- | :--- | :--- |
| **Rotary Dial** | `esphome.rotary_dial` | `number` (e.g., "110", "1") | Fired when a number is dialed and the timeout expires. |
| **Buttons 2-4** | `esphome.rotary_phone_button` | `button` (e.g., "2", "3", "4") | Fired when one of the custom buttons is pressed. |
| **Hook Flash** | `esphome.rotary_phone_flash` | - | Fired when the hook is tapped briefly (short press). |
| **Low Battery** | `esphome.rotary_phone_low_battery` | - | Fired once when the battery voltage drops below the threshold. |

### Example Audio Automation (The "Phonebook")

Instead of saving numbers on the device, handle them in Home Assistant:

```yaml
# Home Assistant Automation Example
alias: "Rotary Phone Logic"
mode: queued
trigger:
  - platform: event
    event_type: esphome.rotary_dial
action:
  - choose:
      # Option 1: Call Mama
      - conditions: "{{ trigger.event.data.number == '1' }}"
        sequence:
          - service: tts.google_say
            data:
              entity_id: media_player.rotary_phone_handset_speaker
              message: "Calling Mama..."
          # Add notify / VoIP calls here
      
      # Option 2: Local AI (Home Control)
      # Switch to "Local Pipeline" and listen
      - conditions: "{{ trigger.event.data.number == '42' }}"
        sequence:
          - service: select.select_option
            target:
              entity_id: select.rotary_phone_pipeline
            data:
              option: "Home Assistant" # Name of your Local Pipeline
          - service: esphome.rotary_phone_start_listening

      # Option 3: Cloud AI (ChatGPT / DeepSeek)
      # Switch to "Cloud Pipeline" and listen
      - conditions: "{{ trigger.event.data.number == '666' }}"
        sequence:
          - service: select.select_option
            target:
              entity_id: select.rotary_phone_pipeline
            data:
              option: "LLM Cloud Pipeline" # Name of your Cloud Pipeline
          - service: esphome.rotary_phone_start_listening
      
      # Option 4: Activate "Cinema Mode" (Command Mode Example)
      - conditions: "{{ trigger.event.data.number == '5' }}"
        sequence:
          - service: scene.turn_on
            target: { entity_id: scene.living_room_cinema }
            
    # Default: Unknown Number
    default:
      - service: tts.google_say
        data:
          entity_id: media_player.rotary_phone_handset_speaker
          message: "The number {{ trigger.event.data.number }} is not in service."
```

### 4. Answering Machine (Home Assistant Logic)

Since the R.O.T.A.R.I. phone is fully integrated into Home Assistant, you can recreate a **classic Answering Machine** (or "Voicemail") experience purely via automation. 
When you dial "1" (classic shortcut), Home Assistant will read out your critical notifications or calendar events using a synthesized voice, complete with authentic pauses.

#### Example Automation (YAML)
Add this to your `automations.yaml` or create a new automation in the GUI:

```yaml
alias: "Rotary Phone - Answering Machine"
description: "Reads notifications when '1' is dialed."
trigger:
  - platform: event
    event_type: esphome.rotary_dial
    event_data:
      number: "1"
condition: []
action:
  # 1. Visual Feedback: The Phone LEDs turn Blue via firmware automatically when playing media.
  
  # 2. Greeting (Authentic 70s Style)
  - service: tts.speak
    target:
      entity_id: tts.piper  # Or tts.google_en_com
    data:
      media_player_entity_id: media_player.rotary_phone_base_speaker
      message: >
        You have... {{ states('sensor.number_of_notifications') }} ... new messages.
        
        First message...
        
        {{ states('sensor.first_notification_content') }}
        
        ... Beep.

  # 3. Optional: Clear notifications after reading
  # - service: persistent_notification.dismiss_all
```

#### How it feels:
1.  **Dial "1"**: You hear the rotary click back.
2.  **Silence**: A short pause (processing).
3.  **Voice**: "You have... two... new messages." (Audio plays through the base speaker or handset if lifted).
4.  **LEDs**: While speaking, the LEDs glow **Blue** (handled by firmware `on_play` event).


## Maintenance

*   **Battery Thresholds:** Adjusted via "Low Battery Threshold" number entity in HA/Web UI.
*   **Updates:** To update the ESPHome version, simply run the build command again.

## Battery Life Estimates

Estimated runtimes based on active WiFi connection and average idle power consumption (~150mA for Standard, ~250mA for Audio).

| Battery Capacity | Standard Mode (Est.) | Performance Mode (Est.) |
| :--- | :--- | :--- |
| **1500 mAh** | ~8 Hours | ~4.5 Hours |
| **3000 mAh** | ~16 Hours | ~9 Hours |
| **5000 mAh** | ~26 Hours | ~16 Hours |

> **Standard Mode:** WiFi Connected, Voice Assistant Ready, occasional interaction.
> **Performance Mode:** Continuous audio playback (Music/TTS).

## Future Roadmap

*   **AI Answering Machine:** Integration for received AI voice messages.
*   **Enhanced Haptics:** More vibration patterns for different notifications.





