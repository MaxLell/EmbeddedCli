#include "Cli.h"
#include "unity.h"

void setUp( void ) {}

void tearDown( void ) {}


void test_Cli_Initialize_sets_initialized_flag( void )
{
    char         rxBuffer[CLI_RX_BUFFER_SIZE] = { 0 };
    Cli_Config_t cfg = { .pFnWriteCharacter = NULL,
                         .bIsInitialized = false,
                         .acRxByteBuffer = rxBuffer,
                         .tRxBufferSize = CLI_RX_BUFFER_SIZE };
    Cli_Initialize( &cfg );

    TEST_ASSERT_TRUE( cfg.bIsInitialized );
}

// void test_Cli_Buffer_overflow_is_prevented( void )
// {
//     char rxBuffer[CLI_RX_BUFFER_SIZE] = { 0 };
//     int  echo_called = 0;
//     int  echo_func( char c )
//     {
//         echo_called++;
//         return c;
//     }
//     Cli_Config_t cfg = { .pFnWriteCharacter = echo_func,
//                          .bIsInitialized = false,
//                          .acRxByteBuffer = rxBuffer,
//                          .tRxBufferSize = CLI_RX_BUFFER_SIZE };
//     Cli_Initialize( &cfg );
//     for( int i = 0; i < CLI_RX_BUFFER_SIZE + 10; ++i )
//     {
//         Cli_AddCharToRxBuffer( 'B' );
//     }
//     TEST_ASSERT_LESS_OR_EQUAL( CLI_RX_BUFFER_SIZE, cfg.tRxBufferSize );
// }

// void test_Cli_ProcessRxBuffer_executes_command( void )
// {
//     // This test is a stub, as command execution needs a real binding table
//     TEST_IGNORE_MESSAGE( "Command execution test needs shell command "
//                          "bindings" );
// }
