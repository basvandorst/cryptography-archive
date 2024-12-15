/*LINTLIBRARY*/
/*PREFIX:"m_"*/
/*   memory.c

    This code provides simple memory manipulation routines.

    Copyright (C) 1992,1993,1994  Richard Gooch

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Richard Gooch may be reached by email at  rgooch@atnf.csiro.au
    The postal address is:
      Richard Gooch, c/o ATNF, P. O. Box 76, Epping, N.S.W., 2121, Australia.
*/

/*  This file contains various memory operation routines for clearing, copying
    and comparing memory.


    Written by      Richard Gooch   5-JUL-1994

    Updated by      Richard Gooch   5-JUL-1994: Extracted from the
  Karma  source/lib/karma/level1/memory.c  file

    Last updated by Richard Gooch   15-OCT-1994: Fixed bug in  m_clear  which
  could result in misaligned accesses (changed in Karma and merged back in).


*/
#include <stdio.h>
#include <karma.h>
#include "pgpsendmail.h"


/*PUBLIC_FUNCTION*/
void m_clear (memory, length)
/*  This routine will clear a block of memory pointed to by  memory  .
    The length of the block (in chars) to clear must be given by  length  .
    The memory is cleared in long integers and chars.
    The routine returns nothing.
*/
char *memory;
unsigned int length;
{
    uaddr front_pad;
    uaddr long_count;
    uaddr char_count;
    uaddr num_long;
    uaddr num_char;
    unsigned long *long_ptr;
    unsigned char *char_ptr;
    static char function_name[] = "m_clear";

    if (memory == NULL)
    {
	(void) fprintf (stderr, "NULL pointer passed\n");
	a_prog_bug (function_name);
    }
    if (length < 1)
    {
	return;
    }
    /*  First clear stray bytes at start of block  */
    front_pad = (uaddr) memory % sizeof (unsigned long);
    front_pad = (front_pad > 0) ? (sizeof (unsigned long) - front_pad) : 0;
    for (char_count = front_pad; (char_count > 0) && (length > 0);
	 --char_count, --length)
	*memory++ = (char) NULL;
    num_long = length / sizeof (unsigned long);
    num_char = length - num_long * sizeof (unsigned long);
    long_ptr = (unsigned long *) memory;
    for (long_count = 0; long_count < num_long; long_count++)
        *long_ptr++ = (unsigned long) NULL;
    char_ptr = (unsigned char *) long_ptr;
    for (char_count = 0; char_count < num_char; ++char_count)
        *char_ptr++ = (unsigned char) NULL;
}   /*  End Function m_clear  */

/*PUBLIC_FUNCTION*/
void m_copy (dest, source, length)
/*  This routine will copy a block of memory pointed to by  source  to the
    block of memory pointed to by  dest  .
    The number of chars to transfer must be given by  length  .If this is zero,
    it is permissable for  source  and  dest  to be NULL.
    The routine copies long integers and chars.
    The routine returns nothing.
*/
char *dest;
CONST char *source;
unsigned int length;
{
    unsigned int long_count;
    unsigned int char_count;
    unsigned int num_long;
    unsigned int num_char;
    unsigned long *source_long;
    unsigned long *dest_long;
    unsigned char *source_char;
    unsigned char *dest_char;
    static char function_name[] = "m_copy";

    if (length < 1)
    {
	return;
    }
    if (dest == NULL)
    {
	(void) fprintf (stderr, "NULL destination pointer passed\n");
	a_prog_bug (function_name);
    }
    if (source == NULL)
    {
	(void) fprintf (stderr, "NULL source pointer passed\n");
	a_prog_bug (function_name);
    }
    if ( (unsigned int) dest % sizeof (unsigned long) !=
	(unsigned int) source % sizeof (unsigned long) )
    {
	/*  Blocks have different misalignment: byte copy  */
	for (; length > 0; --length)
	    *dest++ = *source++;
	return;
    }
    /*  First copy stray bytes at start of blocks  */
    for (char_count = sizeof (unsigned long) -
	 (unsigned int) dest % sizeof (unsigned long);
	 (char_count > 0) && (length > 0);
	 --char_count, --length)
	*dest++ = *source++;
    num_long = length / sizeof (unsigned long);
    num_char = length - num_long * sizeof (unsigned long);
    source_long = (unsigned long *) source;
    dest_long = (unsigned long *) dest;
    for (long_count = 0; long_count < num_long; ++long_count)
        *dest_long++ = *source_long++;
    source_char = (unsigned char *) source_long;
    dest_char = (unsigned char *) dest_long;
    for (char_count = 0; char_count < num_char; ++char_count)
        *dest_char++ = *source_char++;
}   /*  End Function m_copy   */
