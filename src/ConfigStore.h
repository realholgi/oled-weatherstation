#pragma once

#include <Arduino.h>

struct AppConfig {
    String ntpServer;
    String timezonePosix;
    float tempOffsetIndoor;
    uint8_t outdoorSensorChannel;
};

class ConfigStore {
public:
    static AppConfig load(const char *defaultNtpServer, const char *defaultTimezonePosix, float defaultTempOffsetIndoor,
                          uint8_t defaultOutdoorSensorChannel);
    static bool save(const AppConfig &config);
};
