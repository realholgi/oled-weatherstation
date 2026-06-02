#include "IndoorSensorState.h"
#include "SensorSanity.h"
#include "HumidityMath.h"

namespace IndoorSensorState {

Values invalid() {
    return {false, -273.0f, 0.0f, -1.0f, -273.0f};
}

Values fromReading(float rawTemperature, float rawHumidity, float temperatureOffset) {
    if (!SensorSanity::isPlausibleTemperature(rawTemperature) ||
        !SensorSanity::isPlausibleHumidity(rawHumidity)) {
        return invalid();
    }

    float adjustedTemperature = rawTemperature - temperatureOffset;

    Values v;
    v.valid = true;
    v.temperature = adjustedTemperature;
    v.humidity = rawHumidity;
    v.absoluteHumidity = static_cast<float>(
        HumidityMath::calculateAbsoluteHumidity(adjustedTemperature, rawHumidity));
    v.dewPoint = static_cast<float>(
        HumidityMath::calculateDewPoint(adjustedTemperature, rawHumidity));
    return v;
}

}
