/*
 * ping.c  --  Implement the ping(8) command.
 *
 * Copyright 1995 by Dan McDonald, Randall Atkinson, and Bao Phan
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
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Muuss.
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
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)ping.c	8.1 (Berkeley) 6/5/93";
#endif /* not lint */

/*
 *			P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *	Mike Muuss
 *	U. S. Army Ballistic Research Laboratory
 *	December, 1983
 *
 * Status -
 *	Public Domain.  Distribution Unlimited.
 * Bugs -
 *	More statistics could always be gathered.
 *	This program has to run SUID to ROOT to access the ICMP socket.
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/signal.h>

#include <net/if.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h> 

#define	DEFDATALEN	(64 - 8)	/* default data length */
#define	MAXIPLEN	60
#define MAXIP6LEN       128  
#define	MAXICMPLEN	76
#define	MAXPACKET	(65536 - 60 - 8)/* max packet size */
#define	MAXWAIT		10		/* max seconds to wait for response */
#define	NROUTES		9		/* number of record route slots */

#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))

/* various options */
int options;
#define	F_FLOOD		0x001
#define	F_INTERVAL	0x002
#define	F_NUMERIC	0x004
#define	F_PINGFILLED	0x008
#define	F_QUIET		0x010
#define	F_RROUTE	0x020
#define	F_SO_DEBUG	0x040
#define	F_SO_DONTROUTE	0x080
#define	F_VERBOSE	0x100

/*
 * MAX_DUP_CHK is the number of bits in received table, i.e. the maximum
 * number of received sequence numbers we can keep track of.  Change 128
 * to 8192 for complete accuracy...
 */
#define	MAX_DUP_CHK	(8 * 128)
int mx_dup_ck = MAX_DUP_CHK;
char rcvd_tbl[MAX_DUP_CHK / 8];

union {
  struct sockaddr       gen;
  struct sockaddr_in    in;
  struct sockaddr_in6   in6;
} whereto;                      /* who to ping */

int datalen = DEFDATALEN;
int s;				/* socket file descriptor */
u_char outpack[MAXPACKET];
char BSPACE = '\b';		/* characters written for flood */
char DOT = '.';
char *hostname;
int ident;			/* process id to identify our packets */
int af;                         /* address family */

/* counters */
long npackets;			/* max packets to transmit */
long nreceived;			/* # of packets we got back */
long nrepeats;			/* number of duplicates */
long ntransmitted;		/* sequence # for outbound packets = #sent */
int interval = 1;		/* interval between packets */

/* timing */
int timing;			/* flag to do timing */
double tmin = 999999999.0;	/* minimum round trip time */
double tmax = 0.0;		/* maximum round trip time */
double tsum = 0.0;		/* sum of all times, for doing average */

char *pr_addr();
void catcher(), finish();

main(argc, argv)
	int argc;
	char **argv;
{
	extern int errno, optind;
	extern char *optarg;
	struct timeval timeout;
	struct hostent *hp;
	struct protoent *proto;
	register int i;
	int ch, fdmask, hold, packlen, preload;
	u_char *datap, *packet;
	char *target, hnamebuf[MAXHOSTNAMELEN], *malloc();
#ifdef IP_OPTIONS
	char rspace[3 + 4 * NROUTES + 1];	/* record route space */
#endif
	char tempbuf[64];

	af = AF_INET;
	preload = 0;
	datap = &outpack[8 + sizeof(struct timeval)];
	while ((ch = getopt(argc, argv, "Rc:dfa:h:i:l:np:qrs:v")) != EOF)
		switch(ch) {
		case 'a':
		        if (strcmp(optarg, "inet") == 0)
			        af = AF_INET;
			else if (strcmp(optarg, "inet6") == 0)
			        af = AF_INET6;
			else {
			        (void)fprintf(stderr,
				    "ping: invalid family: %s\n",optarg);
			        exit(1);
			}
		        break;
		case 'c':
			npackets = atoi(optarg);
			if (npackets <= 0) {
				(void)fprintf(stderr,
				    "ping: bad number of packets to transmit.\n");
				exit(1);
			}
			break;
		case 'd':
			options |= F_SO_DEBUG;
			break;
		case 'f':
			if (getuid()) {
				(void)fprintf(stderr,
				    "ping: %s\n", strerror(EPERM));
				exit(1);
			}
			options |= F_FLOOD;
			setbuf(stdout, (char *)NULL);
			break;
		case 'i':		/* wait between sending packets */
			interval = atoi(optarg);
			if (interval <= 0) {
				(void)fprintf(stderr,
				    "ping: bad timing interval.\n");
				exit(1);
			}
			options |= F_INTERVAL;
			break;
		case 'l':
			preload = atoi(optarg);
			if (preload < 0) {
				(void)fprintf(stderr,
				    "ping: bad preload value.\n");
				exit(1);
			}
			break;
		case 'n':
			options |= F_NUMERIC;
			break;
		case 'p':		/* fill buffer with user pattern */
			options |= F_PINGFILLED;
			fill((char *)datap, optarg);
				break;
		case 'q':
			options |= F_QUIET;
			break;
		case 'R':
			options |= F_RROUTE;
			break;
		case 'r':
			options |= F_SO_DONTROUTE;
			break;
		case 's':		/* size of packet to send */
			datalen = atoi(optarg);
			if (datalen > MAXPACKET) {
				(void)fprintf(stderr,
				    "ping: packet size too large.\n");
				exit(1);
			}
			if (datalen <= 0) {
				(void)fprintf(stderr,
				    "ping: illegal packet size.\n");
				exit(1);
			}
			break;
		case 'v':
			options |= F_VERBOSE;
			break;
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();
	target = *argv;

	bzero((char *)&whereto, sizeof(whereto));
	whereto.gen.sa_family = af;
#ifdef SIN6_LEN  /* 4.4 BSD based systems have sa_len  */
	if (af == AF_INET)
	        whereto.gen.sa_len = sizeof(struct sockaddr_in);
	else if (af == AF_INET6)
	        whereto.gen.sa_len = sizeof(struct sockaddr_in6);
#endif /* SIN6_LEN */

	if (ascii2addr(af, target, tempbuf) > 0) {
		if (af == AF_INET)
		        bcopy(tempbuf, (char *)&whereto.in.sin_addr, 
			      sizeof(whereto.in.sin_addr));
	        else if (af == AF_INET6)
		        bcopy(tempbuf, (char *)&whereto.in6.sin6_addr, 
			      sizeof(whereto.in6.sin6_addr));
		hostname = target;
	} else {
		hp = (struct hostent *)hostname2addr(target, af);
		if (!hp) {
			(void)fprintf(stderr,
			    "ping: unknown host %s\n", target);
			exit(1);
		}
		if (af == AF_INET)
		        bcopy(hp->h_addr_list[0], 
			      (char *)&whereto.in.sin_addr, hp->h_length);
		else if (af == AF_INET6)
		        bcopy(hp->h_addr_list[0], 
			      (char *)&whereto.in6.sin6_addr, hp->h_length);

		(void)strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
		hostname = hnamebuf;
	}

	if (options & F_FLOOD && options & F_INTERVAL) {
		(void)fprintf(stderr,
		    "ping: -f and -i incompatible options.\n");
		exit(1);
	}

	if (datalen >= sizeof(struct timeval))	/* can we time transfer */
		timing = 1;
	packlen = datalen + MAXICMPLEN +((af==AF_INET) ? MAXIPLEN : MAXIP6LEN);

	if (!(packet = (u_char *)malloc((u_int)packlen))) {
		(void)fprintf(stderr, "ping: out of memory.\n");
		exit(1);
	}
	if (!(options & F_PINGFILLED))
		for (i = 8; i < datalen; ++i)
			*datap++ = i;

	ident = getpid() & 0xFFFF;

	if (!(proto = getprotobyname((af == AF_INET6) ? "icmpv6" : "icmp"))) {
		(void)fprintf(stderr, "ping: unknown protocol icmp.\n");
		exit(1);
	}

	if ((s = socket((af == AF_INET6) ? PF_INET6 : PF_INET, SOCK_RAW, 
	    proto->p_proto)) < 0) {
		perror("ping: socket");
		exit(1);
	}

	hold = 1;
	if (options & F_SO_DEBUG)
		(void)setsockopt(s, SOL_SOCKET, SO_DEBUG, (char *)&hold,
		    sizeof(hold));
	if (options & F_SO_DONTROUTE)
		(void)setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (char *)&hold,
		    sizeof(hold));

	/* record route option */
	if (options & F_RROUTE) {
#ifdef IP_OPTIONS
		rspace[IPOPT_OPTVAL] = IPOPT_RR;
		rspace[IPOPT_OLEN] = sizeof(rspace)-1;
		rspace[IPOPT_OFFSET] = IPOPT_MINOFF;
		if (setsockopt(s, IPPROTO_IP, IP_OPTIONS, rspace,
		    sizeof(rspace)) < 0) {
			perror("ping: record route");
			exit(1);
		}
#else
		(void)fprintf(stderr,
		  "ping: record route not available in this implementation.\n");
		exit(1);
#endif /* IP_OPTIONS */
	}

	/*
	 * When pinging the broadcast address, you can get a lot of answers.
	 * Doing something so evil is useful if you are trying to stress the
	 * ethernet, or just want to fill the arp cache to get some stuff for
	 * /etc/ethers.
	 */
	hold = 48 * 1024;
	(void)setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&hold,
	    sizeof(hold));

	if (af == AF_INET)
		(void)printf("PING %s (%s): %d data bytes\n", hostname,
		    inet_ntoa(*(struct in_addr *)&whereto.in.sin_addr.s_addr),
		    datalen);
	else if (af == AF_INET6)
	        (void)printf("PING %s (%s): %d data bytes\n", hostname,
		    addr2ascii(af, (char *)&whereto.in6.sin6_addr, 
		    sizeof(struct in_addr6), NULL), datalen); 
	else
		(void)printf("PING %s: %d data bytes\n", hostname, datalen);

	(void)signal(SIGINT, finish);
	(void)signal(SIGALRM, catcher);

	while (preload--)		/* fire off them quickies */
		pinger();

	if ((options & F_FLOOD) == 0)
		catcher();		/* start things going */

	for (;;) {
	        union {
		  struct sockaddr_in in;
		  struct sockaddr_in6 in6;
		} from;  
		register int cc;
		int fromlen;

		if (options & F_FLOOD) {
			pinger();
			timeout.tv_sec = 0;
			timeout.tv_usec = 10000;
			fdmask = 1 << s;
			if (select(s + 1, (fd_set *)&fdmask, (fd_set *)NULL,
			    (fd_set *)NULL, &timeout) < 1)
				continue;
		}
		fromlen = sizeof(from);
		if ((cc = recvfrom(s, (char *)packet, packlen, 0,
		    (struct sockaddr *)&from, &fromlen)) < 0) {
			if (errno == EINTR)
				continue;
			perror("ping: recvfrom");
			continue;
		}
		pr_pack((char *)packet, cc, (char *)&from);
		if (npackets && nreceived >= npackets)
			break;
	}
	finish();
	/* NOTREACHED */
}

/*
 * catcher --
 *	This routine causes another PING to be transmitted, and then
 * schedules another SIGALRM for 1 second from now.
 *
 * bug --
 *	Our sense of time will slowly skew (i.e., packets will not be
 * launched exactly at 1-second intervals).  This does not affect the
 * quality of the delay and loss statistics.
 */
void
catcher()
{
	int waittime;

	pinger();
	(void)signal(SIGALRM, catcher);
	if (!npackets || ntransmitted < npackets)
		alarm((u_int)interval);
	else {
		if (nreceived) {
			waittime = 2 * tmax / 1000;
			if (!waittime)
				waittime = 1;
		} else
			waittime = MAXWAIT;
		(void)signal(SIGALRM, finish);
		(void)alarm((u_int)waittime);
	}
}

/*
 * pinger --
 *	Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time.
 */
pinger()
{
	register struct icmp *icp;
	register struct ipv6_icmp *icp6;
	register int cc;
	int i;

	icp6 = (struct ipv6_icmp *)outpack;
	icp = (struct icmp *)outpack;

	/*
 	 * We could use only icp, since IPv6 icmp echo packet format 
	 * is identical to IPv4, but this is cleaner at the cost of  
	 * additional code
	 */  
	if (af == AF_INET) {
	        icp->icmp_type = ICMP_ECHO;
		icp->icmp_code = 0;
		icp->icmp_cksum = 0;
		icp->icmp_seq = ntransmitted++;
		icp->icmp_id = ident;			/* ID */
		CLR(icp->icmp_seq % mx_dup_ck);
	} else if (af == AF_INET6) {
	        icp6->icmp_type = ICMPV6_ECHO;
		icp6->icmp_code = 0;
		icp6->icmp_cksum = 0;
		icp6->icmp_echoid = ident;
		icp6->icmp_echoseq = ntransmitted++;
		CLR(icp6->icmp_echoseq % mx_dup_ck);
	} 

	if (timing)
		(void)gettimeofday((struct timeval *)&outpack[8],
		    (struct timezone *)NULL);

	cc = datalen + 8;			/* skips ICMP portion */

	/*
	 * compute IPv4 ICMP checksum here.
	 * for IPv6, just let the kernel do the checksum calculation in 
	 * ipv6_output, since the checksum now includes an ipv6 pseudo 
	 * header, which requires us to know the source address of the
	 * sending interface. 
	 */  
	if (af == AF_INET)
	        icp->icmp_cksum = in_cksum((u_short *)icp, cc);

	i = sendto(s, (char *)outpack, cc, 0, (struct sockaddr *)&whereto,
	    whereto.gen.sa_len);

	if (i < 0 || i != cc)  {
		if (i < 0)
			perror("ping: sendto");
		(void)printf("ping: wrote %s %d chars, ret=%d\n",
		    hostname, cc, i);
	}
	if (!(options & F_QUIET) && options & F_FLOOD)
		(void)write(STDOUT_FILENO, &DOT, 1);
}


/*
 * pr_pack --
 *	Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
pr_pack(buf, cc, from)
	char *buf;
	int cc;
        char *from;

{
	register struct icmp *icp;
	register struct ipv6_icmp *icp6;
	u_long l;
	register int i, j;
	register u_char *cp,*dp;
	static int old_rrlen;
	static char old_rr[MAX_IPOPTLEN];
	struct timeval tv, *tp;
	double triptime;
	int hlen, headerlen,dupflag;
	int htype, hsize, ttl;
	char *addr, *icmpdata;
	short icmptype;
	int icmpid, icmpseq;

	(void)gettimeofday(&tv, (struct timezone *)NULL);

	/* Parse IPv6 packet for icmp header */
	if (af == AF_INET6) {
	    char *nexthdr;

	    nexthdr = buf;
	    htype = ((struct ipv6 *)buf)->ipv6_nexthdr;
	    hsize = sizeof(struct ipv6);
	    hlen = 0;

	    do {
	        nexthdr += hsize;
	        hlen += hsize;
		/*
		 * Need to check for other ipv6 header type 
		 */
		switch(htype) {
		case NEXTHDR_HOP:
		  /* punt for now*/
		  hsize = ((struct ipv6_opthdr *)nexthdr)->oh_nexthdr + 8;
		  htype = ((struct ipv6_opthdr *)nexthdr)->oh_nexthdr;
		  break;
		case NEXTHDR_AUTH:
		  hsize = (((struct ipv6_auth *)nexthdr)->auth_datalen << 2) + 8;
		  htype = ((struct ipv6_auth *)nexthdr)->auth_nexthdr;
		  break;
		case NEXTHDR_ROUTING:
		  if (((struct ipv6_srcroute0 *)nexthdr)->i6sr_type == 0) {
		    hsize = ((struct ipv6_srcroute0 *)nexthdr)->i6sr_numaddrs *
		      sizeof(struct in_addr6) + 8;
		    htype = ((struct ipv6_srcroute0 *)nexthdr)->i6sr_nexthdr;
		  } else {
		    (void)fprintf(stderr, "ping: ipv6 routing type unknown\n");
		    return;
		  }
		  break;	      
		case NEXTHDR_FRAGMENT:
		  hsize = 8;
		  htype = ((struct ipv6_fraghdr *)nexthdr)->frag_nexthdr;
		  break;
		case NEXTHDR_ICMPV6:
		  hsize = -1;
		  break;	      
		default:
		  return;	    
		}
	    } while (hsize > 0);	
	    ttl = ((struct ipv6 *)buf)->ipv6_hoplimit;
	} else {
	    hlen = ((struct ip *)buf)->ip_hl << 2;
	    ttl = ((struct ip *)buf)->ip_ttl;
	}

	/* Keep this information, we'll need it later. */
	headerlen = hlen;

	/* Figure out the from address */
	addr = (af == AF_INET6) ? 
	      (char *)&(((struct sockaddr_in6 *)from)->sin6_addr): 
	      (char *)&(((struct sockaddr_in *)from)->sin_addr);

	if (cc < hlen + ICMP_MINLEN) {
		if (options & F_VERBOSE) 
		    (void)fprintf(stderr,
		      "ping: packet too short (%d bytes) from %s\n", cc,
		      addr2ascii(af, addr, (af == AF_INET6) ? 
		      sizeof(struct in_addr6) : sizeof(struct in_addr), NULL));
		return;
	}


	  
	/* Now the ICMP part */
	cc -= hlen;
	if (af == AF_INET6) {
	  icp6 = (struct ipv6_icmp *)(buf + hlen);
	  icmptype = icp6->icmp_type;
	  icmpid = icp6->icmp_echoid;
	  icmpdata = (char *)&icp6->icmp_echodata;
	  icmpseq = icp6->icmp_echoseq;
	} else {
	  icp = (struct icmp *)(buf + hlen);
	  icmptype = icp->icmp_type;
	  icmpid = icp->icmp_id;
	  icmpdata = (char *)&icp->icmp_data;
	  icmpseq = icp->icmp_seq;
        }

	if (((af == AF_INET) && (icmptype == ICMP_ECHOREPLY)) 
	     || ((af == AF_INET6) && (icmptype == ICMPV6_ECHOREPLY))) {
		if (icmpid != ident)
			return;			/* 'Twas not our ECHO */
		++nreceived;
		if (timing) {
			tp = (struct timeval *)icmpdata;
			tvsub(&tv, tp);
			triptime = ((double)tv.tv_sec) * 1000.0 +
			    ((double)tv.tv_usec) / 1000.0;
			tsum += triptime;
			if (triptime < tmin)
				tmin = triptime;
			if (triptime > tmax)
				tmax = triptime;
		}

		if (TST(icmpseq % mx_dup_ck)) {
			++nrepeats;
			--nreceived;
			dupflag = 1;
		} else {
			SET(icmpseq % mx_dup_ck);
			dupflag = 0;
		}

		if (options & F_QUIET)
			return;

		if (options & F_FLOOD)
			(void)write(STDOUT_FILENO, &BSPACE, 1);
		else {
			(void)printf("%d bytes from %s: icmp_seq=%u", cc,
			   addr2ascii(af, addr, (af == AF_INET6) ?
			   sizeof(struct in_addr6) : sizeof(struct in_addr), 
			   NULL), icmpseq);
			(void)printf(" ttl=%d", ttl);
			if (timing)
				(void)printf(" time=%g ms", triptime);
			if (dupflag)
				(void)printf(" (DUP!)");
			/* check the data */
			cp = (u_char*)&icmpdata[8];
			dp = &outpack[8 + sizeof(struct timeval)];
			for (i = 8; i < datalen; ++i, ++cp, ++dp) {
				if (*cp != *dp) {
	(void)printf("\nwrong data byte #%d should be 0x%x but was 0x%x",
	    i, *dp, *cp);
					cp = (u_char*)&icmpdata[0];
					for (i = 8; i < datalen; ++i, ++cp) {
						if ((i % 32) == 8)
							(void)printf("\n\t");
						(void)printf("%x ", *cp);
					}
					break;
				}
			}
		}
	} else {
		/* We've got something other than an ECHOREPLY */
		if (!(options & F_VERBOSE))
			return;
		(void)printf("%d bytes from %s: ", cc,
		    pr_addr(addr));
		if (af == AF_INET)
		        pr_icmph(buf + headerlen);
		else
		        pr_icmpv6h(buf + headerlen);
	}

	/* Display any IP options */
        /* We need to do the same for IPv6 */

if (af == AF_INET) {
	cp = (u_char *)buf + sizeof(struct ip);

	for (; hlen > (int)sizeof(struct ip); --hlen, ++cp)
		switch (*cp) {
		case IPOPT_EOL:
			hlen = 0;
			break;
		case IPOPT_LSRR:
			(void)printf("\nLSRR: ");
			hlen -= 2;
			j = *++cp;
			++cp;
			if (j > IPOPT_MINOFF)
				for (;;) {
					l = *++cp;
					l = (l<<8) + *++cp;
					l = (l<<8) + *++cp;
					l = (l<<8) + *++cp;
					if (l == 0)
						(void)printf("\t0.0.0.0");
				else {
				  l = ntohl(l);
				  (void)printf("\t%s", pr_addr((char *)&l));
				}
				hlen -= 4;
				j -= 4;
				if (j <= IPOPT_MINOFF)
					break;
				(void)putchar('\n');
			}
			break;
		case IPOPT_RR:
			j = *++cp;		/* get length */
			i = *++cp;		/* and pointer */
			hlen -= 2;
			if (i > j)
				i = j;
			i -= IPOPT_MINOFF;
			if (i <= 0)
				continue;
			if (i == old_rrlen
			    && cp == (u_char *)buf + sizeof(struct ip) + 2
			    && !bcmp((char *)cp, old_rr, i)
			    && !(options & F_FLOOD)) {
				(void)printf("\t(same route)");
				i = ((i + 3) / 4) * 4;
				hlen -= i;
				cp += i;
				break;
			}
			old_rrlen = i;
			bcopy((char *)cp, old_rr, i);
			(void)printf("\nRR: ");
			for (;;) {
				l = *++cp;
				l = (l<<8) + *++cp;
				l = (l<<8) + *++cp;
				l = (l<<8) + *++cp;
				if (l == 0)
					(void)printf("\t0.0.0.0");
				else {
				  l = ntohl(l); 
				  (void)printf("\t%s",pr_addr((char *)&l));
				}
				hlen -= 4;
				i -= 4;
				if (i <= 0)
					break;
				(void)putchar('\n');
			}
			break;
		case IPOPT_NOP:
			(void)printf("\nNOP");
			break;
		default:
			(void)printf("\nunknown option %x", *cp);
			break;
		}
      }
	if (!(options & F_FLOOD)) {
		(void)putchar('\n');
		(void)fflush(stdout);
	}
}

/*
 * in_cksum --
 *	Checksum routine for Internet Protocol family headers (C Version)
 */
in_cksum(addr, len)
	u_short *addr;
	int len;
{
	register int nleft = len;
	register u_short *w = addr;
	register int sum = 0;
	u_short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}

/*
 * tvsub --
 *	Subtract 2 timeval structs:  out = out - in.  Out is assumed to
 * be >= in.
 */
tvsub(out, in)
	register struct timeval *out, *in;
{
	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

/*
 * finish --
 *	Print out statistics, and give up.
 */
void
finish()
{
	register int i;

	(void)signal(SIGINT, SIG_IGN);
	(void)putchar('\n');
	(void)fflush(stdout);
	(void)printf("--- %s ping statistics ---\n", hostname);
	(void)printf("%ld packets transmitted, ", ntransmitted);
	(void)printf("%ld packets received, ", nreceived);
	if (nrepeats)
		(void)printf("+%ld duplicates, ", nrepeats);
	if (ntransmitted)
		if (nreceived > ntransmitted)
			(void)printf("-- somebody's printing up packets!");
		else
			(void)printf("%d%% packet loss",
			    (int) (((ntransmitted - nreceived) * 100) /
			    ntransmitted));
	(void)putchar('\n');
	if (nreceived && timing) {
		/* Only display average to microseconds */
		i = 1000.0 * tsum / (nreceived + nrepeats);
		(void)printf("round-trip min/avg/max = %g/%g/%g ms\n",
		    tmin, ((double)i) / 1000.0, tmax);
	}
	exit(0);
}

#ifdef notdef
static char *ttab[] = {
	"Echo Reply",		/* ip + seq + udata */
	"Dest Unreachable",	/* net, host, proto, port, frag, sr + IP */
	"Source Quench",	/* IP */
	"Redirect",		/* redirect type, gateway, + IP  */
	"Echo",
	"Time Exceeded",	/* transit, frag reassem + IP */
	"Parameter Problem",	/* pointer + IP */
	"Timestamp",		/* id + seq + three timestamps */
	"Timestamp Reply",	/* " */
	"Info Request",		/* id + sq */
	"Info Reply"		/* " */
};
#endif


/*
 * pr_icmph --
 *	Print a descriptive string about an ICMP header.
 */
pr_icmph(cp)
	char  *cp;
{
        struct icmp *icp;
	short icmptype, icmpcode;
	int icmpseq;
	char *icmpdata;

	icp = (struct icmp *)cp;
	icmptype = icp->icmp_type;
	icmpcode = icp->icmp_code;
        icmpdata = (char *)&icp->icmp_data;

	switch(icmptype) {
	case ICMP_ECHOREPLY:
		(void)printf("Echo Reply\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMP_UNREACH:
		switch(icmpcode) {
		case ICMP_UNREACH_NET:
			(void)printf("Destination Net Unreachable\n");
			break;
		case ICMP_UNREACH_HOST:
			(void)printf("Destination Host Unreachable\n");
			break;
		case ICMP_UNREACH_PROTOCOL:
			(void)printf("Destination Protocol Unreachable\n");
			break;
		case ICMP_UNREACH_PORT:
			(void)printf("Destination Port Unreachable\n");
			break;
		case ICMP_UNREACH_NEEDFRAG:
			(void)printf("frag needed and DF set\n");
			break;
		case ICMP_UNREACH_SRCFAIL:
			(void)printf("Source Route Failed\n");
			break;
		default:
			(void)printf("Dest Unreachable, Bad Code: %d\n",
			    icmpcode);
			break;
		}
		/* Print returned IP header information */

		pr_retip(icmpdata);

		break;
	case ICMP_SOURCEQUENCH:
		(void)printf("Source Quench\n");

		pr_retip(icmpdata);

		break;
	case ICMP_REDIRECT:
		switch(icmpcode) {
		case ICMP_REDIRECT_NET:
			(void)printf("Redirect Network");
			break;
		case ICMP_REDIRECT_HOST:
			(void)printf("Redirect Host");
			break;
		case ICMP_REDIRECT_TOSNET:
			(void)printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIRECT_TOSHOST:
			(void)printf("Redirect Type of Service and Host");
			break;
		default:
			(void)printf("Redirect, Bad Code: %d", icmpcode);
			break;
		}
		(void)printf("(New addr: 0x%08lx)\n", icp->icmp_gwaddr.s_addr);

		pr_retip(icmpdata);

		break;
	case ICMP_ECHO:
		(void)printf("Echo Request\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMP_TIMXCEED:
		switch(icmpcode) {
		case ICMP_TIMXCEED_INTRANS:
			(void)printf("Time to live exceeded\n");
			break;
		case ICMP_TIMXCEED_REASS:
			(void)printf("Frag reassembly time exceeded\n");
			break;
		default:
			(void)printf("Time exceeded, Bad Code: %d\n",
			    icmpcode);
			break;
		}
		pr_retip(icmpdata);

		break;
	case ICMP_PARAMPROB:
		(void)printf("Parameter problem: pointer = 0x%02x\n",
		    icp->icmp_hun.ih_pptr);

		pr_retip(icmpdata);

		break;
	case ICMP_TSTAMP:
		(void)printf("Timestamp\n");
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_TSTAMPREPLY:
		(void)printf("Timestamp Reply\n");
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_IREQ:
		(void)printf("Information Request\n");
		/* XXX ID + Seq */
		break;
	case ICMP_IREQREPLY:
		(void)printf("Information Reply\n");
		/* XXX ID + Seq */
		break;
#ifdef ICMP_MASKREQ
	case ICMP_MASKREQ:
		(void)printf("Address Mask Request\n");
		break;
#endif
#ifdef ICMP_MASKREPLY
	case ICMP_MASKREPLY:
		(void)printf("Address Mask Reply\n");
		break;
#endif
	default:
		(void)printf("Bad ICMP type: %d\n", icmptype);
	}
}



/*
 * pr_icmpv6h --
 *	Print a descriptive string about an ICMPv6 header.
 */
pr_icmpv6h(cp)
	char  *cp;
{
	struct ipv6_icmp *icp6;
	short icmptype, icmpcode;
	int icmpseq;
	char *icmpdata;

	  icp6 = (struct ipv6_icmp *)cp;
	  icmptype = icp6->icmp_type;
	  icmpcode = icp6->icmp_code;
          icmpdata = (char *)&icp6->icmp_echodata;

	switch(icmptype) {
	case ICMPV6_ECHOREPLY:
		(void)printf("Echo Reply\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMPV6_UNREACH:
		switch(icmpcode) {
		case ICMPV6_UNREACH_NOROUTE:
			(void)printf("Destination Route Not Available\n");
			break;
		case ICMPV6_UNREACH_ADMIN:
			(void)printf("Destination Administratively Prohibited\n");
			break;
		case ICMPV6_UNREACH_NOTNEIGHBOR:
			(void)printf("Destination Neighbor Unreachable\n");
			break;
		case ICMPV6_UNREACH_ADDRESS:
			(void)printf("Destination Address Unreachable\n");
			break;
		case ICMPV6_UNREACH_PORT:
			(void)printf("Destination Port Unreachable\n");
			break;
		default:
			(void)printf("Dest Unreachable, Bad Code: %d\n",
			    icmpcode);
			break;
		}
		/* Print returned IP header information */

		pr_retip(icmpdata);

		break;
	case ICMPV6_TOOBIG:
		(void)printf("Packet too big\n");
		pr_retip(icmpdata);
		break;
	case ICMPV6_TIMXCEED:
		switch(icmpcode) {
		case ICMPV6_TIMXCEED_INTRANS:
			(void)printf("Hop limit exceeded\n");
			break;
		case ICMPV6_TIMXCEED_REASS:
			(void)printf("Frag reassembly time exceeded\n");
			break;
		default:
			(void)printf("Time exceeded, Bad Code: %d\n",
			    icmpcode);
			break;
		}
		pr_retip(icmpdata);

		break;
	case ICMPV6_PARAMPROB:
		switch(icmpcode) {
		case ICMPV6_PARAMPROB_PROB:
		        (void)printf("Parameter Problem\n");
			break;
		case ICMPV6_PARAMPROB_NEXTHDR:
			(void)printf("Bad Next Header\n");
			break;
		case ICMPV6_PARAMPROB_BADOPT:
			(void)printf("Unrecognized IPv6 Option\n");
			break;
		}
		pr_retip(icmpdata);

		break;
	case ICMPV6_ECHO:
		(void)printf("Echo Request\n");
		/* XXX ID + Seq + Data */
		break;
	case ICMPV6_GRPQUERY:
		(void)printf("Group Membership Query\n");
		break;
	case ICMPV6_GRPREPORT:
		(void)printf("Group Membership Report\n");
		break;
	case ICMPV6_GRPTERM:
		(void)printf("Group Membership Termination\n");
		break;
/*
	case ICMPV6_SOLICIT:
		switch(icmpcode) {
		case ICMPV6_SOLICIT_GENERAL:
			(void)printf("General Solicit");
			break;
		case ICMPV6_SOLICIT_ROUTER:
			(void)printf("Router Solicit");
			break;
		case ICMPV6_SOLICIT_SERVICE:
			(void)printf("Service Solicit");
			break;
		default:
			(void)printf("Solicit, Bad Code: %d", icmpcode);
			break;
		}

		pr_retip(icmpdata);

		break;
	case ICMPV6_ADVERT:
		switch(icmpcode) {
		case ICMPV6_ADVERT_GENERAL:
			(void)printf("General Advertizement\n");
			break;
		case ICMPV6_ADVERT_ROUTER:
			(void)printf("Router Advertizement\n");
			break;
		case ICMPV6_ADVERT_SERVICE:
			(void)printf("Service Advertizement\n");
			break;
		case ICMPV6_ADVERT_LOCREDIR:
			(void)printf("Local Redirect\n");
			break;
		case ICMPV6_ADVERT_REMREDIR:
			(void)printf("Remote Redirect\n");
			break;
		default:
			(void)printf("Solicit, Bad Code: %d", icmpcode);
			break;
		}
		pr_retip(icmpdata);

		break;
*/
	default:
		(void)printf("Bad ICMPv6 type: %d\n", icmptype);
	}
}

/*
 * pr_iph --
 *	Print an IP (v4 or v6) header with options.
 */
pr_iph(bufp)
	char *bufp;
{
	int hlen;
	u_char *cp;

	if (af == AF_INET) {
	struct ip *ip;
	ip = (struct ip *)bufp;

	hlen = ip->ip_hl << 2;
	cp = (u_char *)ip + sizeof(struct ip);	/* point to options */

	(void)printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n");
	(void)printf(" %1x  %1x  %02x %04x %04x",
	    ip->ip_v, ip->ip_hl, ip->ip_tos, ip->ip_len, ip->ip_id);
	(void)printf("   %1x %04x", ((ip->ip_off) & 0xe000) >> 13,
	    (ip->ip_off) & 0x1fff);
	(void)printf("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ip->ip_sum);
	(void)printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->ip_src.s_addr));
	(void)printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->ip_dst.s_addr));
	/* dump and option bytes */
	while (hlen-- > 20) {
		(void)printf("%02x", *cp++);
	}
	(void)putchar('\n');
        } else if (af == AF_INET6) {
	/* IPv6 header (no options for now) */
	  struct ipv6 *ip6;

	  ip6 = (struct ipv6 *)bufp;
	  (void)printf("Vr FlowLbl  Len Typ Hop Source                              Dest\n");
	  (void)printf(" %1x %07x %4x  %2x %3x %19s %19s\n",
		ntohl(ip6->ipv6_versfl) >> 28,
		ntohl(ip6->ipv6_versfl) & 0x00ffffff,
		ntohl(ip6->ipv6_length),       
		ip6->ipv6_nexthdr, ip6->ipv6_hoplimit,
		addr2ascii(AF_INET6, (char *)&(ip6->ipv6_src), 
		    sizeof(struct in_addr6), NULL),
		addr2ascii(AF_INET6, (char *)&(ip6->ipv6_dst), 
		    sizeof(struct in_addr6), NULL));   
	}
}

/*
 * pr_addr --
 *	Return an ascii host address as a dotted quad and optionally with
 * a hostname.
 */
char *
pr_addr(ap)
	char *ap;
{
	struct hostent *hp;
	static char buf[80];
	int addrsize;

	addrsize = (af == AF_INET) ? sizeof(struct in_addr) : 
	  sizeof(struct in_addr6);

	if ((options & F_NUMERIC) ||
	    !(hp = (struct hostent *)addr2hostname(ap, addrsize, af)))
		(void)sprintf(buf, "%s", addr2ascii(af, ap, addrsize, NULL));
	else
		(void)sprintf(buf, "%s (%s)", hp->h_name,
		    addr2ascii(af, ap, addrsize, NULL));
	return(buf);
}

/*
 * pr_retip --
 *	Dump some info on a returned (via ICMP) IP packet.
 */
pr_retip(cp)
	char *cp;
{
	int proto;

	pr_iph(cp);

	if (af == AF_INET) {
	  proto = ((struct ip *)cp)->ip_p;
	  cp += ((struct ip *)cp)->ip_hl << 2;
	} else {
	  int size;
	 
	  /* Parse packet for transport layer data */
	  proto = ((struct ipv6 *)cp)->ipv6_nexthdr;
	  size = sizeof(struct ipv6);

	  do {
	    cp += size;
	    switch(proto) {
		case NEXTHDR_HOP:
		  /* punt for now*/
		  size = ((struct ipv6_opthdr *)cp)->oh_nexthdr + 8;
		  proto = ((struct ipv6_opthdr *)cp)->oh_nexthdr;
		  break;
		case NEXTHDR_AUTH:
		  size = ((struct ipv6_auth *)cp)->auth_datalen * 8 + 8;
		  proto = ((struct ipv6_auth *)cp)->auth_nexthdr;
		  break;
		case NEXTHDR_ROUTING:
		  if (((struct ipv6_srcroute0 *)cp)->i6sr_type == 0) {
		    size = ((struct ipv6_srcroute0 *)cp)->i6sr_numaddrs *
		      sizeof(struct in_addr6) + 8;
		    proto = ((struct ipv6_srcroute0 *)cp)->i6sr_nexthdr;
		  } else {
		    (void)fprintf(stderr, "ping: ipv6 routing type unknown\n");
		    return;
		  }
		  break;	      
		case NEXTHDR_FRAGMENT:
		  size = 8;
		  proto = ((struct ipv6_fraghdr *)cp)->frag_nexthdr;
		  break;
		default:
		  size = -1;
		  break;	    
		}
	    } while (size > 0);	
	    
	}

	if (proto == 6)
		(void)printf("TCP: from port %u, to port %u (decimal)\n",
		    (*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
	else if (proto == 17)
		(void)printf("UDP: from port %u, to port %u (decimal)\n",
			(*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
}

fill(bp, patp)
	char *bp, *patp;
{
	register int ii, jj, kk;
	int pat[16];
	char *cp;

	for (cp = patp; *cp; cp++)
		if (!isxdigit(*cp)) {
			(void)fprintf(stderr,
			    "ping: patterns must be specified as hex digits.\n");
			exit(1);
		}
	ii = sscanf(patp,
	    "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
	    &pat[0], &pat[1], &pat[2], &pat[3], &pat[4], &pat[5], &pat[6],
	    &pat[7], &pat[8], &pat[9], &pat[10], &pat[11], &pat[12],
	    &pat[13], &pat[14], &pat[15]);

	if (ii > 0)
		for (kk = 0;
		    kk <= MAXPACKET - (8 + sizeof(struct timeval) + ii);
		    kk += ii)
			for (jj = 0; jj < ii; ++jj)
				bp[jj + kk] = pat[jj];
	if (!(options & F_QUIET)) {
		(void)printf("PATTERN: 0x");
		for (jj = 0; jj < ii; ++jj)
			(void)printf("%02x", bp[jj] & 0xFF);
		(void)printf("\n");
	}
}

usage()
{
	(void)fprintf(stderr,
	    "usage: ping [-Rdfnqrv] [-c count] [-i wait] [-l preload]\n\t[-p pattern] [-s packetsize] host\n");
	exit(1);
}
