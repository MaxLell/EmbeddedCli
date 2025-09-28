#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Cli.h"
#include "custom_assert.h"
#include "custom_types.h"
#include "unity.h"

/**
 * Unit Tests for CLI Module
 * 
 * Test Coverage:
 * - cli_init: Initialization with valid/invalid parameters
 * - cli_register: Command registration and validation
 * - cli_unregister: Command removal
 * - cli_receive: Character reception and buffer handling
 * - cli_process: Command processing and execution
 * - cli_receive_and_process: Combined receive and process
 * - cli_print: Formatted output
 * 
 * Edge Cases:
 * - Buffer overflow scenarios
 * - Invalid input parameters
 * - Command not found
 * - Maximum number of commands
 */

typedef struct
{
    char* last_assert_file;
    u32 last_assert_line;
    char* last_assert_expr;
} assert_trigger_t;

// Test configuration and mocks
static cli_cfg_t test_cli_cfg;
static char output_buffer[1024];
static size_t output_buffer_index;
static int assert_call_count;
static assert_trigger_t last_assert_trigger;

// Mock put_char function for testing
static int mock_put_char(char c)
{
    if (output_buffer_index < sizeof(output_buffer) - 1)
    {
        output_buffer_index++;
        output_buffer[output_buffer_index] = c;
        output_buffer[output_buffer_index] = '\0';
    }
    return 0;
}

// Mock assert callback for testing
static void mock_assert_callback(const char* file, uint32_t line, const char* expr)
{
    assert_call_count++;
    last_assert_trigger.last_assert_file = file;
    last_assert_trigger.last_assert_line = line;
    last_assert_trigger.last_assert_expr = expr;
}

// Test command handler functions
static int test_command_handler_success(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;
    return CLI_OK_STATUS;
}

static int test_command_handler_fail(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;
    return CLI_FAIL_STATUS;
}

static int test_command_handler_with_context(int argc, char* argv[], void* context)
{
    int* counter = (int*)context;
    if (counter != NULL)
    {
        (*counter)++;
    }
    return CLI_OK_STATUS;
}

void setUp(void)
{
    // Reset test configuration
    memset(&test_cli_cfg, 0, sizeof(test_cli_cfg));

    // Reset output buffer
    memset(output_buffer, 0, sizeof(output_buffer));
    output_buffer_index = 0;

    // Reset assert tracking
    assert_call_count = 0;
    last_assert_file = NULL;
    last_assert_line = 0;
    last_assert_expr = NULL;

    // Register mock assert callback
    custom_assert_register(mock_assert_callback);
}

void tearDown(void)
{
    // Unregister assert callback
    custom_assert_unregister();
}

// ============================================================================
// Test Functions for cli_init
// ============================================================================

void test_cli_init_valid_parameters(void)
{
    // Test successful initialization
    cli_init(&test_cli_cfg, mock_put_char);

    TEST_ASSERT_EQUAL(1, test_cli_cfg.is_initialized);
    TEST_ASSERT_EQUAL_PTR(mock_put_char, test_cli_cfg.put_char_fn);
    TEST_ASSERT_EQUAL(0, test_cli_cfg.nof_stored_chars_in_rx_buffer);
    TEST_ASSERT_EQUAL(2, test_cli_cfg.nof_stored_cmd_bindings); // help + clear commands
    TEST_ASSERT_EQUAL(0, assert_call_count);

    // Check that output was generated (welcome message and prompt)
    TEST_ASSERT_TRUE(strlen(output_buffer) > 0);
}

void test_cli_init_null_config(void)
{
    // Test initialization with NULL config - should trigger assert
    cli_init(NULL, mock_put_char);

    TEST_ASSERT_EQUAL(1, assert_call_count);
}

void test_cli_init_null_put_char(void)
{
    // Test initialization with NULL put_char function - should trigger assert
    cli_init(&test_cli_cfg, NULL);

    TEST_ASSERT_EQUAL(1, assert_call_count);
}

void test_cli_init_already_initialized(void)
{
    // Initialize once
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0; // Reset counter after first init

    // Try to initialize again - should trigger assert
    cli_cfg_t second_cfg;
    memset(&second_cfg, 0, sizeof(second_cfg));
    cli_init(&second_cfg, mock_put_char);

    TEST_ASSERT_EQUAL(1, assert_call_count);
}

// ============================================================================
// Test Functions for cli_register
// ============================================================================

void test_cli_register_valid_command(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    cli_binding_t test_binding = {"test_cmd", test_command_handler_success, NULL, "Test command for unit testing"};

    cli_register(&test_binding);

    TEST_ASSERT_EQUAL(0, assert_call_count);
    TEST_ASSERT_EQUAL(3, test_cli_cfg.nof_stored_cmd_bindings); // help + clear + test_cmd
}

void test_cli_register_null_binding(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    cli_register(NULL);

    TEST_ASSERT_EQUAL(1, assert_call_count);
}

void test_cli_register_duplicate_command(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    cli_binding_t test_binding = {"help", // Duplicate of existing command
                                  test_command_handler_success, NULL, "Duplicate help command"};

    cli_register(&test_binding);

    TEST_ASSERT_EQUAL(1, assert_call_count);
}

void test_cli_register_max_commands(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    // Register commands until we hit the limit
    for (int i = 0; i < CLI_MAX_NOF_CALLBACKS - 2; i++)
    { // -2 because help and clear are already registered
        cli_binding_t test_binding;
        memset(&test_binding, 0, sizeof(test_binding));
        snprintf((char*)test_binding.cmd_name_string, CLI_MAX_CMD_NAME_LENGTH, "cmd%d", i);
        test_binding.cmd_handler_fn = test_command_handler_success;
        snprintf((char*)test_binding.cmd_helper_string, CLI_MAX_HELPER_STRING_LENGTH, "Test command %d", i);

        cli_register(&test_binding);
    }

    TEST_ASSERT_EQUAL(0, assert_call_count);
    TEST_ASSERT_EQUAL(CLI_MAX_NOF_CALLBACKS, test_cli_cfg.nof_stored_cmd_bindings);

    // Try to register one more - should trigger assert
    cli_binding_t overflow_binding = {"overflow", test_command_handler_success, NULL, "This should cause overflow"};

    cli_register(&overflow_binding);
    TEST_ASSERT_EQUAL(1, assert_call_count);
}

// ============================================================================
// Test Functions for cli_unregister
// ============================================================================

void test_cli_unregister_existing_command(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    cli_binding_t test_binding = {"test_cmd", test_command_handler_success, NULL, "Test command for unit testing"};

    cli_register(&test_binding);
    TEST_ASSERT_EQUAL(3, test_cli_cfg.nof_stored_cmd_bindings);

    assert_call_count = 0;
    cli_unregister("test_cmd");

    TEST_ASSERT_EQUAL(0, assert_call_count);
    TEST_ASSERT_EQUAL(2, test_cli_cfg.nof_stored_cmd_bindings);
}

void test_cli_unregister_nonexistent_command(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    cli_unregister("nonexistent");

    TEST_ASSERT_EQUAL(1, assert_call_count);
}

void test_cli_unregister_null_command(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    cli_unregister(NULL);

    TEST_ASSERT_EQUAL(1, assert_call_count);
}

// ============================================================================
// Test Functions for cli_receive
// ============================================================================

void test_cli_receive_normal_characters(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    // Send some characters
    cli_receive('h');
    cli_receive('e');
    cli_receive('l');
    cli_receive('p');

    TEST_ASSERT_EQUAL(0, assert_call_count);
    TEST_ASSERT_EQUAL(4, test_cli_cfg.nof_stored_chars_in_rx_buffer);
    TEST_ASSERT_EQUAL_CHAR('h', test_cli_cfg.rx_char_buffer[0]);
    TEST_ASSERT_EQUAL_CHAR('e', test_cli_cfg.rx_char_buffer[1]);
    TEST_ASSERT_EQUAL_CHAR('l', test_cli_cfg.rx_char_buffer[2]);
    TEST_ASSERT_EQUAL_CHAR('p', test_cli_cfg.rx_char_buffer[3]);
}

void test_cli_receive_carriage_return(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    cli_receive('a');
    cli_receive('\r'); // Should be ignored
    cli_receive('b');

    TEST_ASSERT_EQUAL(0, assert_call_count);
    TEST_ASSERT_EQUAL(2, test_cli_cfg.nof_stored_chars_in_rx_buffer);
    TEST_ASSERT_EQUAL_CHAR('a', test_cli_cfg.rx_char_buffer[0]);
    TEST_ASSERT_EQUAL_CHAR('b', test_cli_cfg.rx_char_buffer[1]);
}

void test_cli_receive_backspace(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    // Add some characters
    cli_receive('a');
    cli_receive('b');
    cli_receive('c');
    TEST_ASSERT_EQUAL(3, test_cli_cfg.nof_stored_chars_in_rx_buffer);

    // Send backspace
    cli_receive('\b');

    TEST_ASSERT_EQUAL(0, assert_call_count);
    TEST_ASSERT_EQUAL(2, test_cli_cfg.nof_stored_chars_in_rx_buffer);
    TEST_ASSERT_EQUAL_CHAR('\0', test_cli_cfg.rx_char_buffer[2]); // Should be cleared
}

void test_cli_receive_backspace_empty_buffer(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    // Send backspace on empty buffer
    cli_receive('\b');

    TEST_ASSERT_EQUAL(0, assert_call_count);
    TEST_ASSERT_EQUAL(0, test_cli_cfg.nof_stored_chars_in_rx_buffer);
}

void test_cli_receive_buffer_full(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Fill the buffer to capacity
    for (int i = 0; i < CLI_MAX_RX_BUFFER_SIZE; i++)
    {
        cli_receive('a');
    }

    TEST_ASSERT_EQUAL(CLI_MAX_RX_BUFFER_SIZE, test_cli_cfg.nof_stored_chars_in_rx_buffer);

    // Clear output buffer to check for "Buffer is full" message
    memset(output_buffer, 0, sizeof(output_buffer));
    output_buffer_index = 0;

    // Try to add one more character
    cli_receive('b');

    // Should still be at max capacity and show error message
    TEST_ASSERT_EQUAL(CLI_MAX_RX_BUFFER_SIZE, test_cli_cfg.nof_stored_chars_in_rx_buffer);
    TEST_ASSERT_TRUE(strstr(output_buffer, "Buffer is full") != NULL);
}

// ============================================================================
// Test Functions for cli_process
// ============================================================================

void test_cli_process_valid_command(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Send "help" command
    cli_receive('h');
    cli_receive('e');
    cli_receive('l');
    cli_receive('p');
    cli_receive('\n');

    // Clear output buffer to capture only process output
    memset(output_buffer, 0, sizeof(output_buffer));
    output_buffer_index = 0;

    cli_process();

    // Should have processed the command and reset buffer
    TEST_ASSERT_EQUAL(0, test_cli_cfg.nof_stored_chars_in_rx_buffer);
    TEST_ASSERT_TRUE(strlen(output_buffer) > 0); // Should have help output
}

void test_cli_process_unknown_command(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Send unknown command
    cli_receive('u');
    cli_receive('n');
    cli_receive('k');
    cli_receive('n');
    cli_receive('o');
    cli_receive('w');
    cli_receive('n');
    cli_receive('\n');

    // Clear output buffer to capture only process output
    memset(output_buffer, 0, sizeof(output_buffer));
    output_buffer_index = 0;

    cli_process();

    // Should have error message
    TEST_ASSERT_TRUE(strstr(output_buffer, "Unknown command") != NULL);
    TEST_ASSERT_EQUAL(0, test_cli_cfg.nof_stored_chars_in_rx_buffer);
}

void test_cli_process_command_with_arguments(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Register a test command
    cli_binding_t test_binding = {"test", test_command_handler_success, NULL, "Test command"};
    cli_register(&test_binding);

    // Send command with arguments
    const char* cmd = "test arg1 arg2";
    for (size_t i = 0; i < strlen(cmd); i++)
    {
        cli_receive(cmd[i]);
    }
    cli_receive('\n');

    cli_process();

    TEST_ASSERT_EQUAL(0, test_cli_cfg.nof_stored_chars_in_rx_buffer);
}

void test_cli_process_empty_command(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Send only newline
    cli_receive('\n');

    cli_process();

    // Should not crash and reset buffer
    TEST_ASSERT_EQUAL(0, test_cli_cfg.nof_stored_chars_in_rx_buffer);
}

void test_cli_process_buffer_not_ready(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Send command without newline
    cli_receive('h');
    cli_receive('e');
    cli_receive('l');
    cli_receive('p');

    size_t chars_before = test_cli_cfg.nof_stored_chars_in_rx_buffer;

    cli_process();

    // Should not process and buffer should remain unchanged
    TEST_ASSERT_EQUAL(chars_before, test_cli_cfg.nof_stored_chars_in_rx_buffer);
}

// ============================================================================
// Test Functions for cli_receive_and_process
// ============================================================================

void test_cli_receive_and_process_complete_command(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Send characters one by one
    cli_receive_and_process('h');
    cli_receive_and_process('e');
    cli_receive_and_process('l');
    cli_receive_and_process('p');

    // Buffer should contain characters but not processed yet
    TEST_ASSERT_EQUAL(4, test_cli_cfg.nof_stored_chars_in_rx_buffer);

    // Clear output buffer
    memset(output_buffer, 0, sizeof(output_buffer));
    output_buffer_index = 0;

    // Send newline - should trigger processing
    cli_receive_and_process('\n');

    // Should have processed and reset buffer
    TEST_ASSERT_EQUAL(0, test_cli_cfg.nof_stored_chars_in_rx_buffer);
    TEST_ASSERT_TRUE(strlen(output_buffer) > 0);
}

// ============================================================================
// Test Functions for cli_print
// ============================================================================

void test_cli_print_simple_string(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Clear output buffer
    memset(output_buffer, 0, sizeof(output_buffer));
    output_buffer_index = 0;

    cli_print("Test message");

    TEST_ASSERT_TRUE(strstr(output_buffer, "Test message") != NULL);
    TEST_ASSERT_TRUE(strstr(output_buffer, "\r\n") != NULL); // Should end with newline
}

void test_cli_print_formatted_string(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Clear output buffer
    memset(output_buffer, 0, sizeof(output_buffer));
    output_buffer_index = 0;

    cli_print("Number: %d, String: %s", 42, "test");

    TEST_ASSERT_TRUE(strstr(output_buffer, "Number: 42") != NULL);
    TEST_ASSERT_TRUE(strstr(output_buffer, "String: test") != NULL);
}

void test_cli_print_null_format(void)
{
    cli_init(&test_cli_cfg, mock_put_char);
    assert_call_count = 0;

    cli_print(NULL);

    TEST_ASSERT_EQUAL(1, assert_call_count);
}

// ============================================================================
// Integration Tests
// ============================================================================

void test_cli_integration_command_with_context(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    int counter = 0;
    cli_binding_t test_binding = {"count", test_command_handler_with_context, &counter, "Increment counter"};
    cli_register(&test_binding);

    // Execute command multiple times
    const char* cmd = "count\n";
    for (int i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < strlen(cmd); j++)
        {
            cli_receive_and_process(cmd[j]);
        }
    }

    TEST_ASSERT_EQUAL(3, counter);
}

void test_cli_integration_full_session(void)
{
    cli_init(&test_cli_cfg, mock_put_char);

    // Register test commands
    cli_binding_t cmd1 = {"cmd1", test_command_handler_success, NULL, "Command 1"};
    cli_binding_t cmd2 = {"cmd2", test_command_handler_fail, NULL, "Command 2"};

    cli_register(&cmd1);
    cli_register(&cmd2);

    // Test help command
    const char* help_cmd = "help\n";
    for (size_t i = 0; i < strlen(help_cmd); i++)
    {
        cli_receive_and_process(help_cmd[i]);
    }

    // Test custom command
    const char* custom_cmd = "cmd1\n";
    for (size_t i = 0; i < strlen(custom_cmd); i++)
    {
        cli_receive_and_process(custom_cmd[i]);
    }

    // Test unknown command
    const char* unknown_cmd = "unknown\n";
    for (size_t i = 0; i < strlen(unknown_cmd); i++)
    {
        cli_receive_and_process(unknown_cmd[i]);
    }

    // Unregister and test
    cli_unregister("cmd1");

    for (size_t i = 0; i < strlen(custom_cmd); i++)
    {
        cli_receive_and_process(custom_cmd[i]);
    }

    // Should complete without crashes
    TEST_ASSERT_EQUAL(0, test_cli_cfg.nof_stored_chars_in_rx_buffer);
}
