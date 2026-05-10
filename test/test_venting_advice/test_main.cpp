#include <unity.h>
#include "VentingAdvice.h"

void setUp(void) {}
void tearDown(void) {}

// diff >= MARGINAL_THRESHOLD (3.0): clear benefit
void test_large_positive_difference_vents(void) {
    VentingAdvice::Result r = VentingAdvice::calculate(12.0f, 8.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 4.0f, r.difference);
    TEST_ASSERT_EQUAL(VentingAdvice::Recommendation::VENT, r.recommendation);
}

// 0 < diff < MARGINAL_THRESHOLD: not worth venting
void test_small_positive_difference_is_marginal(void) {
    VentingAdvice::Result r = VentingAdvice::calculate(9.5f, 8.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.5f, r.difference);
    TEST_ASSERT_EQUAL(VentingAdvice::Recommendation::MARGINAL, r.recommendation);
}

// diff < 0: outdoor more humid, don't vent
void test_negative_difference_waits(void) {
    VentingAdvice::Result r = VentingAdvice::calculate(8.0f, 10.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -2.0f, r.difference);
    TEST_ASSERT_EQUAL(VentingAdvice::Recommendation::WAIT, r.recommendation);
}

// |diff| < 0.05: rounds to zero → WAIT
void test_near_zero_positive_rounds_to_zero(void) {
    VentingAdvice::Result r = VentingAdvice::calculate(8.04f, 8.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, r.difference);
    TEST_ASSERT_EQUAL(VentingAdvice::Recommendation::WAIT, r.recommendation);
}

void test_near_zero_negative_rounds_to_zero(void) {
    VentingAdvice::Result r = VentingAdvice::calculate(8.0f, 8.04f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, r.difference);
    TEST_ASSERT_EQUAL(VentingAdvice::Recommendation::WAIT, r.recommendation);
}

// boundary: exactly at MARGINAL_THRESHOLD → VENT (>= is inclusive)
void test_difference_at_threshold_vents(void) {
    VentingAdvice::Result r = VentingAdvice::calculate(11.0f, 8.0f);  // diff = 3.0
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.0f, r.difference);
    TEST_ASSERT_EQUAL(VentingAdvice::Recommendation::VENT, r.recommendation);
}

// boundary: just below MARGINAL_THRESHOLD → MARGINAL
void test_difference_just_below_threshold_is_marginal(void) {
    VentingAdvice::Result r = VentingAdvice::calculate(10.99f, 8.0f);  // diff = 2.99
    TEST_ASSERT_EQUAL(VentingAdvice::Recommendation::MARGINAL, r.recommendation);
}

// boundary: just above rounding threshold — not rounded, still MARGINAL
void test_difference_just_above_rounding_threshold_not_rounded(void) {
    VentingAdvice::Result r = VentingAdvice::calculate(8.06f, 8.0f);  // diff = 0.06
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.06f, r.difference);
    TEST_ASSERT_EQUAL(VentingAdvice::Recommendation::MARGINAL, r.recommendation);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_large_positive_difference_vents);
    RUN_TEST(test_small_positive_difference_is_marginal);
    RUN_TEST(test_negative_difference_waits);
    RUN_TEST(test_near_zero_positive_rounds_to_zero);
    RUN_TEST(test_near_zero_negative_rounds_to_zero);
    RUN_TEST(test_difference_at_threshold_vents);
    RUN_TEST(test_difference_just_below_threshold_is_marginal);
    RUN_TEST(test_difference_just_above_rounding_threshold_not_rounded);
    return UNITY_END();
}
