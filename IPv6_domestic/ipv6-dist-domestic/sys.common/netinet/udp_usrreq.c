/*
 * udp_usrreq.c  --  All UDP routines.  
 *                   Includes mods for UDP to run over IPv6.
 *
 * Modifications to support IPv6 are copyright 1995 
 * by Bao Phan, Dan McDonald, and Randall Atkinson, All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 */

/*----------------------------------------------------------------------
#       @(#)COPYRIGHT   1.1 (NRL) 17 January 1995

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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993
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
 *
 *	@(#)udp_usrreq.c	8.4 (Berkeley) 1/21/94
 */

#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>

#ifdef INET6
#include <netinet6/in6.h>
#include <netinet6/ipv6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h>
#include <netinet6/in6_debug.h>
#include <netinet6/ipsec.h>
#include <netinet6/key.h>
#include <netinet6/ipsec_policy.h>
#endif /* INET6 */

#include <netinet/udp.h>
#include <netinet/udp_var.h>

/*
 * UDP protocol implementation.
 * Per RFC 768, August, 1980.
 */

#ifndef	COMPAT_42
int	udpcksum = 1;
#else
int	udpcksum = 0;		/* XXX */
#endif

char udp_in[24];
struct	inpcb *udp_last_inpcb = &udb;
#ifdef INET6
extern int ipv6_defhoplmt;
#endif /* INET6 */

static	void udp_detach __P((struct inpcb *));
static	void udp_notify __P((struct inpcb *, int));
static	struct mbuf *udp_saveopt __P((caddr_t, int, int));

void
udp_init()
{
	udb.inp_next = udb.inp_prev = &udb;
}

void
udp_input(m, iphlen
#ifdef INET6
	  , ihi, ihioff
#endif
	  )
     register struct mbuf *m;
     int iphlen;
#ifdef INET6
     /*
      * Extra args ihi and ihioff are header indices and which one is the
      * UDP header.  When ipv6_preparse is eliminated or severely optimized,
      * these might change as well.  In UDP, they're here mostly for
      * ipv6_stripoptions().
      */
     struct in6_hdrindex *ihi;
     int ihioff;
#endif
{
	register struct ip *ip;
#ifdef INET6
	register struct ipv6 *ip6;
	int v6;
	struct sockaddr_in6 *sa6;
#endif /* INET6 */
	register struct udphdr *uh;
	register struct inpcb *inp;
	struct mbuf *opts = 0;
	int len;
	struct ip save_ip;
	struct sockaddr_in *sa;

#ifdef INET6
	DPRINTF(IDL_GROSS_EVENT,("Entering udp_input() with iphlen=%d\n",\
				 iphlen));
#endif /* INET6 */

	udpstat.udps_ipackets++;

	/*
	 * Strip IP options, if any; should skip this,
	 * make available to user, and use on returned packets,
	 * but we don't yet have a way to check the checksum
	 * with options still present.
	 */
#ifdef INET6

	/*
	 * (contd. from above...)  Furthermore, we may want to strip options
	 * for such things as ICMP errors, where options just get in the way.
	 */

	ip6 = mtod(m, struct ipv6 *);
	ip = mtod(m, struct ip *);
	v6 = (ip->ip_v == 6);
        DPRINTF(IDL_FINISHED, ("v6 == %d\n", v6));
	if (v6) {
	  DPRINTF(IDL_GROSS_EVENT,("udp_input() got a v6 datagram\n"));
	  DDO(IDL_GROSS_EVENT,printf("mbuf is:\n");dump_mbuf(m));
	  if (iphlen > sizeof (struct ipv6)) {
	    DPRINTF(IDL_GROSS_EVENT,("Calling ipv6_stripoptions()\n"));
	        ipv6_stripoptions(m, ihi[ihioff - 1].ihi_mbuf,
			ihi[ihioff].ihi_nexthdr);
		iphlen = sizeof(struct ipv6);
	  }
	} else 
#endif /* INET6 */
	if (iphlen > sizeof (struct ip)) {
		ip_stripoptions(m, (struct mbuf *)0);
		iphlen = sizeof(struct ip);
	}

	/*
	 * Get IP and UDP header together in first mbuf.
	 */
#ifndef INET6
	ip = mtod(m, struct ip *);
#endif /* INET6 */
	if (m->m_len < iphlen + sizeof(struct udphdr)) {
		if ((m = m_pullup(m, iphlen + sizeof(struct udphdr))) == 0) {
			udpstat.udps_hdrops++;
			return;
		}
		ip = mtod(m, struct ip *);
#ifdef INET6
		/*
		 * NOTE:  With ipv6_preparse() in place, this fragment will
		 *        (sigh) always execute.
		 */

		DDO(IDL_FINISHED,printf("mbuf after m_pullup(): \n");\
		    dump_mchain(m));
		ip6 = mtod(m, struct ipv6 *);
#endif /* INET6 */
        }

#ifdef INET6
	uh = v6 ? (struct udphdr *)((caddr_t)ip6 + iphlen) :
	          (struct udphdr *)((caddr_t)ip + iphlen);
#else
	uh = (struct udphdr *)((caddr_t)ip + iphlen);
#endif /* INET6 */

	/*
	 * Make mbuf data length reflect UDP length.
	 * If not enough data to reflect UDP length, drop.
	 */
	len = ntohs((u_short)uh->uh_ulen);

/*
 *  Introduced a bug fix proposed by Dave Borman on the IRTF's 
 *  end2end-interest list around 8-17-95.  (See the 8-17 messages in the
 *  list archives.)  The bug involves the UDP length field being less 
 *  than the size of the UDP header.
 */
#ifdef INET6 
	if ((v6 ? (m->m_pkthdr.len - sizeof(struct ipv6)) : ip->ip_len) != len) {
	        DPRINTF(IDL_MAJOR_EVENT,\
			("Warning..ipv4/6 len conflicts with udp len!\n"));
	        if (len > (v6 ? (m->m_pkthdr.len - sizeof(struct ipv6))
			: ip->ip_len) || len < sizeof(struct udphdr)) {
#else
	if (ip->ip_len != len) {
		if (len > ip->ip_len || len < sizeof(struct udphdr)) {
#endif /* INET6 */
			udpstat.udps_badlen++;
			goto bad;
		}
#ifdef INET6
		m_adj(m, len - (v6 ? (m->m_pkthdr.len - sizeof(struct ipv6)) :
			ip->ip_len));
#else
		m_adj(m, len - ip->ip_len);
#endif /* INET6 */
		/* ip->ip_len = len; */
	}

	/*
	 * Save a copy of the IP header in case we want to restore it
	 * for sending an ICMP error message in response.
	 * (For IPv4 only...)
	 */
	save_ip = *ip;

	/*
	 * Checksum extended UDP header and data.
	 */

#ifdef INET6
	if (v6) {
	        /*
		 * In IPv6, the UDP checksum is ALWAYS used.
		 */
	        DPRINTF(IDL_FINISHED,("Calling in6_cksum()\n"));
	        if (uh->uh_sum = in6_cksum(m, IPPROTO_UDP, len, iphlen)) {
		        udpstat.udps_badsum++;
			goto bad;
		}
	} else
#endif /* INET6 */
	if (udpcksum && uh->uh_sum) {
		((struct ipovly *)ip)->ih_next = 0;
		((struct ipovly *)ip)->ih_prev = 0;
		((struct ipovly *)ip)->ih_x1 = 0;
		((struct ipovly *)ip)->ih_len = uh->uh_ulen;
		if (uh->uh_sum = in_cksum(m, len + sizeof (struct ip))) {
			udpstat.udps_badsum++;
			m_freem(m);
			return;
		}
	 }

#ifdef INET6
	if ((v6 && IS_IN6_MULTICAST(ip6->ipv6_dst)) ||
	    (!v6 && IN_MULTICAST(ntohl(ip->ip_dst.s_addr))) ||
	    (!v6 && in_broadcast(ip->ip_dst, m->m_pkthdr.rcvif))) {
		struct sockaddr_in6 v4mapped;
#else
	if (IN_MULTICAST(ntohl(ip->ip_dst.s_addr)) ||
	    in_broadcast(ip->ip_dst, m->m_pkthdr.rcvif)) {
#endif /* INET6 */
		struct socket *last;

		/*
		 * Deliver a multicast or broadcast datagram to *all* sockets
		 * for which the local and remote addresses and ports match
		 * those of the incoming datagram.  This allows more than
		 * one process to receive multi/broadcasts on the same port.
		 * (This really ought to be done for unicast datagrams as
		 * well, but that would cause problems with existing
		 * applications that open both address-specific sockets and
		 * a wildcard socket listening to the same port -- they would
		 * end up receiving duplicates of every unicast datagram.
		 * Those applications open the multiple sockets to overcome an
		 * inadequacy of the UDP socket interface, but for backwards
		 * compatibility we avoid the problem here rather than
		 * fixing the interface.  Maybe 4.5BSD will remedy this?)
		 */

		/*
		 * Construct sockaddr format source address.
		 */
		bzero(&udp_in, sizeof(udp_in));
#ifdef INET6
		if (v6) {
		  sa6 = (struct sockaddr_in6 *)(&udp_in);
		  sa6->sin6_len = sizeof(struct sockaddr_in6);
		  sa6->sin6_family = AF_INET6;
		  sa6->sin6_port = uh->uh_sport;
		  IN6_ADDR_COPY(sa6->sin6_addr, ip6->ipv6_src);
		  m->m_len -= (sizeof(struct ipv6) + sizeof(struct udphdr));
		  m->m_data -= (sizeof(struct ipv6) + sizeof(struct udphdr));
		} else {
		  bzero(&v4mapped,sizeof(v4mapped));
		  v4mapped.sin6_family = AF_INET6;
		  v4mapped.sin6_len = sizeof(v4mapped);
		  CREATE_IPV6_MAPPED(v4mapped.sin6_addr,ip->ip_src.s_addr);
		  v4mapped.sin6_port = uh->uh_sport;
#endif /* INET6 */
		  sa = (struct sockaddr_in *)(&udp_in);
		  sa->sin_len = sizeof(struct sockaddr_in);
		  sa->sin_family = AF_INET;
		  sa->sin_port = uh->uh_sport;
		  sa->sin_addr = ip->ip_src;
		  m->m_len -= sizeof (struct udpiphdr);
		  m->m_data += sizeof (struct udpiphdr); 
#ifdef INET6
		}
#endif /* INET6 */

		/*
		 * Locate pcb(s) for datagram.
		 * (Algorithm copied from raw_intr().)
		 */
		last = NULL;
		for (inp = udb.inp_next; inp != &udb; inp = inp->inp_next) {
			if (inp->inp_lport != uh->uh_dport)
				continue;
#ifdef INET6
			if (v6) {
			  if (!IS_IN6_UNSPEC(inp->inp_laddr6))
			      if (!IN6_ADDR_EQUAL(inp->inp_laddr6,
			          ip6->ipv6_dst))
			          continue;
			} else  
#endif
			if (inp->inp_laddr.s_addr != INADDR_ANY) {
				if (inp->inp_laddr.s_addr !=
				    ip->ip_dst.s_addr)
					continue;
			}
#ifdef INET6
			if (v6) {
			  if (!IS_IN6_UNSPEC(inp->inp_faddr6))
			    if (!IN6_ADDR_EQUAL(inp->inp_faddr6,
				ip6->ipv6_src) ||
				inp->inp_fport != uh->uh_sport)
			        continue;
			} else
#endif /* INET6 */
			if (inp->inp_faddr.s_addr != INADDR_ANY) {
				if (inp->inp_faddr.s_addr !=
				    ip->ip_src.s_addr ||
				    inp->inp_fport != uh->uh_sport)
					continue;
			}

#ifdef INET6
                        if (ipsec_input_policy(m,inp->inp_socket, NEXTHDR_UDP))
			  /* should increment "Received packet failed IPsec
			     input policy check" counter here, but have not
			     yet instrumented IPsec.  rja XXX */
                            continue;
#endif /* INET6 */

			if (last != NULL) {
				struct mbuf *n;

				if ((n = m_copy(m, 0, M_COPYALL)) != NULL) {
				  if (sbappendaddr(&last->so_rcv,
#ifdef INET6							 
    /* This cruft is needed in (the rare) case I deliver a {multi,broad}cast
       IPv4 packet to an AF_INET6 socket. */
    ((((struct inpcb *)last->so_pcb)->inp_flags & INP_IPV6) && !v6)?
						 (struct sockaddr *)&v4mapped:
#endif
						   (struct sockaddr *)&udp_in,
						n, (struct mbuf *)0) == 0) {
						m_freem(n);
						udpstat.udps_fullsock++;
					} else
						sorwakeup(last);
				}
			}
			last = inp->inp_socket;
			/*
			 * Don't look for additional matches if this one does
			 * not have either the SO_REUSEPORT or SO_REUSEADDR
			 * socket options set.  This heuristic avoids searching
			 * through all pcbs in the common case of a non-shared
			 * port.  It * assumes that an application will never
			 * clear these options after setting them.
			 */
			if ((last->so_options&(SO_REUSEPORT|SO_REUSEADDR) == 0))
				break;
		}

		if (last == NULL) {
			/*
			 * No matching pcb found; discard datagram.
			 * (No need to send an ICMP Port Unreachable
			 * for a broadcast or multicast datgram.)
			 */
			udpstat.udps_noportbcast++;
			goto bad;
		}
		if (sbappendaddr(&last->so_rcv,
#ifdef INET6							 
	        /* This cruft is needed in (the rare) case I deliver a
		   {multi,broad}cast IPv4 packet to an AF_INET6 socket. */
	       ((((struct inpcb *)last->so_pcb)->inp_flags & INP_IPV6) && !v6)?
				 (struct sockaddr *)&v4mapped:
#endif
				 (struct sockaddr *)&udp_in,
		     m, (struct mbuf *)0) == 0) {
			udpstat.udps_fullsock++;
			goto bad;
		}
		sorwakeup(last);
		return;
	}
	/*
	 * Locate pcb for datagram.
	 */
	inp = udp_last_inpcb;
#ifdef INET6 
	/* For IPv6, don't bother to search the last matched pcb 
	 * since cache hit is so low.  Need to fix this later.
	 */
	if (v6) {
	    struct in_addr src, dst;

	    src.s_addr = (u_long)&(ip6->ipv6_src);
	    dst.s_addr = (u_long)&(ip6->ipv6_dst);
	    inp = in_pcblookup(&udb, src, uh->uh_sport, dst, uh->uh_dport, 
		INPLOOKUP_WILDCARD | INPLOOKUP_IPV6);
	    if (inp)
	            udp_last_inpcb = inp;     
	    udpstat.udpps_pcbcachemiss++;
	    
	} else
#endif /* INET6 */
	if (inp->inp_lport != uh->uh_dport ||
	    inp->inp_fport != uh->uh_sport ||
	    inp->inp_faddr.s_addr != ip->ip_src.s_addr ||
	    inp->inp_laddr.s_addr != ip->ip_dst.s_addr) {
		inp = in_pcblookup(&udb, ip->ip_src, uh->uh_sport,
		    ip->ip_dst, uh->uh_dport, INPLOOKUP_WILDCARD);
		if (inp)
			udp_last_inpcb = inp;
		udpstat.udpps_pcbcachemiss++;
	}

#ifdef INET6
        if (inp)
            if (ipsec_input_policy(m, inp->inp_socket, NEXTHDR_UDP)) {
	      /* should increment "Received packet failed input IPsec
		 policy check" counter here, but have not yet instrumented
		 IPsec.  rja XXX */
	      goto bad;
	      }
#endif /* INET6 */

	if (inp == 0) {
		udpstat.udps_noport++;
		if (m->m_flags & (M_BCAST | M_MCAST)) {
			udpstat.udps_noportbcast++;
			goto bad;
		}
#ifdef INET6
		if (v6) {
		  DPRINTF(IDL_MAJOR_EVENT,("Calling ipv6_icmp_error\n"));
		  ipv6_icmp_error(m, ICMPV6_UNREACH,ICMPV6_UNREACH_PORT,0);
		} else {
#endif /* INET6 */
		*ip = save_ip;
		ip->ip_len += iphlen;  
		icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_PORT, 0, 0);
#ifdef INET6
	         }
#endif /* INET6 */
		return;
	}

	/*
	 * Construct sockaddr format source address.
	 * Stuff source address and datagram in user buffer.
	 */
	bzero(&udp_in, sizeof(udp_in));

#ifdef INET6
	if (inp->inp_flags & INP_IPV6) {
	  /* Create sockaddr_in6 for AF_INET6 sockets. */
	  sa6 = (struct sockaddr_in6 *)(&udp_in);
	  sa6->sin6_len = sizeof(struct sockaddr_in6);
	  sa6->sin6_family = AF_INET6;
	  sa6->sin6_port = uh->uh_sport;
	  if (v6) { 
	    IN6_ADDR_COPY(sa6->sin6_addr, ip6->ipv6_src);
	  } else {
	    DPRINTF(IDL_EVENT,("IPv4 packet destined for IPv6 socket!\n"));
	    CREATE_IPV6_MAPPED(sa6->sin6_addr, ip->ip_src.s_addr);
	  }
	} else {
#endif /* INET6 */
   	sa = (struct sockaddr_in *)(&udp_in);
	sa->sin_len = sizeof(struct sockaddr_in);
	sa->sin_family = AF_INET;
	sa->sin_port = uh->uh_sport;
	sa->sin_addr = ip->ip_src;
#ifdef INET6
        }
#endif /* INET6 */

	if (inp->inp_flags & INP_CONTROLOPTS) {
		struct mbuf **mp = &opts;

#ifdef INET6
		DPRINTF(IDL_MAJOR_EVENT,\
			("Yikes... pcb has INP_CONTROLOPTS with flags=0x%x\n",\
			 inp->inp_flags));
#endif
		if (inp->inp_flags & INP_RECVDSTADDR) {
#ifdef INET6
		        caddr_t dst;
			struct in_addr6 addr6;
			
			/*
			 * Attempt to perform saveopt() for IPv6.
			 */
			if (v6)
			  dst = (caddr_t)&(ip6->ipv6_dst);
			else if (inp->inp_flags & INP_IPV6) {
			  CREATE_IPV6_MAPPED(addr6, ip->ip_dst.s_addr);
			  dst = (caddr_t)&addr6;    
		        } else 
			  dst = (caddr_t)&(ip->ip_dst);

			*mp = udp_saveopt(dst, 
			   (inp->inp_flags & INP_IPV6) ?
			   sizeof(struct in_addr6):sizeof(struct in_addr),
			   IP_RECVDSTADDR);
#else
			*mp = udp_saveopt((caddr_t) &ip->ip_dst, 
			   sizeof(struct in_addr), IP_RECVDSTADDR);
#endif /* INET6 */
			if (*mp)
				mp = &(*mp)->m_next;
		}
#ifdef notyet
		/* options were tossed above */
		if (inp->inp_flags & INP_RECVOPTS) {
			*mp = udp_saveopt((caddr_t) opts_deleted_above,
			    sizeof(struct in_addr), IP_RECVOPTS);
			if (*mp)
				mp = &(*mp)->m_next;
		}
		/* ip_srcroute doesn't do what we want here, need to fix */
		if (inp->inp_flags & INP_RECVRETOPTS) {
			*mp = udp_saveopt((caddr_t) ip_srcroute(),
			    sizeof(struct in_addr), IP_RECVRETOPTS);
			if (*mp)
				mp = &(*mp)->m_next;
		}
#endif
	}
	iphlen += sizeof(struct udphdr);
	m->m_len -= iphlen;
	m->m_pkthdr.len -= iphlen;
	m->m_data += iphlen;

	if (sbappendaddr(&inp->inp_socket->so_rcv, (struct sockaddr *)&udp_in,
	    m, opts) == 0) {
		udpstat.udps_fullsock++;
		goto bad;
	}
	sorwakeup(inp->inp_socket);
	return;
bad:
	m_freem(m);
	if (opts)
		m_freem(opts);
}

/*
 * Create a "control" mbuf containing the specified data
 * with the specified type for presentation with a datagram.
 */
struct mbuf *
udp_saveopt(p, size, type)
	caddr_t p;
	register int size;
	int type;
{
	register struct cmsghdr *cp;
	struct mbuf *m;

	if ((m = m_get(M_DONTWAIT, MT_CONTROL)) == NULL)
		return ((struct mbuf *) NULL);
	cp = (struct cmsghdr *) mtod(m, struct cmsghdr *);
	bcopy(p, CMSG_DATA(cp), size);
	size += sizeof(*cp);
	m->m_len = size;
	cp->cmsg_len = size;
	cp->cmsg_level = IPPROTO_IP;
	cp->cmsg_type = type;
	return (m);
}

/*
 * Notify a udp user of an asynchronous error;
 * just wake up so that he can collect error status.
 */
static void
udp_notify(inp, errno)
	register struct inpcb *inp;
	int errno;
{
	inp->inp_socket->so_error = errno;
	sorwakeup(inp->inp_socket);
	sowwakeup(inp->inp_socket);
}


void
udp_ctlinput(cmd, sa, ip
#ifdef INET6
	     ,incoming
#endif /* INET6 */
	     )
     int cmd;
     struct sockaddr *sa;
     register struct ip *ip;
#ifdef INET6
     /*
      * Extra argument "incoming" is for passing onward to in{,6}_pcbnotify.
      * (See that function for details.)
      */
     struct mbuf *incoming;
#endif /* INET6 */
{
	register struct udphdr *uh;
	extern struct in_addr zeroin_addr;
	extern u_char inetctlerrmap[];
#ifdef INET6
	extern struct in_addr6 zeroin_addr6;

	DPRINTF(IDL_EVENT,("Entering udp_ctlinput() with cmd=%d\n",cmd));
#endif /* INET6 */

	if (!PRC_IS_REDIRECT(cmd) &&
	    ((unsigned)cmd >= PRC_NCMDS || inetctlerrmap[cmd] == 0))
		return;
#ifdef INET6
	if (sa->sa_family == AF_INET6) {
	  if (ip) {
	        struct ipv6 *ip6 = (struct ipv6 *)ip;
		
		uh = (struct udphdr *)((caddr_t)ip6 + sizeof(struct ipv6));
		in6_pcbnotify(&udb, sa, uh->uh_dport, &(ip6->ipv6_src),
			uh->uh_sport, cmd, udp_notify, incoming,
			NEXTHDR_UDP);
	  } else
	        in6_pcbnotify(&udb, sa, 0, &zeroin_addr6, 0, cmd, udp_notify,
			incoming, NEXTHDR_UDP);
	} else {
#endif /* INET6 */
	if (ip) {
		uh = (struct udphdr *)((caddr_t)ip + (ip->ip_hl << 2));
		in_pcbnotify(&udb, sa, uh->uh_dport, ip->ip_src, uh->uh_sport,
#ifdef INET6
			cmd, udp_notify, incoming, NEXTHDR_UDP);
#else /* INET6 */
			cmd, udp_notify);
#endif /* INET6 */
	} else
		in_pcbnotify(&udb, sa, 0, zeroin_addr, 0, cmd, udp_notify
#ifdef INET6
			     ,incoming, NEXTHDR_UDP
#endif /* INET6 */
			     );

#ifdef INET6
        }
#endif /* INET6 */
}

int
udp_output(inp, m, addr, control)
	register struct inpcb *inp;
	register struct mbuf *m;
	struct mbuf *addr, *control;
{
	register struct udpiphdr *ui;
	register int len = m->m_pkthdr.len;
	struct in_addr laddr;
	int s, error = 0;
#ifdef INET6
	register struct in_addr6 laddr6;
	register struct udpip6hdr {
	  struct ipv6 i;
	  struct udphdr u;
	} *ui6;
	int v6packet = 0;
	int pcbflags = 0;

	DPRINTF(IDL_EVENT,("Entering udp_output()\n"));
#endif /* INET6 */

	if (control)
		m_freem(control);		/* XXX */

	if (addr) {
#ifdef INET6
	        /*
		 * Save current PCB flags because they may change during
		 * temporary connection, particularly the INP_IPV6_UNDEC
		 * flag.
		 */
                pcbflags = inp->inp_flags;

	        if (inp->inp_flags & INP_IPV6) {
		  DPRINTF(IDL_EVENT,("udpoutput() got a PF_INET6 socket!\n"));
		  IN6_ADDR_COPY(laddr6, inp->inp_laddr6);
		} else  
#endif /* INET6 */
		laddr = inp->inp_laddr;

#ifdef INET6
		if (((inp->inp_flags & INP_IPV6) &&
		     !IS_IN6_UNSPEC(inp->inp_faddr6))
		    || (inp->inp_faddr.s_addr != INADDR_ANY))
#else
		if (inp->inp_faddr.s_addr != INADDR_ANY) 
#endif /* INET6 */
		{
			error = EISCONN;
			goto release;
		}
		/*
		 * Must block input while temporarily connected.
		 */

		s = splnet();
		error = in_pcbconnect(inp, addr);

		if (error) {
			splx(s);
			goto release;
		}
	} else {
#ifdef INET6
	        if (((inp->inp_flags & INP_IPV6) && 
		     IS_IN6_UNSPEC(inp->inp_faddr6))
		    || (inp->inp_faddr.s_addr == INADDR_ANY))
#else
		if (inp->inp_faddr.s_addr == INADDR_ANY) 
#endif /* INET6 */
		{
			error = ENOTCONN;
			goto release;
		}
	}
	/*
	 * Calculate data length and get a mbuf
	 * for UDP and IP headers.
	 */
#ifdef INET6
	/*
	 * Handles IPv4-mapped IPv6 address because temporary connect sets
	 * the right flag.
	 */
	v6packet = ((inp->inp_flags & INP_IPV6) &&
		    !(inp->inp_flags & INP_IPV6_MAPPED));

	M_PREPEND(m, v6packet ? sizeof(struct udpip6hdr) : 
	    sizeof(struct udpiphdr), M_DONTWAIT);
#else
	M_PREPEND(m, sizeof(struct udpiphdr), M_DONTWAIT);
#endif /* INET6 */
	if (m == 0) {
	 /*
	  * Shouldn't this execute splx(s) if necessary before
	  * the goto?
	  */
		error = ENOBUFS;
		goto release;
	}

	/*
	 * Fill in mbuf with extended UDP header
	 * and addresses and length put into network format.
	 */
#ifdef INET6
	if (v6packet) {
	  ui6 = mtod(m, struct udpip6hdr *);
	  bcopy(&(inp->inp_ipv6), &(ui6->i), sizeof(struct ipv6)); 

	  /*
	   * Specify the version here, but should get flow label
	   * from the pcb.  Need to fix this later.
	   */
	  ui6->i.ipv6_versfl = 0x60000000;
	  ui6->i.ipv6_nexthdr = IPPROTO_UDP;
	  IN6_ADDR_COPY(ui6->i.ipv6_src, inp->inp_laddr6);
	  IN6_ADDR_COPY(ui6->i.ipv6_dst, inp->inp_faddr6);
	  ui6->i.ipv6_length = (u_short)len + sizeof(struct udphdr);
	  
	  ui6->u.uh_sport = inp->inp_lport;
	  ui6->u.uh_dport = inp->inp_fport;
	  ui6->u.uh_ulen = htons(ui6->i.ipv6_length);
	  ui6->u.uh_sum = 0;
	} else {    
#endif /* INET6 */
	ui = mtod(m, struct udpiphdr *);
	ui->ui_next = ui->ui_prev = 0;
	ui->ui_x1 = 0;
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_len = htons((u_short)len + sizeof (struct udphdr));
	ui->ui_src = inp->inp_laddr;
	ui->ui_dst = inp->inp_faddr;
	ui->ui_sport = inp->inp_lport;
	ui->ui_dport = inp->inp_fport;
	ui->ui_ulen = ui->ui_len;
#ifdef INET6
        }
#endif /* INET6 */

	
	/*
	 * Stuff checksum and output datagram.
	 */

#ifdef INET6
	if (v6packet) {
	  /* 
	   * Always calculate udp checksum for IPv6 datagrams
	   */
DPRINTF(IDL_EVENT,("Calling in6_cksum()\n")); 
	  if (!(ui6->u.uh_sum = in6_cksum(m, IPPROTO_UDP, 
	     len + sizeof(struct udphdr), sizeof(struct ipv6))))
	        ui6->u.uh_sum = 0xffff; 
	} else {  
#endif /* INET6 */
	ui->ui_sum = 0;
	if (udpcksum) {
	    if ((ui->ui_sum = in_cksum(m, sizeof (struct udpiphdr) + len)) == 0)
		ui->ui_sum = 0xffff;
	}
	((struct ip *)ui)->ip_len = sizeof (struct udpiphdr) + len;
	((struct ip *)ui)->ip_ttl = inp->inp_ip.ip_ttl;	/* XXX */
	((struct ip *)ui)->ip_tos = inp->inp_ip.ip_tos;	/* XXX */
#ifdef INET6
        }
#endif /* INET6 */

	udpstat.udps_opackets++;

#ifdef INET6
	/* Attach socket ptr for security handling down in IP{v6}. */
        (struct socket *)m->m_pkthdr.rcvif = inp->inp_socket;
        DPRINTF(IDL_GROSS_EVENT, ("m->m_pkthdr.rcvif = %08x\n",
            m->m_pkthdr.rcvif));
	if (v6packet) {
DPRINTF(IDL_EVENT,("udp_output calling ipv6_output()\n"));
	  error = ipv6_output(m, &inp->inp_route6, 
	    inp->inp_socket->so_options & SO_DONTROUTE,
	    inp->inp_moptions6);
	} else
#endif /* INET6 */
	error = ip_output(m, inp->inp_options, &inp->inp_route,
	    inp->inp_socket->so_options & (SO_DONTROUTE | SO_BROADCAST),
	    inp->inp_moptions);

	if (addr) {
		in_pcbdisconnect(inp);
#ifdef INET6
                inp->inp_flags = pcbflags;
		if (inp->inp_flags & INP_IPV6) {
		  IN6_ADDR_COPY(inp->inp_laddr6, laddr6);
	        } else
#endif /* INET6 */
		inp->inp_laddr = laddr;
		splx(s);
	}
	return (error);

release:
	m_freem(m);
	return (error);
}

#ifdef INET6
u_long  udp_sendspace = 9224;           /* 9200+sizeof(struct sockaddr_in6) */
u_long	udp_recvspace = 40 * (1024 + sizeof(struct sockaddr_in6));
#else
u_long	udp_sendspace = 9216;		/* really max datagram size */
u_long	udp_recvspace = 40 * (1024 + sizeof(struct sockaddr_in));
					/* 40 1K datagrams */
#endif /* INET6 */

/*ARGSUSED*/
int
udp_usrreq(so, req, m, addr, control)
	struct socket *so;
	int req;
	struct mbuf *m, *addr, *control;
{
	struct inpcb *inp = sotoinpcb(so);
	int error = 0;
	int s;

	if (inp == NULL && req != PRU_ATTACH) {
		error = EINVAL;
		goto release;
	}
	if (req == PRU_CONTROL)
#ifdef INET6
	  if (inp->inp_flags & INP_IPV6)
	        return (in6_control(so, (int)m, (caddr_t)addr,
			(struct ifnet *)control, 0));
	  else
#endif /* INET6 */
		return (in_control(so, (int)m, (caddr_t)addr,
			(struct ifnet *)control));
	/*
	 * Note: need to block udp_input while changing
	 * the udp pcb queue and/or pcb addresses.
	 */
	switch (req) {

	case PRU_ATTACH:
		if (inp != NULL) {
			error = EINVAL;
			break;
		}
		s = splnet();
		error = in_pcballoc(so, &udb);
		splx(s);
		if (error)
			break;
		error = soreserve(so, udp_sendspace, udp_recvspace);
		if (error)
			break;
#ifdef INET6
		if (((struct inpcb *)so->so_pcb)->inp_flags & INP_IPV6)
		  ((struct inpcb *) so->so_pcb)->inp_ipv6.ipv6_hoplimit =
		    ipv6_defhoplmt;
		else
#endif /* INET6 */		
		((struct inpcb *) so->so_pcb)->inp_ip.ip_ttl = ip_defttl;
		break;

	case PRU_DETACH:
		udp_detach(inp);
		break;

	case PRU_BIND:
		s = splnet();
		error = in_pcbbind(inp, addr);
		splx(s);
		break;

	case PRU_LISTEN:
		error = EOPNOTSUPP;
		break;

	case PRU_CONNECT:
#ifdef INET6 
		if (inp->inp_flags & INP_IPV6) {
		  if (!IS_IN6_UNSPEC(inp->inp_faddr6)) {
		        error = EISCONN;
			break;
		  }
		} else
#endif /* INET6 */
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			break;
		}
		s = splnet();
		error = in_pcbconnect(inp, addr);
		splx(s);
		if (error == 0)
			soisconnected(so);
		break;

	case PRU_CONNECT2:
		error = EOPNOTSUPP;
		break;

	case PRU_ACCEPT:
		error = EOPNOTSUPP;
		break;

	case PRU_DISCONNECT:
#ifdef INET6 
		if (inp->inp_flags & INP_IPV6) {
		  if (IS_IN6_UNSPEC(inp->inp_faddr6)) {
		        error = ENOTCONN;
			break;
		  }
		} else
#endif /* INET6 */
		if (inp->inp_faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			break;
		}
		s = splnet();
		in_pcbdisconnect(inp);
#ifdef INET6
		if (inp->inp_flags & INP_IPV6) {
		  IN6_ADDR_ASSIGN(inp->inp_laddr6, 0, 0, 0, 0);
		} else
#endif /* INET6 */
		  inp->inp_laddr.s_addr = INADDR_ANY;
		splx(s);
		so->so_state &= ~SS_ISCONNECTED;		/* XXX */
		break;

	case PRU_SHUTDOWN:
		socantsendmore(so);
		break;

	case PRU_SEND:
		return (udp_output(inp, m, addr, control));

	case PRU_ABORT:
		soisdisconnected(so);
		udp_detach(inp);
		break;

	case PRU_SOCKADDR:
		in_setsockaddr(inp, addr);
		break;

	case PRU_PEERADDR:
		in_setpeeraddr(inp, addr);
		break;

	case PRU_SENSE:
		/*
		 * stat: don't bother with a blocksize.
		 */
		/*
		 * Perhaps Path MTU might be returned for a connected
		 * UDP socket in this case.
		 */
		return (0);

	case PRU_SENDOOB:
	case PRU_FASTTIMO:
	case PRU_SLOWTIMO:
	case PRU_PROTORCV:
	case PRU_PROTOSEND:
		error =  EOPNOTSUPP;
		break;

	case PRU_RCVD:
	case PRU_RCVOOB:
		return (EOPNOTSUPP);	/* do not free mbuf's */

	default:
		panic("udp_usrreq");
	}

release:
	if (control) {
		printf("udp control data unexpectedly retained\n");
		m_freem(control);
	}
	if (m)
		m_freem(m);
	return (error);
}

static void
udp_detach(inp)
	struct inpcb *inp;
{
	int s = splnet();

	if (inp == udp_last_inpcb)
		udp_last_inpcb = &udb;
	in_pcbdetach(inp);
	splx(s);
}

/*
 * Sysctl for udp variables.
 */
udp_sysctl(name, namelen, oldp, oldlenp, newp, newlen)
	int *name;
	u_int namelen;
	void *oldp;
	size_t *oldlenp;
	void *newp;
	size_t newlen;
{
#ifdef INET6
DPRINTF(IDL_EVENT,("Entering udp_sysctl()\n"));
#endif
	/* All sysctl names at this level are terminal. */
	if (namelen != 1)
		return (ENOTDIR);

	switch (name[0]) {
	case UDPCTL_CHECKSUM:
		return (sysctl_int(oldp, oldlenp, newp, newlen, &udpcksum));
	default:
		return (ENOPROTOOPT);
	}
	/* NOTREACHED */
}
