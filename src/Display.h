#pragma once

#include <Arduino.h>
#include "WeatherOled.h"

class SensorIndoor;
class SensorOutdoor;
class TimeClient;

class Display {
public:
    Display();
    Display(const Display &) = delete;
    Display &operator=(const Display &) = delete;

    void begin();
    void showSensorFailure();
    void showStartupConfig();
    void showStartupWifi();
    void showStartupHttp();
    void showStartupTime();
    void showStartupOutdoorSensor();
    void showWifiFailure();
    void showConfigPortalNoCredentials();
    void showConfigPortalReset();
    void showConfigPortalSsid(const String &ssid);
    void appendWifiConnectionProgress();
    void renderMeasurements(TimeClient &timeClient, const SensorIndoor &indoorSensor, const SensorOutdoor &outdoorSensor);

private:
    void drawTextAt(int x, int y, const char *text, boolean updateDisplay = true);
    void drawTextAt(int x, int y, const String &text, boolean updateDisplay = true);
    void drawIntegerAt(int x, int y, int value);
    void drawFloatAt(int x, int y, double value, byte decimals = 1, int minimumWidth = 0);

    WeatherOled oled;
};
