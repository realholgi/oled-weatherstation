#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "WebServer.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "config.h"
#include "PAGE_wetter.h"

ESP8266WebServer HTTP(80);

void setupWebserver() {
    HTTP.on("/", handleRoot);
    HTTP.on("/data.json", HTTP_GET, [&]() {
        HTTP.sendHeader("Connection", "close");
        HTTP.sendHeader("Access-Control-Allow-Origin", "*");
        return handleJsonData();
    });
    HTTP.onNotFound(handleNotFound);
    HTTP.begin();
    MDNS.addService("http", "tcp", 80);
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

    doc["t_in"] = temperature_indoor - TEMP_OFFSET_INDOOR;
    doc["h_in"] = int(humidity_indoor);
    doc["f_in"] = humidity_abs_indoor;
    doc["dp_in"] = dp_indoor;

    doc["t_out"] = temperature_outdoor;
    doc["h_out"] = int(humidity_outdoor);
    doc["f_out"] = humidity_abs_outdoor;
    doc["b_out"] = battery_outdoor;
    doc["last_out"] = (millis() - last_received_ext) / 1000;

    doc["f_diff"] = humidity_abs_indoor - humidity_abs_outdoor;

    String message = "";
    serializeJson(doc, message);

    HTTP.send(200, "application/json;charset=utf-8", message);
}

void handleRoot() {
    HTTP.send_P(200, "text/html", PAGE_Wetter);
}
