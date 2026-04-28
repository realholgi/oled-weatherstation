#pragma once

#include <WiFiManager.h>
#include <DoubleResetDetector.h>

class Display;

class Wifi {
public:
    Wifi();
    bool shouldStartSetup(Display &screen);
    void doSetup(Display &screen);
    void setup(Display &screen);
    void loop();

private:
    WiFiManager wifiManager;
    DoubleResetDetector drd;

    static bool shouldSaveConfig;
    static bool initialConfig;
    static Display *activeDisplay;

    static void useDisplay(Display &screen);
    static Display &display();

    static void saveConfigCallback();
    static void configModeCallback(WiFiManager *myWiFiManager);
    static ICACHE_RAM_ATTR void flash();
};
