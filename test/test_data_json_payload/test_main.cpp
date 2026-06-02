#include <unity.h>
#include <cstring>
#include "DataJsonPayload.h"

void setUp(void) {}
void tearDown(void) {}

// ---- helpers ----------------------------------------------------------------

static DataJsonPayload::IndoorInputs plausibleIndoor() {
    DataJsonPayload::IndoorInputs in{};
    in.sensorIsValid      = true;
    in.temperature        = 21.5f;
    in.humidity           = 55.0f;
    in.absoluteHumidity   = 10.8f;
    in.dewPoint           = 11.9f;
    return in;
}

static DataJsonPayload::OutdoorInputs plausibleOutdoor() {
    DataJsonPayload::OutdoorInputs out{};
    out.sensorIsValid              = true;
    out.temperature                = 5.0f;
    out.humidity                   = 80.0f;
    out.absoluteHumidity           = 5.5f;
    out.batteryOk                  = true;
    out.secondsSinceLastPacket     = 30;
    return out;
}

// ---- both valid path --------------------------------------------------------

void test_both_valid_sets_valid_flags(void) {
    DataJsonPayload::Payload p = DataJsonPayload::build(plausibleIndoor(), plausibleOutdoor(), 3.0f);
    TEST_ASSERT_TRUE(p.indoorValid);
    TEST_ASSERT_TRUE(p.outdoorValid);
}

void test_both_valid_populates_indoor_numeric_fields(void) {
    DataJsonPayload::Payload p = DataJsonPayload::build(plausibleIndoor(), plausibleOutdoor(), 3.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 21.5f, p.indoorTemperatureCelsius);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.8f, p.indoorAbsoluteHumidityGm3);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 11.9f, p.indoorDewPointCelsius);
}

void test_both_valid_has_advice(void) {
    DataJsonPayload::Payload p = DataJsonPayload::build(plausibleIndoor(), plausibleOutdoor(), 3.0f);
    TEST_ASSERT_TRUE(p.hasAdvice);
    TEST_ASSERT_NOT_NULL(p.ventingRecommendation);
}

void test_both_valid_recommendation_is_vent_when_diff_large(void) {
    // indoor abs 10.8, outdoor abs 5.5 → diff 5.3 >= 3.0 → VENT
    DataJsonPayload::Payload p = DataJsonPayload::build(plausibleIndoor(), plausibleOutdoor(), 3.0f);
    TEST_ASSERT_EQUAL_STRING("vent", p.ventingRecommendation);
}

// ---- humidity int truncation ------------------------------------------------

void test_humidity_int_cast_truncates(void) {
    DataJsonPayload::IndoorInputs in = plausibleIndoor();
    in.humidity = 55.7f;
    DataJsonPayload::Payload p = DataJsonPayload::build(in, plausibleOutdoor(), 3.0f);
    TEST_ASSERT_EQUAL_INT(55, p.indoorHumidityPercent);
}

// ---- indoor invalid (implausible temperature) --------------------------------

void test_indoor_implausible_temp_sets_indoor_invalid(void) {
    DataJsonPayload::IndoorInputs in = plausibleIndoor();
    in.temperature = 80.0f;   // above 60°C limit → implausible
    DataJsonPayload::Payload p = DataJsonPayload::build(in, plausibleOutdoor(), 3.0f);
    TEST_ASSERT_FALSE(p.indoorValid);
}

void test_indoor_invalid_sets_no_advice(void) {
    DataJsonPayload::IndoorInputs in = plausibleIndoor();
    in.temperature = 80.0f;
    DataJsonPayload::Payload p = DataJsonPayload::build(in, plausibleOutdoor(), 3.0f);
    TEST_ASSERT_FALSE(p.hasAdvice);
    TEST_ASSERT_NULL(p.ventingRecommendation);
}

// ---- outdoor stale gate -----------------------------------------------------

void test_outdoor_stale_121s_is_invalid(void) {
    DataJsonPayload::OutdoorInputs out = plausibleOutdoor();
    out.secondsSinceLastPacket = 121;
    DataJsonPayload::Payload p = DataJsonPayload::build(plausibleIndoor(), out, 3.0f);
    TEST_ASSERT_FALSE(p.outdoorValid);
}

void test_outdoor_boundary_120s_is_valid(void) {
    DataJsonPayload::OutdoorInputs out = plausibleOutdoor();
    out.secondsSinceLastPacket = 120;   // inclusive boundary
    DataJsonPayload::Payload p = DataJsonPayload::build(plausibleIndoor(), out, 3.0f);
    TEST_ASSERT_TRUE(p.outdoorValid);
}

// ---- both invalid -----------------------------------------------------------

void test_both_invalid_no_advice(void) {
    DataJsonPayload::IndoorInputs in = plausibleIndoor();
    in.temperature = 80.0f;   // implausible
    DataJsonPayload::OutdoorInputs out = plausibleOutdoor();
    out.secondsSinceLastPacket = 121;  // stale
    DataJsonPayload::Payload p = DataJsonPayload::build(in, out, 3.0f);
    TEST_ASSERT_FALSE(p.indoorValid);
    TEST_ASSERT_FALSE(p.outdoorValid);
    TEST_ASSERT_FALSE(p.hasAdvice);
    TEST_ASSERT_NULL(p.ventingRecommendation);
}

// ---- plausibility re-check (sensorIsValid=true but implausible value) -------

void test_plausibility_recheck_overrides_sensor_flag(void) {
    // sensorIsValid is true but temperature is out of plausible range
    DataJsonPayload::IndoorInputs in = plausibleIndoor();
    in.sensorIsValid  = true;
    in.temperature    = 80.0f;  // implausible even though flag says valid
    DataJsonPayload::Payload p = DataJsonPayload::build(in, plausibleOutdoor(), 3.0f);
    TEST_ASSERT_FALSE(p.indoorValid);
}

// ---- main -------------------------------------------------------------------

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_both_valid_sets_valid_flags);
    RUN_TEST(test_both_valid_populates_indoor_numeric_fields);
    RUN_TEST(test_both_valid_has_advice);
    RUN_TEST(test_both_valid_recommendation_is_vent_when_diff_large);
    RUN_TEST(test_humidity_int_cast_truncates);
    RUN_TEST(test_indoor_implausible_temp_sets_indoor_invalid);
    RUN_TEST(test_indoor_invalid_sets_no_advice);
    RUN_TEST(test_outdoor_stale_121s_is_invalid);
    RUN_TEST(test_outdoor_boundary_120s_is_valid);
    RUN_TEST(test_both_invalid_no_advice);
    RUN_TEST(test_plausibility_recheck_overrides_sensor_flag);
    return UNITY_END();
}
