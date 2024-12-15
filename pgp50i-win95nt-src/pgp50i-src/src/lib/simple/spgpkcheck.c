/*
* spgpkcheck.c -- Simple PGP API helper -- Check key validity
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpkcheck.c,v 1.4.2.1 1997/06/07 09:51:55 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpTimeDate.h"
#include "pgpTypes.h"
#include "spgp.h"
#include "spgpint.h"

/* Return 1 if key is OK to use, else 0 */
int
spgpKeyOKToEncrypt (RingSet const *set, RingObject *key, int UseUntrustedKeys)
{
	union RingObject *name;
	struct RingIterator *iter;
	PGPTime expiration;
	int trust;
	word16 validity;
	PgpTrustModel pgptrustmodel;

	pgptrustmodel = pgpTrustModel (ringSetPool (set));

	if (ringKeyRevoked (set, key)) {
		return 0;
	}
	if (ringKeyDisabled (set, key)) {
		return 0;
	}

	if ((expiration = ringKeyExpiration (set, key)) > 0 &&
	expiration <= pgpGetTime()) {
		return 0;
	}

	iter = ringIterCreate (set);
	pgpAssert(iter != NULL);
	ringIterSeekTo (iter, key);
	while (ringIterNextObject (iter, 2) > 0) {
		name = ringIterCurrentObject (iter, 2);
		pgpAssert(name != NULL);
		if (ringObjectType (name) == RINGTYPE_NAME) {
			ringIterDestroy (iter);
			if (pgptrustmodel == PGPTRUST0) {
				trust = ringNameTrust (set, name);
				switch (trust) {
				case PGP_NAMETRUST_UNKNOWN:
				case PGP_NAMETRUST_UNTRUSTED:
				case PGP_NAMETRUST_MARGINAL:
					return UseUntrustedKeys;
				default:
					return 1;
				}
			} else {			/* new trust model */
				validity = ringNameValidity (set, name);
				if (validity == 0 || !ringTrustValid (set, validity)) {
					return UseUntrustedKeys;
				}
				return 1;
			}
		}
	}
	/* No name on key */
	return 0;
}

/* Return 1 if key is OK to use for sig verification, else 0 */
int
spgpKeyOKToSign (RingSet const *set, RingObject *key)
{
	/* Same test as above, don't allow untrusted keys */
	return spgpKeyOKToEncrypt (set, key, 0);
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
