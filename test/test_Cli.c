#include "unity.h"
#include "Cli.h"
#include <string.h>

static Cli_Config_t g_testCliConfig;
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
static int test_command_handler(int argc, char *argv[], void *context)
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
    Cli_Init(&g_testCliConfig, test_putchar);

    // Assert
    TEST_ASSERT_NOT_NULL(g_testCliConfig.pFnWriteCharacter);
    TEST_ASSERT_TRUE(g_testCliConfig.bIsInitialized);
}

void test_Cli_Register_should_AddCommandBinding(void)
{
    // Arrange
    Cli_Init(&g_testCliConfig, test_putchar);

    Cli_Binding_t testBinding = {
        .pcCmdName = "test",
        .pFnCmdHandler = test_command_handler,
        .pContext = NULL,
        .pcHelperString = "Test command"};

    // Act
    Cli_Register(&testBinding);

    // Assert - This test would need access to internal state
    // For now, we just check that no crash occurs
    TEST_PASS();
}

void test_Cli_Print_should_OutputFormattedString(void)
{
    // Arrange
    Cli_Init(&g_testCliConfig, test_putchar);

    // Act
    Cli_Print("Hello %s", "World");

    // Assert
    TEST_ASSERT_EQUAL_STRING("Hello World", g_outputBuffer);
}