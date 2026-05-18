#pragma once

#include "config.h"

namespace VentingAdvice {

enum class Recommendation { VENT, MARGINAL, WAIT };

struct Result {
    float difference;              // indoor - outdoor abs humidity, rounded to 0 if |diff| < 0.05
    Recommendation recommendation;
};

Result calculate(float indoorAbsHumidity, float outdoorAbsHumidity, float ventingThreshold = DEFAULT_VENTING_THRESHOLD);

}
