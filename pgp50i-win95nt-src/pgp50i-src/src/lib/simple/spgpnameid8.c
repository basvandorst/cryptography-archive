/*
* spgpnameid8.c -- Simple PGP API helper -- Name from key ID
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpnameid8.c,v 1.4.2.1 1997/06/07 09:51:55 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

/*
* Get a name associated with the specified key, known by its ID and alg.
* Return a pointer to the name and set its length in *namelen. Return
* NULL if there is an error.
*/
char const *
spgpNameByID8 (RingSet const *set, byte pkalg, byte keyID[8], size_t *namelen)
{
	RingObject		*key;
	RingIterator	*iter;
	RingObject		*name,
					*bestname = NULL;
	unsigned		validity,
					bestvalidity = 0;
	int				level;
	size_t			lname;
	Boolean			firstpass = 1;
	char const		*sname;
	PgpTrustModel	 pgptrustmodel;

	pgptrustmodel = pgpTrustModel (ringSetPool (set));

	if (namelen)
		*namelen = 0;

	key = ringKeyById8 (set, pkalg, keyID);
	if (!key)
		return NULL;

	iter = ringIterCreate (set);
	if (!set)
		return NULL;

	level = ringIterSeekTo (iter, key);
	if (level <= 0) {
		ringIterDestroy (iter);
		return NULL;
	}
	level += 1;

	/* Select most valid name */
	while (ringIterNextObject (iter, level) > 0) {
		name = ringIterCurrentObject (iter, level);
		if (ringObjectType(name) != RINGTYPE_NAME)
			continue;
		if (pgptrustmodel == PGPTRUST0)
			validity = ringNameTrust (set, name);
		else
			validity = ringNameValidity (set, name);
		if (validity > bestvalidity || firstpass) {
			if (bestname)
				ringObjectRelease (bestname);
			bestvalidity = validity;
			bestname = name;
			ringObjectHold (bestname);
			firstpass = 0;
		}
	}
	ringIterDestroy (iter);

	sname = ringNameName (set, bestname, &lname);
	ringObjectRelease (bestname);

	if (namelen)
		*namelen = lname;
	return sname;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
