/*
 * pgpKeySpec.c
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpKeySpec.c,v 1.4.2.2 1997/06/07 09:51:27 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>

#include "pgpKeySpec.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpTimeDate.h"
#include "pgpUsuals.h"

/*
 * The things in a public key that aren't part of the (algorithm-specific)
 * mathematical public key.  Personally, I'd prefer that this didn't exist!
 */
struct PgpKeySpec {
	PgpVersion version;			/* Version of key to create */
	word32 creation;			/* Creation date */
	word16 validity;			/* Validity period */
};

struct PgpKeySpec *
pgpKeySpecCreate(struct PgpEnv const *env)
{
	struct PgpKeySpec *ks;

	ks = (struct PgpKeySpec *)pgpMemAlloc(sizeof(*ks));
	if (ks) {
		/* Default values, based on environment */
		/*ks->version = pgpenvGetInt(env, PGPENV_VERSION, NULL, NULL);*/
		ks->version = PGPVERSION_2_6;	/* Default to this unless overridden */
		ks->creation = pgpTimeStamp(pgpenvGetInt(env, PGPENV_TZFIX,
		                                         NULL, NULL));
		ks->validity = 0;	/* Forever */
	}
	return ks;
}

struct PgpKeySpec *
pgpKeySpecCopy(struct PgpKeySpec const *ks)
{
	struct PgpKeySpec *ks2;

	ks2 = (struct PgpKeySpec *)pgpMemAlloc(sizeof(*ks2));
	if (ks2)
		memcpy(ks2, ks, sizeof(*ks2));
	return ks2;
}

void
pgpKeySpecDestroy(struct PgpKeySpec *ks)
{
	memset(ks, 0, sizeof(*ks));
	pgpMemFree(ks);
}

PgpVersion
pgpKeySpecVersion(struct PgpKeySpec const *ks)
{
	return ks->version;
}

word32
pgpKeySpecCreation(struct PgpKeySpec const *ks)
{
	return ks->creation;
}

word16
pgpKeySpecValidity(struct PgpKeySpec const *ks)
{
	return ks->validity;
}

int
pgpKeySpecSetVersion(struct PgpKeySpec *ks, PgpVersion ver)
{
	switch(ver) {
	  case PGPVERSION_2:
	  case PGPVERSION_2_6:
	  case PGPVERSION_3:
		ks->version = ver;
		return 0;
	}
	/* default */
	return PGPERR_KEY_VERSION;
}

int
pgpKeySpecSetCreation(struct PgpKeySpec *ks, word32 creation)
{
	ks->creation = creation;
	return 0;
}

int
pgpKeySpecSetValidity(struct PgpKeySpec *ks, word16 validity)
{
	ks->validity = validity;
	return 0;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
