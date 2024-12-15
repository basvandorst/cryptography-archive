/*
 * in6_pcb.c  --  Protocol Control Block (session state) functions
 *                for IPv6.  Many of these are rehashes of IP PCB
 *                functions, with support for 16-byte addresses.
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

/*
 * Copyright (c) 1982, 1986, 1990, 1993
 *	Regents of the University of California.  All rights reserved.
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
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/proc.h>

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
#include <netinet6/in6_debug.h>
#include <netinet6/ipsec.h>

/*
 * External globals
 */

extern struct in6_ifaddr *in6_ifaddr;
extern struct in_ifaddr *in_ifaddr;

/*
 * Globals
 */

struct in_addr6 zeroin_addr6;

/*
 * Keep separate inet6ctlerrmap, because I may remap some of these.
 * I also put it here, because, quite frankly, it belongs here, not in
 * ip{v6,}_input().
 */
u_char inet6ctlerrmap[PRC_NCMDS] = {
        0,              0,              0,              0,
        0,              EMSGSIZE,       EHOSTDOWN,      EHOSTUNREACH,
        EHOSTUNREACH,   EHOSTUNREACH,   ECONNREFUSED,   ECONNREFUSED,
        EMSGSIZE,       EHOSTUNREACH,   0,              0,
        0,              0,              0,              0,
        ENOPROTOOPT
};

/*----------------------------------------------------------------------
 * Bind an address (or at least a port) to an PF_INET6 socket.
 ----------------------------------------------------------------------*/

int
in6_pcbbind(inp, nam)
     register struct inpcb *inp;
     struct mbuf *nam;
{
  register struct socket *so = inp->inp_socket;
  register struct inpcb *head = inp->inp_head;
  register struct sockaddr_in6 *sin6;
  struct proc *p = curproc;		/* XXX */
  u_short lport = 0;
  int wild = INPLOOKUP_IPV6, reuseport = (so->so_options & SO_REUSEPORT);
  int error;

  /*
   * REMINDER:  Once up to speed, flow label processing should go here,
   *            too.  (Same with in6_pcbconnect.)  
   */

  if (in6_ifaddr == 0 || in_ifaddr == 0)
    return EADDRNOTAVAIL;
  if (inp->inp_lport != 0 || !IS_IN6_UNSPEC(inp->inp_laddr6))
    return EINVAL;   /* If already bound, EINVAL! */

  if ((so->so_options & (SO_REUSEADDR | SO_REUSEPORT)) == 0 &&
      ((so->so_proto->pr_flags & PR_CONNREQUIRED) == 0 ||
       (so->so_options & SO_ACCEPTCONN) == 0))
    wild |= INPLOOKUP_WILDCARD;

  /*
   * If I did get a sockaddr passed in...
   */
  if (nam)
    {
      sin6 = mtod(nam, struct sockaddr_in6 *);
      if (nam->m_len != sizeof (*sin6))
	return EINVAL;

      /*
       * Unlike v4, I have no qualms about EAFNOSUPPORT if the
       * wretched family is not filled in!
       */
      if (sin6->sin6_family != AF_INET6)
	return EAFNOSUPPORT;
      lport = sin6->sin6_port;

      if (IS_IN6_MULTICAST(sin6->sin6_addr))
	{
	  /*
	   * Treat SO_REUSEADDR as SO_REUSEPORT for multicast;
	   * allow complete duplication of binding if
	   * SO_REUSEPORT is set, or if SO_REUSEADDR is set
	   * and a multicast address is bound on both
	   * new and duplicated sockets.
	   */
	  if (so->so_options & SO_REUSEADDR)
	    reuseport = SO_REUSEADDR | SO_REUSEPORT;
	}
      else if (!IS_IN6_UNSPEC(sin6->sin6_addr))
	{
	  struct sockaddr_in sin;

	  sin.sin_port = 0;
	  sin.sin_len = sizeof(sin);
	  sin.sin_family = AF_INET;
	  sin.sin_addr.s_addr = sin6->sin6_addr.in6a_words[3];
	  bzero(&sin.sin_zero,8);

	  sin6->sin6_port = 0;  /* Yechhhh, because of upcoming call to
				   ifa_ifwithaddr(), which does bcmp's
				   over the PORTS as well.  (What about flow?)
				   */
	  if (ifa_ifwithaddr((struct sockaddr *)sin6) == 0)
	    if (!IS_IN6_IPV4_MAPPED(sin6->sin6_addr) ||
		ifa_ifwithaddr((struct sockaddr *)&sin) == 0)
	      return EADDRNOTAVAIL;
	}
      if (lport)
	{
	  struct inpcb *t;
	  struct in_addr fa,la;
	  int wildto = wild;

	  /* Question:  Do we wish to continue the Berkeley tradition of
	     ports < IPPORT_RESERVED be only for root? 
	   
	     Answer:  For now yes, but IMHO, it should be REMOVED! 

	     OUCH:  One other thing, is there no better way of finding
	     a process for a socket instead of using curproc?  (Marked
	     with BSD's {in,}famous XXX ? */
	  if (ntohs(lport) < IPPORT_RESERVED &&
	      (error = suser(p->p_ucred, &p->p_acflag)))
	    return error;

	  if (IS_IN6_IPV4_MAPPED(sin6->sin6_addr))
	    {
	      fa.s_addr = 0;
	      la.s_addr = sin6->sin6_addr.in6a_words[3];
	      wildto &= ~INPLOOKUP_IPV6;
	    }
	  else
	    {
	      fa.s_addr = (u_long)&zeroin_addr6;
	      la.s_addr = (u_long)&sin6->sin6_addr;
	    }
	  t = in_pcblookup(head, fa, 0, la, lport, wildto);

	  if (t && (reuseport & t->inp_socket->so_options) == 0)
	    return EADDRINUSE;
	}
      inp->inp_laddr6 = sin6->sin6_addr;

      /*
       * Unroll first 2 compares of IS_IN6_{UNSPEC,IPV4_MAPPED}.
       * Mark PF_INET6 socket as undecided (bound to port-only) or
       * mapped (INET6 socket talking IPv4) here.  I may need to move
       * this code out of this if (nam) clause, and put it just before
       * function return.
       *
       * Then again, the only time this function is called with NULL nam
       * might be during a *_pcbconnect(), which then sets the local address
       * ANYWAY.
       */
      if (inp->inp_laddr6.in6a_words[0] == 0 && 
	  inp->inp_laddr6.in6a_words[1] == 0)
	{
	  if (inp->inp_laddr6.in6a_words[2] == ntohl(0xffff))
	    inp->inp_flags |= INP_IPV6_MAPPED;
	  if (inp->inp_laddr6.in6a_words[2] == 0 &&
	      inp->inp_laddr6.in6a_words[3] == 0)
	    inp->inp_flags |= INP_IPV6_UNDEC;
	}
    }

  /*
   * UMMM, worst case, couldn't this do loop go on forever?
   * (i.e. ALL ports are taken?!?)
   */

  if (lport == 0)
    {
      struct in_addr fa,la;
      int wildto = wild;
      /* This code fragment may get more complicated.
	 (This code fragment finds an available port.) */

      if (inp->inp_flags & INP_IPV6_MAPPED)
	{
	  la.s_addr = inp->inp_laddr6.in6a_words[3];
	  fa.s_addr = 0;
	  wildto &= ~INPLOOKUP_IPV6;
	}
      else
	{
	  fa.s_addr = (u_long)&zeroin_addr6;
	  la.s_addr = (u_long)&inp->inp_laddr6;
	}

      do
	{
	  if (head->inp_lport++ < IPPORT_RESERVED ||
	      head->inp_lport > IPPORT_USERRESERVED)
	    head->inp_lport = IPPORT_RESERVED;
	  lport = htons(head->inp_lport);
	}
      while (in_pcblookup(head, fa, 0, la, lport, wildto));
    }

  inp->inp_lport = lport;
  return 0;
}

/*----------------------------------------------------------------------
 * Connect from a socket to a specified address.
 * Both address and port must be specified in argument sin6.
 * Eventually, flow labels will have to be dealt with here, as well.
 *
 * If don't have a local address for this socket yet,
 * then pick one.
 *
 * I believe this has to be called at splnet().
 ----------------------------------------------------------------------*/

int
in6_pcbconnect(inp, nam)
	register struct inpcb *inp;
	struct mbuf *nam;
{
  struct in6_ifaddr *i6a;
  struct sockaddr_in6 *ifaddr;
  register struct sockaddr_in6 *sin6 = mtod(nam, struct sockaddr_in6 *);
  struct in_addr fa,la;  /* For fooling in_pcblookup, will break on 64-bit
			    boxes. */
  
  if (nam->m_len != sizeof (*sin6))
    return (EINVAL);
  if (sin6->sin6_family != AF_INET6)
    return (EAFNOSUPPORT);
  if (sin6->sin6_port == 0)
    return (EADDRNOTAVAIL);


  if (IS_IN6_IPV4_MAPPED(sin6->sin6_addr))
    {
      struct domain hackdomain, *save;
      struct mbuf hackmbuf;
      struct sockaddr_in *sin;
      int rc;

      /*
       * Hmmm, if this is a v4-mapped v6 address, re-call in_pcbconnect
       * with a fake domain for now.  Then re-adjust the socket, and
       * return out of here.  Since this is called at splnet(), I don't
       * think temporarily altering the socket will matter.  XXX
       */

      /*
       * Can't have v6 talking to v4, now can we!
       */
      if (!IS_IN6_UNSPEC(inp->inp_laddr6) && 
	  !IS_IN6_IPV4_MAPPED(inp->inp_laddr6))
	return EINVAL;  

      bzero(&hackdomain,sizeof(hackdomain));
      bzero(&hackmbuf,sizeof(hackmbuf));
      save = inp->inp_socket->so_proto->pr_domain;
      inp->inp_socket->so_proto->pr_domain = &hackdomain;
      hackdomain.dom_family = PF_INET;
      hackmbuf.m_hdr = nam->m_hdr;
      hackmbuf.m_len = sizeof(*sin);
      hackmbuf.m_data = hackmbuf.m_dat;
      sin = mtod(&hackmbuf,struct sockaddr_in *);
      sin->sin_len = sizeof(*sin);
      sin->sin_family = AF_INET;
      sin->sin_port = sin6->sin6_port;
      sin->sin_addr.s_addr = sin6->sin6_addr.in6a_words[3];

      rc = in_pcbconnect(inp,&hackmbuf);

      inp->inp_socket->so_proto->pr_domain = save;

      if (rc == 0)
	{
	  inp->inp_laddr6.in6a_words[2] = htonl(0xffff);
	  inp->inp_faddr6.in6a_words[2] = htonl(0xffff);
	  inp->inp_flags |= INP_IPV6_MAPPED;
	  inp->inp_flags &= ~INP_IPV6_UNDEC;
	}

      return rc;
    }

  if (in6_ifaddr && IS_IN6_UNSPEC(sin6->sin6_addr))
    {
      /*
       * If the destination address is all 0's,
       * use the first non-loopback (and if possibly, non-local,
       * else use the LAST local, non-loopback) address as the destination.
       */
#define	satosin6(sa)	((struct sockaddr_in6 *)(sa))
#define sin6tosa(sin6)	((struct sockaddr *)(sin6))
#define ifatoi6a(ifa)	((struct in6_ifaddr *)(ifa))
      struct in6_ifaddr *ti6a = NULL;

      for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
        {
          /* Find first (non-local if possible) address for
             source usage.  If multiple locals, use last one found. */
	  if (IS_IN6_LINKLOC(I6A_SIN(i6a)->sin6_addr))
	    ti6a=i6a;
	  else if (!IS_IN6_LOOPBACK(I6A_SIN(i6a)->sin6_addr))
	    break;
        }
      if (i6a == NULL && ti6a != NULL)
	i6a = ti6a;
    }

  if (IS_IN6_UNSPEC(inp->inp_laddr6))
    {
      register struct route6 *ro;

      i6a = NULL;
      /* 
       * If route is known or can be allocated now,
       * our src addr is taken from the rt_ifa, else punt.
       */
      ro = &inp->inp_route6;
      if (ro->ro_rt &&
	  (ro->ro_dst.sin6_family != sin6->sin6_family ||	
	   rt_key(ro->ro_rt)->sa_family != sin6->sin6_family ||
	   !IN6_ADDR_EQUAL(ro->ro_dst.sin6_addr,sin6->sin6_addr) ||
	   inp->inp_socket->so_options & SO_DONTROUTE))
	{
	  RTFREE(ro->ro_rt);
	  ro->ro_rt = NULL;
	}
      if ((inp->inp_socket->so_options & SO_DONTROUTE) == 0 && /*XXX*/
	  (ro->ro_rt == NULL || ro->ro_rt->rt_ifp == NULL))
	{
	  /* No route yet, try and acquire one. */
	  ro->ro_dst.sin6_family = AF_INET6;  /* Is ro blanked out? */
	  ro->ro_dst.sin6_len = sizeof(struct sockaddr_in6);
	  ro->ro_dst.sin6_addr = sin6->sin6_addr;
	  /* 
	   *  Need to wipe out the flowlabel for ifa_ifwith*
	   *  but don't need to for rtalloc.
	   */
	  rtalloc((struct route *)ro);
	}

      /*
       * If we found a route, use the address
       * corresponding to the outgoing interface
       * unless it is the loopback (in case a route
       * to our address on another net goes to loopback).
       */
      if (ro->ro_rt && ro->ro_rt->rt_ifp &&
		!(ro->ro_rt->rt_ifp->if_flags & IFF_LOOPBACK))
	i6a = ifatoi6a(ro->ro_rt->rt_ifa);
      if (i6a == NULL)
	{
	  u_short fport = sin6->sin6_port;

	  sin6->sin6_port = 0;
	  i6a = ifatoi6a(ifa_ifwithdstaddr(sin6tosa(sin6)));
	  if (i6a == NULL)
	    i6a = ifatoi6a(ifa_ifwithnet(sin6tosa(sin6)));
	  sin6->sin6_port = fport;
	  if (i6a == NULL)
	    {
	      struct in6_ifaddr *ti6a = NULL;
	      
	      for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
		{
		  /* Find first (non-local if possible) address for
		     source usage.  If multiple locals, use last one found. */
		  if (IS_IN6_LINKLOC(I6A_SIN(i6a)->sin6_addr))
		    ti6a=i6a;
		  else if (!IS_IN6_LOOPBACK(I6A_SIN(i6a)->sin6_addr))
		    break;
		}
	      if (i6a == NULL && ti6a != NULL)
		i6a = ti6a;
	    }
	  if (i6a == NULL)
	    return EADDRNOTAVAIL;
	}

      if (IS_IN6_MULTICAST(sin6->sin6_addr) && inp->inp_moptions != NULL)
	{
	  struct ipv6_moptions *i6mo;
	  struct ifnet *ifp;

	  i6mo = inp->inp_moptions6;
	  if (i6mo->i6mo_multicast_ifp != NULL)
	    {
	      ifp = i6mo->i6mo_multicast_ifp;
	      for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
		if (i6a->i6a_ifp == ifp)
		  break;
	      if (i6a == NULL)
		return EADDRNOTAVAIL;
	    }
	}
      ifaddr = (struct sockaddr_in6 *)&i6a->i6a_addr;
    }

  la.s_addr = (u_long)&sin6->sin6_addr;
  fa.s_addr = (u_long)((IS_IN6_UNSPEC(inp->inp_laddr6)) ? &ifaddr->sin6_addr :
                        &inp->inp_laddr6);
  if (in_pcblookup(inp->inp_head,fa,sin6->sin6_port,la,inp->inp_lport,
		    INPLOOKUP_IPV6))
    return EADDRINUSE;
  if (IS_IN6_UNSPEC(inp->inp_laddr6))
    {
      if (inp->inp_lport == 0)
	(void)in6_pcbbind(inp, NULL);  /* To find free port & bind to it. */
      inp->inp_laddr6 = ifaddr->sin6_addr;
    }
  inp->inp_faddr6 = sin6->sin6_addr;
  inp->inp_fport = sin6->sin6_port;
  return 0;
}

/*----------------------------------------------------------------------
 * Pass some notification to all connections of a protocol
 * associated with address dst.  The local address and/or port numbers
 * may be specified to limit the search.  The "usual action" will be
 * taken, depending on the ctlinput cmd.  The caller must filter any
 * cmds that are uninteresting (e.g., no error in the map).
 * Call the protocol specific routine (if any) to report
 * any errors for each matching socket.
 *
 * Also perform input-side security policy check 
 *    once PCB to be notified has been located.
 *
 * Must be called at splnet.
 ----------------------------------------------------------------------*/

int
in6_pcbnotify(head, dst, fport_arg, la, lport_arg, cmd, notify, m, nexthdr)
     struct inpcb *head;
     struct sockaddr *dst;
     u_int fport_arg, lport_arg;
     struct in_addr6 *la;
     int cmd;
     void (*notify) __P((struct inpcb *, int));
     struct mbuf *m;
     int nexthdr;
{
  register struct inpcb *inp, *oinp;
  struct in_addr6 *faddr,laddr = *la;
  u_short fport = fport_arg, lport = lport_arg;
  int errno;

  DPRINTF(IDL_EVENT,("Entering in6_pcbnotify.  head = 0x%x, dst is\n",head));
  DDO(IDL_EVENT,dump_smart_sockaddr(dst));
  DPRINTF(IDL_EVENT,("fport_arg = %d, lport_arg = %d, cmd = %d\n",\
			   fport_arg, lport_arg, cmd));
  DDO(IDL_EVENT,printf("la is ");dump_in_addr6(la));

  if ((unsigned)cmd > PRC_NCMDS || dst->sa_family != AF_INET6)
    return;
  faddr = &(((struct sockaddr_in6 *)dst)->sin6_addr);
  if (IS_IN6_UNSPEC(*faddr))
    return;
  if (IS_IN6_IPV4_MAPPED(*faddr))
    {
      printf("Huh?  Thought in6_pcbnotify() never got called with mapped!\n");
    }
  
  /*
   * Redirects go to all references to the destination,
   * and use in_rtchange to invalidate the route cache.
   * Dead host indications: notify all references to the destination.
   * Otherwise, if we have knowledge of the local port and address,
   * deliver only to that socket.
   */

  if (PRC_IS_REDIRECT(cmd) || cmd == PRC_HOSTDEAD)
    {
      fport = 0;
      lport = 0;
      IN6_ADDR_ASSIGN(laddr,0,0,0,0);
      if (cmd != PRC_HOSTDEAD)
	notify = in_rtchange;
    }
  errno = inet6ctlerrmap[cmd];
  for (inp = head->inp_next; inp != head;)
    {
      if (!IN6_ADDR_EQUAL(inp->inp_faddr6,*faddr) ||
	  inp->inp_socket == 0 ||
	  (lport && inp->inp_lport != lport) ||
	  (!IS_IN6_UNSPEC(laddr) && !IN6_ADDR_EQUAL(inp->inp_laddr6,laddr)) ||
	  (fport && inp->inp_fport != fport))
	{
	  inp = inp->inp_next;
	  continue;
	}
      oinp = inp;
      inp = inp->inp_next;
      if (notify)
	if (!ipsec_input_policy(m, oinp->inp_socket, nexthdr))
	  (*notify)(oinp, errno);
    }
}

/*----------------------------------------------------------------------
 * Get the local address/port, and put it in a sockaddr_in6.
 * This services the getsockname(2) call.
 ----------------------------------------------------------------------*/

int
in6_setsockaddr(inp, nam)
     register struct inpcb *inp;
     struct mbuf *nam;
{
  register struct sockaddr_in6 *sin6;

  nam->m_len = sizeof(struct sockaddr_in6);
  sin6 = mtod(nam,struct sockaddr_in6 *);
  bzero ((caddr_t)sin6,sizeof(struct sockaddr_in6));
  sin6->sin6_family = AF_INET6;
  sin6->sin6_len = sizeof(struct sockaddr_in6);
  sin6->sin6_port = inp->inp_lport;
  sin6->sin6_addr = inp->inp_laddr6;

  return 0;
}

/*----------------------------------------------------------------------
 * Get the foreign address/port, and put it in a sockaddr_in6.
 * This services the getpeername(2) call.
 ----------------------------------------------------------------------*/

int
in6_setpeeraddr(inp, nam)
     register struct inpcb *inp;
     struct mbuf *nam;
{
  register struct sockaddr_in6 *sin6;

  nam->m_len = sizeof(struct sockaddr_in6);
  sin6 = mtod(nam,struct sockaddr_in6 *);
  bzero ((caddr_t)sin6,sizeof(struct sockaddr_in6));
  sin6->sin6_family = AF_INET6;
  sin6->sin6_len = sizeof(struct sockaddr_in6);
  sin6->sin6_port = inp->inp_fport;
  sin6->sin6_addr = inp->inp_faddr6;

  return 0;
}
