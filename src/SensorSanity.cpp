#include "SensorSanity.h"

namespace SensorSanity {

bool isPlausibleTemperature(float temperature) {
    return temperature > -40.0f && temperature < 60.0f;
}

bool isPlausibleHumidity(float humidity) {
    return humidity > 0.0f && humidity < 100.0f;
}

}
