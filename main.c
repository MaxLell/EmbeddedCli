#include "Cli.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE( arr ) ( sizeof( arr ) / sizeof( arr[0] ) )

extern void CliBinding_HelloWorld( int argc, char *argv[] );
extern void Cli_DisplayArgs( int argc, char *argv[] );
extern void Cli_ClearScreen( int argc, char *argv[] );
extern void Cli_EchoString( int argc, char *argv[] );
extern void CliBinding_HelpHandler( int argc, char *argv[] );

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
    static char acRxByteBuffer[CLI_MAX_RX_BUFFER_SIZE] = { 0 };

    Cli_Binding_t atCliBindings[] = {
        { "hello", CliBinding_HelloWorld, "Say hello" },
        { "help", CliBinding_HelpHandler, "Lists all commands" },
        { "display_args", Cli_DisplayArgs, "Displays the given cli arguments" },
        { "clear", Cli_ClearScreen, "Clears the screen" },
        { "echo", Cli_EchoString, "Echoes the given string" },
    };

    Cli_Config_t tCliCfg = { .pFnWriteCharacter = console_putc,
                             .bIsInitialized = false,
                             .acRxByteBuffer = acRxByteBuffer,
                             .tCurrentRxBufferSize = 0,
                             .atCliCmdBindingsBuffer = atCliBindings,
                             .tNofBindings = ARRAY_SIZE( atCliBindings ) };

    Cli_Initialize( &tCliCfg );

    while( true )
    {
        char c = console_getc();
        Cli_AddCharacter( &tCliCfg, c );
        Cli_Process( &tCliCfg );
    }
    return 0;
}
