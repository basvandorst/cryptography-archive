/* textfilter.c
 *	Copyright (C) 1998,1999 Free Software Foundation, Inc.
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

#include "errors.h"
#include "iobuf.h"
#include "memory.h"
#include "util.h"
#include "filter.h"
#include "i18n.h"


#define MAX_LINELEN 19995 /* a little bit smaller than in armor.c */
			  /* to make sure that a warning is displayed while */
			  /* creating a message */

static unsigned
len_without_trailing_chars( byte *line, unsigned len, const char *trimchars )
{
    byte *p, *mark;
    unsigned n;

    for(mark=NULL, p=line, n=0; n < len; n++, p++ ) {
	if( strchr( trimchars, *p ) ) {
	    if( !mark )
		mark = p;
	}
	else
	    mark = NULL;
    }

    return mark? (mark - line) : len;
}

unsigned
len_without_trailing_ws( byte *line, unsigned len )
{
    return len_without_trailing_chars( line, len, " \t\r\n" );
}




static int
standard( text_filter_context_t *tfx, IOBUF a,
	  byte *buf, size_t size, size_t *ret_len)
{
    int rc=0;
    size_t len = 0;
    unsigned maxlen;

    assert( size > 10 );
    size -= 2;	/* reserve 2 bytes to append CR,LF */
    while( !rc && len < size ) {
	int lf_seen;

	while( len < size && tfx->buffer_pos < tfx->buffer_len )
	    buf[len++] = tfx->buffer[tfx->buffer_pos++];
	if( len >= size )
	    continue;

	/* read the next line */
	maxlen = MAX_LINELEN;
	tfx->buffer_pos = 0;
	tfx->buffer_len = iobuf_read_line( a, &tfx->buffer,
					   &tfx->buffer_size, &maxlen );
	if( !maxlen )
	    tfx->truncated++;
	if( !tfx->buffer_len ) {
	    if( !len )
		rc = -1; /* eof */
	    break;
	}
	lf_seen = tfx->buffer[tfx->buffer_len-1] == '\n';
	tfx->buffer_len = trim_trailing_ws( tfx->buffer, tfx->buffer_len );
	if( lf_seen ) {
	    tfx->buffer[tfx->buffer_len++] = '\r';
	    tfx->buffer[tfx->buffer_len++] = '\n';
	}
    }
    *ret_len = len;
    return rc;
}




/****************
 * The filter is used to make canonical text: Lines are terminated by
 * CR, LF, trailing white spaces are removed.
 */
int
text_filter( void *opaque, int control,
	     IOBUF a, byte *buf, size_t *ret_len)
{
    size_t size = *ret_len;
    text_filter_context_t *tfx = opaque;
    int rc=0;

    if( control == IOBUFCTRL_UNDERFLOW ) {
	rc = standard( tfx, a, buf, size, ret_len );
    }
    else if( control == IOBUFCTRL_FREE ) {
	if( tfx->truncated )
	    log_error(_("can't handle text lines longer than %d characters\n"),
			MAX_LINELEN );
	m_free( tfx->buffer );
	tfx->buffer = NULL;
    }
    else if( control == IOBUFCTRL_DESC )
	*(char**)buf = "text_filter";
    return rc;
}


/****************
 * Copy data from INP to OUT and do some escaping if requested.
 * md is updated as required by rfc2440
 */
int
copy_clearsig_text( IOBUF out, IOBUF inp, MD_HANDLE md,
		    int escape_dash, int escape_from, int pgp2mode )
{
    unsigned maxlen;
    byte *buffer = NULL;    /* malloced buffer */
    unsigned bufsize;	    /* and size of this buffer */
    unsigned n;
    int truncated = 0;
    int pending_lf = 0;

    if( !escape_dash )
	escape_from = 0;

    for(;;) {
	maxlen = MAX_LINELEN;
	n = iobuf_read_line( inp, &buffer, &bufsize, &maxlen );
	if( !maxlen )
	    truncated++;

	if( !n )
	    break; /* read_line has returned eof */

	/* update the message digest */
	if( escape_dash ) {
	    if( pending_lf ) {
		md_putc( md, '\r' );
		md_putc( md, '\n' );
	    }
	    md_write( md, buffer,
		     len_without_trailing_chars( buffer, n,
						 pgp2mode? " \r\n":" \t\r\n"));
	}
	else
	    md_write( md, buffer, n );
	pending_lf = buffer[n-1] == '\n';

	/* write the output */
	if(    ( escape_dash && *buffer == '-')
	    || ( escape_from && n > 4 && !memcmp(buffer, "From ", 5 ) ) ) {
	    iobuf_put( out, '-' );
	    iobuf_put( out, ' ' );
	}
      #ifdef __MINGW32__
	/* make sure the lines do end in CR,LF */
	if( n > 1 && ( (buffer[n-2] == '\r' && buffer[n-1] == '\n' )
			    || (buffer[n-2] == '\n' && buffer[n-1] == '\r'))) {
	    iobuf_write( out, buffer, n-2 );
	    iobuf_put( out, '\r');
	    iobuf_put( out, '\n');
	}
	else if( n && buffer[n-1] == '\n' ) {
	    iobuf_write( out, buffer, n-1 );
	    iobuf_put( out, '\r');
	    iobuf_put( out, '\n');
	}
	else
	    iobuf_write( out, buffer, n );

      #else
	iobuf_write( out, buffer, n );
      #endif
    }

    /* at eof */
    if( !pending_lf ) { /* make sure that the file ends with a LF */
      #ifndef __MINGW32__
	iobuf_put( out, '\r');
      #endif
	iobuf_put( out, '\n');
	if( !escape_dash )
	    md_putc( md, '\n' );
    }

    if( truncated )
	log_info(_("input line longer than %d characters\n"), MAX_LINELEN );

    return 0; /* okay */
}

