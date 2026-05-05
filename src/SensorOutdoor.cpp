#include <Arduino.h>
#include <fws433.h>
#include "SensorOutdoor.h"
#include "WeatherDebug.h"
#include "HumidityMath.h"
#include "SensorSanity.h"
#include "config.h"

SensorOutdoor::SensorOutdoor()
    : expectedChannelValue(OUTDOOR_SENSOR_CHANNEL),
      humidityValue(0),
      temperatureValue(-273),
      batteryValue(0),
      absoluteHumidityValue(-1),
      lastPacketReceivedAtMillis(0),
      staleReadingCheckDue(false) {}

void SensorOutdoor::begin(uint8_t expectedChannel) {
    expectedChannelValue = expectedChannel;
    receiver.start(RECEIVER_PIN);
    lastPacketReceivedAtMillis = millis();
}

bool SensorOutdoor::hasPendingPacket() {
    return receiver.isDataAvailable();
}

void SensorOutdoor::refreshMeasurements() {
    fwsResult receivedPacket = receiver.getData();

    if (receivedPacket.channel == expectedChannelValue) {
        const float decodedTemperature = receivedPacket.temperature / 10.0f;

        if (!SensorSanity::isPlausibleTemperature(decodedTemperature) ||
            !SensorSanity::isPlausibleHumidity(receivedPacket.humidity)) {
            DEBUG_MSG("Ignoring implausible external reading: %d.%d deg, %u%% REL, ID: %u\n", receivedPacket.temperature / 10,
                      abs(receivedPacket.temperature % 10), receivedPacket.humidity, receivedPacket.id);
            return;
        }

        const uint32_t receivedAtMillis = millis();
        const float absoluteHumidity = HumidityMath::calculateAbsoluteHumidity(decodedTemperature, receivedPacket.humidity);

        noInterrupts();
        lastPacketReceivedAtMillis = receivedAtMillis;
        humidityValue = receivedPacket.humidity;
        temperatureValue = decodedTemperature;
        batteryValue = receivedPacket.battery ? 1 : 0;
        absoluteHumidityValue = absoluteHumidity;
        interrupts();

        DEBUG_MSG("Temperature: %d.%d deg, Humidity: %u%% REL, ID: %u\n", receivedPacket.temperature / 10,
                  abs(receivedPacket.temperature % 10), receivedPacket.humidity, receivedPacket.id);
    }
}

IRAM_ATTR void SensorOutdoor::markReadingStale() {
    staleReadingCheckDue = true;
}

void SensorOutdoor::applyPendingUpdates() {
    if (!staleReadingCheckDue) return;

    staleReadingCheckDue = false;
    if (millis() - lastPacketReceivedAtMillis >= MAX_RECEIVE_WAIT_EXT) {
        DEBUG_MSG("No External Sensor Signal received for a long time!");
        temperatureValue = -273;
        humidityValue = 0;
        absoluteHumidityValue = -1;
    }
}

float SensorOutdoor::humidity() const {
    return humidityValue;
}

float SensorOutdoor::temperature() const {
    return temperatureValue;
}

int SensorOutdoor::batteryStatus() const {
    return batteryValue;
}

float SensorOutdoor::absoluteHumidity() const {
    return absoluteHumidityValue;
}

uint32_t SensorOutdoor::secondsSinceLastPacket() const {
    const uint32_t now = millis();
    if (now < lastPacketReceivedAtMillis) return 0;
    return (now - lastPacketReceivedAtMillis) / 1000;
}
