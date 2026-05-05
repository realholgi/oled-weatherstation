#pragma once

#define WATER_VAPOR 17.62f
#define BAROMETRIC_PRESSURE 243.12f

namespace HumidityMath {

double calculateAbsoluteHumidity(double temperatureCelsius, double relativeHumidityPercent);
double calculateDewPoint(double temperatureCelsius, double relativeHumidityPercent);

}
