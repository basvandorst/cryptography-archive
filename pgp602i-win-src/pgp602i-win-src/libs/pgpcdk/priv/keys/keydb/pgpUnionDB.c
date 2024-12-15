/*
 * pgpUnionDB.c
 * Manage PGPKeyDB's which are unions of others
 *
 * Copyright (C) 1996,1997 Network Associates, Inc. and its affiliates.
 * All rights reserved
 *
 * $Id: pgpUnionDB.c,v 1.21.10.1 1998/11/12 03:22:05 heller Exp $
 */

#include "pgpConfig.h"

#include "pgpKDBInt.h"
#include "pgpDebug.h"
#include "pgpContext.h"

/* One list element per unioned PGPKeyDB */
typedef struct UnionDBRec {
	struct UnionDBRec  	*next;
	PGPKeyDB			*keyDB;
} UnionDBRec;


/* Private data for union type PGPKeyDB */
typedef struct UnionDB
{
	PGPBoolean     bdirty;
	RingSet        *rset;
	UnionDBRec  	 *udbr;
	DEBUG_STRUCT_CONSTRUCTOR( UnionDB )
} UnionDB;




/********************  Virtual Functions  ************************/


/* Returns true if any component is mutable */
static PGPBoolean
udbIsMutable (PGPKeyDB *kdb)
{
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;

	while (udbr) {
		if (((udbr->keyDB)->isMutable)(udbr->keyDB)) {
			return 1;
		}
		udbr = udbr->next;
	}
	return 0;
}

/* True if object is mutable in any component */
static PGPBoolean
udbObjIsMutable (PGPKeyDB *kdb, RingObject *testObj)
{	
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;

	while (udbr) {
		if ((udbr->keyDB)->objIsMutable (udbr->keyDB, testObj))
			return 1;
		udbr = udbr->next;
	}
	return 0;
}

/* Returns true if any component is dirty */
static PGPBoolean
udbIsDirty (PGPKeyDB *kdb) {
	UnionDB         *udb = (UnionDB *)kdb->priv;

	return udb->bdirty;
}

/* Mark it dirty */
static void
udbDirty (UnionDB *udb)
{
	udb->bdirty = 1;
	if (udb->rset) {
		ringSetDestroy (udb->rset);
		udb->rset = NULL;
	}
}

/* Return a union of all keys in sets */
static RingSet const *
udbGetRingSet (PGPKeyDB *kdb) {
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;
	RingSet         *rset;
	RingSet const         *rs1;


	if (udb->rset)
		return udb->rset;

	/* Must construct a new set */
	rset = NULL;
	if (udb->rset)
		ringSetDestroy (udb->rset);
	udbr = udb->udbr;
	while (udbr) {
		rs1 = (udbr->keyDB)->getRingSet (udbr->keyDB);
		pgpAssert (rs1);
		if (!rset) {
			rset = ringSetCreate (ringSetPool (rs1));
			if (!rset) {
				/* Out of ringsets error */
				return NULL;
			}
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
 * key addition failed.  Could have a writeable pubring but due
 * to some error a read-only secring.  One of our add attempts
 * may return an error due to immutability, but others will succeed.
 * How to distinguish this from the case of one writeable and one
 * read-only pubring, which is OK but returns similar status.
 */

static PGPError
udbAdd (PGPKeyDB *kdb, RingSet *toAdd)
{
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;
	PGPError         err, err1;

	err = err1 = kPGPError_NoErr;
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
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;
	PGPError         err, err1;            

	err = err1 = kPGPError_NoErr;
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
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;
	PGPError         err, err1;
	
	err = err1 = kPGPError_NoErr;
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
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;
	PGPError         err, err1;            

	err = kPGPError_NoErr;
	while (udbr) {
		err1 = (udbr->keyDB)->commit (udbr->keyDB);
		if (err1)
			err = err1;
		udbr = udbr->next;
	}
	udb->bdirty = 0;
	/* Committing can eliminate redundant secret objects. */
	/* We need to re-create our rset afterwards */
	if (udb->rset) {
		ringSetDestroy (udb->rset);
		udb->rset = 0;
	}
	return err;
}

static PGPError
udbRevert (PGPKeyDB *kdb)
{
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;
	PGPError         err, err1;            

	err = kPGPError_NoErr;
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
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;
	PGPError         err, err1;            

	err = kPGPError_NoErr;
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
	UnionDB         *udb = (UnionDB *)kdb->priv;
	UnionDBRec      *udbr = udb->udbr;
	UnionDBRec      *udbrnext;
	PGPContextRef	context	= pgpGetKeyDBContext( kdb );

	if (udb->rset)
	{
		ringSetDestroy (udb->rset);
		udb->rset = 0;
	}
	while (udbr)
	{
		pgpFreeKeyDB (udbr->keyDB);
		udbrnext = udbr->next;
		pgpContextMemFree( context, udbr);
		udbr = udbrnext;
	}
	pgpContextMemFree( context, udb);
}


/****************************  Constructor  **************************/


PGPKeyDB *
pgpCreateUnionKeyDB (PGPContextRef context, PGPError *error)
{
	PGPKeyDB       *kdb;
	UnionDB        *udb;

	*error = kPGPError_NoErr;
	kdb = pgpKeyDBCreateInternal (context);
	if (!kdb) {
		*error = kPGPError_OutOfMemory;
		return NULL;
	}
	udb = (UnionDB *) pgpContextMemAlloc( context,
		sizeof (UnionDB), kPGPMemoryMgrFlags_Clear );
	if (!udb) {
		*error = kPGPError_OutOfMemory;
		pgpKeyDBDestroyInternal (kdb);
		return( NULL );
	}

	kdb->priv    	= udb;
	kdb->typeMagic		= PGPKDBUNIONMAGIC;
	kdb->fixedMagic		= kPGPKeyDBMagic;
	kdb->isMutable  	= udbIsMutable;
	kdb->objIsMutable	= udbObjIsMutable;
	kdb->isDirty    	= udbIsDirty;
	kdb->getRingSet		= udbGetRingSet;
	kdb->add        	= udbAdd;
	kdb->remove     	= udbRemove;
	kdb->changed		= udbChanged;
	kdb->commit     	= udbCommit;
	kdb->revert     	= udbRevert;
	kdb->reload     	= udbReload;
	kdb->destroy		= udbDestroy;

	pgpKeyDBInitInternal(kdb);

	return kdb;
}

PGPError
pgpUnionKeyDBAdd (PGPKeyDB *kdb, PGPKeyDB *kdbnew)
{
	UnionDBRec     *udbr;
	UnionDB        *udb = (UnionDB *)kdb->priv;
	PGPContextRef	context	= pgpGetKeyDBContext( kdb );

	udbr = (UnionDBRec *) pgpContextMemAlloc(
		context, sizeof (UnionDBRec), kPGPMemoryMgrFlags_Clear);
	if (!udbr)
	{
		return kPGPError_OutOfMemory;
	}

	udbr->keyDB = kdbnew;
	udbr->next = udb->udbr;
	udb->udbr = udbr;

	return kPGPError_NoErr;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */