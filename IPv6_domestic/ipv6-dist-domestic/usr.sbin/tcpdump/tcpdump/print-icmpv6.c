/*
 * print-icmpv6.c  --  Display ICMPv6 data.
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
 * Copyright (c) 1988, 1989, 1990, 1991, 1993, 1994
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
    "@(#) /master/usr.sbin/tcpdump/tcpdump/print-icmp.c,v 2.1 1995/02/03 18:14:42 polk Exp (LBL)";
#endif

#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

#include <netinet6/ipv6.h>
#include <netinet6/in6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h>
#include <stdio.h>

#include <string.h>

#include "interface.h"
#include "addrtoname.h"

struct nbr_ad 
        {
                u_int8 type;
                u_int8 code;
                u_int16 chksum;
                u_int32 flags;
		struct in_addr6 targ_addr;
        };

struct nbr_sol
{
  u_int8 type;
  u_int8 code;
  u_int16 cksum;
  u_int32 reserved;
  struct in_addr6 sender;
  struct in_addr6 target;
};

struct na_opt
	{
		u_int8 type;
		u_int8 length;
		u_int8 data[6];
	};
		
void na_opt_print(struct na_opt *na, char *s)
{
	char *addr;
	int i;
	char buf[256];

	switch(na->type)  {
	case 1:
		s = strcat(s, " src[");		
		break;
	case 2:
		s = strcat(s, " targ[");
		break;
	}
	for (i=0; i< (na->length * 8)-2; i++) {
		sprintf(buf,"%x", na->data[i]);
		if ( i == (na->length*8)-3 )  { 
			s = strcat(s,(char *)buf);
			s = strcat(s, "]");
		}  else  {
			s = strcat(s,(char*)buf);
			s = strcat(s,":");
		}
	}
}

void
icmpv6_print(register const u_char *bp, int length)
{
	register const struct ipv6_icmp *dp;
	register char *str;
	register const struct ipv6 *oip;
	register const struct udphdr *ouh;
	char buf[256];
	char src[80];
	char dst[80];
	char *cp;
	struct nbr_ad *na;
	struct nbr_sol *ns;
	struct na_opt *opt;
	char *targ_addr;
	int na_len, left;

	dp = (struct ipv6_icmp *)bp;
	left = (u_int)snapend - (u_int)bp;

	str = buf;
	*str = '\0';

	if (left < ICMPV6_MINLEN)
	  goto trunc;

	switch (dp->icmp_type) {
	case ICMPV6_ECHOREPLY:
		str = "echo reply";
		break;
	case ICMPV6_UNREACH:
		switch (dp->icmp_code) {
		case ICMPV6_UNREACH_NOROUTE:
			str = "no route";
			break;
		case ICMPV6_UNREACH_ADMIN:
			str = "admin prohibited";
			break;
		case ICMPV6_UNREACH_NOTNEIGHBOR:
			str ="not a neighbor";
			break;
		case ICMPV6_UNREACH_ADDRESS:
			str = "host unreachable";
 			break;
/*		case ICMPV6_UNREACH_PROTOCOL:
			str = "protocol unreachable";
			break;*/
		case ICMPV6_UNREACH_PORT:
			str = "port unreachable";
			break;
		}
		break;
	case ICMPV6_TOOBIG:
		sprintf(str, "packet too big: mtu[%d]", dp->icmp_nexthopmtu);
		break;
	case ICMPV6_ECHO:
		str = "echo request";
		break;
	case ICMPV6_GRPQUERY:
		str = "query group membership";
		break;
	case ICMPV6_GRPREPORT:
		str = "join mcast group";
		break;
	case ICMPV6_GRPTERM:
		str = "leave mcast group";
		break;
	case ICMPV6_TIMXCEED:
		switch (dp->icmp_code) {
		case ICMPV6_TIMXCEED_INTRANS:
			str = "time exceeded in-transit";
			break;
		case ICMPV6_TIMXCEED_REASS:
			str = "ip reassembly time exceeded";
			break;
		}
		break;
	case ICMPV6_PARAMPROB:
		switch(dp->icmp_code) {
		case ICMPV6_PARAMPROB_PROB:
		  strcpy(str,"incorrect parameter");
		  break;
		case ICMPV6_PARAMPROB_NEXTHDR:
		  strcpy(str,"bad next header");
		  break;
		case ICMPV6_PARAMPROB_BADOPT:
		  strcpy(str,"unrec. option");
		  break;
		}
		sprintf(str+strlen(str), " ptr[%x]", dp->icmp_paramptr);
		break;
	case ICMPV6_ROUTERSOL:
		str = "router solicit";
		break;
	case ICMPV6_ROUTERADV:
		str = "router ad";
		break;
	case ICMPV6_NEIGHBORSOL:
		if (left < sizeof(struct nbr_sol))
		  goto trunc;
                str = strcat(str, "neigh sol ");
		ns = (struct nbr_sol *)bp;
                na_len = sizeof (struct nbr_sol);
		if (vflag)
		  {
		    str = strcat(str,"from ");
		    (void) addr2ascii(AF_INET6, (char *)&(ns->sender),
				      sizeof(struct in_addr6), &targ_addr);
		    str = strcat(str, (char *)&targ_addr);
		    str = strcat(str, " for ");
		    (void) addr2ascii(AF_INET6, (char *)&(ns->target),
				      sizeof(struct in_addr6), &targ_addr);
		    str = strcat(str, (char *)&targ_addr);
		  }
                ns++;   
                cp = (char *)ns;
                opt = (struct na_opt *)ns;
		if (vflag)
		  while (((na_len + opt->length * 8) <= left) && opt->length) {
                        na_opt_print((struct na_opt *)opt, str);
                        cp += opt->length;
                        na_len += (opt->length * 8);
                        opt = (struct na_opt *)cp;
		      }
		break;
	case ICMPV6_NEIGHBORADV:
		if (left < sizeof(struct nbr_ad))
		  goto trunc;
		str = strcat(str, "neigh adv ");
		na = (struct nbr_ad *)bp;
		na_len = sizeof (struct nbr_ad);
		str = strcat(str, "(");
		/*
		 * Flags are in network order, but so are bitmasks.
		 * (See netinet6/ipv6_icmp.h for details.)
		 */
		if (na->flags & ICMPV6_NEIGHBORADV_RTR)
		  str = strcat(str, "R");
		if (na->flags & ICMPV6_NEIGHBORADV_SOL)
		  str = strcat(str, "S");
		if (na->flags & ICMPV6_NEIGHBORADV_SECOND)
		  str = strcat(str, "N");
		str = strcat(str, ") ");
		if (vflag)  {
		  str = strcat(str, "for ");
		  (void) addr2ascii(AF_INET6, (char *)&(na->targ_addr),
				    sizeof(struct in_addr6), &targ_addr);
		  str = strcat(str, (char *)&targ_addr);
		}
		na++;
		cp = (char *)na; 
		opt = (struct na_opt *)na;
		if (vflag)
		while (((na_len + opt->length*8) <= left) && opt->length)  {
			na_opt_print((struct na_opt *)opt, str);
			cp += opt->length;
			na_len += (opt->length * 8);
			opt = (struct na_opt *)cp;
		}
		break;
	default:
		(void)sprintf(buf, "type#%d", dp->icmp_type);
		break;
	}
        (void)printf("icmpv6: %s", str);
	return;
trunc:
	fputs("[|icmpv6]", stdout);
#undef TCHECK
}

