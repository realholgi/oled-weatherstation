#pragma once

#include <Arduino.h>
#include <Adafruit_HTU21DF.h>

class SensorIndoor {
public:
    SensorIndoor();

    bool begin();
    void refreshMeasurements();
    bool isMeasurementDue() const;
    IRAM_ATTR void markMeasurementDue();
    void setTemperatureOffset(float offset);

    float humidity() const;
    float temperature() const;
    float absoluteHumidity() const;
    float dewPoint() const;

private:
    Adafruit_HTU21DF sensorChip;
    float humidityValue;
    float temperatureValue;
    float absoluteHumidityValue;
    float dewPointValue;
    float temperatureOffset;
    volatile bool measurementDueFlag;
};
