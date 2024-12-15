/* TCPDUMP 4 IPv6 */
/*
 * print-ipv6.c  --  Prints IPv6 information for tcpdump.
 *
 * Originally from 4.4-Lite BSD.  Modifications to support IPv6 and IPsec
 * are copyright 1995 by Ken Chin, Bao Phan, Randall Atkinson, & Dan McDonald,
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
static char rcsid[] =
    "@(#) /master/usr.sbin/tcpdump/tcpdump/print-ip.c,v 2.1 1995/02/03 18:14:45 polk Exp (LBL)";
#endif

#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

#include <netdb.h>
#include <netinet6/in6.h>
#include <netinet6/in6_types.h>
#include <netinet6/ipsec.h>
#include <netinet6/ipsec_ah.h>
#include <netinet6/ipsec_esp.h>
#include <netinet6/ipv6.h>

#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include <unistd.h>

#include "interface.h"
#include "addrtoname.h"

static void hop_print(const u_char * bp)
{
	printf("hop");
}

static void frag_print(struct ipv6_fraghdr *ip)
{
  u_int16 bitoffset;

	printf("frag[%d", ip->frag_reserved);
	bitoffset = ntohs(ip->frag_bitsoffset);
	printf(" %d", bitoffset & 0xfff8);
	printf(" %d", bitoffset & 0x1);
	printf(" %u] ", ntohl(ip->frag_id));

}

void auth_print(struct ipv6_auth *ip)
{
	printf("auth[%d", ip->auth_datalen);
	printf(" %d", ip->reserved);
	printf(" 0x%x] ", ntohl(ip->auth_spi));

}

static void route_print(struct ipv6_srcroute0 *ip)
{
	printf(" %d", ip->i6sr_type);
	if (!&ip->i6sr_type)  {
		printf(" %d", ip->i6sr_numaddrs);
	}
}

static void dest_print(struct ipv6_opthdr *ip)
{
	printf(" %d", ip-> oh_extlen);
}

void esp_print(char *ip)
{
	printf(" esp 0x%x", ntohl(*(u_int32 *)ip));
}

void ipv6_print (register const u_char *bp, register int length)
{
	char buf[80];
	register const struct ipv6 *ip;
	u_int8 nhdr;
	int first = 1, len=0;
	char *cp;
	u_int32 p;	
	struct hostent *name;

	ip= (const struct ipv6 *) bp;
	if (length < sizeof(struct ipv6)){
	  (void)printf("truncated-ipv6 %d", length);
	  return;
	}
	
	len = sizeof(struct ipv6) + ntohs(ip->ipv6_length);

	if (length < len)
	  (void)printf("truncated-ipv6 - %d bytes missing!",
		       len - length);

	len = sizeof(struct ipv6);

	if (((name=(struct hostent *)addr2hostname(&(ip->ipv6_src), sizeof(struct in_addr6), AF_INET6)) == NULL) || (nflag)){
		(void) addr2ascii(AF_INET6, &(ip->ipv6_src), sizeof (struct in_addr6), &buf);
		printf(" %s", buf);               /* print source address */
	} else {
		printf(" %s", name->h_name);		
	}
	
	if (((name=(struct hostent *)addr2hostname(&(ip->ipv6_dst), sizeof(struct in_addr6), AF_INET6)) == NULL) || (nflag)) {
		(void) addr2ascii(AF_INET6, &(ip->ipv6_dst), sizeof (struct in_addr6), &buf);
	printf(" %s", buf);		  /* print destination address */
	} else {
		printf(" %s", name->h_name);
	}
	p = ntohl(ip->ipv6_versfl);
	printf(" %d", p>>28); /* print version */
	printf(" %d", (p&0x0f000000)>>24);  /* print priority value */
	printf(" %d", p&0x00ffffff);	  /* print flow label */
	printf(" %d", ntohs(ip->ipv6_length));    /* print payload length */
	printf(" %d ", ip->ipv6_hoplimit);
	
	nhdr = ip->ipv6_nexthdr;
	cp = (char *)(ip + 1);

	while ((nhdr!=NEXTHDR_NONE)&&(nhdr!=NEXTHDR_TCP)&&(nhdr!=NEXTHDR_UDP)&&(nhdr!=NEXTHDR_ICMPV6))
		switch (nhdr) {

		case NEXTHDR_HOP:	/* hop by hop (0)*/
			len += sizeof(ip);
			if (len<snaplen)
				hop_print(cp);
			else
				printf(" auth-hdr");
			break;

		case NEXTHDR_ROUTING:	/* routing header (43)*/
			len += sizeof (struct ipv6_srcroute0 );
			if ((len<snaplen)&&(vflag))
				route_print((struct ipv6_srcroute0 *)cp);
			else
				printf(" route-hdr");
			nhdr = ((struct ipv6_srcroute0 *)cp)->i6sr_nexthdr;
			break;

		case NEXTHDR_FRAGMENT:	/* fragment header (44)*/
			len += sizeof (struct ipv6_fraghdr);
			if ((len<snaplen)&&(vflag))
				frag_print((struct ipv6_fraghdr *)cp);
			else
				printf(" frag ");
			nhdr = ((struct ipv6_fraghdr *) cp)->frag_nexthdr;
			printf("nhdr=");
			switch (nhdr)  {
			case NEXTHDR_HOP:
				printf("hop");
				break;
			case NEXTHDR_IGMP:
				printf("igmp");
				break;
			case NEXTHDR_TCP:
				printf("tcp");
				break;
			case NEXTHDR_UDP:
				printf("udp");
				break;
			case NEXTHDR_IPV6:
				printf("ipv6");
				break;
			case NEXTHDR_ROUTING:
				printf("routing");
				break;
			case NEXTHDR_FRAGMENT:
				printf("frag");
				break;
			case NEXTHDR_ESP:
				printf("esp");
				break;
			case NEXTHDR_AUTH:
				printf("auth");
				break;
			case NEXTHDR_ICMPV6:
				printf("icmpv6");
				break;
			case NEXTHDR_NONE: 
				printf("no next");
				break;
			case NEXTHDR_DEST:
				printf("dest");
				break;
			default:
				printf("%d", nhdr);
			}
			return;
		case NEXTHDR_ESP:	/* encapsulation header (50)*/
			len += sizeof (u_int32);
			if ((len<snaplen)&&(vflag))
				esp_print((char *)cp);
			else
				printf(" esp-hdr");
			return;

		case NEXTHDR_AUTH:	/* authentication header (51)*/
			len += sizeof (struct ipv6_auth) + sizeof(u_int32) * ((struct ipv6_auth *)cp)->auth_datalen;
			if ((len<snaplen)&&(vflag))
				auth_print((struct ipv6_auth *)cp);
			else
				printf(" auth-hdr");
			nhdr = ((struct ipv6_auth *)cp)->auth_nexthdr;
			cp += ((struct ipv6_auth *)cp)->auth_datalen * sizeof (u_int32)+ sizeof(struct ipv6_auth);
			break;

		case NEXTHDR_DEST:	/* destination options header (60)*/
			len += sizeof (struct ipv6_opthdr);
			if ((len<snaplen)&&(vflag))
				dest_print((struct ipv6_opthdr *)cp);
			else
				printf(" dest-hdr");
			nhdr = ((struct ipv6_opthdr *)cp)->oh_nexthdr;
			cp += sizeof(struct ipv6_opthdr);
			break;
		default:
			printf("unknown nexthdr %d", nhdr);
			return;

	      }
	len = length - len;
	switch (nhdr) {
		case NEXTHDR_TCP:
			tcp_print(cp, len, (const u_char *)ip);
                        break; 
		case NEXTHDR_UDP:
			udp_print(cp, len, (const u_char *)ip);
                        break; 
		case NEXTHDR_ICMPV6:
			icmpv6_print(cp, len);
                        break;
		default:
			printf("unknown transport protocol");
			return;
	}
}

