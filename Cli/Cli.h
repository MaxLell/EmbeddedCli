#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CLI_MAX_RX_BUFFER_SIZE ( 256 )
#define CLI_OK_PROMPT          "\033[32m[OK]  \033[0m "
#define CLI_FAIL_PROMPT        "\033[31m[FAIL]\033[0m "
#define CLI_OK_STATUS          ( 0 )
#define CLI_FAIL_STATUS        ( -1 )

#define CLI_GET_ARRAY_SIZE( arr ) ( sizeof( arr ) / sizeof( arr[0] ) )

typedef struct
{
    const char *pcCmdName;
    int ( *pFnCmdHandler )( int argc, char *argv[] );
    const char *pcHelperString;
} Cli_Binding_t;

typedef struct
{
    uint32_t u32CfgCanaryStart;
    int ( *pFnWriteCharacter )( char c );
    bool           bIsInitialized;
    size_t         tNofStoredCharacters;
    char           acRxByteBuffer[CLI_MAX_RX_BUFFER_SIZE];
    uint32_t       u32BufferCanary;
    Cli_Binding_t *atCliCmdBindingsBuffer;
    size_t         tNofBindings;
    uint32_t       u32CfgCanaryEnd;
} Cli_Config_t;


void Cli_Initialize( Cli_Config_t *const inout_ptCfg );

void Cli_AddCharacter( Cli_Config_t *const inout_ptCfg, char in_cChar );

void Cli_Process( Cli_Config_t *const inout_ptCfg );

void Cli_Print( const char *const fmt, ... );
