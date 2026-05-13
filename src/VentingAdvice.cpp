#include "VentingAdvice.h"
#include <math.h>

namespace VentingAdvice {

Result calculate(float indoorAbsHumidity, float outdoorAbsHumidity, float ventingThreshold) {
    float diff = indoorAbsHumidity - outdoorAbsHumidity;
    if (fabsf(diff) < 0.05f) diff = 0.0f;
    Recommendation rec = diff >= ventingThreshold  ? Recommendation::VENT
                       : diff >  0.0f              ? Recommendation::MARGINAL
                                                   : Recommendation::WAIT;
    return {diff, rec};
}

}
