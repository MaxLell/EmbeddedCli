#pragma once

#include <stddef.h>

typedef struct
{
    const char *command;
    int ( *handler )( int argc, char *argv[] );
    const char *help;
} Cli_Binding_t;

extern const Cli_Binding_t *const g_shell_commands;
extern const size_t               g_num_shell_commands;

typedef struct
{
    int ( *send_char )( char c );
} Cli_Config_t;

void Cli_Initialize( const Cli_Config_t *impl );

void Cli_ReadAndProcessCharacter( char c );

void Cli_WriteString( const char *str );
