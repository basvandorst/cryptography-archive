/*
 * tftpd.c  --  Implement TFTP server.
 *
 * Copyright 1995 by Bao Phan, Randall Atkinson, and Dan McDonald
 *      All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 */

/*----------------------------------------------------------------------
#	@(#)COPYRIGHT	1.1 (NRL) 17 January 1995

COPYRIGHT NOTICE

All of the documentation and software included in this software
distribution from the US Naval Research Laboratory (NRL) are
copyrighted by their respective developers.

Portions of the software are derived from the Net/2 and 4.4 Berkeley
Software Distributions (BSD) of the University of California at
Berkeley and those portions are copyright by The Regents of the
University of California. All Rights Reserved.  The UC Berkeley
Copyright and License agreement is binding on those portions of the
software.  In all cases, the NRL developers have retained the original
UC Berkeley copyright and license notices in the respective files in
accordance with the UC Berkeley copyrights and license.

Portions of this software and documentation were developed at NRL by
various people.  Those developers have each copyrighted the portions
that they developed at NRL and have assigned All Rights for those
portions to NRL.  Outside the USA, NRL has copyright on some of the
software developed at NRL. The affected files all contain specific
copyright notices and those notices must be retained in any derived
work.

NRL LICENSE

NRL grants permission for redistribution and use in source and binary
forms, with or without modification, of the software and documentation
created at NRL provided that the following conditions are met:

1. All terms of the UC Berkeley copyright and license must be followed.
2. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
3. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
4. All advertising materials mentioning features or use of this software
   must display the following acknowledgements:

	This product includes software developed by the University of
	California, Berkeley and its contributors.

	This product includes software developed at the Information
	Technology Division, US Naval Research Laboratory.

5. Neither the name of the NRL nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THE SOFTWARE PROVIDED BY NRL IS PROVIDED BY NRL AND CONTRIBUTORS ``AS
IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL NRL OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation
are those of the authors and should not be interpreted as representing
official policies, either expressed or implied, of the US Naval
Research Laboratory (NRL).

----------------------------------------------------------------------*/
/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)tftpd.c	8.1 (Berkeley) 6/4/93";
#endif /* not lint */

/*
 * Trivial file transfer protocol server.
 *
 * This version includes many modifications by Jim Guyton
 * <guyton@rand-unix>.
 */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/tftp.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "tftpsubs.h"

#ifdef DEBUG
#include <sys/types.h>
#include <sys/time.h>

int     debug = 0;
int     port;
#endif /* DEBUG */

#ifdef INET6
#include <netinet6/in6.h>
#endif /* INET6 */
int     debugmsg = 1;

#define	TIMEOUT		5

int	peer;
#ifdef INET6
int     peeraf;
#endif /* INET6 */
int	rexmtval = TIMEOUT;
int	maxtimeout = 5*TIMEOUT;

#define	PKTSIZE	SEGSIZE+4
char	buf[PKTSIZE];
char	ackbuf[PKTSIZE];
#ifdef INET6
struct sockaddr_in6 from;
#else
struct	sockaddr_in from;
#endif
int	fromlen;

void	tftp __P((struct tftphdr *, int));

/*
 * Null-terminated directory prefix list for absolute pathname requests and
 * search list for relative pathname requests.
 *
 * MAXDIRS should be at least as large as the number of arguments that
 * inetd allows (currently 20).
 */
#define MAXDIRS	20
static struct dirlist {
	char	*name;
	int	len;
} dirs[MAXDIRS+1];
static int	suppress_naks;
static int	logging;

static char *errtomsg __P((int));
static void  nak __P((int));
static char *verifyhost __P((struct sockaddr_in *));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register struct tftphdr *tp;
	register int n;
	int ch, on;
#ifdef INET6
	struct sockaddr_in6 sin;
#else
	struct sockaddr_in sin;
#endif /* INET6 */

	openlog("tftpd", LOG_PID, LOG_FTP);
#ifdef DEBUG
	while ((ch = getopt(argc, argv, "lnd:")) != EOF) {	
#else
	while ((ch = getopt(argc, argv, "ln")) != EOF) {
#endif /* DEBUG */
		switch (ch) {
		case 'l':
			logging = 1;
			break;
		case 'n':
			suppress_naks = 1;
			break;
#ifdef DEBUG
	        case 'd':
			debug = 1;
			if ((port = atoi(optarg)) < 0) {
			    fprintf(stderr, "tftp: invalid port number\n");
			    exit(1);
			}
			break;
#endif /* DEBUG */
		default:
			syslog(LOG_WARNING, "ignoring unknown option -%c", ch);
		}
	}
	if (optind < argc) {
		struct dirlist *dirp;

		/* Get list of directory prefixes. Skip relative pathnames. */
		for (dirp = dirs; optind < argc && dirp < &dirs[MAXDIRS];
		     optind++) {
			if (argv[optind][0] == '/') {
				dirp->name = argv[optind];
				dirp->len  = strlen(dirp->name);
				dirp++;
			}
		}
	}

	on = 1;

#ifdef DEBUG
	if (debug) {	
	        struct sockaddr_in6 sin6;
		int s;

		bzero((char *)&sin6, sizeof(sin6));
		sin6.sin6_family = AF_INET6;
		sin6.sin6_len = sizeof(struct sockaddr_in6);
		sin6.sin6_port = htons(port);

		s = socket(PF_INET6, SOCK_DGRAM, 0);
		if (s < 0) {
		    perror("tftpd: socket");
		    exit(1);
		}

		if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, 
		  (char *)&on, sizeof(on)) < 0) {
		    perror("tftpd: setsockopt");
		    exit(1);
		}

		if (bind(s, (struct sockaddr *)&sin6, sizeof(sin6)) < 0) {
		    perror("tftpd (debug): bind");
		    exit(1);
		}

		(void) dup2(s, 0);
		(void) close(s);
	} else
#endif /* DEBUG */
 	if (ioctl(0, FIONBIO, &on) < 0) {
		syslog(LOG_ERR, "ioctl(FIONBIO): %m\n");
		exit(1);
	} 
	fromlen = sizeof (from);
if (debugmsg)
fprintf(stderr, "Waiting for initial datagram...");
	n = recvfrom(0, buf, sizeof (buf), 0,
	    (struct sockaddr *)&from, &fromlen);

	if (n < 0) {
	        perror("tftpd: recvfrom");
		syslog(LOG_ERR, "recvfrom: %m\n");
		exit(1);
	}
if (debugmsg)
fprintf(stderr, "got it!\n");

	/*
	 * Now that we have read the message out of the UDP
	 * socket, we fork and exit.  Thus, inetd will go back
	 * to listening to the tftp port, and the next request
	 * to come in will start up a new instance of tftpd.
	 *
	 * We do this so that inetd can run tftpd in "wait" mode.
	 * The problem with tftpd running in "nowait" mode is that
	 * inetd may get one or more successful "selects" on the
	 * tftp port before we do our receive, so more than one
	 * instance of tftpd may be started up.  Worse, if tftpd
	 * break before doing the above "recvfrom", inetd would
	 * spawn endless instances, clogging the system.
	 */
	{
		int pid;
		int i, j;

		for (i = 1; i < 20; i++) {
		    pid = fork();
		    if (pid < 0) {
				sleep(i);
				/*
				 * flush out to most recently sent request.
				 *
				 * This may drop some request, but those
				 * will be resent by the clients when
				 * they timeout.  The positive effect of
				 * this flush is to (try to) prevent more
				 * than one tftpd being started up to service
				 * a single request from a single client.
				 */
				j = sizeof from;
				i = recvfrom(0, buf, sizeof (buf), 0,
				    (struct sockaddr *)&from, &j);
				if (i > 0) {
					n = i;
					fromlen = j;
				}
		    } else {
				break;
		    }
		}
		if (pid < 0) {
			syslog(LOG_ERR, "fork: %m\n");
			exit(1);
		} else if (pid != 0) {
			exit(0);
		}
	}
/*	(struct sockaddr_in)from.sin_family = AF_INET; */      /* XXX */
	alarm(0);
	close(0);
	close(1);

#ifdef INET6
if (debugmsg)
fprintf(stderr, "Creating socket to dest...");

	peeraf = ((struct sockaddr *)&from)->sa_family;
	peer = socket(PF_INET6, SOCK_DGRAM, 0);
#else
	peer = socket(AF_INET, SOCK_DGRAM, 0);
#endif /* INET6 */
	if (peer < 0) {
		syslog(LOG_ERR, "socket: %m\n");
		exit(1);
	}
if (debugmsg)
fprintf(stderr, "done!\n");

	memset(&sin, 0, sizeof(sin));
#ifdef INET6
	((struct sockaddr *)&sin)->sa_family = peeraf;
	((struct sockaddr *)&sin)->sa_len = (peeraf == AF_INET6) ?
	     sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
#else
	sin.sin_family = AF_INET;
#endif /* INET6 */

	if (bind(peer, (struct sockaddr *)&sin, 
            ((struct sockaddr *)&sin)->sa_len) < 0) {
                perror("tftpd: bind");
		syslog(LOG_ERR, "bind: %m\n");
		exit(1);
	}
	if (connect(peer, (struct sockaddr *)&from, 
	    ((struct sockaddr *)&from)->sa_len) < 0) {
                perror("tftpd (debug): connect");
		syslog(LOG_ERR, "connect: %m\n");
		exit(1);
	}
	tp = (struct tftphdr *)buf;
	tp->th_opcode = ntohs(tp->th_opcode);
if (debugmsg)
fprintf(stderr, "Got opcode=%d\n", tp->th_opcode);
	if (tp->th_opcode == RRQ || tp->th_opcode == WRQ)
		tftp(tp, n);
	exit(1);
}

struct formats;
int	validate_access __P((char **, int));
void	sendfile __P((struct formats *));
void	recvfile __P((struct formats *));

struct formats {
	char	*f_mode;
	int	(*f_validate) __P((char **, int));
	void	(*f_send) __P((struct formats *));
	void	(*f_recv) __P((struct formats *));
	int	f_convert;
} formats[] = {
	{ "netascii",	validate_access,	sendfile,	recvfile, 1 },
	{ "octet",	validate_access,	sendfile,	recvfile, 0 },
#ifdef notdef
	{ "mail",	validate_user,		sendmail,	recvmail, 1 },
#endif
	{ 0 }
};

/*
 * Handle initial connection protocol.
 */
void
tftp(tp, size)
	struct tftphdr *tp;
	int size;
{
	register char *cp;
	int first = 1, ecode;
	register struct formats *pf;
	char *filename, *mode;
if (debugmsg)
fprintf(stderr, "Entering tftp...\n");

	filename = cp = tp->th_stuff;
again:
	while (cp < buf + size) {
		if (*cp == '\0')
			break;
		cp++;
	}
	if (*cp != '\0') {
		nak(EBADOP);
		exit(1);
	}
	if (first) {
		mode = ++cp;
		first = 0;
		goto again;
	}
	for (cp = mode; *cp; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);
	for (pf = formats; pf->f_mode; pf++)
		if (strcmp(pf->f_mode, mode) == 0)
			break;
	if (pf->f_mode == 0) {
		nak(EBADOP);
		exit(1);
	}
	ecode = (*pf->f_validate)(&filename, tp->th_opcode);
	if (logging) {
		syslog(LOG_INFO, "%s: %s request for %s: %s",
			verifyhost((struct sockaddr_in *)&from),
			tp->th_opcode == WRQ ? "write" : "read",
			filename, errtomsg(ecode));
	}
	if (ecode) {
		/*
		 * Avoid storms of naks to a RRQ broadcast for a relative
		 * bootfile pathname from a diskless Sun.
		 */
		if (suppress_naks && *filename != '/' && ecode == ENOTFOUND)
			exit(0);
		nak(ecode);
		exit(1);
	}
if (debugmsg)
fprintf(stderr, "Calling handling routine.\n");
	if (tp->th_opcode == WRQ)
		(*pf->f_recv)(pf);
	else
		(*pf->f_send)(pf);
	exit(0);
}


FILE *file;

/*
 * Validate file access.  Since we
 * have no uid or gid, for now require
 * file to exist and be publicly
 * readable/writable.
 * If we were invoked with arguments
 * from inetd then the file must also be
 * in one of the given directory prefixes.
 * Note also, full path name must be
 * given as we have no login directory.
 */
int
validate_access(filep, mode)
	char **filep;
	int mode;
{
	struct stat stbuf;
	int	fd;
	struct dirlist *dirp;
	static char pathname[MAXPATHLEN];
	char *filename = *filep;
if (debugmsg)
fprintf(stderr, "Entering validate_access...\n");
	/*
	 * Prevent tricksters from getting around the directory restrictions
	 */
	if (strstr(filename, "/../"))
		return (EACCESS);

	if (*filename == '/') {
		/*
		 * Allow the request if it's in one of the approved locations.
		 * Special case: check the null prefix ("/") by looking
		 * for length = 1 and relying on the arg. processing that
		 * it's a /.
		 */
		for (dirp = dirs; dirp->name != NULL; dirp++) {
			if (dirp->len == 1 ||
			    (!strncmp(filename, dirp->name, dirp->len) &&
			     filename[dirp->len] == '/'))
				    break;
		}
		/* If directory list is empty, allow access to any file */
		if (dirp->name == NULL && dirp != dirs)
			return (EACCESS);
		if (stat(filename, &stbuf) < 0)
			return (errno == ENOENT ? ENOTFOUND : EACCESS);
		if ((stbuf.st_mode & S_IFMT) != S_IFREG)
			return (ENOTFOUND);
		if (mode == RRQ) {
			if ((stbuf.st_mode & S_IROTH) == 0)
				return (EACCESS);
		} else {
			if ((stbuf.st_mode & S_IWOTH) == 0)
				return (EACCESS);
		}
	} else {
		int err;

		/*
		 * Relative file name: search the approved locations for it.
		 * Don't allow write requests or ones that avoid directory
		 * restrictions.
		 */

		if (mode != RRQ || !strncmp(filename, "../", 3))
			return (EACCESS);

		/*
		 * If the file exists in one of the directories and isn't
		 * readable, continue looking. However, change the error code
		 * to give an indication that the file exists.
		 */
		err = ENOTFOUND;
		for (dirp = dirs; dirp->name != NULL; dirp++) {
			sprintf(pathname, "%s/%s", dirp->name, filename);
			if (stat(pathname, &stbuf) == 0 &&
			    (stbuf.st_mode & S_IFMT) == S_IFREG) {
				if ((stbuf.st_mode & S_IROTH) != 0) {
					break;
				}
				err = EACCESS;
			}
		}
		if (dirp->name == NULL)
			return (err);
		*filep = filename = pathname;
	}
	fd = open(filename, mode == RRQ ? 0 : 1);
	if (fd < 0)
		return (errno + 100);
	file = fdopen(fd, (mode == RRQ)? "r":"w");
	if (file == NULL) {
		return errno+100;
	}
	return (0);
}

int	timeout;
jmp_buf	timeoutbuf;

void
timer()
{

	timeout += rexmtval;
	if (timeout >= maxtimeout)
		exit(1);
	longjmp(timeoutbuf, 1);
}

/*
 * Send the requested file.
 */
void
sendfile(pf)
	struct formats *pf;
{
	struct tftphdr *dp, *r_init();
	register struct tftphdr *ap;    /* ack packet */
	register int size, n;
	volatile int block;
if (debugmsg)
fprintf(stderr, "Entering sendfile...\n");

	signal(SIGALRM, timer);
	dp = r_init();
	ap = (struct tftphdr *)ackbuf;
	block = 1;
	do {
		size = readit(file, &dp, pf->f_convert);
		if (size < 0) {
			nak(errno + 100);
			goto abort;
		}
		dp->th_opcode = htons((u_short)DATA);
		dp->th_block = htons((u_short)block);
		timeout = 0;
		(void)setjmp(timeoutbuf);

send_data:
if (debugmsg)
fprintf(stderr, "Sending data to peer...");
		if (send(peer, dp, size + 4, 0) != size + 4) {
		        perror("tftpd: send");
			syslog(LOG_ERR, "tftpd: write: %m\n");
			goto abort;
		}
if (debugmsg)
fprintf(stderr, "done!\n");
		read_ahead(file, pf->f_convert);
		for ( ; ; ) {
			alarm(rexmtval);        /* read the ack */
if (debugmsg)
fprintf(stderr, "Waiting for ack from peer...");
			n = recv(peer, ackbuf, sizeof (ackbuf), 0);
			alarm(0);
			if (n < 0) {
			        perror("tftpd: recv");
				syslog(LOG_ERR, "tftpd: read: %m\n");
				goto abort;
			}
if (debugmsg)
fprintf(stderr, "got it!\n");
			ap->th_opcode = ntohs((u_short)ap->th_opcode);
			ap->th_block = ntohs((u_short)ap->th_block);

			if (ap->th_opcode == ERROR)
				goto abort;

			if (ap->th_opcode == ACK) {
				if (ap->th_block == block)
					break;
				/* Re-synchronize with the other side */
				(void) synchnet(peer);
				if (ap->th_block == (block -1))
					goto send_data;
			}

		}
		block++;
	} while (size == SEGSIZE);
abort:
if (debugmsg)
fprintf(stderr, "Done sending!\n");	
	(void) fclose(file);
}

void
justquit()
{
	exit(0);
}


/*
 * Receive a file.
 */
void
recvfile(pf)
	struct formats *pf;
{
	struct tftphdr *dp, *w_init();
	register struct tftphdr *ap;    /* ack buffer */
	register int n, size;
	volatile int block;
if (debugmsg)
fprintf(stderr, "Entering recvfile...\n");
	signal(SIGALRM, timer);
	dp = w_init();
	ap = (struct tftphdr *)ackbuf;
	block = 0;
	do {
		timeout = 0;
		ap->th_opcode = htons((u_short)ACK);
		ap->th_block = htons((u_short)block);
		block++;
		(void) setjmp(timeoutbuf);
send_ack:
		if (send(peer, ackbuf, 4, 0) != 4) {
			syslog(LOG_ERR, "tftpd: write: %m\n");
			goto abort;
		}
		write_behind(file, pf->f_convert);
		for ( ; ; ) {
			alarm(rexmtval);
			n = recv(peer, dp, PKTSIZE, 0);
			alarm(0);
			if (n < 0) {            /* really? */
				syslog(LOG_ERR, "tftpd: read: %m\n");
				goto abort;
			}
			dp->th_opcode = ntohs((u_short)dp->th_opcode);
			dp->th_block = ntohs((u_short)dp->th_block);
			if (dp->th_opcode == ERROR)
				goto abort;
			if (dp->th_opcode == DATA) {
				if (dp->th_block == block) {
					break;   /* normal */
				}
				/* Re-synchronize with the other side */
				(void) synchnet(peer);
				if (dp->th_block == (block-1))
					goto send_ack;          /* rexmit */
			}
		}
		/*  size = write(file, dp->th_data, n - 4); */
		size = writeit(file, &dp, n - 4, pf->f_convert);
		if (size != (n-4)) {                    /* ahem */
			if (size < 0) nak(errno + 100);
			else nak(ENOSPACE);
			goto abort;
		}
	} while (size == SEGSIZE);
	write_behind(file, pf->f_convert);
	(void) fclose(file);            /* close data file */

	ap->th_opcode = htons((u_short)ACK);    /* send the "final" ack */
	ap->th_block = htons((u_short)(block));
	(void) send(peer, ackbuf, 4, 0);

	signal(SIGALRM, justquit);      /* just quit on timeout */
	alarm(rexmtval);
	n = recv(peer, buf, sizeof (buf), 0); /* normally times out and quits */
	alarm(0);
	if (n >= 4 &&                   /* if read some data */
	    dp->th_opcode == DATA &&    /* and got a data block */
	    block == dp->th_block) {	/* then my last ack was lost */
		(void) send(peer, ackbuf, 4, 0);     /* resend final ack */
	}
abort:
	return;
}

struct errmsg {
	int	e_code;
	char	*e_msg;
} errmsgs[] = {
	{ EUNDEF,	"Undefined error code" },
	{ ENOTFOUND,	"File not found" },
	{ EACCESS,	"Access violation" },
	{ ENOSPACE,	"Disk full or allocation exceeded" },
	{ EBADOP,	"Illegal TFTP operation" },
	{ EBADID,	"Unknown transfer ID" },
	{ EEXISTS,	"File already exists" },
	{ ENOUSER,	"No such user" },
	{ -1,		0 }
};

static char *
errtomsg(error)
	int error;
{
	static char buf[20];
	register struct errmsg *pe;
	if (error == 0)
		return "success";
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			return pe->e_msg;
	sprintf(buf, "error %d", error);
	return buf;
}

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
static void
nak(error)
	int error;
{
	register struct tftphdr *tp;
	int length;
	register struct errmsg *pe;
if (debugmsg)
fprintf(stderr, "Entering nak...\n");

	tp = (struct tftphdr *)buf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = strerror(error - 100);
		tp->th_code = EUNDEF;   /* set 'undef' errorcode */
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg);
	tp->th_msg[length] = '\0';
	length += 5;
	if (send(peer, buf, length, 0) != length)
		syslog(LOG_ERR, "nak: %m\n");
}

static char *
verifyhost(fromp)
	struct sockaddr_in *fromp;
{
	struct hostent *hp;

#ifdef INET6
	int af;
if (debugmsg)
fprintf(stderr, "Entering verifyhost...\n");
	af = ((struct sockaddr *)fromp)->sa_family;
	hp = (struct hostent *)addr2hostname((af == AF_INET6) ?
	    (char *)&((struct sockaddr_in6 *)fromp)->sin6_addr :
	    (char *)&fromp->sin_addr,
	    (af == AF_INET6) ? sizeof(struct in_addr6) : 
	    sizeof(struct in_addr), af);
#else
	hp = gethostbyaddr((char *)&fromp->sin_addr, sizeof (fromp->sin_addr),
			    fromp->sin_family);
#endif /* INET6 */
	if (hp)
		return hp->h_name;
	else
#ifdef INET6
	  if (af == AF_INET6)
	        return ((char *)addr2ascii(af, 
	            (char *)&(((struct sockaddr_in6 *)fromp)->sin6_addr), 
	            sizeof(struct in_addr6), NULL)); 
	  else
#endif /* INET6 */
		return inet_ntoa(fromp->sin_addr);

}
