#pragma once

#include <memory>
#include <WiFiManager.h>
#include <DoubleResetDetector.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DataJsonPayload.h"
#include "ConfigStore.h"
#include "WifiConfigParameters.h"

class Display;

class Wifi {
public:
    Wifi();
    bool shouldStartConfigPortal(Display &screen);
    void startConfigPortal(Display &screen, AppConfig &config);
    bool connect(Display &screen, AppConfig &config);
    bool isMdnsReady() const;
    bool poll();
    void configureMqtt(const AppConfig &config);
    void publishSensorStates(const DataJsonPayload::Payload &payload);
    bool isMqttConnected() const;
private:
    WiFiManager wifiManager;
    DoubleResetDetector doubleResetDetector;
    std::unique_ptr<WifiConfigParameters> configParameters;
    WiFiClient mqttTransport;
    PubSubClient mqttClient;
    String mqttHost;
    String mqttUsername;
    String mqttPassword;
    uint16_t mqttPort = 1883;
    unsigned long lastMqttAttempt = 0;
    bool mdnsReady = false;
    bool wasConnected = false;

    static Display *activeDisplay;
    static AppConfig *activeConfig;
    static WifiConfigParameters *activeConfigParameters;

    static void setActiveDisplay(Display &screen);
    static Display &activeDisplayRef();

    static void handleConfigPortalStart(WiFiManager *wifiManagerInstance);
    static void saveConfigParameters();
    static IRAM_ATTR void toggleStatusLed();

    void prepareConfigPortalParameters(AppConfig &config);
    bool mqttEnabled() const;
    bool connectMqtt();
    void publishDiscovery();
};
