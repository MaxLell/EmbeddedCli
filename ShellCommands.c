#include "CustomAssert.h"
#include "Shell.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE( arr ) ( sizeof( arr ) / sizeof( arr[0] ) )

int CliCmd_HelloWorld( int in_s32Argc, char *in_acArgv[] )
{
    Shell_WriteLine( "Hello World!" );
    return 0;
}

int CliCmd_AddTwoNumbers( int in_s32Argc, char *in_acArgv[] )
{
    // We expect 3 arguments
    // Cmd Name
    // Number 1 and Number 2

    if( 3 != in_s32Argc )
    {
        Shell_WriteLine( "> FAIL, more numbers needed" );
        return -1;
    }
    char *pc_cmdName = in_acArgv[0];
    if( 0 != strcmp( pc_cmdName, "add" ) )
    {
        Shell_WriteLine( "> FAIL, wrong command" );
        return -1;
    }
    int32_t s32Number1 = atoi( in_acArgv[1] );
    int32_t s32Number2 = atoi( in_acArgv[2] );

    int32_t s32Result = s32Number1 + s32Number2;
    char    ac_str[32];
    sprintf( ac_str, "> OK, result = %d", s32Result );
    Shell_WriteLine( ac_str );
    return 0;
}


static const Shell_Cmd_t atShellCmds[] = {
    { "hello", CliCmd_HelloWorld, "Say hello" },
    { "add", CliCmd_AddTwoNumbers, "adds 2 numbers" },
    { "help", Shell_HelpHandler, "Lists all commands" },
};

const Shell_Cmd_t *const g_atShellCmds = atShellCmds;
const size_t             g_tNofShellCmds = ARRAY_SIZE( atShellCmds );
