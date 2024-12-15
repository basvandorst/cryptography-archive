/*  misc.c

    Source file for miscellaneuos operations for the  PGPsendmail/Unix Suite

    Copyright (C) 1994  Richard Gooch

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

    Last updated by Richard Gooch   3-DEC-1994: Cleared buffer in  copy_data
  after use.


*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pgpsendmail.h"

#define BUF_SIZE 16384


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
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

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
	    (void) fprintf (stderr, "Error writing data\t%s\n",
			    sys_errlist[errno]);
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
    (void) fprintf (stderr, "Error reading data\t%s\n", sys_errlist[errno]);
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

    (void) sprintf (env_string, "%s=%s", env_name, env_value);
    if ( ( str = m_alloc (strlen (env_string) + 1) ) == NULL )
    {
	(void) fprintf (stderr, "Error allocating string\n");
	return (-1);
    }
    (void) strcpy (str, env_string);
    for (num_strings = 0; environ[num_strings] != NULL; ++num_strings);
    if ( ( env = (char **) m_alloc ( sizeof *env * (num_strings + 2) ) )
	== NULL )
    {
	m_free (str);
	(void) fprintf (stderr, "Error allocating environment\n");
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
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    m_clear (buf, BUF_SIZE);
    if ( ( fd = open (filename, O_RDWR, 0) ) < 0 )
    {
	(void) fprintf (stderr, "Error opening file: \"%s\"\t%s\n",
			filename, sys_errlist[errno]);
	return (FALSE);
    }
    if (fstat (fd, &statbuf) != 0)
    {
	(void) fprintf (stderr, "Error stat'ing file: \"%s\"\t%s\n",
			filename, sys_errlist[errno]);
	(void) close (fd);
	return (FALSE);
    }
    for (filelen = statbuf.st_size; filelen > 0; filelen -= blocklen)
    {
	blocklen = (filelen > BUF_SIZE) ? BUF_SIZE : filelen;
	if (write (fd, buf, blocklen) < blocklen)
	{
	    (void) fprintf (stderr, "Error scrubbing file: \"%s\" data\t%s\n",
			    filename, sys_errlist[errno]);
	    (void) close (fd);
	    return (FALSE);
	}
    }
    if (fsync (fd) != 0)
    {
	(void) fprintf (stderr, "Error fsync'ing file: \"%s\"\t%s\n",
			filename, sys_errlist[errno]);
	(void) close (fd);
	return (FALSE);
    }
    (void) close (fd);
    if (!remove) return (TRUE);
    if (unlink (filename) != 0)
    {
	(void) fprintf (stderr, "Error deleting file: \"%s\"\t%s\n",
			filename, sys_errlist[errno]);
	return (FALSE);
    }
    return (TRUE);
}   /*  End Function scrub_file  */
