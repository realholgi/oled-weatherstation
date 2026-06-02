#pragma once

#include <Arduino.h>
#include "ConfigLoadStatus.h"

struct AppConfig {
    String ntpServer;
    String timezonePosix;
    float tempOffsetIndoor;
    uint8_t outdoorSensorChannel;
    String webLanguage;
    float ventingThreshold;
};

struct ConfigLoadResult {
    AppConfig config;
    ConfigLoadStatus::Outcome outcome;
};

class ConfigStore {
public:
    static ConfigLoadResult load(const char *defaultNtpServer, const char *defaultTimezonePosix, float defaultTempOffsetIndoor,
                                 uint8_t defaultOutdoorSensorChannel, const char *defaultWebLanguage, float defaultVentingThreshold);
    static bool save(const AppConfig &config);
};
