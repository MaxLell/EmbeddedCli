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
    Cli_Config_t tCliCfg = {
        .send_char = console_putc,
    };
    Cli_Initialize( &tCliCfg );

    char c;
    while( true )
    {
        c = console_getc();
        Cli_ReadAndProcessCharacter( c );
    }
    return 0;
}
