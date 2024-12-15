/*  sendpgppass.c

    Source file for  sendpgppass  (sends passphrase to PGPdaemon).

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


    Written by      Richard Gooch   5-JUN-1994

    Updated by      Richard Gooch   10-JUN-1994

    Updated by      Richard Gooch   21-JUN-1994: Added  #ifndef TCGETS  .

    Updated by      Richard Gooch   23-JUN-1994: Added  #ifdef _AIX

    Updated by      Richard Gooch   27-JUN-1994: Added tests to ensure
  passphrase is good, prior to sending to PGPdaemon.

    Updated by      Richard Gooch   28-JUN-1994: Switched to  tcgetattr  and
  tcsetattr  .

    Updated by      Richard Gooch   30-JUN-1994: Created  get_userid  .

    Updated by      Richard Gooch   5-JUL-1994: Changed to use of  m_clear  .

    Updated by      Richard Gooch   16-JUL-1994: Changed to use of  scrub_file

    Last updated by Richard Gooch   3-DEC-1994: Took account of changes to
  connect_to_pgpdaemon  .


*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _AIX
#  include <termios.h>
#  include <termio.h>
#else
#  include <sys/termios.h>
#endif
#include <fcntl.h>
#include "pgpsendmail.h"

#define TTY     "/dev/tty"

#ifdef obsolete
/*  Oh, fuck allmighty! How many ways are there to do this???  */
#ifndef TCGETS
#  ifdef TIOCGETA
#    define TCGETS TIOCGETA
#    define TCSETS TIOCSETA
#  else
#    ifdef TCGETATTR
#      define TCGETS TCGETATTR
#      define TCSETS TCSETATTR
#    else
#      include <sys/ioctl.h>
#      ifdef TCGETP
#        define TCGETS TCGETP
#        define TCSETS TCSANOW
#      else
!!! ERROR: will not be able to get  termios  structure
#      endif
#    endif
#  endif
#endif
#endif

STATIC_FUNCTION (char *mygetpass, (CONST char *prompt) );
STATIC_FUNCTION (char *get_userid, (char *pgppath) );


void main (argc, argv, envp)
int argc;
char *argv[];
char *envp[];
{
    int to_fd, from_fd;
    int len;
    int in_fd, out_fd, err_fd;
    int child, status;
    SECRING_SIZE_TYPE secring_bytes;
    int secring_fd;
    struct stat statbuf;
    char *buf;
    char *pgppath;
    char *passphrase;
    char *command;
    char *userid;
    FILE *fp;
    char secring_filename[STRING_LENGTH];
    char txt[STRING_LENGTH];
    char *pgp_argv[7];
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    if ( ( pgppath = getenv ("PGPPATH") ) == NULL )
    {
	(void) fprintf (stderr, "No PGPPATH environment variable\n");
	exit (RV_UNDEF_ERROR);
    }
    /*  Get userID  */
    if ( ( userid = get_userid (pgppath) ) == NULL )
    {
	(void) fprintf (stderr, "Could not get userID\n");
	exit (RV_UNDEF_ERROR);
    }
    (void) fprintf (stderr, "UserID: \"%s\"\n", userid);
    if (set_env ("PGPPASSFD", "0") != 0)
    {
	(void) fprintf (stderr,
			"Error setting PGPPASSFD environment variable\n");
	exit (RV_UNDEF_ERROR);
    }
    /*  Get the secret keyring  */
    (void) sprintf (secring_filename, "%s/secring.pgp", pgppath);
    if ( ( secring_fd = open (secring_filename, O_RDONLY, 0) ) < 0 )
    {
	(void) fprintf (stderr, "Error opening file: \"%s\"\t%s\n",
			secring_filename, sys_errlist[errno]);
	exit (RV_UNDEF_ERROR);
    }
    if (fstat (secring_fd, &statbuf) != 0)
    {
	(void) fprintf (stderr, "Error stat'ing file: \"%s\"\t%s\n",
			secring_filename, sys_errlist[errno]);
	(void) close (secring_fd);
	exit (RV_UNDEF_ERROR);
    }
    secring_bytes = statbuf.st_size;
    if ( ( buf = m_alloc (secring_bytes) ) == NULL )
    {
	(void) fprintf (stderr, "Error allocating: %d bytes\n", secring_bytes);
	(void) close (secring_fd);
	exit (RV_MEM_ERROR);
    }
    if (read (secring_fd, buf, secring_bytes) < secring_bytes)
    {
	(void) fprintf (stderr, "Error reading secret keyring\t%s\n",
			sys_errlist[errno]);
	(void) close (secring_fd);
	exit (RV_READ_ERROR);
    }
    (void) close (secring_fd);
    if ( !connect_to_pgpdaemon (pgppath, &to_fd, &from_fd) )
    {
	(void) fprintf (stderr, "Could not connect to PGPdaemon\n");
	exit (RV_UNDEF_ERROR);
    }
    if ( ( passphrase = mygetpass ("Enter your PGP passphrase: ") ) == NULL )
    {
	(void) fprintf (stderr, "Error getting passphrase\t%s\n",
			sys_errlist[errno]);
	(void) close (to_fd);
	(void) close (from_fd);
	m_clear (buf, secring_bytes);
	exit (RV_SYS_ERROR);
    }
    len = strlen (passphrase);
    /*  Now check that passphrase is OK  */
    (void) sprintf (txt, "%s/sigtest", pgppath);
    if ( ( fp = fopen (txt, "w") ) == NULL )
    {
	(void) fprintf (stderr, "Error opening file: \"%s\"\t%s\n",
			sys_errlist[errno]);
	m_clear ( passphrase, strlen (passphrase) );
	m_clear (buf, secring_bytes);
	exit (RV_SYS_ERROR);
    }
    (void) fprintf (fp, "test line\n");
    (void) fclose (fp);
    in_fd = -1;
    out_fd = ERROR_FD;
    err_fd = ERROR_FD;
    /*  pgp -sb +batchmode=on <file>  */
    pgp_argv[0] = "pgp";
    pgp_argv[1] = "-sb";
    pgp_argv[2] = "+batchmode=on";
    pgp_argv[3] = txt;
    pgp_argv[4] = "-u";
    pgp_argv[5] = userid;
    pgp_argv[6] = NULL;
    /*  Start PGP  */
    if ( ( child = spawn_job ("pgp", pgp_argv, &in_fd, &out_fd, &err_fd) )
	< 0 )
    {
	(void) fprintf (stderr, "Error spawning PGP\n");
	m_clear ( passphrase, strlen (passphrase) );
	m_clear (buf, secring_bytes);
	exit (RV_SYS_ERROR);
    }
    /*  Send passphrase  */
    if (write (in_fd, passphrase, len) < len)
    {
	(void) fprintf (stderr, "Error writing passphrase\t%s\n",
			sys_errlist[errno]);
	(void) close (in_fd);
	(void) waitpid (child, &status, 0);
	m_clear ( passphrase, strlen (passphrase) );
	m_clear (buf, secring_bytes);
	exit (RV_SYS_ERROR);
    }
    (void) close (in_fd);
    if (waitpid (child, &status, 0) < 0)
    {
	(void) fprintf (stderr, "Error reaping child\t%s\n",
			sys_errlist[errno]);
	m_clear ( passphrase, strlen (passphrase) );
	m_clear (buf, secring_bytes);
	exit (RV_SYS_ERROR);
    }
    (void) unlink (txt);
    (void) sprintf (txt, "%s/sigtest.sig", pgppath);
    (void) unlink (txt);
    if (status != 0)
    {
	(void) fprintf (stderr, "Bad child status: %d\n", status);
	m_clear ( passphrase, strlen (passphrase) );
	m_clear (buf, secring_bytes);
	exit (RV_SYS_ERROR);
    }
    command = "PASSPHRASE\n";
    len = strlen (command);
    if (write (to_fd, command, len) < len)
    {
	(void) fprintf (stderr, "Error writing command\t%s\n",
			sys_errlist[errno]);
	(void) close (to_fd);
	m_clear ( passphrase, strlen (passphrase) );
	m_clear (buf, secring_bytes);
	exit (RV_SYS_ERROR);
    }
    len = strlen (passphrase);
    if (write (to_fd, passphrase, len) < len)
    {
	(void) fprintf (stderr, "Error writing passphrase\t%s\n",
			sys_errlist[errno]);
	(void) close (to_fd);
	m_clear ( passphrase, strlen (passphrase) );
	m_clear (buf, secring_bytes);
	exit (RV_SYS_ERROR);
    }
    /*  Scrub-up  */
    m_clear ( passphrase, strlen (passphrase) );
    if (write (to_fd, "\n", 1) < 1)
    {
	(void) fprintf (stderr, "Error writing NEWLINE\t%s\n",
			sys_errlist[errno]);
	(void) close (to_fd);
	m_clear (buf, secring_bytes);
	exit (RV_SYS_ERROR);
    }
    /*  Write secret keyring length  */
    if (write (to_fd, (char *) &secring_bytes, sizeof secring_bytes) <
	sizeof secring_bytes)
    {
	(void) fprintf (stderr, "Error writing secret keyring length\t%s\n",
			sys_errlist[errno]);
	(void) close (to_fd);
	m_clear (buf, secring_bytes);
	exit (RV_WRITE_ERROR);
    }
    /*  Write secret keyring  */
    if (write (to_fd, buf, secring_bytes) < secring_bytes)
    {
	(void) fprintf (stderr, "Error writing secret keyring data\t%s\n",
			sys_errlist[errno]);
	(void) close (to_fd);
	m_clear (buf, secring_bytes);
	exit (RV_WRITE_ERROR);
    }
    (void) close (to_fd);
    (void) close (from_fd);
    /*  Scrub-up  */
    m_clear (buf, secring_bytes);
    m_free (buf);
    if ( (argc == 2) && (strcmp (argv[1], "-wipe") == 0) )
    {
	if ( !scrub_file (secring_filename, TRUE) )
	{
	    (void) fprintf (stderr, "Error scrubbing file: \"%s\"\n",
			    secring_filename);
	    exit (RV_UNDEF_ERROR);
	}
    }
    (void) sync ();
}   /*  End Function main  */

static char *mygetpass (prompt)
/*  This routine is similar to the  getpass(3)  C library call, only it doesn't
    have that stupid 8 character limitation.
*/
CONST char *prompt;
{
    struct termios tty_new, tty_orig;
    char ch;
    int count;
    int fd;
    ERRNO_TYPE errno;
    extern char *sys_errlist[];
    static char buffer[STRING_LENGTH];

    if ( ( fd = open (TTY, O_RDWR, 0) ) < 0 )
    {
	(void) fprintf (stderr, "Error opening terminal\t%s\n",
			sys_errlist[errno]);
	exit (RV_CANNOT_OPEN);
    }
#ifdef obsolete
    (void) ioctl (fd, TCGETS, &tty_new);
    (void) ioctl (fd, TCGETS, &tty_orig);
#else
    (void) tcgetattr (fd, &tty_new);
    (void) tcgetattr (fd, &tty_orig);
#endif
    tty_new.c_lflag &= ~(ECHO | ISIG);
#ifdef obsolete
    (void) ioctl (fd, TCSETS, &tty_new);
#else
#  ifdef TCSANOW
    tcsetattr (fd, TCSANOW, &tty_new);
#  else
    tcsetattr (fd, 0,  &tty_new);
#  endif
#endif
    (void) fprintf (stderr, "%s", prompt);
    fflush (stderr);
    /*  Read from terminal  */
    for (count = 0; count < STRING_LENGTH - 1; ++count)
    {
	if (read (fd, &ch, 1) < 1)
	{
	    (void) fprintf (stderr, "Error reading\t%s\n", sys_errlist[errno]);
	    exit (RV_READ_ERROR);
	}
	if ( (ch == '\n') || (ch == '\r') )
	{
	    count = STRING_LENGTH;
	    continue;
	}
	buffer[count] = ch;
	buffer[count + 1] = '\0';
    }
    (void) fprintf (stderr, "\r\n");
    fflush (stderr);
#ifdef obsolete
    (void) ioctl (fd, TCSETS, &tty_orig);
#else
#  ifdef TCSANOW
    tcsetattr (fd, TCSANOW, &tty_orig);
#  else
    tcsetattr (fd, 0,  &tty_orig);
#  endif
#endif
    return (buffer);
}   /*  End Function mygetpass  */

static char *get_userid (pgppath)
/*  This routine will read the configuration file:  $PGPPATH/PGPdaemon.config
    searching for the userID specified by the "USERID" keyword.
    The value of the PGPPATH environment variable must be pointed to by
    pgppath  .
    The routine returns a pointer to the userID on succes, else it return NULL.
*/
char *pgppath;
{
    char *keyword, *rest, *userid;
    FILE *fp;
    char config_filename[STRING_LENGTH];
    char txt[STRING_LENGTH];
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    (void) sprintf (config_filename, "%s/PGPdaemon.config", pgppath);
    if ( ( fp = fopen (config_filename, "r") ) == NULL )
    {
	(void) fprintf (stderr, "Error reading file: \"%s\"\t%s\n",
			config_filename, sys_errlist[errno]);
	exit (RV_CANNOT_OPEN);
    }
    /*  Read in lines  */
    while (fgets (txt, STRING_LENGTH - 1, fp) != NULL)
    {
	/*  Strip newline  */
	txt[strlen (txt) - 1] = '\0';
	if (strlen (txt) < 1) continue;
	if (txt[0] == '#') continue;
	if ( ( keyword = ex_str (txt, &rest) ) == NULL ) continue;
	if (strcmp (keyword, "USERID") == 0)
	{
	    if ( ( userid = ex_str (rest, &rest) ) == NULL )
	    {
		(void) fprintf (stderr, "Could not get userID\n");
		m_free (keyword);
		return (NULL);
	    }
	    m_free (keyword);
	    return (userid);
	}
	m_free (keyword);
    }
    (void) fprintf (stderr, "USERID keyword not found\n");
    return (NULL);
}   /*  End Function get_userid  */
