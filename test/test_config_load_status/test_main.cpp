#include <unity.h>

#include "ConfigLoadStatus.h"

void setUp(void) {}
void tearDown(void) {}

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

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_loaded_outcome_reports_loaded_without_defaults);
    RUN_TEST(test_default_outcomes_report_used_defaults);
    return UNITY_END();
}
