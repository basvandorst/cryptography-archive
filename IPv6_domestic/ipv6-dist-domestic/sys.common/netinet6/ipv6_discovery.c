/*----------------------------------------------------------------------
 * ipv6_discovery.c -- Routines for IPv6 Neighbor Discovery.
 *                     These routines should be generic enough to work
 *                     with most interface types, unlike IPv4 ARP
 *
 * Copyright 1995 by Dan McDonald, Bao Phan, and Randall Atkinson,
 *	All Rights Reserved.  
 *      All Rights under this copyright have been assigned to NRL.
 *      The NRL Copyright Notice and License below apply to distribution
 *      and use of this software and documentation.
 */
/*---------------------------------------------------------------------- 
#	@(#)COPYRIGHT	1.1 (NRL) 17 January 1995

COPYRIGHT NOTICE

All of the documentation and software included in this software
distribution from the US Naval Research Laboratory (NRL) are
copyrighted by their respective developers.

This software and documentation were developed at NRL by various
people.  Those developers have each copyrighted the portions that they
developed at NRL and have assigned All Rights for those portions to
NRL.  Outside the USA, NRL has copyright on some of the software
developed at NRL. The affected files all contain specific copyright
notices and those notices must be retained in any derived work.

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
   must display the following acknowledgement:

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
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/if_dl.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h>
#include <netinet6/in6_debug.h>

#include <netinet6/ipsec.h>


/*
 * Globals (and forward function declarations).
 */

struct discq dqhead;
struct v6router defrtr,nondefrtr;

static struct sockaddr_in6 allones;

void ipv6_nsolicit __P((struct ifnet *, struct mbuf *, struct rtentry *));
void ipv6_uni_nsolicit __P((struct rtentry *));

/*
 * These should be sysctl-tweakable.  See ipv6_var.h for units. 
 * They share the common prefix v6d_.
 */

u_long v6d_maxinitrai	= MAX_INITIAL_RTR_ADVERT_INTERVAL;
u_long v6d_maxinitra	= MAX_INITIAL_RTR_ADVERTISEMENTS;
u_long v6d_maxrtrdel	= MAX_RTR_RESPONSE_DELAY;

u_long v6d_maxrtsoldel	= MAX_RTR_SOLICITATION_DELAY;
u_long v6d_rtsolint	= RTR_SOLICITATION_INTERVAL;    
u_long v6d_maxrtsol	= MAX_RTR_SOLICITATIONS;     

u_long v6d_resretrans	= RESOLVE_RETRANS_TIMER;       
u_long v6d_maxmcastsol	= MAX_MULTICAST_SOLICIT;
u_long v6d_maxucastsol	= MAX_UNICAST_SOLICIT;
u_long v6d_maxnadv	= MAX_NEIGHBOR_ADVERTISEMENTS;
u_long v6d_minnadvint	= MIN_NEIGHBOR_ADVERT_INTERVAL;
u_long v6d_reachtime	= REACHABLE_TIME;
u_long v6d_reachretrans	= REACHABLE_RETRANS_TIME;
u_long v6d_delfirstprobe= DELAY_FIRST_PROBE_TIME;  

u_long v6d_nexthopclean	= NEXTHOP_CLEAN_INTERVAL;

u_long v6d_down = 20;  /* Dead node routes are marked REJECT for this amt. */

/*
 * External globals.
 */

extern struct ifnet *ifnet;
extern struct in6_ifaddr *in6_ifaddr;

extern int ipv6forwarding;

/*
 * General notes:
 *
 * Currently this module does not support encryption/authentication 
 * of ND messages.  That support is probably needed in some environments.
 * NRL intends to add it in a later release.
 *
 * Please keep this in mind.
 *    danmcd rja
 */


/*
 * Functions and macros.
 */

/*----------------------------------------------------------------------
  void padto(u_char *ptr, u_long mask, int alignto)

  Examples: padto(foo, 0x3, 0) will get 4-byte alignment
             padto(foo, 0x7, 6) will get to 2 bytes before an 8-byte
                                boundary.

  Padding generated will be most efficient.

  This is now deprecated for discovery, but it still has
  uses in generating IPv6 options (the option bags).

----------------------------------------------------------------------*/
#define padto(ptr,mask,alignto)\
{\
  int difference = 0;\
  \
  if (((u_long)(ptr) & (mask)) != (alignto))\
    {\
      difference = (((u_long)(ptr) & ~(mask)) + (alignto)) - (long)(ptr);\
      if (difference < 0)\
	difference += ((mask) +1);\
      if (difference == 1)\
        (ptr)[0] = EXT_PAD1;\
      else\
	{\
	  (ptr)[0] = EXT_PADN;\
	  (ptr)[1] = (difference) - 2; /* difference >= 2 always here. */\
	  bzero((ptr)+2,(ptr)[1]);\
	}\
      (ptr) += difference;\
    }\
}


/*---------------------------------------------------------------------- 
 * ipv6_neighbor_timer():
 *	Scan neighbor list and resend (1) mcast ND solicits for all 
 *      neighbors in INCOMPLETE state and (2) unicast ND solicits 
 *      for all neighbors in PROBE state
 ----------------------------------------------------------------------*/
void
ipv6_neighbor_timer(whocares)
     void *whocares;
{
  struct discq *dq;
  int s = splnet();
  struct rtentry *rt;

  /*
   * 1. Scan all neighbors (go down dq list) and retransmit those which
   *    are still in INCOMPLETE.
   * 2. Also, those in PROBE state should have unicast solicits sent.
   * 3. ...
   */

  dq = dqhead.dq_next;
  while (dq != &dqhead)
    {
      struct discq *current = dq;

      dq = dq->dq_next;  /* Before any rtrequest */

      rt = current->dq_rt;
      if (rt->rt_rmx.rmx_expire <= time.tv_sec)
	{
	  struct sockaddr_dl *sdl = (struct sockaddr_dl *)rt->rt_gateway;

	  /*
	   * Gotta do SOMETHING...
	   */
	  if (sdl->sdl_alen == 0)    {
	    /* If RTF_REJECT, then delete... */
	    if (rt->rt_flags & RTF_REJECT)
	      rtrequest(RTM_DELETE,rt_key(rt),NULL,NULL,0,NULL);

	    /* Am in INCOMPLETE mode... */
	    
	    if (current->dq_unanswered >= 0)  /* If not newly created... */
	      if (current->dq_unanswered >= v6d_maxmcastsol)
		{
		  /* Dead node.  For now, just delete.  
		     May want to do an RTF_REJECT entry.  
		     May also want to send ICMP errors on enqueued packet(s). 
		     XXX */
		  DPRINTF(IDL_MAJOR_EVENT,("Deleting dead node... "));
		  rt->rt_flags |= RTF_REJECT;
		  rt->rt_rmx.rmx_expire = time.tv_sec + v6d_down;
		  /*rtrequest(RTM_DELETE,rt_key(rt),NULL,NULL,0,NULL);*/
		}
	      else
		{
		  /* Send another solicit.  The ipv6_nsolicit call will
		     ensure the rate-limitation.  Send the queued packet
		     to ipv6_nsolicit for source address determination. */
		  ipv6_nsolicit(rt->rt_ifp,NULL,rt);
		}
	  }
	  else
	    /* Am in either PROBE mode or REACHABLE mode.  
             *
	     * Either way, that will (in the case of REACHABLE->PROBE)
	     * or might (in the case of PROBE->INCOMPLETE) change.
	     */
	    if (current->dq_unanswered >= 0)  /* PROBE */
	      if (current->dq_unanswered >= v6d_maxucastsol)
		/* PROBE -> INCOMPLETE */
		if (rt->rt_refcnt > 0)
		  {
		    /* The code below is an optimisation for the case
		     * when someone is using this route : 
		     * - defer actual deletion until mcast solicits fail. */
		    sdl->sdl_alen = 0;
		    current->dq_unanswered = 0;
		    rt->rt_rmx.rmx_expire = time.tv_sec + v6d_resretrans;
		  }
		else
		  {
		    /* Unicast probes failed and no one is using this route.  
		       Delete and let address resolution take its course. */
		    rtrequest(RTM_DELETE,rt_key(rt),NULL,NULL,0,NULL);
		  }
	      else
		{
		  /* Retry PROBE */
		  ipv6_uni_nsolicit(rt);
		}
	    else
	      {
		/* Do nothing if REACHABLE expires.  Only on output,
		   BUT... some of these might hang around for too long.
		   Perhaps do some garbage collection... XXX */
		
	      }
	}
    }

  timeout(ipv6_neighbor_timer,NULL,v6d_resretrans * hz);
  splx(s);
}

/*----------------------------------------------------------------------
 * ipv6_clean_nexthop:
 *	Delete requested route, if either the route is both GATEWAY and HOST  
 *      and not held (or the caller REALLY requests it).
 ----------------------------------------------------------------------*/
int
ipv6_clean_nexthop(rn, arg)
     struct radix_node *rn;
     void *arg;
{
  struct rtentry *rt = (struct rtentry *)rn;
  int hardcore = (int)arg; 
  /* If hardcore is not zero, then the caller REALLY insists that we
     delete the route. A drain function would call this with hardcore != 0. */

  DPRINTF(IDL_GROSS_EVENT,("Entering ipv6_clean_nexthop... "));
  if (!(rt->rt_flags & (RTF_GATEWAY|RTF_TUNNEL)) || 
      !(rt->rt_flags & RTF_HOST)    ||
      (!hardcore && rt->rt_refcnt > 0)
      )
    {
      /* Don't delete ANY routes that aren't both RTF_HOST && 
	 (RTF_GATEWAY | RTF_TUNNEL).  For now, TUNNEL implies GATEWAY, but
	 that may change! */
      /* Unless asked (i.e. a hardcore clean :), don't delete held routes. */
      DPRINTF(IDL_GROSS_EVENT,("not deleting.\n"));
      return 0;
    }

  /*
   * At this point, the route is RTF_GATEWAY, RTF_HOST, and is either a
   * force or a legitimate node to be cleaned.  Call RTM_DELETE...
   */
  DPRINTF(IDL_GROSS_EVENT,("deleting.\n"));

  return rtrequest(RTM_DELETE,rt_key(rt),NULL,NULL,0,NULL);
}

/*----------------------------------------------------------------------
 * ipv6_nexthop_timer():
 *      Keeps routing table from getting too filled up with off-net host
 *      routes.
 * 
 * NOTES:
 * 	Later might want to put some intelligence in here like FreeBSD does, 
 * but for now just do an rnh->rnh_walktree() every so often.
 *
 * 	A smarter function might:
 *
 *         - Keep a count of how many off-net host routes we have.
 *         - Maintain upper bounds on said count.
 *         - Avoid deletion if said count is low.
 *         - Etc.
 ----------------------------------------------------------------------*/

void
ipv6_nexthop_timer(whocares)
     void *whocares;
{
  struct radix_node_head *rnh = rt_tables[AF_INET6];
  int s, rc;

  DPRINTF(IDL_MAJOR_EVENT,("Entering ipv6_nexthop_timer().\n"));
  s = splnet();
  rc = rnh->rnh_walktree(rnh, ipv6_clean_nexthop, (void *)0);
  splx(s);

  DDO(IDL_CRITICAL,if (rc != 0)\
                     printf("walktree rc is %d in nexthop_timer.\n"));

  timeout(ipv6_nexthop_timer,NULL,v6d_nexthopclean * hz);
}

/*----------------------------------------------------------------------
 * ipv6_discovery_init():
 *	Initialises ND data structures.
 ----------------------------------------------------------------------*/
void
ipv6_discovery_init()
{
  int i;

  dqhead.dq_next = dqhead.dq_prev = &dqhead;
  defrtr.v6r_next = defrtr.v6r_prev = &defrtr;
  nondefrtr.v6r_next = nondefrtr.v6r_prev = &nondefrtr;

  allones.sin6_family = AF_INET6;
  allones.sin6_len = sizeof(allones);
  /* Other fields are bzeroed. */
  IN6_ADDR_ASSIGN(allones.sin6_addr, \
		  0xffffffff,0xffffffff,0xffffffff,0xffffffff);

  timeout(ipv6_nexthop_timer,NULL,hz);
  timeout(ipv6_neighbor_timer,NULL,hz);
}

/*----------------------------------------------------------------------
 * get_discov_cluster():
 * 	Allocates a single-cluster mbuf and sets it up for use by ND.
 *
 ----------------------------------------------------------------------*/

struct mbuf *
get_discov_cluster()
{
  struct mbuf *rc;

  MGET(rc,MT_HEADER,M_DONTWAIT);
  if (rc != NULL)
    {
      MCLGET(rc,M_DONTWAIT);
      if (rc->m_flags & M_EXT == 0)
	{
	  m_free(rc);
	  rc = NULL;
	}
    }

  /* Make it a pkthdr appropriately. */
  rc->m_flags |= M_PKTHDR;
  rc->m_pkthdr.rcvif = NULL;
  rc->m_pkthdr.len = 0;
  rc->m_len = 0;
  return rc;
}

/*----------------------------------------------------------------------
 * send_nsolicit():
 * 	Send a neighbor solicit for destination in route entry rt,
 * across interface pointed by ifp.  Use source address in src (see below),
 * and either unicast or multicast depending on the mcast flag.
 *
 * NOTES:	The entry pointed to by rt MUST exist.
 *   If the caller has to, it may cruft up a dummy rtentry with at least
 *   a valid rt_key() for the neighbor's IPv6 address, and maybe
 *   a dummy rt_gateway.
 ----------------------------------------------------------------------*/
void
send_nsolicit(rt,ifp,src,mcast)
     struct rtentry *rt;
     struct ifnet *ifp;
     struct in_addr6 *src;    /* Source address of invoking packet... */
     int mcast;
{
  struct mbuf *solicit = NULL;
  struct ipv6 *header;
  struct ipv6_icmp *icmp;
  struct ipv6_moptions i6mo,*i6mop = NULL;
  struct in6_ifaddr *i6a;
  struct sockaddr_in6 *neighbor  = (struct sockaddr_in6 *)rt_key(rt);
  struct sockaddr_dl *sdl = (struct sockaddr_dl *)rt->rt_gateway;

  if ((solicit = get_discov_cluster()) == NULL)
    {
      DPRINTF(IDL_CRITICAL, ("Can't allocate mbuf in send_gsolicit().\n"));
      return;
    }

  header = mtod(solicit,struct ipv6 *);
  icmp = (struct ipv6_icmp *)(header + 1);/* I want the bytes after the hdr. */

  if (mcast)
    {
      bzero(&i6mo,sizeof(struct ipv6_moptions));
      i6mo.i6mo_multicast_ifp = ifp;
      i6mop = &i6mo;
      IN6_ADDR_ASSIGN(header->ipv6_dst,htonl(0xff020000),0,htonl(1),\
		      neighbor->sin6_addr.in6a_words[3]);
    }
  else
    {
      if (sdl == NULL || sdl->sdl_alen == 0)
	{
	  DPRINTF(IDL_CRITICAL, ("Can't unicast if I don't know address.\n"));
	  m_freem(solicit);
	  return;
	}
      header->ipv6_dst = neighbor->sin6_addr;
      /* If rmx_expire is not already set, set it to avoid chicken-egg loop. */
    }

  header->ipv6_versfl = htonl(0x60000000);
  header->ipv6_hoplimit = 1;  /* Intra-link, so why not? */
  header->ipv6_nexthdr = NEXTHDR_ICMP;
  header->ipv6_length = ICMPV6_NSOLMINLEN;  /* For now. */

  /* Find source link-local or use unspec. */
  for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
    if (i6a->i6a_ifp == ifp && (i6a->i6a_addrflags & I6AF_LINKLOC))
      break;
  if (i6a == NULL)
    if (mcast)
      {
	IN6_ADDR_ASSIGN(header->ipv6_src,0,0,0,0);
      }
    else panic("Unicast solicit w/o any known source address.");
  else header->ipv6_src = i6a->i6a_addr.sin6_addr;

  /*
   * Use link-local if no explicit src passed in, otherwise, do sanity
   * check of src.
   */

  if (src != NULL)
    {
      for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
	if (IN6_ADDR_EQUAL(i6a->i6a_addr.sin6_addr,*src))
	  break;
      if (i6a == NULL)
	panic("Requested source address failed sanity check.");
      else if (i6a->i6a_ifp != ifp)
	panic("Requested source address failed ifp check.");
      else icmp->icmp_nsolsrc = *src;
    }
  else icmp->icmp_nsolsrc = header->ipv6_src;

  /* Have source address, now create outbound packet */
  icmp->icmp_type = ICMPV6_NEIGHBORSOL;
  icmp->icmp_code = ipv6forwarding;     /* Or however else I determine
					   a host/router distinction. */
  icmp->icmp_unused = 0;
  icmp->icmp_cksum = 0;
  icmp->icmp_nsoltarg = neighbor->sin6_addr;

  /*
   * Put ND extensions here, if any.
   * This next code fragment could be its own function if there
   * were enough callers of the fragment to make that sensible.
   */
  if (ifp->if_addrlen != 0)    
    {
      struct icmp_exthdr *ext = (struct icmp_exthdr *)&(icmp->icmp_nsolext);
      struct ifaddr *ifa;
      struct sockaddr_dl *lsdl;

      ext->ext_id = EXT_SOURCELINK;
      ext->ext_length = 1 + ((ifp->if_addrlen + 1) >> 3);
      for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
	if (ifa->ifa_addr->sa_family == AF_LINK)
	  break;
      if (ifa == NULL)
	{
	  DPRINTF(IDL_CRITICAL, ("Can't find link addr. in nsolicit().\n"));
	  m_freem(solicit);
	  return;
	}
      lsdl = (struct sockaddr_dl *)ifa->ifa_addr;
      bzero(ext->ext_data,ext->ext_length*8 - 2);
      bcopy(LLADDR(lsdl),ext->ext_data,ifp->if_addrlen);
      header->ipv6_length += ext->ext_length <<3;
    }

  solicit->m_len = solicit->m_pkthdr.len = 
                       header->ipv6_length + sizeof(*header);
  icmp->icmp_cksum = in6_cksum(solicit,NEXTHDR_ICMP,header->ipv6_length,
			       sizeof(struct ipv6));

  /* NOTE: The solicit mbuf chain will have a NULL instead of a valid
     socket ptr.  When ipv6_output() calls ipsec_output_policy(),
     this socket ptr will STILL be NULL.  Sooo, the security
     policy on outbound packets from here will == system security
     level (set in ipsec_init() of netinet6/ipsec.c).  If your
     system security level is paranoid, then you won't move packets
     unless you have _preloaded_ keys for at least the ND addresses. 
     danmcd rja */
  ipv6_output(solicit, NULL, IPV6_RAWOUTPUT, i6mop);
}

/*----------------------------------------------------------------------
 * ipv6_nsolicit:   
 *	Send an IPv6 Neighbor Solicit message
 *
 * NOTES:
 * State checking is needed so that a neighbor can be declared unreachable.
 *
 * newrt == NULL iff this is a virgin packet with no known i/f,
 * otherwise valid newrt MUST be passed in.
 *
 * If ifp == NULL, ipv6_nsolicit() executes potentially multihomed code.
 * (For now, I guess I should pick an arbitrary interface.  For single-homed
 * nodes, this is the optimal behavior.)
 * This is called when a neighbor is in INCOMPLETE state.
 *
 * The NUD INCOMPLETE and new-neighbor detection state belongs here.
 *
 * Not yet clear how will implement duplicate address detection.
 *                       (See send_nsolicit.)
 *                  Should I splnet() inside here?
 ----------------------------------------------------------------------*/
void
ipv6_nsolicit(outifp,outgoing,newrt)
     struct ifnet *outifp;
     struct mbuf *outgoing;
     struct rtentry *newrt;
{
  int s,enqueued = (outgoing == NULL);
  struct discq *dq;
  struct ipv6 *ipv6 = (outgoing == NULL) ? NULL : 
                     mtod(outgoing,struct ipv6 *);  
  /* ASSERT: The header is pulled up, and has either the
     unspecified address or one of MY valid ipv6 addresses. */
  struct ifnet *ifp = (outifp == NULL)?ifnet:outifp;

  if (newrt == NULL)
    {
      DPRINTF(IDL_CRITICAL, 
	      ("ipv6_nsolicit() called with newrt == NULL.\n"));
      m_freem(outgoing);
      return;
    }

  dq = (struct discq *)newrt->rt_llinfo;
  DDO(IDL_CRITICAL,if (dq == NULL) \
                        panic("dq == NULL in nsolicit().\n"); \
                      if (dq->dq_rt != newrt)\
                        panic("dq <-> rt mismatch in nsolicit.\n");\
                      {\
		       struct sockaddr_dl *sdl = (struct sockaddr_dl *)\
			                          newrt->rt_gateway;\
			 \
		       if (sdl->sdl_alen != 0)\
			 panic("sdl_alen not 0 in nsolicit!");
                      });
  DDO(IDL_MAJOR_EVENT,if (outifp == NULL)
                        printf("nsolicit passed in outifp of NULL.\n"));

  do
    {
      if (ifp->if_type != IFT_LOOP && ipv6_enabled(ifp))
	{
	  if (!enqueued)
	    {
	      enqueued = 1;
	      if (dq->dq_queue != NULL)
		m_freem(dq->dq_queue);
	      dq->dq_queue = outgoing;
	    }

	  if (newrt->rt_rmx.rmx_expire == 0)
	    {
	      /* New entry which is about to be used. */
	      dq->dq_unanswered = 0;
	      newrt->rt_rmx.rmx_expire = time.tv_sec - 1;
	    }

	  /* We want to rate-limit these, 
	     so only send out if the time has expired. */
	  if (newrt->rt_rmx.rmx_expire < time.tv_sec)
	    {
	      /* Set the rmx expiration. */
	      newrt->rt_rmx.rmx_expire = time.tv_sec + v6d_resretrans;

	      if (ipv6 == NULL && dq->dq_queue != NULL)
		ipv6 = mtod(dq->dq_queue,struct ipv6 *);
	      send_nsolicit(newrt,ifp,(ipv6 == NULL) ? NULL : &ipv6->ipv6_src,
			    1);
	      dq->dq_unanswered++;   /* Overload for "number of m-cast
					probes sent". */
	    }

	}

      if (outifp == NULL)
	ifp = ifp->if_next;
      else
	ifp = NULL;

    }
  while (ifp != NULL);
}


/*----------------------------------------------------------------------
 * ipv6_uni_nsolicit():
 * 	Send a unicast neighbor solicit to neighbor specified in rt.
 *      Also update the next-probe time.
 *
 * NOTES:
 *    This is so small that it might be better inlined.
 ----------------------------------------------------------------------*/

void
ipv6_uni_nsolicit(rt)
     struct rtentry *rt;
{
  struct discq *dq = (struct discq *)rt->rt_llinfo;

  rt->rt_rmx.rmx_expire = time.tv_sec + v6d_reachretrans;
  /* If ifa->ifa_addr doesn't work, revert to NULL. */
  send_nsolicit(rt,rt->rt_ifp,&(I6A_SIN(rt->rt_ifa)->sin6_addr),0);
  dq->dq_unanswered ++;
}


/*----------------------------------------------------------------------
 *ipv6_nadvert():
 *   Construct an IPv6 neighbor advertisement, 
 *   and send it out via either unicast or multicast.
 * 
 * NOTES:
 *   Might later add a proxy advertisement bit or anycast bit to the flags
 *   parameter.
 ----------------------------------------------------------------------*/
void
ipv6_nadvert(i6a,ifp,dstrt,flags)
     struct in6_ifaddr *i6a;
     struct ifnet *ifp;
     struct rtentry *dstrt;     /* If null, send m-cast neighbor advert. */
     u_int32 flags;
{
  struct ipv6_moptions i6mo,*i6mop = NULL;
  struct mbuf *advert;
  struct ipv6 *ipv6;
  struct ipv6_icmp *icmp;
  struct ifaddr *ifa;
  struct in6_ifaddr *linkloc;

  advert = get_discov_cluster();
  if (advert == NULL)
    return;

  ipv6 = mtod(advert,struct ipv6 *);
  icmp = (struct ipv6_icmp *)&(advert->m_data[sizeof(struct ipv6)]);

  for (linkloc = in6_ifaddr;linkloc;linkloc = linkloc->i6a_next)
    if (linkloc->i6a_ifp == ifp && (linkloc->i6a_addrflags & I6AF_LINKLOC))
      break;
  if (linkloc == NULL)
    panic("No link-local found in ipv6_nadvert().");
  ipv6->ipv6_src        = linkloc->i6a_addr.sin6_addr;
  ipv6->ipv6_versfl	= htonl(0x60000000);
  ipv6->ipv6_hoplimit	= 1;  		/* Intra-link, so set to 1 hop. */
  ipv6->ipv6_nexthdr	= NEXTHDR_ICMP;
  ipv6->ipv6_length	= ICMPV6_NADVMINLEN; /* For now */
  icmp->icmp_type	= ICMPV6_NEIGHBORADV;
  icmp->icmp_code	= 0;
  icmp->icmp_cksum	= 0;
  icmp->icmp_nadvbits	= flags;

  /* Flags in ipv6_icmp.h are in endian-specific #ifdefs, so
     there is no need to HTONL(icmp->icmp_nadvbits);   */ 

  /* If proxy advert, set proxy bits. */
  icmp->icmp_nadvaddr = i6a->i6a_addr.sin6_addr;


  if (dstrt == NULL)
    {
      i6mo.i6mo_multicast_ifp = ifp;
      i6mop = &i6mo;
      SET_IN6_ALLNODES(ipv6->ipv6_dst);
      SET_IN6_MCASTSCOPE(ipv6->ipv6_dst, IN6_INTRA_LINK);
    }
  else ipv6->ipv6_dst = ((struct sockaddr_in6 *)rt_key(dstrt))->sin6_addr;

  /*
   * Set up extensions (if any)
   */

  /* Perhaps create a separate function to look through interface's address
     list to find my data link address, but if this would really be called
     enough other places... */
  if (i6a->i6a_ifp->if_addrlen != 0)    
    {
      struct icmp_exthdr *ext = (struct icmp_exthdr *)&(icmp->icmp_nadvext);
      struct ifaddr *ifa;
      struct sockaddr_dl *lsdl;   /* Target's Local Link-layer Sockaddr */

      ext->ext_id = EXT_TARGETLINK;
      ext->ext_length = 1 + ((ifp->if_addrlen +1) >> 3);
      for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
	if (ifa->ifa_addr->sa_family == AF_LINK)
	  break;
      if (ifa == NULL)
	{
	  DPRINTF(IDL_CRITICAL, ("Can't find link addr. in nadvert().\n"));
	  m_freem(advert);
	  return;
	}
      lsdl = (struct sockaddr_dl *)ifa->ifa_addr;
      bzero(ext->ext_data,ext->ext_length*8 - 2);
      bcopy(LLADDR(lsdl),ext->ext_data,ifp->if_addrlen);
      ipv6->ipv6_length += ext->ext_length <<3;
    }

  advert->m_len = advert->m_pkthdr.len = ipv6->ipv6_length + sizeof(*ipv6);
  icmp->icmp_cksum = in6_cksum(advert,NEXTHDR_ICMP,ipv6->ipv6_length,
			       sizeof(struct ipv6));
  ipv6_output(advert,NULL,IPV6_RAWOUTPUT,i6mop);
}

/*----------------------------------------------------------------------
 * ipv6_discov_pullup():
 *	Front-end to m_pullup2() that ensures the whole ND header
 *      is contiguous so no mbuf chain traversal will be needed.
 *      Used by many of the following input functions.
 *
 * NOTES:
 *   This function is a good candidate for inlining.
 * 
 *   Like other ICMP packets, these will be not freed locally,
 *   since ripv6_input() will take care of freeing them.
 ----------------------------------------------------------------------*/

struct mbuf *                 
ipv6_discov_pullup(m,extra,ihi,ihioff)
     struct mbuf *m;
     struct in6_hdrindex *ihi;
     int ihioff,extra;
{
  struct mbuf *mine;

  mine = ihi[ihioff - 1].ihi_mbuf;
  while (mine->m_next != ihi[ihioff].ihi_mbuf)
    mine = mine->m_next;

  mine->m_next = m_pullup2(mine->m_next, m->m_pkthdr.len - extra);
  if (mine->m_next == NULL)
    {
      DPRINTF(IDL_CRITICAL, ("m_pullup2() failed in ipv6_discov_pullup().\n"));
      return NULL;
    }

  ihi[ihioff].ihi_mbuf = mine->m_next;

  return mine->m_next;
}

/*----------------------------------------------------------------------
 * ipv6_routersol_input():
 *	Handle reception of Router Solicit messages.
 *
 * NOTES:
 *   This is not implemented yet.
 ----------------------------------------------------------------------*/
void
ipv6_routersol_input(incoming,extra,ihi,ihioff)
     struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *ihi;
     int ihioff;
{
}

/*----------------------------------------------------------------------
 * ipv6_routeradv_input():
 *	Handle reception of Router Advertisement messages.
 *
 * NOTES:
 *   This is not implemented yet.
 ----------------------------------------------------------------------*/
void
ipv6_routeradv_input(incoming,extra,ihi,ihioff)
     struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *ihi;
     int ihioff;
{
}

/*----------------------------------------------------------------------
 * ipv6_new_neighbor():
 * 	Return a new neighbor-cache entry for address sin6 on interface ifp.
 *      A front-end for rtrequest(RTM_ADD, ...).  This returns NULL
 *      if there is a problem.
 * 
 * NOTES:  
 *	May want to handle case of ifp == NULL.  
 *	ipv6_discov_rtrequest() will handle ancillary structure setup.  
 ----------------------------------------------------------------------*/

struct rtentry *
ipv6_new_neighbor(sin6, ifp)
     struct sockaddr_in6 *sin6;   /* Neighbor's IPv6 address. */
     struct ifnet *ifp;           /* Interface neighbor lies off this. */
{
  struct sockaddr_dl lsdl;	/* Target's Link-local Address sockaddr */
  struct sockaddr *dst = (struct sockaddr *)sin6;
  struct sockaddr *gateway = (struct sockaddr *)&lsdl;
  struct sockaddr *netmask = (struct sockaddr *)&allones;
  struct rtentry *newrt = NULL;
  struct ifaddr *ifa;
  int flags = RTF_HOST;

  /* Set up Target (my) Link-layer Sockaddr (lsdl) */
  if (ifp != NULL)
    {
      for (ifa = ifp->if_addrlist ; ifa != NULL; ifa = ifa->ifa_next)
	if (ifa->ifa_addr->sa_family == AF_LINK)
	  break;
      if (ifa == NULL)
	{
	  DPRINTF(IDL_CRITICAL, 
		  ("ipv6_new_neighbor() can't find AF_LINK for ifp.\n"));
	  return NULL;
	}

      bcopy(ifa->ifa_addr,&lsdl,min(sizeof(lsdl),ifa->ifa_addr->sa_len));
      lsdl.sdl_alen = 0;
      lsdl.sdl_nlen = 0;
      lsdl.sdl_slen = 0;
    }
  else
    {
      /* ifp == NULL handling. */
      return NULL;
    }

#ifdef DIAGNOSTIC
  if (lsdl.sdl_index == 0)
    panic("sdl_index is 0 in ipv6_new_neighbor().");
#endif

  /* ASSUMES:  there is enough room for the link address shoved in here */
  if (rtrequest(RTM_ADD,dst,gateway,netmask,flags,&newrt) == EEXIST)
    {
      DPRINTF(IDL_MAJOR_EVENT,("Can't add neighbor that already exists?\n"));
    }

  if (newrt != NULL)
    {
      /* Fill in any other goodies, especially MTU. */
      DDO(IDL_EVENT, printf("New route okay, before MTU setup...\n");\
	  dump_rtentry(newrt));
      newrt->rt_rmx.rmx_mtu = ifp->if_mtu;
      newrt->rt_refcnt = 0;
      DDO(IDL_EVENT,printf("New route okay, after MTU setup...\n");\
	  dump_rtentry(newrt));
    }
  return newrt;
}

/*----------------------------------------------------------------------
 * ipv6_neighborsol_input():
 *	Handle input processing for Neighbor Solicit messages.
 *
 ----------------------------------------------------------------------*/
void
ipv6_neighborsol_input(incoming,extra,ihi,ihioff)
     struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *ihi;
     int ihioff;
{
  struct ipv6 *ipv6;
  struct ipv6_icmp *icmp;
  struct rtentry *rt,*rt2;
  struct discq *dq;
  struct sockaddr_in6 sin6;
  struct sockaddr_dl *sdl;
  struct in6_ifaddr *i6a;
  /*struct ext_linkinfo *li = NULL;*/
  struct icmp_exthdr *ext;
  struct mbuf *mine = ipv6_discov_pullup(incoming,extra,ihi,ihioff);

  /* Thanks to ipv6_icmp.h, ICMP_NEIGHBORADV_* are already in network order */
  u_int32 flags = (ipv6forwarding) ? ICMPV6_NEIGHBORADV_RTR : 0;

  if (mine == NULL)
    return;

  /* ASSERT: ipv6 header is first in chain, and is already pulled up. */
  ipv6 = mtod(incoming,struct ipv6 *);
  icmp = mtod(mine,struct ipv6_icmp *);

  /* Verify that length looks OK */
  if (incoming->m_pkthdr.len - extra < ICMPV6_NSOLMINLEN)
    return;

  /* Check to see if source is link-local. */
  if (!IS_IN6_LINKLOC(ipv6->ipv6_src))
    {
      DPRINTF(IDL_MAJOR_EVENT,
	      ("Received Neighbor Solicit from non-link-local address.\n"));
      return;
    }

  if (IS_IN6_MULTICAST(icmp->icmp_nsoltarg))
    {
      DPRINTF(IDL_MAJOR_EVENT, ("Received multicast address in solicit!\n"));
      return;
    }
  /*
   * Have a Neighbor Solicit message. 
   */

  /* Verify this is for me.   */
  /* Eventually proxy & anycast checking will go here. */
  for (i6a = in6_ifaddr ; i6a ; i6a = i6a->i6a_next)
    if (IN6_ADDR_EQUAL(i6a->i6a_addr.sin6_addr,icmp->icmp_nsoltarg))
      break;
  if (i6a == NULL)
    {
      /* Not for me. */
      return;
    }

  if (i6a->i6a_ifp != incoming->m_pkthdr.rcvif)
    {
      DPRINTF(IDL_MAJOR_EVENT, 
	      ("Received off-link Neighbor Solicit for self\n"));
      return;
    }

  /*
   * Create neighbor cache entry for neighbor to send back advertisement.
   * Also create neighbor cache entry for source in solicit packet.
   */

  if (!IS_IN6_UNSPEC(ipv6->ipv6_src))
    {
      sin6.sin6_family = AF_INET6;
      sin6.sin6_len = sizeof(sin6);
      sin6.sin6_port = 0;
      sin6.sin6_flowinfo = 0;
      sin6.sin6_addr = ipv6->ipv6_src;

      rt = rtalloc1((struct sockaddr *)&sin6, 0);

      /*
       * I just did a passive route lookup.  I'll either get:
       *
       * 1. No route, meaning I have to create one.
       * 2. A host neighbor (RTF_LLINFO) route, meaning I might update it.
       * 3. A host off-link (RTF_GATEWAY) route, meaning I possibly missed a
       *    redirect.
       * 4. An on-link prefix (RTF_CLONING, no RTF_GATEWAY) route, meaning
       *    I have to create one, like no route.
       * 5. The default route (RTF_DEFAULT), meaning the same as no route.
       * 6. A network route, meaning either a subset of that prefix is on-link,
       *    or my routing table is bogus.  I'll create one.
       *
       * In any case I actually get one, I should decrement the rt_refcnt.
       *
       * Future support for RTF_TUNNEL needed here.
       */
      
      if (rt == NULL  ||  !(rt->rt_flags & RTF_HOST)  )
	{
	  /*
	   * No available host route, create a new entry.
	   */
	  if (rt != NULL)
	    rt->rt_refcnt--;

	  rt = ipv6_new_neighbor(&sin6,incoming->m_pkthdr.rcvif);
	  if (rt == NULL)
	    {
	      DPRINTF(IDL_CRITICAL, 
		      ("Can't allocate soliciting neighbor route.\n"));
	      return;
	    }
	}
      else if (rt->rt_flags & RTF_LLINFO)
	{
	  rt->rt_refcnt--;
	  if (rt->rt_gateway->sa_family != AF_LINK)
	    panic("LLINFO but gateway != AF_LINK.");
	}
      else
	{
	  /* 
	   * Found off-net route for link-local address.  Really strange.
	   */
	  panic("Found off-net route for supposed link-local address.");
	}
      
      sin6.sin6_addr = icmp->icmp_nsolsrc;

      rt2 = rtalloc1((struct sockaddr *)&sin6, 0);

      /*
       * I just did a passive route lookup.  I'll either get:
       *
       * 1. No route, meaning I have to create one.
       * 2. A host neighbor (RTF_LLINFO) route, meaning I might update it.
       * 3. A host off-link (RTF_GATEWAY) route, meaning I possibly missed a
       *    redirect.
       * 4. An on-link prefix (RTF_CLONING, no RTF_GATEWAY) route, meaning
       *    I have to create one, like no route.
       * 5. The default route (RTF_DEFAULT), meaning the same as no route.
       * 6. A network route, meaning either a subset of that prefix is on-link,
       *    or my routing table is bogus.  I'll create one.
       *
       * In any case I actually get one, I should decrement the rt_refcnt.
       *
       * Future RTF_TUNNEL support needed here.
       */
      
      if (rt2 == NULL  ||  !(rt2->rt_flags & RTF_HOST)  )
	{
	  /*
	   * No available host route, create a new entry.
	   */
	  if (rt2 != NULL)
	    rt2->rt_refcnt--;

	  rt2 = ipv6_new_neighbor(&sin6,incoming->m_pkthdr.rcvif);
	  if (rt2 == NULL)
	    {
	      DPRINTF(IDL_CRITICAL, 
		      ("Can't allocate soliciting neighbor route.\n"));
	      return;
	    }
	}
      else if (rt2->rt_flags & RTF_LLINFO)
	{
	  rt2->rt_refcnt--;
	  if (rt2->rt_gateway->sa_family != AF_LINK)
	    panic("LLINFO but gateway != AF_LINK.");
	}
      else
	{
	  /* 
	   * Received Neighbor Solicit from an address which I have
	   * an off-net host route for.  For now, bail.
	   */
	  DPRINTF(IDL_MAJOR_EVENT, 
		  ("Received Neighbor Solicit from unknown target.\n"));
	  return;
	}
      
      /*
       * If new, or inactive, set to probe.
       */
      
      /* All of this data will fit in one mbuf as long as the upper limit
	 for ICMP message size <= 576 <= MCLBYTES. */
      if (ihi[ihioff].ihi_mbuf->m_len > ICMPV6_NSOLMINLEN )
	{
	  u_char *data = (u_char *)&icmp->icmp_nsolext, 
	  *bounds = (data + ihi[ihioff].ihi_mbuf->m_len);
	  
	  /* Only possible extension (so far) in a neighbor advert 
	     is a source link-layer address, but be careful anyway. */
	  
	  ext = (struct icmp_exthdr *)data;
	  while (ext->ext_id != EXT_SOURCELINK && (data<bounds))
	    {
	      DPRINTF(IDL_MAJOR_EVENT,("Got extension other than source link.\n"));
	      data += ext->ext_length<<3;
	      ext = (struct icmp_exthdr *)data;
	    }
	  if (data >= bounds)
	    panic("couldn't find SOURCELINK");
	}
      
      sdl = (struct sockaddr_dl *)rt->rt_gateway;
      rt->rt_flags &= ~RTF_REJECT;
      if (sdl->sdl_alen == 0)  /* New or inactive */
	{
	  rt->rt_rmx.rmx_expire = time.tv_sec + v6d_delfirstprobe;
	  dq = (struct discq *)rt->rt_llinfo;
	  dq->dq_unanswered = 0;  /* In PROBE state */
	  /*
	   * Checks for non-broadcast multiple-access (NBMA) links
	   * such as PPP, Frame Relay, and ATM are probably needed here.
	   */
	  if (ext != NULL)
	    {
	      sdl->sdl_alen = 6;  
	      bcopy(ext->ext_data,LLADDR(sdl),sdl->sdl_alen);
	    }
	}
      else
	{
	  /*
	   * Verify link address equality.
	   */
	}

      sdl = (struct sockaddr_dl *)rt2->rt_gateway;
      rt2->rt_flags &= ~RTF_REJECT;
      if (sdl->sdl_alen == 0)  /* New or inactive */
	{
	  rt2->rt_rmx.rmx_expire = time.tv_sec + v6d_delfirstprobe;
	  dq = (struct discq *)rt2->rt_llinfo;
	  dq->dq_unanswered = 0;  /* In PROBE state */
	  /*
	   * Checks for non-broadcast multiple-access (NBMA) links
	   * such as PPP, Frame Relay, and ATM are probably needed here.
	   */
	  if (ext != NULL)
	    {
	      sdl->sdl_alen = 6;  
	      bcopy(ext->ext_data,LLADDR(sdl),sdl->sdl_alen);
	    }
	}
      else
	{
	  /*
	   * Should check link address equality.
	   */
	}

      /*
       * Neighbor Cache is now updated.  
       * Now, send out my unicast advertisement.
       *
       * NB: ICMPV6_* symbol was already htonl()'d in the header file.
       */
      flags |= ICMPV6_NEIGHBORADV_SOL;   
      ipv6_nadvert(i6a,i6a->i6a_ifp,rt,flags);
    }
  else
    {
      /*
       * Send multicast advertisement rather than unicast advertisement
       * because the solicit contained the unspecified address.
       */
      flags |= ICMPV6_NEIGHBORADV_SOL;   
      ipv6_nadvert(i6a,incoming->m_pkthdr.rcvif,NULL,flags);
    }
}


/*----------------------------------------------------------------------
 * ipv6_neighboradv_input():
 *	Handle reception of a Neighbor Advertisement message.
 *
 ----------------------------------------------------------------------*/
void
ipv6_neighboradv_input(incoming,extra,ihi,ihioff)
     struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *ihi;
     int ihioff;
{
  struct ipv6 *ipv6;
  struct ipv6_icmp *icmp;
  struct mbuf *mine = ipv6_discov_pullup(incoming,extra,ihi,ihioff);
  struct rtentry *rt;
  struct sockaddr_in6 sin6;
  int s;

  if (mine == NULL)
    return;

  ipv6 = mtod(incoming,struct ipv6 *);
  icmp = mtod(mine,struct ipv6_icmp *);

  /* Verify that incoming length looks plausible */
  if (incoming->m_pkthdr.len - extra < ICMPV6_NADVMINLEN)
    return;

  if (IS_IN6_MULTICAST(icmp->icmp_nadvaddr))
    {
      DPRINTF(IDL_MAJOR_EVENT, 
	      ("Received Neighbor Solicit with multicast address.\n"));
      return;
    }

  /*
   * Have a Neighbor Advertisement.
   */

  s = splnet();

  /* Lookup and see if I have something waiting for it... */
  sin6.sin6_len = sizeof(sin6);
  sin6.sin6_family = AF_INET6;
  sin6.sin6_addr = icmp->icmp_nadvaddr;

  /* Next 2 lines might not be strictly needed since this is an rtalloc, 
     but they're included to be safe.  */
  sin6.sin6_port = 0;
  sin6.sin6_flowinfo = 0;

  rt = rtalloc1((struct sockaddr *)&sin6,0);

  if (rt != NULL)
    rt->rt_refcnt--;

  /*
   * After passive route lookup, I have either:
   *
   * 1. No route, drop the advert.
   * 2. Route with no RTF_HOST, drop the advert.
   * 3. Route with no RTF_LLINFO, for now, drop the advert, this could be
   *    a redirect weirdness?
   * 4. Route with RTF_LLINFO, try and update.
   */

  if (rt == NULL || !(rt->rt_flags & RTF_HOST))   
    {
      /* Cases 1 and 2. */
      splx(s);
      return;
    }

  if (rt->rt_flags & RTF_LLINFO)  
    {
      /* Case 4. */
      struct sockaddr_dl *sdl;
      /*struct ext_linkinfo *li = NULL;*/ 
      struct icmp_exthdr *liext = NULL;

      /*
       * Update at least the link-layer address, 
       * and maybe change to REACHABLE state.
       */
      rt->rt_flags &= ~RTF_REJECT;  /* That neighbor talked to me! */
      if (icmp->icmp_nadvbits & ICMPV6_NEIGHBORADV_SECOND)
	{
	  /* Handle proxy/anycast advert here.  For now, drop. */
	  DPRINTF(IDL_MAJOR_EVENT,("Proxy or anycast case, for now, dropping.\n"));
	  DDO(IDL_MAJOR_EVENT,dump_in_addr6(&ipv6->ipv6_src);\
	                      dump_in_addr6(&icmp->icmp_nadvaddr));
	  splx(s);
	  return;
	}

      if (ihi[ihioff].ihi_mbuf->m_len > ICMPV6_NADVMINLEN)
	{
	  u_char *data = (u_char *)&icmp->icmp_nadvext, 
	  *bounds = (data + ihi[ihioff].ihi_mbuf->m_len);
	  struct icmp_exthdr *ext = (struct icmp_exthdr *)data;
	  
	  /* Only possible extension (so far) in a neighbor advert is a 
	     source link-layer address, but be careful anyway. */

	  while (ext->ext_id != EXT_TARGETLINK && (data < bounds))
	    {
	      DPRINTF(IDL_MAJOR_EVENT,("Got extension other than source link.\n"));
	      data += ext->ext_length<<3;
	      ext = (struct icmp_exthdr *)data;
	    }
	  if (data >= bounds)
	    panic("couldn't find TARGETLINK");
	  liext = ext;
	}

      sdl = (struct sockaddr_dl *)rt->rt_gateway;
      if (liext != NULL)
	{
	  sdl->sdl_alen = 6;  
	  bcopy(&liext->ext_data,LLADDR(sdl),sdl->sdl_alen);
	}

      /* The ICMP_NEIGHBORADV_* bits are already machine-specific.  
         So no HTONLs/NTOHLs need to be done here.  */

      if (icmp->icmp_nadvbits & ICMPV6_NEIGHBORADV_SOL)
	{
	  /* Now in REACHABLE state. */
	  struct discq *dq = (struct discq *)rt->rt_llinfo;

	  rt->rt_rmx.rmx_expire = time.tv_sec + v6d_reachtime;
	  dq->dq_unanswered = -1;
	  if (dq->dq_queue != NULL)
	    {
	      /*ipv6_output(dq->dq_queue,NULL,IPV6_RAWOUTPUT,NULL);*/
	      rt->rt_ifp->if_output(rt->rt_ifp,dq->dq_queue, rt_key(rt),
				    rt);
	      dq->dq_queue = NULL;
	    }
	}

      /* Put in checking here for routers becoming hosts. */
      if (icmp->icmp_nadvbits & ICMPV6_NEIGHBORADV_RTR)
	rt->rt_flags |= RTF_ISAROUTER;
    }
  else
    {
      /* Case 3. */
      /* Should consider adding a counter for these */
      DPRINTF(IDL_MAJOR_EVENT, 
	      ("Received Neighbor Advert for off-link host.\n"));
      splx(s);
      return;
    }
}


/*----------------------------------------------------------------------
 * ipv6_discov_resolve(): 
 *      Called by LAN output routine.  Will either queue outgoing packet
 *   on the discovery structure that hangs off the route and return 0,
 *   OR copies LAN MAC address into edst and returns 1.
 *
 * NOTES:
 *    Neighbor Unreachable Detection (NUD) should be done here.
 ----------------------------------------------------------------------*/

int ipv6_discov_resolve(ifp, rt, outgoing, dst, edst)
     struct ifnet *ifp;
     struct rtentry *rt;
     struct mbuf *outgoing;
     struct sockaddr *dst;
     u_char *edst;
{
  struct discq       *dq  = NULL;
  struct sockaddr_dl *sdl;
  struct ipv6        *ipv6;

  DPRINTF(IDL_EVENT,("Entering ipv6_discov_resolve().\n"));
  if (rt)
    dq = (struct discq *)rt->rt_llinfo;
  else
    {
      rt = rtalloc1(dst, 0);
      
      /*
       * I just did a passive route lookup.  I'll either get:
       *
       * 1. No route, meaning I have to create one.
       * 2. A host neighbor (RTF_LLINFO) route, meaning I'm good to go.
       * 3. A host off-link route, meaning I possibly missed a redirect.
       * 4. An on-link prefix (RTF_CLONING, no RTF_GATEWAY) route, meaning
       *    I have to create one, like no route.
       * 5. The default route (RTF_DEFAULT), meaning the same as no route.
       * 6. A network route, meaning either a subset of that prefix is on-link,
       *    or my routing table is invalid.  I'll create a new route entry.
       */
      
      if (rt == NULL  ||  !(rt->rt_flags & RTF_HOST))
	{
	  /*
	   * No available host route, create a new entry.
	   * (Cases 1, 4, 5, 6.)
	   */
	  if (rt != NULL)
	    rt->rt_refcnt--;
	  
	  rt = ipv6_new_neighbor((struct sockaddr_in6 *)dst,ifp);
	  if (rt == NULL)
	    {
	      DPRINTF(IDL_CRITICAL, 
		      ("Can't allocate soliciting neighbor route.\n"));
	      m_freem(outgoing);
	      return 0;
	    }
	}
      else if (rt->rt_flags & RTF_LLINFO)
	{
	  /* (Case 2) */
	  rt->rt_refcnt--;
	  if (rt->rt_gateway->sa_family != AF_LINK)
	    panic("LLINFO but gateway != AF_LINK.");
	}
      else
	{
	  /* 
	   * I just got a neighbor solicit from an address which I have
	   * an off-net host route for.  For now, bail.  (Case 3.)
	   */
	  DPRINTF(IDL_MAJOR_EVENT,
		  ("Received Neighbor Solicit from unknown target.\n"));
	  return 0;
	}

      dq = (struct discq *)rt->rt_llinfo;
    }

  if (dq == NULL)
    {
      DPRINTF(IDL_MAJOR_EVENT,("No discq structure hanging off route.\n"));
      m_freem(outgoing);
      return 0;
    }
  if (dq->dq_rt != rt)
    panic("discov_resolve route passed in (rt) != dq->dq_rt");

  sdl = (struct sockaddr_dl *)rt->rt_gateway;
  if (sdl->sdl_family != AF_LINK)
    panic("ipv6_discov_resolve called with rt->rt_gateway->sa_family == %d.\n",
	  sdl->sdl_family);

  if (sdl->sdl_alen == 0)
    {
      /*
       * I'm in INCOMPLETE mode or a new entry.
       *
       * Also, if this is a LINK-LOCAL address, (or there is some other 
       * reason that it isn't clear which interface the address is on)
       * I might want to send the solicit out all interfaces.
       */

      rt->rt_flags &= ~RTF_REJECT;  /* Clear RTF_REJECT in case LAN output
				       routine caught expiration before
				       the timer did. */
      ipv6_nsolicit(ifp,outgoing,rt);
      return 0;
    }

  if (dq->dq_unanswered < 0 && time.tv_sec >= rt->rt_rmx.rmx_expire)
    {
      /*
       * Timeout on REACHABLE entry.  Process accordingly.
       * Change the timed out REACHABLE entry into PROBE state.
       * ( REACHABLE -> PROBE )
       * PROBE state handling is the job of ipv6_discovery_timer().
       */
      rt->rt_rmx.rmx_expire = time.tv_sec + v6d_delfirstprobe;
      dq->dq_unanswered = 0;
      ipv6 = mtod(outgoing,struct ipv6 *);
    }

  DPRINTF(IDL_EVENT,("ipv6_discov_resolve() returning 1.\n"));
  bcopy(LLADDR(sdl),edst, sdl->sdl_alen);
  return 1;
}


/*----------------------------------------------------------------------
 * tunnel_parent():
 * 	Set up tunnel state for a network (cloning?) tunnel route.
 * Right now, there is no tunnel state unless an IPv6 secure tunnel
 * (rt->rt_gateway->sa_family == AF_INET6 && (RTF_CRYPT || RTF_AUTH))
 ----------------------------------------------------------------------*/

void tunnel_parent(rt)
     register struct rtentry *rt;
{
  struct ifnet *ifp;
  struct rtentry *chaser = rt;

  DPRINTF(IDL_EVENT,("In tunnel_parent()."));

  /*
   * For now, set up master tunnel MTU.  Chase rt_gwroute until no more, and
   * see if there is either rmx_mtu or ifp->mtu to transfer.  This should
   * work on both cloning and non-cloning tunnel routes.
   *
   * Q:  Do I want to chase it all the way?  Or just to the next one?
   * A:  For now go to the next one.  Change the following "if" to a
   *     "while" if you want to switch.
   */

/* Change "if" to "while" for all-the-way chasing. */
  if (chaser->rt_gwroute != NULL)   
    chaser = chaser->rt_gwroute;

  DDO(IDL_EVENT,printf("Last route in gwroute chain is:\n");\
      dump_rtentry(rt));

  if (chaser->rt_rmx.rmx_mtu != 0)
    {
      DPRINTF(IDL_EVENT,("Chaser's route MTU (%d) is set.\n",\
			       chaser->rt_rmx.rmx_mtu));
      rt->rt_rmx.rmx_mtu = chaser->rt_rmx.rmx_mtu;
    }
  else
    {
      DPRINTF(IDL_EVENT,("Chaser's route MTU is not set.  "));
      DPRINTF(IDL_EVENT,("Attempting ifp check.\n"));
      if (chaser->rt_ifp == NULL)
	{
	  DPRINTF(IDL_EVENT,\
		  ("Can't find ifp.  Using IPV6_MINMTU (%d).\n",IPV6_MINMTU));
	  rt->rt_rmx.rmx_mtu = IPV6_MINMTU;
	}
      else
	{
	  DPRINTF(IDL_EVENT,("Found ifp with mtu of (%d).\n",\
				   chaser->rt_ifp->if_mtu));
	  rt->rt_rmx.rmx_mtu = chaser->rt_ifp->if_mtu;
	}
    }

  /*
   * Adjust based on any known encapsulations:
   */

  /* Use the rt->rt_gateway->sa_family to determine kind of tunnel */
  if (rt->rt_gateway->sa_family == AF_INET)  /* v6 in v4 tunnel */
    {
      rt->rt_rmx.rmx_mtu -= sizeof(struct ip);
    }
  if (rt->rt_gateway->sa_family == AF_INET6)  
    /* NB: v6-in-v6 tunnel might have secure bit set. */
    {
      /* Perhaps need to ensure that the rt_gateway is a neighbor. */
      rt->rt_rmx.rmx_mtu -= sizeof(struct ipv6);
      if (rt->rt_flags & (RTF_CRYPT|RTF_AUTH))
	{
	  struct socket *so;

	  DPRINTF(IDL_EVENT,("Setting up secure tunnel state.\n"));
	  rt->rt_llinfo = malloc(sizeof(*so),M_SOCKET,M_NOWAIT);
	  if (rt->rt_llinfo == NULL)
	    {
	      DPRINTF(IDL_CRITICAL,\
		      ("WARNING:  Cannot allocate socket.\n"));
	    }
	  so = (struct socket *)rt->rt_llinfo;
	  bzero(so,sizeof(*so));
	  so->so_securitylevel[0] = 1;  /* element 0 is refcnt. */

	  /* These should be configurable, possibly by a sysctl or
	     by calling the IPsec security policy engine */
	  if (rt->rt_flags & RTF_AUTH)
	    so->so_securitylevel[SS_AUTHENTICATION] = IPSEC_LEVEL_UNIQUE;
	  if (rt->rt_flags & RTF_CRYPT)
	    so->so_securitylevel[SS_ENCRYPTION_TRANSPORT] = IPSEC_LEVEL_UNIQUE;
	}
    }
}

/*----------------------------------------------------------------------
 * tunnel_parent_clean():
 *	Frees dummy socket of secure tunnel.
 *  Any other tunnel state should also be cleaned up here for the parent.
 ----------------------------------------------------------------------*/
void tunnel_parent_clean(rt)
     register struct rtentry *rt;
{
  if (rt->rt_flags & (RTF_CRYPT|RTF_AUTH))
    {
      struct socket *so = (struct socket *) rt->rt_llinfo;

      DPRINTF(IDL_EVENT,("Tearing down secure tunnel state.\n"));
      if (so == NULL)
	{
	  DPRINTF(IDL_CRITICAL,("WARNING: Secure tunnel w/o dummy socket.\n"));
	  return;
	}
      if (--(so->so_securitylevel[0]))  /* Refcnt != 0 */
	return;
      free(so,M_SOCKET);
      rt->rt_llinfo = NULL;
    }
}

/*----------------------------------------------------------------------
 * tunnel_child():
 * 	Set up tunnel state for a host or cloned tunnel route.
 * Right now, there is no tunnel state except for IPv6 secure tunnels
 * (rt->rt_gateway->sa_family == AF_INET6 && (RTF_CRYPT || RTF_AUTH))
 ----------------------------------------------------------------------*/

void tunnel_child(rt)
     register struct rtentry *rt;
{
  struct sockaddr_in6 *dst = (struct sockaddr_in6 *)rt_key(rt);
  struct sockaddr_in *sin;

  DPRINTF(IDL_EVENT,("In tunnel_child()."));

  /* Turn off STATIC flag only if cloned. */
  if (rt->rt_parent != NULL)
    rt->rt_flags &= ~RTF_STATIC;

  /* 
   * If additional tunnel state were needed, it could be hung off rt_llinfo.
   */

  if (rt->rt_gateway->sa_family == AF_INET)
    if (IS_IN6_IPV4_COMPAT(dst->sin6_addr))
      {
	/* Create new self-tunneling IPv6 over IPv4 route. */
	/* DANGER:  If original setgate doesn't work properly, this
	            could be trouble. */
	sin = (struct sockaddr_in *)rt->rt_gateway;
	sin->sin_addr.s_addr = dst->sin6_addr.in6a_words[3];
	if (rt_setgate(rt,rt_key(rt),rt->rt_gateway))
	  {
	    DPRINTF(IDL_MAJOR_EVENT, 
		    ("rt_setgate failed in tunnel_child().\n"));
	    rt->rt_rmx.rmx_mtu = 0;
	    /* More should probably be done here. */
	  }
      }
    else
      {
	/* Trust that the rt_gateway & rt_gwroute is good. */
      }

  if (rt->rt_gateway->sa_family == AF_INET6)
    if (rt->rt_flags & (RTF_CRYPT|RTF_AUTH))
      {
	DPRINTF(IDL_EVENT,("Host secure route.  "));
	if (rt->rt_parent == NULL)
	  {
	    struct socket *so;
	    
	    DPRINTF(IDL_EVENT,("Added fresh.\n"));
	    DPRINTF(IDL_EVENT,("Setting up secure tunnel state.\n"));
	    rt->rt_llinfo = malloc(sizeof(*so),M_SOCKET,M_NOWAIT);
	    if (rt->rt_llinfo == NULL)
	      {
		DPRINTF(IDL_CRITICAL,\
			("WARNING:  Cannot allocate socket.\n"));
	      }
	    so = (struct socket *)rt->rt_llinfo;
	    bzero(so,sizeof(*so));
	    so->so_securitylevel[0] = 1;  /* element 0 is refcnt. */
	    /* Security Level should be more configurable, possibly using
	       a sysctl or by calling the IPsec security policy engine */
	    if (rt->rt_flags & RTF_AUTH)
	      so->so_securitylevel[SS_AUTHENTICATION] = IPSEC_LEVEL_UNIQUE;
	    if (rt->rt_flags & RTF_CRYPT)
	      so->so_securitylevel[SS_ENCRYPTION_TRANSPORT] = IPSEC_LEVEL_UNIQUE;
	  }
	else
	  {
	    struct socket *so = (struct socket *) rt->rt_parent->rt_llinfo;

	    DPRINTF(IDL_EVENT,("Cloned.\n"));
	    if (so == NULL)
	      {
		DPRINTF(IDL_CRITICAL,("DANGER: No socket in parent.\n"));
		return;
	      }
	    rt->rt_llinfo = (caddr_t)so;
	    so->so_securitylevel[0]++;  /* Bump refcnt. */
	  }
      }

  if (rt->rt_gwroute == NULL)
    DPRINTF(IDL_CRITICAL, ("rt_gwroute == NULL in tunnel_child().\n"));
  else
    {
      if (rt->rt_gwroute->rt_ifp != rt->rt_ifp)
	rt->rt_ifp = rt->rt_gwroute->rt_ifp;
      /* Should also handle Path MTU increases. */
    }
}

/*----------------------------------------------------------------------
 * tunnel_child_clean():
 *     probably does something like tunnel_parent_clean...
 ----------------------------------------------------------------------*/
void tunnel_child_clean(rt)
     register struct rtentry *rt;
{
  DPRINTF(IDL_EVENT,("In tunnel_child_clean() (currently NOP).\n"));
  if (rt->rt_flags & (RTF_CRYPT|RTF_AUTH))
    {
      struct socket *so = (struct socket *)rt->rt_llinfo;

      DPRINTF(IDL_EVENT,("Tearing down child secure state."));
      if (so == NULL)
	{
	  DPRINTF(IDL_CRITICAL,("WARNING:  Secure tunnel w/o dummy socket.\n"));
	  return;
	}
      if (--(so->so_securitylevel[0]))  /* Refcnt != 0 */
	return;
      free(so,M_SOCKET);
      rt->rt_llinfo = NULL;
    }
}

/* EOF */
