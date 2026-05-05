#pragma once

#include <Arduino.h>
#include <Adafruit_HTU21DF.h>

class SensorIndoor {
public:
    SensorIndoor();

    bool setup();
    void update();
    bool isReadyForUpdate() const;
    IRAM_ATTR void setReadyForUpdate();
    void setTemperatureOffset(float offset);

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
    float temperatureOffset;
    volatile bool readyForUpdateFlag;
};
