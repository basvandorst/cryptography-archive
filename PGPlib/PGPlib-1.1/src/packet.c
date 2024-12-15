/*********************************************************************
 * Filename:      packet.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:42:30 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/



#include <pgplib.h>

#include <stdlib.h>
#include <netinet/in.h>


/* We try to read in a packet from a file.  Some special handling is
 * required.  Returns are:
 *	 0: OK.
 *	-1: No good PGP packet, or CTB_COMPRESSED_TYPE, or some error.
 * 	    The file is advanced to where the error occurred (more or less).
 *
 * This code is very similar to packet_length; can't be helped.
 */

int
read_packet(FILE	*fd,
	    u_char	**packet)
{
    u_char	*buf;
    int		howfar;
    fpos_t	pos;
    int		ll;
    int		i;
    int		p_len;
    u_long	ul;
    u_short	us;
    u_char	*p;

    if(fd == NULL || packet == NULL)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    *packet = NULL;
    fgetpos(fd, &pos);

    buf = malloc(1);
    if ( buf == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    howfar = 0;

    i = fread(buf, 1, 1, fd);
    if ( i != 1 ) {
	free(buf);
	if ( feof(fd) == 0 ) {
	    pgplib_errno = PGPLIB_ERROR_EINVAL;
	    return(-1);
	}
	else {
	    pgplib_errno = PGPLIB_ERROR_EOF;
	    return(1);
	}
    }
    if ( ! is_ctb(buf[0]) || is_ctb_type(buf[0], CTB_COMPRESSED_TYPE) ) {
	ungetc(*buf, fd);
	free(buf);
	pgplib_errno = is_ctb(buf[0]) ? PGPLIB_ERROR_COMPRESSED_CTB
	    : PGPLIB_ERROR_BAD_CTB;
	return(1);
    }

    howfar += 1;

    ll = ctb_llength(buf[0]);
    p = realloc(buf, howfar+ll);
    if ( p == NULL ) {
	free(buf);
	fsetpos(fd, &pos);
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    buf = p;
    p_len = 0;

    switch (ll) {
    case 1:
	p_len = 0;
	i = fread( buf+howfar, 1, 1, fd);
	if ( i != 1 ) {
	    int rn = i;
	    free(buf);
	    fsetpos(fd, &pos);
	    if ( feof(fd) == 0 )
	    {
		errno = rn;
		pgplib_errno = PGPLIB_ERROR_SYSERR;
		return(-1);
	    }
	    else
	    {
		errno = rn;
		pgplib_errno = PGPLIB_ERROR_EOF;
		return(1);
	    }
	}
	p_len = (int)*(buf+howfar);
	howfar += 1;
	break;
    case 2:
	i = fread(&us, 2, 1, fd);
	if ( i != 1 ) {
	    int rn = errno;
	    fsetpos(fd, &pos);
	    free(buf);
	    if ( feof(fd) == 0 )
	    {
		errno = rn;
		pgplib_errno = PGPLIB_ERROR_SYSERR;
		return(-1);
	    }
	    else
	    {
		errno = rn;
		pgplib_errno = PGPLIB_ERROR_EOF;
		return(1);
	    }
	}
	p_len = (int)ntohs(us);
	memcpy(buf+howfar, &us, 2);
	howfar += 2;
	break;
    case 4:
	i = fread(&ul, 4, 1, fd);
	if ( i != 1 ) {
	    int rn = errno;
	    fsetpos(fd, &pos);
	    free(buf);
	    if ( feof(fd) == 0 )
	    {
		errno = rn;
		pgplib_errno = PGPLIB_ERROR_SYSERR;
		return(-1);
	    }
	    else
	    {
		errno = rn;
		pgplib_errno = PGPLIB_ERROR_EOF;
		return(1);
	    }
	}
	p_len = (int)ntohl(ul);
	memcpy(buf+howfar, &ul, 4);
	howfar += 4;
	break;
    default:
	fsetpos(fd, &pos);
	free(buf);
	pgplib_errno = PGPLIB_ERROR_IMPOSSIBLE;
	return -1;
	break;
    }

    if ( p_len == 0 ) {
	pgplib_errno = PGPLIB_ERROR_BAD_PACKET_LENGTH;
	return(-1);
    }

    p = realloc(buf, howfar + p_len);
    if ( p == NULL ) {
	free(buf);
	fsetpos(fd, &pos);
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    buf = p;

    i = fread(buf+howfar, p_len, 1, fd);
    if ( i != 1 ) {
	fsetpos(fd, &pos);
	free(buf);
	if ( feof(fd) == 0 ) {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	else {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
    }

    *packet = buf;
    return(0);
}


/* We try to copy a packet from a buffer.  Some special handling is
 * required.  Returns are:
 *	 0: OK.
 * 	 1: No PGP packet _or_ CTB_COMPRESSED_TYPE.
 *	-1: Some error.
 *
 * How buffer we used can be found in packet->length (after casting to
 * the correct type).
 */


int
find_packet_in_buffer(u_char	*buffer,
		      int	buf_len,
		      u_char	**packet)
{

    int	p_len, ll;

    while(1)
    {
	if ( buffer == NULL || buf_len < 3 ) {
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	
	/* skip PGP 5.0 packets. */

	if((buffer[0] & 0xc0) == 0xc0)
	{
	    ll = packet_length(buffer);
	    buffer += ll;
	    buf_len -= ll;
	} else break;
    }
	    

    if ( ! is_ctb(buffer[0]) || is_ctb_type(buffer[0], CTB_COMPRESSED_TYPE))
    {
	pgplib_errno = is_ctb(buffer[0]) ? PGPLIB_ERROR_COMPRESSED_CTB
	    : PGPLIB_ERROR_BAD_CTB;
	return (1);
    }

    ll = ctb_llength(buffer[0]);
    if ( ll + 1 >= buf_len) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }

    p_len = packet_length(buffer);
    if ( p_len == -1 )
	return (-1);
    if ( p_len > buf_len ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }

    *packet = calloc(1, p_len);
    if ( *packet == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    memcpy(*packet, buffer, p_len);
    return(0);
}

/*
 * Find the real packetlength of a PGP packet.  p must point to the CTB.
 * We return the length of the _entire_ packet.
 * We access p and 1, 2 or 4 more bytes depending of the length; don't
 * use this to find out how large a packet will become based on the
 * CTB !
 *
 * PGP-5 aware. ;)
 *
 */
int
packet_length(u_char *p)
{
    int 	i, ret;
    u_short 	us;
    u_long	ul;

    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    switch(*p & 0xc0)
    {
    case 0x80:	/* PGP 2 */
	i = ctb_llength(*p);
	p++;
	switch (i) {
	case 1:
	    ret = 1 + i + (int)*p;
	    break;
	case 2:
	    memcpy(&us, p, 2);
	    ret = 1 + i + (int) ntohs(us);;
	    break;
	case 4:
	    memcpy(&ul, p, 4);
	    ret =  1 + i + (int) ntohl(ul);
	    break;
	default:
	    /* This isn't a PGP packet */
	    pgplib_errno = PGPLIB_ERROR_EINVAL;
	    return(-1);
	}
	break;
    case 0xc0:	/* PGP 5 */
	if(p[1] < 192)
	    return p[1] + 2;
	else if(p[1] < 224)
	    return (p[1] - 192) * 256 + p[2] + 192 + 3;
	else 
	    return (1 << (p[1] &0x1f)) + 2;
	break;
    default:
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    return(ret);
}

