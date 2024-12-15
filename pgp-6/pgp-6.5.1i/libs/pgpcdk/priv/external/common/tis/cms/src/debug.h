/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#ifndef _INCL_DEBUG_H

/* macro for printing  debug messages */
#ifdef DEBUG
    #define DPRINT(f)
#else
    #define DEBUGSYSLOG(f)
#endif

/* macro for printing binary data in hex to syslog */
#ifdef DEBUG
    extern void _PrintBytes(unsigned char *, unsigned int);
    #define DEBUGPRINTDATA(a, b) _PrintBytes((a),(b))
#else
    #define DEBUGPRINTDATA(a, b)
#endif

/* macro for doing assertions, see "Writing Solid Code" */
#ifdef DEBUG
    extern void _Assert(char *, unsigned);
    #define ASSERT(f) \
        if (f)  \
            {/*EMPTY*/} \
        else    \
            _Assert(__FILE__,__LINE__)
#else
    #define ASSERT(f)
#endif


#define _INCL_DEBUG_H
#endif
