/*********************************************************************
 * Filename:      misc.c
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 09:14:56 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/


#include <pgplib.h>

#include <ctype.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

/* SSLeay */
#include <bn.h>

/*
 * Added to satisfy Linux.
 */
#ifndef NBBY
#  define NBBY 8
#endif

/*
 * The length of the length of a PGP packet depends on the length
 * of the data, and the type.  Take a type, a length, and a "force"
 * (which forces the use of 4 bytes length) and a buffer (at least 5
 * bytes long) and write a CTB and the length (in Network Byte Order)
 * into the buffer.
 * Return the number of bytes written into the buffer.
 * The idea was taken from write_ctb_len in PGP/crypto.c
 *
 * Return -1 on error;
 */
int
make_ctb(u_char ctb_type, u_long length, int force, u_char *buf)
{
    int		llength, llenb, ret = 0;
    u_short	us;
    u_long	ul;

    if ( buf == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    if (force || (length > 0xffffL)) {
	llength = 4;
	llenb = 2;
    }
    else if ( (u_short)length > 0xff) {
	llength = 2;
	llenb = 1;
    }
    else {
	llength = 1;
	llenb = 0;
    }

    buf[0] = CTB_BYTE(ctb_type, llenb);
    /* convert length to external byteorder */
    switch ( llength) {
    case 1:
	buf[1] = (u_char)length;
	ret = 2;
	break;
    case 2:
	us = htons( (u_short)length);
	memcpy(buf+1, &us, 2);
	ret = 3;
	break;
    case 4:
	ul = htonl(length);
	memcpy(buf+1, &ul, 4);
	ret = 5;
	break;
    default:
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	ret = -1;
    }
    return(ret);
}


/*
 * Copy at most length-1 bytes from buffer into line, stopping after
 * the first newline has been found or buflen bytes has been
 * copied.  In any case, a NULL is appended to the string.
 * If some parameter is unreasonable, 0 is returned, if not, the
 * number of bytes consumed from buf is returned.
 * If a one-byte line is passed in, 0 will obviously be returned.
 */

int
bufgets(u_char	*line,
	int 	linelen,
	u_char 	*buf,
	int 	buflen)
{
    int		i;

    if ( line == NULL || linelen < 1 || buf == NULL || buflen < 1)
	return(0);

    i = 0;
    while ( i < linelen && i < buflen) {
	line[i] = buf[i];
	if ( line[i] == '\n' )
	    if ( i+1 == linelen ) {
		line[i] = 0;
		return(i);
	    }
	    else {
		line[i+1] = 0;
		return(i+1);
	    }
	i += 1;
	continue;
    }
    if ( i == linelen ) {
	line[i-1] = 0;
	/* The last char was overwritten, not consumed */
	return(i-1);
    }
    if ( i == buflen ) {
	if ( i == linelen ) {
	    line[i-1] = 0;
	    return(i-1);
	}
	else {
	    line[i] = 0;
	    return(i);
	}
    }
    if ( i == linelen ) {
	line[i-1] = 0;
	return(i-1);
    }
    return(0);
}


/* PGPKEYID_FromBN - convert a BN into a PGPKEYID simply looking at the
 *		     bottom 64 bits of the BN (unlike BN_get_quad)
 */
int 
PGPKEYID_FromBN(PGPKEYID *key_id,BIGNUM *a)
{
    PGPKEYID ret;

    /* detect zero specially */
    if ( BN_is_zero(a)) {
      	PGPKEYID_ZERO(ret);
	goto done;
    }

    /* shortcut little BNs */
    if ( a->top == 1 ) {
#ifdef USE_LONGLONG
	ret = a->d[0];
#else
	(*(unsigned long long *)&ret) = a->d[0];
#endif
	goto done;
    }

    /* XXX Byte order ? */
#ifdef USE_LONGLONG
    ret = a->d[0];
    ret |= ( (PGPKEYID) a->d[1] << 32);
#else
    (*(unsigned long long *)&ret) = a->d[0];
    (*(unsigned long long *)&ret) |= ( (unsigned long long) a->d[1] << 32);
#endif

done: ;
    memcpy(key_id,&ret,sizeof(PGPKEYID));
    return(1);
}

int 
PGPKEYID_ExtractShort(PGPKEYID 	*key_id,
		      unsigned 	long *short_id)
{
#ifdef USE_LONGLONG
  (*short_id)=(*key_id) & 0xffffffff;
#else
  (*short_id)=(*(unsigned long long *)key_id) & 0xffffffff;
#endif
}

/*
 * Take 8 bytes (in network order) and make a quad
 * was to_quad
 */
int 
PGPKEYID_FromBytes(PGPKEYID *key_id, u_char *buf)
{
    int 	i;
    PGPKEYID	q;

    PGPKEYID_ZERO(q);
    for(i=0; i < 8; i++) {
#ifdef USE_LONGLONG
	q |= ((PGPKEYID) buf[i] << 8*(7-i));
#else
	(*(unsigned long long *)&q) |= ((unsigned long long) buf[i] << 8*(7-i));
#endif
    }

    memcpy(key_id,&q,sizeof(PGPKEYID));
    return(1);
}

/*
 * Copy a quad into a buffer, in network order.
 * was copy_quad
 */
int
PGPKEYID_ToBytes(PGPKEYID 	*q, 
		 u_char        	*buf)
{
    int	i,j;

    for(j=0, i=7; i>=0; i--, j++) {
#ifdef USE_LONGLONG
        buf[j] =  (u_char) ((*q) >> 8*i) & 0xff;
#else
        buf[j] =  (u_char) ((*(unsigned long long *)q) >> 8*i) & 0xff;
#endif
    }
    return(1);
}

/*
 * This is how PGP generates "checksum" of a buffer.  It is only used
 * to add two bytes to a DEK in order to be able to verify that the
 * correct key  has been used.  I've read the sources for PGP to find
 * this out, in particular the file mpiio.c
 */
u_short
checksum(u_char	*buf,
	 int 	count)
{
    u_short cksum;
    int	    i;

    for(i = 0, cksum = 0; count > 0; i++, count--)
	cksum += buf[i];

    return (cksum);
}

/*
 * A buffer, containing a PGP MPI, is converted into a SSLeay
 * BIGNUM. If everything is OK, we return 0.  On error, we return -1.
 */
int
mpi2bignum(u_char	*buf,
	   int		buflen,
	   int		*used,
	   BIGNUM	**N)
{
    u_short	us;
    int		length, numbit;
    int		sofar;
    /* scratch */
    u_char	*p;

    if ( buf == NULL || buflen < 2 || used == NULL || N == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(0);
    }
    sofar = 0;

    /* Length of N */
    memcpy( &us, buf, 2);
    numbit = (int)ntohs(us);
    length = numbit / NBBY;
    if ( numbit % NBBY  != 0 )
	length += 1;
    p = calloc( length, 1);
    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    sofar += 2;

    if ( buflen - (sofar + length) < 0) {
	free(p);
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    /* Read N */
    memcpy( p, buf+sofar, length);
    sofar += length;
    /* Turn N into a BIGNUM */
    *N = BN_bin2bn(p, length, NULL);
    memset(p, 0, length);
    free(p);
    if (*N == NULL) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    *used = sofar;
    return(0);
}


/*
 * Used to be this macro.
 *   "length of length" field of packet, in bytes (1, 2, 4, 8 bytes):
 *   #define ctb_llength(ctb) ((int) 1 << (int) ((ctb) & CTB_LLEN_MASK))
 * Routine written by Thomas Roessler <roessler@guug.de>
 */

int 
ctb_llength(int ctb)
{
    int n;

    n = ctb & CTB_LLEN_MASK;

    if( ! is_ctb(ctb)) {
	pgplib_errno = PGPLIB_ERROR_BAD_CTB;
	return -1;
    }

    if(n > 2) {
	pgplib_errno = PGPLIB_ERROR_BAD_CTB_LENGTH;
	return -1;
    }

    return 1 << n;
}

int 
PGPKEYID_FromAscii(char 	*str,
		   PGPKEYID	*key_id)
{
#ifdef USE_LONGLONG
  PGPKEYID tmp;

  tmp = strtouq(str, NULL, 0);
  (*key_id) = tmp;
  return(1);
#else
  unsigned long long tmp;

  tmp = strtouq(str, NULL, 0);
  (*(unsigned long long *)key_id) = tmp;
  return(1);
#endif
}

int 
PGPKEYID_ToAscii(PGPKEYID	*key_id,
		 char 		*buf,
		 int 		maxsize)
{
#ifdef USE_LONGLONG
    sprintf(buf,"%#.16" PRIx64,(*key_id));
#else
    sprintf(buf,"%#.16" PRIx64,(*(unsigned long long *)key_id));
#endif
}

char 
*PGPKEYID_ToStr(PGPKEYID *key_id)
{
    static char buf[128];

    if (PGPKEYID_ToAscii(key_id,buf,sizeof(buf)))
	return buf;
    else
	return "";
}


/* ------------------------------------------------------------------------ */
/* stuff that is not used but might be useful to still have handy for later */
/* ------------------------------------------------------------------------ */

#ifdef NOT_USED

/* BN_get_quad - convert a BN into a PGPKEYID and complain if the
 * 		 BN is too large to fit
 */
int 
BN_get_quad(BIGNUM *a,PGPKEYID *key_id)
{
    int i;
    PGPKEYID ret;

    /* detect zero specially */
    if ( BN_is_zero(a)) {
      	PGPKEYID_ZERO(ret);
	goto done;
    }

    i = BN_num_bytes(a);
    if ( i > sizeof(PGPKEYID)) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	/*
	return ( ~0 );
	*/
	return (0);
    }

    if ( a->top == 1 ) {
#ifdef USE_LONGLONG
	ret = a->d[0];
#else
	(*(unsigned long long *)&ret) = a->d[0];
#endif
	goto done;
    }

    /* XXX Byte order ? */
#ifdef USE_LONGLONG
    ret = a->d[0];
    ret |= ( (PGPKEYID) a->d[1] << 32);
#else
    (*(unsigned long long *)&ret) = a->d[0];
    (*(unsigned long long *)&ret) |= ( (unsigned long long) a->d[1] << 32);
#endif

done: ;
    memcpy(key_id,&ret,sizeof(PGPKEYID));
    return(1);
}

/* I thought that this might be needed ... but I was wrong --tjh */
int BN_get_shortkeyid(BIGNUM *a,unsigned long *short_id)
{
    unsigned long ret;

    /* detect zero specially */
    if ( BN_is_zero(a)) {
      	ret=0;
	goto done;
    }

    /* XXX Byte order ? */
    ret = a->d[0];

done: ;
    (*short_id)=ret;
    return(1);
}

#endif /* NOT_USED */
