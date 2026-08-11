#include "MqttDiscovery.h"

#include <cmath>
#include <cstdio>

namespace {
constexpr MqttDiscovery::Entity ENTITIES[] = {
    {"indoor_temperature", "Indoor Temperature", "°C", "temperature", false},
    {"indoor_humidity", "Indoor Humidity", "%", "humidity", false},
    {"indoor_absolute_humidity", "Indoor Absolute Humidity", "g/m³", "", false},
    {"indoor_dew_point", "Indoor Dew Point", "°C", "temperature", false},
    {"outdoor_temperature", "Outdoor Temperature", "°C", "temperature", false},
    {"outdoor_humidity", "Outdoor Humidity", "%", "humidity", false},
    {"outdoor_absolute_humidity", "Outdoor Absolute Humidity", "g/m³", "", false},
    {"outdoor_battery", "Outdoor Battery", "", "battery", true},
};

std::string formatNumber(float value) {
    char buffer[24];
    const int written = snprintf(buffer, sizeof(buffer), "%.2f", value);
    return written > 0 && static_cast<size_t>(written) < sizeof(buffer) ? buffer : std::string();
}

std::string devicePayload(const char *firmwareVersion) {
    return "\"device\":{\"identifiers\":[\"wetter\"],\"name\":\"WetterStation\",\"manufacturer\":\"realholgi\","
           "\"model\":\"WetterStation\",\"sw_version\":\"" + std::string(firmwareVersion) + "\"}";
}
}

namespace MqttDiscovery {
const Entity &entity(size_t index) {
    return ENTITIES[index];
}

std::string stateTopic(size_t index) {
    return std::string(BASE_TOPIC) + "/sensor/" + entity(index).key + "/state";
}

std::string discoveryTopic(size_t index) {
    const Entity &configuredEntity = entity(index);
    return std::string(DISCOVERY_PREFIX) + "/" + (configuredEntity.binary ? "binary_sensor/" : "sensor/") +
           DEVICE_IDENTIFIER + "_" + configuredEntity.key + "/config";
}

std::string discoveryPayload(size_t index, const char *firmwareVersion) {
    const Entity &configuredEntity = entity(index);
    std::string payload = "{\"name\":\"" + std::string(configuredEntity.name) +
                          "\",\"unique_id\":\"wetter_" + configuredEntity.key +
                          "\",\"state_topic\":\"" + stateTopic(index) +
                          "\",\"availability_topic\":\"" + AVAILABILITY_TOPIC +
                          "\",\"payload_available\":\"online\",\"payload_not_available\":\"offline\",";
    if (configuredEntity.binary) {
        payload += "\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"device_class\":\"battery\",";
    } else {
        payload += "\"unit_of_measurement\":\"" + std::string(configuredEntity.unit) + "\",";
        if (configuredEntity.deviceClass[0] != '\0') {
            payload += "\"device_class\":\"" + std::string(configuredEntity.deviceClass) + "\",";
        }
        payload += "\"state_class\":\"measurement\",";
    }
    return payload + devicePayload(firmwareVersion) + "}";
}

std::vector<StateMessage> stateMessages(const DataJsonPayload::Payload &payload) {
    std::vector<StateMessage> messages;
    if (payload.indoorValid) {
        const float values[] = {payload.indoorTemperatureCelsius, static_cast<float>(payload.indoorHumidityPercent),
                                payload.indoorAbsoluteHumidityGm3, payload.indoorDewPointCelsius};
        for (size_t index = 0; index < 4; ++index) {
            if (std::isfinite(values[index])) messages.push_back({stateTopic(index), formatNumber(values[index])});
        }
    }
    if (payload.outdoorValid) {
        const float values[] = {payload.outdoorTemperatureCelsius, static_cast<float>(payload.outdoorHumidityPercent),
                                payload.outdoorAbsoluteHumidityGm3};
        for (size_t index = 0; index < 3; ++index) {
            if (std::isfinite(values[index])) messages.push_back({stateTopic(index + 4), formatNumber(values[index])});
        }
        messages.push_back({stateTopic(7), payload.outdoorBatteryOk ? "ON" : "OFF"});
    }
    return messages;
}
}
