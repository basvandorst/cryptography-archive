/*  misc.c

    Source file for miscellaneuos operations for the  PGPsendmail/Unix Suite

    Copyright (C) 1994-1997  Richard Gooch

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

/*  This file contains miscellaneous routines.


    Written by      Richard Gooch   14-JUL-1994: Copied routines from  spawn.c
  and  pgpdaemon.c

    Updated by      Richard Gooch   14-JUL-1994

    Updated by      Richard Gooch   16-JUL-1994: Created  scrub_file  .

    Updated by      Richard Gooch   3-DEC-1994: Cleared buffer in  copy_data
  after use.

    Last updated by Richard Gooch   25-SEP-1997: Used new ERRSTRING macro.


*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include "pgpsendmail.h"

#define BUF_SIZE 16384


/*  Private functions  */


/*  Public functions follow  */

flag copy_data (out_fd, in_fd, return_on_terminator)
/*  This routine will copy data from one file descriptor to another, until the
    input closes (EndOfFile is reached).
    The output file descriptor must be given by  out_fd  .
    The input file descriptor must be given by  in_fd  .
    If the value of  return_on_terminator  is TRUE, then the routine will
    return when a string terminator '\0' character is received.
    The routine returns TRUE on success, else it returns FALSE.
*/
int out_fd;
int in_fd;
flag return_on_terminator;
{
    int len;
    int count = -1;
    char buffer[BUF_SIZE];

    while ( ( len = read (in_fd, buffer, BUF_SIZE) ) > 0 )
    {
	if (return_on_terminator)
	{
	    for (count = len - 1; (buffer[count] != '\0') && (count >= 0);
		 --count);
	    if (count >= 0) len = count;
	}
	if (write (out_fd, buffer, len) < len)
	{
	    fprintf (stderr, "Error writing data\t%s\n", ERRSTRING);
	    m_clear (buffer, BUF_SIZE);
	    return (FALSE);
	}
	if (count >= 0)
	{
	    m_clear (buffer, BUF_SIZE);
	    return (TRUE);
	}
    }
    m_clear (buffer, BUF_SIZE);
    if (len == 0) return (TRUE);
    fprintf (stderr, "Error reading data\t%s\n", ERRSTRING);
    return (TRUE);
}   /*  End Function copy_data  */

int set_env (env_name, env_value)
/*  This routine will set an environment variable. Because different platforms
    have different C library calls to do this: I do it myself.
    The environment variable to create or change must be named by  env_name  .
    The string value to set the variable to must be pointed to by  env_value  .
    The routine returns 0 on success, else it returns -1.
*/
CONST char *env_name;
CONST char *env_value;
{
    int num_strings;
    char *str, **env;
    char env_string[STRING_LENGTH];
    extern char **environ;
    static char **old_environ = NULL;

    sprintf (env_string, "%s=%s", env_name, env_value);
    if ( ( str = m_alloc (strlen (env_string) + 1) ) == NULL )
    {
	fprintf (stderr, "Error allocating string\n");
	return (-1);
    }
    strcpy (str, env_string);
    for (num_strings = 0; environ[num_strings] != NULL; ++num_strings);
    if ( ( env = (char **) m_alloc ( sizeof *env * (num_strings + 2) ) )
	== NULL )
    {
	m_free (str);
	fprintf (stderr, "Error allocating environment\n");
	return (-1);
    }
    m_copy ( (char *) env, (char *) environ, num_strings * sizeof *env );
    env[num_strings++] = str;
    env[num_strings] = NULL;
    if (old_environ == environ) m_free ( (char *) environ );
    environ = env;
    old_environ = env;
    return (0);
}   /*  End Function set_env  */

flag read_line_fd (fd, buffer, length, discard_newline)
/*  This routine will read a character string from a file into a buffer.
    The file descriptor must be given by  fd  .
    The buffer to write the data to must be pointed to by  buffer  .
    The routine will write a NULL terminator character at the end of the
    string.
    If the value of  discard_newline  is TRUE, the newline chanacter '\n' is
    NOT copied into the buffer.
    The length of the buffer must be given by  length  .If the buffer is not
    large enough to contain the string, then the remainder of the string is NOT
    read.
    The routine returns TRUE on success,
    else it returns FALSE (indicating end-of-file was encountered).
*/
int fd;
char *buffer;
unsigned int length;
flag discard_newline;
{
    flag return_value = TRUE;
    flag another = TRUE;

    while (another)
    {
	if (read (fd, buffer, 1) < 1)
	{
	    /*  Error occurred  */
	    another = FALSE;
	    return_value = FALSE;
	    continue;
	}
	if (*buffer == '\n')
	{
	    another = FALSE;
	    if (discard_newline) continue;
	}
	++buffer;
	--length;
	if (length < 2)
	{
	    another = FALSE;
	}
    }
    /*  Write NULL terminator  */
    *buffer = '\0';
    return (return_value);
}   /*  End Function read_line_fd  */

flag scrub_file (filename, remove)
/*  This routine will scrub (overwrite with zeros) the contents of a file and
    (optionally) remove it.
    The filename must be given by  filename  .
    The routine will remove the file if the value of  remove  is TRUE.
    The routine returns TRUE on success, else it returns FALSE.
*/
char *filename;
flag remove;
{
    int fd, filelen, blocklen;
    struct stat statbuf;
    char buf[BUF_SIZE];

    m_clear (buf, BUF_SIZE);
    if ( ( fd = open (filename, O_RDWR, 0) ) < 0 )
    {
	fprintf (stderr, "Error opening file: \"%s\"\t%s\n",
		 filename, ERRSTRING);
	return (FALSE);
    }
    if (fstat (fd, &statbuf) != 0)
    {
	fprintf (stderr, "Error stat'ing file: \"%s\"\t%s\n",
		 filename, ERRSTRING);
	close (fd);
	return (FALSE);
    }
    for (filelen = statbuf.st_size; filelen > 0; filelen -= blocklen)
    {
	blocklen = (filelen > BUF_SIZE) ? BUF_SIZE : filelen;
	if (write (fd, buf, blocklen) < blocklen)
	{
	    fprintf (stderr, "Error scrubbing file: \"%s\" data\t%s\n",
		     filename, ERRSTRING);
	    close (fd);
	    return (FALSE);
	}
    }
    if (fsync (fd) != 0)
    {
	fprintf (stderr, "Error fsync'ing file: \"%s\"\t%s\n",
		 filename, ERRSTRING);
	close (fd);
	return (FALSE);
    }
    close (fd);
    if (!remove) return (TRUE);
    if (unlink (filename) != 0)
    {
	fprintf (stderr, "Error deleting file: \"%s\"\t%s\n",
		 filename, ERRSTRING);
	return (FALSE);
    }
    return (TRUE);
}   /*  End Function scrub_file  */

flag search_file (CONST char *filename, CONST char **strings, FILE *debug_fp,
		  flag *found)
/*  [SUMMARY] Search a file for one or more strings.
    <filename> The filename to search.
    <strings> The NULL-terminated array of strings.
    <debug_fp> Debugging information will be written here. If this is NULL no
    debugging information is written.
    <found> If a match is found, the value TRUE is written here, else FALSE is
    written.
    [RETURNS] TRUE on success, else FALSE.
*/
{
    flag eof;
    int err;
    regex_t pbuf;
    regmatch_t mbuf;
    char *ptr;
    CONST char **addr;
    FILE *fp;
    char buffer[STRING_LENGTH];
    static char function_name[] = "search_file";

    *found = FALSE;
    if ( ( fp = fopen (filename, "r") ) == NULL )
    {
	if (debug_fp != NULL)
	    fprintf (debug_fp, "%s: error opening file: \"%s\"\t%s\n",
		     function_name, filename, ERRSTRING);
	return (FALSE);
    }
    while (TRUE)
    {
	if ( !read_fp_line (fp, buffer, STRING_LENGTH, debug_fp, &eof) )
	{
	    if (debug_fp != NULL)
		fprintf (debug_fp, "%s: error reading file: \"%s\"\t%s\n",
			 function_name, filename, ERRSTRING);
	    return (FALSE);
	}
	if (eof) break;
	if (buffer[0] != '!')
	{
	    for (addr = strings; *addr != NULL; ++addr)
	    {
		if (st_icmp (*addr, buffer) == 0)
		{
		    *found = TRUE;
		    return (TRUE);
		}
	    }
	    continue;
	}
	/*  Something special  */
	if (st_nicmp (buffer, "!include ", 9) == 0)
	{
	    for (ptr = buffer + 9; isspace (*ptr) && (*ptr != '\0'); ++ptr);
	    if (*ptr == '\0')
	    {
		if (debug_fp != NULL)
		    fprintf (debug_fp, "Illegal line: \"%s\"\n", buffer);
		return (FALSE);
	    }
	    if ( !search_file (ptr, strings, debug_fp, found) ) return (FALSE);
	    if (*found) return (TRUE);
	    continue;
	}
	if (st_nicmp (buffer, "!regexp ", 8) == 0)
	{
	    for (ptr = buffer + 8; isspace (*ptr) && (*ptr != '\0'); ++ptr);
	    if (*ptr == '\0')
	    {
		if (debug_fp != NULL)
		    fprintf (debug_fp, "Illegal line: \"%s\"\n", buffer);
		return (FALSE);
	    }
	    if ( ( err = regcomp (&pbuf, ptr, REG_ICASE) ) != 0 )
	    {
		regerror (err, &pbuf, buffer, STRING_LENGTH);
		if (debug_fp != NULL)
		    fprintf (debug_fp, "Error compiling regexp: \"%s\"\t%s\n",
			     ptr, buffer);
		return (FALSE);
	    }
	    for (addr = strings; *addr != NULL; ++addr)
	    {
		if (regexec (&pbuf, *addr, 1, &mbuf, 0) == 0)
		{
		    *found = TRUE;
		    return (TRUE);
		}
	    }
	    regfree (&pbuf);
	    continue;
	}
    }
    /*  No match found  */
    fclose (fp);
    return (TRUE);
}   /*  End Function search_file  */

flag read_fp_line (FILE *fp, char *buffer, unsigned int buf_len,
		   FILE *debug_fp, flag *eof)
/*  [SUMMARY] Read a line from a file, ignoring commented lines.
    <fp> The file to read from.
    <buffer> The buffer to write the line to.
    <buf_len> The length of the buffer.
    <eof> If the EndOfFile is reached, the value TRUE is written here.
    [RETURNS] TRUE on success, else FALSE indicating error.
*/
{
    int len;

    *eof = FALSE;
    while (fgets (buffer, buf_len - 1, fp) != NULL)
    {
	buffer[buf_len - 1] = '\0';
	len = strlen (buffer);
	if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';
	if (buffer[0] == '#') continue;
	return (TRUE);
    }
    if (errno == 0)
    {
	*eof = TRUE;
	return (TRUE);
    }
    if (debug_fp != NULL)
	fprintf (debug_fp, "Error reading file\t%s\n", ERRSTRING);
    return (FALSE);
}   /*  End Function read_fp_line  */


/*  Private functions follow  */
