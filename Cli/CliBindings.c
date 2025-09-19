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
        Cli_WriteString( "> FAIL,1" );
        return -1;
    }

    const char *key = argv[1];
    const char *value = argv[2];

    bool result = kv_store_write( key, value, strlen( value ) );
    if( !result )
    {
        Cli_WriteString( "> FAIL,2" );
        return -1;
    }
    Cli_WriteString( "> OK" );
    return 0;
}

int CliBinding_HelloWorld( int argc, char *argv[] )
{
    Cli_WriteString( "Hello World!" );
    return 0;
}
