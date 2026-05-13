#pragma once

#include <Arduino.h>

struct AppConfig {
    String ntpServer;
    String timezonePosix;
    float tempOffsetIndoor;
    uint8_t outdoorSensorChannel;
    String webLanguage;
    float ventingThreshold;
};

class ConfigStore {
public:
    static AppConfig load(const char *defaultNtpServer, const char *defaultTimezonePosix, float defaultTempOffsetIndoor,
                          uint8_t defaultOutdoorSensorChannel, const char *defaultWebLanguage, float defaultVentingThreshold);
    static bool save(const AppConfig &config);
};
