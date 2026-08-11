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
    String mqttHost;
    uint16_t mqttPort;
    String mqttUsername;
    String mqttPassword;
};

struct ConfigLoadResult {
    AppConfig config;
    ConfigLoadStatus::Outcome outcome;
};

class ConfigStore {
public:
    static ConfigLoadResult load(const char *defaultNtpServer, const char *defaultTimezonePosix, float defaultTempOffsetIndoor,
                                 uint8_t defaultOutdoorSensorChannel, const char *defaultWebLanguage, float defaultVentingThreshold,
                                 const char *defaultMqttHost, uint16_t defaultMqttPort, const char *defaultMqttUsername,
                                 const char *defaultMqttPassword);
    static bool save(const AppConfig &config);
};
