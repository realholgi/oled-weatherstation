#pragma once

#include "config.h"

extern volatile float humidity_outdoor;
extern volatile float temperature_outdoor;
extern volatile int battery_outdoor;
extern volatile float humidity_abs_outdoor;
extern volatile uint32_t last_received_ext;

void setupSensorOutdoor();
bool isExternalDataAvailable();
void updateExternalSensor();
ICACHE_RAM_ATTR void setExternalSensorInvalid();
