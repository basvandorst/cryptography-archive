/*
 * in6_debug.c  --  Insipired by Craig Metz's Net/2 in6_debug.c, but
 *                  not quite as heavyweight (initially, anyway).
 *
 * The idea is to have globals here, and dump netinet6/ data structures.
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


#define INET6_DEBUG_C

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/ipv6_icmp.h>
#include <netinet6/in6_debug.h>

/*
 * Globals
 */

/* The following should be sysctl-tweakable. */

unsigned int in6_debug_level = IDL_CRITICAL + 1;  /* 0 is no debugging */

/*
 * Functions and macros.
 */

void
in6_debug_init(void)
{
  /* For now, nothing. */
}

/*----------------------------------------------------------------------
 * dump_* dumps various data structures.  These should be called within
 * the context of a DDO() macro.  They assume address and port fields
 * are in network order.
 ----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
 * Dump an IPv6 address.  Don't compress 0's out because of debugging.
 ----------------------------------------------------------------------*/

void
dump_in_addr6(in_addr6)
     struct in_addr6 *in_addr6;
{
  u_short *shorts = (u_short *)in_addr6;
  int i = 0;

  if (!in_addr6) {
    printf("Dereference a NULL in_addr6? I don't think so.\n");
    return;
  }

  printf("(conv. for printing) ");
  while (i < 7)
    printf("%4x:",htons(shorts[i++]));
  printf("%4x\n",htons(shorts[7]));
}

/*----------------------------------------------------------------------
 * Dump and IPv4 address in x.x.x.x form.
 ----------------------------------------------------------------------*/

void
dump_in_addr(in_addr)
     struct in_addr *in_addr;
{
  u_char *chars = (u_char *)in_addr;
  int i = 0;

  if (!in_addr) {
    printf("Dereference a NULL in_addr? I don't think so.\n");
    return;
  }

  while (i < 3)
    printf("%d.",chars[i++]);
  printf("%d\n",chars[3]);
}

/*----------------------------------------------------------------------
 * Dump an IPv6 socket address.
 ----------------------------------------------------------------------*/

void
dump_sockaddr_in6(sin6)
     struct sockaddr_in6 *sin6;
{
  if (!sin6) {
    printf("Dereference a NULL sockaddr_in6? I don't think so.\n");
    return;
  }

  printf("sin6_len = %d, sin6_family = %d, sin6_port = %d (0x%x)\n",
	 sin6->sin6_len,sin6->sin6_family, htons(sin6->sin6_port),
	 htons(sin6->sin6_port));
  printf("sin6_flowinfo = 0x%x\n",sin6->sin6_flowinfo);
  printf("sin6_addr = ");
  dump_in_addr6(&sin6->sin6_addr);
}

/*----------------------------------------------------------------------
 * Dump an IPv4 socket address.
 ----------------------------------------------------------------------*/

void
dump_sockaddr_in(sin)
     struct sockaddr_in *sin;
{
  int i;

  if (!sin) {
    printf("Dereference a NULL sockaddr_in? I don't think so.\n");
    return;
  }

  printf("sin_len = %d, sin_family = %d, sin_port (conv.) = %d (0x%x)\n",
	 sin->sin_len,sin->sin_family, htons(sin->sin_port),
	 htons(sin->sin_port));
  printf("sin_addr = ");
  dump_in_addr(&sin->sin_addr);
  printf("sin_zero == ");
  for(i=0;i<8;i++)
    printf("0x%2x ",sin->sin_zero[i]);
  printf("\n");
}

/*----------------------------------------------------------------------
 * Dump a generic socket address.  Use if no family-specific routine is
 * available.
 ----------------------------------------------------------------------*/
void
dump_sockaddr(sa)
     struct sockaddr *sa;
{
  int i;

  if (!sa) {
	printf("Dereference a NULL sockaddr? I don't think so.\n");
        return;
  }

  printf("sa_len = %d, sa_family = %d, remaining bytes are:\n",
	 sa->sa_len, sa->sa_family);
  for (i = 0; i <sa->sa_len - 2; i++)
    printf("0x%2x ",(unsigned char)sa->sa_data[i]);
  printf("\n");
}

/*----------------------------------------------------------------------
 * Dump a link-layer socket address.  (Not that there are user-level link
 * layer sockets, but there are plenty of link-layer addresses in the kernel.)
 ----------------------------------------------------------------------*/

void
dump_sockaddr_dl(sdl)
     struct sockaddr_dl *sdl;
{
  char buf[256];

  if (!sdl) {
	printf("Dereference a NULL sockaddr_dl? I don't think so.\n");
        return;
  }

  printf("sdl_len = %d, sdl_family = %d, sdl_index = %d, sdl_type = %d,\n",
	 sdl->sdl_len, sdl->sdl_family, sdl->sdl_index, sdl->sdl_type);
  buf[sdl->sdl_nlen] = 0;
  if (sdl->sdl_nlen)
    bcopy(sdl->sdl_data,buf,sdl->sdl_nlen);
  printf("sdl_nlen = %d, (name = '%s'\n",sdl->sdl_nlen,buf);
  printf("sdl_alen = %d, ",sdl->sdl_alen);
  if (sdl->sdl_alen)
    {
      int i;

      printf("(addr = ");
      for (i = 0; i<sdl->sdl_alen; i++)
	printf("0x%2x ",(unsigned char)sdl->sdl_data[i+sdl->sdl_nlen]);
    }
  printf("\n");
  printf("sdl_slen = %d, ",sdl->sdl_slen);
  if (sdl->sdl_slen)
    {
      int i;

      printf("(addr = ");
      for (i = 0; i<sdl->sdl_slen; i++)
	printf("0x%2x ",
	       (unsigned char)sdl->sdl_data[i+sdl->sdl_nlen+sdl->sdl_alen]);
    }
  printf("\n");
}

/*----------------------------------------------------------------------
 * Dump a socket address, calling a family-specific routine if available.
 ----------------------------------------------------------------------*/

void
dump_smart_sockaddr(sa)
     struct sockaddr *sa;
{
  if (!sa) {
	printf("Dereference a NULL sockaddr? I don't think so.\n");
        return;
  }

  switch (sa->sa_family)
    {
    case AF_INET6:
      dump_sockaddr_in6((struct sockaddr_in6 *)sa);
      break;
    case AF_INET:
      dump_sockaddr_in((struct sockaddr_in *)sa);
      break;
    case AF_LINK:
      dump_sockaddr_dl((struct sockaddr_dl *)sa);
      break;
    default:
      dump_sockaddr(sa);
      break;
    }
}

/*----------------------------------------------------------------------
 * Dump an IPv6 header.
 ----------------------------------------------------------------------*/

void
dump_ipv6(ipv6)
     struct ipv6 *ipv6;
{
  if (!ipv6) {
	printf("Dereference a NULL ipv6? I don't think so.\n");
        return;
  }

  printf("Vers & flow label (conv to host order) 0x%x\n",
	 htonl(ipv6->ipv6_versfl));
  printf("Length (conv) = %d, nexthdr = %d, hoplimit = %d.\n",
	 htons(ipv6->ipv6_length),ipv6->ipv6_nexthdr,ipv6->ipv6_hoplimit);
  printf("Src: ");
  dump_in_addr6(&ipv6->ipv6_src);
  printf("Dst: ");
  dump_in_addr6(&ipv6->ipv6_dst);
}

/*----------------------------------------------------------------------
 * Dump an ICMPv6 header.  This function is not very smart beyond the
 * type, code, and checksum.
 ----------------------------------------------------------------------*/

dump_ipv6_icmp(icp)
     struct ipv6_icmp *icp;
{
  int i;

  if (!icp) {
	printf("Dereference a NULL ipv6_icmp? I don't think so.\n");
        return;
  }

  printf("type %d, code %d, cksum (conv) = 0x%x\n",icp->icmp_type,
	 icp->icmp_code,htons(icp->icmp_cksum));
  printf("First four bytes:  0x%x",htonl(icp->icmp_unused));
  printf("Next four bytes: 0x");
  for (i=0;i<4;i++)
    printf("%x",icp->icmp_echodata[i]);
  printf("\n");
}

/*----------------------------------------------------------------------
 * Dump only the header fields of a single mbuf.
 ----------------------------------------------------------------------*/

void
dump_mbuf_hdr(m)
     struct mbuf *m;
{
  if (!m) {
	printf("Dereference a NULL mbuf? I don't think so.\n");
        return;
  }

  printf("Single mbuf at %08x\n", m);
  printf("m_len = %d, m_data = 0x%x, m_type = %d\n",m->m_len,
	 m->m_data, m->m_type);
  printf("m_flags = 0x%x ",m->m_flags);
  if (m->m_flags & M_PKTHDR)
    printf("m_pkthdr.len = %d, m_pkthdr.rcvif = 0x%x",m->m_pkthdr.len,
	   m->m_pkthdr.rcvif);
  if (m->m_flags & M_EXT)
    printf(" (IS CLUSTER MBUF)");
  printf("\nm_next = 0x%x  m_nextpkt = 0x%x\n",m->m_next, m->m_nextpkt);
}

/*----------------------------------------------------------------------
 * Dump the entire contents of a single mbuf.
 ----------------------------------------------------------------------*/

void
dump_mbuf(m)
     struct mbuf *m;
{
  int i;

  dump_mbuf_hdr(m);
  printf("m_data:\n");
  for (i = 0; i < m->m_len; i++)
    printf("0x%2x%s",(unsigned char)m->m_data[i] , ((i+1) % 16)?" ":"\n");
  printf((i % 16)?"\n":"");
}

/*----------------------------------------------------------------------
 * Dump the contents of an mbuf chain.  (WARNING:  Lots of text may
 * result.
 ----------------------------------------------------------------------*/

void
dump_mchain(m)
     struct mbuf *m;
{
  struct mbuf *walker;
  int i;

  for (walker = m, i = 0; walker != NULL && (i < 10); 
       walker = walker->m_next, i++)
    dump_mbuf(walker);
}

/*----------------------------------------------------------------------
 * Dump an mbuf chain's data in a format similar to tcpdump(8).
 ----------------------------------------------------------------------*/

void
dump_tcpdump(m)
     struct mbuf *m;
{
  struct mbuf *walker;
  int i, j, count;

  for (i = count = 0; m && (i < 10); m = m->m_next, i++) {
    for (j = 0; j < m->m_len; j++, count++) {
      if (!(count % (2 * 8)))
	printf("\n\t\t\t");
      if (!(count % 2))
	printf(" ");
      printf("%02x", (u_int8)(m->m_data[j]));
    }
  }
}

/*----------------------------------------------------------------------
 * Dump an IPv6 header index table, which is terminated by an entry with
 * a NULL mbuf pointer.
 ----------------------------------------------------------------------*/

void
dump_ihitab(ihi)
     struct in6_hdrindex *ihi;
{
  int i=0;

  if (!ihi) {
    printf("Dereference a NULL hdrindex/ihi? I don't think so.\n");
    return;
  }

  /* This is dangerous, make sure ihitab was bzeroed. */
  while (ihi[i].ihi_mbuf)
    {
      printf("ihi_nexthdr = %d, ihi_mbuf = 0x%x.\n",ihi[i].ihi_nexthdr,
	     ihi[i].ihi_mbuf);
      i++;
    }
}

/*----------------------------------------------------------------------
 * Dump an interface address.
 ----------------------------------------------------------------------*/

void
dump_ifa(ifa)
     struct ifaddr *ifa;
{
  if (ifa == NULL)
    {
      printf("ifa of NULL.\n");
      return;
    }

  printf("ifa_addr: ");
  dump_smart_sockaddr(ifa->ifa_addr);
  printf("ifa_netmask: ");
  dump_smart_sockaddr(ifa->ifa_netmask);
}

/*----------------------------------------------------------------------
 * Dump an interface structure.
 ----------------------------------------------------------------------*/

void
dump_ifp(ifp)
     struct ifnet *ifp;
{
  if (!ifp) {
    printf("Dereference a NULL ifnet/ifp? I don't think so.\n");
    return;
  }

  printf("Interface name: %s.\n",ifp->if_name);
  printf("Interface type: %d.  ",ifp->if_type);
  printf("MTU: %d.\n",ifp->if_mtu);
}

/*----------------------------------------------------------------------
 * Dump a route structure (sockaddr/rtentry pair).
 ----------------------------------------------------------------------*/

void
dump_route(ro)
     struct route *ro;
{
  if (!ro) {
    printf("Dereference a NULL route? I don't think so.\n");
    return;
  }

  printf("ro_rt = 0x%x, ro_dst is:\n",ro->ro_rt);
  dump_smart_sockaddr(&ro->ro_dst);
}

/*----------------------------------------------------------------------
 * Dump a routing entry.
 ----------------------------------------------------------------------*/

void
dump_rtentry(rt)
     struct rtentry *rt;
{
  if (!rt) {
    printf("Dereference a NULL rtentry? I don't think so.\n");
    return;
  }

  printf("rt_key is:\n");
  dump_smart_sockaddr(rt_key(rt));
  printf("rt_llinfo = 0x%x ",rt->rt_llinfo);
  printf("rt_rmx.rmx_mtu = %d ",rt->rt_rmx.rmx_mtu);
  printf("rt_refcnt = %d ",rt->rt_refcnt);
  printf("rt_flags = 0x%x\n",rt->rt_flags);
}

/*----------------------------------------------------------------------
 * Dump an Internet (v4/v6) protocol control block.
 ----------------------------------------------------------------------*/
void
dump_inpcb(inp)
     struct inpcb *inp;
{
  if (!inp) {
    printf("Dereference a NULL inpcb? I don't think so.\n");
    return;
  }

  printf("inp_next = 0x%x, inp_prev = 0x%x, inp_head = 0x%x.\n",inp->inp_next,
	 inp->inp_prev, inp->inp_head);
  printf("inp_socket = 0x%x, inp_ppcb\n",inp->inp_socket,inp->inp_ppcb);
  printf("faddr, faddr6:\n");
  dump_in_addr(&inp->inp_faddr); dump_in_addr6(&inp->inp_faddr6);
  printf("laddr, laddr6:\n");
  dump_in_addr(&inp->inp_laddr); dump_in_addr6(&inp->inp_laddr6);
  printf("inp_route: ");
  dump_route(&inp->inp_route);
  printf("inp_ipv6:");
  dump_ipv6(&inp->inp_ipv6);
  printf("inp_ip:");
  printf("<Coming soon.>\n");
  printf("inp_options = 0x%x, inp_moptions{6,} = 0x%x,\n",inp->inp_options,
	 inp->inp_moptions);
  printf("inp_flags = 0x%x, inp_fport = %d, inp_lport = %d.\n",
	 (unsigned)inp->inp_flags,inp->inp_fport, inp->inp_lport);
}

/*----------------------------------------------------------------------
 * Dump an IPv6 discovery queue structure.
 ----------------------------------------------------------------------*/

void
dump_discq(dq)
     struct discq *dq;
{
  if (!dq) {
    printf("Dereference a NULL discq? I don't think so.\n");
    return;
  }

  printf("dq_next = 0x%x, dq_prev = 0x%x, dq_rt = 0x%x,\n",dq->dq_next,
	 dq->dq_prev, dq->dq_rt);
  printf("dq_queue = 0x%x.\n",dq->dq_queue);
  /* Dump first mbuf chain? */
  /*printf("dq_expire = %d (0x%x).\n",dq->dq_expire,dq->dq_expire);*/
}
