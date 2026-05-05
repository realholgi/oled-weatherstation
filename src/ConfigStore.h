#pragma once

#include <Arduino.h>

struct AppConfig {
    String ntpServer;
    String timezonePosix;
    float tempOffsetIndoor;
};

class ConfigStore {
public:
    static AppConfig load(const char *defaultNtpServer, const char *defaultTimezonePosix, float defaultTempOffsetIndoor);
    static void save(const AppConfig &config);
};
