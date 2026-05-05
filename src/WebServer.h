#pragma once

#include <ESP8266WebServer.h>

class SensorIndoor;
class SensorOutdoor;

class WebServer {
public:
    WebServer();
    void begin(SensorIndoor &indoorSensor, SensorOutdoor &outdoorSensor);
    void handleClient();

private:
    ESP8266WebServer server;

    SensorIndoor *indoorSensorRef = nullptr;
    SensorOutdoor *outdoorSensorRef = nullptr;

    void handleRoot();
    void handleNotFound();
    void handleDataJson();
};
