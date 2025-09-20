/* #############################################################################
 * # Includes
 * ###########################################################################*/

#include "Cli.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* #############################################################################
 * # Defines
 * ###########################################################################*/

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

/* #############################################################################
 * # static variables
 * ###########################################################################*/
static Cli_Config_t *g_tCli_Config = NULL;

/* #############################################################################
 * # static function prototypes
 * ###########################################################################*/

static void Cli_AssertFail( const char *msg );
static void Cli_WriteCharacter( char c );
static void Cli_EchoCharacter( char c );
static char Cli_GetLastEntryFromRxBuffer();
static bool Cli_IsRxBufferFull();
static void Cli_ResetRxBuffer();
static void Cli_EchoString( const char *str );
static void Cli_WritePrompt();
static void Cli_HandleUnknownCommand( const char *const in_pcCmdName );
static const Cli_Binding_t *Cli_FindCommand( const char *const in_pcCommandName );

/* #############################################################################
 * # global function implementations
 * ###########################################################################*/

void Cli_Initialize( Cli_Config_t *const inout_ptCfg )
{
    { // Input Checks
        CLI_ASSERT( inout_ptCfg );
        CLI_ASSERT( inout_ptCfg->acRxByteBuffer );
        CLI_ASSERT( 0 == inout_ptCfg->tNofStoredCharacters );
        CLI_ASSERT( inout_ptCfg->atCliCmdBindingsBuffer );
        CLI_ASSERT( inout_ptCfg->pFnWriteCharacter );
        CLI_ASSERT( false == inout_ptCfg->bIsInitialized );
        CLI_ASSERT( inout_ptCfg->tNofBindings > 0 );
    }

    inout_ptCfg->u32CfgCanaryStart = CLI_CANARY;
    inout_ptCfg->u32CfgCanaryEnd = CLI_CANARY;
    inout_ptCfg->u32BufferCanary = CLI_CANARY;
    inout_ptCfg->bIsInitialized = true;

    g_tCli_Config = inout_ptCfg;

    Cli_ResetRxBuffer();
    Cli_EchoString( "CLI was started - enter your commands\n" );
    Cli_EchoString( CLI_PROMPT );

    CLI_ASSERT( inout_ptCfg == g_tCli_Config );
}

void Cli_AddCharacter( Cli_Config_t *const inout_ptCfg, char in_cChar )
{
    { // Input Checks
        CLI_ASSERT( inout_ptCfg );
        CLI_ASSERT( true == inout_ptCfg->bIsInitialized );
        CLI_ASSERT( inout_ptCfg->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config == inout_ptCfg );
        CLI_ASSERT( inout_ptCfg->pFnWriteCharacter );
        CLI_ASSERT( CLI_CANARY == inout_ptCfg->u32CfgCanaryStart );
        CLI_ASSERT( CLI_CANARY == inout_ptCfg->u32CfgCanaryEnd );
        CLI_ASSERT( CLI_CANARY == inout_ptCfg->u32BufferCanary );
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
        if( inout_ptCfg->tNofStoredCharacters > 0 )
        {
            inout_ptCfg->tNofStoredCharacters--;
            size_t idx = inout_ptCfg->tNofStoredCharacters;
            inout_ptCfg->acRxByteBuffer[idx] = '\0';
        }
        return;
    }

    size_t idx = inout_ptCfg->tNofStoredCharacters;
    inout_ptCfg->acRxByteBuffer[idx] = in_cChar;
    inout_ptCfg->tNofStoredCharacters++;
    CLI_ASSERT( inout_ptCfg->tNofStoredCharacters < CLI_MAX_RX_BUFFER_SIZE );
}

void Cli_Process( Cli_Config_t *const inout_ptCfg )
{
    char *acArguments[CLI_MAX_NOF_ARGUMENTS] = { 0 };
    int   s32NofArguments = 0;
    char *pcNextArgument = NULL;

    { // Input Checks
        CLI_ASSERT( inout_ptCfg );
        CLI_ASSERT( inout_ptCfg->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config == inout_ptCfg );
        CLI_ASSERT( inout_ptCfg->pFnWriteCharacter );
        CLI_ASSERT( true == inout_ptCfg->bIsInitialized );
        CLI_ASSERT( CLI_CANARY == inout_ptCfg->u32CfgCanaryStart );
        CLI_ASSERT( CLI_CANARY == inout_ptCfg->u32CfgCanaryEnd );
        CLI_ASSERT( CLI_CANARY == inout_ptCfg->u32BufferCanary );
    }
    { // Do nothing, if these conditions are not met
        if( Cli_GetLastEntryFromRxBuffer() != '\n' &&
            ( false == Cli_IsRxBufferFull() ) )
        {
            return;
        }
    }

    // Process the Buffer
    for( size_t i = 0; i < inout_ptCfg->tNofStoredCharacters; i++ )
    {
        if( s32NofArguments >= CLI_MAX_NOF_ARGUMENTS )
        {
            Cli_EchoString( "Too many arguments \n" );
            break;
        }

        char *const c = &inout_ptCfg->acRxByteBuffer[i];
        if( ' ' == *c || '\n' == *c ||
            ( inout_ptCfg->tNofStoredCharacters - 1 ) == i )
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

    if( CLI_MAX_RX_BUFFER_SIZE == inout_ptCfg->tNofStoredCharacters )
    {
        Cli_EchoCharacter( '\n' );
    }

    // call the command handler (if available)
    if( s32NofArguments >= 1 )
    {
        const Cli_Binding_t *ptCmdBinding = Cli_FindCommand( acArguments[0] );
        if( NULL == ptCmdBinding )
        {
            Cli_HandleUnknownCommand( acArguments[0] );
        }
        else
        {
            ptCmdBinding->pFnCmdHandler( s32NofArguments, acArguments );
        }
    }

    // Reset the cli buffer and write the prompt again for a new user input
    Cli_ResetRxBuffer( inout_ptCfg );
    Cli_WritePrompt( inout_ptCfg );
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

void Cli_HelpCommand( int argc, char *argv[] )
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

/* #############################################################################
 * # static function implementations
 * ###########################################################################*/

static void Cli_WriteCharacter( char c )
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    g_tCli_Config->pFnWriteCharacter( c );
}

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

static char Cli_GetLastEntryFromRxBuffer()
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    return g_tCli_Config->acRxByteBuffer[g_tCli_Config->tNofStoredCharacters - 1];
}

static bool Cli_IsRxBufferFull()
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    return ( g_tCli_Config->tNofStoredCharacters >= CLI_MAX_RX_BUFFER_SIZE );
}

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

static void Cli_WritePrompt()
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    Cli_EchoString( CLI_PROMPT );
}

static const Cli_Binding_t *Cli_FindCommand( const char *const in_pcCommandName )
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

static void Cli_HandleUnknownCommand( const char *const in_pcCmdName )
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

static void Cli_AssertFail( const char *const msg )
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
