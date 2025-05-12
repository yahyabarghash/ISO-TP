/*
 
/**
 * \file   logger.h
 * 
 * The logger provides three distinct verbosity levels:
 * 
 * @li LOG_XXX   - Essential operational messages (errors, critical warnings)
 * @li LOG1_XXX  - Debug information (function tracing, state changes)
 * @li LOG2_XXX  - High-volume data output (packet dumps, hex data)
 * 
 * Logger behavior can be configured in two ways:
 * 
 * 1. Per-module control:
 * @code
 * #define LOGGER_ENABLE MODULE_NAME  // Before including logger.h
 * #include "logger.h"
 * @endcode
 * Where MODULE_NAME and LOGGER_LEVEL can be defined externally
 * 
 * 2. Global control:
 * @code
 * #define LOGGER_LEVEL 1  // Show only LOG_ and LOG1_ messages
 * @endcode
 * When LOGGER_LEVEL < message level (0-2), the message is suppressed
 */
#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS    8
#endif

/*!
\brief   Printout data in a standard hex view
\param    p_buf        Pointer to data which should be printed out.
\param    l_len        Length of a data
\return   None
\example
0x000000: 2e 2f 68 65 78 64 75 6d ./hexdum
0x000008: 70 00 53 53 48 5f 41 47 p.SSH_AG
0x000010: 45 4e 54 5f             ENT_
*/
/* We need a printable char, so it's not an warning or error
 * "else if( isprint( ( (char*)p_buf )[j] ) )"
 *  */
#pragma GCC diagnostic ignored "-Wchar-subscripts"
inline static void log_hexdump(const void* p_buf, uint32_t l_len)
{
    unsigned int i, j;

    for( i = 0; i < l_len
            + ( ( l_len % HEXDUMP_COLS ) ? ( HEXDUMP_COLS
            - l_len % HEXDUMP_COLS ) : 0 ); i++ )
    {
        /* print offset */
        if( i % HEXDUMP_COLS == 0 )
        {
            printf( "0x%06x: ", i );
        }

        /* print hex data */
        if( i < l_len )
        {
            printf( "%02x ", 0xFF & ( (char*)p_buf )[i] );
        }
        else /* end of block, just aligning for ASCII dump */
        {
            printf( "   " );
        }

        /* print ASCII dump */
        if( i % HEXDUMP_COLS == ( HEXDUMP_COLS - 1 ) )
        {
            for( j = i - ( HEXDUMP_COLS - 1 ); j <= i; j++ )
            {
                putchar( '.' );
               /* if( j >= l_len )  end of block, not really printing
                {
                    putchar( ' ' );
                }
                else if( isprint( ( (char*)p_buf )[j] ) )  printable char
                {
                    putchar( 0xFF & ( (char*)p_buf )[j] );
                }
                else  other char
                {
                    putchar( '.' );
                }*/
            }
            putchar( '\r' );putchar( '\n' );
        }
    }
}


#define LOG2_OK(msg, ...)       LOGGER_OK(2,msg, ##__VA_ARGS__)
#define LOG2_ERR(msg, ...)      LOGGER_ERR(2,msg, ##__VA_ARGS__)
#define LOG2_INFO(msg, ...)     LOGGER_INFO(2,msg, ##__VA_ARGS__)
#define LOG2_WARN(msg, ...)     LOGGER_WARN(2,msg, ##__VA_ARGS__)
#define LOG2_FAIL(msg, ...)     LOGGER_FAIL(2,msg, ##__VA_ARGS__)
#define LOG2_DBG(msg, ...)      LOGGER_DBG(2,msg, ##__VA_ARGS__)
#define LOG2_RAW(...)           LOGGER_RAW(2, __VA_ARGS__)
#define LOG2_HEXDUMP(buf,len)   LOGGER_HEXDUMP(2, buf,len)

#define LOG1_OK(msg, ...)       LOGGER_OK(1,msg, ##__VA_ARGS__)
#define LOG1_ERR(msg, ...)      LOGGER_ERR(1,msg, ##__VA_ARGS__)
#define LOG1_INFO(msg, ...)     LOGGER_INFO(1,msg, ##__VA_ARGS__)
#define LOG1_WARN(msg, ...)     LOGGER_WARN(1,msg, ##__VA_ARGS__)
#define LOG1_FAIL(msg, ...)     LOGGER_FAIL(1,msg, ##__VA_ARGS__)
#define LOG1_DBG(msg, ...)      LOGGER_DBG(1,msg, ##__VA_ARGS__)
#define LOG1_RAW(...)           LOGGER_RAW(1, __VA_ARGS__)
#define LOG1_HEXDUMP(buf,len)   LOGGER_HEXDUMP(1, buf,len)

#define LOG_OK(msg, ...)        LOGGER_OK(0,msg, ##__VA_ARGS__)
#define LOG_ERR(msg, ...)       LOGGER_ERR(0,msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...)      LOGGER_INFO(0,msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...)      LOGGER_WARN(0,msg, ##__VA_ARGS__)
#define LOG_FAIL(msg, ...)      LOGGER_FAIL(0,msg, ##__VA_ARGS__)
#define LOG_DBG(msg, ...)       LOGGER_DBG(0,msg, ##__VA_ARGS__)
#define LOG_RAW(msg, ...)       LOGGER_RAW(0,msg, ##__VA_ARGS__)
#define LOG_HEXDUMP(buf,len)    LOGGER_HEXDUMP(0, buf,len)

#define log_entry()             LOG2_INFO( "Enter %s function",__func__ );
#define log_leave()             LOG2_INFO( "Leave %s function",__func__ );

#pragma GCC diagnostic ignored "-Wformat"
#define LOGGER_OK(log_lvl, msg, ...)        \
    do { if ((LOGGER_ENABLE) && (LOGGER_LEVEL > log_lvl)) printf("   ok | %5s (%d)| " msg "\r\n", __FILE__, __LINE__, ##__VA_ARGS__); }while (0)

#pragma GCC diagnostic ignored "-Wformat"
#define LOGGER_ERR(log_lvl, msg, ...)       \
    do { if ((LOGGER_ENABLE) && (LOGGER_LEVEL > log_lvl)) printf("  err | %5s (%d)| " msg "\r\n", __FILE__, __LINE__, ##__VA_ARGS__); }while (0)

#pragma GCC diagnostic ignored "-Wformat"
#define LOGGER_INFO(log_lvl, msg, ...)      \
    do { if ((LOGGER_ENABLE) && (LOGGER_LEVEL > log_lvl)) printf(" info | %5s (%d)| " msg "\r\n", __FILE__, __LINE__, ##__VA_ARGS__); }while (0)

#pragma GCC diagnostic ignored "-Wformat"
#define LOGGER_WARN(log_lvl, msg, ...)      \
    do { if ((LOGGER_ENABLE) && (LOGGER_LEVEL > log_lvl)) printf(" warn | %5s (%d)| " msg "\r\n", __FILE__, __LINE__, ##__VA_ARGS__); }while (0)

#pragma GCC diagnostic ignored "-Wformat"
#define LOGGER_FAIL(log_lvl, msg, ...)      \
    do { if ((LOGGER_ENABLE) && (LOGGER_LEVEL > log_lvl)) printf(" fail | %5s (%d)| " msg "\r\n", __FILE__, __LINE__, ##__VA_ARGS__); }while (0)

#pragma GCC diagnostic ignored "-Wformat"
#define LOGGER_DBG(log_lvl, msg, ...)       \
    do { if ((LOGGER_ENABLE) && (LOGGER_LEVEL > log_lvl)) printf(" dbg  | %5s (%d)| " msg "\r\n", __FILE__, __LINE__, ##__VA_ARGS__); }while (0)

#pragma GCC diagnostic ignored "-Wformat"
#define LOGGER_RAW(log_lvl, msg, ...)       \
    do { if ((LOGGER_ENABLE) && (LOGGER_LEVEL > log_lvl)) printf(msg , ##__VA_ARGS__); }while (0)

#pragma GCC diagnostic ignored "-Wformat"
#define LOGGER_HEXDUMP(log_lvl, buf,len)        \
    do { if ((LOGGER_ENABLE) && (LOGGER_LEVEL > log_lvl)) log_hexdump(buf,len); }while (0)

#endif /* LOGGER_H_ */
