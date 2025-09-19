
#include "Cli.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define CLI_RX_BUFFER_SIZE    ( 256 )
#define CLI_MAX_NOF_ARGUMENTS ( 16 )
#define CLI_PROMPT            "cli> "


#define CLI_FOR_ALL_COMMANDS( ptCommandBinding )                               \
    for( const Cli_Binding_t *ptCommandBinding = g_shell_commands;             \
         ptCommandBinding < &g_shell_commands[g_num_shell_commands];           \
         ptCommandBinding++ )

typedef struct
{
    int ( *send_char )( char c );
    size_t rx_size;
    char   rx_buffer[CLI_RX_BUFFER_SIZE];
} Cli_Context_t;

static Cli_Context_t g_tCli_Context = { 0 };

/**
 * @brief Checks if the shell is initialized.
 * @return true if send_char is set, false otherwise.
 */
static bool Cli_IsInitialized( void )
{
    return g_tCli_Context.send_char != NULL;
}


/**
 * @brief Sends a character via the shell interface.
 * @param c The character to send.
 */
static void Cli_WriteCharacter( char c )
{
    if( !Cli_IsInitialized() )
    {
        return;
    }
    g_tCli_Context.send_char( c );
}


/**
 * @brief Echoes a character to the shell (handles newline and backspace).
 * @param c The character to echo.
 */
static void Cli_EchoCharacter( char c )
{
    if( '\n' == c )
    {
        Cli_WriteCharacter( '\r' );
        Cli_WriteCharacter( '\n' );
    }
    else if( '\b' == c )
    {
        Cli_WriteCharacter( '\b' );
        Cli_WriteCharacter( ' ' );
        Cli_WriteCharacter( '\b' );
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
    return g_tCli_Context.rx_buffer[g_tCli_Context.rx_size - 1];
}


/**
 * @brief Checks if the RX buffer is full.
 * @return true if the buffer is full, false otherwise.
 */
static bool Cli_IsRxBufferFull( void )
{
    return ( g_tCli_Context.rx_size >= CLI_RX_BUFFER_SIZE );
}


/**
 * @brief Resets the receive buffer and clears all received characters.
 */
static void Cli_ResetRxBuffer( void )
{
    memset( g_tCli_Context.rx_buffer, 0, sizeof( g_tCli_Context.rx_buffer ) );
    g_tCli_Context.rx_size = 0;
}


/**
 * @brief Echoes a string to the shell.
 * @param str The string to echo.
 */
static void Cli_EchoString( const char *str )
{
    for( const char *c = str; *c != '\0'; ++c )
    {
        Cli_EchoCharacter( *c );
    }
}


/**
 * @brief Outputs the shell prompt.
 */
static void Cli_WritePrompt( void )
{
    Cli_EchoString( CLI_PROMPT );
}


/**
 * @brief Searches for a shell command by its name.
 * @param name Name of the command.
 * @return Pointer to the found command or NULL.
 */
static const Cli_Binding_t *Cli_FindCommand( const char *name )
{
    CLI_FOR_ALL_COMMANDS( command )
    {
        if( strcmp( command->command, name ) == 0 )
        {
            return command;
        }
    }
    return NULL;
}


/**
 * @brief Processes the current content of the receive buffer and executes a
 * command if available.
 *
 * Splits the input into arguments, searches for the matching command, and calls
 * its handler.
 */
static void Cli_ProcessRxBuffer( void )
{
    if( Cli_GetLastEntryFromRxBuffer() != '\n' && !Cli_IsRxBufferFull() )
    {
        return;
    }

    char *argv[CLI_MAX_NOF_ARGUMENTS] = { 0 };
    int   argc = 0;

    char *next_arg = NULL;
    for( size_t i = 0;
         i < g_tCli_Context.rx_size && argc < CLI_MAX_NOF_ARGUMENTS; ++i )
    {
        char *const c = &g_tCli_Context.rx_buffer[i];
        if( *c == ' ' || *c == '\n' || i == g_tCli_Context.rx_size - 1 )
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

    if( g_tCli_Context.rx_size == CLI_RX_BUFFER_SIZE )
    {
        Cli_EchoCharacter( '\n' );
    }

    if( argc >= 1 )
    {
        const Cli_Binding_t *command = Cli_FindCommand( argv[0] );
        if( !command )
        {
            Cli_EchoString( "Unknown command: " );
            Cli_EchoString( argv[0] );
            Cli_EchoCharacter( '\n' );
            Cli_EchoString( "Type 'help' to list all commands\n" );
        }
        else
        {
            command->handler( argc, argv );
        }
    }
    Cli_ResetRxBuffer();
    Cli_WritePrompt();
}

void Cli_Initialize( const Cli_Config_t *impl )
{
    g_tCli_Context.send_char = impl->send_char;
    Cli_ResetRxBuffer();
    Cli_EchoString( "\n" CLI_PROMPT );
}

void Cli_ReadAndProcessCharacter( char c )
{
    if( c == '\r' || Cli_IsRxBufferFull() || !Cli_IsInitialized() )
    {
        return;
    }
    Cli_EchoCharacter( c );

    if( c == '\b' )
    {
        if( g_tCli_Context.rx_size > 0 )
        {
            g_tCli_Context.rx_buffer[--g_tCli_Context.rx_size] = '\0';
        }
        return;
    }

    g_tCli_Context.rx_buffer[g_tCli_Context.rx_size++] = c;

    Cli_ProcessRxBuffer();
}

void Cli_WriteString( const char *str )
{
    Cli_EchoString( str );
    Cli_EchoCharacter( '\n' );
}

int CliBinding_HelpHandler( int argc, char *argv[] )
{
    CLI_FOR_ALL_COMMANDS( command )
    {
        Cli_EchoString( command->command );
        Cli_EchoString( ": " );
        Cli_EchoString( command->help );
        Cli_EchoCharacter( '\n' );
    }
    return 0;
}
