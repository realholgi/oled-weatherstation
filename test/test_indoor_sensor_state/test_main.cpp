#include <unity.h>
#include "IndoorSensorState.h"

void setUp(void) {}
void tearDown(void) {}

void test_invalid_returns_false_valid(void) {
    IndoorSensorState::Values v = IndoorSensorState::invalid();
    TEST_ASSERT_FALSE(v.valid);
}

void test_invalid_returns_sentinel_temperature(void) {
    IndoorSensorState::Values v = IndoorSensorState::invalid();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -273.0f, v.temperature);
}

void test_invalid_returns_sentinel_humidity(void) {
    IndoorSensorState::Values v = IndoorSensorState::invalid();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, v.humidity);
}

void test_invalid_returns_sentinel_absolute_humidity(void) {
    IndoorSensorState::Values v = IndoorSensorState::invalid();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.0f, v.absoluteHumidity);
}

void test_invalid_returns_sentinel_dew_point(void) {
    IndoorSensorState::Values v = IndoorSensorState::invalid();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -273.0f, v.dewPoint);
}

void test_plausible_reading_returns_valid_true(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 50.0f, 1.0f);
    TEST_ASSERT_TRUE(v.valid);
}

void test_plausible_reading_applies_temperature_offset(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 50.0f, 1.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 20.5f, v.temperature);
}

void test_plausible_reading_stores_humidity(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 50.0f, 1.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 50.0f, v.humidity);
}

void test_plausible_reading_has_positive_absolute_humidity(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 50.0f, 1.0f);
    TEST_ASSERT_GREATER_THAN(0.0f, v.absoluteHumidity);
}

void test_plausible_reading_has_plausible_dew_point(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 50.0f, 1.0f);
    TEST_ASSERT_GREATER_THAN(-273.0f, v.dewPoint);
}

void test_implausible_temperature_returns_valid_false(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(-40.0f, 50.0f, 0.0f);
    TEST_ASSERT_FALSE(v.valid);
}

void test_implausible_temperature_clears_temperature_to_sentinel(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(-40.0f, 50.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -273.0f, v.temperature);
}

void test_implausible_temperature_clears_humidity_to_sentinel(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(-40.0f, 50.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, v.humidity);
}

void test_implausible_temperature_clears_absolute_humidity_to_sentinel(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(-40.0f, 50.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.0f, v.absoluteHumidity);
}

void test_implausible_temperature_clears_dew_point_to_sentinel(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(-40.0f, 50.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -273.0f, v.dewPoint);
}

void test_implausible_humidity_returns_valid_false(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 100.0f, 0.0f);
    TEST_ASSERT_FALSE(v.valid);
}

void test_implausible_humidity_clears_temperature_to_sentinel(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 100.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -273.0f, v.temperature);
}

void test_implausible_humidity_clears_humidity_to_sentinel(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 100.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, v.humidity);
}

void test_implausible_humidity_clears_absolute_humidity_to_sentinel(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 100.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.0f, v.absoluteHumidity);
}

void test_implausible_humidity_clears_dew_point_to_sentinel(void) {
    IndoorSensorState::Values v = IndoorSensorState::fromReading(21.5f, 100.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -273.0f, v.dewPoint);
}

void test_offset_applied_before_derived_values(void) {
    IndoorSensorState::Values v_with_offset = IndoorSensorState::fromReading(21.5f, 50.0f, 2.0f);
    IndoorSensorState::Values v_no_offset = IndoorSensorState::fromReading(19.5f, 50.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, v_no_offset.absoluteHumidity, v_with_offset.absoluteHumidity);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, v_no_offset.dewPoint, v_with_offset.dewPoint);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_invalid_returns_false_valid);
    RUN_TEST(test_invalid_returns_sentinel_temperature);
    RUN_TEST(test_invalid_returns_sentinel_humidity);
    RUN_TEST(test_invalid_returns_sentinel_absolute_humidity);
    RUN_TEST(test_invalid_returns_sentinel_dew_point);
    RUN_TEST(test_plausible_reading_returns_valid_true);
    RUN_TEST(test_plausible_reading_applies_temperature_offset);
    RUN_TEST(test_plausible_reading_stores_humidity);
    RUN_TEST(test_plausible_reading_has_positive_absolute_humidity);
    RUN_TEST(test_plausible_reading_has_plausible_dew_point);
    RUN_TEST(test_implausible_temperature_returns_valid_false);
    RUN_TEST(test_implausible_temperature_clears_temperature_to_sentinel);
    RUN_TEST(test_implausible_temperature_clears_humidity_to_sentinel);
    RUN_TEST(test_implausible_temperature_clears_absolute_humidity_to_sentinel);
    RUN_TEST(test_implausible_temperature_clears_dew_point_to_sentinel);
    RUN_TEST(test_implausible_humidity_returns_valid_false);
    RUN_TEST(test_implausible_humidity_clears_temperature_to_sentinel);
    RUN_TEST(test_implausible_humidity_clears_humidity_to_sentinel);
    RUN_TEST(test_implausible_humidity_clears_absolute_humidity_to_sentinel);
    RUN_TEST(test_implausible_humidity_clears_dew_point_to_sentinel);
    RUN_TEST(test_offset_applied_before_derived_values);
    return UNITY_END();
}
