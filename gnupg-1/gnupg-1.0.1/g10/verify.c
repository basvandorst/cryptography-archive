/* verify.c - verify signed data
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

#include "options.h"
#include "packet.h"
#include "errors.h"
#include "iobuf.h"
#include "keydb.h"
#include "memory.h"
#include "util.h"
#include "main.h"
#include "status.h"
#include "filter.h"
#include "ttyio.h"
#include "i18n.h"



/****************
 * Assume that the input is a signature and verify it without
 * generating any output.  With no arguments, the signature packet
 * is read from stdin (it may be a detached signature when not
 * used in batch mode). If only a sigfile is given, it may be a complete
 * signature or a detached signature in which case the signed stuff
 * is expected from stdin. With more than 1 argument, the first should
 * be a detached signature and the remaining files are the signed stuff.
 */

int
verify_signatures( int nfiles, char **files )
{
    IOBUF fp;
    armor_filter_context_t afx;
    const char *sigfile;
    int i, rc;
    STRLIST sl;

    memset( &afx, 0, sizeof afx);
    sigfile = nfiles? *files : NULL;

    /* open the signature file */
    fp = iobuf_open(sigfile);
    if( !fp ) {
	log_error(_("can't open `%s'\n"), print_fname_stdin(sigfile));
	return G10ERR_OPEN_FILE;
    }

    if( !opt.no_armor && use_armor_filter( fp ) )
	iobuf_push_filter( fp, armor_filter, &afx );

    sl = NULL;
    for(i=1 ; i < nfiles; i++ )
	add_to_strlist( &sl, files[i] );
    rc = proc_signature_packets( NULL, fp, sl, sigfile );
    free_strlist(sl);
    iobuf_close(fp);
    if( afx.no_openpgp_data && rc == -1 ) {
	log_error(_("the signature could not be verified.\n"
		   "Please remember that the signature file (.sig or .asc)\n"
		   "should be the first file given on the command line.\n") );
	rc = 0;
    }

    return rc;
}


static void
print_file_status( int status, const char *name, int what )
{
    char *p = m_alloc(strlen(name)+10);
    sprintf(p, "%d %s", what, name );
    write_status_text( status, p );
    m_free(p);
}


static int
verify_one_file( const char *name )
{
    IOBUF fp;
    armor_filter_context_t afx;
    int rc;

    print_file_status( STATUS_FILE_START, name, 1 );
    fp = iobuf_open(name);
    if( !fp ) {
	print_file_status( STATUS_FILE_ERROR, name, 1 );
	log_error(_("can't open `%s'\n"), print_fname_stdin(name));
	return G10ERR_OPEN_FILE;
    }

    if( !opt.no_armor ) {
	if( use_armor_filter( fp ) ) {
	    memset( &afx, 0, sizeof afx);
	    iobuf_push_filter( fp, armor_filter, &afx );
	}
    }

    rc = proc_signature_packets( NULL, fp, NULL, name );
    iobuf_close(fp);
    write_status( STATUS_FILE_DONE );
    return rc;
}

/****************
 * Verify each file given in the files array or read the names of the
 * files from stdin.
 * Note:  This function can not handle detached signatures.
 */
int
verify_files( int nfiles, char **files )
{
    int i;

    if( !nfiles ) { /* read the filenames from stdin */
	char line[2048];
	unsigned int lno = 0;

	while( fgets(line, DIM(line), stdin) ) {
	    lno++;
	    if( !*line || line[strlen(line)-1] != '\n' ) {
		log_error(_("input line %u too long or missing LF\n"), lno );
		return G10ERR_GENERAL;
	    }
	    /* This code does not work on MSDOS but how cares there are
	     * also no script languages available.  We don't strip any
	     * spaces, so that we can process nearly all filenames */
	    line[strlen(line)-1] = 0;
	    verify_one_file( line );
	}

    }
    else {  /* take filenames from the array */
	for(i=0; i < nfiles; i++ )
	    verify_one_file( files[i] );
    }
    return 0;
}

