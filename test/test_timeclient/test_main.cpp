#include <unity.h>
#include "DSTEurope.h"


void setUp(void) {}
void tearDown(void) {}

void test_adjust_dst_before_summer_switch_hour(void) {
    TEST_ASSERT_EQUAL_INT(0, DSTEurope::adjust(2026, 3, 29, 0));
}

void test_adjust_dst_after_summer_switch_hour(void) {
    TEST_ASSERT_EQUAL_INT(1, DSTEurope::adjust(2026, 3, 29, 1));
}

void test_adjust_dst_before_winter_switch_hour(void) {
    TEST_ASSERT_EQUAL_INT(1, DSTEurope::adjust(2026, 10, 25, 0));
}

void test_adjust_dst_after_winter_switch_hour(void) {
    TEST_ASSERT_EQUAL_INT(0, DSTEurope::adjust(2026, 10, 25, 1));
}

void test_adjust_dst_mid_summer(void) {
    TEST_ASSERT_EQUAL_INT(1, DSTEurope::adjust(2026, 7, 15, 12));
}

void test_adjust_dst_mid_winter(void) {
    TEST_ASSERT_EQUAL_INT(0, DSTEurope::adjust(2026, 1, 15, 12));
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_adjust_dst_before_summer_switch_hour);
    RUN_TEST(test_adjust_dst_after_summer_switch_hour);
    RUN_TEST(test_adjust_dst_before_winter_switch_hour);
    RUN_TEST(test_adjust_dst_after_winter_switch_hour);
    RUN_TEST(test_adjust_dst_mid_summer);
    RUN_TEST(test_adjust_dst_mid_winter);
    return UNITY_END();
}
