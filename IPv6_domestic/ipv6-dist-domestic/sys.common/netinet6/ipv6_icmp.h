/*
 * ipv6_icmp.h  --  Definitions and constants for ICMP in IPv6.
 *                  (Including discovery and multicast group messages.)
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


#include <netinet6/in6_types.h>

/*
 * ICMPv6 header.
 */

struct ipv6_icmp
{
  u_int8 icmp_type;
  u_int8 icmp_code;
  u_int16 icmp_cksum;
  union
    {
      u_int32 ih_reserved;
      struct
	{
	  u_int16 ihs_id;
	  u_int16 ihs_seq;
	} ih_idseq;
    } icmp_hun;
#define icmp_unused icmp_hun.ih_reserved
#define icmp_nexthopmtu icmp_hun.ih_reserved
#define icmp_paramptr icmp_hun.ih_reserved
#define icmp_echoid icmp_hun.ih_idseq.ihs_id
#define icmp_echoseq icmp_hun.ih_idseq.ihs_seq
#define icmp_grpdelay icmp_hun.ih_idseq.ihs_id
#define icmp_grpunused icmp_hun.ih_idseq.ihs_seq
#define icmp_radvbits icmp_hun.ih_idseq.ihs_id
#define icmp_radvlifetime icmp_hun.ih_idseq.ihs_seq
#define icmp_nadvbits icmp_hun.ih_reserved
  union
    {
      struct
	{
	  struct ipv6 ido_ipv6;
	  u_int8 ido_remaining[1];
	} id_offending;
      u_int8 id_data[1];
      struct
	{
	  struct in_addr6 idn_addr;
	  u_int8 idn_ext[1];
	} id_neighbor;
      struct
	{
	  struct in_addr6 idr_addr1;
	  struct in_addr6 idr_addr2;
	  u_int8 idr_ext[1];
	} id_redirect;
    } icmp_dun;
#define icmp_offending icmp_dun.id_offending
#define icmp_ipv6 icmp_dun.id_offending.ido_ipv6

#define icmp_echodata icmp_dun.id_data

#define icmp_grpaddr icmp_dun.id_neighbor.idn_addr

#define icmp_radvext icmp_dun.id_data

#define icmp_nsolsrc icmp_dun.id_redirect.idr_addr1
#define icmp_nsoltarg icmp_dun.id_redirect.idr_addr2
#define icmp_nsolext icmp_dun.id_redirect.idr_ext
#define icmp_nadvaddr icmp_dun.id_neighbor.idn_addr
#define icmp_nadvext icmp_dun.id_neighbor.idn_ext

#define icmp_redirtarg icmp_dun.id_redirect.idr_addr1
#define icmp_redirdest icmp_dun.id_redirect.idr_addr2
#define icmp_redirext icmp_dun.id_redirect.idr_ext
};

/*
 * ICMPv6 extension constants.
 */

#define EXT_SOURCELINK 1
#define EXT_TARGETLINK 2
#define EXT_PREFIX 3
#define EXT_REDIR 4
#define EXT_MTU 5

/*
 * Extension structures for IPv6 discovery messages.
 */

struct icmp_exthdr    /* Generic extension */
{
  u_int8 ext_id;
  u_int8 ext_length;    /* Length is 8 * this field, 0 is invalid. */
  u_int8 ext_data[6];   /* Padded to 8 bytes. */
};

struct ext_prefinfo    /* Prefix information */
{
  u_int8 pre_extid;
  u_int8 pre_length;

  u_int8 pre_prefixsize;
  u_int8 pre_bits;

  u_int32 pre_inval;
  u_int32 pre_deprec;
  u_int32 pre_reserved;

  struct in_addr6 pre_prefix;
};

struct ext_redir    /* Redirected header */
{
  u_int8 rd_extid;
  u_int8 rd_length;
  u_int8 rd_reserved[6];
  struct ipv6 rd_header;
};

struct ext_mtu      /* Recommended link MTU. */
{
  u_int8 mtu_extid;
  u_int8 mtu_length;
  u_int16 mtu_reserved;
  u_int32 mtu_mtu;
};

/*
 * Constants
 */

/*
 * Lower bounds on packet lengths for various types.
 * For the error advice packets must first insure that the
 * packet is large enought to contain the returned ip header.
 * Only then can we do the check to see if enough bits of packet
 * data have been returned, since we need to check the returned
 * ipv6 header length.
 */
#define	ICMPV6_MINLEN	8				/* abs minimum */
#define	ICMPV6_TSLEN	(8 + 3 * sizeof (n_time))	/* timestamp */
#define	ICMPV6_NADVMINLEN 24	/* min neighbor advertisement */
#define ICMPV6_NSOLMINLEN 40    /* min neighbor solicit */
#define ICMPV6_HLPMINLEN (8 + sizeof(struct ipv6) + 8)  /* HLP demux len. */
#define ICMPV6_MAXLEN     576   /* This should be whatever IPV6_MINMTU
				   will be.  I take this to be the WHOLE
				   packet, including IPv6 header, and any
				   IPv6 options before the ICMP message. */

/*
 * Definition of type and code field values.
 * ICMPv6 fixes things so that info messages are >= 128.
 */

/* Error messages and codes. */

#define	ICMPV6_UNREACH		1		/* dest unreachable, codes: */
#define         ICMPV6_UNREACH_NOROUTE 0                  /* No route to dest. */
#define         ICMPV6_UNREACH_ADMIN   1                  /* Admin. prohibited */
#define         ICMPV6_UNREACH_NOTNEIGHBOR 2              /* For strict source
							   routing. */
#define         ICMPV6_UNREACH_ADDRESS 3                  /* Address unreach. */
#define         ICMPV6_UNREACH_PORT  4                    /* Port unreachable */
#define ICMPV6_TOOBIG             2               /* Packet too big. */
#define	ICMPV6_TIMXCEED		3	        /* time exceeded, code: */
#define		ICMPV6_TIMXCEED_INTRANS	0		/* ttl==0 in transit */
#define		ICMPV6_TIMXCEED_REASS	1		/* Reassembly t.o. */
#define	ICMPV6_PARAMPROB		4		/* ip header bad */
#define		ICMPV6_PARAMPROB_PROB    0                /* Actual incorrect
							   parameter. */
#define         ICMPV6_PARAMPROB_NEXTHDR 1	        /* Bad next hdr. */
#define         ICMPV6_PARAMPROB_BADOPT  2                /* Unrec. option */

#define ICMPV6_REDIRECT         5               /* ICMPv6 redirect. */

/* Info messages. */

#define	ICMPV6_ECHO		128		/* echo service */
#define	ICMPV6_ECHOREPLY	129		/* echo reply */
#define ICMPV6_GRPQUERY		130		/* Query group membership. */
#define ICMPV6_GRPREPORT	131		/* Join mcast group. */
#define ICMPV6_GRPTERM		132		/* Leave mcast group. */

#define ICMPV6_ROUTERSOL        133             /* Router solicit. */
#define ICMPV6_ROUTERADV        134             /* Router advertisement. */
#define ICMPV6_NEIGHBORSOL      135             /* Neighbor solicit. */
#define ICMPV6_NEIGHBORADV      136             /* Neighbor advertisement. */

/* Defined this way to save some HTONL cycles on little-endian boxes. */
#if BYTE_ORDER == BIG_ENDIAN
#define           ICMPV6_NEIGHBORADV_RTR   0x80000000  /* Router flag. */
#define           ICMPV6_NEIGHBORADV_SOL   0x40000000  /* Solicited flag. */
#define           ICMPV6_NEIGHBORADV_SECOND 0x20000000 /* Secondary flag. */
#else  /* BYTE_ORDER == LITTLE_ENDIAN */
#define           ICMPV6_NEIGHBORADV_RTR   0x80  /* Router flag. */
#define           ICMPV6_NEIGHBORADV_SOL   0x40  /* Solicited flag. */
#define           ICMPV6_NEIGHBORADV_SECOND 0x20 /* Secondary flag. */
#endif

#define	ICMPV6_MAXTYPE		137

#define	ICMPV6_INFOTYPE(type) ((type) >= 128)

#ifdef KERNEL
/* Function prototypes */
void ipv6_icmp_error __P((struct mbuf *, int, int, u_int32));
void ipv6_icmp_input __P((struct mbuf *, int, struct in6_hdrindex *,int));
void ipv6_gsolicit __P((struct ifnet *, struct mbuf *, struct rtentry *));
void ipv6_rtrequest __P((int, struct rtentry *, struct sockaddr *));
int ipv6_icmp_output __P((struct mbuf *,struct socket *, struct in_addr6 *));
int ipv6_icmp_usrreq __P((struct socket *,int, struct mbuf *,struct mbuf *, struct mbuf *));

void ipv6_routersol_input __P((struct mbuf *, int, struct in6_hdrindex *,int));
void ipv6_routeradv_input __P((struct mbuf *, int, struct in6_hdrindex *,int));
void ipv6_neighborsol_input __P((struct mbuf *, int, struct in6_hdrindex *,int));
void ipv6_neighboradv_input __P((struct mbuf *, int, struct in6_hdrindex *,int));
#endif
