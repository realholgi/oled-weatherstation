#include <Arduino.h>
#include <fws433.h>
#include "SensorOutdoor.h"
#include "WetterDebug.h"
#include "HumidityMath.h"
#include "SensorSanity.h"
#include "config.h"

static FWS433 fws;

volatile float humidity_outdoor = 0;
volatile float temperature_outdoor = -273;
volatile int battery_outdoor = 0;
volatile float humidity_abs_outdoor = -1;
volatile uint32_t last_received_ext = 0;

void setupSensorOutdoor() {
    fws.start(RECEIVER_PIN);
    last_received_ext = millis() + MIN_RECEIVE_WAIT_EXT + 1;
}

bool isExternalDataAvailable() {
    return fws.isDataAvailable();
}

void updateExternalSensor() {
    fwsResult result = fws.getData();

    if (result.channel == 3) {
        const float decodedTemperature = result.temperature / 10.0f;

        if (!isPlausibleTemperature(decodedTemperature) || !isPlausibleHumidity(result.humidity)) {
            DEBUG_MSG("Ignoring implausible external reading: %d.%d deg, %u%% REL, ID: %u\n", result.temperature / 10,
                      abs(result.temperature % 10), result.humidity, result.id);
            return;
        }

        last_received_ext = millis();

        humidity_outdoor = result.humidity;
        temperature_outdoor = decodedTemperature;
        battery_outdoor = result.battery ? 1 : 0;

        humidity_abs_outdoor = berechneTT(temperature_outdoor, humidity_outdoor);

        DEBUG_MSG("Temperature: %d.%d deg, Humidity: %u%% REL, ID: %u\n", result.temperature / 10,
                  abs(result.temperature % 10), result.humidity, result.id);
    }
}

ICACHE_RAM_ATTR void setExternalSensorInvalid() {
    if (millis() - last_received_ext >= MAX_RECEIVE_WAIT_EXT) {
        DEBUG_MSG("No External Sensor Signal received for a long time!");
        temperature_outdoor = -273;
        humidity_outdoor = 0;
    }
}
