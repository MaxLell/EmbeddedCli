#include "Cli.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE( arr ) ( sizeof( arr ) / sizeof( arr[0] ) )

extern int CliBinding_KvWrite( int argc, char *argv[] );
extern int CliBinding_HelloWorld( int argc, char *argv[] );
extern int CliBinding_HelpHandler( int argc, char *argv[] );

int console_putc( char c )
{
    return putchar( c );
}

char console_getc( void )
{
    return (char)getchar();
}

int main( void )
{
    static char acRxBuffer[CLI_RX_BUFFER_SIZE] = { 0 };

    Cli_Binding_t atCliBindings[] = {
        { "kv_write", CliBinding_KvWrite, "Write a Key/Value pair" },
        { "hello", CliBinding_HelloWorld, "Say hello" },
        { "help", CliBinding_HelpHandler, "Lists all commands" },
    };

    Cli_Config_t tCliCfg = { .pFnWriteCharacter = console_putc,
                             .bIsInitialized = false,
                             .acRxBuffer = acRxBuffer,
                             .tRxBufferSize = CLI_RX_BUFFER_SIZE,
                             .atBindings = atCliBindings,
                             .tNofBindings = ARRAY_SIZE( atCliBindings ) };

    Cli_Initialize( &tCliCfg );

    while( true )
    {
        char c = console_getc();
        Cli_AddCharToRxBuffer( c );
        Cli_ProcessRxBuffer();
    }
    return 0;
}
