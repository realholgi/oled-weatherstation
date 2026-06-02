#pragma once

#include <cstdint>

namespace StartupRetryPolicy {
uint8_t maxAttempts();
uint16_t retryDelayMs();
bool shouldRetryAfterAttempt(uint8_t attempt);
}
