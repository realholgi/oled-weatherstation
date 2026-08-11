#include <unity.h>

#include <string>
#include <vector>

#include "DataJsonPayload.h"
#include "MqttDiscovery.h"

void setUp(void) {}
void tearDown(void) {}

static void assertContains(const std::string &value, const char *expected) {
    TEST_ASSERT_NOT_EQUAL(std::string::npos, value.find(expected));
}

void test_discovery_uses_stable_topics_and_identifiers(void) {
    const char *keys[] = {"indoor_temperature", "indoor_humidity", "indoor_absolute_humidity", "indoor_dew_point",
                          "outdoor_temperature", "outdoor_humidity", "outdoor_absolute_humidity", "outdoor_battery"};

    TEST_ASSERT_EQUAL_STRING("wetter/status", MqttDiscovery::AVAILABILITY_TOPIC);
    TEST_ASSERT_EQUAL_UINT32(8, MqttDiscovery::ENTITY_COUNT);
    for (size_t index = 0; index < MqttDiscovery::ENTITY_COUNT; ++index) {
        TEST_ASSERT_EQUAL_STRING(keys[index], MqttDiscovery::entity(index).key);
        TEST_ASSERT_EQUAL_STRING((std::string("wetter/sensor/") + keys[index] + "/state").c_str(),
                                 MqttDiscovery::stateTopic(index).c_str());
        const std::string expectedTopic = index == 7
            ? "homeassistant/binary_sensor/wetter_outdoor_battery/config"
            : std::string("homeassistant/sensor/wetter_") + keys[index] + "/config";
        TEST_ASSERT_EQUAL_STRING(expectedTopic.c_str(), MqttDiscovery::discoveryTopic(index).c_str());
    }
}

void test_numeric_discovery_payloads_include_home_assistant_contract(void) {
    const std::string payload = MqttDiscovery::discoveryPayload(0, "1.0.2-dev");

    assertContains(payload, "\"name\":\"Indoor Temperature\"");
    assertContains(payload, "\"unique_id\":\"wetter_indoor_temperature\"");
    assertContains(payload, "\"state_topic\":\"wetter/sensor/indoor_temperature/state\"");
    assertContains(payload, "\"availability_topic\":\"wetter/status\"");
    assertContains(payload, "\"payload_available\":\"online\"");
    assertContains(payload, "\"payload_not_available\":\"offline\"");
    assertContains(payload, "\"unit_of_measurement\":\"°C\"");
    assertContains(payload, "\"device_class\":\"temperature\"");
    assertContains(payload, "\"state_class\":\"measurement\"");
    assertContains(payload, "\"identifiers\":[\"wetter\"]");
    assertContains(payload, "\"device\":{\"identifiers\":[\"wetter\"],\"name\":\"WetterStation\",\"manufacturer\":\"realholgi\",\"model\":\"WetterStation\",\"sw_version\":\"1.0.2-dev\"}");
}

void test_battery_discovery_payload_is_binary_sensor(void) {
    const std::string payload = MqttDiscovery::discoveryPayload(7, "1.0.2-dev");

    assertContains(payload, "\"unique_id\":\"wetter_outdoor_battery\"");
    assertContains(payload, "\"payload_on\":\"ON\"");
    assertContains(payload, "\"payload_off\":\"OFF\"");
    assertContains(payload, "\"device_class\":\"battery\"");
}

void test_all_discovery_documents_are_stable_for_reconnect_rediscovery(void) {
    for (size_t index = 0; index < MqttDiscovery::ENTITY_COUNT; ++index) {
        const std::string initialPayload = MqttDiscovery::discoveryPayload(index, "1.0.2-dev");
        const std::string rediscoveredPayload = MqttDiscovery::discoveryPayload(index, "1.0.2-dev");
        TEST_ASSERT_EQUAL_STRING(initialPayload.c_str(), rediscoveredPayload.c_str());
    }
}

void test_valid_payload_publishes_all_eight_retained_state_candidates(void) {
    DataJsonPayload::Payload payload{};
    payload.indoorValid = true;
    payload.outdoorValid = true;
    payload.indoorTemperatureCelsius = 21.25f;
    payload.indoorHumidityPercent = 42;
    payload.indoorAbsoluteHumidityGm3 = 7.11f;
    payload.indoorDewPointCelsius = 8.25f;
    payload.outdoorTemperatureCelsius = 4.5f;
    payload.outdoorHumidityPercent = 80;
    payload.outdoorAbsoluteHumidityGm3 = 5.2f;
    payload.outdoorBatteryOk = true;

    const std::vector<MqttDiscovery::StateMessage> messages = MqttDiscovery::stateMessages(payload);

    TEST_ASSERT_EQUAL_UINT32(8, messages.size());
    TEST_ASSERT_EQUAL_STRING("21.25", messages[0].payload.c_str());
    TEST_ASSERT_EQUAL_STRING("42.00", messages[1].payload.c_str());
    TEST_ASSERT_EQUAL_STRING("ON", messages[7].payload.c_str());
}

void test_invalid_or_non_finite_readings_do_not_replace_retained_state(void) {
    DataJsonPayload::Payload payload{};
    payload.indoorValid = false;
    payload.outdoorValid = false;

    TEST_ASSERT_EQUAL_UINT32(0, MqttDiscovery::stateMessages(payload).size());

    payload.indoorValid = true;
    payload.indoorTemperatureCelsius = __builtin_nanf("");
    payload.indoorHumidityPercent = 42;
    payload.indoorAbsoluteHumidityGm3 = 7.11f;
    payload.indoorDewPointCelsius = 8.25f;
    const std::vector<MqttDiscovery::StateMessage> messages = MqttDiscovery::stateMessages(payload);
    TEST_ASSERT_EQUAL_UINT32(3, messages.size());
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_discovery_uses_stable_topics_and_identifiers);
    RUN_TEST(test_numeric_discovery_payloads_include_home_assistant_contract);
    RUN_TEST(test_battery_discovery_payload_is_binary_sensor);
    RUN_TEST(test_valid_payload_publishes_all_eight_retained_state_candidates);
    RUN_TEST(test_all_discovery_documents_are_stable_for_reconnect_rediscovery);
    RUN_TEST(test_invalid_or_non_finite_readings_do_not_replace_retained_state);
    return UNITY_END();
}
