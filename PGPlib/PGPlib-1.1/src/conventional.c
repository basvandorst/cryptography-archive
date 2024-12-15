/*********************************************************************
 * Filename:      conventional.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:42:33 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/


#include <pgplib.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include <idea.h>
#include <rand.h>


/* This constant has been taken from PGP src/crypto.c */
#define	RAND_PREFIX_LENGTH	8

/*
 * Decrypt a conventional encrypted package, given the MD5
 * of the password.  Done in a PGP-compatible way.  We can not
 * assume the resulting data is a package, thus the lenght.
 * OK : return 0
 * Not a packet : return 1
 * Wrong password: return 2
 * Error: return -1
 */

int
conventional_key_decrypt(u_char	*inbuf,
			 u_char	digest[16],
			 int	*outlen,
			 u_char	**outbuf)
{
    IDEA_KEY_SCHEDULE 	ks;
    int 		num;
    int			inputlen;
    u_char		*tmp, header[10];
    /* variables used by IDEA, to mimic the PGP behaviour */
    u_char 		iv[8];

    if ( inbuf == NULL || digest == NULL || outlen == NULL || outbuf == NULL) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    if ( ((inbuf[0] & CTB_TYPE_MASK) >> 2) != CTB_CKE_TYPE ) {
	pgplib_errno = PGPLIB_ERROR_BAD_CTB;
	return(1);
    }

    /* So that we can type 10 rather than RAN...+2 */
    assert(RAND_PREFIX_LENGTH == 8 );

    inputlen = packet_length(inbuf) - 5;
    /* This get's us beyond the header */
    inbuf += 5;

    if ( inputlen > MAX_ENC_PACKET ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }

    /* 8+2=10 bytes of header for iv */
    tmp = malloc(inputlen-10);
    if ( tmp == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    /* Let us get ready to IDEA-decrypt the data */
    idea_set_encrypt_key(digest,&ks);

    /* We are ready to decrypt the data.  As described in PGFORMAT.DOC
     * and in PGP/src/idea.c.  We decrypt the first 10 bytes to verify
     * that the key we have is correct.
     */
    memset(iv, 0, 8);
    num = 0;

    idea_cfb64_encrypt(inbuf, header, RAND_PREFIX_LENGTH +2, &ks, iv,
		       &num, IDEA_DECRYPT);

    /* We decrypt 10 bytes.  If this isn't 8+2, we are lost.  Thus
     * the assert
     */
    assert ( num == 2 );

    /* Verify that it has been decrypted correctly */
    if ( header[6] != header[8] || header[7] != header[9] ) {
	/* No it didn't */
	free(tmp);
	*outbuf = NULL;
	pgplib_errno = PGPLIB_ERROR_WRONG_PASS;
	return(2);
    }

    /* The key was correct.  We have used 10 bytes of iv so far
     * (that is 8+2) and Zimmerman's idea is to restart from here.
     */

    memmove(iv + 6, iv, 2);
    memmove(iv, inbuf + 2, 6);
    num = 0;

    inbuf += 10;
    idea_cfb64_encrypt(inbuf, tmp, inputlen - 10, &ks, iv,
		       &num, IDEA_DECRYPT);

    *outlen = inputlen - 10;
    *outbuf = tmp;
    return(0);
}



/*
 * Take a hash and genereate a Conventional packet.
 * Return -1 on error, 0 if OK.
 *
 * We use RAND_bytes.  Remember to feed it well before using this
 * routine.
 *
 */

int
conventional_key_encrypt(u_char	*inbuf,
			 int	inbuflen,
			 u_char digest[16],
			 int	do_compress,
			 int	*outbuflen,
			 u_char	**outbuf)
{

    IDEA_KEY_SCHEDULE	ks;
    u_char	*buf, *tmp;
    int		header_len;
    u_char	header[5];
    int		buf_len, num;
    u_char 	iv[8];
    /* scratch */
    int 	i;

    if (inbuf == NULL || inbuflen < 1 || digest == NULL
	|| outbuflen == NULL || outbuf == NULL)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }


    if ( do_compress ) {
	i = compress_buffer(inbuf, inbuflen, &buf, &buf_len);
	if ( i != 0 ) {
	    return( i );
	}
	/* ensure that inbuflen holds how much we shall encrypt */
	inbuflen = buf_len;
	inbuf = buf;
    }

    /* So that we can type 10 rather than RAN...+2 */
    assert(RAND_PREFIX_LENGTH == 8 );

    /*
     * We have have a buffer buf, that contains inbuflen bytes to be
     * encrypted.
     * To the data we prepend 64bit random + 16bit keycheck = 10 bytes.
     */
    header_len = make_ctb(CTB_CKE_TYPE, inbuflen+10, 1, header);

    /* This assert ensures that we (still) knows the format of PGP
     * packets, and the the '1' above indeed gave us a full length
     * header.  Call it an internal consistency check......
     */
    assert(header_len == 5 );

    /* The to the buffer, we prepend the header (header_len bytes) */
    tmp = calloc(1, header_len + inbuflen + 10);
    if ( tmp == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    /* Save if here */
    *outbuf = tmp;

    memcpy(tmp, header, header_len);
    assert(tmp[0] == 0xa6);

    tmp += header_len;

    /* RAND_bytes(tmp, 8);
     * tmp += 8;
     */

    /* Include keycheck material */
    tmp[8] = tmp[6];
    tmp[9] = tmp[7];

    /* Get ready to IDEA-encrypt the header*/
    idea_set_encrypt_key(digest,&ks);

    memset(iv, 0, 8);
    num = 0;

    /* Encrypt 10 bytes into the same buffer */
    idea_cfb64_encrypt(tmp, tmp, 10, &ks, iv, &num, IDEA_ENCRYPT);

    /* We decrypt 10 bytes.  If this isn't 8+2, we are lost.  Thus
     * the assert
     */
    assert(num == 2 );

    /* Now, as Zimmerman says, resync the stream as we cross a semantic
     * border.  Refill iv with the encphered data.  We have used 2 (10-8)
     * bytes of iv, save the last 2.
     */
    memmove(iv+6, iv, 2);
    memcpy(iv, tmp+2, 6);
    num = 0;

    /* skip the first 10 bytes */
    tmp += 10;

    /* Then encrypt the rest, that is the text. */
    idea_cfb64_encrypt(inbuf, tmp, inbuflen, &ks, iv, &num, IDEA_ENCRYPT);
    *outbuflen = header_len + 10 + inbuflen;
    return(0);

}
