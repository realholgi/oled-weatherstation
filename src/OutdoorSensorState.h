#pragma once

#include <stdint.h>

namespace OutdoorSensorState {

struct Packet {
    uint8_t channel;
    int16_t temperatureTenths;
    uint8_t humidity;
    bool batteryOk;
};

struct Values {
    bool valid;
    float temperature;
    float humidity;
    bool batteryOk;
    float absoluteHumidity;
    uint32_t lastPacketAtMillis;
};

enum class Decision {
    Accepted,
    WrongChannel,
    Implausible
};

struct ProcessResult {
    Values values;
    Decision decision;
};

Values initial();
ProcessResult processPacket(const Values &current, const Packet &packet, uint8_t expectedChannel, uint32_t nowMillis);
Values applyStale(const Values &current, uint32_t nowMillis, uint32_t maxWaitMillis);
uint32_t secondsSince(uint32_t nowMillis, uint32_t lastPacketAtMillis);

}
