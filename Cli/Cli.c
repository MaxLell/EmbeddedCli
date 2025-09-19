#include "Cli.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define CLI_MAX_NOF_ARGUMENTS         ( 16 )
#define CLI_PROMPT                    "-------- \n> "
#define CLI_NEWLINE_CHARACTER         '\n'
#define CLI_CARRIAGE_RETURN_CHARACTER '\r'
#define CLI_BACKSPACE_CHARACTER       '\b'
#define CLI_NULL_TERMINATOR_CHARACTER '\0'
#define CLI_SPACE_CHARACTER           ' '


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
    if( CLI_NEWLINE_CHARACTER == c )
    {
        Cli_WriteCharacter( CLI_CARRIAGE_RETURN_CHARACTER );
        Cli_WriteCharacter( CLI_NEWLINE_CHARACTER );
    }
    else if( CLI_BACKSPACE_CHARACTER == c )
    {
        Cli_WriteCharacter( CLI_BACKSPACE_CHARACTER );
        Cli_WriteCharacter( CLI_SPACE_CHARACTER );
        Cli_WriteCharacter( CLI_BACKSPACE_CHARACTER );
    }
    else
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
    return g_tCli_Config->acRxBuffer[g_tCli_Config->tRxBufferSize - 1];
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
    memset( g_tCli_Config->acRxBuffer, 0, CLI_RX_BUFFER_SIZE );
    g_tCli_Config->tRxBufferSize = 0;
}


/**
 * @brief Echoes a string to the shell.
 * @param str The string to echo.
 */
static void Cli_EchoString( const char *str )
{
    assert( Cli_IsInitialized() );
    for( const char *c = str; *c != CLI_NULL_TERMINATOR_CHARACTER; c++ )
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
 * @brief Searches for a shell command by its name.
 * @param name Name of the command.
 * @return Pointer to the found command or NULL.
 */
static const Cli_Binding_t *Cli_FindCommand( const char *pcCommandName )
{
    assert( Cli_IsInitialized() );
    for( const Cli_Binding_t *command = g_tCli_Config->atBindings;
         command < &g_tCli_Config->atBindings[g_tCli_Config->tNofBindings];
         command++ )
    {
        if( strcmp( command->command, pcCommandName ) == 0 )
        {
            return command;
        }
    }
    return NULL;
}


void Cli_ProcessRxBuffer( void )
{
    assert( Cli_IsInitialized() );

    if( Cli_GetLastEntryFromRxBuffer() != CLI_NEWLINE_CHARACTER &&
        !Cli_IsRxBufferFull() )
    {
        return;
    }

    char *argv[CLI_MAX_NOF_ARGUMENTS] = { 0 };
    int   argc = 0;

    char *next_arg = NULL;
    for( size_t i = 0;
         i < g_tCli_Config->tRxBufferSize && argc < CLI_MAX_NOF_ARGUMENTS; ++i )
    {
        char *const c = &g_tCli_Config->acRxBuffer[i];
        if( CLI_SPACE_CHARACTER == *c || CLI_NEWLINE_CHARACTER == *c ||
            ( g_tCli_Config->tRxBufferSize - 1 ) == i )
        {
            *c = CLI_NULL_TERMINATOR_CHARACTER;
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
        Cli_EchoCharacter( CLI_NEWLINE_CHARACTER );
    }

    if( argc >= 1 )
    {
        const Cli_Binding_t *ptCmdBinding = Cli_FindCommand( argv[0] );
        if( !ptCmdBinding )
        {
            Cli_EchoString( "Unknown command: " );
            Cli_EchoString( argv[0] );
            Cli_EchoCharacter( CLI_NEWLINE_CHARACTER );
            Cli_EchoString( "Type 'help' to list all commands\n" );
        }
        else
        {
            ptCmdBinding->handler( argc, argv );
        }
    }
    Cli_ResetRxBuffer();
    Cli_WritePrompt();
}

void Cli_Initialize( Cli_Config_t *in_ptCfg )
{
    { // Input Checks
        assert( in_ptCfg );
        assert( in_ptCfg->acRxBuffer );
        assert( in_ptCfg->atBindings );
        assert( in_ptCfg->pFnWriteCharacter );
        assert( in_ptCfg->tNofBindings > 0 );
    }

    g_tCli_Config = in_ptCfg;
    g_tCli_Config->bIsInitialized = true;

    Cli_ResetRxBuffer();
    Cli_WriteString( "CLI was started - enter your commands" );
    Cli_EchoString( CLI_PROMPT );
}

void Cli_AddCharToRxBuffer( char c )
{
    assert( Cli_IsInitialized() );
    if( c == CLI_CARRIAGE_RETURN_CHARACTER || Cli_IsRxBufferFull() ||
        !Cli_IsInitialized() )
    {
        return;
    }
    // Cli_EchoCharacter( c );

    if( c == CLI_BACKSPACE_CHARACTER )
    {
        if( g_tCli_Config->tRxBufferSize > 0 )
        {
            g_tCli_Config->acRxBuffer[--g_tCli_Config->tRxBufferSize] =
                CLI_NULL_TERMINATOR_CHARACTER;
        }
        return;
    }

    g_tCli_Config->acRxBuffer[g_tCli_Config->tRxBufferSize++] = c;
}

void Cli_WriteString( const char *str )
{
    assert( Cli_IsInitialized() );
    Cli_EchoString( str );
    Cli_EchoCharacter( CLI_NEWLINE_CHARACTER );
}

int CliBinding_HelpHandler( int argc, char *argv[] )
{
    assert( Cli_IsInitialized() );
    for( const Cli_Binding_t *command = g_tCli_Config->atBindings;
         command < &g_tCli_Config->atBindings[g_tCli_Config->tNofBindings];
         command++ )
    {
        Cli_EchoString( "* " );
        Cli_EchoString( command->command );
        Cli_EchoString( ": \n              " );
        Cli_EchoString( command->help );
        Cli_EchoCharacter( CLI_NEWLINE_CHARACTER );
    }
    return 0;
}
