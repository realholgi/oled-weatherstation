#pragma once

#include <ESP8266WebServer.h>
#include "config.h"

class SensorIndoor;
class SensorOutdoor;

class WebServer {
public:
    WebServer();
    void begin(SensorIndoor &indoorSensor, SensorOutdoor &outdoorSensor, bool advertiseMdns, const String &webLanguage, float threshold);
    void handleClient();

private:
    ESP8266WebServer server;
    bool started = false;
    String pageLanguage = "de";

    SensorIndoor *indoorSensorRef = nullptr;
    SensorOutdoor *outdoorSensorRef = nullptr;
    float ventingThreshold = DEFAULT_VENTING_THRESHOLD;

    void handleRoot();
    void handleNotFound();
    void handleDataJson();
};
