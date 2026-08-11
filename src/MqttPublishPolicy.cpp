#include "MqttPublishPolicy.h"

namespace MqttPublishPolicy {

bool shouldPublish(bool mqttConnected, bool mqttJustConnected, uint32_t nowMillis, uint32_t lastPublishedAtMillis) {
    return mqttConnected && (mqttJustConnected || nowMillis - lastPublishedAtMillis >= INTERVAL_MS);
}

}  // namespace MqttPublishPolicy
