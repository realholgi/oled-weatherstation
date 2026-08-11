#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "DataJsonPayload.h"

namespace MqttDiscovery {
constexpr const char *DISCOVERY_PREFIX = "homeassistant";
constexpr const char *BASE_TOPIC = "wetter";
constexpr const char *AVAILABILITY_TOPIC = "wetter/status";
constexpr const char *PAYLOAD_AVAILABLE = "online";
constexpr const char *PAYLOAD_NOT_AVAILABLE = "offline";
constexpr const char *DEVICE_IDENTIFIER = "wetter";
constexpr const char *DEVICE_NAME = "WetterStation";
constexpr size_t ENTITY_COUNT = 8;

struct Entity {
    const char *key;
    const char *name;
    const char *unit;
    const char *deviceClass;
    bool binary;
};

struct StateMessage {
    std::string topic;
    std::string payload;
};

const Entity &entity(size_t index);
std::string stateTopic(size_t index);
std::string discoveryTopic(size_t index);
std::string discoveryPayload(size_t index, const char *firmwareVersion);
std::vector<StateMessage> stateMessages(const DataJsonPayload::Payload &payload);
}
