#include "Cli.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


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

    Cli_Config_t tCliCfg = { .pFnWriteCharacter = console_putc,
                             .bIsInitialized = false,
                             .acRxBuffer = acRxBuffer,
                             .tRxBufferSize = CLI_RX_BUFFER_SIZE };

    Cli_Initialize( &tCliCfg );

    char c;
    while( true )
    {
        c = console_getc();
        Cli_ReadAndProcessCharacter( c );
    }
    return 0;
}
