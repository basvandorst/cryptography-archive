/*
 * pgpMemDB.c -- Memory-based implementation of PGPKeyDB
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpMemDB.c,v 1.14.2.1 1997/06/07 09:50:28 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpKDBint.h"
#include "pgpDebug.h"
#include "pgpTypes.h"
#include "pgpMem.h"
#include "pgpUsuals.h"
#include "pgpEnv.h"
#include "pgpMemPool.h"
#include "pgpRngPub.h"

typedef struct MemDB_
{
		RingSet *				currentSet;
	RingSet * frozenSet;
} MemDB;


static Boolean
memDBIsMutable(PGPKeyDB *db)
{
		(void)db;		/*	Avoid warning */
		return TRUE;
}

static Boolean
memDBObjIsMutable(PGPKeyDB *db, RingObject *testObj)
{
		(void)db;				/*	Avoid warning */
		(void)testObj;		/*	Avoid warning */
		return TRUE;
}

static Boolean
memDBIsDirty(PGPKeyDB *db)
{
		(void)db;		/*	Avoid warning */
		return FALSE;
}

static RingSet *
memDBGetRingSet(PGPKeyDB *db)
{
		MemDB *			mdb;
		RingSet *newset;

		pgpa(pgpaPGPKeyDBValid(db));
		mdb = (MemDB *)db->private;
		if (!mdb->frozenSet) {
			newset = ringSetCreate (ringSetPool (mdb->currentSet));
				pgpAssert (newset);
				ringSetAddSet (newset, mdb->currentSet);
				ringSetFreeze (newset);
				mdb->frozenSet = newset;
		}
		return mdb->frozenSet;
}

static PGPError
memDBAdd(PGPKeyDB *db, RingSet *toAdd)
{
		MemDB *			mdb;

		pgpa(pgpaPGPKeyDBValid(db));
		mdb = (MemDB *)db->private;
		if (mdb->frozenSet) {
			ringSetDestroy (mdb->frozenSet);
				mdb->frozenSet = NULL;
		}
		return ringSetAddSet(mdb->currentSet, toAdd);
}

static PGPError
memDBChanged(PGPKeyDB *db, RingSet *changedkeys)
{
		return pgpReSortKeys (db, changedkeys);
}

static PGPError
memDBRemove(PGPKeyDB *db, RingObject *toRemove)
{
		MemDB *			mdb;

		pgpa(pgpaPGPKeyDBValid(db));
		mdb = (MemDB *)db->private;
		if (mdb->frozenSet) {
			ringSetDestroy (mdb->frozenSet);
				mdb->frozenSet = NULL;
		}
		return ringSetRemObject(mdb->currentSet, toRemove);
}

static PGPError
memDBDummy(PGPKeyDB *db)
{
		pgpa(pgpaPGPKeyDBValid(db));

		(void)db;				/*	Avoid warning */
		return PGPERR_OK;
}

static void
memDBDestroy(PGPKeyDB *db)
{
		MemDB *			mdb;

		/*	Can't assert pgpaPGPKeyDBValid because refCount == 0 now */
		pgpa((pgpaAddrValid(db, PGPKeyDB),pgpaAssert(db->refCount==0)));

		mdb = (MemDB *)db->private;
		ringSetDestroy((RingSet *)mdb->currentSet);
		if (mdb->frozenSet)
			ringSetDestroy (mdb->frozenSet);

		pgpFree(mdb);
}

/*	Creates a new empty memory-based key database */
PGPKeyDB *
pgpKeyDBCreate(void)
{
		PGPKeyDB *		db;
		MemDB *			mdb;

		db = pgpKeyDBCreateInternal();
		if (db == NULL)
				return NULL;

		mdb = pgpNew(MemDB);
		if (mdb == NULL)
		{
				pgpFree(db);
				return NULL;
		}
		mdb->currentSet = (void *)ringSetCreate(pgpRingPool);
		mdb->frozenSet = NULL;

		db->private = mdb;
		db->magic = PGPKDBMEMMAGIC;
		db->isMutable = memDBIsMutable;
		db->objIsMutable = memDBObjIsMutable;
		db->isDirty = memDBIsDirty;
		db->getRingSet = memDBGetRingSet;
		db->add = memDBAdd;
		db->changed = memDBChanged;
		db->remove = memDBRemove;
		db->commit = memDBDummy;
		db->revert = memDBDummy;
		db->reload = memDBDummy;
		db->destroy = memDBDestroy;

		pgpKeyDBInitInternal(db);

		return db;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
*/
