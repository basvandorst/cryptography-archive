/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/
 
/*****************************************************************
*
* Debugging routines for various stuff
*
*****************************************************************/

#include "debug.h"

/* this is so there is at least a line for compiling to get away
   from the compiler warnings when not using DEBUG */
typedef int EMPTY_FILE;

#ifdef DEBUG

/*  This procedure will print out in hex what's in the buffer.
    It only bothers to print the first 8 bytes...
 */
void _PrintBytes (
unsigned char *buffer,
unsigned int bufferLen)
{
    if (bufferLen == 1)
        printf("%02x",buffer[0]);
    else if (bufferLen == 2)
        printf("%02x %02x",buffer[0],buffer[1]);
    else if (bufferLen == 3)
        printf("%02x %02x %02x",buffer[0],buffer[1],buffer[2]);
    else
        printf("%02x %02x %02x %02x %02x %02x %02x %02x",buffer[0],
               buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
               buffer[6], buffer[7]);

} /* _PrintBytes */


/* Deal with assertions, see "Writing Solid Code"
 */
void _Assert(char *strFile, unsigned uLine)
{
    (void)fflush((FILE *)0);
    (void)fprintf(stderr,"\nAssertion failed: %s, line %u\n", strFile, uLine);
    (void)fflush(stderr);
    abort();
} /* _Assert */

#endif /* DEBUG */