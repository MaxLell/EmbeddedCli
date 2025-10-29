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

/**
 * @file main.c
 * @brief Example program that initializes the CLI and registers demo commands.
 *
 * This file provides simple command implementations used to exercise the
 * CLI library. 
 */

#include "Cli.h"
#include "custom_assert.h"

#include <stdio.h>

// ###########################################################################
// # Private function decleration
// ###########################################################################
static int prv_cmd_hello_world(int argc, char* argv[], void* context);
static int prv_cmd_echo_string(int argc, char* argv[], void* context);
static int prv_cmd_display_args(int argc, char* argv[], void* context);
static int prv_cmd_dummy(int argc, char* argv[], void* context);

static int prv_console_put_char(char in_char);
static char prv_console_get_char(void);
static void prv_assert_failed(const char* file, uint32_t line, const char* expr);

// ###########################################################################
// # Private Variables
// ###########################################################################
static cli_cfg_t g_cli_cfg = {0};

static cli_binding_t cli_bindings[] = {
    {"hello", prv_cmd_hello_world, NULL, "Say hello"},
    {"args", prv_cmd_display_args, NULL, "Displays the given cli arguments"},
    {"echo", prv_cmd_echo_string, NULL, "Echoes the given string"},
    {"dummy", prv_cmd_dummy, NULL, "dummy stuffens"},
};

// #############################################################################
// # Main
// ###########################################################################

int main(void)
{
    // sets up the assert with its assert_failed function
    custom_assert_init(prv_assert_failed);

    /**
     * Hands over the statically allocated cli_cfg_t struct.
     * The Cli's memory is to be managed by the user. Internally the cli only works with a 
     * reference to this allocated memory. (There is a lot of sanity checking with ASSERTs
     * on the state of this memory).
     */
    cli_init(&g_cli_cfg, prv_console_put_char);

    /**
     * Register all external command bindings - these are the ones listed here in this demo
     * There are some internal command bindings too - like for example the clear, help and reset
     * binding.
     */
    for (size_t i = 0; i < CLI_GET_ARRAY_SIZE(cli_bindings); i++)
    {
        cli_register(&cli_bindings[i]);
    }

    /**
     * remove the "dummy" command from the internally stored cli bindings - so now this binding is no longer available during runtime.
     * You can register and unregister cli bindings at runtime. The memory for that is fixed size. For the case that you are adding too many
     * bindings, the ASSERTs will catch that in the cli.
     */
    cli_unregister("dummy");

    while (1)
    {
        // Get a character entered by the user
        char c = prv_console_get_char();

        // Add the character to a queue for later processing
        cli_receive(c);

        // Process the rx buffer (all entered characters)
        cli_process();
    }
    return 0;
}

// ###########################################################################
// # Private function implementation
// ###########################################################################

// ============================
// = Commands
// ============================

static int prv_cmd_hello_world(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;
    cli_print("Hello World!\n");
    return CLI_OK_STATUS;
}

static int prv_cmd_echo_string(int argc, char* argv[], void* context)
{
    if (argc != 2)
    {
        cli_print("Give one argument\n");
        return CLI_FAIL_STATUS;
    }
    (void)argv;
    (void)context;
    cli_print("-> %s\n", argv[1]);
    return CLI_OK_STATUS;
}

static int prv_cmd_display_args(int argc, char* argv[], void* context)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        cli_print("argv[%d] --> \"%s\" \n", i, argv[i]);
    }

    (void)context;
    return CLI_OK_STATUS;
}

static int prv_cmd_dummy(int argc, char* argv[], void* context)
{
    (void)argc;
    (void)argv;
    (void)context;
    return CLI_OK_STATUS;
}

// ============================
// = Console Setup
// ============================

static int prv_console_put_char(char in_char) { return putchar(in_char); }

static char prv_console_get_char(void) { return (char)getchar(); }

static void prv_assert_failed(const char* file, uint32_t line, const char* expr)
{
    printf("%s(%u): ASSERT failed: %s\n", file, line, expr);
    while (1)
        ;
}