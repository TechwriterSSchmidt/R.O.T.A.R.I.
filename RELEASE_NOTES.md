# Release Notes

## [v2026.1.0] - 2026-01-12 - The "Voice Perfection" Update

### 🎤 Audio & Voice
*   **Auto-Gain Control:** Replaced complex manual gain sliders with ESPHome's native `auto_gain`. The microphone now automatically adjusts to 0dBFS, handling whisper-quiet inputs and room-scale commands seamlessly.
*   **MuxSpeaker Component:** Introduced a custom C++ component (`mux_speaker`) to intelligently route audio. It provides independent volume controls for the Handset and Base Speaker in Home Assistant.
*   **Dual Mode Logic:**
    *   **Call Mode:** Lifting the handset triggers full vintage simulation (dial tones, noise).
    *   **Command Mode:** Dialing while on-hook sends immediate commands to HA without audio feedback.
*   **"AI-Agnostic" Dialing:** Removed hardcoded logic for dialing '0'. The device now exposes a `start_listening` service. This allows Home Assistant automations to map different numbers to different AI Pipelines (e.g. Dial 1 for Home Control, 0 for ChatGPT).

### 🛡 Stability
*   **Safe Mode:** Enabled `safe_mode` to prevent boot loops during configuration errors.
*   **Watchdogs:** Added 5-minute timeout watchdogs for WiFi and API connections.

### 🐛 Bug Fixes
*   **Crash Fix:** Resolved a `LoadProhibited` panic in the `vintage_tone_generator`.
*   **Wiring:** Updated `GPIO4` voltage divider factor for accurate battery readings on WEMOS S3 Pro.

## [v1.0.0] - 2026-01-09 - Initial "R.O.T.A.R.I." Launch

### 🚀 Major Features & Rebranding
*   **Project Renamed:** Now officially **R.O.T.A.R.I.** (The _Robotic Operative Telecommunication & Artificial Response Interface_). Note: Hostname remains `rotary-phone-agent` to preserve network bindings.
*   **Voice Assistant Integration:** Full I2S audio pipeline with Wake-on-LAN/Dial support.
*   **New "Vintage" Audio Engine (C++):**
    *   **Custom Component:** `vintage_tone_generator` replaces MP3 files for low-latency feedback.
    *   **Features:** Synthesized Dial Tone (425Hz with authentic wobble), Busy Tone, Comfort Noise injection, and zero-latency electronic clicking for rotary dialing.
*   **Button Functions:**
    *   Button 1: System (Short: DND Toggle, Long: Battery Check).
    *   Button 2-4: Exposed as Home Assistant events for custom automation.
*   **LED Signaling:** Organic "Incandescent" fade effects for Thinking, Error, and Mute states.

### 🛠 Technical Improvements
*   **Refactored Architecture:** Split monolithic YAML into `external_components` (C++) and `rotary_helpers.h`.
*   **Audio Routing:** Separated I2S channels for Handset (Ear/Mic) and Base (Loudspeaker/Ringtone).
*   **Debouncing:** Optimized software debouncing (15ms) for reliable rotary pulse counting.
*   **Battery Logic:** 4-LED Bar graph display for voltage monitoring.

### 📦 Removed / Deprecated
*   **MP3 Files:** `00002.mp3` (Clicks) and `00003.mp3` (Hook Flash) are no longer required; functionality is now synthesized electronically.
