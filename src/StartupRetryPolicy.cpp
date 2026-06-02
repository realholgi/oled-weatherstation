#include "StartupRetryPolicy.h"

namespace {
static constexpr uint8_t INDOOR_SENSOR_MAX_ATTEMPTS = 3;
static constexpr uint16_t INDOOR_SENSOR_RETRY_DELAY_MS = 250;
}

namespace StartupRetryPolicy {
uint8_t maxAttempts() {
    return INDOOR_SENSOR_MAX_ATTEMPTS;
}

uint16_t retryDelayMs() {
    return INDOOR_SENSOR_RETRY_DELAY_MS;
}

bool shouldRetryAfterAttempt(uint8_t attempt) {
    return attempt > 0 && attempt < maxAttempts();
}
}
