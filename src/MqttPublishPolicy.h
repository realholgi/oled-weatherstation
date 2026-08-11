#pragma once

#include <cstdint>

namespace MqttPublishPolicy {

constexpr uint32_t INTERVAL_MS = 30000;

bool shouldPublish(bool mqttConnected, bool mqttJustConnected, uint32_t nowMillis, uint32_t lastPublishedAtMillis);

}  // namespace MqttPublishPolicy
