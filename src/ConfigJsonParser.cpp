#include "ConfigJsonParser.h"

#include <ArduinoJson.h>
#include <cmath>
#include <cstring>

namespace {
std::string textOrEmpty(const char *value) {
    return value == nullptr ? std::string() : std::string(value);
}

ConfigJsonParser::Values valuesFromDefaults(const ConfigJsonParser::Defaults &defaults) {
    return {textOrEmpty(defaults.ntpServer),
            textOrEmpty(defaults.timezonePosix),
            defaults.tempOffsetIndoor,
            defaults.outdoorSensorChannel,
            textOrEmpty(defaults.webLanguage),
            defaults.ventingThreshold};
}
}

namespace ConfigJsonParser {
Result parse(const char *json, const Defaults &defaults) {
    Values values = valuesFromDefaults(defaults);
    if (json == nullptr || json[0] == '\0') {
        return {values, ConfigLoadStatus::invalidJson()};
    }

    JsonDocument jsonDocument;
    if (deserializeJson(jsonDocument, json) != DeserializationError::Ok) {
        return {values, ConfigLoadStatus::invalidJson()};
    }

    const char *ntpServerValue = jsonDocument["ntp_server"] | "";
    const char *timezoneValue = jsonDocument["timezone_posix"] | "";
    const char *webLanguageValue = jsonDocument["web_language"] | "";

    if (ntpServerValue[0] != '\0') values.ntpServer = ntpServerValue;
    if (timezoneValue[0] != '\0') values.timezonePosix = timezoneValue;
    if (strcmp(webLanguageValue, "en") == 0 || strcmp(webLanguageValue, "de") == 0) {
        values.webLanguage = webLanguageValue;
    }

    if (jsonDocument["temp_offset_indoor"].is<float>()) {
        const float tempOffsetIndoor = jsonDocument["temp_offset_indoor"].as<float>();
        if (std::isfinite(tempOffsetIndoor)) values.tempOffsetIndoor = tempOffsetIndoor;
    }

    if (jsonDocument["outdoor_sensor_channel"].is<int>()) {
        const int outdoorSensorChannel = jsonDocument["outdoor_sensor_channel"].as<int>();
        if (outdoorSensorChannel >= 1 && outdoorSensorChannel <= 3) {
            values.outdoorSensorChannel = static_cast<uint8_t>(outdoorSensorChannel);
        }
    }

    if (jsonDocument["venting_threshold"].is<float>()) {
        const float ventingThreshold = jsonDocument["venting_threshold"].as<float>();
        if (std::isfinite(ventingThreshold) && ventingThreshold > 0.0f) {
            values.ventingThreshold = ventingThreshold;
        }
    }

    return {values, ConfigLoadStatus::loaded()};
}
}
