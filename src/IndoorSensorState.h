#pragma once

namespace IndoorSensorState {

struct Values {
    bool valid;
    float temperature;
    float humidity;
    float absoluteHumidity;
    float dewPoint;
};

Values invalid();
Values fromReading(float rawTemperature, float rawHumidity, float temperatureOffset);

}
