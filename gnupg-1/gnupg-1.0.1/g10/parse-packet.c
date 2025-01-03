/* parse-packet.c  - read packets
 *	Copyright (C) 1998, 1999 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "packet.h"
#include "iobuf.h"
#include "mpi.h"
#include "util.h"
#include "cipher.h"
#include "memory.h"
#include "filter.h"
#include "options.h"
#include "main.h"
#include "i18n.h"

static int mpi_print_mode = 0;
static int list_mode = 0;

static int  parse( IOBUF inp, PACKET *pkt, int reqtype,
		   ulong *retpos, int *skip, IOBUF out, int do_skip
	    #ifdef DEBUG_PARSE_PACKET
		   ,const char *dbg_w, const char *dbg_f, int dbg_l
	    #endif
		 );
static int  copy_packet( IOBUF inp, IOBUF out, int pkttype,
					       unsigned long pktlen );
static void skip_packet( IOBUF inp, int pkttype, unsigned long pktlen );
static void skip_rest( IOBUF inp, unsigned long pktlen );
static void *read_rest( IOBUF inp, size_t pktlen );
static int  parse_symkeyenc( IOBUF inp, int pkttype, unsigned long pktlen,
							     PACKET *packet );
static int  parse_pubkeyenc( IOBUF inp, int pkttype, unsigned long pktlen,
							     PACKET *packet );
static int  parse_signature( IOBUF inp, int pkttype, unsigned long pktlen,
							 PKT_signature *sig );
static int  parse_onepass_sig( IOBUF inp, int pkttype, unsigned long pktlen,
							PKT_onepass_sig *ops );
static int  parse_key( IOBUF inp, int pkttype, unsigned long pktlen,
				      byte *hdr, int hdrlen, PACKET *packet );
static int  parse_user_id( IOBUF inp, int pkttype, unsigned long pktlen,
							   PACKET *packet );
static int  parse_photo_id( IOBUF inp, int pkttype, unsigned long pktlen,
							   PACKET *packet );
static int  parse_comment( IOBUF inp, int pkttype, unsigned long pktlen,
							   PACKET *packet );
static void parse_trust( IOBUF inp, int pkttype, unsigned long pktlen,
							   PACKET *packet );
static int  parse_plaintext( IOBUF inp, int pkttype, unsigned long pktlen,
					       PACKET *packet, int new_ctb);
static int  parse_compressed( IOBUF inp, int pkttype, unsigned long pktlen,
					       PACKET *packet, int new_ctb );
static int  parse_encrypted( IOBUF inp, int pkttype, unsigned long pktlen,
					       PACKET *packet, int new_ctb);

static unsigned short
read_16(IOBUF inp)
{
    unsigned short a;
    a = iobuf_get_noeof(inp) << 8;
    a |= iobuf_get_noeof(inp);
    return a;
}

static unsigned long
read_32(IOBUF inp)
{
    unsigned long a;
    a =  iobuf_get_noeof(inp) << 24;
    a |= iobuf_get_noeof(inp) << 16;
    a |= iobuf_get_noeof(inp) << 8;
    a |= iobuf_get_noeof(inp);
    return a;
}


int
set_packet_list_mode( int mode )
{
    int old = list_mode;
    list_mode = mode;
    mpi_print_mode = DBG_MPI;
    return old;
}

static void
unknown_pubkey_warning( int algo )
{
    static byte unknown_pubkey_algos[256];

    algo &= 0xff;
    if( !unknown_pubkey_algos[algo] ) {
	if( opt.verbose )
	    log_info(_("can't handle public key algorithm %d\n"), algo );
	unknown_pubkey_algos[algo] = 1;
    }
}

/****************
 * Parse a Packet and return it in packet
 * Returns: 0 := valid packet in pkt
 *	   -1 := no more packets
 *	   >0 := error
 * Note: The function may return an error and a partly valid packet;
 * caller must free this packet.
 */
#ifdef DEBUG_PARSE_PACKET
int
dbg_parse_packet( IOBUF inp, PACKET *pkt, const char *dbg_f, int dbg_l )
{
    int skip, rc;

    do {
	rc = parse( inp, pkt, 0, NULL, &skip, NULL, 0, "parse", dbg_f, dbg_l );
    } while( skip );
    return rc;
}
#else
int
parse_packet( IOBUF inp, PACKET *pkt )
{
    int skip, rc;

    do {
	rc = parse( inp, pkt, 0, NULL, &skip, NULL, 0 );
    } while( skip );
    return rc;
}
#endif

/****************
 * Like parse packet, but only return packets of the given type.
 */
#ifdef DEBUG_PARSE_PACKET
int
dbg_search_packet( IOBUF inp, PACKET *pkt, int pkttype, ulong *retpos,
		   const char *dbg_f, int dbg_l )
{
    int skip, rc;

    do {
	rc = parse( inp, pkt, pkttype, retpos, &skip, NULL, 0, "search", dbg_f, dbg_l );
    } while( skip );
    return rc;
}
#else
int
search_packet( IOBUF inp, PACKET *pkt, int pkttype, ulong *retpos )
{
    int skip, rc;

    do {
	rc = parse( inp, pkt, pkttype, retpos, &skip, NULL, 0 );
    } while( skip );
    return rc;
}
#endif

/****************
 * Copy all packets from INP to OUT, thereby removing unused spaces.
 */
#ifdef DEBUG_PARSE_PACKET
int
dbg_copy_all_packets( IOBUF inp, IOBUF out,
		   const char *dbg_f, int dbg_l )
{
    PACKET pkt;
    int skip, rc=0;
    do {
	init_packet(&pkt);
    } while( !(rc = parse( inp, &pkt, 0, NULL, &skip, out, 0, "copy", dbg_f, dbg_l )));
    return rc;
}
#else
int
copy_all_packets( IOBUF inp, IOBUF out )
{
    PACKET pkt;
    int skip, rc=0;
    do {
	init_packet(&pkt);
    } while( !(rc = parse( inp, &pkt, 0, NULL, &skip, out, 0 )));
    return rc;
}
#endif

/****************
 * Copy some packets from INP to OUT, thereby removing unused spaces.
 * Stop at offset STOPoff (i.e. don't copy packets at this or later offsets)
 */
#ifdef DEBUG_PARSE_PACKET
int
dbg_copy_some_packets( IOBUF inp, IOBUF out, ulong stopoff,
		   const char *dbg_f, int dbg_l )
{
    PACKET pkt;
    int skip, rc=0;
    do {
	if( iobuf_tell(inp) >= stopoff )
	    return 0;
	init_packet(&pkt);
    } while( !(rc = parse( inp, &pkt, 0, NULL, &skip, out, 0,
				     "some", dbg_f, dbg_l )) );
    return rc;
}
#else
int
copy_some_packets( IOBUF inp, IOBUF out, ulong stopoff )
{
    PACKET pkt;
    int skip, rc=0;
    do {
	if( iobuf_tell(inp) >= stopoff )
	    return 0;
	init_packet(&pkt);
    } while( !(rc = parse( inp, &pkt, 0, NULL, &skip, out, 0 )) );
    return rc;
}
#endif

/****************
 * Skip over N packets
 */
#ifdef DEBUG_PARSE_PACKET
int
dbg_skip_some_packets( IOBUF inp, unsigned n,
		   const char *dbg_f, int dbg_l )
{
    int skip, rc=0;
    PACKET pkt;

    for( ;n && !rc; n--) {
	init_packet(&pkt);
	rc = parse( inp, &pkt, 0, NULL, &skip, NULL, 1, "skip", dbg_f, dbg_l );
    }
    return rc;
}
#else
int
skip_some_packets( IOBUF inp, unsigned n )
{
    int skip, rc=0;
    PACKET pkt;

    for( ;n && !rc; n--) {
	init_packet(&pkt);
	rc = parse( inp, &pkt, 0, NULL, &skip, NULL, 1 );
    }
    return rc;
}
#endif


/****************
 * Parse packet. Set the variable skip points to to 1 if the packet
 * should be skipped; this is the case if either there is a
 * requested packet type and the parsed packet doesn't match or the
 * packet-type is 0, indicating deleted stuff.
 * if OUT is not NULL, a special copymode is used.
 */
static int
parse( IOBUF inp, PACKET *pkt, int reqtype, ulong *retpos,
       int *skip, IOBUF out, int do_skip
#ifdef DEBUG_PARSE_PACKET
       ,const char *dbg_w, const char *dbg_f, int dbg_l
#endif
     )
{
    int rc=0, c, ctb, pkttype, lenbytes;
    unsigned long pktlen;
    byte hdr[8];
    int hdrlen;
    int new_ctb = 0;

    *skip = 0;
    assert( !pkt->pkt.generic );
    if( retpos )
	*retpos = iobuf_tell(inp);

    if( (ctb = iobuf_get(inp)) == -1 ) {
	rc = -1;
	goto leave;
    }
    hdrlen=0;
    hdr[hdrlen++] = ctb;
    if( !(ctb & 0x80) ) {
	log_error("%s: invalid packet (ctb=%02x) near %lu\n",
			    iobuf_where(inp), ctb, iobuf_tell(inp) );
	rc = G10ERR_INVALID_PACKET;
	goto leave;
    }
    pktlen = 0;
    new_ctb = !!(ctb & 0x40);
    if( new_ctb ) {
	pkttype =  ctb & 0x3f;
	if( (c = iobuf_get(inp)) == -1 ) {
	    log_error("%s: 1st length byte missing\n", iobuf_where(inp) );
	    rc = G10ERR_INVALID_PACKET;
	    goto leave;
	}
	hdr[hdrlen++] = c;
	if( c < 192 )
	    pktlen = c;
	else if( c < 224 ) {
	    pktlen = (c - 192) * 256;
	    if( (c = iobuf_get(inp)) == -1 ) {
		log_error("%s: 2nd length byte missing\n", iobuf_where(inp) );
		rc = G10ERR_INVALID_PACKET;
		goto leave;
	    }
	    hdr[hdrlen++] = c;
	    pktlen += c + 192;
	}
	else if( c == 255 ) {
	    pktlen  = (hdr[hdrlen++] = iobuf_get_noeof(inp)) << 24;
	    pktlen |= (hdr[hdrlen++] = iobuf_get_noeof(inp)) << 16;
	    pktlen |= (hdr[hdrlen++] = iobuf_get_noeof(inp)) << 8;
	    if( (c = iobuf_get(inp)) == -1 ) {
		log_error("%s: 4 byte length invalid\n", iobuf_where(inp) );
		rc = G10ERR_INVALID_PACKET;
		goto leave;
	    }
	    pktlen |= (hdr[hdrlen++] = c );
	}
	else { /* partial body length */
	    iobuf_set_partial_block_mode(inp, c & 0xff);
	    pktlen = 0;/* to indicate partial length */
	}
    }
    else {
	pkttype = (ctb>>2)&0xf;
	lenbytes = ((ctb&3)==3)? 0 : (1<<(ctb & 3));
	if( !lenbytes ) {
	    pktlen = 0; /* don't know the value */
	    if( pkttype != PKT_COMPRESSED )
		iobuf_set_block_mode(inp, 1);
	}
	else {
	    for( ; lenbytes; lenbytes-- ) {
		pktlen <<= 8;
		pktlen |= hdr[hdrlen++] = iobuf_get_noeof(inp);
	    }
	}
    }

    if( out && pkttype	) {
	if( iobuf_write( out, hdr, hdrlen ) == -1 )
	    rc = G10ERR_WRITE_FILE;
	else
	    rc = copy_packet(inp, out, pkttype, pktlen );
	goto leave;
    }

    if( do_skip || !pkttype || (reqtype && pkttype != reqtype) ) {
	skip_rest(inp, pktlen);
	*skip = 1;
	rc = 0;
	goto leave;
    }

    if( DBG_PACKET ) {
      #ifdef DEBUG_PARSE_PACKET
	log_debug("parse_packet(iob=%d): type=%d length=%lu%s (%s.%s.%d)\n",
		   iobuf_id(inp), pkttype, pktlen, new_ctb?" (new_ctb)":"",
		    dbg_w, dbg_f, dbg_l );
      #else
	log_debug("parse_packet(iob=%d): type=%d length=%lu%s\n",
		   iobuf_id(inp), pkttype, pktlen, new_ctb?" (new_ctb)":"" );
      #endif
    }
    pkt->pkttype = pkttype;
    rc = G10ERR_UNKNOWN_PACKET; /* default error */
    switch( pkttype ) {
      case PKT_PUBLIC_KEY:
      case PKT_PUBLIC_SUBKEY:
	pkt->pkt.public_key = m_alloc_clear(sizeof *pkt->pkt.public_key );
	rc = parse_key(inp, pkttype, pktlen, hdr, hdrlen, pkt );
	break;
      case PKT_SECRET_KEY:
      case PKT_SECRET_SUBKEY:
	pkt->pkt.secret_key = m_alloc_clear(sizeof *pkt->pkt.secret_key );
	rc = parse_key(inp, pkttype, pktlen, hdr, hdrlen, pkt );
	break;
      case PKT_SYMKEY_ENC:
	rc = parse_symkeyenc( inp, pkttype, pktlen, pkt );
	break;
      case PKT_PUBKEY_ENC:
	rc = parse_pubkeyenc(inp, pkttype, pktlen, pkt );
	break;
      case PKT_SIGNATURE:
	pkt->pkt.signature = m_alloc_clear(sizeof *pkt->pkt.signature );
	rc = parse_signature(inp, pkttype, pktlen, pkt->pkt.signature );
	break;
      case PKT_ONEPASS_SIG:
	pkt->pkt.onepass_sig = m_alloc_clear(sizeof *pkt->pkt.onepass_sig );
	rc = parse_onepass_sig(inp, pkttype, pktlen, pkt->pkt.onepass_sig );
	break;
      case PKT_USER_ID:
	rc = parse_user_id(inp, pkttype, pktlen, pkt );
	break;
      case PKT_PHOTO_ID:
	pkt->pkttype = pkttype = PKT_USER_ID;  /* must fix it */
	rc = parse_photo_id(inp, pkttype, pktlen, pkt);
	break;
      case PKT_OLD_COMMENT:
      case PKT_COMMENT:
	rc = parse_comment(inp, pkttype, pktlen, pkt);
	break;
      case PKT_RING_TRUST:
	parse_trust(inp, pkttype, pktlen, pkt);
	rc = 0;
	break;
      case PKT_PLAINTEXT:
	rc = parse_plaintext(inp, pkttype, pktlen, pkt, new_ctb );
	break;
      case PKT_COMPRESSED:
	rc = parse_compressed(inp, pkttype, pktlen, pkt, new_ctb );
	break;
      case PKT_ENCRYPTED:
      case PKT_ENCRYPTED_MDC:
	rc = parse_encrypted(inp, pkttype, pktlen, pkt, new_ctb );
	break;
      default:
	skip_packet(inp, pkttype, pktlen);
	break;
    }

  leave:
    if( !rc && iobuf_error(inp) )
	rc = G10ERR_INV_KEYRING;
    return rc;
}

static void
dump_hex_line( int c, int *i )
{
    if( *i && !(*i%8) ) {
	if( *i && !(*i%24) )
	    printf("\n%4d:", *i );
	else
	    putchar(' ');
    }
    if( c == -1 )
	printf(" EOF" );
    else
	printf(" %02x", c );
    ++*i;
}


static int
copy_packet( IOBUF inp, IOBUF out, int pkttype, unsigned long pktlen )
{
    int n;
    char buf[100];

    if( iobuf_in_block_mode(inp) ) {
	while( (n = iobuf_read( inp, buf, 100 )) != -1 )
	    if( iobuf_write(out, buf, n ) )
		return G10ERR_WRITE_FILE; /* write error */
    }
    else if( !pktlen && pkttype == PKT_COMPRESSED ) {
	log_debug("copy_packet: compressed!\n");
	/* compressed packet, copy till EOF */
	while( (n = iobuf_read( inp, buf, 100 )) != -1 )
	    if( iobuf_write(out, buf, n ) )
		return G10ERR_WRITE_FILE; /* write error */
    }
    else {
	for( ; pktlen; pktlen -= n ) {
	    n = pktlen > 100 ? 100 : pktlen;
	    n = iobuf_read( inp, buf, n );
	    if( n == -1 )
		return G10ERR_READ_FILE;
	    if( iobuf_write(out, buf, n ) )
		return G10ERR_WRITE_FILE; /* write error */
	}
    }
    return 0;
}


static void
skip_packet( IOBUF inp, int pkttype, unsigned long pktlen )
{
    if( list_mode ) {
	if( pkttype == PKT_MARKER )
	    fputs(":marker packet:\n", stdout );
	else
	    printf(":unknown packet: type %2d, length %lu\n", pkttype, pktlen);
	if( pkttype ) {
	    int c, i=0 ;
	    if( pkttype != PKT_MARKER )
		fputs("dump:", stdout );
	    if( iobuf_in_block_mode(inp) ) {
		while( (c=iobuf_get(inp)) != -1 )
		    dump_hex_line(c, &i);
	    }
	    else {
		for( ; pktlen; pktlen-- )
		    dump_hex_line(iobuf_get(inp), &i);
	    }
	    putchar('\n');
	    return;
	}
    }
    skip_rest(inp,pktlen);
}

static void
skip_rest( IOBUF inp, unsigned long pktlen )
{
    if( iobuf_in_block_mode(inp) ) {
	while( iobuf_get(inp) != -1 )
		;
    }
    else {
	for( ; pktlen; pktlen-- )
	    if( iobuf_get(inp) == -1 )
		break;
    }
}


static void *
read_rest( IOBUF inp, size_t pktlen )
{
    byte *p;
    int i;

    if( iobuf_in_block_mode(inp) ) {
	log_error("read_rest: can't store stream data\n");
	p = NULL;
    }
    else {
	p = m_alloc( pktlen );
	for(i=0; pktlen; pktlen--, i++ )
	    p[i] = iobuf_get(inp);
    }
    return p;
}



static int
parse_symkeyenc( IOBUF inp, int pkttype, unsigned long pktlen, PACKET *packet )
{
    PKT_symkey_enc *k;
    int i, version, s2kmode, cipher_algo, hash_algo, seskeylen, minlen;

    if( pktlen < 4 ) {
	log_error("packet(%d) too short\n", pkttype);
	goto leave;
    }
    version = iobuf_get_noeof(inp); pktlen--;
    if( version != 4 ) {
	log_error("packet(%d) with unknown version %d\n", pkttype, version);
	goto leave;
    }
    if( pktlen > 200 ) { /* (we encode the seskeylen in a byte) */
	log_error("packet(%d) too large\n", pkttype);
	goto leave;
    }
    cipher_algo = iobuf_get_noeof(inp); pktlen--;
    s2kmode = iobuf_get_noeof(inp); pktlen--;
    hash_algo = iobuf_get_noeof(inp); pktlen--;
    switch( s2kmode ) {
      case 0:  /* simple s2k */
	minlen = 0;
	break;
      case 1:  /* salted s2k */
	minlen = 8;
	break;
      case 3:  /* iterated+salted s2k */
	minlen = 9;
	break;
      default:
	log_error("unknown S2K %d\n", s2kmode );
	goto leave;
    }
    if( minlen > pktlen ) {
	log_error("packet with S2K %d too short\n", s2kmode );
	goto leave;
    }
    seskeylen = pktlen - minlen;
    k = packet->pkt.symkey_enc = m_alloc_clear( sizeof *packet->pkt.symkey_enc
						+ seskeylen - 1 );
    k->version = version;
    k->cipher_algo = cipher_algo;
    k->s2k.mode = s2kmode;
    k->s2k.hash_algo = hash_algo;
    if( s2kmode == 1 || s2kmode == 3 ) {
	for(i=0; i < 8 && pktlen; i++, pktlen-- )
	    k->s2k.salt[i] = iobuf_get_noeof(inp);
    }
    if( s2kmode == 3 ) {
	k->s2k.count = iobuf_get(inp); pktlen--;
    }
    k->seskeylen = seskeylen;
    for(i=0; i < seskeylen && pktlen; i++, pktlen-- )
	k->seskey[i] = iobuf_get_noeof(inp);
    assert( !pktlen );

    if( list_mode ) {
	printf(":symkey enc packet: version %d, cipher %d, s2k %d, hash %d\n",
			    version, cipher_algo, s2kmode, hash_algo);
	if( s2kmode == 1 || s2kmode == 3 ) {
	    printf("\tsalt ");
	    for(i=0; i < 8; i++ )
		printf("%02x", k->s2k.salt[i]);
	    if( s2kmode == 3 )
		printf(", count %lu\n", (ulong)k->s2k.count );
	    printf("\n");
	}
    }

  leave:
    skip_rest(inp, pktlen);
    return 0;
}

static int
parse_pubkeyenc( IOBUF inp, int pkttype, unsigned long pktlen, PACKET *packet )
{
    unsigned n;
    int i, ndata;
    PKT_pubkey_enc *k;

    k = packet->pkt.pubkey_enc = m_alloc_clear(sizeof *packet->pkt.pubkey_enc);
    if( pktlen < 12 ) {
	log_error("packet(%d) too short\n", pkttype);
	goto leave;
    }
    k->version = iobuf_get_noeof(inp); pktlen--;
    if( k->version != 2 && k->version != 3 ) {
	log_error("packet(%d) with unknown version %d\n", pkttype, k->version);
	goto leave;
    }
    k->keyid[0] = read_32(inp); pktlen -= 4;
    k->keyid[1] = read_32(inp); pktlen -= 4;
    k->pubkey_algo = iobuf_get_noeof(inp); pktlen--;
    k->throw_keyid = 0; /* only used as flag for build_packet */
    if( list_mode )
	printf(":pubkey enc packet: version %d, algo %d, keyid %08lX%08lX\n",
	  k->version, k->pubkey_algo, (ulong)k->keyid[0], (ulong)k->keyid[1]);

    ndata = pubkey_get_nenc(k->pubkey_algo);
    if( !ndata ) {
	if( list_mode )
	    printf("\tunsupported algorithm %d\n", k->pubkey_algo );
	unknown_pubkey_warning( k->pubkey_algo );
	k->data[0] = NULL;  /* no need to store the encrypted data */
    }
    else {
	for( i=0; i < ndata; i++ ) {
	    n = pktlen;
	    k->data[i] = mpi_read(inp, &n, 0); pktlen -=n;
	    if( list_mode ) {
		printf("\tdata: ");
		mpi_print(stdout, k->data[i], mpi_print_mode );
		putchar('\n');
	    }
	}
    }

  leave:
    skip_rest(inp, pktlen);
    return 0;
}


static void
dump_sig_subpkt( int hashed, int type, int critical,
		 const byte *buffer, size_t buflen, size_t length )
{
    const char *p=NULL;
    int i;

    printf("\t%s%ssubpkt %d len %u (", /*)*/
	      critical ? "critical ":"",
	      hashed ? "hashed ":"", type, (unsigned)length );
    buffer++;
    length--;
    if( length > buflen ) {
	printf("too short: buffer is only %u)\n", (unsigned)buflen );
	return;
    }
    switch( type ) {
      case SIGSUBPKT_SIG_CREATED:
	if( length >= 4 )
	    printf("sig created %s", strtimestamp( buffer_to_u32(buffer) ) );
	break;
      case SIGSUBPKT_SIG_EXPIRE:
	if( length >= 4 )
	    printf("sig expires after %s",
				     strtimevalue( buffer_to_u32(buffer) ) );
	break;
      case SIGSUBPKT_EXPORTABLE:
	if( length )
	    printf("%sexportable", *buffer? "":"not ");
	break;
      case SIGSUBPKT_TRUST:
	p = "trust signature";
	break;
      case SIGSUBPKT_REGEXP:
	p = "regular expression";
	break;
      case SIGSUBPKT_REVOCABLE:
	p = "revocable";
	break;
      case SIGSUBPKT_KEY_EXPIRE:
	if( length >= 4 )
	    printf("key expires after %s",
				    strtimevalue( buffer_to_u32(buffer) ) );
	break;
      case SIGSUBPKT_ARR:
	p = "additional recipient request";
	break;
      case SIGSUBPKT_PREF_SYM:
	fputs("pref-sym-algos:", stdout );
	for( i=0; i < length; i++ )
	    printf(" %d", buffer[i] );
	break;
      case SIGSUBPKT_REV_KEY:
	fputs("revocation key: ", stdout );
	if( length < 22 )
	    p = "[too short]";
	else {
	    printf("c=%02x a=%d f=", buffer[0], buffer[1] );
	    for( i=2; i < length; i++ )
		printf("%02X", buffer[i] );
	}
	break;
      case SIGSUBPKT_ISSUER:
	if( length >= 8 )
	    printf("issuer key ID %08lX%08lX",
		      (ulong)buffer_to_u32(buffer),
		      (ulong)buffer_to_u32(buffer+4) );
	break;
      case SIGSUBPKT_NOTATION:
	{
	    fputs("notation: ", stdout );
	    if( length < 8 )
		p = "[too short]";
	    else if( !(*buffer & 0x80) )
		p = "[not human readable]";
	    else {
		const byte *s = buffer;
		size_t n1, n2;

		n1 = (s[4] << 8) | s[5];
		n2 = (s[6] << 8) | s[7];
		s += 8;
		if( 8+n1+n2 != length )
		    p = "[error]";
		else {
		    print_string( stdout, s, n1, ')' );
		    putc( '=', stdout );
		    print_string( stdout, s+n1, n2, ')' );
		}
	    }
	}
	break;
      case SIGSUBPKT_PREF_HASH:
	fputs("pref-hash-algos:", stdout );
	for( i=0; i < length; i++ )
	    printf(" %d", buffer[i] );
	break;
      case SIGSUBPKT_PREF_COMPR:
	fputs("pref-zip-algos:", stdout );
	for( i=0; i < length; i++ )
	    printf(" %d", buffer[i] );
	break;
      case SIGSUBPKT_KS_FLAGS:
	p = "key server preferences";
	break;
      case SIGSUBPKT_PREF_KS:
	p = "preferred key server";
	break;
      case SIGSUBPKT_PRIMARY_UID:
	p = "primary user ID";
	break;
      case SIGSUBPKT_POLICY:
	fputs("policy: ", stdout );
	print_string( stdout, buffer, length, ')' );
	break;
      case SIGSUBPKT_KEY_FLAGS:
	p = "key flags";
	break;
      case SIGSUBPKT_SIGNERS_UID:
	p = "signer's user ID";
	break;
      case SIGSUBPKT_PRIV_ADD_SIG:
	p = "signs additional user ID";
	break;
      default: p = "?"; break;
    }

    printf("%s)\n", p? p: "");
}

/****************
 * Returns: >= 0 offset into buffer
 *	    -1 unknown type
 *	    -2 unsupported type
 *	    -3 subpacket too short
 */
static int
parse_one_sig_subpkt( const byte *buffer, size_t n, int type )
{
    switch( type ) {
      case SIGSUBPKT_SIG_CREATED:
      case SIGSUBPKT_SIG_EXPIRE:
      case SIGSUBPKT_KEY_EXPIRE:
	if( n < 4 )
	    break;
	return 0;
      case SIGSUBPKT_EXPORTABLE:
	if( !n )
	    break;
	return 0;
      case SIGSUBPKT_ISSUER:/* issuer key ID */
	if( n < 8 )
	    break;
	return 0;
      case SIGSUBPKT_NOTATION:
	if( n < 8 ) /* minimum length needed */
	    break;
	return 0;
      case SIGSUBPKT_PREF_SYM:
      case SIGSUBPKT_PREF_HASH:
      case SIGSUBPKT_PREF_COMPR:
      case SIGSUBPKT_POLICY:
	return 0;
      case SIGSUBPKT_PRIV_ADD_SIG:
	/* because we use private data, we check the GNUPG marker */
	if( n < 24 )
	    break;
	if( buffer[0] != 'G' || buffer[1] != 'P' || buffer[2] != 'G' )
	    return -2;
	return 3;
      default: return -1;
    }
    return -3;
}


static int
can_handle_critical( const byte *buffer, size_t n, int type )
{
    switch( type ) {
      case SIGSUBPKT_NOTATION:
	if( n >= 8 && (*buffer & 0x80) )
	    return 1; /* human readable is handled */
	return 0;

      case SIGSUBPKT_SIG_CREATED:
      case SIGSUBPKT_SIG_EXPIRE:
      case SIGSUBPKT_KEY_EXPIRE:
      case SIGSUBPKT_EXPORTABLE:
      case SIGSUBPKT_ISSUER:/* issuer key ID */
      case SIGSUBPKT_PREF_SYM:
      case SIGSUBPKT_PREF_HASH:
      case SIGSUBPKT_PREF_COMPR:
	return 1;

      case SIGSUBPKT_POLICY: /* Is enough to show the policy? */
      default:
	return 0;
    }
}


const byte *
enum_sig_subpkt( const byte *buffer, sigsubpkttype_t reqtype,
		 size_t *ret_n, int *start )
{
    int buflen;
    int type;
    int critical;
    int offset;
    size_t n;
    int seq = 0;
    int reqseq = start? *start: 0;

    if( !buffer || reqseq == -1 )
	return NULL;
    buflen = (*buffer << 8) | buffer[1];
    buffer += 2;
    while( buflen ) {
	n = *buffer++; buflen--;
	if( n == 255 ) {
	    if( buflen < 4 )
		goto too_short;
	    n = (buffer[0] << 24) | (buffer[1] << 16)
				  | (buffer[2] << 8) | buffer[3];
	    buffer += 4;
	    buflen -= 4;

	}
	else if( n >= 192 ) {
	    if( buflen < 2 )
		goto too_short;
	    n = (( n - 192 ) << 8) + *buffer + 192;
	    buffer++;
	    buflen--;
	}
	if( buflen < n )
	    goto too_short;
	type = *buffer;
	if( type & 0x80 ) {
	    type &= 0x7f;
	    critical = 1;
	}
	else
	    critical = 0;
	if( !(++seq > reqseq) )
	    ;
	else if( reqtype == SIGSUBPKT_TEST_CRITICAL ) {
	    if( critical ) {
		if( n-1 > buflen+1 )
		    goto too_short;
		if( !can_handle_critical(buffer+1, n-1, type ) ) {
		    log_info(_("subpacket of type %d has critical bit set\n"),
									type);
		    if( start )
			*start = seq;
		    return NULL; /* this is an error */
		}
	    }
	}
	else if( reqtype < 0 ) /* list packets */
	    dump_sig_subpkt( reqtype == SIGSUBPKT_LIST_HASHED,
				    type, critical, buffer, buflen, n );
	else if( type == reqtype ) { /* found */
	    buffer++;
	    n--;
	    if( n > buflen )
		goto too_short;
	    if( ret_n )
		*ret_n = n;
	    offset = parse_one_sig_subpkt(buffer, n, type );
	    switch( offset ) {
	      case -3:
		log_error("subpacket of type %d too short\n", type);
		return NULL;
	      case -2:
		return NULL;
	      case -1:
		BUG(); /* not yet needed */
	      default:
		break;
	    }
	    if( start )
		*start = seq;
	    return buffer+offset;
	}
	buffer += n; buflen -=n;
    }
    if( reqtype == SIGSUBPKT_TEST_CRITICAL )
	return buffer; /* as value true to indicate that there is no */
		       /* critical bit we don't understand */
    if( start )
	*start = -1;
    return NULL; /* end of packets; not found */

  too_short:
    log_error("buffer shorter than subpacket\n");
    if( start )
	*start = -1;
    return NULL;
}


const byte *
parse_sig_subpkt( const byte *buffer, sigsubpkttype_t reqtype, size_t *ret_n )
{
    return enum_sig_subpkt( buffer, reqtype, ret_n, NULL );
}

const byte *
parse_sig_subpkt2( PKT_signature *sig, sigsubpkttype_t reqtype, size_t *ret_n )
{
    const byte *p;

    p = parse_sig_subpkt( sig->hashed_data, reqtype, ret_n );
    if( !p )
	p = parse_sig_subpkt( sig->unhashed_data, reqtype, ret_n );
    return p;
}



static int
parse_signature( IOBUF inp, int pkttype, unsigned long pktlen,
					  PKT_signature *sig )
{
    int md5_len=0;
    unsigned n;
    int is_v4=0;
    int rc=0;
    int i, ndata;

    if( pktlen < 16 ) {
	log_error("packet(%d) too short\n", pkttype);
	goto leave;
    }
    sig->version = iobuf_get_noeof(inp); pktlen--;
    if( sig->version == 4 )
	is_v4=1;
    else if( sig->version != 2 && sig->version != 3 ) {
	log_error("packet(%d) with unknown version %d\n", pkttype, sig->version);
	goto leave;
    }

    if( !is_v4 ) {
	md5_len = iobuf_get_noeof(inp); pktlen--;
    }
    sig->sig_class = iobuf_get_noeof(inp); pktlen--;
    if( !is_v4 ) {
	sig->timestamp = read_32(inp); pktlen -= 4;
	sig->keyid[0] = read_32(inp); pktlen -= 4;
	sig->keyid[1] = read_32(inp); pktlen -= 4;
    }
    sig->pubkey_algo = iobuf_get_noeof(inp); pktlen--;
    sig->digest_algo = iobuf_get_noeof(inp); pktlen--;
    if( is_v4 ) { /* read subpackets */
	n = read_16(inp); pktlen -= 2; /* length of hashed data */
	if( n > 10000 ) {
	    log_error("signature packet: hashed data too long\n");
	    rc = G10ERR_INVALID_PACKET;
	    goto leave;
	}
	if( n ) {
	    sig->hashed_data = m_alloc( n + 2 );
	    sig->hashed_data[0] = n >> 8;
	    sig->hashed_data[1] = n;
	    if( iobuf_read(inp, sig->hashed_data+2, n ) != n ) {
		log_error("premature eof while reading hashed signature data\n");
		rc = -1;
		goto leave;
	    }
	    pktlen -= n;
	}
	n = read_16(inp); pktlen -= 2; /* length of unhashed data */
	if( n > 10000 ) {
	    log_error("signature packet: unhashed data too long\n");
	    rc = G10ERR_INVALID_PACKET;
	    goto leave;
	}
	if( n ) {
	    sig->unhashed_data = m_alloc( n + 2 );
	    sig->unhashed_data[0] = n >> 8;
	    sig->unhashed_data[1] = n;
	    if( iobuf_read(inp, sig->unhashed_data+2, n ) != n ) {
		log_error("premature eof while reading unhashed signature data\n");
		rc = -1;
		goto leave;
	    }
	    pktlen -= n;
	}
    }

    if( pktlen < 5 ) { /* sanity check */
	log_error("packet(%d) too short\n", pkttype);
	rc = G10ERR_INVALID_PACKET;
	goto leave;
    }

    sig->digest_start[0] = iobuf_get_noeof(inp); pktlen--;
    sig->digest_start[1] = iobuf_get_noeof(inp); pktlen--;

    if( is_v4 && sig->pubkey_algo ) { /*extract required information */
	const byte *p;

	/* set sig->flags.unknown_critical if there is a
	 * critical bit set for packets which we do not understand */
	if( !parse_sig_subpkt( sig->hashed_data, SIGSUBPKT_TEST_CRITICAL, NULL)
	   || !parse_sig_subpkt( sig->unhashed_data, SIGSUBPKT_TEST_CRITICAL,
									NULL) )
	{
	    sig->flags.unknown_critical = 1;
	}

	p = parse_sig_subpkt( sig->hashed_data, SIGSUBPKT_SIG_CREATED, NULL );
	if( !p )
	    log_error("signature packet without timestamp\n");
	else
	    sig->timestamp = buffer_to_u32(p);
	p = parse_sig_subpkt2( sig, SIGSUBPKT_ISSUER, NULL );
	if( !p )
	    log_error("signature packet without keyid\n");
	else {
	    sig->keyid[0] = buffer_to_u32(p);
	    sig->keyid[1] = buffer_to_u32(p+4);
	}
    }

    if( list_mode ) {
	printf(":signature packet: algo %d, keyid %08lX%08lX\n"
	       "\tversion %d, created %lu, md5len %d, sigclass %02x\n"
	       "\tdigest algo %d, begin of digest %02x %02x\n",
		sig->pubkey_algo,
		(ulong)sig->keyid[0], (ulong)sig->keyid[1],
		sig->version, (ulong)sig->timestamp, md5_len, sig->sig_class,
		sig->digest_algo,
		sig->digest_start[0], sig->digest_start[1] );
	if( is_v4 ) {
	    parse_sig_subpkt( sig->hashed_data,  SIGSUBPKT_LIST_HASHED, NULL );
	    parse_sig_subpkt( sig->unhashed_data,SIGSUBPKT_LIST_UNHASHED, NULL);
	}
    }

    ndata = pubkey_get_nsig(sig->pubkey_algo);
    if( !ndata ) {
	if( list_mode )
	    printf("\tunknown algorithm %d\n", sig->pubkey_algo );
	unknown_pubkey_warning( sig->pubkey_algo );
	/* we store the plain material in data[0], so that we are able
	 * to write it back with build_packet() */
	sig->data[0] = mpi_set_opaque(NULL, read_rest(inp, pktlen), pktlen );
	pktlen = 0;
    }
    else {
	for( i=0; i < ndata; i++ ) {
	    n = pktlen;
	    sig->data[i] = mpi_read(inp, &n, 0 );
	    pktlen -=n;
	    if( list_mode ) {
		printf("\tdata: ");
		mpi_print(stdout, sig->data[i], mpi_print_mode );
		putchar('\n');
	    }
	}
    }

  leave:
    skip_rest(inp, pktlen);
    return rc;
}


static int
parse_onepass_sig( IOBUF inp, int pkttype, unsigned long pktlen,
					     PKT_onepass_sig *ops )
{
    int version;

    if( pktlen < 13 ) {
	log_error("packet(%d) too short\n", pkttype);
	goto leave;
    }
    version = iobuf_get_noeof(inp); pktlen--;
    if( version != 3 ) {
	log_error("onepass_sig with unknown version %d\n", version);
	goto leave;
    }
    ops->sig_class = iobuf_get_noeof(inp); pktlen--;
    ops->digest_algo = iobuf_get_noeof(inp); pktlen--;
    ops->pubkey_algo = iobuf_get_noeof(inp); pktlen--;
    ops->keyid[0] = read_32(inp); pktlen -= 4;
    ops->keyid[1] = read_32(inp); pktlen -= 4;
    ops->last = iobuf_get_noeof(inp); pktlen--;
    if( list_mode )
	printf(":onepass_sig packet: keyid %08lX%08lX\n"
	       "\tversion %d, sigclass %02x, digest %d, pubkey %d, last=%d\n",
		(ulong)ops->keyid[0], (ulong)ops->keyid[1],
		version, ops->sig_class,
		ops->digest_algo, ops->pubkey_algo, ops->last );


  leave:
    skip_rest(inp, pktlen);
    return 0;
}




static int
parse_key( IOBUF inp, int pkttype, unsigned long pktlen,
			      byte *hdr, int hdrlen, PACKET *pkt )
{
    int i, version, algorithm;
    unsigned n;
    unsigned long timestamp, expiredate;
    int npkey, nskey;
    int is_v4=0;
    int rc=0;

    version = iobuf_get_noeof(inp); pktlen--;
    if( pkttype == PKT_PUBLIC_SUBKEY && version == '#' ) {
	/* early versions of G10 use old PGP comments packets;
	 * luckily all those comments are started by a hash */
	if( list_mode ) {
	    printf(":rfc1991 comment packet: \"" );
	    for( ; pktlen; pktlen-- ) {
		int c;
		c = iobuf_get_noeof(inp);
		if( c >= ' ' && c <= 'z' )
		    putchar(c);
		else
		    printf("\\x%02x", c );
	    }
	    printf("\"\n");
	}
	skip_rest(inp, pktlen);
	return 0;
    }
    else if( version == 4 )
	is_v4=1;
    else if( version != 2 && version != 3 ) {
	log_error("packet(%d) with unknown version %d\n", pkttype, version);
	goto leave;
    }

    if( pktlen < 11 ) {
	log_error("packet(%d) too short\n", pkttype);
	goto leave;
    }

    timestamp = read_32(inp); pktlen -= 4;
    if( is_v4 )
	expiredate = 0; /* have to get it from the selfsignature */
    else {
	unsigned short ndays;
	ndays = read_16(inp); pktlen -= 2;
	if( ndays )
	    expiredate = timestamp + ndays * 86400L;
	else
	    expiredate = 0;
    }
    algorithm = iobuf_get_noeof(inp); pktlen--;
    if( list_mode )
	printf(":%s key packet:\n"
	       "\tversion %d, algo %d, created %lu, expires %lu\n",
		pkttype == PKT_PUBLIC_KEY? "public" :
		pkttype == PKT_SECRET_KEY? "secret" :
		pkttype == PKT_PUBLIC_SUBKEY? "public sub" :
		pkttype == PKT_SECRET_SUBKEY? "secret sub" : "??",
		version, algorithm, timestamp, expiredate );

    if( pkttype == PKT_SECRET_KEY || pkttype == PKT_SECRET_SUBKEY )  {
	PKT_secret_key *sk = pkt->pkt.secret_key;

	sk->timestamp = timestamp;
	sk->expiredate = expiredate;
	sk->hdrbytes = hdrlen;
	sk->version = version;
	sk->is_primary = pkttype == PKT_SECRET_KEY;
	sk->pubkey_algo = algorithm;
	sk->pubkey_usage = 0; /* not yet used */
    }
    else {
	PKT_public_key *pk = pkt->pkt.public_key;

	pk->timestamp = timestamp;
	pk->expiredate = expiredate;
	pk->hdrbytes	= hdrlen;
	pk->version	= version;
	pk->pubkey_algo = algorithm;
	pk->pubkey_usage = 0; /* not yet used */
	pk->keyid[0] = 0;
	pk->keyid[1] = 0;
    }
    nskey = pubkey_get_nskey( algorithm );
    npkey = pubkey_get_npkey( algorithm );
    if( !npkey ) {
	if( list_mode )
	    printf("\tunknown algorithm %d\n", algorithm );
	unknown_pubkey_warning( algorithm );
    }


    if( pkttype == PKT_SECRET_KEY || pkttype == PKT_SECRET_SUBKEY ) {
	PKT_secret_key *sk = pkt->pkt.secret_key;
	byte temp[16];

	if( !npkey ) {
	    sk->skey[0] = mpi_set_opaque( NULL,
					  read_rest(inp, pktlen), pktlen );
	    pktlen = 0;
	    goto leave;
	}

	for(i=0; i < npkey; i++ ) {
	    n = pktlen; sk->skey[i] = mpi_read(inp, &n, 0 ); pktlen -=n;
	    if( list_mode ) {
		printf(  "\tskey[%d]: ", i);
		mpi_print(stdout, sk->skey[i], mpi_print_mode  );
		putchar('\n');
	    }
	}
	sk->protect.algo = iobuf_get_noeof(inp); pktlen--;
	if( sk->protect.algo ) {
	    sk->is_protected = 1;
	    sk->protect.s2k.count = 0;
	    if( sk->protect.algo == 255 ) {
		if( pktlen < 3 ) {
		    rc = G10ERR_INVALID_PACKET;
		    goto leave;
		}
		sk->protect.algo = iobuf_get_noeof(inp); pktlen--;
		sk->protect.s2k.mode  = iobuf_get_noeof(inp); pktlen--;
		sk->protect.s2k.hash_algo = iobuf_get_noeof(inp); pktlen--;
		switch( sk->protect.s2k.mode ) {
		  case 1:
		  case 3:
		    for(i=0; i < 8 && pktlen; i++, pktlen-- )
			temp[i] = iobuf_get_noeof(inp);
		    memcpy(sk->protect.s2k.salt, temp, 8 );
		    break;
		}
		switch( sk->protect.s2k.mode ) {
		  case 0: if( list_mode ) printf(  "\tsimple S2K" );
		    break;
		  case 1: if( list_mode ) printf(  "\tsalted S2K" );
		    break;
		  case 3: if( list_mode ) printf(  "\titer+salt S2K" );
		    break;
		  default:
		    if( list_mode )
			printf(  "\tunknown S2K %d\n",
					    sk->protect.s2k.mode );
		    rc = G10ERR_INVALID_PACKET;
		    goto leave;
		}

		if( list_mode ) {
		    printf(", algo: %d, hash: %d",
				     sk->protect.algo,
				     sk->protect.s2k.hash_algo );
		    if( sk->protect.s2k.mode == 1
			|| sk->protect.s2k.mode == 3 ) {
			printf(", salt: ");
			for(i=0; i < 8; i++ )
			    printf("%02x", sk->protect.s2k.salt[i]);
		    }
		    putchar('\n');
		}

		if( sk->protect.s2k.mode == 3 ) {
		    if( pktlen < 1 ) {
			rc = G10ERR_INVALID_PACKET;
			goto leave;
		    }
		    sk->protect.s2k.count = iobuf_get(inp);
		    pktlen--;
		    if( list_mode )
			printf("\tprotect count: %lu\n",
					    (ulong)sk->protect.s2k.count);
		}
	    }
	    else { /* old version; no S2K, so we set mode to 0, hash MD5 */
		sk->protect.s2k.mode = 0;
		sk->protect.s2k.hash_algo = DIGEST_ALGO_MD5;
		if( list_mode )
		    printf(  "\tprotect algo: %d  (hash algo: %d)\n",
			 sk->protect.algo, sk->protect.s2k.hash_algo );
	    }
	    /* It is really ugly that we don't know the size
	     * of the IV here in cases we are not aware of the algorithm.
	     * so a
	     *	 sk->protect.ivlen = cipher_get_blocksize(sk->protect.algo);
	     * won't work.  The only solution I see is to hardwire it here.
	     * NOTE: if you change the ivlen above 16, don't forget to
	     * enlarge temp.
	     */
	    switch( sk->protect.algo ) {
	      case 7: case 8: case 9: /* reserved for AES */
	      case 10: /* Twofish */
		sk->protect.ivlen = 16;
		break;
	      default:
		sk->protect.ivlen = 8;
	    }
	    if( pktlen < sk->protect.ivlen ) {
		rc = G10ERR_INVALID_PACKET;
		goto leave;
	    }
	    for(i=0; i < sk->protect.ivlen && pktlen; i++, pktlen-- )
		temp[i] = iobuf_get_noeof(inp);
	    if( list_mode ) {
		printf(  "\tprotect IV: ");
		for(i=0; i < sk->protect.ivlen; i++ )
		    printf(" %02x", temp[i] );
		putchar('\n');
	    }
	    memcpy(sk->protect.iv, temp, sk->protect.ivlen );
	}
	else
	    sk->is_protected = 0;
	/* It does not make sense to read it into secure memory.
	 * If the user is so careless, not to protect his secret key,
	 * we can assume, that he operates an open system :=(.
	 * So we put the key into secure memory when we unprotect it. */
	if( is_v4 && sk->is_protected ) {
	    /* ugly; the length is encrypted too, so we read all
	     * stuff up to the end of the packet into the first
	     * skey element */
	    sk->skey[npkey] = mpi_set_opaque(NULL,
					     read_rest(inp, pktlen), pktlen );
	    pktlen = 0;
	    if( list_mode ) {
		printf("\tencrypted stuff follows\n");
	    }
	}
	else { /* v3 method: the mpi length is not encrypted */
	    for(i=npkey; i < nskey; i++ ) {
		n = pktlen; sk->skey[i] = mpi_read(inp, &n, 0 ); pktlen -=n;
		if( sk->is_protected )
		    mpi_set_protect_flag(sk->skey[i]);
		if( list_mode ) {
		    printf(  "\tskey[%d]: ", i);
		    if( sk->is_protected )
			printf(  "[encrypted]\n");
		    else {
			mpi_print(stdout, sk->skey[i], mpi_print_mode  );
			putchar('\n');
		    }
		}
	    }

	    sk->csum = read_16(inp); pktlen -= 2;
	    if( list_mode ) {
		printf("\tchecksum: %04hx\n", sk->csum);
	    }
	}
    }
    else {
	PKT_public_key *pk = pkt->pkt.public_key;

	if( !npkey ) {
	    pk->pkey[0] = mpi_set_opaque( NULL,
					  read_rest(inp, pktlen), pktlen );
	    pktlen = 0;
	    goto leave;
	}

	for(i=0; i < npkey; i++ ) {
	    n = pktlen; pk->pkey[i] = mpi_read(inp, &n, 0 ); pktlen -=n;
	    if( list_mode ) {
		printf(  "\tpkey[%d]: ", i);
		mpi_print(stdout, pk->pkey[i], mpi_print_mode  );
		putchar('\n');
	    }
	}
    }

  leave:
    skip_rest(inp, pktlen);
    return rc;
}


static int
parse_user_id( IOBUF inp, int pkttype, unsigned long pktlen, PACKET *packet )
{
    byte *p;

    packet->pkt.user_id = m_alloc(sizeof *packet->pkt.user_id  + pktlen);
    packet->pkt.user_id->len = pktlen;
    packet->pkt.user_id->photo = NULL;
    packet->pkt.user_id->photolen = 0;
    p = packet->pkt.user_id->name;
    for( ; pktlen; pktlen--, p++ )
	*p = iobuf_get_noeof(inp);
    *p = 0;

    if( list_mode ) {
	int n = packet->pkt.user_id->len;
	printf(":user ID packet: \"");
	/* fixme: Hey why don't we replace this wioth print_string?? */
	for(p=packet->pkt.user_id->name; n; p++, n-- ) {
	    if( *p >= ' ' && *p <= 'z' )
		putchar(*p);
	    else
		printf("\\x%02x", *p );
	}
	printf("\"\n");
    }
    return 0;
}


/****************
 * PGP generates a packet of type 17. We assume this is a photo ID and
 * simply store it here as a comment packet.
 */
static int
parse_photo_id( IOBUF inp, int pkttype, unsigned long pktlen, PACKET *packet )
{
    byte *p;

    packet->pkt.user_id = m_alloc(sizeof *packet->pkt.user_id  + 30);
    sprintf( packet->pkt.user_id->name, "[image of size %lu]", pktlen );
    packet->pkt.user_id->len = strlen(packet->pkt.user_id->name);

    packet->pkt.user_id->photo = m_alloc(sizeof *packet->pkt.user_id + pktlen);
    packet->pkt.user_id->photolen = pktlen;
    p = packet->pkt.user_id->photo;
    for( ; pktlen; pktlen--, p++ )
	*p = iobuf_get_noeof(inp);

    if( list_mode ) {
	printf(":photo_id packet: %s\n", packet->pkt.user_id->name );
    }
    return 0;
}


static int
parse_comment( IOBUF inp, int pkttype, unsigned long pktlen, PACKET *packet )
{
    byte *p;

    packet->pkt.comment = m_alloc(sizeof *packet->pkt.comment + pktlen - 1);
    packet->pkt.comment->len = pktlen;
    p = packet->pkt.comment->data;
    for( ; pktlen; pktlen--, p++ )
	*p = iobuf_get_noeof(inp);

    if( list_mode ) {
	int n = packet->pkt.comment->len;
	printf(":%scomment packet: \"", pkttype == PKT_OLD_COMMENT?
					 "OpenPGP draft " : "" );
	for(p=packet->pkt.comment->data; n; p++, n-- ) {
	    if( *p >= ' ' && *p <= 'z' )
		putchar(*p);
	    else
		printf("\\x%02x", *p );
	}
	printf("\"\n");
    }
    return 0;
}


static void
parse_trust( IOBUF inp, int pkttype, unsigned long pktlen, PACKET *pkt )
{
    int c;

    c = iobuf_get_noeof(inp);
    pkt->pkt.ring_trust = m_alloc( sizeof *pkt->pkt.ring_trust );
    pkt->pkt.ring_trust->trustval = c;
    if( list_mode )
	printf(":trust packet: flag=%02x\n", c );
}


static int
parse_plaintext( IOBUF inp, int pkttype, unsigned long pktlen,
					PACKET *pkt, int new_ctb )
{
    int mode, namelen;
    PKT_plaintext *pt;
    byte *p;
    int c, i;

    if( pktlen && pktlen < 6 ) {
	log_error("packet(%d) too short (%lu)\n", pkttype, (ulong)pktlen);
	goto leave;
    }
    mode = iobuf_get_noeof(inp); if( pktlen ) pktlen--;
    namelen = iobuf_get_noeof(inp); if( pktlen ) pktlen--;
    pt = pkt->pkt.plaintext = m_alloc(sizeof *pkt->pkt.plaintext + namelen -1);
    pt->new_ctb = new_ctb;
    pt->mode = mode;
    pt->namelen = namelen;
    if( pktlen ) {
	for( i=0; pktlen > 4 && i < namelen; pktlen--, i++ )
	    pt->name[i] = iobuf_get_noeof(inp);
    }
    else {
	for( i=0; i < namelen; i++ )
	    if( (c=iobuf_get(inp)) == -1 )
		break;
	    else
		pt->name[i] = c;
    }
    pt->timestamp = read_32(inp); if( pktlen) pktlen -= 4;
    pt->len = pktlen;
    pt->buf = inp;
    pktlen = 0;

    if( list_mode ) {
	printf(":literal data packet:\n"
	       "\tmode %c, created %lu, name=\"",
		    mode >= ' ' && mode <'z'? mode : '?',
		    (ulong)pt->timestamp );
	for(p=pt->name,i=0; i < namelen; p++, i++ ) {
	    if( *p >= ' ' && *p <= 'z' )
		putchar(*p);
	    else
		printf("\\x%02x", *p );
	}
	printf("\",\n\traw data: %lu bytes\n", (ulong)pt->len );
    }

  leave:
    return 0;
}


static int
parse_compressed( IOBUF inp, int pkttype, unsigned long pktlen,
		  PACKET *pkt, int new_ctb )
{
    PKT_compressed *zd;

    /* pktlen is here 0, but data follows
     * (this should be the last object in a file or
     *	the compress algorithm should know the length)
     */
    zd = pkt->pkt.compressed =	m_alloc(sizeof *pkt->pkt.compressed );
    zd->len = 0; /* not yet used */
    zd->algorithm = iobuf_get_noeof(inp);
    zd->new_ctb = new_ctb;
    zd->buf = inp;
    if( list_mode )
	printf(":compressed packet: algo=%d\n", zd->algorithm);
    return 0;
}


static int
parse_encrypted( IOBUF inp, int pkttype, unsigned long pktlen,
				       PACKET *pkt, int new_ctb )
{
    PKT_encrypted *ed;

    ed = pkt->pkt.encrypted =  m_alloc(sizeof *pkt->pkt.encrypted );
    ed->len = pktlen;
    ed->buf = NULL;
    ed->new_ctb = new_ctb;
    ed->mdc_method = 0;
    if( pkttype == PKT_ENCRYPTED_MDC ) {
	/* test: this is the new encrypted_mdc packet */
	/* fixme: add some pktlen sanity checks */
	int version, method;

	version = iobuf_get_noeof(inp); pktlen--;
	if( version != 1 ) {
	    log_error("encrypted_mdc packet with unknown version %d\n",
								version);
	    goto leave;
	}
	method = iobuf_get_noeof(inp); pktlen--;
	if( method != DIGEST_ALGO_SHA1 ) {
	    log_error("encrypted_mdc does not use SHA1 method\n" );
	    goto leave;
	}
	ed->mdc_method = method;
    }
    if( pktlen && pktlen < 10 ) { /* actually this is blocksize+2 */
	log_error("packet(%d) too short\n", pkttype);
	skip_rest(inp, pktlen);
	goto leave;
    }
    if( list_mode ) {
	if( pktlen )
	    printf(":encrypted data packet:\n\tlength: %lu\n", pktlen);
	else
	    printf(":encrypted data packet:\n\tlength: unknown\n");
	if( ed->mdc_method )
	    printf("\tmdc_method: %d\n", ed->mdc_method );
    }

    ed->buf = inp;
    pktlen = 0;

  leave:
    return 0;
}

