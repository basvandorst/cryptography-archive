/*
 * pgpPassCach.h -- A Passphrase Cache
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPassCach.h,v 1.8 1997/06/25 19:41:27 lloyd Exp $
 */

#ifndef Included_pgpPassCach_h
#define Included_pgpPassCach_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"


PGPPassCache  *pgpPassCacheCreate (PGPEnv const *env);
void  pgpPassCacheDestroy (PGPPassCache *cache);
int  pgpPassCacheAdd (PGPPassCache *cache, char const *pass,
		     size_t passlen);
int  pgpPassCacheTryEsk (PGPPassCache const *cache,
			PGPESK const *esk,
			int (*tryKey) (void *arg, PGPByte const *key,
				       size_t keylen),
			void *tryarg, PGPByte *key, size_t *keylen);
int  pgpPassCacheTryKey (PGPPassCache const *cache,
			PGPSecKey *seckey);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpPassCach_h */
