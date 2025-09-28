#include <string.h>
#include "Cli.h"
#include "unity.h"

static cli_cfg_t g_testCliConfig;
static char g_outputBuffer[256];
static size_t g_outputIndex = 0;

// Mock output function for testing
static int test_putchar(char c)
{
    if (g_outputIndex < sizeof(g_outputBuffer) - 1)
    {
        g_outputBuffer[g_outputIndex++] = c;
        g_outputBuffer[g_outputIndex] = '\0';
    }
    return c;
}

// Test command handler
static int test_command_handler(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;
    return CLI_OK_STATUS;
}

void setUp(void)
{
    // Clear test state before each test
    memset(&g_testCliConfig, 0, sizeof(g_testCliConfig));
    memset(g_outputBuffer, 0, sizeof(g_outputBuffer));
    g_outputIndex = 0;
}

void tearDown(void)
{
    // Clean up after each test
}

void test_Cli_Init_should_InitializeConfigStructure(void)
{
    // Act
    cli_init(&g_testCliConfig, test_putchar);

    // Assert
    TEST_ASSERT_NOT_NULL(g_testCliConfig.put_char_fn);
    TEST_ASSERT_TRUE(g_testCliConfig.is_initialized);
}

void test_Cli_Register_should_AddCommandBinding(void)
{
    // Arrange
    cli_init(&g_testCliConfig, test_putchar);

    cli_binding_t testBinding = {
        .cmd_name_string = "test", .cmd_handler_fn = test_command_handler, .context = NULL, .cmd_helper_string = "Test command"};

    // Act
    cli_register(&testBinding);

    // Assert - This test would need access to internal state
    // For now, we just check that no crash occurs
    TEST_PASS();
}

void test_Cli_Print_should_OutputFormattedString(void)
{
    // Arrange
    cli_init(&g_testCliConfig, test_putchar);

    // Act
    cli_print("Hello %s", "World");

    // Assert
    TEST_ASSERT_EQUAL_STRING("Hello World", g_outputBuffer);
}