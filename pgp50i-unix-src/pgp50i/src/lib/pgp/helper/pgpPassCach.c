/*
* pgpPassCach.c -- A Passphrase Cache -- keep a cache of passphrases and
* try them against ESKs and Secret Keys. This way you only have to
* type your passphrase once, it gets cached, and then it gets used
* again and again.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpPassCach.c,v 1.2.2.1 1997/06/07 09:50:11 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpESK.h"
#include "pgpMem.h"
#include "pgpPassCach.h"
#include "pgpErr.h"
#include "pgpPubKey.h"
#include "pgpUsuals.h"

struct PgpPass {
	char *pass;
	size_t passlen;
	struct PgpPass *next;
};

struct PgpPassCache {
	struct PgpEnv const *env;
	struct PgpPass *passlist;
};

struct PgpPassCache *
pgpPassCacheCreate (struct PgpEnv const *env)
	{
			struct PgpPassCache *cache;

			cache = (struct PgpPassCache *) pgpMemAlloc (sizeof (*cache));
			if (cache) {
				 memset (cache, 0, sizeof (*cache));
				 cache->env = env;
			}
				
			return cache;
	}

void
pgpPassCacheDestroy (struct PgpPassCache *cache)
{
	struct PgpPass *p, *t;

	if (!cache)
		return;

	for (p = cache->passlist; p; p = t) {
			t = p->next;

			memset (p->pass, 0, p->passlen);
			pgpMemFree (p->pass);
			memset (p, 0, sizeof (*p));
			pgpMemFree (p);
	}
memset (cache, 0, sizeof (*cache));
pgpMemFree (cache);
}

int
pgpPassCacheAdd (struct PgpPassCache *cache, char const *pass, size_t passlen)
	{
		struct PgpPass *p;
		char *buf;

		if (!cache || !pass || ! passlen)
			return PGPERR_BADPARAM;

		p = (struct PgpPass *) pgpMemAlloc (sizeof (*p));
		if (!p)
			return PGPERR_NOMEM;

		buf = (char *)pgpMemAlloc (passlen + 1);
		if (!buf) {
			pgpMemFree (p);
			return PGPERR_NOMEM;
}

		memcpy (buf, pass, passlen);
		buf[passlen] = '\0';
		memset (p, 0, sizeof (*p));

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
pgpPassCacheTryEsk (struct PgpPassCache const *cache, struct PgpESK const *esk,
			int (*tryKey) (void *arg, byte const *key, size_t keylen),
			void *tryarg, byte *key, size_t *keylen)
	{
		struct PgpPass *p;
		int i;

			if (!cache || !esk || !key || !keylen || !tryKey)
				 return PGPERR_BADPARAM;

			if (pgpEskType (esk) != PGP_ESKTYPE_PASSPHRASE)
				 return PGPERR_BADPARAM;

			for (p = cache->passlist; p; p = p->next) {
					i = pgpEskConvDecrypt (esk, cache->env, p->pass, p->passlen,
								key);

					if (i < 0)	 		/* error */
						return i;

					if (i > 0) {			/* got a key -- try it */
							*keylen = i;
							i = tryKey (tryarg, key, *keylen);
							if (!i)		 /* it worked */
								 return 1;
					}
			}
			/* none of the cached passphrases worked */
			return 0;
}

/*
* Returns:
* < 0 : error
* = 0 : could not unlock key
* > 0 : success -- key unlocked
*/
int
pgpPassCacheTryKey (struct PgpPassCache const *cache, struct PgpSecKey *seckey)
	{
			struct PgpPass *p;
			int i;

			if (!cache || !seckey)
				 return PGPERR_BADPARAM;

			for (p = cache->passlist; p; p = p->next) {
				 i = pgpSecKeyUnlock (seckey, cache->env, p->pass, p->passlen);
				 if (i)
				 		return i;
			}
			return 0;
	}
