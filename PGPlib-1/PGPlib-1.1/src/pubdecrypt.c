/*********************************************************************
 * Filename:      pubdecrypt.c
 * Description:
 * Author:        Thomas Roessler <roessler@guug.de>
 * Modified at:   Mon Jan 19 09:51:48 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) Thomas Roessler
 * 	See the file COPYING for details
 ********************************************************************/



#include <pgplib.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

/* SSLeay */
#include <rsa.h>


/* take a secret key, a public-key encrypted packet,
 * a conventionally encrypted packet, and tries to
 * decrypt this stuff.
 *
 * Return value:
 *
 * 0 = ok, 1 = packet invalid, 2 = wrong secret key, -1 = other failure.
 *
 */

int
sec_decrypt_packet(PGPseckey_t 	*key,
		   u_char 	*PKE_packet,
		   int 		pke_l,
		   u_char 	*conv_packet,
		   int 		conv_l,
		   u_char 	**outbuf,
		   int 		*outlen)
{
    RSA 	*rsa_info;
    PGPpke_t 	pke;
    PGPKEYID 	seckeyid;
    int 	buflen;
    u_char 	*edek_buf;
    u_char 	*dek_buf;
    u_char 	*dek;
    u_short 	cksum;
    int 	i;

    if (key == NULL || PKE_packet == NULL || conv_packet == NULL
	|| outbuf == NULL || outlen == NULL)
	return -1;
    if ( conv_l <= 5 )
	return 1;

    
    *outbuf = NULL;

    i = buf2pke(PKE_packet, pke_l, &pke);
    if ( i != 0 )
	return i;

    PGPKEYID_FromBN(&seckeyid,key->N);

    if(memcmp(&seckeyid, &pke.key_id, sizeof(PGPKEYID))) {
	free_pke(&pke);
	return 2;
    }

    buflen = BN_num_bytes(pke.E);
    edek_buf = calloc(buflen, 1);
    if ( edek_buf == NULL) {
	free_pke(&pke);
	return -1;
    }
    dek_buf = calloc(buflen, 1);
    if ( dek_buf == NULL) {
	free_pke(&pke);
	free(edek_buf);
	return -1;
    }

    BN_bn2bin(pke.E, edek_buf);

    rsa_info = RSA_new();
    rsa_info->n = BN_dup(key->N);
    rsa_info->e = BN_dup(key->E);
    rsa_info->d = BN_dup(key->D);

    if (pke.algorithm != RSA_ALGORITHM_BYTE)
    {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
	return 1;
    }

    i = RSA_private_decrypt(buflen, edek_buf, dek_buf, rsa_info,
			    RSA_PKCS1_PADDING);
    RSA_free(rsa_info);
    free(edek_buf);
    free_pke(&pke);

    if (i == -1 || i < 19) {
	free(dek_buf);
	return 1;
    }

    dek = dek_buf + i - 19;

    if(*dek++ != 1) {
	free(dek_buf);
	return 1;
    }

    cksum = htons(checksum(dek, 16));
    if(*((u_short *) (dek + 16)) != cksum) {
	free(dek_buf);
	return 2;
    }

    i = conventional_key_decrypt(conv_packet, dek, outlen, outbuf);
    free(dek_buf);
    return i;
}
