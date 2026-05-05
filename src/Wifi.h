#pragma once

#include <WiFiManager.h>
#include <DoubleResetDetector.h>
#include "ConfigStore.h"

class Display;

class Wifi {
public:
    Wifi();
    bool shouldStartConfigPortal(Display &screen);
    void startConfigPortal(Display &screen, AppConfig &config);
    void connect(Display &screen, AppConfig &config);
    void poll();

private:
    WiFiManager wifiManager;
    DoubleResetDetector doubleResetDetector;

    static Display *activeDisplay;
    static AppConfig *activeConfig;
    static WiFiManagerParameter *ntpParam;
    static WiFiManagerParameter *tzParam;
    static WiFiManagerParameter *tempOffsetIndoorParam;

    static void setActiveDisplay(Display &screen);
    static Display &activeDisplayRef();

    static void handleConfigPortalStart(WiFiManager *wifiManagerInstance);
    static void saveConfigParameters();
    static IRAM_ATTR void toggleStatusLed();

    static String buildTimezoneSelectHtml(const String &currentPosix);
    static String formatFloatValue(float value, uint8_t decimals);
};
