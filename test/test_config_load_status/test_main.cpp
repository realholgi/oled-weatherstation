#include <unity.h>

#include "ConfigJsonParser.h"
#include "ConfigLoadStatus.h"

void setUp(void) {}
void tearDown(void) {}

static ConfigJsonParser::Defaults testDefaults() {
    return {"pool.ntp.org", "CET-1CEST,M3.5.0,M10.5.0/3", 0.5f, 2, "de", 2.5f};
}

void test_loaded_outcome_reports_loaded_without_defaults(void) {
    const ConfigLoadStatus::Outcome outcome = ConfigLoadStatus::loaded();

    TEST_ASSERT_EQUAL(ConfigLoadStatus::Status::Loaded, outcome.status);
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Reason::LoadedFromFile, outcome.reason);
    TEST_ASSERT_FALSE(outcome.usedDefaults);
    TEST_ASSERT_EQUAL_STRING("loaded", ConfigLoadStatus::statusLabel(outcome.status));
    TEST_ASSERT_EQUAL_STRING("loaded_from_file", ConfigLoadStatus::reasonLabel(outcome.reason));
}

void test_default_outcomes_report_used_defaults(void) {
    const ConfigLoadStatus::Outcome missing = ConfigLoadStatus::missingFile();
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Status::UsedDefaults, missing.status);
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Reason::MissingFile, missing.reason);
    TEST_ASSERT_TRUE(missing.usedDefaults);
    TEST_ASSERT_EQUAL_STRING("used_defaults", ConfigLoadStatus::statusLabel(missing.status));
    TEST_ASSERT_EQUAL_STRING("missing_file", ConfigLoadStatus::reasonLabel(missing.reason));

    const ConfigLoadStatus::Outcome mount = ConfigLoadStatus::mountFailed();
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Status::UsedDefaults, mount.status);
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Reason::MountFailed, mount.reason);
    TEST_ASSERT_TRUE(mount.usedDefaults);
    TEST_ASSERT_EQUAL_STRING("mount_failed", ConfigLoadStatus::reasonLabel(mount.reason));

    const ConfigLoadStatus::Outcome invalid = ConfigLoadStatus::invalidJson();
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Status::UsedDefaults, invalid.status);
    TEST_ASSERT_EQUAL(ConfigLoadStatus::Reason::InvalidJson, invalid.reason);
    TEST_ASSERT_TRUE(invalid.usedDefaults);
    TEST_ASSERT_EQUAL_STRING("invalid_json", ConfigLoadStatus::reasonLabel(invalid.reason));
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
    TEST_ASSERT_EQUAL_STRING(defaults.ntpServer, result.values.ntpServer.c_str());
    TEST_ASSERT_EQUAL_STRING(defaults.timezonePosix, result.values.timezonePosix.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.tempOffsetIndoor, result.values.tempOffsetIndoor);
    TEST_ASSERT_EQUAL_UINT8(defaults.outdoorSensorChannel, result.values.outdoorSensorChannel);
    TEST_ASSERT_EQUAL_STRING(defaults.webLanguage, result.values.webLanguage.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, defaults.ventingThreshold, result.values.ventingThreshold);
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
    RUN_TEST(test_loaded_outcome_reports_loaded_without_defaults);
    RUN_TEST(test_default_outcomes_report_used_defaults);
    RUN_TEST(test_parser_loads_valid_json_values);
    RUN_TEST(test_parser_invalid_json_uses_defaults);
    RUN_TEST(test_parser_valid_json_with_invalid_fields_still_reports_loaded);
    return UNITY_END();
}
