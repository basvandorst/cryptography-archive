/*
 * Charmap.h
 *
 * Mapping tables between different character sets.
 *
 * $Id: pgpCharMap.h,v 1.6 1997/06/25 19:41:05 lloyd Exp $
 */
#ifndef Included_pgpCharMap_h
#define Included_pgpCharMap_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

struct PGPEnv;

/* The identity charmap; maps all characters onto themselves */
extern unsigned char const charMapIdentity[256];

/*
 * Fills in the charmaps associated with the charset and returns 0
 * or returns a PGP error if the set isn't found or a paramter is
 * invalid.
 */
PGPError pgpCharmaps (char const *charset, int setlen,
			 PGPByte const **toLocal, PGPByte const **toLatin1);

/*
 * Converts a string from one charset to another using the input
 * charmap.  The input, of size inlen, is converted through the map
 * and put into output.
 */
void pgpCharmapConvert (PGPByte const *input, size_t inlen, PGPByte *output,
			PGPByte const *charmap);

/* Two macros to convert strings between the local and latin1 charsets */
#define PGP_CONVERT_TO_LOCAL(in,len,out,env) \
	pgpCharmapConvert (in,len,out,\
		pgpenvGetPointer (env, PGPENV_CHARMAPTOLOCAL,NULL))

#define PGP_CONVERT_TO_LATIN1(in,len,out,env) \
	pgpCharmapConvert (in,len,out,\
		pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1,NULL))

PGP_END_C_DECLARATIONS

#endif /* Included_pgpCharMap_h */
