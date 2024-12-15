/*  match-email-addr.c

    Source file for matching email addresses.

    Copyright (C) 1997  Richard Gooch

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Richard Gooch may be reached by email at  rgooch@atnf.csiro.au
    The postal address is:
      Richard Gooch, c/o ATNF, P. O. Box 76, Epping, N.S.W., 2121, Australia.
*/

/*  This file is a utility programme to match email addresses.


    Written by      Richard Gooch   28-SEP-1997

    Last updated by Richard Gooch   1-OCT-1997: Moved <search_file> to misc.c


*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "pgpsendmail.h"


/*  Private functions  */


/*  Public functions follow  */

int main (int argc, char **argv)
{
    flag eof, found;
    int count;
    decode_type info;
    stringlist_type list;
    char *ptr;
    char error[STRING_LENGTH];
    static char usage_string[] = "Usage:\tmatch-mail-addr files...";

    /*  First construct the list of input addresses  */
    m_clear ( (char *) &info, sizeof info );
    m_clear ( (char *) &list, sizeof list );
    while (TRUE)
    {
	if ( !read_line (INPUT_FD, &info, NULL, &eof) )
	{
	    fprintf (stderr, "Error reading\n");
	    exit (1);
	}
	if (eof) break;
	if ( !add_string (&list, info.linebuf, TRUE) ) exit (1);
    }
    /*  Now process each file, checking for matches  */
    for (--argc, ++argv; argc > 0; --argc, ++argv)
    {
	if ( !search_file (*argv, (CONST char **) list.strings, stderr,
			   &found) ) exit (1);
	if (found) exit (0);
    }
    /*  No match found  */
    exit (1);
}   /*  End Function main  */
