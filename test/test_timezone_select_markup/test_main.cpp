#include <unity.h>
#include "TimezoneSelectMarkup.h"
#include "Timezones.h"

void setUp(void) {}
void tearDown(void) {}

void test_stub_compiles(void) {
    // Stub: full assertions land in plan 04-02
    TEST_ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_stub_compiles);
    return UNITY_END();
}
