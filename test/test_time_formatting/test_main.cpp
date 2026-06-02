#include <unity.h>
#include <ctime>
#include "TimeFormatting.h"

void setUp(void) {}
void tearDown(void) {}

void test_null_localtime_formats_placeholder(void) {
    char buffer[8] = {};
    TimeFormatting::formatClock(buffer, sizeof(buffer), nullptr);
    TEST_ASSERT_EQUAL_STRING("--:--", buffer);
}

void test_valid_localtime_formats_hour_and_minute(void) {
    char buffer[8] = {};
    std::tm localTime = {};
    localTime.tm_hour = 7;
    localTime.tm_min = 5;
    TimeFormatting::formatClock(buffer, sizeof(buffer), &localTime);
    TEST_ASSERT_EQUAL_STRING("07:05", buffer);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_null_localtime_formats_placeholder);
    RUN_TEST(test_valid_localtime_formats_hour_and_minute);
    return UNITY_END();
}
