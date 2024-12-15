/*
 * The author of this software is John Ioannidis, ji@cs.columbia.edu.
 * Copyright (C) 1993, by John Ioannidis.
 *	
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTY. IN PARTICULAR, THE AUTHOR DOES MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE
 * MERCHANTABILITY OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR
 * PURPOSE.
 */

/*
 * swIPe virtual interface module
 */

#ifdef DEBUGSW
#include "debug.h"
unsigned long debugsw = 0xffffffff;
#endif DEBUGSW

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/domain.h>
#include <sys/protosw.h>

#include <net/if.h>
#include <net/af.h>
#include <net/netisr.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>

#include <netinet/ip_swipe.h>
#include <netinet/if_sw.h>

#ifdef NSW

sw_softc_t sw_softc[NSW];
static struct route swroute;
int sw_inited = 0;
struct ifnet *ifsw = &(sw_softc[0].sw_if);

extern struct sockaddr_in wildcard; /* zero valued cookie for w/c searches */

int swoutput(), swifioctl();

int
swopen(dev, flag)
int dev, flag;
{
        return 0;
}

int
swclose(dev, flag)
int dev, flag;
{
	return 0;
}

/*
 * This makes it possible to load a new protocol without messing
 * with the protocol switch.
 */

int (*swsavedrip_input)();
int swinput(m, ifp)
register struct mbuf *m;
struct ifnet *ifp;
{
	struct ip *ip = mtod(m, struct ip *);
	if (ip->ip_p == IPPROTO_IPIP)
	  return swipe_input(m, ifp);
	else
	  return (*swsavedrip_input)(m, ifp);
}
	
extern struct domain inetdomain;

swinit(dev)
int dev;
{
	
	int status;
	struct protosw *dp;
	
        dp = pffindproto(PF_INET, IPPROTO_RAW, SOCK_RAW);
        if (dp == 0)
	  return ENOSPC;
	swsavedrip_input = dp->pr_input;
	dp->pr_input = swinput;
	return swattach();
}

/*
 * Attach routine. Called from the modload driver.
 */

int
swattach()
{
	register int i;
	register struct ifnet *ifp;
	
	if (sw_inited)
	  return EBUSY;

	sw_inited = 1;
	
	for (i=0; i<NSW; i++)
	{
		ifp = &sw_softc[i].sw_if;
		ifp->if_name = "sw";
		ifp->if_unit = i;
		ifp->if_mtu = SWMTU;
		ifp->if_flags = IFF_NOARP | IFF_POINTOPOINT;
		ifp->if_ioctl = swifioctl;
		ifp->if_output = swoutput;
		if_attach(ifp);
	}
#ifdef DEBUGSW
	printf("keysnd  = 0x%x\n", keysnd);
	printf("keyrcv  = 0x%x\n", keyrcv);
	printf("ipolicy  = 0x%x\n", ipolicy);
	printf("opolicy  = 0x%x\n", opolicy);
#endif

	return 0;
}

/*
 * Process an ioctl request.
 * Nothing exciting happens here -- this is boilerplate.
 */

swifioctl(ifp, cmd, data)
register struct ifnet *ifp;
int cmd;
caddr_t data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s, error = 0;

	s = splimp();

	switch (cmd) 
	{
	      case SIOCSIFADDR:
		if (ifa->ifa_addr.sa_family == AF_INET)
		  ifp->if_flags |= IFF_UP;
		else
		  error = EAFNOSUPPORT;
		break;
		
	      case SIOCSIFDSTADDR:
		if (ifa->ifa_addr.sa_family != AF_INET)
		  error = EAFNOSUPPORT;
		break;
		
	      default:
		error = EINVAL;
	}
	splx(s);
	return (error);
}

/*
 * swoutput() routine.
 *
 * This is the main driver of the output part of swIPe.
 *
 * We get called when the routing logic has decided that a packet
 * should be routed through the swIPe virtual interface.
 *
 * After some sanity checks, we test whether the packet is for one
 * of our own addresses, and if so, loop it back.
 *
 * Then, we consult the output policy engine (in reality, the kernel
 * cache of the in-use policies). If no such policy exists, then we 
 * should not have received the packet, and declare that the host is
 * unreachable. 
 *
 * Otherwise, we obtain the appropriate output policy entry, which will
 * give tell us what kind of security functions to apply to the packet,
 * and how to encapsulate it.
 * 
 * After the encapsulation, we figure out what the route 
 * for the packet should be, ignoring routing entries that contain
 * our interface.
 *
 * Finally, we pass the packet, the policy information, and the routing
 * information, on to swipe_output().
 */

swoutput(ifp, m0, dst)
	struct ifnet *ifp;
	register struct mbuf *m0;
	struct sockaddr *dst;
{
	int s;
	int error;
	register struct ifqueue *ifq;
	struct ip *iip;
	struct mbuf *m, *m1;
	struct sockaddr_in *din;
	struct sw_opol *opol;
	struct swkey_ent *se;
	register struct route *ro = &swroute;
	
	/*
	 * Sanity checks.
	 */

	if (dst->sa_family != AF_INET)
	{
		printf("%s%d: can't handle af%d\n", 
		       ifp->if_name, ifp->if_unit,
		       dst->sa_family);
		m_freem(m0);
		return (EAFNOSUPPORT);
	}

	din = (struct sockaddr_in *)dst;

	/*
	 * Check for loopback. This is vanilla BSD code.
	 */
	
	if (din->sin_addr.s_addr == IA_SIN(ifp->if_addrlist)->sin_addr.s_addr)
	{
		/*
		 * Place interface pointer before the data
		 * for the receiving protocol.
		 */
		if (m0->m_off <= MMAXOFF &&
		    m0->m_off >= MMINOFF + sizeof(struct ifnet *))
		{
			m0->m_off -= sizeof(struct ifnet *);
			m0->m_len += sizeof(struct ifnet *);
		} 
		else
		{
			MGET(m, M_DONTWAIT, MT_HEADER);
			if (m == (struct mbuf *)0)
			  return ENOBUFS;
			m->m_off = MMINOFF;
			m->m_len = sizeof(struct ifnet *);
			m->m_next = m0;
			m0 = m;
		}
		*(mtod(m0, struct ifnet **)) = ifp;
		s = splimp();
		ifp->if_opackets++;
		ifq = &ipintrq;
		if (IF_QFULL(ifq)) 
		{
			IF_DROP(ifq);
			m_freem(m0);
			splx(s);
			return ENOBUFS;
		}
		IF_ENQUEUE(ifq, m0);
		schednetisr(NETISR_IP);
		ifp->if_ipackets++;
		splx(s);
		return 0;
	}

	/*
	 * Packet must be sent.
	 * iip is the pointer to the  "internal" packet.
	 */

#ifdef DEBUGSW
	if (debugsw & DSW_SENDINGTO)
	  printf("swoutput: sending to 0x%x\n", 
		 htonl(IA_SIN(ifp->if_addrlist)->sin_addr.s_addr));
#endif DEBUGSW

	iip = mtod(m0, struct ip *);

	/*
	 * Consult the output policy engine. If no policy, fail.
	 */

	opol = swipe_getopol(iip->ip_src, iip->ip_dst);
	if (!opol)
	{
#ifdef DEBUGSW
		if (debugsw & DSW_NOOPOL)
		  printf("swoutput: no policy from 0x%x to 0x%x!\n",
			 htonl(iip->ip_src.s_addr), htonl(iip->ip_dst.s_addr));
#endif DEBUGSW
		m_freem(m0);
		return EHOSTUNREACH;
	}
	
	/*
	 * Get the keys. If no keys, signal the user-level process
	 * handling key management to obtain keys for us.
	 */

	se = swipe_get(opol->so_osrc, opol->so_odst, keysnd, SWSEL_DST);
	
	if (!se)
	{
		caddr_t pkt;
		struct ip *iip;
		struct swipe *sp;
		register u_short sum;
		
#ifdef DEBUGSW
		if (debugsw & DSW_NOKEY)
		  printf("%s%d: no key for 0x%x\n", ifp->if_name, ifp->if_unit, opol->so_odst);
#endif DEBUGSW
		/*
		 * Hand-craft a packet to send up.
		 */

		m_freem(m0->m_next);
		m0->m_next = NULL;

		if (m0->m_off < MMAXOFF)
		{
			m0->m_off = MMINOFF;
		}
		
		m0->m_len = sizeof(struct ifnet *) + sizeof(struct ip) + 2*sizeof(struct in_addr);
		
		pkt = mtod(m0, caddr_t);
		
		*((struct ifnet **)pkt) = ifp;
		((long)pkt) += sizeof(struct ifnet *);
		iip = (struct ip *)pkt;
		iip->ip_v = 4;
		iip->ip_hl = 5;
		iip->ip_tos = 0;
		iip->ip_len = htons(sizeof(struct ip) + 2*sizeof(struct in_addr));
		iip->ip_id = ip_id++;
		iip->ip_off = 0;
		iip->ip_ttl = MAXTTL;
		iip->ip_p = IPPROTO_IPIP;
#define ips ((u_short *) iip)
		sum = ips[0] + ips[1] + ips[2] + ips[3] + ips[4] + 
		  ips[6] + ips[7] + ips[8] + ips[9];
		iip->ip_sum = ~(sum + (sum >> 16));
#undef ips
		iip->ip_src = IA_SIN(ifp->if_addrlist)->sin_addr;
		iip->ip_dst = iip->ip_src;

		((long)pkt) += sizeof(struct ip);

		sp = (struct swipe *)pkt;
		sp->sw_type = IPIP_GETSNDKEY;
		sp->sw_hlen = 3;
		sp->sw_polid = 0;
		
		((long)pkt) += 4;
		*((struct in_addr *)pkt) = opol->so_osrc;
		((long)pkt) += 4;
		*((struct in_addr *)pkt) = opol->so_odst;

		s = splimp();
		ifp->if_opackets++;
		ifq = &ipintrq;
		if (IF_QFULL(ifq)) {
			IF_DROP(ifq);
			m_freem(m0);
			splx(s);
			return (ENOBUFS);
		}
		IF_ENQUEUE(ifq, m0);
		schednetisr(NETISR_IP);
		ifp->if_ipackets++;
		splx(s);
		return (0);
	}
	
	iip->ip_off = htons(iip->ip_off);
	iip->ip_len = htons(iip->ip_len);

	
	/*
	 * This is essentially the routing code from 
	 * Look through routes, ignoring routes that point to us
	 */

	/*
	 * Route packet.
	 */

#ifdef DEBUGSW
	if (debugsw & DSW_TRYROUTE)
	  printf("swoutput: trying to route\n");
#endif DEBUGSW

	din = (struct sockaddr_in *)&ro->ro_dst;
	
	/*
	 * If there is a cached route,
	 * check that it is to the same destination
	 * and is still up.  If not, free it and try again.
	 */
	if (ro->ro_rt && ((ro->ro_rt->rt_flags & RTF_UP) == 0 ||
			  din->sin_addr.s_addr != iip->ip_dst.s_addr))
	{
		RTFREE(ro->ro_rt);
		ro->ro_rt = (struct rtentry *)0;
	}
	if (ro->ro_rt == 0)
	{
		register struct rtentry *rt;
		register struct mbuf *m;
		register u_long hash;
		int (*match)(), doinghost, s;
		struct afhash h;
		struct mbuf **table;
		
		din->sin_family = AF_INET;
		din->sin_addr = opol->so_odst;
		
		(*afswitch[AF_INET].af_hash)(din, &h);
		match = afswitch[AF_INET].af_netmatch;
		hash = h.afh_hosthash, table = rthost, doinghost = 1;
		s = splnet();

	      again:
		for (m = table[RTHASHMOD(hash)]; m; m = m->m_next)
		{
			rt = mtod(m, struct rtentry *);
			if (rt->rt_hash != hash)
			  continue;
#ifdef DEBUGSW
			if (debugsw & DSW_ROUTECHECK)
			  printf("swoutput: checking 0x%x\n", htonl(((struct sockaddr_in *)&(rt->rt_dst))->sin_addr.s_addr));
#endif DEBUGSW

			if ((rt->rt_flags & RTF_UP) == 0 ||
			    (rt->rt_ifp->if_flags & IFF_UP) == 0 ||
			    (rt->rt_ifp == ifp))
			  continue;
			if (doinghost) {
				if (bcmp((caddr_t)&rt->rt_dst, (caddr_t)din,
					 sizeof (*din)))
				  continue;
			} else {
				if (rt->rt_dst.sa_family != AF_INET ||
				    !(*match)(&rt->rt_dst, din))
				  continue;
			}
			rt->rt_refcnt++;
			splx(s);
			if (din == &wildcard)
			  rtstat.rts_wildcard++;
			ro->ro_rt = rt;
			goto foundroute;
		}
		if (doinghost) 
		{
			doinghost = 0;
			hash = h.afh_nethash, table = rtnet;
			goto again;
		}
		/*
		 * Check for wildcard gateway, by convention network 0.
		 */
		if (din != &wildcard) {
			din = &wildcard, hash = 0;
			goto again;
		}
		splx(s);
		rtstat.rts_unreach++;
	}
	
      foundroute:
	
	if (ro->ro_rt == 0 || (ifp = ro->ro_rt->rt_ifp) == 0)
	{
		if (in_localaddr(opol->so_odst))
		  error = EHOSTUNREACH;
		else
		  error = ENETUNREACH;
		goto bad;
	}
	ro->ro_rt->rt_use++;

#ifdef DEBUGSW
	if (debugsw & DSW_SHOWROUTE)
	  printf("swoutput: using  0x%x 0x%x %s%d\n", 
		 htonl(((struct sockaddr_in *)&(ro->ro_rt->rt_dst))->sin_addr.s_addr),
		 htonl(((struct sockaddr_in *)&(ro->ro_rt->rt_gateway))->sin_addr.s_addr),
		 ro->ro_rt->rt_ifp->if_name, ro->ro_rt->rt_ifp->if_unit);
	
#endif DEBUGSW

	/*
	 * Copy the packet -- the overhead of doing this is nothing
	 * compared to doing an MD5 hash, and less than nothing compared
	 * do doing DES or, worse, triple-DES.
	 */


	if ((m1 =m_copy(m0, 0, (int)M_COPYALL)) == 0)
	{
		error = ENOSPC;
		goto bad;
	}
	m_freem(m0);

	return swipe_output(m1, se, ro);

      bad:
	m_freem(m0);
	return error;
	
}

#endif /* NSW */
