#ifndef CUSTOM_ASSERT_H
#define CUSTOM_ASSERT_H

#include <stdio.h>
#ifndef TEST
#define ASSERT( in_pCondition )                                                \
    do                                                                         \
    {                                                                          \
        if( !( in_pCondition ) )                                               \
        {                                                                      \
            printf( "\n\n" );                                                  \
            printf( "#########################\n" );                           \
            printf( "#      ASSERT FAILED    #\n" );                           \
            printf( "#########################\n" );                           \
            printf( "----> Condition: (%s)\n", #in_pCondition );               \
            printf( "----> File: %s:%d\n", __FILE__, __LINE__ );               \
            while( 1 )                                                         \
            {                                                                  \
            }                                                                  \
        }                                                                      \
    } while( false )

#else
#define ASSERT( in_pCondition )                                                \
    do                                                                         \
    {                                                                          \
        if( !( in_pCondition ) )                                               \
        {                                                                      \
            printf( "\n\n" );                                                  \
            printf( "#########################\n" );                           \
            printf( "#      ASSERT FAILED    #\n" );                           \
            printf( "#########################\n" );                           \
            printf( "----> Condition: (%s)\n", #in_pCondition );               \
            printf( "----> File: %s:%d\n", __FILE__, __LINE__ );               \
        }                                                                      \
    } while( false )
#endif /* TEST */

#endif /* CUSTOM_ASSERT_H */
