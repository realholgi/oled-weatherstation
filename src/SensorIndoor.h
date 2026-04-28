#pragma once

#include <Arduino.h>
#include <Adafruit_HTU21DF.h>

class SensorIndoor {
public:
    SensorIndoor();

    bool setup();
    void update();
    bool isReadyForUpdate() const;
    ICACHE_RAM_ATTR void setReadyForUpdate();

    float humidity() const;
    float temperature() const;
    float absoluteHumidity() const;
    float dewPoint() const;

private:
    Adafruit_HTU21DF htu;
    float humidityValue;
    float temperatureValue;
    float absoluteHumidityValue;
    float dewPointValue;
    volatile bool readyForUpdateFlag;
};
