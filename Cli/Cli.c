#include "Cli.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CLI_MAX_NOF_ARGUMENTS ( 16 )
#define CLI_PROMPT            "-------- \n> "

#define CLI_ASSERT_STRINGIFY2( x ) #x
#define CLI_ASSERT_STRINGIFY( x )  CLI_ASSERT_STRINGIFY2( x )
#define CLI_ASSERT( expr )                                                     \
    do                                                                         \
    {                                                                          \
        if( !( expr ) )                                                        \
        {                                                                      \
            Cli_AssertFail( ": " #expr " "                                     \
                            "(" __FILE__                                       \
                            ":" CLI_ASSERT_STRINGIFY( __LINE__ ) ")\n" );      \
        }                                                                      \
    } while( 0 )


/**
 * This global variable is only to be used:
 * - by the 'help' command function
 * - by the Cli_AssertFail function
 */
static Cli_Config_t *g_tCli_Config = NULL;

static void Cli_AssertFail( const char *msg );
static bool Cli_IsInitialized( Cli_Config_t *ptCfg );
static void Cli_WriteCharacter( Cli_Config_t *ptCfg, char c );
static void Cli_EchoCharacter( Cli_Config_t *ptCfg, char c );
static char Cli_GetLastEntryFromRxBuffer( Cli_Config_t *ptCfg );
static bool Cli_IsRxBufferFull( Cli_Config_t *ptCfg );
static void Cli_ResetRxBuffer( Cli_Config_t *ptCfg );
static void Cli_EchoString( Cli_Config_t *ptCfg, const char *str );
static void Cli_WritePrompt( Cli_Config_t *ptCfg );
static const Cli_Binding_t *Cli_FindCommand( Cli_Config_t *ptCfg,
                                             const char   *pcCommandName );

/**
 * @brief Checks if the shell is initialized.
 * @return true if pFnWriteCharacter is set, false otherwise.
 */
static bool Cli_IsInitialized( Cli_Config_t *ptCfg )
{
    return ptCfg->pFnWriteCharacter != NULL;
}

/**
 * @brief Sends a character via the shell interface.
 * @param c The character to send.
 */
static void Cli_WriteCharacter( Cli_Config_t *ptCfg, char c )
{
    ptCfg->pFnWriteCharacter( c );
}


/**
 * @brief Echoes a character to the shell (handles newline and backspace).
 * @param c The character to echo.
 */
static void Cli_EchoCharacter( Cli_Config_t *ptCfg, char c )
{
    if( '\n' == c ) // User pressed Enter
    {
        Cli_WriteCharacter( ptCfg, '\r' );
        Cli_WriteCharacter( ptCfg, '\n' );
    }
    else if( '\b' == c ) // User pressed Backspace
    {
        Cli_WriteCharacter( ptCfg, '\b' );
        Cli_WriteCharacter( ptCfg, ' ' );
        Cli_WriteCharacter( ptCfg, '\b' );
    }
    else // Every other character
    {
        Cli_WriteCharacter( ptCfg, c );
    }
}


/**
 * @brief Returns the last character of the Rx Buffer
 * @return The last character in the receive buffer.
 */
static char Cli_GetLastEntryFromRxBuffer( Cli_Config_t *ptCfg )
{

    return ptCfg->acRxByteBuffer[g_tCli_Config->tRxBufferSize - 1];
}


/**
 * @brief Checks if the RX buffer is full.
 * @return true if the buffer is full, false otherwise.
 */
static bool Cli_IsRxBufferFull( Cli_Config_t *ptCfg )
{
    return ( ptCfg->tRxBufferSize >= CLI_RX_BUFFER_SIZE );
}


/**
 * @brief Resets the receive buffer and clears all received characters.
 */
static void Cli_ResetRxBuffer( Cli_Config_t *ptCfg )
{
    memset( ptCfg->acRxByteBuffer, 0, CLI_RX_BUFFER_SIZE );
    ptCfg->tRxBufferSize = 0;
}


/**
 * @brief Echoes a string to the shell.
 * @param str The string to echo.
 */
static void Cli_EchoString( Cli_Config_t *ptCfg, const char *str )
{
    for( const char *c = str; *c != '\0'; c++ )
    {
        Cli_EchoCharacter( ptCfg, *c );
    }
}


/**
 * @brief Outputs the shell prompt.
 */
static void Cli_WritePrompt( Cli_Config_t *ptCfg )
{
    Cli_EchoString( ptCfg, CLI_PROMPT );
}


/**
 * @brief Searches for a shell pcCmdName by its name.
 * @param name Name of the pcCmdName.
 * @return Pointer to the found pcCmdName or NULL.
 */
static const Cli_Binding_t *Cli_FindCommand( Cli_Config_t *ptCfg,
                                             const char   *pcCommandName )
{
    for( size_t i = 0; i < ptCfg->tNofBindings; i++ )
    {
        const Cli_Binding_t *ptBinding = &ptCfg->atCliCmdBindingsBuffer[i];
        if( 0 == strcmp( ptBinding->pcCmdName, pcCommandName ) )
        {
            return ptBinding;
        }
    }
    return NULL;
}

void Cli_Initialize( Cli_Config_t *in_ptCfg )
{
    { // Input Checks
        }

    g_tCli_Config = in_ptCfg;
    g_tCli_Config->bIsInitialized = true;

    Cli_ResetRxBuffer( in_ptCfg );
    Cli_EchoString( in_ptCfg, "CLI was started - enter your commands\n" );
    Cli_EchoString( in_ptCfg, CLI_PROMPT );
}

void Cli_AddCharacter( Cli_Config_t *ptCfg, char in_cChar )
{
    if( '\r' == in_cChar || Cli_IsRxBufferFull( ptCfg ) ||
        !Cli_IsInitialized( ptCfg ) )
    {
        return;
    }

    if( '\b' == in_cChar )
    {
        if( ptCfg->tRxBufferSize > 0 )
        {
            ptCfg->tRxBufferSize--;
            size_t idx = ptCfg->tRxBufferSize;
            ptCfg->acRxByteBuffer[idx] = '\0';
        }
        return;
    }

    size_t idx = ptCfg->tRxBufferSize;
    ptCfg->acRxByteBuffer[idx] = in_cChar;
    ptCfg->tRxBufferSize++;
}

void Cli_HandleUnknownCommand( Cli_Config_t     *ptCfg,
                               const char *const in_pcCmdName )
{
    Cli_EchoString( ptCfg, CLI_FAIL_PROMPT "Unknown command: " );
    Cli_EchoString( ptCfg, in_pcCmdName );
    Cli_EchoCharacter( ptCfg, '\n' );
    Cli_EchoString( ptCfg, "Type 'help' to list all commands\n" );
}

void Cli_Process( Cli_Config_t *ptCfg )
{
    if( Cli_GetLastEntryFromRxBuffer( ptCfg ) != '\n' &&
        !Cli_IsRxBufferFull( ptCfg ) )
    {
        return;
    }

    char *acArguments[CLI_MAX_NOF_ARGUMENTS] = { 0 };
    int   s32NofArguments = 0;

    char *next_arg = NULL;
    for( size_t i = 0;
         i < ptCfg->tRxBufferSize && s32NofArguments < CLI_MAX_NOF_ARGUMENTS; ++i )
    {
        char *const c = &ptCfg->acRxByteBuffer[i];
        if( ' ' == *c || '\n' == *c || ( ptCfg->tRxBufferSize - 1 ) == i )
        {
            *c = '\0';
            if( next_arg )
            {
                int idx = s32NofArguments;
                acArguments[idx] = next_arg;
                s32NofArguments++;
                next_arg = NULL;
            }
        }
        else if( !next_arg )
        {
            next_arg = c;
        }
    }

    if( CLI_RX_BUFFER_SIZE == ptCfg->tRxBufferSize )
    {
        Cli_EchoCharacter( ptCfg, '\n' );
    }

    if( s32NofArguments >= 1 )
    {
        const Cli_Binding_t *ptCmdBinding = Cli_FindCommand( ptCfg,
                                                             acArguments[0] );
        if( !ptCmdBinding )
        {
            Cli_HandleUnknownCommand( ptCfg, acArguments[0] );
        }
        else
        {
            ptCmdBinding->pFnCmdHandler( s32NofArguments, acArguments );
        }
    }
    Cli_ResetRxBuffer( ptCfg );
    Cli_WritePrompt( ptCfg );
}

void Cli_WriteString( Cli_Config_t *ptCfg, const char *str )
{
    Cli_EchoString( ptCfg, str );
    Cli_EchoCharacter( ptCfg, '\n' );
}

void CliBinding_HelpHandler( int argc, char *argv[] )
{
    Cli_Config_t *ptCfg = g_tCli_Config;
    Cli_EchoString( ptCfg, "\n" );
    for( size_t i = 0; i < g_tCli_Config->tNofBindings; ++i )
    {
        const Cli_Binding_t *ptCmdBinding =
            &g_tCli_Config->atCliCmdBindingsBuffer[i];
        Cli_EchoString( ptCfg, "* " );
        Cli_EchoString( ptCfg, ptCmdBinding->pcCmdName );
        Cli_EchoString( ptCfg, ": \n              " );
        Cli_EchoString( ptCfg, ptCmdBinding->pcHelperString );
        Cli_EchoCharacter( ptCfg, '\n' );
    }
    return;
}

void Cli_AssertFail( const char *msg )
{
    // ANSI-Farbcode für Rot: \033[31m ... \033[0m
    Cli_EchoString( g_tCli_Config, "\033[31m[CLI ASSERT FAIL]\033[0m " );
    Cli_EchoString( g_tCli_Config, msg );
    while( 1 )
    {
    }
}
