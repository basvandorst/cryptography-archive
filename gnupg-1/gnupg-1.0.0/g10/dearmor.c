/* dearmor.c - Armor utility
 *	Copyright (C) 1998 Free Software Foundation, Inc.
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
#include "packet.h"
#include "options.h"
#include "main.h"


/****************
 * Take an armor file and write it out without armor
 */
int
dearmor_file( const char *fname )
{
    armor_filter_context_t afx;
    IOBUF inp = NULL, out = NULL;
    int rc = 0;
    int c;

    memset( &afx, 0, sizeof afx);

    /* prepare iobufs */
    if( !(inp = iobuf_open(fname)) ) {
	log_error("can't open %s: %s\n", fname? fname: "[stdin]",
					strerror(errno) );
	rc = G10ERR_OPEN_FILE;
	goto leave;
    }

    iobuf_push_filter( inp, armor_filter, &afx );

    if( (rc = open_outfile( fname, 0, &out )) )
	goto leave;



    while( (c = iobuf_get(inp)) != -1 )
	iobuf_put( out, c );


  leave:
    if( rc )
	iobuf_cancel(out);
    else
	iobuf_close(out);
    iobuf_close(inp);
    return rc;
}


/****************
 * Take file and write it out with armor
 */
int
enarmor_file( const char *fname )
{
    armor_filter_context_t afx;
    IOBUF inp = NULL, out = NULL;
    int rc = 0;
    int c;

    memset( &afx, 0, sizeof afx);

    /* prepare iobufs */
    if( !(inp = iobuf_open(fname)) ) {
	log_error("can't open %s: %s\n", fname? fname: "[stdin]",
					strerror(errno) );
	rc = G10ERR_OPEN_FILE;
	goto leave;
    }


    if( (rc = open_outfile( fname, 1, &out )) )
	goto leave;

    afx.what = 4;
    afx.hdrlines = "Comment: Use \"gpg --dearmor\" for unpacking\n";
    iobuf_push_filter( out, armor_filter, &afx );

    while( (c = iobuf_get(inp)) != -1 )
	iobuf_put( out, c );


  leave:
    if( rc )
	iobuf_cancel(out);
    else
	iobuf_close(out);
    iobuf_close(inp);
    return rc;
}


