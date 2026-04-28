#pragma once

#include "config.h"

extern float humidity_indoor;
extern float temperature_indoor;
extern float humidity_abs_indoor;
extern float dp_indoor;
extern volatile bool readyForInternalSensorUpdate;

bool setupSensorIndoor();
void updateInternalSensor();
ICACHE_RAM_ATTR void setReadyForInternalSensorUpdate();
