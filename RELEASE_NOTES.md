# Release Notes

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
