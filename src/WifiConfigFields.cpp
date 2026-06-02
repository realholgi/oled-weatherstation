#include "WifiConfigFields.h"

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace {
bool parseFiniteFloat(const char *value, float &out) {
    if (!WifiConfigFields::hasTextValue(value)) return false;

    char *end = nullptr;
    const float parsed = strtof(value, &end);
    if (end == value || *end != '\0' || !std::isfinite(parsed)) return false;

    out = parsed;
    return true;
}
}

namespace WifiConfigFields {
bool hasTextValue(const char *value) {
    return value != nullptr && value[0] != '\0';
}

bool isSupportedWebLanguage(const char *value) {
    return value != nullptr && (strcmp(value, "de") == 0 || strcmp(value, "en") == 0);
}

bool parseIndoorTemperatureOffset(const char *value, float &out) {
    return parseFiniteFloat(value, out);
}

bool parseOutdoorSensorChannel(const char *value, uint8_t &out) {
    if (!hasTextValue(value)) return false;

    char *end = nullptr;
    const long parsed = strtol(value, &end, 10);
    if (end == value || *end != '\0' || parsed < 1 || parsed > 3) return false;

    out = static_cast<uint8_t>(parsed);
    return true;
}

bool parseVentingThreshold(const char *value, float &out) {
    float parsed = 0.0f;
    if (!parseFiniteFloat(value, parsed) || parsed <= 0.0f) return false;

    out = parsed;
    return true;
}
}
