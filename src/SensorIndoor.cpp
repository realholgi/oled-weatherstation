#include <Arduino.h>
#include <Wire.h>
#include "SensorIndoor.h"
#include "HumidityMath.h"
#include "SensorSanity.h"

SensorIndoor::SensorIndoor()
    : humidityValue(0),
      temperatureValue(-273),
      absoluteHumidityValue(-1),
      dewPointValue(-273),
      readyForUpdateFlag(true) {}

bool SensorIndoor::setup() {
    return htu.begin();
}

void SensorIndoor::update() {
    readyForUpdateFlag = false;
    humidityValue = htu.readHumidity();
    temperatureValue = htu.readTemperature();

    if (SensorSanity::isPlausibleTemperature(temperatureValue) &&
        SensorSanity::isPlausibleHumidity(humidityValue)) {
        absoluteHumidityValue = HumidityMath::berechneTT(temperatureValue, humidityValue);
        dewPointValue = HumidityMath::RHtoDP(temperatureValue, humidityValue);
    }
}

bool SensorIndoor::isReadyForUpdate() const {
    return readyForUpdateFlag;
}

ICACHE_RAM_ATTR void SensorIndoor::setReadyForUpdate() {
    readyForUpdateFlag = true;
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
