/*
 * in_pcb.h  --  Protocol Control Block (PCB) definitions.  Bloated for
 *               use with IPv6 addresses.
 *
 * Modifications to support IPv6 are copyright 1995 
 * by Dan McDonald, Bao Phan, and Randall Atkinson, All Rights Reserved.  
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
 * Copyright (c) 1982, 1986, 1990, 1993
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
 *	@(#)in_pcb.h	8.1 (Berkeley) 6/10/93
 */

/*
 * Common structure pcb for internet protocol implementation.
 * Here are stored pointers to local and foreign host table
 * entries, local and foreign socket numbers, and pointers
 * up (to a socket structure) and down (to a protocol-specific)
 * control block.
 */
#ifdef INET6

/*
 * If INET6 is being used, anything that uses in_pcb.h will also have to
 * include: 
 *
 *               netinet6/in6.h
 *               netinet6/ipv6.h
 *               netinet6/ipv6_var.h
 *
 * These files are wrapped in ifdefs.  Perhaps putting in6.h in with in.h, and
 * correspondingly ipv6.h with ip.h, etc. may be better.
 */

#include <netinet6/in6.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>

union inpaddru
{
  struct in_addr6 iau_addr6;
  struct
    {
      u_int8 pad[12];
      struct in_addr inaddr;           /* I put the in_addr here because this
					  makes transition easier. */
    } iau_a4u;
};

struct inpcb
{
  struct inpcb *inp_next, *inp_prev;    /* To others in 2-way linked list. */
  struct inpcb *inp_head;               /* Back pointer to list head. */
  struct socket *inp_socket;            /* Back pointer to socket. */

  union inpaddru inp_faddru;            /* Foreign address. */
  union inpaddru inp_laddru;            /* Local address. */

#define inp_faddr inp_faddru.iau_a4u.inaddr
#define inp_faddr6 inp_faddru.iau_addr6
#define inp_laddr inp_laddru.iau_a4u.inaddr
#define inp_laddr6 inp_laddru.iau_addr6

  caddr_t inp_ppcb;			/* Pointer to per-protocol pcb */

  union                                 /* Route (notice increased size). */
    {
      struct route ru_route;
      struct route6 ru_route6;
    }inp_ru;
#define inp_route inp_ru.ru_route
#define inp_route6 inp_ru.ru_route6

  union                                 /* Header prototype. */
    {
      struct ip hu_ip;
      struct ipv6 hu_ipv6;
    } inp_hu;
#define inp_ip inp_hu.hu_ip
#define inp_ipv6 inp_hu.hu_ipv6

  struct mbuf *inp_options;             /* IP/IPv6 options mbuf chain. */
  union
    {
      struct ip_moptions *mou_mo;       /* IP multicast options. */
      struct ipv6_moptions *mou_mo6;    /* IPv6 multicast options. */
    } inp_mou;
#define inp_moptions inp_mou.mou_mo
#define inp_moptions6 inp_mou.mou_mo6
  int inp_flags;			/* PCB flags. */
  u_short inp_fport;                    /* Foreign port. */
  u_short inp_lport;                    /* Local port. */
};

#else /* Use original inpcb structure. */

struct inpcb {
	struct	inpcb *inp_next,*inp_prev;
					/* pointers to other pcb's */
	struct	inpcb *inp_head;	/* pointer back to chain of inpcb's
					   for this protocol */
	struct	in_addr inp_faddr;	/* foreign host table entry */
	u_short	inp_fport;		/* foreign port */
	struct	in_addr inp_laddr;	/* local host table entry */
	u_short	inp_lport;		/* local port */
	struct	socket *inp_socket;	/* back pointer to socket */
	caddr_t	inp_ppcb;		/* pointer to per-protocol pcb */
	struct	route inp_route;	/* placeholder for routing entry */
	int	inp_flags;		/* generic IP/datagram flags */
	struct	ip inp_ip;		/* header prototype; should have more */
	struct	mbuf *inp_options;	/* IP options */
	struct	ip_moptions *inp_moptions; /* IP multicast options */
};
#endif /* INET6 */

/* flags in inp_flags: */
#define	INP_RECVOPTS		0x01	/* receive incoming IP options */
#define	INP_RECVRETOPTS		0x02	/* receive IP options for reply */
#define	INP_RECVDSTADDR		0x04	/* receive IP dst address */
#define	INP_CONTROLOPTS		(INP_RECVOPTS|INP_RECVRETOPTS|INP_RECVDSTADDR)
#define	INP_HDRINCL		0x08	/* user supplies entire IP header */

#ifdef INET6
/*
 * Both of these flags' values should be determined by either the transport
 * protocol at PRU_BIND, PRU_LISTEN, PRU_CONNECT, etc, or by in_pcb*().
 */
#define INP_IPV6                0x10    /* This flag is set iff
					   sotopf(inp->inp_socket) == PF_INET6.
					   */
#define INP_IPV6_UNDEC          0x20    /* PCB is an PF_INET6 PCB, but
					   listening for both v4 and v6. */
#define INP_IPV6_MAPPED         0x40    /* PF_INET6 PCB which is connected to
					   an IPv4 host, or is bound to
					   and IPv4 address (specified with
					   the mapped form of v6 addresses). */
#endif /* INET6 */

#define	INPLOOKUP_WILDCARD	1
#define	INPLOOKUP_SETLOCAL	2

#ifdef INET6
#define INPLOOKUP_IPV6          4       /* Address arguments are actually
					   pointers to IPv6 addrs.  
					   This'll break on 64-bit machines. */
#endif /* INET6 */

#define	sotoinpcb(so)	((struct inpcb *)(so)->so_pcb)

#ifdef KERNEL
#ifdef INET6
/*
 * Some of these can be easily modified to work for both v4 and v6.  Others
 * (like in_pcblookup) will have to be modified most heinously.
 */

/* Macros to determine protocol family of socket. */
#define sotopf(so)  (so->so_proto->pr_domain->dom_family)
#endif /* INET6 */

int	 in_losing __P((struct inpcb *));
int	 in_pcballoc __P((struct socket *, struct inpcb *));
int	 in_pcbbind __P((struct inpcb *, struct mbuf *));
int	 in_pcbconnect __P((struct inpcb *, struct mbuf *));
int	 in_pcbdetach __P((struct inpcb *));
int	 in_pcbdisconnect __P((struct inpcb *));
struct inpcb *
	 in_pcblookup __P((struct inpcb *,
	    struct in_addr, u_int, struct in_addr, u_int, int));

/*
 * In order to do policy checking, pcbnotify has new args.
 */
#ifdef INET6
int	 in_pcbnotify __P((struct inpcb *, struct sockaddr *,
	    u_int, struct in_addr, u_int, int, void (*)(struct inpcb *, int),
		struct mbuf *, int));
#else /* INET6 */
int	 in_pcbnotify __P((struct inpcb *, struct sockaddr *,
	    u_int, struct in_addr, u_int, int, void (*)(struct inpcb *, int)));
#endif /* INET6 */

void	 in_rtchange __P((struct inpcb *, int));
int	 in_setpeeraddr __P((struct inpcb *, struct mbuf *));
int	 in_setsockaddr __P((struct inpcb *, struct mbuf *));
#endif
