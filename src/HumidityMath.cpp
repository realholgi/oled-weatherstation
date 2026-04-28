#include "HumidityMath.h"
#include <cmath>

namespace HumidityMath {

// https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/2_Humidity_Sensors/Sensirion_Humidity_Sensors_at_a_Glance_V1.pdf
double berechneTT(double t, double RH) {
    return 216.7f * (RH / 100.0f * 6.112f * exp(WATER_VAPOR * t / (BAROMETRIC_PRESSURE + t)) / (273.15f + t));
}

double RHtoDP(double t, double RH) {
    double H = (log10(RH) - 2.0) / 0.4343 + (WATER_VAPOR * t) / (BAROMETRIC_PRESSURE + t);
    return BAROMETRIC_PRESSURE * H / (WATER_VAPOR - H);
}

}
