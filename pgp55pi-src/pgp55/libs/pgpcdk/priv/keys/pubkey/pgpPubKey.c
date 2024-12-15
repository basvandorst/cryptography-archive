/*
 * $Id: pgpPubKey.c,v 1.17 1997/09/30 21:03:02 lloyd Exp $
 */

#include "pgpSDKBuildFlags.h"
#include "pgpConfig.h"

#include "pgpDebug.h"
#include "pgpErrors.h"
#include "pgpPubKey.h"
#include "pgpKeyMisc.h"
#include "pgpRSAKey.h"
#include "pgpElGKey.h"
#include "pgpDSAKey.h"
#include "pgpFixedKey.h"

#ifndef PGP_RSA
#error "PGP_RSA requires a value"
#endif

#ifndef PGP_RSA_KEYGEN
#error "PGP_RSA_KEYGEN requires a value"
#endif

/* Error code used within this module for RSA not available */
#define kPGPLocalError_RSANotAvailable	kPGPError_FeatureNotAvailable

struct PGPPkAlg
{
	PGPByte pkAlg;
	int use;
} ;

static PGPPkAlg const pgpKnownPkAlgs[] = {
#if PGP_RSA
	{ kPGPPublicKeyAlgorithm_RSA,     PGP_PKUSE_SIGN_ENCRYPT },
	{ kPGPPublicKeyAlgorithm_RSASignOnly, PGP_PKUSE_SIGN },
	{ kPGPPublicKeyAlgorithm_RSAEncryptOnly, PGP_PKUSE_ENCRYPT },
#else	/* PGP_RSA */
	{ kPGPPublicKeyAlgorithm_RSA,     			0 },
	{ kPGPPublicKeyAlgorithm_RSASignOnly,		0 },
	{ kPGPPublicKeyAlgorithm_RSAEncryptOnly, 	0 },
#endif	/* PGP_RSA */
	{ kPGPPublicKeyAlgorithm_ElGamal, PGP_PKUSE_ENCRYPT },
	{ kPGPPublicKeyAlgorithm_DSA,     PGP_PKUSE_SIGN }
};


/* Error codes which permit ringPoolCheck to continue */
PGPBoolean
pgpIsKeyRelatedError( PGPError err )
{
	if (err >= kPGPError_KEY_MIN && err <= kPGPError_KEY_MAX)
		return TRUE;
	if (err == kPGPLocalError_RSANotAvailable)
		return TRUE;
	return FALSE;
}


/* Return whether an algorithm can sign, encrypt, or do both */
PGPPkAlg const *
pgpPkalgByNumber(PGPByte pkalg)
{
	unsigned i;

	pkalg = ALGMASK(pkalg);
	for (i = 0; i < sizeof(pgpKnownPkAlgs)/sizeof(*pgpKnownPkAlgs); i++) {
		if (pgpKnownPkAlgs[i].pkAlg == pkalg)
			return pgpKnownPkAlgs + i;
	}
	return NULL;
}

int
pgpKeyUse(PGPPkAlg const *pkalg)
{
	if (!pkalg)
		return 0;		/* It's no use... */
	return pkalg->use;
}

PGPPubKey *
pgpPubKeyFromBuf(
	PGPContextRef	context,
	PGPByte			pkAlg,
	PGPByte const *	p,
	size_t			len,
	PGPError *		error)
{
	PGPPubKey *pub;

	switch (ALGMASK(pkAlg)) {
	  case kPGPPublicKeyAlgorithm_RSA:
	  case kPGPPublicKeyAlgorithm_RSASignOnly:
	  case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
#if PGP_RSA
		pub = rsaPubFromBuf( context, p, len, error);
		if (pub)
			pub->pkAlg = pkAlg;
#else	/* PGP_RSA */
		(void) p;
		(void) len;
		pub = NULL;
		*error = kPGPLocalError_RSANotAvailable;
#endif	/* PGP_RSA */
		return pub;
	  case kPGPPublicKeyAlgorithm_ElGamal:
	  	pub = elgPubFromBuf( context, p, len, error);
		if (pub)
			pub->pkAlg = pkAlg;
		return pub;
	  case kPGPPublicKeyAlgorithm_DSA:
	  	pub = dsaPubFromBuf( context, p, len, error);
		if (pub)
			pub->pkAlg = pkAlg;
		return pub;
	  default:
		*error = kPGPError_UnknownPublicKeyAlgorithm;
		return 0;
	}
	
}

PGPSecKey *
pgpSecKeyFromBuf(
	PGPContextRef	context,
	PGPByte			pkAlg,
	PGPByte const *	p,
	size_t			len,
	PGPError *		error)
{
	PGPSecKey *sec;

	switch(ALGMASK(pkAlg)) {
	  case kPGPPublicKeyAlgorithm_RSA:
	  case kPGPPublicKeyAlgorithm_RSASignOnly:
	  case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
#if PGP_RSA
		sec = rsaSecFromBuf( context, p, len, error);
		if (sec)
			sec->pkAlg = pkAlg;
#else	/* PGP_RSA */
		(void) p;
		(void) len;
		sec = NULL;
		*error = kPGPLocalError_RSANotAvailable;
#endif	/* PGP_RSA */
		return sec;
	  case kPGPPublicKeyAlgorithm_ElGamal:
		sec = elgSecFromBuf( context, p, len, error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
	  case kPGPPublicKeyAlgorithm_DSA:
		sec = dsaSecFromBuf( context, p, len, error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
	  default:
		*error = kPGPError_UnknownPublicKeyAlgorithm;
		return 0;
	}
}

/*
 * Scan a buffer and return the size of the part which corresponds to
 * a public key.  This is intended to be "lightweight" and not require
 * the overhead of instantiating a PGPPubKey.
 *
 * This rturns 0 if the size is unknown.
 */
size_t
pgpPubKeyPrefixSize(PGPByte pkAlg, PGPByte const *p, size_t len)
{
	switch(ALGMASK(pkAlg)) {
	case kPGPPublicKeyAlgorithm_RSA:
	case kPGPPublicKeyAlgorithm_RSASignOnly:
	case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
#if PGP_RSA
		  return rsaPubKeyPrefixSize(p, len);
#else
		  /* We know it is 2 MPI values, just parse past those */
		  return pgpBnParse(p, len, 2, NULL, NULL);
#endif
	case kPGPPublicKeyAlgorithm_ElGamal:
		  return elgPubKeyPrefixSize(p, len);
	case kPGPPublicKeyAlgorithm_DSA:
		  return dsaPubKeyPrefixSize(p, len);
	default:
		return 0;
	}
}

/*
 * The "progress" function is called periodically during key
 * generation to indicate that *something* is happening.
 * The "int c" gives a bit of indication as to *what* is
 * happening.  In fact, this is just the character that text-mode
 * PGP prints!  It will do as well as any other sort of enum
 * and makes life simple.
 *
 * The meaning of the character is somewhat type-dependent, but
 * the convention so far is:
 * '.' - Failed pseudoprimality test
 * '/' - Redoing initial setup work
 * '-' - Passed pseudoprimality test, further work needed
 * '+' - Passed pseudoprimality test, further work needed
 * '*' - Passed pseudoprimality test - almost done!
 * ' ' - Completed part of key generation, on to next phase.
 *       (Sorry, no way to know how many phases there are...)
 */

PGPSecKey *
pgpSecKeyGenerate(
	PGPContextRef	context,
	PGPPkAlg const *alg, unsigned bits,
	PGPBoolean fastgen,
	PGPRandomContext const *rc,
	int (*progress)(void *arg, int c), void *arg, PGPError *error)
{
	PGPSecKey *sec;
	PGPByte pkAlg;
 
	pgpAssert (alg);
	pkAlg = alg->pkAlg;
	switch(ALGMASK(pkAlg)) {
	  case kPGPPublicKeyAlgorithm_RSA:
#if PGP_RSA && PGP_RSA_KEYGEN
		sec = rsaSecGenerate( context, bits, fastgen, rc, progress, arg,
							  error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
#else
		*error = kPGPLocalError_RSANotAvailable;
		return NULL;
#endif
	  case kPGPPublicKeyAlgorithm_ElGamal:
		sec = elgSecGenerate(context, bits, fastgen, rc, progress, arg, error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
	  case kPGPPublicKeyAlgorithm_DSA:
		sec = dsaSecGenerate(context, bits, fastgen, rc, progress, arg, error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
	  default:
		*error = kPGPError_UnknownPublicKeyAlgorithm;
		return 0;
	}
}

/*
 * Return the amount of entropy needed to generate this key.
 * Entropy is a security parameter, controlling the amount of
 * unpredictability needed in the key.
 * We have to be sure to return a value at least as big as the actual
 * depletion of the randpool which the keygen creates.
 * For simplicity and predictability, we return the non-fastgen values
 * throughout, even though they are a bit larger for DSA & ELG keys.
 * (The +64 in the RSA case, and +128 in the DSA case, is for the secret
 * key encryption IV and salt when the secret key is encrypted with the
 * passphrase.)
 */
unsigned
pgpSecKeyEntropy(PGPPkAlg const *pkAlg, unsigned bits, PGPBoolean fastgen)
{
	unsigned xbits;

	(void)fastgen;

	pgpAssert (pkAlg);
	switch (ALGMASK(pkAlg->pkAlg)) {
	  case kPGPPublicKeyAlgorithm_RSA:
	  case kPGPPublicKeyAlgorithm_RSASignOnly:
	  case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
		  return bits + 64;
	  case kPGPPublicKeyAlgorithm_ElGamal:
		  xbits = pgpDiscreteLogExponentBits(bits)*3/2 + 128;
		  /* Always return fastgen value
		   * if (!fastgen || !pgpElGfixed (bits, NULL, NULL, NULL, NULL))
		   */
			  xbits += ELGDUMMYBITS;
		  return xbits;
	  case kPGPPublicKeyAlgorithm_DSA:
		  /*
		   * Count the ~160 bits each needed to generate the self signature
		   * on the key's userid, and on the subkey, in addition to the
		   * "q bits" secret exponent.  That is why the 2* below.
		   */
		  xbits = 2*((bits<=1024)?160:pgpDiscreteLogExponentBits(bits))
			  	+ ((bits<=1024)?160:pgpDiscreteLogQBits(bits)) + 128;
		  /* Always return fastgen value
		   * if (!fastgen || !pgpDSAfixed (bits, NULL, NULL, NULL, NULL))
		   */
			  xbits += DSADUMMYBITS;
		  return xbits;
	}
	pgpAssert (0);
	return 0;
}