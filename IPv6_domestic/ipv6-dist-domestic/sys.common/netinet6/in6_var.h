/*
 * in6_var.h  --  Analagous to BSD's in_var.h file, definitions for this
 *                implementation of IPv6 and layers above and below it.
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

#ifndef _IN6_VAR_H
#define _IN6_VAR_H

/*
 * IPv6 interface request and alias request.  Use sockaddr_in6 because
 * it is larger than sockaddr.
 */

struct inet6_ifreq
{
  #define IFNAMSIZ        16
        char    ifr_name[IFNAMSIZ];             /* if name, e.g. "en0" */
        union {
                struct  sockaddr_in6 ifru_addr;
                struct  sockaddr_in6 ifru_dstaddr;
                struct  sockaddr_in6 ifru_broadaddr;
                short   ifru_flags;
                int     ifru_metric;
                caddr_t ifru_data;
	      } ifr_ifru;
#define ifr_addr        ifr_ifru.ifru_addr      /* address */
#define ifr_dstaddr     ifr_ifru.ifru_dstaddr   /* other end of p-to-p link */
#define ifr_broadaddr   ifr_ifru.ifru_broadaddr /* broadcast address */
#define ifr_flags       ifr_ifru.ifru_flags     /* flags */
#define ifr_metric      ifr_ifru.ifru_metric    /* metric */
#define ifr_data        ifr_ifru.ifru_data      /* for use by interface */
      };

/*
 * IPv6 interface "alias" request.  Used to add interface addresses.  This 
 * may be needed to be expanded to pass down/up permanancy information, and
 * possibly deprecation lifetime values.  (That is, if the kernel doesn't
 * compute that stuff itself.)
 */

struct inet6_aliasreq
{
  char ifra_name[IFNAMSIZ];
  struct sockaddr_in6 ifra_addr;
  struct sockaddr_in6 ifra_dstaddr;
  struct sockaddr_in6 ifra_mask;
};

/* ioctl()'s for stuff with inet6_{aliasreq,ifreq}  (gag!) */

#define SIOCDIFADDR_INET6 _IOW('i',25, struct inet6_ifreq)  /* delete IF addr */
#define SIOCAIFADDR_INET6 _IOW('i',26, struct inet6_aliasreq)/* add/chg IFalias */
#define SIOCGIFADDR_INET6 _IOWR('i',33, struct inet6_ifreq) /* get ifnet address */
#define SIOCGIFDSTADDR_INET6 _IOWR('i',34, struct inet6_ifreq) /* get dst address */
#define SIOCSIFDSTADDR_INET6 _IOW('i', 14, struct inet6_ifreq) /* set dst address */
#define SIOCGIFNETMASK_INET6 _IOWR('i',37, struct inet6_ifreq) /* get netmask */

/*
 * INET6 interface address.  This might also serve as the prefix list,
 * with the help of the I6AF_PREFIX flag.
 */

struct in6_ifaddr
{
  struct ifaddr i6a_ifa; /* protocol-independent info (32 bytes) */
#define i6a_ifp    i6a_ifa.ifa_ifp
#ifdef KERNEL
#define i6a_flags  i6a_ifa.ifa_flags
#endif
  
  /* All sorts of INET6-specific junk, some of it, very similar to IP's
     in_ifaddr. */
  
  /* Put any subnetting, etc here. */
  
  struct in6_ifaddr *i6a_next;
  struct sockaddr_in6 i6a_addr;          /* Address. */
  struct sockaddr_in6 i6a_dstaddr;       /* Dest. if PPP link. */
  struct sockaddr_in6 i6a_sockmask;      /* Netmask.  This is IPv6, so
					    there is no "subnet/net"
					    distinction. */
  
  /* Multicast stuff here. */
  struct in6_multi *i6a_multiaddrs;      /* Multicast addresses. 
					    They should only hang off the
					    permanent link-local i6a. */
  
  /*
   * IPv6 addresses have lifetimes.  Put in expiration information in
   * here.  A DEPRECATED address is still valid for inbound, but not for
   * outbound.  An EXPIRED address is invalid for both inbound and outbound,
   * and should be put out of its misery (and our in6_ifaddr list) ASAP.
   */
  u_long i6a_deprecate;                 /* Deprecation time. */
  u_long i6a_expire;                    /* Expiration time. */
  u_long i6a_addrflags;                 /* Additional flags because
					   ifa_flags isn't big enough. */
};

#define I6A_SIN(i6a) (&(((struct in6_ifaddr *)(i6a))->i6a_addr))
#define IS_EXPIRED(i6a) ((i6a->i6a_expire != 0 && \
			  i6a->i6a_expire < time.tv_sec))
#define IS_DEPRECATED(i6a) ((i6a->i6a_deprecate != 0 && \
			     i6a->i6a_deprecate < time.tv_sec))

#define I6AF_LINKLOC 0x1    /* Link-local address.  Saves the IS_IN6_LINKLOC
			       check. */
#define I6AF_PERMANENT 0x2  /* Permanent address */
#define I6AF_PREFIX 0x4     /* I am a, "prefix list entry," meaning that
			       the portion of the address inside the mask
			       is directly attached to the link. */


/*
 * IPv6 multicast structures and macros.
 */

struct in6_multi
{
  struct in6_multi *in6m_next;     /* Ptr. to next one. */
  struct in6_multi *in6m_prev;     /* Ptr. to prev one. */
  struct in_addr6 in6m_addr;       /* Multicast address. */
  struct ifnet *in6m_ifp;          /* Pointer to interface. */
  struct in6_ifaddr *in6m_i6a;     /* Back ptr. to in6_ifaddr. */
  u_int in6m_refcount;             /* Number of memb. claims by sockets. */
  u_int in6m_timer;                /* IGMP membership report timer. */
};

#ifdef KERNEL
/* General case IN6 multicast lookup.  Can be optimized out in certain
   places (like netinet6/ipv6_input.c ?). */

#define IN6_LOOKUP_MULTI(addr,ifp,in6m) \
{\
  register struct in6_ifaddr *i6a;\
\
  for (i6a=in6_ifaddr; i6a != NULL && i6a->i6a_ifp != ifp; i6a=i6a->i6a_next)\
   ;\
  if (i6a == NULL)\
    in6m=NULL;\
  else\
    for ((in6m) = i6a->i6a_multiaddrs;\
       (in6m) != NULL && !IN6_ADDR_EQUAL((in6m)->in6m_addr,(*(addr)));\
       (in6m) = (in6m)->in6m_next) ;\
}

#define IN6_MCASTOPTS 0x2

#define ETHER_MAP_IN6_MULTICAST(in6addr,enaddr)\
{\
   (enaddr)[0] = 0x33;(enaddr)[1] = 0x33;\
   (enaddr)[2] = (in6addr).in6a_u.bytes[12];\
   (enaddr)[3] = (in6addr).in6a_u.bytes[13];\
   (enaddr)[4] = (in6addr).in6a_u.bytes[14];\
   (enaddr)[5] = (in6addr).in6a_u.bytes[15];\
   }

#endif /* KERNEL */

#endif /* _IN6_VAR_H */

