/*
 *   derkey.c
 *	Routines to translate to R_RSA_{PUBLIC,PRIVATE}_KEY
 *	from ASN.1 DER encodings.
 */

#include <stdio.h>
#include "global.h"
#include "rsaref.h"
#include "ripem.h"
#include "derkeypr.h"

/* Error return codes */
#define DK_ERR_FORMAT	-1		/* Badly formatted DER string */
#define	DK_ERR_ALG	-2		/* Unrecognized algorithm */

/* DER class/tag codes */
#define	DER_INT		0x02
#define	DER_BITSTRING	0x03
#define	DER_OCTETSTRING	0x04
#define	DER_NULL	0x05
#define	DER_OBJID	0x06
#define	DER_SEQ		0x30
#define	DER_SET		0x31


/* Alg ID - rsa - {2, 5, 8, 1, 1}*/
static unsigned char rsa_alg[] = { DER_OBJID, 4, 2*40+5, 0x08, 0x01, 0x01 };

/* rsaEncryption data structure, with algorithm {1 2 840 113549 1 1 1} and
 * NULL parameter
 */
static unsigned char rsaEnc_alg[] = { DER_SEQ, 13, DER_OBJID, 9,
			1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
			DER_NULL, 0 };

/* Version = 0 */
static unsigned char version[] = { DER_INT, 1, 0 };


/* Return the number of bits in large unsigned n */
static unsigned int
largeunsignedbits (n, nsize)
unsigned char		*n;
unsigned int		nsize;
{
    unsigned int	i, j;

    for (i=0; i<nsize && n[i]==0; ++i)
	;		/* Intentionally empty */
    if (i==nsize)
	return 0;
    j = n[i];
    i = ((nsize-i-1) << 3) + 1;
    while ((j>>=1))
	++i;
    return i;
}


/* Read the tag and length information from a DER string.  Advance
 * der to past the length.  Return negative on error.
 */
static int			/* Return < 0 on error */
gettaglen(tag, len, p)
UINT2		*tag;
unsigned int		*len;
unsigned char		**p;
{
    UINT2	t;
    unsigned int	l;
    int		c;
    int		n;

    t = *(*p)++;
    if (!t)
	return -1;
    c = *(*p)++;
    if (c & 0x80) {
	if (!(n = c & 0x7f))
	    return -1;
	l = 0;
	if (n > sizeof(unsigned int))
	    return -1;
	while (n--) {
	    c = *(*p)++;
	    l = (l<<8) + c;
	}
    } else {
	l = c & 0x7f;
    }
    *tag = t;
    *len = l;
    return 0;
}


/* Check DER byte string against literal data to make sure they match.
 * Return negative on error.  Advance der pointer p.
 */
static int
chkdata (p, s, len)
unsigned char		**p;
unsigned char		*s;
unsigned int		len;
{
    while (len--)
	if (*(*p)++ != *s++)
	    return -1;
    return 0;
}


/* Read an integer from DER byte string.  It must be small enough to
 * fit in an int.  Return negative on error.
 */
static int
getsmallint (n, p)
unsigned int		*n;
unsigned char		**p;
{
    UINT2	tag;
    unsigned int	len;
    unsigned int	v;

    if (gettaglen(&tag,&len,p) < 0)
	return -1;
    if (tag != DER_INT)
	return -1;
    if (len > sizeof(int)  ||  len == 0)
	return -1;
    v = 0;
    while (len--)
	v = (v << 8) + *(*p)++;
    *n = v;
    return 0;
}


/* Read a large integer from the DER byte string pointed to by p.
 * Advance p as we read.  Put it into buffer n, of length nsize,
 * right justified.  Clear the rest of n.
 * Return negative on error.
 */
static int
getlargeunsigned (n, nsize, p)
unsigned char		*n;
unsigned int		nsize;
unsigned char		**p;
{
    UINT2	tag;
    unsigned int	len;

   if (gettaglen(&tag,&len,p) < 0)
		return -1;
   if (tag != DER_INT)
		return -1;
	 /* Skip a leading zero  in the input; it may overflow the output
	  * buffer if the large unsigned is just the same size as the output buffer.
	  */
	if(! **p) {
	 	(*p)++;
		len--;
	}
   if (len > nsize)
		return -1;
   nsize -= len;
   while (nsize--)
		*n++ = 0;
   while (len--)
		*n++ = *(*p)++;
   return 0;
}



/*
 *	Beginning of public entry points for this module
 */



/*   int DERToPubKey (der, key)
 *	Translate the byte string DER, in ASN.1 syntax using the
 *	Distinguished Encoding Rules, into RSA public key.
 *	Return 0 on success, nonzero on error.
 */
int				/* 0 for OK, nonzero on error */
DERToPubKey (der, key)
unsigned char	*der;
R_RSA_PUBLIC_KEY	*key;
{
    UINT2		tag;
    unsigned int		len;
    unsigned int		bits;
    unsigned char		*der1, *der2;

    if (gettaglen(&tag,&len,&der) < 0)
	return DK_ERR_FORMAT;
    if (tag != DER_SEQ)
	return DK_ERR_FORMAT;
    der1 = der + len;		/* Position of end of string */
    if (gettaglen(&tag, &len, &der) < 0)
	return DK_ERR_FORMAT;
    if (tag != DER_SEQ)
	return DK_ERR_FORMAT;
    der2 = der + len;		/* Position of end of alg info */
    if (chkdata(&der, rsa_alg, (unsigned int)sizeof(rsa_alg)) < 0)
	return DK_ERR_ALG;
    if (getsmallint(&bits, &der) < 0)
	return DK_ERR_FORMAT;
    key->bits = (int)bits;
    if (der != der2)		/* Check end of alg info */
	return DK_ERR_FORMAT;
    if (gettaglen(&tag, &len, &der) < 0)
	return DK_ERR_FORMAT;
    if (tag != DER_BITSTRING)
	return DK_ERR_FORMAT;
    if (der + len != der1)	/* Should also be end of string */
	return DK_ERR_FORMAT;
    if (*der++ != 0)		/* Bitstring must be a mult of 8 bits */
	return DK_ERR_FORMAT;
    if (gettaglen(&tag, &len, &der) < 0)
	return DK_ERR_FORMAT;
    if (tag != DER_SEQ)
	return DK_ERR_FORMAT;
    if (der + len != der1)	/* Should also be end of string */
	return DK_ERR_FORMAT;
    if (getlargeunsigned (key->modulus, (unsigned int)sizeof(key->modulus), &der) < 0)
	return DK_ERR_FORMAT;
    if (getlargeunsigned (key->exponent, (unsigned int)sizeof(key->exponent),&der) < 0)
	return DK_ERR_FORMAT;
    if (der != der1)		/* Check end of string */
	return DK_ERR_FORMAT;
    return 0;
}

/*   int DERToPrivKey (der, key)
 *	Translate the byte string DER, in ASN.1 syntax using the
 *	Distinguished Encoding Rules, into RSA private key.
 *	Return 0 on success, nonzero on error.
 */
int				/* 0 for OK, nonzero on error */
DERToPrivKey (der, key)
unsigned char			*der;
R_RSA_PRIVATE_KEY	*key;
{
    UINT2		tag;
    unsigned int		len;
    unsigned char		*der1;

	R_memset((POINTER)key,0,sizeof *key);

    if (gettaglen(&tag,&len,&der) < 0)
	return DK_ERR_FORMAT;
    if (tag != DER_SEQ)
	return DK_ERR_FORMAT;
    der1 = der + len;		/* Position of end of string */
    if (chkdata(&der, version, (unsigned int)sizeof(version)) < 0)
	return DK_ERR_ALG;
    if (chkdata(&der, rsaEnc_alg, (unsigned int)sizeof(rsaEnc_alg)) < 0)
	return DK_ERR_ALG;
    if (gettaglen(&tag, &len, &der) < 0)
	return DK_ERR_FORMAT;
    if (tag != DER_OCTETSTRING)
	return DK_ERR_FORMAT;
    if (der+len != der1)	/* Should match end of string */
	return DK_ERR_FORMAT;
    if (gettaglen(&tag,&len,&der) < 0)
	return DK_ERR_FORMAT;
    if (tag != DER_SEQ)
	return DK_ERR_FORMAT;
    if (der+len != der1)	/* Should match end of string */
	return DK_ERR_FORMAT;
    if (chkdata(&der, version, (unsigned int)sizeof(version)) < 0)
	return DK_ERR_ALG;
    if (getlargeunsigned (key->modulus, (unsigned int)sizeof(key->modulus), &der) < 0)
	return DK_ERR_FORMAT;
    if (getlargeunsigned (key->publicExponent,
				(unsigned int)sizeof(key->publicExponent), &der) < 0)
	return DK_ERR_FORMAT;
    if (getlargeunsigned (key->exponent, (unsigned int)sizeof(key->exponent),&der) < 0)
	return DK_ERR_FORMAT;
    if (getlargeunsigned (key->prime[0], (unsigned int)sizeof(key->prime[0]),&der) < 0)
	return DK_ERR_FORMAT;
    if (getlargeunsigned (key->prime[1], (unsigned int)sizeof(key->prime[1]),&der) < 0)
	return DK_ERR_FORMAT;
    if (getlargeunsigned (key->primeExponent[0],
				(unsigned int)sizeof(key->primeExponent[0]), &der) < 0)
	return DK_ERR_FORMAT;
    if (getlargeunsigned (key->primeExponent[1],
				(unsigned int)sizeof(key->primeExponent[1]), &der) < 0)
	return DK_ERR_FORMAT;
    if (getlargeunsigned (key->coefficient,
				(unsigned int)sizeof(key->coefficient), &der) < 0)
	return DK_ERR_FORMAT;
    if (der != der1)		/* Check end of string */
	return DK_ERR_FORMAT;
    /* This info isn't in the DER format, so we have to calculate it */
    key->bits = (int)largeunsignedbits (key->modulus,
						 (unsigned int)sizeof(key->modulus));
    return 0;
}

/* Data structure specifying "algorithm=pbeWithMD5AndDES-CBC"
 * for encoding of encrypted private key.
 * Decodes to OBJECT_ID = 1 2 840 113549 1 5 3
 */
static unsigned char pbeWithMD5AndDES_CBC[] = { DER_OBJID, 9,
			1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x05, 0x03 };

/*--- function int DERToEncryptedPrivKey --------------------------
 *
 *	Translate the byte string DER, in ASN.1 syntax using the
 *	Distinguished Encoding Rules, into encrypted RSA private key.
 *	Return 0 on success, nonzero on error.
 *
 * Encrypted key encoding looks like this:
 *
 *	Sequence
 * 	Sequence                           # encryption algorithm
 *			Object ID 1 2 840 113549 1 5 3  # algorithm MD5AndDES-CBC
 *			Sequence                        # algorithm parameters:
 *				Octet string, 8 bytes long   # salt
 *			   Integer                      # iteration count
 *		Octet string							  # encrypted data
 */
int				/* 0 for OK, nonzero on error */
DERToEncryptedPrivKey (der,maxLen, digestAlgorithm,salt,iterationCount,encBytes,encLen)
unsigned char			*der;
unsigned int maxLen;
int         *digestAlgorithm;
unsigned char *salt;
unsigned int *iterationCount;
unsigned char *encBytes;
unsigned int  *encLen;

{
   UINT2		tag;
   unsigned int		len;
   unsigned char		*der_end;

	/* Check first Sequence */
   if (gettaglen(&tag,&len,&der) < 0)
		return DK_ERR_FORMAT;
   if (tag != DER_SEQ)
		return DK_ERR_FORMAT;
   der_end = der + len;		/* Position of end of string */

	/* Check second Sequence */
	if(gettaglen(&tag,&len,&der) < 0)
		return DK_ERR_FORMAT;
	if(tag != DER_SEQ)
		return DK_ERR_FORMAT;

	/* Check algorithm */
   if (chkdata(&der,pbeWithMD5AndDES_CBC, (unsigned int)sizeof(pbeWithMD5AndDES_CBC)) < 0)
		return DK_ERR_ALG;
	*digestAlgorithm = DA_MD5;

	/* Check Sequence of algorithm parameters. */
	if(gettaglen(&tag,&len,&der) < 0)
		return DK_ERR_FORMAT;
	if(tag != DER_SEQ)
		return DK_ERR_FORMAT;

	/* Fetch salt */
	if(gettaglen(&tag,&len,&der) < 0)
		return DK_ERR_FORMAT;
	if(tag != DER_OCTETSTRING)
		return DK_ERR_FORMAT;
	if(len != 8)
		return DK_ERR_FORMAT;
	R_memcpy(salt,der,8);
	der += 8;

	/* Fetch iteration count */

	if(getsmallint(iterationCount,&der) < 0)
		return DK_ERR_FORMAT;

   /* Fetch encrypted private key */
   if (gettaglen(&tag, &len, &der) < 0)
		return DK_ERR_FORMAT;
   if (tag != DER_OCTETSTRING)
		return DK_ERR_FORMAT;
   if (der+len != der_end)	/* Should match end of string */
		return DK_ERR_FORMAT;

	if(len > maxLen)
		return DK_ERR_FORMAT;
	R_memcpy(encBytes,der,len);
	*encLen = len;

	return 0;
}
