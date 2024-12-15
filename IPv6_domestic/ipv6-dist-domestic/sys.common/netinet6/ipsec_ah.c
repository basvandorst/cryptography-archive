/*----------------------------------------------------------------------
  ip_ah.c:      IPv4 & IPv6 Authentication Header routines.

  Copyright 1995 by Bao Phan, Dan McDonald, & Randall Atkinson.
  All Rights Reserved.  All Rights have been assigned to the US
  Naval Research Laboratory (NRL).  The NRL COPYRIGHT NOTICE and
  LICENSE governs distribution and use of this software.
----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
#	@(#)COPYRIGHT	1.1a (NRL) 17 August 1995

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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/route.h>
#include <sys/protosw.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>

#include <netinet6/in6.h>
#include <netinet6/ipv6.h>
#include <netinet6/ipv6_var.h>
#include <netinet6/in6_debug.h>

#include <netinet6/ipsec.h>
#include <netinet6/key.h>

#include "md5.h"

extern int ipsec_initted;            /* lives in ipsec.c */

#define IPV6_STORE_LENGTH(m) (mtod(m, struct ipv6 *))->ipv6_length = \
	((m->m_pkthdr.len - sizeof(struct ipv6)) > 0xffff) ? 0 : \
	htons(m->m_pkthdr.len - sizeof(struct ipv6));
#define IPV4_STORE_LENGTH(m) (mtod(m, struct ip *))->ip_len = \
        htons(m->m_pkthdr.len);

/* NB:
     Whenever a function moves data from a source to a dest, its calling
     convention is ANSI C style -- dest first, then source. Do not confuse
     this with BSD style -- source first, then dest. */

#ifndef min
#define min(x, y) (x < y) ? x : y
#endif /* min(x, y) */

#ifndef max
#define max(x, y) (x > y) ? x : y
#endif /* max(x, y) */

/* EXTERNALS */
extern struct protosw inet6sw[];         /* See in6_proto.c */
extern u_char ipv6_protox[NEXTHDR_MAX];

extern struct protosw inetsw[];
extern u_char ip_protox[IPPROTO_MAX];

struct mbuf *m_split __P((register struct mbuf *m0, int len0, int wait));

#ifdef INET6_DEBUG
int ah_debug_count = -1;
#endif /* INET6_DEBUG */

#define INIT if (!ipsec_initted) ipsec_init() 

/* AH Algorithm Switch */

#ifdef DEBUG
/* debug includes the cksum and dummy algorithms, 
   which aren't secure but are handy for testing.  */

/* NAUTH is the highest valid authentication algorithm index */
#define NAUTH 2
#else
#define NAUTH 0
#endif


/* NOTE:  There are 3 separate AH algorithms implemented here, but 2 of
          them are dummy and useful only for testing.  The "dummy" 
	  functions are NOPs.  The "cksum" functions just calculate
	  a simple insecure checksum.  Only the "MD5" routines should
	  be used for anything other than testing.  Part of the reason
	  these are retained in this file is to show one way an IP AH
	  implementation can support multiple algorithms at the same
	  time.   rja
*/

struct md5state {
  struct MD5_CTX isb;               /* Internal state block for MD5 */
  struct ipsec_assoc *assoc;       /* Security association (keys!) */
};


/*----------------------------------------------------------------------
  md5init():   Initialise MD5 state and stash pertinent
               Security Association away in md5state.
----------------------------------------------------------------------*/
static caddr_t md5init(assoc)
     struct ipsec_assoc *assoc;
{
  struct md5state *md5state = malloc(sizeof(struct md5state), M_TEMP, M_WAIT);
  u_int32 zero[4] = { 0, 0, 0, 0 };
  
  if (md5state) {
    MD5Init(&(md5state->isb));
    MD5Update(&(md5state->isb), (unsigned char *)assoc->key, assoc->keylen);
    MD5Final(NULL, &(md5state->isb));
    md5state->assoc = assoc;
  }

  return (caddr_t)md5state;
};


/*----------------------------------------------------------------------
  md5update():  Continue the MD5 calculation over some data in the packet.

----------------------------------------------------------------------*/
static caddr_t md5update(md5state, data, len)
     struct md5state *md5state;
     caddr_t data;
     int len;
{
  MD5Update(&(md5state->isb), (unsigned char *)data, len);

#ifdef INET6_DEBUG
  if (ah_debug_count >= 0) {
    int count;
    for (count = 0; count < len; count++, ah_debug_count++) {
      if (!(ah_debug_count % (2 * 8))) {
	printf("\n\t\t\t");
	ah_debug_count = 0;
      }
      if (!(ah_debug_count % 2))
	printf(" ");
      printf("%02x", ((u_int8 *)data)[count]);
    }
  }
#endif /* INET6_DEBUG */

  return (caddr_t)md5state;
};


/*----------------------------------------------------------------------
  md5finish():  Conclude the MD5 calculation for AH and return AH data (?).

----------------------------------------------------------------------*/
static caddr_t md5finish(md5state, data)
     struct md5state *md5state;
     caddr_t data;
{
  u_int32 zero[4] = { 0, 0, 0, 0 };
  int i = (md5state->assoc->keylen % 16);
  MD5Update(&(md5state->isb), (unsigned char *)md5state->assoc->key,
	    md5state->assoc->keylen);
  MD5Final((unsigned char *)data, &(md5state->isb));
  free(md5state, M_TEMP);
  return data;
};



struct cksumstate {
  u_int16 sum;
  u_int8 odd;
};

#ifdef DEBUG
/*----------------------------------------------------------------------
  cksuminit():   Initialise the insecure checksum calculations for AH.

----------------------------------------------------------------------*/
static caddr_t cksuminit(assoc)
     struct ipsec_assoc *assoc;
{
  struct cksumstate *cksumstate = malloc(sizeof(struct cksumstate), 
					 M_TEMP, M_WAIT);
  
  if (cksumstate)
    bzero(cksumstate, sizeof(struct cksumstate));

  return (caddr_t)cksumstate;
};

/*----------------------------------------------------------------------
  chsumupdate():  Update the insecure checksum calculation with new data.

----------------------------------------------------------------------*/
static caddr_t cksumupdate(cksumstate, data, len)
     struct cksumstate *cksumstate;
     caddr_t data;
     int len;
{
  u_int32 sum = cksumstate->sum;

  if (cksumstate->odd) {
    sum += *((u_int8 *)data);
    len--;
    data++;
  }

  while (len > 1) {
    sum += *((u_int16 *)data);
    len -= 2;
  }

  if (len) {
    sum += ((*((u_int8 *)data)) << 8);
  }

  cksumstate->odd = len;
  cksumstate->sum = (sum & 0xffff) | (sum >> 16);

  return (caddr_t)cksumstate;
}

/*----------------------------------------------------------------------
  cksumfinish():  Conclude the insecure checksum calculation and return
                  the calculated Authentication data.
----------------------------------------------------------------------*/
static caddr_t cksumfinish(cksumstate, data)
     struct cksumstate *cksumstate;
     caddr_t data;
{
  *((u_int16 *)data) = cksumstate->sum;
  free(cksumstate, M_TEMP);
  return data;
};


/*----------------------------------------------------------------------
  dummyauthinit():  A no op authentication function initialisation.

----------------------------------------------------------------------*/
static caddr_t dummyauthinit(assoc)
     struct ipsec_assoc *assoc;
{
  return (caddr_t)assoc;
};


/*----------------------------------------------------------------------
  dummyauthupdate():  A no op authentication update function.

----------------------------------------------------------------------*/
static caddr_t dummyauthupdate(state, data, len)
     caddr_t state;
     caddr_t data;
     int len;
{
  return state;
};

/*----------------------------------------------------------------------
  dummyauthfinish():  A no op Authentication finish routine. 

----------------------------------------------------------------------*/
static caddr_t dummyauthfinish(state, data)
     caddr_t state;
     caddr_t data;
{
  return data;
};
#endif /* DEBUG */

/* Array of function ptrs for the AH authentication algorithms */
static struct auth_algorithm {
  caddr_t (*init)(struct ipsec_assoc *);       /* ptr to init fn for alg. */
  caddr_t (*update)(caddr_t, caddr_t, int);    /* ptr to update fn for alg */
  caddr_t (*finish)(caddr_t, caddr_t);         /* ptr to finish fn for alg */
  int datalen;                               /* Length (in u_int8s) of output 
					       data. MUST be a multiple of 4 */
} auth_algorithms[] = { 
{ md5init, md5update, md5finish, 16 }, 		/* 0 = envelope MD5 */
#ifdef DEBUG
{ dummyauthinit, dummyauthupdate, dummyauthfinish, 0 },
                                                /* 1 = dummy/no auth */
{ cksuminit, cksumupdate, cksumfinish, 4 },	/* 2 = checksum (for debug) */
#endif
 /* if any new algorithms are added, then the IPSEC_ALGTYPE_AUTH_* symbols
    in ipsec.h and also the NAUTH symbol above will also need updates. */
{ NULL, NULL, NULL, 0 }				/* graceful terminator */
};



/*----------------------------------------------------------------------
  ipsec_ah_calc():    This handles parsing and then decides whether
                      data ought to be included in the AH computation
                      or zeroed for AH computation.  It also makes the
                      appropriate calls to compute the Authentication Data.
		      This function is called to handle both for arriving 
		      and departing IP packets.  Used for both IPv4 and
		      IPv6.
 NOTES:  

     Some of the IPv4 options supported in this routine are NOT
   supported in other parts of this kernel.  This is the only reasonable
   way to build an AH because the rest of IPv4 is supposed to ignore 
   any unknown options (per RFC-1122).  IPv4 opts not supported 
   elsewhere in the kernel generally have a number in the CASE statement 
   rather than a symbolic name.

     We could either do this the hard way and handle the mbufisms in line
   which would probably have better performance iff preparsing were never
   done, or we can do this the easy way and use preparse since its being 
   done anyway.      We currently do the latter. 

     Performance improvements are needed and planned for the future.
----------------------------------------------------------------------*/
int ipsec_ah_calc(packet, assoc, algorithm, data, index)
     struct mbuf           *packet;
     struct ipsec_assoc    *assoc;
     struct auth_algorithm *algorithm;
     char                  *data;
     struct in6_hdrindex   *index;
{
  u_int8 type;
  struct mbuf *m;
  int i;
  int j;
  int rval = -1;
  unsigned int option_length;
  caddr_t state;
  int indexoff = 1;

  DPRINTF(IDL_GROSS_EVENT, ("Entering ipsec_ah_calc\n"));
  DDO(IDL_GROSS_EVENT, dump_tcpdump(packet));
  DPRINTF(IDL_GROSS_EVENT, ("..."));

  if (!packet->m_len)
    packet = m_free(packet);

  type = (*(mtod(packet, u_int8 *))) >> 4;

  if (!(state = algorithm->init(assoc))) {
    DPRINTF(IDL_CRITICAL, ("AH algorithm init failed!\n"));
    m_freem(packet);
    return NULL;
  }

#ifdef INET6
  if (mtod(packet, struct ip *)->ip_v == 6) {
    struct ipv6    *ipv6;
    struct in_addr6 savedst;
    u_int8          savehoplimit;


    ipv6 = mtod(packet, struct ipv6 *);
    type = ipv6->ipv6_nexthdr;
    savehoplimit = ipv6->ipv6_hoplimit;
    ipv6->ipv6_hoplimit = 0;

    bcopy(&(ipv6->ipv6_dst), &savedst, sizeof(struct in_addr6));

    for (indexoff = 1; index[indexoff].ihi_mbuf && 
	 index[indexoff].ihi_nexthdr != NEXTHDR_ROUTING; indexoff++);
    if (index[indexoff].ihi_nexthdr == NEXTHDR_ROUTING) {
      if ((mtod(index[indexoff].ihi_mbuf,
	    struct ipv6_srcroute0 *))->i6sr_type) {
	DP(GROSS_EVENT, (mtod(index[indexoff].ihi_mbuf, 
			      struct ipv6_srcroute0 *))->i6sr_type, 02x);
	goto ahcret;
      }
      bcopy((mtod(index[indexoff].ihi_mbuf,
	    struct ipv6_srcroute0 *))->i6sr_numaddrs * 
	    sizeof(struct in_addr6) - 8, sizeof(struct in_addr6));
    }

    if (!(state = algorithm->update(state, mtod(packet, caddr_t), 
	sizeof(struct ipv6)))) {
      DPRINTF(IDL_MAJOR_EVENT, ("AH update failed!\n"));
      goto ahcret;
    }

    bcopy(&savedst, &(ipv6->ipv6_dst), sizeof(struct in_addr6));

    ipv6->ipv6_hoplimit = savehoplimit;

  } else {
#endif /* INET6 */

    /* Must be IPv4; If this is ST-2, we'll die badly. */
    struct ip *ip;
    struct in_addr savedst;
    u_int8  savetos;
    u_int8  savettl;
    u_int16 savesum;
    u_int16 saveoff;
    u_int8 *optp;
    u_int8 *optend;
    u_int8 zero[32];
    /* create a 32-byte array containing all zeroes;
       this will be used whenever we need to zero an option for AH calc. */
    bzero(zero, sizeof(zero)); 

    DDO(IDL_GROSS_EVENT, ah_debug_count = 0);
    ip = mtod(packet, struct ip *);

    bcopy(&(ip->ip_dst), &savedst, sizeof(struct in_addr));

    type = ip->ip_p;

    savetos = ip->ip_tos;
    savettl = ip->ip_ttl;
    savesum = ip->ip_sum;
    saveoff = ip->ip_off;
    
    ip->ip_tos = 0;
    ip->ip_ttl = 0;
    ip->ip_sum = 0;
    ip->ip_off = 0;

    if (!(state = algorithm->update(state, (caddr_t)ip, 
				    sizeof(struct ip)))) {
      DPRINTF(IDL_CRITICAL, ("algorithm->update failed (IPv4 header)\n"));
      goto ahcret;
    }

    ip->ip_tos = savetos;
    ip->ip_ttl = savettl;
    ip->ip_sum = savesum;
    ip->ip_off = saveoff;

    bcopy(&savedst, &(ip->ip_dst), sizeof(struct in_addr));
    
    optp = (u_int8 *)ip + sizeof(struct ip);
    optend = (u_int8 *)ip + (ip->ip_hl << 2);


/* pass 2: Call (algorithm->update)() to calculate AH value, 
           zeroing where appropriate */

    while(optp < optend) {
      DP(GROSS_EVENT, optp, 08x);
      DP(GROSS_EVENT, optend, 08x);
      switch(*optp) 
	{

	  /* ZEROED for AH computation */
	case IPOPT_EOL:    
	  if (!(state = algorithm->update(state, (caddr_t)zero, 1))) {
	    DPRINTF(IDL_CRITICAL, ("algorithm->update failed (EOL)\n"));
	    goto ahcret;
	  }
	  optp = optend;
	  break;

	case IPOPT_NOP: 
	  if (!(state = algorithm->update(state, (caddr_t)zero, 1))) {
	    DPRINTF(IDL_CRITICAL, ("algorithm->update failed (NOP)\n"));
	    goto ahcret;
	  }
	  optp++;
	  break;

	case IPOPT_RR:
	case IPOPT_TS:
	  /* While RFC-791 is not crystal clear about the above two options,
	     there is good reason to believe that -- AS IMPLEMENTED in
	     The Internet -- these two options vary during transit in 
	     a manner unpredictable at the sending side.  */
	case IPOPT_LSRR:
	case IPOPT_SSRR:
	  /* The above two vary unpredictably 
	     because of the record-route action. */
	case 10:    /* Z Su's "Experimental Measurement" */
	case 15:    /* VerSteeg's ENCODE option */
	case 142:   /* Estrin's VISA option */
	case 144:   /* Lee's IMI Traffic Descriptor */
	case 145:   /* EIP per RFC-1385 */
	case 205:   /* Greg Finn's "Experimental Flow Control" */
	  /* This last set of  option numbers are from the 
	     Assigned Numbers RFC; included only for completeness. */
	  {
	    option_length = optp[1];
	    if (!(state = algorithm->update(state, (caddr_t)zero,
					    option_length))) {
	      DPRINTF(IDL_CRITICAL, ("algorithm->update failed (TS/RR)\n"));
	      goto ahcret;
	    }
	    optp += option_length;
	  }
	  break;


	case 11:  /* MTUP   - MTU Probe */
	case 12:  /* MTUR   - MTU Reply */
                  /* These have a length of 4 bytes */
	  {
	    if (!(state = algorithm->update(state, (caddr_t)zero, 4))) {
	      DPRINTF(IDL_CRITICAL, ("algorithm->update failed (MTU)\n"));
	      goto ahcret;
	    }
	    optp += 4;
	  }
	  break;

	case 82:  /* TR     - Traceroute; has fixed length of 12 bytes */
	  {
	    if (!(state = algorithm->update(state, (caddr_t)zero, 12))) {
	      DPRINTF(IDL_CRITICAL, ("algorithm->update failed (tracerte)\n"));
	      goto ahcret;
	    }
	    optp += 12;
	  }
	  break;


	  /* INCLUDED in AH computation */
	case IPOPT_SATID:
	case IPOPT_SECURITY:  /* IPSO - BSO, see RFC-1108 */
	case 133:             /* IPSO - ESO, see RFC-1108 */
	case 134:             /* CIPSO, no published RFC */
	  option_length = optp[1];
	  if (!(state = algorithm->update(state, (caddr_t)optp, 
					  option_length))) {
	    DPRINTF(IDL_CRITICAL, ("algorithm->update failed (opt %d)\n", 
				   *optp));
	    goto ahcret;
	  }
	  optp += option_length;
	  break;

        /* anything else we assume is completely invariant */
	default:
	  if (!(state = algorithm->update(state, (caddr_t)optp, optp[2]))) {
	    DPRINTF(IDL_CRITICAL, ("algorithm->update failed (default)\n"));
	    goto ahcret;
	  } /* endif */
	  optp += optp[1];
	} /* end switch (*optp) */
    } /* end while (optp < optend) */
#ifdef INET6
  }; /* endif version is 6 */
#endif

  DPRINTF(IDL_EVENT, ("AH calc: Starting main loop\n"));

  m = packet->m_next;
  while(m) {
    DP(GROSS_EVENT, m, 08x);
    DP(GROSS_EVENT, type, d);
    switch(type) {
#ifdef INET6
      case NEXTHDR_HOP:
      case NEXTHDR_DEST:
        /* Do bag processing here; this code needs to be written. rja */
      break;

      case NEXTHDR_ROUTING:
	{
	  u_int8 savenextaddr;
	  struct ipv6_srcroute0 *srh = mtod(m, struct ipv6_srcroute0 *);

	  /* next addr is variant, 
	     but its value at the receiver is known to be num addrs */
	  savenextaddr = srh->i6sr_nextaddr;
	  srh->i6sr_nextaddr = srh->i6sr_numaddrs;

	  /* compute up to next addr */
	  if (!(state = algorithm->update(state, (caddr_t)srh,
		8 + savenextaddr * sizeof(struct in_addr6))))
	    goto ahcret;

	  if (srh->i6sr_numaddrs > savenextaddr) {
	    /* insert current address at pointer */
	    if (!(state = algorithm->update(state,
		    (caddr_t)&(mtod(packet, struct ipv6 *)->ipv6_dst),
      		    sizeof(struct in_addr6))))
	      goto ahcret;
	    
	    /* sum the rest up to the last address, 
	       which we got in the base header */
	    if (!(state = algorithm->update(state, (caddr_t)((u_int8 *)srh
		    + 8 + savenextaddr * sizeof(struct in_addr6)),
		    (srh->i6sr_numaddrs - savenextaddr - 1) * 
					    sizeof(struct in_addr6))))
		  goto ahcret;
	  }

	  srh->i6sr_nextaddr = savenextaddr;
	}
	m = m->m_next;
	break;

      /* terminal headers for AH computation 
	 - everything including and after is opaque. */
      case NEXTHDR_IPV6:
      case NEXTHDR_ICMPV6:
#endif INET6
      case NEXTHDR_IPV4:
      case NEXTHDR_ICMPV4:
      case NEXTHDR_IGMP:
      case NEXTHDR_TCP:
      case NEXTHDR_UDP:
      case NEXTHDR_ESP:
        rval = 0;
	while (m) {
     	  if (!(state = algorithm->update(state, mtod(m, caddr_t), 
					  m->m_len))) {
	    DPRINTF(IDL_CRITICAL, ("AH calc: (transport) update failed\n"));
	    goto ahcret;
	  }
	  m = m->m_next;
	};
	break;

      case NEXTHDR_AUTH:
        {
	  struct ipv6_auth *ah = mtod(m, struct ipv6_auth *);
	  char zero[32];

	  DPRINTF(IDL_GROSS_EVENT, ("AH calc: Now summing AH\n"));

	  if (!(state = algorithm->update(state, mtod(m, caddr_t), 
					  sizeof(struct ipv6_auth)))) {
	    DPRINTF(IDL_CRITICAL, ("AH calc: auth header update failed\n"));
	    goto ahcret;
	  }

	  type = ah->auth_nexthdr;
	  DP(GROSS_EVENT, type, d);

	  bzero(zero, sizeof(zero));
	  i = ah->auth_datalen << 2;
	  DP(GROSS_EVENT, i, d);
	  while (i > 0) {
	    j = min(sizeof(zero), i);
	    if (!(state = algorithm->update(state, zero, j))) {
	      DPRINTF(IDL_CRITICAL, ("AH calc: (auth data) update failed\n"));
	      goto ahcret;
	    }
	    i -= j;
          };

	  m = m->m_next;
	};
        break;

#ifdef INET6
      case NEXTHDR_FRAGMENT:
#endif
      default:
        DPRINTF(IDL_CRITICAL, ("AH calc: illegal packet format\n"));
        goto ahcret;
    }; /* end switch */
  }; /* end while */
 /* intentionally fall thru the ahcret code */

 ahcret:
  /* cleanup then bail */ 
  DPRINTF(IDL_MAJOR_EVENT, ("\nipsec_ah_calc() finishing. rval=%d\n", rval));
  algorithm->finish(state, data);
  return rval;
}



/*----------------------------------------------------------------------
  ipsec_ah_input():   Handles input-side AH processing.

  NOTES:
     Because this is called from the inet6sw[] protocol switch, it
  MUST return void and cannot return int.  When this function fails
  for any reason, it increments appropriate counters and then drops
  the incoming mbuf chain (hence packet) on the floor silently.

     To prevent a spoofing attack from succeeding, the input check
  MUST also verify that the alleged Source Address is legitimate
  for this AH Security Association.  In the case of IP-AH-IP-ULP,
  the input check MUST also verify that either the Inner source address
  is equal to the Outer sources address or the M_AUTHENTIC bit is cleared.
----------------------------------------------------------------------*/
void ipsec_ah_input(incoming, extra, hdr_index, indexoff)
     struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *hdr_index;
     int indexoff;
{
  u_int32       spi;
  int           status;
  unsigned int  is_ipv6;
  struct sockaddr_in6 src;
  struct sockaddr_in6 dst;
  struct key_tblnode *ah_ktn;
  struct ipv6_auth *ah;
  struct ipsec_assoc *assoc;
  struct mbuf *datam;
  struct auth_algorithm *algorithm;
  struct in6_hdrindex newindex[IPV6_MAXHDRCNT];
  struct ipv6 *inner_v6;
  struct ip   *inner_v4;

  INIT;

  DPRINTF(IDL_GROSS_EVENT, 
	  ("Entering ah_input(in=%08x,extra=%d,hdr_index=%08x,indexoff=%d)\n",
	   incoming, extra, hdr_index, indexoff));

  DP(GROSS_EVENT, ntohs((mtod(incoming, struct ipv6 *))->ipv6_length), d);
  DP(GROSS_EVENT, incoming->m_pkthdr.len, d);
  DP(GROSS_EVENT, assoc, 08x);

 if (hdr_index == NULL) {
   /* NB: For IPv4, we might not have a header index because the normal
          IPv4 code doesn't use preparse, so we call preparse if needed */
    bzero(newindex, IPV6_MAXHDRCNT * sizeof(struct in6_hdrindex));
    incoming = ipv6_preparse(incoming, newindex, NEXTHDR_IPV4);
    if (!incoming)
      goto ahiret;

    hdr_index = newindex;

    for (indexoff = 1; hdr_index[indexoff].ihi_mbuf && 
	 (hdr_index[indexoff].ihi_nexthdr != NEXTHDR_AUTH); hdr_index++);
    if (!(hdr_index[indexoff].ihi_mbuf))
      goto ahiret;
  }

  ah = mtod(hdr_index[indexoff].ihi_mbuf, struct ipv6_auth *);

  /* Now build sockaddrs needed for getassocbyspi() call */
  /* Assumes: first mbuf in the incoming chain has initial part of hdr ! */
  is_ipv6 = (mtod(incoming, struct ip *)->ip_v == 6);

  if (is_ipv6)  {
    /* Is IPv6 */
    src.sin6_len    = sizeof(struct sockaddr_in6);
    src.sin6_family = AF_INET6;  
    src.sin6_flowinfo = 0; /* irrelevant */
    src.sin6_port   = 0;    /* irrelevant */
    IN6_ADDR_COPY(src.sin6_addr , mtod(incoming, struct ipv6 *)->ipv6_src);

    dst.sin6_len    = sizeof(struct sockaddr_in6);
    dst.sin6_family = AF_INET6;  
    dst.sin6_port   = 0;    /* irrelevant */
    dst.sin6_flowinfo = 0; /* irrelevant */
    IN6_ADDR_COPY(dst.sin6_addr , mtod(incoming, struct ipv6 *)->ipv6_dst);
  } else {
    /* must be IPv4, will choke on ST-II  */  
    ((struct sockaddr_in *) &src)->sin_len    = sizeof(struct sockaddr_in);
    ((struct sockaddr_in *) &src)->sin_family = AF_INET;  
    ((struct sockaddr_in *) &src)->sin_port   = 0; /* encrypted just now */ 
    ((struct sockaddr_in *) &src)->sin_addr   = 
                                    mtod(incoming, struct ip *)->ip_src;

    ((struct sockaddr_in *) &dst)->sin_len    = sizeof(struct sockaddr_in);
    ((struct sockaddr_in *) &dst)->sin_family = AF_INET;  
    ((struct sockaddr_in *) &dst)->sin_port   = 0; /* encrypted just now */
    ((struct sockaddr_in *) &dst)->sin_addr   = 
                                    mtod(incoming, struct ip *)->ip_dst; 
  } /* endif IP version */ 

  /* call Key Engine to obtain relevant Security Association. 
     Key Engine always needs SPIs in _host_ order.  rja */
  status = getassocbyspi(SS_AUTHENTICATION,
			 (struct sockaddr *) &src, (struct sockaddr *) &dst, 
			 ntohl(ah->auth_spi), &ah_ktn);

  if (status != 0)
    /* Key Engine didn't find Key Table Node/Security Association, so bail */
    goto ahiret;

  if (ah_ktn == NULL)
    goto ahiret;

  assoc = ah_ktn->secassoc;
  
  if (!assoc)
    goto ahiret;

  if (assoc->algorithm >= NAUTH)
    goto ahiret;

  algorithm = &(auth_algorithms[assoc->algorithm]);

  if (!algorithm->init)
    goto ahiret;

  DP(GROSS_EVENT, ah->auth_datalen, d);
  DP(GROSS_EVENT, algorithm->datalen, d);
  if (ah->auth_datalen != ((algorithm->datalen + 3) >> 2)) {
    goto ahiret;
  }

  if (!(datam = m_get(M_WAIT, MT_DATA)))
    goto ahiret;

  DP(GROSS_EVENT, algorithm->datalen, d);

  datam->m_len = algorithm->datalen;

  bzero(mtod(datam, caddr_t), algorithm->datalen);

  DP(GROSS_EVENT, datam->m_len, d);
  DDO(IDL_GROSS_EVENT, dump_mbuf(datam));

  DDO(IDL_GROSS_EVENT, dump_mchain(incoming));

  if (mtod(incoming, struct ip *)->ip_v == 4) 
    {
      IPV4_STORE_LENGTH(incoming);
      mtod(incoming, struct ip *)->ip_id = 
	htons(mtod(incoming, struct ip *)->ip_id);
    }

  /* call ipsec_ah_calc() to compute the Authentication Data 
     from the bits we received... */ 
  if (ipsec_ah_calc(incoming, assoc, algorithm, 
		    mtod(datam, caddr_t), hdr_index))
    goto ahiret;

  DDO(IDL_GROSS_EVENT, dump_mbuf(datam));
  DP(GROSS_EVENT, datam->m_len, d);

  DP(GROSS_EVENT, *((u_int8 *)((caddr_t)ah + sizeof(struct ipv6_auth))),02x);
  DP(GROSS_EVENT, *(mtod(datam, u_int8 *)), 02x);

  /* compare transmitted Auth Data to computed Auth Data; 
     if they don't match then increment counter and drop packet */
  if (bcmp(mtod(datam, caddr_t), 
	   (caddr_t)ah + sizeof(struct ipv6_auth), 
	   algorithm->datalen)) {
    /* If you *know* it's not authentic, then drop it and bail cleanly! */
    DPRINTF(IDL_CRITICAL, ("authentication failed!\n"));
    goto ahiret;
  }

  DPRINTF(IDL_MAJOR_EVENT, ("authentication passed!\n"));

  m_free(datam);

  incoming->m_flags |= M_AUTHENTIC;

  /* To prevent spoofing attacks that use authenticated IP-IP tunnels, 
     we NEED to force the M_AUTHENTIC flag to 0 whenever
     (1) inner IPv4 addr isn't same as outer IPv4 addr
     (2) inner IPv6 addr isn't same as outer IPv6 addr
  OR (3) inner packet and outer packet are different IP versions. */

  if ((is_ipv6 == 0) && (hdr_index[indexoff +1].ihi_nexthdr == NEXTHDR_IPV4)) {
    inner_v4 = mtod(hdr_index[indexoff +1].ihi_mbuf, struct ip *);
    if (inner_v4->ip_src.s_addr != 
	((struct sockaddr_in *)&src)->sin_addr.s_addr) {
      incoming->m_flags &= ~M_AUTHENTIC;
      ah_stats.ah_spoof++;
    }
  } else if ((is_ipv6) && 
	     (hdr_index[indexoff +1].ihi_nexthdr == NEXTHDR_IPV6)) {
    inner_v6 = mtod(hdr_index[indexoff +1].ihi_mbuf, struct ipv6 *);
    if (!(IN6_ADDR_EQUAL(inner_v6->ipv6_src, src.sin6_addr))) {
      incoming->m_flags &= ~M_AUTHENTIC;
      ah_stats.ah_spoof++;
    }
  } else if (( (is_ipv6) && 
	      (hdr_index[indexoff +1].ihi_nexthdr == NEXTHDR_IPV4)) ||
	     (!(is_ipv6) && 
	      (hdr_index[indexoff +1].ihi_nexthdr == NEXTHDR_IPV6))) {
    incoming->m_flags &= ~M_AUTHENTIC;
    ah_stats.ah_spoof++;
  }


  DPRINTF(IDL_GROSS_EVENT, ("Calling next\n"));
  DP(GROSS_EVENT, datam->m_len, d);
  DP(GROSS_EVENT, extra, d);


#ifdef INET6
/* IPv6 */
  if ((mtod(incoming, struct ip *))->ip_v == 6) {
    DP(GROSS_EVENT, extra, d);
    extra += sizeof(struct ipv6_auth) + (ah->auth_datalen << 2);

    DP(GROSS_EVENT, ntohs((mtod(incoming, struct ipv6 *))->ipv6_length), d);
    DP(GROSS_EVENT, incoming->m_pkthdr.len, d);

    DDO(IDL_EVENT, dump_ihitab(hdr_index));
    
    indexoff++;
    (*inet6sw[ipv6_protox[hdr_index[indexoff].ihi_nexthdr]].pr_input)
      (incoming, extra, hdr_index, indexoff);

  } else {
#endif /* IPv6 */
    /* Must be IPv4; will fail badly if fed ST-2 */

    struct ip *ip = mtod(incoming, struct ip *);
    int i;

    /* for now, we must eat the AH */
    incoming->m_pkthdr.len -= sizeof(struct ipv6_auth) + 
                              (ah->auth_datalen << 2);
    ip->ip_len = incoming->m_pkthdr.len - (ip->ip_hl << 2);
    ip->ip_id = ntohs(ip->ip_id);

    ip->ip_p = hdr_index[indexoff + 1].ihi_nexthdr;
    incoming->m_next = hdr_index[indexoff + 1].ihi_mbuf;

    for (datam = hdr_index[indexoff].ihi_mbuf; 
	 datam !=  hdr_index[indexoff + 1].ihi_mbuf; 
	 datam = m_free(datam));

    for (i = indexoff; hdr_index[i].ihi_mbuf; i++)
      hdr_index[i] = hdr_index[i+1];

    DDO(IDL_EVENT, dump_ihitab(hdr_index));

    (*inetsw[ip_protox[ip->ip_p]].pr_input)(incoming, extra, 
					    hdr_index, indexoff);

#ifdef INET6
  } /* endif was ipv6 packet */
#endif

  /* Inform the Key Engine that we are no longer using this Key Table Node */
  key_free(ah_ktn);
  ah_ktn = NULL;
  DPRINTF(IDL_EVENT, ("Leaving ah_input() OK\n"));
  ah_stats.ah_accepted++;
  return;

ahiret:
  ah_stats.ah_in_errs++;

  /* If Key Table Node pointer is valid,
     Inform the Key Engine that we are no longer using this Key Table Node */
  if (ah_ktn != NULL)
    key_free(ah_ktn);

  /* free incoming mbuf chain, perform any other needed cleanup, then bail */
  if (incoming)
    m_freem(incoming);

  DPRINTF(IDL_MAJOR_EVENT, ("Leaving ah_input() with ERROR\n"));
  return;
}


/*----------------------------------------------------------------------
 ipsec_ah_output():     Handles output-side AH processing.

 NOTES:
     Formerly in6_ahoutput().

     The assumption is made that the AH + the AH data fits into one normal 
   MLEN (=108 byte) mbuf.     If you're jumbo (>= 64 Kb) here, you LOSE.
----------------------------------------------------------------------*/
struct mbuf *ipsec_ah_output(outgoing, sa)
     struct mbuf *outgoing;  /* ptr to mbuf chain for outbound packet */
     struct ipsec_assoc *sa; /* ptr to IPsec Security Association to use */
{
  int divpoint;         /* index into hdrindex that pts to where AH will be */
  struct mbuf *ahm;
  struct mbuf  *m;
  struct ipv6_auth *ah;
  struct auth_algorithm *algorithm;
  struct in6_hdrindex index[IPV6_MAXHDRCNT];

  INIT;

  if ((sa == NULL) || (sa->algorithm >= NAUTH))  {
    ah_stats.ah_sa_invalid++;
    goto  ahoret;
  }

  DPRINTF(IDL_GROSS_EVENT, ("Entered ipsec_ah_output\n"));

  DP(GROSS_EVENT, outgoing->m_pkthdr.len, d);

  algorithm = &(auth_algorithms[sa->algorithm]);

  if (!algorithm->init)
    goto ahoret;

  if (!(ahm = m_get(M_WAIT, MT_DATA)))
    goto ahoret;

  ah = mtod(ahm, struct ipv6_auth *);

  /* Key Engine always keeps SPIs in _host_ order */
  ah->auth_spi = htonl(sa->spi);  
  ah->auth_datalen = (algorithm->datalen + 3) >> 2;
  ah->reserved = 0;
  bzero((ahm->m_data + sizeof(struct ipv6_auth)), algorithm->datalen);
  ahm->m_len = sizeof(struct ipv6_auth) + ((algorithm->datalen + 3) & (~3));

  bzero(index, IPV6_MAXHDRCNT * sizeof(struct in6_hdrindex));
  if (!(outgoing = ipv6_preparse(outgoing, index,
      ((mtod(outgoing, struct ip *))->ip_v == 4) ? 
      NEXTHDR_IPV4 : NEXTHDR_IPV6)))
    goto ahoret;

  {
    int i;
    u_int8 *nexthdr;

    if ((mtod(outgoing, struct ip *))->ip_v == 6)
      nexthdr = &(mtod(outgoing, struct ipv6 *)->ipv6_nexthdr);
    else {
      nexthdr = &(mtod(outgoing, struct ip *)->ip_p);
      mtod(outgoing, struct ip *)->ip_off = 
	htons(mtod(outgoing, struct ip *)->ip_off);
    }

    for (divpoint = 0, i = 1; index[i].ihi_mbuf && !divpoint; i++) {
      switch(index[divpoint].ihi_nexthdr) {

      case NEXTHDR_HOP:
      case NEXTHDR_DEST:
	nexthdr = &(mtod(index[i].ihi_mbuf, struct ipv6_opthdr *)->oh_nexthdr);
	break;
      case NEXTHDR_ROUTING:
	nexthdr = &(mtod(index[i].ihi_mbuf, struct ipv6_srcroute0 *)->i6sr_nexthdr);
	break;
      case NEXTHDR_FRAGMENT:
      case NEXTHDR_AUTH:
	DPRINTF(IDL_CRITICAL, ("pathological security processing!\n"));
	goto ahoret;
      default:
	divpoint = i;
      }
    }

    DPRINTF(IDL_GROSS_EVENT, ("divpoint = %d, nexthdr = %d\n", divpoint,
			      index[divpoint].ihi_nexthdr));

    ah->auth_nexthdr = *nexthdr;
    *nexthdr = NEXTHDR_AUTH;

    for (i = divpoint; index[i].ihi_mbuf; i++);
    for (; i >= divpoint; i--)
      bcopy(&(index[i]), &(index[i+1]), sizeof(struct in6_hdrindex));

    index[divpoint].ihi_mbuf = ahm;
    index[divpoint].ihi_nexthdr = NEXTHDR_AUTH;

    ahm->m_next = index[divpoint+1].ihi_mbuf;

    for (m = index[divpoint-1].ihi_mbuf; m->m_next && 
	 m->m_next != index[divpoint+1].ihi_mbuf; m = m->m_next);

    if (!(m->m_next)) {
      /* mbuf chain is hosed */
      m_free(ahm);
      goto ahoret;
    }

    m->m_next = ahm;
  }

  DP(GROSS_EVENT, outgoing->m_pkthdr.len, d);
  DP(GROSS_EVENT, ahm->m_len, d);
  outgoing->m_pkthdr.len += ahm->m_len;
  DP(GROSS_EVENT, outgoing->m_pkthdr.len, d);

  /* update the length field in the base IP header;
     assumes that m->pkthdr.len is valid */
  if (mtod(outgoing, struct ip *)->ip_v == 6) {
    IPV6_STORE_LENGTH(outgoing);
  } else {
    IPV4_STORE_LENGTH(outgoing);
  }

  DDO(IDL_GROSS_EVENT, dump_mchain(outgoing));

  if (ipsec_ah_calc(outgoing, sa, algorithm, 
	(ahm->m_data + sizeof(struct ipv6_auth)), index))
    goto ahoret;

  DP(GROSS_EVENT, *((u_int8 *)(ahm->m_data + sizeof(struct ipv6_auth))),
    02x);
  DDO(IDL_GROSS_EVENT, dump_mchain(outgoing));

  if (mtod(outgoing, struct ip *)->ip_v == 4) {
    mtod(outgoing, struct ip *)->ip_off = 
      htons(mtod(outgoing, struct ip *)->ip_off);
    mtod(outgoing, struct ip *)->ip_len = 
      htons(mtod(outgoing, struct ip *)->ip_len);
  }

  DP(GROSS_EVENT, outgoing->m_pkthdr.len, d);
  
  ah_stats.ah_sent++;
  return outgoing;

 ahoret:
  DPRINTF(IDL_CRITICAL, ("ipsec_ah_output returning ERROR\n"));
  if (outgoing)
    m_freem(outgoing);
  ah_stats.ah_out_errs++;
  return NULL;
}

/* EOF */
