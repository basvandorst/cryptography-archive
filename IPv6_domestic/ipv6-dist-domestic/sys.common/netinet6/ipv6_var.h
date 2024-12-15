/*
 * ipv6_var.h  --  Internal variables and structures for this implementation
 *                 of IP version 6.
 *
 * Copyright 1995 by Dan McDonald, Bao Phan, and Randall Atkinson,
 *	All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 */

/*----------------------------------------------------------------------
#       @(#)COPYRIGHT   1.1a (NRL) 17 August 1995

COPYRIGHT NOTICE

All of the documentation and software included in this software
distribution from the US Naval Research Laboratory (NRL) are
copyrighted by their respective developers.

This software and documentation were developed at NRL by various
people.  Those developers have each copyrighted the portions that they
developed at NRL and have assigned All Rights for those portions to
NRL.  Outside the USA, NRL also has copyright on the software
developed at NRL. The affected files all contain specific copyright
notices and those notices must be retained in any derived work.

NRL LICENSE

NRL grants permission for redistribution and use in source and binary
forms, with or without modification, of the software and documentation
created at NRL provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:

        This product includes software developed at the Information
        Technology Division, US Naval Research Laboratory.

4. Neither the name of the NRL nor the names of its contributors
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

#ifndef _IPV6_VAR_H
#define _IPV6_VAR_H

#include <netinet6/in6_types.h>

/*
 * Evil! Evil!
 * IPv6 overlay structure, used by TCP for reassembly.  (Later, it may be
 * used by other functions.)
 */

struct ipv6ovly
{
  caddr_t ih_next, ih_prev;
  struct in_addr6 ih_src;
  struct in_addr6 ih_dst;
};

/*
 * IPv6 multicast "options".  Session state for multicast, including
 * weird per-session multicast things.
 */

struct ipv6_moptions
{
  struct ifnet *i6mo_multicast_ifp;     /* ifp for outgoing multicasts */
  u_char i6mo_multicast_ttl;            /* TTL for outgoing multicasts.
					   Does this matter in IPv6? */
  u_char i6mo_multicast_loop;           /* 1 => hear sends if a member */
  u_short i6mo_num_memberships;         /* no. memberships this socket */
  struct in6_multi *i6mo_membership[IN6_MAX_MEMBERSHIPS];
};

/*
 * IPv6 stats.
 */

struct	ipv6stat {
	u_long	ips_total;		/* total packets received */
	u_long	ips_tooshort;		/* packet too short */
	u_long	ips_toosmall;		/* not enough data */
	u_long	ips_fragments;		/* fragments received */
	u_long	ips_fragdropped;	/* frags dropped (dups, out of space) */
	u_long	ips_fragtimeout;	/* fragments timed out */
	u_long	ips_forward;		/* packets forwarded */
	u_long	ips_cantforward;	/* packets rcvd for unreachable dest */
	u_long	ips_redirectsent;	/* packets forwarded on same net */
	u_long	ips_noproto;		/* unknown or unsupported protocol */
	u_long	ips_delivered;		/* datagrams delivered to upper level*/
	u_long	ips_localout;		/* total ip packets generated here */
	u_long	ips_odropped;		/* lost packets due to nobufs, etc. */
	u_long	ips_reassembled;	/* total packets reassembled ok */
	u_long	ips_fragmented;		/* datagrams sucessfully fragmented */
	u_long	ips_ofragments;		/* output fragments created */
	u_long	ips_cantfrag;		/* don't fragment flag was set, etc. */
	u_long	ips_badoptions;		/* error in option processing */
	u_long	ips_noroute;		/* packets discarded due to no route */
	u_long  ips_badvers;            /* IPv6 version != 6 */
	u_long	ips_rawout;		/* total raw ip packets generated */
};

#ifdef KERNEL

/*
 * The IPv6 fragment queue entry structure.
 * Notes:
 *   Nodes are stored in ttl order.
 *   prefix comes from whichever packet gets here first.
 *   data contains a chain of chains of mbufs (m_next down a chain, m_nextpkt
 *     chaining chains together) where the chains are ordered by assembly
 *     position. When two chains are contiguous for reassembly, they are
 *     combined and the frag header disappears.
 *   The structure is deliberately sized so MALLOC will round up on the order
 *     of much less than the total size instead of doubling the size.
 */

struct ipv6_fragment
{
  struct ipv6_fragment *next;           /* Next fragment chain */
  struct mbuf *prefix;                  /* Headers before frag header(s) */
  struct mbuf *data;                    /* Frag headers + whatever data */
  u_char ttl;                           /* Fragment chain TTL. */
  u_char flags;				/* Bit 0 indicates got end of chain */
};

/*
 * Structures and definitions for discovery mechanisms in IPv6.
 */

/*
 * Neighbor cache:
 *
 *     Number of unanswered probes is in discq.
 *     "Time of next event" will be in rt->rt_rmx.rmx_expire
 *          (rmx_expire will actually be quite overloaded, actually.)
 *     Status REACHABLE will be dq_unanswered < 0
 *     Status PROBE will be dq_unanswered >= 0
 *     Status INCOMPLETE will be link addr length of 0 if held,
 *     or deleted if not held.
 *
 *     If held, but INCOMPLETE fails set RTF_REJECT and make sure
 *     IPv6 and HLP's know how to deal with RTF_REJECT being set.
 */

struct discq   /* Similar to v4's llinfo_arp, discovery's "neighbor entry". */
{
  struct discq *dq_next,*dq_prev;        /* For {ins,rem}que(). */
  struct rtentry *dq_rt;                 /* Back pointer to routing entry for
					    an address that may be dead. */
  struct mbuf *dq_queue;                 /* Queue of outgoing messages. */
  int dq_unanswered;                     /* Number of unanswered probes. */
};

/* Routing flag redefinitions */
#define RTF_ISAROUTER RTF_PROTO2         /* Neighbor is a router. */
#define RTF_DEFAULT RTF_PROTO1           /* Default route. */

/*
 * These should be configurable parameters, see ipv6_discovery.c.
 * All units are in comments besides constants.
 */

#define MAX_INITIAL_RTR_ADVERT_INTERVAL		16  /* seconds */
#define MAX_INITIAL_RTR_ADVERTISEMENTS          3   /* transmissions */
#define MAX_RTR_RESPONSE_DELAY                  2   /* seconds */

#define MAX_RTR_SOLICITATION_DELAY              1   /* second */
#define RTR_SOLICITATION_INTERVAL               3   /* seconds */
#define MAX_RTR_SOLICITATIONS                   3   /* transmissions */

#define RESOLVE_RETRANS_TIMER                   1   /* seconds */
#define MAX_MULTICAST_SOLICIT                   3   /* transmissions */
#define MAX_UNICAST_SOLICIT                     3   /* transmissions */
#define MAX_NEIGHBOR_ADVERTISEMENTS             3   /* transmissions */
#define MIN_NEIGHBOR_ADVERT_INTERVAL            16  /* seconds */
#define REACHABLE_TIME                          30  /* seconds */
#define REACHABLE_RETRANS_TIME                  3   /* seconds */
#define DELAY_FIRST_PROBE_TIME                  3   /* seconds */

#define NEXTHOP_CLEAN_INTERVAL                  600 /* seconds */

/* 
 * Child of a router or tunnel.  Is a "meta-entry" for garbage collection.
 */

struct v6child
{
  struct v6child *v6c_next,*v6c_prev;   /* For {ins,rem}que() */
  struct v6router *v6c_parent;         /* Parent router.  I'm null if
					  I'm the router, or a tunnel
					  child. */
  struct rtentry *v6c_route;           /* Next-hop cache entry.  I won't
					  be holding it, but I'm attached
					  to it, like discq is to neighbor
					  cache entries. */
};

/*
 * Default router list entry.  Should be inserted
 * in priority order.  Will also have entries for non-
 * default routers, because I may be a router myself.
 */

struct v6router
{
  struct v6router *v6r_next,*v6r_prev;  /* For {ins,rem}que() */
  struct rtentry *v6r_rt;       /* Neighbor-cache entry.  Yes we will be
				   holding it.  This could cause...
				   problems. (But see route.c for hope!)
				   
				   For non-default route will not be held? */
  struct v6child v6r_children;  /* Children of this router. */
  /* Metric information? */
};

/*
 * Flags for "flags" argument in ipv6_output().
 */

#define IPV6_FORWARDING 0x1          /* Most of IPv6 header exists? */
#define IPV6_RAWOUTPUT 0x2           /* Raw IPv6 packet! */
#define IPV6_ROUTETOIF SO_DONTROUTE  /* Include sys/socket.h... */

/*
 * A new funky thing to allow easier header parsing,
 * given all of the weirdness with out-of-order header parsing and what-not.
 *
 * Some function will create one of these given an mbuf chain.
 * The idea is that each IPv6 header will have its own mbuf in the chain.
 *
 * In later versions, the "one-header-per-mbuf" should be thrown out, and
 * these should have more intelligence in them
 */

struct in6_hdrindex
{
  u_int8 ihi_nexthdr;         /* "Next header" value. */
  u_int8 ihi_filler[3];
  struct mbuf *ihi_mbuf;      /* Actual mbuf where this header is located. */
};

#define IPV6_MAXHDRCNT  7  /* Maximum number of option headers in an IPv6
			      datagram.  Derived from:

			      1. IPv6
			      2. Hop-by-hop
			      3. Dest. options (per-source-route-hop)
			      4. Source route
			      5. Auth. Header
			      6. Dest. options (end-to-end)
			      7. ESP/TCP/UDP/ICMP/etc.
			    */

void ipv6_init __P((void));
void ipv6_drain __P((void));
void ipv6_slowtimo __P((void));
int ipv6_sysctl __P((int *, u_int, void *, size_t *, void *, size_t));
int ipv6_output __P((struct mbuf *, struct route6 *, int,
		     struct ipv6_moptions *));
int ipv6_ctloutput __P((int, struct socket *,int,int, struct mbuf **));
int	 ripv6_ctloutput __P((int, struct socket *, int, int, struct mbuf **));
void	 ripv6_init __P((void));
void	 ripv6_input __P((struct mbuf *,int,struct in6_hdrindex *,int));
int	 ripv6_output __P((struct mbuf *, struct socket *, struct in_addr6 *));
int	 ripv6_usrreq __P((struct socket *,
	    int, struct mbuf *, struct mbuf *, struct mbuf *));
void ipv6_input __P((struct mbuf *,int,struct in6_hdrindex *,int));
void ipv6_hop __P((struct mbuf *,int,struct in6_hdrindex *,int));
int in6_control __P((struct socket *,int, caddr_t, struct ifnet *,int));
void ipv6_stripoptions __P((struct mbuf *, struct mbuf *, int));
struct mbuf *ipv6_preparse __P((struct mbuf *, struct in6_hdrindex *, int));
#endif /* KERNEL */

#endif /* _IPV6_VAR_H */
