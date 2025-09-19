#include "Cli.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define CLI_MAX_NOF_ARGUMENTS ( 16 )
#define CLI_PROMPT            "-------- \n> "

static Cli_Config_t *g_tCli_Config = NULL;

/**
 * @brief Checks if the shell is initialized.
 * @return true if pFnWriteCharacter is set, false otherwise.
 */
static bool Cli_IsInitialized( void )
{
    return g_tCli_Config->pFnWriteCharacter != NULL;
}

/**
 * @brief Sends a character via the shell interface.
 * @param c The character to send.
 */
static void Cli_WriteCharacter( char c )
{
    assert( Cli_IsInitialized() );

    g_tCli_Config->pFnWriteCharacter( c );
}


/**
 * @brief Echoes a character to the shell (handles newline and backspace).
 * @param c The character to echo.
 */
static void Cli_EchoCharacter( char c )
{
    assert( Cli_IsInitialized() );
    if( '\n' == c ) // User pressed Enter
    {
        Cli_WriteCharacter( '\r' );
        Cli_WriteCharacter( '\n' );
    }
    else if( '\b' == c ) // User pressed Backspace
    {
        Cli_WriteCharacter( '\b' );
        Cli_WriteCharacter( ' ' );
        Cli_WriteCharacter( '\b' );
    }
    else // Every other character
    {
        Cli_WriteCharacter( c );
    }
}


/**
 * @brief Returns the last character of the Rx Buffer
 * @return The last character in the receive buffer.
 */
static char Cli_GetLastEntryFromRxBuffer( void )
{
    assert( Cli_IsInitialized() );
    return g_tCli_Config->acRxByteBuffer[g_tCli_Config->tRxBufferSize - 1];
}


/**
 * @brief Checks if the RX buffer is full.
 * @return true if the buffer is full, false otherwise.
 */
static bool Cli_IsRxBufferFull( void )
{
    assert( Cli_IsInitialized() );
    return ( g_tCli_Config->tRxBufferSize >= CLI_RX_BUFFER_SIZE );
}


/**
 * @brief Resets the receive buffer and clears all received characters.
 */
static void Cli_ResetRxBuffer( void )
{
    assert( Cli_IsInitialized() );
    memset( g_tCli_Config->acRxByteBuffer, 0, CLI_RX_BUFFER_SIZE );
    g_tCli_Config->tRxBufferSize = 0;
}


/**
 * @brief Echoes a string to the shell.
 * @param str The string to echo.
 */
static void Cli_EchoString( const char *str )
{
    assert( Cli_IsInitialized() );
    for( const char *c = str; *c != '\0'; c++ )
    {
        Cli_EchoCharacter( *c );
    }
}


/**
 * @brief Outputs the shell prompt.
 */
static void Cli_WritePrompt( void )
{
    assert( Cli_IsInitialized() );
    Cli_EchoString( CLI_PROMPT );
}


/**
 * @brief Searches for a shell pcCmdName by its name.
 * @param name Name of the pcCmdName.
 * @return Pointer to the found pcCmdName or NULL.
 */
static const Cli_Binding_t *Cli_FindCommand( const char *pcCommandName )
{
    assert( Cli_IsInitialized() );
    for( size_t i = 0; i < g_tCli_Config->tNofBindings; ++i )
    {
        const Cli_Binding_t *ptBinding = &g_tCli_Config->atCliCmdBindingsBuffer[i];
        if( strcmp( ptBinding->pcCmdName, pcCommandName ) == 0 )
        {
            return ptBinding;
        }
    }
    return NULL;
}

void Cli_Initialize( Cli_Config_t *in_ptCfg )
{
    { // Input Checks
        assert( in_ptCfg );
        assert( in_ptCfg->acRxByteBuffer );
        assert( in_ptCfg->atCliCmdBindingsBuffer );
        assert( in_ptCfg->pFnWriteCharacter );
        assert( in_ptCfg->tNofBindings > 0 );
    }

    g_tCli_Config = in_ptCfg;
    g_tCli_Config->bIsInitialized = true;

    Cli_ResetRxBuffer();
    Cli_EchoString( "CLI was started - enter your commands\n" );
    Cli_EchoString( CLI_PROMPT );
}

void Cli_AddCharacter( char in_cChar )
{
    assert( Cli_IsInitialized() );
    if( in_cChar == '\r' || Cli_IsRxBufferFull() || !Cli_IsInitialized() )
    {
        return;
    }
    // Cli_EchoCharacter( c );

    if( in_cChar == '\b' )
    {
        if( g_tCli_Config->tRxBufferSize > 0 )
        {
            g_tCli_Config->acRxByteBuffer[--g_tCli_Config->tRxBufferSize] = '\0';
        }
        return;
    }

    g_tCli_Config->acRxByteBuffer[g_tCli_Config->tRxBufferSize++] = in_cChar;
}

void Cli_Process( void )
{
    assert( Cli_IsInitialized() );

    if( Cli_GetLastEntryFromRxBuffer() != '\n' && !Cli_IsRxBufferFull() )
    {
        return;
    }

    char *argv[CLI_MAX_NOF_ARGUMENTS] = { 0 };
    int   argc = 0;

    char *next_arg = NULL;
    for( size_t i = 0;
         i < g_tCli_Config->tRxBufferSize && argc < CLI_MAX_NOF_ARGUMENTS; ++i )
    {
        char *const c = &g_tCli_Config->acRxByteBuffer[i];
        if( ' ' == *c || '\n' == *c || ( g_tCli_Config->tRxBufferSize - 1 ) == i )
        {
            *c = '\0';
            if( next_arg )
            {
                argv[argc++] = next_arg;
                next_arg = NULL;
            }
        }
        else if( !next_arg )
        {
            next_arg = c;
        }
    }

    if( g_tCli_Config->tRxBufferSize == CLI_RX_BUFFER_SIZE )
    {
        Cli_EchoCharacter( '\n' );
    }

    if( argc >= 1 )
    {
        const Cli_Binding_t *ptCmdBinding = Cli_FindCommand( argv[0] );
        if( !ptCmdBinding )
        {
            Cli_EchoString( CLI_FAIL_PROMPT "Unknown command: " );
            Cli_EchoString( argv[0] );
            Cli_EchoCharacter( '\n' );
            Cli_EchoString( "Type 'help' to list all commands\n" );
        }
        else
        {
            ptCmdBinding->pFnCmdHandler( argc, argv );
        }
    }
    Cli_ResetRxBuffer();
    Cli_WritePrompt();
}

void Cli_WriteString( const char *str )
{
    assert( Cli_IsInitialized() );
    Cli_EchoString( str );
    Cli_EchoCharacter( '\n' );
}

int CliBinding_HelpHandler( int argc, char *argv[] )
{
    assert( Cli_IsInitialized() );

    Cli_EchoString( "\n" );
    for( size_t i = 0; i < g_tCli_Config->tNofBindings; ++i )
    {
        const Cli_Binding_t *ptCmdBinding =
            &g_tCli_Config->atCliCmdBindingsBuffer[i];
        Cli_EchoString( "* " );
        Cli_EchoString( ptCmdBinding->pcCmdName );
        Cli_EchoString( ": \n              " );
        Cli_EchoString( ptCmdBinding->pcHelperString );
        Cli_EchoCharacter( '\n' );
    }
    return 0;
}
