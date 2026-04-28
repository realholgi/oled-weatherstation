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

void test_adjust_dst_day_before_summer_switch(void) {
    TEST_ASSERT_EQUAL_INT(0, DSTEurope::adjust(2026, 3, 28, 23));
}

void test_adjust_dst_day_after_summer_switch(void) {
    TEST_ASSERT_EQUAL_INT(1, DSTEurope::adjust(2026, 3, 30, 0));
}

void test_adjust_dst_day_before_winter_switch(void) {
    TEST_ASSERT_EQUAL_INT(1, DSTEurope::adjust(2026, 10, 24, 23));
}

void test_adjust_dst_day_after_winter_switch(void) {
    TEST_ASSERT_EQUAL_INT(0, DSTEurope::adjust(2026, 10, 26, 0));
}

void test_adjust_dst_2024_summer_switch_last_sunday(void) {
    TEST_ASSERT_EQUAL_INT(0, DSTEurope::adjust(2024, 3, 31, 0));
    TEST_ASSERT_EQUAL_INT(1, DSTEurope::adjust(2024, 3, 31, 1));
}

void test_adjust_dst_2025_winter_switch_last_sunday(void) {
    TEST_ASSERT_EQUAL_INT(1, DSTEurope::adjust(2025, 10, 26, 0));
    TEST_ASSERT_EQUAL_INT(0, DSTEurope::adjust(2025, 10, 26, 1));
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
    RUN_TEST(test_adjust_dst_day_before_summer_switch);
    RUN_TEST(test_adjust_dst_day_after_summer_switch);
    RUN_TEST(test_adjust_dst_day_before_winter_switch);
    RUN_TEST(test_adjust_dst_day_after_winter_switch);
    RUN_TEST(test_adjust_dst_2024_summer_switch_last_sunday);
    RUN_TEST(test_adjust_dst_2025_winter_switch_last_sunday);
    return UNITY_END();
}
