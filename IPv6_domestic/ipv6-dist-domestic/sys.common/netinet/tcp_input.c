/*
 * tcp_input.c  --  Processes incoming TCP segments.  It is a rather large
 *                  chunk of code.
 *
 * Modifications to support IPv6 are copyright 1995 
 * by Randall Atkinson, Dan McDonald, and Bao Phan, All Rights Reserved.  
 * All Rights under this copyright have been assigned to NRL.
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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1994
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
 *	@(#)tcp_input.c	8.5 (Berkeley) 4/10/94
 * tcp_input.c,v 1.10 1994/10/13 18:36:32 wollman Exp
 */

#ifndef TUBA_INCLUDE
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#ifdef INET6
#include <sys/domain.h>
#endif
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>

#ifdef INET6
#include <netinet6/in6_debug.h>
#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipsec.h>
#include <netinet6/key.h>
#include <netinet6/ipsec_policy.h>
#endif /* INET6 */

#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>

#ifdef INET6
#include <netinet6/tcpipv6.h>
#endif /* INET6 */

#ifdef TCPDEBUG
#include <netinet/tcp_debug.h>
struct	tcpiphdr tcp_saveti;
#ifdef INET6
struct  tcpipv6hdr tcp_saveti6;
#endif /* INET6 */
#endif /* TCPDEBUG */

int	tcprexmtthresh = 3;
struct	inpcb *tcp_last_inpcb = &tcb;

#endif /* TUBA_INCLUDE */

#define TCP_PAWS_IDLE	(24 * 24 * 60 * 60 * PR_SLOWHZ)

/* for modulo comparisons of timestamps */
#define TSTMP_LT(a,b)	((int)((a)-(b)) < 0)
#define TSTMP_GEQ(a,b)	((int)((a)-(b)) >= 0)

#ifdef INET6
/* for the packet header length in the mbuf */
#define M_PH_LEN(m)      (((struct mbuf *)(m))->m_pkthdr.len)
#define M_V6_LEN(m)      (M_PH_LEN(m) - sizeof(struct ipv6))
#define M_V4_LEN(m)      (M_PH_LEN(m) - sizeof(struct ip))
#endif /* INET6 */

/*
 * Insert segment ti into reassembly queue of tcp with
 * control block tp.  Return TH_FIN if reassembly now includes
 * a segment with FIN.  The macro form does the common case inline
 * (segment is the next to be received on an established connection,
 * and the queue is empty), avoiding linkage into and removal
 * from the queue and repetition of various conversions.
 * Set DELACK for segments received in order, but ack immediately
 * when segments are out of order (so fast retransmit can work).
 */
#define	TCP_REASS(tp, ti, m, so, flags) { \
	if ((ti)->ti_seq == (tp)->rcv_nxt && \
	    (tp)->seg_next == (struct tcpiphdr *)(tp) && \
	    (tp)->t_state == TCPS_ESTABLISHED) { \
		(tp)->t_flags |= TF_DELACK; \
		(tp)->rcv_nxt += (ti)->ti_len; \
		flags = (ti)->ti_flags & TH_FIN; \
		tcpstat.tcps_rcvpack++;\
		tcpstat.tcps_rcvbyte += (ti)->ti_len;\
		sbappend(&(so)->so_rcv, (m)); \
		sorwakeup(so); \
	} else { \
		(flags) = tcp_reass((tp), (ti), (m)); \
		tp->t_flags |= TF_ACKNOW; \
	} \
}

#ifdef INET6
/*
 * IPv6 version of TCP_REASS.
 *
 * Note that in here is the first instance of a question about where TCP
 * should update neighbor reachability information.  This question pervades
 * this portion of TCP code.  It might be that this updating should happen
 * only in the timer, or in a timer, so as not to clog incoming data too much.
 */

#define titoth(x) (&(((struct tcpipv6hdr *)(x))->ti6_t))

#define	TCP_V6_REASS(tp, ti, m, so, flags, tlen) { \
	if (titoth(ti)->th_seq == (tp)->rcv_nxt && \
	    (caddr_t)((tp)->seg_next) == (caddr_t)(tp) && \
	    (tp)->t_state == TCPS_ESTABLISHED) { \
		(tp)->t_flags |= TF_DELACK; \
		(tp)->rcv_nxt += *(tlen); \
		flags = titoth(ti)->th_flags & TH_FIN; \
		tcpstat.tcps_rcvpack++; \
		tcpstat.tcps_rcvbyte += *(tlen); \
		  /*\
		   * For IPv6 datagrams, you ought to update the \
		   * appropriate neighbor cache (i.e. on-link \
		   * host route) entry as REACHABLE here, \
		   * but the performance impact would be grim.\
		   */\
		sbappend(&(so)->so_rcv, (m)); \
		sorwakeup(so); \
	} else { \
		   (flags) = tcpv6_reass((tp), (ti), (m), (tlen)); \
		   tp->t_flags |= TF_ACKNOW; \
	} \
}
#endif /* INET6 */

#ifndef TUBA_INCLUDE

/*
 * Reassembly function for TCP over IPv4 packets ONLY.
 */

int
tcp_reass(tp, ti, m)
	register struct tcpcb *tp;
	register struct tcpiphdr *ti;
	struct mbuf *m;
{
	register struct tcpiphdr *q;
	struct socket *so = tp->t_inpcb->inp_socket;
	int flags;

	/*
	 * Call with ti==0 after become established to
	 * force pre-ESTABLISHED data up to user socket.
	 */
	if (ti == 0)
		goto present;

	/*
	 * Find a segment which begins after this one does.
	 */
	for (q = tp->seg_next; q != (struct tcpiphdr *)tp;
	    q = (struct tcpiphdr *)q->ti_next)
		if (SEQ_GT(q->ti_seq, ti->ti_seq))
			break;

	/*
	 * If there is a preceding segment, it may provide some of
	 * our data already.  If so, drop the data from the incoming
	 * segment.  If it provides all of our data, drop us.
	 */
	if ((struct tcpiphdr *)q->ti_prev != (struct tcpiphdr *)tp) {
		register int i;
		q = (struct tcpiphdr *)q->ti_prev;
		/* conversion to int (in i) handles seq wraparound */
		i = q->ti_seq + q->ti_len - ti->ti_seq;
		if (i > 0) {
			if (i >= ti->ti_len) {
				tcpstat.tcps_rcvduppack++;
				tcpstat.tcps_rcvdupbyte += ti->ti_len;
				m_freem(m);
				return (0);
			}
			m_adj(m, i);
			ti->ti_len -= i;
			ti->ti_seq += i;
		}
		q = (struct tcpiphdr *)(q->ti_next);
	}
	tcpstat.tcps_rcvoopack++;
	tcpstat.tcps_rcvoobyte += ti->ti_len;
	REASS_MBUF(ti) = m;		/* XXX */

	/*
	 * While we overlap succeeding segments trim them or,
	 * if they are completely covered, dequeue them.
	 */
	while (q != (struct tcpiphdr *)tp) {
		register int i = (ti->ti_seq + ti->ti_len) - q->ti_seq;
		if (i <= 0)
			break;
		if (i < q->ti_len) {
			q->ti_seq += i;
			q->ti_len -= i;
			m_adj(REASS_MBUF(q), i);
			break;
		}
		q = (struct tcpiphdr *)q->ti_next;
		m = REASS_MBUF((struct tcpiphdr *)q->ti_prev);
		remque(q->ti_prev);
		m_freem(m);
	}

	/*
	 * Stick new segment in its place.
	 */
	insque(ti, q->ti_prev);

present:
	/*
	 * Present data to user, advancing rcv_nxt through
	 * completed sequence space.
	 */
	if (TCPS_HAVEESTABLISHED(tp->t_state) == 0)
		return (0);
	ti = tp->seg_next;
	if (ti == (struct tcpiphdr *)tp || ti->ti_seq != tp->rcv_nxt)
		return (0);
	if (tp->t_state == TCPS_SYN_RECEIVED && ti->ti_len)
		return (0);
	do {
		tp->rcv_nxt += ti->ti_len;
		flags = ti->ti_flags & TH_FIN;
		remque(ti);
		m = REASS_MBUF(ti);
		ti = (struct tcpiphdr *)ti->ti_next;
		if (so->so_state & SS_CANTRCVMORE)
			m_freem(m);
		else
			sbappend(&so->so_rcv, m);
	} while (ti != (struct tcpiphdr *)tp && ti->ti_seq == tp->rcv_nxt);
	sorwakeup(so);
	return (flags);
}

#ifdef INET6

/*
 * Reassembly function for TCP over IPv6 only!
 * Since we don't entirely trust struct tcpipv6hdr, we continue to use
 * the titoth() macro.  This function may be one of the few places we trust
 * struct tcpipv6hdr, however.
 */

int
tcpv6_reass(tp, ti, m, tlen)
	register struct tcpcb *tp;
	register struct tcpipv6hdr *ti;
	struct mbuf *m;
	int *tlen;
{
	register struct tcpipv6hdr *q;
	struct socket *so = tp->t_inpcb->inp_socket;
	int     flags;

	/*
	 * Call with ti==0 after become established to
	 * force pre-ESTABLISHED data up to user socket.
	 */
	if (ti == NULL)
		goto present2;

	/*
	 * Find a segment which begins after this one does.
	 */
	DPRINTF(IDL_GROSS_EVENT,("Enqueueing TCP/v6 segment.\n"));
	for (q  = (struct tcpipv6hdr *)tp->seg_next; 
	     q != (struct tcpipv6hdr *)tp;
	     q  = (struct tcpipv6hdr *)q->ti6_next)
	  if (SEQ_GT(titoth(q)->th_seq, titoth(ti)->th_seq))
	    break;

	/*
	 * If there is a preceding segment, it may provide some of
	 * our data already.  If so, drop the data from the incoming
	 * segment.  If it provides all of our data, drop us.
	 */
	if (q->ti6_prev != (caddr_t)tp) {
		register int i;

		DPRINTF(IDL_GROSS_EVENT,("Preceding TCP/v6 segment found.\n"));
		q = (struct tcpipv6hdr *)q->ti6_prev;

		/* conversion to int (in i) handles seq wraparound */
		i = titoth(q)->th_seq 
		  + (M_V6_LEN(REASSV6_MBUF(q)) - (q->ti6_off<<2))
		  - titoth(ti)->th_seq;

		if (i > 0) {
			if (i >= *tlen) {
				tcpstat.tcps_rcvduppack++;
				/*tcpstat.tcps_rcvdupbyte += M_V6_LEN(m);*/
				tcpstat.tcps_rcvdupbyte += *tlen;
				m_freem(m);
				return (0);
			}
			m_adj(m, i);
			*tlen -= i;
			titoth(ti)->th_seq += i;
		}
		q = (struct tcpipv6hdr *)q->ti6_next;
	}
	tcpstat.tcps_rcvoopack++;
	tcpstat.tcps_rcvoobyte += *tlen;

	/* next line puts ptr to mbuf into th_sport/th_dport of TCP hdr */
        /* This assumes that  (sizeof(pointer) <= 32 bits).  */
	REASSV6_MBUF(ti) = m;

	/*
	 * While we overlap succeeding segments trim them or,
	 * if they are completely covered, dequeue them.
	 */
	while (q != (struct tcpipv6hdr *)tp) {
		register int i = (titoth(ti)->th_seq + *tlen) - 
				  titoth(q)->th_seq;
		if (i <= 0)
			break;

		/* next line uses the mbuf backptr to get the length since
		   it isn't available from struct tcpipv6hdr. sigh */
		if (i < (M_V6_LEN(&(titoth(q)->th_sport)) - (q->ti6_off<<2))) {
		    titoth(q)->th_seq += i;
		    m_adj((REASSV6_MBUF(q)), i);
		    break;
		}
		q = (struct tcpipv6hdr *)q->ti6_next;
		m = REASSV6_MBUF((struct tcpipv6hdr *)q->ti6_prev);
		remque(q->ti6_prev);
		m_freem(m);
	}

	/*
	 * Stick new segment in its place.
	 */
DPRINTF(IDL_GROSS_EVENT,("TCP/v6 packet being enqueued.\n"));
	insque(ti, q->ti6_prev);

present2:
	/*
	 * Present data to user, advancing rcv_nxt through
	 * completed sequence space.
	 */
	if (TCPS_HAVEESTABLISHED(tp->t_state) == 0)
		return (0);
	ti = (struct tcpipv6hdr *)tp->seg_next;
	if (ti == (struct tcpipv6hdr *)tp || titoth(ti)->th_seq != tp->rcv_nxt)
		return (0);
	if (tp->t_state == TCPS_SYN_RECEIVED && 
	    (M_V6_LEN(REASSV6_MBUF(ti)) - (ti->ti6_off<<2)))
		return (0);
	do {
DPRINTF(IDL_GROSS_EVENT,("rcv_nxt is 0x%x, %d (%d - %d) is being added.\n",\
		      tp->rcv_nxt,\
		      (M_V6_LEN(REASSV6_MBUF(ti)) - (ti->ti6_off<<2)),\
		      M_V6_LEN(REASSV6_MBUF(ti)), ti->ti6_off<<2));

                tp->rcv_nxt += (M_V6_LEN(REASSV6_MBUF(ti)) - (ti->ti6_off<<2));

DPRINTF(IDL_GROSS_EVENT,("rcv_nxt is now 0x%x in present2 of tcpv6_reass().\n",\
		      tp->rcv_nxt));
		flags = titoth(ti)->th_flags & TH_FIN;
		remque(ti);
		m = REASSV6_MBUF(ti);
		ti = (struct tcpipv6hdr *)((struct ipv6ovly *)ti)->ih_next;
		if (so->so_state & SS_CANTRCVMORE)
			m_freem(m);
		else
			sbappend(&so->so_rcv, m);
	} while (ti != (struct tcpipv6hdr *)tp && 
		 titoth(ti)->th_seq == tp->rcv_nxt);
	/*
	 * For IPv6 datagrams, perhaps ought to update the
	 * appropriate neighbor cache (i.e. on-link host
	 * route) entry as REACHABLE here,
	 * but the adverse performance impact would be grim.
	 */
	sorwakeup(so);
	return (flags);
}
#endif /* INET6 */


/*
 * TCP input routine, follows pages 65-76 of the
 * protocol specification dated September, 1981 very closely.
 */
void
tcp_input(m, iphlen
#ifdef INET6
	  , ihi,ihioff
#endif /* INET6 */
)
	register struct mbuf *m;
	int iphlen;
#ifdef INET6
        /*
	 * Extra parameters are primarily for ipv6_stripoptions.
	 */
        struct in6_hdrindex *ihi;
        int ihioff;
#endif /* INET6 */
{
	register struct tcpiphdr *ti = NULL;
	register struct tcphdr   *th;
	register struct inpcb *inp;
	caddr_t optp = NULL;
	int optlen = 0;
	int len, tlen, off;
	register struct tcpcb *tp = NULL;
	register int tiflags;
	struct socket *so = NULL;
	int todrop, acked, ourfinisacked, needoutput = 0;
	struct in_addr laddr;
	int dropsocket = 0;
	int iss = 0;
	u_long tiwin, ts_val, ts_ecr;
	int ts_present = 0;
	unsigned int    ih_size;    /* size of IPvN header  */
#ifdef INET6
	struct in_addr6 laddr6;
	unsigned short is_ipv6;     /* Type of incoming datagram. */
	register struct tcpipv6hdr *ti6 = NULL;
#endif /* INET6 */
#ifdef TCPDEBUG
	short ostate = 0;
#endif

	tcpstat.tcps_rcvtotal++;
	/*
	 * Get IP and TCP header together in first mbuf.
	 * Note: IP leaves IP header in first mbuf.
	 */
	ti = mtod(m, struct tcpiphdr *);

#ifdef INET6
	/*
	 * Before we do ANYTHING, we have to figure out if it's TCP/IPv6 or
	 * TCP/IPv4.
	 */
	is_ipv6 = (((struct ip*)ti)->ip_v != 4);

	/*
	 * After that, do initial segment processing which is still very
	 * dependent on what IP version you're using.
	 */
	if (is_ipv6) {

	  DPRINTF(IDL_EVENT, ("Entering TCP input with v6 datagram!\n"));
	  ih_size = sizeof(struct ipv6);
	  ti6 = mtod(m, struct tcpipv6hdr *);

	/* strip off any options */
	  if (iphlen > sizeof(struct ipv6))
	    ipv6_stripoptions(m,ihi[ihioff - 1].ihi_mbuf, 
			      ihi[ihioff].ihi_nexthdr); /* Last arg better
							   be NEXTHDR_TCP! */

	  /* If hdrs not in 1st mbuf, pull them into 1st mbuf */
	  /* m_pullup() HAS to be called here, because of the way
	     ipv6_preparse works.  Preparsing is a big performance
	     loss when only have TCP and IPv6. Sigh.  */
	  if (!(m = m_pullup(m, sizeof (struct tcpipv6hdr)))) {
	    DPRINTF(IDL_GROSS_EVENT,("m_pullup failed for TCP/IPv6 incoming.\n"));
	    tcpstat.tcps_rcvshort++;
	    return;
	  } /* endif m_pullup */
	  ti6 = mtod(m, struct tcpipv6hdr *);

	  th = &(ti6->ti6_t);
	  
	  /* NB: ipv6.ipv6_length will be zero for a non-zero jumbogram */ 
	  tlen = M_V6_LEN(m);

	  /* Perform TCP checksum with pseudo-header */
	  if (in6_cksum(m, NEXTHDR_TCP, tlen, sizeof(struct ipv6))) {
	    DPRINTF(IDL_EVENT,("in6_cksum failed in tcp_input().\n"));
	    tcpstat.tcps_rcvbadsum++;
	    goto drop;
	  } /* endif in6_cksum */
	  DPRINTF(IDL_EVENT,("Segment passed in6_cksum().\n"));

	} else {
#endif /* INET6 */
	  ih_size = sizeof(struct ip);

	  /* If options present, strip them */
	  if (iphlen > sizeof (struct ip))
	    ip_stripoptions(m, (struct mbuf *)0);

	  /* If hdrs not in 1st mbuf, pull them into 1st mbuf */
	  if (m->m_len < sizeof (struct tcpiphdr)) {
	    if ((m = m_pullup(m, sizeof (struct tcpiphdr))) == NULL) {
	      tcpstat.tcps_rcvshort++;
	      return;
	    }
	    ti = mtod(m, struct tcpiphdr *);
	  }
	  th = (struct tcphdr *) ((caddr_t)ti + sizeof (struct ipovly));

	  /*
	   * Checksum extended TCP header and data.
	   */
	  tlen = ((struct ip *)ti)->ip_len;
	  len = sizeof (struct ip) + tlen;
	  ti->ti_next = ti->ti_prev = 0;
	  ti->ti_x1 = 0;
	  ti->ti_len = htons((u_short)tlen);
	  th->th_sum = in_cksum(m, len);
	  /* At this point, the variable "len" is dead. */
	  if (th->th_sum) {
	    tcpstat.tcps_rcvbadsum++;
	    goto drop;
	  } /* endif th->th_sum */
#ifdef INET6
	}
	DP(EVENT, tlen, d);
	DP(EVENT, len, d);

#endif /* INET6 */

#endif /* TUBA_INCLUDE */

	/*
	 * Check that TCP offset makes sense,
	 * pull out TCP options and adjust length.		XXX
	 */

	off = th->th_off << 2;

	if (off < sizeof (struct tcphdr) || off > tlen) {
		tcpstat.tcps_rcvbadoff++;
		goto drop;
	}
	tlen -= off;

#undef ti->ti_len

        /* 
	 * Since tcpipv6hdr can't store the length, tcp_input() has been
	 * modified to use tlen to mean the TCP data length throughout
	 * the function.  Other functions can use m->m_pkthdr.len as
	 * the basis for calculating the TCP data length.  rja
	 */

	if (off > sizeof (struct tcphdr)) {
		if (m->m_len < ih_size + off) {
			if ((m = m_pullup(m, ih_size + off)) == 0) {
				tcpstat.tcps_rcvshort++;
				return;
			}
#ifdef INET6
			if (is_ipv6) {
			  ti6 = mtod(m, struct tcpipv6hdr *);
			  th = &ti6->ti6_t;
			} else
#endif /* INET6 */
			  {
			    ti = mtod(m, struct tcpiphdr *);
			    th = &ti->ti_t;
			  } /* endif is_ipv6 */
		      }
		optlen = off - sizeof (struct tcphdr);

		optp = mtod(m, caddr_t) + ih_size + sizeof (struct tcphdr);

		/* 
		 * Do quick retrieval of timestamp options ("options
		 * prediction?").  If timestamp is the only option and it's
		 * formatted as recommended in RFC 1323 appendix A, we
		 * quickly get the values now and not bother calling
		 * tcp_dooptions(), etc.
		 */
		if ((optlen == TCPOLEN_TSTAMP_APPA ||
		     (optlen > TCPOLEN_TSTAMP_APPA &&
			optp[TCPOLEN_TSTAMP_APPA] == TCPOPT_EOL)) &&
		     *(u_long *)optp == htonl(TCPOPT_TSTAMP_HDR) &&
		     !(th->th_flags & TH_SYN)) {
			ts_present = 1;
			ts_val = ntohl(*(u_long *)(optp + 4));
			ts_ecr = ntohl(*(u_long *)(optp + 8));
			optp = NULL;	/* we've parsed the options */
		}
	}
	tiflags = th->th_flags;

	/*
	 * Convert TCP protocol specific fields to host format.
	 */
	NTOHL(th->th_seq);
	NTOHL(th->th_ack);
	NTOHS(th->th_win);
	NTOHS(th->th_urp);

	/*
	 * Locate pcb for segment.
	 */
findpcb:
	inp = tcp_last_inpcb;

	/*
	 * Quite frankly, it is cleaner to keep the v4/v6 case separate
	 * from the v4-only case.  There are uses of ti in the v4-only case,
	 * but only this once...
	 */
#ifndef INET6
	if (inp->inp_lport != ti->ti_dport ||
	    inp->inp_fport != ti->ti_sport ||
	    inp->inp_faddr.s_addr != ti->ti_src.s_addr ||
	    inp->inp_laddr.s_addr != ti->ti_dst.s_addr) {
		inp = in_pcblookup(&tcb, ti->ti_src, ti->ti_sport,
		    ti->ti_dst, ti->ti_dport, INPLOOKUP_WILDCARD);
		if (inp)
			tcp_last_inpcb = inp;
		++tcpstat.tcps_pcbcachemiss;
	}
#else /* INET6 */
	{
	  int miss = 0;
	  if (inp->inp_lport != th->th_dport || inp->inp_fport != th->th_sport)
	    miss++;
	  else
	    /* If connected to IPv6, and IPv4 dgram arrives, it's a miss. */
	    if ((inp->inp_flags & INP_IPV6) &&
		!(inp->inp_flags & INP_IPV6_MAPPED) && !is_ipv6)
	      miss++;
	    else
	      /* Right version of IP, anyway.  Check for address matches. */
	      if (is_ipv6) {
		if (!IN6_ADDR_EQUAL(inp->inp_faddr6,ti6->ti6_src) ||
		    !IN6_ADDR_EQUAL(inp->inp_laddr6,ti6->ti6_dst))
		  miss++;
	      } else {
		if ((inp->inp_faddr.s_addr != ti->ti_src.s_addr)
		    || (inp->inp_laddr.s_addr != ti->ti_dst.s_addr))
		  miss++;
	      }
	  if (miss) {
	    if (is_ipv6) {
	      register struct in_addr src, dst;
	      src.s_addr = (u_long)&(ti6->ti6_src);
	      dst.s_addr = (u_long)&(ti6->ti6_dst);
	      DPRINTF(IDL_EVENT,("Looking up pcb for IPv6 dgram.\n"));
	      inp = in_pcblookup(&tcb, src, th->th_sport, dst, th->th_dport,
				 INPLOOKUP_WILDCARD | INPLOOKUP_IPV6);
	      DPRINTF(IDL_EVENT,("inp returned is: 0x%x\n",inp));
	    } else {
	      inp = in_pcblookup(&tcb, ((struct ip *)ti)->ip_src,
				 th->th_sport, ((struct ip *)ti)->ip_dst,
				 th->th_dport, INPLOOKUP_WILDCARD);
	    } /* endif is_ipv6 */
	    if (inp)
	      tcp_last_inpcb = inp;
	    tcpstat.tcps_pcbcachemiss++;
	  }
	}
#endif /* INET6 */

	/*
	 * If the state is CLOSED (i.e., TCB does not exist) then
	 * all data in the incoming segment is discarded.
	 * If the TCB exists but is in CLOSED state, it is embryonic,
	 * but should either do a listen or a connect soon.
	 */
	if (inp == 0)
		goto dropwithreset;
	tp = intotcpcb(inp);
	if (tp == 0)
		goto dropwithreset;

	if (tp->t_state == TCPS_CLOSED)
		goto drop;
	
#ifdef INET6  /* symbol arguably should be IPSEC rather than INET6 */
	/*
	 * Perform IPsec input policy check on incoming packet,
	 * If security policy check fails, drop this segment.
	 *
	 * NB:  Deliberately doesn't send a RESET because doing that 
	 * would open up a denial of service attack that is easily avoided.
	 */
	if (ipsec_input_policy(m, inp->inp_socket, NEXTHDR_TCP))
	  goto drop;
#endif /* INET6 */

	/* Unscale the window into a 32-bit value. */
	if ((tiflags & TH_SYN) == 0)
	  tiwin = th->th_win << tp->snd_scale;
	else
	  tiwin = th->th_win;

	so = inp->inp_socket;
	if (so->so_options & (SO_DEBUG|SO_ACCEPTCONN)) {
#ifdef TCPDEBUG
		if (so->so_options & SO_DEBUG) {
		  ostate = tp->t_state;
#ifdef INET6
		  if (is_ipv6)
		    tcp_saveti6 = *ti6;
		  else
#endif /* INET6 */
		    tcp_saveti = *ti;
		}
#endif /* TCPDEBUG */

		if (so->so_options & SO_ACCEPTCONN) {
		  /*
		   * If new connection, new socket will inherit parent
		   * socket's security properties.  (See uipc_socket2.c)
		   */
			so = sonewconn(so, 0);
			if (so == 0)
				goto drop;
			/*
			 * This is ugly, but ....
			 *
			 * Mark socket as temporary until we're
			 * committed to keeping it.  The code at
			 * ``drop'' and ``dropwithreset'' check the
			 * flag dropsocket to see if the temporary
			 * socket created here should be discarded.
			 * We mark the socket as discardable until
			 * we're committed to it below in TCPS_LISTEN.
			 */
			dropsocket++;
#ifdef INET6
			/*
			 * inp still has the OLD in_pcb stuff, set the
			 * v6-related flags on the new guy, too.   This is
			 * done particularly for the case where an AF_INET6
			 * socket is bound only to a port, and a v4 connection
			 * comes in on that port.
			 */
			{
			  int flags = inp->inp_flags;

			  inp = (struct inpcb *)so->so_pcb;
			  inp->inp_flags |= (flags & (INP_IPV6 | INP_IPV6_UNDEC
						      | INP_IPV6_MAPPED));
			}
#else /* INET6 */
			inp = (struct inpcb *)so->so_pcb;
#endif /* INET6 */

			inp->inp_lport = th->th_dport;
#ifdef INET6
			if (is_ipv6) {
			  IN6_ADDR_COPY(inp->inp_laddr6,ti6->ti6_dst);
			  /*inp->inp_options = ipv6_srcroute();*/ /* soon. */
			  /* still need to tweak outbound options
			     processing to include this mbuf in
			     the right place and put the correct
			     NextHdr values in the right places.
			     XXX  rja */
			} else {
			  if (inp->inp_flags & INP_IPV6)  /* v4 to v6 socket */
			    {
			      CREATE_IPV6_MAPPED(inp->inp_laddr6,\
						 ti->ti_dst.s_addr);
			    }
			  else
#endif /* INET6 */			    
			  inp->inp_laddr = ti->ti_dst;
			  inp->inp_options = ip_srcroute();
#ifdef INET6
			}
#endif /* INET6 */

			tp = intotcpcb(inp);
			tp->t_state = TCPS_LISTEN;

			/* Compute proper scaling value from buffer space
			 */
			while (tp->request_r_scale < TCP_MAX_WINSHIFT &&
			   TCP_MAXWIN << tp->request_r_scale < so->so_rcv.sb_hiwat)
				tp->request_r_scale++;
		}
	}

	/*
	 * Segment received on connection.
	 * Reset idle time and keep-alive timer.
	 */
	tp->t_idle = 0;
	tp->t_timer[TCPT_KEEP] = tcp_keepidle;

	/*
	 * Process options if not in LISTEN state,
	 * else do it below (after getting remote address).
	 */
	if (optp && tp->t_state != TCPS_LISTEN)
	  tcp_dooptions(tp, (u_char *) optp, optlen, th, &ts_present,
			&ts_val, &ts_ecr);

	/* 
	 * Header prediction: check for the two common cases
	 * of a uni-directional data xfer.  If the packet has
	 * no control flags, is in-sequence, the window didn't
	 * change and we're not retransmitting, it's a
	 * candidate.  If the length is zero and the ack moved
	 * forward, we're the sender side of the xfer.  Just
	 * free the data acked & wake any higher level process
	 * that was blocked waiting for space.  If the length
	 * is non-zero and the ack didn't move, we're the
	 * receiver side.  If we're getting packets in-order
	 * (the reassembly queue is empty), add the data to
	 * the socket buffer and note that we need a delayed ack.
	 */
	if (tp->t_state == TCPS_ESTABLISHED &&
	    (tiflags & (TH_SYN|TH_FIN|TH_RST|TH_URG|TH_ACK)) == TH_ACK &&
	    (!ts_present || TSTMP_GEQ(ts_val, tp->ts_recent)) &&
	    th->th_seq == tp->rcv_nxt &&
	    tiwin && tiwin == tp->snd_wnd &&
	    tp->snd_nxt == tp->snd_max) {

		/* 
		 * If last ACK falls within this segment's sequence numbers,
		 *  record the timestamp.
		 */
	        /* Original 4.4-Lite code was not correct here, according to 
		   Section 26.6 of [WS95].  Next code fragment has been
		   revised to follow [Bra93], as reported in [WS95].  
		   NB: TSTMP_GEQ() test was done in the "if" just above. rja */
		if (ts_present && SEQ_LEQ(th->th_seq, tp->last_ack_sent)) 
		  {
		    tp->ts_recent_age = tcp_now;
		    tp->ts_recent = ts_val;
		  }

		if (tlen == 0) {
		  /* there is no TCP data present */
		  if (SEQ_GT(th->th_ack, tp->snd_una) &&
		      SEQ_LEQ(th->th_ack, tp->snd_max) &&
		      tp->snd_cwnd >= tp->snd_wnd) {
		    /*
		     * this is a pure ack for outstanding data.
		     *
		     * Perhaps update IPv6 neighbor reachability here.
		     */
		    ++tcpstat.tcps_predack;
		    if (ts_present)
		      tcp_xmit_timer(tp, tcp_now-ts_ecr+1);
		    else if (tp->t_rtt &&
			     SEQ_GT(th->th_ack, tp->t_rtseq))
		      tcp_xmit_timer(tp, tp->t_rtt);
		    acked = th->th_ack - tp->snd_una;
		    tcpstat.tcps_rcvackpack++;
		    tcpstat.tcps_rcvackbyte += acked;
		    sbdrop(&so->so_snd, acked);
		    tp->snd_una = th->th_ack;
		    
		    m_freem(m);
		    /*
		     * If all outstanding data are acked, stop
		     * retransmit timer, otherwise restart timer
		     * using current (possibly backed-off) value.
		     * If process is waiting for space,
		     * wakeup/selwakeup/signal.  If data
		     * are ready to send, let tcp_output
		     * decide between more output or persist.
		     */
		    if (tp->snd_una == tp->snd_max)
		      tp->t_timer[TCPT_REXMT] = 0;
		    else if (tp->t_timer[TCPT_PERSIST] == 0)
		      tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;
		    
		    if (so->so_snd.sb_flags & SB_NOTIFY)
		      sowwakeup(so);
		    if (so->so_snd.sb_cc)
		      (void) tcp_output(tp);
		    return;
		  }
		} else if ((th->th_ack == tp->snd_una) &&
		      (tp->seg_next == (struct tcpiphdr *)tp) && 
		      (tlen <= sbspace(&so->so_rcv))) {
			/*
			 * this is a pure, in-sequence data packet
			 * with nothing on the reassembly queue and
			 * we have enough buffer space to take it.
			 */
			++tcpstat.tcps_preddat;
			tp->rcv_nxt += tlen;
			tcpstat.tcps_rcvpack++;
			tcpstat.tcps_rcvbyte += tlen;

			/*
			 * Drop TCP, IP headers and TCP options then add data
			 * to socket buffer.
			 */

			m->m_data += ih_size + off;
			m->m_len  -= ih_size + off;

			/*
			 * Perhaps update IPv6 neighbor reachability here.
			 * Performance impact would be grim though.
			 */
			sbappend(&so->so_rcv, m);
			sorwakeup(so);
			/*
			 * If this is a small packet, then ACK now - with Nagel
			 * congestion avoidance sender won't send more until
			 * he gets an ACK.
			 */
			if ((unsigned)tlen < tp->t_maxseg) {
				tp->t_flags |= TF_ACKNOW;
				tcp_output(tp);
			} else {
				tp->t_flags |= TF_DELACK;
			}
			return;
		}
	}

	/*
	 * Drop TCP, IP headers and TCP options.
	 */
	m->m_data += ih_size + off;
	m->m_len  -= ih_size + off;

	/*
	 * Calculate amount of space in receive window,
	 * and then do TCP input processing.
	 * Receive window is amount of space in rcv queue,
	 * but not less than advertised window.
	 */
	{ int win;

	win = sbspace(&so->so_rcv);
	if (win < 0)
		win = 0;
	tp->rcv_wnd = max(win, (int)(tp->rcv_adv - tp->rcv_nxt));
	}

	switch (tp->t_state) {

	/*
	 * If the state is LISTEN then ignore segment if it contains an RST.
	 * If the segment contains an ACK then it is bad and send a RST.
	 * If it does not contain a SYN then it is not interesting; drop it.
	 * Don't bother responding if the destination was a broadcast.
	 * Otherwise initialize tp->rcv_nxt, and tp->irs, select an initial
	 * tp->iss, and send a segment:
	 *     <SEQ=ISS><ACK=RCV_NXT><CTL=SYN,ACK>
	 * Also initialize tp->snd_nxt to tp->iss+1 and tp->snd_una to tp->iss.
	 * Fill in remote peer address fields if not previously specified.
	 * Enter SYN_RECEIVED state, and process any other fields of this
	 * segment in this state.
	 */
	case TCPS_LISTEN: {
		struct mbuf *am;
		register struct sockaddr_in *sin;
#ifdef INET6
		register struct sockaddr_in6 *sin6;
#endif /* INET6 */

		if (tiflags & TH_RST)
			goto drop;
		if (tiflags & TH_ACK)
			goto dropwithreset;
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		/*
		 * RFC1122 4.2.3.10, p. 104: discard bcast/mcast SYN
		 * in_broadcast() should never return true on a received
		 * packet with M_BCAST not set.
		 */
#ifndef INET6
		if (m->m_flags & (M_BCAST|M_MCAST) ||
		    IN_MULTICAST(ntohl(ti->ti_dst.s_addr)))
#else /* INET6 */
		if (m->m_flags & (M_BCAST|M_MCAST))
#endif /* INET6 */
			goto drop;
#ifdef INET6
		if (is_ipv6)
		  /* XXX What about IPv6 Anycasting ?? :-(  rja */
		  if (IS_IN6_MULTICAST(ti6->ti6_dst))
		    goto drop;
		else
		  if (IN_MULTICAST(ntohl(ti->ti_dst.s_addr)))
		    goto drop;
#endif /* INET6 */
		am = m_get(M_DONTWAIT, MT_SONAME);	/* XXX */
		if (am == NULL)
			goto drop;

#ifdef INET6
		if (is_ipv6) {
		  /*
		   * This is probably the place to set the tp->pf value.
		   * (Don't forget to do it in the v4 code as well!)
		   *
		   * Also, remember to blank out things like flowlabel, or
		   * set flowlabel for accepted sockets in v6.
		   *
		   * FURTHERMORE, this is PROBABLY the place where the whole
		   * business of key munging is set up for passive
		   * connections.
		   */
		  DPRINTF(IDL_EVENT,("Setting up TCP/IPv6 passive conn.\n"));
		  am->m_len = sizeof(struct sockaddr_in6);
		  sin6 = mtod(am, struct sockaddr_in6 *);
		  sin6->sin6_family = AF_INET6;
		  sin6->sin6_len = sizeof(*sin6);
		  IN6_ADDR_COPY(sin6->sin6_addr, 
				((struct ipv6 *)ti6)->ipv6_src);
		  sin6->sin6_port = th->th_sport;
		  sin6->sin6_flowinfo = 0;
		  IN6_ADDR_COPY(laddr6, inp->inp_laddr6);
		  if (IS_IN6_UNSPEC(inp->inp_laddr6))
		    IN6_ADDR_COPY(inp->inp_laddr6, 
				  ((struct ipv6 *)ti6)->ipv6_dst);
		  /* This is a good optimization. */
		  if (in6_pcbconnect(inp, am)) {
		    DPRINTF(IDL_EVENT,("Failed in6_pcbconnect().\n"));
		    IN6_ADDR_COPY(inp->inp_laddr6, laddr6);
		    (void) m_free(am);
		    goto drop;
		  } /* endif in6_pcbconnect() */
		  tp->pf = PF_INET6;
		} else {
		  /*
		   * Letting v4 incoming datagrams to reach valid 
		   * PF_INET6 sockets causes some overhead here.
		   */
		  if (inp->inp_flags & INP_IPV6)
		    {
		      if (!(inp->inp_flags & (INP_IPV6_UNDEC|INP_IPV6_MAPPED)))
			{
			  (void) m_free(am);
			  goto drop;
			}

		      am->m_len = sizeof(struct sockaddr_in6);
		      sin6 = mtod(am, struct sockaddr_in6 *);
		      sin6->sin6_family = AF_INET6;
		      sin6->sin6_len = sizeof(*sin6);
		      IN6_ADDR_ASSIGN(sin6->sin6_addr,0,0,htonl(0xffff),\
				      ti->ti_src.s_addr);
		      sin6->sin6_port = th->th_sport;
		      sin6->sin6_flowinfo = 0;
		      laddr6 = inp->inp_laddr6;
		      if (inp->inp_laddr.s_addr == INADDR_ANY)
			{
			  IN6_ADDR_ASSIGN(inp->inp_laddr6,0,0,htonl(0xffff),\
					  ti->ti_dst.s_addr);
			}
		      if (in6_pcbconnect(inp, am)) {
			inp->inp_laddr6 = laddr6;
			(void) m_freem(am);
			DPRINTF(IDL_EVENT,("Bad in6_pcbconnect().\n"));
			goto drop;
		      }
		      DPRINTF(IDL_EVENT,("Good in6_pcbconnect().\n"));
		      tp->pf = PF_INET;
		    }
		  else
		    {
		      DPRINTF(IDL_EVENT,("\n"));
#endif /* INET6 */
		am->m_len = sizeof (struct sockaddr_in);
		sin = mtod(am, struct sockaddr_in *);
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(*sin);
		sin->sin_addr = ti->ti_src;
		sin->sin_port = ti->ti_sport;
		bzero((caddr_t)sin->sin_zero, sizeof(sin->sin_zero));
		laddr = inp->inp_laddr;
		if (inp->inp_laddr.s_addr == INADDR_ANY)
			inp->inp_laddr = ti->ti_dst;
		if (in_pcbconnect(inp, am)) {
			inp->inp_laddr = laddr;
			(void) m_free(am);
			goto drop;
		}
#ifdef INET6
	            }  /* if (inp->inp_flags & INP_IPV6) */
		} /* if (is_ipv6) */
#endif /* INET6 */
		(void) m_free(am);
		tp->t_template = tcp_template(tp);
		if (tp->t_template == 0) {
			tp = tcp_drop(tp, ENOBUFS);
			dropsocket = 0;		/* socket is already gone */
			goto drop;
		}
		if (optp)
			tcp_dooptions(tp, (u_char *) optp, optlen, th,
				      &ts_present, &ts_val, &ts_ecr);
		if (iss)
			tp->iss = iss;
		else
			tp->iss = tcp_iss;
		tcp_iss += TCP_ISSINCR/2;
		tp->irs = th->th_seq;

		tcp_sendseqinit(tp);
		tcp_rcvseqinit(tp);
		tp->t_flags |= TF_ACKNOW;
		tp->t_state = TCPS_SYN_RECEIVED;
		tp->t_timer[TCPT_KEEP] = TCPTV_KEEP_INIT;
		dropsocket = 0;		/* committed to socket */
		tcpstat.tcps_accepts++;
		goto trimthenstep6;
		}
	/*
	 * If the state is SYN_SENT:
	 *	if seg contains an ACK, but not for our SYN, drop the input.
	 *	if seg contains a RST, then drop the connection.
	 *	if seg does not contain SYN, then drop it.
	 * Otherwise this is an acceptable SYN segment
	 *	initialize tp->rcv_nxt and tp->irs
	 *	if seg contains ack then advance tp->snd_una
	 *	if SYN has been acked change to ESTABLISHED else SYN_RCVD state
	 *	arrange for segment to be acked (eventually)
	 *	continue processing rest of data/controls, beginning with URG
	 */
	case TCPS_SYN_SENT:
		if ((tiflags & TH_ACK) &&
		    (SEQ_LEQ(th->th_ack, tp->iss) ||
		     SEQ_GT(th->th_ack, tp->snd_max)))
			goto dropwithreset;

		if (tiflags & TH_RST) {
			if (tiflags & TH_ACK)
				tp = tcp_drop(tp, ECONNREFUSED);
			goto drop;
		}
		if ((tiflags & TH_SYN) == 0)
			goto drop;
		if (tiflags & TH_ACK) {
			tp->snd_una = th->th_ack;
			if (SEQ_LT(tp->snd_nxt, tp->snd_una))
				tp->snd_nxt = tp->snd_una;
		}
	        /* only turn off timer if ACK flag is set because
                   otherwise not sure if other end received our SYN.
		   See Section 28.6 of [WS95] for details.  rja   */
  	        if (tiflags & TH_ACK)
		        tp->t_timer[TCPT_REXMT] = 0;

		tp->irs = th->th_seq;
		tcp_rcvseqinit(tp);
		tp->t_flags |= TF_ACKNOW;
	        /* Section 28.6 of [WS95] claims the right side of the
		   logical and in the "if" below is needless.  rja */
		if (tiflags & TH_ACK && SEQ_GT(tp->snd_una, tp->iss)) {
			tcpstat.tcps_connects++;
			soisconnected(so);
			tp->t_state = TCPS_ESTABLISHED;
			/* Do window scaling on this connection? */
			if ((tp->t_flags & (TF_RCVD_SCALE|TF_REQ_SCALE)) ==
				(TF_RCVD_SCALE|TF_REQ_SCALE)) {
				tp->snd_scale = tp->requested_s_scale;
				tp->rcv_scale = tp->request_r_scale;
			}
#ifdef INET6
			if (is_ipv6)
			  (void) tcpv6_reass(tp, (struct tcpipv6hdr *) NULL,
					     (struct mbuf *) NULL, NULL);
			else
#endif /* INET6 */
			(void) tcp_reass(tp, (struct tcpiphdr *) NULL,
				(struct mbuf *) NULL);
			/*
			 * if we didn't have to retransmit the SYN,
			 * use its rtt as our initial srtt & rtt var.
			 */
			if (tp->t_rtt)
				tcp_xmit_timer(tp, tp->t_rtt);
		} else
			tp->t_state = TCPS_SYN_RECEIVED;

trimthenstep6:
		/*
		 * Advance th->th_seq to correspond to first data byte.
		 * If data, trim to stay within window,
		 * dropping FIN if necessary.
		 */
		th->th_seq++;

		if (tlen > tp->rcv_wnd) {
			todrop = tlen - tp->rcv_wnd;
			m_adj(m, -todrop);
			tlen = tp->rcv_wnd;
			tiflags &= ~TH_FIN;
			tcpstat.tcps_rcvpackafterwin++;
			tcpstat.tcps_rcvbyteafterwin += todrop;
		}

		tp->snd_wl1 = th->th_seq - 1;
		tp->rcv_up  = th->th_seq;

		goto step6;
	}

	/*
	 * States other than LISTEN or SYN_SENT.
	 * First check timestamp, if present.
	 * Then check that at least some bytes of segment are within 
	 * receive window.  If segment begins before rcv_nxt,
	 * drop leading data (and SYN); if nothing left, just ack.
	 * 
	 * RFC 1323 PAWS: If we have a timestamp reply on this segment
	 * and it's less than ts_recent, drop it.
	 */
	if (ts_present && (tiflags & TH_RST) == 0 && tp->ts_recent &&
	    TSTMP_LT(ts_val, tp->ts_recent)) {

		/* Check to see if ts_recent is over 24 days old.  */
		if ((int)(tcp_now - tp->ts_recent_age) > TCP_PAWS_IDLE) {
			/*
			 * Invalidate ts_recent.  If this segment updates
			 * ts_recent, the age will be reset later and ts_recent
			 * will get a valid value.  If it does not, setting
			 * ts_recent to zero will at least satisfy the
			 * requirement that zero be placed in the timestamp
			 * echo reply when ts_recent isn't valid.  The
			 * age isn't reset until we get a valid ts_recent
			 * because we don't want out-of-order segments to be
			 * dropped when ts_recent is old.
			 */
			tp->ts_recent = 0;
		} else {
			tcpstat.tcps_rcvduppack++;
			tcpstat.tcps_rcvdupbyte += tlen;
			tcpstat.tcps_pawsdrop++;
			goto dropafterack;
		}
	}

	todrop = tp->rcv_nxt - th->th_seq;

	if (todrop > 0) {
		if (tiflags & TH_SYN) {
			tiflags &= ~TH_SYN;
			th->th_seq++;
			if (th->th_urp > 1) 
				th->th_urp--;
			else
				tiflags &= ~TH_URG;
			todrop--;
		}

		/* This code has been corrected as suggested in Section
		   28.8 of [WS95].  Consult that reference for more
		   information on the changes.  rja  */

		if (todrop > tlen ||
		    todrop == tlen && (tiflags & TH_FIN) == 0) {
		  /* 
		   * Any valid FIN must be to the left of the window.
		   * At this point the FIN must be either duplicate
		   * or out of sequence, so drop it.
		   */
		  tiflags &= ~TH_FIN;

		  /*
		   * Send an ACK to resynchronise and drop any data.
		   * But keep on processing for RST or ACK.
		   */
		  tp->t_flags |= TF_ACKNOW;
		  todrop = tlen;
		  tcpstat.tcps_rcvdupbyte += todrop;
		  tcpstat.tcps_rcvduppack++;
		} else {
		  tcpstat.tcps_rcvpartduppack++;
		  tcpstat.tcps_rcvpartdupbyte += todrop;
		}
		m_adj(m, todrop);
		th->th_seq += todrop;
		tlen -= todrop;
		if (th->th_urp > todrop)
			th->th_urp -= todrop;
		else {
			tiflags &= ~TH_URG;
			th->th_urp = 0;
		}
	}

	/*
	 * If new data are received on a connection after the
	 * user processes are gone, then RST the other end.
	 */
	if ((so->so_state & SS_NOFDREF) &&
	    tp->t_state > TCPS_CLOSE_WAIT && tlen) {
		tp = tcp_close(tp);
		tcpstat.tcps_rcvafterclose++;
		goto dropwithreset;
	}

	/*
	 * If segment ends after window, drop trailing data
	 * (and PUSH and FIN); if nothing left, just ACK.
	 */
	todrop = (th->th_seq + tlen) - (tp->rcv_nxt + tp->rcv_wnd);
	if (todrop > 0) {
		tcpstat.tcps_rcvpackafterwin++;
		if (todrop >= tlen) {
			tcpstat.tcps_rcvbyteafterwin += tlen;
			/*
			 * If a new connection request is received
			 * while in TIME_WAIT, drop the old connection
			 * and start over if the sequence numbers
			 * are above the previous ones.
			 */
			if (tiflags & TH_SYN &&
			    tp->t_state == TCPS_TIME_WAIT &&
			    SEQ_GT(th->th_seq, tp->rcv_nxt)) {
				iss = tp->rcv_nxt + TCP_ISSINCR;
				tp = tcp_close(tp);
				goto findpcb;
			}
			/*
			 * If window is closed can only take segments at
			 * window edge, and have to drop data and PUSH from
			 * incoming segments.  Continue processing, but
			 * remember to ack.  Otherwise, drop segment
			 * and ack.
			 */
			if (tp->rcv_wnd == 0 && th->th_seq == tp->rcv_nxt) {
				tp->t_flags |= TF_ACKNOW;
				tcpstat.tcps_rcvwinprobe++;
			} else
				goto dropafterack;
		} else
			tcpstat.tcps_rcvbyteafterwin += todrop;
		m_adj(m, -todrop);
		tlen -= todrop;
		tiflags &= ~(TH_PUSH|TH_FIN);
	}

	/*
	 * If last ACK falls within this segment's sequence numbers,
	 * record its timestamp.
	 */
	/* Revised to follow [Bra93], per [WS95].  rja */
	if (ts_present &&
 	    TSTMP_GEQ(ts_val, tp->ts_recent) && 
	    SEQ_LEQ(th->th_seq, tp->last_ack_sent)) 
	  {
		tp->ts_recent_age = tcp_now;
		tp->ts_recent = ts_val;
	}

	/*
	 * If the RST bit is set examine the state:
	 *    SYN_RECEIVED STATE:
	 *	If passive open, return to LISTEN state.
	 *	If active open, inform user that connection was refused.
	 *    ESTABLISHED, FIN_WAIT_1, FIN_WAIT2, CLOSE_WAIT STATES:
	 *	Inform user that connection was reset, and close tcb.
	 *    CLOSING, LAST_ACK, TIME_WAIT STATES
	 *	Close the tcb.
	 */
	if (tiflags&TH_RST) switch (tp->t_state) {

	case TCPS_SYN_RECEIVED:
		so->so_error = ECONNREFUSED;
		goto close;

	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
		so->so_error = ECONNRESET;
	close:
		tp->t_state = TCPS_CLOSED;
		tcpstat.tcps_drops++;
		tp = tcp_close(tp);
		goto drop;

	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:
		tp = tcp_close(tp);
		goto drop;
	}

	/*
	 * If a SYN is in the window, then this is an
	 * error and we send an RST and drop the connection.
	 */
	if (tiflags & TH_SYN) {
		tp = tcp_drop(tp, ECONNRESET);
		goto dropwithreset;
	}

	/*
	 * If the ACK bit is off we drop the segment and return.
	 */
	if ((tiflags & TH_ACK) == 0)
		goto drop;
	
	/*
	 * Ack processing.
	 */
	switch (tp->t_state) {

	/*
	 * In SYN_RECEIVED state if the ack ACKs our SYN then enter
	 * ESTABLISHED state and continue processing, otherwise
	 * send an RST.
	 */
	case TCPS_SYN_RECEIVED:
		if (SEQ_GT(tp->snd_una, th->th_ack) ||
		    SEQ_GT(th->th_ack, tp->snd_max))
			goto dropwithreset;
		tcpstat.tcps_connects++;
		soisconnected(so);
		/*
		 * Perhaps should update IPv6 Neighbor Reachability here.
		 * As always, performance is an issue.
		 */
		tp->t_state = TCPS_ESTABLISHED;
		/* Do window scaling? */
		if ((tp->t_flags & (TF_RCVD_SCALE|TF_REQ_SCALE)) ==
			(TF_RCVD_SCALE|TF_REQ_SCALE)) {
			tp->snd_scale = tp->requested_s_scale;
			tp->rcv_scale = tp->request_r_scale;
		}
#ifdef INET6
		if (is_ipv6)
		  (void) tcpv6_reass(tp, (struct tcpipv6hdr *) NULL,
				     (struct mbuf *) NULL, NULL);
		else
#endif /* INET6 */
		(void) tcp_reass(tp, (struct tcpiphdr *)0, (struct mbuf *)0);
		tp->snd_wl1 = th->th_seq - 1;
		/* fall into ... */

	/*
	 * In ESTABLISHED state: drop duplicate ACKs; ACK out of range
	 * ACKs.  If the ack is in the range
	 *	tp->snd_una < ti->ti_ack <= tp->snd_max
	 * then advance tp->snd_una to ti->ti_ack and drop
	 * data from the retransmission queue.  If this ACK reflects
	 * more up to date window information we update our window information.
	 */
	case TCPS_ESTABLISHED:
	case TCPS_FIN_WAIT_1:
	case TCPS_FIN_WAIT_2:
	case TCPS_CLOSE_WAIT:
	case TCPS_CLOSING:
	case TCPS_LAST_ACK:
	case TCPS_TIME_WAIT:

		if (SEQ_LEQ(th->th_ack, tp->snd_una)) {
		  if (tlen == 0 && tiwin == tp->snd_wnd) {
				tcpstat.tcps_rcvdupack++;
				/*
				 * If we have outstanding data (other than
				 * a window probe), this is a completely
				 * duplicate ack (ie, window info didn't
				 * change), the ack is the biggest we've
				 * seen and we've seen exactly our rexmt
				 * threshhold of them, assume a packet
				 * has been dropped and retransmit it.
				 * Kludge snd_nxt & the congestion
				 * window so we send only this one
				 * packet.
				 *
				 * We know we're losing at the current
				 * window size so do congestion avoidance
				 * (set ssthresh to half the current window
				 * and pull our congestion window back to
				 * the new ssthresh).
				 *
				 * Dup acks mean that packets have left the
				 * network (they're now cached at the receiver) 
				 * so bump cwnd by the amount in the receiver
				 * to keep a constant cwnd packets in the
				 * network.
				 */
				if (tp->t_timer[TCPT_REXMT] == 0 ||
				    th->th_ack != tp->snd_una)
					tp->t_dupacks = 0;
				else if (++tp->t_dupacks == tcprexmtthresh) {
					tcp_seq onxt = tp->snd_nxt;
					u_int win =
					    min(tp->snd_wnd, tp->snd_cwnd) / 2 /
						tp->t_maxseg;

					if (win < 2)
						win = 2;
					tp->snd_ssthresh = win * tp->t_maxseg;
					tp->t_timer[TCPT_REXMT] = 0;
					tp->t_rtt = 0;
					tp->snd_nxt = th->th_ack;
					tp->snd_cwnd = tp->t_maxseg;
					(void) tcp_output(tp);
					tp->snd_cwnd = tp->snd_ssthresh +
					       tp->t_maxseg * tp->t_dupacks;
					if (SEQ_GT(onxt, tp->snd_nxt))
						tp->snd_nxt = onxt;
					goto drop;
				} else if (tp->t_dupacks > tcprexmtthresh) {
					tp->snd_cwnd += tp->t_maxseg;
					(void) tcp_output(tp);
					goto drop;
				}
			} else
				tp->t_dupacks = 0;
			break;
		}
		/*
		 * If the congestion window was inflated to account
		 * for the other side's cached packets, retract it.
		 */
		if (tp->t_dupacks > tcprexmtthresh &&
		    tp->snd_cwnd > tp->snd_ssthresh)
			tp->snd_cwnd = tp->snd_ssthresh;
		tp->t_dupacks = 0;
		if (SEQ_GT(th->th_ack, tp->snd_max)) {
			tcpstat.tcps_rcvacktoomuch++;
			goto dropafterack;
		}
		acked = th->th_ack - tp->snd_una;
		tcpstat.tcps_rcvackpack++;
		tcpstat.tcps_rcvackbyte += acked;

		/*
		 * If we have a timestamp reply, update smoothed
		 * round trip time.  If no timestamp is present but
		 * transmit timer is running and timed sequence
		 * number was acked, update smoothed round trip time.
		 * Since we now have an rtt measurement, cancel the
		 * timer backoff (cf., Phil Karn's retransmit alg.).
		 * Recompute the initial retransmit timer.
		 */
		if (ts_present)
			tcp_xmit_timer(tp, tcp_now-ts_ecr+1);
		else if (tp->t_rtt && SEQ_GT(th->th_ack, tp->t_rtseq))
			tcp_xmit_timer(tp,tp->t_rtt);

		/*
		 * If all outstanding data is acked, stop retransmit
		 * timer and remember to restart (more output or persist).
		 * If there is more data to be acked, restart retransmit
		 * timer, using current (possibly backed-off) value.
		 */
		if (th->th_ack == tp->snd_max) {
			tp->t_timer[TCPT_REXMT] = 0;
			needoutput = 1;
		} else if (tp->t_timer[TCPT_PERSIST] == 0)
			tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;
		/*
		 * When new data is acked, open the congestion window.
		 * If the window gives us less than ssthresh packets
		 * in flight, open exponentially (maxseg per packet).
		 * Otherwise open linearly: maxseg per window
		 * (maxseg^2 / cwnd per packet).
		 *
		 * [WS95] reports that [Floyd94] describes why the 
		 * older BSD practice of adding (maxseg/8) is wrong.
		 *   rja
		 */
		{
		register u_int cw = tp->snd_cwnd;
		register u_int incr = tp->t_maxseg;

		if (cw > tp->snd_ssthresh)
			incr = incr * incr / cw;
		tp->snd_cwnd = min(cw + incr, TCP_MAXWIN<<tp->snd_scale);
		}
		if (acked > so->so_snd.sb_cc) {
			tp->snd_wnd -= so->so_snd.sb_cc;
			sbdrop(&so->so_snd, (int)so->so_snd.sb_cc);
			ourfinisacked = 1;
		} else {
			sbdrop(&so->so_snd, acked);
			tp->snd_wnd -= acked;
			ourfinisacked = 0;
		}
		if (so->so_snd.sb_flags & SB_NOTIFY)
			sowwakeup(so);
		tp->snd_una = th->th_ack;
		if (SEQ_LT(tp->snd_nxt, tp->snd_una))
			tp->snd_nxt = tp->snd_una;

		switch (tp->t_state) {

		/*
		 * In FIN_WAIT_1 STATE in addition to the processing
		 * for the ESTABLISHED state if our FIN is now acknowledged
		 * then enter FIN_WAIT_2.
		 */
		case TCPS_FIN_WAIT_1:
			if (ourfinisacked) {
				/*
				 * If we can't receive any more
				 * data, then closing user can proceed.
				 * Starting the timer is contrary to the
				 * specification, but if we don't get a FIN
				 * we'll hang forever.
				 */
				if (so->so_state & SS_CANTRCVMORE) {
					soisdisconnected(so);
					tp->t_timer[TCPT_2MSL] = tcp_maxidle;
				}
				tp->t_state = TCPS_FIN_WAIT_2;
			}
			break;

	 	/*
		 * In CLOSING STATE in addition to the processing for
		 * the ESTABLISHED state if the ACK acknowledges our FIN
		 * then enter the TIME-WAIT state, otherwise ignore
		 * the segment.
		 */
		case TCPS_CLOSING:
			if (ourfinisacked) {
				tp->t_state = TCPS_TIME_WAIT;
				tcp_canceltimers(tp);
				tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
				soisdisconnected(so);
			}
			break;

		/*
		 * In LAST_ACK, we may still be waiting for data to drain
		 * and/or to be acked, as well as for the ack of our FIN.
		 * If our FIN is now acknowledged, delete the TCB,
		 * enter the closed state and return.
		 */
		case TCPS_LAST_ACK:
			if (ourfinisacked) {
				tp = tcp_close(tp);
				goto drop;
			}
			break;

		/*
		 * In TIME_WAIT state the only thing that should arrive
		 * is a retransmission of the remote FIN.  Acknowledge
		 * it and restart the finack timer.
		 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			goto dropafterack;
		}
	}

step6:
	/*
	 * Update window information.
	 * Don't look at window if no ACK: TAC's send garbage on first SYN.
	 */
	if ((tiflags & TH_ACK) &&
	    (SEQ_LT(tp->snd_wl1, th->th_seq) || 
	    (tp->snd_wl1 == th->th_seq && (SEQ_LT(tp->snd_wl2, th->th_ack) ||
	     (tp->snd_wl2 == th->th_ack && tiwin > tp->snd_wnd))))) {
		/* keep track of pure window updates */
		if (!tlen && tp->snd_wl2 == th->th_ack && tiwin > tp->snd_wnd)
			tcpstat.tcps_rcvwinupd++;
		tp->snd_wnd = tiwin;
		tp->snd_wl1 = th->th_seq;
		tp->snd_wl2 = th->th_ack;
		if (tp->snd_wnd > tp->max_sndwnd)
			tp->max_sndwnd = tp->snd_wnd;
		needoutput = 1;
	}

	/*
	 * Process segments with URG.
	 */
	if ((tiflags & TH_URG) && th->th_urp &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {
		/*
		 * This is a kludge, but if we receive and accept
		 * random urgent pointers, we'll crash in
		 * soreceive.  It's hard to imagine someone
		 * actually wanting to send this much urgent data.
		 */
		if (th->th_urp + so->so_rcv.sb_cc > sb_max) {
			th->th_urp = 0;			/* XXX */
			tiflags &= ~TH_URG;		/* XXX */
			goto dodata;			/* XXX */
		}
		/*
		 * If this segment advances the known urgent pointer,
		 * then mark the data stream.  This should not happen
		 * in CLOSE_WAIT, CLOSING, LAST_ACK or TIME_WAIT STATES since
		 * a FIN has been received from the remote side. 
		 * In these states we ignore the URG.
		 *
		 * According to RFC961 (Assigned Protocols),
		 * the urgent pointer points to the last octet
		 * of urgent data.  We continue, however,
		 * to consider it to indicate the first octet
		 * of data past the urgent section as the original 
		 * spec states (in one of two places).
		 */
		if (SEQ_GT(th->th_seq + th->th_urp, tp->rcv_up)) {
			tp->rcv_up = th->th_seq + th->th_urp;
			so->so_oobmark = so->so_rcv.sb_cc +
			    (tp->rcv_up - tp->rcv_nxt) - 1;
			if (so->so_oobmark == 0)
				so->so_state |= SS_RCVATMARK;
			sohasoutofband(so);
			tp->t_oobflags &= ~(TCPOOB_HAVEDATA | TCPOOB_HADDATA);
		}
		/*
		 * Remove out of band data so doesn't get presented to user.
		 * This can happen independent of advancing the URG pointer,
		 * but if two URG's are pending at once, some out-of-band
		 * data may creep in... ick.
		 */
		if (th->th_urp <= (u_long)tlen
#ifdef SO_OOBINLINE
		     && (so->so_options & SO_OOBINLINE) == 0
#endif /* SO_OOBINLINE */
		     )
			tcp_pulloutofband(so, th->th_urp, m);
	} else
		/*
		 * If no out of band data is expected,
		 * pull receive urgent pointer along
		 * with the receive window.
		 */
		if (SEQ_GT(tp->rcv_nxt, tp->rcv_up))
			tp->rcv_up = tp->rcv_nxt;
dodata:							/* XXX */

	/*
	 * Process the segment text, merging it into the TCP sequencing queue,
	 * and arranging for acknowledgment of receipt if necessary.
	 * This process logically involves adjusting tp->rcv_wnd as data
	 * is presented to the user (this happens in tcp_usrreq.c,
	 * case PRU_RCVD).  If a FIN has already been received on this
	 * connection then we just ignore the text.
	 */
	if ((tlen || (tiflags & TH_FIN)) &&
	    TCPS_HAVERCVDFIN(tp->t_state) == 0) {
#ifdef INET6
	  if (is_ipv6) {
	    DPRINTF(IDL_GROSS_EVENT,("Calling TCP_V6_REASS, tlen = %d\n",tlen));
	    TCP_V6_REASS(tp, ti6, m, so, tiflags, &tlen);
	  } else
#endif /* INET6 */
	    {
	      /* Must be IPv4 */
	      ti->ti_len = tlen;	  /* For the benefit of TCP_REASS */
	      TCP_REASS(tp, ti, m, so, tiflags);
	    } /* endif is_ipv6 */

		/*
		 * Note the amount of data that peer has sent into
		 * our window, in order to estimate the sender's
		 * buffer size.
		 */
#ifndef INET6
#if (BSD <= 43)
		/* 
		 * variable len never referenced again in modern BSD,
		 * so why bother computing it ??
		 */
		len = so->so_rcv.sb_hiwat - (tp->rcv_adv - tp->rcv_nxt);
#endif
#endif /* INET6 */
	} else {
		m_freem(m);
		tiflags &= ~TH_FIN;
	}

	/*
	 * If FIN is received ACK the FIN and let the user know
	 * that the connection is closing.
	 */
	if (tiflags & TH_FIN) {
		if (TCPS_HAVERCVDFIN(tp->t_state) == 0) {
			socantrcvmore(so);
			tp->t_flags |= TF_ACKNOW;
			tp->rcv_nxt++;
		}
		switch (tp->t_state) {

	 	/*
		 * In SYN_RECEIVED and ESTABLISHED STATES
		 * enter the CLOSE_WAIT state.
		 */
		case TCPS_SYN_RECEIVED:
		case TCPS_ESTABLISHED:
			tp->t_state = TCPS_CLOSE_WAIT;
			break;

	 	/*
		 * If still in FIN_WAIT_1 STATE FIN has not been acked so
		 * enter the CLOSING state.
		 */
		case TCPS_FIN_WAIT_1:
			tp->t_state = TCPS_CLOSING;
			break;

	 	/*
		 * In FIN_WAIT_2 state enter the TIME_WAIT state,
		 * starting the time-wait timer, turning off the other 
		 * standard timers.
		 */
		case TCPS_FIN_WAIT_2:
			tp->t_state = TCPS_TIME_WAIT;
			tcp_canceltimers(tp);
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			soisdisconnected(so);
			break;

		/*
		 * In TIME_WAIT state restart the 2 MSL time_wait timer.
		 */
		case TCPS_TIME_WAIT:
			tp->t_timer[TCPT_2MSL] = 2 * TCPTV_MSL;
			break;
		}
	}
#ifdef TCPDEBUG

	if (so->so_options & SO_DEBUG)
#ifdef INET6
	  if (tp->pf == PF_INET6)
	    tcp_trace(TA_INPUT, ostate, tp,
		      (struct tcpiphdr *) &tcp_saveti6, 0, tlen);
	  else
#endif /* INET6 */
	    tcp_trace(TA_INPUT, ostate, tp, &tcp_saveti, 0, tlen);

#endif /* TCPDEBUG */

	/*
	 * If this is a small packet, then ACK now - with Nagel
	 *      congestion avoidance sender won't send more until
	 *      he gets an ACK.
	 */
	if (tlen && ((unsigned) tlen < tp->t_maxseg))
		tp->t_flags |= TF_ACKNOW;

	/*
	 * Return any desired output.
	 */
	if (needoutput || (tp->t_flags & TF_ACKNOW))
		(void) tcp_output(tp);
	return;

dropafterack:
	/*
	 * Generate an ACK dropping incoming segment if it occupies
	 * sequence space, where the ACK reflects our state.
	 */
	if (tiflags & TH_RST)
		goto drop;
	m_freem(m);
	tp->t_flags |= TF_ACKNOW;
	(void) tcp_output(tp);
	return;

dropwithreset:
	/*
	 * Generate a RST, dropping incoming segment.
	 * Make ACK acceptable to originator of segment.
	 * Don't bother to respond if destination was broadcast/multicast.
	 */
#ifndef INET6
	if ((tiflags & TH_RST) || m->m_flags & (M_BCAST|M_MCAST) ||
	    IN_MULTICAST(ntohl(ti->ti_dst.s_addr)))
		goto drop;
#else /* INET6 */
	if ((tiflags & TH_RST) || m->m_flags & (M_BCAST|M_MCAST))
	    goto drop;
	if (is_ipv6) {
	  ti = (struct tcpiphdr *)ti6;  /* For following calls to tcp_respond */
	  if (IS_IN6_MULTICAST(((struct ipv6 *)ti6)->ipv6_dst))
	    goto drop;
	} else {
	    if (IN_MULTICAST(ntohl(ti->ti_dst.s_addr)))
	      goto drop;
	}

#endif /* INET6 */

	if (tiflags & TH_ACK)
		tcp_respond(tp, ti, m, (tcp_seq)0, th->th_ack, TH_RST);
	else {
	  if (tiflags & TH_SYN)
	    tlen++;
	  tcp_respond(tp, ti, m, th->th_seq+tlen, (tcp_seq)0,TH_RST|TH_ACK);
	}
	/* destroy temporarily created socket */
	if (dropsocket)
		(void) soabort(so);
	return;

drop:
	/*
	 * Drop space held by incoming segment and return.
	 */
#ifdef TCPDEBUG
	if (tp && (tp->t_inpcb->inp_socket->so_options & SO_DEBUG))
#ifdef INET6
	  if (tp->pf == PF_INET6)
	        tcp_trace(TA_DROP, ostate, tp, (struct tcpiphdr *)
			  &tcp_saveti6, 0, tlen);
	  else
#endif /* INET6 */
		tcp_trace(TA_DROP, ostate, tp, &tcp_saveti, 0, tlen);
#endif /* TCPDEBUG */
	m_freem(m);
	/* destroy temporarily created socket */
	if (dropsocket)
		(void) soabort(so);
	return;
#ifndef TUBA_INCLUDE
}

void
tcp_dooptions(tp, cp, cnt, th, ts_present, ts_val, ts_ecr)
	struct tcpcb *tp;
	u_char *cp;
	int cnt;
	struct tcphdr *th;
	int *ts_present;
	u_long *ts_val, *ts_ecr;
{
	u_short mss;
	int opt, optlen;

	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[0];
		if (opt == TCPOPT_EOL)
			break;
		if (opt == TCPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[1];
			if (optlen <= 0)
				break;
		}
		switch (opt) {

		default:
			continue;

		case TCPOPT_MAXSEG:
			if (optlen != TCPOLEN_MAXSEG)
				continue;
			if (!(th->th_flags & TH_SYN))
				continue;
			bcopy((char *) cp + 2, (char *) &mss, sizeof(mss));
			NTOHS(mss);
			(void) tcp_mss(tp, mss);	/* sets t_maxseg */
			break;

		case TCPOPT_WINDOW:
			if (optlen != TCPOLEN_WINDOW)
				continue;
			if (!(th->th_flags & TH_SYN))
				continue;
			tp->t_flags |= TF_RCVD_SCALE;
			tp->requested_s_scale = min(cp[2], TCP_MAX_WINSHIFT);
			break;

		case TCPOPT_TIMESTAMP:
			if (optlen != TCPOLEN_TIMESTAMP)
				continue;
			*ts_present = 1;
			bcopy((char *)cp + 2, (char *) ts_val, sizeof(*ts_val));
			NTOHL(*ts_val);
			bcopy((char *)cp + 6, (char *) ts_ecr, sizeof(*ts_ecr));
			NTOHL(*ts_ecr);

			/* 
			 * A timestamp received in a SYN makes
			 * it ok to send timestamp requests and replies.
			 */
			if (th->th_flags & TH_SYN) {
				tp->t_flags |= TF_RCVD_TSTMP;
				tp->ts_recent = *ts_val;
				tp->ts_recent_age = tcp_now;
			}
			break;
		}
	}
}

/*
 * Pull out of band byte out of a segment so
 * it doesn't appear in the user's data queue.
 * It is still reflected in the segment length for
 * sequencing purposes.
 */
void
tcp_pulloutofband(so, urgent, m)
	struct socket *so;
        int urgent;
	register struct mbuf *m;
{
	int cnt = urgent - 1;

	while (cnt >= 0) {
		if (m->m_len > cnt) {
			char *cp = mtod(m, caddr_t) + cnt;
			struct tcpcb *tp = sototcpcb(so);

			tp->t_iobc = *cp;
			tp->t_oobflags |= TCPOOB_HAVEDATA;
			bcopy(cp+1, cp, (unsigned)(m->m_len - cnt - 1));
			m->m_len--;
			return;
		}
		cnt -= m->m_len;
		m = m->m_next;
		if (m == 0)
			break;
	}
	panic("tcp_pulloutofband");
}

/*
 * Collect new round-trip time estimate
 * and update averages and current timeout.
 */
void
tcp_xmit_timer(tp, rtt)
	register struct tcpcb *tp;
	short rtt;
{
	register short delta;

	tcpstat.tcps_rttupdated++;
	if (tp->t_srtt != 0) {
		/*
		 * srtt is stored as fixed point with 3 bits after the
		 * binary point (i.e., scaled by 8).  The following magic
		 * is equivalent to the smoothing algorithm in rfc793 with
		 * an alpha of .875 (srtt = rtt/8 + srtt*7/8 in fixed
		 * point).  Adjust rtt to origin 0.
		 */
		delta = rtt - 1 - (tp->t_srtt >> TCP_RTT_SHIFT);
		if ((tp->t_srtt += delta) <= 0)
			tp->t_srtt = 1;
		/*
		 * We accumulate a smoothed rtt variance (actually, a
		 * smoothed mean difference), then set the retransmit
		 * timer to smoothed rtt + 4 times the smoothed variance.
		 * rttvar is stored as fixed point with 2 bits after the
		 * binary point (scaled by 4).  The following is
		 * equivalent to rfc793 smoothing with an alpha of .75
		 * (rttvar = rttvar*3/4 + |delta| / 4).  This replaces
		 * rfc793's wired-in beta.
		 */
		if (delta < 0)
			delta = -delta;
		delta -= (tp->t_rttvar >> TCP_RTTVAR_SHIFT);
		if ((tp->t_rttvar += delta) <= 0)
			tp->t_rttvar = 1;
	} else {
		/* 
		 * No rtt measurement yet - use the unsmoothed rtt.
		 * Set the variance to half the rtt (so our first
		 * retransmit happens at 3*rtt).
		 */
		tp->t_srtt = rtt << TCP_RTT_SHIFT;
		tp->t_rttvar = rtt << (TCP_RTTVAR_SHIFT - 1);
	}
	tp->t_rtt = 0;
	tp->t_rxtshift = 0;

	/*
	 * the retransmit should happen at rtt + 4 * rttvar.
	 * Because of the way we do the smoothing, srtt and rttvar
	 * will each average +1/2 tick of bias.  When we compute
	 * the retransmit timer, we want 1/2 tick of rounding and
	 * 1 extra tick because of +-1/2 tick uncertainty in the
	 * firing of the timer.  The bias will give us exactly the
	 * 1.5 tick we need.  But, because the bias is
	 * statistical, we have to test that we don't drop below
	 * the minimum feasible timer (which is 2 ticks).
	 */
	TCPT_RANGESET(tp->t_rxtcur, TCP_REXMTVAL(tp),
	    (short) tp->t_rttmin, TCPTV_REXMTMAX);
	
	/*
	 * We received an ack for a packet that wasn't retransmitted;
	 * it is probably safe to discard any error indications we've
	 * received recently.  This isn't quite right, but close enough
	 * for now (a route might have failed after we sent a segment,
	 * and the return path might not be symmetrical).
	 */
	tp->t_softerror = 0;
}

/*
 * Determine a reasonable value for maxseg size.
 * If the route is known, check route for mtu.
 * If none, use an mss that can be handled on the outgoing
 * interface without forcing IP to fragment; if bigger than
 * an mbuf cluster (MCLBYTES), round down to nearest multiple of MCLBYTES
 * to utilize large mbufs.  If no route is found, route has no mtu,
 * or the destination isn't local, use a default, hopefully conservative
 * size (usually 512 or the default IP max size, but no more than the mtu
 * of the interface), as we can't discover anything about intervening
 * gateways or networks.  We also initialize the congestion/slow start
 * window to be a single segment if the destination isn't local.
 * While looking at the routing entry, we also initialize other path-dependent
 * parameters from pre-set or cached values in the routing entry.
 */
int
tcp_mss(tp, offer)
	register struct tcpcb *tp;
	u_int offer;
{
	struct route *ro;
	register struct rtentry *rt;
	struct ifnet *ifp;
	register int rtt, mss;
	u_long bufsize;
	struct inpcb *inp;
	struct socket *so;
	extern int tcp_mssdflt;

	inp = tp->t_inpcb;
	ro = &inp->inp_route;
	so = inp->inp_socket;

	if ((rt = ro->ro_rt) == (struct rtentry *)0) {
	  /* No route yet, so try to acquire one */
	  
#ifdef INET6
	  /*
	   * Get a new IPv6 route if an IPv6 destination, otherwise, get
	   * and IPv4 route (including those pesky IPv4-mapped addresses).
	   */
	  bzero(ro,sizeof(struct route6));
	  if (sotopf(so) == AF_INET6)
	    {
	      if (IS_IN6_IPV4_MAPPED(inp->inp_faddr6))
		{
		  /* Get an IPv4 route. */
		  ro->ro_dst.sa_family = AF_INET;
		  ro->ro_dst.sa_len = sizeof(ro->ro_dst);
		  ((struct sockaddr_in *) &ro->ro_dst)->sin_addr =
		    inp->inp_faddr;
		  rtalloc(ro);
		}
	      else
		{
		  ro->ro_dst.sa_family = AF_INET6;
		  ro->ro_dst.sa_len = sizeof(struct sockaddr_in6);
		  ((struct sockaddr_in6 *) &ro->ro_dst)->sin6_addr =
		    inp->inp_faddr6;
		  rtalloc(ro);
		}
	    }
	  else
#endif
	    if (inp->inp_faddr.s_addr != INADDR_ANY) {
	      ro->ro_dst.sa_family = AF_INET;
	      ro->ro_dst.sa_len = sizeof(ro->ro_dst);
	      ((struct sockaddr_in *) &ro->ro_dst)->sin_addr =
		inp->inp_faddr;
	      rtalloc(ro);
	    }
	  if ((rt = ro->ro_rt) == (struct rtentry *)0)
	    return (tcp_mssdflt);
	}
	ifp = rt->rt_ifp;

#ifdef RTV_MTU	/* if route characteristics exist ... */
	/*
	 * While we're here, check if there's an initial rtt
	 * or rttvar.  Convert from the route-table units
	 * to scaled multiples of the slow timeout timer.
	 */
	if (tp->t_srtt == 0 && (rtt = rt->rt_rmx.rmx_rtt)) {
		/*
		 * XXX the lock bit for MTU indicates that the value
		 * is also a minimum value; this is subject to time.
		 */
		if (rt->rt_rmx.rmx_locks & RTV_RTT)
			tp->t_rttmin = rtt / (RTM_RTTUNIT / PR_SLOWHZ);
		tp->t_srtt = rtt / (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTT_SCALE));
		if (rt->rt_rmx.rmx_rttvar)
			tp->t_rttvar = rt->rt_rmx.rmx_rttvar /
			    (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTTVAR_SCALE));
		else
			/* default variation is +- 1 rtt */
			tp->t_rttvar =
			    tp->t_srtt * TCP_RTTVAR_SCALE / TCP_RTT_SCALE;
		TCPT_RANGESET(tp->t_rxtcur,
		    ((tp->t_srtt >> 2) + tp->t_rttvar) >> 1,
		    (short)tp->t_rttmin, TCPTV_REXMTMAX);
	}
	/*
	 * if there's an mtu associated with the route, use it
	 */
	if (rt->rt_rmx.rmx_mtu)
#ifdef INET6
	  /*
	   * One may wish to lower MSS to take into account options,
	   * especially security-related options.
	   */
		if (tp->pf == AF_INET6)
			mss = rt->rt_rmx.rmx_mtu - sizeof(struct tcpipv6hdr);
		else
#endif
		mss = rt->rt_rmx.rmx_mtu - sizeof(struct tcpiphdr);
	else
#endif /* RTV_MTU */
	{
		mss = ifp->if_mtu - sizeof(struct tcpiphdr);
#if	(MCLBYTES & (MCLBYTES - 1)) == 0
		if (mss > MCLBYTES)
			mss &= ~(MCLBYTES-1);
#else
		if (mss > MCLBYTES)
			mss = mss / MCLBYTES * MCLBYTES;
#endif
		/* This check is okay, because this codepath (AFAIK) is
		   only executed for IPv4. */
		if (!in_localaddr(inp->inp_faddr))
			mss = min(mss, tcp_mssdflt);
	}
	/*
	 * The current mss, t_maxseg, is initialized to the default value.
	 * If we compute a smaller value, reduce the current mss.
	 * If we compute a larger value, return it for use in sending
	 * a max seg size option, but don't store it for use
	 * unless we received an offer at least that large from peer.
	 * However, do not accept offers under 32 bytes.
	 */
	if (offer)
		mss = min(mss, offer);
	mss = max(mss, 32);		/* sanity */
	if (mss < tp->t_maxseg || offer != 0) {
		/*
		 * If there's a pipesize, change the socket buffer
		 * to that size.  Make the socket buffers an integral
		 * number of mss units; if the mss is larger than
		 * the socket buffer, decrease the mss.
		 */
#ifdef RTV_SPIPE
		if ((bufsize = rt->rt_rmx.rmx_sendpipe) == 0)
#endif
			bufsize = so->so_snd.sb_hiwat;
		if (bufsize < mss)
			mss = bufsize;
		else {
			bufsize = roundup(bufsize, mss);
			if (bufsize > sb_max)
				bufsize = sb_max;
			(void)sbreserve(&so->so_snd, bufsize);
		}
		tp->t_maxseg = mss;

#ifdef RTV_RPIPE
		if ((bufsize = rt->rt_rmx.rmx_recvpipe) == 0)
#endif
			bufsize = so->so_rcv.sb_hiwat;
		if (bufsize > mss) {
			bufsize = roundup(bufsize, mss);
			if (bufsize > sb_max)
				bufsize = sb_max;
			(void)sbreserve(&so->so_rcv, bufsize);
		}
	}
	tp->snd_cwnd = mss;

#ifdef RTV_SSTHRESH
	if (rt->rt_rmx.rmx_ssthresh) {
		/*
		 * There's some sort of gateway or interface
		 * buffer limit on the path.  Use this to set
		 * the slow start threshhold, but set the
		 * threshold to no less than 2*mss.
		 */
		tp->snd_ssthresh = max(2 * mss, rt->rt_rmx.rmx_ssthresh);
	}
#endif /* RTV_MTU */
	return (mss);
}
#endif /* TUBA_INCLUDE */
