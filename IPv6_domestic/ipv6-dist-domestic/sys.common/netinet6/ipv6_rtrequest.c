/*
 * ipv6_rtrequest.c -- Routines for handling route requests in IPv6.
 * 
 * Copyright 1995 by Dan McDonald, Bao Phan, & Randall Atkinson.
 * All Rights Reserved.
 * All Rights have been assigned to the US Naval Research Laboratory.
 * The NRL Copyright Notice & License Agreement applies to this software.  
 */

/*----------------------------------------------------------------------
#	@(#)COPYRIGHT	1.1a (NRL) 17 August 1995

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
static struct sockaddr_in6 allones;

/*
 * External globals.
 */
extern struct discq    dqhead;
extern struct v6router defrtr;
extern struct v6router nondefrtr;

extern struct ifnet   *ifnet;
extern struct in6_ifaddr *in6_ifaddr;

extern int ipv6forwarding;

/*
 * General notes:
 *
 * These functions are mainly called from ipv6_discovery.c but are
 * fairly generic and generally useful and so live in their own file.
 *     danmcd rja
 */


/*
 * Functions and macros.
 */


/*----------------------------------------------------------------------
 * get_defrouter():
 * 	Find the best default router out of our list and use it
 *    for this destination route.
 ----------------------------------------------------------------------*/
static void
get_defrouter(rt)
     struct rtentry *rt;
{
  struct v6router *v6r = defrtr.v6r_next, *highesttry= NULL;

  /* List should be sorted in priority order. */
  while (v6r != &defrtr)
    {
      struct sockaddr_dl *sdl;

      if (v6r->v6r_rt == NULL)
	panic("v6r_rt is NULL in default router list.");

      sdl = (struct sockaddr_dl *)v6r->v6r_rt->rt_gateway;
      if (sdl->sdl_family != AF_LINK)
	panic("sdl->sdl_family != AF_LINK");

      if (sdl->sdl_alen != 0)
	{
	  struct v6child *v6c;

	  rt_setgate(rt,rt_key(rt),rt_key(v6r->v6r_rt));
	  rt->rt_llinfo = malloc(sizeof(*v6c),M_DISCQ,M_NOWAIT);
	  if (rt->rt_llinfo == NULL)
	    {
	      printf("get_defrouter(): malloc failed.\n");
	      /* Perhaps route should be freed here */
	    }
	  else
	    {
	      v6c = (struct v6child *)rt->rt_llinfo;
	      insque(v6c,&v6r->v6r_children);
	      v6c->v6c_route = rt;
	    }
	  return;
	}
      else
	{
	  highesttry = v6r;
	}

      v6r = v6r->v6r_next;
    }

  /* No default router entry was found. */
  if (highesttry != NULL)
    {
      /* Have a INCOMPLETE router. */
      DPRINTF(IDL_CRITICAL, ("Incomplete Router in ipv6_rtrequest\n"));
    }

  /* Either  on-link or  in trouble. Needs to be coded. */
     DPRINTF(IDL_CRITICAL, ("On-link or in trouble in ipv6_rtrequest\n"));
}


/*----------------------------------------------------------------------
 * add_netchild():
 * 	Given a network route child (i.e. non-default), 
 *    put this child in the parent's descendant list.
 ----------------------------------------------------------------------*/

static void
add_netchild(rt)
     struct rtentry *rt;
{
  struct v6router *v6r;
  struct v6child *v6c;

  if (rt->rt_parent == NULL)
    {
      DPRINTF(IDL_CRITICAL,
	      ("add_netchild: No parent route. (Must be manually added.)\n"));
      return;
    }
  v6r = (struct v6router *)rt->rt_parent->rt_llinfo;

  v6c = malloc(sizeof(*v6c),M_DISCQ,M_NOWAIT);
  if (v6c == NULL)
    {
      DPRINTF(IDL_CRITICAL, ("add_netchild(): malloc failed.\n"));
      /* Perhaps should free route here */
      return;
    }
  insque(v6c,&v6r->v6r_children);
  v6c->v6c_route = rt;
  rt->rt_llinfo = (caddr_t)v6c;
}

/*----------------------------------------------------------------------
 * add_non_default():
 *	add a non-default routing entry.
 *
 ----------------------------------------------------------------------*/
static void
add_non_default(rt)
     struct rtentry *rt;
{
  struct v6router *newbie;

  if (rt_key(rt)->sa_family != AF_INET6)
    {
      DPRINTF(IDL_CRITICAL, 
	      ("IPv6 off-net non-tunnel route w/o IPv6 gwaddr.\n"));
      return;
    }
    
  newbie = malloc(sizeof(*newbie),M_DISCQ,M_NOWAIT);
  if (newbie == NULL)
    {
      DPRINTF(IDL_CRITICAL, ("add_non_default(): malloc failed.\n"));
      /* Should probably free route */
      return;
    }
  bzero(newbie,sizeof(*newbie));
  newbie->v6r_children.v6c_next = newbie->v6r_children.v6c_prev = 
    &newbie->v6r_children;
  insque(newbie,&nondefrtr);

  /*
   * On creation of rt, rt_setgate() was called, therefore we take on blind
   * faith that an appropriate neighbor cache entry was created.  If not,
   * we're in deep trouble.
   */
  newbie->v6r_rt = rt->rt_gwroute;
  rt->rt_llinfo = (caddr_t)newbie;
}


/*----------------------------------------------------------------------
 * ipv6_setrtifa():
 *	Set route's interface address.
 *
 ----------------------------------------------------------------------*/
void
ipv6_setrtifa(rt)
     struct rtentry *rt;
{
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)rt_key(rt);
  struct sockaddr_in6 *src  = (struct sockaddr_in6 *)rt->rt_ifa->ifa_addr;
  struct ifaddr *ifa = NULL;

  DPRINTF(IDL_EVENT,("Entering ipv6_setrtifa.\n"));

  /*
   * This is called iff I'm a HOST route.
   * ASSUMES: rt->rt_ifp is set CORRECTLY.
   */

  if (IS_IN6_LINKLOC(sin6->sin6_addr))
    {
      if (IS_IN6_LINKLOC(src->sin6_addr))
	return;
      for (ifa = rt->rt_ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next)
	{
	  struct sockaddr_in6 *current = (struct sockaddr_in6 *)ifa->ifa_addr;

	  if (current->sin6_family != AF_INET6)
	    continue;  /* For loop. */
	  if (IS_IN6_LINKLOC(current->sin6_addr))
	    break;   /* For loop. */
	}
      if (ifa == NULL)
	return;
    }

  if (ifa == NULL && IS_IN6_IPV4_COMPAT(sin6->sin6_addr))
    {
      if (IS_IN6_IPV4_COMPAT(src->sin6_addr))
	return;
      for (ifa = rt->rt_ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next)
	{
	  struct sockaddr_in6 *current = (struct sockaddr_in6 *)ifa->ifa_addr;

	  if (current->sin6_family != AF_INET6)
	    continue;  /* For loop. */
	  if (IS_IN6_IPV4_COMPAT(current->sin6_addr))
	    break;   /* For loop. */
	}
      if (ifa == NULL)
	return;
    }

  if (ifa == NULL && IS_IN6_SITELOC(sin6->sin6_addr))
    {
      if (IS_IN6_SITELOC(src->sin6_addr))
	return;
      for (ifa = rt->rt_ifp->if_addrlist; ifa != NULL; ifa = ifa->ifa_next)
	{
	  struct sockaddr_in6 *current = (struct sockaddr_in6 *)ifa->ifa_addr;

	  if (current->sin6_family != AF_INET6)
	    continue;  /* For loop. */
	  if (IS_IN6_SITELOC(current->sin6_addr))
	    break;   /* For loop. */
	}
      if (ifa == NULL)
	return;
    }

  if (ifa == NULL)
    {
      /* Punt for now. Later modify to attempt to pick something reasonable. */
    }

  if (ifa != NULL)
    {
      IFAFREE(rt->rt_ifa);
      ifa->ifa_refcnt++;
      rt->rt_ifa = ifa;
    }
}


/*----------------------------------------------------------------------
 * ipv6_rtrequest():
 *	IPv6-specific route ADD, RESOLVE, and DELETE function.
 * Typically called from route.c's rtrequest() function to handle
 * the IPv6 unique things.
 ----------------------------------------------------------------------*/
void
ipv6_rtrequest(req, rt, sa)
     int req;
     register struct rtentry *rt;
     struct sockaddr *sa;
{
  static struct sockaddr_dl null_sdl = {sizeof(null_sdl), AF_LINK};
  struct in6_ifaddr *i6a;

  DPRINTF(IDL_EVENT,("Entering ipv6_discov_rtrequest..."));
  DPRINTF(IDL_EVENT,("req = %d, rt = 0x%x, sa = 0x%x\n",req,\
			   rt,sa));
  DDO(IDL_EVENT,dump_rtentry(rt);dump_smart_sockaddr(sa));

  switch (req)
    {
    case RTM_ADD:

      /*
       * Explicitly add the cloning bit to the route entry.
       *
       * One fallout from this is that if someone wants to disable cloning,
       * the disabling will have to be explicitly done after adding the route.
       */
      if ((rt->rt_flags & RTF_HOST) == 0 &&
	  /* Multicast check may go away if we allow multicast cloning... */
	  !IS_IN6_MULTICAST(((struct sockaddr_in6 *)rt_key(rt))->sin6_addr) &&
	  !IN6_ADDR_EQUAL(((struct sockaddr_in6 *)rt_mask(rt))->sin6_addr,
			  allones.sin6_addr))
	rt->rt_flags |= RTF_CLONING;

      if (rt->rt_flags & RTF_CLONING)
	{
	  DPRINTF(IDL_EVENT,("In 'add cloning route' case.\n"));

	  /*
	   * Order of bit checking is very important here.  
	   * Check TUNNEL first because it's REALLY special,
	   * then check for GATEWAY.
	   */

	  if (rt->rt_flags & RTF_TUNNEL)
	    {
	      DPRINTF(IDL_EVENT,\
		      ("New tunnel route.  Set up structs accordingly?!?\n"));
	      /* Set up MTU?!? */
	      tunnel_parent(rt);
	    }
	  else if (rt->rt_flags & RTF_GATEWAY)
	    {
	      DPRINTF(IDL_EVENT,\
		      ("New router route.  Set up structs accordingly.\n"));

	      /*
	       * If adding the default route, 
	       *     then don't really want to do anything.
	       * Otherwise, might want to add a router list entry.
	       */

	      if (rt_key(rt)->sa_len >=
		  (sizeof(struct sockaddr_in6) - sizeof(struct in_addr6))
		  &&
		  !IS_IN6_UNSPEC(((struct sockaddr_in6 *)rt_key(rt))->sin6_addr)
		  )
		add_non_default(rt);
	      else
		{
		  DPRINTF(IDL_EVENT,("Added default route.\n"));
		  rt->rt_flags |= RTF_DEFAULT;
		}

	      if (rt->rt_gwroute != NULL)
		{
		  /*
		   * Neighbor cache router indication.   Should I set
		   *  this here?
		   */
		  rt->rt_gwroute->rt_flags |= RTF_ISAROUTER;
		}

	    }
	  else
	    {
	      struct sockaddr_dl *sdlp;

	      DPRINTF(IDL_EVENT,("Setting up i/f route.\n"));
	      rt_setgate(rt, rt_key(rt), (struct sockaddr *)&null_sdl);
	      sdlp = (struct sockaddr_dl *)rt->rt_gateway;
	      sdlp->sdl_type = rt->rt_ifp->if_type;
	      sdlp->sdl_index = rt->rt_ifp->if_index;
	    }
	  /* Any other cloning that might be needed should be handled here */
	  
	  DPRINTF(IDL_EVENT,("Out of cloning add code, rt now:\n"));
	  DDO(IDL_EVENT,dump_rtentry(rt));
	  break;  /* Out of switch. */
	}
      /*
       * If needed, check for rt_gateway being the loopback AF_LINK address.
       */

      /* FALLTHROUGH... */
    case RTM_RESOLVE:
      DPRINTF(IDL_EVENT,("In RTM_RESOLVE case.\n"));
      /*
       * First, check if ifa addr is same as route addr, 
       * If yes, then wire the loopback interface as the destination.
       */
      if (IN6_ADDR_EQUAL(((struct sockaddr_in6 *)rt_key(rt))->sin6_addr,
			 I6A_SIN(rt->rt_ifa)->sin6_addr))
	{
	  extern struct ifnet loif;

	  /* Change ifp to loopback and gateway to itself. */
	  rt->rt_ifp = &loif;
	  rt->rt_gateway = rt_key(rt);
	  /* Initialize rt->rt_rmx.rmx_mtu ??!!?? */
	  break;
	}

      /*
       * Second, check the accuracy of the route's interface address (ifa).
       * If possible, update the route's ifa such that it is the "best"
       * source address for packets bound for this destination.
       *
       * This is done iff I'm a HOST route and is subtle enough
       * to merit its own separate function:
       */
      if (rt->rt_flags & RTF_HOST)
	ipv6_setrtifa(rt);

      /*
       * At this point, I know I'm a host route which has either been
       * added manually, or cloned off either a:
       *
       * a tunnelling route child,
       *      RTF_TUNNEL is set on this entry.
       *      I should be okay, although tunnel state may become necessary.
       *      I'm a next-hop cache entry.
       *
       * default router child,
       * a non-default network route child,
       *      RTF_GATEWAY is set on this entry.
       *      I have to insert this child into the router's children list.
       *      My child entry hangs off rt_llinfo.  I'm a next-hop cache
       *      entry.
       *
       * an interface (i.e. neighbor) route child,
       * a link-local (i.e. off link-local mask) route child
       *      rt->rt_gateway is AF_LINK
       *      I should set RTF_LLINFO, and set up a discq entry because I'm
       *      a neighbor cache entry.  (Neighbor caches double as next-hop
       *      cache entries.)
       *
       * none of the above (i.e. not a child)
       *      In case I don't WANT a route to clone.  (Useful if I'm a
       *      router.)  Right now, multicast is the only known case.
       */

      if ((rt->rt_flags & RTF_HOST) == 0)
	{
	  /*
	   * This should be the "none of the above" case.  For now,
	   * do nothing and just return.
	   */
	  DPRINTF(IDL_EVENT,("In RESOLVE/lateADD, but not RTF_HOST.\n"));
	  break;
	}

      if (rt->rt_gateway == NULL)
	panic("ipv6_rtrequest():  No rt_gateway at the RTM_RESOLVE code.\n");

      /*if (rt->rt_refcnt == 0)
	rt->rt_refcnt = 1;
      else panic("ipv6_rtrequest(): Cloning created refcnt != 0\n");*/

      /*
       * Order of bit checking is very important.  I check TUNNEL first,
       * because it's REALLY special.  Then I check GATEWAY or not.
       */

      if (rt->rt_flags & RTF_TUNNEL)
	{
	  tunnel_child(rt);
	  break;
	}

      if (rt->rt_flags & RTF_GATEWAY)
	{
	  if (rt->rt_flags & RTF_DEFAULT)
	    get_defrouter(rt);
	  else add_netchild(rt);
	  break;
	}

      if (rt->rt_gateway->sa_family == AF_LINK)
	{
	  struct discq *dq;

	  rt->rt_llinfo = malloc(sizeof(*dq),M_DISCQ,M_NOWAIT);
	  dq = (struct discq *)rt->rt_llinfo;
	  if (dq == NULL)
	    {
	      DPRINTF(IDL_CRITICAL,
		      ("ipv6_rtrequest(): malloc for rt_llinfo failed.\n"));
	      /* Probably should free route */
	      break;
	    }
	  bzero(dq,sizeof(*dq));
	  dq->dq_rt = rt;
	  dq->dq_unanswered = -1;
	  rt->rt_flags |= RTF_LLINFO;
	  insque(dq,&dqhead);
	  /*
	   * State is already INCOMPLETE, because of link stuff.
	   */
	  break;
	}
      else
	/* support for PPP, Frame Relay, and ATM should probably be added */
	panic("ipv6_rtrequest: Not tunnel, not off-net, and not neighbor.");

      break;
    case RTM_DELETE:
      DPRINTF(IDL_EVENT,("In RTM_DELETE case.\n"));
      if ((rt->rt_flags & RTF_HOST) == 0)
	{
	  /* Clean up after network routes. */
	  if (rt->rt_flags & RTF_TUNNEL)
	    {
	      DPRINTF(IDL_EVENT,("Cleaning up tunnel route.\n"));
	      tunnel_parent_clean(rt);
	      return;
	    }
	  if (rt->rt_flags & RTF_GATEWAY)
	    if (rt->rt_flags & RTF_DEFAULT)
	      {
		DPRINTF(IDL_EVENT,\
			("Cleaning up THE default route.\n"));
	      }
	    else
	      {
		struct v6router *v6r = (struct v6router *)rt->rt_llinfo;
		struct v6child *v6c;

		/* Non-default router.  Default router taken care of
		   elsewhere, because of multiple defaults. */
		DPRINTF(IDL_EVENT,\
			("Cleaning up a non-{default,host} route.\n"));
		
		if (v6r == NULL)
		  panic("Non-default w/o v6router entry.");
		
		v6c = v6r->v6r_children.v6c_next;
		while (v6c != &v6r->v6r_children)
		  {
		  /* rtrequest() should remove children from the linked list */
		    rtrequest(RTM_DELETE,rt_key(v6c->v6c_route),
			      NULL,NULL,0,NULL);
		    v6c = v6r->v6r_children.v6c_next;
		  }
	      }
	  /* Anything else that isn't a HOST needs no work, so return. */
	  return;
	}
      else
	{
	  DPRINTF(IDL_EVENT,("v6_discov_rtrequest() deleting a host\n"));
	}

      DPRINTF(IDL_EVENT,("I'm at a host-route point.\n"));
      if (rt->rt_flags & RTF_TUNNEL)
	{
	  DPRINTF(IDL_EVENT,("Tunneling child.\n"));
	  tunnel_child_clean(rt);
	}
      else if (rt->rt_flags & RTF_GATEWAY)
	{
	  struct v6child *v6c = (struct v6child *) rt->rt_llinfo;

	  if (v6c == NULL)
	    if (rt->rt_flags & RTF_HOST)
	      DPRINTF(IDL_CRITICAL, 
		      ("no v6c in RTM_DELETE of RTF_GATEWAY.\n"));
	  else
	    {
	      remque(v6c);
	      rt->rt_llinfo = NULL;
	      free(v6c,M_DISCQ);
	    }
	}
      else if (rt->rt_flags & RTF_LLINFO)
	{
	  struct discq *dq;

	  /* Neighbor cache entry. */
	  if (rt->rt_flags & RTF_ISAROUTER)
	    {
	      struct v6router *v6r = defrtr.v6r_next;

	      /* This had better be a default router.  
		 If not, then it is a non-default router, 
		 and the cleanup code above will have taken care of this.

		 Clean up all children of this default router. */
	      
	      DPRINTF(IDL_EVENT,\
		      ("Cleaning up default router neighbor.\n"));
	      while (v6r != &defrtr)
		{
		  if (v6r->v6r_rt == rt)
		    {
		      struct v6child *v6c = v6r->v6r_children.v6c_next;

		      while (v6c != &v6r->v6r_children)
			{
		  /* rtrequest() should remove children from the linked list.*/
			  rtrequest(RTM_DELETE,rt_key(v6c->v6c_route),
				    NULL,NULL,0,NULL);
			  v6c = v6r->v6r_children.v6c_next;
			}
		      break;   /* Out of while loop. */
		    }
		  v6r = v6r->v6r_next;
		}
	    }

	  dq = (struct discq *)rt->rt_llinfo;
	  rt->rt_flags &= ~RTF_LLINFO;
	  if (dq == NULL)
	    panic("No discq or other rt_llinfo in RTM_DELETE");
	  remque(dq);
	  rt->rt_llinfo = NULL;
	  if (dq->dq_queue)
	    {
	      /* Send ICMP unreachable error. */
	      ipv6_icmp_error(dq->dq_queue, ICMPV6_UNREACH,
			      ICMPV6_UNREACH_ADDRESS, 0);
	      /* m_freem(dq->dq_queue);*/
	    }
	  free(dq,M_DISCQ);
	}
      else
	{
	  DPRINTF(IDL_MAJOR_EVENT,\
		  ("Freeing self-wired address.  Doing nothing.\n"));
	}
      break;
    }
}

/* EOF */
