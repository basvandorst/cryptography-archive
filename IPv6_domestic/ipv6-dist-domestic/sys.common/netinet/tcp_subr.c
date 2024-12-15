/*
 * tcp_subr.c  --  Miscellaneous TCP subroutines. 
 *
 * Modfiications to support IPv6 are copyright 1995 
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
 *	@(#)tcp_subr.c	8.1 (Berkeley) 6/10/93
 * tcp_subr.c,v 1.5 1994/10/08 22:39:58 phk Exp
 */


#include <sys/param.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/errno.h>

#include <net/route.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcpip.h>

#ifdef INET6
#include <netinet6/in6_debug.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/tcpipv6.h>
#include <sys/domain.h>
#endif

/* patchable/settable parameters for tcp */
int 	tcp_mssdflt = TCP_MSS;
int 	tcp_rttdflt = TCPTV_SRTTDFLT / PR_SLOWHZ;
int	tcp_do_rfc1323 = 1;

extern	struct inpcb *tcp_last_inpcb;

#ifdef INET6
extern int ipv6_defhoplmt;
#endif /* INET6 */

/*
 * Tcp initialization
 */
void
tcp_init()
{
	tcp_iss = 1;		/* wrong; see Section 24.8, [WS95]  */
	tcb.inp_next = tcb.inp_prev = &tcb;

#ifdef INET6
	/* Since sizeof(struct ipv6) > sizeof(struct ip), we do max length
	   checks/computations only on the former. */
	if (max_protohdr < (sizeof(struct ipv6) + sizeof(struct tcphdr)))
	  max_protohdr = (sizeof(struct ipv6) + sizeof(struct tcphdr));
	if ((max_linkhdr + sizeof(struct ipv6) + sizeof(struct tcphdr)) >
	    MHLEN)
	  panic("tcp_init");
#else /* INET6 */
	if (max_protohdr < sizeof(struct tcpiphdr))
		max_protohdr = sizeof(struct tcpiphdr);
	if (max_linkhdr + sizeof(struct tcpiphdr) > MHLEN)
		panic("tcp_init");
#endif /* INET6 */

}

/*
 * Create template to be used to send tcp packets on a connection.
 * Call after host entry created, allocates an mbuf and fills
 * in a skeletal tcp/ip header, minimizing the amount of work
 * necessary when the connection is used.
 */

/*
 * To support IPv6 in addition to IPv4 and considering that the sizes of
 * the IPv4 and IPv6 headers are not the same, we now use a separate pointer
 * for the TCP header.  Also, we made the former tcpiphdr header pointer 
 * into just an IP overlay pointer, with casting as appropriate for v6. rja
 */

struct tcpiphdr *
tcp_template(tp)
	struct tcpcb *tp;
{
	register struct inpcb *inp = tp->t_inpcb;
	register struct mbuf *m;
	register struct tcpiphdr *n;
	register struct tcphdr  *th;
#ifdef INET6
	register struct tcpipv6hdr *ti6;
	register struct ipv6 *ipv6;
#endif /* INET6 */

	if ((n = tp->t_template) == 0) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == NULL)
			return (0);
#ifdef INET6
	        if (tp->pf == PF_INET6) 
	          m->m_len = sizeof (struct tcphdr) + sizeof(struct ipv6);
	        else 
#endif /* INET6 */
		  m->m_len = sizeof (struct tcpiphdr);
		n = mtod(m, struct tcpiphdr *);
	}

#ifdef INET6
	if (tp->pf == PF_INET6) {

	  ti6 = (struct tcpipv6hdr *)n;

	  ipv6 = (struct ipv6 *)n;
	  th = &ti6->ti6_t;

	  IN6_ADDR_COPY(ipv6->ipv6_src, inp->inp_laddr6);
	  IN6_ADDR_COPY(ipv6->ipv6_dst, inp->inp_faddr6);
	  ipv6->ipv6_versfl = htonl(0x60000000);  /* Add flow label later?
						   * And what about priority?
						   */
	  ipv6->ipv6_nexthdr = NEXTHDR_TCP;
	  ipv6->ipv6_length = htons(sizeof(struct tcphdr));
	  ipv6->ipv6_hoplimit = ipv6_defhoplmt;
	} else
#endif /* INET6 */
	  {
	    th = &n->ti_t;
	    n->ti_next = n->ti_prev = 0;
	    n->ti_x1 = 0;
	    n->ti_pr = IPPROTO_TCP;
	    n->ti_len = htons(sizeof (struct tcpiphdr) - sizeof (struct ip));
	    n->ti_src = inp->inp_laddr;
	    n->ti_dst = inp->inp_faddr;
	  }

	th->th_sport = inp->inp_lport;
	th->th_dport = inp->inp_fport;
	th->th_seq = 0;
	th->th_ack = 0;
	th->th_x2  = 0;
	th->th_off = 5;
	th->th_flags = 0;
	th->th_win = 0;
	th->th_sum = 0;
	th->th_urp = 0;

	return (n);
}

/*
 * Send a single message to the TCP at address specified by
 * the given TCP/IP header.  If m == 0, then we make a copy
 * of the tcpiphdr at ti and send directly to the addressed host.
 * This is used to force keep alive messages out using the TCP
 * template for a connection tp->t_template.  If flags are given
 * then we send a message back to the TCP which originated the
 * segment ti, and discard the mbuf containing it and any other
 * attached mbufs.
 *
 * In any case the ack and sequence number of the transmitted
 * segment are as specified by the parameters.
 *
 * This function looks hairy, because it was so IPv4-dependent.
 */

void
tcp_respond(tp, ti, m, ack, seq, flags)
	struct tcpcb *tp;
	register struct tcpiphdr *ti;
	register struct mbuf *m;
	tcp_seq ack, seq;
	int flags;
{
	register int tlen;
	int win = 0;
	struct route *ro = 0;
        register struct tcphdr *th;
#ifdef INET6
	int is_ipv6 = 0;   /* true iff IPv6 */
#endif /* INET6 */

	if (tp) {
		win = sbspace(&tp->t_inpcb->inp_socket->so_rcv);
#ifdef INET6
		/* If this is called with an unconnected
		   socket/tp/pcb (tp->pf is 0), we lose. */
		is_ipv6 = (tp->pf == PF_INET6);

		/* The route/route6 distinction is meaningless
		   unless you're allocating space or passing parameters. */
#endif /* INET6 */
		ro = &tp->t_inpcb->inp_route;
	} 
#ifdef INET6
        else
	  is_ipv6 = (((struct ip *)ti)->ip_v == 6);
#endif /* INET6 */

	if (m == 0) {
		m = m_gethdr(M_DONTWAIT, MT_HEADER);
		if (m == NULL)
			return;
#ifdef TCP_COMPAT_42
		tlen = 1;
#else
		tlen = 0;
#endif
		m->m_data += max_linkhdr;
#ifndef INET6
		*(mtod(m, struct tcpiphdr *)) = *ti;
#else /* INET6 */
		bcopy((caddr_t)ti, mtod(m, caddr_t), sizeof(struct tcphdr) +
			(is_ipv6 ? sizeof(struct ipv6) : sizeof(struct ip)));
#endif /* INET6 */
		ti = mtod(m, struct tcpiphdr *);
		flags = TH_ACK;
	} else {
		m_freem(m->m_next);
		m->m_next = 0;
		m->m_data = (caddr_t)ti;
		tlen = 0;
#define xchg(a,b,type) { type t; t=a; a=b; b=t; }
#ifdef INET6
		if (is_ipv6) {
		  m->m_len = sizeof(struct tcphdr) + sizeof(struct ipv6);
		  xchg(((struct ipv6 *)ti)->ipv6_dst,\
		       ((struct ipv6 *)ti)->ipv6_src,\
		       struct in_addr6);
		  th = &(((struct tcpipv6hdr *)ti)->ti6_t);
		} else
#endif /* INET6 */
		  {
		    m->m_len = sizeof(struct tcphdr) + sizeof(struct ip);
		    xchg(ti->ti_dst, ti->ti_src, struct in_addr);
		    th = &ti->ti_t;
		  } /* endif is_ipv6 */
		xchg(th->th_dport, th->th_sport, u_short);

#undef xchg
	}

#ifdef INET6
	if (is_ipv6) {
	  tlen += sizeof(struct tcphdr) + sizeof(struct ipv6); 
	  DP(MAJOR_EVENT, tlen, d);
	  th = (struct tcphdr *)((caddr_t)ti + sizeof(struct ipv6));
	  DP(MAJOR_EVENT, th, 08x);
	} else
#endif /* INET6 */
	  {
	    ti->ti_len = htons((u_short)(sizeof(struct tcphdr) + tlen));
	    tlen += sizeof (struct tcpiphdr);
	    th = (struct tcphdr *)((caddr_t)ti + sizeof(struct ip));
	  } /* endif is_ipv6 */

	m->m_len = tlen;
	m->m_pkthdr.len = tlen;
	m->m_pkthdr.rcvif = (struct ifnet *) 0;

	th->th_seq = htonl(seq);
	th->th_ack = htonl(ack);
	th->th_x2 = 0;
	th->th_off = sizeof (struct tcphdr) >> 2;
	th->th_flags = flags;
	if (tp)
		th->th_win = htons((u_short) (win >> tp->rcv_scale));
	else
		th->th_win = htons((u_short)win);
	th->th_urp = 0;
	th->th_sum = 0;

#ifdef INET6
	if (is_ipv6) {
	  ((struct ipv6 *)ti)->ipv6_versfl   = htonl(0x60000000);
	  ((struct ipv6 *)ti)->ipv6_nexthdr  = NEXTHDR_TCP;
	  ((struct ipv6 *)ti)->ipv6_hoplimit = MAXHOPLIMIT;
	  ((struct ipv6 *)ti)->ipv6_length = tlen - sizeof(struct ipv6);
	  th->th_sum = in6_cksum(m, NEXTHDR_TCP, 
		      ((struct ipv6 *)ti)->ipv6_length, sizeof(struct ipv6));
	  HTONS(((struct ipv6 *)ti)->ipv6_length);
	  /*
	   * Tag outbound message with socket for security processing.
	   */
	  if (tp)
	    m->m_pkthdr.rcvif = (struct ifnet *)tp->t_inpcb->inp_socket;
	  (void) ipv6_output(m, (struct route6 *)ro, 0, NULL);
	} else
#endif /* INET6 */
	  {
	    ti->ti_x1 = 0;
	    ti->ti_next = ti->ti_prev = 0;
	    ti->ti_len = htons((u_short)tlen - sizeof(struct ip));
	    th->th_sum = in_cksum(m, tlen);
	    ((struct ip *)ti)->ip_len = tlen;
	    ((struct ip *)ti)->ip_ttl = ip_defttl;
	    (void) ip_output(m, NULL, ro, 0, NULL);
	  } /* endif is_ipv6 */ 
}


/*
 * Create a new TCP control block, making an
 * empty reassembly queue and hooking it to the argument
 * protocol control block.
 */
struct tcpcb *
tcp_newtcpcb(inp)
	struct inpcb *inp;
{
	register struct tcpcb *tp;

	tp = malloc(sizeof(*tp), M_PCB, M_NOWAIT);
	if (tp == NULL)
		return ((struct tcpcb *)0);
	bzero((char *) tp, sizeof(struct tcpcb));
	tp->seg_next = tp->seg_prev = (struct tcpiphdr *)tp;
	tp->t_maxseg = tcp_mssdflt;

	tp->t_flags = tcp_do_rfc1323 ? (TF_REQ_SCALE|TF_REQ_TSTMP) : 0;
	tp->t_inpcb = inp;
	/*
	 * Init srtt to TCPTV_SRTTBASE (0), so we can tell that we have no
	 * rtt estimate.  Set rttvar so that srtt + 2 * rttvar gives
	 * reasonable initial retransmit time.
	 */
	tp->t_srtt = TCPTV_SRTTBASE;
	tp->t_rttvar = tcp_rttdflt * PR_SLOWHZ << 2;
	tp->t_rttmin = TCPTV_MIN;
	TCPT_RANGESET(tp->t_rxtcur, 
	    ((TCPTV_SRTTBASE >> 2) + (TCPTV_SRTTDFLT << 2)) >> 1,
	    TCPTV_MIN, TCPTV_REXMTMAX);
	tp->snd_cwnd = TCP_MAXWIN << TCP_MAX_WINSHIFT;
	tp->snd_ssthresh = TCP_MAXWIN << TCP_MAX_WINSHIFT;
#ifdef INET6
	/* If we want to use tp->pf for a quick-n-easy way to determine
	   the outbound dgram type, we cannot make this decision
	   until a connection is established!  Bzero() sets pf to zero, and
	   that's the way we want it, unless, of course, it's an AF_INET
	   socket... */
	if ((inp->inp_flags & INP_IPV6) == 0)
	  tp->pf = PF_INET;  /* If AF_INET socket, we can't do v6 from it. */

	/* These will be filled in at tcp_template()? */
	/*if (tp->pf == PF_INET6) 
	  inp->inp_ipv6.ipv6_hoplimit = MAXHOPLIMIT;
	else*/
#endif /* INET6 */
	inp->inp_ip.ip_ttl = ip_defttl;
	inp->inp_ppcb = (caddr_t)tp;
	return (tp);
}

/*
 * Drop a TCP connection, reporting
 * the specified error.  If connection is synchronized,
 * then send a RST to peer.
 */
struct tcpcb *
tcp_drop(tp, errno)
	register struct tcpcb *tp;
	int errno;
{
	struct socket *so = tp->t_inpcb->inp_socket;

	if (TCPS_HAVERCVDSYN(tp->t_state)) {
		tp->t_state = TCPS_CLOSED;
		(void) tcp_output(tp);
		tcpstat.tcps_drops++;
	} else
		tcpstat.tcps_conndrops++;
	if (errno == ETIMEDOUT && tp->t_softerror)
		errno = tp->t_softerror;
	so->so_error = errno;
	return (tcp_close(tp));
}

/*
 * Close a TCP control block:
 *	discard all space held by the tcp
 *	discard internet protocol block
 *	wake up any sleepers
 */
struct tcpcb *
tcp_close(tp)
	register struct tcpcb *tp;
{
	register struct tcpiphdr *t;
	struct inpcb *inp = tp->t_inpcb;
	struct socket *so = inp->inp_socket;
	register struct mbuf *m;
#ifdef INET6
	register struct tcpipv6hdr *ti6;
#endif
#ifdef RTV_RTT
	register struct rtentry *rt;
#ifdef INET6
	register int bound_to_specific;  /* I.e. non-default */

	/*
	 * This code checks the nature of the route for this connection.
	 * Normally this is done by two simple checks in the next
	 * INET/INET6 ifdef block, but because of two possible lower layers,
	 * that check is done here.
	 *
	 * Perhaps should be doing this only for a RTF_HOST route.
	 */
	rt = inp->inp_route.ro_rt;  /* Same for route or route6. */
	if (tp->pf == PF_INET6) {
	  if (rt)
	    bound_to_specific = 
	      !(IS_IN6_UNSPEC(((struct sockaddr_in6 *)rt_key(rt))->sin6_addr));
	} else {
	  if (rt)
	    bound_to_specific =
	      (((struct sockaddr_in *)rt_key(rt))->sin_addr.s_addr !=
	                                                           INADDR_ANY);
	} /* endif isipv6 */
#endif /* INET6 */

	/*
	 * If we sent enough data to get some meaningful characteristics,
	 * save them in the routing entry.  'Enough' is arbitrarily 
	 * defined as the sendpipesize (default 4K) * 16.  This would
	 * give us 16 rtt samples assuming we only get one sample per
	 * window (the usual case on a long haul net).  16 samples is
	 * enough for the srtt filter to converge to within 5% of the correct
	 * value; fewer samples and we could save a very bogus rtt.
	 *
	 * Don't update the default route's characteristics and don't
	 * update anything that the user "locked".
	 */
#ifndef INET6
	/*
	 * This is the original way to check to see the nature of the route.
	 */
	if (SEQ_LT(tp->iss + so->so_snd.sb_hiwat * 16, tp->snd_max) &&
	    (rt = inp->inp_route.ro_rt) &&
	    ((struct sockaddr_in *)rt_key(rt))->sin_addr.s_addr != INADDR_ANY) {
#else /* INET6 */
	/*
	 * Note that rt and bound_to_specific are set above.
	 */
	if (SEQ_LT(tp->iss + so->so_snd.sb_hiwat * 16, tp->snd_max) 
	    && rt && bound_to_specific)
	  {
#endif /* INET6 */
		register u_long i = 0L;

		if ((rt->rt_rmx.rmx_locks & RTV_RTT) == 0) {
			i = tp->t_srtt *
			    (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTT_SCALE));
			if (rt->rt_rmx.rmx_rtt && i)
				/*
				 * filter this update to half the old & half
				 * the new values, converting scale.
				 * See route.h and tcp_var.h for a
				 * description of the scaling constants.
				 */
				rt->rt_rmx.rmx_rtt =
				    (rt->rt_rmx.rmx_rtt + i) / 2;
			else
				rt->rt_rmx.rmx_rtt = i;
		}
		if ((rt->rt_rmx.rmx_locks & RTV_RTTVAR) == 0) {
			i = tp->t_rttvar *
			    (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTTVAR_SCALE));
			if (rt->rt_rmx.rmx_rttvar && i)
				rt->rt_rmx.rmx_rttvar =
				    (rt->rt_rmx.rmx_rttvar + i) / 2;
			else
				rt->rt_rmx.rmx_rttvar = i;
		}
		/*
		 * update the pipelimit (ssthresh) if it has been updated
		 * already or if a pipesize was specified & the threshhold
		 * got below half the pipesize.  I.e., wait for bad news
		 * before we start updating, then update on both good
		 * and bad news.
		 */
		if (((rt->rt_rmx.rmx_locks & RTV_SSTHRESH) == 0 &&
		    ((i = tp->snd_ssthresh) != 0) && rt->rt_rmx.rmx_ssthresh) ||
		    i < (rt->rt_rmx.rmx_sendpipe / 2)) {
			/*
			 * convert the limit from user data bytes to
			 * packets then to packet data bytes.
			 */
			i = (i + tp->t_maxseg / 2) / tp->t_maxseg;
			if (i < 2)
				i = 2;
#ifdef INET6
			if (tp->pf == PF_INET6)
			  i *= (u_long)(tp->t_maxseg + sizeof (struct tcphdr) 
					+ sizeof(struct ipv6));
			else
#endif /* INET6 */
			  i *= (u_long)(tp->t_maxseg + sizeof (struct tcpiphdr));
			if (rt->rt_rmx.rmx_ssthresh)
				rt->rt_rmx.rmx_ssthresh =
				    (rt->rt_rmx.rmx_ssthresh + i) / 2;
			else
				rt->rt_rmx.rmx_ssthresh = i;
		}
	}
#endif /* RTV_RTT */
	/* free the reassembly queue, if any */

#ifdef INET6
	/* Reassembling TCP segments in v6 might be sufficiently different
	     to merit two codepaths to free the reasssembly queue.   
	   If an undecided TCP socket, then the IPv4 codepath will be used 
	     because it won't matter much anyway. */

	if (tp->pf == AF_INET6)
	  {
	    ti6 = (struct tcpipv6hdr *)tp->seg_next;
	    while (ti6 != (struct tcpipv6hdr *)tp)
	      {
		ti6 = (struct tcpipv6hdr *)ti6->ti6_next;
		m = REASSV6_MBUF((struct tcpipv6hdr *)ti6->ti6_prev);
		remque(ti6->ti6_prev);
		m_freem(m);
	      }
	  }
	else
#endif
	  {
	    t = tp->seg_next;
	    while (t != (struct tcpiphdr *)tp) {
	      t = (struct tcpiphdr *)t->ti_next;
	      m = REASS_MBUF((struct tcpiphdr *)t->ti_prev);
	      remque(t->ti_prev);
	      m_freem(m);
	    }
	  }
	if (tp->t_template)
	  (void) m_free(dtom(tp->t_template));

	free(tp, M_PCB);
	inp->inp_ppcb = 0;
	soisdisconnected(so);
	/* clobber input pcb cache if we're closing the cached connection */
	if (inp == tcp_last_inpcb)
		tcp_last_inpcb = &tcb;
	in_pcbdetach(inp);
	tcpstat.tcps_closed++;
	return ((struct tcpcb *)0);
}

void
tcp_drain()
{

}

/*
 * Notify a tcp user of an asynchronous error;
 * store error as soft error, but wake up user
 * (for now, won't do anything until can select for soft error).
 */
void
tcp_notify(inp, error)
	struct inpcb *inp;
	int error;
{
	register struct tcpcb *tp = (struct tcpcb *)inp->inp_ppcb;
	register struct socket *so = inp->inp_socket;

	/*
	 * Ignore some errors if we are hooked up.
	 * If connection hasn't completed, has retransmitted several times,
	 * and receives a second error, give up now.  This is better
	 * than waiting a long time to establish a connection that
	 * can never complete.
	 */
	if (tp->t_state == TCPS_ESTABLISHED &&
	     (error == EHOSTUNREACH || error == ENETUNREACH ||
	      error == EHOSTDOWN)) {
		return;
	} else if (tp->t_state < TCPS_ESTABLISHED && tp->t_rxtshift > 3 &&
	    tp->t_softerror)
		so->so_error = error;
	else 
		tp->t_softerror = error;
	wakeup((caddr_t) &so->so_timeo);
	sorwakeup(so);
	sowwakeup(so);
}

void
tcp_ctlinput(cmd, sa, ip
#ifdef INET6
	     ,incoming
#endif /* INET6 */
	     )
     int cmd;
     struct sockaddr *sa;
     register struct ip *ip;
#ifdef INET6
     /*
      * Extra parameter is here for passing to in{,6}_pcbnotify for security
      * policy checking.  Lots of the cruft in this function is because
      * of the *pcbnotify extra parameters.
      */
     struct mbuf *incoming;
#endif /* INET6 */
{
	register struct tcphdr *th;
#ifdef INET6
	struct in_addr6 zeroin_addr6;
#endif /* INET6 */
	extern struct in_addr zeroin_addr;
	extern u_char inetctlerrmap[];
	void (*notify) __P((struct inpcb *, int)) = tcp_notify;

	if (cmd == PRC_QUENCH)
		notify = tcp_quench;
	else if (!PRC_IS_REDIRECT(cmd) &&
		 ((unsigned)cmd > PRC_NCMDS || inetctlerrmap[cmd] == 0))
		return;

#ifdef INET6
	if (sa->sa_family == AF_INET6) {
	  if (ip) {
	    th = (struct tcphdr *)((caddr_t)ip + sizeof(struct ipv6));
	    in6_pcbnotify(&tcb, sa, th->th_dport, 
			  &(((struct ipv6 *)ip)->ipv6_src),
			  th->th_sport, cmd, notify, incoming,
				NEXTHDR_TCP);
	  } else {
	    IN6_ADDR_ASSIGN(zeroin_addr6, 0, 0, 0, 0);
	    in6_pcbnotify(&tcb, sa, 0, &zeroin_addr6, 0, cmd, notify,
		incoming, NEXTHDR_TCP);
	  } /* endif ip */
	} else {
#endif /* INET6 */
	if (ip) {
		th = (struct tcphdr *)((caddr_t)ip + (ip->ip_hl << 2));
		in_pcbnotify(&tcb, sa, th->th_dport, ip->ip_src, th->th_sport,
#ifdef INET6
			cmd, notify, incoming, NEXTHDR_TCP);
#else /* INET6 */
			cmd, notify);
#endif /* INET6 */
	} else
#ifdef INET6
		in_pcbnotify(&tcb, sa, 0, zeroin_addr, 0, cmd, notify,
			incoming, NEXTHDR_TCP);
#else /* INET6 */
		in_pcbnotify(&tcb, sa, 0, zeroin_addr, 0, cmd, notify);
#endif /* INET6 */
#ifdef INET6
	} /* endif sa->sa_family == AF_INET6 */
#endif /* INET6 */
}

/*
 * When a source quench is received, close congestion window
 * to one segment.  We will gradually open it again as we proceed.
 */
void
tcp_quench(inp, errno)
	struct inpcb *inp;
	int errno;
{
	struct tcpcb *tp = intotcpcb(inp);

	if (tp)
		tp->snd_cwnd = tp->t_maxseg;
}
