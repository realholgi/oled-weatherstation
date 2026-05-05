# WetterStation

ESP8266-based weather station with OLED display, indoor/outdoor sensors, accurate time, and a live web interface.
The main purpose is to show the absolute humidity between in- and outdoor and if venting is safe.

Outdoor readings come from a 433 MHz wireless sensor. Indoor readings come from an HTU21/SHT21 I2C sensor. 
The device serves a small web UI at `http://wetter.local` and exposes raw readings at `http://wetter.local/data.json`.

## Hardware

| Component | Details                                                   |
|---|-----------------------------------------------------------|
| Microcontroller | Wemos D1 Mini (ESP8266)                                   |
| Display | 2.42" SSD1305 128×64 OLED                                 |
| Indoor sensor | HTU21 / SHT21 (temperature + humidity)                    |
| Outdoor sensor | ALDI FWS 433 MHz wireless sensor (temperature + humidity) |

**Pin assignments:**

- OLED reset: D3
- 433 MHz receiver: D6

## Features

- Displays current time, indoor and outdoor temperature, relative and absolute humidity, and their respective difference
- Live web interface at `http://wetter.local` with auto-updating values
- JSON endpoint at `http://wetter.local/data.json`
- WiFi setup via captive portal and on double-reset
- Online time sync via NTP with timezone/DST handling from the configured POSIX timezone

## Build & Flash

Requires [PlatformIO](https://platformio.org/).

```bash
# Build firmware
pio run

# Flash to Wemos D1 Mini
pio run --target upload

# Upload over Wi-Fi after the first USB flash
pio run -e d1_mini_ota -t upload

# Clean build artifacts
pio run --target clean

# Open serial monitor (115200 baud)
pio device monitor

# Run unit tests (native, no hardware needed)
pio test --environment native
```

`platformio.ini` is the authoritative build configuration. `CMakeLists.txt` is only present for IDE integration.

## OTA Uploads

The firmware exposes Arduino OTA on the hostname `wetter.local` after it has connected to WiFi.

- The first flash must still be done over USB.
- After that, use `pio run -e d1_mini_ota -t upload`.
- If mDNS resolution is unreliable on your network, replace `upload_port` with the device IP address for the upload.
- To require an OTA password, add a build flag such as `"-D OTA_PASSWORD=\\\"secret\\\""` to the OTA environment.

## First-Time Setup

On first boot (or after a double-reset within 2 seconds), the device starts a WiFi access point named **wetter**. 
Connect to it and open the captive portal to enter your WiFi credentials. The device reboots and connects automatically on subsequent boots.

The captive portal also stores:

- the NTP server
- the POSIX timezone string used by `configTzTime()`
- the indoor temperature calibration offset

Configuration is persisted in LittleFS as `/config.json`.

## Architecture

The firmware is organized around a few small components:

- `TimeClient` configures NTP and local time formatting
- `SensorIndoor` reads the onboard temperature and humidity sensor
- `SensorOutdoor` decodes the 433 MHz receiver data and applies plausibility checks
- `Display` renders the current readings to the SSD1305 OLED
- `WebServer` serves the HTML UI and `/data.json`
- `Wifi` owns WiFiManager setup, captive portal flow, and mDNS setup
- `ConfigStore` loads and saves persisted NTP, timezone, and indoor temperature offset settings
- `HumidityMath` calculates absolute humidity and dew point
- `SensorSanity` provides range-checking and plausibility filters for sensor values

Two `Ticker` instances drive periodic work without blocking:

- `tickerForInternalSensorUpdate` triggers indoor sensor updates every 2 seconds
- `tickerForExternalSensorInvalidate` marks outdoor data stale after 120 seconds without a fresh packet

Time sync uses the ESP8266 time stack directly through `configTzTime()` and `localtime()`. DST handling therefore comes from the configured POSIX timezone.

The 433 MHz receiver path is interrupt-driven. Functions that run on that boundary must stay in IRAM and use `IRAM_ATTR`.

## Testing

Native PlatformIO tests cover standalone logic that does not require ESP8266 hardware:

- `test/test_humidity`
- `test/test_sensor_sanity`

When adding testable logic, keep it in small standalone units that can run in the `native` environment.

## Development Notes

- Use `StaticJsonDocument` or `JsonDocument`; do not reintroduce deprecated `DynamicJsonDocument` patterns.
- Keep large HTML strings and bitmap assets in `PROGMEM` to preserve RAM.
- The board uses `eagle.flash.4m1m.ld`: 4 MB flash with 1 MB reserved for LittleFS.
- If you change ISR-adjacent code, keep it short, non-blocking, and safe for interrupt context.

## Dependencies

Managed via PlatformIO (`platformio.ini`):

- ArduinoJson
- WiFiManager
- DoubleResetDetector
- Adafruit HTU21DF Library
- Adafruit GFX Library
- Adafruit SSD1305 (subclassed as `WetterOLED` in `src/` for hardware-specific offset)
- FWS_433_Receiver (bundled in `lib/`)

## License

See [LICENSE](LICENSE).
