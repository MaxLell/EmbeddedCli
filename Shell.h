#include "CommonTypes.h"

#include <stddef.h>

typedef struct
{
    const char *command;
    s32 ( *handler )( s32 argc, char *argv[] );
    const char *help;
} Shell_Cmd_t;

extern const Shell_Cmd_t *const g_atShellCmds;
extern const size_t             g_tNofShellCmds;

typedef struct
{
    //! Function to call whenever a character needs to be sent out.
    s32 ( *send_char )( char c );
} Shell_Config_t;

//! Initializes the demo shell. To be called early at boot.
void Shell_Init( const Shell_Config_t *in_ptShellCfg );

//! Call this when a character is received. The character is processed
//! synchronously.
void Shell_ReadCharacter( char c );

//! Prs32 help command
s32 Shell_HelpHandler( s32 argc, char *argv[] );

//! Prs32s a line then a newline
void Shell_WriteLine( const char *str );
