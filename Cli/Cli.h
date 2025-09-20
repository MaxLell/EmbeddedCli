#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CLI_MAX_RX_BUFFER_SIZE ( 256 )
#define CLI_OK_PROMPT          "\033[32m[OK]  \033[0m "
#define CLI_FAIL_PROMPT        "\033[31m[FAIL]\033[0m "

typedef struct
{
    const char *pcCmdName;
    void ( *pFnCmdHandler )( int argc, char *argv[] );
    const char *pcHelperString;
} Cli_Binding_t;

typedef struct
{
    uint32_t u32CfgCanaryStart;
    int ( *pFnWriteCharacter )( char c );
    bool           bIsInitialized;
    size_t         tCurrentRxBufferSize;
    char           acRxByteBuffer[CLI_MAX_RX_BUFFER_SIZE];
    uint32_t       u32BufferCanary;
    Cli_Binding_t *atCliCmdBindingsBuffer;
    size_t         tNofBindings;
    uint32_t       u32CfgCanaryEnd;
} Cli_Config_t;


void Cli_Initialize( Cli_Config_t *ptCfg );

void Cli_AddCharacter( Cli_Config_t *ptCfg, char c );

void Cli_Process( Cli_Config_t *ptCfg );

void Cli_WriteString( const char *in_pcString );
