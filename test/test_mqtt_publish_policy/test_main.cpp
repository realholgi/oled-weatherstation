#include <unity.h>

#include <cstdint>

#include "MqttPublishPolicy.h"

void setUp(void) {}
void tearDown(void) {}

void test_disconnected_broker_suppresses_due_and_reconnect_publication(void) {
    TEST_ASSERT_FALSE(MqttPublishPolicy::shouldPublish(false, false, 30000, 0));
    TEST_ASSERT_FALSE(MqttPublishPolicy::shouldPublish(false, true, 0, 0));
}

void test_new_connection_publishes_immediately(void) {
    TEST_ASSERT_TRUE(MqttPublishPolicy::shouldPublish(true, true, 1, 0));
}

void test_established_connection_waits_until_interval(void) {
    TEST_ASSERT_FALSE(MqttPublishPolicy::shouldPublish(true, false, 29999, 0));
    TEST_ASSERT_TRUE(MqttPublishPolicy::shouldPublish(true, false, 30000, 0));
}

void test_elapsed_time_handles_millis_wraparound(void) {
    const uint32_t lastPublishedAtMillis = UINT32_MAX - 10000;
    const uint32_t nowMillis = 19998;

    TEST_ASSERT_FALSE(MqttPublishPolicy::shouldPublish(true, false, nowMillis, lastPublishedAtMillis));
    TEST_ASSERT_TRUE(MqttPublishPolicy::shouldPublish(true, false, nowMillis + 1, lastPublishedAtMillis));
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_disconnected_broker_suppresses_due_and_reconnect_publication);
    RUN_TEST(test_new_connection_publishes_immediately);
    RUN_TEST(test_established_connection_waits_until_interval);
    RUN_TEST(test_elapsed_time_handles_millis_wraparound);
    return UNITY_END();
}
