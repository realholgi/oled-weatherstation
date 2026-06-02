#include <unity.h>

#include "StartupRetryPolicy.h"

void setUp(void) {}
void tearDown(void) {}

void test_max_attempts_is_small_fixed_count(void) {
    const uint8_t attempts = StartupRetryPolicy::maxAttempts();

    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(2, attempts);
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(5, attempts);
}

void test_retry_delay_is_short_and_bounded(void) {
    const uint16_t delayMs = StartupRetryPolicy::retryDelayMs();

    TEST_ASSERT_GREATER_THAN_UINT16(0, delayMs);
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(1000, delayMs);
}

void test_retry_policy_retries_after_first_failed_attempt(void) {
    TEST_ASSERT_TRUE(StartupRetryPolicy::shouldRetryAfterAttempt(1));
}

void test_retry_policy_stops_at_final_attempt(void) {
    const uint8_t finalAttempt = StartupRetryPolicy::maxAttempts();

    TEST_ASSERT_FALSE(StartupRetryPolicy::shouldRetryAfterAttempt(finalAttempt));
}

void test_retry_policy_stops_after_final_attempt(void) {
    const uint8_t afterFinalAttempt = StartupRetryPolicy::maxAttempts() + 1;

    TEST_ASSERT_FALSE(StartupRetryPolicy::shouldRetryAfterAttempt(afterFinalAttempt));
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_max_attempts_is_small_fixed_count);
    RUN_TEST(test_retry_delay_is_short_and_bounded);
    RUN_TEST(test_retry_policy_retries_after_first_failed_attempt);
    RUN_TEST(test_retry_policy_stops_at_final_attempt);
    RUN_TEST(test_retry_policy_stops_after_final_attempt);
    return UNITY_END();
}
