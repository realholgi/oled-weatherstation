#pragma once

namespace HumidityMath {

constexpr double WATER_VAPOR = 17.62;
constexpr double BAROMETRIC_PRESSURE = 243.12;

double calculateAbsoluteHumidity(double temperatureCelsius, double relativeHumidityPercent);
double calculateDewPoint(double temperatureCelsius, double relativeHumidityPercent);

}
