#include <unity.h>

#include <string>

#include "ConfigJsonParser.h"
#include "ConfigLoadStatus.h"

void setUp(void) {}
void tearDown(void) {}

static ConfigJsonParser::Defaults testDefaults() {
    return {"pool.ntp.org", "CET-1CEST,M3.5.0,M10.5.0/3", 0.5f, 2, "de", 2.5f};
}

static void assertAllDefaults(const ConfigJsonParser::Result &result, const ConfigJsonParser::Defaults &defaults) {
    TEST_ASSERT_EQUAL_STRING(defaults.ntpServer, result.values.ntpServer.c_str());
    TEST_ASSERT_EQUAL_STRING(defaults.timezonePosix, result.values.timezonePosix.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.tempOffsetIndoor, result.values.tempOffsetIndoor);
    TEST_ASSERT_EQUAL_UINT8(defaults.outdoorSensorChannel, result.values.outdoorSensorChannel);
    TEST_ASSERT_EQUAL_STRING(defaults.webLanguage, result.values.webLanguage.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.ventingThreshold, result.values.ventingThreshold);
}

void test_parser_loads_valid_json_values(void) {
    const char *json =
        "{\"ntp_server\":\"time.example.org\","
        "\"timezone_posix\":\"UTC0\","
        "\"temp_offset_indoor\":-1.25,"
        "\"outdoor_sensor_channel\":3,"
        "\"web_language\":\"en\","
        "\"venting_threshold\":4.75}";

    const ConfigJsonParser::Result result = ConfigJsonParser::parse(json, testDefaults());

    TEST_ASSERT_EQUAL(ConfigLoadStatus::Status::Loaded, result.outcome.status);
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Reason::LoadedFromFile, result.outcome.reason);
    TEST_ASSERT_FALSE(result.outcome.usedDefaults);
    TEST_ASSERT_EQUAL_STRING("time.example.org", result.values.ntpServer.c_str());
    TEST_ASSERT_EQUAL_STRING("UTC0", result.values.timezonePosix.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.25f, result.values.tempOffsetIndoor);
    TEST_ASSERT_EQUAL_UINT8(3, result.values.outdoorSensorChannel);
    TEST_ASSERT_EQUAL_STRING("en", result.values.webLanguage.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 4.75f, result.values.ventingThreshold);
}

void test_parser_invalid_json_uses_defaults(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();

    const ConfigJsonParser::Result result = ConfigJsonParser::parse("{not valid json", defaults);

    TEST_ASSERT_EQUAL(ConfigLoadStatus::Status::UsedDefaults, result.outcome.status);
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Reason::InvalidJson, result.outcome.reason);
    TEST_ASSERT_TRUE(result.outcome.usedDefaults);
    assertAllDefaults(result, defaults);
}

void test_parser_nullptr_json_uses_defaults(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();

    const ConfigJsonParser::Result result = ConfigJsonParser::parse(nullptr, defaults);

    TEST_ASSERT_EQUAL(ConfigLoadStatus::Reason::InvalidJson, result.outcome.reason);
    TEST_ASSERT_TRUE(result.outcome.usedDefaults);
    assertAllDefaults(result, defaults);
}

void test_parser_empty_json_uses_defaults(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();

    const ConfigJsonParser::Result result = ConfigJsonParser::parse("", defaults);

    TEST_ASSERT_EQUAL(ConfigLoadStatus::Reason::InvalidJson, result.outcome.reason);
    TEST_ASSERT_TRUE(result.outcome.usedDefaults);
    assertAllDefaults(result, defaults);
}

void test_parser_empty_object_keeps_defaults_but_reports_loaded(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();

    const ConfigJsonParser::Result result = ConfigJsonParser::parse("{}", defaults);

    TEST_ASSERT_EQUAL(ConfigLoadStatus::Status::Loaded, result.outcome.status);
    TEST_ASSERT_FALSE(result.outcome.usedDefaults);
    assertAllDefaults(result, defaults);
}

void test_parser_missing_fields_fall_back_individually(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();
    const char *json = "{\"ntp_server\":\"only.example.org\"}";

    const ConfigJsonParser::Result result = ConfigJsonParser::parse(json, defaults);

    TEST_ASSERT_EQUAL(ConfigLoadStatus::Status::Loaded, result.outcome.status);
    TEST_ASSERT_EQUAL_STRING("only.example.org", result.values.ntpServer.c_str());
    TEST_ASSERT_EQUAL_STRING(defaults.timezonePosix, result.values.timezonePosix.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.tempOffsetIndoor, result.values.tempOffsetIndoor);
    TEST_ASSERT_EQUAL_UINT8(defaults.outdoorSensorChannel, result.values.outdoorSensorChannel);
    TEST_ASSERT_EQUAL_STRING(defaults.webLanguage, result.values.webLanguage.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.ventingThreshold, result.values.ventingThreshold);
}

void test_parser_rejects_out_of_range_channels(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();
    const char *channels[] = {"0", "4", "-1", "\"x\"", "1.5"};

    for (const char *channel : channels) {
        std::string json = std::string("{\"outdoor_sensor_channel\":") + channel + "}";
        const ConfigJsonParser::Result result = ConfigJsonParser::parse(json.c_str(), defaults);
        TEST_ASSERT_EQUAL_UINT8(defaults.outdoorSensorChannel, result.values.outdoorSensorChannel);
    }
}

void test_parser_accepts_channel_bounds(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();

    const ConfigJsonParser::Result low = ConfigJsonParser::parse("{\"outdoor_sensor_channel\":1}", defaults);
    TEST_ASSERT_EQUAL_UINT8(1, low.values.outdoorSensorChannel);

    const ConfigJsonParser::Result high = ConfigJsonParser::parse("{\"outdoor_sensor_channel\":3}", defaults);
    TEST_ASSERT_EQUAL_UINT8(3, high.values.outdoorSensorChannel);
}

void test_parser_rejects_non_finite_floats(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();
    const char *json =
        "{\"temp_offset_indoor\":1e39,"
        "\"venting_threshold\":-1e39}";

    const ConfigJsonParser::Result result = ConfigJsonParser::parse(json, defaults);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.tempOffsetIndoor, result.values.tempOffsetIndoor);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.ventingThreshold, result.values.ventingThreshold);
}

void test_parser_rejects_invalid_languages(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();
    const char *languages[] = {"\"fr\"", "\"\"", "\"DE\"", "1"};

    for (const char *language : languages) {
        std::string json = std::string("{\"web_language\":") + language + "}";
        const ConfigJsonParser::Result result = ConfigJsonParser::parse(json.c_str(), defaults);
        TEST_ASSERT_EQUAL_STRING(defaults.webLanguage, result.values.webLanguage.c_str());
    }
}

void test_parser_rejects_non_positive_venting_threshold(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();
    const char *thresholds[] = {"0", "-0.5", "-3"};

    for (const char *threshold : thresholds) {
        std::string json = std::string("{\"venting_threshold\":") + threshold + "}";
        const ConfigJsonParser::Result result = ConfigJsonParser::parse(json.c_str(), defaults);
        TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.ventingThreshold, result.values.ventingThreshold);
    }
}

void test_parser_schema_version_defaults_to_current(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();

    const ConfigJsonParser::Result result = ConfigJsonParser::parse("{}", defaults);

    TEST_ASSERT_EQUAL_INT(ConfigJsonParser::CURRENT_SCHEMA_VERSION, result.values.schemaVersion);
}

void test_parser_reads_valid_schema_version(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();

    const ConfigJsonParser::Result current = ConfigJsonParser::parse("{\"schema_version\":1}", defaults);
    TEST_ASSERT_EQUAL_INT(1, current.values.schemaVersion);

    const ConfigJsonParser::Result newer =
        ConfigJsonParser::parse("{\"schema_version\":2,\"ntp_server\":\"future.example.org\"}", defaults);
    TEST_ASSERT_EQUAL_INT(2, newer.values.schemaVersion);
    TEST_ASSERT_EQUAL_STRING("future.example.org", newer.values.ntpServer.c_str());
}

void test_parser_rejects_invalid_schema_versions(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();
    const char *versions[] = {"\"x\"", "0", "-1", "1.5"};

    for (const char *version : versions) {
        std::string json = std::string("{\"schema_version\":") + version + "}";
        const ConfigJsonParser::Result result = ConfigJsonParser::parse(json.c_str(), defaults);
        TEST_ASSERT_EQUAL_INT(ConfigJsonParser::CURRENT_SCHEMA_VERSION, result.values.schemaVersion);
    }
}

void test_parser_valid_json_with_invalid_fields_still_reports_loaded(void) {
    const ConfigJsonParser::Defaults defaults = testDefaults();
    const char *json =
        "{\"ntp_server\":\"saved.example.org\","
        "\"timezone_posix\":\"\","
        "\"temp_offset_indoor\":\"nan\","
        "\"outdoor_sensor_channel\":9,"
        "\"web_language\":\"fr\","
        "\"venting_threshold\":0}";

    const ConfigJsonParser::Result result = ConfigJsonParser::parse(json, defaults);

    TEST_ASSERT_EQUAL(ConfigLoadStatus::Status::Loaded, result.outcome.status);
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Reason::LoadedFromFile, result.outcome.reason);
    TEST_ASSERT_FALSE(result.outcome.usedDefaults);
    TEST_ASSERT_EQUAL_STRING("saved.example.org", result.values.ntpServer.c_str());
    TEST_ASSERT_EQUAL_STRING(defaults.timezonePosix, result.values.timezonePosix.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.tempOffsetIndoor, result.values.tempOffsetIndoor);
    TEST_ASSERT_EQUAL_UINT8(defaults.outdoorSensorChannel, result.values.outdoorSensorChannel);
    TEST_ASSERT_EQUAL_STRING(defaults.webLanguage, result.values.webLanguage.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.ventingThreshold, result.values.ventingThreshold);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_parser_loads_valid_json_values);
    RUN_TEST(test_parser_invalid_json_uses_defaults);
    RUN_TEST(test_parser_nullptr_json_uses_defaults);
    RUN_TEST(test_parser_empty_json_uses_defaults);
    RUN_TEST(test_parser_empty_object_keeps_defaults_but_reports_loaded);
    RUN_TEST(test_parser_missing_fields_fall_back_individually);
    RUN_TEST(test_parser_rejects_out_of_range_channels);
    RUN_TEST(test_parser_accepts_channel_bounds);
    RUN_TEST(test_parser_rejects_non_finite_floats);
    RUN_TEST(test_parser_rejects_invalid_languages);
    RUN_TEST(test_parser_rejects_non_positive_venting_threshold);
    RUN_TEST(test_parser_schema_version_defaults_to_current);
    RUN_TEST(test_parser_reads_valid_schema_version);
    RUN_TEST(test_parser_rejects_invalid_schema_versions);
    RUN_TEST(test_parser_valid_json_with_invalid_fields_still_reports_loaded);
    return UNITY_END();
}
