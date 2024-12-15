/*
 * pgpBSRSAGlue.c - Interface to BSafe Cryptographic toolkit from
 * RSA Data Security, Inc.
 *
 * This is an alternative to pgpRSAGlue.c, and should be used if the
 * use of the BSafe library is required for legal reasons.
 *
 * This file uses BSafe to perform the actual encryption and decryption.
 * It must be linked with the BSafe library to function.
 *
 * This code only accepts PKCS-style padding.  Very old versions of PGP
 * used a different padding style, which will not be compatible with
 * this module.
 *
 * $Id: pgpBSRSAGlue.c,v 1.5.2.1 1998/10/06 22:42:10 hal Exp $
 */
 
/*
 * An alternative version of this module is used if BSafe is NOT needed.
 * This entire source file is under the control of the following conditional:
 */

#include "pgpSDKBuildFlags.h"

#if PGP_USEBSAFEFORRSA	/* [ */

#if PGP_MACINTOSH
#include "string.h"
#endif

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
Uses the BSafe(tm) Toolkit, which is copyright RSA Data Security, Inc.\n";


/* Include files from BSafe */
#include "aglobal.h"
#include "bsafe.h"


/* Control what is linked in from BSafe */
static B_ALGORITHM_METHOD *RSA_CHOOSER[] = {
	&AM_RSA_ENCRYPT,
	&AM_RSA_CRT_DECRYPT,
	(B_ALGORITHM_METHOD *)NULL
};


/* Return a standard error code on BSAFE errors */
#define CHKERR(err) \
	if (err != 0) { \
		err = kPGPError_PublicKeyUnimplemented; \
		goto error; \
	}

/*
 * This returns TRUE if the key is too big, returning the
 * maximum number of bits that the library can accept.
 */
#define MAXSIZE 2048
int
rsaKeyTooBig(RSApub const *pub, RSAsec const *sec)
{
	if (pub) {
		if (bnBits(&pub->n) > MAXSIZE)
			return MAXSIZE;
	}
	if (sec) {
		if (bnBits(&sec->n) > MAXSIZE)
			return MAXSIZE;
	}
	return 0; /* OK */
}

/* Initialize BSafe pubkey structure from a RSApub. */
static int
rpubk_init(B_KEY_OBJ rpubk, RSApub const *pub,
	PGPMemoryMgrRef	mgr)
{
	A_RSA_KEY kdata;
	PGPByte *buf;
	PGPSize bufsize;
	int err;

	bufsize = bnBytes(&pub->n) + bnBytes(&pub->e);
	buf = PGPNewSecureData( mgr, bufsize, 0 );
	kdata.modulus.data = buf;
	kdata.modulus.len = bnBytes(&pub->n);
	kdata.exponent.data = buf + kdata.modulus.len;
	kdata.exponent.len = bnBytes(&pub->e);
	bnExtractBigBytes (&pub->n, kdata.modulus.data, 0,
						kdata.modulus.len);
	bnExtractBigBytes (&pub->e, kdata.exponent.data, 0,
						kdata.exponent.len);
	err = B_SetKeyInfo (rpubk, KI_RSAPublic, (POINTER)&kdata);
	pgpAssert (err == 0);
	pgpClearMemory (buf, bufsize);
	PGPFreeData (buf);
	return err;
}

/* Initialize BSafe privkey structure from a RSAsec. */
static int
rprivk_init(B_KEY_OBJ rprivk, RSAsec const *sec,
	PGPMemoryMgrRef	mgr)
{
	BigNum dmodp, dmodq, tmp;
	A_RSA_CRT_KEY kdata;
	PGPByte *buf;
	PGPSize bufsize;
	int err;

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

	bufsize = bnBytes(&sec->n) + bnBytes(&sec->q) + bnBytes(&sec->p)
			+ bnBytes(&dmodq) + bnBytes(&dmodp) + bnBytes(&sec->u);
	buf = PGPNewSecureData( mgr, bufsize, 0 );
	kdata.modulus.data = buf;
	kdata.modulus.len = bnBytes(&sec->n);
	kdata.prime[0].data = kdata.modulus.data + kdata.modulus.len;
	kdata.prime[0].len = bnBytes(&sec->q);
	kdata.prime[1].data = kdata.prime[0].data + kdata.prime[0].len;
	kdata.prime[1].len = bnBytes(&sec->p);
	kdata.primeExponent[0].data = kdata.prime[1].data
				+ kdata.prime[1].len;
	kdata.primeExponent[0].len = bnBytes(&dmodq);
	kdata.primeExponent[1].data = kdata.primeExponent[0].data
				+ kdata.primeExponent[0].len;
	kdata.primeExponent[1].len = bnBytes(&dmodp);
	kdata.coefficient.data = kdata.primeExponent[1].data
				+ kdata.primeExponent[1].len;
	kdata.coefficient.len = bnBytes(&sec->u);
	bnExtractBigBytes (&sec->n, kdata.modulus.data, 0,
						kdata.modulus.len);
	bnExtractBigBytes (&sec->q, kdata.prime[0].data, 0,
						kdata.prime[0].len);
	bnExtractBigBytes (&sec->p, kdata.prime[1].data, 0,
						kdata.prime[1].len);
	bnExtractBigBytes (&dmodq, kdata.primeExponent[0].data, 0,
						kdata.primeExponent[0].len);
	bnExtractBigBytes (&dmodp, kdata.primeExponent[1].data, 0,
						kdata.primeExponent[1].len);
	bnExtractBigBytes (&sec->u, kdata.coefficient.data, 0,
						kdata.coefficient.len);
	err = B_SetKeyInfo (rprivk, KI_RSA_CRT, (POINTER)&kdata);
	pgpAssert (err == 0);
	pgpClearMemory (buf, bufsize);
	PGPFreeData (buf);

	bnEnd(&dmodp);
	bnEnd(&dmodq);
	bnEnd(&tmp);
	return err;
}

/*
 * Encrypt a buffer holding a session key with an RSA public key
 */
int
rsaPublicEncrypt(BigNum *bn, PGPByte const *in, unsigned len,
	RSApub const *pub, PGPRandomContext const *rc)
{
	unsigned bytes = bnBytes(&pub->n);
	PGPByte *buf = NULL;
	B_ALGORITHM_OBJ bobj = NULL;
	B_KEY_OBJ rpubk = NULL;
	unsigned int bufoutlen;
	PGPMemoryMgrRef	mgr	= bn->mgr;
	int err = 0;

	pgpPKCSPack(bn, in, len, PKCS_PAD_ENCRYPTED, bytes, rc);
	buf = PGPNewSecureData (mgr, bytes, 0);
	if (buf == NULL) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	bnExtractBigBytes (bn, buf, 0, bytes);
	bnSetQ (bn, 0);

	/* Initialize BSafe public key structure */
	err = B_CreateAlgorithmObject (&bobj);
	CHKERR(err);
	err = B_SetAlgorithmInfo (bobj, AI_RSAPublic, NULL);
	CHKERR(err);
	err = B_CreateKeyObject (&rpubk);
	CHKERR(err);
	err = rpubk_init(rpubk, pub, mgr);
	CHKERR(err);
	err = B_EncryptInit (bobj, rpubk, RSA_CHOOSER, (A_SURRENDER_CTX *)0);
	CHKERR(err);

	/* Encrypt data */
	err = B_EncryptUpdate (bobj, buf, &bufoutlen, bytes, buf, bytes,
						(B_ALGORITHM_OBJ)NULL, (A_SURRENDER_CTX *)NULL);
	CHKERR(err);
	err = B_EncryptFinal (bobj, buf+bufoutlen, &bufoutlen, bytes-bufoutlen,
						(B_ALGORITHM_OBJ)NULL, (A_SURRENDER_CTX *)NULL);
	CHKERR(err);

	B_DestroyKeyObject (&rpubk);
	rpubk = NULL;
	B_DestroyAlgorithmObject (&bobj);
	bobj = NULL;

	/* Return to bn format */
	bnInsertBigBytes (bn, buf, 0, bytes);
	pgpClearMemory (buf, bytes);
	PGPFreeData (buf);
	buf = NULL;

error:
	if (buf) {
		pgpClearMemory (buf, bytes);
		PGPFreeData (buf);
	}
	if (rpubk)
		B_DestroyKeyObject (&rpubk);
	if (bobj)
		B_DestroyAlgorithmObject (&bobj);
	
	return err;
}


/*
 * Do an RSA signing operation using the secret key
 */
int
rsaPrivateEncrypt(BigNum *bn, PGPByte const *in, unsigned len,
	RSAsec const *sec)
{
	unsigned bytes = bnBytes(&sec->n);
	PGPByte *buf = NULL;
	B_ALGORITHM_OBJ bobj = NULL;
	B_KEY_OBJ rprivk = NULL;
	unsigned int bufoutlen;
	PGPMemoryMgrRef	mgr	= bn->mgr;
	int err;

	pgpPKCSPack(bn, in, len, PKCS_PAD_SIGNED, bytes,
		(PGPRandomContext const *)NULL);
	buf = PGPNewSecureData (mgr, bytes, 0);
	if (buf == NULL) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	bnExtractBigBytes (bn, buf, 0, bytes);
	bnSetQ (bn, 0);

	/* Initialize BSafe private key structure */
	err = B_CreateAlgorithmObject (&bobj);
	CHKERR(err);
	err = B_SetAlgorithmInfo (bobj, AI_RSAPrivate, NULL);
	CHKERR(err);
	err = B_CreateKeyObject (&rprivk);
	CHKERR(err);
	err = rprivk_init(rprivk, sec, mgr);
	CHKERR(err);
	err = B_DecryptInit (bobj, rprivk, RSA_CHOOSER, (A_SURRENDER_CTX *)0);
	CHKERR(err);

	/* "Decrypt" (actually sign) data */
	err = B_DecryptUpdate (bobj, buf, &bufoutlen, bytes, buf, bytes,
						(B_ALGORITHM_OBJ)NULL, (A_SURRENDER_CTX *)NULL);
	CHKERR(err);
	err = B_DecryptFinal (bobj, buf+bufoutlen, &bufoutlen, bytes-bufoutlen,
						(B_ALGORITHM_OBJ)NULL, (A_SURRENDER_CTX *)NULL);
	CHKERR(err);

	B_DestroyKeyObject (&rprivk);
	rprivk = NULL;
	B_DestroyAlgorithmObject (&bobj);
	bobj = NULL;

	/* Return to bn format */
	bnInsertBigBytes (bn, buf, 0, bytes);
	pgpClearMemory (buf, bytes);
	PGPFreeData (buf);
	buf = NULL;
	
error:
	if (buf) {
		pgpClearMemory (buf, bytes);
		PGPFreeData (buf);
	}
	if (rprivk)
		B_DestroyKeyObject (&rprivk);
	if (bobj)
		B_DestroyAlgorithmObject (&bobj);
	
	return err;
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
	unsigned bytes = bnBytes(&pub->n);
	PGPByte *buf = NULL;
	B_ALGORITHM_OBJ bobj = NULL;
	B_KEY_OBJ rpubk = NULL;
	unsigned int bufoutlen;
	PGPMemoryMgrRef	mgr	= bn->mgr;
	int err;

	buf = PGPNewSecureData (mgr, bytes, 0);
	if (buf == NULL) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	bnExtractBigBytes (bn, buf, 0, bytes);

	/* Initialize BSafe public key structure */
	err = B_CreateAlgorithmObject (&bobj);
	CHKERR(err);
	err = B_SetAlgorithmInfo (bobj, AI_RSAPublic, NULL);
	CHKERR(err);
	err = B_CreateKeyObject (&rpubk);
	CHKERR(err);
	err = rpubk_init(rpubk, pub, mgr);
	CHKERR(err);
	err = B_EncryptInit (bobj, rpubk, RSA_CHOOSER, (A_SURRENDER_CTX *)0);
	CHKERR(err);

	/* Do an RSA encryption to recover PKCS-1 padded signature */
	err = B_EncryptUpdate (bobj, buf, &bufoutlen, bytes, buf, bytes,
						(B_ALGORITHM_OBJ)NULL, (A_SURRENDER_CTX *)NULL);
	CHKERR(err);
	err = B_EncryptFinal (bobj, buf+bufoutlen, &bufoutlen, bytes-bufoutlen,
						(B_ALGORITHM_OBJ)NULL, (A_SURRENDER_CTX *)NULL);
	CHKERR(err);

	B_DestroyKeyObject (&rpubk);
	rpubk = NULL;
	B_DestroyAlgorithmObject (&bobj);
	bobj = NULL;

	/* Return to bn format */
	bnSetQ (bn, 0);
	bnInsertBigBytes (bn, buf, 0, bytes);
	pgpClearMemory (buf, bytes);
	PGPFreeData (buf);
	buf = NULL;
	
	err = pgpPKCSUnpack(outbuf, len, bn, PKCS_PAD_SIGNED, bytes);

error:
	if (buf) {
		pgpClearMemory (buf, bytes);
		PGPFreeData (buf);
	}
	if (rpubk)
		B_DestroyKeyObject (&rpubk);
	if (bobj)
		B_DestroyAlgorithmObject (&bobj);
	
	return err;
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
	unsigned bytes = bnBytes(&sec->n);
	PGPByte *buf = NULL;
	B_ALGORITHM_OBJ bobj = NULL;
	B_KEY_OBJ rprivk = NULL;
	unsigned int bufoutlen;
	PGPMemoryMgrRef	mgr	= bn->mgr;
	int err;

	buf = PGPNewSecureData (mgr, bytes, 0);
	if (buf == NULL) {
		err = kPGPError_OutOfMemory;
		goto error;
	}
	bnExtractBigBytes (bn, buf, 0, bytes);

	/* Initialize BSafe private key structure */
	err = B_CreateAlgorithmObject (&bobj);
	CHKERR(err);
	err = B_SetAlgorithmInfo (bobj, AI_RSAPrivate, NULL);
	CHKERR(err);
	err = B_CreateKeyObject (&rprivk);
	CHKERR(err);
	err = rprivk_init(rprivk, sec, mgr);
	CHKERR(err);
	err = B_DecryptInit (bobj, rprivk, RSA_CHOOSER, (A_SURRENDER_CTX *)0);
	CHKERR(err);

	/* Do an RSA decryption to recover PKCS-1 padded key */
	err = B_DecryptUpdate (bobj, buf, &bufoutlen, bytes, buf, bytes,
						(B_ALGORITHM_OBJ)NULL, (A_SURRENDER_CTX *)NULL);
	CHKERR(err);
	err = B_DecryptFinal (bobj, buf+bufoutlen, &bufoutlen, bytes-bufoutlen,
						(B_ALGORITHM_OBJ)NULL, (A_SURRENDER_CTX *)NULL);
	CHKERR(err);

	B_DestroyKeyObject (&rprivk);
	rprivk = NULL;
	B_DestroyAlgorithmObject (&bobj);
	bobj = NULL;

	/* Return to bn format */
	bnSetQ (bn, 0);
	bnInsertBigBytes (bn, buf, 0, bytes);
	pgpClearMemory (buf, bytes);
	PGPFreeData (buf);
	buf = NULL;
	
	err = pgpPKCSUnpack(outbuf, len, bn, PKCS_PAD_ENCRYPTED, bytes);

error:
	if (buf) {
		pgpClearMemory (buf, bytes);
		PGPFreeData (buf);
	}
	if (rprivk)
		B_DestroyKeyObject (&rprivk);
	if (bobj)
		B_DestroyAlgorithmObject (&bobj);
	
	return err;
}


/************************* BSAFE Required Functions *******************/

/*
 * These functions are called by the BSAFE library to perform elementary
 * operations
 */


void CALL_CONV T_memset (p, c, count)
POINTER p;
int c;
unsigned int count;
{
	if (count != 0)
		pgpFillMemory (p, count, c);
}

void CALL_CONV T_memcpy (d, s, count)
POINTER d, s;
unsigned int count;
{
	if (count != 0)
		pgpCopyMemory (s, d, count);
}

void CALL_CONV T_memmove (d, s, count)
POINTER d, s;
unsigned int count;
{
	if (count != 0)
		pgpCopyMemory (s, d, count);
}

int CALL_CONV T_memcmp (s1, s2, count)
POINTER s1, s2;
unsigned int count;
{
	if (count == 0)
		return 0;
    return memcmp (s1, s2, count);
}

POINTER CALL_CONV T_malloc (size)
unsigned int size;
{
	PGPMemoryMgrRef mgr = PGPGetDefaultMemoryMgr();
	
	if (IsNull (mgr))
		return NULL;
	return (POINTER) PGPNewSecureData( mgr, size, 0 );
}

POINTER CALL_CONV T_realloc (p, size)
POINTER p;
unsigned int size;
{
	PGPMemoryMgrRef mgr = PGPGetDefaultMemoryMgr();
	
	if (IsNull (mgr))
		return NULL;
	/* pgpReallocData will not know to do securely if pass null ptr */
	if (IsNull (p))
		return (POINTER) PGPNewSecureData( mgr, size, 0 );
	PGPReallocData (mgr, &p, size, 0);
	return p;
}

void CALL_CONV T_free (p)
POINTER p;
{
	if (IsntNull (p))
		PGPFreeData (p);
}



#endif	/* ] PGP_USEBSAFEFORRSA */
