#pragma once

#include <cstdint>

namespace WifiConfigFields {
bool hasTextValue(const char *value);
bool isSupportedWebLanguage(const char *value);
bool parseIndoorTemperatureOffset(const char *value, float &out);
bool parseOutdoorSensorChannel(const char *value, uint8_t &out);
bool parseVentingThreshold(const char *value, float &out);
}
