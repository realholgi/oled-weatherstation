#include <Arduino.h>
#include <Wire.h>
#include "SensorIndoor.h"
#include "IndoorSensorState.h"
#include "config.h"

SensorIndoor::SensorIndoor()
    : validValue(false),
      humidityValue(0),
      temperatureValue(-273),
      absoluteHumidityValue(-1),
      dewPointValue(-273),
      temperatureOffset(DEFAULT_TEMP_OFFSET_INDOOR),
      measurementDueFlag(true) {}

bool SensorIndoor::begin() {
    return sensorChip.begin();
}

void SensorIndoor::refreshMeasurements() {
    measurementDueFlag = false;
    const float rawHumidity = sensorChip.readHumidity();
    const float rawTemperature = sensorChip.readTemperature();

    IndoorSensorState::Values state = IndoorSensorState::fromReading(rawTemperature, rawHumidity, temperatureOffset);
    validValue = state.valid;
    temperatureValue = state.temperature;
    humidityValue = state.humidity;
    absoluteHumidityValue = state.absoluteHumidity;
    dewPointValue = state.dewPoint;
}

bool SensorIndoor::isMeasurementDue() const {
    return measurementDueFlag;
}

IRAM_ATTR void SensorIndoor::markMeasurementDue() {
    measurementDueFlag = true;
}

void SensorIndoor::setTemperatureOffset(float offset) {
    temperatureOffset = offset;
}

bool SensorIndoor::isValid() const {
    return validValue;
}

float SensorIndoor::humidity() const {
    return humidityValue;
}

float SensorIndoor::temperature() const {
    return temperatureValue;
}

float SensorIndoor::absoluteHumidity() const {
    return absoluteHumidityValue;
}

float SensorIndoor::dewPoint() const {
    return dewPointValue;
}
