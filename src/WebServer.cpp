#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "BuildInfoGenerated.h"
#include "WebServer.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "SensorSanity.h"
#include "VentingAdvice.h"
#include "config.h"
#include "PAGE_weather.h"
#include "TimeClient.h"
#include "WebNotFound.h"
#include "DataJsonPayload.h"

WebServer::WebServer() : server(80) {
}

void WebServer::begin(SensorIndoor &indoorSensor, SensorOutdoor &outdoorSensor, const TimeClient &timeClient, bool advertiseMdns, const String &webLanguage, float threshold) {
    indoorSensorRef = &indoorSensor;
    outdoorSensorRef = &outdoorSensor;
    timeClientRef = &timeClient;
    pageLanguage = (webLanguage == "en") ? "en" : "de";
    ventingThreshold = threshold;
    server.on("/", [this]() { handleRoot(); });
    server.on("/data.json", HTTP_GET, [this]() {
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Cache-Control", "no-store");
        handleDataJson();
    });
    server.onNotFound([this]() { handleNotFound(); });
    server.collectHeaders("If-None-Match");
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
    server.send(404, WebNotFound::CONTENT_TYPE, WebNotFound::BODY);
}

void WebServer::handleDataJson() {
    JsonDocument jsonDocument;

    const SensorIndoor &indoorSensor = *indoorSensorRef;
    const SensorOutdoor &outdoorSensor = *outdoorSensorRef;

    DataJsonPayload::IndoorInputs indoor;
    indoor.sensorIsValid    = indoorSensor.isValid();
    indoor.temperature      = indoorSensor.temperature();
    indoor.humidity         = indoorSensor.humidity();
    indoor.absoluteHumidity = indoorSensor.absoluteHumidity();
    indoor.dewPoint         = indoorSensor.dewPoint();

    DataJsonPayload::OutdoorInputs outdoor;
    outdoor.sensorIsValid          = outdoorSensor.isValid();
    outdoor.temperature            = outdoorSensor.temperature();
    outdoor.humidity               = outdoorSensor.humidity();
    outdoor.absoluteHumidity       = outdoorSensor.absoluteHumidity();
    outdoor.batteryOk              = outdoorSensor.batteryStatus();
    outdoor.secondsSinceLastPacket = outdoorSensor.secondsSinceLastPacket();

    const DataJsonPayload::Payload payload = DataJsonPayload::build(indoor, outdoor, ventingThreshold, timeClientRef->isTimeSet());

    jsonDocument["indoorValid"]  = payload.indoorValid;
    jsonDocument["outdoorValid"] = payload.outdoorValid;

    if (payload.indoorValid) {
        jsonDocument["indoorTemperatureCelsius"]  = payload.indoorTemperatureCelsius;
        jsonDocument["indoorHumidityPercent"]     = payload.indoorHumidityPercent;
        jsonDocument["indoorAbsoluteHumidityGm3"] = payload.indoorAbsoluteHumidityGm3;
        jsonDocument["indoorDewPointCelsius"]     = payload.indoorDewPointCelsius;
    } else {
        jsonDocument["indoorTemperatureCelsius"]  = nullptr;
        jsonDocument["indoorHumidityPercent"]     = nullptr;
        jsonDocument["indoorAbsoluteHumidityGm3"] = nullptr;
        jsonDocument["indoorDewPointCelsius"]     = nullptr;
    }

    if (payload.outdoorValid) {
        jsonDocument["outdoorTemperatureCelsius"]  = payload.outdoorTemperatureCelsius;
        jsonDocument["outdoorHumidityPercent"]     = payload.outdoorHumidityPercent;
        jsonDocument["outdoorAbsoluteHumidityGm3"] = payload.outdoorAbsoluteHumidityGm3;
        jsonDocument["outdoorBatteryOk"]           = payload.outdoorBatteryOk;
    } else {
        jsonDocument["outdoorTemperatureCelsius"]  = nullptr;
        jsonDocument["outdoorHumidityPercent"]     = nullptr;
        jsonDocument["outdoorAbsoluteHumidityGm3"] = nullptr;
        jsonDocument["outdoorBatteryOk"]           = nullptr;
    }

    jsonDocument["outdoorSecondsSinceLastReading"] = payload.outdoorSecondsSinceLastReading;
    jsonDocument["ventingThresholdGm3"]            = payload.ventingThresholdGm3;
    jsonDocument["timeSynced"]                     = payload.timeSynced;

    if (payload.hasAdvice) {
        jsonDocument["absoluteHumidityDifferenceGm3"] = payload.absoluteHumidityDifferenceGm3;
        jsonDocument["ventingRecommendation"]          = payload.ventingRecommendation;
    } else {
        jsonDocument["absoluteHumidityDifferenceGm3"] = nullptr;
        jsonDocument["ventingRecommendation"]          = nullptr;
    }

    server.setContentLength(measureJson(jsonDocument));
    server.send(200, "application/json;charset=utf-8", "");
    serializeJson(jsonDocument, server.client());
}

void WebServer::handleRoot() {
    // HTML content varies with firmware version and configured default language
    const String etag = String("\"" VERSION_STRING "-") + pageLanguage + "\"";
    if (server.header("If-None-Match") == etag) {
        server.send(304, "text/html", "");
        return;
    }
    server.sendHeader("ETag", etag);
    server.sendHeader("Cache-Control", "max-age=86400");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent_P(PAGE_WEATHER_1);
    server.sendContent(pageLanguage);
    server.sendContent_P(PAGE_WEATHER_2);
    server.sendContent(pageLanguage);
    server.sendContent_P(PAGE_WEATHER_3);
    server.sendContent("");
}
