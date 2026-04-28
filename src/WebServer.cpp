#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "WebServer.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "config.h"
#include "PAGE_wetter.h"

namespace WebServer {

static ESP8266WebServer HTTP(80);

static SensorIndoor *activeIndoorSensor = nullptr;
static SensorOutdoor *activeOutdoorSensor = nullptr;

static SensorIndoor &indoorSensor() {
    return *activeIndoorSensor;
}

static SensorOutdoor &outdoorSensor() {
    return *activeOutdoorSensor;
}

static void useSensors(SensorIndoor &indoor, SensorOutdoor &outdoor) {
    activeIndoorSensor = &indoor;
    activeOutdoorSensor = &outdoor;
}

void setup() {
    HTTP.on("/", handleRoot);
    HTTP.on("/data.json", HTTP_GET, []() {
        HTTP.sendHeader("Connection", "close");
        HTTP.sendHeader("Access-Control-Allow-Origin", "*");
        return handleJsonData();
    });
    HTTP.onNotFound(handleNotFound);
    HTTP.begin();
    MDNS.addService("http", "tcp", 80);
}

void handleClient(SensorIndoor &indoor, SensorOutdoor &outdoor) {
    useSensors(indoor, outdoor);
    HTTP.handleClient();
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += HTTP.uri();
    message += "\nMethod: ";
    message += (HTTP.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += HTTP.args();
    message += "\n";
    for (uint8_t i = 0; i < HTTP.args(); i++) {
        message += " " + HTTP.argName(i) + ": " + HTTP.arg(i) + "\n";
    }
    HTTP.send(404, "text/plain", message);
}

void handleJsonData() {
    JsonDocument doc;

    doc["t_in"] = indoorSensor().temperature() - TEMP_OFFSET_INDOOR;
    doc["h_in"] = int(indoorSensor().humidity());
    doc["f_in"] = indoorSensor().absoluteHumidity();
    doc["dp_in"] = indoorSensor().dewPoint();

    doc["t_out"] = outdoorSensor().temperature();
    doc["h_out"] = int(outdoorSensor().humidity());
    doc["f_out"] = outdoorSensor().absoluteHumidity();
    doc["b_out"] = outdoorSensor().battery();
    doc["last_out"] = outdoorSensor().secondsSinceLastReceived();

    doc["f_diff"] = indoorSensor().absoluteHumidity() - outdoorSensor().absoluteHumidity();

    String message = "";
    serializeJson(doc, message);

    HTTP.send(200, "application/json;charset=utf-8", message);
}

void handleRoot() {
    HTTP.send_P(200, "text/html", PAGE_Wetter);
}

}
