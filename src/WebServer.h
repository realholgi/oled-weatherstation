#pragma once

#include <ESP8266WebServer.h>

class SensorIndoor;
class SensorOutdoor;

class WebServer {
public:
    WebServer();
    void setup(SensorIndoor &indoor, SensorOutdoor &outdoor);
    void handleClient();

private:
    ESP8266WebServer server;

    SensorIndoor *activeIndoorSensor = nullptr;
    SensorOutdoor *activeOutdoorSensor = nullptr;

    void handleRoot();
    void handleNotFound();
    void handleJsonData();

    SensorIndoor &indoorSensor();
    SensorOutdoor &outdoorSensor();
};
