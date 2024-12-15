/*********************************************************************
 * Filename:      trust.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:42:28 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <pgplib.h>

#include <stdlib.h>
#include <strings.h>

int
buf2trust(u_char *packet, int p_len, PGPtrust_t *trust)
{
    int	len;

    if ( packet == NULL || p_len < 1 || trust == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    len = packet_length(packet);
    if ( len != 3 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }

    trust->byte = packet[2];

    return(0);
}

int
trust2buf(PGPtrust_t	*trust,
	  u_char	**buf)
{
    u_char *p;

    if ( trust == NULL || buf == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    p = malloc(3);
    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    p[0] = CTB_KEYCTRL;
    p[1] = 1;
    p[2] = trust->byte;

    *buf = p;

    return(0);
}

/* purge trust packets - needed to export a key.  buf is modified
 * in place. */

int 
purge_trust(u_char *buffer, long bufl, long *fbufl)
{
    long how_far;
    long p_len;

    if(!buffer || !bufl || !fbufl)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }
    
    for(how_far = 0; how_far < bufl;)
    {
	
	if(!is_ctb(buffer[how_far]) || is_ctb_type(buffer[how_far],
						   CTB_COMPRESSED_TYPE))
	{
	    pgplib_errno = is_ctb(buffer[how_far]) ?
		PGPLIB_ERROR_COMPRESSED_CTB
		: PGPLIB_ERROR_BAD_CTB;
	    return -1;
	}

	if((p_len = packet_length(buffer + how_far)) == -1)
	    return -1;

	if(is_ctb_type(buffer[how_far], CTB_KEYCTRL_TYPE))
	{
	    bufl -= p_len;
	    memmove(buffer + how_far, buffer + how_far + p_len, bufl);
	} else
	    how_far += p_len;

    }

    *fbufl = bufl;
    return 0;
}
