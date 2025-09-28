#include "Cli.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* #############################################################################
 * # Defines
 * ###########################################################################*/

#define CLI_MAX_NOF_ARGUMENTS    (16)
#define CLI_PROMPT               "-------- \n> "
#define CLI_CANARY               (0xA5A5A5A5U)
#define CLI_TRUE                 (1)
#define CLI_FALSE                (0)

#define CLI_ASSERT_STRINGIFY2(x) #x
#define CLI_ASSERT_STRINGIFY(x)  CLI_ASSERT_STRINGIFY2(x)

#define CLI_ASSERT(expr)                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(expr))                                                                                                   \
        {                                                                                                              \
            prv_assert_fail(": " #expr " "                                                                             \
                            "(" __FILE__ ":" CLI_ASSERT_STRINGIFY(__LINE__) ")\n");                                    \
        }                                                                                                              \
    } while (0)

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

static void prv_reset_rx_buffer(void);
static uint8_t prv_is_rx_buffer_full(void);
static char prv_get_last_recv_char_from_rx_buffer(void);

static const cli_binding_t* prv_find_cmd(const char* const in_cmd_name);

static int prv_cmd_handler_help(int argc, char* argv[], void* context);
static int prv_cmd_handler_clear_screen(int argc, char* argv[], void* context);

static void prv_assert_fail(const char* in_msg);

/* #############################################################################
 * # global function implementations
 * ###########################################################################*/

void cli_init(cli_cfg_t* const inout_module_cfg, cli_put_char_fn in_put_char_fn)
{
    { // Input Checks
        CLI_ASSERT(inout_module_cfg);
        CLI_ASSERT(false == inout_module_cfg->is_initialized);
        CLI_ASSERT(NULL == g_cli_cfg); // only one instance allowed
        CLI_ASSERT(in_put_char_fn);
    }

    cli_binding_t help_cmd_binding = {"help", prv_cmd_handler_help, NULL, "Lists all commands"};
    cli_binding_t clear_cmd_binding = {"clear", prv_cmd_handler_clear_screen, NULL, "Clears the screen"};

    inout_module_cfg->start_canary_word = CLI_CANARY;
    inout_module_cfg->end_canary_word = CLI_CANARY;
    inout_module_cfg->mid_canary_word = CLI_CANARY;
    inout_module_cfg->put_char_fn = in_put_char_fn;
    inout_module_cfg->nof_stored_chars_in_rx_buffer = 0;
    inout_module_cfg->nof_stored_cmd_bindings = 0;

    // Store the config locally in a static variable
    g_cli_cfg = inout_module_cfg;

    g_cli_cfg->is_initialized = true;

    cli_register(&help_cmd_binding);
    cli_register(&clear_cmd_binding);

    // Clear the screen at startup
    prv_cmd_handler_clear_screen(0, NULL, NULL);

    // Reset the Rx Buffer and print the welcome message
    prv_reset_rx_buffer();
    prv_write_string("CLI was started - enter your commands (or enter "
                     "'help')\n");
    prv_write_string(CLI_PROMPT);

    return;
}

void cli_receive(char in_char)
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(g_cli_cfg->put_char_fn);
        CLI_ASSERT(g_cli_cfg->nof_stored_chars_in_rx_buffer < CLI_MAX_RX_BUFFER_SIZE);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
        CLI_ASSERT(CLI_CANARY == g_cli_cfg->start_canary_word);
        CLI_ASSERT(CLI_CANARY == g_cli_cfg->end_canary_word);
        CLI_ASSERT(CLI_CANARY == g_cli_cfg->mid_canary_word);
    }

    if ('\r' == in_char)
    {
        return;
    }

    if (true == prv_is_rx_buffer_full())
    {
        prv_write_string("Buffer is full");
        return;
    }

    if ('\b' == in_char) // Character delete (backspace)
    {
        if (g_cli_cfg->nof_stored_chars_in_rx_buffer > 0)
        {
            g_cli_cfg->nof_stored_chars_in_rx_buffer--;
            size_t idx = g_cli_cfg->nof_stored_chars_in_rx_buffer;

            // Remove the last character (the one that was deleted)
            g_cli_cfg->rx_char_buffer[idx] = '\0';
        }
    }
    else // add the character to the buffer
    {
        size_t idx = g_cli_cfg->nof_stored_chars_in_rx_buffer;
        g_cli_cfg->rx_char_buffer[idx] = in_char;
        g_cli_cfg->nof_stored_chars_in_rx_buffer++;
    }
    CLI_ASSERT(g_cli_cfg->nof_stored_chars_in_rx_buffer < CLI_MAX_RX_BUFFER_SIZE);
}

void cli_process()
{
    char* array_of_arguments[CLI_MAX_NOF_ARGUMENTS] = {0};
    uint8_t nof_identified_arguments = 0;
    char* next_argument = NULL;

    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->put_char_fn);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
        CLI_ASSERT(CLI_CANARY == g_cli_cfg->start_canary_word);
        CLI_ASSERT(CLI_CANARY == g_cli_cfg->end_canary_word);
        CLI_ASSERT(CLI_CANARY == g_cli_cfg->mid_canary_word);
    }
    { // Do nothing, if these conditions are not met
        if (prv_get_last_recv_char_from_rx_buffer() != '\n' && (false == prv_is_rx_buffer_full()))
        {
            return;
        }
    }

    // Process the Buffer
    for (size_t i = 0; i < g_cli_cfg->nof_stored_chars_in_rx_buffer; i++)
    {
        if (nof_identified_arguments >= CLI_MAX_NOF_ARGUMENTS)
        {
            prv_write_string("Too many arguments \n");
            break;
        }

        char* const current_char = &g_cli_cfg->rx_char_buffer[i];
        if (' ' == *current_char || '\n' == *current_char || (g_cli_cfg->nof_stored_chars_in_rx_buffer - 1) == i)
        {
            *current_char = '\0';
            if (next_argument)
            {
                int idx = nof_identified_arguments;
                array_of_arguments[idx] = next_argument;
                nof_identified_arguments++;
                next_argument = NULL;
            }
        }
        else if (!next_argument)
        {
            next_argument = current_char;
        }
    }

    if (CLI_MAX_RX_BUFFER_SIZE == g_cli_cfg->nof_stored_chars_in_rx_buffer)
    {
        prv_write_char('\n');
    }

    // call the command handler (if available)
    if (nof_identified_arguments >= 1)
    {
        const cli_binding_t* ptCmdBinding = prv_find_cmd(array_of_arguments[0]);
        if (NULL == ptCmdBinding)
        {
            prv_write_cmd_unknown(array_of_arguments[0]);
        }
        else
        {
            ptCmdBinding->cmd_handler_fn(nof_identified_arguments, array_of_arguments, ptCmdBinding->context);
        }
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
        CLI_ASSERT(in_cmd_binding);
        CLI_ASSERT(in_cmd_binding->cmd_name_string);
        CLI_ASSERT(in_cmd_binding->cmd_helper_string);
        CLI_ASSERT(in_cmd_binding->cmd_handler_fn);
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->is_initialized);
    }

    uint8_t bBindingExists = false;
    uint8_t bBindingIsStored = false;

    for (size_t i = 0; i < g_cli_cfg->nof_stored_cmd_bindings; i++)
    {
        const cli_binding_t* ptBinding = &g_cli_cfg->cmd_bindings_buffer[i];
        if (0 == strncmp(ptBinding->cmd_name_string, in_cmd_binding->cmd_name_string, CLI_MAX_CMD_NAME_LENGTH))
        {
            bBindingExists = true;
            break;
        }
    }
    CLI_ASSERT(false == bBindingExists);

    if (g_cli_cfg->nof_stored_cmd_bindings < CLI_MAX_NOF_CALLBACKS)
    {
        //  Deep Copy the binding into the buffer
        size_t idx = g_cli_cfg->nof_stored_cmd_bindings;
        memcpy(&g_cli_cfg->cmd_bindings_buffer[idx], in_cmd_binding, sizeof(cli_binding_t));
        g_cli_cfg->nof_stored_cmd_bindings++;

        // Mark that the binding was stored
        bBindingIsStored = true;
    }

    CLI_ASSERT(true == bBindingIsStored);

    return;
}

void cli_unregister(const char* const in_cmd_name)
{
    {
        // Input Checks - inout_ptCfg
        CLI_ASSERT(in_cmd_name);
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->is_initialized);
    }

    bool bBindingFound = false;

    for (size_t i = 0; i < g_cli_cfg->nof_stored_cmd_bindings; i++)
    {
        cli_binding_t* ptBinding = &g_cli_cfg->cmd_bindings_buffer[i];
        if (0 == strncmp(ptBinding->cmd_name_string, in_cmd_name, CLI_MAX_CMD_NAME_LENGTH))
        {
            bBindingFound = true;
            // Shift all following bindings one position to the left
            for (size_t j = i; j < g_cli_cfg->nof_stored_cmd_bindings - 1; j++)
            {
                memcpy(&g_cli_cfg->cmd_bindings_buffer[j], &g_cli_cfg->cmd_bindings_buffer[j + 1],
                       sizeof(cli_binding_t));
            }
            g_cli_cfg->nof_stored_cmd_bindings--;
            break;
        }
    }
    CLI_ASSERT(true == bBindingFound);

    return;
}

void cli_print(const char* fmt, ...)
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(fmt);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(g_cli_cfg->put_char_fn);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
        CLI_ASSERT(CLI_CANARY == g_cli_cfg->start_canary_word);
        CLI_ASSERT(CLI_CANARY == g_cli_cfg->end_canary_word);
        CLI_ASSERT(CLI_CANARY == g_cli_cfg->mid_canary_word);
    }

    char buffer[128]; // Temporary buffer for formatted string
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    prv_write_string(buffer);
    prv_write_char('\n');
}

/* #############################################################################
 * # static function implementations
 * ###########################################################################*/

static void prv_write_string(const char* in_string)
{
    {
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(in_string);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(g_cli_cfg->put_char_fn);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
    }
    for (const char* current_char = in_string; *current_char != '\0'; current_char++)
    {
        prv_write_char(*current_char);
    }
}

static void prv_write_char(char in_char)
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->put_char_fn);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
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
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->put_char_fn);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
    }
    g_cli_cfg->put_char_fn(in_char);
}

static void prv_write_cli_prompt()
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
    }
    prv_write_string(CLI_PROMPT);
}

static void prv_write_cmd_unknown(const char* const in_cmd_name)
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(in_cmd_name);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(g_cli_cfg->put_char_fn);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
    }
    prv_write_string(CLI_FAIL_PROMPT "Unknown command: ");
    prv_write_string(in_cmd_name);
    prv_write_char('\n');
    prv_write_string("Type 'help' to list all commands\n");
}

static void prv_reset_rx_buffer()
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
    }
    memset(g_cli_cfg->rx_char_buffer, 0, CLI_MAX_RX_BUFFER_SIZE);
    g_cli_cfg->nof_stored_chars_in_rx_buffer = 0;
}

static uint8_t prv_is_rx_buffer_full()
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
    }
    return (g_cli_cfg->nof_stored_chars_in_rx_buffer >= CLI_MAX_RX_BUFFER_SIZE);
}

static char prv_get_last_recv_char_from_rx_buffer(void)
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
    }
    return g_cli_cfg->rx_char_buffer[g_cli_cfg->nof_stored_chars_in_rx_buffer - 1];
}

static const cli_binding_t* prv_find_cmd(const char* const in_cmd_name)
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(in_cmd_name);
        CLI_ASSERT(g_cli_cfg->rx_char_buffer);
        CLI_ASSERT(g_cli_cfg->cmd_bindings_buffer);
        CLI_ASSERT(g_cli_cfg->nof_stored_cmd_bindings > 0);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
    }

    for (size_t idx = 0; idx < g_cli_cfg->nof_stored_cmd_bindings; ++idx)
    {
        const cli_binding_t* cmd_binding = &g_cli_cfg->cmd_bindings_buffer[idx];
        if (0 == strncmp(cmd_binding->cmd_name_string, in_cmd_name, CLI_MAX_CMD_NAME_LENGTH))
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

static int prv_cmd_handler_help(int argc, char* argv[], void* context)
{
    { // Input Checks
        CLI_ASSERT(g_cli_cfg);
        CLI_ASSERT(g_cli_cfg->put_char_fn);
        CLI_ASSERT(true == g_cli_cfg->is_initialized);
    }
    prv_write_string("\n");

    // Create a list of all registered commands
    for (size_t i = 0; i < g_cli_cfg->nof_stored_cmd_bindings; ++i)
    {
        const cli_binding_t* ptCmdBinding = &g_cli_cfg->cmd_bindings_buffer[i];
        prv_write_string("* ");
        prv_write_string(ptCmdBinding->cmd_name_string);
        prv_write_string(": \n              ");
        prv_write_string(ptCmdBinding->cmd_helper_string);
        prv_write_char('\n');
    }

    (void)argc;
    (void)argv;
    (void)context;

    return CLI_OK_STATUS;
}

static void prv_assert_fail(const char* const in_msg)
{
    // If the CLI is not initialized, we cannot print the assert message
    // Therefore we only enter an infinite loop
    if (NULL != g_cli_cfg)
    {
        if (NULL != in_msg)
        {
            // ANSI-Color Code for Red: \033[31m ... \033[0m
            prv_write_string("\033[31m[CLI ASSERT FAIL]\033[0m ");
            prv_write_string(in_msg);
        }
    }

    while (1)
    {
    }
}
