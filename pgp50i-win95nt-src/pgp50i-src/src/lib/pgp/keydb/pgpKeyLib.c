/*
 * pgpKeyLib.c -- Initialization and cleanup functions related to the
 * keydb library
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpKeyLib.c,v 1.41.2.8 1997/06/13 04:15:24 hal Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if MACINTOSH
#include "MacStrings.h"
#include "MacFiles.h"
#endif

#include "pgpKeyDB.h"
#include "pgpDebug.h"
#include "pgpKDBint.h"
#include "pgpEnv.h"
#include "pgpFileNames.h"
#include "pgpFileRef.h"
#include "pgpRndPool.h"
#include "pgpRndSeed.h"

#if defined(_WIN32) && !defined(NODLL)

/* In Windows, the DllMain function is called when a process or
thread attaches to, or detaches from, the DLL. Other platforms
should call pgpLibInit() and pgpLibCleanup () directly. */

#include "windows.h"

BOOL WINAPI DllMain (HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) {
#if !DEBUG
/* Malloc/free fails in debug mode because app hasn't set up leak context */
	case DLL_PROCESS_ATTACH:
		return (pgpLibInit () == PGPERR_OK);
#endif
	case DLL_PROCESS_DETACH:
		if (pgpLibSetup > 1)
			pgpLibSetup = 1;		/* force final cleanup */
		pgpLibCleanup ();
		return 1;
	}
	return 1;
}
#endif


/* Global variables for PGP library */
RingPool *			pgpRingPool;	/* RingPool used for all keysets */
PgpEnv *			pgpEnv;			/* User specified configuration values */
Boolean				pgpPrefsChanged = FALSE;
Boolean				pgpRngSeeded;	 /* Found a good randseed.bin */
int					pgpLibSetup;	/* # calls to pgpLibInit - pgpLibCleanup */
PgpRandomContext *	pgpRng;			/* Random number generator */

#if MACINTOSH	/* [ */

	static PGPFileRef *
RandSeedFileRef(void)
{
	FSSpec			fsSpec;
	
	if (FindPGPPreferencesFolder(kOnSystemDisk, &fsSpec.vRefNum,
								&fsSpec.parID) != noErr)
		return NULL;
	
	CopyPString("\pPGP Random Seed", fsSpec.name);
	return pgpNewFileRefFromFSSpec(&fsSpec);
}

#else	/* ] MACINTOSH [ */

	static PGPFileRef *
RandSeedFileRef(void)
{
	char const *	fileName;
	
	fileName = pgpenvGetString(pgpEnv, PGPENV_RANDSEED, NULL, NULL);
	if (fileName == NULL)
		return NULL;
	
	return pgpNewFileRefFromFullPath(fileName);
}

#endif	/* ] MACINTOSH */

/*
 * Initialize the PGP KeyDB library. This should be called for any
 * function which the user might call first, ones which don't rely on
 * the previous existence of KeySets and such.
 *
 * This sets up a global PgpEnv and RingPool, and initializes the fields
 * of the PgpEnv from the preference database.
 */
PGPError
pgpLibInit(void)
{
	char const *randseed;	 	/* Name of randseed file */
	PGPFileRef *randSeedRef;	/* File reference to randseed file */
	FILE	*rs;			 	/* Handle to randseed file */
	PGPError	result;

	if (pgpLibSetup) {
		pgpLibSetup += 1;		 /* Allow nesting */
		/* Reload prefs on every init call unless we've changed the prefs */
		return pgpPrefsChanged ? PGPERR_OK : pgpLoadPrefs();
	}

	pgpEnv = pgpenvCreate();
	if (!pgpEnv)
		return PGPERR_NOMEM;
	pgpAssertAddrValid(pgpEnv, VoidAlign);

	/* Set pgpEnv defaults */

	/* Default verbosity level */
	pgpenvSetInt(pgpEnv, PGPENV_VERBOSE, 1, PGPENV_PRI_PRIVDEFAULT);

	/* Read info from preference file */
	result = pgpLoadPrefs();
	if (result != PGPERR_OK)
		return result;

	/* Figure out where randseed file is */
	randseed = pgpenvGetString (pgpEnv, PGPENV_RANDSEED, NULL, NULL);
	if (!randseed) {
		char *pubdir, *rseed;
		char const *pubring;
		pubring = pgpenvGetString (pgpEnv, PGPENV_PUBRING, NULL, NULL);
		pgpAssert (pubring && *pubring);
		pubdir = pgpFileNameDirectory (pubring);
		rseed = pgpFileNameBuild (pubdir, "randseed.bin", NULL);
		pgpenvSetString (pgpEnv, PGPENV_RANDSEED, rseed,
						PGPENV_PRI_PRIVDEFAULT);
		pgpFree (rseed);
		pgpFree (pubdir);
	}

	pgpRng = pgpRandomCreate ();
	if (!pgpRng) {
	pgpenvDestroy (pgpEnv);
		return PGPERR_NOMEM;
	}

	/* Read in the randseed file, set pgpRngSeeded if succeed */
	randSeedRef = RandSeedFileRef();
	rs = pgpFileRefStdIOOpen(randSeedRef, kPGPFileOpenReadWritePerm,
							kPGPFileTypeNone, NULL);
	pgpFreeFileRef(randSeedRef);
	if (rs) {
		result = pgpRandSeedRead (rs, NULL);
		pgpStdIOClose (rs);
	} else {
		result = PGPERR_RANDSEED_TOOSMALL;
	}
	pgpRngSeeded = (result == PGPERR_OK);

	pgpRingPool = ringPoolCreate(pgpEnv);
	if (!pgpRingPool) {
		pgpenvDestroy (pgpEnv);
		pgpRandomDestroy (pgpRng);
		return PGPERR_NOMEM;
	}
	pgpAssertAddrValid(pgpRingPool, VoidAlign);
	pgpLibSetup = 1;
	return PGPERR_OK;
}

/* This should be called by the application when it is through with the
 * Aurora library. It should also be called by the operating system if
 * possible when the application exits.
 * Calls to pgpLibInit and pgpLibCleanup can be paired. pgpLibCleanup will
 * output any changed prefs and update the random seed file any time it
 * is called, but only release resources on the last call.
 */
void
pgpLibCleanup(void)
{
	PGPFileRef *randSeedRef;	/* File reference to randseed file */
	FILE	*rs;			 	/* Handle to randseed file */
	
	/* If not open, just return */ 	
if (!pgpLibSetup)
		return;
	
	/* Decrement open count, if goes to zero close things */
	pgpLibSetup -= 1;
	
	pgpSavePrefs();
	
	if (pgpLibSetup)
		return;

	/* Update random data */
	pgpRandPoolKeystroke (0);	 /* Get current time into rand pool */

	/*
	* Don't write out randseed.bin if we didn't read one and haven't
	* accumulated significant entropy. Otherwise we might produce
	* weak session keys on future runs.
	*/
	if (pgpRngSeeded
		|| pgpRandPoolInflow() >= PGP_SEED_MIN_BYTES*8) {
		randSeedRef = RandSeedFileRef();
		rs = pgpFileRefStdIOOpen(randSeedRef, kPGPFileOpenStdWriteFlags,
								kPGPFileTypeRandomSeed, NULL);
		pgpFreeFileRef(randSeedRef);
		if (rs) {
			pgpRandSeedWrite (rs, NULL, NULL);
			pgpStdIOClose (rs);
		}
	}

	ringPoolDestroy (pgpRingPool);
	pgpenvDestroy (pgpEnv);
	pgpRandomDestroy (pgpRng);
	pgpRingPool = NULL;
	pgpEnv = NULL;
	pgpRng = NULL;
	pgpRngSeeded = FALSE;
	pgpLibSetup = FALSE;
}

PGPError
pgpGetKeyRingFileRefs(PGPFileRef **pubRef, PGPFileRef **privRef)
{
	PGPError		result = PGPERR_OK;

	if (privRef != NULL)
		*privRef = NULL;
	if (pubRef != NULL)
		*pubRef = NULL;

	if (privRef != NULL)
	{
		*privRef = pgpGetPrefFileRef(kPGPPrefPrivRingRef);
		if (*privRef == NULL)
			goto memError;
	}

	if (pubRef != NULL)
	{
		*pubRef = pgpGetPrefFileRef(kPGPPrefPubRingRef);
		if (*pubRef == NULL)
			goto memError;
	}
	return PGPERR_OK;
memError:
	result = PGPERR_NOMEM;
	if (privRef != NULL && *privRef != NULL)
	{
		pgpFreeFileRef(*privRef);
		*privRef = NULL;
	}
	if (pubRef != NULL && *pubRef != NULL)
	{
		pgpFreeFileRef(*pubRef);
		*pubRef = NULL;
	}
	return result;
}

/*
 * Open default keyrings for user, return keyset for it.
 * If isMutable is false, keyrings are read only.
 * If <errorPtr> is non-NULL, it is filled with an appropriate error code.
 */
PGPKeySet *
pgpOpenDefaultKeyRings(Boolean isMutable, PGPError *errorPtr)
{
	PGPFileRef *secFileRef = NULL;	/* File reference for secret keyring */
	PGPFileRef *pubFileRef = NULL;	/* File reference for public keyring */
	PGPKeySet	 *set = NULL;
	PGPError		errorVal = PGPERR_OK;

	pgpAssertMsg(pgpLibSetup > 0,
			"pgpOpenDefaultKeyRings: pgpLibInit hasn't been called");

	if (pgpGetKeyRingFileRefs(&pubFileRef, &secFileRef) != PGPERR_OK)
	{
		errorVal = PGPERR_NO_FILE;
		goto error;
	}
	set = pgpOpenKeyRingPair(isMutable, pubFileRef, secFileRef, &errorVal);
	
error:
	if (secFileRef != NULL)
		pgpFreeFileRef(secFileRef);
	if (pubFileRef != NULL)
		pgpFreeFileRef(pubFileRef);
	if (set != NULL && errorVal != PGPERR_OK)
	{
		pgpFreeKeySet(set);
		set = NULL;
	}
	if (errorPtr != NULL)
	{
		pgpAssertAddrValid(errorPtr, PGPError);
		*errorPtr = errorVal;
	}
	return set;
}

/*
 * Open the specified keyrings for user, return keyset for it.
 * If isMutable is false, keyrings are read only.
 * If <errorPtr> is non-NULL, it is filled with an appropriate error code.
 */
PGPKeySet *
pgpOpenKeyRingPair(Boolean isMutable, PGPFileRef *pubFileRef,
					PGPFileRef *secFileRef, PGPError *errorPtr)
{
	PGPKeyDB		*dbsec = NULL,		/* KeyDB for secret keyring */
					*dbpub = NULL,		/* KeyDB for public keyring */
					*dbunion = NULL;		/* KeyDB for union of both keyrings */
	PGPKeySet		*set = NULL;
	PGPError		errorVal = PGPERR_OK;

	pgpAssertMsg(pgpLibSetup > 0,
			"pgpOpenKeyRingPair: pgpLibInit hasn't been called");

	/* Create key databases for these files. Don't bother with keypool.
	Private keyring is not trusted (no trust packets) */
	if ((dbsec = pgpCreateFileKeyDB(secFileRef, 0, 1, isMutable,
								pgpRingPool, &errorVal)) == NULL)
		goto error;
	if ((dbpub = pgpCreateFileKeyDB(pubFileRef, 1, 0, isMutable,
								pgpRingPool, &errorVal)) == NULL)
		goto error;

	/* Create union database for these two files */
	if ((dbunion = pgpCreateUnionKeyDB(&errorVal)) == NULL)
		goto error;
	
	if ((errorVal = pgpUnionKeyDBAdd(dbunion, dbsec)) != PGPERR_OK)
		goto error;
	dbsec = NULL;	 /* dbunion now has responsibility for freeing dbsec */
	
	if ((errorVal = pgpUnionKeyDBAdd(dbunion, dbpub)) != PGPERR_OK)
		goto error;
	dbpub = NULL;	 /* dbunion now has responsibility for freeing dbsec */

	if ((errorVal = pgpBuildKeyPool(dbunion, 0)) != PGPERR_OK)
		goto error;

	if ((set = pgpKeyDBRootSet(dbunion)) == NULL)
	{
		errorVal = PGPERR_NOMEM;	/* XXX Improve error */
		goto error;
	}

	/* Make sure trust and validity info is correct. This also
	ensures the keyrings are rewritten if ringFileOpen set
		the RINGFILEF_TRUSTCHANGED or RINGFILEF_DIRTY flags. */
	if ((errorVal = pgpCommitKeyRingChanges(set)) != PGPERR_OK)
		goto error;
	errorVal = PGPERR_OK;
	
error:
	if (dbsec != NULL)
		pgpFreeKeyDB(dbsec);
	if (dbpub != NULL)
		pgpFreeKeyDB(dbpub);
	if (dbunion != NULL)
		pgpFreeKeyDB(dbunion);
	if (set != NULL && errorVal != PGPERR_OK)
	{
		pgpFreeKeySet(set);
		set = NULL;
	}
	if (errorPtr != NULL)
	{
		pgpAssertAddrValid(errorPtr, PGPError);
		*errorPtr = errorVal;
	}
	return set;
}

/*
 * Open a single specified keyring file for user, return keyset for it.
 * If isMutable is false, keyrings are read only.
 * If isTrusted is false, trust packets are ignored.
 * If <errorPtr> is non-NULL, it is filled with an appropriate error code.
 */
PGPKeySet *
pgpOpenKeyRing(Boolean isMutable, Boolean isPrivate, Boolean isTrusted,
				PGPFileRef *fileRef, PGPError *errorPtr)
{
	PGPKeyDB	*db = NULL;
	PGPKeySet	 *set = NULL;
	PGPError		errorVal = PGPERR_OK;

	pgpAssertMsg(pgpLibSetup > 0,
			"pgpOpenKeyRing: pgpLibInit hasn't been called");

	/* Create a key databases for this file. Don't bother with keypool.
	Private keyring is not trusted (no trust packets) */
	if ((db = pgpCreateFileKeyDB(fileRef, isTrusted, isPrivate, isMutable,
								pgpRingPool, &errorVal)) == NULL)
		goto error;

	if ((errorVal = pgpBuildKeyPool(db, 0)) != PGPERR_OK)
		goto error;

	if ((set = pgpKeyDBRootSet(db)) == NULL)
	{
		errorVal = PGPERR_NOMEM;	/* XXX Improve error */
		goto error;
	}

	/* Make sure trust and validity info is correct. This also
	ensures the keyrings are rewritten if ringFileOpen set
		the RINGFILEF_TRUSTCHANGED or RINGFILEF_DIRTY flags. */
	if ((errorVal = pgpCommitKeyRingChanges(set)) != PGPERR_OK)
		goto error;
	errorVal = PGPERR_OK;
	
error:
	if (db != NULL)
		pgpFreeKeyDB(db);
	if (set != NULL && errorVal != PGPERR_OK)
	{
		pgpFreeKeySet(set);
		set = NULL;
	}
	if (errorPtr != NULL)
	{
		pgpAssertAddrValid(errorPtr, PGPError);
		*errorPtr = errorVal;
	}
	return set;
}

/* Add keys to a keyset from a dynamically allocated binary key buffer */
static PGPKeySet *
pgpImportKeyX (byte *buffer, size_t length)
{
	PGPKeyDB *kdb;
	PGPKeySet *set;
	PGPError	error;

	/* Create a file type KeyDB from the buffer */
	kdb = pgpCreateMemFileKeyDB (buffer, length, pgpRingPool, &error);
	if (!kdb) {
		pgpMemFree (buffer);
		return NULL;
	}
	error = pgpBuildKeyPool (kdb, 0);
	if (error) {
		pgpFreeKeyDB (kdb);
		return NULL;
	}

	set = pgpKeyDBRootSet (kdb);
	pgpFreeKeyDB (kdb);
	return set;
}


/* Open the specified keyfile and add to the existing KeySet */
PGPKeySet *
pgpImportKeyFile (PGPFileRef *fileRef)
{
	byte	*filebuf;
	size_t		filebuflen;
	PGPError	err;

	err = pgpDearmorKeyFile (pgpEnv, fileRef, &filebuf, &filebuflen);
	if (err)
		return NULL;
	return pgpImportKeyX (filebuf, filebuflen);
}

/* Add the data from the specified memory buffer to the existing KeySet */
PGPKeySet *
pgpImportKeyBuffer (byte *buffer, size_t length)
{
	byte	*filebuf;
	size_t		filebuflen;
	PGPError	err;

	err = pgpDearmorKeyBuffer (pgpEnv, buffer, length, &filebuf, &filebuflen);
	if (err)
		return NULL;
	return pgpImportKeyX (filebuf, filebuflen);
}


/*
* Filter function for extraction. Remove any secret objects.
* If addmrks is true, also add any message recovery key objects to
* the set. This will cause MRK's to be extracted with the keys
* that use them.
* XXX THIS DOES NOT YET WORK. There is no RingSet available in which
* to look for MRK's. The export functions typically are called with
* just a memory RingSet. We need to add versions which take an extra
* PGPKeySet to flag that MRK's should be looked for there.
*/
static RingSet *
filterPubRingSet (RingSet *rset, PGPKeySet *keys, Boolean addmrks)
{
	RingSet		*rsetnew;	/* Set of recipients */
	RingSet		*rkeyset;	/* Set of message recovery keys */
	RingIterator *riter;		/* Iterator over adding sets */
	int				level;

	if (!rset)
		return NULL;
	rkeyset = NULL;
	rsetnew = ringSetCreate (ringSetPool (rset));
	if (!rsetnew)
		return NULL;
	riter = ringIterCreate (rset);
	if (!riter) {
		ringSetDestroy (rsetnew);
		return NULL;
	}
		/*
		* Copy objects in PGPKeySet to rsetnew except secret objects.
		* At the same time, accumulate any message recovery keys into
		* rkeyset.
		*/
	while ((level = ringIterNextObjectAnywhere(riter)) > 0) {
		RingObject *obj = ringIterCurrentObject (riter, level);
		/* Skip secret objects and non-members */
		if (ringObjectType (obj) == RINGTYPE_SEC)
			continue;
		if (!(*keys->isMember)(keys, obj))
			continue;
		ringSetAddObject (rsetnew, obj);
		/* For key objects, look for message recovery keys */
		if (addmrks && ringObjectType (obj) == RINGTYPE_KEY) {
			RingObject	*rkey;		/* Recovery key */
			unsigned		nrkeys;		/* Number of recovery keys */
			if (ringKeyRecoveryKey (obj, rset, 0, NULL, &nrkeys, NULL)) {
				/* Add to special set for recovery keys */
				while (nrkeys-- > 0) {
					rkey = ringKeyRecoveryKey (obj, rset, nrkeys, NULL,
						NULL, NULL);
					pgpAssert (rkey);
					if (!rkeyset) {
						rkeyset = ringSetCreate (ringSetPool(rset));
						if (!rkeyset) {
							ringIterDestroy (riter);
							ringSetDestroy (rsetnew);
							return NULL;
						}
					}
					ringSetAddObjectChildren (rkeyset, rset, rkey);
				}
			}
		}
	}
	ringIterDestroy (riter);

	/* Last, merge rkeyset into rsetnew, also stripping secrets */
	if (rkeyset) {
		ringSetFreeze (rkeyset);
		riter = ringIterCreate (rkeyset);
		if (!riter) {
			ringSetDestroy (rkeyset);
			ringSetDestroy (rsetnew);
			return NULL;
		}
		/* Loop over rsetnew iterator, adding non-secret objects */
		while ((level = ringIterNextObjectAnywhere(riter)) > 0) {
			RingObject *obj = ringIterCurrentObject (riter, level);
			if (ringObjectType (obj) == RINGTYPE_SEC)
				continue;
			ringSetAddObject (rsetnew, obj);
		}
		ringIterDestroy (riter);
		ringSetDestroy (rkeyset);
	}

	/* Return new set in frozen form */
	ringSetFreeze (rsetnew);
	return rsetnew;
}


/*
* Extract all keys in the specified KeySet to the specified file. This
* version always ascii armors them.
* Only extracts public portions of keys.
*/
PGPError
pgpExportKeyFile (PGPKeySet *keys, PGPFileRef *fileRef)
{
	RingSet		*rset,
				*rsetpub;
	FILE		*fp;
	PGPError	error;

	fp = pgpFileRefStdIOOpen(fileRef, (kPGPFileOpenStdWriteFlags
										| kPGPFileOpenTextMode),
							kPGPFileTypeExportedKeys, &error);
	if (!fp)
		return error;
	rset = pgpKeyDBRingSet (keys->keyDB);
	rsetpub = filterPubRingSet (rset, keys, FALSE);
	if (!rsetpub) {
		pgpStdIOClose (fp);
		pgpDeleteFile(fileRef);
		return PGPERR_NOMEM;
	}
	error = (PGPError)pgpWriteArmoredSetFile (fp, rsetpub, pgpEnv);
	pgpStdIOClose (fp);
	ringSetDestroy (rsetpub);
	return error;
}


/*
* Extract all keys in the specified KeySet to the specified memory
* buffer, which is of size *length. Return number of bytes written
* in *length. This version always ascii armors the keys.
* Only extracts public portions of keys.
*/
PGPError
pgpExportKeyBuffer (PGPKeySet *keys, byte *buffer, size_t *length)
{
	RingSet	*rset,
			*rsetpub;
	PGPError	error;

	rset = pgpKeyDBRingSet (keys->keyDB);
	rsetpub = filterPubRingSet (rset, keys, FALSE);
	if (!rsetpub) {
		return PGPERR_NOMEM;
	}
	error = (PGPError) pgpWriteArmoredSetBuffer (buffer, length, rsetpub,
		pgpEnv);
	ringSetDestroy (rsetpub);
	return error;
}


#if defined(_WIN32)

#define SEMAPHORENAME	("PGPkeysInstSem")
static HANDLE hSem;

/*
* Return TRUE if we have the interlock semaphore, FALSE if someone else
* has it
*/
Boolean
pgpOpenSemaphore ()
{

	/* If already been called, return false */
	if (hSem)
		return FALSE;

/* Create or open a named semaphore */
hSem = CreateSemaphore (NULL, 0, 1, SEMAPHORENAME);

	/* If another instance exists, close handle and return false */
if ((hSem != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)) {
	CloseHandle(hSem);
		hSem = (HANDLE)0;
	return FALSE;
}

/* If new semaphore was created, return TRUE. */
return TRUE;
}

/* Clear the semaphore which keeps others from running */
void
pgpCloseSemaphore ()
{
	if (!hSem)
		return;

	CloseHandle (hSem);
	hSem = (HANDLE)0;
}

#else

/*
* Return TRUE if we have the interlock semaphore, FALSE if someone else
* has it
*/
Boolean
pgpOpenSemaphore ()
{
	return TRUE;
}

/* Release the interlock semaphore */
void
pgpCloseSemaphore ()
{
}

#endif


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
