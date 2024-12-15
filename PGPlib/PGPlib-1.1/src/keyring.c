/*********************************************************************
 * Filename:      keyring.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:42:32 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/



#include <pgplib.h>

#include <stdlib.h>


/*
 * Parsing a BUFFER (presumaly holding a keyring) and return public
 * keys, certificates and UserIDs as we find them.  One packet is
 * returned on each call.
 * If a packet was indeed found, we return 0 and *used shows how much
 * we consumed.  If there was a pointer into which we could store data,
 * we do so.
 * If we don't find a packet or the buffer is exhausted, we return 1.
 * On error, we return -1;
 *
 * Although some checks are performed, sending in random data is probably
 * a reliable way to obtain a coredump.  In other words, quite a few
 * checks for sanity should be inserted.
 */

int
get_keyentry_from_buf(u_char		*buf,
		      int		buf_len,
		      res_t		*what,
		      PGPsig_t		**sig,
		      PGPpubkey_t	**pubkey,
		      PGPuserid_t	**uid,
		      PGPseckey_t	**seckey,
		      PGPtrust_t	**trust,
		      int		*used)
{
    int		how_far;
    int		p_len;
    u_char	*packet;
    int		ret;
    /* scratch */
    int		i;

    if ( buf == NULL || buf_len == 0 || used == NULL )
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    *what = NONE;
    how_far = 0;
    p_len = 0;
    while ( how_far <= buf_len ) {
	ret = find_packet_in_buffer(buf+how_far, buf_len-how_far,
				  &packet);
	switch ( ret ) {
	case -1:
	    return(-1);
	case 1:
	    /* Syntax error _or_ packet without length (compressed) */
	    return(1);
	case 0:
	    p_len = packet_length(packet);
	    if ( p_len == -1 ) {
		free(packet);
		return(-1);
	    }
	    *used = how_far + p_len;

	    /* Some packet, p_len bytes long */
	    switch ( (packet[0] & CTB_TYPE_MASK) >> 2) {
	    default:
		free(packet);
		how_far += p_len;
		break;
	    case CTB_USERID_TYPE:
		if ( uid != NULL ) {
		    *uid = malloc(sizeof(PGPuserid_t));
		    if (*uid == NULL ) {
			free(packet);
			pgplib_errno = PGPLIB_ERROR_SYSERR;
			return(-1);
		    }
		    i = buf2uid(packet, p_len, *uid);
		    free(packet);
		    if ( i != 0 )
			return(-1);
		    else {
			*what = UID;
			return(0);
		    }
		}
		else {
		    free(packet);
		    return(0);
		}
	    case CTB_KEYCTRL_TYPE:
		if ( trust != NULL ) {
		    *trust = malloc(sizeof(PGPtrust_t));
		    if ( *trust == NULL ) {
			pgplib_errno = PGPLIB_ERROR_SYSERR;
			free(packet);
			return(-1);
		    }
		    i = buf2trust(packet, p_len, *trust);
		    free(packet);
		    if ( i != 0 )
			return(-1);
		    else {
			*what = TRUSTBYTE;
			return(0);
		    }
		}
		else {
		    free(packet);
		    return(0);
		}
	    case CTB_CERT_PUBKEY_TYPE:
		if ( pubkey != NULL ) {
		    *pubkey = malloc(sizeof(PGPpubkey_t));
		    if (*pubkey == NULL ) {
			free(packet);
			pgplib_errno = PGPLIB_ERROR_SYSERR;
			return(-1);
		    }
		    i = buf2pubkey(packet, p_len, *pubkey);
		    free(packet);
		    if ( i != 0 )
		    {
			return(-1);
		    }
		    else {
			*what = PUBKEY;
			return(0);
		    }
		}
 		else {
		    free(packet);
		    return(0);
		}
	    case CTB_CERT_SECKEY_TYPE:
		if ( seckey != NULL ) {
		    *seckey = malloc(sizeof(PGPseckey_t));
		    if (*seckey == NULL ) {
			free(packet);
			pgplib_errno = PGPLIB_ERROR_SYSERR;
			return(-1);
		    }
		    i = buf2seckey(packet, p_len, *seckey);
		    free(packet);
		    if ( i != 0 )
		    {
			return(-1);
		    }
		    else {
			*what = SECKEY;
			return(0);
		    }
		}
	    case CTB_SKE_TYPE:
		if ( sig != NULL ) {
		    *sig = malloc(sizeof(PGPsig_t));
		    if (*sig == NULL ) {
			pgplib_errno = PGPLIB_ERROR_SYSERR;
			return(-1);
		    }
		    i = buf2signature(packet, p_len, *sig);
		    free(packet);
		    if ( i != 0 )
		    {
			return(-1);
		    }
		    else {
			*what = SIG;
			return(0);
		    }
		}
		else {
		    free(packet);
		    return(0);
		}
	    } /* switch CTB */
	} /* switch find_packet_in_buffer */
    } /* while buffer */

    return(1);
}

/*
 * Find a secret key in a buffer.  Return the number of bytes consumed.
 * If there is an error (or no seckey) return -1 or 0, respectively.
 */

int
get_seckey_from_buf(u_char	*buf,
		    int 	buflen,
		    PGPseckey_t **key)
{
    res_t	what;
    int		used, tot_used;
    int		ret;
    PGPseckey_t	*tmpkey;

    if ( buf == NULL || buflen < 34 || key == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }

    *key = NULL;
    used = tot_used = 0;

    while ( buflen > 0 ) {
	ret = get_keyentry_from_buf(buf, buflen, &what, NULL, NULL,
				    NULL, &tmpkey, NULL, &used);
	buflen -= used;
	buf += used;
	tot_used += used;

	if ( what != SECKEY )
	    continue;

	*key = tmpkey;
	return(tot_used);
    }
    return(0);
}


/*
 * Parsing a FILE (presumably a keyring) and return public keys
 * certificates and UserID's as we find them.  One is returned on
 * each call.  That is, if you don't like the key you get, throw it away
 * and get a new one.
 * If something is found, 0 is returned and *what is set to the type
 * in question; if a pointer was provided, we create a struct of the
 * correct type.  Upon EOF or syntax error 1 is returned.
 * An (real) error will give you -1.
 *
 */

int
get_keyentry_from_file(FILE		*fd,
		       res_t		*what,
		       PGPsig_t		**sig,
		       PGPpubkey_t 	**pubkey,
		       PGPuserid_t 	**uid,
		       PGPseckey_t	**seckey,
		       PGPtrust_t	**trust)
{
    u_char	*buf;
    int		length, ret;
    /* scratch */
    int		i,j;

    if ( fd == NULL || what == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    *what = NONE;

    i = read_packet(fd, &buf);
    switch ( i ) {
    case -1:
	return(-1);
    case 1:
	/* Syntax error, EOF or packet without length (compressed) */
	return(1);
    case 0:
	break;
    default:
	/*assert0);*/
	return(-1);
    }
    length = packet_length(buf);
    if ( length < 0 )
	return (length);
    ret = get_keyentry_from_buf(buf, length, what, sig,
				pubkey, uid, seckey, trust, &j);
    free(buf);
    return(ret);
}

/*
 * The idea is that you can scan a file of secret keys, obtaining
 * one for each call.  The return codes are inherited from
 * get_keyentry_from_file: 0 on success, 1 on EOF -1 on error.
 */

int
get_seckey_from_file(FILE 	*keyring,
		     PGPseckey_t **key)
{
    res_t	what;
    PGPseckey_t *tmpkey;
    int		ret;

    if ( keyring == NULL || key == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(-1);
    }
    while ( ( ret = get_keyentry_from_file(keyring, &what, NULL, NULL,
					   NULL, &tmpkey, NULL)) == 0)
    {
	if ( what == SECKEY ) {
	    *key = tmpkey;
	    return(0);
	}
	else
	    continue;
    }

    /* no seckey */
    return(ret);
}
