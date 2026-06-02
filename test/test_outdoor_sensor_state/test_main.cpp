#include <unity.h>
#include "OutdoorSensorState.h"

void setUp(void) {}
void tearDown(void) {}

// --- initial() ---

void test_initial_is_invalid(void) {
    OutdoorSensorState::Values v = OutdoorSensorState::initial();
    TEST_ASSERT_FALSE(v.valid);
}

void test_initial_temperature_sentinel(void) {
    OutdoorSensorState::Values v = OutdoorSensorState::initial();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -273.0f, v.temperature);
}

void test_initial_humidity_sentinel(void) {
    OutdoorSensorState::Values v = OutdoorSensorState::initial();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, v.humidity);
}

void test_initial_absolute_humidity_sentinel(void) {
    OutdoorSensorState::Values v = OutdoorSensorState::initial();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.0f, v.absoluteHumidity);
}

void test_initial_battery_false(void) {
    OutdoorSensorState::Values v = OutdoorSensorState::initial();
    TEST_ASSERT_FALSE(v.batteryOk);
}

void test_initial_last_packet_millis_zero(void) {
    OutdoorSensorState::Values v = OutdoorSensorState::initial();
    TEST_ASSERT_EQUAL_UINT32(0, v.lastPacketAtMillis);
}

// --- processPacket: accepted ---

void test_accepted_packet_returns_accepted_decision(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_EQUAL_INT((int)OutdoorSensorState::Decision::Accepted, (int)r.decision);
}

void test_accepted_packet_sets_valid(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_TRUE(r.values.valid);
}

void test_accepted_packet_updates_temperature(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 21.5f, r.values.temperature);
}

void test_accepted_packet_updates_humidity(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 55.0f, r.values.humidity);
}

void test_accepted_packet_updates_battery(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_TRUE(r.values.batteryOk);
}

void test_accepted_packet_updates_absolute_humidity(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_GREATER_THAN(0.0f, r.values.absoluteHumidity);
}

void test_accepted_packet_updates_last_packet_millis(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_EQUAL_UINT32(1000, r.values.lastPacketAtMillis);
}

// --- processPacket: wrong channel ---

void test_wrong_channel_packet_returns_wrong_channel_decision(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 2, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_EQUAL_INT((int)OutdoorSensorState::Decision::WrongChannel, (int)r.decision);
}

void test_wrong_channel_packet_leaves_valid_false(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 2, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_FALSE(r.values.valid);
}

void test_wrong_channel_packet_does_not_change_last_packet_millis(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 2, 215, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_EQUAL_UINT32(0, r.values.lastPacketAtMillis);
}

void test_wrong_channel_does_not_change_accepted_state(void) {
    // Accept a packet first
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet good = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult accepted = OutdoorSensorState::processPacket(init, good, 3, 1000);

    // Then receive a wrong-channel packet
    OutdoorSensorState::Packet bad = { 2, 0, 99, false };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(accepted.values, bad, 3, 2000);

    TEST_ASSERT_TRUE(r.values.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 21.5f, r.values.temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 55.0f, r.values.humidity);
    TEST_ASSERT_TRUE(r.values.batteryOk);
    TEST_ASSERT_EQUAL_UINT32(1000, r.values.lastPacketAtMillis);
}

// --- processPacket: implausible same-channel ---

void test_implausible_temp_returns_implausible_decision(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, -400, 55, true }; // -40.0 deg — at boundary, implausible
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_EQUAL_INT((int)OutdoorSensorState::Decision::Implausible, (int)r.decision);
}

void test_implausible_humidity_returns_implausible_decision(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 100, true }; // 100% — at boundary, implausible
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_EQUAL_INT((int)OutdoorSensorState::Decision::Implausible, (int)r.decision);
}

void test_implausible_packet_does_not_change_last_packet_millis(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, -400, 55, true };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(init, pkt, 3, 1000);
    TEST_ASSERT_EQUAL_UINT32(0, r.values.lastPacketAtMillis);
}

void test_implausible_packet_does_not_change_accepted_state(void) {
    // Accept a packet first
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet good = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult accepted = OutdoorSensorState::processPacket(init, good, 3, 1000);

    // Then receive an implausible same-channel packet
    OutdoorSensorState::Packet bad = { 3, -400, 0, false };
    OutdoorSensorState::ProcessResult r = OutdoorSensorState::processPacket(accepted.values, bad, 3, 2000);

    TEST_ASSERT_TRUE(r.values.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 21.5f, r.values.temperature);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 55.0f, r.values.humidity);
    TEST_ASSERT_TRUE(r.values.batteryOk);
    TEST_ASSERT_EQUAL_UINT32(1000, r.values.lastPacketAtMillis);
}

// --- applyStale ---

void test_apply_stale_before_timeout_leaves_valid(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult accepted = OutdoorSensorState::processPacket(init, pkt, 3, 1000);

    // nowMillis = 5000, lastPacketAtMillis = 1000, maxWait = 120000
    OutdoorSensorState::Values after = OutdoorSensorState::applyStale(accepted.values, 5000, 120000);
    TEST_ASSERT_TRUE(after.valid);
}

void test_apply_stale_after_timeout_clears_valid(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult accepted = OutdoorSensorState::processPacket(init, pkt, 3, 1000);

    // nowMillis = 122000, maxWait = 120000 -> elapsed = 121000 >= 120000 -> stale
    OutdoorSensorState::Values after = OutdoorSensorState::applyStale(accepted.values, 122000, 120000);
    TEST_ASSERT_FALSE(after.valid);
}

void test_apply_stale_clears_temperature_to_sentinel(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult accepted = OutdoorSensorState::processPacket(init, pkt, 3, 1000);

    OutdoorSensorState::Values after = OutdoorSensorState::applyStale(accepted.values, 122000, 120000);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -273.0f, after.temperature);
}

void test_apply_stale_clears_humidity_to_sentinel(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult accepted = OutdoorSensorState::processPacket(init, pkt, 3, 1000);

    OutdoorSensorState::Values after = OutdoorSensorState::applyStale(accepted.values, 122000, 120000);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, after.humidity);
}

void test_apply_stale_clears_absolute_humidity_to_sentinel(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult accepted = OutdoorSensorState::processPacket(init, pkt, 3, 1000);

    OutdoorSensorState::Values after = OutdoorSensorState::applyStale(accepted.values, 122000, 120000);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.0f, after.absoluteHumidity);
}

void test_apply_stale_preserves_battery_ok(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult accepted = OutdoorSensorState::processPacket(init, pkt, 3, 1000);

    OutdoorSensorState::Values after = OutdoorSensorState::applyStale(accepted.values, 122000, 120000);
    TEST_ASSERT_TRUE(after.batteryOk);
}

void test_apply_stale_preserves_last_packet_millis(void) {
    OutdoorSensorState::Values init = OutdoorSensorState::initial();
    OutdoorSensorState::Packet pkt = { 3, 215, 55, true };
    OutdoorSensorState::ProcessResult accepted = OutdoorSensorState::processPacket(init, pkt, 3, 1000);

    OutdoorSensorState::Values after = OutdoorSensorState::applyStale(accepted.values, 122000, 120000);
    TEST_ASSERT_EQUAL_UINT32(1000, after.lastPacketAtMillis);
}

// --- secondsSince ---

void test_seconds_since_reports_age(void) {
    uint32_t age = OutdoorSensorState::secondsSince(5000, 2000);
    TEST_ASSERT_EQUAL_UINT32(3, age);
}

void test_seconds_since_handles_unsigned_wrap(void) {
    // Simulates millis() overflow: nowMillis wraps below lastPacketAtMillis
    // Using unsigned subtraction: (uint32_t)(100 - 4294967196) = 200
    uint32_t last = 4294967196u;  // near max uint32
    uint32_t now = 100u;          // wrapped around
    uint32_t age = OutdoorSensorState::secondsSince(now, last);
    // (100 - 4294967196) in unsigned 32-bit = 200 ms = 0 seconds
    TEST_ASSERT_EQUAL_UINT32(0, age);
}

void test_seconds_since_zero_elapsed(void) {
    uint32_t age = OutdoorSensorState::secondsSince(1000, 1000);
    TEST_ASSERT_EQUAL_UINT32(0, age);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();

    // initial()
    RUN_TEST(test_initial_is_invalid);
    RUN_TEST(test_initial_temperature_sentinel);
    RUN_TEST(test_initial_humidity_sentinel);
    RUN_TEST(test_initial_absolute_humidity_sentinel);
    RUN_TEST(test_initial_battery_false);
    RUN_TEST(test_initial_last_packet_millis_zero);

    // processPacket: accepted
    RUN_TEST(test_accepted_packet_returns_accepted_decision);
    RUN_TEST(test_accepted_packet_sets_valid);
    RUN_TEST(test_accepted_packet_updates_temperature);
    RUN_TEST(test_accepted_packet_updates_humidity);
    RUN_TEST(test_accepted_packet_updates_battery);
    RUN_TEST(test_accepted_packet_updates_absolute_humidity);
    RUN_TEST(test_accepted_packet_updates_last_packet_millis);

    // processPacket: wrong channel
    RUN_TEST(test_wrong_channel_packet_returns_wrong_channel_decision);
    RUN_TEST(test_wrong_channel_packet_leaves_valid_false);
    RUN_TEST(test_wrong_channel_packet_does_not_change_last_packet_millis);
    RUN_TEST(test_wrong_channel_does_not_change_accepted_state);

    // processPacket: implausible same-channel
    RUN_TEST(test_implausible_temp_returns_implausible_decision);
    RUN_TEST(test_implausible_humidity_returns_implausible_decision);
    RUN_TEST(test_implausible_packet_does_not_change_last_packet_millis);
    RUN_TEST(test_implausible_packet_does_not_change_accepted_state);

    // applyStale
    RUN_TEST(test_apply_stale_before_timeout_leaves_valid);
    RUN_TEST(test_apply_stale_after_timeout_clears_valid);
    RUN_TEST(test_apply_stale_clears_temperature_to_sentinel);
    RUN_TEST(test_apply_stale_clears_humidity_to_sentinel);
    RUN_TEST(test_apply_stale_clears_absolute_humidity_to_sentinel);
    RUN_TEST(test_apply_stale_preserves_battery_ok);
    RUN_TEST(test_apply_stale_preserves_last_packet_millis);

    // secondsSince
    RUN_TEST(test_seconds_since_reports_age);
    RUN_TEST(test_seconds_since_handles_unsigned_wrap);
    RUN_TEST(test_seconds_since_zero_elapsed);

    return UNITY_END();
}
