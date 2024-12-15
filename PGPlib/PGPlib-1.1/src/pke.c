/*********************************************************************
 * Filename:      pke.c
 * Description:
 * Author:        Thomas Roessler <roessler@guug.de>
 * Modified at:   Mon Jan 19 09:50:42 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) Thomas Roessler
 * 	See the file COPYING for details
 ********************************************************************/


#include <pgplib.h>

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

/* SSLeay */
#include <rsa.h>

/*
 * A Public Key Encrypted "package" consists of two packets.  First
 * the PKE, which is an IDEA key encrypted with RSA wrapped up
 * in a stand-alone package, follwed by a conventionally encrypted
 * package.  Read in the first package (the PKE).
 * IF all is OK, return 0, else return -1.
 */

int
buf2pke(u_char 	*inbuf,
	int 	inbuflen,
	PGPpke_t *pke)
{
    int count;
    /* scratch */
    int i, j;

    /* A PKE must be at least 13 bytes long */
    if (inbuf == NULL || pke == NULL) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }
    if ( inbuflen < 13 ) {
	pgplib_errno = PGPLIB_ERROR_BAD_PACKET_LENGTH;
	return -1;
    }

    if( !is_ctb(inbuf[0]) || !is_ctb_type(inbuf[0], CTB_PKE_TYPE)) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    pke->length = packet_length(inbuf);
    if(pke->length > inbuflen || pke->length < 13 )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return 1;
    }

    count = ctb_llength(inbuf[0]) + 1;
    pke->version = inbuf[count];
    count += 1;
    PGPKEYID_FromBytes(&pke->key_id, inbuf + count);
    count += 8;
    pke->algorithm = inbuf[count];
    count += 1;

    i = mpi2bignum(inbuf + count, inbuflen - count, &j, &pke->E);
    if ( i != 0 ) {
	return -1;
    }
    /* count += j */
    return 0;
}


void
free_pke(PGPpke_t *pke)
{
    if(pke == NULL)
	return;

    if(pke->E != NULL)
	BN_free(pke->E);
    memset(pke, 0, sizeof(PGPpke_t));
}
