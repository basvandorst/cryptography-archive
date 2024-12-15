/*********************************************************************
 * Filename:      compress.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:44:35 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 *
 * $Id: compress.c,v 1.14 1997/12/11 18:55:19 tage Exp $
 *
 ********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <zlib.h>

#include <pgplib.h>


/*
 * Decompress a buffer containing a PGP compressed-package.
 * Return -1 on error, 0 if all is well.
 *
 * You should actually be ready to capture SIGSEGV before calling
 * this routine.  See zlib.h for details.
 */

int
decompress_buffer(u_char	*inbuf,
		  int 		inbuf_len,
		  u_char 	**outbuf,
		  int		*outbuf_len)
{
    int 	count;
    u_char	alg;
    u_char	*tmp, *p;
    z_stream	stream;
    int		bufsize;
    /* scratch */
    int i;

    if ( inbuf==NULL || inbuf_len < 3 || outbuf_len == NULL || outbuf == NULL)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    if ( ! is_ctb(inbuf[0]) || ! is_ctb_type(inbuf[0], CTB_COMPRESSED_TYPE))
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    count = 1;

    alg = inbuf[count];
    count++;

    *outbuf_len = 0;
    *outbuf = NULL;

    if (alg != ZIP2_ALGORITHM_BYTE ) {
	return(-1);
    }

    /* We guess that the data will double in size.  If it gets bigger, we
     * pay in realloc, if not, we pay in wasted memory.  A real lose-lose
     * situation :-)
     */
    bufsize = 2 * inbuf_len;
    p = malloc( bufsize );
    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    /* 0 and 1 is CTB and compression type */
    memset(&stream, 0, sizeof(stream));
    stream.next_in  = inbuf + 2;
    stream.avail_in = inbuf_len - 2;
    stream.next_out = p;
    stream.avail_out= bufsize;

    stream.zalloc   = (alloc_func)0;
    stream.zfree    = (free_func)0;
    stream.opaque   = NULL;

    /* -13 is undocumented.   Info received from Assar Westerlund
     * <assar@sics.se>, 11 Mar 1997.
     */
    i = inflateInit2(&stream, -13);
    if ( i != Z_OK ) {
	free(p);
	pgplib_errno = PGPLIB_ERROR_ZIP;
	return(-1);
    }

    /* Loop and aquire more memory if need be */

    while ( 1 ) {
	i = inflate( &stream, Z_PARTIAL_FLUSH);
	switch (i) {
	default :
	    pgplib_errno = PGPLIB_ERROR_IMPOSSIBLE;
	    free(p);
	    return(-1);
	case Z_OK:
	    break;
	case Z_STREAM_END:
	    i = inflateEnd(&stream);
	    if ( i != Z_OK ) {
		pgplib_errno = PGPLIB_ERROR_ZIP;
		free(p);
		return(-1);
	    }
	    *outbuf = p;
	    *outbuf_len = bufsize;
	    /* The normal return */
	    return(0);

	case Z_NEED_DICT:
	case Z_DATA_ERROR:
	case Z_BUF_ERROR:
	case Z_STREAM_ERROR:
	    pgplib_errno = PGPLIB_ERROR_ZIP;
	    free(p);
	    return(-1);
	case Z_MEM_ERROR:
	    free(p);
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}

	/* Each time we get here we are out of space.  We allocate the
	 * double of how much is left to decompress, rouned up to
	 * the nearest 1K.
	 */
	i = ((2 * (inbuf_len - 2 - stream.total_in) + 0x3FF) & ~0x3FF)
	    + 0x3ff;

	/* allocate at least 1kB. -tlr */

	stream.avail_out = i;
	bufsize += i;
	tmp = realloc(p, bufsize);
	if ( tmp == NULL ) {
	    free(p);
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	p = tmp;
	stream.next_out = p + stream.total_out;
    }


}

/*
 * Take a buffer, compress it, and create a compressed packet.
 */

int
compress_buffer(u_char	*inbuf,
		int	inbuflen,
		u_char	**outbuf,
		int	*outbuflen)
{

    u_char	*p;
    z_stream	stream;
    int		bufsize;
    /* scratch variables */
    int i;

    if ( inbuf == NULL || inbuflen == 0 || outbuf == NULL ||
	 outbuflen == NULL)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    *outbuf = NULL;
    *outbuflen = 0;

    /* The output migth in the worst case be as large as 0.1% larger
     * than the input, plus 12 bytes.  We add 1 (a total of 13) to
     * ensure that the casting rounds the right way.
     */
    bufsize = (int) (inbuflen * 1.01 + 13);
    p = malloc( bufsize );
    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    stream.next_in  = inbuf;
    stream.avail_in = inbuflen;
    p[0] = CTB_COMPRESSED;
    p[1] = ZIP2_ALGORITHM_BYTE;
    stream.next_out = p+2;
    stream.avail_out= bufsize-2;

    stream.zalloc   = (alloc_func)0;
    stream.zfree    = (free_func)0;
    stream.opaque   = NULL;

    /* I received the magic number -13 from
     * Assar Westerlund <assar@sics.se>, 11 Mar 1997
     */
    i = deflateInit2 (&stream, Z_DEFAULT_COMPRESSION,
		      Z_DEFLATED, -13, 8,
		      Z_DEFAULT_STRATEGY);
    if ( i != Z_OK ) {
	pgplib_errno = PGPLIB_ERROR_ZIP;
	free(p);
	return(-1);
    }
    i = deflate(&stream,  Z_FINISH);
    if ( i != Z_STREAM_END ) {
	free(p);
	return(-1);
    }
    /* The 2 are CTB and algorithm byte. */
    *outbuflen = stream.total_out + 2;

    i = deflateEnd(&stream);
    if ( i != Z_OK ) {
	pgplib_errno = PGPLIB_ERROR_ZIP;
	free(p);
	return(-1);
    }
    *outbuf = p;

    return(0);
}

/* Decompressing FILEs are really not in our ballpark, and we will
 * do it the stupid way.  Reading a buffer, decompressing,
 * writing.
 * If all is well, return 0, upon error, return -1.
 */
#define	OBUF	(2*BUFSIZ)
#define	IBUF	(BUFSIZ)

int
decompress_file(FILE 	*fin,
		FILE	*fout)
{
    int 	i;
    u_char	CTB, alg;
    u_char	inbuf[IBUF], outbuf[OBUF];
    z_stream	stream;

    if ( fin == NULL || fout == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    i  = fgetc(fin);
    if ( i == EOF ) {
	pgplib_errno = PGPLIB_ERROR_EOF;
        return(-1);
    }

    CTB = (u_char)i;
    if ( ! is_ctb_type(CTB,  CTB_COMPRESSED_TYPE) )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
        return(-1);
    }

    i  = fgetc(fin);
    if ( i == EOF ) {
	pgplib_errno = PGPLIB_ERROR_EOF;
        return(-1);
    }

    alg = (u_char) i;

    switch ( alg ) {
    case ZIP2_ALGORITHM_BYTE:
        break;
    default:
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
        return(-1);
    }

    memset(&stream, 0, sizeof(stream));
    stream.zalloc   = (alloc_func)0;
    stream.zfree    = (free_func)0;
    stream.opaque   = NULL;

    /* -13 is undocumented; received from Assar Westerlund
     * <assar@sics.se>, 11 Mar 1997
     */
    i = inflateInit2(&stream, -13);
    if ( i != Z_OK ) {
	pgplib_errno = PGPLIB_ERROR_ZIP;
	return(-1);
    }
    stream.next_out = outbuf;
    stream.avail_out = OBUF;
    /* Loop until decompressed.  We return from inside the loop */
    while ( 1 ) {
	i = fread(inbuf, 1, IBUF, fin);
	if ( i == EOF )
	    /* done = 1 */
	    break;
	stream.next_in  = inbuf;
	stream.avail_in = i;

	/* Loop until inbuf is empty */
	do {
	    i = inflate( &stream, Z_PARTIAL_FLUSH);
	    switch ( i )
	    case Z_STREAM_END: {
		/* Write out the rest */
		i = fwrite(outbuf, 1, OBUF - stream.avail_out, fout);
		if ( i < OBUF - stream.avail_out)
		    return(-1);
		i = inflateEnd(&stream);
		if ( i != Z_OK )
		    return(-1);
		memset(outbuf, 0, OBUF);
		memset(inbuf, 0, IBUF);

		return(0);

	    case Z_OK:
		if ( stream.avail_out == 0 ) {

		    /* More space for output is needed. */
		    i = fwrite(outbuf, 1, OBUF, fout);
		    if ( i < OBUF ) {
			return(-1);
		    }
		    stream.next_out = outbuf;
		    stream.avail_out = OBUF;
		    /* decompress some more */
		    continue;

		    /*NOTREACHED*/
		}
		if ( stream.avail_in == 0 )
		    break;

		/* If we have understood the manual we will never
		 * get here */
		return(-1);
	    default:
		/* Could very well be corrupt data */
		return(1);
	    }
	} while (stream.avail_in > 0 );

	/* We need more input */
    }
    /*NOTREACHED*/
    return(0);
}
