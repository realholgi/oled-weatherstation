#include <Arduino.h>
#include <fws433.h>
#include "SensorOutdoor.h"
#include "OutdoorSensorState.h"
#include "WeatherDebug.h"
#include "config.h"

SensorOutdoor::SensorOutdoor()
    : expectedChannelValue(DEFAULT_OUTDOOR_SENSOR_CHANNEL),
      validValue(false),
      humidityValue(0),
      temperatureValue(-273),
      batteryValue(false),
      absoluteHumidityValue(-1),
      lastPacketReceivedAtMillis(0),
      staleReadingCheckDue(false) {}

void SensorOutdoor::begin(uint8_t expectedChannel) {
    expectedChannelValue = expectedChannel;
    receiver.start(RECEIVER_PIN);
    // lastPacketReceivedAtMillis remains 0 until first accepted packet;
    // receiver start time is not treated as freshness
}

bool SensorOutdoor::hasPendingPacket() {
    return receiver.isDataAvailable();
}

void SensorOutdoor::refreshMeasurements() {
    fwsResult receivedPacket = receiver.getData();

    OutdoorSensorState::Values current;
    current.valid = validValue;
    current.temperature = temperatureValue;
    current.humidity = humidityValue;
    current.batteryOk = batteryValue;
    current.absoluteHumidity = absoluteHumidityValue;
    current.lastPacketAtMillis = lastPacketReceivedAtMillis;

    OutdoorSensorState::Packet pkt;
    pkt.channel = receivedPacket.channel;
    pkt.temperatureTenths = receivedPacket.temperature;
    pkt.humidity = receivedPacket.humidity;
    pkt.batteryOk = receivedPacket.battery;

    OutdoorSensorState::ProcessResult result =
        OutdoorSensorState::processPacket(current, pkt, expectedChannelValue, millis());

    if (result.decision == OutdoorSensorState::Decision::Accepted) {
        validValue = result.values.valid;
        temperatureValue = result.values.temperature;
        humidityValue = result.values.humidity;
        batteryValue = result.values.batteryOk;
        absoluteHumidityValue = result.values.absoluteHumidity;
        lastPacketReceivedAtMillis = result.values.lastPacketAtMillis;
        DEBUG_MSG("Temperature: %d.%d deg, Humidity: %u%% REL, ID: %u\n", receivedPacket.temperature / 10,
                  abs(receivedPacket.temperature % 10), receivedPacket.humidity, receivedPacket.id);
    } else if (result.decision == OutdoorSensorState::Decision::Implausible) {
        DEBUG_MSG("Ignoring implausible external reading: %d.%d deg, %u%% REL, ID: %u\n", receivedPacket.temperature / 10,
                  abs(receivedPacket.temperature % 10), receivedPacket.humidity, receivedPacket.id);
        // state unchanged
    }
    // WrongChannel: return without logging and without changing state
}

IRAM_ATTR void SensorOutdoor::markReadingStale() {
    staleReadingCheckDue = true;
}

void SensorOutdoor::applyPendingUpdates() {
    if (!staleReadingCheckDue) return;

    staleReadingCheckDue = false;

    OutdoorSensorState::Values current;
    current.valid = validValue;
    current.temperature = temperatureValue;
    current.humidity = humidityValue;
    current.batteryOk = batteryValue;
    current.absoluteHumidity = absoluteHumidityValue;
    current.lastPacketAtMillis = lastPacketReceivedAtMillis;

    OutdoorSensorState::Values after =
        OutdoorSensorState::applyStale(current, millis(), MAX_RECEIVE_WAIT_EXT);

    if (!after.valid && current.valid) {
        DEBUG_MSG("No External Sensor Signal received for a long time!");
    }

    validValue = after.valid;
    temperatureValue = after.temperature;
    humidityValue = after.humidity;
    // batteryValue and lastPacketReceivedAtMillis are preserved by applyStale
    batteryValue = after.batteryOk;
    absoluteHumidityValue = after.absoluteHumidity;
    lastPacketReceivedAtMillis = after.lastPacketAtMillis;
}

bool SensorOutdoor::isValid() const {
    return validValue;
}

float SensorOutdoor::humidity() const {
    return humidityValue;
}

float SensorOutdoor::temperature() const {
    return temperatureValue;
}

bool SensorOutdoor::batteryStatus() const {
    return batteryValue;
}

float SensorOutdoor::absoluteHumidity() const {
    return absoluteHumidityValue;
}

uint32_t SensorOutdoor::secondsSinceLastPacket() const {
    return OutdoorSensorState::secondsSince(millis(), lastPacketReceivedAtMillis);
}
