#pragma once

#include <cstdint>
#include <string>

#include "ConfigLoadStatus.h"

namespace ConfigJsonParser {
constexpr int CURRENT_SCHEMA_VERSION = 2;

struct Defaults {
    const char *ntpServer;
    const char *timezonePosix;
    float tempOffsetIndoor;
    uint8_t outdoorSensorChannel;
    const char *webLanguage;
    float ventingThreshold;
    const char *mqttHost;
    uint16_t mqttPort;
    const char *mqttUsername;
    const char *mqttPassword;
};

struct Values {
    std::string ntpServer;
    std::string timezonePosix;
    float tempOffsetIndoor;
    uint8_t outdoorSensorChannel;
    std::string webLanguage;
    float ventingThreshold;
    std::string mqttHost;
    uint16_t mqttPort;
    std::string mqttUsername;
    std::string mqttPassword;
    int schemaVersion;
};

struct Result {
    Values values;
    ConfigLoadStatus::Outcome outcome;
};

Result parse(const char *json, const Defaults &defaults);
}
