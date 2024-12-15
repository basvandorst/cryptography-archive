/*
 * Originally from 4.4-Lite BSD.  Modifications to support IPv6 and IPsec
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
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef lint
char copyright[] =
    "@(#) Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994\nThe Regents of the University of California.  All rights reserved.\n";
static  char rcsid[] =
    "@(#)/master/usr.sbin/tcpdump/tcpdump/tcpdump.c,v 2.1 1995/02/03 18:15:27 polk Exp (LBL)";
#endif

/*
 * tcpdump - monitor tcp/ip traffic on an ethernet.
 *
 * First written in 1987 by Van Jacobson, Lawrence Berkeley Laboratory.
 * Mercilessly hacked and occasionally improved since then via the
 * combined efforts of Van, Steve McCanne and Craig Leres of LBL.
 *
 * IPv6 mods, and the -a flag by Ken Chin, Bao Phan, and Dan McDonald of NRL.
 */

#include <sys/types.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include <unistd.h>
#include <string.h>

#include "interface.h"
#include "addrtoname.h"

int aflag;                      /* print packet in ascii */
int fflag;			/* don't translate "foreign" IP address */
int nflag;			/* leave addresses as numbers */
int Nflag;			/* remove domains from printed host names */
int pflag;			/* don't go promiscuous */
int qflag;			/* quick (shorter) output */
int tflag = 1;			/* print packet arrival time */
int eflag;			/* print ethernet header */
int vflag;			/* verbose */
int xflag;			/* print packet in hex */
int Oflag = 1;			/* run filter code optimizer */
int Sflag;			/* print raw TCP sequence numbers */
int packettype;

int dflag;			/* print filter code */

char *program_name;

int thiszone;

SIGRET cleanup(int);
extern void bpf_dump(struct bpf_program *, int);

/* Length of saved portion of packet. */
int snaplen = DEFAULT_SNAPLEN;

struct printer {
	pcap_handler f;
	int type;
};

static struct printer printers[] = {
	{ ether_if_print,	DLT_EN10MB },
	{ sl_if_print,		DLT_SLIP },
	{ ppp_if_print,		DLT_PPP },
	{ fddi_if_print,	DLT_FDDI },
	{ null_if_print,	DLT_NULL },
	{ NULL,			0 },
};

static pcap_handler
lookup_printer(int type)
{
	struct printer *p;

	for (p = printers; p->f; ++p)
		if (type == p->type)
			return p->f;

	error("unknown data link type 0x%x", type);
	/* NOTREACHED */
}

static pcap_t *pd;

#ifdef __osf__
#include <sys/sysinfo.h>
#include <sys/proc.h>
void
abort_on_misalignment()
{
	int buf[2];
	
	buf[0] = SSIN_UACPROC;
	buf[1] = UAC_SIGBUS;
	if (setsysinfo(SSI_NVPAIRS, buf, 1, 0, 0) < 0) {
		perror("setsysinfo");
		exit(1);
	}
}

#endif

int
main(int argc, char **argv)
{
	register int cnt, op;
	u_long localnet, netmask;
	register char *cp, *infile, *cmdbuf, *device, *RFileName, *WFileName;
	pcap_handler printer;
	struct bpf_program fcode;
	u_char *pcap_userdata;
	char errbuf[PCAP_ERRBUF_SIZE];

	extern char *optarg;
	extern int optind, opterr;

#ifdef __osf__
	abort_on_misalignment();
#endif

	cnt = -1;
	device = NULL;
	infile = NULL;
	RFileName = NULL;
	WFileName = NULL;
	if ((cp = strrchr(argv[0], '/')) != NULL)
		program_name = cp + 1;
	else
		program_name = argv[0];

	opterr = 0;
	while ((op = getopt(argc, argv, "ac:defF:i:lnNOpqr:s:StT:vw:xY")) != EOF)
		switch (op) {
		case 'a':
			++aflag;
			break;

		case 'c':
			cnt = atoi(optarg);
			break;

		case 'd':
			++dflag;
			break;

		case 'e':
			++eflag;
			break;

		case 'f':
			++fflag;
			break;

		case 'F':
			infile = optarg;
			break;

		case 'i':
			device = optarg;
			break;

		case 'l':
			setlinebuf(stdout);
			break;

		case 'n':
			++nflag;
			break;

		case 'N':
			++Nflag;
			break;

		case 'O':
			Oflag = 0;
			break;

		case 'p':
			++pflag;
			break;

		case 'q':
			++qflag;
			break;

		case 'r':
			RFileName = optarg;
			break;

		case 's':
			snaplen = atoi(optarg);
			break;

		case 'S':
			++Sflag;
			break;

		case 't':
			--tflag;
			break;

		case 'T':
			if (strcasecmp(optarg, "vat") == 0)
				packettype = 1;
			else if (strcasecmp(optarg, "wb") == 0)
				packettype = 2;
			else if (strcasecmp(optarg, "rpc") == 0)
				packettype = 3;
			else if (strcasecmp(optarg, "rtp") == 0)
				packettype = 4;
			else
				error("unknown packet type `%s'", optarg);
			break;

		case 'v':
			++vflag;
			break;

		case 'w':
			WFileName = optarg;
			break;
#ifdef YYDEBUG
		case 'Y':
			{
			extern int yydebug;
			yydebug = 1;
			}
			break;
#endif
		case 'x':
			++xflag;
			break;

		default:
			usage();
			/* NOTREACHED */
		}

	if (tflag > 0)
		thiszone = gmt2local();

	if (RFileName != NULL) {
		/*
		 * We don't need network access, so set it back to the user id.
		 * Also, this prevents the user from reading anyone's
		 * trace file.
		 */
		setuid(getuid());

		pd = pcap_open_offline(RFileName, errbuf);
		if (pd == NULL)
			error(errbuf);
		localnet = 0;
		netmask = 0;
		if (fflag != 0)
			error("-f and -r options are incompatible");
	} else {
		if (device == NULL) {
			device = pcap_lookupdev(errbuf);
			if (device == NULL)
				error(errbuf);
		}
		pd = pcap_open_live(device, snaplen, !pflag, 1000, errbuf);
		if (pd == NULL)
			error(errbuf);
		if (pcap_lookupnet(device, &localnet, &netmask, errbuf) < 0)
			error(errbuf);
		/*
		 * Let user own process after socket has been opened.
		 */
		setuid(getuid());
	}
	if (infile)
		cmdbuf = read_infile(infile);
	else
		cmdbuf = copy_argv(&argv[optind]);

	if (pcap_compile(pd, &fcode, cmdbuf, Oflag, netmask) < 0)
		error(pcap_geterr(pd));
	if (dflag) {
		bpf_dump(&fcode, dflag);
		exit(0);
	}
	init_addrtoname(fflag, localnet, netmask);

	(void)signal(SIGTERM, cleanup);
	(void)signal(SIGINT, cleanup);
	(void)signal(SIGHUP, cleanup);

	if (pcap_setfilter(pd, &fcode) < 0)
		error(pcap_geterr(pd));
	if (WFileName) {
		pcap_dumper_t *p = pcap_dump_open(pd, WFileName);
		if (p == NULL)
			error(pcap_geterr(pd));
		printer = pcap_dump;
		pcap_userdata = (u_char *)p;
	} else {
		printer = lookup_printer(pcap_datalink(pd));
		pcap_userdata = 0;
	}
	if (RFileName == NULL) {
		fprintf(stderr, "%s: listening on %s\n", program_name, device);
		fflush(stderr);
	}
	pcap_loop(pd, cnt, printer, pcap_userdata);
	pcap_close(pd);
	exit(0);
}

/* make a clean exit on interrupts */
SIGRET
cleanup(int signo)
{
	struct pcap_stat stat;

	/* Can't print the summary if reading from a savefile */
	if (pd != NULL && pcap_file(pd) == NULL) {
		(void)fflush(stdout);
		putc('\n', stderr);
		if (pcap_stats(pd, &stat) < 0)
			(void)fprintf(stderr, "pcap_stats: %s\n",
			    pcap_geterr(pd));
		else {
			(void)fprintf(stderr, "%d packets received by filter\n",
			    stat.ps_recv);
			(void)fprintf(stderr, "%d packets dropped by kernel\n",
			    stat.ps_drop);
		}
	}
	exit(0);
}

/*
 * Nice character printer, prints characters in two spaces, attempting to
 * print nasty ones nicely.  Returns string for printf.  Uses static storage,
 * so be nice and use results before calling again.
 *
 * (note: Should eliminate sprintf(), as that is time-consuming.)
 */
char *
nice_char_print(u_char toprint)
{
  static char buf[3];

  if (toprint >= ' ' && toprint <= '~')
    (void)sprintf(buf,"%c ",toprint);
  else if (toprint >= 1 && toprint <= 26)
    (void)sprintf(buf,"^%c",'@'+toprint);
  else if (toprint == 0)
    strcpy(buf,"^@");
  else if (toprint == 127)
    strcpy(buf,"^?");
  else if (toprint == 27)
    strcpy(buf,"^[");
  else strcpy(buf,"..");

  return buf;
}

/*
 * Print buffer out as ASCII characters.  Alignment is not an issue.
 */
void
ascii_print(register const u_char *bp, register int length)
{
  int i = 0, even = (length & 1)?length -1:length;

  fputs("\n",stdout);
  while (i < even) {
    if ((i % 16) == 0) 
      (void)printf("\n\t\t\t");
    fputs(" ",stdout);
    fputs(nice_char_print(*bp),stdout);
    fputs(nice_char_print(*(bp+1)),stdout);
    i += 2;
    bp += 2;
  }
  if (length & 1) {
    if ((i % 8) == 0)
      (void)fputs("\n\t\t\t",stdout);
    fputs(" ",stdout);
    fputs(nice_char_print(*bp),stdout);
  }
}

/* Like default_print() but data need not be aligned */
void
default_print_unaligned(register const u_char *cp, register int length)
{
	register u_int i, s;
	register int nshorts;

	nshorts = (u_int) length / sizeof(u_short);
	i = 0;
	while (--nshorts >= 0) {
		if ((i++ % 8) == 0)
			(void)printf("\n\t\t\t");
		s = *cp++;
		(void)printf(" %02x%02x", s, *cp++);
	}
	if (length & 1) {
		if ((i % 8) == 0)
			(void)printf("\n\t\t\t");
		(void)printf(" %02x", *cp);
	}
}

void
default_print(register const u_char *bp, register int length)
{
	register const u_short *sp;
	register u_int i;
	register int nshorts;

	if ((int)bp & 1) {
		default_print_unaligned(bp, length);
		return;
	}
	sp = (u_short *)bp;
	nshorts = (u_int) length / sizeof(u_short);
	i = 0;
	while (--nshorts >= 0) {
		if ((i++ % 8) == 0)
			(void)printf("\n\t\t\t");
		(void)printf(" %04x", ntohs(*sp++));
	}

	if (length & 1) {
		if ((i % 8) == 0)
			(void)printf("\n\t\t\t");
		(void)printf(" %02x", *(u_char *)sp);
	}
}

void
usage()
{
	extern char version[];

	(void)fprintf(stderr, "Version %s\n", version);
	(void)fprintf(stderr,
"Usage: tcpdump [-deflnOpqtvx] [-c count] [-i interface]\n");
	(void)fprintf(stderr,
"\t\t[-r filename] [-w filename] [expr]\n");
	exit(-1);
}
