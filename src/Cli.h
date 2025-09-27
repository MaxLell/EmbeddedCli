#ifndef CLI_H
#define CLI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CLI_OK_PROMPT "\033[32m[OK]  \033[0m "
#define CLI_FAIL_PROMPT "\033[31m[FAIL]\033[0m "

#define CLI_OK_STATUS (0)
#define CLI_FAIL_STATUS (-1)

#define CLI_MAX_NOF_CALLBACKS (10)
#define CLI_MAX_CMD_NAME_LENGTH (32)
#define CLI_MAX_HELPER_STRING_LENGTH (64)

#define CLI_MAX_RX_BUFFER_SIZE (256)

#define CLI_GET_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

typedef int (*Cli_CommandHandler_t)(int argc, char *argv[], void *context);

typedef int (*Cli_PutCharacter_fn)(char c);

typedef struct Cli_Binding
{
    const char pcCmdName[CLI_MAX_CMD_NAME_LENGTH];
    Cli_CommandHandler_t pFnCmdHandler;
    void *pContext;
    const char pcHelperString[CLI_MAX_HELPER_STRING_LENGTH];
} Cli_Binding_t;

typedef struct
{
    uint32_t u32CfgCanaryStart;
    Cli_PutCharacter_fn pFnWriteCharacter;
    bool bIsInitialized;

    size_t tNofStoredCharacters;
    char acRxByteBuffer[CLI_MAX_RX_BUFFER_SIZE];
    uint32_t u32CanaryMid;

    size_t tNofBindings;
    Cli_Binding_t atCliCmdBindingsBuffer[CLI_MAX_NOF_CALLBACKS];
    uint32_t u32CfgCanaryEnd;
} Cli_Config_t;

void Cli_Init(Cli_Config_t *const inout_ptCfg,
              Cli_PutCharacter_fn in_pFnWriteCharacter);

void Cli_Register(const Cli_Binding_t *const in_ptBinding);

void Cli_Unregister(const char *const in_pcCmdName);

void Cli_Receive(char in_cChar);

void Cli_Process(void);

void Cli_ReceiveAndProcess(char in_cChar);

void Cli_Print(const char *const fmt, ...);

#endif // CLI_H
