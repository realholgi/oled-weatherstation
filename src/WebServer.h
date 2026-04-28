#pragma once

#include <ESP8266WebServer.h>

class SensorIndoor;
class SensorOutdoor;

class WebServer {
public:
    WebServer();
    void setup();
    void handleClient(SensorIndoor &indoorSensor, SensorOutdoor &outdoorSensor);

private:
    ESP8266WebServer server;

    SensorIndoor *activeIndoorSensor = nullptr;
    SensorOutdoor *activeOutdoorSensor = nullptr;

    void handleRoot();
    void handleNotFound();
    void handleJsonData();

    void useSensors(SensorIndoor &indoor, SensorOutdoor &outdoor);
    SensorIndoor &indoorSensor();
    SensorOutdoor &outdoorSensor();
};
