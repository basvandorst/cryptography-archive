/*
* pgpFilteredSet.c -- Filtered variant on pgpKeySet.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpFilteredSet.c,v 1.3.2.5 1997/06/07 09:50:23 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <string.h>
#include <ctype.h>

#include "pgpKDBint.h"
#include "pgpDebug.h"
#include "pgpTypes.h"
#include "pgpMem.h"
#include "pgpUsuals.h"


/* Data types */

/* Type of filter */
typedef enum {
	kFilterAll = 1,				/* Allow all keys */
	kFilterUserID,				/* Single userid substring match */
	kFilterKeyID				/* KeyID match */
} FilterType;

/* Filter specifiers */
typedef struct FilterSpecUserID_
{
	char		name[256];		/* Userid substring (not null termed) */
	size_t		namelen;		/* Length of name */
} FilterSpecUserID;

typedef struct FilterSpecKeyID_
{
	size_t		length;
	byte		keyID[8];	 	/* KeyID (raw bytes) */
} FilterSpecKeyID;

/* Union of all filtering modes */
typedef union FilterSpec_
{
	FilterSpecUserID	fsUid;
	FilterSpecKeyID		fsKid;
} FilterSpec;

/* Private data for KeySet of type filter */
typedef struct FilterPriv_
{
	FilterType		ftype;
	FilterSpec		fspec;
} FilterPriv;


/* UserID Filtering */


/*
* Stolen from pgpRngPub.c:
*
* Return pointer to first instance of (s1,l1) in (s0,l0),
* ignoring case. Uses a fairly simple-minded algorithm.
* Search for the first char of s1 in s0, and when we have it,
* scan for the rest.
*
* Is it worth mucking with Boyer-Moore or the like?
*/
static char const *
xmemimem(char const *s0, size_t l0, char const *s1, size_t l1)
{
	char c0, c1, c2;
	size_t l;

	/*
	* The trivial cases - this means that NULL inputs are very legal
	* if the correspinding lengths are zero.
	*/
	if (l0 < l1)
		return NULL;
	if (!l1)
		return s0;
	l0 -= l1;

	c1 = tolower((unsigned char)*s1);
	do {
		c0 = tolower((unsigned char)*s0);
		if (c0 == c1) {
			l = 0;
			do {
				if (++l == l1)
					return s0;
				c0 = tolower((unsigned char)s0[l]);
				c2 = tolower((unsigned char)s1[l]);
			} while (c0 == c2);
		}
		s0++;
	} while (l0--);
	return NULL;
}

/*
* Check to see if object is a member of filtered set. Get all names
* associated with the specified object and see if any match the
* filter spec.
*/
static Boolean
filterUserIDIsMember (PGPKeySet *set, FilterSpecUserID *fs,
	RingObject const *obj)
{
	RingSet		*rset = pgpKeyDBRingSet (set->keyDB);
	RingIterator *riter;
	char const	*keyname;
	size_t			keynamelen;

	if (!ringSetIsMember (rset, obj))
		return 0;

	riter = ringIterCreate (rset);
	pgpAssert (riter);
	ringIterSeekTo (riter, (RingObject *)obj);
	ringIterRewind (riter, 2);	/* Point at first name on key */
	while (ringIterNextObject (riter, 2) == 2) {
		RingObject *nameobj = ringIterCurrentObject (riter, 2);
		if (ringObjectType (nameobj) != RINGTYPE_NAME)
		 continue;
		keyname = ringNameName (rset, nameobj, &keynamelen);
		if (xmemimem (keyname, keynamelen, fs->name, fs->namelen)) {
			/* Success */
			ringObjectRelease (nameobj);
			ringIterDestroy (riter);
			return 1;
		}
	}
	/* No matches */
	ringIterDestroy (riter);
	return 0;
}

/*
* Check to see if object is a member of filtered set.
*/
static Boolean
filterKeyIDIsMember (PGPKeySet *set, FilterSpecKeyID *fs,
	RingObject const *obj)
{
	RingSet		*rset = pgpKeyDBRingSet (set->keyDB);
	RingIterator *riter;
	RingObject const *key = NULL;
	byte			pkalg;
	byte			keyid[8];

	if (!ringSetIsMember (rset, obj))
		return 0;

	riter = ringIterCreate (rset);
	pgpAssert (riter != NULL);
	ringIterSeekTo (riter, (RingObject *) obj);
	key = ringIterCurrentObject(riter, 1);
	if (key != NULL)
		ringKeyID8 (rset, key, &pkalg, keyid);
	ringIterDestroy (riter);

	return key != NULL && !memcmp (keyid + sizeof(keyid) - fs->length,
									fs->keyID, fs->length);
}


/* General filtering routines */


static Boolean
filteredSetIsMember (PGPKeySet *keys, RingObject const *obj)
{
	FilterPriv	*fpriv;
	Boolean			rslt = 0;

	pgpa(pgpaPGPKeySetValid(keys));

	fpriv = (FilterPriv *) keys->private;
	pgpAssert (fpriv);

	switch (fpriv->ftype) {
	case kFilterAll:
		rslt = ringSetIsMember (pgpKeyDBRingSet(keys->keyDB), obj);
		break;
	case kFilterUserID:
		rslt = filterUserIDIsMember (keys, &fpriv->fspec.fsUid, obj);
		break;
	case kFilterKeyID:
		rslt = filterKeyIDIsMember (keys, &fpriv->fspec.fsKid, obj);
		break;
	}
	return rslt;
}	

static void
filteredSetDestroy (PGPKeySet *keys)
{
	FilterPriv *fpriv = (FilterPriv *) keys->private;
	if (fpriv)
		pgpFree (fpriv);
	return;
}


/*
* Create a filtered KeySet based on the same KeyDB as the specified
* original. The original must be unfiltered. The filtering is based
* on a name substring (not keyid).
*/
PGPKeySet *
pgpFilterKeySetUserID (PGPKeySet *origset, char const *name, size_t namelen)
{
	PGPKeySet	 	*newset;
	FilterPriv		*fpriv;

	/* Check for substring too long to fit */
	if (namelen > sizeof(fpriv->fspec.fsUid.name))
		return NULL;

	/* Can only filter a set once at present, error if already filtered */
	if (origset->destroy == filteredSetDestroy)
		return NULL;

	fpriv = (FilterPriv *) pgpMemAlloc (sizeof (*fpriv));
	if (!fpriv)
		return NULL;

	pgpClearMemory (fpriv, sizeof (*fpriv));
	fpriv->ftype = kFilterUserID;
	pgpCopyMemory (name, fpriv->fspec.fsUid.name, namelen);
  fpriv->fspec.fsUid.namelen = namelen;

	newset = pgpCopyKeySet (origset);
	if (!newset) {
  	pgpFree (fpriv);
		return NULL;
	}

	newset->private = fpriv;
	newset->isMember = filteredSetIsMember;
	newset->destroy = filteredSetDestroy;
	return newset;
}

/*
* Create a filtered KeySet based on the same KeyDB as the specified
* original. The original must be unfiltered. The filtering is based
* on KeyID.
*/
PGPKeySet *
pgpFilterKeySetKeyID (PGPKeySet *origset, byte const *keyID,
						size_t keyIDLength)
{
	PGPKeySet	 	*newset;
	FilterPriv		*fpriv;

	/* Can only filter a set once at present, error if already filtered */
	if (origset->destroy == filteredSetDestroy)
		return NULL;

	fpriv = (FilterPriv *) pgpMemAlloc (sizeof (*fpriv));
	if (!fpriv)
		return NULL;

	pgpClearMemory (fpriv, sizeof (*fpriv));
	fpriv->ftype = kFilterKeyID;
	pgpCopyMemory (keyID, fpriv->fspec.fsKid.keyID, keyIDLength);
	fpriv->fspec.fsKid.length = keyIDLength;

	newset = pgpCopyKeySet (origset);
	if (!newset) {
		pgpFree (fpriv);
		return NULL;
	}

	newset->private = fpriv;
	newset->isMember = filteredSetIsMember;
	newset->destroy = filteredSetDestroy;
	return newset;
}


/*
* Create a filtered KeySet based on the same KeyDB as the specified
* original. The original must be unfiltered. The filtering type
* is chosen automatically based on the string.
*
* Currently, a "0x" prefix looks up by keyID, otherwise
* by userid name substring matching.
*/
PGPKeySet *
pgpFilterKeySetAuto (PGPKeySet *origset, char const *pattern)
{
	PGPKeySet *	subset = NULL;

	if (pattern[0] == '0' && tolower(pattern[1]) == 'x')
	{
		size_t	keyIDLength;
		byte	keyID[8];
		int		idIndex;
		int		patIndex;
		char 	ch;
		byte	theByte;

		if (pattern[2] == '\0')
			return NULL;
		idIndex = sizeof(keyID);
		for (patIndex = strlen(pattern) - 2;
				patIndex > 0 && idIndex > 0;
				patIndex -= 2)
		{
			ch = tolower(pattern[patIndex]);
			if (ch >= '0' && ch <= '9')
				theByte = ch - '0';
			else if (ch >= 'a' && ch <= 'f')
				theByte = ch - 'a' + 10;
			else if (patIndex < 2)
				theByte = 0;
			else
				return NULL;
			theByte <<= 4;

			ch = tolower(pattern[patIndex + 1]);
			if (ch >= '0' && ch <= '9')
				theByte += ch - '0';
			else if (ch >= 'a' && ch <= 'f')
				theByte += ch - 'a' + 10;
			else
				return NULL;

			keyID[--idIndex] = theByte;
		}
		while (idIndex > 0 && (idIndex % 4) != 0)
			keyID[--idIndex] = 0;
		keyIDLength = sizeof(keyID) - idIndex;
		subset = pgpFilterKeySetKeyID(origset, keyID + idIndex, keyIDLength);
	}
	else
	{
		subset = pgpFilterKeySetUserID(origset, pattern, strlen(pattern));
	}
	return subset;
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
