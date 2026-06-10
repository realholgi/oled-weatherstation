#pragma once

#include <memory>
#include <WiFiManager.h>
#include <DoubleResetDetector.h>
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
    void poll();

private:
    WiFiManager wifiManager;
    DoubleResetDetector doubleResetDetector;
    std::unique_ptr<WifiConfigParameters> configParameters;
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
};
