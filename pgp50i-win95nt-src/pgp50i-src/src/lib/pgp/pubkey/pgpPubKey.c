/*
* pgpPubKey.c
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpPubKey.c,v 1.8.2.6 1997/06/10 04:26:34 hal Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpDebug.h"
#include "pgpErr.h"
#include "pgpPubKey.h"
#include "pgpKeyMisc.h"
#include "pgpRSAKey.h"
#include "pgpElGKey.h"
#include "pgpDSAKey.h"
#include "pgpFixedKey.h"

struct PgpPkAlg {
	byte pkAlg;
	int use;
};

static struct PgpPkAlg const pgpKnownPkAlgs[] = {
	{ PGP_PKALG_RSA, PGP_PKUSE_SIGN_ENCRYPT },
	{ PGP_PKALG_RSA_SIG, PGP_PKUSE_SIGN },
	{ PGP_PKALG_RSA_ENC, PGP_PKUSE_ENCRYPT },
	{ PGP_PKALG_ELGAMAL, PGP_PKUSE_ENCRYPT },
	{ PGP_PKALG_DSA, PGP_PKUSE_SIGN }
};

/* Return whether an algorithm can sign, encrypt, or do both */
struct PgpPkAlg const *
pgpPkalgByNumber(byte pkalg)
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
pgpKeyUse(struct PgpPkAlg const *pkalg)
{
	if (!pkalg)
		return 0;		 /* It's no use... */
	return pkalg->use;
}

struct PgpPubKey *
pgpPubKeyFromBuf(byte pkAlg, byte const *p, size_t len, int *error)
{
	struct PgpPubKey *pub;

	switch (ALGMASK(pkAlg)) {
	case PGP_PKALG_RSA:
	case PGP_PKALG_RSA_SIG:
	case PGP_PKALG_RSA_ENC:
#if(!NO_RSA)
		pub = rsaPubFromBuf(p, len, error);
		if (pub)
			pub->pkAlg = pkAlg;
#else
		pub = NULL;
		*error = PGPERR_KEY_NO_RSA;
		/*Avoid Warnings:*/
		(void) p;
		(void) len;
#endif
		return pub;
	case PGP_PKALG_ELGAMAL:
		pub = elgPubFromBuf(p, len, error);
		if (pub)
			pub->pkAlg = pkAlg;
		return pub;
	case PGP_PKALG_DSA:
		pub = dsaPubFromBuf(p, len, error);
		if (pub)
			pub->pkAlg = pkAlg;
		return pub;
	default:
		*error = PGPERR_KEY_PKALG;
		return 0;
	}
	
}

struct PgpSecKey *
pgpSecKeyFromBuf(byte pkAlg, byte const *p, size_t len, int *error)
{
	struct PgpSecKey *sec;

	switch(ALGMASK(pkAlg)) {
	case PGP_PKALG_RSA:
	case PGP_PKALG_RSA_SIG:
	case PGP_PKALG_RSA_ENC:
#if (!NO_RSA)
		sec = rsaSecFromBuf(p, len, error);
		if (sec)
			sec->pkAlg = pkAlg;
#else
		sec = NULL;
		*error = PGPERR_KEY_NO_RSA;
		/*Avoid Warnings:*/
		(void) p;
		(void) len;
#endif
		return sec;
	case PGP_PKALG_ELGAMAL:
		sec = elgSecFromBuf(p, len, error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
	case PGP_PKALG_DSA:
		sec = dsaSecFromBuf(p, len, error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
	default:
		*error = PGPERR_KEY_PKALG;
		return 0;
	}
}

/*
* Scan a buffer and return the size of the part which corresponds to
* a public key. This is intended to be "lightweight" and not require
* the overhead of instantiating a struct PgpPubKey.
*
* This rturns 0 if the size is unknown.
*/
size_t
pgpPubKeyPrefixSize(byte pkAlg, byte const *p, size_t len)
{
	switch(ALGMASK(pkAlg)) {
	case PGP_PKALG_RSA:
	case PGP_PKALG_RSA_SIG:
	case PGP_PKALG_RSA_ENC:
#if (!NO_RSA)
				return rsaPubKeyPrefixSize(p, len);
#else
				/* We know it is 2 MPI values, just parse past those */
				return pgpBnParse(p, len, 2, NULL, NULL);
#endif
		
	case PGP_PKALG_ELGAMAL:
		return elgPubKeyPrefixSize(p, len);
	case PGP_PKALG_DSA:
		return dsaPubKeyPrefixSize(p, len);
	default:
		return 0;
	}
}

/*
* The "progress" function is called periodically during key
* generation to indicate that *something* is happening.
* The "int c" gives a bit of indication as to *what* is
* happening. In fact, this is just the character that text-mode
* PGP prints! It will do as well as any other sort of enum
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
* (Sorry, no way to know how many phases there are...)
*/

struct PgpSecKey *
pgpSecKeyGenerate(struct PgpPkAlg const *alg, unsigned bits,
	Boolean fastgen,
	struct PgpRandomContext const *rc,
	int (*progress)(void *arg, int c), void *arg, int *error)
{
	struct PgpSecKey *sec;
	byte pkAlg;

	pgpAssert (alg);
	pkAlg = alg->pkAlg;
	switch(ALGMASK(pkAlg)) {
	case PGP_PKALG_RSA:
#if (!NO_RSA && !NO_RSA_KEYGEN)
		sec = rsaSecGenerate(bits, fastgen, rc, progress, arg, error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
#else
		*error = PGPERR_KEY_NO_RSA;
		return(NULL);
#endif
	case PGP_PKALG_ELGAMAL:
		sec = elgSecGenerate(bits, fastgen, rc, progress, arg, error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
	case PGP_PKALG_DSA:
		sec = dsaSecGenerate(bits, fastgen, rc, progress, arg, error);
		if (sec)
			sec->pkAlg = pkAlg;
		return sec;
	default:
		*error = PGPERR_KEY_PKALG;
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
pgpSecKeyEntropy(struct PgpPkAlg const *pkAlg, unsigned bits, Boolean fastgen)
{
	unsigned xbits;

	(void)fastgen;

	pgpAssert (pkAlg);
	switch (ALGMASK(pkAlg->pkAlg)) {
		case PGP_PKALG_RSA:
		case PGP_PKALG_RSA_SIG:
		case PGP_PKALG_RSA_ENC:
			return bits + 64;
		case PGP_PKALG_ELGAMAL:
			xbits = pgpDiscreteLogExponentBits(bits)*3/2 + 128;
			/* Always return fastgen value
			* if (!fastgen || !pgpElGfixed (bits, NULL, NULL, NULL, NULL))
			*/
				xbits += ELGDUMMYBITS;
			return xbits;
		case PGP_PKALG_DSA:
			/*
			* Count the ~160 bits each needed to generate the self signature
			* on the key's userid, and on the subkey, in addition to the
			* secret exponent. That is why the 3* below.
			*/
			xbits = 3*((bits<=1024)?160:pgpDiscreteLogExponentBits(bits)) + 128;
			/* Always return fastgen value
			* if (!fastgen || !pgpDSAfixed (bits, NULL, NULL, NULL, NULL))
			*/
				xbits += DSADUMMYBITS;
			return xbits;
	}
	pgpAssert (0);
	return 0;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
