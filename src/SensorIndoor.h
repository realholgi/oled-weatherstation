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

    bool isValid() const;
    float humidity() const;
    float temperature() const;
    float absoluteHumidity() const;
    float dewPoint() const;

private:
    Adafruit_HTU21DF sensorChip;
    bool validValue;
    float humidityValue;
    float temperatureValue;
    float absoluteHumidityValue;
    float dewPointValue;
    float temperatureOffset;
    // Ticker callback sets this flag; loop() consumes it via isMeasurementDue() / refreshMeasurements()
    volatile bool measurementDueFlag;
};
