/*
 * pgpUnionDB.c -- Manage PGPKeyDBs which are unions of others
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpUnionDB.c,v 1.15.2.2 1997/06/07 09:50:31 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpKDBint.h"
#include "pgpDebug.h"

/*	Private data for union type PGPKeyDB */
typedef struct UnionDB {
		Boolean bdirty;
		RingSet *rset;
		struct UnionDBRec *udbr;
} UnionDB;


/*	One list element per unioned PGPKeyDB */
typedef struct UnionDBRec {
		struct UnionDBRec 	*next;
		PGPKeyDB						*keyDB;
} UnionDBRec;



/******************** Virtual Functions ************************/


/*	Returns true if any component is mutable */
static Boolean
udbIsMutable (PGPKeyDB *kdb)
{
		UnionDB *udb = (UnionDB *)kdb->private;
		UnionDBRec *udbr = udb->udbr;

		while (udbr) {
				if (((udbr->keyDB)->isMutable)(udbr->keyDB)) {
						return 1;
				}
				udbr = udbr->next;
		}
		return 0;
}

/*	True if object is mutable in any component */
static Boolean
udbObjIsMutable (PGPKeyDB *kdb, RingObject *testObj)
{	
		UnionDB *udb = (UnionDB *)kdb->private;
		UnionDBRec *udbr = udb->udbr;

		while (udbr) {
				if ((udbr->keyDB)->objIsMutable (udbr->keyDB, testObj))
						return 1;
				udbr = udbr->next;
		}
		return 0;
}

/*	Returns true if any component is dirty */
static Boolean
udbIsDirty (PGPKeyDB *kdb) {
		UnionDB *udb = (UnionDB *)kdb->private;

		return udb->bdirty;
}

/*	Mark it dirty */
static void
udbDirty (UnionDB *udb)
{
		udb->bdirty = 1;
		if (udb->rset) {
				ringSetDestroy (udb->rset);
				udb->rset = NULL;
		}
}

/*	Return a union of all keys in sets */
static RingSet *
udbGetRingSet (PGPKeyDB *kdb) {
		UnionDB	*udb = (UnionDB *)kdb->private;
		UnionDBRec	*udbr = udb->udbr;
		RingSet	*rset;
		RingSet	*rs1;


		if (udb->rset)
				return udb->rset;

		/*	Must construct a new set */
		rset = NULL;
		if (udb->rset)
				ringSetDestroy (udb->rset);
		udbr = udb->udbr;
		while (udbr) {
				rs1 = (udbr->keyDB)->getRingSet (udbr->keyDB);
				pgpAssert (rs1);
				if (!rset) {
						rset = ringSetCreate (ringSetPool (rs1));
				}
				ringSetAddSet (rset, rs1);
				udbr = udbr->next;
		}
		ringSetFreeze (rset);
		udb->rset = rset;
		return rset;
}

/*
 * Big problem here - it's hard to tell whether an important
 * key addition failed. Could have a writeable pubring but due
 * to some error a read-only secring. One of our add attempts
 * may return an error due to immutability, but others will succeed.
 * How to distinguish this from the case of one writeable and one
 * read-only pubring, which is OK but returns similar status.
 */

static PGPError
udbAdd (PGPKeyDB *kdb, RingSet *toAdd)
{
		UnionDB	*udb = (UnionDB *)kdb->private;
		UnionDBRec	*udbr = udb->udbr;
		PGPError	err, err1;

		err = err1 = 0;
		while (udbr) {
				err1 = (udbr->keyDB)->add (udbr->keyDB, toAdd);
				if (err1)
						err = err1;
				udbr = udbr->next;
		}
		udbDirty (udb);
		return err;
}

static PGPError
udbRemove (PGPKeyDB *kdb, RingObject *toRemove)
	{
		UnionDB	*udb = (UnionDB *)kdb->private;
		UnionDBRec	*udbr = udb->udbr;
		PGPError	err, err1;

		err = err1 = 0;
		while (udbr) {
				err1 = (udbr->keyDB)->remove (udbr->keyDB, toRemove);
				if (err1)
						err = err1;
				udbr = udbr->next;
		}
		udbDirty (udb);
		return err;
}

static PGPError
udbChanged (PGPKeyDB *kdb, RingSet *changedkeys)
{
		UnionDB	*udb = (UnionDB *)kdb->private;
		UnionDBRec	*udbr = udb->udbr;
		PGPError	err, err1;
	
		err = err1 = 0;
		while (udbr) {
				err1 = (udbr->keyDB)->changed (udbr->keyDB, changedkeys);
				if (err1)
						err = err1;
				udbr = udbr->next;
		}
		udbDirty (udb);
		err1 = pgpReSortKeys (kdb, changedkeys);
		if (err1)
				err = err1;
		return err;
}

static PGPError
udbCommit (PGPKeyDB *kdb)
{
		UnionDB	*udb = (UnionDB *)kdb->private;
		UnionDBRec	*udbr = udb->udbr;
		PGPError	err, err1;

		err = 0;
		while (udbr) {
				err1 = (udbr->keyDB)->commit (udbr->keyDB);
				if (err1)
						err = err1;
				udbr = udbr->next;
		}
		udb->bdirty = 0;
		return err;
}

static PGPError
udbRevert (PGPKeyDB *kdb)
{
		UnionDB	*udb = (UnionDB *)kdb->private;
		UnionDBRec	*udbr = udb->udbr;
		PGPError	err, err1;

		err = 0;
		while (udbr) {
				err1 = (udbr->keyDB)->revert (udbr->keyDB);
				if (err1)
						err = err1;
				udbr = udbr->next;
		}
		udb->bdirty = 0;
		if (udb->rset) {
				ringSetDestroy (udb->rset);
				udb->rset = 0;
		}
		return err;
}

static PGPError
udbReload (PGPKeyDB *kdb)
{
		UnionDB	*udb = (UnionDB *)kdb->private;
		UnionDBRec	*udbr = udb->udbr;
		PGPError	err, err1;

		err = 0;
		while (udbr) {
				err1 = (udbr->keyDB)->reload (udbr->keyDB);
				if (err1)
						err = err1;
				udbr = udbr->next;
		}
		udb->bdirty = 0;
		if (udb->rset) {
				ringSetDestroy (udb->rset);
				udb->rset = 0;
		}
		return err;
}

static void
udbDestroy (PGPKeyDB *kdb)
{
		UnionDB *udb = (UnionDB *)kdb->private;
		UnionDBRec *udbr = udb->udbr;
		UnionDBRec *udbrnext;

		if (udb->rset) {
				ringSetDestroy (udb->rset);
				udb->rset = 0;
		}
		while (udbr) {
				pgpFreeKeyDB (udbr->keyDB);
				udbrnext = udbr->next;
				pgpMemFree (udbr);
				udbr = udbrnext;
		}
		pgpMemFree (udb);
}


/**************************** Constructor **************************/


PGPKeyDB *
pgpCreateUnionKeyDB (PGPError *error)
{
		PGPKeyDB *kdb;
		UnionDB *udb;

		*error = 0;
		kdb = pgpKeyDBCreateInternal ();
		if (!kdb) {
				*error = PGPERR_NOMEM;
				return NULL;
		}
		udb = (UnionDB *) pgpMemAlloc (sizeof (UnionDB));
		if (!udb) {
				*error = PGPERR_NOMEM;
				pgpKeyDBDestroyInternal (kdb);
		}
		memset (udb, 0, sizeof (*udb));

		kdb->private 		= udb;
		kdb->magic						= PGPKDBUNIONMAGIC;
		kdb->isMutable 		= udbIsMutable;
		kdb->objIsMutable		= udbObjIsMutable;
		kdb->isDirty 		= udbIsDirty;
		kdb->getRingSet			= udbGetRingSet;
		kdb->add 		= udbAdd;
		kdb->remove 		= udbRemove;
		kdb->changed				= udbChanged;
		kdb->commit 		= udbCommit;
		kdb->revert 		= udbRevert;
		kdb->reload 		= udbReload;
		kdb->destroy				= udbDestroy;

		pgpKeyDBInitInternal(kdb);

		return kdb;
}

PGPError
pgpUnionKeyDBAdd (PGPKeyDB *kdb, PGPKeyDB *kdbnew)
{
		UnionDBRec *udbr;
		UnionDB *udb = (UnionDB *)kdb->private;

		udbr = (UnionDBRec *) pgpMemAlloc (sizeof (UnionDBRec));
		if (!udbr) {
				return PGPERR_NOMEM;
		}

		memset (udbr, 0, sizeof (*udbr));
		udbr->keyDB = kdbnew;
		udbr->next = udb->udbr;
		udb->udbr = udbr;

		return PGPERR_OK;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
