#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "WebServer.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "PAGE_wetter.h"

WebServer::WebServer() : server(80) {
}

SensorIndoor &WebServer::indoorSensor() {
    return *activeIndoorSensor;
}

SensorOutdoor &WebServer::outdoorSensor() {
    return *activeOutdoorSensor;
}

void WebServer::useSensors(SensorIndoor &indoor, SensorOutdoor &outdoor) {
    activeIndoorSensor = &indoor;
    activeOutdoorSensor = &outdoor;
}

void WebServer::setup() {
    server.on("/", [this]() { handleRoot(); });
    server.on("/data.json", HTTP_GET, [this]() {
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        handleJsonData();
    });
    server.onNotFound([this]() { handleNotFound(); });
    server.begin();
    MDNS.addService("http", "tcp", 80);
}

void WebServer::handleClient(SensorIndoor &indoor, SensorOutdoor &outdoor) {
    useSensors(indoor, outdoor);
    server.handleClient();
}

void WebServer::handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

void WebServer::handleJsonData() {
    JsonDocument doc;

    doc["t_in"] = indoorSensor().temperature();
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

    server.send(200, "application/json;charset=utf-8", message);
}

void WebServer::handleRoot() {
    server.send_P(200, "text/html", PAGE_Wetter);
}
