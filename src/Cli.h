#ifndef CLI_H
#define CLI_H

/**
 * @file Cli.h
 * @brief Small embedded command line interface (CLI) public API.
 *
 * The CLI provides a tiny command dispatcher suitable for embedded targets.
 * Applications register command bindings (name, handler, helper string) and
 * feed received characters to the CLI by calling Cli_ReceiveCharacter(). The
 * CLI writes characters using a user-supplied output function (for example
 * putchar).
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/** ANSI prompt fragments for success/failure messages printed by the CLI */
#define CLI_OK_PROMPT   "\033[32m[OK]  \033[0m "
#define CLI_FAIL_PROMPT "\033[31m[FAIL]\033[0m "

/** Common return values for command handlers */
#define CLI_OK_STATUS   ( 0 )
#define CLI_FAIL_STATUS ( -1 )

/** Limits for the CLI (max callbacks, max name/helper lengths) */
#define CLI_MAX_NOF_CALLBACKS        ( 10 )
#define CLI_MAX_CMD_NAME_LENGTH      ( 32 )
#define CLI_MAX_HELPER_STRING_LENGTH ( 64 )

/** Maximum number of bytes in the RX line buffer */
#define CLI_MAX_RX_BUFFER_SIZE ( 256 )

/** Helper to determine array size at compile time */
#define CLI_GET_ARRAY_SIZE( arr ) ( sizeof( arr ) / sizeof( arr[0] ) )


/**
 * Signature of a registered CLI command handler.
 *
 * argc and argv follow the usual C convention: argv[0] is the command name.
 * context is the optional user-provided context pointer that was stored with
 * the command binding.
 *
 * Returns CLI_OK_STATUS on success or CLI_FAIL_STATUS on error.
 */
typedef int ( *Cli_CommandHandler_t )( int argc, char *argv[], void *context );

/**
 * Signature of the function used by the CLI to output a single character.
 * Implementations should return a non-negative value on success or a
 * negative value on error (matches putchar/putc behaviour).
 */
typedef int ( *Cli_WriteCharacterHandler_t )( char c );

/**
 * Binding structure used to register a command with the CLI.
 *
 * pcCmdName: NUL-terminated command name (max length CLI_MAX_CMD_NAME_LENGTH).
 * pFnCmdHandler: function called when the command is invoked.
 * pContext: optional user pointer passed to the handler.
 * pcHelperString: short help string shown by the built-in help command.
 */
typedef struct Cli_Binding
{
    const char           pcCmdName[CLI_MAX_CMD_NAME_LENGTH];
    Cli_CommandHandler_t pFnCmdHandler;
    void                *pContext;
    const char           pcHelperString[CLI_MAX_HELPER_STRING_LENGTH];
} Cli_Binding_t;

/**
 * CLI runtime configuration and state structure.
 *
 * Applications should allocate and zero-initialize one instance of this
 * structure and pass a pointer to Cli_Init(). The structure holds internal
 * buffers and must remain valid for the entire runtime of the CLI.
 */
typedef struct
{
    uint32_t                    u32CfgCanaryStart; /**< internal canary */
    Cli_WriteCharacterHandler_t pFnWriteCharacter; /**< output function */
    bool                        bIsInitialized;    /**< initialization flag */

    size_t   tNofStoredCharacters; /**< count of bytes in rx buffer */
    char     acRxByteBuffer[CLI_MAX_RX_BUFFER_SIZE]; /**< input buffer */
    uint32_t u32CanaryMid;                           /**< internal canary */

    size_t        tNofBindings; /**< number of registered commands */
    Cli_Binding_t atCliCmdBindingsBuffer[CLI_MAX_NOF_CALLBACKS]; /**< registered
                                                                    commands */
    uint32_t u32CfgCanaryEnd; /**< internal canary */
} Cli_Config_t;

/**
 * Initialize the CLI instance.
 *
 * @param[in,out] inout_ptCfg Pointer to an application-allocated Cli_Config_t
 * structure.
 * @param[in]     in_pFnWriteCharacter Function used to write single characters
 * (e.g. putchar).
 */
void Cli_Init( Cli_Config_t *const         inout_ptCfg,
               Cli_WriteCharacterHandler_t in_pFnWriteCharacter );

/**
 * Register a command binding with the CLI. The binding is copied into the
 * internal registry, so the caller may free or reuse the passed structure
 * afterwards.
 *
 * @param[in] in_ptBinding Pointer to the binding to register.
 */
void Cli_RegisterBinding( const Cli_Binding_t *const in_ptBinding );

/**
 * Unregister a previously registered command by name.
 *
 * @param[in] in_pcCmdName NUL-terminated name of the command to remove.
 */
void Cli_UnregisterBinding( const char *const in_pcCmdName );

/**
 * Feed one received character into the CLI state machine. Typically called
 * from the console/uart receive ISR or a polling loop.
 *
 * @param[in] in_cChar Received character.
 */
void Cli_ReceiveCharacter( char in_cChar );

/**
 * Print a formatted message through the CLI output function. Works like
 * printf but writes to the configured CLI output handler.
 *
 * @param[in] fmt printf-style format string and following arguments.
 */
void Cli_Print( const char *const fmt, ... );

#endif // CLI_H
