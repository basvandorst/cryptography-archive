/*
 * pgpStr2Key.h -- A PGP String to Key Conversion System
 *
 * $Id: pgpStr2Key.h,v 1.12 1997/06/25 19:41:31 lloyd Exp $
 */
#ifndef Included_pgpStr2Key_h
#define Included_pgpStr2Key_h

#include "pgpUsuals.h"	/* For PGPByte */

PGP_BEGIN_C_DECLARATIONS


#include "pgpOpaqueStructs.h"


struct PGPStringToKey {
	void *priv;
	PGPByte const *encoding;	/* For transmission */
	unsigned encodelen;
	void (*destroy) (struct PGPStringToKey *s2k);
	int (*stringToKey) (struct PGPStringToKey const *s2k, char const *str,
			    size_t slen, PGPByte *key, size_t klen);
};


/* Deallocate the structure */
#define pgpS2Kdestroy(s2k) (s2k)->destroy(s2k)

/* Turn a string into a key */
#define pgpStringToKey(s2k,s,sl,k,kl) (s2k)->stringToKey(s2k,s,sl,k,kl)

/*
 * Decodes a byte string into a PGPStringToKey object.  Returns either an
 * error <0 or the length >=0 of the encoded string2key data found in
 * the buffer.  If s2k is non-NULL, a structure is allocated and
 * returned.
 */
int  pgpS2Kdecode (PGPStringToKey **s2k,
	PGPEnv const *env, PGPByte const *buf, size_t len);

/*
 * Returns the default PGPStringToKey based on the setting in the
 * environment.  This is expected to be the usual way to get such
 * structures.
 */
PGPStringToKey  *pgpS2Kdefault (PGPEnv const *env,
				      PGPRandomContext const *rc);

/*
 * Return old default PGPStringToKey compatible with PGP V2 */
PGPStringToKey  *pgpS2KdefaultV2(PGPEnv const *env,
					  PGPRandomContext const *rc);

int  pgpS2KisOldVers (PGPStringToKey const *s2k);

/*
 * Specific PGPStringToKey creation functions.  Use these functions to
 * specifically request a certain string to key algorithm.
 */
PGPStringToKey  *pgpS2Ksimple (PGPEnv const *env,
				     PGPHashVTBL const *h);
PGPStringToKey  *pgpS2Ksalted (PGPEnv const *env,
				     PGPHashVTBL const *h,
				     PGPByte const *salt8);
PGPStringToKey  *pgpS2Kitersalt (PGPEnv const *env,
				       PGPHashVTBL const *h,
				       PGPByte const *salt8,
				       PGPUInt32 bytes);

/*
 * The PGPStringToKey objects are encoded using the follow table:
 *
 * type		number	s2k arguments
 * ----		-----	-------------
 * simple	\000	hash specifier
 * salted	\001	hash specifier + salt8
 * itersalt	\003	hash specifier + salt8 + (compressed) count
 *
 */

PGP_END_C_DECLARATIONS

#endif /* Included_pgpStr2Key_h */
