/*
 * raw_ipv6.h  --  Raw IPv6 socket routines.  Like IPv4, we will not be
 *                 using the generic raw_cb routines in net/.
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
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/ip_mroute.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>

#include <netinet6/in6_debug.h>

/*
 * Globals
 */

struct inpcb rawin6pcb;
struct sockaddr_in6 ripv6src = { sizeof(struct sockaddr_in6), AF_INET6 };

/*
 * Nominal space allocated to a raw ip socket.
 */

#define	RIPV6SNDQ		8192
#define	RIPV6RCVQ		8192
u_long ripv6_sendspace = RIPV6SNDQ;
u_long ripv6_recvspace = RIPV6RCVQ;

/*
 * External globals
 */

extern struct ipv6stat ipv6stat;

/*----------------------------------------------------------------------
 * Raw IPv6 PCB initialization.
 ----------------------------------------------------------------------*/

void
ripv6_init()
{
  rawin6pcb.inp_next = rawin6pcb.inp_prev = &rawin6pcb;
}

/*----------------------------------------------------------------------
 * If no HLP's are found for an IPv6 datagram, this routine is called.
 ----------------------------------------------------------------------*/

void
ripv6_input(m,extra,ihi,ihioff)
     struct mbuf *m;
     int extra;
     struct in6_hdrindex *ihi;
     int ihioff;
{
  register struct ipv6 *ipv6 = mtod(m, struct ipv6 *); /* Will have been
							  pulled up by 
							  ipv6_input(). */
  register struct inpcb *inp;
  struct socket *last = 0;

  DDO(IDL_GROSS_EVENT,printf("In ripv6_input(), header is:\n");dump_mchain(m));
  DPRINTF(IDL_MAJOR_EVENT, ("In ripv6_input()\n"));
  DPRINTF(IDL_MAJOR_EVENT, ("Header is: "));
  DDO(IDL_EVENT, dump_ipv6(ipv6));

  if (!ihi) {
    DPRINTF(IDL_CRITICAL, ("NULL header index -- programming error!"));
    m_freem(m);
    return;
  }

  ripv6src.sin6_addr = ipv6->ipv6_src;  /* For call to sbappendaddr() */

  /*
   * Locate raw PCB for incoming datagram.
   */
  for (inp = rawin6pcb.inp_next; inp != &rawin6pcb; inp = inp->inp_next)
    {
      if (inp->inp_ipv6.ipv6_nexthdr != ihi[ihioff].ihi_nexthdr)
	continue;
      if (!IS_IN6_UNSPEC(inp->inp_laddr6)  &&
	  !IN6_ADDR_EQUAL(inp->inp_laddr6,ipv6->ipv6_dst))
	continue;
      if (!IS_IN6_UNSPEC(inp->inp_faddr6)  &&
	  !IN6_ADDR_EQUAL(inp->inp_faddr6,ipv6->ipv6_src))
	continue;
      if (last)
	{
	  struct mbuf *n;

	  /* 
	   * We use m_copym() because we don't care about overwriting
	   * clusters.
	   */
	  if (n = m_copym(m,0,(int)M_COPYALL,M_DONTWAIT)) {
            DPRINTF(IDL_MAJOR_EVENT, ("Found a raw pcb (>1)\n"));
	    if (sbappendaddr(&last->so_rcv, &ripv6src, n, (struct mbuf *)0) 
		== 0)
	      m_freem(n);
	    else sorwakeup(last);
          }
	}
      last = inp->inp_socket;
    }

  if (last){
    DPRINTF(IDL_MAJOR_EVENT, ("Found a raw pcb\n"));
    if (sbappendaddr(&last->so_rcv, &ripv6src, m, (struct mbuf *)0) == 0)
      m_freem(m);
    else
      {
	sorwakeup(last);
      }
  } else
    {
      m_freem(m);
      /*
       * Perhaps should send an ICMPv6 protocol unreachable here,
       * though original UCB 4.4-lite BSD's IPv4 does not do so.
       */
      ipv6stat.ips_noproto++;
      ipv6stat.ips_delivered--;
    }
}

/*----------------------------------------------------------------------
 * Output function for raw IPv6.  Called from ripv6_usrreq(), and
 * ipv6_icmp_usrreq().
 ----------------------------------------------------------------------*/

int
ripv6_output(m, so, dst)
     struct mbuf *m;
     struct socket *so;
     struct in_addr6 *dst;
{
  register struct ipv6 *ipv6;
  register struct inpcb *inp = sotoinpcb(so);
  int flags = (so->so_options & SO_DONTROUTE);

  if (inp->inp_flags & INP_HDRINCL)
    {
      flags |= IPV6_RAWOUTPUT;
      ipv6stat.ips_rawout++;
      /* Maybe m_pullup() ipv6 header here for ipv6_output(), which
	 expects it to be contiguous. */
    }
  else
    {
      M_PREPEND(m, sizeof(struct ipv6), M_WAIT);
      ipv6 = mtod(m, struct ipv6 *);
      ipv6->ipv6_nexthdr = inp->inp_ipv6.ipv6_nexthdr;
      ipv6->ipv6_hoplimit = MAXHOPLIMIT;
      ipv6->ipv6_src = inp->inp_laddr6;
      ipv6->ipv6_dst = *dst;
      ipv6->ipv6_versfl = 0;  /* Or possibly user flow label, in host order. */
      /*
       * Question:  How do I handle options?
       *
       * Answer:  I put them in here, but how?
       */
    }
  (struct socket *)m->m_pkthdr.rcvif = so;
  return ipv6_output(m,&inp->inp_route6,flags,inp->inp_moptions6);
}

/*----------------------------------------------------------------------
 * Handles [gs]etsockopt() calls.
 ----------------------------------------------------------------------*/

int
ripv6_ctloutput (op, so, level, optname, m)
     int op;
     struct socket *so;
     int level, optname;
     struct mbuf **m;
{
  register struct inpcb *inp = sotoinpcb(so);
  register int error;

  if (level != IPPROTO_IP)  /* For [gs]etsockopt(), I assume the same
			       protocol level ID's as IPv4. */
    {
      if (op == PRCO_SETOPT && *m)
	(void)m_free(*m);
      return EINVAL;
    }

  switch (optname)
    {
    case IP_HDRINCL:
      if (op == PRCO_SETOPT || op == PRCO_GETOPT)
	{
	  if (m == 0 || *m == 0 || (*m)->m_len < sizeof(int))
	    return EINVAL;
	  if (op == PRCO_SETOPT)
	    {
	      if (*mtod(*m, int *))
		inp->inp_flags |= INP_HDRINCL;
	      else inp->inp_flags &= ~INP_HDRINCL;
	      m_free(*m);
	    }
	  else
	    {
	      (*m)->m_len = sizeof(int);
	      *(mtod(*m, int *)) = inp->inp_flags & INP_HDRINCL;
	    }
	  return 0;
	}
      break;
    case DVMRP_INIT:
    case DVMRP_DONE:
    case DVMRP_ADD_VIF:
    case DVMRP_DEL_VIF:
    case DVMRP_ADD_LGRP:
    case DVMRP_DEL_LGRP:
    case DVMRP_ADD_MRT:
    case DVMRP_DEL_MRT:
#ifdef MROUTING

/* Be careful here! */
      if (op == PRCO_SETOPT)
	{
	  error = ipv6_mrouter_cmd(optname, so, *m);
	  if (*m)
	    (void)m_free(*m);
	}
      else error = EINVAL;
      return (error);
#else
      if (op == PRCO_SETOPT && *m)
	(void)m_free(*m);
      return (EOPNOTSUPP);
#endif
    }
  return ipv6_ctloutput(op, so, level, optname, m);
}

/*----------------------------------------------------------------------
 * Handles PRU_* for raw IPv6 sockets.
 ----------------------------------------------------------------------*/

int
ripv6_usrreq(so, req, m, nam, control)
     struct socket *so;
     int req;
     struct mbuf *m, *nam, *control;
{
  register int error = 0;
  register struct inpcb *inp = sotoinpcb(so);

#ifdef MROUTING
  /*
   * Ummm, like, multicast routing stuff goes here, huh huh huh.
   *
   * Seriously, this would be for user-level multicast routing daemons.  With
   * multicast being a requirement for IPv6, code like what might go here
   * may go away.
   */
#endif

  switch (req)
    {
    case PRU_ATTACH:
      if (inp)
	panic("ripv6_attach - Already got PCB");
      if ((so->so_state & SS_PRIV) == 0)
	{
	  error = EACCES;
	  break;
	}
      if ((error = soreserve(so, ripv6_sendspace, ripv6_recvspace)) ||
	  (error = in_pcballoc(so, &rawin6pcb)))
	break;
      inp = sotoinpcb(so);
      inp->inp_ipv6.ipv6_nexthdr = (int)nam;  /* Nam contains protocol
						 type, apparently. */
      break;
    case PRU_DISCONNECT:
      if ((so->so_state & SS_ISCONNECTED) == 0)
	{
	  error = ENOTCONN;
	  break;
	}
      /* FALLTHROUGH */
    case PRU_ABORT:
      soisdisconnected(so);
      /* FALLTHROUGH */
    case PRU_DETACH:
      if (inp == 0)
	panic("ripv6_detach");
#ifdef MROUTING
      /* More MROUTING stuff. */
#endif
      in_pcbdetach(inp);
      break;
    case PRU_BIND:
      {
	struct sockaddr_in6 *addr = mtod(nam, struct sockaddr_in6 *);

	/*
	 * Be strict regarding sockaddr_in6 fields.
	 */
	if (nam->m_len != sizeof(struct sockaddr_in6))
	  {
	    error = EINVAL;
	    break;
	  }
	/* 'ifnet' is declared in one of the net/ header files. */
	if ((ifnet == 0) ||
	    (addr->sin6_family != AF_INET6) ||    /* I only allow AF_INET6 */
	    (!IS_IN6_UNSPEC(addr->sin6_addr) &&
	     ifa_ifwithaddr((struct sockaddr *)addr) == 0 ) )
	  {
	    error = EADDRNOTAVAIL;
	    break;
	  }
	inp->inp_laddr6 = addr->sin6_addr;
	break;
      }
    case PRU_CONNECT:
      {
	struct sockaddr_in6 *addr = mtod(nam, struct sockaddr_in6 *);

	/*
	 * Be strict regarding sockaddr_in6 fields.
	 */
	if (nam->m_len != sizeof(struct sockaddr_in6))
	  {
	    error = EINVAL;
	    break;
	  }
	if (addr->sin6_family != AF_INET6)
	  {
	    error = EAFNOSUPPORT;
	    break;
	  }

	if (ifnet == 0)
	  {
	    error = EADDRNOTAVAIL;  /* This is a weird way to say there
				       are no interfaces, no? */
	    break;
	  }
	inp->inp_faddr6 = addr->sin6_addr;  /* Will structure assignment
					       work with this compiler? */
	soisconnected(so);
	break;
      }
    case PRU_SHUTDOWN:
      socantsendmore(so);
      break;
    case PRU_SEND:
      {
	struct in_addr6 *dst;

	/*
	 * Check "connected" status, and if there is a supplied destination
	 * address.
	 */
	if (so->so_state & SS_ISCONNECTED)
	  {
	    if (nam)
	      {
		error = EISCONN;
		break;
	      }
	    dst = &(inp->inp_faddr6);
	  }
	else
	  {
	    if (nam == NULL)
	      {
		error = ENOTCONN;
		break;
	      }
	    dst = &(mtod(nam, struct sockaddr_in6 *)->sin6_addr);
	  }

	error = ripv6_output(m,so,dst);
	m = NULL;
	break;
      }
    case PRU_SENSE:
      /* services stat(2) call. */
      return 0;
    case PRU_CONNECT2:
    case PRU_RCVOOB:
    case PRU_LISTEN:
    case PRU_SENDOOB:
    case PRU_RCVD:
    case PRU_ACCEPT:
      error = EOPNOTSUPP;
      break;
    case PRU_SOCKADDR:
      in6_setsockaddr(inp, nam);
      break;
    case PRU_PEERADDR:
      in6_setpeeraddr(inp, nam);
      break;
    default:
      panic ("ripv6_usrreq - unknown req\n");
    }
  if (m != NULL)
    m_freem(m);
  return error;
}
