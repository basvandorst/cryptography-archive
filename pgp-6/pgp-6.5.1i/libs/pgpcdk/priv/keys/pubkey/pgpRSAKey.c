/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpRSAKey.c,v 1.65 1999/04/22 00:37:43 hal Exp $
____________________________________________________________________________*/

#include "pgpSDKBuildFlags.h"

#ifndef PGP_RSA
#error "PGP_RSA requires a value"
#endif

#ifndef PGP_RSA_KEYGEN
#error "PGP_RSA_KEYGEN requires a value"
#endif

/* This entire module is dependent on RSA being enabled */
#if PGP_RSA

#include "pgpConfig.h"
#include <string.h>
#include <stddef.h>

#include "pgpDebug.h"
#include "pgpKeyMisc.h"
#include "pgpRSAGlue.h"
#include "pgpRSAKey.h"
#include "bn.h"
#include "pgpCFBPriv.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpHashPriv.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "bnprime.h"
#include "pgpPubKey.h"
#include "pgpRandomX9_17.h"
#include "pgpStr2Key.h"
#include "pgpContext.h"
#include "pgpEnv.h"



#define RSA_DEFAULT_EXPONENT	17

#define ASSERTRSA(alg) pgpAssert((ALGMASK(alg))==kPGPPublicKeyAlgorithm_RSA || \
			      (ALGMASK(alg))==kPGPPublicKeyAlgorithm_RSAEncryptOnly || \
			      (ALGMASK(alg))==kPGPPublicKeyAlgorithm_RSASignOnly)
#define ASSERTRSASIG(alg) \
	pgpAssert((ALGMASK(alg))==kPGPPublicKeyAlgorithm_RSA || \
	 (ALGMASK(alg))==kPGPPublicKeyAlgorithm_RSASignOnly)
#define ASSERTRSAENC(alg) \
		pgpAssert((ALGMASK(alg))==kPGPPublicKeyAlgorithm_RSA || \
		(ALGMASK(alg))==kPGPPublicKeyAlgorithm_RSAEncryptOnly)


/* A PGPSecKey's priv points to this, an RSAsec plus the encrypted form... */
/* This struct is always allocated using PGPNewSecureData */
typedef struct RSAsecPlus
{
	PGPContextRef	context;
	
	RSAsec			s;
	PGPByte *		cryptkey;
	size_t			ckalloc;
	size_t			cklen;
	int				locked;
	PGPBoolean		v3;	/* True if key is in a pre PGPVERSION_4 packet */
	DEBUG_STRUCT_CONSTRUCTOR( RSAsecPlus )
} RSAsecPlus ;

/** Public key functions **/

static void
rsaPubDestroy(PGPPubKey *pubkey)
{
	RSApub *pub = (RSApub *)pubkey->priv;
	PGPContextRef	context;

	pgpAssertAddrValid( pubkey, PGPPubKey );
	context	= pubkey->context;

	ASSERTRSA(pubkey->pkAlg);
	
	bnEnd(&pub->n);
	bnEnd(&pub->e);
	pgpClearMemory( pub,  sizeof(pub));
	pgpContextMemFree( context, pub);
	pgpClearMemory( pubkey,  sizeof(pubkey));
	pgpContextMemFree(context, pubkey);
}

/* Return the largest possible PGPESK size for a given key */
static size_t
rsaPubMaxesk(PGPPubKey const *pubkey, PGPPublicKeyMessageFormat format)
{
	RSApub const *pub = (RSApub *)pubkey->priv;

	ASSERTRSAENC(pubkey->pkAlg);
	if (format == kPGPPublicKeyMessageFormat_PGP)
		return 2 + bnBytes(&pub->n);
	else if (format == kPGPPublicKeyMessageFormat_PKCS1 ||
			 format == kPGPPublicKeyMessageFormat_X509  ||
			 format == kPGPPublicKeyMessageFormat_IKE)
		return bnBytes(&pub->n);

	pgpAssert(0);
	return 0;
}

/* Return the largest possible input size for rsaEncrypt */
static size_t
rsaPubMaxdecrypted(PGPPubKey const *pubkey, PGPPublicKeyMessageFormat format)
{
	RSApub const *pub = (RSApub *)pubkey->priv;

	(void) format;
	ASSERTRSAENC(pubkey->pkAlg);

	/* Minimum padding could be just 0 2 0 */
	return bnBytes(&pub->n) - 3;
}

/* Return the largest possible signature input to rsaVerify */
static size_t
rsaPubMaxsig(PGPPubKey const *pubkey, PGPPublicKeyMessageFormat format)
{
	RSApub const *pub = (RSApub *)pubkey->priv;

	ASSERTRSASIG(pubkey->pkAlg);

	if (format == kPGPPublicKeyMessageFormat_PGP)
		return 2 + bnBytes(&pub->n);
	else if (format == kPGPPublicKeyMessageFormat_PKCS1 ||
			 format == kPGPPublicKeyMessageFormat_IKE ||
			 format == kPGPPublicKeyMessageFormat_X509)
		return bnBytes(&pub->n);

	pgpAssert(0);
	return 0;
}


/*
 * Given a buffer of at least "maxesk" bytes, make an PGPESK
 * into it and return the size of the PGPESK, or <0.
 */
static int
rsaEncrypt(PGPPubKey const *pubkey, PGPByte const *key,
           size_t keylen, PGPByte *esk, size_t *esklen,
           PGPRandomContext const *rc, PGPPublicKeyMessageFormat format)
{
#if PGP_ENCRYPT_DISABLE /* [ */

	(void)pubkey;
	(void)key;
	(void)keylen;
	(void)esk;
	(void)esklen;
	(void)rc;
	(void)format;
	return kPGPError_FeatureNotAvailable;

#else /* PGP_ENCRYPT_DISABLE */  /* ]  [ */

	RSApub const *pub = (RSApub *)pubkey->priv;
	BigNum bn;
	unsigned t;
	int i;
	PGPMemoryMgrRef		mgr	= NULL;

	/* We don't need these arguments, although other algorithms may... */
	(void)rc;

	mgr	= PGPGetContextMemoryMgr( pubkey->context );

	ASSERTRSAENC(pubkey->pkAlg);
	t = bnBits(&pub->n);
	if (t > 0xffff)
		return kPGPError_PublicKeyTooLarge;
	if (keylen > t)
		return kPGPError_PublicKeyTooSmall; /* data too big for pubkey */

	if( format == kPGPPublicKeyMessageFormat_PGP ) {
		/* Add checksum to key, place temporarily in esk buffer */
		t = 0;
		esk[0] = key[0];
		for (i = 1; i < (int)keylen; i++)
			t += esk[i] = key[i];
		esk[keylen] = (PGPByte)(t >> 8 & 255);
		esk[keylen+1] = (PGPByte)(t & 255);
		keylen += 2;
		key = esk;
	}

	bnBegin(&bn, mgr, TRUE);
	i = rsaPublicEncrypt(&bn, key, keylen, pub, rc);
	if (i < 0) {
		bnEnd(&bn);
		return i;
	}
	
	t = pgpBnPutFormatted(&bn, esk, bnBytes(&pub->n), format);
	bnEnd(&bn);

	if (esklen)
		*esklen = (size_t)t;
	return 0;

#endif /* PGP_ENCRYPT_DISABLE */ /* ] */
}


/*
 * Return 1 if (sig,siglen) is a valid MPI which signs
 * hash, of type h.  Check the DER-encoded prefix and the
 * hash itself.
 */
static int
rsaVerify(PGPPubKey const *pubkey, PGPByte const *sig,
	size_t siglen, PGPHashVTBL const *h, PGPByte const *hash,
	PGPPublicKeyMessageFormat format)
{
#if PGP_VERIFY_DISABLE /* [ */

	(void)pubkey;
	(void)sig;
	(void)siglen;
	(void)h;
	(void)hash;
	(void)format;
	return kPGPError_FeatureNotAvailable;

#else /* PGP_VERIFY_DISABLE */  /* ]  [ */

	RSApub const *pub = (RSApub *)pubkey->priv;
	BigNum bn;
	PGPByte buf[64];	/* largest hash size + DER prefix */
	int i;
	size_t off = 0;
	PGPMemoryMgrRef	mgr	= PGPGetContextMemoryMgr( pubkey->context );

	ASSERTRSASIG(pubkey->pkAlg);

	bnBegin(&bn, mgr, FALSE);
	if (format == kPGPPublicKeyMessageFormat_X509) {
		/* Unformatted data, taking up whole length of buffer */
		format = kPGPPublicKeyMessageFormat_PKCS1;
		i = pgpBnGetFormatted(&bn, sig+off, siglen-off, siglen-off, format);
	} else {
		i = pgpBnGetFormatted(&bn, sig+off, siglen-off, bnBytes(&pub->n),
							  format);
	}
	if (i <= 0)
		return kPGPError_BadSignatureSize;

#if PGP_USECAPIFORRSA
	i = rsaVerifyHashSignature(&bn, pub, h, hash);
#else
	i = rsaPublicDecrypt(buf, sizeof(buf), &bn, pub);

	if (i >= 0) {
		/* Check that the returned data is correct */
		unsigned t = h->DERprefixsize;
		/* IKE does not put in hash OID */
		if (format == kPGPPublicKeyMessageFormat_IKE)
			t = 0;
		i = (size_t)i <= sizeof(buf)
		    && (unsigned)i == h->hashsize + t
		    && memcmp(buf, h->DERprefix, t) == 0
		    && memcmp(buf+t, hash, h->hashsize) == 0;
	}
#endif
	bnEnd(&bn);
	pgpClearMemory( buf,  sizeof(buf));
	return i;

#endif /* PGP_VERIFY_DISABLE */ /* ] */
}


/*
 * Turn a PGPPubKey into the algorithm-specific parts of a public key.
 * A public key's RSA-specific part is:
 *
 *  0      2+i  MPI for modulus
 * 2+i     2+t  MPI for exponent
 * 4+i+t
 */
static size_t
rsaPubBufferLength(PGPPubKey const *pubkey)
{
	RSApub const *pub = (RSApub *)pubkey->priv;

	return 4 + bnBytes(&pub->n) + bnBytes(&pub->e);
}

static void
rsaPubToBuffer(PGPPubKey const *pubkey, PGPByte *buf)
{
	RSApub const *pub = (RSApub *)pubkey->priv;
	unsigned i, t;

	i = bnBits(&pub->n);
	pgpAssert(i <= 0xffff);
	buf[0] = (PGPByte)(i >> 8);
	buf[1] = (PGPByte)i;
	i = (i+7)/8;
	bnExtractBigBytes(&pub->n, buf+2, 0, i);
	t = bnBits(&pub->e);
	pgpAssert(t <= 0xffff);
	buf[2+i] = (PGPByte)(t >> 8);
	buf[3+i] = (PGPByte)t;
	t = (t+7)/8;
	bnExtractBigBytes(&pub->e, buf+4+i, 0, t);
}


/* A little helper function that's used twice */
static void
rsaFillPubkey(PGPPubKey *pubkey, RSApub *pub)
{
	pubkey->next	 = NULL;
	pubkey->pkAlg	 = kPGPPublicKeyAlgorithm_RSA;
	pubkey->priv	 = pub;
	pubkey->destroy  = rsaPubDestroy;
	pubkey->maxesk   = rsaPubMaxesk;
	pubkey->maxdecrypted   = rsaPubMaxdecrypted;
	pubkey->maxsig   = rsaPubMaxsig;
	pubkey->encrypt  = rsaEncrypt;
	pubkey->verify   = rsaVerify;
	pubkey->bufferLength  = rsaPubBufferLength;
	pubkey->toBuffer = rsaPubToBuffer;
}


/*
 * Turn the algorithm-specific parts of a public key into a PGPPubKey
 * structure.  A public key's RSA-specific part is:
 *
 *  0      2+i  MPI for modulus
 * 2+i     2+t  MPI for exponent
 * 4+i+t
 */
PGPPubKey *
rsaPubFromBuf(
	PGPContextRef	context,
	PGPByte const *	buf,
	size_t			size,
	PGPError *		error)
{
	PGPPubKey *pubkey;
	RSApub *pub;
	unsigned i, t;
	PGPError	err	= kPGPError_OutOfMemory;
	PGPMemoryMgrRef	mgr	= PGPGetContextMemoryMgr( context );

	bnInit();

	if (size < 4)
		return NULL;

	i = ((unsigned)buf[0] << 8) + buf[1];
	if (!i || buf[2] >> ((i-1) & 7) != 1) {
		*error = kPGPError_MalformedKeyModulus;
		return NULL;	/* Bad bit length */
	}
	i = (i+7)/8;
	if (size < 4+i) {
		*error = kPGPError_KeyPacketTruncated;
		return NULL;
	}
	if ((buf[1+i] & 1) == 0) {	/* Too small or even modulus */
		*error = kPGPError_RSAPublicModulusIsEven;
		return NULL;
	}
	t = ((unsigned)buf[2+i] << 8) + buf[3+i];
	if (!t || buf[4+i] >> ((t-1) & 7) != 1) {
		*error = kPGPError_MalformedKeyExponent;
		return NULL;	/* Bad bit length */
	}
	t = (t+7)/8;
	if (size < 4+i+t) {
		*error = kPGPError_KeyPacketTruncated;
		return NULL;
	}

	pub = (RSApub *)pgpContextMemAlloc( context,
		sizeof(*pub), kPGPMemoryMgrFlags_Clear );
	if (pub) {
		pubkey = (PGPPubKey *)pgpContextMemAlloc( context,
			sizeof(*pubkey), kPGPMemoryMgrFlags_Clear);
		if (pubkey) {
			pubkey->context	= context;
			bnBegin(&pub->n, mgr, FALSE);
			bnBegin(&pub->e, mgr, FALSE);
			if (bnInsertBigBytes(&pub->n, buf+2, 0, i) >= 0
			    && bnInsertBigBytes(&pub->e, buf+4+i, 0, t) >= 0)
			{
				if (rsaKeyTooBig (pub, NULL) ||
					bnBits(&pub->n) > 2048 ) {
					err = kPGPError_KeyTooLarge;
				} else {
					rsaFillPubkey(pubkey, pub);
					*error = 0;
					return pubkey;
				}
			}
			/* Failed = clean up and return NULL */
			bnEnd(&pub->n);
			bnEnd(&pub->e);
			pgpContextMemFree( context, pubkey);
		}
		pgpContextMemFree( context, pub);
	}
	*error = err;
	return NULL;
}

/*
 * Return the size of the public portion of a key buffer.
 */
int
rsaPubKeyPrefixSize(PGPByte const *buf, size_t size)
{
	return pgpBnParse(buf, size, 2, NULL, NULL);
}



/** Secret key functions **/

static void
rsaSecDestroy(PGPSecKey *seckey)
{
	RSAsecPlus *sec = (RSAsecPlus *)seckey->priv;
	PGPContextRef	context;

	pgpAssertAddrValid( seckey, PGPPubKey );
	context	= seckey->context;

	ASSERTRSA(seckey->pkAlg);
	bnEnd(&sec->s.n);
	bnEnd(&sec->s.e);
	bnEnd(&sec->s.d);
	bnEnd(&sec->s.p);
	bnEnd(&sec->s.q);
	bnEnd(&sec->s.u);
	pgpClearMemory(sec->cryptkey, sec->ckalloc);
	pgpContextMemFree( context, sec->cryptkey);
	PGPFreeData( sec );			/* Wipes as it frees */
	pgpClearMemory( seckey,  sizeof(seckey));
	pgpContextMemFree( context, seckey);
}

/*
 * Generate a PGPPubKey from a PGPSecKey
 */
static PGPPubKey *
rsaPubkey(PGPSecKey const *seckey)
{
	RSAsecPlus const *sec = (RSAsecPlus *)seckey->priv;
	PGPPubKey *pubkey;
	RSApub *pub;
	PGPContextRef	context;
	PGPMemoryMgrRef		mgr	= NULL;

	pgpAssertAddrValid( seckey, PGPSecKey );
	context	= seckey->context;
	mgr	= PGPGetContextMemoryMgr( context );

	ASSERTRSA(seckey->pkAlg);
	pub = (RSApub *)pgpContextMemAlloc( context, 
		sizeof(*pub), kPGPMemoryMgrFlags_Clear);
	if (pub) {
		pubkey = (PGPPubKey *)pgpContextMemAlloc( context,
			sizeof(*pubkey), kPGPMemoryMgrFlags_Clear );
		if (pubkey) {
			pubkey->context	= seckey->context;
			
			bnBegin(&pub->n, mgr, FALSE);
			bnBegin(&pub->e, mgr, FALSE);
			if (bnCopy(&pub->n, &sec->s.n) >= 0
			    && bnCopy(&pub->e, &sec->s.e) >= 0)
			{
				rsaFillPubkey(pubkey, pub);
				pubkey->pkAlg = seckey->pkAlg;
				memcpy(pubkey->keyID, seckey->keyID,
				       sizeof(pubkey->keyID));
				return pubkey;
			}
			/* Failed = clean up and return NULL */
			bnEnd(&pub->n);
			bnEnd(&pub->e);
			pgpContextMemFree( context, pubkey);
		}
		pgpContextMemFree(context, pub);
	}
	return NULL;
}

/*
 * Yes, there *is* a reason that this is a function and no a variable.
 * On a hardware device with an automatic timeout,
 * it actually might need to do some work to find out.
 */
static int
rsaIslocked(PGPSecKey const *seckey)
{
	RSAsecPlus const *sec = (RSAsecPlus *)seckey->priv;

	ASSERTRSA(seckey->pkAlg);
	return sec->locked;
}


/*
 * Return the algorithm and (symmetric) key size used for locking/unlocking
 * the secret key.
 */
static PGPError
rsaLockingAlgorithm(
	PGPSecKey const *seckey,
	PGPCipherAlgorithm *pAlg,
	PGPSize *pAlgKeySize
	)
{
	RSAsecPlus *sec = (RSAsecPlus *)seckey->priv;
	PGPCipherVTBL const *cipher;
	PGPByte alg;
	int i;

	ASSERTRSA(seckey->pkAlg);

	if( IsntNull( pAlg ) )
		*pAlg = (PGPCipherAlgorithm) 0;
	if( IsntNull( pAlgKeySize ) )
		*pAlgKeySize = (PGPSize) 0;

	/* Check packet for basic consistency */
	i = pgpBnParse(sec->cryptkey, sec->cklen, 2, NULL, NULL);
	if (i < 0)
		return (PGPError)i;

	/* Get the encryption algorithm (cipher number).  0 == no encryption */
	alg = sec->cryptkey[i] & 255;

	/* New style has 255 then algorithm value */
	if (alg == 255)
		alg = sec->cryptkey[i+1] & 255;

	cipher = pgpCipherGetVTBL( (PGPCipherAlgorithm)alg);
	if (!cipher)
		return kPGPError_BadCipherNumber;

	/* Success */
	if( IsntNull( pAlg ) )
		*pAlg = (PGPCipherAlgorithm) alg;
	if( IsntNull( pAlgKeySize ) )
		*pAlgKeySize = cipher->keysize;

	return kPGPError_NoErr;
}


/*
 * Return the StringToKey type for unlocking the given key.  We use
 * kPGPStringToKey_Literal to flag a secret split unlocking buffer.
 * Returns kPGPStringToKey_Simple if key has no passphrase.
 */
static PGPError
rsaS2KType(
	PGPSecKey const *seckey,
	PGPStringToKeyType *s2kType
	)
{
	RSAsecPlus *sec = (RSAsecPlus *)seckey->priv;
	PGPByte alg;
	int i;

	ASSERTRSA(seckey->pkAlg);

	/* note that 0 is a valid type, but use it as default anyway */
	if( IsntNull( s2kType ) )
		*s2kType = (PGPStringToKeyType) 0;

	/* Check packet for basic consistency */
	i = pgpBnParse(sec->cryptkey, sec->cklen, 2, NULL, NULL);
	if (i < 0)
		return (PGPError)i;

	/* Get the encryption algorithm (cipher number).  0 == no encryption */
	alg = sec->cryptkey[i] & 255;

	if (alg == 255) {
		/* New style has 255 then algorithm value then S2K */
		*s2kType = (PGPStringToKeyType) sec->cryptkey[i+2];
	} else {
		/* Unencrypted or old-style simple encryption */
		*s2kType = kPGPStringToKey_Simple;
	}

	return kPGPError_NoErr;
}


/*
 * Convert a passphrase into a s2k literal buffer for the key.
 * Returns error code.  Output buffer will be size of the *pAlgKeySize
 * parameter from pgpSecKeyLockingalgorithm.
 */
static PGPError
rsaConvertPassphrase(PGPSecKey *seckey, PGPEnv const *env,
	  char const *phrase, PGPSize plen, PGPByte *outbuf)
{
	RSAsecPlus *sec = (RSAsecPlus *)seckey->priv;
	PGPStringToKey *s2k;
	PGPByte alg;
	PGPBoolean hasS2K;
	PGPCipherVTBL const *cipher;
	int i;

	ASSERTRSA(seckey->pkAlg);
	pgpAssert (IsntNull( outbuf ) );

	/* Check packet for basic consistency */
	i = pgpBnParse(sec->cryptkey, sec->cklen, 2, NULL, NULL, NULL, NULL);
	if (i < 0)
		return (PGPError)i;

	/* Get the encryption algorithm (cipher number).  0 == no encryption */
	alg = sec->cryptkey[i++] & 255;

	hasS2K = (alg == 255);

	/* New style has 255 then algorithm value */
	if (hasS2K)
		alg = sec->cryptkey[i++] & 255;

	/* Now we are looking at the s2k object if there is one. */
	if (alg == 0) {
		/* Key is not locked */
		return kPGPError_BadParams;
	}
	cipher = pgpCipherGetVTBL( (PGPCipherAlgorithm)alg);
	if( IsNull( cipher ) )
		return kPGPError_BadCipherNumber;

	if (hasS2K) {
		pgpS2Kdecode(&s2k, env, sec->cryptkey+i, sec->cklen-i);
	} else {
		s2k = pgpS2Ksimple(env, pgpHashByNumber(kPGPHashAlgorithm_MD5));
	}
	if (IsNull( s2k ) )
		return kPGPError_OutOfMemory;
	pgpStringToKey(s2k, phrase, plen, outbuf, cipher->keysize);
	pgpS2Kdestroy (s2k);

	return kPGPError_NoErr;
}


/*
 * Try to decrypt the secret key wih the given passphrase.  Returns >0
 * if it was the correct passphrase. =0 if it was not, and <0 on error.
 * Does not alter the key even if it's the wrong passphrase and already
 * unlocked.  A NULL passphrae will work if the key is unencrypted.
 * 
 * A (secret) key's RSA-specific part is:
 *
 *  0                2+u  MPI for modulus
 *  2+u              2+v  MPI for exponent
 *  4+u+v            1    Encryption algorithm (0 for none, 1 for IDEA)
 *  5+u+v            t    Encryption IV: 0 or 8 bytes
 *  5+t+u+v          2+w  MPI for d
 *  7+t+u+v+w        2+x  MPI for p
 *  9+t+u+v+w+x      2+y  MPI for q
 * 11+t+u+v+w+x+y    2+z  MPI for u
 * 13+t+u+v+w+x+y+z  2    Checksum
 * 15+t+u+v+w+x+y+z
 *
 * Actually, that's the old-style, if pgpS2KoldVers is true.
 * If it's false, the algoruthm is 255, and is followed by the
 * algorithm, then the (varaible-length, self-delimiting)
 * string-to-key descriptor.
 */

static int
rsaUnlock(PGPSecKey *seckey, PGPEnv const *env,
	  char const *phrase, size_t plen, PGPBoolean hashedPhrase)
{
	RSAsecPlus *sec = (RSAsecPlus *)seckey->priv;
	BigNum d, p, q, u, bn;
	PGPCFBContext *cfb = NULL;	/* Necessary */
	unsigned v, t;
	unsigned alg;
	unsigned checksum;
	int i;
	PGPMemoryMgrRef		mgr	= NULL;

	mgr	= PGPGetContextMemoryMgr( seckey->context );

	ASSERTRSA(seckey->pkAlg);
	bnInit();

	if (sec->cklen < 5)
		return kPGPError_KeyPacketTruncated;
	v = ((unsigned)sec->cryptkey[0] << 8) + sec->cryptkey[1];
	v = (v+7)/8;
	if (sec->cklen < 5+v)
		return kPGPError_KeyPacketTruncated;
	if (bnInsertBigBytes(&sec->s.n, sec->cryptkey+2, 0, v) < 0)
		return kPGPError_OutOfMemory;
	t = ((unsigned)sec->cryptkey[2+v] << 8) + sec->cryptkey[3+v];
	t = (t+7)/8;
	if (sec->cklen < 4+v+t)
		return kPGPError_KeyPacketTruncated;
	if (bnInsertBigBytes(&sec->s.e, sec->cryptkey+4+v, 0, t) < 0)
		return kPGPError_OutOfMemory;
	v += t + 4;
	if (sec->cklen < v+1)
		return kPGPError_KeyPacketTruncated;

	/* Get the encryption algorithm (cipher number).  0 == no encryption */
	alg  = sec->cryptkey[v];

	/* If the phrase is empty, set it to NULL */
	if (plen == 0)
		phrase = NULL;
	/*
	 * We need a pass if it is encrypted, and we cannot have a
	 * password if it is NOT encrypted.  I.e., this is a logical
	 * xor (^^)
	 */
	if (!phrase != !alg)
		return 0;

	i = pgpCipherSetup(sec->cryptkey + v, sec->cklen - v, phrase, plen,
					   hashedPhrase, env, &cfb);
	if (i < 0)
		return i;
	v += i;

	checksum = 0;
	bnBegin(&d, mgr, TRUE);
	bnBegin(&p, mgr, TRUE);
	bnBegin(&q, mgr, TRUE);
	bnBegin(&u, mgr, TRUE);
	bnBegin(&bn, mgr, TRUE);

	i = pgpBnGet(&d, sec->cryptkey + v, sec->cklen - v, cfb, &checksum,
				 sec->v3);
	if (i <= 0)
		goto fail;
	v += i;
	if (bnCmp(&d, &sec->s.n) >= 0)
		goto badpass;	/* Wrong passphrase: d must be < n */
	i = pgpBnGet(&p, sec->cryptkey + v, sec->cklen - v, cfb, &checksum,
				 sec->v3);
	if (i <= 0)
		goto fail;
	if ((bnLSWord(&p) & 1) == 0)
		goto badpass;
	v += i;
	i = pgpBnGet(&q, sec->cryptkey + v, sec->cklen - v, cfb, &checksum,
				 sec->v3);
	if (i <= 0)
		goto fail;
	if ((bnLSWord(&q) & 1) == 0)
		goto badpass;
	v += i;

	/* Extremely high-powered check.  Verify that p*q == n */
	if (bnMul(&bn, &p, &q) < 0)
		goto nomem;
	if (bnCmp(&bn, &sec->s.n) != 0)
		goto badpass;

	/* Verify that d*e == 1 mod p-1 */
	(void)bnSubQ(&p, 1);
	if (bnMul(&bn, &d, &sec->s.e) < 0 || bnMod(&bn, &bn, &p) < 0)
		goto nomem;
	if (bnCmpQ(&bn, 1) != 0)
		goto badpass;
	(void)bnAddQ(&p, 1);

	/* Verify that d*e == 1 mod q-1 */
	(void)bnSubQ(&q, 1);
	if (bnMul(&bn, &d, &sec->s.e) < 0 || bnMod(&bn, &bn, &q) < 0)
		goto nomem;
	if (bnCmpQ(&bn, 1) != 0)
		goto badpass;
	(void)bnAddQ(&q, 1);

	i = pgpBnGet(&u, sec->cryptkey + v, sec->cklen - v, cfb, &checksum,
				 sec->v3);
	if (i <= 0)
		goto fail;
	v += i;

	/* Check that we ended in the right place */
	if (sec->cklen - v != 2) {
		i = kPGPError_KEY_LONG;
		goto fail;
	}
	checksum &= 0xffff;
	if (checksum != ((unsigned)sec->cryptkey[v]<<8) + sec->cryptkey[1+v])
		goto badpass;

	/* Verify that u = p^-1 mod q is less than q */
	if (bnCmp(&u, &q) >= 0)
		goto badpass;
	/* Verify that u * p == 1 mod q */
	if (bnMul(&bn, &p, &u) < 0 || bnMod(&bn, &bn, &q) < 0)
		goto nomem;
	if (bnCmpQ(&bn, 1) != 0)
		goto badpass;

	/*
	 * Okay, we've verified every single value in the secret key,
	 * against the public key, so it is *definitely* the right
	 * secret key.  Note that the "nomem" case calls bnEnd()
	 * more than once, but this is guaranteed harmless.
 	 */
	bnEnd(&bn);
	if (bnCopy(&sec->s.d, &d) < 0)
		goto nomem;
	bnEnd(&d);
	if (bnCopy(&sec->s.p, &p) < 0)
		goto nomem;
	bnEnd(&p);
	if (bnCopy(&sec->s.q, &q) < 0)
		goto nomem;
	bnEnd(&q);
	if (bnCopy(&sec->s.u, &u) < 0)
		goto nomem;
	bnEnd(&u);

	i = 1;	/* Decrypted! */
	sec->locked = 0;
	if (cfb)
		PGPFreeCFBContext (cfb);
	return 1;	/* Decrypted */

nomem:
	i = kPGPError_OutOfMemory;
	goto done;
badpass:
	i = 0;	/* Incorrect passphrase */
	goto done;
fail:
	if (!i)
		i = kPGPError_KeyPacketTruncated;
	goto done;
done:
	if (cfb)
		PGPFreeCFBContext (cfb);
	bnEnd(&bn);
	bnEnd(&u);
	bnEnd(&q);
	bnEnd(&p);
	bnEnd(&d);
	return i;
}

/*
 * Relock the key.
 */
static void
rsaLock(PGPSecKey *seckey)
{
	RSAsecPlus *sec = (RSAsecPlus *)seckey->priv;

	ASSERTRSA(seckey->pkAlg);
	sec->locked = 1;
	/* bnEnd is documented as also doing a bnBegin */
	bnEnd(&sec->s.d);
	bnEnd(&sec->s.p);
	bnEnd(&sec->s.q);
	bnEnd(&sec->s.u);
}

static size_t
rsaSecMaxdecrypted(PGPSecKey const *seckey, PGPPublicKeyMessageFormat format);


/*
 * Try to decrypt the given esk.  If the key is locked, try the given
 * passphrase.  It may or may not leave the key unlocked in such a case.
 * (Some hardware implementations may insist on a password per usage.)
 */
static int
rsaDecrypt(PGPSecKey *seckey, PGPEnv const *env,
		   PGPByte const *esk, size_t esklen,
		   PGPByte *key, size_t *keylen,
		   char const *phrase, size_t plen,
		   PGPPublicKeyMessageFormat format)
{
#if PGP_DECRYPT_DISABLE /* [ */

	(void)seckey;
	(void)env;
	(void)esk;
	(void)esklen;
	(void)key;
	(void)keylen;
	(void)phrase;
	(void)plen;
	(void)format;
	return kPGPError_FeatureNotAvailable;

#else /* PGP_DECRYPT_DISABLE */  /* ]  [ */

	RSAsecPlus *sec = (RSAsecPlus *)seckey->priv;
	BigNum bn;
	int i, j;
	unsigned t;
	size_t max;
	PGPMemoryMgrRef		mgr	= NULL;

	mgr	= PGPGetContextMemoryMgr( seckey->context );

	ASSERTRSAENC(seckey->pkAlg);
	if (sec->locked) {
		i = rsaUnlock(seckey, env, phrase, plen, FALSE);
		if (i <= 0)
			return i ? i : kPGPError_KeyIsLocked;
		pgpAssert(!sec->locked);
	}

	if (esklen < 2)
		return kPGPError_BadSessionKeySize;
	
	bnBegin(&bn, mgr, TRUE);
	i = pgpBnGetFormatted(&bn, esk, esklen, bnBytes(&sec->s.n), format);
	if (i <= 0)
		return kPGPError_BadSessionKeySize;

	max = rsaSecMaxdecrypted(seckey, format);
	i = rsaPrivateDecrypt(key, max, &bn, &sec->s);
	bnEnd(&bn);
	if (i < 0)
		return i;
	if ((size_t)i > max || i < 3)
		return kPGPError_CorruptData;

	if (format == kPGPPublicKeyMessageFormat_PGP) {
		/* Check checksum (should this be here?) */
		t = 0;
		for (j = 1; j < i-2; j++)
			t += key[j];
		if (t != ((unsigned)key[i-2]<<8) + key[i-1])
			return kPGPError_CorruptData;
		pgpClearMemory(key+i-2, 2);

		/* The actual key */
		if (keylen)
			*keylen = (size_t)i-2;
	} else {
		/* The actual key */
		if (keylen)
			*keylen = (size_t)i;
	}

	return 0;

#endif /* PGP_DECRYPT_DISABLE */ /* ] */
}


/*
 * Return the size of the buffer needed, worst-case, for the decrypted
 * output.  A trivially padded key (random padding length = 0)
 * can just be 0 2 0 <key>.
 */
static size_t
rsaSecMaxdecrypted(PGPSecKey const *seckey, PGPPublicKeyMessageFormat format)
{
	RSAsecPlus const *sec = (RSAsecPlus *)seckey->priv;
	size_t size;

	(void) format;
	ASSERTRSAENC(seckey->pkAlg);

	size = bnBytes(&sec->s.n);
	return size < 3 ? 0 : size-3;
}

/* Return the largest possible PGPESK size for a given key */
static size_t
rsaSecMaxesk(PGPSecKey const *seckey, PGPPublicKeyMessageFormat format)
{
	RSAsecPlus const *sec = (RSAsecPlus *)seckey->priv;

	ASSERTRSAENC(seckey->pkAlg);
	if (format == kPGPPublicKeyMessageFormat_PGP)
		return 2 + bnBytes(&sec->s.n);
	else if (format == kPGPPublicKeyMessageFormat_PKCS1 ||
			 format == kPGPPublicKeyMessageFormat_X509  ||
			 format == kPGPPublicKeyMessageFormat_IKE)
		return bnBytes(&sec->s.n);

	pgpAssert(0);
	return 0;
}

static size_t
rsaSecMaxsig(PGPSecKey const *seckey, PGPPublicKeyMessageFormat format)
{
	RSAsecPlus const *sec = (RSAsecPlus *)seckey->priv;

	ASSERTRSASIG(seckey->pkAlg);
	if (format == kPGPPublicKeyMessageFormat_PGP)
		return 2 + bnBytes(&sec->s.n);
	else if (format == kPGPPublicKeyMessageFormat_PKCS1 ||
			 format == kPGPPublicKeyMessageFormat_IKE ||
			 format == kPGPPublicKeyMessageFormat_X509)
		return bnBytes(&sec->s.n);

	pgpAssert(0);
	return 0;
}

static int
rsaSign(PGPSecKey *seckey, PGPHashVTBL const *h, PGPByte const *hash,
	PGPByte *sig, size_t *siglen, PGPRandomContext const *rc,
	PGPPublicKeyMessageFormat format)
{
#if PGP_SIGN_DISABLE /* [ */

	(void)seckey;
	(void)h;
	(void)hash;
	(void)sig;
	(void)siglen;
	(void)rc;
	(void)format;
	return kPGPError_FeatureNotAvailable;

#else

	RSAsecPlus *sec = (RSAsecPlus *)seckey->priv;
	BigNum bn;
	int i;
	unsigned t;
	PGPMemoryMgrRef		mgr	= NULL;
	
	mgr	= PGPGetContextMemoryMgr( seckey->context );

	/* We don't need these arguments, although other algorithms may... */
	(void)rc;
	(void)format;

	ASSERTRSASIG(seckey->pkAlg);
	if (sec->locked)
		return kPGPError_KeyIsLocked;

	bnBegin(&bn, mgr, TRUE);

#if PGP_USECAPIFORRSA
	i = rsaSignHash(&bn, &sec->s, h, hash);
#else
	t = h->DERprefixsize;
	/* IKE does not put in hash OID */
	if (format == kPGPPublicKeyMessageFormat_IKE)
		t = 0;
	if (t+h->hashsize  > rsaSecMaxsig(seckey, format))
		return kPGPError_PublicKeyTooSmall;
	memcpy(sig, h->DERprefix, t);
	memcpy(sig+t, hash, h->hashsize);
	t += h->hashsize;

	i = rsaPrivateEncrypt(&bn, sig, t, &sec->s);
	pgpClearMemory( sig,  t);
#endif
	if (i >= 0) {
		t = 0;
		if (format == kPGPPublicKeyMessageFormat_X509) {
			/* Output unformatted, but with no leading zeros */
			format = kPGPPublicKeyMessageFormat_PKCS1;
			t += pgpBnPutFormatted(&bn, sig+t, bnBytes(&bn), format);
		} else {
			t += pgpBnPutFormatted(&bn, sig+t, bnBytes(&sec->s.n), format);
		}
		if (siglen)
			*siglen = (size_t)t;
		i = 0;
	}
	bnEnd(&bn);
	return i;

#endif /* PGP_SIGN_DISABLE */ /* ] */
}


/*
 * Re-encrypt a PGpSecKey with a new urn a PGPSecKey into a secret key.
 * A secret key is, after a non-specific prefix:
 *  0       1    Version (= 2 or 3)
 *  1       4    Timestamp
 *  5       2    Validity (=0 at present)
 *  7       1    Algorithm (=1 for RSA)
 * The following:
 *  0                2+u  MPI for modulus
 *  2+u              2+v  MPI for exponent
 *  4+u+v            1    Encryption algorithm (0 for none, 1 for IDEA)
 *  5+u+v            t    Encryption IV: 0 or 8 bytes
 *  5+t+u+v          2+w  MPI for d
 *  7+t+u+v+w        2+x  MPI for p
 *  9+t+u+v+w+x      2+y  MPI for q
 * 11+t+u+v+w+x+y    2+z  MPI for u
 * 13+t+u+v+w+x+y+z  2    Checksum (big-endian sum of all the bytes)
 * 15+t+u+v+w+x+y+z
 *
 * The Encryption algorithm is the cipher algorithm for the old-style
 * string-to-key conversion.  For the new type, it's 255, then a cipher
 * algorithm, then a string-to-key algorithm (variable-length),
 * then the encryption IV.  That's 16 bytes plus the string-to-key
 * conversion length.
 */

#if PGP_MACINTOSH
#pragma global_optimizer on
#endif
static int
rsaChangeLock(PGPSecKey *seckey, PGPEnv const *env, 
	PGPRandomContext const *rc, char const *phrase, size_t plen,
	PGPStringToKeyType s2ktype)
{
	RSAsecPlus *sec = (RSAsecPlus *)seckey->priv;
	PGPStringToKey *s2k = NULL;	/* Shut up warnings */
	PGPCipherVTBL const *cipher = NULL;	/* Shut up warnings */
	PGPCFBContext *cfb = NULL;	/* This is realy needed */
	PGPByte *p;
	PGPByte key[PGP_CIPHER_MAXKEYSIZE];
	int oldf = 0;				/* Shut up warnings */
	unsigned len;
	unsigned checksum;

	ASSERTRSA(seckey->pkAlg);
	if (sec->locked)
		return kPGPError_KeyIsLocked;

	len = bnBytes(&sec->s.n) + bnBytes(&sec->s.e) +
	      bnBytes(&sec->s.d) + bnBytes(&sec->s.p) +
	      bnBytes(&sec->s.q) + bnBytes(&sec->s.u) + 15;
	if (phrase) {
		/* Create old-style s2k unless new features requested */
		if (s2ktype == kPGPStringToKey_Simple)
			s2k = pgpS2KdefaultV3(env, rc);
		else
			s2k = pgpS2Kcreate(env, rc, s2ktype);
		if (!s2k)
			return kPGPError_OutOfMemory;
		cipher = pgpCipherDefaultKeyV3(env);
		pgpAssert(cipher);
		if (!cipher) {
			pgpS2Kdestroy(s2k);
			return kPGPError_OutOfMemory;
		}
		len += cipher->blocksize;
		cfb = pgpCFBCreate( PGPGetContextMemoryMgr( pgpenvGetContext( env ) ),
							cipher);
		if (!cfb) {
			pgpS2Kdestroy(s2k);
			return kPGPError_OutOfMemory;
		}
		oldf = pgpS2KisOldVers(s2k);
		if (!oldf)
			len += 1 + s2k->encodelen;
	}
	if (len > sec->ckalloc) {
		PGPError err = kPGPError_NoErr;
		if( IsNull( sec->cryptkey ) ) {
			sec->cryptkey = (PGPByte *)
				pgpContextMemAlloc( sec->context, len, 0 );
			if( IsNull( sec->cryptkey ) ) {
				err = kPGPError_OutOfMemory;
			}
		} else {
			err = pgpContextMemRealloc( sec->context,
				(void **)&sec->cryptkey, len, 0 );
		}
		if( IsPGPError( err ) ) {
			PGPFreeCFBContext(cfb);
			pgpS2Kdestroy(s2k);
			return err;
		}
		sec->ckalloc = (size_t)len;
	}
	sec->cklen = len;
	p = sec->cryptkey;

	/* Okay, no more errors possible!   Start installing data */
	p += pgpBnPutPlain(&sec->s.n, p);
	p += pgpBnPutPlain(&sec->s.e, p);

	/* Encryption parameters */
	if (!phrase) {
		*p++ = 0;	/* Unencrypted */
	} else {
		if (oldf) {
			*p++ = cipher->algorithm;
		} else {
			*p++ = 255;
			*p++ = cipher->algorithm;
			memcpy(p, s2k->encoding, s2k->encodelen);
			p += s2k->encodelen;
		}
		/* Create IV */
		pgpRandomGetBytes(rc, p, cipher->blocksize);
		pgpStringToKey(s2k, phrase, plen, key, cipher->keysize);
		PGPInitCFB(cfb, key, p);
		pgpS2Kdestroy(s2k);
		p += cipher->blocksize;
		/* Wipe key *immediately* */
		pgpClearMemory( key,  cipher->keysize);
	}

	/* Now install d, p, q and u, encrypted */
	checksum = 0;
	p += pgpBnPut(&sec->s.d, p, cfb, &checksum, sec->v3);
	p += pgpBnPut(&sec->s.p, p, cfb, &checksum, sec->v3);
	p += pgpBnPut(&sec->s.q, p, cfb, &checksum, sec->v3);
	p += pgpBnPut(&sec->s.u, p, cfb, &checksum, sec->v3);
	pgpChecksumPutOld(checksum, p, cfb);
	p += 2;
	pgpAssert((ptrdiff_t)len == p - sec->cryptkey);

	if (cfb)
		PGPFreeCFBContext(cfb);
	return 0;	/* Success */
}
#if PGP_MACINTOSH
#pragma global_optimizer reset
#endif

static size_t
rsaSecBufferLength(PGPSecKey const *seckey)
{
	RSAsecPlus const *sec = (RSAsecPlus *)seckey->priv;

	return sec->cklen;
}

static void
rsaSecToBuffer(PGPSecKey const *seckey, PGPByte *buf)
{
	RSAsecPlus const *sec = (RSAsecPlus *)seckey->priv;

	memcpy(buf, sec->cryptkey, sec->cklen);
}

/* Fill in secret key structure */
static void
rsaFillSecKey(PGPSecKey *seckey, RSAsecPlus *sec)
{
	seckey->pkAlg	            = kPGPPublicKeyAlgorithm_RSA;
	seckey->priv	            = sec;
	seckey->destroy             = rsaSecDestroy;
	seckey->pubkey              = rsaPubkey;
	seckey->islocked            = rsaIslocked;
	seckey->lockingalgorithm    = rsaLockingAlgorithm;
	seckey->s2ktype             = rsaS2KType;
	seckey->convertpassphrase   = rsaConvertPassphrase;
	seckey->unlock              = rsaUnlock;
	seckey->lock                = rsaLock;
	seckey->decrypt             = rsaDecrypt;
	seckey->maxdecrypted        = rsaSecMaxdecrypted;
	seckey->maxsig              = rsaSecMaxsig;
	seckey->maxesk              = rsaSecMaxesk;
	seckey->sign                = rsaSign;
	seckey->changeLock          = rsaChangeLock;
	seckey->bufferLength        = rsaSecBufferLength;
	seckey->toBuffer            = rsaSecToBuffer;
}


PGPSecKey *
rsaSecFromBuf(
	PGPContextRef	context,
	PGPByte const *	buf,
	size_t			size,
	PGPBoolean		v3,
	PGPError *		error)
{
	PGPSecKey *seckey;
	RSAsecPlus *sec;
	PGPByte *cryptk;
	PGPError	err	= kPGPError_OutOfMemory;
	PGPMemoryMgrRef		mgr	= PGPGetContextMemoryMgr( context );
	PGPEnv *			pgpEnv = pgpContextGetEnvironment( context );

	bnInit();
	cryptk = (PGPByte *)pgpContextMemAlloc(context,
		size, kPGPMemoryMgrFlags_Clear);
	if (cryptk) {
		sec = (RSAsecPlus *)PGPNewSecureData( mgr, sizeof(*sec), 0 );
		if (sec) {
			pgpClearMemory( sec, sizeof(*sec) );
			sec->context	= context;
			sec->v3			= v3;
			seckey = (PGPSecKey *)
				pgpContextMemAlloc(context,
					sizeof(*seckey), kPGPMemoryMgrFlags_Clear);
			if (seckey) {
				seckey->context	= context;
				
				memcpy(cryptk, buf, size);
				bnBegin(&sec->s.n, mgr, FALSE);
				bnBegin(&sec->s.e, mgr, FALSE);
				bnBegin(&sec->s.d, mgr, TRUE);
				bnBegin(&sec->s.p, mgr, TRUE);
				bnBegin(&sec->s.q, mgr, TRUE);
				bnBegin(&sec->s.u, mgr, TRUE);
				sec->cryptkey = cryptk;
				sec->cklen = sec->ckalloc = size;
				sec->locked = 1;
				/* We only need this to try unlocking... */
				seckey->pkAlg = kPGPPublicKeyAlgorithm_RSA;
				seckey->priv = sec;
				
				if (rsaUnlock(seckey, pgpEnv, NULL, 0, FALSE) >= 0) {
					if (rsaKeyTooBig (NULL, &sec->s) ||
						bnBits(&sec->s.n) > 2048) {
						bnEnd (&sec->s.n);
						bnEnd (&sec->s.e);
						err = kPGPError_KeyTooLarge;
					} else {
						rsaFillSecKey(seckey, sec);
						*error = 0;
						return seckey;	/* Success! */
					}
				}

				/* Ka-boom.  Delete and free everything. */
				pgpClearMemory( cryptk,  size);
				pgpContextMemFree( context, seckey);
			}
			PGPFreeData( sec );			/* Wipes as it frees */
		}
		pgpContextMemFree(context, cryptk);
	}
	*error = err;
	return NULL;
}

#if PGP_RSA_KEYGEN

/*
 * PGPRandomContext to use for primeGen callback.  We really should enhance
 * primeGen to pass an arg parameter along with the limit value.
 */
static PGPRandomContext const *staticrc;

/* Random callback for primeGen */
static unsigned randcallback(unsigned limit)
{
	return pgpRandomRange(staticrc, limit);
}

/*
 * Generate an RSA secret key with modulus of the specified number of bits.
 * We choose public exponent from the #define value above.
 * The high two bits of each prime are always
 * set to make the number more difficult to factor by forcing the
 * number into the high end of the range.
 * Make callbacks to progress function periodically.
 * Secret key is returned in the unlocked form, with no passphrase set.
 * fastgen is an unused flag which is used by the discrete log keygens to
 * allow use of canned primes.
 */
PGPSecKey *
rsaSecGenerate(
	PGPContextRef	context,
	unsigned bits, PGPBoolean fastgen,
	PGPRandomContext const *rc,
	int progress(void *arg, int c), void *arg, PGPError *error)
{
	PGPSecKey *seckey;
	RSAsecPlus *sec;
	BigNum t;		/* temporary */
	unsigned ent;			/* Entropy */
	int i;
	int exp = RSA_DEFAULT_EXPONENT;
	PGPMemoryMgrRef		mgr	= PGPGetContextMemoryMgr( context );
	PGPEnv *			pgpEnv = pgpContextGetEnvironment( context );

	(void) fastgen;

	*error = kPGPError_NoErr;

	/* Initialize local pointers (simplify cleanup below) */
	seckey = NULL;
	sec = NULL;
	bnBegin(&t, mgr, TRUE);

	/* Allocate data structures */
	seckey = (PGPSecKey *)pgpContextMemAlloc( context, 
		sizeof(*seckey), kPGPMemoryMgrFlags_Clear );
	if (!seckey)
		goto memerror;
	seckey->context	= context;
	sec = (RSAsecPlus *)PGPNewSecureData( mgr, sizeof(*sec), 0 );
	if (!sec)
		goto memerror;
	sec->context	= context;
	sec->v3			= TRUE;
	
	/* n is not inherently sensitive, but holds sensitive intermediates */
	bnBegin(&sec->s.n, mgr, TRUE);
	bnBegin(&sec->s.e, mgr, FALSE);
	bnBegin(&sec->s.d, mgr, TRUE);
	bnBegin(&sec->s.p, mgr, TRUE);
	bnBegin(&sec->s.q, mgr, TRUE);
	bnBegin(&sec->s.u, mgr, TRUE);
	
	if (bnSetQ(&sec->s.e, exp))
		goto bnerror;

	/* Find p - choose a starting place */
	if (pgpBnGenRand(&sec->s.p, rc, bits/2, 0xC0, 1, bits/2-3) < 0)
		goto bnerror;

	/* And search for a prime */
	staticrc = rc;
	i = bnPrimeGen(&sec->s.p, randcallback, progress, arg, exp, 0);
	if (i < 0)
		goto bnerror;
	pgpAssert(bnModQ(&sec->s.p, exp) != 1);

	/* Make sure p and q aren't too close together */

	/* Bits of entropy needed to generate q. */
	ent = (bits+1)/2 - 3;
	/* Pick random q until we get one not too close to p */
	do {
		/* Visual separator between the two progress indicators */
		if (progress != NULL)
			progress(arg, ' ');
		if (pgpBnGenRand(&sec->s.q, rc, (bits+1)/2, 0xC0, 1, ent) < 0)
			goto bnerror;
		ent = 0;	/* No entropy charge next time around */
		if (bnCopy(&sec->s.n, &sec->s.q) < 0)
			goto bnerror;
		if (bnSub(&sec->s.n, &sec->s.p) < 0)
			goto bnerror;
		/* Note that bnSub(a,b) returns abs(a-b) */
	} while (bnBits(&sec->s.n) < bits/2-5);

	i = bnPrimeGen(&sec->s.q, randcallback, progress, arg, exp, 0);
	if (i < 0)
		goto bnerror;
	pgpAssert(bnModQ(&sec->s.p, exp) != 1);

	/* Wash the random number pool. */
	pgpRandomStir(rc);

	/* Ensure that q is larger */
	if (bnCmp(&sec->s.p, &sec->s.q) > 0)
		bnSwap(&sec->s.p, &sec->s.q);

	/*
	 * Now we compute d,
	 * the decryption exponent, from the encryption exponent.
	 */

	/* Decrement q temporarily */
	(void)bnSubQ(&sec->s.q, 1);
	/* And u = p-1, to be divided by gcd(p-1,q-1) */
	if (bnCopy(&sec->s.u, &sec->s.p) < 0)
		goto bnerror;
	(void)bnSubQ(&sec->s.u, 1);

	/* Use t to store gcd(p-1,q-1) */
	if (bnGcd(&t, &sec->s.q, &sec->s.u) < 0) {
		goto bnerror;
	}

	/* Let d = (p-1) / gcd(p-1,q-1) (n is scratch for the remainder) */
	i = bnDivMod(&sec->s.d, &sec->s.n, &sec->s.u, &t);
	if (i < 0)
		goto bnerror;
	pgpAssert(bnBits(&sec->s.n) == 0);

	/* Now we have q-1 and d = (p-1) / gcd(p-1,q-1) */
	/* Find the product, n = lcm(p-1,q-1) = c * d */
	if (bnMul(&sec->s.n, &sec->s.q, &sec->s.d) < 0)
		goto bnerror;

	/* Find the inverse of the exponent mod n */
	i = bnInv(&sec->s.d, &sec->s.e, &sec->s.n);
	if (i < 0)
		goto bnerror;
	pgpAssert(!i);	/* We should NOT get an error here */

	/*
	 * Now we have the comparatively simple task of computing
	 * u = p^-1 mod q.
	 */

	/* But it *would* be nice to have q back first. */
	(void)bnAddQ(&sec->s.q, 1);

	/* Now compute u = p^-1 mod q */
	i = bnInv(&sec->s.u, &sec->s.p, &sec->s.q);
	if (i < 0)
		goto bnerror;
	pgpAssert(!i);	/* p and q had better be relatively prime! */

	/* And finally,  n = p * q */
	if (bnMul(&sec->s.n, &sec->s.p, &sec->s.q) < 0)
		goto bnerror;

	/* And that's it... success! */

	/* Fill in structs */
	sec->cryptkey = NULL;
	sec->ckalloc = sec->cklen = 0;
	sec->locked = 0;
	rsaFillSecKey(seckey, sec);

	/* Fill in cryptkey structure, unencrypted */
	rsaChangeLock (seckey, pgpEnv, rc, NULL, 0, kPGPStringToKey_Simple);

	goto done;

bnerror:
	bnEnd(&sec->s.n);
	bnEnd(&sec->s.e);
	bnEnd(&sec->s.d);
	bnEnd(&sec->s.p);
	bnEnd(&sec->s.q);
	bnEnd(&sec->s.u);
	/* Fall through */
memerror:
	if ( IsntNull( seckey ) )
		pgpContextMemFree( context, seckey);
	if ( IsntNull( sec ) )
		PGPFreeData( sec );			/* Wipes as it frees */
	seckey = NULL;
	*error = kPGPError_OutOfMemory;
	/* Fall through */
done:
	bnEnd(&t);
	return seckey;
}

#endif /* PGP_RSA_KEYGEN */


#endif /* PGP_RSA */
