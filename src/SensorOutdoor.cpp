#include <Arduino.h>
#include <fws433.h>
#include "SensorOutdoor.h"
#include "WetterDebug.h"
#include "HumidityMath.h"
#include "SensorSanity.h"
#include "config.h"

SensorOutdoor::SensorOutdoor()
    : humidityValue(0),
      temperatureValue(-273),
      batteryValue(0),
      absoluteHumidityValue(-1),
      lastReceivedAtValue(0) {}

void SensorOutdoor::setup() {
    fws.start(RECEIVER_PIN);
    lastReceivedAtValue = millis() + MIN_RECEIVE_WAIT_EXT + 1;
}

bool SensorOutdoor::isDataAvailable() {
    return fws.isDataAvailable();
}

void SensorOutdoor::update() {
    fwsResult result = fws.getData();

    if (result.channel == 3) {
        const float decodedTemperature = result.temperature / 10.0f;

        if (!SensorSanity::isPlausibleTemperature(decodedTemperature) ||
            !SensorSanity::isPlausibleHumidity(result.humidity)) {
            DEBUG_MSG("Ignoring implausible external reading: %d.%d deg, %u%% REL, ID: %u\n", result.temperature / 10,
                      abs(result.temperature % 10), result.humidity, result.id);
            return;
        }

        lastReceivedAtValue = millis();

        humidityValue = result.humidity;
        temperatureValue = decodedTemperature;
        batteryValue = result.battery ? 1 : 0;

        absoluteHumidityValue = HumidityMath::berechneTT(temperatureValue, humidityValue);

        DEBUG_MSG("Temperature: %d.%d deg, Humidity: %u%% REL, ID: %u\n", result.temperature / 10,
                  abs(result.temperature % 10), result.humidity, result.id);
    }
}

IRAM_ATTR void SensorOutdoor::invalidate() {
    if (millis() - lastReceivedAtValue >= MAX_RECEIVE_WAIT_EXT) {
        DEBUG_MSG("No External Sensor Signal received for a long time!");
        temperatureValue = -273;
        humidityValue = 0;
    }
}

float SensorOutdoor::humidity() const {
    return humidityValue;
}

float SensorOutdoor::temperature() const {
    return temperatureValue;
}

int SensorOutdoor::battery() const {
    return batteryValue;
}

float SensorOutdoor::absoluteHumidity() const {
    return absoluteHumidityValue;
}

uint32_t SensorOutdoor::secondsSinceLastReceived() const {
    return (millis() - lastReceivedAtValue) / 1000;
}
