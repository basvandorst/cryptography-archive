/*  header.c

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

/*  This file contains routines to extract email addresses from a mail header.


    Written by      Richard Gooch   26-SEP-1997

    Updated by      Richard Gooch   27-SEP-1997

    Last updated by Richard Gooch   1-OCT-1997: Fixed problem with lines like:
  "username, username (Real Name [,Organisation])".


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
STATIC_FUNCTION (char *get_address,
		 (decode_type *info, char error[STRING_LENGTH]) );


/*  Public functions follow  */

char *get_next_address (decode_type *info, char error[STRING_LENGTH])
/*  [SUMMARY] Get the next address from a mail header.
    <info> A pointer to the decode_info structure. The line buffer is modified.
    <error> The error string is written here. If no error was generated the
    first character will be '\0'.
    [RETURNS] A pointer to an email address on success, else NULL indicating no
    more addresses on this line or error.
*/
{
    char *ptr, *addr;

    error[0] = '\0';
    if (info->next_pos == 0)
    {
	/*  Remove trailing whitespace  */
	for (ptr = info->linebuf + info->line_length - 1;
	     isspace (*ptr) && (ptr > info->linebuf);
	     --ptr) *ptr = '\0';
	info->line_length = ptr - info->linebuf + 1;
    }
    if ( info->continuation_line || (info->next_pos > 0) )
    {
	return get_address (info, error);
    }
    /*  Start of a new line  */
    info->continuation_line = FALSE;
    if ( info->scan_to && (st_nicmp (info->linebuf, "to:", 3) == 0) )
	info->next_pos = 3;
    else if ( info->scan_cc && (st_nicmp (info->linebuf, "cc:", 3) == 0) )
	info->next_pos = 3;
    else if ( info->scan_bcc && (st_nicmp (info->linebuf, "bcc:", 4) == 0) )
	info->next_pos = 4;
    else if ( info->scan_from && (st_nicmp (info->linebuf, "from:", 5) == 0) )
	info->next_pos = 5;
    else if ( info->scan_from && (st_nicmp (info->linebuf, "from ", 5) == 0) )
    {
	for (addr = info->linebuf + 5; isspace (*addr) && (*addr != '\0');
	     ++addr);
	if (*addr == '\0')
	{	
	    fprintf (stderr, "Bad address: \"%s\"\n", info->linebuf);
	    strcpy (error, "bad address");
	    return (NULL);
	}
	for (ptr = addr; !isspace (*ptr) && (*ptr != '\0'); ++ptr);
	*ptr = '\0';
	info->next_pos = info->line_length;
	return (addr);
    }
    else if ( info->scan_sender && (st_nicmp (info->linebuf, "sender:", 7)
				    == 0) )
	info->next_pos = 7;
    else return (NULL);
    return get_address (info, error);
}   /*  End Function get_next_address  */

flag read_line (int fd, decode_type *info, FILE *debug_fp, flag *eof)
/*  [SUMMARY] Read a line from a mail header.
    <fd> The file descriptor to read from.
    <info> A pointer to the decode_info structure.
    <debug_fp> Debugging information will be written here. If this is NULL, no
    debugging information is written.
    <eof> TRUE is written here on EndOfFile.
    [RETURNS] TRUE on success or EndOfFile, else FALSE.
*/
{
    char ch;
    int new_len;
    char *tmp;

    info->line_length = 0;
    info->next_pos = 0;
    *eof = FALSE;
    while (read (fd, &ch, 1) == 1)
    {
	if (info->line_length >= info->linebuf_size)
	{
	    /*  Need a bigger buffer  */
	    new_len = (info->linebuf_size <
		       BUF_LEN) ? BUF_LEN : info->linebuf_size * 2;
	    if ( ( tmp = m_alloc (new_len) ) == NULL )
	    {
		if (debug_fp != NULL)
		{
		    fprintf (debug_fp,
			     "read_line: error allocating: %d bytes\n",
			     new_len);
		    fflush (debug_fp);
		}
		fprintf (stderr, "Error allocating: %d bytes\n", new_len);
		return (FALSE);
	    }
	    if (info->linebuf_size > 0)
	    {
		m_copy (tmp, info->linebuf, info->linebuf_size);
		m_free (info->linebuf);
	    }
	    info->linebuf = tmp;
	    info->linebuf_size = new_len;
	}
	/*  Have space in buffer  */
	if (ch == '\n') ch = '\0';
	info->linebuf[info->line_length] = ch;
	if (ch == '\0') return (TRUE);
	++info->line_length;
    }
    if (debug_fp != NULL)
    {
	fprintf (debug_fp, "read_line: EndOfFile\n");
	fflush (debug_fp);
    }
    *eof = TRUE;
    return (TRUE);
}   /*  End Function read_line  */


/*  Private functions follow  */

static char *get_address (decode_type *info, char error[STRING_LENGTH])
/*  [SUMMARY] Get the next address from a mail header.
    <info> A pointer to the decode_info structure. The line buffer is modified.
    <error> The error string is written here. If no error was generated the
    first character will be '\0'.
    [RETURNS] A pointer to an email address on success, else NULL indicating no
    more addresses on this line or error.
*/
{
    flag comma = FALSE;
    char *ptr1, *ptr2, *addr;

    error[0] = '\0';
    if (info->next_pos >= info->line_length) return (NULL);
    addr = info->linebuf + info->next_pos;
    /*  Check for trailing ','  */
    info->continuation_line = (info->linebuf[info->line_length - 1]
			       == ',') ? TRUE : FALSE;
    /*  Skip leading whitespace and ',' characters  */
    while ( ( isspace (*addr) || (*addr == ',') ) && (*addr != '\0' ) ) ++addr;
    if (*addr == '\0')
    {
	info->next_pos = info->line_length;
	return (NULL);
    }
    /*  First handle address formats with a leading '"'  */
    if (*addr == '\"')
    {
	/*  Address formats:
		"Real Name"@email.address
	        "Real Name" <username@email.address>
		"Real Name"\n<username@email.address>
	*/
	if ( ( ptr1 = strchr (addr + 1, '\"') ) == NULL )
	{
	    /*  Stuffed address  */
	    fprintf (stderr, "Bad address (missing close quote): \"%s\"\n",
		     addr);
	    strcpy (error, "bad address");
	    return (NULL);
	}
	if (ptr1[1] == '@')
	{
	    /*  Address format:
		"Real Name"@email.address
	    */
	    while ( !isspace (*ptr1) && (*ptr1 != '\0') && (*ptr1 != ',') )
		++ptr1;
	    if (*ptr1 != '\0') *ptr1++ = '\0';
	    info->next_pos = ptr1 - info->linebuf;
	    return (addr);
	}
	/*  Address format:
	        "Real Name" <username@email.address>
		"Real Name"\n<username@email.address>
	*/
	/*  Discard bit in quotes  */
	for (addr = ptr1 + 1; isspace (*addr) && (*addr != '\0'); ++addr);
	if (*addr == '\0')
	{
	    /*  Address format:
		    "Real Name"\n<username@email.address>
	    */
	    /*  Leave it for later  */
	    info->continuation_line = TRUE;
	    info->next_pos = info->line_length;
	    return (NULL);
	}
	if (*addr != '<')
	{
	    /*  Stuffed address  */
	    fprintf (stderr, "Bad address (missing '<'): \"%s\"\n", addr);
	    strcpy (error, "bad address");
	    return (NULL);
	}
	++addr;
	if ( ( ptr1 = strchr (addr, '>') ) == NULL )
	{
	    /*  Stuffed address  */
	    fprintf (stderr, "Bad address: \"%s\"\n", addr);
	    strcpy (error, "bad address");
	    return (NULL);
	}
	/*  Address format:
	        "Real Name" <username@email.address>
	*/
	*ptr1++ = '\0';
	info->next_pos = ptr1 - info->linebuf;
	return (addr);
    }
    if ( ( ptr1 = strchr (addr, '<') ) != NULL )
    {
	/*  Address formats:
	        Real Name <username@email.address>
	        <username@email.address>
	*/
	if ( ( ptr2 = strchr (addr, '>') ) == NULL )
	{
	    /*  Stuffed address  */
	    fprintf (stderr, "Bad address: \"%s\"\n", addr);
	    strcpy (error, "bad address");
	    return (NULL);
	}
	*ptr2++ = '\0';
	info->next_pos = ptr2 - info->linebuf;
	return (ptr1 + 1);
    }
    /*  Address formats:
	    username (Real Name [,Organisation])
	    username@email.address (Real Name [,Organisation])
	    username
	    username@email.address
    */
    /*  Skip past address until first whitespace, ',' or terminator  */
    for (ptr1 = addr + 1;
	 !isspace (*ptr1) && (*ptr1 != ',') && (*ptr1 != '\0');
	 ++ptr1);
    if ( isspace (*ptr1) )
    {
	*ptr1 = '\0';
	for (++ptr1; isspace (*ptr1); ++ptr1);
    }
    if ( (*ptr1 == ',') || (*ptr1 == '\0') )
    {
	/*  Address formats:
	        username
	        username@email.address
	*/
	if (*ptr1 != '\0') *ptr1++ = '\0';
	info->next_pos = ptr1 - info->linebuf;
	return (addr);
    }
    if (*ptr1 == '(')
    {
	/*  Address formats:
	        username (Real Name [,Organisation])
	        username@email.address (Real Name [,Organisation])
	*/
	if ( ( ptr2 = strchr (ptr1, ')') ) == NULL )
	{
	    /*  Stuffed address  */
	    fprintf (stderr, "Bad address: \"%s\"\n", addr);
	    strcpy (error, "bad address");
	    return (NULL);
	}
	/*  Go backwards and skip trailing spaces  */
	*ptr1-- = '\0';
	while ( isspace (*ptr1) && (ptr1 > addr) ) *ptr1-- = '\0';
	*ptr2++ = '\0';
	info->next_pos = ptr2 - info->linebuf;
	return (addr);
    }
    /*  Stuffed address  */
    fprintf (stderr, "Bad address: \"%s\"\n", addr);
    strcpy (error, "bad address");
    return (NULL);
}   /*  End Function get_address  */


/*  Valid address formats:

    "Real Name"@email.address
    "Real Name" <username@email.address>
    "Real Name"\n<username@email.address>
    Real Name <username@email.address>
    <username@email.address>
    username@email.address
    username
    username (Real Name [,Organisation])
    username@email.address (Real Name [,Organisation])
*/
