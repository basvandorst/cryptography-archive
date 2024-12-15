/*
 *   keyder.c
 *	Routines to translate from R_RSA_{PUBLIC,PRIVATE}_KEY
 *	to ASN.1 DER encodings.
 */

#include <stdio.h>
#include "global.h"
#include "rsaref.h"
#include "ripem.h"
#include "keyderpr.h"

#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

static void put_der_len P((unsigned char **p , unsigned int len ));
static void put_der_data P((unsigned char **p , unsigned char *dat , unsigned int len ));
static void put_der_small_int P((unsigned char **p , unsigned int n ));
static void put_der_large_unsigned P((unsigned char **p , unsigned char *n , unsigned int nsize , unsigned int len ));

#undef P

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


/* Return the number of bytes taken for the DER-encoding of the data
 * structure of specified length.  This includes the bytes themselves,
 * the 1 byte for the DER class and tag, and the variable number of bytes
 * for the length encoding.
 */
static unsigned int
der_len (len)
unsigned int		len;
{
    if (len < 0x80)
		return 2+len;
    if (len < 0x100)
		return 3+len;
    if ((long unsigned int)len < 0x10000)
		return 4+len;
    if ((long unsigned int)len < 0x1000000)
		return 5+len;
    return 6+len;
}


/* Return the number of bytes for the DER-encoding of the specified small
 * signed integer.  Do not include the type byte and the length bytes.
 */
static unsigned int
len_small_int (n)
unsigned int			n;
{
    if ((((n&0xff)^0x80)-0x80) == n)
	return 1;
    else if ((((n&0xffff)^0x8000)-0x8000) == n)
	return 2;
    else if ((((n&0xffffffL)^0x800000L)-0x800000L) == n)
	return 3;
    else
	return 4;
}
 
/* Return the number of bytes for the DER-encoding of the specified large
 * unsigned integer.  Do not include the type byte and the length bytes.
 */
static unsigned int
len_large_unsigned (n, nsize)
unsigned char		*n;
unsigned int		nsize;
{
    unsigned int		i;

    for (i=0; i<nsize && n[i]==0; ++i)
	;	/* Intentionally blank */
    if (i == nsize)
		return 1;	/* Value is 0 */
    if (n[i]&0x80)
		return nsize-i + 1;	/* Need an extra byte so doesn't sign extend */
    else
		return nsize-i;
}


/* Output DER encoding for length */
static void
put_der_len (p, len)
unsigned char		**p;
unsigned int		len;
{
    if (len < 0x80) {
	*(*p)++ = len;
    } else if (len < 0x100) {
	*(*p)++ = 0x81;
	*(*p)++ = len;
    } else if ((long unsigned int) len < 0x10000L) {
	*(*p)++ = 0x82;
	*(*p)++ = (len>>8);
	*(*p)++ = len&0xff;
    } else if ((long unsigned int) len < 0x1000000L) {
	*(*p)++ = 0x83;
	*(*p)++ = (len>>16)&0xff;
	*(*p)++ = (len>>8)&0xff;
	*(*p)++ = len&0xff;
    } else {
	*(*p)++ = 0x84;
	*(*p)++ = (len>>24)&0xff;
	*(*p)++ = (len>>16)&0xff;
	*(*p)++ = (len>>8)&0xff;
	*(*p)++ = len&0xff;
    }
}


static void
put_der_data (p, dat, len)
unsigned char		**p;
unsigned char		*dat;
unsigned int		len;
{
    while (len--) {
	*(*p)++ = *dat++;
    }
}


static void
put_der_small_int (p, n)
unsigned char		**p;
unsigned int		n;
{
    unsigned int	len;

    *(*p)++ = DER_INT;
    len = len_small_int(n);
    put_der_len (p, len);
    while (len--)
	*(*p)++ = (n >> (len*8)) & 0xff;
}


/* Output the DER encoding for the large unsigned integer n, which is in
 * an array of size nsize, but whose length (from len_large_unsigned) is
 * len.
 */
static void
put_der_large_unsigned (p, n, nsize, len)
unsigned char		**p;
unsigned char		*n;
unsigned int		nsize;
unsigned int		len;
{
   *(*p)++ = DER_INT;
   put_der_len (p, len);
	
	/* Catch the boundary condition in which the integer entirely fills
	 * the output buffer, and has the high bit set.
	 * In this case, we put out an explicit zero and compensate for this
	 * zero, which was allowed for in len_large_unsigned.
	 */
	if(len==nsize+1 && (0x80 & *n)) {
	 	*(*p)++ = 0;
		len--;
	}
	/* Skip past leading zeros. */
   n += nsize-len;
   while (len--)
		*(*p)++ = *n++;
}



/*
 *	Beginning of public entry points for this module
 */



/* function  int PubKeyToDERLen (key)
 *
 *	Return the length in bytes of the DER translation of the RSA
 *	public key given.
 */
unsigned int
PubKeyToDERLen (key)
R_RSA_PUBLIC_KEY	*key;
{
    unsigned int		alglen, modexplen, keybitlen, keylen, tlen;
    unsigned int		modlen, explen;
    unsigned int		bits = key->bits;

    alglen = sizeof(rsa_alg) + der_len(len_small_int(bits));
    modlen = len_large_unsigned(key->modulus,sizeof(key->modulus));
    explen = len_large_unsigned(key->exponent,sizeof(key->exponent));
    modexplen = der_len(modlen) + der_len(explen);
    keybitlen = der_len (modexplen);
    keylen = 1 + keybitlen;	/* Padding byte for bit string */
    tlen = der_len(alglen) + der_len(keylen);
    return der_len(tlen);
}

/*   int pubkeytoder (key, der, derlen)
 *	Translate RSA public key using Distinguished Encoding Rules into
 *	a byte string.  Return the string in der and the length of the
 *	string in derlen.
 *	Return 0 on success, nonzero on failure.
 */
int				/* Return < 0 if error, 0 if OK */
PubKeyToDER (key, der, derlen)
R_RSA_PUBLIC_KEY	*key;
unsigned char			*der;
unsigned int			*derlen;
{
    unsigned int		alglen, modexplen, keybitlen, keylen, tlen;
    unsigned int		modlen, explen;
    unsigned int		bits = key->bits;

    alglen = sizeof(rsa_alg) + der_len(len_small_int(bits));
    modlen = len_large_unsigned(key->modulus,(unsigned int)sizeof(key->modulus));
    explen = len_large_unsigned(key->exponent,(unsigned int)sizeof(key->exponent));
    modexplen = der_len(modlen) + der_len(explen);
    keybitlen = der_len (modexplen);
    keylen = 1 + keybitlen;	/* Padding byte for bit string */
    tlen = der_len(alglen) + der_len(keylen);
    *derlen = der_len(tlen);
    *der++ = DER_SEQ;
    put_der_len (&der, tlen);
    /* Now output algorithm info */
    *der++ = DER_SEQ;
    put_der_len (&der, alglen);
    put_der_data (&der, rsa_alg, (unsigned int)sizeof(rsa_alg));
    put_der_small_int(&der, bits);
    *der++ = DER_BITSTRING;
    put_der_len (&der, keylen);
    *der++ = 0;			/* Padding for key bits */
    *der++ = DER_SEQ;
    put_der_len (&der, modexplen);
    put_der_large_unsigned(&der, key->modulus,
				 (unsigned int)sizeof(key->modulus), modlen);
    put_der_large_unsigned (&der, key->exponent,
				 (unsigned int)sizeof(key->exponent), explen);
    return 0;
}

/*   int privkeytoderlen (key)
 *	Return the length in bytes of the DER translation of the RSA
 *	public key given.
 */
unsigned int
PrivKeyToDERLen (key)
R_RSA_PRIVATE_KEY	*key;
{
    unsigned int	alglen, modlen, pexplen, explen, p1len, p2len;
    unsigned int	pexp1len, pexp2len, coeflen, pklen, tlen;

    alglen = sizeof (rsaEnc_alg) + sizeof (version);
    modlen = len_large_unsigned(key->modulus,(unsigned int)sizeof(key->modulus));
    pexplen = len_large_unsigned(key->publicExponent,
					(unsigned int)sizeof(key->publicExponent));
    explen = len_large_unsigned(key->exponent,(unsigned int)sizeof(key->exponent));
    p1len = len_large_unsigned(key->prime[0],(unsigned int)sizeof(key->prime[0]));
    p2len = len_large_unsigned(key->prime[1],(unsigned int)sizeof(key->prime[1]));
    pexp1len = len_large_unsigned(key->primeExponent[0],
					(unsigned int)sizeof(key->primeExponent[0]));
    pexp2len = len_large_unsigned(key->primeExponent[1],
					(unsigned int)sizeof(key->primeExponent[1]));
    coeflen = len_large_unsigned(key->coefficient,
					(unsigned int)sizeof(key->coefficient));
    pklen = sizeof(version) + der_len(modlen) + der_len(pexplen) +
		der_len(explen) + der_len(p1len) + der_len(p2len) +
		der_len(pexp1len) + der_len(pexp2len) + der_len(coeflen);
    tlen = alglen + der_len(der_len(pklen));
    return der_len(tlen);
}


/*   int privkeytoder (key, der, derlen)
 *	Translate RSA private key using Distinguished Encoding Rules into
 *	a byte string.  Return the string in der and the length of the
 *	string in derlen.
 *	Return 0 on success, nonzero on failure.
 */
int				/* Return < 0 if error, 0 if OK */
PrivKeyToDER (key, der, derlen)
R_RSA_PRIVATE_KEY	*key;
unsigned char			*der;
unsigned int			*derlen;
{
    unsigned int	alglen, modlen, pexplen, explen, p1len, p2len;
    unsigned int	pexp1len, pexp2len, coeflen, pklen, tlen;

    alglen = sizeof (rsaEnc_alg) + sizeof (version);
    modlen = len_large_unsigned(key->modulus,(unsigned int)sizeof(key->modulus));
    pexplen = len_large_unsigned(key->publicExponent,
					(unsigned int)sizeof(key->publicExponent));
    explen = len_large_unsigned(key->exponent,(unsigned int)sizeof(key->exponent));
    p1len = len_large_unsigned(key->prime[0],(unsigned int)sizeof(key->prime[0]));
    p2len = len_large_unsigned(key->prime[1],(unsigned int)sizeof(key->prime[1]));
    pexp1len = len_large_unsigned(key->primeExponent[0],
					(unsigned int)sizeof(key->primeExponent[0]));
    pexp2len = len_large_unsigned(key->primeExponent[1],
					(unsigned int)sizeof(key->primeExponent[1]));
    coeflen = len_large_unsigned(key->coefficient,
					(unsigned int)sizeof(key->coefficient));
    pklen = sizeof(version) + der_len(modlen) + der_len(pexplen) +
		der_len(explen) + der_len(p1len) + der_len(p2len) +
		der_len(pexp1len) + der_len(pexp2len) + der_len(coeflen);
    tlen = alglen + der_len(der_len(pklen));
    *derlen = der_len(tlen);
    *der++ = DER_SEQ;
    put_der_len (&der, tlen);
    put_der_data (&der, version, (unsigned int)sizeof(version));
    put_der_data (&der, rsaEnc_alg, (unsigned int)sizeof(rsaEnc_alg));
    *der++ = DER_OCTETSTRING;
    put_der_len (&der, der_len(pklen));
    /* Now the RSAPrivateKey */
    *der++ = DER_SEQ;
    put_der_len (&der, pklen);
    put_der_data (&der, version, (unsigned int)sizeof(version));
    put_der_large_unsigned(&der, key->modulus,
				(unsigned int)sizeof(key->modulus), modlen);
    put_der_large_unsigned(&der, key->publicExponent,
				(unsigned int)sizeof(key->publicExponent), pexplen);
    put_der_large_unsigned(&der, key->exponent,
				(unsigned int)sizeof(key->exponent), explen);
    put_der_large_unsigned(&der, key->prime[0],
				(unsigned int)sizeof(key->prime[0]), p1len);
    put_der_large_unsigned(&der, key->prime[1], 
				(unsigned int)sizeof(key->prime[1]), p2len);
    put_der_large_unsigned(&der, key->primeExponent[0],
				(unsigned int)sizeof(key->primeExponent[0]), pexp1len);
    put_der_large_unsigned(&der, key->primeExponent[1],
				(unsigned int)sizeof(key->primeExponent[1]), pexp2len);
    put_der_large_unsigned(&der, key->coefficient,
				(unsigned int)sizeof(key->coefficient), coeflen);
    return 0;
}

/* Data structure specifying "algorithm=pbeWithMD5AndDES-CBC"
 * for encoding of encrypted private key.
 * Decodes to OBJECT_ID = 1 2 840 113549 1 5 3
 */
static unsigned char pbeWithMD5AndDES_CBC[] = { DER_OBJID, 9,
			1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x05, 0x03 };


/*--- function EncryptedPrivKeyToDERLen ---------------------------
 *
 *	Return the length in bytes of the DER translation of the
 *	encrypted public key given.
 */
unsigned int
EncryptedPrivKeyToDERLen (iterationCount,encLen)
unsigned int iterationCount;
unsigned int encLen;
{
   unsigned int alg_len, salt_len, iter_len, param_len;
   unsigned int octet_len, enc_seq_len, total_len;

   alg_len = sizeof(pbeWithMD5AndDES_CBC);

   salt_len = der_len(8);
   iter_len = der_len(len_small_int(iterationCount));
	param_len = der_len(salt_len+iter_len);

	octet_len = der_len(encLen);

	enc_seq_len = der_len(alg_len+param_len);

	total_len = der_len(enc_seq_len+octet_len);
   return der_len(total_len);
}



/*--- function EncryptedPrivKeyToDER ---------------------------
 *
 * Encode an encrypted RSA private key into DER form.
 *
 *	Return 0 on success, nonzero on failure.
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
int				/* Return < 0 if error, 0 if OK */
EncryptedPrivKeyToDER (salt,iterationCount,encBytes,encLen, der, derlen)
unsigned char *salt;
unsigned int iterationCount;
unsigned char *encBytes;
unsigned int  encLen;
unsigned char			*der;
unsigned int			*derlen;
{

   unsigned int alg_len, salt_len, iter_len, param_len;
   unsigned int octet_len, enc_seq_len, tlen;

   alg_len = sizeof (pbeWithMD5AndDES_CBC);

   salt_len = der_len(8);
   iter_len = der_len(len_small_int(iterationCount));
	param_len = der_len(salt_len+iter_len);

	octet_len = der_len(encLen);

	enc_seq_len = der_len(alg_len+param_len);

	tlen = enc_seq_len+octet_len;


	/* Output highest level sequence indicator */
   *derlen = der_len(tlen);
   *der++ = DER_SEQ;
   put_der_len (&der, tlen);

	/* Output sequence indicator for encryption Algorithm (which
	 * includes algorithm + parameters.
	 */
	*der++ = DER_SEQ;
	put_der_len(&der,alg_len+param_len);

	/* Output encryption algorithm */
   put_der_data (&der, pbeWithMD5AndDES_CBC, alg_len );

   /* Output sequence for parameters */
	*der++ = DER_SEQ;
	put_der_len(&der,salt_len + iter_len);

	/* Output salt (first parameter) */
	*der++ = DER_OCTETSTRING;
	put_der_len(&der,8);
   put_der_data (&der, salt,8);

	/* Output iteration count (second parameter) */
	put_der_small_int(&der,iterationCount);

	/* Output the encrypted key */
   *der++ = DER_OCTETSTRING;
	put_der_len(&der,encLen);
	put_der_data(&der,encBytes,encLen);

   return 0;
}
