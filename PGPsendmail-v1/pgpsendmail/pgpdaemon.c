/*  pgpdaemon.c

    Source file for  PGPdaemon  (automatic signer/encryptor for PGP).

    Copyright (C) 1994,1995  Richard Gooch

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


    Written by      Richard Gooch   4-JUN-1994

    Updated by      Richard Gooch   5-JUN-1994

    Updated by      Richard Gooch   10-JUN-1994: Changed from using  stricmp
  to  st_icmp  ,since some platforms don't have  stricmp  .

    Updated by      Richard Gooch   20-JUN-1994: Added mailbox decrypting
  functionality.

    Updated by      Richard Gooch   21-JUN-1994: Added  #include <sys/types.h>

    Updated by      Richard Gooch   22-JUN-1994: Added IN_SPOOL_DIR  and
  OUT_SPOOL_FILE  keywords for config file.

    Updated by      Richard Gooch   23-JUN-1994: Added MAIL_CHECK_INTERVAL
  keyword for config file.

    Updated by      Richard Gooch   23-JUN-1994: Added  #ifdef _AIX  and made
  set_lockfile  a little more verbose if lockfiles can't be created.

    Updated by      Richard Gooch   27-JUN-1994: Moved  set_env  to
  spawn.c  and added  -detach  and  -pgppath  options.

    Updated by      Richard Gooch   28-JUN-1994: Improved logging information
  when processing mail.

    Updated by      Richard Gooch   1-JUL-1994: Used [-u userID] option to PGP
  when signing messages.

    Updated by      Richard Gooch   2-JUL-1994: Trap unterminated PGP messages.

    Updated by      Richard Gooch   2-JUL-1994: Truncate input mail spool
  rather than removing, since  sendmail  under OSF/1 will create the spoolfile
  even if the lockfiles exist, it just waits for the lockfiles to go before it
  starts writing to the spoolfile. This caused some messages to be lost, since
  the file  sendmail  would eventually write to was unlinked (ie. only
  sendmail  had a handle to the file it was writing to).

    Updated by      Richard Gooch   3-JUL-1994: Added some imformative
  messages when decryption of mail fails.

    Updated by      Richard Gooch   3-JUL-1994: Recover from unterminated PGP
  messages so that subsequent messages can be decrypted.

    Updated by      Richard Gooch   3-JUL-1994: Trap when PGP stops when
  decrypting mail. Usually because message encrypted "For-your-eyes-only".

    Updated by      Richard Gooch   4-JUL-1994: Added  #ifdef O_SYNC  and
  #ifdef __bsdi__  for MAIL_SPOOL_DIR  .

    Updated by      Richard Gooch   5-JUL-1994: Changed to use of  m_clear  .

    Updated by      Richard Gooch   11-JUL-1994: Added check for writability of
  incoming spool directory

    Uupdated by     Richard Gooch   11-JUL-1994: Use MAIL environment variable
  in preference to hard-coded default if  IN_SPOOL_DIR  option not defined.

    Updated by      Richard Gooch   12-JUL-1994: Added support for leading '~'
  in filenames in config file.

    Updated by      Richard Gooch   13-JUL-1994: Limited core dumps to 0 bytes.

    Updated by      Richard Gooch   14-JUL-1994: Added flag response with
  connecting.

    Updated by      Richard Gooch   14-JUL-1994: Added calls to  fsync(2)
  after creating lockfiles.

    Updated by      Richard Gooch   16-JUL-1994: Added hostname to socket
  and log filenames and added secret keyring creation/deletion.

    Uupdated by     Richard Gooch   28-JUL-1994: Dump secret keyring when
  SIGUSR1 is caught.

    Updated by      Richard Gooch   3-AUG-1994: Added  -nomail  option.

    Updated by      Richard Gooch   25-AUG-1994: Fixed descriptor leak in
  write_secring  .

    Updated by      Richard Gooch   26-AUG-1994: Pass  check_mail  flag to
  read_config  so that writability of spool file is not checked when the
  -nomail  option was given.

    Updated by      Richard Gooch   11-NOV-1994: Changed from aborting if
  file locking fails to waiting until file locks released. Added
  -no_filelocking  option.

    Updated by      Richard Gooch   17-NOV-1994: Removed extraneous file
  unlocking in  check_and_process_mail  .

    Updated by      Richard Gooch   3-DEC-1994: Changed to pair of named pipes
  and PID file rather than listening on a Unix domain socket.

    Updated by      Richard Gooch   4-DEC-1994: Added  +batchmode  option to
  PGP when decrypting.

    Updated by      Richard Gooch   9-DEC-1994: Create pipes if they don't
  exist.

    Updated by      Richard Gooch   9-DEC-1994: Changed from  rand48(3)  to
  random(3)

    Last updated by Richard Gooch   4-JAN-1995: Changed from  random(3)  to
  rand(3)  because Slowass 2 doesn't always have  random(3)  .


*/
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "pgpsendmail.h"

#ifndef MAIL_SPOOL_DIR
#  if defined(sparc) || defined(linux)
#    define MAIL_SPOOL_DIR "/var/spool/mail"
#  else
#    if defined(__bsdi__) || ( defined(sparc) && defined(__sysvr4__) )
#      define MAIL_SPOOL_DIR "/var/mail"
#    else
#      define MAIL_SPOOL_DIR "/usr/spool/mail"
#    endif
#  endif
#endif

#define MIN_INTERVAL 1
#define DEFAULT_INTERVAL 15
#define MAX_INTERVAL 60

STATIC_FUNCTION (flag service_connect,
		 (CONST char *inpipe_filename, CONST char *outpipe_filename) );
STATIC_FUNCTION (void sig_handler, () );
STATIC_FUNCTION (flag process_message, (int in_fd, int out_fd, flag sign) );
STATIC_FUNCTION (void myexit, (int code) );
STATIC_FUNCTION (flag set_lockfile, (char *filename, flag lock) );
STATIC_FUNCTION (flag set_fd_lock, (int fd, flag lock, flag no_filelocking) );
STATIC_FUNCTION (void check_and_process_mail, () );
STATIC_FUNCTION (void read_config, (char *pgppath, flag *mailwait,
				    long *interval, flag check_mail) );
STATIC_FUNCTION (flag write_secring, () );
STATIC_FUNCTION (flag scrub_secring, () );
STATIC_FUNCTION (int get_rand, () );


/*  Private data  */
static char *pgppath = NULL;
static char passphrase[STRING_LENGTH];
static SECRING_SIZE_TYPE secring_bytes = 0;
static char *secring_buf = NULL;
static flag keep_going = TRUE;
static flag restart = FALSE;
static flag incoming_locked = FALSE;
static flag insecure = TRUE;
static flag dump_secring = FALSE;
static flag check_mail = TRUE;
static flag no_filelocking = FALSE;
static flag caught_sigio = FALSE;
static char incoming_spool[STRING_LENGTH];
static char outgoing_spool[STRING_LENGTH];
static char my_userid[STRING_LENGTH];


/*  Code for  myexit  must come first.  */

static void myexit (code)
/*  This routine will first scrub sensitive information prior to calling the
    exit(3) function.
    The exit code must be given by  code  .
*/
int code;
{
    extern flag insecure;
    extern SECRING_SIZE_TYPE secring_bytes;
    extern char passphrase[STRING_LENGTH];
    extern char *secring_buf;

    if (insecure) exit (code);
    (void) fprintf (stderr, "Erasing sensitive information...\n");
    m_clear (passphrase, STRING_LENGTH);
    (void) write_secring ();
    if (secring_bytes > 0) m_clear (secring_buf, secring_bytes);
    (void) sync ();
    exit (code);
#undef exit
#define exit ___illegal_use_of_exit___
}   /*  End Function myexit  */


/*  Now everything else may follow.  */

void main (argc, argv, envp)
int argc;
char *argv[];
char *envp[];
{
    FILE *fp;
    flag mailwait = FALSE;
    flag login_session = TRUE;
    int ppid;
    int sock, fd;
    int tmp, count, icount;
    long timeout_s = DEFAULT_INTERVAL;
    fd_set input_fds, output_fds, exception_fds;
    struct timeval timeout;
    struct sigaction new_action;
    struct rlimit rlp;
    struct stat statbuf;
    char hostname[STRING_LENGTH];
    char pid_filename[STRING_LENGTH];
    char inpipe_filename[STRING_LENGTH];
    char outpipe_filename[STRING_LENGTH];
    char txt[LINE_LENGTH];
    extern flag keep_going;
    extern flag restart;
    extern flag incoming_locked;
    extern flag insecure;
    extern flag dump_secring;
    extern flag check_mail;
    extern flag no_filelocking;
    extern flag caught_sigio;
    extern SECRING_SIZE_TYPE secring_bytes;
    extern char *pgppath;
    extern char passphrase[STRING_LENGTH];
    extern char my_userid[STRING_LENGTH];
    extern char incoming_spool[STRING_LENGTH];
    extern char outgoing_spool[STRING_LENGTH];
    ERRNO_TYPE errno;
    static char usage_string[] =
    "Usage:\tpgpdaemon [-mailwait] [-detach] [-pgppath <path>]";
    extern char *sys_errlist[];

    /*  Read arguments  */
    for (count = 1; count < argc; ++count)
    {
	if (strcmp (argv[count], "-mailwait") == 0) mailwait = TRUE;
	else if (strcmp (argv[count], "-detach") == 0) login_session = FALSE;
	else if (strcmp (argv[count], "-pgppath") == 0)
	{
	    if (++count >= argc)
	    {
		(void) fprintf (stderr, "%s\n", usage_string);
		myexit (RV_BAD_PARAM);
	    }
	    if (set_env ("PGPPATH", argv[count]) != 0)
	    {
		(void)fprintf (stderr,
			       "Error setting PGPPATH environment variable\n");
		myexit (RV_UNDEF_ERROR);
	    }
	}
	else if (strcmp (argv[count], "-nomail") == 0) check_mail = FALSE;
	else if (strcmp (argv[count],
			 "-no_filelocking") == 0) no_filelocking = TRUE;
	else
	{
	    (void) fprintf (stderr, "%s\n", usage_string);
	    myexit (RV_BAD_PARAM);
	}
    }
    if ( ( ppid = getppid () ) < 0 )
    {
	(void) fprintf (stderr, "Error getting parent process ID\t%s\n",
			sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
    if (ppid == 1)
    {
	/*  Init was parent!  */
	(void) fprintf (stderr,
			"Cannot operate with  init  as parent process\n");
	myexit (RV_UNDEF_ERROR);
    }
    if ( ( pgppath = getenv ("PGPPATH") ) == NULL )
    {
	(void) fprintf (stderr, "No PGPPATH environment variable\n");
	myexit (RV_UNDEF_ERROR);
    }
    (void) umask ( ~(S_IRUSR | S_IWUSR) );
    m_clear (passphrase, STRING_LENGTH);
    new_action.sa_handler = sig_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;
#ifdef SA_RESTART
    new_action.sa_flags |= SA_RESTART;
#endif
    sigaction (SIGINT, &new_action, (struct sigaction *) NULL);
    sigaction (SIGTERM, &new_action, (struct sigaction *) NULL);
    sigaction (SIGHUP, &new_action, (struct sigaction *) NULL);
    sigaction (SIGPIPE, &new_action, (struct sigaction *) NULL);
    sigaction (SIGTTIN, &new_action, (struct sigaction *) NULL);
    sigaction (SIGTTOU, &new_action, (struct sigaction *) NULL);
    sigaction (SIGUSR1, &new_action, (struct sigaction *) NULL);
    sigaction (SIGIO, &new_action, (struct sigaction *) NULL);
#ifdef RLIMIT_CORE
    /*  Prevent core dumps  */
    rlp.rlim_cur = 0;
    rlp.rlim_max = 0;
    if (setrlimit (RLIMIT_CORE, &rlp) != 0)
    {
	(void) fprintf (stderr, "Error limiting core dump size\t%s\n",
			sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
#endif
    if (set_env ("PGPPASSFD", "0") != 0)
    {
	(void) fprintf (stderr,
			"Error setting PGPPASSFD environment variable\n");
	myexit (RV_UNDEF_ERROR);
    }
    /*  Sanity checks for files the user should have  */
    (void) sprintf (txt, "%s/randseed.bin", pgppath);
    if (access (txt, R_OK) != 0)
    {
	(void) fprintf (stderr, "Could not find file: \"%s\"\t%s\n",
			txt, sys_errlist[errno]);
	myexit (RV_UNDEF_ERROR);
    }
    read_config (pgppath, &mailwait, &timeout_s, check_mail);
    if (gethostname (hostname, STRING_LENGTH - 1) != 0)
    {
	(void) fprintf (stderr, "Error getting hostname\t%s\n",
			sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
    hostname[STRING_LENGTH - 1] = '\0';
    /*  Worry about communication files  */
    (void) sprintf (pid_filename, "%s/.pgpd.PID.%s", pgppath, hostname);
    (void) sprintf (inpipe_filename, "%s/.pgpd.input.%s", pgppath, hostname);
    (void) sprintf (outpipe_filename, "%s/.pgpd.output.%s", pgppath, hostname);
    if (access (pid_filename, F_OK) == 0)
    {
	(void) fprintf (stderr, "File: \"%s\" exists:\n", pid_filename);
	(void) fprintf (stderr, "PGPdaemon did not clean up last time\n");
	myexit (RV_UNDEF_ERROR);
    }
    if (errno != ENOENT)
    {
	(void) fprintf (stderr, "Error testing for PID file: \"%s\"\t%s\n",
			pid_filename, sys_errlist[errno]);
	myexit (RV_UNDEF_ERROR);
    }
    if (stat (inpipe_filename, &statbuf) == 0)
    {
	if ( !S_ISFIFO (statbuf.st_mode) )
	{
	    (void) fprintf (stderr, "File: \"%s\" is not a named pipe\n",
			    inpipe_filename);
	    myexit (RV_SYS_ERROR);
	}
	(void) chmod (inpipe_filename, S_IRUSR | S_IWUSR);
    }
    else
    {
	if (errno != ENOENT)
	{
	    (void) fprintf (stderr, "Error stat'ing file: \"%s\"\t%s\n",
			    inpipe_filename, sys_errlist[errno]);
	    myexit (RV_SYS_ERROR);
	}
	if (mkfifo (inpipe_filename, S_IRUSR | S_IWUSR) != 0)
	{
	    (void) fprintf (stderr, "Error creating pipe: \"%s\"\t%s\n",
			    inpipe_filename, sys_errlist[errno]);
	    myexit (RV_SYS_ERROR);
	}
    }
    if (stat (outpipe_filename, &statbuf) == 0)
    {
	if ( !S_ISFIFO (statbuf.st_mode) )
	{
	    (void) fprintf (stderr, "File: \"%s\" is not a named pipe\n",
			    outpipe_filename);
	    myexit (RV_SYS_ERROR);
	}
	(void) chmod (outpipe_filename, S_IRUSR | S_IWUSR);
    }
    else
    {
	if (errno != ENOENT)
	{
	    (void) fprintf (stderr, "Error stat'ing file: \"%s\"\t%s\n",
			    outpipe_filename, sys_errlist[errno]);
	    myexit (RV_SYS_ERROR);
	}
	if (mkfifo (outpipe_filename, S_IRUSR | S_IWUSR) != 0)
	{
	    (void) fprintf (stderr, "Error creating pipe: \"%s\"\t%s\n",
			    outpipe_filename, sys_errlist[errno]);
	    myexit (RV_SYS_ERROR);
	}
    }
    /*  Write PID  */
    if ( ( fp = fopen (pid_filename, "w") ) == NULL )
    {
	(void) fprintf (stderr, "Error writing PID file: \"%s\"\t%s\n",
			pid_filename, sys_errlist[errno]);
	myexit (RV_CANNOT_OPEN);
    }
    (void) fprintf ( fp, "%d\n", getpid () );
    (void) fclose (fp);
    /*  Open logfile  */
    (void) sprintf (txt, "%s/PGPdaemon.log.%s", pgppath, hostname);
    if ( ( fd = open (txt, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR) )
	< 0 )
    {
	(void) fprintf (stderr, "Error opening file: \"%s\"\t%s\n",
			txt, sys_errlist[errno]);
	myexit (RV_UNDEF_ERROR);
    }
    if (dup2 (fd, 1) == -1)
    {
	(void) fprintf (stderr, "Error dup2'ing\t%s\n", sys_errlist[errno]);
	myexit (RV_UNDEF_ERROR);
    }
    if (dup2 (fd, 2) == -1)
    {
	(void) fprintf (stderr, "Error dup2'ing\t%s\n", sys_errlist[errno]);
	myexit (RV_UNDEF_ERROR);
    }
    (void) fprintf (stderr,
		    "-----------------------------------------------------\n");
    (void) fprintf (stderr, "PGPdaemon %s: appending to logfile\n", VERSION);
    (void) fprintf (stderr, "USERID is: \"%s\"\n", my_userid);
    (void) fprintf (stderr, "Incoming spool file is: \"%s\"%s\n",
		    incoming_spool, check_mail ? "" : "\tIGNORED");
    (void) fprintf (stderr, "Outgoing spool file is: \"%s\"%s\n",
		    outgoing_spool, check_mail ? "" : "\tIGNORED");
    (void) fprintf (stderr, "Mail check interval: %ld seconds\n", timeout_s);
    icount = 0;
    while (keep_going)
    {
	/*  Keep passphrase in RAM (hopefully out of swap) by touching it  */
	for (count = 0, tmp = 0; count < STRING_LENGTH; ++count)
	{
	    tmp += passphrase[count];
	}
	tmp = 0;
	(void) sleep (1);
	if ( login_session && (getppid () != ppid) )
	{
	    keep_going = FALSE;
	    continue;
	}
	if ( check_mail && ( !mailwait || (secring_bytes > 0) ) )
	{
	    if (++icount >= timeout_s)
	    {
		icount = 0;
		/*  Check and process incoming mail spool file  */
		check_and_process_mail ();
	    }
	}
	if (dump_secring)
	{
	    (void) fprintf (stderr,
			    "Caught SIGUSR1: dumping secret keyring\n");
	    (void) write_secring ();
	    dump_secring = FALSE;
	}
	/*  Check for connections  */
	if (caught_sigio)
	{
	    (void) service_connect (inpipe_filename, outpipe_filename);
	    caught_sigio = FALSE;
	}
    }
    (void) fprintf (stderr, "Removing PID file\n");
    (void) unlink (pid_filename);
    if (restart)
    {
	if (!insecure)
	{
	    (void) fprintf (stderr, "Erasing sensitive information...\n");
	    m_clear (passphrase, STRING_LENGTH);
	    (void) write_secring ();
	    if (secring_bytes > 0) m_clear (secring_buf, secring_bytes);
	    (void) sync ();
	}
	(void) fprintf (stderr, "Restarting\n");
	(void) execvp (argv[0], argv);
	(void) fprintf (stderr, "Error calling  execvp(3)\t%s\n",
			sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
    myexit (RV_OK);
}   /*  End Function main  */

static flag service_connect (inpipe_filename, outpipe_filename)
/*  This routine will service a connection.
    The name of the input named pipe must be pointed to by  inpipe_filename  .
    The name of the output named pipe must be pointed to by  outpipe_filename
    The routine returns TRUE on succes, else it returns FALSE.
*/
CONST char *inpipe_filename;
CONST char *outpipe_filename;
{
    flag bool_val;
    int in_fd, out_fd = -1;
    int count, randval, readval;
    char command[STRING_LENGTH];
    extern flag insecure;
    extern SECRING_SIZE_TYPE secring_bytes;
    extern char passphrase[STRING_LENGTH];
    extern char *secring_buf;
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    /*  Connect to output pipe  */
    for (count = 0; (count < 2) && (out_fd < 0); ++count)
    {
	if ( ( out_fd = open (outpipe_filename, O_WRONLY | O_NDELAY) ) < 0 )
	{
	    if (errno == ENXIO)
	    {
		(void) sleep (1);
		continue;
	    }
	    (void) fprintf (stderr, "Error opening named pipe\t%s\n",
			    sys_errlist[errno]);
	    return (FALSE);
	}
    }
    if (out_fd < 0)
    {
	(void) fprintf (stderr, "Timeout opening named pipe\n");
	return (FALSE);
    }
    if (fcntl (out_fd, F_SETFL, 0) == -1)
    {
	(void) fprintf (stderr, "Error controlling named pipe\t%s\n",
			sys_errlist[errno]);
	(void) close (out_fd);
	return (FALSE);
    }
    randval = get_rand ();
    if (write (out_fd, (char *) &randval, sizeof randval) != sizeof randval)
    {
	(void) fprintf (stderr, "Error writing random value\t%s\n",
			sys_errlist[errno]);
	(void) close (out_fd);
	return (FALSE);
    }
    if ( ( in_fd = open (inpipe_filename, O_RDONLY) ) < 0 )
    {
	(void) fprintf (stderr, "Error opening named pipe\t%s\n",
			sys_errlist[errno]);
	(void) close (out_fd);
	return (FALSE);
    }
    if (read (in_fd, (char *) &readval, sizeof readval) != sizeof readval)
    {
	(void) fprintf (stderr, "Error reading confirmation value\t%s\n",
			sys_errlist[errno]);
	(void) close (out_fd);
	(void) close (in_fd);
	return (FALSE);
    }
    if (readval != randval)
    {
	(void) fprintf (stderr, "Confirmation value incorrect!\n");
	(void) close (out_fd);
	(void) close (in_fd);
	return (FALSE);
    }
    if ( !read_line_fd (in_fd, command, STRING_LENGTH, TRUE) )
    {
	(void) close (out_fd);
	(void) close (in_fd);
	return (TRUE);
    }
    (void) fprintf (stderr, "Received command: \"%s\"\n", command);
    /*  Process command  */
    if (st_icmp (command, "passphrase") == 0)
    {
	insecure = FALSE;
	if ( !read_line_fd (in_fd, passphrase, STRING_LENGTH - 1, TRUE) )
	{
	    (void) fprintf (stderr, "Error reading passphrase\t%s\n",
			    sys_errlist[errno]);
	    (void) close (out_fd);
	    (void) close (in_fd);
	    return (FALSE);
	}
	passphrase[strlen (passphrase)] = '\n';
	if (secring_bytes > 0)
	{
	    m_clear (secring_buf, secring_bytes);
	    m_free (secring_buf);
	}
	secring_bytes = 0;
	secring_buf = NULL;
	if (read (in_fd, (char *) &secring_bytes, sizeof secring_bytes) <
	    sizeof secring_bytes)
	{
	    (void) fprintf (stderr,
			    "Error reading secret keyring length\t%s\n",
			    sys_errlist[errno]);
	    (void) close (out_fd);
	    (void) close (in_fd);
	    myexit (RV_WRITE_ERROR);
	}
	if ( ( secring_buf = m_alloc (secring_bytes) ) == NULL )
	{
	    (void) fprintf (stderr, "Error allocating: %d bytes\n",
			    secring_bytes);
	    (void) close (out_fd);
	    (void) close (in_fd);
	    myexit (RV_MEM_ERROR);
	}
	if (read (in_fd, secring_buf, secring_bytes) < secring_bytes)
	{
	    (void) fprintf (stderr, "Error reading secret keyring\t%s\n",
			    sys_errlist[errno]);
	    (void) close (out_fd);
	    (void) close (in_fd);
	    myexit (RV_READ_ERROR);
	}
	(void) fprintf (stderr,
			"Passphrase and secret keyring (%d bytes) received\n",
			secring_bytes);
	(void) close (out_fd);
	(void) close (in_fd);
	return (TRUE);
    }
    /*  At this point, all requests are for signing or decryption  */
    bool_val = (secring_bytes > 0) ? TRUE : FALSE;
    /*  Return success/failure message  */
    if (write (out_fd, &bool_val, sizeof bool_val) < sizeof bool_val)
    {
	(void) fprintf (stderr, "Error writing boolean\t%s\n",
			sys_errlist[errno]);
	(void) close (out_fd);
	(void) close (in_fd);
	return (FALSE);
    }
    if (!bool_val)
    {
	(void) close (out_fd);
	(void) close (in_fd);
	return (TRUE);
    }
    if (st_icmp (command, "SIGN") == 0)
    {
	if ( !process_message (in_fd, out_fd, TRUE) )
	{
	    (void) close (out_fd);
	    (void) close (in_fd);
	    return (FALSE);
	}
	(void) close (out_fd);
	(void) close (in_fd);
	return (TRUE);
    }
    if (st_icmp (command, "DECRYPT") == 0)
    {
	if ( !process_message (in_fd, out_fd, FALSE) )
	{
	    (void) close (out_fd);
	    (void) close (in_fd);
	    return (FALSE);
	}
	(void) close (out_fd);
	(void) close (in_fd);
	return (TRUE);
    }
    (void) fprintf (stderr, "Illegal command: \"%s\"\n", command);
    (void) close (out_fd);
    (void) close (in_fd);
    return (FALSE);
}   /*  End Function service_connect  */

static void sig_handler (sig, code, scp, addr)
/*  This routine will handle signals. It sets the global flag  keep_going  to
    FALSE.
    The signal number will be given by  sig  .
    The routine returns nothing.
*/
int sig;
int code;
struct sigcontext *scp;
char *addr;
{
    extern flag keep_going;
    extern flag restart;
    extern flag dump_secring;
    extern flag caught_sigio;

    switch (sig)
    {
      case SIGINT:
      case SIGTERM:
      case SIGPIPE:
	keep_going = FALSE;
	break;
      case SIGHUP:
	keep_going = FALSE;
	restart = TRUE;
	break;
      case SIGTTIN:
      case SIGTTOU:
	break;
      case SIGUSR1:
	dump_secring = TRUE;
	break;
      case SIGIO:
	caught_sigio = TRUE;
	break;
      default:
	(void) fprintf (stderr, "Illegal signal: %d\n", sig);
	myexit (RV_UNDEF_ERROR);
	break;
    }
    /*  Install the signal handler again  */
/*
    (void) signal (sig, sig_handler);
*/
}   /*  End Function sig_handler  */

static flag process_message (in_fd, out_fd, sign)
/*  This routine will process a message, either signing or decrypting it.
    The file descriptor from which the message should be read must be given by
    in_fd  .
    The file descriptor to which the processed message should be written must
    be given by  out_fd  .
    If the value of  sign  is TRUE, the message is signed, else it is decrypted
    The routine returns TRUE on success, else it returns FALSE.
*/
int in_fd;
int out_fd;
flag sign;
{
    flag remove_secring;
    int pgp_in_fd = -1;
    int pgp_out_fd = out_fd;
    int pgp_err_fd = ERROR_FD;
    int child;
    int status;
    int len;
    char *argv[7];
    extern char my_userid[STRING_LENGTH];
    extern char passphrase[STRING_LENGTH];
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    /*  Setup arguments to PGP  */
    if (sign)
    {
	/*  pgp -fsta +clearsig=on +batchmode  */
	argv[0] = "pgp";
	argv[1] = "-fsta";
	argv[2] = "+clearsig=on";
	argv[3] = "+batchmode";
	argv[4] = "-u";
	argv[5] = my_userid;
	argv[6] = NULL;
	/*  The following line is a hack so that PGPsendmail won't drop the
	    first line, which is the signed message delimiter.
	    Maybe one day I'll work out *why* PGP often (but not always!)
	    drops the first line of a message it encrypts.  */
	(void) write (out_fd, "\n", 1);
	(void) fprintf (stderr, "Request to sign a message\n");
    }
    else
    {
	/*  pgp -f  */
	argv[0] = "pgp";
	argv[1] = "-f";
	argv[2] = "+batchmode";
	argv[3] = NULL;
	(void) fprintf (stderr, "Request to decrypt a message\n");
    }
    remove_secring = write_secring ();
    /*  Start PGP  */
    if ( ( child = spawn_job ("pgp", argv,
			      &pgp_in_fd, &pgp_out_fd, &pgp_err_fd) ) < 0 )
    {
	(void) fprintf (stderr, "Error spawning PGP\n");
	if (remove_secring) (void) scrub_secring ();
	return (FALSE);
    }
    /*  Send passphrase  */
    len = strlen (passphrase);
    if (write (pgp_in_fd, passphrase, len) < len)
    {
	(void) fprintf (stderr, "Error writing passphrase\t%s\n",
			sys_errlist[errno]);
	(void) close (pgp_in_fd);
	(void) waitpid (child, &status, 0);
	if (remove_secring) (void) scrub_secring ();
	return (FALSE);
    }
    /*  Copy input message to PGP  */
    if ( !copy_data (pgp_in_fd, in_fd, FALSE) )
    {
	(void) fprintf (stderr, "Error writing data\t%s\n",
			sys_errlist[errno]);
	(void) close (pgp_in_fd);
	(void) waitpid (child, &status, 0);
	if (remove_secring) (void) scrub_secring ();
	return (FALSE);
    }
    (void) close (pgp_in_fd);
    if (waitpid (child, &status, 0) < 0)
    {
	(void) fprintf (stderr, "Error reaping child\t%s\n",
			sys_errlist[errno]);
	if (remove_secring) (void) scrub_secring ();
	return (FALSE);
    }
    if (status != 0)
    {
	(void) fprintf (stderr, "Bad child status: %d\n", status);
	if (remove_secring) (void) scrub_secring ();
	return (FALSE);
    }
    if (remove_secring) (void) scrub_secring ();
    (void) fprintf (stderr, "\n");
    return (TRUE);
}   /*  End Function process_message  */

static flag set_lockfile (filename, lock)
/*  This routine will create or delete a lockfile for a specified filename.
    The filename must be pointed to by  filename  .
    The routine creates or deletes the following lock files:
        <filename>.lock
	/tmp/<basename>.mlk
    where  basename  is the trailing path component of  filename  .
    The file will be created if the value of  lock  is TRUE, else it will be
    deleted.
    The routine returns TRUE on success, else it returns FALSE.
*/
char *filename;
flag lock;
{
    int fd;
    char *ptr;
    char lock_name[STRING_LENGTH];
    char mlk_name[STRING_LENGTH];
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    (void) strcpy (lock_name, filename);
    (void) strcat (lock_name, ".lock");
    ptr = strrchr (filename, '/');
    ptr = (ptr == NULL) ? filename : ptr + 1;
    (void) strcpy (mlk_name, "/tmp/");
    (void) strcat (mlk_name, ptr);
    (void) strcat (mlk_name, ".mlk");
    if (!lock)
    {
	if (unlink (lock_name) != 0)
	{
	    (void) fprintf (stderr, "Error removing file: \"%s\"\t%s\n",
			    lock_name, sys_errlist[errno]);
	    (void) unlink (mlk_name);
	    return (FALSE);
	}
	if (unlink (mlk_name) != 0)
	{
	    (void) fprintf (stderr, "Error removing file: \"%s\"\t%s\n",
			    mlk_name, sys_errlist[errno]);
	    return (FALSE);
	}
	(void) sync ();
	return (TRUE);
    }
    if ( ( fd = open (lock_name, O_RDWR | O_CREAT | O_EXCL,
		      S_IRUSR | S_IWUSR) ) < 0 )
    {
	if (errno == EEXIST)
	{
	    (void) fprintf (stderr, "lockfile: \"%s\" exists\n", lock_name);
	    return (FALSE);
	}
	(void) fprintf (stderr, "Error creating file: \"%s\"\t%s\n",
			lock_name, sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
    (void) fsync (fd);
    (void) close (fd);
    if ( ( fd = open (mlk_name, O_RDWR | O_CREAT | O_EXCL,
		      S_IRUSR | S_IWUSR) ) < 0 )
    {
	(void) unlink (lock_name);
	if (errno == EEXIST)
	{
	    (void) fprintf (stderr, "lockfile: \"%s\" exists\n", mlk_name);
	    return (FALSE);
	}
	(void) fprintf (stderr, "Error creating file: \"%s\"\t%s\n",
			mlk_name, sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
    (void) fsync (fd);
    (void) close (fd);
    return (TRUE);
}   /*  End Function set_lockfile  */

static flag set_fd_lock (fd, lock, no_filelocking)
/*  This routine will (un)lock a file associated with a file descriptor.
    The file descriptor must be given by  fd  .
    The file will be lock if the value of  lock  is TRUE, else it will be
    unlocked.
    If the value of  no_filelocking  is TRUE, the routine ignores
    all requests for file locking/unlocking and returns TRUE.
    The routine returns TRUE on success, else it returns FALSE.
*/
int fd;
flag lock;
flag no_filelocking;
{
    long pos;
#ifdef F_SETLK
    struct flock fl;
#endif
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    if (no_filelocking) return (TRUE);
#undef LOCKING_WORKS
#ifdef F_SETLK
#define LOCKING_WORKS
    fl.l_type = lock ? F_WRLCK : F_UNLCK;
    fl.l_whence = 0;
    fl.l_start = 0L;
    fl.l_len = 0L;
    if (fcntl (fd, F_SETLK, &fl) == -1)
    {
	(void) fprintf (stderr, "Error %slocking file: %d with  fcntl\t%s\n",
			lock ? "" : "un", fd, sys_errlist[errno]);
	return (FALSE);
    }
    return (TRUE);
#endif

#if !defined(LOCKING_WORKS) && defined(F_LOCK)
#define LOCKING_WORKS
    if ( (pos = lseek (fd, 0L, 0) ) == -1)
    {
	(void) fprintf (stderr, "Error seeking in file\t%s\n",
			sys_errlist[errno]);
	return (FALSE);
    }
    if (lockf (fd, lock ? F_LOCK : F_ULOCK, 0L) != 0)
    {
	
	(void) fprintf (stderr, "Error locking file with  lockf\t%s\n",
			sys_errlist[errno]);
	return (FALSE);
    }
    if ( (pos = lseek (fd, pos, 0) ) == -1)
    {
	(void) fprintf (stderr, "Error seeking in file\t%s\n",
			sys_errlist[errno]);
	return (FALSE);
    }
    return (TRUE);
#endif

#ifndef LOCKING_WORKS
    (void) fprintf (stderr, "File locking not implemented\n");
    my_exit (RV_UNDEF_ERROR);
    return (FALSE);
#endif
}   /*  End Function set_fd_lock  */

static void check_and_process_mail ()
/*  This routine will check for any incoming mail and will process that mail.
    The routine returns nothing.
*/
{
    flag pgp_data, bad_previous_pgp, read_more;
    flag remove_secring = FALSE;
    flag first_pgp = TRUE;
    int ch, len;
    int fd, oflags;
    int in_fd, out_fd, err_fd;
    int child;
    int status;
    long bytes_read, pgp_offset;
    int line_length;
    FILE *in_fp, *out_fp;
    struct stat statbuf;
    char line[STRING_LENGTH];
    char *argv[3];
    char *pgp_start_line = "-----BEGIN PGP MESSAGE-----\n";
    extern flag incoming_locked;
    extern flag no_filelocking;
    extern char incoming_spool[STRING_LENGTH];
    extern char outgoing_spool[STRING_LENGTH];
    extern char passphrase[STRING_LENGTH];
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    if (stat (incoming_spool, &statbuf) != 0)
    {
	if (errno == ENOENT) return;
	(void) fprintf (stderr, "Error stat'ing file: \"%s\"\t%s\n",
			incoming_spool, sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
    if (statbuf.st_size < 1) return;
    (void) fprintf (stderr, "%d bytes of mail...\n", statbuf.st_size);
    /*  Lock output file  */
    if ( !set_lockfile (outgoing_spool, TRUE) ) return;
    if ( ( out_fp = fopen (outgoing_spool, "a") ) == NULL )
    {
	(void) fprintf (stderr, "Error opening file: \"%s\"\t%s\n",
			outgoing_spool, sys_errlist[errno]);
	(void) set_lockfile (outgoing_spool, FALSE);
	myexit (RV_SYS_ERROR);
    }
    if ( !set_fd_lock (fileno (out_fp), TRUE, no_filelocking) )
    {
	(void) fprintf (stderr, "Error locking output spool: waiting\n");
	(void) fclose (out_fp);
	(void) set_lockfile (outgoing_spool, FALSE);
	return;
    }
    /*  Now lock input file  */
    if ( !set_lockfile (incoming_spool, TRUE) )
    {
	(void) set_fd_lock (fileno (out_fp), FALSE, no_filelocking);
	(void) fclose (out_fp);
	(void) set_lockfile (outgoing_spool, FALSE);
	return;
    }
    if ( ( in_fp = fopen (incoming_spool, "r+") ) == NULL )
    {
	(void) fprintf (stderr, "Error opening file: \"%s\"\t%s\n",
			incoming_spool, sys_errlist[errno]);
	(void) set_fd_lock (fileno (out_fp), FALSE, no_filelocking);
	(void) fclose (out_fp);
	(void) set_lockfile (outgoing_spool, FALSE);
	(void) set_lockfile (incoming_spool, FALSE);
	myexit (RV_SYS_ERROR);
    }
    if ( !set_fd_lock (fileno (in_fp), TRUE, no_filelocking) )
    {
	(void) fprintf (stderr, "Error locking input spool: waiting\n");
	(void) set_fd_lock (fileno (out_fp), FALSE, no_filelocking);
	(void) fclose (out_fp);
	(void) set_lockfile (outgoing_spool, FALSE);
	(void) fclose (in_fp);
	(void) set_lockfile (incoming_spool, FALSE);
	return;
    }
    /*  Process incoming and produce outgoing  */
    bytes_read = 0;
    pgp_data = FALSE;
    bad_previous_pgp = FALSE;
    (void) fprintf (stderr, "Spoolfiles locked\n");
    read_more = TRUE;
    while (read_more)
    {
	if (fgets (line, STRING_LENGTH, in_fp) == NULL)
	{
	    /*  Must be EndOfFile  */
	    if (!pgp_data)
	    {
		/*  Nothing special, just stop trying to read  */
		read_more = FALSE;
		continue;
	    }
	    (void) fprintf (stderr, "\nPGP message terminated prematurely\n");
	    (void) fprintf (stderr, "Message not decrypted\n");
	    (void) fprintf (out_fp, "PGP message terminated prematurely\n");
	    (void) fprintf (out_fp, "Message not decrypted\n\n");
	    /*  PGP message terminated without end line: plain copy  */
	    (void) kill (child, SIGKILL);
	    (void) waitpid (child, &status, 0);
	    if (fseek (in_fp, pgp_offset, SEEK_SET) != 0)
	    {
		(void) fprintf (stderr, "Error seeking\t%s\n",
				sys_errlist[errno]);
		myexit (RV_SYS_ERROR);
	    }
	    pgp_data = FALSE;
	    bad_previous_pgp = TRUE;
	    bytes_read = pgp_offset;
	    continue;
	}
	/*  New line of data is available  */
	line_length = strlen (line);
	bytes_read += line_length;
	if (pgp_data)
	{
	    /*  Check to see if PGP is still running  */
	    switch ( waitpid (child, &status, WNOHANG | WUNTRACED) )
	    {
	      case 0:
		/*  PGP still running  */
		break;
	      case -1:
		(void) fprintf (stderr, "Error getting child status\t%s\n",
				sys_errlist[errno]);
		myexit (RV_SYS_ERROR);
		break;
	      default:
		if ( WIFSTOPPED (status) )
		{
		    /*  PGP was stopped: probably because message was encrypted
			with that fucking "For-your-eyes-only" option, and so
			PGP *insists* on asking if you want to view the
			message.  */
		    /*  Note that at this point, this condition is not likely,
			rather, reaping the child will show this condition.  */
		    (void) fprintf(stderr,
				   "\nPGP stopped while decrypting, probably");
		    (void) fprintf (stderr,
				    " because it was encrypted with that\n");
		    (void) fprintf (stderr,
				    "fucking \"For-your-eyes-only\" option. ");
		    (void) fprintf (out_fp,
				    "PGP stopped while decrypting, probably");
		    (void) fprintf (out_fp,
				    " because it was encrypted with that\n");
		    (void) fprintf (out_fp,
				    "fucking \"For-your-eyes-only\" option. ");
		}
		else
		{
		    (void) fprintf (stderr, "\nBad PGP child status: %d\n",
				    status);
		    (void) fprintf (out_fp, "Bad PGP child status: %d\n",
				    status);
		}
		(void) fprintf (stderr, "Message not decrypted\n");
		(void) fprintf (out_fp, "Message not decrypted\n");
		(void) kill (child, SIGKILL);
		(void) waitpid (child, &status, 0);
		if (fseek (in_fp, pgp_offset, SEEK_SET) != 0)
		{
		    (void) fprintf (stderr, "Error seeking\t%s\n",
				    sys_errlist[errno]);
		    myexit (RV_SYS_ERROR);
		}
		pgp_data = FALSE;
		bad_previous_pgp = TRUE;
		bytes_read = pgp_offset;
		continue;
/*
		break;
*/
	    }
	}
	if (st_nicmp (line, "subject:", 8) == 0)
	{
	    (void) fprintf (stderr, "> %s", line);
	}
	if ( (strcmp (line, pgp_start_line) == 0) && pgp_data )
	{
	    /*  Hm. Looks like the last PGP message wasn't terminated
		properly. I suppose PGP gave no output  */
	    (void) fprintf (stderr,
			    "\nStart of new PGP message without termination");
	    (void) fprintf (stderr,
			    " of previous message\nMessage not decrypted\n");
	    (void) fprintf (out_fp,
			    "Start of new PGP message without termination");
	    (void) fprintf (out_fp,
			    " of previous message\nMessage not decrypted\n");
	    /*  PGP message terminated without end line: plain copy  */
	    (void) kill (child, SIGKILL);
	    (void) waitpid (child, &status, 0);
	    if (fseek (in_fp, pgp_offset, SEEK_SET) != 0)
	    {
		(void) fprintf (stderr, "Error seeking\t%s\n",
				sys_errlist[errno]);
		myexit (RV_SYS_ERROR);
	    }
	    pgp_data = FALSE;
	    bad_previous_pgp = TRUE;
	    bytes_read = pgp_offset;
	    continue;
	}
	if ( (strcmp (line, pgp_start_line) == 0) &&
	    (passphrase[0] != '\0') && (!bad_previous_pgp) )
	{
	    pgp_data = TRUE;
	    pgp_offset = ftell (in_fp) - line_length;
	    in_fd = -1;
	    out_fd = fileno (out_fp);
	    err_fd = ERROR_FD;
	    (void) fprintf (out_fp,
			    "PGPdaemon %s: automatically decrypted message:\n\n",
			    VERSION);
	    (void) fflush (out_fp);
	    (void) fprintf (stderr, "DECRYPTING...\n");
	    if (first_pgp)
	    {
		remove_secring = write_secring ();
		first_pgp = FALSE;
	    }
	    /*  Start PGP  */
	    /*  pgp -f  */
	    argv[0] = "pgp";
	    argv[1] = "-f";
	    argv[2] = NULL;
	    if ( ( child = spawn_job ("pgp", argv, &in_fd, &out_fd, &err_fd) )
		< 0 )
	    {
		(void) fprintf (stderr, "Error spawning PGP\n");
		myexit (RV_SYS_ERROR);
	    }
	    /*  Send passphrase  */
	    len = strlen (passphrase);
	    if (write (in_fd, passphrase, len) < len)
	    {
		(void) fprintf (stderr, "Error writing passphrase\t%s\n",
				sys_errlist[errno]);
		(void) close (in_fd);
		(void) waitpid (child, &status, 0);
		myexit (RV_SYS_ERROR);
	    }
	}
	if (pgp_data)
	{
	    if (write (in_fd, line, line_length) < line_length)
	    {
		if (errno == EPIPE)
		{
		    /*  PGP dropped out for some reason: plain copy  */
		    (void) kill (child, SIGKILL);
		    (void) waitpid (child, &status, 0);
		    if (fseek (in_fp, pgp_offset, SEEK_SET) != 0)
		    {
			(void) fprintf (stderr, "Error seeking\t%s\n",
					sys_errlist[errno]);
			myexit (RV_SYS_ERROR);
		    }
		    pgp_data = FALSE;
		    bad_previous_pgp = TRUE;
		    bytes_read = pgp_offset;
		    continue;
		}
		(void) fprintf (stderr, "Error writing data to PGP\t%s\n",
				sys_errlist[errno]);
		myexit (RV_SYS_ERROR);
	    }
	}
	else
	{
	    if (fputs (line, out_fp) == EOF)
	    {
		(void) fprintf (stderr, "Error writing\t%s\n",
				sys_errlist[errno]);
		(void) fprintf (stderr, "Error copying data\n");
		(void) set_fd_lock (fileno (out_fp), FALSE, no_filelocking);
		(void) fclose (out_fp);
		(void) set_lockfile (outgoing_spool, FALSE);
		(void) set_fd_lock (fileno (in_fp), FALSE, no_filelocking);
		(void) fclose (in_fp);
		(void) set_lockfile (incoming_spool, FALSE);
		myexit (RV_UNDEF_ERROR);
	    }
	}
	if ( (strcmp (line, pgp_start_line) == 0) && bad_previous_pgp )
	{
	    /*  This line started off a bad PGP message on the previous round.
		Now that we've gone past the line, we can clear the
		bad_previous_pgp  flag.  */
	    bad_previous_pgp = FALSE;
	    pgp_data = FALSE;  /*  What the hell, clear it just in case  */
	    continue;
	}
	if (strcmp (line, "-----END PGP MESSAGE-----\n") == 0)
	{
	    bad_previous_pgp = FALSE;
	    if (!pgp_data) continue;
	    pgp_data = FALSE;
	    (void) close (in_fd);
	    (void) waitpid (child, &status, WUNTRACED);
	    if (status != 0)
	    {
		if ( WIFSTOPPED (status) )
		{
		    /*  PGP was stopped: probably because message was encrypted
			with that fucking "For-your-eyes-only" option, and so
			PGP *insists* on asking if you want to view the
			message.  */
		    (void) kill (child, SIGKILL);
		    (void) fprintf(stderr,
				   "\nPGP stopped while decrypting, probably");
		    (void) fprintf (stderr,
				    " because it was encrypted with that\n");
		    (void) fprintf (stderr,
				    "fucking \"For-your-eyes-only\" option. ");
		    (void) fprintf (out_fp,
				    "PGP stopped while decrypting, probably");
		    (void) fprintf (out_fp,
				    " because it was encrypted with that\n");
		    (void) fprintf (out_fp,
				    "fucking \"For-your-eyes-only\" option. ");
		}
		else
		{
		    (void) fprintf (stderr, "Bad PGP child status: %d\n",
				    status);
		    (void) fprintf (out_fp, "Bad PGP child status: %d\n",
				    status);
		}
		(void) fprintf (stderr, "Message not decrypted\n");
		(void) fprintf (out_fp, "Message not decrypted\n\n");
		if (fseek (in_fp, pgp_offset, SEEK_SET) != 0)
		{
		    (void) fprintf (stderr, "Error seeking\t%s\n",
				    sys_errlist[errno]);
		    myexit (RV_SYS_ERROR);
		}
		bad_previous_pgp = TRUE;
		bytes_read = pgp_offset;
		continue;
	    }
	    (void) fprintf (stderr, "\nDECRYPTED\n");
	    (void) fprintf (out_fp, "\nEnd automatically decrypted message\n");
	}
    }
    if (bytes_read < statbuf.st_size)
    {
	(void) fprintf (stderr, "Error reading\t%s\n", sys_errlist[errno]);
	(void) fprintf (stderr, "Error copying data\n");
	(void) set_fd_lock (fileno (out_fp), FALSE, no_filelocking);
	(void) fclose (out_fp);
	(void) set_lockfile (outgoing_spool, FALSE);
	(void) set_fd_lock (fileno (in_fp), FALSE, no_filelocking);
	(void) fclose (in_fp);
	(void) set_lockfile (incoming_spool, FALSE);
	myexit (RV_UNDEF_ERROR);
    }
    (void) fflush (out_fp);
    (void) fsync ( fileno (out_fp) );
    (void) fprintf (stderr, "Unlocking spoolfiles...");
    /*  Unlock output spoolfile  */
    (void) set_fd_lock (fileno (out_fp), FALSE, no_filelocking);
    (void) fclose (out_fp);
    (void) set_lockfile (outgoing_spool, FALSE);
#ifdef dummy
    /*  Remove input spoolfile  */
    if (unlink (incoming_spool) != 0)
    {
	(void) fprintf (stderr, "Error removing file: \"%s\"\t%s\n",
			incoming_spool, sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
    /*  Unlock input spoolfile  */
#else
    /*  Empty input spoolfile  */
    oflags = O_WRONLY | O_TRUNC;
#  ifdef O_SYNC
    oflags |= O_SYNC;
#  endif
    if ( ( fd = open (incoming_spool, oflags, 0) ) < 0 )
    {
	(void) fprintf (stderr, "Error truncating file: \"%s\"\t%s\n",
			incoming_spool, sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
#  ifndef O_SYNC
    (void) fsync (fd);
#  endif
    (void) close (fd);
    (void) set_fd_lock (fileno (in_fp), FALSE, no_filelocking);
#endif
    (void) fclose (in_fp);
    (void) set_lockfile (incoming_spool, FALSE);
    (void) fprintf (stderr, "\tmail processed\n");
    if (remove_secring) (void) scrub_secring ();
}   /*  End Function check_and_process_mail  */

static void read_config (pgppath, mailwait, interval, check_mail)
/*  This routine will read the configuration file:  $PGPPATH/PGPdaemon.config
    The value of the PGPPATH environment variable must be pointed to by
    pgppath  .
    If the "MAILWAIT" keyword is present, the value TRUE will be written to the
    storage pointed to by  mailwait  .
    If the "MAIL_CHECK_INTERVAL" keyword is present, the associated integer
    value will be written to the storage pointed to by  interval  .
    If the value of  check_mail  is TRUE, the routine will check for the
    writability of the mail spool directory.
    The routine returns nothing.
*/
char *pgppath;
flag *mailwait;
long *interval;
flag check_mail;
{
    char *keyword, *rest;
    char *tmp;
    FILE *fp;
    char config_filename[STRING_LENGTH];
    char txt[STRING_LENGTH];
    extern char my_userid[STRING_LENGTH];
    extern char incoming_spool[STRING_LENGTH];
    extern char outgoing_spool[STRING_LENGTH];
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    (void) sprintf (config_filename, "%s/PGPdaemon.config", pgppath);
    if ( ( fp = fopen (config_filename, "r") ) == NULL )
    {
	(void) fprintf (stderr, "Error reading file: \"%s\"\t%s\n",
			config_filename, sys_errlist[errno]);
	myexit (RV_CANNOT_OPEN);
    }
    /*  Read in lines  */
    m_clear (my_userid, STRING_LENGTH);
    m_clear (incoming_spool, STRING_LENGTH);
    m_clear (outgoing_spool, STRING_LENGTH);
    while (fgets (txt, STRING_LENGTH - 1, fp) != NULL)
    {
	/*  Strip newline  */
	txt[strlen (txt) - 1] = '\0';
	if ( (int) strlen (txt) < 1 ) continue;
	if (txt[0] == '#') continue;
	if ( ( keyword = ex_str (txt, &rest) ) == NULL ) continue;
	if (strcmp (keyword, "USERID") == 0) (void) strcpy (my_userid, rest);
	else if (strcmp (keyword, "MAILWAIT") == 0) *mailwait = TRUE;
	else if (strcmp (keyword, "IN_SPOOL_DIR") == 0)
	{
	    (void) strcpy (incoming_spool, rest);
	}
	else if (strcmp (keyword, "OUT_SPOOL_FILE") == 0)
	{
	    (void) strcpy (outgoing_spool, rest);
	}
	else if (strcmp (keyword, "MAIL_CHECK_INTERVAL") == 0)
	{
	    if (rest == NULL)
	    {
		(void) fprintf (stderr, "No interval specified\n");
		myexit (RV_BAD_DATA);
	    }
	    if ( ( *interval = atol (rest) ) < MIN_INTERVAL )
	    {
		(void) fprintf (stderr,
				"Interval: %ld is less than minimum: %ld\n",
				*interval, MIN_INTERVAL);
		myexit (RV_BAD_DATA);
	    }
	    if (*interval > MAX_INTERVAL)
	    {
		(void) fprintf (stderr,
				"Interval: %ld is greater than maximum: %ld\n",
				*interval, MAX_INTERVAL);
		myexit (RV_BAD_DATA);
	    }
	}
	else
	{
	    (void) fprintf (stderr, "Illegal config line: \"%s\"\n", txt);
	    myexit (RV_BAD_DATA);
	}
	m_free (keyword);
    }
    if (my_userid[0] == '\0')
    {
	(void) fprintf (stderr, "No USERID specified in config file\n");
	myexit (RV_BAD_DATA);
    }
    if (incoming_spool[0] == '\0')
    {
	/*  No existing spool directory named  */
	if ( ( tmp = getenv ("MAIL") ) == NULL )
	{
	    (void) strcpy (incoming_spool, MAIL_SPOOL_DIR);
	    (void) strcat (incoming_spool, "/");
	    (void) strcat ( incoming_spool, getenv ("USER") );
	}
	else
	{
	    (void) strcpy (incoming_spool, tmp);
	}
    }
    else
    {
	(void) strcat (incoming_spool, "/");
	(void) strcat ( incoming_spool, getenv ("USER") );
    }
    if (strncmp (incoming_spool, "~/", 2) == 0)
    {
	/*  Expand leading "~/"  */
	(void) strcpy ( txt, getenv ("HOME") );
	(void) strcat (txt, "/");
	(void) strcat (txt, incoming_spool + 2);
	(void) strcpy (incoming_spool, txt);
    }
    /*  Extract spool directory  */
    (void) strcpy (txt, incoming_spool);
    if ( ( tmp = strrchr (txt, '/') ) == NULL )
    {
	(void) fprintf (stderr, "No '/' in spool directory name: \"%s\"\n",
			txt);
	myexit (RV_BAD_DATA);
    }
    *tmp = '\0';
    if ( check_mail && (access (txt, W_OK) != 0) )
    {
	if (errno == ENOENT)
	{
	    (void) fprintf (stderr, "Spool directory: \"%s\" does not exist\n",
			    txt);
	    myexit (RV_FILE_NOT_FOUND);
	}
	if (errno == EACCES)
	{
	    (void) fprintf (stderr,
			    "Spool directory: \"%s\" is not writable\n",
			    txt);
	    myexit (RV_WRITE_ERROR);
	}
	(void)fprintf(stderr,
		      "Error checking access to spool directory: \"%s\"\t%s\n",
		      txt);
	myexit (RV_SYS_ERROR);
    }
    if (outgoing_spool[0] == '\0')
    {
	(void) sprintf ( outgoing_spool, "%s/decrypted-mail",
			getenv ("HOME") );
    }
    else
    {
	if (strncmp (outgoing_spool, "~/", 2) == 0)
	{
	    /*  Expand leading "~/"  */
	    (void) strcpy ( txt, getenv ("HOME") );
	    (void) strcat (txt, "/");
	    (void) strcat (txt, outgoing_spool + 2);
	    (void) strcpy (outgoing_spool, txt);
	}
    }
}   /*  End Function read_config  */

static flag write_secring ()
/*  This routine will write the secret keyring file.
    The routine returns TRUE if the file did not already exist,
    else it returns FALSE (indicating there is an existing file which will not
    be overwritten).
*/
{
    int fd;
    char filename[STRING_LENGTH];
    extern SECRING_SIZE_TYPE secring_bytes;
    extern char *secring_buf;
    extern char *pgppath;
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    (void) sprintf (filename, "%s/secring.pgp", pgppath);
    if ( ( fd = open (filename, O_WRONLY | O_CREAT | O_EXCL,
		      S_IRUSR | S_IWUSR) ) < 0 )
    {
	if (errno == EEXIST)
	{
	    return (FALSE);
	}
	(void) fprintf (stderr, "Error creating file: \"%s\"\t%s\n",
			filename, sys_errlist[errno]);
	myexit (RV_SYS_ERROR);
    }
    if (write (fd, secring_buf, secring_bytes) < secring_bytes)
    {
	(void) fprintf (stderr, "Error writing\t%s\n", sys_errlist[errno]);
	(void) close (fd);
	myexit (RV_SYS_ERROR);
    }
    (void) close (fd);
    return (TRUE);
}   /*  End Function write_secring  */

static flag scrub_secring ()
/*  This routine will scrub the secret keyring file.
    The routine returns TRUE on success, else it returns FALSE.
*/
{
    int fd;
    char filename[STRING_LENGTH];
    extern char *pgppath;

    (void) sprintf (filename, "%s/secring.pgp", pgppath);
    return ( scrub_file (filename, TRUE) );
}   /*  End Function scrub_secring  */

static int get_rand ()
/*  This routine will compute a random number with a Uniform distribution.
    The routine will return the number.
*/
{   
    struct timeval tv;
    struct timezone tz;
    static int first_time = TRUE;

    if (first_time)
    {
	first_time = FALSE; 
	gettimeofday (&tv, &tz);
	tv.tv_sec ^= tv.tv_usec;
	srand (tv.tv_sec);
    }
    return ( rand () );
}   /*  End Function get_rand  */
