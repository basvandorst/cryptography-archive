/*  pgpsendmail.c

    Source file for  PGPsendmail  (wrapper to sendmail).

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


    Written by      Richard Gooch   3-APR-1994

    Updated by      Richard Gooch   9-APR-1994

    Updated by      Richard Gooch   27-MAY-1994: Created  freshen_keylist  .

    Updated by      Richard Gooch   30-MAY-1994: Created  check_recipients  and
  check_for_header_flags  .

    Updated by      Richard Gooch   2-JUN-1994: Included full message body on
  bounce.

    Updated by      Richard Gooch   5-JUN-1994: Added signing facility.

    Updated by      Richard Gooch   10-JUN-1994: Changed from using  stricmp
  to  st_icmp  ,since some platforms don't have  stricmp  .

    Updated by      Richard Gooch   12-JUN-1994: Fixed bug in  main  where a
  constant string was passed to  process_address  .

    Updated by      Richard Gooch   14-JUN-1994: Passed PID of real  sendmail
  to  check_for_header_flags  so that it can abort the sending process.

    Updated by      Richard Gooch   17-JUN-1994: Added warning if root runs
  with PGPPATH set and added debugging facility.

    Updated by      Richard Gooch   18-JUN-1994: Added startup test for
  $PGPPATH/ranseed.bin  .

    Updated by      Richard Gooch   26-JUN-1994: Added more debug logging
  output.

    Updated by      Richard Gooch   28-JUN-1994: Added more debug logging
  output in  read_line  .

    Updated by      Richard Gooch   30-JUN-1994: Made "GLOBAL" option the first
  set of flags to be processed.

    Updated by      Richard Gooch   5-JUL-1994: Changed to use of  m_copy  .

    Updated by      Richard Gooch   14-JUL-1994: Checked response flag that
  PGPdaemon now returns.

    Updated by      Richard Gooch   14-JUL-1994: Added more debug messages at
  the end and discarded  /tmp/PGPsendmail.err.<PID>  files even if receipt is
  discarded.

    Updated by      Richard Gooch   20-JUL-1994: Added ability to sign all
  outgoing messages (even those going out in plaintext).

    Updated by      Richard Gooch   21-JUL-1994: Added declarations in
  transmit_plaintext  for  errno  and  sys_errlist  .

    Updated by      Richard Gooch   15-AUG-1994: Added  add_key  parameter to
  transmit_plaintext  .

    Updated by      Richard Gooch   22-AUG-1994: Automatically execute real
  sendmail  if running as root (realUID = 0), and trap for invocation as
  mailq  and  newaliases  .

    Updated by      Richard Gooch  23-AUG-1994: Allow continuation of address
  lines with trailing ','.

    Updated by      Richard Gooch   21-NOV-1994: Supported  LOGNAME
  environment variable.

    Updated by      Richard Gooch   3-DEC-1994: Took account of changes to
  connect_to_pgpdaemon  .

    Updated by      Richard Gooch   6-DEC-1994: Wrote message in debug file
  if no $PGPPATH/randseed.bin file exists.

    Updated by      Richard Gooch   13-DEC-1994: Added support for address
  format:  "Real Name"@email.address  in  extract_address  .

    Last updated by Richard Gooch   17-DEC-1994: Fixed silly bug where username
  was not set if no debug file existed.


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
#include "pgpsendmail.h"

#ifdef TEST1
#  undef VERSION
#  define VERSION "private-test1"
#endif

#define SEPARATOR_LENGTH 70


/*  Private data  */
static char *buffer = NULL;
static int buf_len = 0;
static char **recipients = NULL;
static char *pgppath = NULL;
static FILE *debug_fp = NULL;
static char *username = NULL;


STATIC_FUNCTION (flag read_line, (FILE *debug_fp) );
STATIC_FUNCTION (flag process_addresses,
		 (char *list, char *pgppath, option_type *options,
		  FILE *debug_fp, flag *trailing_comma) );
STATIC_FUNCTION (void return_badnews, (char *message) );
STATIC_FUNCTION (char *extract_address, (char *str, char **rest) );
STATIC_FUNCTION (flag check_for_header_flags,
		 (char *line, option_type *options, int sendmail_pid,
		  FILE *debug_fp) );
STATIC_FUNCTION (void sig_handler, () );
STATIC_FUNCTION (void transmit_plaintext, 
		 (flag sign, FILE *debug_fp, int sendmail_pid,
		  flag no_advertising, flag add_key) );


void main (argc, argv, envp)
int argc;
char *argv[];
char *envp[];
{
    int tmp;

    flag bool_val;
    option_type options;
    flag header = TRUE;
    flag quoted_printables = FALSE;
    flag continuation_line = FALSE;
    flag dummy;
    int count;
    int ch;
    int tmpfd;
    int sendmail_in_fd, sendmail_pid;
    int out_fd, err_fd;
    int pgp_in_fd, pgp_pid;
    int status;
    int to_fd, from_fd;
    struct stat statbuf;
    char *bad_recipient;
    char *ptr1, *ptr2;
    char *real_sendmail = SENDMAIL;
    char txt[LINE_LENGTH];
    extern FILE *debug_fp;
    extern char *buffer;
    extern char *pgppath;
    extern char *username;
    extern char **recipients;
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    if (argv[0] == NULL)
    {
	(void) fprintf (stderr, "argv[0] is NULL!\n");
	exit (RV_UNDEF_ERROR);
    }
    /*  Try to trap if SENDMAIL references PGPsendmail. I hope this works.
	I only do this to protect people from themselves: if they *read* the
	documentation, this wouldn't be necessary. Gosh, I'm such a nice
	person ;-)  */
    ptr1 = strrchr (real_sendmail, '/');
    ptr1 = (ptr1 == NULL) ? real_sendmail : ptr1 + 1;
    ptr2 = strrchr (argv[0], '/');
    ptr2 = (ptr2 == NULL) ? argv[0] : ptr2 + 1;
    if (strcmp (ptr1, ptr2) == 0)
    {
	(void) fprintf (stderr, "SENDMAIL refers back to PGPsendmail!\n");
	(void) fprintf (stderr,
			"Someone didn't read the installation notes...\n");
	exit (RV_UNDEF_ERROR);
    }
    if ( (strcmp (ptr2, "mailq") == 0) || (strcmp (ptr2, "newaliases") == 0) )
    {
	(void) sleep (1);
	(void) execve (real_sendmail, argv, envp);
	exit (1);
    }
    if ( ( pgppath = getenv ("PGPPATH") ) == NULL )
    {
	/*  Delay 1 second in case the installer didn't read the instructions
	    and SENDMAIL actually refers back to me. Without this delay, the
	    machine will thrash, continuously exec'ing SENDMAIL.  */
	(void) sleep (1);
	(void) execve (real_sendmail, argv, envp);
	exit (1);
    }
    if (getuid () == 0)
    {
	/*  Hm. root shouldn't be using this.  */
#ifdef DISALLOW_ROOT
	(void) sleep (1);
	(void) execve (real_sendmail, argv, envp);
	exit (1);
#else
	(void) fprintf (stderr,
			"It is not recommended for root to run PGPsendmail\n");
	(void) fprintf (stderr, "with the environment variable PGPPATH set\n");
	(void)fprintf(stderr,
		      "Sleeping 15 seconds to give you a chance to kill me\n");
	(void) sleep (15);
#endif
    }
    (void) sprintf (txt, "%s/PGPsendmail.debug", pgppath);
    if (access (txt, F_OK) == 0)
    {
	/*  Turn on verbose debugging  */
	(void) sprintf ( txt, "/tmp/PGPsendmail.debug.%d", getuid () );
	if ( ( debug_fp = fopen (txt, "w") ) == NULL )
	{
	    (void) fprintf (stderr,
			    "Error opening debugging file: \"%s\"\t%s\n",
			    txt, sys_errlist[errno]);
	    exit (RV_CANNOT_OPEN);
	}
	(void) fprintf (debug_fp, "Debug file for PGPsendmail %s opened\n",
			VERSION);
	(void) fflush (debug_fp);
    }
    else
    {
	debug_fp = NULL;
    }
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Arguments to PGPsendmail:  ");
	for (count = 0; count < argc; ++count)
	{
	    (void) fprintf (debug_fp, "\"%s\"  ", argv[count]);
	}
	(void) fprintf (debug_fp, "\n");
#ifdef NeXT
	(void) fprintf (debug_fp, "Environment:\n");
	for (count = 0; envp[count] != NULL; ++count)
	{
	    (void) fprintf (debug_fp, "  \"%s\"\n", envp[count]);
	}
#endif
	(void) fflush (debug_fp);
    }
    if ( ( username = getenv ("USER") ) == NULL )
    {
	if ( ( username = getenv ("LOGNAME") ) == NULL )
	{
	    if (debug_fp != NULL)
	    {
		(void) fprintf (debug_fp,
				"No USER or LOGNAME environment variable!\n");
		(void) fflush (debug_fp);
	    }
	    exit (RV_UNDEF_ERROR);
	}
    }
    (void) signal (SIGPIPE, sig_handler);
    options.secure = FALSE;
    options.insecure = FALSE;
    options.receipt = TRUE;
    options.add_key = FALSE;
    options.no_advertising = FALSE;
    options.sign = FALSE;
    options.global_done = FALSE;
    (void) sprintf ( txt, "/tmp/PGPsendmail.err.%d", getpid () );
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Opening file: \"%s\"\n", txt);
	(void) fflush (debug_fp);
    }
    if ( ( tmpfd = open (txt, O_CREAT | O_TRUNC | O_WRONLY, 0644) )
	< 0 ) exit (1);
    if (dup2 (tmpfd, 2) == -1) exit (1);
    (void) fprintf (stderr, "To: %s\nSubject: PGPsendmail response\n\n",
		    username);
    (void) fprintf (stderr, "PGPsendmail response message:\n\n");
    /*  Sanity checks for files the user should have  */
    (void) sprintf (txt, "%s/randseed.bin", pgppath);
    if (access (txt, R_OK) != 0)
    {
	(void) fprintf (stderr, "Could not find file: \"%s\"\t%s\n",
			txt, sys_errlist[errno]);
	if (debug_fp != NULL)
	{
	    (void) fprintf(debug_fp,
			   "Could not find file: \"%s\"\t%s\nPGP would hang\n",
			   txt, sys_errlist[errno]);
	    (void) fflush (debug_fp);
	}
	return_badnews ("PGP would hang");
	exit (RV_SYS_ERROR);
    }
    /*  Process "GLOBAL" option in config file  */
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Processing GLOBAL option\n");
	(void) fflush (debug_fp);
    }
    (void) translate_recipient (NULL, pgppath, &options);
    /*  Check input descriptor  */
    if (fstat (INPUT_FD, &statbuf) != 0)
    {
	(void) fprintf (stderr, "Error getting stats on  stdin\t%s\n",
			sys_errlist[errno]);
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Error getting stats on  stdin\t%s\n",
			    sys_errlist[errno]);
	    (void) fflush (debug_fp);
	}
	exit (RV_SYS_ERROR);
    }
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp,
			"stdin  has: %d bytes with: %d hard links  mode: %o  owner: %d\n",
			statbuf.st_size, statbuf.st_nlink, statbuf.st_mode,
			statbuf.st_uid);
	(void) fflush (debug_fp);
    }
    if ( !S_ISFIFO (statbuf.st_mode) && !S_ISREG (statbuf.st_mode) )
    {
	(void) fprintf (stderr, "stdin  is not a pipe/FIFO, type is: %o\n",
			statbuf.st_mode & S_IFMT);
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp,
			    "stdin  is not a pipe/FIFO, type is: %o\n",
			    statbuf.st_mode & S_IFMT);
	    (void) fflush (debug_fp);
	}
/*
	exit (RV_UNDEF_ERROR);
*/
    }
    else
    {
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "stdin  is a %s: good\n",
			    S_ISFIFO (statbuf.st_mode) ? "pipe/FIFO" :
			    "regular file");
	    (void) fflush (debug_fp);
	}
    }
#ifdef TEST1
    if (debug_fp != NULL)
    {
	(void) fprintf ( debug_fp, "old_off: %ld\n",
			lseek (INPUT_FD, 0, SEEK_SET) );
	(void) fprintf (debug_fp, "Reading one line from header...\n");
	(void) fflush (debug_fp);
    }
    if ( ( tmp = read (INPUT_FD, txt, statbuf.st_size) ) < statbuf.st_size )
    {
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Error reading: got: %d bytes\t%s\n",
			    tmp, sys_errlist[errno]);
	    (void) fflush (debug_fp);
	}
	exit (RV_BAD_DATA);
    }
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Header: \"%s\"\n", txt);
	(void) fflush (debug_fp);
    }
    exit (RV_OK);
#endif  /*  TEST1  */
    /*  Start real  sendmail  */
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Starting real  sendmail  ...\n");
	(void) fflush (debug_fp);
    }
    sendmail_in_fd = -1;
    out_fd = OUTPUT_FD;
    err_fd = ERROR_FD;
    /*  Prevent fork bombs bringing the system to it's knees by waiting a bit.
	Fork bombs occur when SENDMAIL references PGPsendmail
	Hopefully this is actually trapped earlier: but I like suspenders *and*
	belt.  */
    (void) sleep (1);
    if ( ( sendmail_pid = spawn_job (real_sendmail, argv, &sendmail_in_fd,
				     &out_fd, &err_fd) ) < 0 )
    {
	return_badnews ("could not spawn real sendmail");
	exit (RV_UNDEF_ERROR);
    }
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Freshening keylist if not already fresh\n");
	(void) fflush (debug_fp);
    }
    if ( !freshen_keylist (pgppath) )
    {
	return_badnews ("could not generate keylist");
	exit (RV_SYS_ERROR);
    }
    /*  Make stdout go to  sendmail  */
    (void) dup2 (sendmail_in_fd, OUTPUT_FD);
    /*  An awful hack to insert fake recipients  */
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Adding fake recipients\n");
	(void) fflush (debug_fp);
    }
    (void) strcpy (txt, "pgp, -feat, +batchmode");
    if ( !process_addresses (txt, NULL, NULL, debug_fp, &dummy) )
    {
	return_badnews ("add fake recipients");
	kill (sendmail_pid, SIGKILL);
	exit (1);
    }
    /*  Process header  */
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Starting to process header...\n");
	(void) fflush (debug_fp);
    }
    while (header)
    {
	if (read_line (debug_fp) != TRUE)
	{
	    if (debug_fp != NULL)
	    {
		(void) fprintf (debug_fp, "Could not read header line\n");
		(void) fflush (debug_fp);
	    }
	    return_badnews ("could not read header");
	    exit (1);
	}
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "  Processing header line: \"%s\"\n",
			    buffer);
	    (void) fflush (debug_fp);
	}
	if ( check_for_header_flags (buffer, &options, sendmail_pid,
				     debug_fp) ) continue;
	fputs (buffer, stdout);
	fputc ('\n', stdout);
	if (buffer[0] == '\0')
	{
	    header = FALSE;
	    continue;
	}
	if (continuation_line)
	{
	    if ( !process_addresses (buffer, pgppath, &options, debug_fp,
				     &continuation_line) )
	    {
		return_badnews ("cannot process continuation header line");
		kill (sendmail_pid, SIGKILL);
		exit (1);
	    }
	    continue;
	}
	if (st_nicmp (buffer, "to:", 3) == 0)
	{
	    if ( !process_addresses (buffer + 3, pgppath, &options, debug_fp,
				     &continuation_line) )
	    {
		return_badnews ("cannot process  to:  header line");
		kill (sendmail_pid, SIGKILL);
		exit (1);
	    }
	    continue;
	}
	if (st_nicmp (buffer, "cc:", 3) == 0)
	{
	    if ( !process_addresses (buffer + 3, pgppath, &options, debug_fp,
				     &continuation_line) )
	    {
		return_badnews ("cannot process  cc:  header line");
		kill (sendmail_pid, SIGKILL);
		exit (1);
	    }
	    continue;
	}
	if (st_nicmp (buffer, "bcc:", 4) == 0)
	{
	    if ( !process_addresses (buffer + 4, pgppath, &options, debug_fp,
				     &continuation_line) )
	    {
		return_badnews ("cannot process  bcc:  header line");
		kill (sendmail_pid, SIGKILL);
		exit (1);
	    }
	    continue;
	}
	if (st_icmp (buffer, "Content-Transfer-Encoding: quoted-printable")
	    == 0)
	{
	    quoted_printables = TRUE;
	}
	if (st_nicmp (buffer, "subject:", 8) == 0)
	{
	    (void) fprintf (stderr, "> %s\n\n", buffer);
	}
    }
    (void) fflush (stdout);
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Header processed\n");
	(void) fflush (debug_fp);
    }
    if (options.insecure)
    {
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp,
			    "Insecure flag set: message will be sent as plaintext\n");
	    (void) fflush (debug_fp);
	}
	transmit_plaintext (options.sign, debug_fp, sendmail_pid,
			    options.no_advertising, options.add_key);
    }
    if (quoted_printables)
    {
	/*  Cannot encrypt this message  */
	if (options.secure)
	{
	    return_badnews ("cannot encrypt messages with quoted printables");
	    kill (sendmail_pid, SIGKILL);
	    exit (1);
	}
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp,
			    "Quoted printables flag set: message will be sent as plaintext\n");
	    (void) fflush (debug_fp);
	}
	transmit_plaintext (options.sign, debug_fp, sendmail_pid,
			    options.no_advertising, options.add_key);
    }
    /*  Determine if all recipients have PGP public keys  */
    switch ( check_recipients (recipients + 3, pgppath, &bad_recipient) )
    {
      case CHECK_CAN_ENCRYPT:
	break;
      case CHECK_NO_KEY:
	if (options.secure)
	{
	    (void) sprintf (txt,
			    "recipient: \"%s\" does not have a PGP public key",
			    bad_recipient);
	    return_badnews (txt);
	    kill (sendmail_pid, SIGKILL);
	    exit (1);
	}
	/*  One of the recipients does not have a PGP key  */
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp,
			    "Not all recipients have a public key: message will be sent as plaintext\n");
	    (void) fflush (debug_fp);
	}
	transmit_plaintext (options.sign, debug_fp, sendmail_pid,
			    options.no_advertising, options.add_key);
	break;
      case CHECK_AMBIGUOUS_KEY:
	if (options.secure)
	{
	    (void) sprintf (txt,
			    "ambiguous recipient: \"%s\" matches multiple PGP public keys",
			    bad_recipient);
	    return_badnews (txt);
	    kill (sendmail_pid, SIGKILL);
	    exit (1);
	}
	/*  One of the recipients does not have a PGP key  */
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp,
			    "A recipient has an ambiguous public key: message will be sent as plaintext\n");
	    (void) fflush (debug_fp);
	}
	transmit_plaintext (options.sign, debug_fp, sendmail_pid,
			    options.no_advertising, options.add_key);
	break;
      case CHECK_NO_KEYLIST:
	return_badnews ("could not read keylist");
	exit (RV_CANNOT_OPEN);
/*
	break;
*/
      default:
	break;
    }
    /*  Every recipient has a PGP public key: encrypt!  */
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Message will be encrypted\n");
	(void) fflush (debug_fp);
    }
    if (!options.no_advertising)
    {
	(void) printf ("\nMessage has been automatically %sencrypted by PGPsendmail %s,\n",
		       options.sign ? "signed and " : "", VERSION);
	(void) printf ("available from ftp.atnf.csiro.au:pub/people/rgooch");
	(void) printf ("\n\n\n");
	(void) fflush (stdout);
    }
    if (options.sign)
    {
	/*  Sign message  */
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Will attempt to sign message\n");
	    (void) fflush (debug_fp);
	}
	if ( !connect_to_pgpdaemon (pgppath, &to_fd, &from_fd) )
	{
	    return_badnews ("could not connect to PGPdaemon");
	    kill (sendmail_pid, SIGKILL);
	    exit (RV_UNDEF_ERROR);
	}
	/*  Set close-on-exec flag for pipe to PGPdaemon  */
	if (fcntl (to_fd, F_SETFD, 1) == -1)
	{
	    return_badnews ("could not set close-on-exec flag");
	    kill (sendmail_pid, SIGKILL);
	    exit (RV_UNDEF_ERROR);
	}
	if (write (to_fd, "SIGN\n", 5) < 5)
	{
	    return_badnews ("could not write to PGPdaemon");
	    kill (sendmail_pid, SIGKILL);
	    exit (RV_UNDEF_ERROR);
	}
	/*  Check if OK so far  */
	if (read (from_fd, (char *) &bool_val, sizeof bool_val) <
	    sizeof bool_val)
	{
	    (void) fprintf (stderr,
			    "Error reading response flag from PGPdaemon\t%s\n",
			    sys_errlist[errno]);
	    return_badnews ("problem communicating with PGPdaemon");
	    kill (sendmail_pid, SIGKILL);
	    exit (RV_UNDEF_ERROR);
	}
	if (!bool_val)
	{
	    return_badnews ("PGPdaemon refused to sign: probably has no passphrase");
	    if (debug_fp != NULL)
	    {
		(void) fprintf (debug_fp,
				"PGPdaemon refused to sign: probably has no passphrase\n");
		(void) fflush (debug_fp);
	    }
	    kill (sendmail_pid, SIGKILL);
	    exit (RV_UNDEF_ERROR);
	}
	pgp_in_fd = from_fd;
	out_fd = OUTPUT_FD;
	err_fd = ERROR_FD;
	if ( ( pgp_pid = spawn_job ("pgp", recipients,
				    &pgp_in_fd, &out_fd, &err_fd) ) < 0 )
	{
	    return_badnews ("could not spawn PGP");
	    kill (sendmail_pid, SIGKILL);
	    exit (RV_UNDEF_ERROR);
	}
	(void) close (from_fd);
	/*  Copy data  */
	if ( !copy_data (to_fd, INPUT_FD, FALSE) )
	{
	    return_badnews ("could not copy data to be signed");
	    kill (sendmail_pid, SIGKILL);
	    exit (RV_UNDEF_ERROR);
	}
	/*  Tell PGPdaemon the end of the message has arrived  */
	(void) close (to_fd);
	(void) fprintf (stderr, "Signed message\n");
    }
    else
    {
	pgp_in_fd = INPUT_FD;
	out_fd = OUTPUT_FD;
	err_fd = ERROR_FD;
	if ( ( pgp_pid = spawn_job ("pgp", recipients,
				    &pgp_in_fd, &out_fd, &err_fd) ) < 0 )
	{
	    return_badnews ("could not spawn PGP");
	    kill (sendmail_pid, SIGKILL);
	    exit (RV_UNDEF_ERROR);
	}
    }
    if (waitpid (pgp_pid, &status, 0) < 0)
    {
	(void) fprintf (stderr, "Error reaping child\t%s\n",
			sys_errlist[errno]);
	return_badnews ("could not reap PGP process");
	kill (sendmail_pid, SIGKILL);
	exit (RV_UNDEF_ERROR);
    }
    if (status != 0)
    {
	(void) fprintf (stderr, "Bad child status: %d\n", status);
	return_badnews ("PGP failure");
	kill (sendmail_pid, SIGKILL);
	exit (1);
    }
    if (options.add_key)
    {
	if ( !include_mykey (pgppath, stdout) )
	{
	    return_badnews ("could not include public key");
	    kill (sendmail_pid, SIGKILL);
	    exit (1);
	}
    }
    (void) fprintf (stderr, "\n\n\n");
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp,
			"Message should have been sent to real  sendmail\n");
	(void) fflush (debug_fp);
    }
    if (options.receipt)
    {
	return_badnews (NULL);
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Receipt returned\n");
	    (void) fflush (debug_fp);
	}
    }
    else
    {
	(void) sprintf ( txt, "/tmp/PGPsendmail.err.%d", getpid () );
	(void) unlink (txt);
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Receipt discarded\n");
	    (void) fflush (debug_fp);
	}
    }
    exit (RV_OK);
}   /*  End Function main  */

static flag read_line (debug_fp)
/*  This routine will read a line from the standard input and will write it to
    the global buffer  buffer  .If the buffer is too small: it is expanded.
    Debugging information will be written to the file pointed to by  debug_fp
    .If this is NULL, no debugging information is written.
    The routine returns TRUE on success, else it returns FALSE.
*/
FILE *debug_fp;
{
    int buf_pos = 0;
    char ch;
    int new_len;
    char *tmp;
    extern int buf_len;
    extern char *buffer;

    while (read (0, &ch, 1) == 1)
    {
	if (buf_pos >= buf_len)
	{
	    /*  Need a bigger buffer  */
	    new_len = (buf_len < BUF_LEN) ? BUF_LEN : buf_len * 2;
	    if ( ( tmp = m_alloc (new_len) ) == NULL )
	    {
		if (debug_fp != NULL)
		{
		    (void) fprintf (debug_fp,
				    "read_line: error allocating: %d bytes\n",
				    new_len);
		    (void) fflush (debug_fp);
		}
		(void) fprintf (stderr, "Error allocating: %d bytes\n",
				new_len);
		return (FALSE);
	    }
	    if (buf_len > 0)
	    {
		m_copy (tmp, buffer, buf_len);
		m_free (buffer);
	    }
	    buffer = tmp;
	    buf_len = new_len;
	}
	/*  Have space in buffer  */
	if (ch == '\n') ch = '\0';
	buffer[buf_pos] = ch;
	if (ch == '\0') return (TRUE);
	++buf_pos;
    }
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "read_line: EndOfFile\n");
	(void) fflush (debug_fp);
    }
    (void) fprintf (stderr, "EndOfFile\n");
    return (FALSE);
}   /*  End Function read_line  */

static flag process_addresses (list, pgppath, options, debug_fp,
			       trailing_comma)
/*  This routine will process a line of addresses, adding each to the list of
    recipients.
    The line of addresses must be pointed to by  list  .
    NOTE: the line of addresses is modified!
    The value of the PGPPATH environment variable must be pointed to by
    pgppath  .If this is NULL, the recipient name will not be translated.
    The storage pointed to by  options  may be modified.
    Debugging information will be written to the file pointed to by  debug_fp
    .If this is NULL, no debugging information is written.
    If the line had a trailing comma, the value TRUE is written to the storage
    pointed to by  trailing_comma  else the value FALSE is written here.
    The routine returns TRUE on succes, else it returns FALSE.
*/
char *list;
char *pgppath;
option_type *options;
FILE *debug_fp;
flag *trailing_comma;
{
    int new_size;
    char *ptr;
    char *recipient;
    char **rec;
    extern char **recipients;
    static int num_recipients = 0;
    static int recipient_buf_size = 0;

    *trailing_comma = FALSE;
    /*  Remove trailing whitespace and check for trailing ','  */
    for (ptr = list + strlen (list) - 1; isspace (*ptr) && (ptr > list);
	 --ptr) *ptr = '\0';
    if (*ptr == ',') *trailing_comma = TRUE;
    while ( ( ptr = extract_address (list, &list) ) != NULL )
    {
	if (pgppath == NULL)
	{
	    if ( ( recipient = m_alloc (strlen (ptr) + 1) ) == NULL )
	    {
		(void) fprintf (stderr,
				"Error copying recipient: \"%s\"\n",
				ptr);
		return (FALSE);
	    }
	    (void) strcpy (recipient, ptr);
	}
	else
	{
	    if ( ( recipient = translate_recipient (ptr, pgppath, options) )
		== NULL )
	    {
		(void) fprintf (stderr,
				"Error translating recipient: \"%s\"\n",
				ptr);
		return (FALSE);
	    }
	}
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp,
			    "    Recipient: \"%s\"\tsynonym: \"%s\"\n",
			    ptr, recipient);
	    (void) fflush (debug_fp);
	}
	/*  Have a new recipient  */
	if (num_recipients + 1 >= recipient_buf_size)
	{
	    /*  Reallocate array of string pointers  */
	    new_size = (recipient_buf_size * 2);
	    if (new_size < 1) new_size = 256;
	    if ( ( rec = (char **) m_alloc (sizeof *rec * new_size) ) == NULL )
	    {
		(void) fprintf (stderr,
				"Error allocatingarray of string pointers\n");
		m_free (recipient);
		return (FALSE);
	    }
	    m_copy ( (char *) rec, (char *) recipients,
		   sizeof *rec * recipient_buf_size );
	    if (recipients != NULL) m_free ( (char *) recipients );
	    recipients = rec;
	    recipient_buf_size = new_size;
	}
	recipients[num_recipients++] = recipient;
	recipients[num_recipients] = NULL;
    }
    return (TRUE);
}   /*  End Function process_addresses  */

static void return_badnews (message)
/*  This routine will mail off a message to the user, including the original
    input message.
    The message must be pointed to by  message  .If this is NULL, a "PGP OK"
    message is mailed and the original message is not included.
    The routine returns nothing.
*/
char *message;
{
    int count;
    int in_fd, out_fd, err_fd;
    int child;
    char txt[LINE_LENGTH];
    char separator[SEPARATOR_LENGTH + 1];
    FILE *err_fp;
    char *argv[6];
    extern char *username;
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    /*  Make stdout go to  stderr  */
    (void) fflush (stdout);
    (void) dup2 (ERROR_FD, OUTPUT_FD);
    if (message == NULL)
    {
	(void) fprintf (stderr, "PGPsendmail: PGP OK\n");
	(void) sprintf ( txt,
			"%s -t -f %s -oi < /tmp/PGPsendmail.err.%d",
			SENDMAIL, username, getpid () );
	(void) system (txt);
	(void) sprintf ( txt, "/tmp/PGPsendmail.err.%d", getpid () );
	(void) unlink (txt);
	return;
    }
    (void) fprintf (stderr, "PGPsendmail: %s\n", message);
    (void) sprintf ( txt, "/tmp/PGPsendmail.err.%d", getpid () );
    if ( ( err_fp = fopen (txt, "r") ) == NULL )
    {
	(void) fprintf (stderr, "Could not open error file: \"%s\"\t%s\n",
			txt, sys_errlist[errno]);
	exit (RV_CANNOT_OPEN);
    }
    /*  Bounce message  */
    argv[0] = SENDMAIL;
    argv[1] = "-t";
    argv[2] = "-f";
    argv[3] = username;
    argv[4] = "-oi";
    argv[5] = NULL;
    in_fd = -1;
    out_fd = OUTPUT_FD;
    err_fd = ERROR_FD;
    if ( ( child = spawn_job (SENDMAIL, argv, &in_fd, &out_fd, &err_fd) ) < 0 )
    {
	(void) fprintf (stderr,
			"Could not spawn  sendmail  to bounce message\n");
	exit (RV_SYS_ERROR);
    }
    /*  Read error messages and pass to  sendmail  */
    while (fgets (txt, STRING_LENGTH - 1, err_fp) != NULL)
    {
	(void) write ( in_fd, txt, strlen (txt) );
    }
    (void) fclose (err_fp);
    (void) strcpy (txt, "\n\nBounced message follows:\n\n");
    (void) write ( in_fd, txt, strlen (txt) );
    for (count = 0; count < SEPARATOR_LENGTH; ++count) separator[count] = '-';
    separator[SEPARATOR_LENGTH] = '\n';
    (void) write (in_fd, separator, SEPARATOR_LENGTH + 1);
    /*  Read message and pass to  sendmail  */
    while (fgets (txt, STRING_LENGTH - 1, stdin) != NULL)
    {
	(void) write ( in_fd, txt, strlen (txt) );
    }
    (void) write (in_fd, separator, SEPARATOR_LENGTH + 1);
    (void) close (in_fd);
    (void) sprintf ( txt, "/tmp/PGPsendmail.err.%d", getpid () );
    (void) unlink (txt);
}   /*  End Function return_badnews  */

static char *extract_address (str, rest)
/*  This routine will extract a sub-string from a string. The sub-string may be
    delimited by any number of ',' characters.
    The input string must be pointed to by  str. The routine will write a
    pointer to the next field in the input string to  rest  .
    NOTE: the input string is modified!
    The double quote character may appear anywhere in the sub-string, and will
    force all ',' characters into the output string. A second double quote
    character unquotes the previous quote. These double quote characters are
    not copied, unless they are consecutive.
    The routine returns a pointer to a copy of the sub-string.
*/
char *str;
char **rest;
{
    flag comma = FALSE;
    char *ptr1, *ptr2;

    if ( (str == NULL) || (*str == '\0') )
    {
	*rest = NULL;
	return (NULL);
    }
    *rest = NULL;
    /*  Skip leading whitespace  */
    while ( isspace (*str) && (*str != '\0' ) ) ++str;
    if (*str == ',') ++str;
    /*  Skip more whitespace  */
    while ( isspace (*str) && (*str != '\0' ) ) ++str;
    if (*str == '\"')
    {
	/*  Address formats:
	        "Real Name" <username@email.address>
		"Real Name"@email.address
	*/
	if ( ( ptr1 = strchr (str + 1, '\"') ) == NULL )
	{
	    /*  Stuffed address  */
	    (void) fprintf (stderr,
			    "Bad address (missing close quote): \"%s\"\n",
			    str);
	    return_badnews ("bad address");
	    exit (1);
	}
	if (ptr1[1] == '@')
	{
	    /*  Address format:
		"Real Name"@email.address
	    */
	    if ( ( *rest = strchr (ptr1, ',') ) != NULL ) ++*rest;
	    return (str);
	}
	/*  Address format:
	        "Real Name" <username@email.address>
	*/
	/*  Discard bit in quotes  */
	str = ptr1 + 1;
	if ( ( ptr1 = strchr (str, '<') ) == NULL )
	{
	    /*  Stuffed address  */
	    (void) fprintf (stderr, "Bad address (missing '<'): \"%s\"\n",
			    str);
	    return_badnews ("bad address");
	    exit (1);
	}
	if ( ( ptr2 = strchr (str, '>') ) == NULL )
	{
	    /*  Stuffed address  */
	    (void) fprintf (stderr, "Bad address: \"%s\"\n", str);
	    return_badnews ("bad address");
	    exit (1);
	}
	*ptr2 = '\0';
	*rest = ptr2 + 1;
	return (ptr1 + 1);
    }
    if ( ( ptr1 = strchr (str, ',') ) != NULL )
    {
	/*  Have a ',' character. Is is between '(' and ')' though?  */
	if ( ( ( ptr2 = strchr (str, '(') ) != NULL ) && (ptr2 < ptr1) )
	{
	    if ( ( ptr1 = strchr (ptr2, ')') ) == NULL )
	    {
		/*  Missing ')'  */
		(void) fprintf (stderr, "Bad address: \"%s\"\n", str);
		return_badnews ("bad address");
		exit (1);
	    }
	    /*  ',' is between '(' and ')'  */
	    if ( ( ptr2 = strchr (ptr1, ',') ) != NULL )
	    {
		*ptr2 = '\0';
		*rest = ptr2 + 1;
		comma = TRUE;
	    }
	}
	else
	{
	    /*  ',' is NOT between '(' and ')'  */
	    *ptr1 = '\0';
	    *rest = ptr1 + 1;
	    comma = TRUE;
	}
    }
    /*  No more ',' characters  */
    if ( ( ptr1 = strchr (str, '<') ) != NULL )
    {
	/*  Address format:
	        Real Name <username@email.address>
	*/
	if ( ( ptr2 = strchr (str, '>') ) == NULL )
	{
	    /*  Stuffed address  */
	    (void) fprintf (stderr, "Bad address: \"%s\"\n", str);
	    return_badnews ("bad address");
	    exit (1);
	}
	*ptr2 = '\0';
	if (comma) return (ptr1 + 1);
	*rest = ptr2 + 1;
	return (ptr1 + 1);
    }
    if ( ( ptr1 = strchr (str, '(') ) != NULL )
    {
	/*  Address format:
	        username (Real Name [,Organisation])
	    OR:
	        username@email.address (Real Name [,Organisation])
	*/
	if ( ( ptr2 = strchr (str, ')') ) == NULL )
	{
	    /*  Stuffed address  */
	    (void) fprintf (stderr, "Bad address: \"%s\"\n", str);
	    return_badnews ("bad address");
	    exit (1);
	}
	/*  Go backwards and skip trailing spaces  */
	*ptr1-- = '\0';
	while ( isspace (*ptr1) && (ptr1 > str) ) *ptr1-- = '\0';
	if (comma) return (str);
	*rest = ptr2 + 1;
	return (str);
    }
    /*  Address format:
	    username
	OR:
	    username@email.address
    */
    /*  Go to next whitespace  */
    ptr1 = str;
    while ( !isspace (*ptr1) && (*ptr1 != '\0' ) ) ++ptr1;
    if (comma) return (str);
    if (*ptr1 == '\0')
    {
	*rest = NULL;
	return (str);
    }
    *ptr1 = '\0';
    *rest = ptr1 + 1;
    return (str);
}   /*  End Function extract_address  */

static flag check_for_header_flags (line, options, sendmail_pid, debug_fp)
/*  This routine will check if the header line contains the "X-Secure:"
    keyword and will process it.
    The header line must be pointed to by  line  .
    The flags will modify the storage pointed to by  options  .See the
    documentation on the  process_flags  routine for details.
    The process ID of the real sendmail must be given by  sendmail_pid  .
    Debugging information will be written to the file pointed to by  debug_fp
    .If this is NULL, no debugging information is written.
    The routine returns TRUE if the header line contains the "X-Secure:"
    keyword, else it returns FALSE.
*/
char *line;
option_type *options;
int sendmail_pid;
FILE *debug_fp;
{
    char *ptr, *rest;

    if (st_nicmp (line, "X-Secure:", 9) != 0) return (FALSE);
    line += 9;
    if ( !process_flags (line, options) )
    {
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "bad X-Secure: line: \"%s\"\n", line);
	    (void) fflush (debug_fp);
	}
	return_badnews ("bad X-Secure: line");
	kill (sendmail_pid, SIGKILL);
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Real  sendmail  killed: exiting\n");
	    (void) fflush (debug_fp);
	}
	exit (1);
    }
    return (TRUE);
}   /*  End Function check_for_header_flags  */

static void sig_handler (sig, code, scp, addr)
/*  This routine will handle signals.
    The signal number will be given by  sig  .
    The routine returns nothing.
*/
int sig;
int code;
struct sigcontext *scp;
char *addr;
{
    extern FILE *debug_fp;

    if (sig == SIGPIPE)
    {
	(void) fprintf (stderr, "Caught SIGPIPE\n");
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Caught SIGPIPE\n");
	    (void) fflush (debug_fp);
	}
    }
}   /*  End Function sig_handler  */

static void transmit_plaintext (sign, debug_fp, sendmail_pid,
				no_advertising, add_key)
/*  This routine will transmit the Email message the message in plaintext form.
    If the value of  sign  is TRUE, the message will be.
    Debugging information will be written to the file pointed to by  debug_fp
    .If this is NULL, no debugging information is written.
    The process ID of the real sendmail must be given by  sendmail_pid  .
    If advertising is not desired the value of  no_advertising  must be TRUE.
    The routine does not return: upon completion the process exits.
*/
flag sign;
FILE *debug_fp;
int sendmail_pid;
flag no_advertising;
flag add_key;
{
    flag bool_val;
    int ch;
    int child_pid, status;
    int to_fd, from_fd;
    extern char *buffer;
    extern char *pgppath;
    ERRNO_TYPE errno;
    extern char *sys_errlist[];

    if (!sign)
    {
	if (debug_fp != NULL)
	{
	    (void) fputs ("Copying unsigned, plaintext message to  sendmail\n",
			  debug_fp);
	    (void) fflush (debug_fp);
	}
	/*  Copy message to  sendmail  */
	while ( ( ch = fgetc (stdin) ) != EOF ) fputc (ch, stdout);
	fflush (stdout);
	if (add_key)
	{
	    if ( !include_mykey (pgppath, stdout) )
	    {
		return_badnews ("could not include public key");
		kill (sendmail_pid, SIGKILL);
		exit (1);
	    }
	}
	/*  Clean up and exit  */
	(void) sprintf ( buffer, "/tmp/PGPsendmail.err.%d", getpid () );
	(void) unlink (buffer);
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Exiting with status 0\n");
	    (void) fflush (debug_fp);
	}
	exit (0);
    }
    /*  Message should be signed  */
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Will attempt to sign message\n");
	(void) fflush (debug_fp);
    }
    if ( !connect_to_pgpdaemon (pgppath, &to_fd, &from_fd) )
    {
	return_badnews ("could not connect to PGPdaemon");
	kill (sendmail_pid, SIGKILL);
	exit (RV_UNDEF_ERROR);
    }
    if (write (to_fd, "SIGN\n", 5) < 5)
    {
	return_badnews ("could not write to PGPdaemon");
	kill (sendmail_pid, SIGKILL);
	exit (RV_UNDEF_ERROR);
    }
    /*  Check if OK so far  */
    if (read (from_fd, (char *) &bool_val, sizeof bool_val) < sizeof bool_val)
    {
	return_badnews ("problem communicating with PGPdaemon");
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp,
			    "Error reading response flag from PGPdaemon\t%s\n",
			    sys_errlist[errno]);
	    (void) fflush (debug_fp);
	}
	kill (sendmail_pid, SIGKILL);
	exit (RV_UNDEF_ERROR);
    }
    if (!bool_val)
    {
	return_badnews ("PGPdaemon refused to sign: probably has no passphrase");
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp,
			    "PGPdaemon refused to sign: probably has no passphrase\n");
	    (void) fflush (debug_fp);
	}
	kill (sendmail_pid, SIGKILL);
	exit (RV_UNDEF_ERROR);
    }
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp,
			"Copying signed, plaintext message to  sendmail\n");
	(void) fflush (debug_fp);
    }
    if (!no_advertising)
    {
	(void) printf ("\nMessage has been automatically signed by PGPsendmail %s,\n",
		       VERSION);
	(void) printf ("available from ftp.atnf.csiro.au:pub/people/rgooch");
	(void) printf ("\n\n");
	(void) fflush (stdout);
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
	    if (debug_fp != NULL)
	    {
		(void) fprintf (debug_fp,
				"Error copying data from PGPdaemon\n");
		(void) fflush (debug_fp);
	    }
	    exit (RV_UNDEF_ERROR);
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
	(void) close (from_fd);
	break;
    }
    /*  Copy  stdin  to PGPdaemon  */
    if ( !copy_data (to_fd, INPUT_FD, FALSE) )
    {
	return_badnews ("Error copying data to PGPdaemon");
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Error copying data to PGPdaemon\n");
	    (void) fflush (debug_fp);
	}
	kill (sendmail_pid, SIGKILL);
	exit (RV_UNDEF_ERROR);
    }
    /*  Tell PGPdaemon the end of message has arrived  */
    (void) close (to_fd);
    /*  Reap child  */
    (void) waitpid (child_pid, &status, 0);
    if (status != 0)
    {
	return_badnews ("Child returned bad status");
	if (debug_fp != NULL)
	{
	    (void) fprintf (debug_fp, "Bad child status: %d\n", status);
	    (void) fflush (debug_fp);
	}
	(void) kill (sendmail_pid, SIGKILL);
	exit (RV_OK);
    }
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Signed message\n");
	(void) fflush (debug_fp);
    }
    if (add_key)
    {
	if ( !include_mykey (pgppath, stdout) )
	{
	    return_badnews ("could not include public key");
	    kill (sendmail_pid, SIGKILL);
	    exit (1);
	}
    }
    /*  Clean up and exit  */
    (void) sprintf ( buffer, "/tmp/PGPsendmail.err.%d", getpid () );
    (void) unlink (buffer);
    if (debug_fp != NULL)
    {
	(void) fprintf (debug_fp, "Exiting with status 0\n");
	(void) fflush (debug_fp);
    }
    exit (0);
}   /*  End Function transmit_plaintext  */


/*  Valid address formats:

    Real Name <username@email.address>
    "Real Name" <username@email.address>
    "Real Name"@email.address
    username@email.address
    username
    username (Real Name [,Organisation])
    username@email.address (Real Name [,Organisation])
*/
