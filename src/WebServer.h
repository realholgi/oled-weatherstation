#pragma once

#include <ESP8266WebServer.h>
#include "config.h"

class SensorIndoor;
class SensorOutdoor;
class TimeClient;

class WebServer {
public:
    WebServer();
    void begin(SensorIndoor &indoorSensor, SensorOutdoor &outdoorSensor, const TimeClient &timeClient, bool advertiseMdns, const String &webLanguage, float threshold);
    void handleClient();

private:
    ESP8266WebServer server;
    bool started = false;
    String pageLanguage = "de";

    SensorIndoor *indoorSensorRef = nullptr;
    SensorOutdoor *outdoorSensorRef = nullptr;
    const TimeClient *timeClientRef = nullptr;
    float ventingThreshold = DEFAULT_VENTING_THRESHOLD;

    void handleRoot();
    void handleNotFound();
    void handleDataJson();
};
