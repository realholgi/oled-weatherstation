#pragma once

#include <ESP8266WebServer.h>

extern ESP8266WebServer HTTP;

void setupWebserver();
void handleRoot();
void handleNotFound();
void handleJsonData();
