/**
 * MIT License
 *
 * Copyright (c) <2025> <Max Koell (maxkoell@proton.me)>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
int prv_cmd_hello_world(int argc, char* argv[], void* context);
int prv_cmd_echo_string(int argc, char* argv[], void* context);
int prv_cmd_display_args(int argc, char* argv[], void* context);

int prv_cmd_hello_world(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;
    cli_print("Hello World!\n");
    return CLI_OK_STATUS;
}

int prv_cmd_echo_string(int argc, char* argv[], void* context)
{
    if (argc != 2)
    {
        cli_print("Give one argument\n");
        return CLI_FAIL_STATUS;
    }
    (void)argv;
    (void)context;
    cli_print("%s\"\"\n", argv[1]);
    return CLI_OK_STATUS;
}

int prv_cmd_display_args(int argc, char* argv[], void* context)
{
    int i;
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
    return CLI_OK_STATUS;
}

static cli_binding_t cli_bindings[] = {
    {"hello", prv_cmd_hello_world, NULL, "Say hello"},
    {"args", prv_cmd_display_args, NULL, "Displays the given cli arguments"},
    {"echo", prv_cmd_echo_string, NULL, "Echoes the given string"},
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
    const char* test_name;
    u32 assert_count;
} assert_trigger_t;

assert_trigger_t last_assert_trigger;

// Current test name tracking
static const char* current_test_name = NULL;

// Helper macro to set test name at the beginning of each test
#define SET_TEST_NAME(name)                                                                                            \
    do                                                                                                                 \
    {                                                                                                                  \
        current_test_name = name;                                                                                      \
    } while (0)

// Helper function to reset assert tracking
static void reset_assert_tracking(void)
{
    last_assert_trigger.last_assert_file = NULL;
    last_assert_trigger.last_assert_line = 0;
    last_assert_trigger.last_assert_expr = NULL;
    last_assert_trigger.test_name = NULL;
}

// Helper function to verify that an assert was triggered
static void verify_assert_triggered(const char* expected_test_name)
{
    TEST_ASSERT_NOT_NULL(last_assert_trigger.last_assert_file);
    TEST_ASSERT_NOT_EQUAL(0, last_assert_trigger.last_assert_line);
    TEST_ASSERT_NOT_NULL(last_assert_trigger.last_assert_expr);

    if (expected_test_name != NULL)
    {
        TEST_ASSERT_EQUAL_STRING(expected_test_name, last_assert_trigger.test_name);
    }

    //    printf("Assert verified for test: %s\n", expected_test_name ? expected_test_name : "Unknown");
}

// Helper function to verify that no assert was triggered
static void verify_no_assert_triggered(void) { TEST_ASSERT_EQUAL(0, last_assert_trigger.assert_count); }

// Mock assert callback for testing
static void mock_assert_callback(const char* file, uint32_t line, const char* expr)
{
    last_assert_trigger.last_assert_file = file;
    last_assert_trigger.last_assert_line = line;
    last_assert_trigger.last_assert_expr = expr;
    last_assert_trigger.test_name = current_test_name;

    printf("=== ASSERT TRIGGERED ===================================================\n");
    printf("Test: %s\n", current_test_name ? current_test_name : "Unknown");
    printf("File: %s(%u)\n", file, line);
    printf("Expression: %s\n", expr);
    printf("========================================================================\n\n\n");
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

    // Reset assert tracking
    reset_assert_tracking();

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

    // Reset assert tracking
    reset_assert_tracking();

    // Reset current test name
    current_test_name = NULL;

    cli_deinit(&g_cli_cfg_test);
}

void test_cli_unknown_command_triggers_error_message(void)
{
    SET_TEST_NAME("test_cli_unknown_command_triggers_error_message");

    // Simulate input for an unknown command
    const char* input = "unknowncmd\n";
    for (size_t i = 0; i < strlen(input); i++)
    {
        cli_receive(input[i]);
    }

    // Process the input
    cli_process();

    // Check that no assert was triggered
    verify_no_assert_triggered();

    // Check that the output contains the unknown command message
    TEST_ASSERT_NOT_EQUAL(0, mock_print_index);
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "Unknown command: unknowncmd"));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "Type 'help' to list all commands"));
}

void test_cli_help_command_lists_registered_commands(void)
{
    SET_TEST_NAME("test_cli_help_command_lists_registered_commands");

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
    verify_no_assert_triggered();

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
        cli_unregister(cli_bindings[i].name);

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

void test_cli_insert_too_many_characters(void)
{
    // Simulate input that exceeds the buffer size
    for (size_t i = 0; i < CLI_MAX_RX_BUFFER_SIZE + 10; i++)
    {
        cli_receive('a'); // Fill the buffer with 'a'
    }
    cli_receive('\n'); // End the command

    // Process the input
    cli_process();

    // Check that assert was triggered due to buffer overflow
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_file);
    TEST_ASSERT_EQUAL(0, last_assert_trigger.last_assert_line);
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_expr);

    // Check that the buffer did not exceed its maximum size
    TEST_ASSERT_LESS_OR_EQUAL(CLI_MAX_RX_BUFFER_SIZE, g_cli_cfg_test.nof_stored_chars_in_rx_buffer);

    // Check that "Buffer is full" message was shown
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "Buffer is full"));
}

void test_cli_empty_command_does_nothing(void)
{
    // Simulate empty input (just newline)
    const char* input = "\n";
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

    // Should just show the prompt again
    TEST_ASSERT_NOT_EQUAL(0, mock_print_index);
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "> "));
}

void test_cli_whitespace_only_command_does_nothing(void)
{
    // Simulate whitespace-only input
    const char* input = "   \n";
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

    // Should just show the prompt again
    TEST_ASSERT_NOT_EQUAL(0, mock_print_index);
}

void test_cli_backspace_removes_characters(void)
{
    // Add some characters, then backspace
    cli_receive('h');  // buffer: "h"
    cli_receive('e');  // buffer: "he"
    cli_receive('l');  // buffer: "hel"
    cli_receive('\b'); // Remove 'l', buffer: "he"
    cli_receive('\b'); // Remove 'e', buffer: "h"
    cli_receive('e');  // buffer: "he"
    cli_receive('l');  // buffer: "hel"
    cli_receive('p');  // buffer: "help"
    cli_receive('\n'); // trigger processing

    // Process the input - should execute "help" command
    cli_process();

    // Check that no assert was triggered
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_file);
    TEST_ASSERT_EQUAL(0, last_assert_trigger.last_assert_line);
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_expr);

    // Check that the help command was executed successfully
    TEST_ASSERT_NOT_EQUAL(0, mock_print_index);
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* help:"));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "* clear:"));
}

void test_cli_backspace_on_empty_buffer(void)
{
    // Try backspace on empty buffer
    cli_receive('\b');

    TEST_ASSERT_NULL(last_assert_trigger.last_assert_file);
    TEST_ASSERT_EQUAL(0, last_assert_trigger.last_assert_line);
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_expr);

    TEST_ASSERT_TRUE((g_cli_cfg_test.nof_stored_chars_in_rx_buffer == 0));
}

void test_cli_command_with_multiple_arguments(void)
{
    // Register a command that uses multiple arguments
    cli_register(&cli_bindings[1]); // args command

    const char* input = "args one two three\n";
    for (size_t i = 0; i < strlen(input); i++)
    {
        cli_receive(input[i]);
    }

    cli_process();

    // Check output contains all arguments
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "argv[0] --> \"args\""));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "argv[1] --> \"one\""));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "argv[2] --> \"two\""));
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "argv[3] --> \"three\""));

    cli_unregister("args");
}

// void test_cli_command_with_too_many_arguments(void)
// {
//     // Register a command
//     cli_register(&cli_bindings[1]); // args command

//     // Create input with more than CLI_MAX_NOF_ARGUMENTS (16) arguments
//     char input[256] = "args";
//     for (int i = 1; i <= 20; i++)
//     {
//         char arg[16];
//         snprintf(arg, sizeof(arg), " arg%d", i);
//         strcat(input, arg);
//     }
//     strcat(input, "\n");

//     for (size_t i = 0; i < strlen(input); i++)
//     {
//         cli_receive(input[i]);
//     }

//     cli_process();

//     // Should show "Too many arguments" message
//     TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "Too many arguments"));

//     cli_unregister("args");
// }

void test_cli_register_duplicate_command_triggers_assert(void)
{
    SET_TEST_NAME("test_cli_register_duplicate_command_triggers_assert");

    // Register a command
    cli_register(&cli_bindings[0]); // hello command

    // Try to register the same command again - should trigger assert
    cli_register(&cli_bindings[0]); // hello command again

    // Check that assert was triggered
    verify_assert_triggered("test_cli_register_duplicate_command_triggers_assert");
}

void test_cli_unregister_nonexistent_command_triggers_assert(void)
{
    SET_TEST_NAME("test_cli_unregister_nonexistent_command_triggers_assert");

    // Try to unregister a command that doesn't exist
    cli_unregister("nonexistent");

    // Check that assert was triggered
    verify_assert_triggered("test_cli_unregister_nonexistent_command_triggers_assert");
}

void test_cli_register_too_many_commands_triggers_assert(void)
{
    SET_TEST_NAME("test_cli_register_too_many_commands_triggers_assert");

    // Register commands until we exceed CLI_MAX_NOF_CALLBACKS
    // First register our test commands
    for (size_t i = 0; i < sizeof(cli_bindings) / sizeof(cli_binding_t); i++)
    {
        cli_register(&cli_bindings[i]);
    }

    // Create additional dummy commands to exceed the limit
    static cli_binding_t dummy_commands[20];
    for (int i = 0; i < 20; i++)
    {
        snprintf((char*)dummy_commands[i].name, CLI_MAX_CMD_NAME_LENGTH, "dummy%d", i);
        dummy_commands[i].cmd_fn = cmd_dummy;
        dummy_commands[i].context = NULL;
        snprintf((char*)dummy_commands[i].help, CLI_MAX_HELPER_STRING_LENGTH, "dummy command");

        cli_register(&dummy_commands[i]);

        // Eventually should trigger assert when buffer is full
        if (last_assert_trigger.last_assert_file != NULL)
        {
            break;
        }
    }

    // Check that assert was triggered
    verify_assert_triggered("test_cli_register_too_many_commands_triggers_assert");
}

void test_cli_clear_command_works(void)
{
    const char* input = "clear\n";
    for (size_t i = 0; i < strlen(input); i++)
    {
        cli_receive(input[i]);
    }

    cli_process();

    // Check that clear screen ANSI codes are present
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "\033[2J\033[H"));
}

void test_cli_buffer_full_message(void)
{
    // Fill buffer to exactly the maximum capacity
    for (size_t i = 0; i < CLI_MAX_RX_BUFFER_SIZE; i++)
    {
        cli_receive('a');
    }

    // Reset mock buffer to capture the "Buffer is full" message
    memset(mock_print_buffer, 0, MOCK_BUFFER_SIZE);
    mock_print_index = 0;

    // Try to add one more character - should trigger buffer full message
    cli_receive('b');

    // Check for buffer full message
    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "Buffer is full"));
}

void test_cli_command_with_context(void)
{
    // Create a command with context
    static int test_context = 42;
    static cli_binding_t context_cmd = {"context", cmd_dummy, &test_context, "Command with context"};

    cli_register(&context_cmd);

    const char* input = "context\n";
    for (size_t i = 0; i < strlen(input); i++)
    {
        cli_receive(input[i]);
    }

    cli_process();

    // Should execute without issues
    TEST_ASSERT_NULL(last_assert_trigger.last_assert_file);

    cli_unregister("context");
}

void test_cli_echo_command_wrong_arguments(void)
{
    cli_register(&cli_bindings[2]); // echo command

    // Test with no arguments
    const char* input1 = "echo\n";
    for (size_t i = 0; i < strlen(input1); i++)
    {
        cli_receive(input1[i]);
    }
    cli_process();

    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "Give one argument"));

    // Reset buffer
    memset(mock_print_buffer, 0, MOCK_BUFFER_SIZE);
    mock_print_index = 0;

    // Test with too many arguments
    const char* input2 = "echo arg1 arg2\n";
    for (size_t i = 0; i < strlen(input2); i++)
    {
        cli_receive(input2[i]);
    }
    cli_process();

    TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "Give one argument"));

    cli_unregister("echo");
}

// void test_cli_recieve_can_autocomplete_inputs(void)
// {
//     // Register the hello world commands (help command is built-in)
//     cli_register(&cli_bindings[0]); // hello command

//     // test with args -> enter 'hel' + tab
//     cli_receive('h');
//     cli_receive('e');
//     cli_receive('l');
//     cli_receive('\t');

//     // Cli returns a list consisting of 'hello, help'
//     TEST_ASSERT_NOT_EQUAL(0, mock_print_index);
//     TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "hello"));
//     TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "help"));

//     // add another character 'l' + tab
//     cli_receive('l');
//     cli_receive('\t');

//     // Cli returns a list consisting of 'hello'
//     TEST_ASSERT_NOT_EQUAL(0, mock_print_index);
//     TEST_ASSERT_NOT_NULL(strstr(mock_print_buffer, "hello"));
// }

void test_prv_find_matching_strings(void)
{
    const char* input_strings[] = {"help", "hello", "clear"};
    const char* matches[10];
    uint8_t num_matches;

    prv_find_matching_strings("he", input_strings, CLI_GET_ARRAY_SIZE(input_strings), matches, &num_matches);

    TEST_ASSERT_EQUAL(2, num_matches);
    TEST_ASSERT_EQUAL_STRING("help", matches[0]);
    TEST_ASSERT_EQUAL_STRING("hello", matches[1]);
}

// void test_cli_receive_found_one_match(void) {}

void test_cli_receive_found_several_matches(void)
{
    // Register the hello world commands (help command is built-in)
    cli_register(&cli_bindings[0]); // hello command

    // test with args -> enter 'hel' + tab
    cli_receive('h');
    cli_receive('e');
    cli_receive('\t');
}
