/*
 * ipv6_input.c  --  Routines to handle upward (through the protocol graph)
 *                   bound IPv6 packets.  Reassembly occurs here, as does
 *                   determination of forwarding and inbound option
 *                   processing.
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
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>

#include <net/if.h>
#include <net/route.h>

#include <netinet6/in6_debug.h>
#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h>

#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <netinet6/ipsec.h>
#include <netinet6/key.h>
#include <netinet6/ipsec_ah.h>
#include <netinet6/ipsec_esp.h>
#include <netinet6/ipsec_policy.h>

/*
 * Globals
 */

u_char ipv6_protox[NEXTHDR_MAX];       /* For easy demuxing to HLP's. */
struct ipv6stat ipv6stat;              /* Stats. */
struct in6_ifaddr *in6_ifaddr = NULL;  /* List of interfaces. */
struct ipv6_fragment *ipv6_fragmentq = NULL;   /* Fragment reassembly queue */

int ipv6qmaxlen = IFQ_MAXLEN;
struct ifqueue ipv6intrq;

/* These variables should be sysctl-tweakable. */
int ipv6forwarding = 0;
int ipv6_defhoplmt = MAXHOPLIMIT;

/*
 * External globals
 */

extern struct protosw inet6sw[];  /* See in6_proto.c */
extern struct domain inet6domain; /* See in6_proto.c */
extern struct discq dqhead;       /* See ipv6_discovery.c */

/*
 * Funct. prototypes.
 */

void ipv6_forward __P((struct mbuf *, int));
struct mbuf *ipv6_reasm __P((struct mbuf *, int, struct in6_hdrindex *, int));

/* For reasons somewhat unknown, <sys/mbuf.h> doesn't prototype this */
struct mbuf *m_split __P((register struct mbuf *, int, int));

/*----------------------------------------------------------------------
 * IPv6 initialization function.
 ----------------------------------------------------------------------*/

void
ipv6_init()
{
  register struct protosw *pr;
  register int i;

#ifdef INET6_DEBUG
  /*
   * Do in6_debug stuff.
   */
  in6_debug_init();
#endif
  DPRINTF(IDL_GROSS_EVENT,("IPv6 initializing..."));

  bzero(&ipv6stat,sizeof(struct ipv6stat));

  pr = pffindproto(PF_INET6, NEXTHDR_RAW, SOCK_RAW);
  if (pr == 0)
    panic("ipv6_init");  

  /*
   * To call the right IPv6 next header function off next header type, have
   * next header numbers index the protocol switch, like protocols in IP
   * (default the switch for this).  Otherwise, just switch off into normal
   * (TCP,UDP) stuff.
   *
   * Initialize ipv6_protox[].
   */

  for (i = 0; i < NEXTHDR_MAX; i++)
    ipv6_protox[i] = pr - inet6sw;
  for (pr = inet6domain.dom_protosw;
       pr < inet6domain.dom_protoswNPROTOSW; pr++)
    if (pr->pr_domain->dom_family == PF_INET6
        && pr->pr_protocol != NEXTHDR_RAW )
      ipv6_protox[pr->pr_protocol] = pr - inet6sw;

  /*
   * Initialize discovery stuff.
   */

  ipv6_discovery_init();

  /*
   * Initialize IPv6 i/f queue.
   */

  ipv6intrq.ifq_maxlen = ipv6qmaxlen;

  /*
   * Initialise IPsec
   */
  ipsec_init();

  DPRINTF(IDL_GROSS_EVENT,("...done\n"));
}

/*----------------------------------------------------------------------
 * IPv6 input queue interrupt handler.
 ----------------------------------------------------------------------*/

void 
ipv6intr()
{
  struct mbuf *m;
  int s;

  while (1)   /* Keep yanking off packets until I hit... */
    {
      s = splimp();
      IF_DEQUEUE(&ipv6intrq, m);
      splx(s);
      
      if (m == NULL)
	return;       /* ...HERE.  THIS is how I exit this endless loop. */

      /* Preparse here, perhaps? */
      
      ipv6_input(m,0,NULL,0);
    }
}

void
ipv6_input(incoming,extra,ihi,ihioff)
     struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *ihi;
     int ihioff;
{
  struct ipv6 *header;
  struct in6_ifaddr *i6a;
  struct in6_multi *in6m = NULL;
  int rc,jumbogram = 0;
  u_int32 length = 0;
  u_int8 nexthdr;
  int payload_len;

  DPRINTF(IDL_MAJOR_EVENT,("Got an IPv6 message!\n"));

  /*
   * Can't do anything until at least an interface is marked as
   * ready for IPv6.
   */

  if (in6_ifaddr == NULL)
    {
      m_freem(incoming);
      return;
    }

  ipv6stat.ips_total++;

  /*
   * IPv6 inside something else.  Discard encapsulating header(s) and
   * (maybe try to) make it look like virgin IPv6 packet.
   *
   * As with other stupid mbuf tricks, there probably is a better way.
   */

  if (extra) {
      struct mbuf *newpacket;

      DP(MAJOR_EVENT, extra, d);

      /*
       * Split packet into what I need, and what was encapsulating what I
       * need.  Discard the encapsulating portion.
       */

      if (!(newpacket = m_split(incoming,extra,M_NOWAIT))) {
	  printf("ipv6_input() couldn't trim extra off.\n");
	  m_freem(incoming);
	  return;
      }

      DDO(IDL_EVENT,printf("incoming is:\n");dump_mchain(incoming));
      DDO(IDL_EVENT,printf("newpacket is:\n");dump_mchain(newpacket));
      m_freem(incoming);
      incoming = newpacket;
      extra = 0;
  }

  /*
   * Even before preparsing (see vitriolic comments later), 
   * I need to have the whole IPv6 header at my disposal.
   */

  if (incoming->m_len < sizeof (struct ipv6) &&
      (incoming = m_pullup(incoming, sizeof(struct ipv6))) == NULL)
      {
	ipv6stat.ips_toosmall++;
	return;
      }

  /*
   * Check version bits immediately.
   */

  if ((incoming->m_data[0] >> 4) != IPV6VERSION)
    {
      ipv6stat.ips_badvers++;
      m_freem(incoming);
      return;
    }

  header = mtod(incoming, struct ipv6 *);

  DDO(IDL_EVENT,dump_ipv6(header));

  /*
   * Save off payload_len because of munging later...
   */
  payload_len = ntohs(header->ipv6_length);

  /*
   * Check that the amount of data in the buffers
   * is as at least much as the IPv6 header would have us expect.
   * Trim mbufs if longer than we expect.
   * Drop packet if shorter than we expect.
   */

  if (incoming->m_pkthdr.len < payload_len + sizeof(struct ipv6))
    {
      ipv6stat.ips_tooshort++;
      m_freem(incoming);
      return;
    }
  if (incoming->m_pkthdr.len > payload_len + sizeof(struct ipv6))
    if (!payload_len)
      jumbogram = 1;  /* We might have a jumbogram here! */
    else if (incoming->m_len == incoming->m_pkthdr.len)
      {
	incoming->m_len = payload_len + sizeof(struct ipv6);
	incoming->m_pkthdr.len = payload_len + sizeof(struct ipv6);
      }
    else m_adj(incoming,
	       (payload_len + sizeof(struct ipv6)) - incoming->m_pkthdr.len );

  /*
   * See if it's for me by checking list of i6a's.  I may want to convert
   * this into a routing lookup and see if rt->rt_ifp is loopback.
   * (Might be quicker. :)
   */

  for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
    if (IN6_ADDR_EQUAL(I6A_SIN(i6a)->sin6_addr,header->ipv6_dst) )
      break;

  /*
   * Check for inbound multicast datagram is for us.
   */

  if (i6a == NULL && IS_IN6_MULTICAST(header->ipv6_dst))
    {
#ifdef MROUTING
      /* Do IPv6 mcast routing stuff here. */
#endif
      DPRINTF(IDL_EVENT,("In multicast case.  Looking up..."));
      IN6_LOOKUP_MULTI(&(header->ipv6_dst),incoming->m_pkthdr.rcvif,in6m);
      if (in6m == NULL)
	{
	  DPRINTF(IDL_MAJOR_EVENT,("mcast lookup failed.\n"));
	  ipv6stat.ips_cantforward++;
	  m_freem(incoming);
	  return;
	}
      i6a = in6m->in6m_i6a;  /* We need to make i6a non-NULL because... */
      DPRINTF(IDL_EVENT,("succeeded.\n"));
      DDO(IDL_EVENT,if (i6a == NULL) panic("i6a == NULL"));
    }

  /* ...i6a will be non-null if we have a datagram bound for us. */

  if (i6a)
    {
      struct in6_hdrindex ihi[IPV6_MAXHDRCNT + 1];  /* Last one will have
						       ihi_mbuf == NULL.
						       This is a terminator. */
reasmloop:

      /*
       * Okay now we go forth and do all sorts of weirdness with 
       * "pre-parsing" (ugggh, the idea turns my stomach) the headers.
       * Hopefully the bletch up front will improve correctness and
       * avoid mbuf brain-damage further up the protocol graph.
       *
       * See ipv6_preparse() for rationales behind such an atrocity.
       */

      bzero(ihi, IPV6_MAXHDRCNT * sizeof(struct in6_hdrindex));
      if ((incoming = ipv6_preparse(incoming,ihi,NEXTHDR_IPV6)) == NULL)
	return;

      DDO(IDL_EVENT,printf("ihi table is:\n");dump_ihitab(ihi));

      /*
       * If there is a fragmentation header in the packet, go to
       * reassembly.  This means no hop-by-hop options will be handled
       * until the IPv6 datagram is reassembled.
       */
      {
	int i;
	for (i = 0; (ihi[i].ihi_mbuf) && (ihi[i].ihi_nexthdr != 
		NEXTHDR_FRAGMENT); i++);
	if (ihi[i].ihi_nexthdr == NEXTHDR_FRAGMENT) {
          if (jumbogram)
	    {
	      m_freem(incoming);
	      return;
	    }
	  ipv6stat.ips_fragments++;
	  if (!(incoming = ipv6_reasm(incoming, 0, ihi, i)))
	    return;
	  /*
	   * If ipv6_reasm() either returns NULL or a reassembled IPv6
	   * datagram.  If reassembled, jump back to preparse.
	   */
	  ipv6stat.ips_reassembled++;
	  goto reasmloop;
	}
      }

      nexthdr = (mtod(incoming, struct ipv6 *))->ipv6_nexthdr;

      DPRINTF(IDL_EVENT, ("nexthdr = %d.\n", nexthdr));

      DPRINTF(IDL_EVENT, ("Calling protoswitch for %d\n", nexthdr));

      ipv6stat.ips_delivered++;
      (*inet6sw[ipv6_protox[nexthdr]].pr_input)
	(incoming, sizeof(struct ipv6) + extra,ihi,1);

      DPRINTF(IDL_EVENT, ("Back from protoswitch for %d\n", nexthdr));
      return;
    }

  if (ipv6forwarding == 0)
    {
      ipv6stat.ips_cantforward++;
      m_freem(incoming);
      return;
    }
  else
    {
      /*
       * Perform hop-by-hop options, if present.  Dest opt. bags, and source
       * routes SHOULD be handled by previous protosw call.
       */
      ipv6_forward(incoming, 0);
    }
}

/*----------------------------------------------------------------------
 * Advance the current scanning data pointer, and mbuf pointer.  This is
 * a support routine for ipv6_preparse().  Returns new data pointer.
 ----------------------------------------------------------------------*/

static u_char *
advance(curscan,curoff,dptr,howfar)
     struct mbuf **curscan;	/* Current mbuf ptr. */
     int *curoff;               /* Current offset into mbuf. */
     u_char *dptr;              /* Current data pointer. */
     int howfar;                /* How many bytes to advance. */
{
  struct mbuf *m = *curscan;
  u_int offset = *curoff, advanced = 0;
  u_char *rc;

  DPRINTF(IDL_GROSS_EVENT,("Entering advance.\n"));
  DPRINTF(IDL_GROSS_EVENT,\
	  ("curscan = 0x%x, dptr = 0x%x, howfar = %d\n",\
	   curscan,dptr,howfar));
  DDO(IDL_GROSS_EVENT,printf("*curscan is:\n");dump_mbuf(*curscan));

  if ((howfar + offset) < m->m_len)
    {
      /*
       * Easy case, I'm still in the current mbuf.
       */
      DPRINTF(IDL_GROSS_EVENT,("In easy case.\n"));
      rc = dptr + howfar;
      *curoff += howfar;
    }
  else
    {
      /*
       * Hard case, I have to advance one or more mbufs.
       */
      DPRINTF(IDL_GROSS_EVENT,("In hard case.\n"));
      howfar -= (m->m_len - offset);
      m = m->m_next;
      while (m->m_len < howfar)
	{
	  howfar -= m->m_len;
	  m = m->m_next;
	}
      rc = (u_char *)(m->m_data + howfar);
      *curoff = howfar;
    }

  DPRINTF(IDL_GROSS_EVENT,("About to return.\n"));

  *curscan = m;
  return rc;
}

/*----------------------------------------------------------------------
 * Gather data from scanned mbuf chaing into a single mbuf.
 ----------------------------------------------------------------------*/

static struct mbuf *
gather(chain,startoff,howmuch,pkthdr)
     struct mbuf *chain;	/* First source mbuf. */
     int startoff;              /* Starting copy from where in first mbuf. */
     int howmuch,pkthdr;        /* How much to copy, and whether or not
				   the returned mbuf should be a packet hdr. */
{
  struct mbuf *rc;

  DPRINTF(IDL_GROSS_EVENT,("Entering gather.\n"));

  if (pkthdr)
    {
      MGETHDR(rc,M_NOWAIT,MT_DATA);
    }
  else
    {
      MGET(rc,M_NOWAIT,MT_DATA);
    }

  DPRINTF(IDL_GROSS_EVENT,("After MGET{,HDR}.\n"));

  if (rc == NULL)
    return NULL;

  rc->m_next = NULL;
  if (howmuch > ((pkthdr)?MHLEN:MLEN))
    if (howmuch > MCLBYTES)
      {
	panic("Oh hell, howmuch (%d) is more than MCLBYTES (%d).\n",howmuch,
	      MCLBYTES);
	/*
	 * Instead of panicking, we should be putting in code to place
	 * the data into multple mbuf chains.
	 */
      }
    else
      {
	MCLGET(rc,M_NOWAIT);
	if (!(rc->m_flags & M_EXT))
	  (void) m_free(rc);
      }

  DPRINTF(IDL_GROSS_EVENT,("After possible cluster get.\n"));

  rc->m_len = howmuch;
  m_copydata(chain,startoff,howmuch,rc->m_data);
  if (pkthdr)
    {
      /*
       * Make sure the preparsed datagram has all of the flags from the
       * original datagram.
       */
      rc->m_flags |= (chain->m_flags & M_COPYFLAGS);
    }

  DPRINTF(IDL_GROSS_EVENT,("After m_copydata.\n"));

  return rc;
}

/*----------------------------------------------------------------------
 * The following function, ipv6_preparse, does a lot of copying, which is
 * a fundementally BAD IDEA in network software.  Unfortunately, to preserve
 * correctness, and to get certain IPv6 options implemented
 * quickly, this function was written.
 *
 * What it does is that every IPv6 header get its own contiguous-data
 * mbuf, and updates a "header index" list which points to the different
 * mbufs.  This way, m_pullup's are saved (theoretically) in IPv6 options
 * processing code.  This also allows ipv6_stripoptions() (see below) to be
 * nothing more than a few pieces of pointer arithmetic.
 *
 * Unfortunately, TCP and UDP call m_pullup anyway, so I lose, especially
 * when the packet is just TCP/IPv6.  In a longer run, what this function
 * might want to do is create the header index, but without copying any data.
 * (Still, scanning all of that data might still be annoying.  Ugggh!)
 *
 * Worse still, other functions beside ipv6_input() call this, often because
 * of packet decapsulation.
 *
 * If firsthdr is NEXTHDR_IPV6, then I assume I got an m_pkthdr, and
 * do extra pkthdr cruft. 
 ----------------------------------------------------------------------*/

struct mbuf *
ipv6_preparse(dgram,ihi,firsthdr)
     struct mbuf *dgram;
     struct in6_hdrindex *ihi;
     int firsthdr;
{
  u_int8 nexthdr = firsthdr;
  struct mbuf *retchain = NULL,*lastone,*curscan,*newbie;
  u_char *dptr;
  int sofar = 0,done = 0, tocopy, i = 0, totallen, curoff = 0;
  int firsttime = ((firsthdr == NEXTHDR_IPV6) || (firsthdr == NEXTHDR_IPV4));
  struct ifnet *ifp;

  if (dgram->m_flags & M_PKTHDR)
    {
      ifp = dgram->m_pkthdr.rcvif;
      totallen = dgram->m_pkthdr.len;
    }

  DPRINTF(IDL_GROSS_EVENT,("Entering ipv6_preparse.\n"));

  /*
   * ASSUMPTIONS:
   *       I can detect the length of an option with the first 8 bytes
   *       in my possession.
   *
   * It would be nice, however, to have some sort of protoswitch entry
   * such that I can figure out length and such.  Without that, this'll have
   * a switch statement and processing for A TON of entries.
   */

  curscan = dgram;
  dptr = (u_char *)dgram->m_data;
  curoff = 0;

  do
    {
#define MINOPTLEN 8
      u_char tbuf[MINOPTLEN],*tptr = tbuf;

      if ( curscan->m_len - curoff < MINOPTLEN)
	{
	  int remaining = MINOPTLEN - curscan->m_len + curoff;
	  struct mbuf *gross = curscan->m_next;

	  /*
	   * I can't even find 8 bytes of contiguous data to START with.
	   * Looks like I can't do things without copying early.
	   * Pardon me while I lose my lunch.
	   */
	  DPRINTF(IDL_CRITICAL,("ipv6_preparse() in copying-off case.\n"));
	  bcopy(dptr,tptr,curscan->m_len - curoff);
	  while (remaining > 0)
	    {
	      if (gross == NULL)
		{
		  DPRINTF(IDL_CRITICAL,\
			  ("Not enough data for MINOPTLEN, dumping"));
		  m_freem(retchain);  /* m_freem() handles NULLs well */
		  m_freem(dgram);
		  return NULL;
		}
	      bcopy(gross,tptr+(MINOPTLEN - remaining),
		    min(remaining,gross->m_len));
	      remaining -= min(remaining,gross->m_len);
	      gross = gross->m_next;
	    }
	}
      else tptr = (u_char *)dptr;

      /*
       * This if...panic might need to be commented out.  If not, tune it
       * for your (32/64) bit system.
       */
      if ((u_int)tptr & 0x3)  /* i.e. not divisible by 4 */
	panic("Pathological alignment of tptr (0x%x)!",tptr);

      ihi[i].ihi_nexthdr = nexthdr;

      /*
       * Each case of this switch should set tocopy, i.e. the number of
       * bytes which need to be copied.
       * At this point, tptr points to enough data (MINOPTLEN bytes)
       * to determine total option length.
       */
      DPRINTF(IDL_GROSS_EVENT,("Nexthdr is %d... ",nexthdr));

      switch (nexthdr)
	{
	case NEXTHDR_IPV6:
	  {
	    struct ipv6 *ipv6 = (struct ipv6 *)tptr;

	    tocopy = sizeof(struct ipv6);
	    nexthdr = ipv6->ipv6_nexthdr;
	    /*
	     * "firsttime" magic is because my first header is
	     * an IPv6 header, but I then may have IPv6-in-IPv6, so I want
	     * to stop after the first time.
	     */
	    done = !firsttime;
	    firsttime = 0;
	  }
	  break;
	case NEXTHDR_IPV4:
	  {
	    struct ip *ip = (struct ip *)tptr;
	    
	    tocopy = ip->ip_hl << 2;
	    nexthdr = ip->ip_p;
	    done = !firsttime;
	    firsttime = 0;
	  }
	  break;
	case NEXTHDR_HOP:
	case NEXTHDR_DEST:
	  {
	    struct ipv6_opthdr *oh = (struct ipv6_opthdr *)tptr;

	    tocopy = 8 + 8*oh->oh_extlen;
	    nexthdr = oh->oh_nexthdr;
	  }
	  break;

	case NEXTHDR_ROUTING:
	  {
	    struct ipv6_srcroute0 *i6sr = (struct ipv6_srcroute0 *)tptr;

	    tocopy = sizeof(struct ipv6_srcroute0) +
	                   (i6sr->i6sr_numaddrs - 1)*sizeof(struct in_addr6);
	    nexthdr = i6sr->i6sr_nexthdr;
	  }
	  break;

	case NEXTHDR_AUTH:
	  {
	    struct ipv6_auth *auth = (struct ipv6_auth *)tptr;

	    tocopy = sizeof(struct ipv6_auth) + (auth->auth_datalen  << 2);
	    nexthdr = auth->auth_nexthdr;
	  }
	  break;
	  
	default:
	  tocopy = 8;    /* Perhaps make this bigger? */
	  done = 1;
	  break;
	}

      DPRINTF(IDL_GROSS_EVENT,("tocopy == %d.\n",tocopy));

      /*
       * Put header into its own mbuf.
       */
      newbie = gather(curscan,curoff,tocopy,
		      (retchain == NULL && (dgram->m_flags & M_PKTHDR)) );

      if (newbie == NULL)
	{
	  DPRINTF(IDL_CRITICAL,("gather() failed in ipv6_preparse().\n"));
	  m_freem(retchain);	
	  m_freem(dgram);
	  return NULL;
	}

      /*
       * Update header index.
       */
      ihi[i].ihi_mbuf = newbie;
      i++;

      sofar += tocopy;
      if (retchain == NULL)
	{
	  retchain = newbie;
	  lastone = retchain;
	}
      else
	{
	  lastone->m_next = newbie;
	  /* In do-while loop in case gather starts returning chains. */
	  do
	    lastone = lastone->m_next;
	  while (lastone->m_next);
	}
      dptr = advance(&curscan,&curoff,dptr,tocopy);
      DPRINTF(IDL_GROSS_EVENT,("curscan is 0x%x, curoff is %d.\n",curscan, curoff));
    }
  while (!done);


  DDO(IDL_GROSS_EVENT,printf("Scanned chain is:\n");dump_mchain(dgram));
  DPRINTF(IDL_GROSS_EVENT,\
	  ("Before m_split(), curscan = 0x%x, curoff = %d.\n",curscan,curoff));

  /*
   * Now that I have preparsed the headers, split original chain, and append
   * it to the end of the preparsed-header chain.
   *
   * Notice how the split happens at the CURRENT mbuf, so m_split doesn't
   * have to do all that much work.
   */

  lastone->m_next = m_split(curscan,curoff,M_NOWAIT);
  if (lastone->m_next == NULL)
    {
      DPRINTF(IDL_CRITICAL,("m_split() failed in ipv6_preparse().\n"));
      m_freem(retchain);
      m_freem(dgram);
      return NULL;
    }
  lastone->m_next->m_flags &= ~M_PKTHDR;

  if (dgram->m_flags & M_PKTHDR)
    {
      if (!(retchain->m_flags & M_PKTHDR))
	panic("Consistency failure.");
      retchain->m_pkthdr.len = totallen;
      retchain->m_pkthdr.rcvif = ifp;
    }

  m_freem(dgram);  

  return retchain;
}

/*----------------------------------------------------------------------
 * IPv6 reassembly code.  Returns an mbuf chain if the fragment completes
 * a message, otherwise it returns NULL.
 *
 * Assumptions: 
 * * The IPv6 header is at the beginning of the incoming structure 
 *   and has already been pulled up. I don't expect this to be
 *   a problem.
 * * Fragments don't overlap (or, if they do, we can discard them).
 *   They can be duplicates (which we drop), but they cannot overlap.
 *   If we have IPv4->IPv6 header translation, this assumption could
 *   be incorrect because IPv4 has intermediate fragmentation (which
 *   you have to have in order for fragments to overlap). This is yet
 *   another reason why header translation is a Bad Thing.
 * * The slowtimeo()-based routine that frees fragments will not
 *   get called during the middle of this routine. This assumption
 *   seems to be made in the IPv4 reassembly code. How this is so
 *   actually so I have yet to discover. Requiring splnet() or some
 *   sort of resource arbitration to handle this assumption being
 *   incorrect could be a major hassle.
 *
 * Other comments:
 *
 * * m_split(), a valuable call that gets no press, does much magic here.
 * * Security against denial-of-service attacks on reassembly cannot be
 *   provided.
 * * frag_id is more or less used as a 32-bit cookie. We don't need to
 *   htonl() it, because it's either equal or it's not, no matter what
 *   byte order you use.
 ----------------------------------------------------------------------*/

struct mbuf *
ipv6_reasm(incoming, extra, ihi, ihioff)
     struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *ihi;
     int ihioff;
{
  struct ipv6_fraghdr *fraghdr;
  struct mbuf *fraghdr_mbuf;
  struct ipv6_fragment *fragment, *fragmentprev;
  struct mbuf *m, *pm;

  DPRINTF(IDL_FINISHED, ("1"));
  DPRINTF(IDL_GROSS_EVENT, ("incoming = %08x\n", incoming));

  if (!ihioff) {
    m_freem(incoming);
    return NULL;
  }

  /*
   * Split chain so that fraghdr_mbuf points to the fragment header and
   * beyond.
   */
  fraghdr_mbuf = ihi[ihioff].ihi_mbuf;
  ihi[ihioff - 1].ihi_mbuf->m_next = NULL;

  /*
   * Since we don't have a valid value for extra, we have to go and do this
   * the hard way. We stash this information for future use. 
   */
  for (extra = 0, m = fraghdr_mbuf->m_next; m; extra += m->m_len, 
	m = m->m_next);

  /*
   * Notice that the length of this fragment is stored in the 2nd mbuf's
   * m_nextpkt field.  (Not the first one, mind you.)  See later in this
   * function for where this gets used.
   */
  DPRINTF(IDL_GROSS_EVENT, ("extra = %d\n", extra));
  fraghdr_mbuf->m_next->m_nextpkt = (struct mbuf *)extra;

  fraghdr = mtod(fraghdr_mbuf, struct ipv6_fraghdr *);
  fraghdr->frag_bitsoffset = htons(fraghdr->frag_bitsoffset);

  DPRINTF(IDL_GROSS_EVENT,\
	  ("bitsoffset = %04x, id = %08x, nexthdr = %d\n",\
	   fraghdr->frag_bitsoffset, fraghdr->frag_id, fraghdr->frag_nexthdr));
  DPRINTF(IDL_FINISHED, ("6"));

  /*
   * Locate reassembly queue for incoming fragment.
   */

  {
    struct ipv6 *ipv6i, *ipv6f;

    DPRINTF(IDL_GROSS_EVENT, ("incoming=%08x\n", incoming));
    DDO(IDL_GROSS_EVENT, dump_mbuf_hdr(incoming));

    ipv6i = mtod(incoming, struct ipv6 *);

    DPRINTF(IDL_GROSS_EVENT, ("ipv6_fragmentq = %08x, ipv6i = %08x\n", \
			      ipv6_fragmentq, ipv6i));

    for (fragmentprev = NULL, fragment = ipv6_fragmentq; fragment; 
	 fragmentprev = fragment, fragment = fragment->next) {

      DPRINTF(IDL_GROSS_EVENT, ("!"));
      DPRINTF(IDL_GROSS_EVENT, ("fragment = %08x\n", fragment));
      DPRINTF(IDL_GROSS_EVENT, ("fragment->prefix = %08x\n",\
				fragment->prefix));
      DDO(IDL_GROSS_EVENT, if (fragment->prefix) \
	                     dump_mbuf_hdr(fragment->prefix));

      ipv6f = mtod(fragment->prefix, struct ipv6 *);

      DPRINTF(IDL_GROSS_EVENT, ("ipv6f = %08x\n", ipv6f));

      if (IN6_ADDR_EQUAL(ipv6i->ipv6_src, ipv6f->ipv6_src)) {
	if (IN6_ADDR_EQUAL(ipv6i->ipv6_dst, ipv6f->ipv6_dst)) {
	  register struct ipv6_fraghdr *fhf = mtod(fragment->data,\
						   struct ipv6_fraghdr *);

	  if ((fraghdr->frag_id == fhf->frag_id) &&
	      (fraghdr->frag_nexthdr == fhf->frag_nexthdr))
	    break;
	}
      }
    }
  }

  DPRINTF(IDL_FINISHED, ("7"));

  if (!fragment) {
    /* 
     * Create a new fragment queue entry - this one looks new.
     *
     * Notice that the order of insertion is such that the newest queues
     * are at the head of the one-way list.  The entry aging code takes
     * advantage of this.
     */
    DPRINTF(IDL_FINISHED, ("8"));

    MALLOC(fragment, struct ipv6_fragment *, sizeof(*fragment), M_FRAGQ,
	   M_NOWAIT);
    if (!fragment) {
      DPRINTF(IDL_MAJOR_EVENT, ("MALLOC failed."));
      goto reasm_cleanup;
    };
    fragment->prefix = incoming;
    incoming->m_pkthdr.len -= extra;
    incoming->m_pkthdr.len -= sizeof(struct ipv6_fraghdr);
    fragment->ttl = 60; /* 30 seconds */
    fraghdr_mbuf->m_nextpkt = NULL;
    fragment->data = fraghdr_mbuf;
    fragment->next = ipv6_fragmentq;
    fragment->flags = (~fraghdr->frag_bitsoffset) & 1;
    ipv6_fragmentq = fragment;
    DPRINTF(IDL_FINISHED, ("9"));
    return NULL;
  }

  /*
   * If two packets have claimed to be the beginning or the end, we don't
   * know which is right. The easiest solution is to drop this packet.
   */

  DPRINTF(IDL_FINISHED, ("b"));

  if (!(fraghdr->frag_bitsoffset & 1))
    if (fragment->flags & 1)  /* i.e. we already have the end... */
      {
	/*
	 * Duplicate (ending) fragment.
	 */
	DPRINTF(IDL_MAJOR_EVENT, ("Got a dupe/overlap fragment"));
	goto reasm_cleanup;
      }
    else fragment->flags |= 1;

  DPRINTF(IDL_FINISHED, ("c"));

  if (!(fraghdr->frag_bitsoffset & 0xFFF8)) {
    /*
     * We want to end up with the part before the frag header for the packet
     * at offset zero. (RFC1123?)
     */
    if (!((mtod(fragment->data, struct ipv6_fraghdr *))->frag_bitsoffset & 
	  0xFFF8)) {
      /*
       * Duplicate (initial) packet.
       */
      DPRINTF(IDL_MAJOR_EVENT, ("Got a dupe/overlap fragment"));
      goto reasm_cleanup;
    } else {
    	m_freem(fragment->prefix);
    	fragment->prefix = incoming;
	incoming->m_pkthdr.len -= extra;
	incoming->m_pkthdr.len -= sizeof(struct ipv6_fraghdr);
    }
  } else
    m_freem(incoming); /* Which only contains pre-fragment headers, which
			  are useless since we have them hanging off
			  fragment->prefix. */

  {
    struct mbuf *hm[3], *dm[3];
    int i;

    {
      struct mbuf *m, *pm;

      /*
       * Find where this fragment fits in.
       */
      for (pm = NULL, m = fragment->data; m; pm = m, m = m->m_nextpkt) {

	DPRINTF(IDL_FINISHED, ("is %d > %d?...", \
		       (mtod(m, struct ipv6_fraghdr *))->frag_bitsoffset & \
			       0xFFF8, fraghdr->frag_bitsoffset & 0xFFF8));

	if (((mtod(m, struct ipv6_fraghdr *))->frag_bitsoffset & 0xFFF8) > 
	    (fraghdr->frag_bitsoffset & 0xFFF8)) {
	  DPRINTF(IDL_GROSS_EVENT, ("yes!\n"));
	  break;
	}

	DPRINTF(IDL_GROSS_EVENT, ("no\n"));

	if ((mtod(m, struct ipv6_fraghdr *))->frag_bitsoffset ==
	    fraghdr->frag_bitsoffset) {
	  /*
	   * Duplicate fragment.
	   */
	  DPRINTF(IDL_MAJOR_EVENT, ("Got a dupe/overlap fragment"));
	  goto reasm_cleanup;
	}
      }

      DPRINTF(IDL_FINISHED, ("d"));
      DPRINTF(IDL_GROSS_EVENT, ("m = %08x, pm = %08x\n", m, pm));
      
      /*
       * Right here, pm will contain the preceeding fragment to the incoming
       * one, and m will contain the succeeding fragment to the incoming one.
       *
       * This is somewhat non-obvious. hm[] is a vector of pointers to the
       * mbufs containing ipv6_fraghdrs and dm[] is a vector of pointers to
       * the mbufs at the head of each of their associated data lists.  [0]
       * is the mbuf in the main chain to the immediate left of where our
       * new data should go, [1] is our new data, and [2] is the mbuf in
       * the main chain to the immediate right of where our new data should
       * go. One of [0] or [2] may be NULL.
       *
       * Each dm[n]->m_nextpkt will have that fragment's length stored.
       *
       * The reason why we do this is so we can bubble together the [0] and
       * [1] elements if possible, make the [1] = [0] if we do, then we can
       * bubble the [1] and the [2] together and it'll do the right thing.
       * We could theoretically do this for the rest of the list except that
       * it is made deliberately unecessary (we bubble on insertion until we
       * have a known-done big bubble so we don't have to do a O(N/2)
       * rescan of the list every time just to figure out whether or not
       * we're done.
       *
       * This seems really ugly, but it does the job and it may even be
       * somewhat efficient. 
       */
      
      hm[0] = pm;		dm[0] = pm ? pm->m_next : NULL;
      hm[1] = fraghdr_mbuf;	dm[1] = fraghdr_mbuf->m_next;
      hm[2] = m;		dm[2] = m  ? m->m_next  : NULL;
      
      if (!pm) {
	fraghdr_mbuf->m_nextpkt = fragment->data;
	fragment->data = fraghdr_mbuf;
      } else {
	pm->m_nextpkt = fraghdr_mbuf;
	fraghdr_mbuf->m_nextpkt = m;
      }
      
    }

    DPRINTF(IDL_FINISHED, ("e"));
    for (i = 0; i < 2; i++) {
      if (!hm[i] || !dm[i] || !hm[i+1] || !dm[i+1])
	continue;
      DPRINTF(IDL_FINISHED, ("f"));
      DPRINTF(IDL_GROSS_EVENT,\
	  ("i = %d, offset1 = %d, len = %d, offset1+len = %d, offset2 = %d\n",\
	   i, (mtod(hm[i], struct ipv6_fraghdr *))->frag_bitsoffset & 0xFFF8,\
	   (int)(dm[i]->m_nextpkt),\
	   ((mtod(hm[i], struct ipv6_fraghdr *))->frag_bitsoffset & 0xFFF8)\
	   + (int)(dm[i]->m_nextpkt),\
	   (mtod(hm[i+1], struct ipv6_fraghdr *))->frag_bitsoffset & 0xFFF8));

      if ((((mtod(hm[i], struct ipv6_fraghdr *))->frag_bitsoffset 
	    & 0xFFF8) + (int)(dm[i]->m_nextpkt)) >
	  ((mtod(hm[i+1], struct ipv6_fraghdr *))->frag_bitsoffset 
	   & 0xFFF8)) {
	/*
	 * Overlapping fragment.
	 */
	DPRINTF(IDL_MAJOR_EVENT, 
		("Got a dupe/overlap fragment"));
	goto reasm_cleanup;
      }
      
      if ((((mtod(hm[i], struct ipv6_fraghdr *))->frag_bitsoffset 
	    & 0xFFF8) + ((int)dm[i]->m_nextpkt)) == 
	  ((mtod(hm[i+1], struct ipv6_fraghdr *))->frag_bitsoffset 
	   & 0xFFF8)) {
	/*
	 * If the fragments are contiguous, bubble them together:
	 * Combine the data chains and increase the appropriate
	 * chain data length. The second fragment header is now
	 * unnecessary.
	 */
	DPRINTF(IDL_GROSS_EVENT, ("Bubbling two frags!"));
	
	if (!((mtod(hm[i+1], struct ipv6_fraghdr *))->frag_bitsoffset & 1)){
	  DPRINTF(IDL_FINISHED, ("$"));
	  (mtod(hm[i], struct ipv6_fraghdr *))->frag_bitsoffset &= (~1);
	}

	DDO(IDL_GROSS_EVENT, dump_mbuf_hdr(hm[i]));
	DDO(IDL_GROSS_EVENT, dump_mbuf_hdr(hm[i+1]));
	DDO(IDL_GROSS_EVENT, dump_mbuf_hdr(dm[i]));
	DDO(IDL_GROSS_EVENT, dump_mbuf_hdr(dm[i+1]));

	for (pm = NULL, m = dm[i]; m; pm = m, m = m->m_next);

	pm->m_next = dm[i+1];
	
	DPRINTF(IDL_FINISHED, ("dm[%d]->m_nextpkt = %d\n", i, 
			       (int)dm[i]->m_nextpkt));
	DPRINTF(IDL_FINISHED, ("dm[%d]->m_nextpkt = %d\n", i+1, 
			       (int)dm[i+1]->m_nextpkt));
	/*
	 * Update newly merged fragment's length.  Notice it's stored
	 * at the dm[i]'s m_nextpkt, which is the 2nd mbuf on that chain.
	 * (hm[i] is the 1st.)
	 */
	dm[i]->m_nextpkt = (struct mbuf *)((int)(dm[i]->m_nextpkt) + 
					   (int)(dm[i+1]->m_nextpkt));
	DPRINTF(IDL_FINISHED, ("dm[%d]->m_nextpkt = %d\n", i, 
			       (int)dm[i]->m_nextpkt));
	hm[i]->m_nextpkt = hm[i+1]->m_nextpkt;
	
	m_free(hm[i+1]);
	if (!i) {
	  hm[i+1] = hm[i];
	  dm[i+1] = dm[i];
	};
	DDO(IDL_GROSS_EVENT, dump_mbuf_hdr(hm[i]));
	DDO(IDL_GROSS_EVENT, dump_mbuf_hdr(dm[i]));
      }
    }
  }

  DPRINTF(IDL_GROSS_EVENT, ("g"));
  
  /*
   * Now, the moment of truth. Do we have a packet? 
   * To be done, we have to have only one packet left in the queue now that
   * we've bubbled together (i.e., one complete packet), have it at offset
   * zero (i.e., there's nothing before it), and not have its more bit set
   * (i.e., there's nothing after it). If we meet these conditions, we are
   * DONE! 
   *
   * Remember when we htons()ed frag_bitsoffset? If we're done, it contains
   * a zero. I don't know of any architecture in which a zero in network
   * byte order is a zero in host byte order, do you? 
   */

  DPRINTF(IDL_GROSS_EVENT, ("fragment->data->m_nextpkt = %04x\n", 
			    fragment->data->m_nextpkt));

  if (!fragment->data->m_nextpkt) {
    DPRINTF(IDL_GROSS_EVENT, ("bitsoffset = %04x\n", 
	    (mtod(fragment->data, struct ipv6_fraghdr *))->frag_bitsoffset));

    if (!(mtod(fragment->data, struct ipv6_fraghdr *))->frag_bitsoffset) {
      struct mbuf *m;
      u_int16 nexthdr = (mtod(fragment->data, struct ipv6_fraghdr *))->
	frag_nexthdr;
      int extra;

      DPRINTF(IDL_FINISHED, ("h"));

      if (fragmentprev)
	fragmentprev->next = fragment->next;
      else
	ipv6_fragmentq = fragment->next;
      DPRINTF(IDL_GROSS_EVENT, ("nexthdr = %d\n", nexthdr));

     /*
      * Pain time.
      * 
      * The fragmentation header must be removed. This requires me to rescan
      * prefix, going through each header until I figure out where the last
      * header before the fragmentation header is. Then I set that header's
      * next header to the fragmentation header's next header.
      *
      * N.B. that this means people adding new random header processing code
      * to this IPv6 implementation need to make the appropriate mods below.
      * Failure to do so will really hose you if your header appears before
      * a fragment header. It is a Good Thing to mod the code below even if
      * you don't *think* it will ever appear before a fragment header, just
      * because it *could*.
      *
      * This is really annoying and somewhat expensive. On the other hand,
      * it might prove itself to be yet another reason for higher level
      * protocols to work at avoiding fragmentation where possible.
      *
      * An assumption is made here that all of the headers before this one
      * are pulled up and contiguous. This is guaranteed by having
      * ipv6_preparse called before this function. We'd just use the map
      * from preparse, but we can't be guaranteed that the map prefixing
      * this fragment is the same as the map prefixing the fragment at
      * offset zero, even though they will be the same 99% of the time.
      */
      
      {
	int totaloff = 0, off = 0;
	char *data;
	u_int8 itype = NEXTHDR_IPV6, *type = &itype;
	
	m = fragment->prefix;
	do {
	  DPRINTF(IDL_FINISHED, ("foo"));
	  while ((off > 0) && m) {
	    if (off >= m->m_len) {
	      off -= m->m_len;
	      m = m->m_next;
	    }
	  }
	  DPRINTF(IDL_FINISHED, ("bar"));
	  data = (mtod(m, caddr_t)) + off;
	  switch(*type) {
	  case NEXTHDR_IPV6:
	    {
	      struct ipv6 *ipv6 = (struct ipv6 *)data;

	      DPRINTF(IDL_FINISHED, ("baz"));

	      type = &(ipv6->ipv6_nexthdr);
	      off  = sizeof(struct ipv6);

	      DPRINTF(IDL_FINISHED, ("kwy"));

	    }
	    break;
	  case NEXTHDR_TCP:
	  case NEXTHDR_UDP:
	  case NEXTHDR_ICMP:
	    DPRINTF(IDL_GROSS_EVENT, ("Transport before fragment? I don't think so."));
	    goto finish_cleanup;
	  case NEXTHDR_ROUTING:
	  case NEXTHDR_ESP:
	  case NEXTHDR_AUTH:
	  case NEXTHDR_DEST:
	  default:
	    DPRINTF(IDL_GROSS_EVENT, ("Got a header that ipv6_reasm doesn't know how to handle.\n"));
	    goto finish_cleanup;
	  case NEXTHDR_FRAGMENT:
	    DPRINTF(IDL_GROSS_EVENT, ("This code never reached."));
	    goto finish_cleanup;
	  }
	} while (*type != NEXTHDR_FRAGMENT);
	DPRINTF(IDL_FINISHED, ("muf"));
	*type = nexthdr;
      }

      DPRINTF(IDL_GROSS_EVENT, ("Got the previous header"));
      
      fragment->data = m_free(fragment->data);
      
      while(m->m_next) 
	m = m->m_next;
      
      DPRINTF(IDL_GROSS_EVENT, ("m = %08x\n", m));
      m->m_next = fragment->data;
      m = fragment->prefix;
      DPRINTF(IDL_GROSS_EVENT, ("m->m_pkthdr.len = %d, fragment->data->m_nextpkt = %d\n", m->m_pkthdr.len, (int)fragment->data->m_nextpkt));
      m->m_pkthdr.len += (int)(fragment->data->m_nextpkt);
      FREE(fragment, M_FRAGQ);
      if (m->m_pkthdr.len > 0xffff) {
        goto finish_cleanup;
      };
      DPRINTF(IDL_FINISHED, ("i"));

      (mtod(m, struct ipv6 *))->ipv6_length = 
	(m->m_pkthdr.len - sizeof(struct ipv6) > 0xffff) ? 0 :
	  htons(m->m_pkthdr.len - sizeof(struct ipv6));
      DPRINTF(IDL_GROSS_EVENT, ("m->m_pkthdr.len = %d", m->m_pkthdr.len));
      return m;
    }
  }
  DPRINTF(IDL_FINISHED, ("j"));
  
  return NULL;

 reasm_cleanup:

  DPRINTF(IDL_FINISHED, ("k"));

  m_freem(fraghdr_mbuf);

  if (incoming)
    m_freem(incoming);

  return NULL;

finish_cleanup:
  DPRINTF(IDL_FINISHED, ("l"));

  m_freem(fragment->data);
  m_freem(fragment->prefix);

  FREE(fragment, M_FRAGQ);

  return NULL;
};

/*----------------------------------------------------------------------
 * IPv6 forwarding engine.  Just discards packets for now.
 ----------------------------------------------------------------------*/

void
ipv6_forward(outbound, srcrt)
     struct mbuf *outbound;
     int srcrt;
{
  m_freem(outbound);
}

/*----------------------------------------------------------------------
 * IPv6 hop-by-hop option handler.
 ----------------------------------------------------------------------*/

void
ipv6_hop(incoming, extra, index, indexoff)
     struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *index;
     u_int8 indexoff;
{
  struct ipv6 *header;
  struct ipv6_opthdr *hopopt;
  struct ipv6_option *option;
  u_int8 *tmp;

  hopopt = mtod(index[indexoff].ihi_mbuf, struct ipv6_opthdr *);

  tmp = hopopt->oh_data;
  /*
   * Slide the char pointer tmp along, parsing each option in the "bag" of
   * hop-by-hop options.
   */
  while (tmp < hopopt->oh_data + (hopopt->oh_extlen << 2))
    {
      option = (struct ipv6_option *)tmp;
      switch (option->opt_type)
	{
	case OPT_PAD1:
	  tmp++;
	  break;
	case OPT_PADN:
	  tmp += option->opt_datalen + 2;
	  break;
	case OPT_JUMBO:
	  tmp += 2;
	  /*
	   * Confirm that the packet header field matches the jumbogram size.
	   */
	  if (incoming->m_pkthdr.len != ntohl(*(u_int32 *)tmp) + sizeof(*header))
	    {
	      /*
	       * For now, bail.  Add instrumenting code here, too.
	       */
	      m_freem(incoming);
	      return;
	    }
	  break;
	default:
	  /*
	   * Handle unknown option by taking appropriate action based on
	   * high bit values.  With this code, this first non-skipping
	   * unknown option will cause the packet to be dropped.
	   */
	  switch (option->opt_type & 0xC0)
	    {
	    case 0:    /* Skip over */
	      tmp += option->opt_datalen + 2;
	      break;
	    case 0xC0: /* Only if not multicast... */
	      if (IS_IN6_MULTICAST(header->ipv6_dst))
		/* FALLTHROUGH */
	    case 0x80: /* Send ICMP Unrecognized type. */
		{
		  /* Issue ICMP parameter problem. */
		  ipv6_icmp_error(incoming,ICMPV6_PARAMPROB,
		   ICMPV6_PARAMPROB_BADOPT,
		   (u_int32)option - (u_int32)hopopt->oh_data + 
				       sizeof(struct ipv6));
		}
	      return;  /* incoming has already been freed. */

	    case 0x40: /* Discard packet */
	      m_freem(incoming);
	      return;
	    }
	}
    }
  DPRINTF(IDL_GROSS_EVENT, ("ipv6_hop calling protoswitch for %d\n",\
			    index[indexoff].ihi_nexthdr));

  indexoff++;
  (*inet6sw[ipv6_protox[index[indexoff].ihi_nexthdr]].pr_input)
        (incoming, extra + sizeof(struct ipv6_opthdr) + 
	(hopopt->oh_extlen << 2), index, indexoff);
    
  DPRINTF(IDL_GROSS_EVENT, ("Leaving ipv6_hop\n"));
}


/*----------------------------------------------------------------------
 * If timers expires on reassembly queues, discard them.
 * Also update any discovery queues.
 ----------------------------------------------------------------------*/

void
ipv6_slowtimo()
{
  struct ipv6_fragment *fragment, *fragmentprev;
  int s = splnet();

  /*
   * Age reasssembly fragments.
   *
   * (Since fragments are inserted at the beginning of the queue, once we've
   * found the first timed-out fragment, we know that everything beyond is
   * also timed-out since it must be older.)
   *
   */
  for (fragmentprev = NULL, fragment = ipv6_fragmentq; fragment; 
       fragmentprev = fragment, fragment = fragment->next)
    if (fragment->ttl <= 1)
      break;
    else
      fragment->ttl--;

  if (fragment)
    {
      struct mbuf *m, *m2;

      if (fragmentprev)
	fragmentprev->next = NULL;
      else
	ipv6_fragmentq = NULL;
      
      /*
       * This loop does most of the work and doesn't require splnet()...?
       */
      splx(s);

      while(fragment) {
	/*
	 * We "should" (says the spec) send an ICMP time exceeded here.
	 * However, among other headaches, we may not actually have a copy
	 * of the real packet sent to us (if we bubbled, we now have a frag
	 * header that never really came from the sender). The solution taken
	 * for now is to continue the BSD tradition of not bothering to send
	 * these messages.
	 */
	m = fragment->data;
	ipv6stat.ips_fragtimeout++;
	while(m) {
	  m2 = m;
	  m = m->m_nextpkt;
	  m_freem(m2);
	}
	if (fragment->prefix)
	  m_freem(fragment->prefix);
	fragmentprev = fragment;
	fragment = fragment->next;
	FREE(fragmentprev, M_FRAGQ);
      }
    }
  else splx(s);
}

/*----------------------------------------------------------------------
 * Drain all fragments & possibly discovery structures.
 ----------------------------------------------------------------------*/

void
ipv6_drain()
{
  struct ipv6_fragment *totrash;
  struct mbuf *m,*m2;
  struct radix_node_head *rnh = rt_tables[AF_INET6];
  int s;
  extern int ipv6_clean_nexthop __P((struct radix_node *,void *));

  while (ipv6_fragmentq != NULL)
    {
      ipv6stat.ips_fragdropped++;
      totrash = ipv6_fragmentq;
      ipv6_fragmentq = totrash->next;
      m = totrash->data;
      while (m)
	{
	  m2 = m;
	  m = m->m_nextpkt;
	  m_freem(m2);
	}
      if (totrash->prefix)
	m_freem(totrash->prefix);
      FREE(totrash, M_FRAGQ);
    }

  /*
   * Might want to delete all off-net host routes, 
   *     and maybe even on-net host routes.  
   *
   * For now, do only the off-net host routes.
   */
  (void) rnh->rnh_walktree(rnh, ipv6_clean_nexthop, (void *)1);
}

/*----------------------------------------------------------------------
 * sysctl(2) handler for IPv6.
 ----------------------------------------------------------------------*/

int
ipv6_sysctl(name, namelen, oldp, oldlenp, newp, newlen)
	int *name;
	u_int namelen;
	void *oldp;
	size_t *oldlenp;
	void *newp;
	size_t newlen;
{
  if (namelen != 1)
    return ENOTDIR;

  switch (name[0])
    {
#ifdef INET6_DEBUG
    case IPV6CTL_DEBUGLEVEL:
      return sysctl_int(oldp, oldlenp, newp, newlen, &in6_debug_level);
#endif
    default:
      return EOPNOTSUPP;  /* For now... */
    }
}

/*----------------------------------------------------------------------
 * Should be a macro, this function determines if IPv6 is running on a
 * given interface.
 ----------------------------------------------------------------------*/

int
ipv6_enabled(ifp)
     struct ifnet *ifp;
{
  struct in6_ifaddr *i6a;

  for (i6a = in6_ifaddr; i6a; i6a = i6a->i6a_next)
    if (i6a->i6a_ifp == ifp)
      return 1;

  return 0;
}

/*----------------------------------------------------------------------
 * Strips IPv6 options for TCP or UDP.
 *
 * This function assumes that the input chain (incoming) has been
 * munged by ipv6_preparse() and starts with an IPv6 header.
 * The header index is invalid after this call. 
 * The IPv6 header is not updated EXCEPT for the length, which must be
 * in HOST order.
 * Note that the API for this function is NOT THE SAME as its IPv4
 * counterpart.
 *
 * Often called like:
 *
 * ipv6_stripoptions(incoming, ihi[ihioff - 1].ihi_mbuf,
 *                   ihi[ihioff].ihi_nexthdr);
 *
 * Where ihi and ihioff are the header index arrays, passed up after
 * ipv6_preparse().  If preparse is eliminated or altered, this code will
 * be too.
 ----------------------------------------------------------------------*/

void ipv6_stripoptions(incoming, lastopt, nexthdr)
register struct mbuf *incoming, *lastopt;
int nexthdr;
{
  struct mbuf *optm, *m;
  int optlen, len;

  if (incoming == lastopt)
    {
      /*
       * i.e. If there are no options...
       */
      optm = NULL;
      optlen = 0;
    }
  else
    {
      optm = incoming->m_next;
      incoming->m_next = lastopt->m_next;
      lastopt->m_next = NULL;
      
      /*
       * Simply walk down, taking each mbuf as an options header, until
       * the last options (lastopt), and remove those mbufs from the chain.
       */

      for (optlen = 0, m = optm; m; optlen += m->m_len, m = m->m_next);

      incoming->m_pkthdr.len -= optlen;

      DPRINTF(IDL_MAJOR_EVENT, ("stripped %d bytes of options\n", optlen));

      (mtod(incoming, struct ipv6 *))->ipv6_length = 
	(incoming->m_pkthdr.len - sizeof(struct ipv6) > 0xffff) ? 0 :
	  htons(incoming->m_pkthdr.len - sizeof(struct ipv6));

      (mtod(incoming, struct ipv6 *))->ipv6_nexthdr = nexthdr;

      /*
       * We should be saving these stripped options somewhere.
       */
      m_freem(optm);
    }
}
