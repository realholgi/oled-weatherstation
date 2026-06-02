#pragma once

#include <stdint.h>

namespace DataJsonPayload {

struct IndoorInputs {
    bool sensorIsValid;
    float temperature;
    float humidity;
    float absoluteHumidity;
    float dewPoint;
};

struct OutdoorInputs {
    bool sensorIsValid;
    float temperature;
    float humidity;
    float absoluteHumidity;
    bool batteryOk;
    uint32_t secondsSinceLastPacket;
};

struct Payload {
    bool indoorValid;
    bool outdoorValid;
    // Indoor fields (meaningful only when indoorValid == true)
    float indoorTemperatureCelsius;
    int   indoorHumidityPercent;      // int cast per D-07
    float indoorAbsoluteHumidityGm3;
    float indoorDewPointCelsius;
    // Outdoor fields (meaningful only when outdoorValid == true)
    float outdoorTemperatureCelsius;
    int   outdoorHumidityPercent;     // int cast per D-07
    float outdoorAbsoluteHumidityGm3;
    bool  outdoorBatteryOk;
    // Always-present fields
    uint32_t outdoorSecondsSinceLastReading;
    float ventingThresholdGm3;
    // Advice fields (meaningful only when indoorValid && outdoorValid)
    bool hasAdvice;
    float absoluteHumidityDifferenceGm3;
    const char* ventingRecommendation;   // "vent"/"marginal"/"wait" or nullptr
};

Payload build(
    const IndoorInputs& indoor,
    const OutdoorInputs& outdoor,
    float ventingThreshold
);

}  // namespace DataJsonPayload
