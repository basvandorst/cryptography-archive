/*
* pgpHash.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpHash.c,v 1.2.2.3 1997/06/07 09:49:57 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpHash.h"
#include "pgpMD5.h"
#include "pgpSHA.h"
#include "pgpRIPEMD160.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpUsuals.h"

static struct PgpHashContext *
dohashInit (struct PgpHash const *h, struct PgpHashContext *hc)
	{
		hc->priv = pgpMemAlloc (h->context_size);
		if (hc->priv) {
		 hc->hash = h;
		 h->init (hc->priv);
		 return hc;
		}
		return NULL;
	}

/* Generic Hash creation code */
struct PgpHashContext *
pgpHashCreate (struct PgpHash const *h)
	{
		struct PgpHashContext *hc;

		hc = (struct PgpHashContext *)pgpMemAlloc (sizeof (*hc));
		if (!hc)
		 return NULL;

		if (!dohashInit (h, hc)) {
		 pgpMemFree (hc);
		 return NULL;
		}

return hc;
}

static void
hashWipe (struct PgpHashContext *hc)
{
 if (hc && hc->priv) {
memset (hc->priv, 0, hc->hash->context_size);
pgpMemFree(hc->priv);
	}
}

void
pgpHashDestroy (struct PgpHashContext *hc)
	{
		if (!hc)
		 return;

hashWipe (hc);
memset (hc, 0, sizeof (*hc));
pgpMemFree (hc);
}

struct PgpHashContext *
pgpHashClone(struct PgpHashContext const *hc)
	{
		struct PgpHashContext *dest;

dest = pgpHashCreate (hc->hash);
if (dest)
 memcpy (dest->priv, hc->priv, hc->hash->context_size);
return dest;
}

void
pgpHashCopy(struct PgpHashContext *dest, struct PgpHashContext const *src)
{
 pgpAssert(dest->hash == src->hash);
 memcpy (dest->priv, src->priv, src->hash->context_size);
}

/* Access to all known hashes */
static struct PgpHash const * const hashList[] = {
		NULL,
		&HashMD5,
		&HashSHA,
		&HashRIPEMD160
	};

struct PgpHash const *
pgpHashByNumber (byte type)
	{
		if (type < sizeof(hashList)/sizeof(*hashList))
		 return hashList[type];
		return NULL;
	}

/*
* Given a hash name, return the corresponding hash.
*/
struct PgpHash const *
pgpHashByName (char const *name, size_t namelen)
{
 unsigned i;
 struct PgpHash const *hash;

		for (i = 0; i < sizeof(hashList)/sizeof(hashList[0]); i++) {
			hash = hashList[i];
			if (hash && !memcmp (name, hash->name, namelen) &&
			hash->name[namelen] == '\0')
			 return hash; /* That's it. */
		}
		return NULL; /* Not found */
	}

/*
* Given a list of hash identifiers, create a list of hash contexts.
* Ignores unknown algorithms. Returns the number of PgpHashContexts
* created and stored in the "hashes" buffer, or an Error (and none created)
* on error.
*/
int
pgpHashListCreate (byte const *buf, struct PgpHashContext **hashes,
		 unsigned len)
	{
		int i;
		struct PgpHash const *hash;
		struct PgpHashContext *hashlist;

		if (!hashes)
		 return PGPERR_BADPARAM;
		*hashes = NULL;

		if (!len)
		 return 0;

		hashlist = (struct PgpHashContext *)pgpMemAlloc(len*sizeof(*hashlist));
		memset (hashlist, 0, len * sizeof (*hashlist));

		i = 0;
		while (len--) {
		hash = pgpHashByNumber (*buf++);
		if (hash) {
			if (!dohashInit (hash, hashlist+i)) {
				while (i--)
				 hashWipe (hashlist+i);
				pgpMemFree (hashlist);
				return PGPERR_NOMEM;
			}
			i++;
		}
	}

*hashes = hashlist;
return i;
}

void
pgpHashListDestroy (struct PgpHashContext *hashes, unsigned len)
{
 while (len--)
  hashWipe (hashes+len);
 pgpMemFree (hashes);
}

struct PgpHashContext *
pgpHashListFind (struct PgpHashContext *hashes, unsigned len,
		 struct PgpHash const *h)
	{
		while (len--) {
		 if (hashes->hash == h)
		  return hashes;
		 hashes++;
		}
		return NULL;
	}
