# WetterStation

ESP8266-based weather station with OLED display, indoor/outdoor sensors, online time sync, and a live web interface.

## Hardware

| Component | Details                                               |
|---|-------------------------------------------------------|
| Microcontroller | Wemos D1 Mini (ESP8266)                               |
| Display | 2.42" SSD1305 128×64 OLED                             |
| Indoor sensor | HTU21 / SHT21 (temperature + humidity)                |
| Outdoor sensor | ALDI 433 MHz wireless sensor (temperature + humidity) |

**Pin assignments:**

- OLED reset: D3
- 433 MHz receiver: D6

## Features

- Displays current time, indoor and outdoor temperature, relative and absolute humidity, and their respective difference
- Live web interface at `http://weather.local` with auto-updating values
- WiFi setup via captive portal and on double-reset
- Online Time sync with European DST handling

## Build & Flash

Requires [PlatformIO](https://platformio.org/).

```bash
# Build and flash to Wemos D1 Mini
pio run -e d1_mini -t upload

# Open serial monitor (115200 baud)
pio device monitor

# Run unit tests (native, no hardware needed)
pio test -e native
```

## First-Time Setup

On first boot (or after a double-reset within 2 seconds), the device starts a WiFi access point named **AutoConnectAP**. 
Connect to it and open the captive portal to enter your WiFi credentials. The device reboots and connects automatically on subsequent boots.

## Dependencies

Managed via PlatformIO (`platformio.ini`):

- ArduinoJson
- WiFiManager
- DoubleResetDetector
- Adafruit HTU21DF Library
- Adafruit GFX Library
- Adafruit SSD1305 (bundled in `lib/`)
- FWS_433_Receiver (bundled in `lib/`)

## License

See [LICENSE](LICENSE).
