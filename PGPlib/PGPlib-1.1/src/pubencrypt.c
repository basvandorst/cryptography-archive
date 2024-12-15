/*********************************************************************
 * Filename:      pubencrypt.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 09:53:32 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/



/*
 * Take a buffer and a (pointer to) a public key, and produce
 * a buffer which contains a RSA encrypted DEK followed by the
 * original data encrypted (with IDEA) by the DEK.
 *
 * On error, return -1, else return 0;
 */



/* From PGFORMAT.DOC
 * ============================
 *
 * Conventional Data Encryption Key (DEK) "packet"
 * -----------------------------------------------
 *
 * The DEK has no CTB packet framing.  The DEK is stored packetless and
 * naked, with padding, encrypted inside the MPI in the RSA
 * public-key-encrypted packet.
 *
 * PGP versions 2.3 and later use a new format for encoding the message
 * digest into the MPI in the signature packet.  (This format is not
 * presently based on any RFCs due to the use of the IDEA encryption
 * system.)  This format is accepted but not written by version 2.2.
 * older format used by versions 2.2 and earlier is also accepted by
 * versions up to 2.4, but the RSAREF code in 2.5 is unable to cope
 * with it.
 *
 * PGP versions 2.3 and later encode the DEK into the MPI as follows:
 *
 *         MSB                     .   .   .                   LSB
 *
 *        0   2   RND(n bytes)   0   1   DEK(16 bytes)   CSUM(2 bytes)
 *
 * CSUM refers to a 16-bit checksum appended to the high end of the DEK.
 * RND is a string of NONZERO pseudorandom bytes, enough to make the length
 * of this whole string equal to the number of bytes in the modulus.
 *
 * For both versions, the 16-bit checksum is computed on the rest of the
 * bytes in the DEK key material, and does not include any other material
 * in the calculation.  In the above MSB-first representation, the
 * checksum is also stored MSB-first.  The checksum is there to help us
 * determine if we used the right RSA secret key for decryption.
 *
 * (end PGFORMAT.DOC)
 *
 * The 1 in the DEK format (to the left of the DEK proper) is the constant
 * IDEA_ALGORITHM_BYTE.
 * =====================
 */


#include <pgplib.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

/* SSLeay */
#include <rsa.h>

/* The DEK is encoded as IDEA_ALGORITHM_BYTE followed by the DEK
 * followed by two bytes of checksum.
 */

#define DEKLEN	(1+16+2)

int
pub_encrypt_buffer(PGPpubkey_t  *key,
		   u_char	*inbuf,
		   int		inbuflen,
		   int		do_compress,
		   u_char	digest[16],
		   int		*outbuflen,
		   u_char 	**outbuf)
{
    u_char 	*enc_packet, *dekptr;
    int		buflen = 0;
    int		keylen;
    int		header_len;
    u_char 	*tmp;
    int		offset;
    u_char	header[5];
    RSA		*rsa_info;
    u_char	dek[DEKLEN];
    u_short	cksum, us;
    /* scratch */
    int		i,j;
    PGPKEYID	key_id;

    if (key == NULL ||
	digest == NULL || outbuflen == 0 || outbuf == NULL)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    /* This will help to ensure that you don't shoot yourself in the
     * foot.  After all, if you choose your keys properly, this assert
     * will strike you only once in every 18.446.744.073.709.551.616 runs.
     */
    for(j=i=0; i< 16; i++)
	if ( digest[i] == 0 )
	    j++;

    if(inbuf && inbuflen) {
	i = conventional_key_encrypt(inbuf,	inbuflen, digest, do_compress,
				     &buflen, &enc_packet);
	if ( i != 0 ) {
	    return(-1);
	}
    }

    cksum = checksum(digest, 16);
    us = htons(cksum);
    /* The DEK is encoded as 1 followed by the DEK followed by two bytes
     * checksum.  Make room for the 1.
     */
    dek[0] = 1;
    memcpy(dek+1, digest, 16);
    memcpy(dek+1+16, &us, 2);

    /* Then, encrypt */
    rsa_info = RSA_new();
    if ( rsa_info == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return -1;
    }
    rsa_info->n = BN_dup( key->N);
    rsa_info->e = BN_dup( key->E);

    /* The 2 is the MPI prefix used by PGP */
    keylen = BN_num_bytes(key->N) + 2 ;
    dekptr = malloc(keylen);
    if ( dekptr == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    /* we offset the result by two in order to make room for the length
     * of the MPI, which is prepended.
     */
    i = RSA_public_encrypt(19, dek, dekptr+2, rsa_info, RSA_PKCS1_PADDING);
    memset(dek, 0, 18);
    if ( i == -1 ) {
	RSA_free(rsa_info);
	free(dekptr);
	free(enc_packet);
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    /* If this doesn't hold, then SSLeay has changed.  You _really_ want
     * to known about it, I guess.
     */
    assert(i == keylen-2);

    RSA_free(rsa_info);

    /* MPI prefix */
    us = htons( (u_short)BN_num_bits(key->N));
    memcpy(dekptr, &us, 2);

    /* Then, make a PGP packet from the entire thing, the 1's are
     * version and algorithm byte, respectively.
     */
    header_len = make_ctb(CTB_PKE_TYPE, 1+KEYFRAGSIZE+1+keylen, 0, header);
    if ( header_len == 0 )
	return(-1);

    /* Allocate space for header and two packets */
    tmp = malloc(header_len + 1+KEYFRAGSIZE+1+keylen + buflen);
    if ( tmp == NULL ) {
	free(enc_packet);
	free(dekptr);
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    /* remember it */
    *outbuf = tmp;
    offset = 0;

    memcpy(tmp, header, header_len);
    offset += header_len;
    tmp[offset] = VERSION_BYTE_NEW;
    offset += 1;

    PGPKEYID_FromBN(&key_id, key->N);
    PGPKEYID_ToBytes(&key_id, tmp+offset);
    offset += PGPKEYID_SIZE;

    tmp[offset] = RSA_ALGORITHM_BYTE;
    offset += 1;
    /* And the rest of this packet*/
    memcpy(tmp+offset, dekptr, keylen);
    free(dekptr);
    offset += keylen;
    /* and the encrypte packet */
    if(inbuflen && inbuf)
    {
	memcpy(tmp+offset, enc_packet, buflen);
	free(enc_packet);
    }

    *outbuflen = offset + buflen;
    return(0);
}

/*
 * As suggested by Andy Dustman <andy@CCMSD.chem.uga.edu>, this routine
 * will simply pub-encrypt a session key, and return the DEK as a
 * buffer.
 *
 * IF everything is OK, return 0, else return -1.
 */

int
pub_encrypt_session_key(PGPpubkey_t	*key,
                        u_char		*skey,
			int		*outbuflen,
			u_char		**outbuf)

{
    /* scratch */
    int	i;

    if ( key == NULL || skey == NULL || outbuf == NULL || outbuflen == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return (-1);
    }

    i = pub_encrypt_buffer(key, NULL, 0, 0, skey, outbuflen, outbuf);
    return(i);
}
