#include <unity.h>
#include "WifiConfigFields.h"

void setUp(void) {}
void tearDown(void) {}

void test_has_text_value_rejects_null_and_empty(void) {
    TEST_ASSERT_FALSE(WifiConfigFields::hasTextValue(nullptr));
    TEST_ASSERT_FALSE(WifiConfigFields::hasTextValue(""));
    TEST_ASSERT_TRUE(WifiConfigFields::hasTextValue("pool.ntp.org"));
}

void test_timezone_accepts_only_the_configured_allowlist(void) {
    TEST_ASSERT_TRUE(WifiConfigFields::isSupportedTimezone("UTC0"));
    TEST_ASSERT_TRUE(WifiConfigFields::isSupportedTimezone("CET-1CEST,M3.5.0,M10.5.0/3"));
    TEST_ASSERT_FALSE(WifiConfigFields::isSupportedTimezone(nullptr));
    TEST_ASSERT_FALSE(WifiConfigFields::isSupportedTimezone("UTC0\" onfocus=alert(1)"));
    TEST_ASSERT_FALSE(WifiConfigFields::isSupportedTimezone("<script>"));
}

void test_ntp_server_requires_a_bounded_hostname_or_ipv4_address(void) {
    TEST_ASSERT_TRUE(WifiConfigFields::isValidNtpServer("pool.ntp.org"));
    TEST_ASSERT_TRUE(WifiConfigFields::isValidNtpServer("ntp-1.example.org"));
    TEST_ASSERT_TRUE(WifiConfigFields::isValidNtpServer("192.0.2.1"));
    TEST_ASSERT_FALSE(WifiConfigFields::isValidNtpServer(nullptr));
    TEST_ASSERT_FALSE(WifiConfigFields::isValidNtpServer(""));
    TEST_ASSERT_FALSE(WifiConfigFields::isValidNtpServer("ntp..example.org"));
    TEST_ASSERT_FALSE(WifiConfigFields::isValidNtpServer("-ntp.example.org"));
    TEST_ASSERT_FALSE(WifiConfigFields::isValidNtpServer("ntp.example.org-"));
    TEST_ASSERT_FALSE(WifiConfigFields::isValidNtpServer("ntp.example.org\" onfocus=alert(1)"));
}

void test_supported_web_language_accepts_only_configured_values(void) {
    TEST_ASSERT_TRUE(WifiConfigFields::isSupportedWebLanguage("de"));
    TEST_ASSERT_TRUE(WifiConfigFields::isSupportedWebLanguage("en"));
    TEST_ASSERT_FALSE(WifiConfigFields::isSupportedWebLanguage(nullptr));
    TEST_ASSERT_FALSE(WifiConfigFields::isSupportedWebLanguage(""));
    TEST_ASSERT_FALSE(WifiConfigFields::isSupportedWebLanguage("fr"));
}

void test_indoor_temperature_offset_requires_complete_finite_float(void) {
    float value = 99.0f;
    TEST_ASSERT_TRUE(WifiConfigFields::parseIndoorTemperatureOffset("-1.25", value));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.25f, value);

    value = 99.0f;
    TEST_ASSERT_FALSE(WifiConfigFields::parseIndoorTemperatureOffset(nullptr, value));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 99.0f, value);
    TEST_ASSERT_FALSE(WifiConfigFields::parseIndoorTemperatureOffset("", value));
    TEST_ASSERT_FALSE(WifiConfigFields::parseIndoorTemperatureOffset("1.2x", value));
    TEST_ASSERT_FALSE(WifiConfigFields::parseIndoorTemperatureOffset("nan", value));
}

void test_outdoor_sensor_channel_requires_integer_in_supported_range(void) {
    uint8_t value = 0;
    TEST_ASSERT_TRUE(WifiConfigFields::parseOutdoorSensorChannel("1", value));
    TEST_ASSERT_EQUAL_UINT8(1, value);
    TEST_ASSERT_TRUE(WifiConfigFields::parseOutdoorSensorChannel("3", value));
    TEST_ASSERT_EQUAL_UINT8(3, value);

    value = 2;
    TEST_ASSERT_FALSE(WifiConfigFields::parseOutdoorSensorChannel(nullptr, value));
    TEST_ASSERT_EQUAL_UINT8(2, value);
    TEST_ASSERT_FALSE(WifiConfigFields::parseOutdoorSensorChannel("", value));
    TEST_ASSERT_FALSE(WifiConfigFields::parseOutdoorSensorChannel("0", value));
    TEST_ASSERT_FALSE(WifiConfigFields::parseOutdoorSensorChannel("4", value));
    TEST_ASSERT_FALSE(WifiConfigFields::parseOutdoorSensorChannel("2.5", value));
}

void test_venting_threshold_requires_complete_positive_finite_float(void) {
    float value = 0.0f;
    TEST_ASSERT_TRUE(WifiConfigFields::parseVentingThreshold("3.5", value));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.5f, value);

    value = 4.0f;
    TEST_ASSERT_FALSE(WifiConfigFields::parseVentingThreshold(nullptr, value));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 4.0f, value);
    TEST_ASSERT_FALSE(WifiConfigFields::parseVentingThreshold("", value));
    TEST_ASSERT_FALSE(WifiConfigFields::parseVentingThreshold("0", value));
    TEST_ASSERT_FALSE(WifiConfigFields::parseVentingThreshold("-1", value));
    TEST_ASSERT_FALSE(WifiConfigFields::parseVentingThreshold("3.5x", value));
    TEST_ASSERT_FALSE(WifiConfigFields::parseVentingThreshold("inf", value));
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_has_text_value_rejects_null_and_empty);
    RUN_TEST(test_timezone_accepts_only_the_configured_allowlist);
    RUN_TEST(test_ntp_server_requires_a_bounded_hostname_or_ipv4_address);
    RUN_TEST(test_supported_web_language_accepts_only_configured_values);
    RUN_TEST(test_indoor_temperature_offset_requires_complete_finite_float);
    RUN_TEST(test_outdoor_sensor_channel_requires_integer_in_supported_range);
    RUN_TEST(test_venting_threshold_requires_complete_positive_finite_float);
    return UNITY_END();
}
