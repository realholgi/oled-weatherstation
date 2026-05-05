#pragma once

#include <Arduino.h>
#include <fws433.h>

class SensorOutdoor {
public:
    SensorOutdoor();

    void begin();
    bool hasPendingPacket();
    void refreshMeasurements();
    IRAM_ATTR void markReadingStale();
    void applyPendingUpdates();

    float humidity() const;
    float temperature() const;
    int batteryStatus() const;
    float absoluteHumidity() const;
    uint32_t secondsSinceLastPacket() const;

private:
    FWS433 receiver;
    volatile float humidityValue;
    volatile float temperatureValue;
    int batteryValue;
    float absoluteHumidityValue;
    volatile uint32_t lastPacketReceivedAtMillis;
    volatile bool staleReadingCheckDue;
};
