#include "Cli.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CLI_MAX_NOF_ARGUMENTS   ( 16 )
#define CLI_PROMPT              "-------- \n> "
#define CLI_MAX_CMD_NAME_LENGTH ( 32U )
#define CLI_CANARY              ( 0xA5A5A5A5U )

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


static Cli_Config_t *g_tCli_Config = NULL;

static void                 Cli_AssertFail( const char *msg );
static void                 Cli_WriteCharacter( char c );
static void                 Cli_EchoCharacter( char c );
static char                 Cli_GetLastEntryFromRxBuffer();
static bool                 Cli_IsRxBufferFull();
static void                 Cli_ResetRxBuffer();
static void                 Cli_EchoString( const char *str );
static void                 Cli_WritePrompt();
static const Cli_Binding_t *Cli_FindCommand( const char *in_pcCommandName );

/**
 * @brief Sends a character via the shell interface.
 * @param c The character to send.
 */
static void Cli_WriteCharacter( char c )
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    g_tCli_Config->pFnWriteCharacter( c );
}


/**
 * @brief Echoes a character to the shell (handles newline and backspace).
 * @param c The character to echo.
 */
static void Cli_EchoCharacter( char c )
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
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
static char Cli_GetLastEntryFromRxBuffer()
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    return g_tCli_Config->acRxByteBuffer[g_tCli_Config->tNofStoredCharacters - 1];
}


/**
 * @brief Checks if the RX buffer is full.
 * @return true if the buffer is full, false otherwise.
 */
static bool Cli_IsRxBufferFull()
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    return ( g_tCli_Config->tNofStoredCharacters >= CLI_MAX_RX_BUFFER_SIZE );
}


/**
 * @brief Resets the receive buffer and clears all received characters.
 */
static void Cli_ResetRxBuffer()
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    memset( g_tCli_Config->acRxByteBuffer, 0, CLI_MAX_RX_BUFFER_SIZE );
    g_tCli_Config->tNofStoredCharacters = 0;
}


/**
 * @brief Echoes a string to the shell.
 * @param str The string to echo.
 */
static void Cli_EchoString( const char *in_pcString )
{
    {
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( in_pcString );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }

    for( const char *pcChar = in_pcString; *pcChar != '\0'; pcChar++ )
    {
        Cli_EchoCharacter( *pcChar );
    }
}


/**
 * @brief Outputs the shell prompt.
 */
static void Cli_WritePrompt()
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    Cli_EchoString( CLI_PROMPT );
}


/**
 * @brief Searches for a shell pcCmdName by its name.
 * @param name Name of the pcCmdName.
 * @return Pointer to the found pcCmdName or NULL.
 */
static const Cli_Binding_t *Cli_FindCommand( const char *in_pcCommandName )
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( in_pcCommandName );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config->atCliCmdBindingsBuffer );
        CLI_ASSERT( g_tCli_Config->tNofBindings > 0 );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }

    for( size_t i = 0; i < g_tCli_Config->tNofBindings; i++ )
    {
        const Cli_Binding_t *ptBinding = &g_tCli_Config->atCliCmdBindingsBuffer[i];
        if( 0 == strcmp( ptBinding->pcCmdName, in_pcCommandName ) )
        {
            return ptBinding;
        }
    }
    return NULL;
}

void Cli_Initialize( Cli_Config_t *in_ptCfg )
{
    { // Input Checks
        CLI_ASSERT( in_ptCfg );
        CLI_ASSERT( in_ptCfg->acRxByteBuffer );
        CLI_ASSERT( 0 == in_ptCfg->tNofStoredCharacters );
        CLI_ASSERT( in_ptCfg->atCliCmdBindingsBuffer );
        CLI_ASSERT( in_ptCfg->pFnWriteCharacter );
        CLI_ASSERT( false == in_ptCfg->bIsInitialized );
        CLI_ASSERT( in_ptCfg->tNofBindings > 0 );
    }

    in_ptCfg->u32CfgCanaryStart = CLI_CANARY;
    in_ptCfg->u32CfgCanaryEnd = CLI_CANARY;
    in_ptCfg->u32BufferCanary = CLI_CANARY;
    in_ptCfg->bIsInitialized = true;

    g_tCli_Config = in_ptCfg;

    Cli_ResetRxBuffer();
    Cli_EchoString( "CLI was started - enter your commands\n" );
    Cli_EchoString( CLI_PROMPT );

    CLI_ASSERT( in_ptCfg == g_tCli_Config );
}

void Cli_AddCharacter( Cli_Config_t *ptCfg, char in_cChar )
{
    { // Input Checks
        CLI_ASSERT( ptCfg );
        CLI_ASSERT( true == ptCfg->bIsInitialized );
        CLI_ASSERT( ptCfg->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config == ptCfg );
        CLI_ASSERT( ptCfg->pFnWriteCharacter );
        CLI_ASSERT( CLI_CANARY == ptCfg->u32CfgCanaryStart );
        CLI_ASSERT( CLI_CANARY == ptCfg->u32CfgCanaryEnd );
        CLI_ASSERT( CLI_CANARY == ptCfg->u32BufferCanary );
    }

    if( '\r' == in_cChar )
    {
        return;
    }

    if( true == Cli_IsRxBufferFull() )
    {
        Cli_EchoString( "Buffer is full" );
        return;
    }

    if( '\b' == in_cChar )
    {
        if( ptCfg->tNofStoredCharacters > 0 )
        {
            ptCfg->tNofStoredCharacters--;
            size_t idx = ptCfg->tNofStoredCharacters;
            ptCfg->acRxByteBuffer[idx] = '\0';
        }
        return;
    }

    size_t idx = ptCfg->tNofStoredCharacters;
    ptCfg->acRxByteBuffer[idx] = in_cChar;
    ptCfg->tNofStoredCharacters++;
    CLI_ASSERT( ptCfg->tNofStoredCharacters < CLI_MAX_RX_BUFFER_SIZE );
}

void Cli_HandleUnknownCommand( const char *const in_pcCmdName )
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( in_pcCmdName );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    Cli_EchoString( CLI_FAIL_PROMPT "Unknown command: " );
    Cli_EchoString( in_pcCmdName );
    Cli_EchoCharacter( '\n' );
    Cli_EchoString( "Type 'help' to list all commands\n" );
}

void Cli_Process( Cli_Config_t *ptCfg )
{
    char *acArguments[CLI_MAX_NOF_ARGUMENTS] = { 0 };
    int   s32NofArguments = 0;
    char *pcNextArgument = NULL;

    { // Input Checks
        CLI_ASSERT( ptCfg );
        CLI_ASSERT( ptCfg->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config == ptCfg );
        CLI_ASSERT( ptCfg->pFnWriteCharacter );
        CLI_ASSERT( true == ptCfg->bIsInitialized );
        CLI_ASSERT( CLI_CANARY == ptCfg->u32CfgCanaryStart );
        CLI_ASSERT( CLI_CANARY == ptCfg->u32CfgCanaryEnd );
        CLI_ASSERT( CLI_CANARY == ptCfg->u32BufferCanary );
    }
    { // Do nothing, if these conditions are not met
        if( Cli_GetLastEntryFromRxBuffer() != '\n' &&
            ( false == Cli_IsRxBufferFull() ) )
        {
            return;
        }
    }

    // Process the Buffer
    for( size_t i = 0; i < ptCfg->tNofStoredCharacters; i++ )
    {
        if( s32NofArguments >= CLI_MAX_NOF_ARGUMENTS )
        {
            Cli_EchoString( "Too many arguments \n" );
            break;
        }

        char *const c = &ptCfg->acRxByteBuffer[i];
        if( ' ' == *c || '\n' == *c || ( ptCfg->tNofStoredCharacters - 1 ) == i )
        {
            *c = '\0';
            if( pcNextArgument )
            {
                int idx = s32NofArguments;
                acArguments[idx] = pcNextArgument;
                s32NofArguments++;
                pcNextArgument = NULL;
            }
        }
        else if( !pcNextArgument )
        {
            pcNextArgument = c;
        }
    }

    if( CLI_MAX_RX_BUFFER_SIZE == ptCfg->tNofStoredCharacters )
    {
        Cli_EchoCharacter( '\n' );
    }

    if( s32NofArguments >= 1 )
    {
        const Cli_Binding_t *ptCmdBinding = Cli_FindCommand( acArguments[0] );
        if( !ptCmdBinding )
        {
            Cli_HandleUnknownCommand( acArguments[0] );
        }
        else
        {
            ptCmdBinding->pFnCmdHandler( s32NofArguments, acArguments );
        }
    }
    Cli_ResetRxBuffer( ptCfg );
    Cli_WritePrompt( ptCfg );
}

void CliBinding_HelpHandler( int argc, char *argv[] )
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config->atCliCmdBindingsBuffer );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( g_tCli_Config->tNofBindings > 0 );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
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

    (void)argc;
    (void)argv;

    return;
}

void Cli_AssertFail( const char *msg )
{
    // If the CLI is not initialized, we cannot print the assert message
    // Therefore we only enter an infinite loop
    if( NULL != g_tCli_Config || NULL != msg )
    {
        // ANSI-Color Code for Red: \033[31m ... \033[0m
        Cli_EchoString( "\033[31m[CLI ASSERT FAIL]\033[0m " );
        Cli_EchoString( msg );
    }
    while( 1 )
    {
    }
}

void Cli_Print( const char *fmt, ... )
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( fmt );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32CfgCanaryStart );
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32CfgCanaryEnd );
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32BufferCanary );
    }

    char    buffer[128]; // Temporary buffer for formatted string
    va_list args;
    va_start( args, fmt );
    vsnprintf( buffer, sizeof( buffer ), fmt, args );
    va_end( args );

    Cli_EchoString( buffer );
    Cli_EchoCharacter( '\n' );
}