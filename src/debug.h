/*
@(#)File:           $RCSfile: debug.h,v $
@(#)Version:        $Revision: 3.8 $
@(#)Last changed:   $Date: 2011/12/31 19:48:31 $
@(#)Purpose:        Definitions for the debugging system
@(#)Author:         J Leffler
@(#)Copyright:      (C) JLSS 1990-93,1997-99,2003,2005,2008,2011
@(#)Product:        Informix Database Driver for Perl DBI Version 2013.0521 (2013-05-21)
*/

#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_INFO (2)
#define DEBUG_DEEP (4)

/*
** Usage:  TRACE((level, fmt, ...))
** "level" is the debugging level which must be operational for the output
** to appear. "fmt" is a printf format string. "..." is whatever extra
** arguments fmt requires (possibly nothing).
** The non-debug macro means that the code is validated but never called.
** -- See chapter 8 of 'The Practice of Programming', by Kernighan and Pike.
*/
#ifdef DEBUG
#define TRACE(x)    dbg_printf x
#else
#define TRACE(x)    do { if (0) dbg_printf x; } while (0)
#endif /* DEBUG */

extern void     dbg_printf(int level, const char *fmt,...);

#endif /* DEBUG_H */