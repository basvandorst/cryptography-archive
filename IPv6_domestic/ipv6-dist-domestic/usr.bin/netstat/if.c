/*
 * if.c  --  Interface information for netstat.
 *
 * Modifications to support IPv6 are copyright 1995 
 * by Randall Atkinson, Dan McDonald, and Bao Phan, All Rights Reserved.  
 * All Rights under this copyright have been assigned to NRL.
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
 * Copyright (c) 1983, 1988, 1993
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
static char sccsid[] = "@(#)if.c	8.2 (Berkeley) 2/21/94";
#endif /* not lint */

#include <sys/types.h>
#include <sys/protosw.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netns/ns.h>
#include <netns/ns_if.h>
#include <netiso/iso.h>
#include <netiso/iso_var.h>
#include <arpa/inet.h>
#include <netinet6/in6.h>
#include <netinet6/in6_var.h>

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "netstat.h"

#define	YES	1
#define	NO	0

static void sidewaysintpr __P((u_int, u_long));
static void catchalarm __P((int));

/*
 * Print a description of the network interfaces.
 */
void
intpr(interval, ifnetaddr)
	int interval;
	u_long ifnetaddr;
{
	struct ifnet ifnet;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
		struct in6_ifaddr in6;
		struct ns_ifaddr ns;
		struct iso_ifaddr iso;
	} ifaddr;
	u_long ifaddraddr, ifnetcurrent ;
	struct sockaddr *sa;
	char name[16];

	if (ifnetaddr == 0) {
		printf("ifnet: symbol not defined\n");
		return;
	}
	if (interval) {
		sidewaysintpr((unsigned)interval, ifnetaddr);
		return;
	}
	if (kread(ifnetaddr, (char *)&ifnetaddr, sizeof ifnetaddr))
		return;
	printf("%-5.5s %-5.5s %-15.15s %-15.15s %8.8s %5.5s %8.8s %5.5s",
		"Name", "Mtu", "Network", "Address", "Ipkts", "Ierrs",
		"Opkts", "Oerrs");
	printf(" %5s", "Coll");
	if (tflag)
		printf(" %s", "Time");
	if (dflag)
		printf(" %s", "Drop");
	putchar('\n');
	ifaddraddr = 0;
/*	ifnetcurrent = 0; */
	while (ifnetaddr || ifaddraddr) {
		struct sockaddr_in *sin;
		register char *cp;
		int n, m;

/*		ifnetcurrent = ifnetaddr; */
		if (ifaddraddr == 0) {
			if (kread(ifnetaddr, (char *)&ifnet, sizeof ifnet) ||
			    kread((u_long)ifnet.if_name, name, 16))
				return;
			name[15] = '\0';
			ifnetaddr = (u_long)ifnet.if_next;
			if (interface != 0 && (strcmp(name, interface) != 0 ||
			    unit != ifnet.if_unit))
				continue;
			cp = index(name, '\0');
			cp += sprintf(cp, "%d", ifnet.if_unit);
			if ((ifnet.if_flags&IFF_UP) == 0)
				*cp++ = '*';
			*cp = '\0';
			ifaddraddr = (u_long)ifnet.if_addrlist;
		}
		printf("%-5.5s %-5d ", name, ifnet.if_mtu);
		if (ifaddraddr == 0) {
			printf("%-11.11s ", "none");
			printf("%-15.15s ", "none");
		} else {
			if (kread(ifaddraddr, (char *)&ifaddr, sizeof ifaddr)) {
				ifaddraddr = 0;
				continue;
			}
#define CP(x) ((char *)(x))
			cp = (CP(ifaddr.ifa.ifa_addr) - CP(ifaddraddr)) +
				CP(&ifaddr); sa = (struct sockaddr *)cp;
			switch (sa->sa_family) {
			case AF_UNSPEC:
				printf("%-11.11s ", "none");
				printf("%-15.15s ", "none");
				break;
			case AF_INET:
				{	
				u_long addr;
				sin = (struct sockaddr_in *)sa;
				addr = htonl(ifaddr.in.ia_subnet);
#ifdef old
				printf("%-11.11s ", netname((char *)&addr, 
				    AF_INET));
#else
				printf("%-15.15s ", netname((char *)&addr, 
				    AF_INET));
#endif
				printf("%-15.15s ",
				    routename((char *)&(sin->sin_addr.s_addr),
				    AF_INET));
			        }
				break;
		        case AF_INET6:
				{
				struct sockaddr_in6 *ss = 
				    (struct sockaddr_in6 *)sa;  
				int i;
				char addr[sizeof(struct in_addr6)];

				/* Use the network mask to isolate the */
				/* network part of the address         */
				for (i = 0; i < sizeof(struct in_addr6); i++)
				    addr[i] = ((char *) \
				      (&ifaddr.in6.i6a_addr.sin6_addr))[i] & \
				      ((char *) \
				      (&ifaddr.in6.i6a_sockmask.sin6_addr))[i];
#ifdef old
				printf("%-11.11s ", netname(addr, AF_INET6));
				printf("%-15.15s ", \
				    routename((char *)&(ss->sin6_addr), \
				    AF_INET6));
#else
				printf("%-15s ", netname(addr, AF_INET6));
				printf("%-15s ", \
				    routename((char *)&(ss->sin6_addr), \
				    AF_INET6));
#endif

				}
				break;
			case AF_NS:
				{
				struct sockaddr_ns *sns =
					(struct sockaddr_ns *)sa;
				u_long net;
				char netnum[8];

				*(union ns_net *) &net = sns->sns_addr.x_net;
				sprintf(netnum, "%lxH", ntohl(net));
				upHex(netnum);
				printf("ns:%-8s ", netnum);
				printf("%-15s ",
				    ns_phost((struct sockaddr *)sns));
				}
				break;
			case AF_LINK:
				{
				struct sockaddr_dl *sdl =
					(struct sockaddr_dl *)sa;
				    cp = (char *)LLADDR(sdl);
				    n = sdl->sdl_alen;
				}
				m = printf("<Link>");
				goto hexprint;
			default:
				m = printf("(%d)", sa->sa_family);
				for (cp = sa->sa_len + (char *)sa;
					--cp > sa->sa_data && (*cp == 0);) {}
				n = cp - sa->sa_data + 1;
				cp = sa->sa_data;
			hexprint:
				while (--n >= 0)
					m += printf("%x%c", *cp++ & 0xff,
						    n > 0 ? '.' : ' ');
#ifdef old
				m = 28 - m;
#else
				m = 32 - m;
#endif
				while (m-- > 0)
					putchar(' ');
				break;
			}
			ifaddraddr = (u_long)ifaddr.ifa.ifa_next;
		}
		printf("%8d %5d %8d %5d %5d",
		    ifnet.if_ipackets, ifnet.if_ierrors,
		    ifnet.if_opackets, ifnet.if_oerrors,
		    ifnet.if_collisions);
		if (tflag)
			printf(" %3d", ifnet.if_timer);
		if (dflag)
			printf(" %3d", ifnet.if_snd.ifq_drops);
		putchar('\n');

		/* Where the hell is the multicast address printing code? */
		/* i.e, netstat -ia                                       */
	}
}

#define	MAXIF	10
struct	iftot {
	char	ift_name[16];		/* interface name */
	int	ift_ip;			/* input packets */
	int	ift_ie;			/* input errors */
	int	ift_op;			/* output packets */
	int	ift_oe;			/* output errors */
	int	ift_co;			/* collisions */
	int	ift_dr;			/* drops */
} iftot[MAXIF];

u_char	signalled;			/* set if alarm goes off "early" */

/*
 * Print a running summary of interface statistics.
 * Repeat display every interval seconds, showing statistics
 * collected over that interval.  Assumes that interval is non-zero.
 * First line printed at top of screen is always cumulative.
 */
static void
sidewaysintpr(interval, off)
	unsigned interval;
	u_long off;
{
	struct ifnet ifnet;
	u_long firstifnet;
	register struct iftot *ip, *total;
	register int line;
	struct iftot *lastif, *sum, *interesting;
	int oldmask;

	if (kread(off, (char *)&firstifnet, sizeof (firstifnet)))
		return;
	lastif = iftot;
	sum = iftot + MAXIF - 1;
	total = sum - 1;
	interesting = iftot;
	for (off = firstifnet, ip = iftot; off;) {
		char *cp;

		if (kread(off, (char *)&ifnet, sizeof ifnet))
			break;
		ip->ift_name[0] = '(';
		if (kread((u_long)ifnet.if_name, ip->ift_name + 1, 15))
			break;
		if (interface && strcmp(ip->ift_name + 1, interface) == 0 &&
		    unit == ifnet.if_unit)
			interesting = ip;
		ip->ift_name[15] = '\0';
		cp = index(ip->ift_name, '\0');
		sprintf(cp, "%d)", ifnet.if_unit);
		ip++;
		if (ip >= iftot + MAXIF - 2)
			break;
		off = (u_long) ifnet.if_next;
	}
	lastif = ip;

	(void)signal(SIGALRM, catchalarm);
	signalled = NO;
	(void)alarm(interval);
banner:
	printf("   input    %-6.6s    output       ", interesting->ift_name);
	if (lastif - iftot > 0) {
		if (dflag)
			printf("      ");
		printf("     input   (Total)    output");
	}
	for (ip = iftot; ip < iftot + MAXIF; ip++) {
		ip->ift_ip = 0;
		ip->ift_ie = 0;
		ip->ift_op = 0;
		ip->ift_oe = 0;
		ip->ift_co = 0;
		ip->ift_dr = 0;
	}
	putchar('\n');
	printf("%8.8s %5.5s %8.8s %5.5s %5.5s ",
		"packets", "errs", "packets", "errs", "colls");
	if (dflag)
		printf("%5.5s ", "drops");
	if (lastif - iftot > 0)
		printf(" %8.8s %5.5s %8.8s %5.5s %5.5s",
			"packets", "errs", "packets", "errs", "colls");
	if (dflag)
		printf(" %5.5s", "drops");
	putchar('\n');
	fflush(stdout);
	line = 0;
loop:
	sum->ift_ip = 0;
	sum->ift_ie = 0;
	sum->ift_op = 0;
	sum->ift_oe = 0;
	sum->ift_co = 0;
	sum->ift_dr = 0;
	for (off = firstifnet, ip = iftot; off && ip < lastif; ip++) {
		if (kread(off, (char *)&ifnet, sizeof ifnet)) {
			off = 0;
			continue;
		}
		if (ip == interesting) {
			printf("%8d %5d %8d %5d %5d",
				ifnet.if_ipackets - ip->ift_ip,
				ifnet.if_ierrors - ip->ift_ie,
				ifnet.if_opackets - ip->ift_op,
				ifnet.if_oerrors - ip->ift_oe,
				ifnet.if_collisions - ip->ift_co);
			if (dflag)
				printf(" %5d",
				    ifnet.if_snd.ifq_drops - ip->ift_dr);
		}
		ip->ift_ip = ifnet.if_ipackets;
		ip->ift_ie = ifnet.if_ierrors;
		ip->ift_op = ifnet.if_opackets;
		ip->ift_oe = ifnet.if_oerrors;
		ip->ift_co = ifnet.if_collisions;
		ip->ift_dr = ifnet.if_snd.ifq_drops;
		sum->ift_ip += ip->ift_ip;
		sum->ift_ie += ip->ift_ie;
		sum->ift_op += ip->ift_op;
		sum->ift_oe += ip->ift_oe;
		sum->ift_co += ip->ift_co;
		sum->ift_dr += ip->ift_dr;
		off = (u_long) ifnet.if_next;
	}
	if (lastif - iftot > 0) {
		printf("  %8d %5d %8d %5d %5d",
			sum->ift_ip - total->ift_ip,
			sum->ift_ie - total->ift_ie,
			sum->ift_op - total->ift_op,
			sum->ift_oe - total->ift_oe,
			sum->ift_co - total->ift_co);
		if (dflag)
			printf(" %5d", sum->ift_dr - total->ift_dr);
	}
	*total = *sum;
	putchar('\n');
	fflush(stdout);
	line++;
	oldmask = sigblock(sigmask(SIGALRM));
	if (! signalled) {
		sigpause(0);
	}
	sigsetmask(oldmask);
	signalled = NO;
	(void)alarm(interval);
	if (line == 21)
		goto banner;
	goto loop;
	/*NOTREACHED*/
}

/*
 * Called if an interval expires before sidewaysintpr has completed a loop.
 * Sets a flag to not wait for the alarm.
 */
static void
catchalarm(signo)
	int signo;
{
	signalled = YES;
}
