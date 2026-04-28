#pragma once

#include <WiFiManager.h>
#include <DoubleResetDetector.h>

extern WiFiManager wifiManager;
extern DoubleResetDetector drd;

bool shouldStartSetup();
void doSetup();
void setupWIFI();
void saveConfigCallback();
void configModeCallback(WiFiManager *myWiFiManager);
ICACHE_RAM_ATTR void flash();
