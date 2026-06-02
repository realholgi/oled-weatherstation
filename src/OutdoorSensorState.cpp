#include "OutdoorSensorState.h"
#include "SensorSanity.h"
#include "HumidityMath.h"

namespace OutdoorSensorState {

Values initial() {
    Values v;
    v.valid = false;
    v.temperature = -273.0f;
    v.humidity = 0.0f;
    v.batteryOk = false;
    v.absoluteHumidity = -1.0f;
    v.lastPacketAtMillis = 0;
    return v;
}

ProcessResult processPacket(const Values &current, const Packet &packet, uint8_t expectedChannel, uint32_t nowMillis) {
    if (packet.channel != expectedChannel) {
        return { current, Decision::WrongChannel };
    }

    const float decodedTemperature = packet.temperatureTenths / 10.0f;

    if (!SensorSanity::isPlausibleTemperature(decodedTemperature) ||
        !SensorSanity::isPlausibleHumidity(packet.humidity)) {
        return { current, Decision::Implausible };
    }

    Values updated;
    updated.valid = true;
    updated.temperature = decodedTemperature;
    updated.humidity = static_cast<float>(packet.humidity);
    updated.batteryOk = packet.batteryOk;
    updated.absoluteHumidity = static_cast<float>(
        HumidityMath::calculateAbsoluteHumidity(decodedTemperature, packet.humidity));
    updated.lastPacketAtMillis = nowMillis;

    return { updated, Decision::Accepted };
}

Values applyStale(const Values &current, uint32_t nowMillis, uint32_t maxWaitMillis) {
    if (nowMillis - current.lastPacketAtMillis < maxWaitMillis) {
        return current;
    }

    Values stale = current;
    stale.valid = false;
    stale.temperature = -273.0f;
    stale.humidity = 0.0f;
    stale.absoluteHumidity = -1.0f;
    // batteryOk and lastPacketAtMillis are preserved
    return stale;
}

uint32_t secondsSince(uint32_t nowMillis, uint32_t lastPacketAtMillis) {
    return (nowMillis - lastPacketAtMillis) / 1000;
}

}
