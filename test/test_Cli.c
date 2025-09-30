#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Cli.h"
#include "custom_assert.h"
#include "custom_types.h"
#include "unity.h"

// #############################################################################
// # Command Implementations - just for demonstration
// ###########################################################################

/** Demo command handlers used in the example program. */
int cmd_hello_world(int argc, char* argv[], void* context);
int cmd_echo_string(int argc, char* argv[], void* context);
int cmd_display_args(int argc, char* argv[], void* context);

int cmd_hello_world(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;
    cli_print("%sHello World!\n", CLI_OK_PROMPT);
    return CLI_OK_STATUS;
}

int cmd_echo_string(int argc, char* argv[], void* context)
{
    if (argc != 2)
    {
        cli_print("%sGive one argument\n", CLI_FAIL_PROMPT);
        return CLI_FAIL_STATUS;
    }
    (void)argv;
    (void)context;
    cli_print("%s\"%s\"\n", CLI_OK_PROMPT, argv[1]);
    return CLI_OK_STATUS;
}

int cmd_display_args(int argc, char* argv[], void* context)
{
    int i;
    cli_print("%s\n", CLI_OK_PROMPT);
    for (i = 0; i < argc; i++)
    {
        cli_print("argv[%d] --> \"%s\" \n", i, argv[i]);
    }

    (void)context;
    return CLI_OK_STATUS;
}

int cmd_dummy(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;
    cli_print("%s\n", CLI_OK_PROMPT);
    return CLI_OK_STATUS;
}

static cli_binding_t cli_bindings[] = {
    {"hello", cmd_hello_world, NULL, "Say hello"},
    {"args", cmd_display_args, NULL, "Displays the given cli arguments"},
    {"echo", cmd_echo_string, NULL, "Echoes the given string"},
    {"dummy", cmd_dummy, NULL, "dummy stuffens"},
};

// #############################################################################
// # Assert Mocks
// ###########################################################################

typedef struct
{
    const char* last_assert_file;
    u32 last_assert_line;
    const char* last_assert_expr;
} assert_trigger_t;

assert_trigger_t last_assert_trigger;

// Mock assert callback for testing
static void mock_assert_callback(const char* file, uint32_t line, const char* expr)
{
    last_assert_trigger.last_assert_file = file;
    last_assert_trigger.last_assert_line = line;
    last_assert_trigger.last_assert_expr = expr;
}

// #############################################################################
// # putchar mock
// ###########################################################################

// Mock put_char function for testing
#define MOCK_BUFFER_SIZE 1024
static char mock_print_buffer[MOCK_BUFFER_SIZE];
static size_t mock_print_index = 0;

static int mock_put_char(char c)
{
    if (mock_print_index < MOCK_BUFFER_SIZE - 1)
    {
        mock_print_buffer[mock_print_index] = c;
        mock_print_index++;
        return 0; // Success
    }
    return -1; // Buffer full
}

// #############################################################################
// # setup & teardown for testing
// ###########################################################################

cli_cfg_t g_cli_cfg_test;

void setUp(void)
{
    // Register mock assert callback
    custom_assert_init(mock_assert_callback);

    // Reset the assert trigger
    last_assert_trigger.last_assert_file = NULL;
    last_assert_trigger.last_assert_line = 0;
    last_assert_trigger.last_assert_expr = NULL;

    // Reset mock print buffer
    memset(mock_print_buffer, 0, MOCK_BUFFER_SIZE);
    mock_print_index = 0;

    // Initialize the cli
    cli_init(&g_cli_cfg_test, mock_put_char);
}

void tearDown(void)
{
    // Unregister assert callback
    custom_assert_deinit();

    cli_deinit(&g_cli_cfg_test);
}

void test_cli_unknown_command_triggers_error_message(void)
{
    // Simulate input for an unknown command
    const char* input = "unknowncmd\n";
    for (size_t i = 0; i < strlen(input); i++)
    {
        cli_receive(input[i]);
    }

    // Process the input
    cli_process();

    // Check that no assert was triggered
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_file);
    TEST_ASSERT_EQUAL(0, last_assert_trigger.last_assert_line);
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_expr);
    // Check that the output contains the unknown command message
    TEST_ASSERT_NOT_EQUAL(0, mock_print_index);
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "Unknown command: unknowncmd"));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "Type 'help' to list all commands"));
}

void test_cli_help_command_lists_registered_commands(void)
{
    const char* input = "help\n";

    // Register all commands
    for (size_t i = 0; i < sizeof(cli_bindings) / sizeof(cli_binding_t); i++)
    {
        cli_register(&cli_bindings[i]);
    }

    // Simulate input for the "help" command

    for (size_t i = 0; i < strlen(input); i++)
    {
        cli_receive(input[i]);
    }

    // Process the input
    cli_process();

    // Check that no assert was triggered
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_file);
    TEST_ASSERT_EQUAL(0, last_assert_trigger.last_assert_line);
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_expr);

    // Check that the output contains the help information
    TEST_ASSERT_NOT_EQUAL(0, mock_print_index);
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* help:"));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* clear:"));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* hello:"));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* args:"));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* echo:"));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* dummy:"));

    // Unregister all commands
    for (size_t i = 0; i < sizeof(cli_bindings) / sizeof(cli_binding_t); i++)
    {
        cli_unregister(cli_bindings[i].cmd_name_string);

        // Check that no assert was triggered during unregister
        TEST_ASSERT_NULL(last_assert_trigger.last_assert_file);
        TEST_ASSERT_EQUAL(0, last_assert_trigger.last_assert_line);
        TEST_ASSERT_NULL(last_assert_trigger.last_assert_expr);
    }

    // resend the help command to see that only the built-in commands are listed
    memset(mock_print_buffer, 0, MOCK_BUFFER_SIZE);
    mock_print_index = 0;
    for (size_t i = 0; i < strlen(input); i++)
    {
        cli_receive(input[i]);

        // Check that no assert was triggered during unregister
        TEST_ASSERT_NULL(last_assert_trigger.last_assert_file);
        TEST_ASSERT_EQUAL(0, last_assert_trigger.last_assert_line);
        TEST_ASSERT_NULL(last_assert_trigger.last_assert_expr);
    }
    cli_process();

    // Check that the output contains the help information for built-in commands only
    TEST_ASSERT_NOT_EQUAL(0, mock_print_index);
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* help:"));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* clear:"));
    TEST_ASSERT_NULL(strstr(mock_print_buffer, "* hello:"));
    TEST_ASSERT_NULL(strstr(mock_print_buffer, "* args:"));
    TEST_ASSERT_NULL(strstr(mock_print_buffer, "* echo:"));
    TEST_ASSERT_NULL(strstr(mock_print_buffer, "* dummy:"));
}