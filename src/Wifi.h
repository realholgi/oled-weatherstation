#pragma once

#include <WiFiManager.h>
#include <DoubleResetDetector.h>

class Display;

namespace Wifi {

extern WiFiManager wifiManager;
extern DoubleResetDetector drd;

bool shouldStartSetup(Display &screen);
void doSetup(Display &screen);
void setup(Display &screen);
void saveConfigCallback();
void configModeCallback(WiFiManager *myWiFiManager);
ICACHE_RAM_ATTR void flash();

}
