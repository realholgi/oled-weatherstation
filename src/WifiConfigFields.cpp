#include "WifiConfigFields.h"

#include "Timezones.h"

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace {
constexpr size_t MAX_NTP_SERVER_LENGTH = 63;
constexpr size_t MAX_MQTT_CREDENTIAL_LENGTH = 63;

bool isAsciiLetterOrDigit(char value) {
    return (value >= 'a' && value <= 'z') ||
           (value >= 'A' && value <= 'Z') ||
           (value >= '0' && value <= '9');
}

bool hasLengthWithinLimit(const char *value, size_t maxLength) {
    if (value == nullptr || value[0] == '\0') return false;

    for (size_t index = 0; index <= maxLength; ++index) {
        if (value[index] == '\0') return true;
    }
    return false;
}

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

bool isSupportedTimezone(const char *value) {
    if (!hasTextValue(value)) return false;

    for (size_t index = 0; index < TZ_COUNT; ++index) {
        if (strcmp(value, TIMEZONES[index].posix) == 0) return true;
    }
    return false;
}

bool isValidNtpServer(const char *value) {
    if (!hasLengthWithinLimit(value, MAX_NTP_SERVER_LENGTH)) return false;

    bool labelStartsWithHyphen = false;
    size_t labelLength = 0;
    char lastCharacter = '\0';
    for (size_t index = 0; value[index] != '\0'; ++index) {
        const char current = value[index];
        if (current == '.') {
            if (labelLength == 0 || labelStartsWithHyphen || value[index - 1] == '-') return false;
            labelLength = 0;
            labelStartsWithHyphen = false;
            continue;
        }
        if (!isAsciiLetterOrDigit(current) && current != '-') return false;
        if (labelLength == 0) labelStartsWithHyphen = (current == '-');
        ++labelLength;
        lastCharacter = current;
        if (labelLength > 63) return false;
    }

    return labelLength != 0 && !labelStartsWithHyphen && lastCharacter != '-';
}

bool isValidMqttHost(const char *value) {
    return value != nullptr && (value[0] == '\0' || isValidNtpServer(value));
}

bool isValidMqttCredential(const char *value) {
    return value != nullptr && (value[0] == '\0' || hasLengthWithinLimit(value, MAX_MQTT_CREDENTIAL_LENGTH));
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

bool parseMqttPort(const char *value, uint16_t &out) {
    if (!hasTextValue(value)) return false;

    char *end = nullptr;
    const long parsed = strtol(value, &end, 10);
    if (end == value || *end != '\0' || parsed < 1 || parsed > 65535) return false;

    out = static_cast<uint16_t>(parsed);
    return true;
}

bool parseVentingThreshold(const char *value, float &out) {
    float parsed = 0.0f;
    if (!parseFiniteFloat(value, parsed) || parsed <= 0.0f) return false;

    out = parsed;
    return true;
}
}
