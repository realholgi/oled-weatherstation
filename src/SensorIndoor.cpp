#include <Arduino.h>
#include <Wire.h>
#include "SensorIndoor.h"
#include "HumidityMath.h"
#include "SensorSanity.h"
#include "config.h"

SensorIndoor::SensorIndoor()
    : humidityValue(0),
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

    if (SensorSanity::isPlausibleTemperature(rawTemperature) &&
        SensorSanity::isPlausibleHumidity(rawHumidity)) {
        humidityValue = rawHumidity;
        temperatureValue = rawTemperature - temperatureOffset;
        absoluteHumidityValue = HumidityMath::calculateAbsoluteHumidity(temperatureValue, humidityValue);
        dewPointValue = HumidityMath::calculateDewPoint(temperatureValue, humidityValue);
    }
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
