#pragma once

#define WATER_VAPOR 17.62f
#define BAROMETRIC_PRESSURE 243.12f

namespace HumidityMath {

double calculateAbsoluteHumidity(double t, double RH);
double calculateDewPoint(double t, double RH);

}
