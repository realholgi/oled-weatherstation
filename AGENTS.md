# AGENTS.md

This file provides guidance to AI agents when working with code in this repository.

## Project Overview

ESP8266-based weather station (Wemos D1 Mini) that displays indoor and outdoor sensor data on a 128x64 OLED. Outdoor readings come from a 433MHz wireless sensor; indoor readings from an HTU21DF I²C sensor. A web interface at `http://weather.local/data.json` exposes all readings as JSON.

## Build System

The project uses **PlatformIO**. CMakeLists.txt exists only for IDE integration and delegates to PlatformIO targets.

```bash
pio run                          # compile
pio run --target upload          # flash to Wemos D1 Mini
pio run --target clean           # clean build artifacts
pio test --environment native    # run unit tests (no hardware required)
pio device monitor               # serial monitor at 115200 baud
```

## Architecture

**Scheduling**: Three `Ticker` instances drive periodic work without blocking:
- `tickerForInternalSensorUpdate` — reads HTU21DF every 2 s
- `tickerForTimeUpdate` — syncs time via HTTP by querying google.com every 3600 s
- `tickerForExternalSensorInvalidate` — marks outdoor data stale if no 433MHz packet arrives within 120 s

**ISR boundary**: The 433MHz receiver (`FWS433`) uses interrupt-driven reception. Any function called from an ISR must carry `ICACHE_RAM_ATTR`. Data crosses the ISR boundary through `volatile` globals declared in `globals.h`.

**Key source files**:
| File | Role |
|------|------|
| `src/WetterStation.cpp` | `setup()` / `loop()`, web server, display rendering |
| `src/TimeClient.{h,cpp}` | NTP sync, UTC offset, DST delegation |
| `src/globals.h` | All configuration constants and `extern` volatile globals |
| `src/display.h` | OLED helper functions |
| `src/PAGE_wetter.h` | HTML/JS web page stored in `PROGMEM` |
| `src/icons.h` | Bitmap icons for the OLED |
| `lib/FWS_433_Receiver/` | Custom 433MHz receiver library (pilight protocol) |
| `lib/Adafruit_SSD1305/` | Vendored OLED display driver |

**WiFi configuration**: `WiFiManager` opens a captive portal on first boot or when a double-reset is detected (`DoubleResetDetector`). Config is persisted to SPIFFS.

## Unit Tests

Native PlatformIO tests cover standalone logic such as `HumidityMath` and `SensorSanity`. Tests run on the native environment (no ESP8266 hardware needed). Add new testable logic as standalone classes in the same pattern.

## Important Constraints

- **ICACHE_RAM_ATTR**: Any function invoked from an interrupt handler (directly or via Ticker callback that touches ISR data) must have this attribute.
- **volatile globals**: Sensor readings shared between the ISR context and the main loop live in `globals.h` as `volatile`. Don't cache these in local variables across yield points.
- **ArduinoJson**: Use `StaticJsonDocument` (not the deprecated `DynamicJsonDocument`).
- **PROGMEM**: Large string literals (HTML, icon bitmaps) should be stored in `PROGMEM` to preserve RAM.
- **Flash layout**: `eagle.flash.4m1m.ld` — 4 MB flash, 1 MB reserved for SPIFFS.
