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

#include "Cli.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "custom_assert.h"
#include "test_support.h"

/* #############################################################################
 * # Defines
 * ###########################################################################*/

#define CLI_MAX_NOF_ARGUMENTS (16)
#define CLI_PROMPT            "> "
#define CLI_PROMPT_SPACER     '='
#define CLI_SECTION_SPACER    '-'
#define CLI_OUTPUT_WIDTH      50
#define CLI_CANARY            (0xA5A5A5A5U)
#define CLI_OK_PROMPT         "\033[32m[OK]  \033[0m "
#define CLI_FAIL_PROMPT       "\033[31m[FAIL]\033[0m "

/* #############################################################################
 * # static variables
 * ###########################################################################*/

static cli_cfg_t* g_cli_cfg = NULL;

/* #############################################################################
 * # static function prototypes
 * ###########################################################################*/

static void prv_write_string(const char* str);
static void prv_write_char(char in_char);
static void prv_put_char(char in_char);
static void prv_write_cli_prompt(void);
static void prv_write_cmd_unknown(const char* const in_cmd_name);
static void prv_plot_lines(char in_char, int length);

static void prv_reset_rx_buffer(void);
static bool prv_is_rx_buffer_full(void);
static char prv_get_last_recv_char_from_rx_buffer(void);

static const cli_binding_t* prv_find_cmd(const char* const in_cmd_name);
static uint8_t prv_get_args_from_rx_buffer(char* array_of_arguments[], uint8_t max_arguments);
STATIC void prv_find_matching_strings(const char* in_partial_string, const char* const in_string_array[],
                                      uint8_t in_nof_strings, const char* out_matches_array[],
                                      uint8_t* out_nof_matches);
static bool prv_is_char_in_string(char character, const char* in_string, uint8_t string_length);
static void prv_autocomplete_command(void);

static int prv_cmd_handler_help(int argc, char* argv[], void* context);
static int prv_cmd_handler_clear_screen(int argc, char* argv[], void* context);
static int prv_cmd_handler_reset_cli(int argc, char* argv[], void* context);

static void prv_verify_object_integrity(const cli_cfg_t* const in_ptCfg);

/* #############################################################################
 * # global function implementations
 * ###########################################################################*/

void cli_init(cli_cfg_t* const inout_module_cfg, cli_put_char_fn in_put_char_fn)
{
    { // Input Checks
        ASSERT(inout_module_cfg);
        ASSERT(false == inout_module_cfg->is_initialized);
        ASSERT(NULL == g_cli_cfg); // only one instance allowed
        ASSERT(in_put_char_fn);
    }

    inout_module_cfg->start_canary_word = CLI_CANARY;
    inout_module_cfg->end_canary_word = CLI_CANARY;
    inout_module_cfg->mid_canary_word = CLI_CANARY;
    inout_module_cfg->put_char_fn = in_put_char_fn;
    inout_module_cfg->nof_stored_chars_in_rx_buffer = 0;
    inout_module_cfg->nof_stored_cmd_bindings = 0;

    // Store the config locally in a static variable
    g_cli_cfg = inout_module_cfg;

    g_cli_cfg->is_initialized = true;

    // Register the default commands
    cli_binding_t help_cmd_binding = {"help", prv_cmd_handler_help, NULL, "List all commands"};
    cli_binding_t clear_cmd_binding = {"clear", prv_cmd_handler_clear_screen, NULL, "Clear the screen"};
    cli_binding_t reset_cmd_binding = {"reset", prv_cmd_handler_reset_cli, NULL, "Reset the CLI"};
    cli_register(&help_cmd_binding);
    cli_register(&clear_cmd_binding);
    cli_register(&reset_cmd_binding);

    // reset the cli
    prv_cmd_handler_clear_screen(0, NULL, NULL);

    // Print the prompt
    prv_write_cli_prompt();

    return;
}

void cli_receive(char in_char)
{
    prv_verify_object_integrity(g_cli_cfg);

    if (true == prv_is_rx_buffer_full())
    {
        // Buffer full - ignore the character
        prv_write_string("Buffer is full\n");

        // Reset the buffer to avoid overflows
        prv_reset_rx_buffer();
        prv_write_cli_prompt();

        return;
    }

    switch (in_char)
    {
        case 0x7F: // DEL
        case '\b': // Backspace
        {
            bool rx_buffer_has_chars = (g_cli_cfg->nof_stored_chars_in_rx_buffer > 0);

            // Only delete characters, when there are characters in the buffer.
            if (true == rx_buffer_has_chars)
            {
                g_cli_cfg->nof_stored_chars_in_rx_buffer--;
                uint8_t idx = g_cli_cfg->nof_stored_chars_in_rx_buffer;

                // Remove the last character (the one that was deleted)
                // Replace it with a null character
                g_cli_cfg->rx_char_buffer[idx] = '\0';

                // Remove character from cli
                prv_write_char('\b');
            }
            break;
        }
        case '\t': // Tab
        {
            // autocomplete the currently incomplete command (if possible)
            prv_autocomplete_command();
            break;
        }
        case '\r': // Carriage Return
        {
            // Convert CR to LF to handle Enter key from terminal programs
            in_char = '\n';
            // Fall through to default case to process as normal character
            __attribute__((fallthrough));
        }
        default:
        {
            // Add the character to the buffer
            uint8_t idx = g_cli_cfg->nof_stored_chars_in_rx_buffer;
            g_cli_cfg->rx_char_buffer[idx] = in_char;
            g_cli_cfg->nof_stored_chars_in_rx_buffer++;

            prv_verify_object_integrity(g_cli_cfg);

            // write the character back out to the console
            prv_write_char(in_char);

            break;
        }
    }

    ASSERT(g_cli_cfg->nof_stored_chars_in_rx_buffer <= CLI_MAX_RX_BUFFER_SIZE);
}

void cli_process()
{
    char* argv[CLI_MAX_NOF_ARGUMENTS] = {0};
    uint8_t argc = 0;
    int cmd_status = CLI_FAIL_STATUS;

    prv_verify_object_integrity(g_cli_cfg);

    if ((prv_get_last_recv_char_from_rx_buffer() != '\n') && ((false == prv_is_rx_buffer_full())))
    {
        // Do nothing, if these conditions are not met
        return;
    }

    argc = prv_get_args_from_rx_buffer(argv, CLI_MAX_NOF_ARGUMENTS);

    if (argc >= 1)
    {
        // plot a line on the console
        prv_plot_lines(CLI_SECTION_SPACER, CLI_OUTPUT_WIDTH);

        // call the command handler (if available)
        const cli_binding_t* ptCmdBinding = prv_find_cmd(argv[0]);
        if (NULL == ptCmdBinding)
        {
            cmd_status = CLI_FAIL_STATUS;
            prv_write_cmd_unknown(argv[0]);
        }
        else
        {
            cmd_status = ptCmdBinding->cmd_fn(argc, argv, ptCmdBinding->context);
        }

        prv_plot_lines(CLI_SECTION_SPACER, CLI_OUTPUT_WIDTH);
        prv_write_string("Status -> ");
        prv_write_string((cmd_status == CLI_OK_STATUS) ? CLI_OK_PROMPT : CLI_FAIL_PROMPT);
        prv_write_char('\n');
    }

    // Reset the cli buffer and write the prompt again for a new user input
    prv_reset_rx_buffer();
    prv_write_cli_prompt();
}

void cli_receive_and_process(char in_char)
{
    cli_receive(in_char);
    cli_process();
}

void cli_register(const cli_binding_t* const in_cmd_binding)
{
    {
        // Input Checks - inout_ptCfg
        ASSERT(in_cmd_binding);
        ASSERT(in_cmd_binding->name);
        ASSERT(in_cmd_binding->help);
        ASSERT(in_cmd_binding->cmd_fn);

        prv_verify_object_integrity(g_cli_cfg);
    }

    if ((NULL == in_cmd_binding) || (0 == strlen(in_cmd_binding->name))
        || (strlen(in_cmd_binding->name) >= CLI_MAX_CMD_NAME_LENGTH) || NULL == in_cmd_binding->cmd_fn)
    {
        return;
    }

    uint8_t does_binding_exist = false;
    uint8_t is_binding_stored = false;

    for (uint8_t i = 0; i < g_cli_cfg->nof_stored_cmd_bindings; i++)
    {
        const cli_binding_t* cmd_binding = &g_cli_cfg->cmd_bindings_buffer[i];
        if (0 == strncmp(cmd_binding->name, in_cmd_binding->name, CLI_MAX_CMD_NAME_LENGTH))
        {
            does_binding_exist = true;
            break;
        }
    }
    ASSERT(false == does_binding_exist);

    if (g_cli_cfg->nof_stored_cmd_bindings < CLI_MAX_NOF_CALLBACKS)
    {
        //  Deep Copy the binding into the buffer
        uint8_t idx = g_cli_cfg->nof_stored_cmd_bindings;
        memcpy(&g_cli_cfg->cmd_bindings_buffer[idx], in_cmd_binding, sizeof(cli_binding_t));
        g_cli_cfg->nof_stored_cmd_bindings++;

        // Mark that the binding was stored
        is_binding_stored = true;
    }

    ASSERT(true == is_binding_stored);

    (void)does_binding_exist;
    (void)is_binding_stored;

    return;
}

void cli_unregister(const char* const in_cmd_name)
{
    {
        // Input Checks - inout_ptCfg
        ASSERT(in_cmd_name);
        ASSERT(strlen(in_cmd_name) > 0);
        ASSERT(strlen(in_cmd_name) < CLI_MAX_CMD_NAME_LENGTH);

        ASSERT(g_cli_cfg->nof_stored_cmd_bindings > 0);

        prv_verify_object_integrity(g_cli_cfg);
    }

    if ((NULL == in_cmd_name) || (0 == strlen(in_cmd_name)) || (strlen(in_cmd_name) >= CLI_MAX_CMD_NAME_LENGTH)
        || (g_cli_cfg->nof_stored_cmd_bindings == 0))
    {
        return;
    }

    uint8_t is_binding_found = false;

    for (uint8_t i = 0; i < g_cli_cfg->nof_stored_cmd_bindings; i++)
    {
        cli_binding_t* cmd_binding = &g_cli_cfg->cmd_bindings_buffer[i];
        if (0 == strncmp(cmd_binding->name, in_cmd_name, CLI_MAX_CMD_NAME_LENGTH))
        {
            is_binding_found = true;
            // Shift all following bindings one position to the left
            for (uint8_t j = i; j < g_cli_cfg->nof_stored_cmd_bindings - 1; j++)
            {
                memcpy(&g_cli_cfg->cmd_bindings_buffer[j], &g_cli_cfg->cmd_bindings_buffer[j + 1],
                       sizeof(cli_binding_t));
            }
            g_cli_cfg->nof_stored_cmd_bindings--;
            break;
        }
    }
    ASSERT(true == is_binding_found);

    (void)is_binding_found;

    return;
}

void cli_print(const char* fmt, ...)
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
        ASSERT(fmt);
    }

    char buffer[128]; // Temporary buffer for formatted string
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    prv_write_string(buffer);
    prv_write_char('\n');
}

void cli_deinit(cli_cfg_t* const inout_module_cfg)
{
    { // Input Checks
        prv_verify_object_integrity(inout_module_cfg);
        ASSERT(inout_module_cfg == g_cli_cfg); // only one instance allowed
    }
    inout_module_cfg->is_initialized = false;
    g_cli_cfg = NULL;

    // Clear the config structure
    memset(inout_module_cfg, 0, sizeof(cli_cfg_t));
}

/* #############################################################################
 * # static function implementations
 * ###########################################################################*/

static void prv_write_string(const char* in_string)
{
    {
        prv_verify_object_integrity(g_cli_cfg);
    }
    for (const char* current_char = in_string; *current_char != '\0'; current_char++)
    {
        prv_write_char(*current_char);
    }
}

static void prv_write_char(char in_char)
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
    }
    if ('\n' == in_char) // User pressed Enter
    {
        prv_put_char('\r');
        prv_put_char('\n');
    }
    else if ('\b' == in_char) // User pressed Backspace
    {
        prv_put_char('\b');
        prv_put_char(' ');
        prv_put_char('\b');
    }
    else // Every other character
    {
        prv_put_char(in_char);
    }
}

static void prv_put_char(char in_char)
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
    }
    g_cli_cfg->put_char_fn(in_char);
}

static void prv_write_cli_prompt()
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
    }
    prv_plot_lines(CLI_PROMPT_SPACER, CLI_OUTPUT_WIDTH);
    prv_write_string(CLI_PROMPT);
}

static void prv_write_cmd_unknown(const char* const in_cmd_name)
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
    }
    prv_write_string("Unknown command: ");
    prv_write_string(in_cmd_name);
    prv_write_char('\n');
    prv_write_string("Type 'help' to list all commands\n");
}

static void prv_reset_rx_buffer()
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
    }
    memset(g_cli_cfg->rx_char_buffer, 0, CLI_MAX_RX_BUFFER_SIZE);
    g_cli_cfg->nof_stored_chars_in_rx_buffer = 0;
}

static bool prv_is_rx_buffer_full()
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
    }
    return (g_cli_cfg->nof_stored_chars_in_rx_buffer < CLI_MAX_RX_BUFFER_SIZE) ? false : true;
}

static char prv_get_last_recv_char_from_rx_buffer(void)
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
    }
    return g_cli_cfg->rx_char_buffer[g_cli_cfg->nof_stored_chars_in_rx_buffer - 1];
}

static const cli_binding_t* prv_find_cmd(const char* const in_cmd_name)
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
        ASSERT(in_cmd_name);
        ASSERT(g_cli_cfg->nof_stored_cmd_bindings > 0);
    }

    for (uint8_t idx = 0; idx < g_cli_cfg->nof_stored_cmd_bindings; ++idx)
    {
        const cli_binding_t* cmd_binding = &g_cli_cfg->cmd_bindings_buffer[idx];
        if (0 == strncmp(cmd_binding->name, in_cmd_name, CLI_MAX_CMD_NAME_LENGTH))
        {
            return cmd_binding;
        }
    }
    return NULL;
}

static int prv_cmd_handler_clear_screen(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;
    // ANSI escape code to clear screen and move cursor to home
    cli_print("\033[2J\033[H");
    return CLI_OK_STATUS;
}

static uint8_t prv_get_args_from_rx_buffer(char* array_of_arguments[], uint8_t max_arguments)
{
    { // Input Checks
        ASSERT(array_of_arguments);
        ASSERT(max_arguments > 0);

        prv_verify_object_integrity(g_cli_cfg);
    }

    uint8_t nof_identified_arguments = 0;
    char* next_argument = NULL;

    // Process the Buffer - tokenize arguments separated by spaces or newlines
    for (uint8_t i = 0; i < g_cli_cfg->nof_stored_chars_in_rx_buffer; i++)
    {
        ASSERT(nof_identified_arguments <= max_arguments);

        char* const current_char = &g_cli_cfg->rx_char_buffer[i];
        const bool is_delimiter_char = (' ' == *current_char || '\n' == *current_char);
        const bool is_last_char = (i == (g_cli_cfg->nof_stored_chars_in_rx_buffer - 1));

        if (is_delimiter_char)
        {
            // Found delimiter - terminate current argument if any
            *current_char = '\0';
            if (next_argument != NULL)
            {
                array_of_arguments[nof_identified_arguments] = next_argument;
                nof_identified_arguments++;
                next_argument = NULL;
            }
        }
        else
        {
            // Non-delimiter character
            if (next_argument == NULL)
            {
                // Start of new argument
                next_argument = current_char;
            }

            // Handle last character if it's not a delimiter
            if (is_last_char && next_argument != NULL)
            {
                array_of_arguments[nof_identified_arguments] = next_argument;
                nof_identified_arguments++;
                next_argument = NULL;
            }
        }
    }

    return nof_identified_arguments;
}

static int prv_cmd_handler_help(int argc, char* argv[], void* context)
{
    { // Input Checks
        prv_verify_object_integrity(g_cli_cfg);
    }

    // Create a list of all registered commands
    for (uint8_t i = 0; i < g_cli_cfg->nof_stored_cmd_bindings; ++i)
    {
        const cli_binding_t* ptCmdBinding = &g_cli_cfg->cmd_bindings_buffer[i];
        prv_write_string("* ");
        prv_write_string(ptCmdBinding->name);
        prv_write_string(": \n              ");
        prv_write_string(ptCmdBinding->help);
        prv_write_char('\n');
    }

    (void)argc;
    (void)argv;
    (void)context;

    return CLI_OK_STATUS;
}

static int prv_cmd_handler_reset_cli(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;

    // Clear the screen
    prv_cmd_handler_clear_screen(0, NULL, NULL);

    // Reset the Rx Buffer and print the welcome message
    prv_reset_rx_buffer();
    prv_write_string("CLI was reset - enter your commands (or enter 'help')\n");
    prv_write_cli_prompt();

    return CLI_OK_STATUS;
}

static void prv_verify_object_integrity(const cli_cfg_t* const in_ptCfg)
{
    ASSERT(in_ptCfg);
    ASSERT(in_ptCfg->rx_char_buffer);
    ASSERT(in_ptCfg->put_char_fn);
    ASSERT(true == in_ptCfg->is_initialized);
    ASSERT(CLI_CANARY == in_ptCfg->start_canary_word);
    ASSERT(CLI_CANARY == in_ptCfg->mid_canary_word);
    ASSERT(CLI_CANARY == in_ptCfg->end_canary_word);
}

static void prv_plot_lines(char in_char, int length)
{
    ASSERT(length < 100);

    for (int counter = 0; counter < length; ++counter)
    {
        prv_write_char(in_char);
    }
    prv_write_char('\n');
}

STATIC void prv_find_matching_strings(const char* in_partial_string, const char* const in_string_array[],
                                      uint8_t in_nof_strings, const char* out_matches_array[], uint8_t* out_nof_matches)
{
    { // Input checks
        ASSERT(in_partial_string);
        ASSERT(in_string_array);
        ASSERT(in_nof_strings > 0);
        ASSERT(out_matches_array);
        ASSERT(out_nof_matches);
    }
    uint8_t in_string_idx = 0;
    uint8_t out_matches_idx = 0;

    for (in_string_idx = 0; in_string_idx < in_nof_strings; ++in_string_idx)
    {
        // Get the current string
        const char* current_string = in_string_array[in_string_idx];

        // Check whether the partial string is in the current string
        if (NULL != strstr(current_string, in_partial_string))
        {
            out_matches_array[out_matches_idx] = current_string;
            out_matches_idx++;
        }
    }

    *out_nof_matches = out_matches_idx;

    ASSERT(*out_nof_matches <= in_nof_strings);
}

static bool prv_is_char_in_string(char character, const char* in_string, uint8_t string_length)
{
    { // Input Checks
        ASSERT(in_string);
        ASSERT(string_length != 0);
        prv_verify_object_integrity(g_cli_cfg);
    }

    bool is_space_char_in_string = false;

    // Check that there is no ' ' character in the rx buffer - no autocompletes on arguments; only commands
    // 'cmd' + ' ' + 'args[]'
    for (uint8_t i = 0; i < string_length; i++)
    {
        if (character == in_string[i])
        {
            is_space_char_in_string = true;
            break;
        }
    }

    return is_space_char_in_string;
}

static void prv_autocomplete_command(void)
{
    // input Checks
    prv_verify_object_integrity(g_cli_cfg);

    bool is_command_string = true;

    // if there is a space character in the string, it means that the command was already entered fully,
    // so there is no reason for further autocompletion
    is_command_string =
        !prv_is_char_in_string(' ', g_cli_cfg->rx_char_buffer, g_cli_cfg->nof_stored_chars_in_rx_buffer);
    if (false == is_command_string)
    {
        return;
    }

    // Create a copy of the command names and place them in an array
    const char* command_names[CLI_MAX_NOF_CALLBACKS] = {0};
    for (uint8_t i = 0; i < g_cli_cfg->nof_stored_cmd_bindings; i++)
    {
        command_names[i] = g_cli_cfg->cmd_bindings_buffer[i].name;
    }

    const char* matches[CLI_MAX_NOF_CALLBACKS] = {0};
    uint8_t nof_matches = 0;

    prv_find_matching_strings(g_cli_cfg->rx_char_buffer, command_names, g_cli_cfg->nof_stored_cmd_bindings, matches,
                              &nof_matches);
    if (nof_matches == 1)
    {
        ASSERT(strlen(matches[0]) > 0);

        // Only one match - autocomplete the command
        // If there are more matches, then the user needs to provide more letters for specification

        // Calculate how many characters we need to delete (current input)
        uint8_t chars_to_delete = g_cli_cfg->nof_stored_chars_in_rx_buffer;

        // Delete the current input from the display and the rx buffer
        for (uint8_t i = 0; i < chars_to_delete; i++)
        {
            g_cli_cfg->rx_char_buffer[i] = '\0';
            prv_write_char('\b');
        }

        // Replace the content of the rx buffer with the match
        strncpy(g_cli_cfg->rx_char_buffer, matches[0], CLI_MAX_RX_BUFFER_SIZE);
        g_cli_cfg->nof_stored_chars_in_rx_buffer = strlen(matches[0]);

        // Write the autocompleted command to the console
        prv_write_string(g_cli_cfg->rx_char_buffer);
    }
}
