#pragma once

#include <stdbool.h>
#include <stddef.h>

#define CLI_RX_BUFFER_SIZE ( 256 )
#define CLI_FAIL_PROMPT    "> [FAIL]; "
#define CLI_OK_PROMPT      "> [OK];   "

typedef struct
{
    const char *command;
    int ( *handler )( int argc, char *argv[] );
    const char *help;
} Cli_Binding_t;

typedef struct
{
    int ( *pFnWriteCharacter )( char c );
    bool           bIsInitialized;
    size_t         tRxBufferSize;
    char          *acRxByteBuffer;
    Cli_Binding_t *atCliCmdBindingsBuffer;
    size_t         tNofBindings;
} Cli_Config_t;


void Cli_Initialize( Cli_Config_t *ptCfg );

void Cli_AddCharToRxBuffer( char c );

void Cli_ProcessRxBuffer();
