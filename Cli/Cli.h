#pragma once

#include <stdbool.h>
#include <stddef.h>

#define CLI_RX_BUFFER_SIZE ( 256 )
#define CLI_OK_PROMPT      "\033[32m[OK]  \033[0m "
#define CLI_FAIL_PROMPT    "\033[31m[FAIL]\033[0m "

typedef struct
{
    const char *pcCmdName;
    void ( *pFnCmdHandler )( int argc, char *argv[] );
    const char *pcHelperString;
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

void Cli_AddCharacter( char c );

void Cli_Process( void );
