#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Cli.h"
#include "custom_assert.h"
#include "custom_types.h"
#include "unity.h"

typedef struct
{
    char* last_assert_file;
    u32 last_assert_line;
    char* last_assert_expr;
} assert_trigger_t;

assert_trigger_t last_assert_trigger;

// Mock put_char function for testing
static int mock_put_char(char c) {}

// Mock assert callback for testing
static void mock_assert_callback(const char* file, uint32_t line, const char* expr)
{
    last_assert_trigger.last_assert_file = file;
    last_assert_trigger.last_assert_line = line;
    last_assert_trigger.last_assert_expr = expr;
}

void setUp(void)
{
    // Register mock assert callback
    custom_assert_register(mock_assert_callback);

    // Reset the assert trigger
    last_assert_trigger.last_assert_file = NULL;
    last_assert_trigger.last_assert_line = 0;
    last_assert_trigger.last_assert_expr = NULL;
}

void tearDown(void)
{
    // Unregister assert callback
    custom_assert_unregister();
}

void test_dummy(void) {}
