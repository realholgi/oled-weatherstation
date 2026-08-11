#pragma once

#include <cstdint>

namespace WifiConfigFields {
bool hasTextValue(const char *value);
bool isSupportedTimezone(const char *value);
bool isValidNtpServer(const char *value);
bool isSupportedWebLanguage(const char *value);
bool isValidMqttHost(const char *value);
bool isValidMqttCredential(const char *value);
bool parseMqttPort(const char *value, uint16_t &out);
bool parseIndoorTemperatureOffset(const char *value, float &out);
bool parseOutdoorSensorChannel(const char *value, uint8_t &out);
bool parseVentingThreshold(const char *value, float &out);
}
