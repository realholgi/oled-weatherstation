#pragma once

#include <Arduino.h>
#include <fws433.h>

class SensorOutdoor {
public:
    SensorOutdoor();

    void begin(uint8_t expectedChannel);
    bool hasPendingPacket();
    void refreshMeasurements();
    IRAM_ATTR void markReadingStale();
    void applyPendingUpdates();

    bool isValid() const;
    float humidity() const;
    float temperature() const;
    bool batteryStatus() const;
    float absoluteHumidity() const;
    uint32_t secondsSinceLastPacket() const;

private:
    FWS433 receiver;
    uint8_t expectedChannelValue;
    bool validValue;
    float humidityValue;
    float temperatureValue;
    bool batteryValue;
    float absoluteHumidityValue;
    uint32_t lastPacketReceivedAtMillis;
    // Ticker callbacks set this flag; loop() consumes it via applyPendingUpdates()
    volatile bool staleReadingCheckDue;
};
