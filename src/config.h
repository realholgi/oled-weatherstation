#pragma once

#include <Arduino.h>

#define FIRMWAREVERSION "0.9.1"
#define HOSTNAME "wetter"
#define OLED_RESET D3
#define RECEIVER_PIN D6

#define MIN_DIFF 3.0f
#define MIN_RECEIVE_WAIT_INT 2
#define MIN_RECEIVE_WAIT_EXT 200
#define MAX_RECEIVE_WAIT_EXT (120 * 1000)
#define UPDATE_NTP_TIME_INTERVAL 3600

#define OFFSET 1
#define TEMP_OFFSET_INDOOR 1.0f
#define TIMEZONE 1

#define DRD_TIMEOUT 2
#define DRD_ADDRESS 0

#define PORTAL_DEFAULT_PASSWORD "password"
#define CONFIG_FILE "/config.json"

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif
