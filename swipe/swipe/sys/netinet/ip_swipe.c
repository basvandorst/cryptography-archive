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
 * swIPe implementation, per [Ioannidis & Blaze 93b]
 */

#ifdef SWIPEDEBUG
#include "debug.h"
unsigned long swipedebug = 0xffffffff;
#endif SWIPEDEBUG

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/ioctl.h>

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


extern struct ifnet *ifsw;

#ifdef DEBUGDES
extern int debugdes;
#endif

struct swipevars swipevars = 
{
	SKHASHSIZ, 
	keysnd, keyrcv, ipolicy, opolicy,
#ifdef DEBUGSW
	&debugsw,
#endif
#ifdef SWIPEDEBUG
	&swipedebug,
#endif
};

static struct sockproto ipipproto = { AF_INET, IPPROTO_IPIP }; 
static struct sockaddr_in ipipssrc = { AF_INET, }; 
static struct sockaddr_in ipipsdst = { AF_INET, }; 

/*
 * ipip_swipecheck
 *
 * Checks if the encapsulated packet pointed to by m passes 
 * authentication/encryption.
 */

int
ipip_swipecheck(m, ifp)
struct mbuf *m;
struct ifnet *ifp;
{
	int s, adjlen, olen, swhlen, ipoerr, seqn, doskip = 0;
	u_long oswh;
	struct in_addr osrc, odst;
	struct ip *oip;			/* outer ip header */
	struct swipe *sp;		/* swipe header */
	struct ip *iip;			/* inner ip header */
	struct swkey_ent *se;		/* key entry from hash tables */
	struct swipe_key *skp;
	struct ifqueue *ifq;
	u_long digest[4];		/* MD5 digest goes here */
	
#ifdef DEBUGDES
	if (debugdes & 0x01000000)
	  mbuf_dump(m);
#endif

	/*
	 * XXX -- what spl level do we run this at? (splnet already?)
	 */

	oip = mtod(m, struct ip *);
	osrc = oip->ip_src;
	odst = oip->ip_dst;
	olen = (u_short) oip->ip_len;	/* payload size in native byte order*/

	m_adj(m, oip->ip_hl << 2);
	sp = mtod(m, struct swipe *);

	if ((sp->sw_type & IPIP_NOHDRMASK) == IPIP_NOHDRVAL)
	{
		adjlen = 0 + sizeof (struct ip);
		doskip = 1;
	}
	else if (sp->sw_type == IPIP_PLAIN)
	{
		adjlen = 4 + sizeof (struct ip);
		doskip = 1;
	}
	else
	  adjlen = (sp->sw_hlen << 2) + sizeof (struct ip);

	if ( (m->m_len < adjlen) && ((m = m_pullup(m, adjlen)) == 0) )
	{
		/*
		 * XXX -- Notify keymgmt? (no)
		 */

#ifdef SWIPEDEBUG
		if (swipedebug & SWD_SHORTENCAP)
		  printf("ipip_swipecheck: encapsulated ip header too short\n");
#endif SWIPEDEBUG
		swipestat.ipip_badheader++;
		goto bad;
	}

	/*
	 * contents may settle during shipment
	 */

	sp = mtod(m, struct swipe *);

	if (doskip)
	  goto decap;

	se = swipe_get(osrc, odst, keyrcv, SWSEL_SRC);
	if (!se)
	{
		/*
		 * XXX -- Notify keymgmt? (yes)
		 */

#ifdef SWIPEDEBUG
		if (swipedebug & SWD_NOKEYS)
		  printf("ipip_swipecheck: can't find keys for (0x%x, 0x%x)\n", ntohl(osrc.s_addr), ntohl(odst.s_addr));
#endif SWIPEDEBUG
		goto sendup;
	}

	
	if ((sp->sw_type == IPIP_CRYPT) || (sp->sw_type == IPIP_AE))
	{
		/* 
		 * packet is encrypted; decrypt first
		 */

		skp = &se->se_cryptkey;
		if (!(skp->sk_algo))
		{
			/*
			 * XXX -- Notify Matt?
			 */
#ifdef SWIPEDEBUG
			if (swipedebug & SWD_NODCRKEY)
			  printf("ipip_swipecheck: couldn't find decryption key for (0x%x, 0x%x)\n", ntohl(osrc.s_addr), ntohl(osrc.s_addr));
#endif SWIPEDEBUG
			swipestat.ipip_badrcvcrypt++;
			goto sendup;
		}
		switch (skp->sk_algo)
		{
		      case SWIPE_E_DES:
			if (swipe_decrypt(m, skp, olen) < 0)
			  goto sendup;
			break;

		      default:
#ifdef SWIPEDEBUG
			if (swipedebug & SWD_BADEALG)
			  printf("ipip_swipecheck: bad encryption algorithm %d for (0x%x, 0x%x)\n", skp->sk_algo, ntohl(osrc.s_addr), ntohl(odst.s_addr));
#endif SWIPEDEBUG
			swipestat.ipip_badrcvcrypt++;
			goto sendup;
		}
	}

	if ((sp->sw_type == IPIP_AUTH) || (sp->sw_type == IPIP_AE))
	{
		
		u_long *authdata = (u_long *)&sp->sw_auth[0];
		u_long authsave[4];
		
		/*
		 * packet is authenticated.
		 */
		
		skp = &se->se_authkey;
		
		if (!(skp->sk_algo))
		{
			/*
			 * XXX -- Notify Matt?
			 */
#ifdef SWIPEDEBUG
			if (swipedebug & SWD_NOAUTHKEY)
			  printf("ipip_swipecheck: couldn't find authentication key for (0x%x, 0x%x)\n", ntohl(osrc.s_addr), ntohl(osrc.s_addr));
#endif SWIPEDEBUG
			swipestat.ipip_badrcvauth++;
			goto sendup;
		}
		
		switch (skp->sk_algo)
		{
		      case SWIPE_A_MD5_128:
			/*
			 * save the last 64 bits
			 */
			authsave[3] = authdata[3];
			authdata[3] = 0;
			authsave[2] = authdata[2];
			authdata[2] = 0;
			
		      case SWIPE_A_MD5_64:
			authsave[1] = authdata[1];
			authdata[1] = 0;
			
		      case SWIPE_A_MD5_32:
			authsave[0] = authdata[0];
			authdata[0] = 0;
			break;
			
		      default:
#ifdef SWIPEDEBUG
			if (swipedebug & SWD_BADAALG)
			  printf("ipip_swipecheck: bad authentication algorithm %d for (0x%x, 0x%x)\n", skp->sk_algo, ntohl(osrc.s_addr), ntohl(osrc.s_addr));
#endif SWIPEDEBUG
			swipestat.ipip_badrcvauth++;
			goto sendup;
		}
			
		/*
		 * The appropriate amount of header data have been zeroed
		 * out, exactly as they were before the packet was
		 * authenticated. Well, let's do it again and 
		 * see if the results match.
		 */

		md5mbuf(m, skp, (caddr_t)digest);
		
		/*
		 * fold as appropriate
		 */

		switch (skp->sk_algo)
		{
		      case SWIPE_A_MD5_64:
			digest[0] ^= digest[2];
			digest[1] ^= digest[3];
			break;
			
		      case SWIPE_A_MD5_32:
			digest[0] ^= digest[2];
			digest[1] ^= digest[3];
			digest[0] ^= digest[1];
			break;

		}

		switch (skp->sk_algo)
		{
		      case SWIPE_A_MD5_128:
			/*
			 * test the last 64 bits
			 */
			if (authsave[3] != digest[3])
			  goto badauth;
			if (authsave[2] != digest[2])
			  goto badauth;
			
		      case SWIPE_A_MD5_64:
			if (authsave[1] != digest[1])
			  goto badauth;
			
		      case SWIPE_A_MD5_32:
			if (authsave[0] != digest[0])
			  goto badauth;
		}
	}
	
	/*
	 * Packet passed encryption and/or authentication.
	 * Time to check if sequence number is good. If not, drop it.
	 *
	 */ 

	if ((seqn = ntohl(sp->sw_seqnum)) > se->se_seqnum)
	  se->se_seqnum = seqn;
	else
	  goto bad;			/* replayed packet! */
	
      decap:

	/*
	 * Let's get rid of the swipe header.
	 * Remember, we pulled up enough bytes for the swipe header 
	 * and the following IP header, so this should leave us with
	 * an IP header.
	 * However, allow enough space for a struct ifnet *
	 */
	
	m_adj(m, adjlen - sizeof(struct ip) - sizeof (struct ifnet *));
	

	/*
	 * We feed this packet back into the IP input queue, specifying
	 * the first virtual interface as an indication that the 
	 * packet has passed swIPe policy and that it should be 
	 * simply passed on.
	 */

	*(mtod(m, struct ifnet **)) = ifsw;
	s = splimp();
	ifq = &ipintrq;
	if (IF_QFULL(ifq))
	{
		IF_DROP(ifq);
		m_freem(m);
		splx(s);
		return (ENOBUFS);
	}
	IF_ENQUEUE(ifq, m);
	/* schednetisr(NETISR_IP); not needed, according to VJ */
	ifsw->if_ipackets++;
	splx(s);
	return 0;

      bad:
	m_freem(m);
	return 0;

      badauth:
	/*
	 * XXX -- Fix this, please!!!
	 */
#ifdef DEBUGDES
	if (debugdes & 0x10000)
	  printf("ipip_swipecheck: failed to authenticate\n"), mbuf_dump(m);
#endif

      sendup:
	
	sp->sw_type = IPIP_GETRCVKEY;
	oswh = *((u_long *)sp);		/* save the first word */
	
	if (m->m_next)
	{
		m_freem(m->m_next);
	}
	m->m_next = NULL;		/* so I won't need another dream (ji) */
	if (m->m_off < MMAXOFF)		/* don't deal with clusters here */
	  m->m_off = MMINOFF;
	m->m_len = 24;
	oip = mtod(m, struct ip *);

	oip->ip_v = 3;			/* just for kicks */
	oip->ip_hl = 5;			/* just in case */
	oip->ip_len = 4;
	oip->ip_p = 94;
	oip->ip_src = osrc;
	oip->ip_dst = odst;
	((u_long *)oip)[5] = oswh;

	ipipssrc.sin_addr = osrc;
	ipipsdst.sin_addr = odst;
	return raw_input(m, &ipipproto,
			 (struct sockaddr *)&ipipssrc,
			 (struct sockaddr *)&ipipsdst);
}	

/*
 * Swipe-specific ioctl's. Should be issued on a raw socket.
 */

int
swioctl(dev, cmd, data)
dev_t dev;
int cmd;
caddr_t data;
{
	int s, cnt, c, i;
	struct swkey_ent *ke, *se;
	struct sw_opol *opol, *opl;
	struct sw_ipol *ipol, *ipl;
	struct swipevars *svrs;

	ke = (struct swkey_ent *)data;
	opol = (struct sw_opol *)data;
	ipol = (struct sw_ipol *)data;
	svrs = (struct swipevars *)data;
	
	if (!suser())
	  return u.u_error;
	

	s = splnet();
	
	/*
	 * This is where the SWSEL_SRC / SWSEL_DST 
	 * distinction is somewhat enforced.
	 *
	 * We ONLY use SWSEL_SRC with keyrcv, and SWSEL_DST with keysnd
	 */

	switch (cmd)
	{
	      case SWIPSSNDKEYS:
		se=swipe_getoralloc(ke->se_osrc, ke->se_odst, keysnd, SWSEL_DST);

		if (!se)
		{
			u.u_error = ENOENT;
			splx(s);
			return u.u_error;
		}
		
		se->se_seqnum = ke->se_seqnum;
		se->se_authkey = ke->se_authkey;
		if (ke->se_cryptkey.sk_algo == SWIPE_E_DES)
		{
			struct mbuf *sm;

			/*
			 * DES key supplied -- init the key schedule
			 */

			if (!se->se_cryptkey.sk_keysched)
			{
				/*
				 * This is probably not very kosher;
				 * I'm gambling that the meta-data of
				 * an mbuf are mine to do what I want
				 * with, and so I can overwrite the
				 * whole thing with 128 bytes worth
				 * of a key schedule. If this turns
				 * out to be wrong, I'll have to allocate
				 * chucks of a cluster, and waste space
				 * in the process. Oh well... 
				 *
				 * /ji
				 */ 
				
				MGET(sm, M_DONTWAIT, MT_SWKEYSCHEDS);
				if (!sm)
				{
					u.u_error = ENOBUFS;
					splx(s);
					return u.u_error;
				}
			}
			else
			  (caddr_t)sm = se->se_cryptkey.sk_keysched;
			
			se->se_cryptkey = ke->se_cryptkey;
			se->se_cryptkey.sk_keysched = (caddr_t)sm;

			for(cnt = 0; cnt < 8; cnt++)
			{
				c = 0;
				for(i=0;i<7;i++)
				  if(se->se_cryptkey.sk_deskey[cnt] & (1 << i))
				    c++;
				if((c & 1) == 0)
				  se->se_cryptkey.sk_deskey[cnt] |= 0x80;
				else
				  se->se_cryptkey.sk_deskey[cnt] &= ~0x80;
			}

			swipe_des_setkey(se->se_cryptkey.sk_deskey,
					 se->se_cryptkey.sk_keysched);
		}
		else
		  se->se_cryptkey = ke->se_cryptkey;

		splx(s);
		return 0;
		
	      case SWIPSRCVKEYS:
		se=swipe_getoralloc(ke->se_osrc, ke->se_odst, keyrcv, SWSEL_SRC);
		if (!se)
		{
			u.u_error = ENOENT;
			splx(s);
			return u.u_error;
		}
		
		se->se_seqnum = ke->se_seqnum;
		se->se_authkey = ke->se_authkey;

		if (ke->se_cryptkey.sk_algo == SWIPE_E_DES)
		{
			struct mbuf *sm;

			/*
			 * DES key supplied -- init the key schedule
			 */

			if (!se->se_cryptkey.sk_keysched)
			{
				/*
				 * This is probably not very kosher;
				 * I'm gambling that the meta-data of
				 * an mbuf are mine to do what I want
				 * with, and so I can overwrite the
				 * whole thing with 128 bytes worth
				 * of a key schedule. If this turns
				 * out to be wrong, I'll have to allocate
				 * chucks of a cluster, and waste space
				 * in the process. Oh well... 
				 *
				 * /ji
				 */ 
				
				MGET(sm, M_DONTWAIT, MT_SWKEYSCHEDS);
				if (!sm)
				{
					u.u_error = ENOBUFS;
					splx(s);
					return u.u_error;
				}
			}
			else
			  (caddr_t)sm = se->se_cryptkey.sk_keysched;
			
			se->se_cryptkey = ke->se_cryptkey;
			se->se_cryptkey.sk_keysched = (caddr_t)sm;

			for(cnt = 0; cnt < 8; cnt++)
			{
				c = 0;
				for(i=0;i<7;i++)
				  if(se->se_cryptkey.sk_deskey[cnt] & (1 << i))
				    c++;
				if((c & 1) == 0)
				  se->se_cryptkey.sk_deskey[cnt] |= 0x80;
				else
				  se->se_cryptkey.sk_deskey[cnt] &= ~0x80;
			}

			swipe_des_setkey(se->se_cryptkey.sk_deskey,
					 se->se_cryptkey.sk_keysched);
		}
		else
		  se->se_cryptkey = ke->se_cryptkey;

		splx(s);
		return 0;
		
	      case SWIPGSNDKEYS:
		se = swipe_get(ke->se_osrc, ke->se_odst, keysnd, SWSEL_DST);
		if (!se)
		{
			u.u_error = ENOENT;
			splx(s);
			return u.u_error;
		}
		
		*ke = *se;
		splx(s);
		return 0;
		
	      case SWIPGRCVKEYS:
		se = swipe_get(ke->se_osrc, ke->se_odst, keyrcv, SWSEL_SRC);
		if (!se)
		{
			u.u_error = ENOENT;
			splx(s);
			return u.u_error;
		}
		
		*ke = *se;
		splx(s);
		return 0;

	      case SWIPSOPOL:
		
		opl = swipe_getorallocopol(opol->so_isrc, opol->so_idst);

		if (!opl)
		{
			u.u_error = ENOENT;
			splx(s);
			return u.u_error;
		}

		opl->so_osrc = opol->so_osrc;
		opl->so_odst = opol->so_odst;

		splx(s);
		return 0;
		
	      case SWIPSIPOL:
		
		ipl = swipe_getorallocipol(ipol->si_isrc, ipol->si_idst);

		if (!ipl)
		{
			u.u_error = ENOENT;
			splx(s);
			return u.u_error;
		}

		ipl->si_accpolicy = ipol->si_accpolicy;
		for (i=0; i<ipol->si_nosrcs; i++)
		  ipl->si_osrcs[i] = ipol->si_osrcs[i];

		splx(s);
		return 0;

	      case SWIPGSWVARS:
		*svrs = swipevars;
		splx(s);
		return 0;
	}
}



/*
 * Find the key entry that matches source and destination in the
 * appropriate table.
 * Must be called at splnet.
 */

struct swkey_ent *
swipe_get(src, dst, where, flags)
struct in_addr src, dst;
struct mbuf *where[];
int flags;
{
	struct mbuf **m0, *m;
	int hv;
	struct swkey_ent *se;

	flags &= (SWSEL_SRC | SWSEL_DST);
	
	hv = SWIPEHASH(src, dst, flags);
	m0 = &where[SKHASHMOD(hv)];
	
	for (m = *m0; m; m = m->m_next)
	{
		se = mtod(m, struct swkey_ent *);
	
		/*
		 * quick check here.
		 */
		if (se->se_hash != hv)
		  continue;

		/*
		 * The following shouldn't happen given the hash
		 * algorithm we are using, but we check it nonetheless
		 */

		if ((flags & SWSEL_DST) && (se->se_odst.s_addr != dst.s_addr))
		  continue;

		if ((flags & SWSEL_SRC) && (se->se_osrc.s_addr != src.s_addr))
		  continue;

		
		return se;
	}
	
	/*
	 * If we're here, it means that we didn't find a matching entry. 
	 */
	
	return 0;
}

/*
 * Try to find an entry, as above, but if you fail, allocate one.
 * Must be called at splnet.
 */

struct swkey_ent *
swipe_getoralloc(src, dst, where, flags)
struct in_addr src, dst;
struct mbuf *where[];
int flags;
{
	struct mbuf **m0, *m;
	int hv;
	struct swkey_ent *se;

	flags &= (SWSEL_SRC | SWSEL_DST);

	hv = SWIPEHASH(src, dst, flags);
	m0 = &where[SKHASHMOD(hv)];
	
	for (m = *m0; m; m = m->m_next)
	{

		se = mtod(m, struct swkey_ent *);
	
		/*
		 * quick check here.
		 */
		if (se->se_hash != hv)
		  continue;

		/*
		 * The following shouldn't happen given the hash
		 * algorithm we are using, but we check it nonetheless
		 */

		if ((flags & SWSEL_DST) && (se->se_odst.s_addr != dst.s_addr))
		  continue;

		if ((flags & SWSEL_SRC) && (se->se_osrc.s_addr != src.s_addr))
		  continue;

		se->se_osrc = src;
		se->se_odst = dst;
		
		return se;
	}
	
	/*
	 * If we're here, it means that we didn't find a matching entry. 
	 * We'll get one.
	 */
	
	MGET(m, M_DONTWAIT, MT_SWKEYENT);
	if (m == 0)
	{
		return 0;
	}
	
	m->m_next = *m0;
	*m0 = m;
	
	se = mtod(m, struct swkey_ent *);
	m->m_len = sizeof (struct swkey_ent);
	bzero((caddr_t)se, m->m_len);

	se->se_hash = hv;
	se->se_osrc = src;
	se->se_odst = dst;

	return se;
}

struct sw_opol *
swipe_getopol(src, dst)
struct in_addr src, dst;
{
	struct mbuf **m0, *m;
	int hv;
	struct sw_opol *opl;

	hv = SWIPEHASH2(src, dst);
	m0 = &opolicy[SKHASHMOD(hv)];
	
	for (m = *m0; m; m = m->m_next)
	{
		opl = mtod(m, struct sw_opol *);
	
		/*
		 * quick check here.
		 */
		if (opl->so_hash != hv)
		  continue;

		/*
		 * The following shouldn't happen given the hash
		 * algorithm we are using, but we check it nonetheless
		 */

		if ((opl->so_idst.s_addr != dst.s_addr) ||
		    (opl->so_isrc.s_addr != src.s_addr))
		  continue;

		return opl;
	}
	
	/*
	 * If we're here, it means that we didn't find a matching entry. 
	 */
	
	return (struct sw_opol *)0;
}

/*
 * Try to find an entry, as above, but if you fail, allocate one.
 * Must be called at splnet.
 */

struct sw_opol *
swipe_getorallocopol(src, dst)
struct in_addr src, dst;
{
	struct mbuf **m0, *m;
	int hv;
	struct sw_opol *opl;

	hv = SWIPEHASH2(src, dst);
	m0 = &opolicy[SKHASHMOD(hv)];
	
	for (m = *m0; m; m = m->m_next)
	{
		opl = mtod(m, struct sw_opol *);
	
		/*
		 * quick check here.
		 */
		if (opl->so_hash != hv)
		  continue;

		/*
		 * The following shouldn't happen given the hash
		 * algorithm we are using, but we check it nonetheless
		 */

		if ((opl->so_idst.s_addr != dst.s_addr) ||
		    (opl->so_isrc.s_addr != src.s_addr))
		  continue;

		return opl;
	}

	/*
	 * If we're here, it means that we didn't find a matching entry. 
	 * We'll get one.
	 */
	
	MGET(m, M_DONTWAIT, MT_SWOPOLICY);
	if (m == 0)
	{
		return 0;
	}
	
	m->m_next = *m0;
	*m0 = m;
	
	opl = mtod(m, struct sw_opol *);
	m->m_len = sizeof (struct sw_opol);
	bzero((caddr_t)opl, m->m_len);

	opl->so_hash = hv;
	opl->so_isrc = src;
	opl->so_idst = dst;

	return opl;
}

struct sw_ipol *
swipe_getipol(src, dst)
struct in_addr src, dst;
{
	struct mbuf **m0, *m;
	int hv;
	struct sw_ipol *ipl;

	hv = SWIPEHASH2(src, dst);
	m0 = &ipolicy[SKHASHMOD(hv)];
	
	for (m = *m0; m; m = m->m_next)
	{
		ipl = mtod(m, struct sw_ipol *);
	
		/*
		 * quick check here.
		 */
		if (ipl->si_hash != hv)
		  continue;

		/*
		 * The following shouldn't happen given the hash
		 * algorithm we are using, but we check it nonetheless
		 */

		if ((ipl->si_idst.s_addr != dst.s_addr) ||
		    (ipl->si_isrc.s_addr != src.s_addr))
		  continue;

		return ipl;
	}
	
	/*
	 * If we're here, it means that we didn't find a matching entry. 
	 */
	
	return (struct sw_ipol *)0;
}

/*
 * Try to find an entry, as above, but if you fail, allocate one.
 * Must be called at splnet.
 */

struct sw_ipol *
swipe_getorallocipol(src, dst)
struct in_addr src, dst;
{
	struct mbuf **m0, *m;
	int hv;
	struct sw_ipol *ipl;

	hv = SWIPEHASH2(src, dst);
	m0 = &ipolicy[SKHASHMOD(hv)];
	
	for (m = *m0; m; m = m->m_next)
	{
		ipl = mtod(m, struct sw_ipol *);
	
		/*
		 * quick check here.
		 */
		if (ipl->si_hash != hv)
		  continue;

		/*
		 * The following shouldn't happen given the hash
		 * algorithm we are using, but we check it nonetheless
		 */

		if ((ipl->si_idst.s_addr != dst.s_addr) ||
		    (ipl->si_isrc.s_addr != src.s_addr))
		  continue;

		return ipl;
	}

	/*
	 * If we're here, it means that we didn't find a matching entry. 
	 * We'll get one.
	 */
	
	MGET(m, M_DONTWAIT, MT_SWOPOLICY);
	if (m == 0)
	{
		return 0;
	}
	
	m->m_next = *m0;
	*m0 = m;
	
	ipl = mtod(m, struct sw_ipol *);
	m->m_len = sizeof (struct sw_ipol);
	bzero((caddr_t)ipl, m->m_len);

	ipl->si_hash = hv;
	ipl->si_isrc = src;
	ipl->si_idst = dst;

	return ipl;
}

/*
 * swipe_output() gets called by ip_output() to encapsulate a packet
 * and send it out.
 */

int
swipe_output(m, se, ro)
struct mbuf *m;
struct swkey_ent *se;
struct route *ro;
{
	struct ip *ip;
	struct swipe *sw;
	struct ipovly *ih;
	struct mbuf *m0 = m;
	register int shlen;
	int ptype = 0;
	u_long digest[4];		/* MD5 digest goes here */
	u_long *authdata;
	int innerlen = 0;
	int padding = 0;		/* DES padding at the end */
	int error;
	
	ip = mtod(m, struct ip *);

	shlen = 8;			/* header + seqnum */
	switch (se->se_authkey.sk_algo)
	{
	      case SWIPE_A_MD5_128:
		shlen += 8;
	      case SWIPE_A_MD5_64:
		shlen += 4;
	      case SWIPE_A_MD5_32:
		shlen += 4;
		ptype = IPIP_AUTH;
	}
	
	if (se->se_cryptkey.sk_algo == SWIPE_E_DES)
	{
		if (ptype == IPIP_AUTH)
		  ptype = IPIP_AE;
		else
		  ptype = IPIP_CRYPT;
	}

	
	/*
	 * Save enough space for the IP header in front,
	 * but leave the mbuf starting at where the swipe header
	 * should start.
	 */

	if (m->m_off >= MMAXOFF ||
	    MMINOFF + sizeof(struct ip) + shlen > m->m_off)
	{
		MGET(m, M_DONTWAIT, MT_HEADER);
		if (m == 0)
		{
			m_freem(m0);
			return ENOBUFS;
		}
		m->m_next = m0;
		m->m_len = shlen;
		m->m_off = MMAXOFF - m->m_len;
	}
	else
	{
		m->m_len += shlen;
		m->m_off -= shlen;
	}
	

	sw = mtod(m, struct swipe *);
	
	/*
	 * Now, prepare the inner packet for sending.
	 */

	innerlen = ip->ip_len;		/* will need it later */
	ip->ip_len = htons(innerlen);
	ip->ip_off = htons(ip->ip_off);
	
	if (!ip->ip_src.s_addr)
	  ip->ip_src = se->se_osrc;

	
	/*
	 * Add the swipe header.
	 */

	bzero((caddr_t) sw, shlen);
	sw->sw_type = ptype;
	sw->sw_hlen = shlen >> 2;
	sw->sw_polid = 0;
	sw->sw_seqnum = htonl(se->se_seqnum++);
	

	if ((ptype == IPIP_AUTH) || (ptype == IPIP_AE))
	{
		md5mbuf(m, &se->se_authkey, (caddr_t)digest);
		authdata = (u_long *)&sw->sw_auth[0];

		/*
		 * fold as appropriate
		 */

		switch (se->se_authkey.sk_algo)
		{
		      case SWIPE_A_MD5_64:
			digest[0] ^= digest[2];
			digest[1] ^= digest[3];
			break;
			
		      case SWIPE_A_MD5_32:
			digest[0] ^= digest[2];
			digest[1] ^= digest[3];
			digest[0] ^= digest[1];
			break;

		}

		switch (se->se_authkey.sk_algo)
		{
		      case SWIPE_A_MD5_128:
			authdata[3] = digest[3];
			authdata[2] = digest[2];
			
		      case SWIPE_A_MD5_64:
			authdata[1] = digest[1];
			
		      case SWIPE_A_MD5_32:
			authdata[0] = digest[0];
		}
	}

	if ((ptype == IPIP_CRYPT) || (ptype == IPIP_AE))
	{
		switch (se->se_cryptkey.sk_algo)
		{
		      case SWIPE_E_DES:
			padding = swipe_encrypt(m, &se->se_cryptkey, innerlen+shlen);
		}
	}
	

	/*
	 * We are done with authentication and encryption.
	 * Time to move up the packet header, add the ip header,
	 * (we know there is enough space)
	 * and send the packet off to its merry way.
	 */
	
	if (padding < 0)
	{
		printf("swipe_output: bad padding %d\n", padding);
		m_freem(m);
		return -1;
	}
	
	

	m->m_off -= sizeof(struct ip);
	m->m_len += sizeof(struct ip);

	ih = mtod(m, struct ipovly *);
	ih->ih_next = ih->ih_prev = (caddr_t)0;
	ih->ih_x1 = 0;
	ih->ih_pr = IPPROTO_IPIP;
	ih->ih_src.s_addr = INADDR_ANY;
	ih->ih_dst = se->se_odst;	/* XXX -- this may not be this :-) */
	((struct ip *)ih)->ip_ttl = MAXTTL;
	((struct ip *)ih)->ip_len = padding + innerlen + 
	  shlen + sizeof(struct ipovly);
	
	error = ip_output(m, 0, ro,
			  IP_ALLOWBROADCAST | IP_SWIPED);

	if (error)
	  return error;
	
	return 0;
	


}

/*
 * This is the decapsulator routine. There is some extra crud
 * here to make it compatible with the IPIP definition in
 * JI's old Mobile*IP code
 */

swipe_input(m, ifp)
struct mbuf *m;
struct ifnet *ifp;
{
	register struct mbuf *m0 = m;
	struct ip *ip = mtod(m, struct ip *);
	struct in_addr ipipsrc, ipipdst;
	struct swipe *pp;
	int hlen;

#ifdef SWIPEDEBUG
	struct in_addr osrc, odst, rsrc, rdst;
	int olen;
#endif

	hlen = (ip->ip_hl << 2) + sizeof(struct swipe) + sizeof (struct ip);
	if ( (m->m_len < hlen) && 
	    ((m = m_pullup(m, hlen)) == 0) )
	{
#ifdef SWIPEDEBUG
		if (swipedebug & SWD_SHORTHDR)
		  printf("ipip_input, initial length too short\n");
#endif
		/*
		 * XXX -- should update some error counter
		 */
		m_freem(m);
		return 0;
	} 

#ifdef SWIPEDEBUG
	if (swipedebug & SWD_PRTHDR)
	{
		osrc = ip->ip_src;
		odst = ip->ip_dst;
		olen = ip->ip_len;	/* the header we get is  */
					/* already byte-swapped  */
		printf("swipe_input: %s%d: [0x%8x->0x%8x], %d bytes\n", 
		       ifp->if_name,
		       ifp->if_unit,
		       htonl(osrc.s_addr),
		       htonl(odst.s_addr),
		       olen);
	}
#endif SWIPEDEBUG

	ipipsrc = ip->ip_src;
	ipipdst = ip->ip_dst;

	hlen = ip->ip_hl << 2;
	
	/*
	 * XXX - Deal with headers. A header may be non-existent, that is, 
	 * the contents of the IPIP packet are just an IP packet. This 
	 * is recognized by the fact that IP packets start with 4X 
	 * (where X is the header length, usually 5).
	 * This is a kludge introduced to accomodate a minor argument
	 * between Phil Karn and JI (Phil won).
	 */

	pp = (struct swipe *)(mtod(m, caddr_t) + hlen);
	if ((pp->sw_type & IPIP_NOHDRMASK) != IPIP_NOHDRVAL)
	{
		switch (pp->sw_type)
		{
		      case IPIP_PLAIN:
		      case IPIP_AUTH:
		      case IPIP_CRYPT:
		      case IPIP_AE:
			ipip_swipecheck(m, ifp);
			return;

		      default:
			ipipssrc.sin_addr = ipipsrc;
			ipipsdst.sin_addr = ipipdst;
			raw_input(m, &ipipproto,
				  (struct sockaddr *)&ipipssrc,
				  (struct sockaddr *)&ipipsdst);
			return;
		}
	}
	else
	{
		ipip_swipecheck(m, ifp);
		return;
	}
}


	
mbuf_dump(m0)
struct mbuf *m0;
{
	struct mbuf *m;
	int i;
	u_char *p;
	
	for (m=m0; m; m=m->m_next)
	{
		printf("\n New mbuf with %d bytes\n", m->m_len);
		p = mtod(m, u_char *);
		i = m->m_len;
		while (i--)
		  printf("%x ", *p++);
	}
}

