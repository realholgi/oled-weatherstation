#include <unity.h>
#include "TimezoneSelectMarkup.h"
#include "Timezones.h"

void setUp(void) {}
void tearDown(void) {}

// capacity(TIMEZONES, TZ_COUNT, true) == 1905 (exact match case)
// Note: timezone names contain UTF-8 em-dashes (U+2013, 3 bytes each); strlen() counts bytes,
// so the formula uses byte lengths. RESEARCH.md stated 1847/1838 but those were computed
// with single-byte char widths — the actual byte counts are 1905/1896.
void test_capacity_with_match_equals_1905(void) {
    TEST_ASSERT_EQUAL(1905u, TimezoneSelectMarkup::capacity(TIMEZONES, TZ_COUNT, true));
}

// capacity(TIMEZONES, TZ_COUNT, false) == 1896 (no match case)
void test_capacity_without_match_equals_1896(void) {
    TEST_ASSERT_EQUAL(1896u, TimezoneSelectMarkup::capacity(TIMEZONES, TZ_COUNT, false));
}

// The difference between match and no-match is exactly 9 bytes (" selected")
void test_capacity_match_minus_nomatch_is_9(void) {
    size_t with_match    = TimezoneSelectMarkup::capacity(TIMEZONES, TZ_COUNT, true);
    size_t without_match = TimezoneSelectMarkup::capacity(TIMEZONES, TZ_COUNT, false);
    TEST_ASSERT_EQUAL(9u, with_match - without_match);
}

// Capacity strictly increases as entry count grows (1 entry < 2 entries)
void test_capacity_monotonic_in_count(void) {
    size_t cap_1 = TimezoneSelectMarkup::capacity(TIMEZONES, 1, false);
    size_t cap_2 = TimezoneSelectMarkup::capacity(TIMEZONES, 2, false);
    TEST_ASSERT_GREATER_THAN(cap_1, cap_2);
}

// Language select capacity is exactly 200 for both "de" and "en"
// Formula: 112 (open) + 70 (both options without selected) + 9 (close) + 9 (one selected) = 200
void test_language_select_capacity_is_200(void) {
    TEST_ASSERT_EQUAL(200u, TimezoneSelectMarkup::LANGUAGE_SELECT_CAPACITY);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_capacity_with_match_equals_1905);
    RUN_TEST(test_capacity_without_match_equals_1896);
    RUN_TEST(test_capacity_match_minus_nomatch_is_9);
    RUN_TEST(test_capacity_monotonic_in_count);
    RUN_TEST(test_language_select_capacity_is_200);
    return UNITY_END();
}
