#pragma once

#include <cstdint>
#include <string>

#include "ConfigLoadStatus.h"

namespace ConfigJsonParser {
struct Defaults {
    const char *ntpServer;
    const char *timezonePosix;
    float tempOffsetIndoor;
    uint8_t outdoorSensorChannel;
    const char *webLanguage;
    float ventingThreshold;
};

struct Values {
    std::string ntpServer;
    std::string timezonePosix;
    float tempOffsetIndoor;
    uint8_t outdoorSensorChannel;
    std::string webLanguage;
    float ventingThreshold;
};

struct Result {
    Values values;
    ConfigLoadStatus::Outcome outcome;
};

Result parse(const char *json, const Defaults &defaults);
}
