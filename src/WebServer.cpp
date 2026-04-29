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

void WebServer::setup(SensorIndoor &indoor, SensorOutdoor &outdoor) {
    activeIndoorSensor = &indoor;
    activeOutdoorSensor = &outdoor;
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

void WebServer::handleClient() {
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
        message += " ";
        message += server.argName(i);
        message += ": ";
        message += server.arg(i);
        message += "\n";
    }
    server.send(404, "text/plain", message);
}

void WebServer::handleJsonData() {
    JsonDocument doc;

    const SensorIndoor &in = *activeIndoorSensor;
    const SensorOutdoor &out = *activeOutdoorSensor;

    const float f_in = in.absoluteHumidity();
    const float f_out = out.absoluteHumidity();

    doc["t_in"] = in.temperature();
    doc["h_in"] = int(in.humidity());
    doc["f_in"] = f_in;
    doc["dp_in"] = in.dewPoint();

    doc["t_out"] = out.temperature();
    doc["h_out"] = int(out.humidity());
    doc["f_out"] = f_out;
    doc["b_out"] = out.battery();
    doc["last_out"] = out.secondsSinceLastReceived();

    doc["f_diff"] = f_in - f_out;

    server.setContentLength(measureJson(doc));
    server.send(200, "application/json;charset=utf-8", "");
    serializeJson(doc, server.client());
}

void WebServer::handleRoot() {
    server.send_P(200, "text/html", PAGE_Wetter);
}
