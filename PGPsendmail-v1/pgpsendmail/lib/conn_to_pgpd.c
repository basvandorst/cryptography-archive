/*  conn_to_pgpd.c

    Source file for conn_to_pgpd  (connect to PGPdaemon).

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

/*  This programme intercepts messages sent by user mail agents to the
    sendmail daemon and checks to see if messages can be encrypted using the
    recipient's PGP public keys.


    Written by      Richard Gooch   5-JUN-1994

    Updated by      Richard Gooch   5-JUN-1994

    Updated by      Richard Gooch   16-JUN-1994: Added tests for existence of
  S_ISSOCK  .

    Updated by      Richard Gooch   17-JUN-1994: Added tests for existence of
  S_IRWXG  S_IRWXO  AF_UNIX  and  SOCK_STREAM  .

    Updated by      Richard Gooch   16-JUL-1994: Added hostname to socket
  filename.

    Updated by      Richard Gooch   15-OCT-1994: Worked around bug in
  Solaris 2.3 which reports Unix sockets as named pipes.

    Updated by      Richard Gooch   3-DEC-1994: Changed to pair of named pipes
  and PID file rather than connecting to a Unix domain socket.

    Last updated by Richard Gooch   25-SEP-1997: Used new ERRSTRING macro.


*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pgpsendmail.h"

/*  Contortion for NEXTSTEP.  */
#ifndef S_IRWXG
#  if defined(S_IRGRP) && defined(S_IWGRP) && defined(S_IXGRP)
#    define S_IRWXG (S_IRGRP|S_IWGRP|S_IXGRP)
#  endif
#endif
#ifndef S_IRWXO
#  if defined(S_IROTH) && defined(S_IWOTH) && defined(S_IXOTH)
#    define S_IRWXO (S_IROTH|S_IWOTH|S_IXOTH)
#  endif
#endif

#if !defined(S_IRWXG) || !defined(S_IRWXO)
    !!!! ERROR !!! *** Cannot determine file protections ****
#endif

flag connect_to_pgpdaemon (CONST char *pgppath, int *to_fd, int *from_fd)
/*  This routine will connect to the PGPdaemon.
    The value of the PGPPATH environment variable must be pointed to by
    pgppath  .
    The descriptor to use when writing to PGPdaemon will be written to the
    storage pointed to by  to_fd  .
    The descriptor to use when reading from PGPdaemon will be written to the
    storage pointed to by  from_fd  .
    The routine returns TRUE on success, else it returns FALSE.
*/
{
    int daemon_pid, randval;
    FILE *fp;
    struct stat statbuf;
    char hostname[STRING_LENGTH];
    char pid_filename[STRING_LENGTH];
    char topipe_filename[STRING_LENGTH];
    char frompipe_filename[STRING_LENGTH];

    if (gethostname (hostname, STRING_LENGTH - 1) != 0)
    {
	fprintf (stderr, "Error getting hostname\t%s\n", ERRSTRING);
	return (FALSE);
    }
    hostname[STRING_LENGTH - 1] = '\0';
    /*  Worry about communication files  */
    sprintf (pid_filename, "%s/.pgpd.PID.%s", pgppath, hostname);
    sprintf (topipe_filename, "%s/.pgpd.input.%s", pgppath, hostname);
    sprintf (frompipe_filename, "%s/.pgpd.output.%s", pgppath,hostname);
    if ( ( fp = fopen (pid_filename, "r") ) == NULL )
    {
	fprintf (stderr, "Error opening: \"%s\"\t%s\n",
		 pid_filename, ERRSTRING);
	return (FALSE);
    }
    if (fscanf (fp, "%d", &daemon_pid) != 1)
    {
	fprintf (stderr, "Error reading: \"%s\"\t%s\n",
		 pid_filename, ERRSTRING);
	return (FALSE);
    }
    fclose (fp);
    if (stat (topipe_filename, &statbuf) != 0)
    {
	fprintf (stderr, "Error stat'ing file: \"%s\"\t%s\n",
		 topipe_filename, ERRSTRING);
	return (FALSE);
    }
    if ( !S_ISFIFO (statbuf.st_mode) )
    {
	fprintf (stderr, "File: \"%s\" is not a named pipe\n",topipe_filename);
	return (FALSE);
    }
    if (getuid () != statbuf.st_uid)
    {
	fprintf (stderr, "File: \"%s\" is not owned by you\n",
		 topipe_filename);
	return (FALSE);
    }
    if (stat (frompipe_filename, &statbuf) != 0)
    {
	fprintf (stderr, "Error stat'ing file: \"%s\"\t%s\n",
		 frompipe_filename, ERRSTRING);
	return (FALSE);
    }
    if ( !S_ISFIFO (statbuf.st_mode) )
    {
	fprintf (stderr, "File: \"%s\" is not a named pipe\n",
		 frompipe_filename);
	return (FALSE);
    }
    if (getuid () != statbuf.st_uid)
    {
	fprintf (stderr, "File: \"%s\" is not owned by you\n",
		 frompipe_filename);
	return (FALSE);
    }
    if ( (statbuf.st_mode & (S_IRWXG | S_IRWXO) ) != 0 )
    {
	fprintf (stderr, "File: \"%s\" is accessible by others\n",
		 frompipe_filename);
	return (FALSE);
    }
    /*  Send signal to daemon: this also checks that the daemon is running  */
    if (kill (daemon_pid, SIGIO) == -1)
    {
	fprintf (stderr, "Error signalling daemon\t%s\n", ERRSTRING);
	return (FALSE);
    }
    if ( ( *from_fd = open (frompipe_filename, O_RDONLY, 0) ) < 0 )
    {
	fprintf (stderr, "Error opening named pipe\t%s\n", ERRSTRING);
	return (FALSE);
    }
    if (read (*from_fd, (char *) &randval, sizeof randval) != sizeof randval)
    {
	fprintf (stderr, "Error reading from named pipe\t%s\n", ERRSTRING);
	close (*from_fd);
	return (FALSE);
    }
    if ( ( *to_fd = open (topipe_filename, O_WRONLY, 0) ) < 0 )
    {
	fprintf (stderr, "Error opening named pipe\t%s\n", ERRSTRING);
	close (*from_fd);
	return (FALSE);
    }
    if (write (*to_fd, (char *) &randval, sizeof randval) != sizeof randval)
    {
	fprintf (stderr, "Error writing to named pipe\t%s\n", ERRSTRING);
	close (*from_fd);
	close (*to_fd);
	return (FALSE);
    }
    return (TRUE);
}   /*  End Function connect_to_pgpdaemon  */
