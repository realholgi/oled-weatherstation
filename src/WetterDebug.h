#pragma once

#include <Arduino.h>

#ifdef DEBUG_ESP_PORT
#include <HardwareSerial.h>

#define DEBUG_SETUP() DEBUG_ESP_PORT.begin(115200)
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf(__VA_ARGS__)
#else
#define DEBUG_SETUP(...)
#define DEBUG_MSG(...)
#endif
