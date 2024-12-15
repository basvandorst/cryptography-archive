/*
 * ipv6_output.c  --  Output (down the protocol stack toward the interface)
 *                    routines for IPv6.  Also contains fragmentation
 *                    code, and IPv6-layer socket option handling.
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
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>

#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <netinet6/in6_debug.h>

#include <netinet6/ipsec.h>
#include <netinet6/ipsec_ah.h>
#include <netinet6/ipsec_esp.h>
#include <netinet6/key.h>
#include <netinet6/ipsec_policy.h>
#include <netinet6/ipsec_queue.h>

/*
 * Globals and function definitions.
 */

u_int32 outfragid = 0;  /* Outbound fragment groups have unique id's. */
struct mbuf *ipv6_fragment __P((struct mbuf *,int));

/*
 * External globals.
 */

extern struct ipv6stat ipv6stat;
extern struct in6_ifaddr *in6_ifaddr;

/*----------------------------------------------------------------------
 * IPv6 output routine.  The mbuf chain contains a near-complete IPv6 header,
 * and an already-inserted list of options.  (I figure it's something for
 * the code with PCB access to handle.)  The options should have their
 * fields in network order.  The header should have its fields in host order.
 * (Save the addresses, which IMHO are always in network order.  Weird.)
 ----------------------------------------------------------------------*/

int
ipv6_output(outgoing,ro,flags,i6mo)
     struct mbuf *outgoing;
     struct route6 *ro;
     int flags;
     struct ipv6_moptions *i6mo;
{
  struct ipv6 *header;
  struct route6 ipv6route;
  struct sockaddr_in6 *dst, secsrc;
  struct in6_ifaddr *i6a;
  struct ifnet *ifp;
  int error=0;
  u_int32 outmtu = 0;
  struct key_tblnode *ah_ktn;
  struct key_tblnode *esp_n_ktn;
  struct key_tblnode *esp_t_ktn;
  u_int needed;
  u_int SA_ready;
  u_int status = 0;

#ifdef DIAGNOSTIC
  if ((outgoing->m_flags & M_PKTHDR) == 0)
    panic("ipv6_output() no HDR");
#endif

  /*
   * Assume the IPv6 header is already contiguous.
   */
  header = mtod(outgoing, struct ipv6 *);
  
  DPRINTF(IDL_GROSS_EVENT,("Entered ipv6_output(): "));
  DDO(IDL_GROSS_EVENT,printf("header is:\n");dump_ipv6(header));
  DPRINTF(IDL_GROSS_EVENT,("\n"));

  /* 
   * Fill in v6 header.  Assume flow id/version field is in host order,
   * and that the high 4 bits are 0's.  Convert that field to network order.
   */
  
  if ((flags & (IPV6_FORWARDING|IPV6_RAWOUTPUT)) == 0)
    {
      header->ipv6_versfl = htonl(0x60000000 |
				  (header->ipv6_versfl & 0xf0000000));
      ipv6stat.ips_localout++;
    }
  
  /*
   * Determine interface and physical destination to send datagram out
   * towards.  Do this by looking up a route, or using the route we were
   * passed.
   */

  if (ro == 0)
    {
      ro = &ipv6route;
      bzero((caddr_t)ro,sizeof(struct route6));
    }
  dst = &ro->ro_dst;
  
  if (ro->ro_rt && ((ro->ro_rt->rt_flags & RTF_UP) == 0 ||
		    !IN6_ADDR_EQUAL(dst->sin6_addr,header->ipv6_dst) ))
    {
      RTFREE(ro->ro_rt);
      ro->ro_rt = NULL;
    }
  
  if (ro->ro_rt == NULL)
    {
      dst->sin6_family = AF_INET6;
      dst->sin6_len = sizeof(struct sockaddr_in6);
      dst->sin6_addr = header->ipv6_dst;
      dst->sin6_port = 0;
      dst->sin6_flowinfo = 0;
    }
  
#define ifatoi6a(ifa) ((struct in6_ifaddr *)(ifa))
#define sin6tosa(sin6) ((struct sockaddr *)(sin6))
  
  if (flags & IPV6_ROUTETOIF)
    {
      /*
       * Check for route to interface only.  (i.e. the user doesn't want
       * to rely on the routing tables, so send it out an interface).
       */
      if ((i6a = ifatoi6a(ifa_ifwithdstaddr(sin6tosa(dst)))) == 0 &&
	  (i6a = ifatoi6a(ifa_ifwithnet(sin6tosa(dst)))) == 0 )
	{
	  ipv6stat.ips_noroute++;
	  error = ENETUNREACH;
	  goto bad;
	}
      ifp = i6a->i6a_ifp;
      header->ipv6_hoplimit = 1;
      outmtu = ifp->if_mtu;
    }
  else
    {
      /*
       * Do normal next-hop determination with the help of the routing tree.
       */
      if (ro->ro_rt == 0)
	rtalloc((struct route *)ro);  /* Look up route. */
      if (ro->ro_rt == 0)
	{

	  /*
	   * Hmm, we have no route.  Assume 'on-link', but which link?
	   * For now, bail.  Later, do something about picking a link, or
	   * sending neighbor solicits out all links in hope of getting a
	   * response.
	   *
	   * It is for this reason netinet6/in6.c sets up a default interface
	   * for link-local addresses, as well as doing the same for multicast.
	   */

	  ipv6stat.ips_noroute++;  /* Better stat to keep? */
	  error = ENETUNREACH;
	  goto bad;
	}

      /*
       * Exploit properties of route.
       */
      
      ifp = ro->ro_rt->rt_ifp;            /* Q:  Is this REALLY the ifp
					     for the route?
					     
					     A:  Maybe.  If multi-homed,
					         and we attempt to
						 intelligently figure out
						 where link-locals are
						 destined, then we're
						 in trouble. */
      /*
       * On-net route exists, but no destination as of yet.  This can
       * be snipped if an ifp is just selected.  (Depends on multihomed
       * experience.)
       *
       * Currently, this code never executes, because we guarantee rt_ifp is
       * set.  This may, however, change in later versions of this code as
       * we gain multihomed experience.
       */
      if (ifp == NULL && ro->ro_rt->rt_gateway->sa_family == AF_LINK)
	{
	  DPRINTF(IDL_MAJOR_EVENT,\
		  ("ipv6_output() calling ipv6_nsolicit(2)\n"));
	  ipv6_nsolicit(NULL, outgoing, ro->ro_rt);
	  DPRINTF(IDL_MAJOR_EVENT,\
		  ("ipv6_output() attempted to send neighbor solicit(2), returning.\n"));
	  goto done;
	}

      /*
       * Q:  What if address has expired?  Perhaps I should use ifp to
       * obtain optimal i6a value.   There's also the question of using
       * link-local source addresses for off-link communication.  (or for
       * that matter, on-link, but not link-local communication.
       *
       * Q2:  Perhaps use this time to reset the route's ifa?
       * Q3:  Perhaps it is better to use the ipv6_rtrequest()?
       * 
       * Regardless, i6a's only use in this function is to determine the
       * source address of the packet.
       *
       * Currently, ipv6_rtrequest() attempts to store a decent in6_ifaddr
       * in rt_ifa.  This also may change with experience.
       */

      i6a = ifatoi6a(ro->ro_rt->rt_ifa);
      ro->ro_rt->rt_use++;
      /*
       * Path MTU comes from the route entry.
       */
      outmtu = ro->ro_rt->rt_rmx.rmx_mtu;
      
      if (ro->ro_rt->rt_flags & RTF_GATEWAY)  /* Gateway/router/whatever. */
	dst = (struct sockaddr_in6 *)ro->ro_rt->rt_gateway;
    }

  /*
   * Handle case of a multicast destination.
   */
  if (IS_IN6_MULTICAST(header->ipv6_dst))
    {
      struct in6_multi *in6m;
      extern struct ifnet loif;
      
      outgoing->m_flags |= M_MCAST;
      
      dst = &ro->ro_dst;
      
      if (ro->ro_rt == NULL)
	{
	  /*
	   * Figure out something for path MTU?  Or is this an error? 
	   * What if the source address has already been set? 
	   *
	   * What if I'm mcasting a solicit?  I'd really better do something
	   * here...
	   *
	   * This is why there is a default multicast interface and route set
	   * in netinet6/in6.c.
	   */
	  panic("No multicast route!\n");
	}
      
      if (i6mo != NULL)
	{
	  /*
	   * As we gain more multicast experience, use i6mo fields to alter
	   * properties of outgoing packet.  (Including, quite possibly,
	   * the source address.)
	   */
	  if (i6mo->i6mo_multicast_ifp != NULL)
	    ifp = i6mo->i6mo_multicast_ifp;
	}
      else
	{
	  /*
	   * Use default values, since there are no multicast options to
	   * use.
	   */
	}
      
      if ((ifp->if_flags & IFF_MULTICAST) == 0)
	{
	  ipv6stat.ips_noroute++;
	  error = ENETUNREACH;
	  goto bad;
	}
      
      if (IS_IN6_UNSPEC(header->ipv6_src))
	{
	  register struct in6_ifaddr *i6a;
	  
	  /*
	   * Source address selection for multicast datagrams.
	   *
	   * Eventually should fix this to perform best match.
	   */
	  for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
	    if (i6a->i6a_ifp == ifp)
	      {
		header->ipv6_src = I6A_SIN(i6a)->sin6_addr;
		break;
              }
        }

      IN6_LOOKUP_MULTI(&header->ipv6_dst, ifp, in6m);
      DPRINTF(IDL_EVENT,("in6m == 0x%x, i6mo == 0x%x\n",in6m,i6mo));
      if (in6m != NULL &&
	  (i6mo == NULL || i6mo->i6mo_multicast_loop))
	{
	  void ipv6_mloopback __P((struct ifnet *, struct mbuf *,\
				   struct sockaddr_in6 *));

	  /*
	   * See ipv6_mloopback for details, but that function will tag
	   * the m_pkthdr.rcvif with the actual interface the multicast is
	   * supposed to go out.  This makes duplicate address detection
	   * harder to implement, because the inbound mbuf SHOULD be tagged
	   * as coming from me for the case of solicits.  (Perhaps burning
	   * another flag...)
	   */
	  DPRINTF(IDL_EVENT,("Calling ipv6_mloopback().\n"));
	  ipv6_mloopback(ifp, outgoing, dst);
	}

#ifdef MROUTING
      /*
       * Do m-cast routing even if I can't find it in my m-cast list.
       */
#endif

      /*
       * If intra-node scope.  I've already hit it with ipv6_mloopback above.
       */

      if (GET_IN6_MCASTSCOPE(header->ipv6_dst) == IN6_INTRA_NODE ||
	  ifp == &loif)
	goto bad;  /* Not really bad, y'know, just getting out of here. */
    }

  if (ro->ro_rt == NULL && outmtu == 0)
    panic("How the hell did I get here without a route or MTU?");

  /*
   * Specify source address.  Use i6a, for now.
   */

  if (IS_IN6_UNSPEC(header->ipv6_src))
    header->ipv6_src = I6A_SIN(i6a)->sin6_addr;

  DPRINTF(IDL_GROSS_EVENT,("header & chain before security check are:\n"));
  DDO(IDL_GROSS_EVENT,dump_ipv6(header));
  DDO(IDL_GROSS_EVENT,dump_mchain(outgoing));

  if (!(flags & IPV6_FORWARDING))
    {
      /*
       * Do security processing now that we have necessary information
       * figured out. 
       * 
       * WARNING:  These three functions WILL alter the mbuf chain.  It is
       *           very possible that the header pointer will be invalid after
       *           these calls.
       */
      
      /* We call ipsec_output_policy() to find out which if any IPsec
	 services should be performed on outgoing.  That function will
	 return whether a service is needed, whether the requisite SAs
	 are ready now/not yet ready/will never be ready, and a key_tblnode
	 pointer for each service that is needed.  The SA hangs off of
	 the key_tblnode.  We pass key_tblnode ptrs around because we
	 will need to call key_free() to decrement the SA refct inside
	 the Key Engine once we are finished with SA.  key_free() needs
	 a key_tblnode ptr as its input parameter.  rja */

      /*
       * I would like to just pass in &ia->ia_addr, but there is a small
       * chance that the source address doesn't match ia->ia_addr.
       *
       * Also, if you need a dest. port, fill in ro->ro_dst with it.
       */
      secsrc.sin6_family = AF_INET6;
      secsrc.sin6_len = sizeof(secsrc);
      secsrc.sin6_port = 0;
      secsrc.sin6_addr = header->ipv6_src;

      ipsec_output_policy((struct socket *)outgoing->m_pkthdr.rcvif, 
			  (struct sockaddr *)&secsrc,
			  (struct sockaddr *)&ro->ro_dst, &needed, &SA_ready, 
			  &ah_ktn, &esp_t_ktn, &esp_n_ktn);

      DPRINTF(IDL_MAJOR_EVENT, ("ipv6_out: needed is %d \n", needed));

      /* ASSERT: ipsec_output_policy() never returns a NULL key_tblnode ptr
	 for a security service whose "needed" flag bit is set when
	 SA_ready is not 0. */
      if (needed) {
	if (SA_ready == IPSEC_SA_READY) {
	  /* all three SAs exist and the *_ktns are all valid now */
	  if (needed & IPSEC_ESP_TRANSPORT_NEEDED) {
	    status = ipsec_esp_transport_output(outgoing, 
						esp_t_ktn->secassoc);
	    key_free(esp_t_ktn);
	    esp_t_ktn = NULL;
	    if (status != 0) 
	      goto ipsec_cleanup;
	  }
	  
	  if (needed & IPSEC_AH_NEEDED) {
	    outgoing = ipsec_ah_output(outgoing, ah_ktn->secassoc);
	    key_free(ah_ktn);
	    ah_ktn = NULL;
	    if (outgoing == NULL)
	      goto ipsec_cleanup;
	  }
	  
	  if (needed & IPSEC_ESP_TUNNEL_NEEDED) {
	    status = ipsec_esp_tunnel_output(outgoing, esp_n_ktn->secassoc);
	    key_free(esp_n_ktn);
	    esp_n_ktn = NULL;
	    if (status != 0) 
	      goto ipsec_cleanup;
	  }
	} else if (SA_ready == 1) {
	  /* not all SAs are ready yet, but they're coming eventually,
	     so queue the outgoing packet on the ipsec waiting pkt que 
	     and return. */
	  ipsec_enqueue(outgoing);
	  return (0);
	} else {
	  /* not all SAs are ready and Key Engine says to give up, 
	     so drop this packet and increment appropriate counters */
	  goto ipsec_cleanup;
	} /* endif SA_ready */
      } /* endif IP security is needed */
    } /* endif not forwarding. */

  /*
   * Assume above three return a contiguous and UPDATED IPv6 header.
   */
  header = mtod(outgoing,struct ipv6 *);

  /*
   * If small enough for path MTU, send, otherwise, fragment.
   */

  DPRINTF(IDL_GROSS_EVENT,("Before output, path mtu = %d, header is:\n",\
			   outmtu));
  DDO(IDL_GROSS_EVENT,dump_ipv6(header));
  DDO(IDL_GROSS_EVENT,printf("Chain is:\n");dump_mchain(outgoing));

  if (outgoing->m_pkthdr.len <= outmtu)
    {
      header->ipv6_length = htons(outgoing->m_pkthdr.len - 
				  sizeof(struct ipv6));


      /*
       * If there is a route, and its TUNNEL bit is turned on, do not send
       * out the interface, but send through a tunneling routine, which will,
       * given information from the route, encapsulate the packet accordingly.
       *
       * Keith Sklower suggested a "rt_output() method" which would save
       * the checking here.
       */
      if (ro->ro_rt && (ro->ro_rt->rt_flags & RTF_TUNNEL))
	{
	  DPRINTF(IDL_MAJOR_EVENT,("Sending out tunnel.\n"));
	  error = ipv6_tunnel_output(outgoing, dst, ro->ro_rt);
	}
      else error = (*ifp->if_output)(ifp, outgoing, (struct sockaddr *)dst,
				ro->ro_rt);
      DPRINTF(IDL_GROSS_EVENT,("Lone IPv6 went out if (error = %d).\n",error));
      goto done;
    }

  /*
   * If I make it here, then the packet is too big for the path MTU, and must
   * be fragmented.
   */

  DPRINTF(IDL_GROSS_EVENT,("Entering fragmenting engine.\n"));

  if (outgoing->m_pkthdr.len > 0xffff) {
    DPRINTF(IDL_CRITICAL,("Jumbogram needs fragmentation, something that can't be done\n"));
    ipv6stat.ips_odropped++;  /* ?!? */
    error = EINVAL;
    goto bad;
  }

  /*
   * The following check should never really take place.
   */
#ifdef DIAGNOSTIC
  if (outmtu < IPV6_MINMTU)
    {
      DPRINTF(IDL_CRITICAL,("Outbound MTU is less than IPV6_MINMTU (%d).\n",\
			    IPV6_MINMTU));
      error = ENETUNREACH;  /* Can you think of a better idea? */
      goto bad;
    }
#endif

  /*
   * ipv6_fragment returns a chain of outgoing packets.  It returns NULL
   * if something went wrong.
   */
  outgoing = ipv6_fragment(outgoing,outmtu);
  if (outgoing == NULL)
    error = ENOBUFS;    /* Can you think of a better idea? */

  DPRINTF(IDL_GROSS_EVENT,\
       ("ipv6_fragment() returned, attempting to send fragments out.\n"));

  /*
   * Walk through chain of fragments, and send them out.
   */
  while (outgoing != NULL)
    {
      struct mbuf *current = outgoing;

      DPRINTF(IDL_GROSS_EVENT,("In fragment-sending loop, error == %d.\n",\
			       error));
      outgoing = current->m_nextpkt;
      current->m_nextpkt = NULL;

      DDO(IDL_FINISHED,printf("Current (0x%x) 1st mbuf is:\n",current);\
	  dump_mbuf(current));

      if (error != 0)
	m_freem(current);
      else
	if (ro->ro_rt && (ro->ro_rt->rt_flags & RTF_TUNNEL))
	  {
	    DPRINTF(IDL_MAJOR_EVENT,("Sending fragments out tunnel.\n"));
	    error = ipv6_tunnel_output(current, dst, ro->ro_rt);
	  }
	else error = (*ifp->if_output)(ifp, current,(struct sockaddr *)dst,
				       ro->ro_rt);
      DPRINTF(IDL_MAJOR_EVENT,("After if_output(), error == %d.\n",error));
    }

  if (error == 0)
    ipv6stat.ips_fragmented++;

 done:
  if (ro == &ipv6route && (flags & IPV6_ROUTETOIF) == 0 && ro->ro_rt)
    RTFREE(ro->ro_rt);
  return (error);

ipsec_cleanup:
	error = EIPSEC; /* IPsec general processing failure */
	if (ah_ktn != NULL)
	  key_free(ah_ktn);
	if (esp_t_ktn != NULL)
	  key_free(esp_t_ktn);
	if (esp_n_ktn != NULL)
	  key_free(esp_n_ktn);

 bad:
  if (outgoing != NULL)
    m_freem(outgoing);
  goto done;
}

/*----------------------------------------------------------------------
 * Handler for IPV6 [gs]etsockopt() calls.  One problem arises when an
 * AF_INET6 socket actually wants to set IPv4 options.
 *
 * The decision to call this or to call ip_ctloutput() is best left in
 * the hands of TCP/UDP/etc., which have information about which IP is
 * in use.
 *
 * This function is far from complete, and the ADDRFORM is solely in there
 * to test the bare functionality.
 ----------------------------------------------------------------------*/

int
ipv6_ctloutput (op, so, level, optname, mp)
     int op;
     struct socket *so;
     int level, optname;
     struct mbuf **mp;
{
  int error = 0;
  struct mbuf *m = *mp;

  if (level != IPPROTO_IP)
    {
      if (op == PRCO_SETOPT && *mp)
	(void) m_free(*mp);
      return EINVAL;
    }

  switch (op)
    {
    case PRCO_SETOPT:
      if (m != NULL)
	m_free(m);
      break;
    case PRCO_GETOPT:
      switch(optname)
	{
	case IPV6_ADDRFORM:
	  DPRINTF(IDL_EVENT, ("ipv6_ctloutput(): Reached IPV6_ADDRFORM\n"));
	  *mp = m = m_get(M_WAIT, MT_SOOPTS);
	  m->m_len = sizeof(int);
	  *mtod(m, int *) = AF_INET6;
	  break;
	}
      break;
    default:
      error = EINVAL;
      break;
    }

  return error;
}

/*----------------------------------------------------------------------
 * Loops back multicast packets to groups of which I'm a member.
 ----------------------------------------------------------------------*/

void
ipv6_mloopback(ifp, m, dst)
     struct ifnet *ifp;
     register struct mbuf *m;
     register struct sockaddr_in6 *dst;
{
  struct mbuf *copym;
  register struct ipv6 *header;

  /*
   * Copy mbuf chain in m, and send to loopback interface.
   */

  copym=m_copym(m,0,M_COPYALL,M_NOWAIT);
  if (copym != NULL)
    {
      header=mtod(copym,struct ipv6 *);
      /* Jumbogram? */
      header->ipv6_length = htons(header->ipv6_length);
      /*
       * Also, there's an issue about address collision.  You may want to
       * check the ipv6 destination (or the dst address) and set the ifp
       * argument to looutput to be the loopback interface itself iff
       * it is to a solicited nodes multicast.
       *
       * Then again, it may be easier for the soliciting code to burn another
       * m_flags bit, and look for it on loopback.
       */
      looutput(ifp,copym,(struct sockaddr *)dst,NULL);
    }
  else 
    DPRINTF(IDL_EVENT,("m_copym() failed in ipv6_mloopback.\n"));
}

/*----------------------------------------------------------------------
 * Fragment IPv6 datagram.
 *
 * Given a large mbuf chain m, with only its ipv6->ipv6_length field in
 * host order, fragment into mtu sized chunks, and return a meta-chain
 * with m->m_nextpkt being the subsequent fragments.  If there's a problem,
 * m_freem all fragments, and return NULL.  Also, all ipv6->ipv6_length
 * fields are in network order, i.e. ready-to-transmit.
 *
 * Note that there is an unusually large amount of calls to m_pullup,
 * m_copym2etc. here.  This will cause performance hits.
 *
 * A redesign of this is in order, but that will have to wait.
 ----------------------------------------------------------------------*/

struct mbuf *
ipv6_fragment(m,mtu)
     struct mbuf *m;
     int mtu;
{
  u_int8 nextopt = NEXTHDR_FRAGMENT;
  u_int divpoint = sizeof(struct ipv6),maybe = 0, chunksize, sofar = 0, goal;
  struct mbuf *retval = NULL, *newfrag = NULL;
  struct ipv6_fraghdr *frag = NULL;

  outfragid++;

  DPRINTF(IDL_GROSS_EVENT,\
	  ("Entering ipv6_fragment, m_pkthdr.len = %d, mtu = %d\n",\
	   m->m_pkthdr.len, mtu));

  /*
   * Find the dividing point between pre-fragment and post-fragment options.
   */
  divpoint = ipv6_finddivpoint(m, &nextopt);

  /*
   * Options being larger than MTU can happen, especially given large routing
   * headers and large options bags.
   */
  if (divpoint + sizeof(struct ipv6_fraghdr) >= mtu)
    {
      DPRINTF(IDL_CRITICAL, 
	      ("ipv6_fragment(): Options are larger than passed-in MTU.\n"));
      m_freem(m);
      return NULL;
    }

#ifdef DIAGNOSTIC
  if (divpoint & 0x7)
    panic("divpoint not a multiple of 8!");
#endif

  /*
   * sofar keeps track of how much I've fragmented, chunksize is how
   * much per fragment, goal is how much data to actually fragment.
   */
  chunksize = mtu - divpoint - sizeof(struct ipv6_fraghdr);
  chunksize &= ~7;
  goal = m->m_pkthdr.len - divpoint;

  DPRINTF(IDL_GROSS_EVENT, \
	  ("Found divpoint (%d), nextopt (%d), chunksize(%d) goal(%d)\n",\
	   divpoint, nextopt, chunksize,goal));

  while (sofar < goal)
    if ((newfrag = m_copym2(m, 0, divpoint, M_NOWAIT)) != NULL)
      {
	struct mbuf *fraghdrmbuf = NULL;
	struct ipv6 *ipv6 = NULL;
	u_int tocopy = (chunksize <= (goal - sofar))?chunksize:(goal - sofar);

	DPRINTF(IDL_GROSS_EVENT,("tocopy == %d\n",tocopy));

	/*
	 * Create a new IPv6 fragment, using the header that was slightly
	 * munged by ipv6_finddivpoint().
	 *
	 * The above m_copym2() creates a copy of the first
	 */

	newfrag->m_nextpkt = retval;
	retval = newfrag;

	/*
	 * Append IPv6 fragment header to pre-fragment
	 */
	for(fraghdrmbuf = retval;fraghdrmbuf->m_next != NULL;)
	  fraghdrmbuf = fraghdrmbuf->m_next;
	MGET(fraghdrmbuf->m_next,M_NOWAIT,MT_DATA);
	if (fraghdrmbuf->m_next == NULL)
	  {
	    DPRINTF(IDL_CRITICAL,("couldn't get new mbuf for frag hdr\n"));
	    ipv6stat.ips_odropped++;
	    goto bail;  
	  }

	fraghdrmbuf = fraghdrmbuf->m_next;
	fraghdrmbuf->m_len = sizeof(struct ipv6_fraghdr);
	retval->m_pkthdr.len += sizeof(struct ipv6_fraghdr);
	frag = mtod(fraghdrmbuf,struct ipv6_fraghdr *);
	frag->frag_nexthdr = nextopt;
	frag->frag_reserved = 0;
	frag->frag_bitsoffset = htons(sofar | ((sofar + tocopy) < goal));
	frag->frag_id = outfragid;

	/*
	 * Copy off (rather than just m_split()) the portion of data that
	 * goes with this fragment.
	 */
	if ((fraghdrmbuf->m_next = m_copym2(m,divpoint + sofar,tocopy,
					    M_NOWAIT)) == NULL)
	  {
	    DPRINTF(IDL_CRITICAL,("couldn't copy segment.\n"));
	    goto bail;  
	  }
	retval->m_pkthdr.len += tocopy;

	/*
	 * Update fragment's IPv6 header appropriately.
	 */
	ipv6 = mtod(retval,struct ipv6 *);
	ipv6->ipv6_length = htons(retval->m_pkthdr.len - sizeof(struct ipv6));
	ipv6->ipv6_nexthdr = NEXTHDR_FRAGMENT;
	ipv6stat.ips_ofragments++;
	sofar += tocopy;
      }
    else
      {
	/*
	 * Creation of new fragment failed.
	 */
	DPRINTF(IDL_CRITICAL,("m_copym2() failed in fragmentation loop.\n"));
	ipv6stat.ips_odropped++;
      bail:   
	DPRINTF(IDL_CRITICAL,("Bailing out of ipv6_fragment()\n"));
	while (retval != NULL)
	  {
	    newfrag = retval;
	    retval = retval->m_nextpkt;
	    m_freem(newfrag);
	  }
	m_freem(m);
	return NULL;
      }

  m_freem(m);

  /* Dump mbuf chain list constructed for debugging purposes. */
  DDO(IDL_GROSS_EVENT,\
      for (newfrag = retval; newfrag != NULL; newfrag = newfrag->m_nextpkt)\
        dump_mbuf(newfrag) );

  return retval;
}

/*----------------------------------------------------------------------
 * Find the dividing point between pre-fragment and post-fragment options.
 * The argument nexthdr is read/write, on input, it is the next header
 * value that should be written into the previous header's "next hdr" field,
 * and what is written back is what used to be in the previous field's
 * "next hdr" field.  For example:
 *
 * IP (next hdr = routing)       becomes -->     IP (next hdr = routing)
 * Routing (next hdr = TCP)                      Routing (next hdr = fragment)
 * TCP                                           TCP
 * argument nexthdr = fragment                   argument nexthdr = TCP
 *
 * This function returns the length of the pre-fragment options, ideal for
 * calls to m_split.
 *
 * As in ipv6_fragment, too many calls to m_pullup/m_pullup2 are performed
 * here.  Another redesign is called for, but not now.
 ----------------------------------------------------------------------*/

int
ipv6_finddivpoint(m, nexthdr)
     struct mbuf *m;
     u_int8 *nexthdr;
{
  u_int8 iprevopt, *prevopt = &(iprevopt), new = *nexthdr;
  u_int8 *nextopt;
  u_int divpoint,maybe = 0, chunksize, sofar = 0, goal;
  struct mbuf *retval = NULL, *newfrag = NULL;
  struct ipv6_fraghdr *frag = NULL;

  /*
   * IPv4 authentication code calls this function too.  It is likely that
   * v4 will just return almost immedately, after determining options
   * length. (i.e. never go through the while loop.)
   */
  if (mtod(m, struct ip *)->ip_v == 4)
    {
      iprevopt = NEXTHDR_IPV4;
      nextopt = &(mtod(m, struct ip *)->ip_p);
      divpoint = sizeof(struct ip);
    }
  else
    {
      iprevopt = NEXTHDR_IPV6;
      nextopt = &(mtod(m, struct ipv6 *)->ipv6_nexthdr);
      divpoint = sizeof(struct ipv6);
    }

  /*
   * Scan through options finding dividing point.  Dividing point
   * for authentication and fragmentation is the same place.
   *
   * Some weirdness here is that there MIGHT be a "Destination options bag"
   * which is actually a "per source-route-hop" bag.  There is a strong
   * argument for giving this particular options bag a separate type, but
   * for now, kludge around it.
   *
   * The "maybe" variable takes into account the length of this options bag.
   */
  while (IS_PREFRAG(*nextopt) && *prevopt != NEXTHDR_ROUTING)
    {
      struct ipv6_srcroute0 *i6sr;
      struct ipv6_opthdr *oh;
      
      /*
       * ASSUMES: both nextopt and length will be in the first
       *          8 bytes of ANY pre-fragment header.
       */

      if ((divpoint + maybe + 8) > MHLEN)
	{
	  /* 
	   * This becomes complicated.  Try and collect invariant part into
	   * first (now cluster) mbuf on chain.  m_pullup() doesn't work with
	   * clusters, so either write m_pullup2() or inline it here.
	   *
	   * m_pullup2(), unlike m_pullup() will only collect exactly
	   * how many bytes the user requested.  This is to avoid problems
	   * with m_copym() and altering data that is merely referenced
	   * multiple times, rather than actually copied.  (We may eliminate
	   * the Net/2 hack of adding m_copym2().)
	   */
	  if ((m = m_pullup2(m,divpoint + maybe + 8)) == NULL)
	    {
	      DPRINTF(IDL_CRITICAL,\
		      ("m_pullup2(%d) failed in ipv6_fragment().\n",\
		       divpoint + maybe + 8));
	      return 0;
	    }
	}
      else
	{
	  if ((m = m_pullup(m,divpoint + maybe + 8)) == NULL)
	    {
	      DPRINTF(IDL_CRITICAL,\
		      ("m_pullup() failed in ipv6_fragment().\n"));
	      return 0;
	    }
	}

      /*
       * Find nextopt, and advance accordingly.
       */
      switch (*nextopt)
	{
	case NEXTHDR_HOP:
	  /*
	   * Hop-by-hops should be right after IPv6 hdr.  If extra is nonzero,
	   * then there was a destination options bag.  If divpoint is not
	   * only the size of the IPv6 header, then something came before
	   * hop-by-hop options.  This is not good.
	   */
	  if (maybe || divpoint != sizeof(struct ipv6))
	    {
	      DPRINTF(IDL_CRITICAL, 
		      ("ipv6_input():  Weird ordering in headers.\n"));
	      m_freem(m);
	      return 0;
	    }
	  oh = (struct ipv6_opthdr *)(m->m_data + divpoint);
	  prevopt = nextopt;
	  nextopt = &(oh->oh_nexthdr);
	  divpoint += 8 + (oh->oh_extlen << 3);
	  if (oh->oh_extlen)
	    if (divpoint > MHLEN)
	      {
		if ((m = m_pullup2(m,divpoint)) == NULL)
		  {
		    DPRINTF(IDL_MAJOR_EVENT,\
			    ("m_pullup2(%d) failed in NEXTHDR_HOP nextopt.\n",\
			     divpoint));
		    return 0;
		  }
	      }
	    else
	      {
		if ((m = m_pullup(m,divpoint)) == NULL)
		  {
		    DPRINTF(IDL_MAJOR_EVENT,\
			    ("m_pullup() failed in NEXTHDR_HOP nextopt.\n"));
		    return 0;
		  }
	      }
	  break;
	case NEXTHDR_DEST:
	  oh = (struct ipv6_opthdr *)(m->m_data + divpoint);	
	  prevopt = nextopt;
	  nextopt = &(oh->oh_nexthdr);
	  maybe = 8 + (oh->oh_extlen << 3);
	  if (oh->oh_extlen)
	    if ( divpoint + maybe > MHLEN)
	      {
		if ((m = m_pullup2(m,divpoint + maybe)) == NULL)
		  {
		    DPRINTF(IDL_MAJOR_EVENT,\
			    ("m_pullup2(%d) failed in NEXTHDR_DEST nextopt.\n",\
			     divpoint+maybe));
		    return 0;
		  }
	      }
	    else
	      {
		if ((m = m_pullup(m,divpoint + maybe)) == NULL)
		  {
		    DPRINTF(IDL_MAJOR_EVENT,\
			    ("m_pullup() failed in NEXTHDR_DEST nextopt.\n"));
		    return 0;
		  }
	      }
	  break;
	case NEXTHDR_ROUTING:
	  if (maybe)  /* i.e. if we had a destination options bag before
			 this routing header, we should advance the dividing
			 point. */
	    divpoint += maybe;
	  maybe = 0;
	  i6sr = (struct ipv6_srcroute0 *)(m->m_data + divpoint);
	  prevopt = nextopt;
	  nextopt = &(i6sr->i6sr_nexthdr);
	  switch (i6sr->i6sr_type)
	    {
	    case 0:
	      divpoint += 8 + (i6sr->i6sr_numaddrs << 4);  /* *16? */
	      break;
	    default:
	      DPRINTF(IDL_CRITICAL, 
		      ("ipv6_input():  Unknown outbound routing header.\n"));
	      break;
	    }
	  if ( divpoint > MHLEN)
	      {
		if ((m = m_pullup2(m,divpoint)) == NULL)
		  {
		    DPRINTF(IDL_MAJOR_EVENT,\
			    ("m_pullup2(%d) failed in NEXTHDR_ROUTING nextopt.\n",\
			     divpoint));
		    return 0;
		  }
	      }
	    else
	      {
		if ((m = m_pullup(m,divpoint)) == NULL)
		  {
		    DPRINTF(IDL_MAJOR_EVENT,\
			    ("m_pullup() failed in NEXTHDR_ROUTING nextopt.\n"));
		    return 0;
		  }
	      }
	  break;
	}  /* End of switch statement. */
    };  /* End of while loop. */
    *nexthdr = *nextopt;
    *nextopt = new;
    return divpoint;
}
