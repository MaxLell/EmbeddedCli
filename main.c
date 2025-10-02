/**
 * @file main.c
 * @brief Example program that initializes the CLI and registers demo commands.
 *
 * This file provides simple command implementations used to exercise the
 * CLI library. It also contains the console input/output glue used by the
 * example.
 */

#include "Cli.h"
#include "custom_assert.h"

#include <stdio.h>

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
    return CLI_OK_STATUS;
}

static cli_binding_t cli_bindings[] = {
    {"hello", cmd_hello_world, NULL, "Say hello"},
    {"args", cmd_display_args, NULL, "Displays the given cli arguments"},
    {"echo", cmd_echo_string, NULL, "Echoes the given string"},
    {"dummy", cmd_dummy, NULL, "dummy stuffens"},
};

// #############################################################################
// # Setup Console I/O
// ###########################################################################

int console_put_char(char in_char) { return putchar(in_char); }

char console_get_char(void) { return (char)getchar(); }

void assert_failed(const char* file, uint32_t line, const char* expr)
{
    printf("%s(%u): ASSERT failed: %s\n", file, line, expr);
    while (1)
        ;
}

// #############################################################################
// # Main
// ###########################################################################

static cli_cfg_t g_cli_cfg = {0};

int main(void)
{
    // sets up the assert with its assert_failed function
    custom_assert_init(assert_failed);

    cli_init(&g_cli_cfg, console_put_char);

    for (size_t i = 0; i < CLI_GET_ARRAY_SIZE(cli_bindings); i++)
    {
        cli_register(&cli_bindings[i]);
    }

    // remove the "dummy" command from the internally stored cli bindings
    cli_unregister("dummy");

    while (1)
    {
        char c = console_get_char();
        cli_receive(c);
        cli_process();
    }
    return 0;
}
