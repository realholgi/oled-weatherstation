# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added

- MQTT broker configuration in the captive portal and LittleFS configuration.
- Home Assistant MQTT auto-discovery for indoor and outdoor temperature, humidity, absolute humidity, dew point, and outdoor battery.
- Retained MQTT availability, state publishing, and discovery republishing after broker reconnects.
- MQTT sensor states are published every 30 seconds and immediately after broker connection or reconnection.

## [1.0.2] - 2026-07-27

### Changed

- Updated the ESP8266 PlatformIO platform to version 4.2.1.

### Fixed

- Configuration files larger than 1 KiB are rejected before memory allocation; the weather station then uses safe defaults.
- NTP servers and time zones from the configuration portal and stored settings are now strictly validated.
- The 433 MHz receiver example now uses `snprintf` instead of unbounded `sprintf`.

## [1.0.1] - 2026-06-22

### Added

- The configuration portal now allows selecting an outdoor sensor channel, a ventilation threshold, and the web interface language.
- The JSON endpoint now provides explicit sensor validity flags, time synchronisation status, and ventilation advice.
- The web interface is available in German and English and visibly warns when outdoor air is more humid than indoor air.
- Native tests, cppcheck, and firmware artifacts are integrated into CI.

### Changed

- Configuration is versioned in LittleFS, and library versions are pinned in PlatformIO.
- The web interface uses cache headers, and the mDNS service is re-announced after Wi-Fi reconnects.

### Fixed

- The NTP server string remains valid after startup, making time synchronisation reliable.
- Processing of indoor and outdoor sensor readings and expired measurements is more robust and easier to test.
- Configuration storage is no longer unintentionally formatted after errors.

## [1.0.0] - 2026-05-05

### Added

- First stable release of the weather station with an OLED display, indoor and 433 MHz outdoor sensors, and a local web interface.
- Wi-Fi configuration portal, NTP time synchronisation, and OTA uploads.
- Display of temperature, relative and absolute humidity, dew point, and ventilation advice.
