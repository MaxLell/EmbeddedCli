#include "Cli.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void CliBinding_HelloWorld( int argc, char *argv[] )
{
    (void)argc;
    (void)argv;
    printf( "%sHello World!\n", CLI_OK_PROMPT );
    return;
}

void Cli_EchoString( int argc, char *argv[] )
{
    if( argc != 2 )
    {
        printf( "%sGive one argument\n", CLI_FAIL_PROMPT );
        return;
    }
    (void)argv;
    printf( "%s\"%s\"\n", CLI_OK_PROMPT, argv[1] );
    return;
}

void Cli_DisplayArgs( int argc, char *argv[] )
{
    int i;
    for( i = 0; i < argc; i++ )
    {
        printf( "argv[%d] --> \"%s\" \n", i, argv[i] );
    }
    return;
}

void Cli_ClearScreen( int argc, char *argv[] )
{
    (void)argc;
    (void)argv;
    // ANSI escape code to clear screen and move cursor to home
    printf( "\033[2J\033[H" );
    return;
}
