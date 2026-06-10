#include "DataJsonPayload.h"
#include "SensorSanity.h"
#include "VentingAdvice.h"
#include "config.h"

namespace DataJsonPayload {

Payload build(
    const IndoorInputs& indoor,
    const OutdoorInputs& outdoor,
    float ventingThreshold,
    bool timeSynced
) {
    Payload p{};

    const bool indoorValid =
        indoor.sensorIsValid &&
        SensorSanity::isPlausibleTemperature(indoor.temperature) &&
        SensorSanity::isPlausibleHumidity(indoor.humidity) &&
        SensorSanity::isPlausibleAbsoluteHumidity(indoor.absoluteHumidity);

    const bool outdoorValid =
        outdoor.sensorIsValid &&
        SensorSanity::isPlausibleTemperature(outdoor.temperature) &&
        SensorSanity::isPlausibleHumidity(outdoor.humidity) &&
        SensorSanity::isPlausibleAbsoluteHumidity(outdoor.absoluteHumidity) &&
        outdoor.secondsSinceLastPacket <= MAX_RECEIVE_WAIT_EXT_S;

    p.indoorValid  = indoorValid;
    p.outdoorValid = outdoorValid;

    if (indoorValid) {
        p.indoorTemperatureCelsius  = indoor.temperature;
        p.indoorHumidityPercent     = int(indoor.humidity);
        p.indoorAbsoluteHumidityGm3 = indoor.absoluteHumidity;
        p.indoorDewPointCelsius     = indoor.dewPoint;
    }

    if (outdoorValid) {
        p.outdoorTemperatureCelsius  = outdoor.temperature;
        p.outdoorHumidityPercent     = int(outdoor.humidity);
        p.outdoorAbsoluteHumidityGm3 = outdoor.absoluteHumidity;
        p.outdoorBatteryOk           = outdoor.batteryOk;
    }

    p.outdoorSecondsSinceLastReading = outdoor.secondsSinceLastPacket;
    p.ventingThresholdGm3            = ventingThreshold;
    p.timeSynced                     = timeSynced;

    if (indoorValid && outdoorValid) {
        const VentingAdvice::Result advice = VentingAdvice::calculate(
            indoor.absoluteHumidity, outdoor.absoluteHumidity, ventingThreshold);
        p.hasAdvice                      = true;
        p.absoluteHumidityDifferenceGm3  = advice.difference;
        p.ventingRecommendation =
            advice.recommendation == VentingAdvice::Recommendation::VENT     ? "vent"
          : advice.recommendation == VentingAdvice::Recommendation::MARGINAL ? "marginal"
                                                                              : "wait";
    } else {
        p.hasAdvice             = false;
        p.ventingRecommendation = nullptr;
    }

    return p;
}

}  // namespace DataJsonPayload
