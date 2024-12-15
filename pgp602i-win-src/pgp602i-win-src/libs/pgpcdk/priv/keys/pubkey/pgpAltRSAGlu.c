/*
 * pgpAltRSAGlu.c - Interface to RSAREF Cryptographic toolkit from
 * RSA Data Security, Inc.
 *
 * This is an alternative to pgpRSAGlue.c, and should be used if the
 * use of the RSAREF library is required for legal reasons.
 *
 * This file uses RSAREF to perform the actual encryption and decryption.
 * It must be linked with the RSAREF library (rsaref.a, rsaref.lib,
 * or whatever it's called on your system) to function.
 *
 * This code only accepts PKCS-style padding.  Very old versions of PGP
 * used a different padding style, which will not be compatible with
 * this module.
 *
 * $Id: pgpAltRSAGlu.c,v 1.13.2.1 1998/10/14 22:44:35 hal Exp $
 */
 
/*
 * An alternative version of this module is used if RSAREF is NOT needed.
 * This entire source file is under the control of the following conditional:
 */

#include "pgpSDKBuildFlags.h"

#if PGP_USERSAREF	/* [ */

/* Default to using our MPI library */
#ifndef PGP_USEMPILIBWITHRSAREF
#define PGP_USEMPILIBWITHRSAREF 1
#endif /* PGP_USEMPILIBWITHRSAREF */

 
#include "pgpConfig.h"

#include "pgpDebug.h"
#include "pgpRSAGlue.h"
#include "pgpKeyMisc.h"
#include "bn.h"
#include "pgpErrors.h"
#include "pgpRandomX9_17.h"
#include "pgpMem.h"


/*
 * If you're using a legally encumbered library this will be
 * printed in the program banner.
 */
char const banner_legalese[] = "\
Uses the RSAREF(tm) Toolkit, which is copyright RSA Data Security, Inc.\n\
Distributed by the Massachusetts Institute of Technology.\n";

/* Include files from RSAREF */
#include <global.h>
#include <rsaref.h>
#include <rsa.h>

/*
 * The functions we call in rsa.h are:
 *  RSAPublicEncrypt
 *  RSAPrivateEncrypt
 *  RSAPublicDecrypt
 *  RSAPrivateDecrypt
 */


/*
 * This returns TRUE if the key is too big, returning the
 * maximum number of bits that the library can accept.
 */
int
rsaKeyTooBig(RSApub const *pub, RSAsec const *sec)
{
	unsigned maxsize = MAX_RSA_MODULUS_BITS;
	if (pub) {
		if (bnBits(&pub->n) > maxsize)
			return maxsize;
		if (bnBits(&pub->e) > maxsize)
			return maxsize;
	}
	if (sec) {
		if (bnBits(&sec->d) > maxsize)
			return maxsize;
		if (bnBits(&sec->p) > MAX_RSA_PRIME_BITS)
			return maxsize;
		if (bnBits(&sec->q) > MAX_RSA_PRIME_BITS)
			return maxsize;
		if (bnBits(&sec->u) > MAX_RSA_PRIME_BITS)
			return maxsize;
	}
	/* Else OK */
	return 0;
}

/* Initialize RSAREF pubkey structure from a RSApub. */
static void
rpubk_init(R_RSA_PUBLIC_KEY *rpubk, RSApub const *pub)
{
	pgpClearMemory(rpubk, sizeof (*rpubk));
	rpubk->bits = bnBits(&pub->n);
	bnExtractBigBytes(&pub->n, rpubk->modulus, 0, MAX_RSA_MODULUS_LEN);
	bnExtractBigBytes(&pub->e, rpubk->exponent, 0, MAX_RSA_MODULUS_LEN);
}

/* Initialize RSAREF privkey structure from a RSAsec. */
static void
rprivk_init(R_RSA_PRIVATE_KEY *rprivk, RSAsec const *sec,
	PGPMemoryMgrRef	mgr)
{
	BigNum dmodp, dmodq, tmp;

	pgpClearMemory(rprivk, sizeof (*rprivk));

	/* Calculate d mod p-1 and d mod q-1 */
	bnBegin(&dmodp, mgr, TRUE);
	bnBegin(&dmodq, mgr, TRUE);
	bnBegin(&tmp, mgr, TRUE);
	bnCopy(&tmp, &sec->p);
	bnSubQ(&tmp, 1);
	bnMod(&dmodp, &sec->d, &tmp);
	bnCopy(&tmp, &sec->q);
	bnSubQ(&tmp, 1);
	bnMod(&dmodq, &sec->d, &tmp);

	/* Fill in structure */
	rprivk->bits = bnBits(&sec->n);
	bnExtractBigBytes(&sec->n, rprivk->modulus, 0, MAX_RSA_MODULUS_LEN);
	bnExtractBigBytes(&sec->e, rprivk->publicExponent, 0,
			  MAX_RSA_MODULUS_LEN);
	bnExtractBigBytes(&sec->d, rprivk->exponent, 0, MAX_RSA_MODULUS_LEN);
	/* RSAREF uses convention p > q, the opposite of ours */
	bnExtractBigBytes(&sec->q, rprivk->prime[0], 0, MAX_RSA_PRIME_LEN);
	bnExtractBigBytes(&sec->p, rprivk->prime[1], 0, MAX_RSA_PRIME_LEN);
	bnExtractBigBytes(&dmodq, rprivk->primeExponent[0], 0,
			  MAX_RSA_PRIME_LEN);
	bnExtractBigBytes(&dmodp, rprivk->primeExponent[1], 0,
			  MAX_RSA_PRIME_LEN);
	bnExtractBigBytes(&sec->u, rprivk->coefficient, 0, MAX_RSA_PRIME_LEN);

	bnEnd(&dmodp);
	bnEnd(&dmodq);
	bnEnd(&tmp);
}

/*
 * Encrypt a buffer holding a session key with an RSA public key
 */
int
rsaPublicEncrypt(BigNum *bn, PGPByte const *in, unsigned len,
	RSApub const *pub, PGPRandomContext const *rc)
{
#ifndef PGP_DISABLE_RSAREF_ENCRYPT

	R_RSA_PUBLIC_KEY rpubk;
	R_RANDOM_STRUCT rrs;
	PGPByte buf[MAX_RSA_MODULUS_LEN];
	unsigned int bufsize;
	int err;


	/* Set the return number to 0 to start */
	(void)bnSetQ(bn, 0);

	/* Initialize RSAREF public key structure */
	rpubk_init(&rpubk, pub);

	/* Initialize RSAREF random numbers */
	R_RandomInit(&rrs);
	do {
		R_GetRandomBytesNeeded(&bufsize, &rrs);
		if (bufsize > sizeof(buf))
			bufsize = sizeof(buf);
		if (bufsize) {
			pgpRandomGetBytes(rc, buf, bufsize);
			R_RandomUpdate(&rrs, buf, bufsize);
		}
	} while (bufsize);

	/* Pad and encrypt */
	err = RSAPublicEncrypt(buf, &bufsize, (PGPByte *)in, len, &rpubk, &rrs);
	R_RandomFinal(&rrs);

	if (err) {
		memset(buf, 0, sizeof(buf));
		memset(&rpubk, 0, sizeof(rpubk));
		if (err==RE_LEN)
			return kPGPError_PublicKeyTooSmall;
		return kPGPError_UnknownError;
	}

	/* Return to bn format */
	bnInsertBigBytes(bn, buf, 0, bufsize);
	memset(buf, 0, sizeof(buf));
	memset(&rpubk, 0, sizeof(rpubk));

	return 0;

#else /* PGP_DISABLE_RSAREF_ENCRYPT */

	(void)in;
	(void)len;
	(void)pub;
	(void)rc;

	(void)bnSetQ(bn, 0);
	return kPGPError_PublicKeyUnimplemented;

#endif /* PGP_DISABLE_RSAREF_ENCRYPT */
}


/*
 * Do an RSA signing operation using the secret key
 */
int
rsaPrivateEncrypt(BigNum *bn, PGPByte const *in, unsigned len,
	RSAsec const *sec)
{
#ifndef PGP_DISABLE_RSAREF_SIGN

	R_RSA_PRIVATE_KEY rprivk;
	PGPByte buf[MAX_RSA_MODULUS_LEN];
	unsigned int bufsize;
	int err;
	PGPMemoryMgrRef	mgr	= bn->mgr;

	/* SET the return number to 0 to start */
	(void)bnSetQ(bn, 0);

	/* Initialize RSAREF private key structure */
	rprivk_init(&rprivk, sec, mgr);

	err = RSAPrivateEncrypt(buf, &bufsize, (PGPByte *)in, len, &rprivk);

	if (err) {
		memset(buf, 0, sizeof(buf));
		memset(&rprivk, 0, sizeof(rprivk));
		if (err==RE_LEN)
			return kPGPError_PublicKeyTooSmall;
		return kPGPError_UnknownError;
	}

	/* Return to bn format */
	bnInsertBigBytes(bn, buf, 0, bufsize);
	memset(buf, 0, sizeof(buf));
	memset(&rprivk, 0, sizeof(rprivk));
	
	return 0;

#else /* PGP_DISABLE_RSAREF_SIGN */

	(void)in;
	(void)len;
	(void)sec;

	(void)bnSetQ(bn, 0);
	return kPGPError_PublicKeyUnimplemented;

#endif /* PGP_DISABLE_RSAREF_SIGN */
}

/*
 * These destroy (actually, replace with a decrypted version) the
 * input bignum bn.
 *
 * Performs an RSA signature check.  Returns a prefix of the unwrapped
 * data in the given buf.  Returns the length of the untruncated
 * data, which may exceed "len". Returns <0 on error.
 */
int
rsaPublicDecrypt(PGPByte *outbuf, unsigned len, BigNum *bn,
	RSApub const *pub)
{
#ifndef PGP_DISABLE_RSAREF_VERIFY

	R_RSA_PUBLIC_KEY rpubk;
	PGPByte buf[MAX_RSA_MODULUS_LEN];
	unsigned int bufsize;
	int err;

	/* Initialize RSAREF public key structure */
	rpubk_init(&rpubk, pub);

	bufsize = (bnBits(bn) + 7) / 8;
	if (bufsize > sizeof(buf))
		bufsize = sizeof(buf);
	bnExtractBigBytes(bn, buf, 0, bufsize);

	err = RSAPublicDecrypt(buf, &bufsize, buf, bufsize, &rpubk);
	if (err) {
		memset(buf, 0, sizeof(buf));
		memset(&rpubk, 0, sizeof(rpubk));
		if (err==RE_LEN)
			return kPGPError_PublicKeyTooSmall;
		return kPGPError_UnknownError;
	}

	/* Copy result to buffer */
	if (len > bufsize)
		len = bufsize;
	memcpy(outbuf, buf, len);

	memset(buf, 0, sizeof(buf));
	memset(&rpubk, 0, sizeof(rpubk));
	
	return bufsize;

#else /* PGP_DISABLE_RSAREF_VERIFY */

	(void)outbuf;
	(void)len;
	(void)bn;
	(void)pub;

	return kPGPError_PublicKeyUnimplemented;

#endif /* PGP_DISABLE_RSAREF_VERIFY */
}

/*
 * Performs an RSA decryption.  Returns a prefix of the unwrapped
 * data in the given buf.  Returns the length of the untruncated
 * data, which may exceed "len". Returns <0 on error.
 */
int
rsaPrivateDecrypt(PGPByte *outbuf, unsigned len, BigNum *bn,
	RSAsec const *sec)
{
#ifndef PGP_DISABLE_RSAREF_DECRYPT

	R_RSA_PRIVATE_KEY rprivk;
	PGPByte buf[MAX_RSA_MODULUS_LEN];
	unsigned int bufsize;
	int err;
	PGPMemoryMgrRef	mgr	= bn->mgr;

	/* Initialize RSAREF private key structure */
	rprivk_init(&rprivk, sec, mgr);

	bufsize = (bnBits(bn) + 7) / 8;
	if (bufsize > sizeof(buf))
		bufsize = sizeof(buf);
	bnExtractBigBytes(bn, buf, 0, bufsize);

	err = RSAPrivateDecrypt(buf, &bufsize, buf, bufsize, &rprivk);

	if (err) {
		memset(buf, 0, sizeof(buf));
		memset(&rprivk, 0, sizeof(rprivk));
		if (err==RE_LEN)
			return kPGPError_PublicKeyTooSmall;
		return kPGPError_UnknownError;
	}

	/* Copy result to buffer */
	if (len > bufsize)
		len = bufsize;
	memcpy(outbuf, buf, len);

	memset(buf, 0, sizeof(buf));
	memset(&rprivk, 0, sizeof(rprivk));
	
	return bufsize;

#else /* PGP_DISABLE_RSAREF_DECRYPT */

	(void)outbuf;
	(void)len;
	(void)bn;
	(void)sec;

	return kPGPError_PublicKeyUnimplemented;

#endif /* PGP_DISABLE_RSAREF_DECRYPT */
}


/*
 * Optional code to replace RSAREF's NN_ModExp with code from the
 * BigNum library, which is much faster.
 * Note that we are still using RSAREF, but we are just using a faster
 * modulo exponentiation routine.
 * If you comment out the following block of code, you get a (much slower)
 * pure RSAREF version.
 */
#if PGP_USEMPILIBWITHRSAREF	/* [ */

typedef PGPUInt32 NN_DIGIT;

/*
 * Within the RSAREF NN module, values are stored in 32 bit units,
 * in little-endian order.  Unfortunately this does not map well onto
 * our bignum format and we have to do some shuffling.
 */
/* Convert an RSAREF NN number to a BigNum */
static void
nntobn(BigNum *bn, NN_DIGIT *nn, unsigned nnlen)
{
	PGPByte buf[MAX_RSA_MODULUS_LEN];
	PGPByte *bp = buf;
	unsigned i;

	pgpAssert (nnlen*4 <= MAX_RSA_MODULUS_LEN);
	for (i=0; i<nnlen; ++i) {
		PGPUInt32 v = nn[i];
		*bp++ = v         & 0xff;
		*bp++ = (v >>  8) & 0xff;
		*bp++ = (v >> 16) & 0xff;
		*bp++ = (v >> 24) & 0xff;
	}
	bnInsertLittleBytes(bn, buf, 0, nnlen*4);
}

/* Convert a BigNum to an RSAREF NN number */
static void
bntonn(BigNum *bn, NN_DIGIT *nn, unsigned nnlen)
{
	PGPByte buf[MAX_RSA_MODULUS_LEN];
	PGPByte *bp = buf;
	unsigned i;

	pgpAssert (nnlen*4 <= MAX_RSA_MODULUS_LEN);
	bnExtractLittleBytes(bn, buf, 0, nnlen*4);
	for (i=0; i<nnlen; ++i) {
		PGPUInt32 v = *bp++;
		v |= (*bp++) << 8;
		v |= (*bp++) << 16;
		v |= (*bp++) << 24;
		nn[i] = v;
	}
}

void
NN_ModExp (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned cDigits,
	   NN_DIGIT *d, unsigned dDigits);

/*
 * Calculate b to the c mod d, result in a.
 * This is called by the RSAREF library and provides higher performance
 * modular exponentiation.
 */
void
NN_ModExp (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, unsigned cDigits,
	   NN_DIGIT *d, unsigned dDigits)
{
	BigNum bnres, bnbase, bnexp, bnmod;
	PGPMemoryMgrRef bnmgr;

	(void) PGPNewMemoryMgr ( 0, &bnmgr);

	bnBegin(&bnres, bnmgr, TRUE);
	bnBegin(&bnbase, bnmgr, TRUE);
	bnBegin(&bnexp, bnmgr, TRUE);
	bnBegin(&bnmod, bnmgr, TRUE);

	nntobn(&bnbase, b, dDigits);
	nntobn(&bnexp, c, cDigits);
	nntobn(&bnmod, d, dDigits);
	
	bnExpMod(&bnres, &bnbase, &bnexp, &bnmod);

	bntonn(&bnres, a, dDigits);

	bnEnd(&bnres);
	bnEnd(&bnbase);
	bnEnd(&bnexp);
	bnEnd(&bnmod);
}

#endif	/* ] PGP_USEMPILIBWITHRSAREF */

#endif	/* ] PGP_USERSAREF */
