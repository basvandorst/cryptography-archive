/*
 * in6.c  --  General IPv6 internet routines, such as interface adding, etc.
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
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/if_dl.h>
#include <net/route.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h>

#include <netinet6/in6_debug.h>

/*
 * Globals
 */

struct ifnet *mcastdefault = NULL;   /* Should be changeable by sysctl(). */
struct ifnet *linklocdefault = NULL; /* Same here. */

/*
 * External globals
 */

extern struct in6_ifaddr *in6_ifaddr;



/*----------------------------------------------------------------------
 * Set the default multicast interface.  In single-homed case, this will
 * always be the non-loopback interface.  In multi-homed cases, the function
 * should be able to set one accordingly.  The multicast route entry
 * (ff00::/8) will have its rt_ifp point to this interface, and its rt_ifa
 * point to whatever rtrequest() does.  The rt_ifa should be more intelligently
 * set eventually.
 ----------------------------------------------------------------------*/

static void
setmcastdef(ifp)
     register struct ifnet *ifp;
{
  struct ifaddr *ifa = ifp->if_addrlist;
  struct sockaddr_dl lsdl;
  struct sockaddr_in6 lsin6;
  struct rtentry *newrt=NULL;
  int s;

  if (ifp == mcastdefault)
    return;

  /*
   * Find link addr for ifp.
   */

  while (ifa != NULL && ifa->ifa_addr->sa_family != AF_LINK)
    ifa = ifa->ifa_next;

  if (ifa == NULL)
    panic("Can't find AF_LINK for new multicast default interface.");

  bcopy(ifa->ifa_addr,&lsdl,ifa->ifa_addr->sa_len);
  DDO(IDL_EVENT,dump_smart_sockaddr(&lsdl));
  lsdl.sdl_alen = 0;
  lsdl.sdl_slen = 0;
  lsdl.sdl_nlen = 0;

  /*
   * Delete old route, and add new one.
   */

  bzero(&lsin6,sizeof(lsin6));
  lsin6.sin6_family = AF_INET6;
  lsin6.sin6_len = sizeof(lsin6);
  lsin6.sin6_addr.in6a_words[0]=htonl(0xff000000); /* Others were bzeroed. */

  /* Neat property, mask and value are identical! */

  s = splnet();
  rtrequest(RTM_DELETE,(struct sockaddr *)&lsin6,NULL,
	    (struct sockaddr *)&lsin6,0,NULL);
  /*
   *
   * NB: If we clone, we have mcast dests being on a route.  
   *     Consider multihomed system with processes talking to the 
   *     same mcast group, but out different interfaces.
   *
   * Also, the RTM_ADD will do its best to find a "source address" to stick
   * in the rt_ifa field.  (See ipv6_rtrequest.c for this code.)
   */
  rtrequest(RTM_ADD,(struct sockaddr *)&lsin6,(struct sockaddr *)&lsdl,
	    (struct sockaddr *)&lsin6,0,&newrt);
  if (newrt == NULL)
    panic("Assigning default multicast if.");
  newrt->rt_rmx.rmx_mtu = ifp->if_mtu;
  newrt->rt_refcnt--;
  mcastdefault = ifp;
  splx(s);
}

/*----------------------------------------------------------------------
 * The same principle that applies to multicast applies to link-local
 * addresses as well.  The difference is, these unicast routes do clone,
 * and that the multi-homed case isn't quite there yet.  ipv6_discovery.c
 * and ipv6_rtrequest.c will be the places to address this.  This function
 * merely picks a default.  It's possible this function will be deprecated
 * if neighbor discovery can be made to work well in the multi-homed case.
 ----------------------------------------------------------------------*/

static void
setlinklocdef(ifp)
     register struct ifnet *ifp;
{
  struct ifaddr *ifa = ifp->if_addrlist;
  struct sockaddr_dl lsdl;
  struct sockaddr_in6 lsin6,lmask;
  struct rtentry *newrt=NULL;
  struct in6_ifaddr *oldi6a = NULL,*newi6a = NULL,*i6a;
  int s;

  if (ifp == linklocdefault)
    return;

  /*
   * Find link addr for ifp.
   */

  while (ifa != NULL && ifa->ifa_addr->sa_family != AF_LINK)
    ifa = ifa->ifa_next;

  if (ifa == NULL)
    panic("Can't find AF_LINK for new link-local default interface.");

  bcopy(ifa->ifa_addr,&lsdl,ifa->ifa_addr->sa_len);
  DDO(IDL_EVENT,dump_smart_sockaddr(&lsdl));
  lsdl.sdl_alen = 0;
  lsdl.sdl_slen = 0;
  lsdl.sdl_nlen = 0;

  /*
   * Delete old route, and add new one.
   */

  bzero(&lsin6,sizeof(lsin6));
  lsin6.sin6_family = AF_INET6;
  lsin6.sin6_len = sizeof(lsin6);
  lsin6.sin6_addr.in6a_words[0]=htonl(0xfe800000); /* Others were bzeroed. */

  bzero(&lmask,sizeof(lmask));
  lmask.sin6_family = AF_INET6;
  lmask.sin6_len = sizeof(lmask);
  lmask.sin6_addr.in6a_words[0]=htonl(0xffc00000); /* Others were bzeroed. */

  s = splnet();

  /*
   * But first, find old link-local in6_ifaddr, and mask it to all-ones 
   * again.  This is because, unlike multicast, the link-local default has
   * a masked in6_ifaddr associated with it.
   */
  if (linklocdefault != NULL)
    {
      for(i6a = in6_ifaddr; i6a != NULL; i6a = i6a->i6a_next)
	{
	  if (IS_IN6_LINKLOC(i6a->i6a_addr.sin6_addr))
	    {
	      if (i6a->i6a_ifp == ifp)
		newi6a = i6a;
	      if (i6a->i6a_ifp == linklocdefault)
		oldi6a = i6a;
	    }
	  if (oldi6a != NULL)
	    break;
	}
      if (oldi6a == NULL)
	panic("No link-local for old default!");
      IN6_ADDR_ASSIGN(oldi6a->i6a_sockmask.sin6_addr,0xffffffff,0xffffffff,\
		      0xffffffff,0xffffffff);
    }
  else i6a = in6_ifaddr;

  if (newi6a == NULL)
    {
      for (;i6a != NULL; i6a = i6a->i6a_next)
	if (IS_IN6_LINKLOC(i6a->i6a_addr.sin6_addr))
	  if (i6a->i6a_ifp == ifp)
	    {
	      newi6a = i6a;
	      break;
	    }
    }

  if (newi6a == NULL)
    panic("No link-local for new default!");

  newi6a->i6a_sockmask.sin6_addr = lmask.sin6_addr;

  rtrequest(RTM_DELETE,(struct sockaddr *)&lsin6,NULL,
	    (struct sockaddr *)&lmask,0,NULL);
  /*
   * This time, yes.  We set the link-local interface, and then we clone
   * link-local children off that route.
   */
  rtrequest(RTM_ADD,(struct sockaddr *)&lsin6,(struct sockaddr *)&lsdl,
	    (struct sockaddr *)&lmask,RTF_CLONING,&newrt);
  if (newrt == NULL)
    panic("Assigning default link-local if.");
  newrt->rt_rmx.rmx_mtu = ifp->if_mtu;
  newrt->rt_refcnt--;
  linklocdefault = ifp;
  splx(s);
}

/*----------------------------------------------------------------------
 * This function is called by the PRU_CONTROL handlers in both TCP and UDP.
 * (Actually raw_ipv6 might need a PRU_CONTROL handler, but raw_ip doesn't
 * have one.)
 ----------------------------------------------------------------------*/

in6_control(so, cmd, data, ifp, internal)
     struct socket *so;
     int cmd;
     caddr_t data;
     register struct ifnet *ifp;
     int internal;
{
  register struct inet6_ifreq *ifr = (struct inet6_ifreq *)data;
  register struct in6_ifaddr *i6a = 0;
  register struct ifaddr *ifa;
  struct in6_ifaddr *oi6a;
  struct inet6_aliasreq *ifra = (struct inet6_aliasreq *)data;
  struct sockaddr_in6 oldaddr;
  struct mbuf *m;
  int error, hostIsNew, maskIsNew;

  /*
   * If given an interface, find first IPv6 address on that interface.
   * I may want to change how this is searched.  I also may want to
   * discriminate between link-local, site-local, v4-compatible, etc.
   *
   * This is used by the SIOCGIFADDR_INET6, and other such things.
   * Those ioctls() currently assume only one IPv6 address on an interface.
   * This is not a good assumption, and this code will have to be modified
   * to correct that assumption.
   */
  if (ifp)
    for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
      if (i6a->i6a_ifp == ifp)
	break;

  switch (cmd)
    {
    case SIOCAIFADDR_INET6:
    case SIOCDIFADDR_INET6:
      /*
       * For adding and deleting an address, find an exact match for
       * that address.
       */
      if (ifra->ifra_addr.sin6_family == AF_INET6)
	for (oi6a = i6a; i6a; i6a = i6a->i6a_next)
	  {
	    if (i6a->i6a_ifp == ifp &&
		IN6_ADDR_EQUAL(i6a->i6a_addr.sin6_addr,\
			       ifra->ifra_addr.sin6_addr) )
	      break; /* Out of for loop. */
	  }

      /*
       * You can't delete what you don't have...
       */
      if (cmd == SIOCDIFADDR_INET6 && i6a == 0)
	return EADDRNOTAVAIL;

      /* FALLTHROUGH TO... */

    case SIOCSIFDSTADDR_INET6:
      if ((so->so_state & SS_PRIV) == 0)
	return EPERM;

      if (ifp==0)
	panic("in6_control, ifp==0");
      if (i6a == NULL)
	{
	  struct in6_ifaddr *tmp;

	  /*
	   * Create new in6_ifaddr (IPv6 interface address) for additions
	   * and destination settings.
	   */
	  if (!(tmp = (struct in6_ifaddr *)malloc(sizeof(struct in6_ifaddr),
						  M_IFADDR,M_NOWAIT)))
	    {
	      return ENOBUFS;
	    }

	  bzero(tmp,sizeof(struct in6_ifaddr));
	  if (i6a = in6_ifaddr)
	    {
	      for (; i6a->i6a_next; i6a=i6a->i6a_next)
		;
	      i6a->i6a_next = tmp;
	    }
	  else in6_ifaddr = tmp;
	  i6a = tmp;
	  if (ifa = ifp->if_addrlist)
	    {
	      for (; ifa->ifa_next; ifa=ifa->ifa_next)
		;
	      ifa->ifa_next = (struct ifaddr *)i6a;
	    }
	  else ifp->if_addrlist = (struct ifaddr *)i6a;
	  i6a->i6a_ifa.ifa_addr = (struct sockaddr *)&i6a->i6a_addr;
	  i6a->i6a_ifa.ifa_dstaddr = (struct sockaddr *)&i6a->i6a_dstaddr;
	  i6a->i6a_ifa.ifa_netmask
	    = (struct sockaddr *)&i6a->i6a_sockmask;
	  i6a->i6a_sockmask.sin6_len = sizeof(struct sockaddr_in6);
	  i6a->i6a_ifp = ifp;
	}
      break;
    case SIOCGIFADDR_INET6:
    case SIOCGIFNETMASK_INET6:
    case SIOCGIFDSTADDR_INET6:
      /*
       * Can't get information on what is not there...
       */
      if (i6a == NULL)
	return EADDRNOTAVAIL;
      break;

    default:
      return EOPNOTSUPP;
    }

  switch (cmd)
    {
      /*
       * The following three cases assume that there is only one address per
       * interface; this is not good in IPv6-land.  Unfortunately, the
       * ioctl() interface, is such that I'll have to rewrite the way things
       * work here, either that, or curious user programs will have to troll
       * /dev/kmem (like netstat(8) does).
       */
    case SIOCGIFADDR_INET6:
      bcopy(&(i6a->i6a_addr),&(ifr->ifr_addr),sizeof(struct sockaddr_in6));
      break;

    case SIOCGIFDSTADDR_INET6:
      if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
	return EINVAL;
      bcopy(&(i6a->i6a_dstaddr),&(ifr->ifr_dstaddr),
	    sizeof(struct sockaddr_in6));
      break;

    case SIOCGIFNETMASK_INET6:
      bcopy(&(i6a->i6a_sockmask),&(ifr->ifr_addr),sizeof(struct sockaddr_in6));
      break;

    case SIOCSIFDSTADDR_INET6:
      if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
	return EINVAL;
      oldaddr = i6a->i6a_dstaddr;
      i6a->i6a_dstaddr = *(struct sockaddr_in6 *)&ifr->ifr_dstaddr;
      if (ifp->if_ioctl && (error = (*ifp->if_ioctl)(ifp, SIOCSIFDSTADDR,
						     (caddr_t)i6a)))
	{
	  i6a->i6a_dstaddr = oldaddr;
	  return error;
	}
      if (i6a->i6a_flags & IFA_ROUTE)
	{
	  i6a->i6a_ifa.ifa_dstaddr = (struct sockaddr *)&oldaddr;
	  rtinit(&(i6a->i6a_ifa), RTM_DELETE, RTF_HOST);
	  i6a->i6a_ifa.ifa_dstaddr = (struct sockaddr *)&i6a->i6a_dstaddr;
	  rtinit(&(i6a->i6a_ifa), RTM_ADD, RTF_HOST|RTF_UP);
	}
      break;

      /*
       * For adding new IPv6 addresses to an interface, I stuck to the way
       * that IPv4 uses, pretty much.
       */
    case SIOCAIFADDR_INET6:
      maskIsNew = 0;
      hostIsNew = 1;
      error = 0;
      if (i6a->i6a_addr.sin6_family == AF_INET6)
	if (ifra->ifra_addr.sin6_len == 0)
	  {
	    bcopy(&(i6a->i6a_addr),&(ifra->ifra_addr),
		  sizeof(struct sockaddr_in6));
	    hostIsNew = 0;
	  }
	else if (IN6_ADDR_EQUAL(ifra->ifra_addr.sin6_addr,
				i6a->i6a_addr.sin6_addr) )
	  hostIsNew = 0;

      if (ifra->ifra_mask.sin6_len)
	{
	  in6_ifscrub(ifp,i6a);
	  bcopy(&(ifra->ifra_mask),&(i6a->i6a_sockmask),
		sizeof(struct sockaddr_in6));
	  maskIsNew = 1;
	}

      if ((ifp->if_flags & IFF_POINTOPOINT) &&
	  (ifra->ifra_dstaddr.sin6_family == AF_INET6))
	{
	  in6_ifscrub(ifp,i6a);
	  bcopy(&(ifra->ifra_dstaddr),&(i6a->i6a_dstaddr),
		sizeof(struct sockaddr_in6));
	  maskIsNew = 1;  /* We lie, so that in6_ifinit() will be called. */
	}
      if (ifra->ifra_addr.sin6_family == AF_INET6 && (hostIsNew || maskIsNew))
	error = in6_ifinit(ifp,i6a,&ifra->ifra_addr,0);
      return error;

    case SIOCDIFADDR_INET6:
      in6_ifscrub(ifp, i6a);
      if ((ifa = ifp->if_addrlist) == (struct ifaddr *)i6a)
	ifp->if_addrlist = ifa->ifa_next;
      else
	{
	  while (ifa->ifa_next &&
		 (ifa->ifa_next != (struct ifaddr *)i6a))
	    ifa=ifa->ifa_next;
	  if (ifa->ifa_next)
	    ifa->ifa_next = i6a->i6a_ifa.ifa_next;
	  else 
	    DPRINTF(IDL_CRITICAL, ("Couldn't unlink in6_ifaddr from ifp!\n"));
	}
      oi6a = i6a;
      if (oi6a == (i6a = in6_ifaddr))
	in6_ifaddr = i6a->i6a_next;
      else
	{
	  while (i6a->i6a_next && (i6a->i6a_next != oi6a))
	    i6a = i6a->i6a_next;
	  if (i6a->i6a_next)
	    i6a->i6a_next = oi6a->i6a_next;
	  else 
	    DPRINTF(IDL_CRITICAL, ("Didn't unlink in6_ifaddr from list.\n"));
	}
      IFAFREE((&oi6a->i6a_ifa));  /* For the benefit of routes pointing
				     to this ifa. */
      break;

    default:
      DPRINTF(IDL_CRITICAL, 
	      ("in6_control(): Default case not implemented.\n"));
      return EOPNOTSUPP;
    }

  return 0;
}

/*----------------------------------------------------------------------
 in6_ifscrub:   
                 Delete any existing route for an IPv6 interface.
 ----------------------------------------------------------------------*/

in6_ifscrub(ifp,i6a)
     register struct ifnet *ifp;
     register struct in6_ifaddr *i6a;
{
  if (!(i6a->i6a_flags & IFA_ROUTE))
    return 1;

  if (ifp->if_flags & (IFF_LOOPBACK|IFF_POINTOPOINT))
    rtinit(&(i6a->i6a_ifa), (int)RTM_DELETE, RTF_HOST);
  else
    rtinit(&(i6a->i6a_ifa), (int)RTM_DELETE, 0);
  i6a->i6a_flags &= ~IFA_ROUTE;

  return 0;
}

/*----------------------------------------------------------------------
 * Initialize an IPv6 address for an interface.
 *
 * When I get around to doing duplicate address detection, this is probably
 * the place to do it.
 ----------------------------------------------------------------------*/

in6_ifinit(ifp, i6a, sin6, scrub)
     register struct ifnet *ifp;
     register struct in6_ifaddr *i6a;
     struct sockaddr_in6 *sin6;
     int scrub;
{
  int s, error, flags = RTF_UP;
  struct sockaddr_in6 oldaddr;

  DPRINTF(IDL_EVENT,("Before splimp in in6_ifinit()\n"));
  s = splimp();

  bcopy(&(i6a->i6a_addr),&oldaddr,sizeof(struct sockaddr_in6));
  bcopy(sin6,&(i6a->i6a_addr),sizeof(struct sockaddr_in6));

  /*
   * Give the interface a chance to initialize
   * if this is its first address,
   * and to validate the address if necesi6ary.
   */

  if (ifp->if_ioctl && (error = (*ifp->if_ioctl)(ifp, SIOCSIFADDR,
						 (caddr_t)i6a)))
    { 
      splx(s);
      bcopy(&oldaddr,&(i6a->i6a_addr),sizeof(struct sockaddr_in6));
      
      return error;
    }

  /*
   * IPv4 in 4.4BSD sets the RTF_CLONING flag here if it's an Ethernet.
   * I will delay this until later.
   */

  splx(s);
  DPRINTF(IDL_EVENT,("After splx() in in6_ifinit().\n"));

  if (scrub)
    {
      i6a->i6a_ifa.ifa_addr = (struct sockaddr *)&oldaddr;
      in6_ifscrub(ifp, i6a);
      i6a->i6a_ifa.ifa_addr = (struct sockaddr *)&i6a->i6a_addr;
    }

  /*
   * Adjust the sin6_len such that it only counts mask bytes with
   * 1's in them.
   */

  {
    register char *cpbase = (char *)&(i6a->i6a_sockmask.sin6_addr);
    register char *cp = cpbase + sizeof(struct in_addr6);

    while (--cp >=cpbase)
      if (*cp)
	{
	  i6a->i6a_sockmask.sin6_len = 1 + cp - (char *)&(i6a->i6a_sockmask);
	  break;
	}
  }

  /*
   * Add route.  Also, set some properties of the interface address here.
   * (Properties include permanance, lifetime, etc.)
   */

  i6a->i6a_ifa.ifa_metric = ifp->if_metric;
  i6a->i6a_ifa.ifa_rtrequest = ipv6_rtrequest;  /* Want this to be true
						   for ALL IPv6 ifaddrs. */
  if (ifp->if_flags & IFF_LOOPBACK)
    {
      i6a->i6a_ifa.ifa_dstaddr = i6a->i6a_ifa.ifa_addr;
      flags |= RTF_HOST;
      if (IS_IN6_LOOPBACK(i6a->i6a_addr.sin6_addr))  /* Loopback is definitely
							a permanent address. */
	i6a->i6a_addrflags = I6AF_PERMANENT;
    }
  else if (ifp->if_flags & IFF_POINTOPOINT)
    {
      if (i6a->i6a_dstaddr.sin6_family != AF_INET6)
	return 0;

      flags |= RTF_HOST;
    }
  else 
    {
      /*
       * No b-cast in IPv6, therefore the ifa_broadaddr (concidentally the
       * dest address filled in above...) should be set to NULL!
       */
      i6a->i6a_ifa.ifa_broadaddr = NULL;

      if (IS_IN6_LINKLOC(i6a->i6a_addr.sin6_addr))
	{
	  flags |= RTF_HOST;
	  i6a->i6a_ifa.ifa_dstaddr = i6a->i6a_ifa.ifa_addr;

	  /*
	   * Possibly do other stuff specific to link-local addresses, hence
	   * keeping this separate from IFF_LOOPBACK case above.  I may move
	   * IS_IN6_LINKLOC check to || with IFF_LOOPBACK.
	   */

	  i6a->i6a_addrflags = I6AF_LINKLOC | I6AF_PERMANENT;
	}
      else
	{
	  flags |= RTF_CLONING;  /* IMHO, ALL network routes
				    have the cloning bit set for next-hop
				    resolution if they aren't loopback or
				    pt. to pt. */
	  i6a->i6a_addrflags = I6AF_PREFIX;  /* I'm a 'prefix list entry'. */
	}
    }

  if ((error = rtinit(&(i6a->i6a_ifa), RTM_ADD,flags)) == 0)
    {
      i6a->i6a_flags |= IFA_ROUTE;
    }

  /*
   * If the interface supports multicast, join the appropriate
   * multicast groups (all {nodes, routers}) on that interface.
   *
   * Also join the solicited nodes discovery multicast group for that
   * destination.
   */
  if (ifp->if_flags & IFF_MULTICAST)
    {
      /*
       * NOTE:  Check if ip_forwarding is on, if so, then join all
       *        routers as well as all-nodes.
       */

      struct in_addr6 addr;
      struct in6_multi *rc,*in6_addmulti();
      
      /* NOTE2:  Set default multicast interface here.
                 Set up cloning route for ff00::0/8 */
      if (ifp->if_type != IFT_LOOP && mcastdefault == NULL)
	{
	  setmcastdef(ifp);

	  /*
	   * NOTE3:  Set up link-local masked route.
	   *         Address resolution code will handle things.
	   *
	   *         For link-local default, 
	   *            tend towards m-cast/b-cast i/fs.
	   *
	   *         If we have both ethernet and slip/ppp, 
	   *            we don't want slip/ppp to get link-locals.
	   */
	  if ((ifp->if_flags & (IFF_MULTICAST|IFF_BROADCAST)) || 
	      linklocdefault == NULL)
	    setlinklocdef(ifp);
	}

      /* All-nodes. */
      SET_IN6_ALLNODES(addr);
      SET_IN6_MCASTSCOPE(addr,IN6_INTRA_LINK);
      rc=in6_addmulti(&addr,ifp);

      /* Solicited-nodes. */
      IN6_ADDR_ASSIGN(addr,htonl(0xff020000),0,htonl(1),\
		      i6a->i6a_addr.sin6_addr.in6a_words[3]);
      rc=in6_addmulti(&addr, ifp);
    }

  return error;
}

/*----------------------------------------------------------------------
 * Add IPv6 multicast address.  IPv6 multicast addresses are handled
 * pretty much like IP multicast addresses for now.
 *
 * Multicast addresses hang off in6_ifaddr's.  Eventually, they should hang
 * off the link-local multicast address, this way, there are no ambiguities.
 ----------------------------------------------------------------------*/

struct in6_multi *in6_addmulti(addr,ifp)
     register struct in_addr6 *addr;
     struct ifnet *ifp;

{
  register struct in6_multi *in6m;
  struct inet6_ifreq ifr;
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&ifr.ifr_addr;
  struct in6_ifaddr *i6a;
  int s = splnet();

  /*
   * See if address is already in list..
   */

  IN6_LOOKUP_MULTI(addr,ifp,in6m);

  if (in6m != NULL)
    {
      /* Increment the reference count. */
      in6m->in6m_refcount++;
    }
  else
    {
      /*
       * Otherwise, allocate a new m-cast record and link it to
       * the interface's multicast list.
       */
      
      if ((in6m=malloc(sizeof(struct in6_multi),M_IPMADDR,M_NOWAIT)) == NULL)
	{
	  splx(s);
	  return NULL;
	}
      bzero(in6m,sizeof(struct in6_multi));
      IN6_ADDR_COPY(in6m->in6m_addr,*addr);
      in6m->in6m_refcount = 1;
      in6m->in6m_ifp = ifp;

      for(i6a=in6_ifaddr; i6a != NULL && i6a->i6a_ifp != ifp; i6a=i6a->i6a_next)
	;
      if (i6a == NULL)
	{
	  free(in6m,M_IPMADDR);
	  splx(s);
	  return NULL;
	}
      in6m->in6m_i6a = i6a;
      in6m->in6m_next = i6a->i6a_multiaddrs;
      i6a->i6a_multiaddrs = in6m;

      /*
       * Ask the network driver to update its multicast reception
       * filter appropriately for the new address.
       */
      sin6->sin6_family=AF_INET6;
      sin6->sin6_len=sizeof(struct sockaddr_in6);
      IN6_ADDR_COPY(sin6->sin6_addr,*addr);
      sin6->sin6_port = 0;
      sin6->sin6_flowinfo = 0;
      if (ifp->if_ioctl == NULL ||
	  (*ifp->if_ioctl)(ifp, SIOCADDMULTI,(caddr_t)&ifr) != 0)
	{
	  i6a->i6a_multiaddrs = in6m->in6m_next;
	  free(in6m,M_IPMADDR);
	  splx(s);
	  return NULL;
	}

      /* Tell IGMP that we've joined a new group. */
      /*ipv6_igmp_joingroup(in6m);*/
    }
  splx(s);
  return in6m;
}

/*----------------------------------------------------------------------
 * Delete IPv6 multicast address.
 ----------------------------------------------------------------------*/

int in6_delmulti(in6m)
     register struct in6_multi *in6m;
{
  register struct in6_multi **p;
  struct inet6_ifreq ifr;
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&(ifr.ifr_addr);
  int s = splnet();

  if (--in6m->in6m_refcount == 0)
    {
      /* Tell IGMP that I'm bailing this group. */
      /* ipv6_igmp_leavegroup(in6m);*/

      /* Unlink from list. */
      for (p = &(in6m->in6m_i6a->i6a_multiaddrs);
	   *p != in6m;
	   p = &(*p)->in6m_next)
	;
      *p = (*p)->in6m_next;

      /*
       * Notify the network driver to update its multicast reception
       * filter.
       */
      sin6->sin6_family = AF_INET6;
      sin6->sin6_len = sizeof(struct sockaddr_in6);
      sin6->sin6_port = 0;
      sin6->sin6_flowinfo = 0;
      IN6_ADDR_COPY(sin6->sin6_addr,in6m->in6m_addr);
      (*(in6m->in6m_ifp->if_ioctl))(in6m->in6m_ifp, SIOCDELMULTI,
					 (caddr_t)&ifr);

      free(in6m,M_IPMADDR);
    }
  splx(s);
}
