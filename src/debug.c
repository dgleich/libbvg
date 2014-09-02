/**
 * @file debug.c
 * Implement routines for debugging macros
 * @author David F. Gleich, based on 
 * @date 01 Sept 2014
 * @brief Simple macros for debug as in:
 *    http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing
 */

#ifdef DEBUG

#include <stdarg.h>
#include <stdio.h>

void dbg_printf(int level, const char *fmt, ...)
{
    va_list args;
    if (level <= DEBUG) { 
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
}

#endif 