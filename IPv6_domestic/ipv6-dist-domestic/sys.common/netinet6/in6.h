/*
 * in6.h  --  Basic definitions of structures and constants of IPv6
 *            internets.   Roughly corresponds to netinet/in.h.
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


#ifndef _IN6_H
#define _IN6_H

/*
 * Next header types (called Protocols in netinet/in.h).
 */

#define NEXTHDR_HOP		0	/* Hop-by-hop option header. */
#define NEXTHDR_ICMPV4		1	/* ICMP message for IPv4. */
#define NEXTHDR_IGMP		2	/* IGMP message. */
#define NEXTHDR_IPV4		4	/* IPv4 in IPv6 (?!?) */
#define NEXTHDR_TCP		6	/* TCP segment. */
#define NEXTHDR_UDP		17	/* UDP message. */
#define NEXTHDR_IPV6		41	/* IPv6 in IPv6 */
#define NEXTHDR_ROUTING		43	/* Routing header. */
#define NEXTHDR_FRAGMENT	44	/* Fragmentation/reassembly header. */
#define NEXTHDR_ESP		50	/* Encapsulating security payload. */
#define NEXTHDR_AUTH		51	/* Authentication header. */
#define NEXTHDR_ICMP		58	/* ICMP for IPv6. */
#define NEXTHDR_ICMPV6		58
#define NEXTHDR_NONE		59	/* No next header */
#define NEXTHDR_DEST		60	/* Destination options header. */

/*
 * Following are TBD, and subject to change rapidly
 */
#define NEXTHDR_RAW		255	/* Payload of unknown type? */
#define NEXTHDR_MAX		256	/* Upper bound for next header type. */

/* NEXTHDR type macros. */

#define IS_PREFRAG(x)   ( (x)==NEXTHDR_HOP || (x)==NEXTHDR_ROUTING || \
			 (x) == NEXTHDR_DEST)
#define IS_IPV6OPT(x)   ( (x)==NEXTHDR_FRAGMENT || (x) == NEXTHDR_AUTH || \
			 IS_PRFRAG(x) )

/* Use netinet/in.h for port information.  Cheesy hack to include in.h
   iff a macro in in.h (IP_OPTIONS, in this case) isn't defined. */

#ifndef IP_OPTIONS
#include <netinet/in.h>
#endif

/*
 * Include vital type information (file is protected against multiple
 * inclusions).
 */

#include <netinet6/in6_types.h>

/*
 * Macros to manipulate in_addr6.
 *
 * Note all macros for dealing with in_addr6 assume you can use '.' rather
 * than '->'.  This is because we might support 16-byte ints, and
 * plug in simple assignment statements for these macros. 
 * (This rationale made much more sense when it was 8-byte SIP{,P}.
 *
 * These macros currently deal with in_addr6's 32-bits at a time.
 * Eventually, 64-bits at a time would be nice, and as mentioned, 128-would
 * be wicked awesome.
 *
 * It is recommended that statement macros are placed in {} in _if_
 * clauses, if they aren't inside them already.  This is so when they are
 * expanded, you don't have:
 *
 * if (foo == bar)
 *   MACRO(x);
 * else printf("error");
 *
 * turn into...
 *
 * if (foo == bar)
 *   {
 *     mystery();
 *   };
 * else printf("error");
 *
 * Because that semicolon will cause an error!
 *
 */

#define IN6_ADDR_ASSIGN(a,w1,w2,w3,w4) {(a).in6a_words[0]=(w1);\
					(a).in6a_words[1]=(w2);\
					(a).in6a_words[2]=(w3);\
					(a).in6a_words[3]=(w4);}
#define IN6_ADDR_COPY(a1,a2) IN6_ADDR_ASSIGN(a1,\
					     (a2).in6a_words[0],\
					     (a2).in6a_words[1],\
					     (a2).in6a_words[2],\
					     (a2).in6a_words[3])
#define IN6_ADDR_EQUAL(a1,a2) ( (a1).in6a_words[3] == (a2).in6a_words[3] &&\
                                 (a1).in6a_words[2] == (a2).in6a_words[2] &&\
                                 (a1).in6a_words[1] == (a2).in6a_words[1] &&\
                                 (a1).in6a_words[0] == (a2).in6a_words[0] )

/* These assume bytes in address 'a' are in network order... */
#define IS_IN6_UNSPEC(a) ( (a).in6a_words[3] == 0 &&\
                                 (a).in6a_words[2] == 0 &&\
                                 (a).in6a_words[1] == 0 &&\
                                 (a).in6a_words[0] == 0 )

/* NOTE:  IS_IN6_ALL* macros only check the 0x1 and 0x2 scoping levels.
          The IN6 ROAD document doesn't say those are good for higher
          scoping levels.
*/

#define IN6_INTRA_NODE 1  /*  intra-node scope */
#define IN6_INTRA_LINK 2  /*  intra-link scope */
/*            3  (unassigned)
            4  (unassigned)
*/
#define IN6_INTRA_SITE 5  /* intra-site scope */
/*            6  (unassigned)
            7  (unassigned)
*/
#define IN6_INTRA_ORG  8  /* intra-organization scope */
/*            9  (unassigned)
            A  (unassigned)
*/
#define IN6_INTRA_COMM 0xB/*  intra-community scope */
/*            C  (unassigned)
            D  (unassigned)
*/

#define IN6_GLOBAL   0xE  /* global scope*/
/*            F  reserved
*/

#if BYTE_ORDER ==  BIG_ENDIAN

#define SET_IN6_ALLNODES(a)  {(a).in6a_words[0]=0xff000000;(a).in6a_words[3]=1;\
                              (a).in6a_words[1]=0;(a).in6a_words[2]=0;}
#define SET_IN6_ALLROUTERS(a)  {(a).in6a_words[0]=0xff000000;(a).in6a_words[3]=2;\
                              (a).in6a_words[1]=0;(a).in6a_words[2]=0;}

#define SET_IN6_MCASTSCOPE(a,bits) {(a).in6a_words[0]&=0xfff0ffff;\
                                    (a).in6a_words[0]|=(bits<<16);}
#define GET_IN6_MCASTSCOPE(a) ( ((a).in6a_words[0] & 0x000f0000) >> 16  )

#define CREATE_IPV6_MAPPED(a, v4) {(a).in6a_words[0] = 0; \
				   (a).in6a_words[1] = 0; \
				   (a).in6a_words[2] = 0xffff; \
				   (a).in6a_words[3] = (v4);}

#define IS_IN6_LOOPBACK(a)  ( (a).in6a_words[0] == 0 && \
                               (a).in6a_words[1] == 0 && (a).in6a_words[2] == 0 && \
                               (a).in6a_words[3] == 1 )

#define IS_IN6_SITELOC(a)   (((a).in6a_words[0] & 0xFFC00000) == 0xFEC00000 )
#define IS_IN6_LINKLOC(a)   (((a).in6a_words[0] & 0xFFC00000) == 0xFE800000 )
#define IS_IN6_NSAP(a)      (((a).in6a_words[0] & 0xfc000000) == 0x02000000)
#define IS_IN6_IPX(a)       (((a).in6a_words[0] & 0xfc000000) == 0x04000000)
#define IS_IN6_MULTICAST(a) (((a).in6a_words[0] & 0xff000000) == 0xff000000 )
#define IS_IN6_MULTICAST_RESERVED(a)  \
  ((a).in6a_words[1] == 0 && (a).in6a_words[2] == 0 && (a).in6a_words[3] == 0 \
   && ((a).in6a_words[0] >> 16) >= 0xff00 \
   && ((a).in6a_words[0] >> 16) <= 0xff0f)

#define IS_IN6_IPV4_COMPAT(a) ((a).in6a_words[0] == 0 && (a).in6a_words[1]==0 \
			       && (a).in6a_words[2] == 0 && \
			       (a).in6a_words[3] != 0)
#define IS_IN6_IPV4_MAPPED(a) ((a).in6a_words[0] == 0 && (a).in6a_words[1]==0 \
			       && (a).in6a_words[2] == 0xffff)

/* Scoping values? */

#define IS_IN6_ALLNODES(a)  (((a).in6a_words[0] == 0xff010000 || \
                              (a).in6a_words[0] == 0xff020000 ) && \
                              (a).in6a_words[1] == 0 && (a).in6a_words[2] == 0 && \
                              (a).in6a_words[3] == 1   )
#define IS_IN6_ALLROUTERS(a)  (((a).in6a_words[0] == 0xff010000 || \
                              (a).in6a_words[0] == 0xff020000 ) && \
                              (a).in6a_words[1] == 0 && (a).in6a_words[2] == 0 && \
                              (a).in6a_words[3] == 2   )
#else   /* BYTE_ORDER == LITTLE_ENDIAN */

#define SET_IN6_ALLNODES(a)  {(a).in6a_words[0]=0xff;(a).in6a_words[3]=0x01000000;\
                              (a).in6a_words[1] = 0; (a).in6a_words[2] = 0;}
#define SET_IN6_ALLROUTERS(a)  {(a).in6a_words[0]=0xff;(a).in6a_words[3]=0x02000000;\
                              (a).in6a_words[1] = 0; (a).in6a_words[2] = 0;}

#define SET_IN6_MCASTSCOPE(a,bits) {(a).in6a_words[0]&=0xfffff0ff;\
                                    (a).in6a_words[0]|=(bits<<8);}
#define GET_IN6_MCASTSCOPE(a)  ( ((a).in6a_words[0] & 0x00000f00) >>8)

#define CREATE_IPV6_MAPPED(a, v4) {(a).in6a_words[0] = 0; \
				   (a).in6a_words[1] = 0; \
				   (a).in6a_words[2] = 0xffff0000; \
				   (a).in6a_words[3] = (v4);}

#define IS_IN6_LOOPBACK(a)  ((a).in6a_words[0] == 0 && \
			     (a).in6a_words[1] == 0 && (a).in6a_words[2] == 0 && \
			     (a).in6a_words[3]==0x01000000 )

#define IS_IN6_SITELOC(a)   (((a).in6a_words[0] & 0xc0ff) == 0xc0fe )
#define IS_IN6_LINKLOC(a)   (((a).in6a_words[0] & 0xc0ff) == 0x80fe )
#define IS_IN6_NSAP(a)     (((a).in6a_words[0] & 0xfc) == 0x02 )
#define IS_IN6_IPX(a)     (((a).in6a_words[0] & 0xfc) == 0x04 )
#define IS_IN6_MULTICAST(a) (((a).in6a_words[0] & 0xff) == 0xff )
#define IS_IN6_MULTICAST_RESERVED(a)  \
  ((a).in6a_words[1] == 0 && (a).in6a_words[2] == 0 && (a).in6a_words[3] == 0 \
   ((a).in6a_words[0] & 0xff) == 0xff && ((a).in6a_words[0] >> 8) <= 0xf )
#define IS_IN6_IPV4_COMPAT(a) ((a).in6a_words[0] == 0 && (a).in6a_words[1]==0 &&\
                              (a).in6a_words[2] == 0 && (a).in6a_words[3] != 0)
#define IS_IN6_IPV4_MAPPED(a) ((a).in6a_words[0] == 0 && (a).in6a_words[1]==0 &&\
                              (a).in6a_words[2] == 0xffff0000)


/* Scoping values? */

#define IS_IN6_ALLNODES(a)  (((a).in6a_words[0] == 0x000001ff || \
                              (a).in6a_words[0] == 0x000002ff ) && \
                              (a).in6a_words[1] == 0 && (a).in6a_words[2] == 0 && \
                              (a).in6a_words[3] == 0x01000000   )
#define IS_IN6_ALLROUTERS(a)  (((a).in6a_words[0] == 0x000001ff || \
                              (a).in6a_words[0] == 0x000002ff ) && \
                              (a).in6a_words[1] == 0 && (a).in6a_words[2] == 0 && \
                              (a).in6a_words[2] == 0x02000000   )

#endif  /* BYTE_ORDER == {BIG,LITTLE}_ENDIAN */

/*
 * Additonal type information.
 */

/* For apps to port between 4.3-derived and 4.4-derived systems. */
#define SIN6_LEN

struct sockaddr_in6 {
  u_char sin6_len;
  u_char sin6_family;
  u_short sin6_port;
  u_long sin6_flowinfo;
  struct in_addr6 sin6_addr;
};

/* Cheesy hack for if net/route.h included... */
#ifdef RTM_VERSION
/*
 * sizeof(struct sockaddr_in6) > sizeof(struct sockaddr), therefore, I
 * need to define... 
 */ 
struct route6
{
  struct  rtentry *ro_rt;
  struct  sockaddr_in6 ro_dst;
};
#endif /* RTM_VERSION */

/*
 * IP options for IPv6.  Note I use the IPV6_* semantics for IPv6-
 * specific options.  Another reason for the inclusion of <netinet/in.h> is
 * for the options that are common between IPv6 and IPv4.
 */

#define	IPV6_OPTIONS		1    /* buf/ipv6_opts; set/get IP options */
#define	IPV6_HDRINCL		2    /* int; header is included with data */
#define	IPV6_TOS		3    /* int; IP type of service and preced. */
#define	IPV6_TTL		4    /* int; IP time to live */
#define	IPV6_RECVOPTS		5    /* bool; receive all IP opts w/dgram */
#define	IPV6_RECVRETOPTS	6    /* bool; receive IP opts for response */
#define	IPV6_RECVDSTADDR	7    /* bool; receive IP dst addr w/dgram */
#define	IPV6_RETOPTS		8    /* ipv6_opts; set/get IP options */
#define	IPV6_MULTICAST_IF	9    /* u_char; set/get IP multicast i/f  */
#define	IPV6_MULTICAST_TTL	10   /* u_char; set/get IP multicast ttl */
#define	IPV6_MULTICAST_LOOP	11   /* u_char; set/get IP multicast loopback
				      */
#define	IPV6_ADD_MEMBERSHIP	12   /* ipv6_mreq; add an IP group membership
				      */
#define	IPV6_DROP_MEMBERSHIP	13   /* ipv6_mreq; drop an IP group membership
				      */
#define IPV6_ADDRFORM   0x16    /* int; get/set form of returned addrs. */
#define IPV6_RCVSRCRT   0x17    /* bool; Enable/disable reception of IPv6
                                       source routes. */

#define IN6_MAX_MEMBERSHIPS 20  /* Maximum number of multicast memberships. */

/*
 * Definitions for inet6 sysctl operations.
 *
 * Third level is protocol number.
 * Fourth level is desired variable within that protocol.
 */

#define	IPV6PROTO_MAXID	(2)	/* Only have 1 right now. */

#define	CTL_IPV6PROTO_NAMES { \
	{ "ipv6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
}

/*
 * Names for IPv6 sysctl objects
 */

/* NOTE: IP forwarding taken care of in IP. */
/*#define	IPCTL_FORWARDING	1*/	/* act as router */

#define	IPV6CTL_SENDREDIRECTS	2	/* may send redirects when forwarding */
#define	IPV6CTL_DEFTTL		3	/* default TTL */
#ifdef notyet
#define	IPV6CTL_DEFMTU		4	/* default MTU */
#endif
#define	IPV6CTL_DEBUGLEVEL	5
#define	IPV6CTL_MAXID		6

#define	IPV6CTL_NAMES { \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "redirect", CTLTYPE_INT }, \
	{ "ttl", CTLTYPE_INT }, \
	{ "mtu", CTLTYPE_INT }, \
	{ "debuglevel", CTLTYPE_INT }, \
	{ 0, 0 }, \
}

#ifdef KERNEL
/* Function prototypes go here. */
int in6_cksum __P((struct mbuf *,int, u_int, u_int));
#endif /* KERNEL */

#endif /* _IN6_H */
