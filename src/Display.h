#pragma once

#include <Arduino.h>
#include <Adafruit_SSD1305.h>

class SensorIndoor;
class SensorOutdoor;
class TimeClient;

class Display {
public:
    Display();
    Display(const Display &) = delete;
    Display &operator=(const Display &) = delete;

    void setup();
    void showSensorFailure();
    void showStartupConfig();
    void showStartupWifi();
    void showStartupHttp();
    void showStartupTime();
    void showStartupOutdoorSensor();
    void showConfigPortalNoCredentials();
    void showConfigPortalReset();
    void showConfigPortalSsid(const String &ssid);
    void appendWifiProgress();
    void renderData(TimeClient &timeClient, const SensorIndoor &indoorSensor, const SensorOutdoor &outdoorSensor);

private:
    void clear();
    void setTextSize(uint8_t size);
    void printAt(int x, int y, const char *st, boolean onScreen = true);
    void printAt(int x, int y, const String &st, boolean onScreen = true);
    void print(const char *st);
    void flush();
    void showStartupStage(int y, const char *message);
    void printNumI(int x, int y, int num);
    void printNumF(int x, int y, double num, byte dec = 1, int length = 0);

    Adafruit_SSD1305 oled;
};
