/* armor.c - Armor flter
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
#include <errno.h>
#include <assert.h>
#include <ctype.h>

#include "errors.h"
#include "iobuf.h"
#include "memory.h"
#include "util.h"
#include "filter.h"
#include "packet.h"
#include "options.h"
#include "main.h"
#include "status.h"
#include "i18n.h"


#define MAX_LINELEN 20000

#define CRCINIT 0xB704CE
#define CRCPOLY 0X864CFB
#define CRCUPDATE(a,c) do {						    \
			a = ((a) << 8) ^ crc_table[((a)&0xff >> 16) ^ (c)]; \
			a &= 0x00ffffff;				    \
		    } while(0)
static u32 crc_table[256];
static byte bintoasc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			 "abcdefghijklmnopqrstuvwxyz"
			 "0123456789+/";
static byte asctobin[256]; /* runtime initialized */
static int is_initialized;


typedef enum {
    fhdrHASArmor = 0,
    fhdrNOArmor,
    fhdrINIT,
    fhdrINITCont,
    fhdrINITSkip,
    fhdrCHECKBegin,
    fhdrWAITHeader,
    fhdrWAITClearsig,
    fhdrSKIPHeader,
    fhdrCLEARSIG,
    fhdrREADClearsig,
    fhdrNullClearsig,
    fhdrEMPTYClearsig,
    fhdrCHECKClearsig,
    fhdrCHECKClearsig2,
    fhdrCHECKDashEscaped,
    fhdrCHECKDashEscaped2,
    fhdrCHECKDashEscaped3,
    fhdrREADClearsigNext,
    fhdrENDClearsig,
    fhdrENDClearsigHelp,
    fhdrTESTSpaces,
    fhdrCLEARSIGSimple,
    fhdrCLEARSIGSimpleNext,
    fhdrTEXT,
    fhdrTEXTSimple,
    fhdrERROR,
    fhdrERRORShow,
    fhdrEOF
} fhdr_state_t;


/* if we encounter this armor string with this index, go
 * into a mode which fakes packets and wait for the next armor */
#define BEGIN_SIGNATURE 2
#define BEGIN_SIGNED_MSG_IDX 3
static char *head_strings[] = {
    "BEGIN PGP MESSAGE",
    "BEGIN PGP PUBLIC KEY BLOCK",
    "BEGIN PGP SIGNATURE",
    "BEGIN PGP SIGNED MESSAGE",
    "BEGIN PGP ARMORED FILE",       /* gnupg extension */
    "BEGIN PGP PRIVATE KEY BLOCK",
    "BEGIN PGP SECRET KEY BLOCK",   /* only used by pgp2 */
    NULL
};
static char *tail_strings[] = {
    "END PGP MESSAGE",
    "END PGP PUBLIC KEY BLOCK",
    "END PGP SIGNATURE",
    "END dummy",
    "END PGP ARMORED FILE",
    "END PGP PRIVATE KEY BLOCK",
    "END PGP SECRET KEY BLOCK",
    NULL
};



static void
initialize(void)
{
    int i, j;
    u32 t;
    byte *s;

    /* init the crc lookup table */
    crc_table[0] = 0;
    for(i=j=0; j < 128; j++ ) {
	t = crc_table[j];
	if( t & 0x00800000 ) {
	    t <<= 1;
	    crc_table[i++] = t ^ CRCPOLY;
	    crc_table[i++] = t;
	}
	else {
	    t <<= 1;
	    crc_table[i++] = t;
	    crc_table[i++] = t ^ CRCPOLY;
	}
    }
    /* build the helptable for radix64 to bin conversion */
    for(i=0; i < 256; i++ )
	asctobin[i] = 255; /* used to detect invalid characters */
    for(s=bintoasc,i=0; *s; s++,i++ )
	asctobin[*s] = i;

    is_initialized=1;
}

/****************
 * Check whether this is an armored file or not
 * See also parse-packet.c for details on this code
 * Returns: True if it seems to be armored
 */
static int
is_armored( const byte *buf )
{
    int ctb, pkttype;

    ctb = *buf;
    if( !(ctb & 0x80) )
	return 1; /* invalid packet: assume it is armored */
    pkttype =  ctb & 0x40 ? (ctb & 0x3f) : ((ctb>>2)&0xf);
    switch( pkttype ) {
      case PKT_MARKER:
      case PKT_SYMKEY_ENC:
      case PKT_PUBLIC_KEY:
      case PKT_SECRET_KEY:
      case PKT_PUBKEY_ENC:
      case PKT_SIGNATURE:
      case PKT_COMMENT:
      case PKT_OLD_COMMENT:
      case PKT_PLAINTEXT:
      case PKT_COMPRESSED:
      case PKT_ENCRYPTED:
	return 0; /* seems to be a regular packet: not armored */
    }

    return 1;
}


/****************
 * Try to check whether the iobuf is armored
 * Returns true if this may be the case; the caller should use the
 *	   filter to do further processing.
 */
int
use_armor_filter( IOBUF a )
{
    byte buf[1];
    int n;

    n = iobuf_peek(a, buf, 1 );
    if( n == -1 )
	return 0; /* EOF, doesn't matter whether armored or not */
    if( !n )
	return 1; /* can't check it: try armored */
    return is_armored(buf);
}




static void
invalid_armor(void)
{
    write_status(STATUS_BADARMOR);
    g10_exit(1); /* stop here */
}


/****************
 * check whether the armor header is valid on a signed message.
 * this is for security reasons: the header lines are not included in the
 * hash and by using some creative formatting rules, Mallory could fake
 * any text at the beginning of a document; assuming it is read with
 * a simple viewer. We only allow the Hash Header.
 */
static int
parse_hash_header( const char *line )
{
    const char *s, *s2;
    unsigned found = 0;

    if( strlen(line) < 6  || strlen(line) > 60 )
	return 0; /* too short or too long */
    if( memcmp( line, "Hash:", 5 ) )
	return 0; /* invalid header */
    s = line+5;
    for(s=line+5;;s=s2) {
	for(; *s && (*s==' ' || *s == '\t'); s++ )
	    ;
	if( !*s )
	    break;
	for(s2=s+1; *s2 && *s2!=' ' && *s2 != '\t' && *s2 != ','; s2++ )
	    ;
	if( !strncmp( s, "RIPEMD160", s2-s ) )
	    found |= 1;
	else if( !strncmp( s, "SHA1", s2-s ) )
	    found |= 2;
	else if( !strncmp( s, "MD5", s2-s ) )
	    found |= 4;
	else if( !strncmp( s, "TIGER", s2-s ) )
	    found |= 8;
	else
	    return 0;
	for(; *s2 && (*s2==' ' || *s2 == '\t'); s2++ )
	    ;
	if( *s2 && *s2 != ',' )
	    return 0;
	if( *s2 )
	    s2++;
    }
    return found;
}



/****************
 * Check whether this is a armor line.
 * returns: -1 if it is not a armor header or the index number of the
 * armor header.
 */
static int
is_armor_header( byte *line, unsigned len )
{
    const char *s;
    byte *save_p, *p;
    int save_c;
    int i;

    if( len < 15 )
	return -1; /* too short */
    if( memcmp( line, "-----", 5 ) )
	return -1; /* no */
    p = strstr( line+5, "-----");
    if( !p )
	return -1;
    save_p = p;
    p += 5;
    if( *p == '\r' )
	p++;
    if( *p == '\n' )
	p++;
    if( *p )
	return -1; /* garbage after dashes */
    save_c = *save_p; *save_p = 0;
    p = line+5;
    for(i=0; (s=head_strings[i]); i++ )
	if( !strcmp(s, p) )
	    break;
    *save_p = save_c;
    if( !s )
	return -1; /* unknown armor line */

    if( opt.verbose > 1 )
	log_info(_("armor: %s\n"), head_strings[i]);
    return i;
}



/****************
 * Parse a header lines
 * Return 0: Empty line (end of header lines)
 *	 -1: invalid header line
 *	 >0: Good header line
 */
static int
parse_header_line( armor_filter_context_t *afx, byte *line, unsigned len )
{
    byte *p;
    int hashes=0;

    /* fixme: why this double check?  I think the original code w/o the
     * second check for an empty line was done from an early draft of
     * of OpenPGP - or simply very stupid code */
    if( *line == '\n' || ( len && (*line == '\r' && line[1]=='\n') ) )
	return 0; /* empty line */
    len = trim_trailing_ws( line, len );
    if( !len )
	return 0; /* WS only same as empty line */

    p = strchr( line, ':');
    if( !p || !p[1] ) {
	log_error(_("invalid armor header: "));
	print_string( stderr, line, len, 0 );
	putc('\n', stderr);
	return -1;
    }

    if( opt.verbose ) {
	log_info(_("armor header: "));
	print_string( stderr, line, len, 0 );
	putc('\n', stderr);
    }

    if( afx->in_cleartext ) {
	if( (hashes=parse_hash_header( line )) )
	    afx->hashes |= hashes;
	else if( strlen(line) > 15 && !memcmp( line, "NotDashEscaped:", 15 ) )
	    afx->not_dash_escaped = 1;
	else {
	    log_error(_("invalid clearsig header\n"));
	    return -1;
	}
    }
    return 1;
}



/* figure out whether the data is armored or not */
static int
check_input( armor_filter_context_t *afx, IOBUF a )
{
    int rc = 0;
    int i;
    byte *line;
    unsigned len;
    unsigned maxlen;
    int hdr_line = -1;

    /* read the first line to see whether this is armored data */
    maxlen = MAX_LINELEN;
    len = afx->buffer_len = iobuf_read_line( a, &afx->buffer,
					     &afx->buffer_size, &maxlen );
    line = afx->buffer;
    if( !maxlen ) {
	/* line has been truncated: assume not armored */
	afx->inp_checked = 1;
	afx->inp_bypass = 1;
	return 0;
    }

    if( !len ) {
	return -1; /* eof */
    }

    /* (the line is always a C string but maybe longer) */
    if( *line == '\n' || ( len && (*line == '\r' && line[1]=='\n') ) )
	;
    else if( !is_armored( line ) ) {
	afx->inp_checked = 1;
	afx->inp_bypass = 1;
	return 0;
    }

    /* find the armor header */
    while(len) {
	i = is_armor_header( line, len );
	if( i >= 0 && !(afx->only_keyblocks && i != 1 && i != 5 && i != 6 )) {
	    hdr_line = i;
	    if( hdr_line == BEGIN_SIGNED_MSG_IDX ) {
		if( afx->in_cleartext ) {
		    log_error(_("nested clear text signatures\n"));
		    rc = G10ERR_INVALID_ARMOR;
		}
		afx->in_cleartext = 1;
	    }
	    break;
	}
	/* read the next line (skip all truncated lines) */
	do {
	    maxlen = MAX_LINELEN;
	    afx->buffer_len = iobuf_read_line( a, &afx->buffer,
					       &afx->buffer_size, &maxlen );
	    line = afx->buffer;
	    len = afx->buffer_len;
	} while( !maxlen );
    }

    /* parse the header lines */
    while(len) {
	/* read the next line (skip all truncated lines) */
	do {
	    maxlen = MAX_LINELEN;
	    afx->buffer_len = iobuf_read_line( a, &afx->buffer,
					       &afx->buffer_size, &maxlen );
	    line = afx->buffer;
	    len = afx->buffer_len;
	} while( !maxlen );

	i = parse_header_line( afx, line, len );
	if( i <= 0 ) {
	    if( i )
		rc = G10ERR_INVALID_ARMOR;
	    break;
	}
    }


    if( rc )
	invalid_armor();
    else if( afx->in_cleartext )
	afx->faked = 1;
    else {
	afx->inp_checked = 1;
	afx->crc = CRCINIT;
	afx->idx = 0;
	afx->radbuf[0] = 0;
    }

    return rc;
}



/****************
 * Fake a literal data packet and wait for the next armor line
 * fixme: empty line handling and null length clear text signature are
 *	  not implemented/checked.
 */
static int
fake_packet( armor_filter_context_t *afx, IOBUF a,
	     size_t *retn, byte *buf, size_t size  )
{
    int rc = 0;
    size_t len = 0;
    int lastline = 0;
    unsigned maxlen, n;
    byte *p;

    len = 2;	/* reserve 2 bytes for the length header */
    size -= 2;	/* and 2 for the terminating header */
    while( !rc && len < size ) {
	/* copy what we have in the line buffer */
	if( afx->faked == 1 )
	    afx->faked++; /* skip the first (empty) line */
	else {
	    while( len < size && afx->buffer_pos < afx->buffer_len )
		buf[len++] = afx->buffer[afx->buffer_pos++];
	    if( len >= size )
		continue;
	}

	/* read the next line */
	maxlen = MAX_LINELEN;
	afx->buffer_pos = 0;
	afx->buffer_len = iobuf_read_line( a, &afx->buffer,
					   &afx->buffer_size, &maxlen );
	if( !afx->buffer_len ) {
	    rc = -1; /* eof (should not happen) */
	    continue;
	}
	if( !maxlen )
	    afx->truncated++;
	if( !afx->not_dash_escaped ) {
	    /* PGP2 does not treat a tab as white space character */
	    afx->buffer_len =
		    trim_trailing_chars( afx->buffer, afx->buffer_len,
					 afx->pgp2mode ? " \r\n" : " \t\r\n");
	    /* the buffer is always allocated with enough space to append
	     * a CR, LF, Nul */
	    afx->buffer[afx->buffer_len++] = '\r';
	    afx->buffer[afx->buffer_len++] = '\n';
	    afx->buffer[afx->buffer_len] = 0;
	}
	p = afx->buffer;
	n = afx->buffer_len;

	if( n > 2 && *p == '-' ) {
	    /* check for dash escaped or armor header */
	    if( p[1] == ' ' && !afx->not_dash_escaped ) {
		/* issue a warning if it is not regular encoded */
		if( p[2] != '-' && !( n > 6 && !memcmp(p+2, "From ", 5))) {
		    log_info(_("invalid dash escaped line: "));
		    print_string( stderr, p, n, 0 );
		    putc('\n', stderr);
		}
		afx->buffer_pos = 2; /* skip */
	    }
	    else if( n >= 15 &&  p[1] == '-' && p[2] == '-' && p[3] == '-' ) {
		int type = is_armor_header( p, n );
		if( afx->not_dash_escaped && type != BEGIN_SIGNATURE )
		    ; /* this is okay */
		else {
		    if( type != BEGIN_SIGNATURE ) {
			log_info(_("unexpected armor:"));
			print_string( stderr, p, n, 0 );
			putc('\n', stderr);
		    }
		    lastline = 1;
		    rc = -1;
		}
	    }
	}
    }

    buf[0] = (len-2) >> 8;
    buf[1] = (len-2);
    if( lastline ) { /* write last (ending) length header */
	if( buf[0] || buf[1] ) { /* only if we have some text */
	    buf[len++] = 0;
	    buf[len++] = 0;
	}
	rc = 0;
	afx->faked = 0;
	afx->in_cleartext = 0;
	/* and now read the header lines */
	afx->buffer_pos = 0;
	for(;;) {
	    int i;

	    /* read the next line (skip all truncated lines) */
	    do {
		maxlen = MAX_LINELEN;
		afx->buffer_len = iobuf_read_line( a, &afx->buffer,
						 &afx->buffer_size, &maxlen );
	    } while( !maxlen );
	    p = afx->buffer;
	    n = afx->buffer_len;
	    if( !n ) {
		rc = -1;
		break; /* eof */
	    }
	    i = parse_header_line( afx, p , n );
	    if( i <= 0 ) {
		if( i )
		    invalid_armor();
		break;
	    }
	}
	afx->inp_checked = 1;
	afx->crc = CRCINIT;
	afx->idx = 0;
	afx->radbuf[0] = 0;
    }

    *retn = len;
    return rc;
}



static int
radix64_read( armor_filter_context_t *afx, IOBUF a, size_t *retn,
	      byte *buf, size_t size )
{
    byte val;
    int c=0, c2; /*init c because gcc is not clever enough for the continue*/
    int checkcrc=0;
    int rc = 0;
    size_t n = 0;
    int  idx, i;
    u32 crc;

    crc = afx->crc;
    idx = afx->idx;
    val = afx->radbuf[0];
    for( n=0; n < size; ) {

	if( afx->buffer_pos < afx->buffer_len )
	    c = afx->buffer[afx->buffer_pos++];
	else { /* read the next line */
	    unsigned maxlen = MAX_LINELEN;
	    afx->buffer_pos = 0;
	    afx->buffer_len = iobuf_read_line( a, &afx->buffer,
					       &afx->buffer_size, &maxlen );
	    if( !maxlen )
		afx->truncated++;
	    if( !afx->buffer_len )
		break; /* eof */
	    continue;
	}

      again:
	if( c == '\n' || c == ' ' || c == '\r' || c == '\t' )
	    continue;
	else if( c == '=' ) { /* pad character: stop */
	    /* some mailers leave quoted-printable encoded characters
	     * so we try to workaround this */
	    if( afx->buffer_pos+2 < afx->buffer_len ) {
		int cc1, cc2, cc3;
		cc1 = afx->buffer[afx->buffer_pos];
		cc2 = afx->buffer[afx->buffer_pos+1];
		cc3 = afx->buffer[afx->buffer_pos+2];
		if( isxdigit(cc1) && isxdigit(cc2)
				  && strchr( "=\n\r\t ", cc3 )) {
		    /* well it seems to be the case - adjust */
		    c = isdigit(cc1)? (cc1 - '0'): (toupper(cc1)-'A'+10);
		    c <<= 4;
		    c |= isdigit(cc2)? (cc2 - '0'): (toupper(cc2)-'A'+10);
		    afx->buffer_pos += 2;
		    afx->qp_detected = 1;
		    goto again;
		}
	    }

	    if( idx == 1 )
		buf[n++] = val;
	    checkcrc++;
	    break;
	}
	else if( (c = asctobin[(c2=c)]) == 255 ) {
	    log_error(_("invalid radix64 character %02x skipped\n"), c2);
	    continue;
	}
	switch(idx) {
	  case 0: val =  c << 2; break;
	  case 1: val |= (c>>4)&3; buf[n++]=val;val=(c<<4)&0xf0;break;
	  case 2: val |= (c>>2)&15; buf[n++]=val;val=(c<<6)&0xc0;break;
	  case 3: val |= c&0x3f; buf[n++] = val; break;
	}
	idx = (idx+1) % 4;
    }

    for(i=0; i < n; i++ )
	crc = (crc << 8) ^ crc_table[((crc >> 16)&0xff) ^ buf[i]];
    crc &= 0x00ffffff;
    afx->crc = crc;
    afx->idx = idx;
    afx->radbuf[0] = val;

    if( checkcrc ) {
	afx->any_data = 1;
	afx->inp_checked=0;
	afx->faked = 0;
	for(;;) { /* skip lf and pad characters */
	    if( afx->buffer_pos < afx->buffer_len )
		c = afx->buffer[afx->buffer_pos++];
	    else { /* read the next line */
		unsigned maxlen = MAX_LINELEN;
		afx->buffer_pos = 0;
		afx->buffer_len = iobuf_read_line( a, &afx->buffer,
						   &afx->buffer_size, &maxlen );
		if( !maxlen )
		    afx->truncated++;
		if( !afx->buffer_len )
		    break; /* eof */
		continue;
	    }
	    if( c == '\n' || c == ' ' || c == '\r'
		|| c == '\t' || c == '=' )
		continue;
	    break;
	}
	if( c == -1 )
	    log_error(_("premature eof (no CRC)\n"));
	else {
	    u32 mycrc = 0;
	    idx = 0;
	    do {
		if( (c = asctobin[c]) == 255 )
		    break;
		switch(idx) {
		  case 0: val =  c << 2; break;
		  case 1: val |= (c>>4)&3; mycrc |= val << 16;val=(c<<4)&0xf0;break;
		  case 2: val |= (c>>2)&15; mycrc |= val << 8;val=(c<<6)&0xc0;break;
		  case 3: val |= c&0x3f; mycrc |= val; break;
		}
		for(;;) {
		    if( afx->buffer_pos < afx->buffer_len )
			c = afx->buffer[afx->buffer_pos++];
		    else { /* read the next line */
			unsigned maxlen = MAX_LINELEN;
			afx->buffer_pos = 0;
			afx->buffer_len = iobuf_read_line( a, &afx->buffer,
							   &afx->buffer_size,
								&maxlen );
			if( !maxlen )
			    afx->truncated++;
			if( !afx->buffer_len )
			    break; /* eof */
			continue;
		    }
		    break;
		}
		if( !afx->buffer_len )
		    break; /* eof */
	    } while( ++idx < 4 );
	    if( c == -1 ) {
		log_error(_("premature eof (in CRC)\n"));
		rc = G10ERR_INVALID_ARMOR;
	    }
	    else if( idx != 4 ) {
		log_error(_("malformed CRC\n"));
		rc = G10ERR_INVALID_ARMOR;
	    }
	    else if( mycrc != afx->crc ) {
		log_error(_("CRC error; %06lx - %06lx\n"),
				    (ulong)afx->crc, (ulong)mycrc);
		rc = G10ERR_INVALID_ARMOR;
	    }
	    else {
		rc = 0;
	      #if 0
		for(rc=0;!rc;) {
		    rc = 0 /*check_trailer( &fhdr, c )*/;
		    if( !rc ) {
			if( (c=iobuf_get(a)) == -1 )
			    rc = 2;
		    }
		}
		if( rc == -1 )
		    rc = 0;
		else if( rc == 2 ) {
		    log_error(_("premature eof (in Trailer)\n"));
		    rc = G10ERR_INVALID_ARMOR;
		}
		else {
		    log_error(_("error in trailer line\n"));
		    rc = G10ERR_INVALID_ARMOR;
		}
	      #endif
	    }
	}
    }

    if( !n )
	rc = -1;

    *retn = n;
    return rc;
}

/****************
 * This filter is used to handle the armor stuff
 */
int
armor_filter( void *opaque, int control,
	     IOBUF a, byte *buf, size_t *ret_len)
{
    size_t size = *ret_len;
    armor_filter_context_t *afx = opaque;
    int rc=0, i, c;
    byte radbuf[3];
    int  idx, idx2;
    size_t n=0;
    u32 crc;
  #if 0
    static FILE *fp ;

    if( !fp ) {
	fp = fopen("armor.out", "w");
	assert(fp);
    }
  #endif

    if( DBG_FILTER )
	log_debug("armor-filter: control: %d\n", control );
    if( control == IOBUFCTRL_UNDERFLOW && afx->inp_bypass ) {
	n = 0;
	if( afx->buffer_len ) {
	    for(; n < size && afx->buffer_pos < afx->buffer_len; n++ )
		buf[n++] = afx->buffer[afx->buffer_pos++];
	    if( afx->buffer_pos >= afx->buffer_len )
		afx->buffer_len = 0;
	}
	for(; n < size; n++ ) {
	    if( (c=iobuf_get(a)) == -1 )
		break;
	    buf[n] = c & 0xff;
	}
	if( !n )
	    rc = -1;
	*ret_len = n;
    }
    else if( control == IOBUFCTRL_UNDERFLOW ) {
	if( size < 15+(4*15) )	/* need space for up to 4 onepass_sigs */
	    BUG(); /* supplied buffer too short */

	if( afx->faked )
	    rc = fake_packet( afx, a, &n, buf, size );
	else if( !afx->inp_checked ) {
	    rc = check_input( afx, a );
	    if( afx->inp_bypass ) {
		for(n=0; n < size && afx->buffer_pos < afx->buffer_len; )
		    buf[n++] = afx->buffer[afx->buffer_pos++];
		if( afx->buffer_pos >= afx->buffer_len )
		    afx->buffer_len = 0;
		if( !n )
		    rc = -1;
	    }
	    else if( afx->faked ) {
		unsigned hashes = afx->hashes;
		/* the buffer is at least 15+n*15 bytes long, so it
		 * is easy to construct the packets */

		hashes &= 1|2|4|8;
		if( !hashes ) {
		    hashes |= 4;  /* default to MD 5 */
		    afx->pgp2mode = 1;
		}
		n=0;
		do {
		    /* first some onepass signature packets */
		    buf[n++] = 0x90; /* old format, type 4, 1 length byte */
		    buf[n++] = 13;   /* length */
		    buf[n++] = 3;    /* version */
		    buf[n++] = 0x01; /* sigclass 0x01 (canonical text mode)*/
		    if( hashes & 1 ) {
			hashes &= ~1;
			buf[n++] = DIGEST_ALGO_RMD160;
		    }
		    else if( hashes & 2 ) {
			hashes &= ~2;
			buf[n++] = DIGEST_ALGO_SHA1;
		    }
		    else if( hashes & 4 ) {
			hashes &= ~4;
			buf[n++] = DIGEST_ALGO_MD5;
		    }
		    else if( hashes & 8 ) {
			hashes &= ~8;
			buf[n++] = DIGEST_ALGO_TIGER;
		    }
		    else
			buf[n++] = 0;	 /* (don't know) */

		    buf[n++] = 0;    /* public key algo (don't know) */
		    memset(buf+n, 0, 8); /* don't know the keyid */
		    n += 8;
		    buf[n++] = !hashes;   /* last one */
		} while( hashes );

		/* followed by a plaintext packet */
		buf[n++] = 0xaf; /* old packet format, type 11, var length */
		buf[n++] = 0;	 /* set the length header */
		buf[n++] = 6;
		buf[n++] = 't';  /* canonical text mode */
		buf[n++] = 0;	 /* namelength */
		memset(buf+n, 0, 4); /* timestamp */
		n += 4;
	    }
	    else if( !rc )
		rc = radix64_read( afx, a, &n, buf, size );
	}
	else
	    rc = radix64_read( afx, a, &n, buf, size );
      #if 0
	if( n )
	    if( fwrite(buf, n, 1, fp ) != 1 )
		BUG();
      #endif
	*ret_len = n;
    }
    else if( control == IOBUFCTRL_FLUSH && !afx->cancel ) {
	if( !afx->status ) { /* write the header line */
	    if( afx->what >= DIM(head_strings) )
		log_bug("afx->what=%d", afx->what);
	    iobuf_writestr(a, "-----");
	    iobuf_writestr(a, head_strings[afx->what] );
	    iobuf_writestr(a, "-----\n");
	    if( !opt.no_version )
		iobuf_writestr(a, "Version: GnuPG v"  VERSION " ("
					      PRINTABLE_OS_NAME ")\n");

	    if( opt.comment_string ) {
		const char *s = opt.comment_string;
		if( *s ) {
		    iobuf_writestr(a, "Comment: " );
		    for( ; *s; s++ ) {
			if( *s == '\n' )
			    iobuf_writestr(a, "\\n" );
			else if( *s == '\r' )
			    iobuf_writestr(a, "\\r" );
			else if( *s == '\v' )
			    iobuf_writestr(a, "\\v" );
			else
			    iobuf_put(a, *s );
		    }
		    iobuf_put(a, '\n' );
		}
	    }
	    else
		iobuf_writestr(a,
		    "Comment: For info see http://www.gnupg.org\n");
	    if( afx->hdrlines )
		iobuf_writestr(a, afx->hdrlines);
	    iobuf_put(a, '\n');
	    afx->status++;
	    afx->idx = 0;
	    afx->idx2 = 0;
	    afx->crc = CRCINIT;
	}
	crc = afx->crc;
	idx = afx->idx;
	idx2 = afx->idx2;
	for(i=0; i < idx; i++ )
	    radbuf[i] = afx->radbuf[i];

	for(i=0; i < size; i++ )
	    crc = (crc << 8) ^ crc_table[((crc >> 16)&0xff) ^ buf[i]];
	crc &= 0x00ffffff;

	for( ; size; buf++, size-- ) {
	    radbuf[idx++] = *buf;
	    if( idx > 2 ) {
		idx = 0;
		c = bintoasc[(*radbuf >> 2) & 077];
		iobuf_put(a, c);
		c = bintoasc[(((*radbuf<<4)&060)|((radbuf[1] >> 4)&017))&077];
		iobuf_put(a, c);
		c = bintoasc[(((radbuf[1]<<2)&074)|((radbuf[2]>>6)&03))&077];
		iobuf_put(a, c);
		c = bintoasc[radbuf[2]&077];
		iobuf_put(a, c);
		if( ++idx2 >= (64/4) ) { /* pgp doesn't like 72 here */
		    iobuf_put(a, '\n');
		    idx2=0;
		}
	    }
	}
	for(i=0; i < idx; i++ )
	    afx->radbuf[i] = radbuf[i];
	afx->idx = idx;
	afx->idx2 = idx2;
	afx->crc  = crc;
    }
    else if( control == IOBUFCTRL_INIT ) {
	if( !is_initialized )
	    initialize();
    }
    else if( control == IOBUFCTRL_CANCEL ) {
	afx->cancel = 1;
    }
    else if( control == IOBUFCTRL_FREE ) {
	if( afx->cancel )
	    ;
	else if( afx->status ) { /* pad, write cecksum, and bottom line */
	    crc = afx->crc;
	    idx = afx->idx;
	    idx2 = afx->idx2;
	    for(i=0; i < idx; i++ )
		radbuf[i] = afx->radbuf[i];
	    if( idx ) {
		c = bintoasc[(*radbuf>>2)&077];
		iobuf_put(a, c);
		if( idx == 1 ) {
		    c = bintoasc[((*radbuf << 4) & 060) & 077];
		    iobuf_put(a, c);
		    iobuf_put(a, '=');
		    iobuf_put(a, '=');
		}
		else { /* 2 */
		    c = bintoasc[(((*radbuf<<4)&060)|((radbuf[1]>>4)&017))&077];
		    iobuf_put(a, c);
		    c = bintoasc[((radbuf[1] << 2) & 074) & 077];
		    iobuf_put(a, c);
		    iobuf_put(a, '=');
		}
		if( ++idx2 >= (64/4) ) { /* pgp doesn't like 72 here */
		    iobuf_put(a, '\n');
		    idx2=0;
		}
	    }
	    /* may need a linefeed */
	    if( idx2 )
		iobuf_put(a, '\n');
	    /* write the CRC */
	    iobuf_put(a, '=');
	    radbuf[0] = crc >>16;
	    radbuf[1] = crc >> 8;
	    radbuf[2] = crc;
	    c = bintoasc[(*radbuf >> 2) & 077];
	    iobuf_put(a, c);
	    c = bintoasc[(((*radbuf<<4)&060)|((radbuf[1] >> 4)&017))&077];
	    iobuf_put(a, c);
	    c = bintoasc[(((radbuf[1]<<2)&074)|((radbuf[2]>>6)&03))&077];
	    iobuf_put(a, c);
	    c = bintoasc[radbuf[2]&077];
	    iobuf_put(a, c);
	    iobuf_put(a, '\n');
	    /* and the the trailer */
	    if( afx->what >= DIM(tail_strings) )
		log_bug("afx->what=%d", afx->what);
	    iobuf_writestr(a, "-----");
	    iobuf_writestr(a, tail_strings[afx->what] );
	    iobuf_writestr(a, "-----\n");
	}
	else if( !afx->any_data && !afx->inp_bypass ) {
	    log_error(_("no valid OpenPGP data found.\n"));
	    afx->no_openpgp_data = 1;
	    write_status_text( STATUS_NODATA, "1" );
	}
	if( afx->truncated )
	    log_info(_("invalid armor: line longer than %d characters\n"),
		      MAX_LINELEN );
	/* issue an error to enforce dissemination of correct software */
	if( afx->qp_detected )
	    log_error(_("quoted printable character in armor - "
			"probably a buggy MTA has been used\n") );
	m_free( afx->buffer );
	afx->buffer = NULL;
    }
    else if( control == IOBUFCTRL_DESC )
	*(char**)buf = "armor_filter";
    return rc;
}


/****************
 * create a radix64 encoded string.
 */
char *
make_radix64_string( const byte *data, size_t len )
{
    char *buffer, *p;

    buffer = p = m_alloc( (len+2)/3*4 + 1 );
    for( ; len >= 3 ; len -= 3, data += 3 ) {
	*p++ = bintoasc[(data[0] >> 2) & 077];
	*p++ = bintoasc[(((data[0] <<4)&060)|((data[1] >> 4)&017))&077];
	*p++ = bintoasc[(((data[1]<<2)&074)|((data[2]>>6)&03))&077];
	*p++ = bintoasc[data[2]&077];
    }
    if( len == 2 ) {
	*p++ = bintoasc[(data[0] >> 2) & 077];
	*p++ = bintoasc[(((data[0] <<4)&060)|((data[1] >> 4)&017))&077];
	*p++ = bintoasc[((data[1]<<2)&074)];
    }
    else if( len == 1 ) {
	*p++ = bintoasc[(data[0] >> 2) & 077];
	*p++ = bintoasc[(data[0] <<4)&060];
    }
    *p = 0;
    return buffer;
}

