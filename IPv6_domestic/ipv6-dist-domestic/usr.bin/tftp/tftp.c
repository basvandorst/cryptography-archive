/*
 * tftp.c  --  Workhorse function for tftp.  Sends/receives files.
 *
 * Originally from 4.4-Lite BSD.  Modifications to support IPv6
 * are copyright 1995 by Bao Phan, Randall Atkinson, & Dan McDonald,
 * All Rights Reserved.  All Rights under this copyright are assigned to NRL.
 ----------------------------------------------------------------------*/

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
static char sccsid[] = "@(#)tftp.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */

/*
 * TFTP User Program -- Protocol Machines
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>
#ifdef INET6
#include <netinet6/in6.h>
#endif /* INET6 */

#include <arpa/tftp.h>

#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "extern.h"
#include "tftpsubs.h"

extern	int errno;

#ifdef INET6
extern  int     fv4;
extern  int     fv6;
extern  struct sockaddr_in6 peeraddr6;
#endif /* INET6 */
extern  struct sockaddr_in peeraddr;	/* filled in by main */
extern  int     f;			/* the opened socket */
extern  int     trace;
extern  int     verbose;
extern  int     rexmtval;
extern  int     maxtimeout;

#define PKTSIZE    SEGSIZE+4
char    ackbuf[PKTSIZE];
int	timeout;
jmp_buf	toplevel;
jmp_buf	timeoutbuf;

static void nak __P((int));
static int makerequest __P((int, const char *, struct tftphdr *, const char *));
static void printstats __P((const char *, unsigned long));
static void startclock __P((void));
static void stopclock __P((void));
static void timer __P((int));
static void tpacket __P((const char *, struct tftphdr *, int));

/*
 * Send the requested file.
 */
void
sendfile(fd, name, mode)
	int fd;
	char *name;
	char *mode;
{
	register struct tftphdr *ap;	   /* data and ack packets */
	struct tftphdr *r_init(), *dp;
	register int n;
	volatile int block, size, convert;
	volatile unsigned long amount;
#ifdef INET6
	struct sockaddr_in6 from;
#else
	struct sockaddr_in from;
#endif /* INET6 */
	int fromlen;
	FILE *file;

	startclock();		/* start stat's clock */
	dp = r_init();		/* reset fillbuf/read-ahead code */
	ap = (struct tftphdr *)ackbuf;
	file = fdopen(fd, "r");
	convert = !strcmp(mode, "netascii");
	block = 0;
	amount = 0;

	signal(SIGALRM, timer);
	do {
		if (block == 0)
			size = makerequest(WRQ, name, dp, mode) - 4;
		else {
		/*	size = read(fd, dp->th_data, SEGSIZE);	 */
			size = readit(file, &dp, convert);
			if (size < 0) {
				nak(errno + 100);
				break;
			}
			dp->th_opcode = htons((u_short)DATA);
			dp->th_block = htons((u_short)block);
		}
		timeout = 0;
		(void) setjmp(timeoutbuf);
send_data:
		if (trace)
			tpacket("sent", dp, size + 4);
#ifdef INET6
		n = sendto(f, dp, size + 4, 0, (f == fv6) ? 
		    (struct sockaddr *)&peeraddr6 : 
		    (struct sockaddr *)&peeraddr,
		    (f == fv6) ? sizeof(peeraddr6) : sizeof(peeraddr));
#else
		n = sendto(f, dp, size + 4, 0,
		    (struct sockaddr *)&peeraddr, sizeof(peeraddr));
#endif /* INET6 */
		if (n != size + 4) {
			perror("tftp: sendto");
			goto abort;
		}
		read_ahead(file, convert);
		for ( ; ; ) {
			alarm(rexmtval);
			do {
				fromlen = sizeof(from);
				n = recvfrom(f, ackbuf, sizeof(ackbuf), 0,
				    (struct sockaddr *)&from, &fromlen);
			} while (n <= 0);
			alarm(0);
			if (n < 0) {
				perror("tftp: recvfrom");
				goto abort;
			}
#ifdef INET6
			if (f == fv6)
			        peeraddr6.sin6_port = from.sin6_port;
			else
			        peeraddr.sin_port = 
				  ((struct sockaddr_in *)&from)->sin_port;
#else
			peeraddr.sin_port = from.sin_port;	/* added */
#endif /* INET6 */
			if (trace)
				tpacket("received", ap, n);
			/* should verify packet came from server */
			ap->th_opcode = ntohs(ap->th_opcode);
			ap->th_block = ntohs(ap->th_block);
			if (ap->th_opcode == ERROR) {
				printf("Error code %d: %s\n", ap->th_code,
					ap->th_msg);
				goto abort;
			}
			if (ap->th_opcode == ACK) {
				int j;

				if (ap->th_block == block) {
					break;
				}
				/* On an error, try to synchronize
				 * both sides.
				 */
				j = synchnet(f);
				if (j && trace) {
					printf("discarded %d packets\n",
							j);
				}
				if (ap->th_block == (block-1)) {
					goto send_data;
				}
			}
		}
		if (block > 0)
			amount += size;
		block++;
	} while (size == SEGSIZE || block == 1);
abort:
	fclose(file);
	stopclock();
	if (amount > 0)
		printstats("Sent", amount);
}

/*
 * Receive a file.
 */
void
recvfile(fd, name, mode)
	int fd;
	char *name;
	char *mode;
{
	register struct tftphdr *ap;
	struct tftphdr *dp, *w_init();
	register int n;
	volatile int block, size, firsttrip;
	volatile unsigned long amount;
#ifdef INET6
	struct sockaddr_in6 from;
#else
	struct sockaddr_in from;
#endif /* INET6 */
	int fromlen;
	FILE *file;
	volatile int convert;		/* true if converting crlf -> lf */

	startclock();
	dp = w_init();
	ap = (struct tftphdr *)ackbuf;
	file = fdopen(fd, "w");
	convert = !strcmp(mode, "netascii");
	block = 1;
	firsttrip = 1;
	amount = 0;

	signal(SIGALRM, timer);
	do {
		if (firsttrip) {
			size = makerequest(RRQ, name, ap, mode);
			firsttrip = 0;
		} else {
			ap->th_opcode = htons((u_short)ACK);
			ap->th_block = htons((u_short)(block));
			size = 4;
			block++;
		}
		timeout = 0;
		(void) setjmp(timeoutbuf);
send_ack:
		if (trace)
			tpacket("sent", ap, size);
#ifdef INET6
		if (sendto(f, ackbuf, size, 0, (f == fv6) ? 
		    (struct sockaddr *)&peeraddr6 : 
		    (struct sockaddr *)&peeraddr, (f == fv6) ?
		    sizeof(peeraddr6) :  sizeof(peeraddr)) != size) {   
#else
		if (sendto(f, ackbuf, size, 0, (struct sockaddr *)&peeraddr,
		    sizeof(peeraddr)) != size) {
#endif /* INET6 */
			alarm(0);
			perror("tftp: sendto");
			goto abort;
		}
		write_behind(file, convert);
		for ( ; ; ) {
			alarm(rexmtval);
			do  {
				fromlen = sizeof(from);
				n = recvfrom(f, dp, PKTSIZE, 0,
				    (struct sockaddr *)&from, &fromlen);
			} while (n <= 0);
			alarm(0);
			if (n < 0) {
				perror("tftp: recvfrom");
				goto abort;
			}
#ifdef INET6
			if (f == fv6)
			        peeraddr6.sin6_port = from.sin6_port;
			else
			        peeraddr.sin_port = 
				  ((struct sockaddr_in *)&from)->sin_port;
#else
			peeraddr.sin_port = from.sin_port;	/* added */
#endif /* INET6 */

			if (trace)
				tpacket("received", dp, n);
			/* should verify client address */
			dp->th_opcode = ntohs(dp->th_opcode);
			dp->th_block = ntohs(dp->th_block);
			if (dp->th_opcode == ERROR) {
				printf("Error code %d: %s\n", dp->th_code,
					dp->th_msg);
				goto abort;
			}
			if (dp->th_opcode == DATA) {
				int j;

				if (dp->th_block == block) {
					break;		/* have next packet */
				}
				/* On an error, try to synchronize
				 * both sides.
				 */
				j = synchnet(f);
				if (j && trace) {
					printf("discarded %d packets\n", j);
				}
				if (dp->th_block == (block-1)) {
					goto send_ack;	/* resend ack */
				}
			}
		}
	/*	size = write(fd, dp->th_data, n - 4); */
		size = writeit(file, &dp, n - 4, convert);
		if (size < 0) {
			nak(errno + 100);
			break;
		}
		amount += size;
	} while (size == SEGSIZE);
abort:						/* ok to ack, since user */
	ap->th_opcode = htons((u_short)ACK);	/* has seen err msg */
	ap->th_block = htons((u_short)block);
#ifdef INET6
	(void) sendto(f, ackbuf, 4, 0, (f == fv6) ?
	    (struct sockaddr *)&peeraddr6 : (struct sockaddr *)&peeraddr,
	    (f == fv6) ? sizeof(peeraddr6) : sizeof(peeraddr));
#else
	(void) sendto(f, ackbuf, 4, 0, (struct sockaddr *)&peeraddr,
	    sizeof(peeraddr));
#endif /* INET6 */
	write_behind(file, convert);		/* flush last buffer */
	fclose(file);
	stopclock();
	if (amount > 0)
		printstats("Received", amount);
}

static int
makerequest(request, name, tp, mode)
	int request;
	const char *name;
	struct tftphdr *tp;
	const char *mode;
{
	register char *cp;

	tp->th_opcode = htons((u_short)request);
	cp = tp->th_stuff;
	strcpy(cp, name);
	cp += strlen(name);
	*cp++ = '\0';
	strcpy(cp, mode);
	cp += strlen(mode);
	*cp++ = '\0';
	return (cp - (char *)tp);
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
	register struct errmsg *pe;
	register struct tftphdr *tp;
	int length;
	char *strerror();

	tp = (struct tftphdr *)ackbuf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = strerror(error - 100);
		tp->th_code = EUNDEF;
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg) + 4;
	if (trace)
		tpacket("sent", tp, length);
#ifdef INET6
	if (sendto(f, ackbuf, length, 0, (f == fv6) ?
	    (struct sockaddr *)&peeraddr6 : (struct sockaddr *)&peeraddr,
	    (f == fv6) ? sizeof(peeraddr6) : sizeof(peeraddr)) != length)
#else
	if (sendto(f, ackbuf, length, 0, (struct sockaddr *)&peeraddr,
	    sizeof(peeraddr)) != length)
#endif /* INET6 */
		perror("nak");
}

static void
tpacket(s, tp, n)
	const char *s;
	struct tftphdr *tp;
	int n;
{
	static char *opcodes[] =
	   { "#0", "RRQ", "WRQ", "DATA", "ACK", "ERROR" };
	register char *cp, *file;
	u_short op = ntohs(tp->th_opcode);
	char *index();

	if (op < RRQ || op > ERROR)
		printf("%s opcode=%x ", s, op);
	else
		printf("%s %s ", s, opcodes[op]);
	switch (op) {

	case RRQ:
	case WRQ:
		n -= 2;
		file = cp = tp->th_stuff;
		cp = index(cp, '\0');
		printf("<file=%s, mode=%s>\n", file, cp + 1);
		break;

	case DATA:
		printf("<block=%d, %d bytes>\n", ntohs(tp->th_block), n - 4);
		break;

	case ACK:
		printf("<block=%d>\n", ntohs(tp->th_block));
		break;

	case ERROR:
		printf("<code=%d, msg=%s>\n", ntohs(tp->th_code), tp->th_msg);
		break;
	}
}

struct timeval tstart;
struct timeval tstop;

static void
startclock()
{

	(void)gettimeofday(&tstart, NULL);
}

static void
stopclock()
{

	(void)gettimeofday(&tstop, NULL);
}

static void
printstats(direction, amount)
	const char *direction;
	unsigned long amount;
{
	double delta;
			/* compute delta in 1/10's second units */
	delta = ((tstop.tv_sec*10.)+(tstop.tv_usec/100000)) -
		((tstart.tv_sec*10.)+(tstart.tv_usec/100000));
	delta = delta/10.;      /* back to seconds */
	printf("%s %d bytes in %.1f seconds", direction, amount, delta);
	if (verbose)
		printf(" [%.0f bits/sec]", (amount*8.)/delta);
	putchar('\n');
}

static void
timer(sig)
	int sig;
{

	timeout += rexmtval;
	if (timeout >= maxtimeout) {
		printf("Transfer timed out.\n");
		longjmp(toplevel, -1);
	}
	longjmp(timeoutbuf, 1);
}
