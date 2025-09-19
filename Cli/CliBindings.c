#include "Cli.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE( arr ) ( sizeof( arr ) / sizeof( arr[0] ) )

int CliBinding_HelpHandler( int argc, char *argv[] );

bool kv_store_write( const char *key, const void *val, uint32_t len )
{
    // Stub
    return true;
}

int CliBinding_KvWrite( int argc, char *argv[] )
{
    // We expect 3 arguments:
    // 1. Command name
    // 2. Key
    // 3. Value
    if( argc != 3 )
    {
        Cli_WriteString( CLI_FAIL_PROMPT "Too few arguments given" );
        return -1;
    }

    const char *key = argv[1];
    const char *value = argv[2];

    bool result = kv_store_write( key, value, strlen( value ) );
    if( !result )
    {
        Cli_WriteString( CLI_FAIL_PROMPT );
        return -1;
    }
    Cli_WriteString( CLI_OK_PROMPT );
    return 0;
}

int CliBinding_HelloWorld( int argc, char *argv[] )
{
    Cli_WriteString( CLI_OK_PROMPT "Hello World!" );
    return 0;
}

int Cli_DisplayArgs( int argc, char *argv[] )
{
    int i;
    for( i = 0; i < argc; i++ )
    {
        printf( "argv[%d]=\"%s\"\n", i, argv[i] );
    }
    return 0;
}

int Cli_ClearScreen( int argc, char *argv[] )
{
    // ANSI escape code to clear screen and move cursor to home
    Cli_WriteString( "\033[2J\033[H" );
    return 0;
}
