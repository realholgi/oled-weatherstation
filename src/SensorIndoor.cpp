#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_HTU21DF.h>
#include "SensorIndoor.h"
#include "HumidityMath.h"
#include "SensorSanity.h"

static Adafruit_HTU21DF htu;

float humidity_indoor = 0;
float temperature_indoor = -273;
float humidity_abs_indoor = -1;
float dp_indoor = -273;
volatile bool readyForInternalSensorUpdate = true;

bool setupSensorIndoor() {
    return htu.begin();
}

void updateInternalSensor() {
    readyForInternalSensorUpdate = false;
    humidity_indoor = htu.readHumidity();
    temperature_indoor = htu.readTemperature();

    if (isPlausibleTemperature(temperature_indoor) && isPlausibleHumidity(humidity_indoor)) {
        humidity_abs_indoor = berechneTT(temperature_indoor, humidity_indoor);
        dp_indoor = RHtoDP(temperature_indoor, humidity_indoor);
    }
}

ICACHE_RAM_ATTR void setReadyForInternalSensorUpdate() {
    readyForInternalSensorUpdate = true;
}
