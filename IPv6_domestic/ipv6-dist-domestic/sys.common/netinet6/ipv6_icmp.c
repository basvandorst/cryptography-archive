/*
 * ipv6_icmp.c  --  ICMPv6 routines for error generation, received packet
 *                  processing, etc.
 *
 * Copyright 1995 by Dan McDonald, Bao Phan, and Randall Atkinson,
 *	All Rights Reserved.  
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/time.h>
#include <sys/kernel.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h>
#include <netinet6/icmpv6_var.h>

#include <netinet6/in6_debug.h>

/*
 * Globals.
 */

static struct sockaddr_in6 icmpsrc = { sizeof (struct sockaddr_in6), 
					 AF_INET6 };
static struct sockaddr_in6 icmpdst = { sizeof (struct sockaddr_in6), 
					 AF_INET6 };
static struct sockaddr_in6 icmpgw = { sizeof (struct sockaddr_in6), AF_INET6 };
struct icmpv6stat icmpv6stat;

/*
 * External globals.
 */

extern struct in6_ifaddr *in6_ifaddr;
extern u_char ipv6_protox[];
extern struct protosw inet6sw[];

/*
 * Functions and macros (and one global that needs to be near the function).
 */

u_int8 ipv6_saved_routing[384]; /* If a routing header has been processed,
				   then it will have been bcopy()d into
				   this buffer. If not, the first byte
				   will be set to zero (which would be
				   a nexthdr of hop-by-hop, and is not
				   valid). */

/*----------------------------------------------------------------------
 * Reverse a saved IPv6 source route, for possible use on replies.
 ----------------------------------------------------------------------*/

struct mbuf *ipv6_srcrt()
{
      struct ipv6_srcroute0 *sr, *osr = (struct ipv6_srcroute0 *)ipv6_saved_routing;
      struct in_addr6 *sra, *osra;
      struct mbuf *srm;
      int i, j;

      if (!osr->i6sr_nexthdr)
        return NULL;
      if (osr->i6sr_type)
	return NULL;
      if (!(srm = m_get(M_NOWAIT, MT_DATA)))
        return NULL;

      sr = mtod(srm, struct ipv6_srcroute0 *);
      bzero(sr, sizeof(struct ipv6_srcroute0));
      sr->i6sr_nexthdr = NEXTHDR_ICMP;
      j = sr->i6sr_numaddrs = osr->i6sr_numaddrs;
/* We probably should reverse the bit mask, but it's painful, and defaulting
   to loose source routing might be preferable anyway. */
      sra = (struct in_addr6 *)((caddr_t)sr + sizeof(struct ipv6_srcroute0));
      osra = (struct in_addr6 *)((caddr_t)osr + sizeof(struct ipv6_srcroute0));
      srm->m_len = sizeof(struct ipv6_srcroute0) + sizeof(struct in_addr6) * j;
      i = 0;
      while(i < sr->i6sr_numaddrs)
        sra[i++] = osra[j--];
      return srm;
}

/*----------------------------------------------------------------------
 * Reflect an IPv6 ICMP packet back to the source.
 ----------------------------------------------------------------------*/

void
ipv6_icmp_reflect(m, extra, index, indexoff)
     struct mbuf *m;
     int extra;
     struct in6_hdrindex *index;
     int indexoff;
{
  int optlen=0;
  struct in_addr6 tmp;
  struct ipv6 *ipv6 = mtod(m,struct ipv6 *);
  struct in6_ifaddr *i6a;
  struct ipv6_icmp *icmp = mtod(index ? index[indexoff].ihi_mbuf : m->m_next, 
				struct ipv6_icmp *);
  struct mbuf *m2, *routing = NULL;

  /*
   * Hmmm, we potentially have authentication, routing, and hop-by-hop 
   * headers behind this.  OUCH.   For now, however, assume only IPv6
   * header, followed by ICMP.
   */

  tmp = ipv6->ipv6_dst;
  ipv6->ipv6_dst = ipv6->ipv6_src;

  /*
   * If the incoming packet was addressed directly to us,
   * use dst as the src for the reply.  Otherwise (multicast
   * or anonymous), use the address which corresponds
   * to the incoming interface.
   */

  if (IS_IN6_MULTICAST(tmp) || IS_IN6_UNSPEC(tmp))
    {
      struct in6_ifaddr *ti6a = NULL;

      for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
	{
	  /* Find first (non-local if possible) address for
	     source usage.  If multiple locals, use last one found. */

	  if (i6a->i6a_ifp == m->m_pkthdr.rcvif)  /* Ahhh, interface check
						     eliminates i6a ending
						     up being loopback. */
	    if (IS_IN6_LINKLOC(I6A_SIN(i6a)->sin6_addr))
	      ti6a=i6a;
	    else break;
	}

      /* Should I panic here?  Or should I set i6a to ANYTHING instead? */
      if (!i6a && !(i6a = ti6a))
	panic("ipv6_icmp_reflect() can't find ipv6 address on interface!");

      ipv6->ipv6_src = I6A_SIN(i6a)->sin6_addr;
    }
  else ipv6->ipv6_src = tmp;

  ipv6->ipv6_nexthdr = NEXTHDR_ICMP;

  if (extra > sizeof(struct ipv6)) {
    if (index) {
      ipv6_stripoptions(m, index[indexoff-1].ihi_mbuf, 
	index[indexoff].ihi_nexthdr);
      extra = sizeof(struct ipv6);
      if (routing = ipv6_srcrt()) {
        ipv6->ipv6_nexthdr = NEXTHDR_ROUTING;
        mtod(routing, struct ipv6_srcroute0 *)->i6sr_nexthdr = NEXTHDR_ICMP;
        routing->m_next = m->m_next;
        m->m_next = routing;
        extra += routing->m_len;
      }
    } else
      DPRINTF(IDL_CRITICAL, ("icmp_reflect() got options but can't strip them\n"));
  }

  m->m_flags &= ~(M_BCAST|M_MCAST);

  /* For errors, anything over the 576 byte mark we discard. */
  if (!(ICMPV6_INFOTYPE(icmp->icmp_type)))
    if (m->m_pkthdr.len > ICMPV6_MAXLEN)
      m_adj(m, -(m->m_pkthdr.len - ICMPV6_MAXLEN));

  icmp->icmp_cksum = 0;
  DPRINTF(IDL_EVENT,("ipv6_icmp_reflect() calling in6_cksum().\n"));
  icmp->icmp_cksum = in6_cksum(m,NEXTHDR_ICMP,m->m_pkthdr.len - 
    sizeof(struct ipv6) - optlen, extra);

  ipv6->ipv6_hoplimit = 255;  
  /* Version 6, priority 12 (info) or 15 (error), flow zero */
  ipv6->ipv6_versfl = htonl(((6) << 28) | 
		((ICMPV6_INFOTYPE(icmp->icmp_type) ? 12 : 15) << 24));

  DDO(IDL_GROSS_EVENT, dump_mchain(m));
  m->m_pkthdr.rcvif = NULL;     /* Null socket pointer for security. */
  icmpv6stat.icps_outhist[icmp->icmp_type]++;
  (void)ipv6_output(m,NULL,IPV6_RAWOUTPUT,NULL);
}

/*----------------------------------------------------------------------
 * Given a bad packet (badpack), generate an ICMP error packet in response.
 * We assume that ipv6_preparse() has been run over badpack.
 ----------------------------------------------------------------------*/

void
ipv6_icmp_error(badpack, type, code, paramptr)
     struct mbuf *badpack;
     int type,code;
     u_int32 paramptr;
{
  struct ipv6 *oipv6, *ipv6;
  int divpoint = sizeof(struct ipv6), icmplen;
  struct ipv6_icmp *icmp;
  struct mbuf *outgoing;

  if ((badpack->m_flags & M_MCAST) /*&& type != ICMPV6_TOOBIG */) {
      m_freem(badpack);
      return;
    }

  /*if (type != ICMPV6_REDIRECT)*/
    icmpv6stat.icps_error++;

  /*
   * Since MTU and max ICMP packet size is less than a cluster (so far...)
   * pull the offending packet into a single cluster.
   *
   * If option-stripping becomes required, here might be the place to do it.
   * (The current design decision is to not strip options.  Besides, one of
   *  the callers of this function is ipv6_hop(), which does hop-by-hop
   *  option processing.)
   */

  oipv6 = mtod(badpack,struct ipv6 *);

  DDO(IDL_GROSS_EVENT,printf("oipv6 (0x%x) is:\n",oipv6);dump_ipv6(oipv6));

  /*
   * Get a new cluster mbuf for ICMP error message.  Since IPv6 ICMP messages
   * have a length limit that should be less than MCLBYTES, one cluster should
   * work nicely.
   */

  if (!(outgoing = m_gethdr(M_DONTWAIT,MT_HEADER))) {
      m_freem(badpack);
      return;
    }

  if (!(outgoing->m_next = m_get(M_DONTWAIT,MT_DATA))) {
      m_free(outgoing);
      m_freem(badpack);
      return;
  }

  outgoing->m_len = sizeof(struct ipv6);
  bcopy(oipv6, mtod(outgoing, caddr_t), sizeof(struct ipv6));

  outgoing->m_next->m_len = ICMPV6_MINLEN;
  icmp = mtod(outgoing->m_next, struct ipv6_icmp *);

  outgoing->m_next->m_next = badpack;
  outgoing->m_pkthdr.len = badpack->m_pkthdr.len + sizeof(struct ipv6) 
	+ ICMPV6_MINLEN;

  icmp->icmp_type = type;
  icmp->icmp_code = code;
  icmp->icmp_unused = 0;
  if (type == ICMPV6_PARAMPROB /*|| type == ICMPV6_TOOBIG*/)
      icmp->icmp_paramptr = paramptr;

  DDO(IDL_GROSS_EVENT, dump_mchain(outgoing));
  ipv6_icmp_reflect(outgoing, divpoint, 0, 0);
}

/*----------------------------------------------------------------------
 * Update path MTU for an IPv6 destination.  This function may have to go
 * scan for TCP control blocks and give them hints to scale down.
 * There is a small denial-of-service attack if MTU messages are
 * unauthenticated.  I can lower MTU to 576.
 ----------------------------------------------------------------------*/

void
update_pathmtu(dst, newmtu)
     struct in_addr6 *dst;
     u_int32 newmtu;       /* ntohl()'ed by caller. */
{
  int s = splnet();
  struct rtentry *rt;
  struct sockaddr_in6 sin6;

  DDO(IDL_GROSS_EVENT,printf("Entering update_pathmtu with:\n");\
      dump_in_addr6(dst);printf("And newmtu of %d.\n",newmtu));

  if (IS_IN6_MULTICAST(*dst))
    {
      /* Multicast MTU Discovery not yet implemented */
      DPRINTF(IDL_CRITICAL, ("Multicast MTU too big message.\n"));
      splx(s);
      return;
    }

  sin6.sin6_family = AF_INET6;
  sin6.sin6_len = sizeof(sin6);
  IN6_ADDR_COPY(sin6.sin6_addr,*dst);
  /*
   * Since I'm doing a rtalloc, no need to zero-out the port and flowlabel.  
   */
  if ((rt = rtalloc1((struct sockaddr *)&sin6,0)) != NULL)
    {
      if (rt->rt_flags & RTF_HOST)
        {
	  if (rt->rt_rmx.rmx_mtu < newmtu)
	    DPRINTF(IDL_CRITICAL, 
	    ("DANGER:  New MTU message is LARGER than current MTU.\n"));

	  rt->rt_rmx.rmx_mtu = newmtu;   /* This should be enough for HLP's to
					    know MTU has changed, IMHO. */
	  rt->rt_refcnt--;
        }
      else
        {
	  DPRINTF(IDL_CRITICAL, 
		 ("Got path MTU message for non-cloned destination route.\n"));
        }
      /*
       * Find all active tcp connections, and indicate they need path MTU
       * updating as well.
       */
    }

  splx(s);
  return;
}

/*----------------------------------------------------------------------
 * ICMPv6 input routine.  Handles inbound ICMPv6 packets, including
 * direct handling of some packets.
 ----------------------------------------------------------------------*/

void
ipv6_icmp_input(incoming, extra, index, indexoff)
     register struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *index;
     int indexoff;
{
  struct ipv6_icmp *icmp;
  struct ipv6 *ipv6;
  int icmplen,code;
  void (*ctlfunc) __P((int, struct sockaddr *, struct ipv6 *, struct mbuf *));

  /*
   * Q:  Any address validity checks beyond those in ipv6_input()?
   */

  ipv6 = mtod(incoming,struct ipv6 *);
  icmp = mtod(index[indexoff].ihi_mbuf, struct ipv6_icmp *);

  DPRINTF(IDL_EVENT,("ipv6_icmp_input -- pkthdr.len = %d, extra = %d\n", 
		     incoming->m_pkthdr.len, extra));
  icmplen = incoming->m_pkthdr.len - extra;

  if (icmplen < ICMPV6_MINLEN)
    {
      /* Not enough for full ICMP packet. */
      icmpv6stat.icps_tooshort++;
      m_freem(incoming);
      return;
    }

  /*
   * Verify checksum with IPv6 header at the top of this chain.
   */

  DPRINTF(IDL_EVENT,("ipv6_icmp_input() calling in6_cksum().\n"));
  DPRINTF(IDL_MAJOR_EVENT,("icmplen = %d\n", icmplen));
  if (in6_cksum(incoming,NEXTHDR_ICMP,icmplen,extra))
    {
      DPRINTF(IDL_CRITICAL,("ipv6_icmp_input() -- checksum failed.\n"));
      m_freem(incoming);
      icmpv6stat.icps_checksum++;
      return;
    }

  /* Perform IPsec input-side policy check.
     Drop packet if policy says to drop it.
     NB: Really paranoid system security level configurations
     can cause chicken/egg problems here.  Be careful.  rja  */
  
  if (ipsec_input_policy(incoming, NULL, NEXTHDR_ICMPV6)) {
      DPRINTF(IDL_CRITICAL,("ipv6_icmp_input() -- policy failure.\n"));
      m_freem(incoming);
      return;
  }

  code = icmp->icmp_code;
  DPRINTF(IDL_MAJOR_EVENT, ("icmp->icmp_type = %d\n", icmp->icmp_type));
  if (icmp->icmp_type < ICMPV6_MAXTYPE)
    {
      icmpv6stat.icps_inhist[icmp->icmp_type]++;

      /*
       * Deal with the appropriate ICMPv6 message type/code.
       */
      switch(icmp->icmp_type)
	{
	case ICMPV6_ECHO:
	  icmp->icmp_type = ICMPV6_ECHOREPLY;
	  icmpv6stat.icps_reflect++;
	  ipv6_icmp_reflect(incoming, extra, index, indexoff);
	  return;
	case ICMPV6_UNREACH:
	  /*
	   * The pair of <type,code> should map into a PRC_*
	   * value such that I don't have to rewrite in_pcb.c.
	   */
	  switch (code)
	    {
	    case ICMPV6_UNREACH_NOROUTE:
	      code = PRC_UNREACH_NET;
	      break;
	    case ICMPV6_UNREACH_ADMIN:
	      /* Subject to change */
	      code = PRC_UNREACH_HOST;
	      break;
	    case ICMPV6_UNREACH_NOTNEIGHBOR:
	      /* Subject to change */
	      code = PRC_UNREACH_HOST;
	      break;
	    case ICMPV6_UNREACH_ADDRESS:
	      code = PRC_UNREACH_HOST;
	      break;
	    case ICMPV6_UNREACH_PORT:
	      code = PRC_UNREACH_PORT;
	      break;
	    default:
	      goto badcode;
	    }
	  goto deliver;
	  break;
	case ICMPV6_TIMXCEED:
	  if (code >1)
	    goto badcode;
	  code += PRC_TIMXCEED_INTRANS;
	  goto deliver;
	  
	case ICMPV6_PARAMPROB:
	  if (code >2)
	    goto badcode;
	  code = PRC_PARAMPROB;

	deliver:
	  /*
	   * Problem with datagram, advice HLP's.
	   */
	  DPRINTF(IDL_MAJOR_EVENT, ("delivering\n"));
	  if (icmplen < ICMPV6_HLPMINLEN)
	    {
	      icmpv6stat.icps_badlen++;
	      m_freem(incoming);
	      return;
	    }
	  
	  /* May want to pullup more than sizeof(struct ipv6) */
	  if (!(index[indexoff].ihi_mbuf->m_next =
		m_pullup(index[indexoff].ihi_mbuf->m_next, sizeof(struct ipv6) + 8)))
	    return;
	  
	  /*
	   * If cannot determine HLP, discard packet.
	   *
	   * For now, I assume that ICMP messages will be generated such that
	   * the enclosed header contains only IPv6+<HLP header>.  This is not
	   * a good assumption to make in light of all sorts of options between
	   * IPv6 and the relevant place to deliver this message.
	   */
	  {
	    struct ipv6 *ipv6 = mtod(index[indexoff].ihi_mbuf->m_next,
				     struct ipv6 *);
	    icmpsrc.sin6_addr = ipv6->ipv6_dst;
	    
	    DPRINTF(IDL_MAJOR_EVENT, ("Finding control function for %d\n",
				      ipv6->ipv6_nexthdr));
	    DDO(IDL_MAJOR_EVENT, dump_mbuf(index[indexoff].ihi_mbuf->m_next));
	    if (ctlfunc = inet6sw[ipv6_protox[ipv6->ipv6_nexthdr]].pr_ctlinput) {
	      DPRINTF(IDL_MAJOR_EVENT, ("Calling control function for %d\n", 
					ipv6->ipv6_nexthdr));
	      (*ctlfunc)(code, (struct sockaddr *)&icmpsrc, ipv6,incoming);
	    }
	  }
	  break;
	  
	badcode:
	  DPRINTF(IDL_MAJOR_EVENT, ("Bad code!\n"));
	  icmpv6stat.icps_badcode++;
	  break;
	  
	case ICMPV6_TOOBIG:
	  update_pathmtu(&icmp->icmp_ipv6.ipv6_dst,htonl(icmp->icmp_nexthopmtu));
	  /*code = PRC_MSGSIZE;
	    goto deliver;*/   /* Do I want to deliver to HLP? */
	  break;
	  
	  /*
	   * IPv6 multicast group messages.
	   */
	case ICMPV6_GRPQUERY:
	case ICMPV6_GRPREPORT:
	case ICMPV6_GRPTERM:
	  break;
	  
	  /*
	   * IPv6 discovery messages.
	   */
	case ICMPV6_ROUTERSOL:
	  ipv6_routersol_input(incoming,extra, index, indexoff);
	  break;
	case ICMPV6_ROUTERADV:
	  ipv6_routeradv_input(incoming,extra, index, indexoff);
	  break;
	case ICMPV6_NEIGHBORSOL:
	  ipv6_neighborsol_input(incoming,extra, index, indexoff);
	  break;
	case ICMPV6_NEIGHBORADV:
	  ipv6_neighboradv_input(incoming,extra, index, indexoff);
	  break;
	  
	default:
	  /* Allow delivery to raw socket. */
	  break;
	}
    }
  DPRINTF(IDL_EVENT, ("Delivering ICMPv6 to raw socket\n"));
  ripv6_input(incoming,extra, index, indexoff); /* Deliver to raw socket. */
}


/*
 * The following functions attempt to address ICMP deficiencies in IPv6.
 * Mostly these are part of a hack to keep the user-level program from
 * computing checksums.  :-P
 */

/*----------------------------------------------------------------------
 * This function should never be called.
 ----------------------------------------------------------------------*/

int
ipv6_icmp_output(m, so, dst)
     struct mbuf *m;
     struct socket *so;
     struct in_addr6 *dst;
{
  DPRINTF(IDL_CRITICAL, 
	  ("ipv6_icmp_output() was called and shouldn't have been.\n"));

  ripv6_output(m,so,dst);
}

/*----------------------------------------------------------------------
 * Prepend IPv6 header, and compute IPv6 checksum for PRU_SEND, otherwise,
 * redirect call to ripv6_usrreq().
 ----------------------------------------------------------------------*/

int
ipv6_icmp_usrreq(so, req, m, nam, control)
     struct socket *so;
     int req;
     struct mbuf *m, *nam, *control;
{
  register struct inpcb *inp = sotoinpcb(so);
  register struct ipv6 *ipv6;
  register struct ipv6_icmp *icp;
  struct in_addr6 *dst;
  int tflags = 0, len ,rc;
  struct in6_ifaddr *i6a;

  DPRINTF(IDL_EVENT,("Entering ipv6_icmp_usrreq(), req == %d\n",req));

  /*
   * If not sending, or sending with the header included (which IMHO means
   * the user filled in the src & dest on his/her own), do normal raw
   * IPv6 user request.
   */ 

  DPRINTF(IDL_EVENT,("Before check for ripv6_usrreq().\n"));
  if (req != PRU_SEND || inp->inp_flags & INP_HDRINCL)
    return ripv6_usrreq(so,req,m,nam,control);

  if (in6_ifaddr == NULL)
    return EADDRNOTAVAIL;
  len = m->m_pkthdr.len;

  /*
   * If we get here, req == PRU_SEND and flags do not have INP_HDRINCL set.
   * What that means in English is that a user process is sending an ICMPv6
   * datagram without constructing an IPv6 header.
   * We will construct an IPv6 header, fill it in completely, THEN COMPUTE
   * THE ICMPv6 CHECKSUM and tell ipv6_output() that we are raw.
   */

  if (so->so_state & SS_ISCONNECTED)
    {
      if (nam)
	{
	  m_freem(m);
	  return EISCONN;
	}
      dst = &(inp->inp_faddr6);
    }
  else
    {
      if (nam == NULL)
	{
	  m_freem(m);
	  return ENOTCONN;
	}
      DPRINTF(IDL_GROSS_EVENT,("Sockaddr in nam is:\n"));
      DDO(IDL_GROSS_EVENT,dump_smart_sockaddr(mtod(nam,struct sockaddr *)));
      dst = &(mtod(nam, struct sockaddr_in6 *)->sin6_addr);
      inp->inp_route6.ro_dst = *(mtod(nam,struct sockaddr_in6 *));
      if (so->so_options & SO_DONTROUTE)
	{
#define ifatoi6a(x)  ((struct in6_ifaddr *)x)
	  if ((i6a = 
	       ifatoi6a(ifa_ifwithdstaddr(mtod(nam,struct sockaddr *)))) == 0
	      &&
	      (i6a = 
	       ifatoi6a(ifa_ifwithnet(mtod(nam,struct sockaddr *)))) == 0)
	    {
	      return ENETUNREACH;
	    }
	  inp->inp_route.ro_rt = NULL;
	}
      else
	{
	  rtalloc(&inp->inp_route);
	  if (inp->inp_route.ro_rt == NULL)
	    return ENETUNREACH;
	  i6a = (struct in6_ifaddr *)inp->inp_route.ro_rt->rt_ifa;
	}
    }

  /*
   * If PCB has options hanging off of it, insert them here.
   */

  DPRINTF(IDL_GROSS_EVENT,("ipv6_icmp_usrreq(): dst is "));
  DDO(IDL_GROSS_EVENT,dump_in_addr6(dst));

  M_PREPEND(m,sizeof(struct ipv6),M_WAIT);
  if (m == NULL)
    panic("M_PREPEND died in ipv6_icmp_usrreq().\n");

  DPRINTF(IDL_EVENT,("Before m_pullup() for %d bytes.\n",\
			   sizeof(struct ipv6) + ICMPV6_MINLEN));
  DDO(IDL_GROSS_EVENT,dump_mchain(m));
  if ((m = m_pullup(m,sizeof(struct ipv6) + ICMPV6_MINLEN)) == NULL)
    {
      DPRINTF(IDL_CRITICAL,("m_pullup in ipv6_icmp_usrreq() failed.\n"));
      return ENOBUFS;  /* Any better ideas? */
    }

  ipv6 = mtod(m,struct ipv6 *);
  ipv6->ipv6_length = len;
  ipv6->ipv6_nexthdr = NEXTHDR_ICMP;
  ipv6->ipv6_hoplimit = MAXHOPLIMIT;
  ipv6->ipv6_dst = *dst;
  ipv6->ipv6_versfl = htonl(0x60000000);  /* Plus flow label stuff. */
  /*
   * i6a pointer should be checked here.
   */
  ipv6->ipv6_src = i6a->i6a_addr.sin6_addr;

  icp = (struct ipv6_icmp *)(m->m_data + sizeof(struct ipv6));
  icp->icmp_cksum = 0;
  icp->icmp_cksum = in6_cksum(m,NEXTHDR_ICMPV6,ipv6->ipv6_length,
			      sizeof(struct ipv6));

  DPRINTF(IDL_GROSS_EVENT,("ipv6_icmp_usrreq(): Headers are\n"));
  DDO(IDL_GROSS_EVENT,dump_ipv6(ipv6));
  DDO(IDL_GROSS_EVENT,dump_ipv6_icmp(icp));

  /*
   * After this comment block you'd probably insert options,
   * and adjust lengths accordingly.
   */ 

  /*
   * Temporarily tweak INP_HDRINCL to fool ripv6_output().  I still don't
   * know how a user who sets INP_HDRINCL for real will prepare ICMP packets.
   * Also, set up data structures for callback routine in ipv6_output().
   */

  if (!(sotoinpcb(so)->inp_flags & INP_HDRINCL))
    {
      sotoinpcb(so)->inp_flags |= INP_HDRINCL;
      tflags = 1;
    }
  rc = ripv6_output(m,so,dst);
  if (!(so->so_state & SS_ISCONNECTED) && !(so->so_options & SO_DONTROUTE))
    {
      RTFREE(inp->inp_route.ro_rt);
      inp->inp_route.ro_rt = NULL;
    }
  if (tflags)
    sotoinpcb(so)->inp_flags &= ~INP_HDRINCL;

  return rc;
}
