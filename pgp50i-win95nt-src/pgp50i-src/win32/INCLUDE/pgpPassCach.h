/*
* pgpPassCach.h -- A Passphrase Cache
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpPassCach.h,v 1.4.2.1 1997/06/07 09:50:12 mhw Exp $
*/

#ifndef PGPPASSCACH_H
#define PGPPASSCACH_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPassCache;
#ifndef TYPE_PGPPASSCACHE
#define TYPE_PGPPASSCACHE 1
typedef struct PgpPassCache PgpPassCache;
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpESK;
#ifndef TYPE_PGPESK
#define TYPE_PGPESK 1
typedef struct PgpESK PgpESK;
#endif

struct PgpSecKey;
#ifndef TYPE_PGPSECKEY
#define TYPE_PGPSECKEY 1
typedef struct PgpSecKey PgpSecKey;
#endif

struct PgpPassCache PGPExport *pgpPassCacheCreate (struct PgpEnv const *env);
void PGPExport pgpPassCacheDestroy (struct PgpPassCache *cache);
int PGPExport pgpPassCacheAdd (struct PgpPassCache *cache, char const *pass,
		size_t passlen);
int PGPExport pgpPassCacheTryEsk (struct PgpPassCache const *cache,
			struct PgpESK const *esk,
			int (*tryKey) (void *arg, byte const *key,
				size_t keylen),
			void *tryarg, byte *key, size_t *keylen);
int PGPExport pgpPassCacheTryKey (struct PgpPassCache const *cache,
			struct PgpSecKey *seckey);

#ifdef __cplusplus
}
#endif

#endif /* PGPPASSCACH_H */
