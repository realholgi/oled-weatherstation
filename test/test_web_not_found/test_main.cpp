#include <unity.h>
#include <cstring>
#include "WebNotFound.h"

void setUp(void) {}
void tearDown(void) {}

void test_body_is_not_empty(void) {
    TEST_ASSERT_NOT_NULL(WebNotFound::BODY);
    TEST_ASSERT_GREATER_THAN(0u, strlen(WebNotFound::BODY));
}

void test_body_contains_no_uri_placeholder(void) {
    TEST_ASSERT_NULL(strstr(WebNotFound::BODY, "URI"));
}

void test_body_contains_no_method_placeholder(void) {
    TEST_ASSERT_NULL(strstr(WebNotFound::BODY, "Method"));
}

void test_content_type_is_text_plain(void) {
    TEST_ASSERT_EQUAL_STRING("text/plain", WebNotFound::CONTENT_TYPE);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    UNITY_BEGIN();
    RUN_TEST(test_body_is_not_empty);
    RUN_TEST(test_body_contains_no_uri_placeholder);
    RUN_TEST(test_body_contains_no_method_placeholder);
    RUN_TEST(test_content_type_is_text_plain);
    return UNITY_END();
}
