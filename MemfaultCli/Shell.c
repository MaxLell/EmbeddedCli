
#include "Shell.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define SHELL_RX_BUFFER_SIZE ( 256 )
#define SHELL_MAX_ARGS       ( 16 )
#define SHELL_PROMPT         "$> "

#define SHELL_FOR_EACH_COMMAND( command )                                      \
    for( const Shell_Cmd_t *command = g_atShellCmds;                           \
         command < &g_atShellCmds[g_tNofShellCmds]; ++command )

typedef struct
{
    int ( *send_char )( char in_cCharacter );
    size_t rx_size;
    char   rx_buffer[SHELL_RX_BUFFER_SIZE];
} Shell_Context_t;

static Shell_Context_t g_tShell = { 0 };

static bool Shell_IsInitialized( void )
{
    return g_tShell.send_char != NULL;
}

static void Shell_WriteCharacter( char in_cCharacter )
{
    if( !Shell_IsInitialized() )
    {
        return;
    }
    g_tShell.send_char( in_cCharacter );
}

static void Shell_Echo( char in_cCharacter )
{
    if( '\n' == in_cCharacter )
    {
        Shell_WriteCharacter( '\r' );
        Shell_WriteCharacter( '\n' );
    }
    else if( '\b' == in_cCharacter )
    {
        Shell_WriteCharacter( '\b' );
        Shell_WriteCharacter( ' ' );
        Shell_WriteCharacter( '\b' );
    }
    else
    {
        Shell_WriteCharacter( in_cCharacter );
    }
}

static char Shell_GetLastCharacterFromBuffer( void )
{
    return g_tShell.rx_buffer[g_tShell.rx_size - 1];
}

static bool Shell_IsRxBufferFull( void )
{
    return g_tShell.rx_size >= SHELL_RX_BUFFER_SIZE;
}

static void Shell_ResetRxBuffer( void )
{
    memset( g_tShell.rx_buffer, 0, sizeof( g_tShell.rx_buffer ) );
    g_tShell.rx_size = 0;
}

static void Shell_EchoString( const char *in_pcString )
{
    for( const char *in_cCharacter = in_pcString; *in_cCharacter != '\0';
         ++in_cCharacter )
    {
        Shell_Echo( *in_cCharacter );
    }
}

static void Shell_SendPrompt( void )
{
    Shell_EchoString( SHELL_PROMPT );
}

static const Shell_Cmd_t *Shell_FindCommand( const char *name )
{
    SHELL_FOR_EACH_COMMAND( command )
    {
        if( strcmp( command->command, name ) == 0 )
        {
            return command;
        }
    }
    return NULL;
}

static void Shell_Process( void )
{
    if( Shell_GetLastCharacterFromBuffer() != '\n' && !Shell_IsRxBufferFull() )
    {
        return;
    }

    char *acArgumentValues[SHELL_MAX_ARGS] = { 0 };
    int   s32ArgumentCount = 0;

    char *pcNextArgument = NULL;
    for( size_t i = 0;
         i < g_tShell.rx_size && s32ArgumentCount < SHELL_MAX_ARGS; ++i )
    {
        char *const in_cCharacter = &g_tShell.rx_buffer[i];
        if( *in_cCharacter == ' ' || *in_cCharacter == '\n' ||
            i == g_tShell.rx_size - 1 )
        {
            *in_cCharacter = '\0';
            if( pcNextArgument )
            {
                acArgumentValues[s32ArgumentCount++] = pcNextArgument;
                pcNextArgument = NULL;
            }
        }
        else if( !pcNextArgument )
        {
            pcNextArgument = in_cCharacter;
        }
    }

    if( g_tShell.rx_size == SHELL_RX_BUFFER_SIZE )
    {
        Shell_Echo( '\n' );
    }

    if( s32ArgumentCount >= 1 )
    {
        const Shell_Cmd_t *command = Shell_FindCommand( acArgumentValues[0] );
        if( !command )
        {
            Shell_EchoString( "Unknown command: " );
            Shell_EchoString( acArgumentValues[0] );
            Shell_Echo( '\n' );
            Shell_EchoString( "Type 'help' to list all commands\n" );
        }
        else
        {
            command->handler( s32ArgumentCount, acArgumentValues );
        }
    }
    Shell_ResetRxBuffer();
    Shell_SendPrompt();
}

void Shell_Init( const Shell_Config_t *impl )
{
    g_tShell.send_char = impl->send_char;
    Shell_ResetRxBuffer();
    Shell_EchoString( "\n" SHELL_PROMPT );
}

void Shell_ReadCharacter( char in_cCharacter )
{
    if( in_cCharacter == '\r' || Shell_IsRxBufferFull() || !Shell_IsInitialized() )
    {
        return;
    }
    Shell_Echo( in_cCharacter );

    if( in_cCharacter == '\b' )
    {
        if( g_tShell.rx_size > 0 )
        {
            g_tShell.rx_buffer[--g_tShell.rx_size] = '\0';
        }
        return;
    }

    g_tShell.rx_buffer[g_tShell.rx_size++] = in_cCharacter;

    Shell_Process();
}

void Shell_WriteLine( const char *str )
{
    Shell_EchoString( str );
    Shell_Echo( '\n' );
}


int Shell_HelpHandler( int s32ArgumentCount, char *acArgumentValues[] )
{
    SHELL_FOR_EACH_COMMAND( command )
    {
        Shell_EchoString( command->command );
        Shell_EchoString( ": " );
        Shell_EchoString( command->help );
        Shell_Echo( '\n' );
    }
    return 0;
}
