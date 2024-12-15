/*  spawn.c

    Source file for spawn operations for  PGPsendmail  (wrapper to sendmail).

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

/*  This programme intercepts messages sent by user mail agents to the
    sendmail daemon and checks to see if messages can be encrypted using the
    recipient's PGP public keys.


    Written by      Richard Gooch   31-MAY-1994

    Updated by      Richard Gooch   31-MAY-1994: Extracted from  pgpsendmail.c

    Updated by      Richard Gooch   18-JUN-1994: Made error messages more
  explicit.

    Updated by      Richard Gooch   27-JUN-1994: Copied  set_env  from
  pgpdaemon.c

    Updated by      Richard Gooch   5-JUL-1994: Changed to use of  m_copy  .

    Updated by      Richard Gooch   14-JUL-1994: Moved  copy_data  and  set_env
  to  misc.c

    Last updated by Richard Gooch   3-DEC-1994: Fixed bug for externally set
  error descriptor.


*/
#include <stdio.h>
#include "pgpsendmail.h"


int spawn_job (path, argv, in_fd, out_fd, err_fd)
/*  This routine will fork(2) and execvp(2) a process.
    The file to execute must be pointed to by  path  .
    The NULL terminated list of arguments which will be passed to  main  must
    be pointed to by  argv  .
    The input file descriptor (fd = 0) for the process must be pointed to by
    in_fd  .If the value here is less than 0, then a pipe to the process is
    opened and the writeable end is written to the storage pointed to by  in_fd
    The standard output file descriptor (fd = 1) for the process must be
    pointed to by  out_fd  .If the value here is less than 0, then a pipe to
    the process is opened and the readable end is written to the storage
    pointed to by  out_fd  .
    The standard error output file descriptor (fd = 2) for the process must be
    pointed to by  err_fd  .If the value here is less than 0, then a pipe to
    the process is opened and the readable end is written to the storage
    pointed to by  err_fd  .
    The routine returns the child process ID on success, else it returns -1.
*/
char *path;
char *argv[];
int *in_fd;
int *out_fd;
int *err_fd;
{
    int child_pid;
    int in_fds[2], out_fds[2], err_fds[2];
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    if (*in_fd < 0)
    {
	/*  Open a pipe  */
	if (pipe (in_fds) != 0)
	{
	    (void) fprintf (stderr, "Could not open input pipe\t%s\n",
			    sys_errlist[errno]);
	    return (-1);
	}
	/*  fd[0]: READ    fd[1]: WRITE  */
    }
    if (*out_fd < 0)
    {
	/*  Open a pipe  */
	if (pipe (out_fds) != 0)
	{
	    (void) fprintf (stderr, "Could not open output pipe\t%s\n",
			    sys_errlist[errno]);
	    return (-1);
	}
	/*  fd[0]: READ    fd[1]: WRITE  */
    }
    if (*err_fd < 0)
    {
	/*  Open a pipe  */
	if (pipe (err_fds) != 0)
	{
	    (void) fprintf (stderr, "Could not open error output pipe\t%s\n",
			    sys_errlist[errno]);
	    return (-1);
	}
	/*  fd[0]: READ    fd[1]: WRITE  */
    }
    /*  Fork and exec  */
    switch ( child_pid = fork () )
    {
      case 0:
	/*  Child: exec  */
	if (*in_fd < 0)
	{
	    (void) close (in_fds[1]);
	    dup2 (in_fds[0], 0);
	}
	else
	{
	    dup2 (*in_fd, 0);
	}
	if (*out_fd < 0)
	{
	    (void) close (out_fds[0]);
	    dup2 (out_fds[1], 1);
	}
	else
	{
	    dup2 (*out_fd, 1);
	}
	if (*err_fd < 0)
	{
	    (void) close (err_fds[0]);
	    dup2 (err_fds[1], 2);
	}
	else
	{
	    dup2 (*err_fd, 2);
	}
	(void) execvp (path, argv);
	(void) fprintf (stderr, "Could not exec: \"%s\"\t%s\n",
			path, sys_errlist[errno]);
	exit (1);
	break;
      case -1:
	/*  Error  */
	(void) fprintf (stderr, "Could not fork\t%s\n", sys_errlist[errno]);
	return (-1);
	break;
      default:
	/*  Parent  */
	break;
    }
    /*  Parent only  */
    if (*in_fd < 0)
    {
	(void) close (in_fds[0]);
	*in_fd = in_fds[1];
    }
    if (*out_fd < 0)
    {
	(void) close (out_fds[1]);
	*out_fd = out_fds[0];
    }
    if (*err_fd < 0)
    {
	(void) close (err_fds[1]);
	*err_fd  = err_fds[0];
    }
    return (child_pid);
}   /*  End Function spawn_job  */
