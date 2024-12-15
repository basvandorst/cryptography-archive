/*
 * pgpPassCach.c -- A Passphrase Cache -- keep a cache of passphrases and
 * try them against ESKs and Secret Keys.  This way you only have to
 * type your passphrase once, it gets cached, and then it gets used
 * again and again.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPassCach.c,v 1.17 1998/02/24 22:38:28 hal Exp $
 */
#include <string.h>

#include "pgpConfig.h"

#include <stdio.h>

#include "pgpESK.h"
#include "pgpMem.h"
#include "pgpPassCach.h"
#include "pgpErrors.h"
#include "pgpPubKey.h"
#include "pgpEnv.h"
#include "pgpContext.h"

typedef struct PgpPass {
	char *				pass;
	size_t 				passlen;
	struct PgpPass *	next;
	DEBUG_STRUCT_CONSTRUCTOR( PgpPass )
} PgpPass ;

struct PGPPassCache {
	PGPContextRef	context;
	PGPEnv const *	env;
	PgpPass *		passlist;
	DEBUG_STRUCT_CONSTRUCTOR( PGPPassCache )
};

PGPPassCache *
pgpPassCacheCreate (PGPEnv const *env)
{
	PGPPassCache *cache;
	PGPContextRef		context	= pgpenvGetContext( env );

	cache = (PGPPassCache *) pgpContextMemAlloc( context,
		sizeof (*cache), kPGPMemoryMgrFlags_Clear);
	if (cache) {
		cache->env = env;
		cache->context	= context;
	}
		
	return cache;
}

void
pgpPassCacheDestroy (PGPPassCache *cache)
{
	PgpPass *p, *t;
	PGPContextRef		context;

	if (!cache)
		return;

	context	= cache->context;

	for (p = cache->passlist; p; p = t) {
		t = p->next;

		pgpClearMemory (p->pass, p->passlen);
		pgpContextMemFree( context, p->pass);
		pgpClearMemory( p,  sizeof (*p));
		pgpContextMemFree( context, p);
	}
	pgpClearMemory( cache,  sizeof (*cache));
	pgpContextMemFree( context, cache);
}

int
pgpPassCacheAdd (PGPPassCache *cache, char const *pass, size_t passlen)
{
	PgpPass *p;
	char *buf;
	PGPContextRef		context;
	
	pgpAssertAddrValid( cache, PGPPassCache );

	if (!cache || !pass || ! passlen)
		return kPGPError_BadParams;
	context	= cache->context;

	context	= cache->context;
	
	p = (PgpPass *) pgpContextMemAlloc( context,
		sizeof (*p), kPGPMemoryMgrFlags_Clear);
	if (!p)
		return kPGPError_OutOfMemory;

	buf = (char *)pgpContextMemAlloc( context,
		passlen + 1, kPGPMemoryMgrFlags_Clear);
	if (!buf) {
		pgpContextMemFree( context, p);
		return kPGPError_OutOfMemory;
	}

	memcpy (buf, pass, passlen);
	buf[passlen] = '\0';
	pgpClearMemory( p,  sizeof (*p));

	p->pass = buf;
	p->passlen = passlen;
	p->next = cache->passlist;
	cache->passlist = p;

	return 0;
}


/*
 * Returns:
 * < 0 : error
 * = 0 : no phrases worked
 * > 0 : a pass phrase worked -- session key in key, size in keylen
 */
int
pgpPassCacheTryEsk (PGPPassCache const *cache, PGPESK const *esk,
		    int (*tryKey) (void *arg, PGPByte const *key, size_t keylen),
		    void *tryarg, PGPByte *key, size_t *keylen)
{
	PgpPass *p;
	int i;

	if (!cache || !esk || !key || !keylen || !tryKey)
		return kPGPError_BadParams;

	if (pgpEskType (esk) != PGP_ESKTYPE_PASSPHRASE)
		return kPGPError_BadParams;

	for (p = cache->passlist; p; p = p->next) {
		i = pgpEskConvDecrypt (esk, cache->env, p->pass, p->passlen,
				       key);

		if (i < 0)		/* error */
			return i;

		if (i > 0) {		/* got a key -- try it */
			*keylen = i;
			i = tryKey (tryarg, key, *keylen);
			if (!i)		/* it worked */
				return 1;
		}
	}
	/* none of the cached passphrases worked */
	return 0;
}

/*
 *  Returns:
 * < 0 : error
 * = 0 : could not unlock key
 * > 0 : success -- key unlocked
 */
int
pgpPassCacheTryKey (PGPPassCache const *cache, PGPSecKey *seckey)
{
	PgpPass *p;
	int i;

	if (!cache || !seckey)
		return kPGPError_BadParams;

	for (p = cache->passlist; p; p = p->next) {
		i = pgpSecKeyUnlock (seckey, cache->env, p->pass, p->passlen, FALSE);
		if (i)
			return i;
	}
	return 0;
}
