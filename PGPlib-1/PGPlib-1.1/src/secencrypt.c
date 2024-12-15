/*********************************************************************
 * Filename:      secencrypt.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 10:20:40 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

/* All routines to deal with secret keys can be found here */

#include <pgplib.h>

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

/* SSLeay*/
#include <md5.h>
#include <rsa.h>


/*
 * Convert a signature into a buffer.
 */

int
signature2buf(PGPsig_t	*sig,
	      u_char	**buf)
{
    int 	sofar;
    u_char 	*tmp;
    u_short	us;
    u_long	ul;
    /* scratch */
    int		i;

    if ( sig == NULL || buf == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return (-1);
    }

    if(sig->pk_alg != RSA_ALGORITHM_BYTE
       || sig->hash_alg != MD5_ALGORITHM_BYTE)
    {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
	return -1;
    }

    /* If the signature has been generated (rather than read) we
     * don't know the size it will occupy.  According to PGFORMAT.DOC
     * the MPI starts at position 22.  The added 2 is for the MPI
     * prefix.
     */

    if ( sig->length == 0 )
	sig->length = BN_num_bytes(sig->I) + 22 + 2;

    tmp = malloc(sig->length);
    if ( tmp == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }

    sofar = 0;
    tmp[sofar] = CTB_SKE;
    sofar++;

    us = htons( (u_short) sig->length - 1 - 2);
    memcpy(tmp+sofar, &us, 2);
    sofar += 2;

    tmp[sofar] = sig->version;
    sofar++;

    tmp[sofar] = sig->included;
    sofar++;

    tmp[sofar] = sig->classification;
    sofar++;

    ul = htonl(sig->timestamp);
    memcpy(tmp+sofar, &ul, 4);
    sofar += 4;

    PGPKEYID_ToBytes(&sig->key_id, tmp+sofar);
    sofar += PGPKEYID_SIZE;

    tmp[sofar] = sig->pk_alg;
    if (sig->pk_alg != RSA_ALGORITHM_BYTE)
	return(-1);
    sofar++;

    tmp[sofar] = sig->hash_alg;
    sofar++;

    tmp[sofar] = sig->ck[0];
    sofar += 1;
    tmp[sofar] = sig->ck[1];
    sofar += 1;

    /* MPI prefix */
    i = BN_num_bits(sig->I);
    us = htons((u_short) i);
    memcpy(tmp+sofar, &us, 2);
    sofar += 2;

    i = BN_num_bytes(sig->I);

    if ( sofar + i > sig->length ) {
	free(tmp);
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    (void) BN_bn2bin(sig->I, tmp+sofar);

    *buf = tmp;
    return(0);
}

/*
 * Convert a buffer into a signature.  What the signature is for
 * can be seen in the signature byte.
 */

int
buf2signature(u_char	*buf,
	      int 	buflen,
	      PGPsig_t 	*sig)
{
    u_long	l;
    int	        i, used;
    int		howfar;

    if ( buf == NULL || sig == NULL || buflen < 24 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    if ( ! is_ctb_type(*buf, CTB_SKE_TYPE) ) {
	pgplib_errno = PGPLIB_ERROR_BAD_CTB;
	return(-1);
    }

    howfar = 0;
    memset(sig, 0, sizeof(PGPsig_t));

    /* XXX What is the maximum ? */
    i = packet_length(buf);
    if ( i == -1 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    sig->length = i;

    /* skip CTB */
    howfar += 1;

    /* and length */
    i = ctb_llength(*buf);
    if ( i < 0 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	goto bad;
    }
    howfar += i;

    sig->version = buf[howfar];
    if (sig->version != VERSION_BYTE_NEW
	&& sig->version != VERSION_BYTE_OLD)
    {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
	goto bad;
    }
    howfar += 1;

    sig->included = buf[howfar];
    howfar += 1;
    if ( sig->included != 5 ) {
	pgplib_errno = PGPLIB_ERROR_BAD_PACKET_LENGTH;
	goto bad;
    }

    sig->classification = buf[howfar];
    howfar += 1;

    l = 0;
    memcpy( &l, buf + howfar, SIZEOF_TIMESTAMP);
    sig->timestamp = ntohl(l);
    if ( sig->timestamp == 0 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	goto bad;
    }
    howfar += SIZEOF_TIMESTAMP;

    PGPKEYID_FromBytes(&sig->key_id,buf+howfar);
    howfar += PGPKEYID_SIZE;

    sig->pk_alg = buf[howfar];
    if ( sig->pk_alg !=  RSA_ALGORITHM_BYTE ) {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
	goto bad;
    }
    howfar += 1;

    sig->hash_alg= buf[howfar];
    if ( sig->hash_alg !=  MD5_ALGORITHM_BYTE) {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
	goto bad;
    }
    howfar += 1;

    sig->ck[0] = buf[howfar];
    howfar += 1;
    sig->ck[1] = buf[howfar];
    howfar += 1;

    i = mpi2bignum(buf+howfar, buflen - howfar, &used, &sig->I);
    if ( i != 0 ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	goto bad;
    }
    howfar += used;

    if ( sig->length != howfar ) {
	pgplib_errno = PGPLIB_ERROR_BAD_PACKET_LENGTH;
	goto bad;
    }

    return(0);

bad:
    BN_free(sig->I);
    return(-1);
}

void
free_signature(PGPsig_t *sig)
{
    if ( sig == NULL)
	return;
    /* free(NULL) is OK */
    BN_free(sig->I);
}


/* The following is from PGFORMAT.DOC:
 * ===========================
 *
 * Message digest "packet"
 * -----------------------
 *
 * The Message digest has no CTB packet framing.  It is stored
 * packetless and naked, with padding, encrypted inside the MPI in the
 * Signature packet.
 *
 * PGP versions 2.3 and later use a new format for encoding the message
 * digest into the MPI in the signature packet, a format which is
 * compatible with RFC1425 (formerly RFC1115).  This format is accepted
 * but not written by version 2.2.  The older format used by versions 2.2
 * is acepted by versions up to 2.4, but the RSAREF code in 2.5 is
 * not capable of parsing it.
 *
 * PGP versions 2.3 and later encode the message digest (MD) into the
 * MPI as follows:
 *       MSB               .   .   .                  LSB
 *        0   1   FF(n bytes)   0   ASN(18 bytes)   MD(16 bytes)
 * See RFC1423 for an explanation of the meaning of the ASN string.
 * It is the following 18 byte long hex value:
 *        3020300c06082a864886f70d020505000410
 * Enough bytes of FF padding are added to make the length of this
 * whole string equal to the number of bytes in the modulus.
 *
 * There is no checksum included.  The padding serves to verify that the
 * correct RSA key was used.
 * =============== (End of PGFORMAT.DOC)

 */

static int
make_bufferhash(u_char		*data,
		int		dlen,
		PGPsig_t	*sig,
		u_char		digest[16])
{
    u_long	ul;
    MD5_CTX	context;

    if( data == NULL || dlen < 0 || sig == NULL || digest == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }
    if(sig->hash_alg != MD5_ALGORITHM_BYTE)
    {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
	return -1;
    }

    MD5_Init(&context);
    MD5_Update(&context, data, dlen);

    /* 5 = 1 + 4 => signature classification + timestamp */
    if (sig->included != 5) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    MD5_Update(&context, &sig->classification, 1);
    ul = htonl(sig->timestamp);
    MD5_Update(&context, (unsigned char *)&ul, 4);

    MD5_Final(digest, &context);
    return(0);
}


/*
 * ASN.1 Distinguished Encoding Rules (DER) encoding for RSA/MD5 (as
 * defined in RFC1423.
 */
static u_char asn1_array[] = {		/* PKCS 01 block type 01 data */
    0x30,0x20,0x30,0x0c,0x06,0x08,
    0x2a,0x86,0x48,0x86,0xf7,0x0d,
    0x02,0x05,0x05,0x00,0x04,0x10 };

/*
 * Verify that a signature is correct.  We take a public key,
 * a signature and a buffer (with a length).  Create a hash and
 * compare it with what we find inside the sig->I, using the ASN.1
 * to verify decryption.
 * Return -1 on error, 1 on incorrect passwd and 0 on sucess.
 */

int
verify_sig(u_char		*data,
	   int		dlen,
	   PGPsig_t   	*sig,
	   PGPpubkey_t	*signkey)
{
    RSA		*rsa_info;
    u_char	*sigbuf;
    u_char	our_digest[16+2]; /* The extra is checksum */
    u_char	*found_digest;
    int		buflen;
    int		ret;
    /* scratch */
    int		i, j;

    if ( sig == NULL || signkey == NULL || data == NULL || dlen == 0 )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    if(sig->pk_alg != RSA_ALGORITHM_BYTE
       || sig->hash_alg != MD5_ALGORITHM_BYTE)
    {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
	return -1;
    }

    /* We have to make sig->I into a buffer.  This is a pity since rsa_*
     * turns it into a number again but they want a buffer.
     */
    buflen = BN_num_bytes(sig->I);
    sigbuf = calloc(buflen, 1);
    if ( sigbuf == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    /* We make buflen just as large, to be sure.  In fact, there is
     * no other size that makes sense.
     */
    found_digest =  calloc(buflen, 1);
    if ( found_digest == NULL ) {
	free(sigbuf);
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    BN_bn2bin(sig->I, sigbuf);
    rsa_info = RSA_new();
    if ( rsa_info == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return -1;
    }
    rsa_info->n = BN_dup(signkey->N);
    rsa_info->e = BN_dup(signkey->E);

    i = RSA_public_decrypt(buflen, sigbuf, found_digest, rsa_info,
			   RSA_PKCS1_PADDING);
    RSA_free(rsa_info);
    free(sigbuf);

    if ( i == -1 ) {
	free(found_digest);
	return(1);
    }
    /* The result should contain 18 bytes of ASN.1 data followed
     * by 16 bytes of MD5 (= 34 bytes).
     */
    if ( i != 16 + sizeof(asn1_array) ) {
	free(found_digest);
	return(1);
    }
    /*
     * Verify that the correct key was used by comparing the ASN.1
     * data.
     */
    for(i=0; i < sizeof(asn1_array); i++ ) {
	if ( found_digest[i] != asn1_array[i] ) {
	    free(found_digest);
	    return(1);
	}
    }

    /* Construct our own hash */
    i = make_bufferhash(data, dlen, sig, our_digest);
    if ( i == -1 ) {
	free(found_digest);
	return(-1);
    }

    /*
     * Well, are they identical ?
     * An interesting question is: What if the digests are identical but
     * the checksums are different ?
     * Since I don't know the answer, I don't known what to do if this
     * turns out to be the case; I'll ignore it (suggestions
     * are most welcome).
     */
    ret = 0;
    for(j = sizeof(asn1_array), i=0; i < 16 ; i++ ) {
	if ( our_digest[i] != found_digest[i+j]) {
	    ret = 1;
	    break;
	}
    }
    free(found_digest);
    return(ret);
}


/*
 * To verify a key-uid binding we concatenate them and verify
 * if the signature is correct.
 */

int
verify_keysig(PGPpubkey_t	*pubkey,
	      PGPuserid_t	*uid,
	      PGPsig_t		*sig,
	      PGPpubkey_t	*signkey)
{
    int 	len;
    int		ret;
    u_char	*buf, *p;
    int		hlen; /* not used but don't delete, see the ifdef below */

    if ( pubkey == NULL || sig == NULL || signkey == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    if(((sig->classification >= K0_SIGNATURE_BYTE &&
	sig->classification <= K3_SIGNATURE_BYTE)
       || sig->classification == KR_SIGNATURE_BYTE) 
       && uid == NULL)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    if(sig->classification == KC_SIGNATURE_BYTE)
	uid = NULL;
    else if (!(sig->classification >= K0_SIGNATURE_BYTE
	       && sig->classification <= K3_SIGNATURE_BYTE)
	     && sig->classification != KR_SIGNATURE_BYTE)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    if ( pubkey2buf(pubkey, &buf) != 0)
	return(-1);
    len = packet_length(buf);
    if ( len == -1 )
	return -1;

/* **************************************************************
 * There is a BUG in the way PGP makes signatures on keys.
 * According to PGFORMAT.DOC (line 220) this is what is supposed to
 * happen:
 *   The packet headers (CTB and length fields) for the public key
 *   packet and the user ID packet are both omitted from the signature
 *   calculation for a key certification.
 * However, the key is read as it appears on file, contrary to what it
 * it supposed to do.
 *
 * The way it is done is probably better than the way it is described.
 * ************************************************************** */

#ifdef NotDef
    hlen = 1 + ctb_llength(buf[0]);	/* CTB + length */
#else
    hlen = 0;
#endif

    if(uid)
    {
	p = realloc(buf, len+strlen(uid->name));
	if ( p == NULL ) {
	    free(buf);
	    pgplib_errno = PGPLIB_ERROR_SYSERR;
	    return(-1);
	}
	buf = p;
	memcpy(buf+len, uid->name, strlen(uid->name));
    }
    
    /* hlen then used here to offset over the CTB and length */
    ret = verify_sig(buf+hlen, len-hlen+ (uid ? strlen(uid->name) : 0),
		     sig, signkey);
    free(buf);
    return(ret);
}


/*
 * Sign a buffer with a secret key.  The result is placed in a
 * PGPsec_t which can be converted into a PGP package if required.
 * The signature->classification must be set by the caller (there is
 * no way we can know).
 */

int
make_sig(u_char		*buffer,
	 int 		bufsize,
	 u_char		class,
	 PGPseckey_t	*key,
	 PGPsig_t 	*sig)
{

    u_char 	mdigest[18+16];
    RSA		*rsa_info;
    u_char	*rsa_res;
    int		rsa_len;
    /* To speed up RSA */
    BIGNUM	*tmp;
    BN_CTX	*ctx;
    /* scratch */

    if ( key == NULL || buffer == NULL || bufsize < 1 || sig == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    sig->classification = class;

    sig->version = VERSION_BYTE_NEW;
    sig->timestamp = (u_long) time(NULL);
    sig->included = 5;
    sig->pk_alg = RSA_ALGORITHM_BYTE;
    sig->hash_alg = MD5_ALGORITHM_BYTE;
    PGPKEYID_FromBN(&sig->key_id,key->N);

    memcpy(mdigest, asn1_array, 18);
    if ( make_bufferhash(buffer, bufsize, sig, mdigest+18) == -1)
	return -1;

    /* First two bytes of the hash (for some definition of "first") */
    memcpy(sig->ck, mdigest+18, 2);

    rsa_info = RSA_new();
    if ( rsa_info == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	memset(sig, 0, sizeof(*sig));
	return -1;
    }
    rsa_info->d = BN_dup(key->D);
    rsa_info->n = BN_dup(key->N);

    /* Accoring to Andy Dustman <andy@CCMSD.chem.uga.edu> and Derek
     * Atkins warlord@MIT.EDU the number U in PGP is inv(p mod q)
     * while the SSLeay number iqmp is inv(q mod p).  In order to
     * use PGP and SSLeay together we have to switch the numbers
     * before we use them.
     */
    tmp = BN_new();
    ctx = BN_CTX_new();

    rsa_info->p = BN_dup(key->Q);
    rsa_info->q = BN_dup(key->P);

    /* p-1 */
    BN_sub(tmp, rsa_info->p, BN_value_one());
    /* d mod p-1 (assigned to dmQ1) */
    BN_mod(rsa_info->dmq1, rsa_info->d, tmp, ctx);

    /* q-1 */
    BN_sub(tmp, rsa_info->q, BN_value_one());
    /* d mod q-1 (assigned to dmP1) */
    BN_mod(rsa_info->dmp1, rsa_info->d, tmp, ctx);

    /* inverse of q mod p.  Well, U, which is p mod q */
    rsa_info->iqmp = BN_dup(key->U);

    BN_clear_free(tmp);
    BN_CTX_free(ctx);

    rsa_res = malloc(RSA_size(rsa_info));
    if ( rsa_res == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	RSA_free(rsa_info);
	return(-1);
    }

    rsa_len = RSA_private_encrypt(18+16, mdigest, rsa_res, rsa_info,
				 RSA_PKCS1_PADDING);

    RSA_free(rsa_info);

    if ( rsa_len == -1 ) {
	/* XXX not precisely syserr.... */
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	free(rsa_res);
	return(-1);
    }

    sig->I = BN_bin2bn(rsa_res, rsa_len, NULL);
    sig->length = BN_num_bytes(sig->I) + 22 + 2;

    return(0);
}


int
sign_buffer(u_char	*buffer,
	    int 	bufsize,
	    PGPseckey_t	*key,
	    PGPsig_t 	*sig)

{
    return(make_sig(buffer, bufsize, SB_SIGNATURE_BYTE, key, sig));
}



/* check for a key compromise certificate */

int 
is_compromised(PGPKEYID keyid)
{
    keyent_t	**contents;
    PGPpubkey_t	*key = NULL;
    PGPsig_t	*sig = NULL;
    int 	i;
    int		is_compromised = 0;

    if((i = get_keycontents(keyid, &contents)))
    {
	if(i == 1)
	    pgplib_errno = PGPLIB_ERROR_EINVAL;

	return -1;
    }

    for(i = 0; contents[i] && !is_compromised; i++)
    {
	switch(contents[i]->what)
	{
	case PUBKEY:
	    key = contents[i]->u.key;
	    break;
	case SIG:
	    sig = contents[i]->u.sig;

	    if(sig->classification == KC_SIGNATURE_BYTE
	       && PGPKEYID_EQUAL(sig->key_id,keyid) 
	       && verify_keysig(key, NULL, sig, key) == 0)
	    {
		is_compromised = 1;
	    }
	    break;
	default:
	    break;
	}
    }
    
    free_keyrecords(contents);
    
    return is_compromised;
}
    

