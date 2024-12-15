/*
 * pgpKDBint.h -- Internal definitions for PGP KeyDB Library
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpKDBint.h,v 1.37.2.2 1997/06/07 09:50:24 mhw Exp $
 */

#ifndef PGPKDBINT_H
#define PGPKDBINT_H

#include <stdio.h>

#include "pgpKeyDB.h"
#include "pgpFileRef.h"
#include "pgpTypes.h"
#include "pgpUsuals.h"
#include "pgpErr.h"
#include "pgpMem.h"
#include "pgpMemPool.h"
#include "pgpRngPub.h"

/*
 * An internal object representing a source of keys, either a file,
 * some memory, or a union of other PGPKeyDBs
 */
struct PGPKeyDB_
{
	int					magic;
#define PGPKDBFILEMAGIC		0x1000
#define PGPKDBUNIONMAGIC	0x2000
#define PGPKDBMEMMAGIC		0x3000
	void *				private;
	int					refCount;
	PGPKeySet *			firstSetInDB;

	MemPool				keyPool;
	long				numKeys;
	PGPKey *			firstKeyInDB;
	PGPKey *			firstFreeKey;
	PGPSubKey * firstFreeSubKey;
	PGPUserID *			firstFreeUserID;
	PGPCert	*	 		 firstFreeCert;
	PGPKeyList *		keysByKeyID;

	Boolean				(*isMutable)	(PGPKeyDB *db);
	Boolean				(*objIsMutable)	(PGPKeyDB *db,
										RingObject *testObj);
	Boolean				(*isDirty)		(PGPKeyDB *db);
	RingSet *	 		 (*getRingSet)	(PGPKeyDB *db);

	/* The RingSet arguments below must be of whole keys */
	PGPError			(*add)			(PGPKeyDB *db,
										RingSet *toAdd);
	PGPError			(*changed)		(PGPKeyDB *db,
										RingSet *changed);
	PGPError			(*remove)		 (PGPKeyDB *db,
										RingObject *toRemove);

	PGPError			(*commit)		 (PGPKeyDB *db);
	PGPError			(*revert)		 (PGPKeyDB *db);
	PGPError			(*reload)		 (PGPKeyDB *db);
	void				(*destroy)		(PGPKeyDB *db);
};


/*
 * Objects that represent Aurora RingObjects.  If an attempt is made to
 * delete an object, and refCount > 0, then the removed flag should
 * be set to TRUE instead. The object can be physically deleted when the
 * refCount drops to zero.
 *
 * struct listhdr saves memory but not having to embed a PGPUserID in
 * a PGPKey, or a PGPCert in a PGPUserID, to maintain the doubly-linked
 * lists. It may be typecast to PGPUserID or PGPCert as required.
 */
struct listhdr {
struct listhdr *	next;
struct listhdr *	prev;
};

enum certtype {keycert, uidcert};

struct PGPCert_
{
PGPCert *		 	next;
PGPCert *		 	prev;
	RingObject *		cert;
	enum certtype	 	type;
	union {
		PGPUserID *		userID; /* type == uidcert */
			 PGPKey *		 key;		/* type == keycert */
		} up;
		uint	 		 	refCount;
		Boolean		 		 removed;
	long userVal;
	};

	struct PGPUserID_
	{
	PGPUserID *			 next;
	PGPUserID *			 prev;
		struct listhdr		 certs;	/* doubly-linked list of UserID certs */
		RingObject *	 	userID;
		PGPKey *	 		 key;
		int		 		 	refCount;
		Boolean		 		 removed;
	long	userVal;
};

	struct PGPSubKey_
	{
	PGPSubKey *	next;
	PGPSubKey *	prev;
	RingObject *	subKey;
	PGPKey *	key;
	int	refCount;
	Boolean	removed;
	long	userVal;
};

struct PGPKey_
{
	int					refCount;
	PGPKeyDB *			keyDB;
	PGPKey *			nextKeyInDB;
	RingObject *		key;
        struct listhdr subKeys;	/* doubly linked list of Subkeys */
	struct listhdr		userIDs;	/* doubly-linked list of UserIDs */
        long userVal;
};

/*
 * The external object which represents a filtered PGPKeyDB
 */
struct PGPKeySet_
{
	void *				private;
	int					refCount;
	PGPKeyDB *			keyDB;
	PGPKeySet *			prevSetInDB;
	PGPKeySet *			nextSetInDB;
	PGPKeyList *		firstListInSet;

	Boolean				(*isMember)	(PGPKeySet *set, RingObject const *obj);
	void				(*destroy)	(PGPKeySet *set);
};

/*
 * The external object which represents a sorted PGPKeySet
 */
struct PGPKeyList_
{
	int					refCount;
	PGPKeySet *			keySet;
	PGPKeyOrdering		order;

	PGPKeyList *		prevListInSet;
	PGPKeyList *		nextListInSet;
	PGPKeyIter *		firstIterInList;

	long				keyCount;	 	/* size of keys array */
	PGPKey **	 		 keys;
};

/*
 * The external object which represents a key iterator
 */
struct PGPKeyIter_
{
	PGPKeyList *		 keyList;		 /* parent keylist */
	PGPKeyIter *		prevIterInList;	/* next iter in keylist */
	PGPKeyIter *		nextIterInList;	/* next iter in keylist */

	long				keyIndex;	 	/* current key index in keylist */
        PGPKey *			key;			/* ptr to current key */
	PGPSubKey * subKey; /* ptr to current subkey */
	PGPUserID *			userID;			/* ptr to current userid */
	PGPCert *	 		 uidCert;		 /* ptr to current userid cert */
};

#define CHECKREMOVED(x) \
if (!(x) || (x)->removed) \
	return PGPERR_KEYDB_OBJECT_DELETED


/* Global variables: These aren't valid until pgpLibInit() is called */

/* Export some of these for simple API support */
extern RingPool PGPKDBExport *		pgpRingPool;
extern PgpEnv PGPKDBExport *		pgpEnv;
extern Boolean PGPKDBExport			pgpRngSeeded;
extern Boolean						pgpPrefsChanged;
extern int							pgpLibSetup;
extern PgpRandomContext *		 	pgpRng;

/* Internal functions */

PGPKeyDB *
pgpKeyDBCreateInternal(void);

void
pgpKeyDBInitInternal(PGPKeyDB *db);

PGPKeyDB *
pgpKeyDBCreate(void);

PGPKeySet *
pgpKeyDBRootSet(PGPKeyDB *db);

RingSet *
pgpKeyDBRingSet(PGPKeyDB *db);

Boolean
pgpKeyDBIsMutable(PGPKeyDB *db);

Boolean
pgpKeyDBIsDirty(PGPKeyDB *db);

PGPError
pgpKeyDBChanged(PGPKeyDB *db, RingSet *changedkeys);

PGPError
pgpCommitKeyDB(PGPKeyDB *db);

PGPError
pgpCheckKeyDB (PGPKeyDB *db);

PGPError
pgpRevertKeyDB(PGPKeyDB *db);

PGPError
pgpReloadKeyDB(PGPKeyDB *db);

PGPKeyDB *
pgpCreateFileKeyDB (PGPFileRef const *fileRef, int trusted, int private,
	int writeable, RingPool *ringpool, PGPError *error);

PGPKeyDB *
pgpCreateMemFileKeyDB (byte *buf, size_t length, RingPool *ringpool,
	PGPError *error);

PGPKeyDB *
pgpCreateUnionKeyDB (PGPError *error);

PGPError
pgpUnionKeyDBAdd (PGPKeyDB *kdb, PGPKeyDB *kdbnew);

void
pgpKeyIterAddKey (PGPKeyList *keys, long idx);

void
pgpKeyIterRemoveKey (PGPKeyList *keys, long idx);

PGPError
pgpCopyKey (struct RingSet *src, union RingObject *obj,
			struct RingSet **dest);

PGPKey *
pgpGetKeyByRingObject (PGPKeyDB *keys, union RingObject *obj);

void pgpDeallocSubKey (PGPSubKey *subkey);

void
pgpDeallocUserID (PGPUserID *userid);

void
pgpDeallocCert (PGPCert *cert);

PGPError
pgpExpandKey (PGPKey *key);

PGPError
pgpCollapseKey (PGPKey *key);

PGPError
pgpAddObjects (PGPKeyDB *keys, struct RingSet *addset);

PGPError
pgpRemoveObject (PGPKeyDB *keys, union RingObject *obj);

void
pgpIncKeyDBRefCount(PGPKeyDB *db);

void
pgpFreeKeyDB(PGPKeyDB *db);

PGPError
pgpBuildKeyPool(PGPKeyDB *db, Boolean deleteFlag);

PGPError
pgpReSortKeys(PGPKeyDB *db, RingSet *set);

PGPKeyDB *
pgpKeyDBCreateInternal (void);

void
pgpKeyDBDestroyInternal (PGPKeyDB *db);

int
pgpDearmorKeyFile (PgpEnv *env, PGPFileRef *InputFileRef,
	byte **OutputBuffer, size_t *OutputBufferLen);

int
pgpDearmorKeyBuffer (PgpEnv *env, byte *InputBuffer, size_t InputBufferLen,
	byte **OutputBuffer, size_t *OutputBufferLen);

int
pgpWriteArmoredSetFile (FILE *fp, RingSet const *set, PgpEnv *env);

int
pgpWriteArmoredSetBuffer (byte *buf, size_t *buflen, RingSet const *set,
	PgpEnv *env);


/* Internal preferences calls */

PGPError
pgpSetDefaultPrefsInternal (void);

PGPError
pgpSetDefaultKeyPathInternal (void);

PGPError
pgpLoadPrefsInternal (void);

PGPError
pgpSavePrefsInternal (void);

#endif /* PGPKBDINT.h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
