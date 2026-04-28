#pragma once

#define WATER_VAPOR 17.62f
#define BAROMETRIC_PRESSURE 243.12f

namespace HumidityMath {

double berechneTT(double t, double RH);
double RHtoDP(double t, double RH);

}
