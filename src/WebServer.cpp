#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "WebServer.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "PAGE_weather.h"

WebServer::WebServer() : server(80) {
}

void WebServer::begin(SensorIndoor &indoorSensor, SensorOutdoor &outdoorSensor) {
    indoorSensorRef = &indoorSensor;
    outdoorSensorRef = &outdoorSensor;
    server.on("/", [this]() { handleRoot(); });
    server.on("/data.json", HTTP_GET, [this]() {
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        handleDataJson();
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

void WebServer::handleDataJson() {
    JsonDocument jsonDocument;

    const SensorIndoor &indoorSensor = *indoorSensorRef;
    const SensorOutdoor &outdoorSensor = *outdoorSensorRef;

    const float indoorAbsoluteHumidity = indoorSensor.absoluteHumidity();
    const float outdoorAbsoluteHumidity = outdoorSensor.absoluteHumidity();

    jsonDocument["indoorTemperatureCelsius"] = indoorSensor.temperature();
    jsonDocument["indoorHumidityPercent"] = int(indoorSensor.humidity());
    jsonDocument["indoorAbsoluteHumidityGm3"] = indoorAbsoluteHumidity;
    jsonDocument["indoorDewPointCelsius"] = indoorSensor.dewPoint();

    jsonDocument["outdoorTemperatureCelsius"] = outdoorSensor.temperature();
    jsonDocument["outdoorHumidityPercent"] = int(outdoorSensor.humidity());
    jsonDocument["outdoorAbsoluteHumidityGm3"] = outdoorAbsoluteHumidity;
    jsonDocument["outdoorBatteryOk"] = outdoorSensor.batteryStatus();
    jsonDocument["outdoorSecondsSinceLastReading"] = outdoorSensor.secondsSinceLastPacket();

    jsonDocument["absoluteHumidityDifferenceGm3"] = indoorAbsoluteHumidity - outdoorAbsoluteHumidity;

    server.setContentLength(measureJson(jsonDocument));
    server.send(200, "application/json;charset=utf-8", "");
    serializeJson(jsonDocument, server.client());
}

void WebServer::handleRoot() {
    server.send_P(200, "text/html", PAGE_Weather);
}
