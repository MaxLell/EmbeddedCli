/**
 * @file main.c
 * @brief Example program that initializes the CLI and registers demo commands.
 *
 * This file provides simple command implementations used to exercise the
 * CLI library. It also contains the console input/output glue used by the
 * example.
 */

#include "Cli.h"

#include <stdio.h>

// #############################################################################
// # Command Implementations - just for demonstration
// ###########################################################################

/** Demo command handlers used in the example program. */
int Cli_HelloWorld( int argc, char *argv[], void *context );
int Cli_EchoString( int argc, char *argv[], void *context );
int Cli_DisplayArgs( int argc, char *argv[], void *context );
int Cli_ClearScreen( int argc, char *argv[], void *context );

int Cli_HelloWorld( int argc, char *argv[], void *context )
{
    (void)argc;
    (void)argv;
    (void)context;
    Cli_Print( "%sHello World!\n", CLI_OK_PROMPT );
    return CLI_OK_STATUS;
}

int Cli_EchoString( int argc, char *argv[], void *context )
{
    if( argc != 2 )
    {
        Cli_Print( "%sGive one argument\n", CLI_FAIL_PROMPT );
        return CLI_FAIL_STATUS;
    }
    (void)argv;
    (void)context;
    Cli_Print( "%s\"%s\"\n", CLI_OK_PROMPT, argv[1] );
    return CLI_OK_STATUS;
}

int Cli_DisplayArgs( int argc, char *argv[], void *context )
{
    int i;
    for( i = 0; i < argc; i++ )
    {
        Cli_Print( "argv[%d] --> \"%s\" \n", i, argv[i] );
    }

    (void)context;
    return CLI_OK_STATUS;
}


static Cli_Binding_t atCliBindings[] = {
    { "hello", Cli_HelloWorld, NULL, "Say hello" },
    { "display_args", Cli_DisplayArgs, NULL, "Displays the given cli arguments" },
    { "echo", Cli_EchoString, NULL, "Echoes the given string" },
};

// #############################################################################
// # Setup Console I/O
// ###########################################################################

int Console_PutCharacter( char c )
{
    return putchar( c );
}

char Console_GetCharacter( void )
{
    return (char)getchar();
}

// #############################################################################
// # Main
// ###########################################################################

static Cli_Config_t tCliCfg = { 0 };

int main( void )
{
    Cli_Init( &tCliCfg, Console_PutCharacter );

    for( size_t i = 0; i < CLI_GET_ARRAY_SIZE( atCliBindings ); i++ )
    {
        Cli_Register( &atCliBindings[i] );
    }

    Cli_Unregister( "echo" );

    while( 1 )
    {
        char c = Console_GetCharacter();
        Cli_Receive( c );
        Cli_Process();
    }
    return 0;
}
