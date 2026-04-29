#pragma once

#include <Arduino.h>
#include <fws433.h>

class SensorOutdoor {
public:
    SensorOutdoor();

    void setup();
    bool isDataAvailable();
    void update();
    IRAM_ATTR void invalidate();

    float humidity() const;
    float temperature() const;
    int battery() const;
    float absoluteHumidity() const;
    uint32_t secondsSinceLastReceived() const;

private:
    FWS433 fws;
    volatile float humidityValue;
    volatile float temperatureValue;
    volatile int batteryValue;
    volatile float absoluteHumidityValue;
    volatile uint32_t lastReceivedAtValue;
};
