#pragma once

#include <WiFiManager.h>
#include <DoubleResetDetector.h>
#include "ConfigStore.h"

class Display;

class Wifi {
public:
    Wifi();
    bool shouldStartSetup(Display &screen);
    void doSetup(Display &screen, AppConfig &config);
    void setup(Display &screen, AppConfig &config);
    void loop();

private:
    WiFiManager wifiManager;
    DoubleResetDetector drd;

    static Display *activeDisplay;
    static AppConfig *activeConfig;
    static WiFiManagerParameter *ntpParam;
    static WiFiManagerParameter *tzParam;
    static WiFiManagerParameter *tempOffsetIndoorParam;

    static void useDisplay(Display &screen);
    static Display &display();

    static void configModeCallback(WiFiManager *myWiFiManager);
    static void saveParamsCallback();
    static IRAM_ATTR void flash();

    static String buildTimezoneSelectHtml(const String &currentPosix);
    static String formatFloat(float value, uint8_t decimals);
};
