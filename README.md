# Rotary Phone Agent

Ein ESPHome-basiertes Projekt, das ein altes Wählscheibentelefon in einen modernen, mobilen Sprachassistenten für Home Assistant verwandelt.

## Funktionen

*   **Push-to-Talk:** Hörer abnehmen startet sofort den Sprachassistenten (kein Wake Word nötig).
*   **Visuelles Feedback:** Die Tasten leuchten Grün, wenn der Assistent zuhört, und Rot bei Fehlern.
*   **Akustisches Feedback:** Authentische Klick-Geräusche und Klingeltöne über einen DFPlayer Mini (MP3).
*   **Wählscheibe:**
    *   **Normalbetrieb:** Sendet Events an Home Assistant (Szenenwahl, etc.).
    *   **Musikbetrieb:** Regelt die Lautstärke des Sockel-Lautsprechers (1=10%, 0=100%).
*   **Schnellwahltasten:** 4 Taster für benutzerdefinierte Aktionen.
    *   **Taste 1 (Long Press):** Schaltet das Mikrofon stumm (Mute). LED 1 pulsiert lila. Auflegen setzt den Mute-Status zurück.
*   **Umweltsensor:** Überwachung von Temperatur, Luftfeuchtigkeit und Luftdruck (BME280).
*   **Präsenzerkennung:** Ein Radarsensor (LD2410) erkennt Bewegungen und Anwesenheit durch das Gehäuse hindurch (z.B. als mobiler Lichtschalter).
*   **Raum-Tracking:** Dank Bluetooth und Bermuda BLE Trilateration weiß das Telefon, in welchem Raum es sich befindet.
*   **Find My Phone:** Alarm-Funktion (Blinken), falls das Telefon verlegt wurde.
*   **Batteriebetrieb:** Überwachung der Batteriespannung mit einstellbarem Alarm (rote LED) und Prozentanzeige.
*   **Watchdog:** Automatischer Neustart bei Verbindungsproblemen (WLAN/API > 15min) oder Systemhängern.

## Hardware

*   **Controller:** LilyGO T7-S3 (ESP32-S3)
    *   Integrierter Laderegler (TP4054) für 3.7V LiPo Akkus.
    *   **Achtung:** Polarität des JST 1.25mm Steckers vor Anschluss prüfen!
*   **Audio Hörer:** I2S Mikrofon (z.B. INMP441) & I2S Verstärker (z.B. MAX98357A)
*   **Audio Sockel:** Zweiter I2S Verstärker (z.B. MAX98357A) für Musik
*   **Audio Sounds:** DFPlayer Mini für Klingeltöne und Klick-Geräusche (eigener Lautsprecher empfohlen)
*   **Sensoren:**
    *   **BME280:** Temperatur, Feuchtigkeit, Druck
    *   **LD2410:** Radar-Präsenzerkennung (Moving/Still Target)
*   **LEDs:** WS2812B LED-Streifen (4 LEDs)
*   **Telefon:** Altes Wählscheibentelefon mit Impulswahlverfahren

## Pinbelegung (LilyGO T7-S3)

| Komponente | Funktion | Pin (GPIO) |
| :--- | :--- | :--- |
| **Audio Hörer (Bus A)** | LRCLK (Word Select) | GPIO 5 |
| | BCLK (Bit Clock) | GPIO 6 |
| | DIN (Mic Data) | GPIO 7 |
| | DOUT (Speaker Data) | GPIO 15 |
| **Audio Sockel (Bus B)** | LRCLK (Word Select) | GPIO 16 |
| | BCLK (Bit Clock) | GPIO 18 |
| | DOUT (Speaker Data) | GPIO 19 |
| **Eingabe** | Hook Switch (Hörer) | GPIO 1 |
| | Schnellwahl 1 | GPIO 10 |
| | Schnellwahl 2 | GPIO 11 |
| | Schnellwahl 3 | GPIO 12 |
| | Schnellwahl 4 | GPIO 13 |
| | Wählscheibe (Impuls) | GPIO 14 |
| **Ausgabe** | WS2812B LEDs (Data) | GPIO 2 |
| | DFPlayer TX | GPIO 43 |
| | DFPlayer RX | GPIO 44 |
| **Sensoren (I2C)** | SDA (Data) | GPIO 8 |
| | SCL (Clock) | GPIO 9 |
| **Sensoren (UART)** | Radar TX (an ESP RX) | GPIO 38 |
| | Radar RX (an ESP TX) | GPIO 39 |
| **Power** | Batterie (ADC) | GPIO 4 |

## Home Assistant Integration

### Sprachassistent & Freizeichen

Das Telefon ist so konfiguriert, dass der Sprachassistent startet, sobald der Hörer abgenommen wird.
*   **Freizeichen:** Um ein "Freizeichen" zu simulieren, aktiviere in den Home Assistant Voice Pipeline Einstellungen den "Start"-Sound. Dieser wird dann beim Abnehmen abgespielt und signalisiert "Ich höre zu".

### Konfiguration (Home Assistant)

Folgende Entitäten stehen zur Verfügung, um das Verhalten des Telefons anzupassen:

*   **Switch:** `switch.rotary_phone_multi_digit_mode`
    *   **Aus (Standard):** Jede gewählte Ziffer wird sofort als Event gesendet (für Szenenwahl etc.).
    *   **Ein:** Ziffern werden gesammelt und erst nach einer Pause als ganzer String gesendet (für Telefonnummern).
*   **Number:** `number.rotary_phone_dial_timeout`
    *   Legt fest, wie lange (in ms) nach der letzten Ziffer gewartet wird, bevor der String gesendet wird (nur im Multi-Digit Modus).
*   **Klingelton-Einstellungen:**
    *   `number.ringtone_count`: Anzahl der Wiederholungen des Klingeltons.
    *   `number.ringtone_duration`: Dauer des Klingeltons (in ms).
    *   `number.ringtone_pause`: Pause zwischen den Wiederholungen (in ms).

### Sensoren

Das Gerät stellt folgende Sensoren in Home Assistant bereit:
*   `sensor.phone_temperature`
*   `sensor.phone_humidity`
*   `sensor.phone_pressure`
*   `sensor.battery_voltage`

### Events

Das Gerät sendet Events an Home Assistant, die du in Automationen nutzen kannst.

*   `esphome.rotary_phone_dial`: Wird ausgelöst, wenn eine Ziffer gewählt wurde (Single-Digit Modus).
    *   Daten: `digit` (die gewählte Ziffer, 0-9)
*   `esphome.rotary_phone_dial_string`: Wird ausgelöst, wenn eine Nummer gewählt wurde (Multi-Digit Modus).
    *   Daten: `number` (die gewählte Nummer als String, z.B. "12345")
*   `esphome.rotary_phone_button`: Wird ausgelöst, wenn eine Schnellwahltaste gedrückt wurde.
    *   Daten: `button` (Nummer der Taste, 1-4)

### Beispiel-Automation (Raumabhängige Steuerung)

```yaml
alias: Rotary Phone Control
trigger:
  - platform: event
    event_type: esphome.rotary_phone_button
action:
  - choose:
      # Wenn im Wohnzimmer
      - conditions:
          - condition: state
            entity_id: sensor.rotary_phone_agent_area
            state: "Wohnzimmer"
        sequence:
          - service: scene.turn_on
            target:
              entity_id: scene.wohnzimmer_entspannen
      # Wenn in der Küche
      - conditions:
          - condition: state
            entity_id: sensor.rotary_phone_agent_area
            state: "Küche"
        sequence:
          - service: switch.turn_on
            target:
              entity_id: switch.kaffeemaschine
```

### LED-Steuerung bei Raumwechsel

Erstelle eine Automation, die auf Änderungen von `sensor.rotary_phone_agent_area` reagiert und die `light.button_leds` entsprechend einfärbt.

## Installation

1.  `secrets.yaml` anpassen (WLAN-Daten).
2.  Projekt mit ESPHome kompilieren und auf den ESP32-S3 flashen.
3.  In Home Assistant integrieren.

### SD-Karte für DFPlayer Mini

Die SD-Karte muss FAT32 formatiert sein. Erstelle folgende Ordnerstruktur:

*   `/01/001.mp3`: Klingelton
*   `/02/001.mp3`: Klick-Geräusch (Wählscheibe)
*   `/02/002.mp3`: Hook Flash Feedback (Doppel-Piep)
