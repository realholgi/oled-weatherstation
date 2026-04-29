#pragma once

#include <Arduino.h>

struct AppConfig {
    String ntpServer;
    String timezonePosix;
};

class ConfigStore {
public:
    static AppConfig load(const char *defaultNtp, const char *defaultTz);
    static void save(const AppConfig &config);
};
