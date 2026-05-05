#pragma once

#include <ESP8266WebServer.h>

class SensorIndoor;
class SensorOutdoor;

class WebServer {
public:
    WebServer();
    void begin(SensorIndoor &indoorSensor, SensorOutdoor &outdoorSensor, bool advertiseMdns, const String &webLanguage);
    void handleClient();

private:
    ESP8266WebServer server;
    bool started = false;
    String pageLanguage = "de";

    SensorIndoor *indoorSensorRef = nullptr;
    SensorOutdoor *outdoorSensorRef = nullptr;

    void handleRoot();
    void handleNotFound();
    void handleDataJson();
};
