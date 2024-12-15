/*********************************************************************
 * Filename:      keymgmnt.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 08:45:38 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/



#include <pgplib.h>

#include <md5.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

/* SSLeay */
#include <idea.h>

/*
 * OK : return 0;
 * error: return -1
 */
int
copy_pubkey(PGPpubkey_t *in, PGPpubkey_t *out)
{

    if ( in == NULL || out == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    /* We copy everyting, then make new BIGNUMs */
    memcpy(out, in, sizeof(PGPpubkey_t));
    out->N = BN_dup(in->N);
    if ( out->N == NULL ) {
	out->E = NULL;
	goto bad;
    }
    out->E = BN_dup(in->E);
    if (out->E == NULL )
	goto bad;

    return(0);

bad:
    BN_free(out->N);
    BN_free(out->E);
    memset(out, 0, sizeof(*out));
    pgplib_errno = PGPLIB_ERROR_SYSERR;
    return(-1);
}

void
free_pubkey(PGPpubkey_t	*key)
{
    if ( key == NULL )
	return;
    if ( key->N != NULL )
	BN_free(key->N);
    if ( key->E != NULL )
	BN_free(key->E);
    memset(key, 0, sizeof(*key));

    return;
}


void
free_keyrecords(keyent_t **records)
{
    int		index;

    if ( records == NULL || records[0] == NULL )
	return;
    for(index=0; records[index] != NULL; index += 1) {
	switch (records[index]->what) {
	case PUBKEY :
	    free_pubkey(records[index]->u.key);
	    free(records[index]->u.key);
	    break;
	case SIG:
	    free_signature(records[index]->u.sig);
	    free(records[index]->u.sig);
	    break;
	case UID:
	    free_uid(records[index]->u.uid);
	    free(records[index]->u.uid);
	    break;
	case TRUSTBYTE:
	    free(records[index]->u.trust);
	    break;
	default:
	    assert(0);
	    /*NOTREACHED*/
	}
    }
    free(records);

    return;
}

/*
 * Calculate the fingerprint of a public key and write the
 * resulting _string_ into the memory pointed to by str.
 * The string occupies 50 chars, including the terminating 0.
 * Return 0 on success, -1 on error.
 */
int
pub_fingerprint(PGPpubkey_t	*key,
		char		str[50])
{
    MD5_CTX	context;
    u_char	digest[16];
    int		len;
    u_char	*p;
    /* scratch */
    int		i;

    if ( key == NULL || key->N == NULL || key->E == NULL || str == NULL ){
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    MD5_Init(&context);

    len = BN_num_bytes(key->N);
    p = malloc(len);
    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    BN_bn2bin(key->N, p);

    MD5_Update(&context, p, len);
    free(p);

    len = BN_num_bytes(key->E);
    p = malloc(len);
    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    BN_bn2bin(key->E, p);

    MD5_Update(&context, p, len);
    free(p);

    MD5_Final(digest, &context);

    for(i=0; i < 8; i++, str++)
	sprintf(str, "%02X ", digest[i]);
    str += 1;
    *str = ' ';
    for(i=8; i < 16; i++, str++)
	sprintf(str, "%02X", digest[i]);
    str += 1;
    *str = 0;

    return(0);
}

/*
 * This routine should not be present in a library such as this, since
 * the library has been written in ordert to make it possible to write
 * routines such as this one.
 * Regard it as left-overs from the developing phase.  At best.
 */

int
print_keyrec(u_char	*buffer,
	     int 	buf_len,
	     FILE 	*fd)
{
    u_char	*packet;
    int		p_len;
    int		how_much;
    PGPpubkey_t	*key;
    PGPsig_t	*sig;
    PGPuserid_t	*uid;
    int		have_key;
    u_char	CTB;
    /* scratch */
    int		i;
    PGPKEYID	key_id;
    
    packet = NULL;
    how_much = 0;

    while ( how_much < buf_len ) {

	free(packet);
	packet = NULL;

	i = find_packet_in_buffer(buffer+how_much, buf_len-how_much,
				  &packet);
	p_len = packet_length(packet);
	switch( i ) {
	case -1:
	    free(packet);
	    fprintf(stderr, "Some error in print_keyrecords\n");
	    return(-1);
	case 1:
	    free(packet);
	    if ( buf_len != how_much)
		fprintf(stderr, "Syntax error in buffer\n");
	    return(1);
	case 0:
	    /* We found a package.  We consumed this many bytes */
	    how_much += p_len;
	    break;
	default:
	    assert(0);
	}

	CTB = (packet[0] & CTB_TYPE_MASK) >> 2;
	switch ( CTB ) {
	case CTB_COMMENT_TYPE:
	case CTB_CKE_TYPE:
	case CTB_COMPRESSED_TYPE:
	    fprintf(stderr, "CTB = %x\n", packet[0]);
	    assert(0);
	    /*NOTREACHED*/
	case CTB_CERT_SECKEY_TYPE: {
	    u_short	us;

	    memcpy(&us, packet+11, 2);
	    us = ntohs(us);
	    fprintf(fd, "\tSecret key (length=%d)!! \n", us);
	    continue;
	}
 	case CTB_KEYCTRL_TYPE:
	    fprintf(fd, "\tKey Control Packet\n");
	    continue;
	case CTB_CERT_PUBKEY_TYPE:
	    have_key = 1;
	    key = malloc(sizeof(PGPpubkey_t));
	    if ( key == NULL ) {
		fprintf(stderr, "Out of memory\n");
		return(-1);
	    }
	    i = buf2pubkey(packet, p_len, key);
	    PGPKEYID_FromBN(&key_id,key->N);
	    fprintf(fd, "\tPublic Key: %s\n", PGPKEYID_ToStr(&key_id));

	    free_pubkey(key);
	    free(key);
	    continue;
 	case CTB_USERID_TYPE:
	    uid = malloc(sizeof(PGPuserid_t));
	    if ( uid == NULL ) {
		fprintf(stderr, "Out of memory\n");
		return(-1);
	    }
	    i = buf2uid(packet, p_len, uid);
	    fprintf(fd, "\tUserID: %s\n", uid->name);
	    free_uid(uid);
	    free(uid);
	    continue;

	case CTB_SKE_TYPE: {
	    /* a local variable */
	    PGPuserid_t	**uidp;

	    sig = malloc(sizeof(PGPsig_t));
	    if ( sig == NULL ) {
		fprintf(stderr, "Out of memory\n");
		return(-1);
	    }
	    i = buf2signature(packet, p_len, sig);
	    fprintf(fd, "\tSignature with KeyID: %s\n",
		    PGPKEYID_ToStr(&sig->key_id));

	    i = uids_from_keyid(sig->key_id, &uidp);
	    if ( i == 1 || i == -1)
		fprintf(fd, "\t\tUnknown\n");
	    else {
		for(i = 0; uidp[i] != NULL; i++) {
		    fprintf(fd, "\t\t\t%s\n", uidp[i]->name);
		    free_uid(uidp[i]);
		}
		free(uidp);
	    }
	    free_signature(sig);
	    free(sig);
	    continue;
	}
	default:
	    fprintf(stderr, "Unknown CTB=%d\n", CTB);
	    assert(0);
	    /*NOTREACHED*/
	}
    } /* while */
    return(0);
}


/*
 * Return 0 if all is OK, -1 upon error.
 */

int
pubkey2buf(PGPpubkey_t 	*key,
	   u_char 	**bufptr)
{
    u_char	CTB;
    u_char 	*buf;
    short 	hs;
    unsigned long hl;
    int		howfar;
    int 	i;

    if ( key == NULL || bufptr == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    CTB = CTB_CERT_PUBKEY;

    buf = calloc(1, key->length);
    if ( buf == NULL ) {
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    howfar = 0;

    /* CTB */
    buf[howfar] = CTB;
    howfar += 1;

    /* length */
    hs = htons( (u_short) (key->length - 1 - ctb_llength(CTB)));
    memcpy(buf+howfar, &hs, 2);
    assert ( ctb_llength(CTB) == 2);
    howfar += 2;

    /* version */
    buf[howfar] = key->version;
    howfar += 1;

    /* timestamp */
    hl = htonl(key->created);
    memcpy(buf+howfar, &hl, SIZEOF_TIMESTAMP);
    howfar += SIZEOF_TIMESTAMP;

    /* Validity */
    hs = htons( (u_short)key->valid);
    memcpy(buf+howfar, &hs, 2);
    howfar += 2;

    /* algorithm */
    buf[howfar] = key->algorithm;
    howfar += 1;

    /* prefix to N */
    hs = htons( BN_num_bits(key->N));
    memcpy(buf+howfar, &hs, 2);
    howfar += 2;

    /* N */
    i = BN_bn2bin(key->N, buf+howfar);
    if ( i == 0 ) {
	free(buf);
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    howfar += i;

    /* prefix to E */
    hs = htons( BN_num_bits(key->E));
    memcpy(buf+howfar, &hs, 2);
    howfar += 2;

    /* E */
    i = BN_bn2bin(key->E, buf+howfar);
    if ( i == 0 ) {
	free(buf);
	pgplib_errno = PGPLIB_ERROR_SYSERR;
	return(-1);
    }
    howfar += i;

    if ( key->length != howfar) {
	free(buf);
	pgplib_errno = PGPLIB_ERROR_IMPOSSIBLE;
	return(-1);
    }
    *bufptr = buf;
    return( 0 );
}


/*
 * Take a buffer and convert the thing into a struct.  It must be
 * assumed that this indeed is the binary representation of a
 * key.  The only thing we can check is the CTB and look for a
 * reasonable size of N.
 * Return 0 on OK, -1 on error, 1 on syntax error (which is unsure).
 */

int
buf2pubkey(u_char	*buf,
	   int 		len,
	   PGPpubkey_t  *key)
{
    int		sofar, ret_value = -1;
    /* scratch */
    u_long 	l;
    u_short	us;
    int		i;

    /* 15 is a minimum number, in that it inluces 2 bytes for MPI for
     * both numbers, but not the numbers themselves.
     */
    if ( buf == NULL || len < 15 || key == NULL) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    sofar = 0;
    memset(key, 0, sizeof(PGPpubkey_t));

    if ( ! is_ctb_type(buf[sofar], CTB_CERT_PUBKEY_TYPE )) {
	pgplib_errno = PGPLIB_ERROR_BAD_CTB;
	return(1);
    }
    sofar++;

    /* length of the _entire_ packet */
    key->length = packet_length(buf);
    if ( key->length == -1 )
	return (-1);

    if ( key->length > len ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }

    sofar += ctb_llength(*buf);

    /*version */
    memcpy( &key->version, buf+sofar, 1);
    sofar++;

    /* time */
    l = 0;
    memcpy( &l, buf+sofar, SIZEOF_TIMESTAMP);
    key->created = ntohl(l);
    /* XXX Sanity check on timestamp ? */
    sofar += SIZEOF_TIMESTAMP;

    /* validity */
    us = 0;
    memcpy( &us, buf+sofar, 2);
    key->valid = ntohs(us);
    /* XXX Sanity check on timestamp ? */
    sofar += 2;

    /* Read algotithm */
    memcpy( &key->algorithm, buf+sofar, 1);
    if (key->algorithm != RSA_ALGORITHM_BYTE ) {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
	goto bad;
    }
    sofar++;

    ret_value = mpi2bignum(buf+sofar, len-sofar, &i, &key->N);
    if ( ret_value != 0 ) {
	ret_value = -1;
	goto bad;
    }
    sofar += i;

    ret_value = mpi2bignum(buf+sofar, len-sofar, &i, &key->E);
    if ( ret_value != 0 ) {
	ret_value = -1;
	goto bad;
    }

    return(0);

    /* We only get here if something is wrong.  Notice that
     * free(NULL) is not an error, thus we have to memset at the top.
     */
bad:
    BN_free(key->N);
    BN_free(key->E);
    return(ret_value);
}


/*
 * Routines to manipulate secret keys.
 */

void
free_seckey(PGPseckey_t	*key)
{
    if ( key == NULL)
	return;
    if ( key->N != NULL )
	BN_free(key->N);
    if ( key->E != NULL )
	BN_free(key->E);
    if ( key->D != NULL )
	BN_clear_free(key->D);
    if ( key->P != NULL )
	BN_clear_free(key->P);
    if ( key->Q != NULL )
	BN_clear_free(key->Q);
    if ( key->U != NULL )
	BN_clear_free(key->U);
    memset(key, 0, sizeof(PGPseckey_t));
}

/*
 * Turn a buffer into a secret key.
 * We must assume that the buffer contains a key, but try
 * to do verification as we go.
 * Return -1 on error, 0 if all is well.
 */


int
buf2seckey(u_char	*buf,
	   int		buflen,
	   PGPseckey_t	*key)
{
    int		sofar, ret_value;
    u_long	ul;
    u_short	us;
    /* scratch */
    int		i;

    /* 34 is the lowest possible number.  Two bytes are assumed per MPI,
     * but no number itself (All MPI's == 0 )
     */
    if ( buf == NULL || buflen < 34 || key == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    sofar = 0;
    memset(key, 0, sizeof(*key));

    if ( ! is_ctb_type(buf[sofar], CTB_CERT_SECKEY_TYPE )) {
	pgplib_errno = PGPLIB_ERROR_BAD_CTB;
	return(-1);
    }
    sofar++;

    /* length of the _entire_ packet */
    key->length = packet_length(buf);
    if ( key->length > buflen || key->length == -1) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    sofar += ctb_llength(*buf);

    /* version*/
    key->version = buf[sofar];
    if (key->version != VERSION_BYTE_NEW
	&& key->version != VERSION_BYTE_OLD)
    {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM; /* well.... */
	return(-1);
    }
    sofar += 1;
    /* timestamp */
    memcpy(&ul, buf+sofar, 4);
    key->created = ntohl(ul);
    /* XXX verification of timestamk ? */
    sofar += 4;

    /* Validity */
    memcpy(&us, buf+sofar, 2);
    key->valid = ntohs(us);
    sofar += 2;

    /* RSA algorithm byte */
    key->algorithm = buf[sofar];
    if ( key->algorithm != RSA_ALGORITHM_BYTE) {
	pgplib_errno = PGPLIB_ERROR_BAD_ALGORITHM;
	return(-1);
    }
    sofar += 1;

    /* Public N */
    ret_value = mpi2bignum(buf+sofar, buflen-sofar, &i, &key->N);
    if ( ret_value != 0 )
	goto bad;
    sofar += i;

    /* Public E */
    ret_value = mpi2bignum(buf+sofar, buflen-sofar, &i, &key->E);
    if ( ret_value != 0 )
	goto bad;
    sofar += i;

    /* Type of chipher used */
    key->algorithm = buf[sofar];
    sofar += 1;
    /* The key is only followed by IV when it exists.  Pointed out by
     * Andy Dustman <andy@CCMSD.chem.uga.edu>
     */
    if ( key->algorithm != 0 ) {
	/* Eight bytes of iv */
	memcpy(key->iv, buf+sofar, 8);
	sofar += 8;
    }

    /* Then the four private numbers */
    ret_value = mpi2bignum(buf+sofar, buflen-sofar, &i, &key->D);
    if ( ret_value != 0 )
	goto bad;
    sofar += i;

    ret_value = mpi2bignum(buf+sofar, buflen-sofar, &i, &key->P);
    if ( ret_value != 0 )
	goto bad;
    sofar += i;

    ret_value = mpi2bignum(buf+sofar, buflen-sofar, &i, &key->Q);
    if ( ret_value != 0 )
	goto bad;
    sofar += i;

    ret_value = mpi2bignum(buf+sofar, buflen-sofar, &i, &key->U);
    if ( ret_value != 0 )
	goto bad;
    sofar += i;

    /* And, at the end, the checksum */
    if ( sofar+2 > buflen ) {
	ret_value = 1;
	/* XXX EINVAL shoud bee syntax error or something */
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	goto bad;
    }

    memcpy(&key->cksum, buf+sofar, 2);

    return(0);

bad:
    free_seckey(key);

    if ( ret_value == 1 || ret_value == -1 )
	return(ret_value);
    else
	return(-1);
}


/*
 * In order to actually use a secret key, the secret parts must
 * be decrypted.  Return -1 on error, 1 on incorrect decryprion
 * and 0 on sucess.
 *
 * An armed (decrypted) key can be recognised by the fact that
 * the field algorithm is set to 0.
 *
 */

int
decrypt_seckey(PGPseckey_t 	*old,
	       u_char 		digest[16],
	       PGPseckey_t	*new)
{
    u_char	*buf;
    IDEA_KEY_SCHEDULE ks;
    int		len1, len2;
    u_char	iv[8];
    int		useiv;	/* Keep track of IV usage */
    BIGNUM	*tmp;
    /* scratch */
    int		i;

    if ( old == NULL || digest == NULL || new == NULL) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    /* Set things up */
    memcpy(new, old, sizeof(*new));
    new->D = new->P = new->Q = new->U = NULL;
    new->algorithm = 0;

    new->N = BN_dup(old->N);
    new->E = BN_dup(old->E);

    idea_set_encrypt_key(digest, &ks);
    memcpy(iv, old->iv, 8);
    useiv = 0;

    pgplib_errno = PGPLIB_ERROR_MAX;

    /* Do the number D */
    len1 = BN_num_bytes(old->D);
    buf = malloc(len1);
    if ( buf == NULL )
	goto bad;
    /* Make binary for decryption */
    len2 = BN_bn2bin(old->D, buf);
    idea_cfb64_encrypt(buf, buf, len1, &ks, iv, &useiv, IDEA_DECRYPT);
    new->D = BN_bin2bn(buf, len1, NULL);
    memset(buf, 0, len1);
    free(buf);
    if ( new->D == NULL )
	goto bad;

    /* The number P */
    len1 = BN_num_bytes(old->P);
    buf = malloc(len1);
    if ( buf == NULL )
	goto bad;
    len2 = BN_bn2bin(old->P, buf);
    idea_cfb64_encrypt(buf, buf, len1, &ks, iv, &useiv, IDEA_DECRYPT);
    new->P = BN_bin2bn(buf, len1, NULL);
    memset(buf, 0, len1);
    free(buf);
    if ( new->P == NULL )
	goto bad;

    /* The number Q */
    len1 = BN_num_bytes(old->Q);
    buf = malloc(len1);
    if ( buf == NULL )
	goto bad;
    len2 = BN_bn2bin(old->Q, buf);
    idea_cfb64_encrypt(buf, buf, len1, &ks, iv, &useiv, IDEA_DECRYPT);
    new->Q = BN_bin2bn(buf, len1, NULL);
    memset(buf, 0, len1);
    free(buf);
    if ( new->Q == NULL )
	goto bad;

    /*
     * There are two ways for us to check whether the password was correct
     * or not: Either verify that P*Q = N, or using the PGP checksum
     * provided with the key.   The latter is calculated over the MPI's
     * (that is, on the numbers, after decryption, but including the
     * prefixed bitcount).  Since we store numbers as BIGNUMs we use the
     * former method.
     * As usual, I solicit your comments.
     */

    tmp = BN_new();
    i = BN_mul(tmp, new->P, new->Q);
    if ( i == 0 )
	goto bad;
    i = BN_cmp(tmp, old->N);
    BN_free(tmp);
    if ( i != 0 ) {
	clear_seckey(new);
	pgplib_errno = PGPLIB_ERROR_WRONG_PASS;
	return(1);
    }

    /* The number U */
    len1 = BN_num_bytes(old->U);
    buf = malloc(len1);
    if ( buf == NULL )
	goto bad;
    len2 = BN_bn2bin(old->U, buf);
    idea_cfb64_encrypt(buf, buf, len1, &ks, iv, &useiv, IDEA_DECRYPT);
    new->U = BN_bin2bn(buf, len1, NULL);
    memset(buf, 0, len1);
    free(buf);
    if ( new->U == NULL )
	goto bad;

    return(0);

bad:

    clear_seckey(new);
    if ( pgplib_errno == PGPLIB_ERROR_MAX )
	pgplib_errno = PGPLIB_ERROR_SYSERR;
    return -1;
}

/* Remove the secrets stored in the key. */

void
clear_seckey(PGPseckey_t	*key)
{
    BN_clear_free(key->D); key->D = NULL;
    BN_clear_free(key->P); key->P = NULL;
    BN_clear_free(key->Q); key->Q = NULL;
    BN_clear_free(key->U); key->U = NULL;
}
