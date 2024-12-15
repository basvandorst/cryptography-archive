/*
 * Memory-based implementation of PGPKeyDB
 * This version allows committing changes and thence uses backing store.
 *
 * Copyright (C) 1996,1997 Network Associates Inc. and affiliated companies.
 * All rights reserved
 *
 * $Id: pgpMemDB.c,v 1.18 1999/03/10 02:52:58 heller Exp $
 */

#include "pgpConfig.h"

#include "pgpContext.h"
#include "pgpKDBInt.h"
#include "pgpDebug.h"
#include "pgpTypes.h"
#include "pgpMem.h"
#include "pgpUsuals.h"
#include "pgpEnv.h"
#include "pgpMemPool.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"

/*
 * Until the first commit, only rsmut and rsfrozen (and bdirty) are used.
 * Only after that is there backing store.
 */
typedef struct MemDB
{
	PGPContextRef	context;
	RingFile       *rfile;			/* RingFile for backing store */
	PGPFile        *pfile;			/* PGPFile for backing store */
	RingSet        *rsmut;			/* Mutable ringset */
	RingSet const  *rsimmut;		/* Immutable ringset (from rfile) */
	RingSet        *rsfrozen;		/* Frozen copy of rsmut */
	PGPBoolean      bdirty;			/* True if rsmut has been changed */
	DEBUG_STRUCT_CONSTRUCTOR( MemDB )
} MemDB;


/* Close the open memdb object */
static PGPError
doClose (MemDB *mdb)
{
	PGPContextRef	context;

	pgpAssertAddrValid( mdb, MemDB );
	context	= mdb->context;
	
	if( IsntNull( mdb->rsfrozen ) ) {
		ringSetDestroy( mdb->rsfrozen );
		mdb->rsfrozen = NULL;
	}
	if( IsntNull( mdb->rsmut ) ) {
		ringSetDestroy (mdb->rsmut);
		mdb->rsmut = NULL;
	}
	if( IsntNull( mdb->rfile ) ) {
		ringFileClose (mdb->rfile);
		mdb->rfile = NULL;
	}
	/* rsimmut is tied to the rfile */
	mdb->rsimmut = NULL;
	if( IsntNull( mdb->pfile ) ) {
		pgpFileClose (mdb->pfile);
		mdb->pfile = NULL;
	}
	return kPGPError_NoErr;
}

/* Mark as dirty */
static void
doMarkDirty (MemDB *mdb)
{
	mdb->bdirty = TRUE;
	if( IsntNull( mdb->rsfrozen ) ) {
		ringSetDestroy( mdb->rsfrozen );
		mdb->rsfrozen = NULL;
	}
}

static PGPBoolean
memDBIsMutable(PGPKeyDB *db)
{
	(void)db;	/* Avoid warning */
	return TRUE;
}

static PGPBoolean
memDBObjIsMutable(PGPKeyDB *db, RingObject *testObj)
{
	(void)db;		/* Avoid warning */
	(void)testObj;	/* Avoid warning */
	return TRUE;
}

static PGPBoolean
memDBIsDirty(PGPKeyDB *db)
{
	MemDB *		mdb;

	pgpa(pgpaPGPKeyDBValid(db));
	mdb = (MemDB *)db->priv;
	return mdb->bdirty;
}

static RingSet const *
memDBGetRingSet(PGPKeyDB *db)
{
	MemDB *		mdb;
	RingSet     *newset;

	pgpa(pgpaPGPKeyDBValid(db));
	mdb = (MemDB *)db->priv;
	if( IsNull( mdb->rsfrozen ) ) {
	    newset = ringSetCreate (ringSetPool (mdb->rsmut));
		pgpAssert (newset);
		ringSetAddSet (newset, mdb->rsmut);
		ringSetFreeze (newset);
		mdb->rsfrozen = newset;
	}
	return mdb->rsfrozen;
}

static PGPError
memDBAdd(PGPKeyDB *db, RingSet *toAdd)
{
	MemDB *		mdb;

	pgpa(pgpaPGPKeyDBValid(db));
	mdb = (MemDB *)db->priv;
	doMarkDirty( mdb );
	return (PGPError)ringSetAddSet(mdb->rsmut, toAdd);
}

static PGPError
memDBChanged(PGPKeyDB *db, RingSet *changedkeys)
{
	MemDB *		mdb;

	pgpa(pgpaPGPKeyDBValid(db));
	mdb = (MemDB *)db->priv;
	doMarkDirty( mdb );
	return pgpReSortKeys (db, changedkeys);
}

static PGPError
memDBRemove(PGPKeyDB *db, RingObject *toRemove)
{
	MemDB *		mdb;

	pgpa(pgpaPGPKeyDBValid(db));
	mdb = (MemDB *)db->priv;
	doMarkDirty( mdb );
	return (PGPError)ringSetRemObject(mdb->rsmut, toRemove);
}

static void
memDBDestroy(PGPKeyDB *db)
{
	MemDB *			mdb;
	PGPContextRef	cdkContext	= pgpGetKeyDBContext( db );

	/* Can't assert pgpaPGPKeyDBValid because refCount == 0 now */
	pgpa((pgpaAddrValid(db, PGPKeyDB),pgpaAssert(db->refCount==0)));

	mdb = (MemDB *)db->priv;
	doClose( mdb );
	pgpContextMemFree( cdkContext, mdb);
}

/*
 * To do a commit, we write out rsmut to a new PGPFile memory buffer.
 * We then close any old backing store, and switch to the new one.
 */
static PGPError
memDBCommit (PGPKeyDB *db)
{
	MemDB *		mdb;
	PGPFile *	pfile;
	RingFile *	rfile;
	RingSet const *rsimmut;
	RingSet *	rsmut;
	PGPError	err;

	pgpa(pgpaPGPKeyDBValid(db));
	mdb = (MemDB *)db->priv;

	pfile = pgpFileMemOpen( mdb->context, NULL, 0 );
	if( IsNull( pfile ) ) {
		return kPGPError_OutOfMemory;
	}
	/* Make sure rsfrozen exists, and write it out, creating new rfile */
	memDBGetRingSet( db );
	err = ringSetWrite( mdb->rsfrozen, pfile, &rfile, PGPVERSION_4,
						PGP_RINGSETWRITE_EXPORTSIGS );
	if( IsPGPError( err ) ) {
		pgpFileClose( pfile );
		return err;
	}
	pgpFileFlush( pfile );
	rsimmut = ringFileSet( rfile );
	rsmut = ringSetCreate( ringSetPool( rsimmut ) );
	if( IsNull( rsmut ) ) {
		err = ringSetError( rsimmut )->error;
		ringFileClose( rfile );
		pgpFileClose( pfile );
		return err;
	}
	ringSetAddSet( rsmut, rsimmut );

	/* Free up previous state and set new values */
	doClose( mdb );
	mdb->rfile = rfile;
	mdb->pfile = pfile;
	mdb->rsimmut = rsimmut;
	mdb->rsmut = rsmut;
	return err;
}	

static PGPError
memDBRevert (PGPKeyDB *db)
{
	MemDB *			mdb;

	pgpa(pgpaPGPKeyDBValid(db));
	mdb = (MemDB *)db->priv;

	if( !mdb->bdirty )
		return kPGPError_NoErr;

	if( IsNull( mdb->rsimmut ) ) {
		/* Can't revert unless we've done a commit */
		return kPGPError_IllegalFileOp;
	}
	
	ringSetDestroy (mdb->rsmut);
	mdb->rsmut = ringSetCreate( ringSetPool( mdb->rsimmut ) );
	if( IsNull( mdb->rsmut ) )
	{
		return ringSetError( mdb->rsimmut )->error;
	}
	ringSetAddSet( mdb->rsmut, mdb->rsimmut );

	if( mdb->rsfrozen )
	{
		ringSetDestroy (mdb->rsfrozen);
		mdb->rsfrozen = NULL;
	}

	mdb->bdirty = FALSE;
	return kPGPError_NoErr;
}


/* Creates a new empty memory-based key database */
PGPKeyDB *
pgpKeyDBCreate(PGPContextRef context)
{
	PGPKeyDB *	db;
	MemDB *		mdb;
	RingPool *	pgpRingPool;

	db = pgpKeyDBCreateInternal(context);
	if (db == NULL)
		return NULL;

	mdb = (MemDB *)pgpContextMemAlloc( context,
		sizeof( MemDB ), kPGPMemoryMgrFlags_Clear);
	if (mdb == NULL)
	{
		pgpContextMemFree( context, db);
		return NULL;
	}
	mdb->context = context;
	pgpRingPool = pgpContextGetRingPool( context );
	mdb->rsmut = (RingSet *)ringSetCreate( pgpRingPool );

	db->priv = mdb;
	db->typeMagic		= PGPKDBMEMMAGIC;
	db->fixedMagic		= kPGPKeyDBMagic;
	db->isMutable		= memDBIsMutable;
	db->objIsMutable	= memDBObjIsMutable;
	db->isDirty			= memDBIsDirty;
	db->getRingSet		= memDBGetRingSet;
	db->add				= memDBAdd;
	db->changed			= memDBChanged;
	db->remove			= memDBRemove;
	db->commit			= memDBCommit;
	db->revert			= memDBRevert;
	db->reload			= memDBRevert;	/* same as revert for mem buffers */
	db->destroy			= memDBDestroy;

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
