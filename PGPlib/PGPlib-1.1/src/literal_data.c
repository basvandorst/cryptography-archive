/*********************************************************************
 * Filename:      literal_data.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:42:31 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/



#include <pgplib.h>

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>


/* Recall that free(NULL) is OK */
void
free_literal(PGPliteral_t *literal)
{
    if (literal == NULL )
	return;
    free(literal->filename);
    free(literal->data);
    memset(literal, 0, sizeof(PGPliteral_t));
}

/* We have a  file which contains a literal packet.  Read in
 * the info and fill out the struct, ut leave the pointer as NULL.
 * The FILE is advanced so that data can be read.  On error return -1,
 * on OK return 0, on syntax error return 1.
 */

int
file2literal(FILE		*fin,
	     PGPliteral_t	*literal)
{
    int		offset, flen;
    u_long	ul, ll;
    u_char	len[5];
    /* scratch */
    int 	i;


    if ( fin == NULL || literal == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    pgplib_errno = PGPLIB_ERROR_MAX;

    i = fgetc(fin);
    if ( i == -1 )
	goto bad;
    /* CTB */
    len[0] = (u_char) i;
    ll = ctb_llength(len[0]);
    /* XXX Shouldn't be hard-coded */
    if ( ll != 4 )
	goto bad;

    i = fread(len+1, 1, ll, fin);
    if ( i != ll )
	goto bad;

    literal->length = packet_length(len);
    i = fgetc(fin);
    if ( i == EOF ) {
	pgplib_errno = PGPLIB_ERROR_EOF;
	goto bad;
    }
    literal->mode = (u_char) i;

    switch ( literal->mode ) {
    case MODE_BINARY:
    case MODE_TEXT:
    case MODE_LOCAL:
	break;
    default:
	goto bad;
    }

    /* Original filename */
    flen = fgetc(fin);
    if ( flen == EOF ) {
	pgplib_errno = PGPLIB_ERROR_EOF;
	goto bad;
    }
    offset = 6 + flen;
    literal->filename = calloc(flen+1, 1);
    if ( literal->filename == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    i = fread(literal->filename, flen, 1, fin);
    if ( i != 1 ) {
	pgplib_errno = i == -1 ? PGPLIB_ERROR_SYSERR : PGPLIB_ERROR_EOF;
	goto bad;
    }
    i = fread(&ul, 4, 1, fin);
    if ( i != 1 ) {
	pgplib_errno = i == -1 ? PGPLIB_ERROR_SYSERR : PGPLIB_ERROR_EOF;
	goto bad;
    }
    offset += 4;
    literal->timestamp = ntohl(ul);

    /* Length of data is always 5 less than total */
    literal->datalen = literal->length - 5;

    return(0);

bad:
    if ( pgplib_errno == PGPLIB_ERROR_MAX )
	pgplib_errno = PGPLIB_ERROR_EINVAL;
    free_literal(literal);
    return(1);
}


/* **************************************************************
 * A buffer contains a literal packet.  Parse and build structure.
 * ************************************************************** */

int
buf2literal(u_char	*inbuf,
	    int  	inbuflen,
	    PGPliteral_t *literal)
{
    int		count;
    char     	*tmp;
    /* scratch */
    int 	i;
    u_long	ul;

    if ( inbuf == NULL || inbuflen < 10 || literal == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    if ( ! is_ctb(inbuf[0]) || ! is_ctb_type(inbuf[0], CTB_LITERAL2_TYPE))
    {
	pgplib_errno = PGPLIB_ERROR_BAD_CTB;
	return(1);
    }

    memset(literal, 0, sizeof(PGPliteral_t));
    literal->length = packet_length(inbuf);
    if ( literal->length > inbuflen) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }

    /* Skip CTB and length of length */
    count = 1 + ctb_llength(inbuf[0]);

    literal->mode = inbuf[count];
    count++;

    switch ( literal->mode ) {
    case MODE_BINARY:
    case MODE_TEXT:
    case MODE_LOCAL:
	break;
    default:
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    /* Original filename */
    i = inbuf[count];
    count++;
    tmp = calloc(1, i+1);
    if ( tmp == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    literal->filename = tmp;
    strncpy(literal->filename, inbuf+count, i);
    count += i;

    /* Timestamp */
    /* XXX Validity check ? */
    memcpy(&ul, inbuf+count, 4);
    literal->timestamp = ntohl(ul);
    count += 4;

    tmp = calloc(1, literal->length - count);
    if ( tmp == NULL ) {
	free(literal->filename);
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    literal->data = (u_char *)tmp;
    memcpy(literal->data, inbuf+count, literal->length - count);
    literal->datalen = literal->length-count;

    return(0);
}

/*
 * Place a literal struct into a buffer.  If all is well, return 0,
 * else, return -1.
 */

int
literal2buf(PGPliteral_t	*literal,
	       u_char		**buf)
{
    u_char	header[5];
    int		header_len;
    int		plen;
    int		namelen;
    u_char	*tmp;

    if ( literal == NULL || buf == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    tmp = malloc(literal->length);
    if ( tmp == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    namelen = strlen(literal->filename);
    if ( namelen >= 255 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	free(tmp);
	return(-1);
    }
    /* mode + (lengh of name + namelength) + timestamp + data */
    plen = 1+(1+namelen)+4+literal->datalen;
    /* CTB + length */
    header_len = make_ctb(CTB_LITERAL2_TYPE, plen, 0, header);
    if ( header_len == -1 ) {
	free(tmp);
	return(-1);
    }

    /* XXX remove */
    assert(is_ctb(header[0]));
    assert(is_ctb_type(header[0], CTB_LITERAL2_TYPE));
    assert(header_len <= 5 );

    /* CTB plus length */
    memcpy(tmp, header, header_len);
    /* mode */
    tmp[header_len] = 'b';
    /* length of the name */
    tmp[header_len+1] =  namelen;
    /* the name proper */
    memcpy(tmp+header_len+2, literal->filename, namelen);
    /* timestamp */
    /* XXX shouldn't be 0 */
    memset(tmp+header_len+2+namelen, 0, 4);
    /* data */
    memcpy(tmp+header_len+2+namelen+4, literal->data, literal->datalen);

    /* XXX remove after a while */
    assert(literal->length == plen + header_len);

    *buf = tmp;

    return(0);
}

/*
 * Take a file name, create a literal struct.  If all is OK, return
 * 0, else -1.
 */

int
create_literal(char		*filename,
	       PGPliteral_t	*literal)
{
    u_char	header[5];
    int		header_len;
    int 	fd;
    int		plen;
    struct stat sbuf;
    /* scratch */
    int      	i;

    if ( filename == NULL || literal == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    pgplib_errno = PGPLIB_ERROR_SYSERR;

    i = stat(filename, &sbuf);
    if ( i < 0 ) {
	return(-1);
    }
    literal->data = malloc(sbuf.st_size);
    if ( literal->data == NULL ) {
	return(-1);
    }
    fd = open(filename,  O_RDONLY);
    if ( fd < 0 ) {
	free(literal->data);
	return(-1);
    }
    i = read(fd, literal->data, sbuf.st_size);
    if ( i != sbuf.st_size) {
	free(literal->data);
	return(-1);
    }
    close(fd);

    /* 'b' is default on UNIX systems, or what ? */
    literal->mode = 'b';
    literal->timestamp = 0;
    literal->filename = strdup(filename);
    literal->datalen = sbuf.st_size;
    /* mode + (length of name + name) + time + data */
    plen =  1+(1+strlen(filename))+4+sbuf.st_size;
    /* CTB + length */
    header_len = make_ctb(CTB_LITERAL2_TYPE, plen, 0, header);
    if ( header_len == -1 )
	return(-1);
    literal->length = header_len + plen;

    return(0);
}
