/*
 * ipv6_trans.c  --  Contains entry points for Simple Internet Transition
 *                   (SIT) functionality.
 *                   Also contains (for now) ipv6_tunnel_output.  This file
 *                   will be split eventually into ipv6_tunnel.c, and
 *                   ipv6_trans.c
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/in6_debug.h>

#include <netinet6/ipsec.h>

/*
 * External globals.
 */

extern struct in6_ifaddr *in6_ifaddr;
extern int ipv6_defhoplmt;

/*----------------------------------------------------------------------
 * Called from ip_icmp.c, this function will reduce the tunnel path MTU
 * precisely.  I know I have enough to reconstruct the IPv6 header, which
 * is all I care about for this case.  Return 1 if m0 is intact, and 0 if
 * m0 is corrupted somehow.  Don't forget to update m0.
 ----------------------------------------------------------------------*/

int
ipv6_trans_mtu(m0, newmtu, len)
     struct mbuf **m0;
     int newmtu,len;
{
  struct ip *ip,*iip;
  struct ipv6 *ipv6;
  struct icmp *icp;
  struct rtentry *rt;
  struct sockaddr_in6 sin6;
  struct in6_ifaddr *i6a;

  /*
   * Make packet contiguous into one block of memory.  If the IPv6 header is
   * beyond MCLBYTES into the packet, then I'm in big trouble.
   */
  *m0 = m_pullup2(*m0,min(len,MCLBYTES));
  if (*m0 == NULL)
    return 0;

  ip = mtod(*m0,struct ip *);
  icp = (struct icmp *) ((caddr_t)ip + (ip->ip_hl << 2));
  iip = &icp->icmp_ip;
  ipv6 = (struct ipv6 *) ((caddr_t)iip + (iip->ip_hl << 2));

  /*
   * Verify source is one of mine?
   */
  for (i6a = in6_ifaddr; i6a != NULL; i6a = i6a->i6a_next)
    if (IN6_ADDR_EQUAL(i6a->i6a_addr.sin6_addr,ipv6->ipv6_src))
      break;
  if (i6a == NULL)
    {
      /* Packet didn't originate with me.  Drop it. */
      return 1;
    }

  /*
   * Find route for this destination and update it.
   */
  sin6.sin6_family = AF_INET6;
  sin6.sin6_len = sizeof(sin6);
  sin6.sin6_port = 0;
  sin6.sin6_flowinfo = 0;
  sin6.sin6_addr = ipv6->ipv6_dst;
  rt = rtalloc1((struct sockaddr *)&sin6, 0);
  if (rt == NULL)
    return 1;
  rt->rt_refcnt--;
  /*
   * Update path MTU.
   */
  if (!(rt->rt_flags & RTF_HOST))
    return 1;   /* Can't update path MTU on non-host-route. */
  if (rt->rt_rmx.rmx_mtu < newmtu - sizeof(struct ip))
    panic("MTU WEIRDNESS !!!");
  rt->rt_rmx.rmx_mtu = newmtu - sizeof(struct ip);
  return 1;
}

/*----------------------------------------------------------------------
 * Handle ICMP errors for IPv6-in-IPv4 tunnels.
 *
 * Security processing should be put in here, as it was with the other
 * ctlinput() functions, but with current ICMP implementations returning
 * only sizeof(struct ip) + 64 bits of offending packet.
 ----------------------------------------------------------------------*/
void
ipv6_trans_ctlinput(cmd, sa, ip, incoming)
     int cmd;
     struct sockaddr *sa;
     register struct ip *ip;
     struct mbuf *incoming;
{
  struct sockaddr_in *sin = (struct sockaddr_in *)sa;
  struct sockaddr_in6 sin6;
  struct ipv6 *ipv6;
  struct rtentry *rt;
  struct in6_ifaddr *i6a;

  sin6.sin6_family = AF_INET6;
  sin6.sin6_len = sizeof(sin6);
  sin6.sin6_port = 0;
  sin6.sin6_flowinfo = 0;
  DPRINTF(IDL_MAJOR_EVENT,("Entered ipv6_trans_ctlinput().\n"));

  /*
   * Do standard checks to see that all parameters are here.
   */
  if ((unsigned)cmd > PRC_NCMDS || sa->sa_family != AF_INET ||
      sin->sin_addr.s_addr == INADDR_ANY || ip == NULL)
    {
      DPRINTF(IDL_MAJOR_EVENT,("Failed one of the four checks.  Returning.\n"));
      return;
    }

  /*
   * Okay, at this point I have a contiguous IPv6 in IPv4 datagram.
   * I achieved this effect by convincing ip_icmp.[ch] to pull up
   * more than the first 64 bits.
   */

  ipv6 = (struct ipv6 *) ((caddr_t)ip + (ip->ip_hl << 2));
  /*
   * Verify source address is one of mine.
   */
  for (i6a = in6_ifaddr; i6a != NULL; i6a = i6a->i6a_next)
    if (IN6_ADDR_EQUAL(i6a->i6a_addr.sin6_addr,ipv6->ipv6_src))
      break;
  if (i6a == NULL)
    {
      /*
       * Packet didn't originate with me.  Drop it.
       */
      return;
    }

  sin6.sin6_addr = ipv6->ipv6_dst;
  rt = rtalloc1((struct sockaddr *)&sin6, 0);
  if (rt == NULL)
    return;
  rt->rt_refcnt--;

  switch (cmd)
    {
    case PRC_MSGSIZE:
      /*
       * This function was called because the actual MTU wasn't grokked 
       * from the ICMP packet.
       *
       * If I get this, drop to IPV6_MINMTU.  If the actual MTU was in the
       * ICMP packet and was read correctly, it went up a different codepath.
       *
       * RFC 1191 talks about a plateau table.   Here's the place to do it,
       * either that, or on increase.
       */
      if (rt->rt_flags & RTF_HOST)
	{
	  /*
	   * Only attempt path MTU update if I'm a host.
	   */
	  if (rt->rt_rmx.rmx_mtu == IPV6_MINMTU)
	    panic("Too big on v6 MTU of 576!!!");
	  rt->rt_rmx.rmx_mtu = IPV6_MINMTU;
	}
      break;
    case PRC_UNREACH_NET:
      rt->rt_flags &= ~RTF_HOST;  /* Is this wise?  I'm doing this to return
				     the right error on future requests. */
      /* FALLTHROUGH */
    case PRC_UNREACH_HOST:
    case PRC_UNREACH_PROTOCOL:
      /*
       * Other end isn't a v6/v4 node.
       */
      rt->rt_flags |= RTF_REJECT;  /* Don't want to send any packets. */
      break;
    default:
      break;
    }
}

/*----------------------------------------------------------------------
 * Output routine for IPv6 in IPv4.  Uses M_PREPEND to prepend an IPv4
 * header, and call ip_output().
 ----------------------------------------------------------------------*/

int
ipv6_trans_output(outgoing, v4dst, v6rt)
     struct mbuf *outgoing;
     struct sockaddr_in *v4dst;
     struct rtentry *v6rt;
{
  struct route v4route;
  struct ip *ip;
  int rc;

  /*
   * Like all below-IP(v6) output routines, check RTF_REJECT flag.
   */
  if (v6rt->rt_flags & RTF_REJECT)
    {
      m_freem(outgoing);
      return (v6rt->rt_flags & RTF_HOST) ? EHOSTUNREACH : ENETUNREACH;
    }

  v4route.ro_rt = NULL;
  bcopy(v4dst,&v4route.ro_dst,v4dst->sin_len);

  /*
   * Prepend IPv4 header.
   */
  M_PREPEND(outgoing,sizeof(struct ip), M_DONTWAIT);
  if (outgoing == NULL)
    return ENOBUFS;

  ip = mtod(outgoing,struct ip *);
  bzero(ip,sizeof(*ip));

  /*
   * Following four lines are done here rather than ip_output() because of
   * the *&*&%^^& don't fragment bit.
   */
  ip->ip_v = IPVERSION;
  ip->ip_id = htons(ip_id++);
  ip->ip_hl = sizeof(*ip)>>2;
  if (v6rt->rt_rmx.rmx_mtu > IPV6_MINMTU)
    ip->ip_off |= IP_DF;
  ipstat.ips_localout++;

  ip->ip_dst = v4dst->sin_addr;
  ip->ip_src.s_addr = INADDR_ANY;
  ip->ip_p = IPPROTO_IPV6;
  ip->ip_ttl = ip_defttl;
  ip->ip_len = outgoing->m_pkthdr.len;

  rc = ip_output(outgoing, NULL, &v4route, IP_RAWOUTPUT, NULL);
  if (rc == EMSGSIZE)
    {
      DPRINTF(IDL_CRITICAL,("Path MTU adjustment needed in trans_output().\n"));
    }
  if (v4route.ro_rt != NULL)
    RTFREE(v4route.ro_rt);
  return rc;
}

/*----------------------------------------------------------------------
 * Encapsulate IPv6 packet in another IPv6 packet.  This, in combination
 * with setting the outgoing packet's rcvif pointer to a doctored socket,
 * can enable a configured secure tunnel.
 ----------------------------------------------------------------------*/

int
ipv6_encaps_output(outgoing, tundst, tunrt)
     struct mbuf *outgoing;
     struct sockaddr_in6 *tundst;
     struct rtentry *tunrt;
{
  struct route6 actroute;
  struct ipv6 *ipv6;
  int rc;

  if (tunrt->rt_flags & RTF_REJECT)
    {
      m_freem(outgoing);
      return (tunrt->rt_flags & RTF_HOST) ? EHOSTUNREACH : ENETUNREACH;
    }

  actroute.ro_rt = NULL;
  bcopy(tundst,&actroute.ro_dst,tundst->sin6_len);

  M_PREPEND(outgoing,sizeof(struct ipv6), M_DONTWAIT);
  if (outgoing == NULL)
    return ENOBUFS;

  ipv6 = mtod(outgoing,struct ipv6 *);
  bzero(ipv6,sizeof(*ipv6));

  
  ipv6->ipv6_versfl = htonl(0x60000000);
  ipv6->ipv6_length = outgoing->m_pkthdr.len - sizeof(struct ipv6);
  ipv6->ipv6_nexthdr = NEXTHDR_IPV6;
  ipv6->ipv6_hoplimit = ipv6_defhoplmt;
  ipv6->ipv6_dst = tundst->sin6_addr;

  if (tunrt->rt_flags & (RTF_CRYPT|RTF_AUTH))
    {
      /*
       * A secure route has hanging off its llinfo field something which
       * can be tagged onto an outgoing mbuf such that ipv6_output can
       * secure the IPv6-in-IPv6 packet.
       */
      DPRINTF(IDL_MAJOR_EVENT,("Secure route, sending cheesy socket.\n"));
      outgoing->m_pkthdr.rcvif = (struct ifnet *) tunrt->rt_llinfo;
    }

  rc = ipv6_output(outgoing, &actroute, IPV6_RAWOUTPUT, NULL);

  if (rc == EMSGSIZE)
    {
      DPRINTF(IDL_CRITICAL,("Path MTU adjustment needed in trans_output().\n"));
    }
  if (actroute.ro_rt != NULL)
    RTFREE(actroute.ro_rt);
  return rc;
  
}

/*----------------------------------------------------------------------
 * Called by ipv6_output if the RTF_TUNNEL bit is set on a route,
 * this function examines the route, and sees what sort of encapsulation is
 * needed.  Often, the rt->rt_gateway sockaddr is used to figure this out.
 ----------------------------------------------------------------------*/

int
ipv6_tunnel_output(outgoing, dst, rt)
     struct mbuf *outgoing;
     struct sockaddr_in6 *dst;
     struct rtentry *rt;
{
  int rc;

  DPRINTF(IDL_MAJOR_EVENT,("In ipv6_tunnel_output().\n"));

  /*
   * Determine what type of tunnel it is with rt.  Perform correct kind
   * of encapsulation (in IPv4, ESP, etc.) and call output routine of
   * what you want encapsulated.
   */

  /* IPv6 in IPv4. */
  if (rt->rt_gateway != NULL && rt->rt_gateway->sa_family == AF_INET)
    return ipv6_trans_output(outgoing,(struct sockaddr_in *)rt->rt_gateway,rt);

  /* IPv6 in IPv6. */
  if (rt->rt_gateway != NULL && rt->rt_gateway->sa_family == AF_INET6)
    return ipv6_encaps_output(outgoing,(struct sockaddr_in6 *)rt->rt_gateway,rt);

  m_freem(outgoing);
  return EHOSTUNREACH;
}
