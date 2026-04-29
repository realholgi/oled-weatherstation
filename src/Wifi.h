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

    static Display *activeDisplay;

    static void useDisplay(Display &screen);
    static Display &display();

    static void configModeCallback(WiFiManager *myWiFiManager);
    static IRAM_ATTR void flash();
};
