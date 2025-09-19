#pragma once

#include <stdbool.h>
#include <stddef.h>

#define CLI_RX_BUFFER_SIZE ( 256 )

typedef struct
{
    const char *command;
    int ( *handler )( int argc, char *argv[] );
    const char *help;
} Cli_Binding_t;

typedef struct
{
    bool bIsInitialized;
    int ( *pFnWriteCharacter )( char c );
    size_t         tRxBufferSize;
    char          *acRxBuffer;
    Cli_Binding_t *atBindings;
    size_t         tNofBindings;
} Cli_Config_t;


void Cli_Initialize( Cli_Config_t *impl );

void Cli_ReadAndProcessCharacter( char c );

void Cli_WriteString( const char *str );
