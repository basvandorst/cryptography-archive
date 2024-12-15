/*  extract-addrs.c

    Source file for extracting email addresses from a header.

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

/*  This file is a utility programme to extract email addresses from a mail
    header.


    Written by      Richard Gooch   27-SEP-1997

    Last updated by Richard Gooch   27-SEP-1997


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
    flag eof;
    int count;
    decode_type info;
    char *ptr;
    char error[STRING_LENGTH];
    static char usage_string[] = "Usage:\textract-addrs [tcbfs]";

    m_clear ( (char *) &info, sizeof info );
    if (argc == 1)
    {
	info.scan_from = TRUE;
	info.scan_sender = TRUE;
    }
    for (--argc, ++argv; argc > 0; --argc, ++argv)
    {
	for (ptr = *argv; *ptr != '\0'; ++ptr)
	{
	    switch (*ptr)
	    {
	      case 't':
	      case 'T':
		info.scan_to = TRUE;
		break;
	      case 'c':
	      case 'C':
		info.scan_cc = TRUE;
		break;
	      case 'b':
	      case 'B':
		info.scan_bcc = TRUE;
		break;
	      case 'f':
	      case 'F':
		info.scan_from = TRUE;
		break;
	      case 's':
	      case 'S':
		info.scan_sender = TRUE;
		break;
	      default:
		fprintf (stderr, "Illegal option: %c\n", *ptr);
		exit (1);
		/*break;*/
	    }
	}
    }
    while ( read_line (INPUT_FD, &info, NULL, &eof) )
    {
	if ( eof || (info.line_length < 1) )
	{
	    fflush (stdout);
	    exit (0);
	}
	while ( ( ptr = get_next_address (&info, error) ) != NULL )
	{
	    puts (ptr);
	}
	if (error[0] == '\0') continue;
	fprintf (stderr, "Error decoding, error: \"%s\"\n", error);
	exit (1);
    }
    fprintf (stderr, "Error reading\n");
    exit (1);
}   /*  End Function main  */
