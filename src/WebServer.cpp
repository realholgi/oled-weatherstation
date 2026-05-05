#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "WebServer.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "SensorSanity.h"
#include "config.h"
#include "PAGE_weather.h"

WebServer::WebServer() : server(80) {
}

void WebServer::begin(SensorIndoor &indoorSensor, SensorOutdoor &outdoorSensor, bool advertiseMdns, const String &webLanguage) {
    indoorSensorRef = &indoorSensor;
    outdoorSensorRef = &outdoorSensor;
    pageLanguage = (webLanguage == "en") ? "en" : "de";
    server.on("/", [this]() { handleRoot(); });
    server.on("/data.json", HTTP_GET, [this]() {
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        handleDataJson();
    });
    server.onNotFound([this]() { handleNotFound(); });
    server.begin();
    started = true;
    if (advertiseMdns) {
        MDNS.addService("http", "tcp", 80);
    }
}

void WebServer::handleClient() {
    if (!started) return;
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
    const uint32_t outdoorSecondsSinceLastReading = outdoorSensor.secondsSinceLastPacket();
    const bool indoorValid =
        SensorSanity::isPlausibleTemperature(indoorSensor.temperature()) &&
        SensorSanity::isPlausibleHumidity(indoorSensor.humidity()) &&
        SensorSanity::isPlausibleHumidity(indoorAbsoluteHumidity);
    const bool outdoorValid =
        SensorSanity::isPlausibleTemperature(outdoorSensor.temperature()) &&
        SensorSanity::isPlausibleHumidity(outdoorSensor.humidity()) &&
        SensorSanity::isPlausibleHumidity(outdoorAbsoluteHumidity) &&
        outdoorSecondsSinceLastReading <= MAX_RECEIVE_WAIT_EXT_S;
    const bool outdoorStale = !outdoorValid;

    jsonDocument["indoorValid"] = indoorValid;
    jsonDocument["outdoorValid"] = outdoorValid;
    jsonDocument["outdoorStale"] = outdoorStale;

    if (indoorValid) {
        jsonDocument["indoorTemperatureCelsius"] = indoorSensor.temperature();
        jsonDocument["indoorHumidityPercent"] = int(indoorSensor.humidity());
        jsonDocument["indoorAbsoluteHumidityGm3"] = indoorAbsoluteHumidity;
        jsonDocument["indoorDewPointCelsius"] = indoorSensor.dewPoint();
    } else {
        jsonDocument["indoorTemperatureCelsius"] = nullptr;
        jsonDocument["indoorHumidityPercent"] = nullptr;
        jsonDocument["indoorAbsoluteHumidityGm3"] = nullptr;
        jsonDocument["indoorDewPointCelsius"] = nullptr;
    }

    if (outdoorValid) {
        jsonDocument["outdoorTemperatureCelsius"] = outdoorSensor.temperature();
        jsonDocument["outdoorHumidityPercent"] = int(outdoorSensor.humidity());
        jsonDocument["outdoorAbsoluteHumidityGm3"] = outdoorAbsoluteHumidity;
        jsonDocument["outdoorBatteryOk"] = outdoorSensor.batteryStatus();
    } else {
        jsonDocument["outdoorTemperatureCelsius"] = nullptr;
        jsonDocument["outdoorHumidityPercent"] = nullptr;
        jsonDocument["outdoorAbsoluteHumidityGm3"] = nullptr;
        jsonDocument["outdoorBatteryOk"] = nullptr;
    }
    jsonDocument["outdoorSecondsSinceLastReading"] = outdoorSecondsSinceLastReading;

    if (indoorValid && outdoorValid) {
        jsonDocument["absoluteHumidityDifferenceGm3"] = indoorAbsoluteHumidity - outdoorAbsoluteHumidity;
    } else {
        jsonDocument["absoluteHumidityDifferenceGm3"] = nullptr;
    }

    server.setContentLength(measureJson(jsonDocument));
    server.send(200, "application/json;charset=utf-8", "");
    serializeJson(jsonDocument, server.client());
}

void WebServer::handleRoot() {
    String page = FPSTR(PAGE_Weather);
    page.replace("__DEFAULT_LANG__", pageLanguage);
    server.send(200, "text/html", page);
}
