#include "HumidityMath.h"
#include <cmath>

namespace HumidityMath {

// https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/2_Humidity_Sensors/Sensirion_Humidity_Sensors_at_a_Glance_V1.pdf
double calculateAbsoluteHumidity(double temperatureCelsius, double relativeHumidityPercent) {
    return 216.7f * (relativeHumidityPercent / 100.0f * 6.112f *
                     exp(WATER_VAPOR * temperatureCelsius / (BAROMETRIC_PRESSURE + temperatureCelsius)) /
                     (273.15f + temperatureCelsius));
}

double calculateDewPoint(double temperatureCelsius, double relativeHumidityPercent) {
    double humidityTerm = (log10(relativeHumidityPercent) - 2.0) / 0.4343 +
                          (WATER_VAPOR * temperatureCelsius) / (BAROMETRIC_PRESSURE + temperatureCelsius);
    return BAROMETRIC_PRESSURE * humidityTerm / (WATER_VAPOR - humidityTerm);
}

}
