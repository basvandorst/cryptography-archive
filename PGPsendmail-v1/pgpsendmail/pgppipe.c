/*  pgppipe.c

    Source file for  pgppipe  (decryption programme).

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

/*  This programme will decrypt messages piped into it and will send the
    decrypted message to it's standard output. It uses PGPdaemon to perform
    the decryption.


    Written by      Richard Gooch   14-JUL-1994

    Updated by      Richard Gooch   15-JUL-1994: Added code to pass
  non-encrypted blocks of data straight through.

    Updated by      Richard Gooch   3-DEC-1994: Took account of changes to
  connect_to_pgpdaemon  .

    Last updated by Richard Gooch   7-DEC-1994: Inserted PGP message header
  line if PGPdaemon not contactable.


*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "pgpsendmail.h"


void main (argc, argv, envp)
int argc;
char *argv[];
char *envp[];
{
    flag bool_val;
    flag pgp_data;
    int to_fd, from_fd;
    int child_pid, status;
    int len;
    char *pgppath;
    char line[STRING_LENGTH];
    char *pgp_start_line = "-----BEGIN PGP MESSAGE-----\n";
    char *pgp_end_line = "-----END PGP MESSAGE-----\n";
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    if ( ( pgppath = getenv ("PGPPATH") ) == NULL )
    {
	/*  Plain copy  */
	(void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	exit (RV_OK);
    }
    /*  Loop through lines  */
    pgp_data = FALSE;
    while ( read_line_fd (INPUT_FD, line, STRING_LENGTH, FALSE) )
    {
	/*  Got a line  */
	len = strlen (line);
	if (pgp_data)
	{
	    /*  Copy straight out to PGPdaemon  */
	    if (write (to_fd, line, len) < len)
	    {
		(void) fprintf (stderr, "Error writing data\t%s\n",
				sys_errlist[errno]);
		(void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
		(void) kill (child_pid, SIGKILL);
		(void) waitpid (child_pid, &status, 0);
		(void) close (to_fd);
		exit (RV_OK);
	    }
	    if (strcmp (line, pgp_end_line) == 0)
	    {
		/*  Tell PGPdaemon the end of the message has arrived  */
		if (write (to_fd, "\0", 1) < 1)
		{
		    (void) fprintf (stderr,
				    "Error copying data to PGPdaemon\t%s\n",
				    sys_errlist[errno]);
		    (void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
		    (void) kill (child_pid, SIGKILL);
		    (void) waitpid (child_pid, &status, 0);
		    (void) close (to_fd);
		    exit (RV_OK);
		}
		(void) close (to_fd);
		/*  Reap child  */
		(void) waitpid (child_pid, &status, 0);
		(void) fprintf (stdout,
				"\nEnd automatically decrypted message\n");
		(void) fflush (stdout);
		pgp_data = FALSE;
	    }
	    continue;
	}
	/*  No existing decryption  */
	if (strcmp (line, pgp_start_line) != 0)
	{
	    /*  Not start of PGP message: plain copy  */
	    if (write (OUTPUT_FD, line, len) < len)
	    {
		(void) fprintf (stderr, "Error writing data\t%s\n",
				sys_errlist[errno]);
		(void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
		exit (RV_OK);
	    }
	    continue;
	}
	/*  Start of PGP message  */
	if ( !connect_to_pgpdaemon (pgppath, &to_fd, &from_fd) )
	{
	    (void) fprintf (stderr, "Could not connect to PGPdaemon\n");
	    (void) fputs (pgp_start_line, stdout);
	    (void) fflush (stdout);
	    (void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	    exit (RV_OK);
	}
	if (write (to_fd, "DECRYPT\n", 8) < 8)
	{
	    (void) fprintf (stderr, "Error writing request code\t%s\n",
			    sys_errlist[errno]);
	    (void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	    exit (RV_OK);
	}
	/*  Check if OK so far  */
	if (read (from_fd, &bool_val, sizeof bool_val) < sizeof bool_val)
	{
	    (void) fprintf (stderr,
			    "Error reading response flag from PGPdaemon\t%s\n",
			    sys_errlist[errno]);
	    (void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	    exit (RV_OK);
	}
	if (!bool_val)
	{
	    (void) fprintf (stderr,
			    "PGPdaemon refused to decrypt: probably has no passphrase\n");
	    (void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	    exit (RV_OK);
	}
	(void) fprintf (stdout,
			"pgppipe %s: automatically decrypted message:\n\n",
			VERSION);
	(void) fflush (stdout);
	if (write (to_fd, line, len) < len)
	{
	    (void) fprintf (stderr, "Error writing data\t%s\n",
			    sys_errlist[errno]);
	    (void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	    (void) close (to_fd);
	    (void) close (from_fd);
	    exit (RV_OK);
	}
	/*  Now have to copy stdin to PGPdaemon input and PGPdaemon output to
	    stdout  */
	/*  Setup a child process. Parent copies stdin to PGPdaemon.
	    Child copies PGPdaemon output to stdout  */
	switch ( child_pid = fork () )
	{
	  case 0:
	    /*  Child  */
	    (void) close (to_fd);
	    if ( !copy_data (OUTPUT_FD, from_fd, FALSE) )
	    {
		(void) fprintf (stderr, "Error copying data from PGPdaemon\n");
		(void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
		exit (RV_OK);
	    }
	    /*  Done  */
	    (void) close (from_fd);
	    exit (RV_OK);
/*
	    break;
*/
	  case -1:
	    /*  Error  */
	    (void) fprintf (stderr, "Could not fork\t%s\n",
			    sys_errlist[errno]);
	    (void) copy_data (OUTPUT_FD, INPUT_FD, FALSE);
	    exit (RV_OK);
/*
	    break;
*/
	  default:
	    /*  Parent  */
	    pgp_data = TRUE;
	    (void) close (from_fd);
	    break;
	}
    }
    if (pgp_data) (void) close (to_fd);
    exit (RV_OK);
}   /*  End Function main  */
