/**
 * @file Cli.c
 * @brief Implementation of a tiny embedded CLI (command dispatcher).
 *
 * This file contains the CLI state machine, command registration and the
 * built-in help command. Internal helper functions are declared static and
 * are not part of the public API.
 */

#include "Cli.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* #############################################################################
 * # Defines
 * ###########################################################################*/

/**
 * @def CLI_MAX_NOF_ARGUMENTS
 * @brief Maximum number of whitespace-separated arguments parsed for a command.
 */
#define CLI_MAX_NOF_ARGUMENTS ( 16 )

/**
 * @def CLI_PROMPT
 * @brief Prompt string printed before each input line.
 */
#define CLI_PROMPT "-------- \n> "

/**
 * @def CLI_CANARY
 * @brief Magic canary value used to detect memory corruption of the
 *        configuration structure.
 */
#define CLI_CANARY ( 0xA5A5A5A5U )

/**
 * @def CLI_ASSERT_STRINGIFY2 / CLI_ASSERT_STRINGIFY
 * @brief Helpers to stringify line numbers in the CLI_ASSERT macro.
 */
#define CLI_ASSERT_STRINGIFY2( x ) #x
#define CLI_ASSERT_STRINGIFY( x )  CLI_ASSERT_STRINGIFY2( x )

/**
 * @def CLI_ASSERT
 * @brief Project-local assertion macro which prints diagnostic information
 *        via Cli_AssertFail() and halts execution on failure.
 */
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
/**
 * @brief Pointer to the currently initialized CLI configuration.
 *
 * This internal pointer is set by Cli_Init() and points to the
 * application-provided Cli_Config_t instance. It is NULL when the CLI is not
 * initialized. This symbol is internal to the implementation and must not be
 * accessed by application code.
 */
static Cli_Config_t *g_tCli_Config = NULL;

/* #############################################################################
 * # static function prototypes
 * ###########################################################################*/
/** Forward declarations for internal helper functions (not part of public API)
 */

/**
 * @internal
 * @brief Handle an assertion failure: optionally print message and halt.
 * @param msg message to print before halting. May be NULL.
 *
 * This function does not return.
 */
static void Cli_AssertFail( const char *msg );

/**
 * @internal
 * @brief Write one character via the configured output handler.
 * @param c Character to write.
 */
static void Cli_WriteCharacter( char c );

/**
 * @internal
 * @brief Echo one character to the console, handling newlines and backspace.
 * @param c Character to echo.
 */
static void Cli_EchoCharacter( char c );

/**
 * @internal
 * @brief Return the last character stored in the RX buffer.
 * @return Last stored character.
 */
static char Cli_GetLastReceivedCharacter( void );

/**
 * @internal
 * @brief Check whether the RX buffer is full.
 * @return true when buffer is full, false otherwise.
 */
static bool Cli_IsRxBufferFull( void );

/**
 * @internal
 * @brief Clear the RX buffer and reset the character counter.
 */
static void Cli_ResetRxBuffer( void );

/**
 * @internal
 * @brief Echo a NUL-terminated string to the configured output.
 * @param str NUL-terminated string to echo.
 */
static void Cli_EchoString( const char *str );

/**
 * @internal
 * @brief Write the configured CLI prompt to the output.
 */
static void Cli_WritePrompt( void );

/**
 * @internal
 * @brief Called when an unknown command name was entered; prints an error.
 * @param in_pcCmdName NUL-terminated unknown command name.
 */
static void Cli_HandleUnknownCommand( const char *const in_pcCmdName );

/**
 * @internal
 * @brief Built-in help command handler.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param context User-provided context (unused).
 * @return CLI_OK_STATUS on success.
 */
static int Cli_HelpCommand( int argc, char *argv[], void *context );


/**
 * @internal
 * @brief Find a command binding by command name.
 * @param in_pcCommandName NUL-terminated command name to find.
 * @return pointer to Cli_Binding_t if found, NULL otherwise.
 */
static const Cli_Binding_t *Cli_FindCommand( const char *const in_pcCommandName );

/* #############################################################################
 * # global function implementations
 * ###########################################################################*/


void Cli_Receive( char in_cChar )
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
        CLI_ASSERT( g_tCli_Config->acRxByteBuffer );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( g_tCli_Config->tNofStoredCharacters < CLI_MAX_RX_BUFFER_SIZE );
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32CfgCanaryStart );
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32CfgCanaryEnd );
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32CanaryMid );
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

    if( '\b' == in_cChar ) // Character delete (backspace)
    {
        if( g_tCli_Config->tNofStoredCharacters > 0 )
        {
            g_tCli_Config->tNofStoredCharacters--;
            size_t idx = g_tCli_Config->tNofStoredCharacters;

            // Remove the last character (the one that was deleted)
            g_tCli_Config->acRxByteBuffer[idx] = '\0';
        }
    }
    else // add the character to the buffer
    {
        size_t idx = g_tCli_Config->tNofStoredCharacters;
        g_tCli_Config->acRxByteBuffer[idx] = in_cChar;
        g_tCli_Config->tNofStoredCharacters++;
    }
    CLI_ASSERT( g_tCli_Config->tNofStoredCharacters < CLI_MAX_RX_BUFFER_SIZE );
}

void Cli_Process()
{
    char *acArguments[CLI_MAX_NOF_ARGUMENTS] = { 0 };
    int   s32NofArguments = 0;
    char *pcNextArgument = NULL;

    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32CfgCanaryStart );
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32CfgCanaryEnd );
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32CanaryMid );
    }
    { // Do nothing, if these conditions are not met
        if( Cli_GetLastReceivedCharacter() != '\n' &&
            ( false == Cli_IsRxBufferFull() ) )
        {
            return;
        }
    }

    // Process the Buffer
    for( size_t i = 0; i < g_tCli_Config->tNofStoredCharacters; i++ )
    {
        if( s32NofArguments >= CLI_MAX_NOF_ARGUMENTS )
        {
            Cli_EchoString( "Too many arguments \n" );
            break;
        }

        char *const c = &g_tCli_Config->acRxByteBuffer[i];
        if( ' ' == *c || '\n' == *c ||
            ( g_tCli_Config->tNofStoredCharacters - 1 ) == i )
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

    if( CLI_MAX_RX_BUFFER_SIZE == g_tCli_Config->tNofStoredCharacters )
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
            ptCmdBinding->pFnCmdHandler( s32NofArguments, acArguments,
                                         ptCmdBinding->pContext );
        }
    }

    // Reset the cli buffer and write the prompt again for a new user input
    Cli_ResetRxBuffer();
    Cli_WritePrompt();
}

void Cli_Init( Cli_Config_t *const inout_ptCfg,
               Cli_PutCharacter_fn in_pFnWriteCharacter )
{
    { // Input Checks
        CLI_ASSERT( inout_ptCfg );
        CLI_ASSERT( false == inout_ptCfg->bIsInitialized );
        CLI_ASSERT( NULL == g_tCli_Config ); // only one instance allowed
        CLI_ASSERT( in_pFnWriteCharacter );
    }

    inout_ptCfg->u32CfgCanaryStart = CLI_CANARY;
    inout_ptCfg->u32CfgCanaryEnd = CLI_CANARY;
    inout_ptCfg->u32CanaryMid = CLI_CANARY;
    inout_ptCfg->pFnWriteCharacter = in_pFnWriteCharacter;
    inout_ptCfg->tNofStoredCharacters = 0;
    inout_ptCfg->tNofBindings = 0;

    // Store the config locally in a static variable
    g_tCli_Config = inout_ptCfg;

    g_tCli_Config->bIsInitialized = true;

    // Register the Help handler
    Cli_Binding_t helpBinding = { "help", Cli_HelpCommand, NULL,
                                  "Lists all commands" };
    Cli_Register( &helpBinding );

    // Reset the Rx Buffer and print the welcome message
    Cli_ResetRxBuffer();
    Cli_EchoString( "CLI was started - enter your commands (or enter "
                    "'help')\n" );
    Cli_EchoString( CLI_PROMPT );

    return;
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
        CLI_ASSERT( CLI_CANARY == g_tCli_Config->u32CanaryMid );
    }

    char    buffer[128]; // Temporary buffer for formatted string
    va_list args;
    va_start( args, fmt );
    vsnprintf( buffer, sizeof( buffer ), fmt, args );
    va_end( args );

    Cli_EchoString( buffer );
    Cli_EchoCharacter( '\n' );
}

void Cli_Register( const Cli_Binding_t *const in_ptBinding )
{
    {
        // Input Checks - inout_ptCfg
        CLI_ASSERT( in_ptBinding );
        CLI_ASSERT( in_ptBinding->pcCmdName );
        CLI_ASSERT( in_ptBinding->pcHelperString );
        CLI_ASSERT( in_ptBinding->pFnCmdHandler );
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->bIsInitialized );
    }

    bool bBindingExists = false;
    bool bBindingIsStored = false;

    for( size_t i = 0; i < g_tCli_Config->tNofBindings; i++ )
    {
        const Cli_Binding_t *ptBinding = &g_tCli_Config->atCliCmdBindingsBuffer[i];
        if( 0 == strncmp( ptBinding->pcCmdName, in_ptBinding->pcCmdName,
                          CLI_MAX_CMD_NAME_LENGTH ) )
        {
            bBindingExists = true;
            break;
        }
    }
    CLI_ASSERT( false == bBindingExists );


    if( g_tCli_Config->tNofBindings < CLI_MAX_NOF_CALLBACKS )
    {
        //  Deep Copy the binding into the buffer
        size_t idx = g_tCli_Config->tNofBindings;
        memcpy( &g_tCli_Config->atCliCmdBindingsBuffer[idx], in_ptBinding,
                sizeof( Cli_Binding_t ) );
        g_tCli_Config->tNofBindings++;

        // Mark that the binding was stored
        bBindingIsStored = true;
    }

    CLI_ASSERT( true == bBindingIsStored );

    return;
}

void Cli_Unregister( const char *const in_pcCmdName )
{
    {
        // Input Checks - inout_ptCfg
        CLI_ASSERT( in_pcCmdName );
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->bIsInitialized );
    }

    bool bBindingFound = false;

    for( size_t i = 0; i < g_tCli_Config->tNofBindings; i++ )
    {
        Cli_Binding_t *ptBinding = &g_tCli_Config->atCliCmdBindingsBuffer[i];
        if( 0 == strncmp( ptBinding->pcCmdName, in_pcCmdName,
                          CLI_MAX_CMD_NAME_LENGTH ) )
        {
            bBindingFound = true;
            // Shift all following bindings one position to the left
            for( size_t j = i; j < g_tCli_Config->tNofBindings - 1; j++ )
            {
                memcpy( &g_tCli_Config->atCliCmdBindingsBuffer[j],
                        &g_tCli_Config->atCliCmdBindingsBuffer[j + 1],
                        sizeof( Cli_Binding_t ) );
            }
            g_tCli_Config->tNofBindings--;
            break;
        }
    }
    CLI_ASSERT( true == bBindingFound );

    return;
}


/* #############################################################################
 * # static function implementations
 * ###########################################################################*/


static int Cli_HelpCommand( int argc, char *argv[], void *context )
{
    { // Input Checks
        CLI_ASSERT( g_tCli_Config );
        CLI_ASSERT( g_tCli_Config->pFnWriteCharacter );
        CLI_ASSERT( true == g_tCli_Config->bIsInitialized );
    }
    Cli_EchoString( "\n" );

    // Create a list of all registered commands
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
    (void)context;

    return CLI_OK_STATUS;
}

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

/**
 * Get the last stored character from the RX buffer.
 *
 * @return last character previously stored in the rx buffer.
 */
static char Cli_GetLastReceivedCharacter( void )
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
    if( NULL != g_tCli_Config )
    {
        if( NULL != msg )
        {
            // ANSI-Color Code for Red: \033[31m ... \033[0m
            Cli_EchoString( "\033[31m[CLI ASSERT FAIL]\033[0m " );
            Cli_EchoString( msg );
        }
    }

    while( 1 )
    {
    }
}
