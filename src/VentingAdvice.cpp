#include "VentingAdvice.h"
#include <math.h>

namespace VentingAdvice {

Result calculate(float indoorAbsHumidity, float outdoorAbsHumidity) {
    float diff = indoorAbsHumidity - outdoorAbsHumidity;
    if (fabsf(diff) < 0.05f) diff = 0.0f;
    Recommendation rec = diff >= VENTING_THRESHOLD  ? Recommendation::VENT
                       : diff >  0.0f              ? Recommendation::MARGINAL
                                                   : Recommendation::WAIT;
    return {diff, rec};
}

}
