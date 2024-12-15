/*----------------------------------------------------------------------
  ipsec_esp.c:   IPv4 & IPv6 Encapsulating Security Payload routines

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
#include <sys/time.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/kernel.h> 
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

#include "des.h"

/* Other Externals */
extern int ipsec_initted;    /* lives in ipsec.c */

extern struct protosw inet6sw[];  /* See in6_proto.c */
extern u_char ipv6_protox[NEXTHDR_MAX];

extern struct protosw inetsw[];
extern u_char ip_protox[IPPROTO_MAX];


/* MACROs */
#define DA(x) if (!(x)) { DPRINTF(IDL_CRITICAL, ("Assertion failed! " #x "\n")); /* panic("Assertion failed! " #x "\n"); */}

#define IPV6_STORE_LENGTH(m) (mtod(m, struct ipv6 *))->ipv6_length = \
	((m->m_pkthdr.len - sizeof(struct ipv6)) > 0xffff) ? 0 : \
	htons(m->m_pkthdr.len - sizeof(struct ipv6));
#define IPV4_STORE_LENGTH(m) (mtod(m, struct ip *))->ip_len = \
        htons(m->m_pkthdr.len);

/* Some implementation notes:

   * Whenever a function moves data from a source to a dest, its calling
     convention is ANSI C style -- dest first, then source. Do not confuse
     this with BSD style -- source first, then dest.

*/

#ifndef min
#define min(x, y) (x < y) ? x : y
#endif /* min(x, y) */
#ifndef max
#define max(x, y) (x > y) ? x : y
#endif /* max(x, y) */

#define INIT if (!ipsec_initted) ipsec_init()


#ifdef DEBUG
/*----------------------------------------------------------------------
  dummy*():    These functions are dummy functions that do not perform
               any encryption.  They are useful for debugging but
	       can't provide any kind of security.  They are also useful
	       to demonstrate how multiple algorithms can be supported
	       in this ESP implementation.  rja
----------------------------------------------------------------------*/
static caddr_t dummyencrinit(assoc, iv)
     struct ipsec_assoc *assoc;
     caddr_t    iv;
{
  return (caddr_t)assoc;
};

static caddr_t dummyencrencrypt(state, red, black)
     caddr_t state;
     caddr_t red;
     caddr_t black;
{
  bcopy(black, red, 8);
  return state;
}

static caddr_t dummyencrdecrypt(state, black, red)
     caddr_t state;
     caddr_t black;
     caddr_t red;
{
  bcopy(red, black, 8);
  return state;
}

static caddr_t dummyencrfinish(state)
     caddr_t state;
{
  return state;
}
#endif /* DEBUG */


struct descbcstate {
  des_key_schedule key;
  u_int8 iv[8];
};

/*----------------------------------------------------------------------
  des_cbc_init:   Initialise data items used for DES CBC algorithm.

----------------------------------------------------------------------*/
static caddr_t descbcinit(assoc, iv)
     struct ipsec_assoc *assoc;
     caddr_t iv;
{
  struct descbcstate *descbcstate;

  DPRINTF(IDL_EVENT, ("Entering descbcinit.\n"));
  descbcstate = malloc(sizeof(struct descbcstate), 
			M_TEMP, M_WAIT);

  DPRINTF(IDL_MAJOR_EVENT, ("descbcinit: DES key is %d \n", assoc->key));
  if (descbcstate) {
    des_cblock key;
    bzero(&key, sizeof(des_cblock));
    bcopy(assoc->key, &key, min(assoc->keylen, sizeof(des_cblock)));
    if (des_set_key(&key, descbcstate->key)) {
      /* NB: des_set_key() failure normally means DES key's parity is wrong. */
      DPRINTF(IDL_CRITICAL, ("descbcinit: des_set_key failed.\n"));
      free(descbcstate, M_TEMP);
      descbcstate = NULL;
    } else {
      bzero(descbcstate->iv, sizeof(descbcstate->iv));
      bcopy(iv, descbcstate->iv, min(assoc->ivlen, sizeof(descbcstate->iv)));
      if (assoc->ivlen == sizeof(u_int32))
	*((u_int32 *)&(descbcstate->iv[4])) = ~(*((u_int32 *)descbcstate->iv));
      DPRINTF(IDL_EVENT, ("Good descbcinit\n"));
    }
  } else {
    DPRINTF(IDL_CRITICAL, ("descbcinit: malloc failed.\n"));
  }

  DPRINTF(IDL_EVENT, ("Leaving descbcinit\n"));
  return (caddr_t)descbcstate;
}


/*----------------------------------------------------------------------
  descbcencrypt:  Probably should be inlined.  Just calls des_cbc_encrypt().

----------------------------------------------------------------------*/
static caddr_t descbcencrypt(descbcstate, black, red)
     struct descbcstate *descbcstate;
     caddr_t black;
     caddr_t red;
{
  des_cbc_encrypt(red, black, 8, descbcstate->key, descbcstate->iv, 1);
  bcopy(black, descbcstate->iv, 8);
  return (caddr_t)descbcstate;
}

/*----------------------------------------------------------------------
  descbcdecrypt:  Probably should be inlined.  Just calls des_cbc_encrypt().

----------------------------------------------------------------------*/
static caddr_t descbcdecrypt(descbcstate, red, black)
     struct descbcstate *descbcstate;
     caddr_t red;
     caddr_t black;
{
  des_cbc_encrypt(black, red, 8, descbcstate->key, descbcstate->iv, 0);
  bcopy(black, descbcstate->iv, 8);
  return (caddr_t)descbcstate;
}

/*----------------------------------------------------------------------
  descbcfinish:  Frees data structures used temporarily by DES CBC
                 and performs any other needed cleanup.  rja
----------------------------------------------------------------------*/
static caddr_t descbcfinish(state)
     caddr_t state;
{
  free(state, M_TEMP);
  return state;
}


struct mbuf *descbcbuildhdr __P((struct mbuf *, struct ipsec_assoc *,
				 u_int *, u_int8 *));
struct mbuf *descbcparsehdr __P((struct mbuf *, struct ipsec_assoc *,
				 u_int *, u_int8 *));


/* NENCR == highest ESP alg number in this implementation */
#ifdef DEBUG
#define NENCR 1
#else
#define NENCR 0     
#endif

static struct encr_algorithm {
  struct mbuf *(*buildhdr)(struct mbuf *, struct ipsec_assoc *, u_int *, 
			   u_int8 *);
  struct mbuf *(*parsehdr)(struct mbuf *, struct ipsec_assoc *, u_int *, 
			   u_int8 *);
  caddr_t (*init)(struct ipsec_assoc *, caddr_t);
  caddr_t (*encrypt)(caddr_t, caddr_t, caddr_t); /* state, dst, src */
  caddr_t (*decrypt)(caddr_t, caddr_t, caddr_t);
  caddr_t (*finish)(caddr_t);
  int blocksize;
} encr_algorithms[] = {
{ descbcbuildhdr, descbcparsehdr, descbcinit, descbcencrypt, 
    descbcdecrypt, descbcfinish, 8 },
#ifdef DEBUG
{ descbcbuildhdr, descbcparsehdr, dummyencrinit, dummyencrencrypt,
    dummyencrdecrypt, dummyencrfinish, 8 },
#endif
 /* if any new algorithms are added here, you will also need to
    update the IPSEC_ALGTYPE_ESP_* symbols in ipsec.h and NENCR above. */
{ NULL, NULL, NULL, NULL, NULL, NULL, 0 }
};


/*----------------------------------------------------------------------
  runblock:  Traverse mbuf chain handling mbuf weirdnesses,
             gather data into cipher block aligned unit, 
	     apply encrypt/decrypt function, and handle padding.
	     Return with resulting encrypted/decrypted data.
----------------------------------------------------------------------*/
static int runblock(dstm, srcm, off, buf, bs, f, state, pad, padparam)
     struct mbuf *dstm;
     struct mbuf *srcm;
     int off;
     caddr_t buf;
     int bs;
     caddr_t (*f)(caddr_t, caddr_t, caddr_t);
     caddr_t state;
     int (*pad)(struct mbuf *, int, caddr_t, int, 
     caddr_t (*)(caddr_t, caddr_t, caddr_t), caddr_t, caddr_t);
     caddr_t padparam;
{
    int dstoff = dstm->m_len;
    int srcoff = 0, len = 0, i;
    caddr_t src = mtod(srcm, caddr_t);
    caddr_t dst = mtod(dstm, caddr_t) + dstoff;

    DPRINTF(IDL_MAJOR_EVENT, ("Entering runblock, off = %d, bs = %d\n", 
			      off, bs));

    while(1) {
      i = min(srcm->m_len - srcoff, bs - off);
      DP(GROSS_EVENT, i, d);
      DP(GROSS_EVENT, off, d);
      DP(GROSS_EVENT, srcoff, d);
      DP(GROSS_EVENT, dstoff, d);
      DP(GROSS_EVENT, srcm, 08x);
      DP(GROSS_EVENT, srcm->m_len, d);
      if (off || (i < bs)) {
	bcopy(src, buf + off, i);
	off += i;
	DPRINTF(IDL_EVENT, ("Reblocking\n"));
	src += i;
      }
      DA(off <= bs);
      if (!off || (off == bs)) {
	state = f(state, dst, off ? buf : src);
	dstoff += bs;
	dst += bs;
	if (!off)
	  src += i;
	DP(GROSS_EVENT, dstoff, d);
	if ((dstoff + bs) > MCLBYTES) {
	  DPRINTF(IDL_GROSS_EVENT, ("Grabbing a new cluster!\n"));
	  if (!(dstm->m_next = m_get(M_NOWAIT, MT_DATA))) {
	    DPRINTF(IDL_CRITICAL, ("runblock: m_get failed!\n"));
	    return -1;
	  }
	  MCLGET(dstm->m_next, M_NOWAIT);
	  if (!(dstm->m_next->m_flags & M_EXT)) {
	    DPRINTF(IDL_CRITICAL, ("runblock: MCLGET failed!\n"));
	    return -1;
	  }
	  dstm->m_len = dstoff;
	  dstoff = 0;
	  dstm = dstm->m_next;
	  dst = mtod(dstm, caddr_t);
	}
	off = 0;
      }
      srcoff += i;
      if (srcoff > srcm->m_len) {
	DDO(IDL_CRITICAL, dump_mbuf(srcm));
        DP(CRITICAL, srcoff, d);
	return -1;
      }
      if (srcm->m_len == srcoff) {
	DPRINTF(IDL_GROSS_EVENT, ("runblock: Advancing\n"));
	srcm = srcm->m_next;
	srcoff = 0;
	if (!srcm)
	  break;
	src = mtod(srcm, caddr_t);
      }
    }
    dstm->m_len = dstoff;
    dstm->m_next = NULL;

    if (pad) {
      if ((len = pad(dstm, off, buf, bs, f, state, padparam)) < 0) {
	DPRINTF(IDL_CRITICAL, ("runblock: pad function failed\n"));
	return -1;
      }
    } else
      len = 0;

    if ((len + off) % bs) {
      DPRINTF(IDL_CRITICAL, 
	      ("runblock: packet isn't an even number of blocks!\n"));
      return (-1);
    }

    return (len);
}

/*----------------------------------------------------------------------
  descbcparsehdr:   High-level input-side ESP processing for DES CBC.
                    Calls runblock() to handle actual decryption.
		    Returns new cluster mbuf chain rather than processing
		    in situ.

		    This is potentially reusable for IDEA and other block
		    oriented algorithms.  Protocol processing is currently
		    pretty slow and needs speedup.
----------------------------------------------------------------------*/
struct mbuf *descbcparsehdr(incoming, assoc, pkthdr_len, nexthdr)
     struct mbuf *incoming;
     struct ipsec_assoc *assoc;
     u_int *pkthdr_len;
     u_int8 *nexthdr;
{
  struct encr_algorithm *algorithm = &(encr_algorithms[assoc->algorithm]);
  u_int8 blockbuffer[algorithm->blocksize];
  caddr_t state;
  int padlen, i;
  struct mbuf *espm;

  DPRINTF(IDL_EVENT, ("entering descbcparsehdr\n"));

  if (!(espm = m_get(M_NOWAIT, MT_DATA)))
    return NULL;

  MCLGET(espm, M_NOWAIT);

  if (!(espm->m_flags & M_EXT)) {
    m_free(espm);
    return NULL;
  }

  espm->m_len = 0;

  DP(EVENT, assoc->ivlen, d);

  if (incoming->m_len < assoc->ivlen) {
    if (!(incoming = m_pullup(incoming, assoc->ivlen))) {
      m_free(espm);
      return NULL;
    }
  }
    
  if (!(state = algorithm->init(assoc, mtod(incoming, caddr_t)))) {
    m_free(espm);
    return NULL;
  }

  incoming->m_data += assoc->ivlen;
  incoming->m_len -= assoc->ivlen;
  if (!incoming->m_len)
    incoming = m_free(incoming);
  *pkthdr_len -= assoc->ivlen;

  DP(GROSS_EVENT, algorithm->blocksize, d);

  if ((runblock(espm, incoming, 0, &blockbuffer, algorithm->blocksize,
		algorithm->decrypt, state, NULL, (caddr_t)nexthdr)) < 0) {

/* N.B. - If algorithm->decrypt returns a different state pointer than is 
   passed in, this will fail. So, for now, don't go changing the state 
   pointer.
*/
    DPRINTF(IDL_CRITICAL, ("descbcparsehdr: runblock failed!\n"));
    algorithm->finish(state);
    m_freem(espm);
    return NULL;
  }

  algorithm->finish(state);

  {
    struct mbuf *m, *pm;
    int i;

    for (pm = NULL, m = espm; m->m_next; pm = m, m = m->m_next);
    *nexthdr = *((u_int8 *)(mtod(m, caddr_t) + m->m_len - 1));
    padlen = *((u_int8 *)(mtod(m, caddr_t) + m->m_len - 2)) + 2;
    i = (int)m->m_len - (int)padlen;
    if (i <= 0) {
      if (!pm) {
	m_freem(espm);
	return NULL;
      }
      padlen -= m->m_len;
      m_free(m);
      pm->m_next = NULL;
      m = pm;
    }
    m->m_len -= padlen;
  }

  *pkthdr_len -= padlen;

  m_freem(incoming);
  return espm;
}

/*----------------------------------------------------------------------
  descbcpad:  Output-side processing function.
              Handles padding out to an integral number of CBC blocks.
              Also handles trailer-processing for the protected block.
----------------------------------------------------------------------*/
static int descbcbpad(dstm, off, buf, bs, f, state, padparam)
     struct mbuf *dstm;
     int off;
     caddr_t buf;
     int bs;
     caddr_t (*f)(caddr_t, caddr_t, caddr_t);
     caddr_t state;
     caddr_t padparam;
{
  int padlen = 0, i;
  u_long randombuffer;

  DPRINTF(IDL_GROSS_EVENT, ("in pad, off = %d, bs = %d, dstm->m_len = %d\n", off, bs, dstm->m_len));

  if (off > (bs - 2)) {
    DPRINTF(IDL_GROSS_EVENT, ("Padding spans two blocks...\n"));
    /* now obtain some not entirely obvious data to use for padding... */ 
    randombuffer = random();
    bcopy((caddr_t)&randombuffer, buf + off, bs - off);
    state = f(state, mtod(dstm, caddr_t) + dstm->m_len, buf);
    dstm->m_len += bs;
    padlen = bs - off;
    off = 0;
    if ((dstm->m_len + bs) > MCLBYTES) {
      if (!(dstm->m_next = m_get(M_NOWAIT, MT_DATA)))
	return -1;
      dstm = dstm->m_next;
      dstm->m_next = NULL;
      dstm->m_len = 0;
    }
  }

  DPRINTF(IDL_GROSS_EVENT, ("Entering main pad loop, padlen = %d.\n", padlen));

  while (off < (bs - 2)) {
    i = min(sizeof(randombuffer), bs - 2 - off);
    DP(GROSS_EVENT, i, d);
    DP(GROSS_EVENT, off, d);
    randombuffer = random();
    bcopy((caddr_t)&randombuffer, buf + off, i);
    off += i;
    padlen += i;
  }

  DP(GROSS_EVENT, padlen, d);
  ((u_int8 *)buf)[bs - 2] = padlen;
  ((u_int8 *)buf)[bs - 1] = *((u_int8 *)padparam);
  DP(GROSS_EVENT, *((u_int8 *)padparam), d);
  DP(GROSS_EVENT, randombuffer, d);
  DP(GROSS_EVENT, dstm->m_len, d);
  state = f(state, mtod(dstm, caddr_t) + dstm->m_len, buf);
  dstm->m_len += bs;

  return (padlen + 2);
}


/*----------------------------------------------------------------------
  descbcbuildhdr:   Main routine called from ipsec_esp_*_output() to
                    process outbound packet, encrypt the appropriate
		    portion of the outbound packet, adjust m_pkthdr.len,
		    and sundry other things.  Calls runblock() to
		    do the actual encryption stuff.
----------------------------------------------------------------------*/
struct mbuf *descbcbuildhdr(outgoing, assoc, pkthdr_len, nexthdr)
     struct mbuf *outgoing;      /* Packet to encrypt */
     struct ipsec_assoc *assoc; 
     u_int *pkthdr_len;          /* ptr to m_pkthdr.len of outgoing */
     u_int8 *nexthdr;            
{
  struct mbuf *espm, *m;
  int padlen, i, off;
  struct encr_algorithm *algorithm = &(encr_algorithms[assoc->algorithm]);
  caddr_t p, state;
  caddr_t iv = assoc->iv;
  u_int32 foldediv[2];
  u_int8 blockbuffer[algorithm->blocksize];
  u_long randombuffer;

  DPRINTF(IDL_EVENT, ("Entered descbcbuildhdr().\n"));

  /* if nothing to encrypt, bail */
  if (outgoing == NULL) {
    DPRINTF(IDL_CRITICAL, ("descbcbuildhdr: outgoing equal to NULL.\n"));
    return NULL;
  }

  /* get a new cluster mbuf to store the results into */
  espm = m_get(M_NOWAIT, MT_DATA);
  if (espm == NULL) {
    DPRINTF(IDL_CRITICAL, ("descbcbuildhdr: m_get failed.\n"));
    return NULL;
  }

  MCLGET(espm, M_NOWAIT);

  if (!(espm->m_flags & M_EXT)) {
    DPRINTF(IDL_CRITICAL, ("descbcbuildhdr: MCLGET failed.\n"));
    m_freem(espm);
    return NULL;
  }

  p = mtod(espm, caddr_t);

  /* stick SPI into front of new cluster mbuf (recalling that the
     Key Engine always keeps SPIs in _host_ order),  then
     adjust pkthdrlen by sizeof(SPI) */

  *((u_int32 *)p) = htonl(assoc->spi);
  espm->m_len = sizeof(u_int32);
  p += sizeof(u_int32);
  *pkthdr_len += sizeof(u_int32);

  /* stick association-dependent IV/Cryptosync data right after SPI,
     adjust pkthdrlen by sizeof(IV) */
  bcopy(assoc->iv, p, assoc->ivlen);
  espm->m_len += assoc->ivlen;
  p += assoc->ivlen;
  *pkthdr_len += assoc->ivlen;

  DP(GROSS_EVENT, espm->m_len, d);

  /* if transmitting a 32-bit IV, 
     convert it into a 64-bit IV for actual encryption purposes.
     NB: This implementation ONLY supports IV sizes of (32 bits, 64 bits). */
  if (assoc->ivlen == sizeof(u_int32)) {
    foldediv[0] = *((u_int32 *)assoc->iv);
    foldediv[1] = ~foldediv[0];
    iv = (caddr_t)&foldediv;
  }

  state = algorithm->init(assoc, iv);
  if (!(state)) {
    DPRINTF(IDL_CRITICAL, ("descbcbuildhdr: alg init failed.\n"));
    m_free(espm);
    return NULL;
  }

  padlen = runblock(espm, outgoing, 0, &blockbuffer, algorithm->blocksize,
		    algorithm->encrypt, state, descbcbpad, (caddr_t)nexthdr);

  /* finish encryption */
  algorithm->finish(state);

  /* if runblock failed, then clean up and bail */
  if (padlen < 0)  {
    /* 
       N.B. - If algorithm->encrypt returns a different state pointer than is 
       passed in, this will fail. So, for now, don't go changing the state 
       pointer.  No, we can't imagine why you'd _want_  to change the state
       pointer but we thought we'd mention it just in case.  cm rja     
    */
    DPRINTF(IDL_CRITICAL, ("descbcbuildhdr: padlen < 0 failure.\n"));
    m_freem(espm);
    return NULL;
  }

  /* adjust pkthdrlen to reflect increased size due to padding out to
     an integral number of CBC blocks */
  *pkthdr_len += padlen;
  DPRINTF(IDL_EVENT, ("Left descbcbuildhdr() OK.\n"));
  return espm;
}


/*----------------------------------------------------------------------
  ipsec_esp_transport_output:   Transport-mode output processing and
           configured ESP tunnel output processings, but not including 
	   socket-requested tunnel-mode processing.  See ip*_output()
	   for magic relating to configured ESP tunnel output processing.

	   For configured tunnel case, "outgoing" mbuf chain contains
	        IP->IP->Upper-layer-protocol
	   when this function is called; 
	   otherwise the "outgoing" mbuf chain just contains 
	        IP->ULP.	
----------------------------------------------------------------------*/
int ipsec_esp_transport_output(outgoing, sa)
     struct mbuf *outgoing;  
     struct ipsec_assoc *sa;
{
  struct ipsec_assoc *transportassoc;
  int divpoint;
  struct encr_algorithm *algorithm;
  u_int8 nexthdr;
  struct mbuf *reddata;
  struct mbuf *m;

  DPRINTF(IDL_MAJOR_EVENT, ("Entering ipsec_esp_transport_output()\n"));

  INIT;

  DP(GROSS_EVENT, ntohs((mtod(outgoing, struct ipv6 *))->ipv6_length), d);
  DP(GROSS_EVENT, outgoing->m_pkthdr.len, d);

  if ((sa == NULL) || (sa->algorithm > NENCR)) {
    esp_stats.esp_sa_invalid++;
    esp_stats.esp_trans_out_errs++;
    return -1;
  }

  algorithm = &(encr_algorithms[sa->algorithm]);

  if (!algorithm->buildhdr) {
    DPRINTF(IDL_CRITICAL, ("esp_transport_output: no buildhdr fn ptr.\n"));
    esp_stats.esp_trans_out_errs++;
    return -1;
  }

  DDO(IDL_GROSS_EVENT, dump_mchain(outgoing));

  nexthdr = NEXTHDR_ESP;
  divpoint = ipv6_finddivpoint(outgoing, &nexthdr);

  DP(GROSS_EVENT, divpoint, d);

  outgoing->m_flags &= ~M_PKTHDR;

  reddata = m_split(outgoing, divpoint, M_WAIT);
  if (!(reddata)) {
    DPRINTF(IDL_CRITICAL, ("esp_transport_output: m_split failed.\n"));
    esp_stats.esp_trans_out_errs++;
    return -1;
  }
  if (!reddata->m_len) {
    DPRINTF(IDL_MAJOR_EVENT, ("In ESP transport output, got a bogon\n"));
    dump_mchain(reddata);
    reddata = m_free(reddata);
    if (!reddata) {
      esp_stats.esp_trans_out_errs++;
      return -1;
    }
  }

  outgoing->m_flags |= M_PKTHDR;

  for (m = outgoing; m->m_next; m = m->m_next);

  /* line below will return RH pieces of mbuf chain, 
     but possibly not the same mbufs as before so be careful... */
  m->m_next = algorithm->buildhdr(reddata, sa, 
				  &outgoing->m_pkthdr.len, &nexthdr);
  m_freem(reddata);

  /* Increment DES Initialisation Vector 
     Will rework this to get more IV stirring (longer-term) */
  if ((sa->algorithm == IPSEC_ALGTYPE_ESP_DES_CBC) ||
      (sa->algorithm == IPSEC_ALGTYPE_ESP_3DES)) {

    /* DES IVs used with IPSEC ESP can be stored as 32 bits or 64 bits */
    if (sa->ivlen == sizeof(u_quad_t)) {
      (*(u_quad_t *) (sa->iv))++; /* increment 64 bits worth of IV */
      if (0 == (*(u_quad_t *)(sa->iv)))
	(*(int *) (sa->iv)) = random();
    } else if (sa->ivlen == sizeof(u_int32)) {
      (*(int *)(sa->iv))++;       /* increment 32 bits worth of IV */
      if (0 == (*(int *)(sa->iv)))
	(*(int *) (sa->iv)) = random();
    } else {
      /* DES IV _must_ be either 32bits or 64bits; bail if not true */
      esp_stats.esp_sa_invalid++;
      esp_stats.esp_trans_out_errs++;
      return -1;
    }/* endif if IV is 64 bits */
  } /* endif this ESP algorithm uses an IV */

  if (m->m_next == NULL) {
    DPRINTF(IDL_CRITICAL, ("esp_transport_output: buildhdr failed. \n"));
    esp_stats.esp_trans_out_errs++;
    return -1;
  }

  DDO(IDL_GROSS_EVENT, dump_mchain(outgoing));

  DP(GROSS_EVENT, ntohs((mtod(outgoing, struct ipv6 *))->ipv6_length), d);
  DP(GROSS_EVENT, outgoing->m_pkthdr.len, d);
  DPRINTF(IDL_MAJOR_EVENT, ("Left ipsec_esp_transport_output()\n"));

  esp_stats.esp_transport_sent++;
  return 0;
}


/*----------------------------------------------------------------------
  ipsec_esp_tunnel_output:  Handles socket-requested tunnel-mode 
                  ESP output processing.  Note that configured ESP
		  tunnels are handled by ipsec_esp_transport_output().
----------------------------------------------------------------------*/
int ipsec_esp_tunnel_output(outgoing, sa)
     struct mbuf *outgoing;  /* outgoing contains IP + ULP */
     struct ipsec_assoc *sa; /* ptr to IPsec Security Association to use */
{
  struct ipsec_assoc *tunnelassoc;
  struct encr_algorithm *algorithm;
  u_int8 nexthdr;
  struct mbuf *reddata, *m;

  DPRINTF(IDL_MAJOR_EVENT, ("Entering ipsec_esp_tunnel_output()\n"));

  INIT;

  DP(GROSS_EVENT, ntohs((mtod(outgoing, struct ipv6 *))->ipv6_length), d);
  DP(GROSS_EVENT, outgoing->m_pkthdr.len, d);

  if ((sa == NULL) || (sa->algorithm > NENCR)) {
    DPRINTF(IDL_CRITICAL, ("esp_tunnel_output:  Invalid Sec Assoc. \n"));
    esp_stats.esp_sa_invalid++;
    esp_stats.esp_tunnel_out_errs++;
    return 0;
  }

  algorithm = &(encr_algorithms[sa->algorithm]);

  if (algorithm->buildhdr == NULL) {
    DPRINTF(IDL_CRITICAL, ("esp_tunnel_output: no buildhdr func ptr.\n"));
    esp_stats.esp_tunnel_out_errs++; 
    return -1;
  }

  DDO(IDL_GROSS_EVENT, dump_mchain(outgoing));

  /* Prepare packet for this algorithm's buildhdr function */
  if (mtod(outgoing, struct ip *)->ip_v == 4) {
    struct ip *ip = mtod(outgoing, struct ip *);
    nexthdr = NEXTHDR_IPV4;
    IPV4_STORE_LENGTH(outgoing);
    ip->ip_sum = 0;
    ip->ip_sum = in_cksum(outgoing, ip->ip_hl << 2);
  } else {
    nexthdr = NEXTHDR_IPV6;
    IPV6_STORE_LENGTH(outgoing);
  }

  /* Call this algorithm's buildhdr function;
     the buildhdr function will cause encryption to occur.  */
  reddata = algorithm->buildhdr(outgoing, sa, 
				&outgoing->m_pkthdr.len, &nexthdr);
  if (reddata == 0) {
    DPRINTF(IDL_CRITICAL, ("esp_tunnel_output: buildhdr failed.\n"));
    esp_stats.esp_tunnel_out_errs++;
    return -1;
  }

  /* Increment DES Initialisation Vector 
     Will rework this to get more IV stirring (longer-term) */
  if ((sa->algorithm == IPSEC_ALGTYPE_ESP_DES_CBC) ||
      (sa->algorithm == IPSEC_ALGTYPE_ESP_3DES)) {

    /* DES IVs used with IPSEC ESP can be stored as 32 bits or 64 bits */
    if (sa->ivlen == sizeof(u_quad_t)) {
      (*(u_quad_t *) (sa->iv))++; /* increment 64 bits worth of IV */
      if (0 == (*(u_quad_t *)(sa->iv)))
	(*(int *) (sa->iv)) = random();
    } else if (sa->ivlen == sizeof(u_int32)) {
      (*(int *)(sa->iv))++;       /* increment 32 bits worth of IV */
      if (0 == (*(int *)(sa->iv)))
	(*(int *) (sa->iv)) = random();
    } else {
      /* DES IV _must_ be either 32bits or 64bits; bail if not true */
      esp_stats.esp_sa_invalid++;
      esp_stats.esp_tunnel_out_errs++;
      return -1;
    }/* endif if IV is 64 bits */
  } /* endif this ESP algorithm uses an IV */

  outgoing->m_flags &= ~M_PKTHDR;

  /* Now split the mbuf chain, insert a second IP hdr if needed, 
     and fix up outer (cleartext) IP hdr with corrected values */
  if (mtod(outgoing, struct ip *)->ip_v == 4) {
    m = m_split(outgoing, sizeof(struct ip), M_NOWAIT);
    if (m == NULL) {
      DPRINTF(IDL_CRITICAL, ("esp_tunnel_output: m_split failed.\n "));
      esp_stats.esp_tunnel_out_errs++;
      return -1;
    }

    outgoing->m_pkthdr.len += sizeof(struct ip);
    mtod(outgoing, struct ip *)->ip_p = NEXTHDR_ESP;
    IPV4_STORE_LENGTH(outgoing);
  } else {
    m = m_split(outgoing, sizeof(struct ipv6), M_NOWAIT);
    if (m == NULL) {
      esp_stats.esp_tunnel_out_errs++;
      return -1;
    }
    outgoing->m_pkthdr.len += sizeof(struct ipv6);
    mtod(outgoing, struct ipv6 *)->ipv6_nexthdr = NEXTHDR_ESP;
    IPV6_STORE_LENGTH(outgoing);
  }

  outgoing->m_flags |= M_PKTHDR;

  m_freem(m);

  /* go to last mbuf and stick ptr to the encrypted data there */
  for (m = outgoing; m->m_next; m = m->m_next);
  m->m_next = reddata;

  DDO(IDL_GROSS_EVENT, dump_mchain(outgoing));

  DP(GROSS_EVENT, ntohs((mtod(outgoing, struct ipv6 *))->ipv6_length), d);
  DP(GROSS_EVENT, outgoing->m_pkthdr.len, d);
  DPRINTF(IDL_MAJOR_EVENT, ("Left ipsec_esp_tunnel_output()\n"));

  esp_stats.esp_tunnel_sent++;
  return 0;
}


/*----------------------------------------------------------------------
  ipsec_esp_input:  This routine handles the IP ESP processing for
           received IPv4 and IPv6 datagrams. 

  NOTES:   Because of the encryption, there cannot be and does not need 
           to be separate processing on the receive-side for tunnel-mode 
           ESP vs. transport-mode ESP.  

	   This returns void rather than int with status because it
	   is called from the inet6sw[] protocol switch.

	   In event of any failure, the function aborts by dropping
	   the incoming mbuf chain (hence packet) silently on the floor
	   and increments appropriate statistics counters.  rja
----------------------------------------------------------------------*/
void ipsec_esp_input(incoming, extra, index, indexoff)
     struct mbuf *incoming;
     int extra;
     struct in6_hdrindex *index;
     int indexoff;
{
  struct key_tblnode *esp_ktn = NULL;
  u_int32 spi;
  struct sockaddr_in6 src;
  struct sockaddr_in6 dst;
  int status;
  unsigned int is_ipv6;
  struct ipsec_assoc *assoc;
  struct encr_algorithm *algorithm;
  u_int8 *nexthdr;
  struct mbuf *m;
  struct in6_hdrindex newindex[IPV6_MAXHDRCNT];
  struct ipv6 *inner_v6;
  struct ip   *inner_v4;

  DPRINTF(IDL_MAJOR_EVENT, 
  ("Entered ipsec_esp_input(incoming=%08x,extra=%d,index=%08x,indexoff=%d)\n", 
   incoming, extra, index, indexoff));

  DP(GROSS_EVENT, ntohs((mtod(incoming, struct ipv6 *))->ipv6_length), d);
  DP(GROSS_EVENT, incoming->m_pkthdr.len, d);

  INIT;

  DDO(IDL_GROSS_EVENT, dump_mchain(incoming));

  if (!index) {
    bzero(newindex, IPV6_MAXHDRCNT * sizeof(struct in6_hdrindex));
    incoming = ipv6_preparse(incoming, newindex, NEXTHDR_IPV4);
    if (!(incoming))
      goto espiret;

    index = newindex;

    for (indexoff = 1; index[indexoff].ihi_mbuf && 
	 (index[indexoff].ihi_nexthdr != NEXTHDR_ESP); index++);
    if (!(index[indexoff].ihi_mbuf))
      goto espiret;
  }

  /* get SPI off the front of the ESP */
  spi = *((u_int32 *)index[indexoff].ihi_mbuf->m_data);

  {
    /* strip SPI from the mbuf containing the ESP */
    index[indexoff].ihi_mbuf->m_data += sizeof(u_int32);
    index[indexoff].ihi_mbuf->m_len -= sizeof(u_int32);
    incoming->m_pkthdr.len -= sizeof(u_int32);
  }
  /* now go and build sockaddrs needed by Key Engine */
  /* Assumes: first mbuf in the incoming chain has initial part of hdr ! */
  is_ipv6 = (mtod(incoming, struct ip *)->ip_v == 6);
  if (is_ipv6) {
    /* Is IPv6 */
    src.sin6_len    = sizeof(struct sockaddr_in6);
    src.sin6_family = AF_INET6;  
    src.sin6_flowinfo = 0; /* irrelevant */
    src.sin6_port   = 0; /* encrypted just now */
    IN6_ADDR_COPY(src.sin6_addr , mtod(incoming, struct ipv6 *)->ipv6_src);

    dst.sin6_len    = sizeof(struct sockaddr_in6);
    dst.sin6_family = AF_INET6;  
    dst.sin6_port   = 0; /* encrypted just now */
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
     Always use SS_ENCRYPTION_TRANSPORT even though we can't really
     know what is in use (tunnel or transport).  Key Engine always
     uses SPIs in _host_ order.  rja */

  status = getassocbyspi(SS_ENCRYPTION_TRANSPORT, 
			 (struct sockaddr *) &src, (struct sockaddr *) &dst, 
			 ntohl(spi), &esp_ktn);
  if (status != 0) {
    esp_stats.esp_sa_unavailable++;
    /* Security Association/Key Table Node was not found by Key Engine, bail */
    goto espiret;
  }

  if (esp_ktn == NULL)
    goto espiret;

  assoc = esp_ktn->secassoc;

  if (assoc == NULL)
    goto espiret;

  if (assoc->algorithm > NENCR)
    goto espiret;

  algorithm = &(encr_algorithms[assoc->algorithm]);

  if (!algorithm->parsehdr)
    goto espiret;

  DP(GROSS_EVENT, extra, d);

  /* now parse the cleartext headers */
  for (m = index[indexoff-1].ihi_mbuf; m->m_next != 
       index[indexoff].ihi_mbuf; m = m->m_next);

  switch(index[indexoff-1].ihi_nexthdr) {

  case NEXTHDR_IPV4:
    nexthdr = &(mtod(index[indexoff-1].ihi_mbuf, struct ip *)->ip_p);
    break;

  case NEXTHDR_IPV6:
    nexthdr = &(mtod(index[indexoff-1].ihi_mbuf, struct ipv6 *)->ipv6_nexthdr);
    break;

  case NEXTHDR_HOP:
  case NEXTHDR_DEST:
    nexthdr = &(mtod(index[indexoff-1].ihi_mbuf, struct ipv6_opthdr *)->oh_nexthdr);
    break;

  case NEXTHDR_AUTH:
    nexthdr = &(mtod(index[indexoff-1].ihi_mbuf, struct ipv6_auth *)->auth_nexthdr);
    break;

  case NEXTHDR_ROUTING:
    /* Needs to switch off type... */
    if (!mtod(index[indexoff-1].ihi_mbuf, struct ipv6_srcroute0 *)->i6sr_type) 
    	nexthdr = &(mtod(index[indexoff-1].ihi_mbuf, struct ipv6_srcroute0 *)->i6sr_nexthdr);

  default:
    printf("Got a header we can't parse! (%d)", index[indexoff-1].ihi_nexthdr);
    goto espiret;
  }

  /* The algorithm-specific parsehdr function actually causes decyption */
  m->m_next = algorithm->parsehdr(index[indexoff].ihi_mbuf, assoc, 
			       &incoming->m_pkthdr.len, nexthdr);
  if (m->m_next == NULL) {
    DPRINTF(IDL_CRITICAL, ("ipsec_esp_input: parsehdr failed.\n"));
    goto espiret;
  }

  DP(FINISHED, *nexthdr, d);
  DDO(IDL_FINISHED, dump_mchain(incoming));

  m->m_next = ipv6_preparse(m->m_next, &(index[indexoff]), *nexthdr);

  DDO(IDL_EVENT,printf("ihi table is:\n");dump_ihitab(index));
  DDO(IDL_EVENT, dump_mchain(incoming));

  incoming->m_flags |= M_DECRYPTED; 

  /* To prevent spoofing attacks that use encrypted IP-IP tunnels, 
     we NEED to force the M_DECRYPTED flag to 0 whenever
     (1) inner IPv4 addr isn't same as outer IPv4 addr
     (2) inner IPv6 addr isn't same as outer IPv6 addr
  OR (3) inner packet and outer packet are different IP versions. 

     If/when we support proxy key mgmt, this might need to be
     reworked somewhat.  rja */

  if ((is_ipv6 == 0) && (index[0].ihi_nexthdr == NEXTHDR_IPV4)) {
    inner_v4 = mtod(index[0].ihi_mbuf, struct ip *);
    if ((u_long)inner_v4->ip_src.s_addr != 
	(u_long)((struct sockaddr_in *)&src)->sin_addr.s_addr) {
      incoming->m_flags &= ~M_DECRYPTED;
      esp_stats.esp_tunnel_spoof++;
    }
  } else if ((is_ipv6) && (index[0].ihi_nexthdr == NEXTHDR_IPV6)) {
    inner_v6 = mtod(index[0].ihi_mbuf, struct ipv6 *);
    if (!(IN6_ADDR_EQUAL(inner_v6->ipv6_src, src.sin6_addr))) {
      incoming->m_flags &= ~M_DECRYPTED;
      esp_stats.esp_tunnel_spoof++;
      }
  } else if (( (is_ipv6) && (index[0].ihi_nexthdr == NEXTHDR_IPV4)) ||
	     (!(is_ipv6) && (index[0].ihi_nexthdr == NEXTHDR_IPV6))) {
    incoming->m_flags &= ~M_DECRYPTED;
    esp_stats.esp_tunnel_spoof++;
  }

    
  if (mtod(incoming, struct ip *)->ip_v == 6) {
    /* is IPv6 */
    IPV6_STORE_LENGTH(incoming);

    (*inet6sw[ipv6_protox[index[indexoff].ihi_nexthdr]].pr_input)
      (incoming, extra, index, indexoff);
  } else {
    /* Must be IPv4, will choke badly on ST-II */
    struct ip *ip = mtod(incoming, struct ip *);
    ip->ip_p = index[indexoff].ihi_nexthdr;

    DP(GROSS_EVENT, ip->ip_p, d);
    DP(GROSS_EVENT, indexoff, d);

    ip->ip_len = incoming->m_pkthdr.len - (ip->ip_hl << 2);
    (*inetsw[ip_protox[ip->ip_p]].pr_input)(incoming, extra, index, indexoff);
  }
  /* Inform the Key Engine that we are no longer using this Key Table Node;
     if esp_ktn was NULL, we would have previously gone to espiret so
     don't have to worry about passing in a NULL ptr to key_free() here. */
  key_free(esp_ktn);
  esp_ktn = NULL;

  DPRINTF(IDL_MAJOR_EVENT, ("Left ipsec_esp_input()\n"));
  esp_stats.esp_accepted++;
  return;

 espiret:
  /* If a Key Table Node pointer is valid, 
     Inform the Key Engine that we are no longer using this Key Table Node */
  if (esp_ktn != NULL)
    key_free(esp_ktn);

  esp_stats.esp_in_errs++;

  if (incoming != NULL)
   m_freem(incoming);
  return;
}

/* EOF */
