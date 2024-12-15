/*  pgppipe.c

    Source file for  pgppipe  (decryption programme).

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

/*  This programme will decrypt messages piped into it and will send the
    decrypted message to it's standard output. It uses PGPdaemon to perform
    the decryption.


    Written by      Richard Gooch   14-JUL-1994: Original pgppipe code.

    Updated by      Anthony Towns   18-JUN-1996: Copied to pgpsign.c and
  modified.

    Last updated by Richard Gooch   30-SEP-1997: Various cleanups. Inclusion in
  PGPsendmail/Unix suite.


*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pgpsendmail.h>
#include <version.h>


void main (argc, argv, envp)
int argc;
char *argv[];
char *envp[];
{
    flag bool_val;
    int to_fd, from_fd;
    int child_pid, status;
    int len;
    char *pgppath;
    char line[STRING_LENGTH];

    if ( ( pgppath = getenv ("PGPPATH") ) == NULL )
    {
	/*  Plain copy  */
	copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	exit (RV_OK);
    }
    if ( !connect_to_pgpdaemon (pgppath, &to_fd, &from_fd) )
    {
	fprintf (stderr, "Could not connect to PGPdaemon\n");
	copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	exit (RV_OK);
    }
    if (write (to_fd, "SIGN\n", 5) < 5)
    {
	fprintf (stderr, "Error writing request code\t%s\n", ERRSTRING);
	copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	exit (RV_OK);
    }
    /*  Check if OK so far  */
    if (read (from_fd, &bool_val, sizeof bool_val) < sizeof bool_val)
    {
	fprintf (stderr, "Error reading response flag from PGPdaemon\t%s\n",
		 ERRSTRING);
	copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	exit (RV_OK);
    }
    if (!bool_val)
    {
	fprintf (stderr,
		 "PGPdaemon refused to decrypt: probably has no passphrase\n");
	copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	exit (RV_OK);
    }
    fprintf (stdout, "pgpsign %s: automatically signed message:\n\n", VERSION);
    fflush (stdout);
    /*  Now have to copy stdin to PGPdaemon input and PGPdaemon output to
	stdout  */
    /*  Setup a child process. Parent copies stdin to PGPdaemon.
	Child copies PGPdaemon output to stdout  */
    switch ( child_pid = fork () )
    {
      case 0:
	/*  Child  */
	close (to_fd);
	if ( !copy_data (OUTPUT_FD, from_fd, FALSE) )
	{
	    fprintf (stderr, "Error copying data from PGPdaemon\n");
	    copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	    exit (RV_OK);
	}
	/*  Done  */
	close (from_fd);
	exit (RV_OK);
      case -1:
	/*  Error  */
	fprintf (stderr, "Could not fork\t%s\n", ERRSTRING);
	copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	exit (RV_OK);
      default:
	/*  Parent  */
	close (from_fd);
	break;
    }
    /*  Loop through lines  */
    while ( read_line_fd (INPUT_FD, line, STRING_LENGTH, FALSE) )
    {
	/*  Got a line  */
	len = strlen (line);
	/*  Copy straight out to PGPdaemon  */
	if (write (to_fd, line, len) < len)
	{
	    fprintf (stderr, "Error writing data\t%s\n", ERRSTRING);
	    copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	    kill (child_pid, SIGKILL);
	    waitpid (child_pid, &status, 0);
	    close (to_fd);
	    exit (RV_OK);
	}
    }
    write (to_fd, "\0", 1);
    close (to_fd);
    waitpid (child_pid, &status, 0);
    exit (RV_OK);
}   /*  End Function main  */
