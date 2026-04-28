#pragma once

class SensorIndoor;
class SensorOutdoor;

namespace WebServer {

void setup();
void handleClient(SensorIndoor &indoorSensor, SensorOutdoor &outdoorSensor);
void handleRoot();
void handleNotFound();
void handleJsonData();

}
