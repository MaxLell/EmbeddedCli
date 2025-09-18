
#include "Shell.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int Console_WriteCharacter( char in_c )
{
    putchar( in_c );
    return 0;
}

char Console_ReadCharacter( void )
{
    return getchar();
}

int main( void )
{
    Shell_Config_t t_shellImpl = {
        .send_char = Console_WriteCharacter,
    };
    Shell_Init( &t_shellImpl );

    char cInputCharacter = 0;
    while( true )
    {
        cInputCharacter = Console_ReadCharacter();
        Shell_ReadCharacter( cInputCharacter );
    }
    return 0;
}
