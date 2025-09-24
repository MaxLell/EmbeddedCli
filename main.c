#include "Cli.h"

#include <stdio.h>

extern int Cli_HelloWorld( int argc, char *argv[] );
extern int Cli_DisplayArgs( int argc, char *argv[] );
extern int Cli_ClearScreen( int argc, char *argv[] );
extern int Cli_EchoString( int argc, char *argv[] );
extern int Cli_HelpCommand( int argc, char *argv[] );

int Console_PutCharacter( char c )
{
    return putchar( c );
}

char Console_GetCharacter( void )
{
    return (char)getchar();
}

static Cli_Config_t  tCliCfg = { 0 };
static Cli_Binding_t atCliBindings[] = {
    { "hello", Cli_HelloWorld, "Say hello" },
    { "help", Cli_HelpCommand, "Lists all commands" },
    { "display_args", Cli_DisplayArgs, "Displays the given cli arguments" },
    { "clear", Cli_ClearScreen, "Clears the screen" },
    { "echo", Cli_EchoString, "Echoes the given string" },
};

int main( void )
{


    Cli_Init( &tCliCfg, atCliBindings, CLI_GET_ARRAY_SIZE( atCliBindings ),
              Console_PutCharacter );

    while( 1 )
    {
        char c = Console_GetCharacter();
        Cli_AddCharacter( &tCliCfg, c );
        Cli_ProcessBuffer( &tCliCfg );
    }
    return 0;
}
