#include <unity.h>
#include "HumidityMath.h"

void setUp(void) {}
void tearDown(void) {}

void test_absolute_humidity_known_room_value(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 8.64f, HumidityMath::berechneTT(20.0, 50.0));
}

void test_dewpoint_known_room_value(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 9.26f, HumidityMath::RHtoDP(20.0, 50.0));
}

void test_absolute_humidity_increases_with_relative_humidity(void) {
    TEST_ASSERT_GREATER_THAN(HumidityMath::berechneTT(20.0, 40.0), HumidityMath::berechneTT(20.0, 60.0));
}

void test_dewpoint_increases_with_temperature_at_same_relative_humidity(void) {
    TEST_ASSERT_GREATER_THAN(HumidityMath::RHtoDP(10.0, 50.0), HumidityMath::RHtoDP(20.0, 50.0));
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_absolute_humidity_known_room_value);
    RUN_TEST(test_dewpoint_known_room_value);
    RUN_TEST(test_absolute_humidity_increases_with_relative_humidity);
    RUN_TEST(test_dewpoint_increases_with_temperature_at_same_relative_humidity);
    return UNITY_END();
}
