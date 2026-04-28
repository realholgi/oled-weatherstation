#include <unity.h>
#include "SensorSanity.h"

void setUp(void) {}
void tearDown(void) {}

void test_accepts_temperature_inside_limits(void) {
    TEST_ASSERT_TRUE(isPlausibleTemperature(-39.9f));
    TEST_ASSERT_TRUE(isPlausibleTemperature(59.9f));
}

void test_rejects_temperature_at_or_below_lower_limit(void) {
    TEST_ASSERT_FALSE(isPlausibleTemperature(-40.0f));
    TEST_ASSERT_FALSE(isPlausibleTemperature(-40.1f));
}

void test_rejects_temperature_at_or_above_upper_limit(void) {
    TEST_ASSERT_FALSE(isPlausibleTemperature(60.0f));
    TEST_ASSERT_FALSE(isPlausibleTemperature(60.1f));
}

void test_accepts_humidity_inside_limits(void) {
    TEST_ASSERT_TRUE(isPlausibleHumidity(0.1f));
    TEST_ASSERT_TRUE(isPlausibleHumidity(99.9f));
}

void test_rejects_humidity_at_or_below_lower_limit(void) {
    TEST_ASSERT_FALSE(isPlausibleHumidity(0.0f));
    TEST_ASSERT_FALSE(isPlausibleHumidity(-0.1f));
}

void test_rejects_humidity_at_or_above_upper_limit(void) {
    TEST_ASSERT_FALSE(isPlausibleHumidity(100.0f));
    TEST_ASSERT_FALSE(isPlausibleHumidity(100.1f));
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_accepts_temperature_inside_limits);
    RUN_TEST(test_rejects_temperature_at_or_below_lower_limit);
    RUN_TEST(test_rejects_temperature_at_or_above_upper_limit);
    RUN_TEST(test_accepts_humidity_inside_limits);
    RUN_TEST(test_rejects_humidity_at_or_below_lower_limit);
    RUN_TEST(test_rejects_humidity_at_or_above_upper_limit);
    return UNITY_END();
}
