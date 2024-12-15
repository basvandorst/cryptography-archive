/*
* pgpFileDB.c -- Manage PGPKeyDBs based on RingFiles
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpFileDB.c,v 1.31.2.5 1997/06/07 09:50:22 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>		/* For O_CREAT, O_EXCL */
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef UNIX
#include <sys/stat.h>
#endif

#ifndef W_OK
#define W_OK 2
#endif

#include "pgpKDBint.h"
#include "pgpFileRef.h"
#include "pgpFileNames.h"
#include "pgpDebug.h"
#include "pgpFile.h"
#include "pgpRngRead.h"

/* Private data for RingFile type of PGPKeyDB */
typedef struct RingDB {
	RingFile *rfile;			/* RingFile for keys */
	PgpFile *pfile;		 	/* PgpFile for keys */
	PGPFileRef *fileRef;		/* fileRef for keys */
	RingSet *rsmut;		 	/* Mutable ringset */
	RingSet const *rsimmut;		 /* Immutable ringset */
		RingSet	*rsfrozen;		 /* Frozen copy of rsmut */
		byte	 		*membuf;			/* Memory buffer for memory pfiles */
		Boolean	bwriteable;		 /* True if a writeable database */
		Boolean	bdirty;		 	/* True if rsmut has been changed */
		Boolean	bprivate;		/* True if a private keyring */
		Boolean	btrusted;		/* True if should get trust packets */
	Boolean			bfreebuf;	 	/* True if should free membuf on close */
} RingDB;



/***************************** File IO ****************************/


/*
* Create a temp key file which is not used by anyone else.
* Return a stream pointer to the file, and set the file reference in
* *tmpFileRef1. Base the file reference on the given file.
* The returned file reference needs to be free'd with pgpFreeFileRef, and of
* course the returned file handle needs to be closed too.
* The implementation of this tends to be OS (and possibly file system)
* dependent.
*/
static FILE *
tmpkeyfile (PGPFileRef const *fileRef, PGPFileRef **tmpFileRef1,
			PGPFileOpenFlags flags, Boolean isPrivate)
{
	char *				file = NULL;
	char *				tmp = NULL;
	char *				nfile = NULL;
	char *				nfilep = NULL;
	PGPFileRef *		tmpFileRef = NULL;
	FILE *				stdFile = NULL;
	
	if ((file = pgpGetFileRefName(fileRef)) == NULL)
		goto error;
	if ((tmp = pgpFileNameContract(file)) == NULL)
		goto error;
	if ((nfile = pgpFileNameExtend(tmp, ".pg0")) == NULL)
		goto error;
	nfilep = nfile + strlen(file) - 1;
	if ((tmpFileRef = pgpCopyFileRef(fileRef)) == NULL)
		goto error;
	for ( ; ; ) {
		if ((pgpSetFileRefName(&tmpFileRef, nfile)) != PGPERR_OK)
			goto error;
		if ((stdFile = pgpFileRefStdIOOpen(tmpFileRef, flags,
							isPrivate ? kPGPFileTypePrivRing
								: kPGPFileTypePubRing, NULL)) != NULL)
			break;
		if ((*nfilep)++ > 'z')
			goto error;
	}
error:
	pgpMemFree(file);
	pgpMemFree(tmp);
	pgpMemFree(nfile);
	if (stdFile != NULL)
		*tmpFileRef1 = tmpFileRef;
	else if (tmpFileRef != NULL)
		pgpFreeFileRef(tmpFileRef);
	return stdFile;
}

/*
* Error recovery routine for do_write, below. Called to re-open the
* original file due to some problem while writing out the updated version.
* The original file is back to its original filename. newfile is the
* tmp ringfile which we have succeeded in outputting. Use it to restore
* rsmut. Closing it will be done by our caller.
*/
static void
do_recover (RingDB *rdb, RingFile *newfile)
{
			FILE *f;
			PGPError error;
			RingPool *ringpool = ringSetPool (rdb->rsimmut);

		f = pgpFileRefStdIOOpen(rdb->fileRef,
				 		 		 	rdb->bwriteable ? kPGPFileOpenReadWritePerm
				 		 		 		 		 	: kPGPFileOpenReadPerm,
				 		 		 	kPGPFileTypeNone, &error);
			pgpAssert (f);
			rdb->pfile = pgpFileReadOpen (f, NULL, NULL);
			pgpAssert (rdb->pfile);
			rdb->rfile = ringFileOpen(ringpool, rdb->pfile, rdb->btrusted, &error);
			pgpAssert (rdb->rfile);
			rdb->rsimmut = ringFileSet (rdb->rfile);
			if (newfile) {
			rdb->rsmut = ringSetCreate (ringpool);
				 ringSetAddSet (rdb->rsmut, ringFileSet (newfile));
			}
}


/*
* This writes out a RingSet which is a replacement for a RingFile
* which is open. We do it carefully since on some OS's we can't
* rename open files. This closes the RingFile but leaves the
* RingSet open and pointing to the filename.
* Renames existing file.ext to file.bak.
* Variables with suffix "1" refer to original file; suffix "2" are
* for a tmp file; suffix "3" are for the updated file.
* The reason the tmp file is used is to have some backing store so
* we can close and rename the original file before outputting the
* updated version.
*
* There are some tricky issues here re error recovery. Especially once
* we get rid of the original key file, we may not be able to get it back
* while we have the other one open. So we will require that the .bak file
* renaming must work, so that we can rename the .bak file back if necessary.
* Otherwise we have problems when we run out of disk space midway. This
* means that the write will fail if the .bak file renaming fails, such as if
* there is a .bak file but it is not writeable or deletable.
*/
static PGPError
do_write (RingDB *rdb)
	{
			char	*file;						/* Filename for keyring */
			char	*bak,						/* .bak filename for keyring */
			*tmp;						/* Temp to help create .bak */
			PGPFileRef	*tmpFileRef;					/* Temp file reference */
			PGPFileRef	*bakFileRef;					/* Bak file reference */
			RingSet		*set;					/* RingSet we are writing out */
			RingFile		*rfile1,					/* Original RingFile for keyring */
					*rfile2,					/* RingFile for temp file */
				 				*rfile3;					/* RingFile for updated keyring file */
		PgpFile		*pfp1,			 		/* Original PgpFile for keyring */
					*pfp2,					/* PgpFile for temp file */
				 				*pfp3;					/* PgpFile for updated keyring file */
			FILE		*fp2,					/* Stream handle for temp file */
					*fp3;					/* Stream handle for updated keyring */
	int 	flags;
	int error = 0, err1;

	set = rdb->rsmut;
	rfile1 = rdb->rfile;
	pfp1 = rdb->pfile;
	flags = (rdb->btrusted && !rdb->bprivate) ? PGP_WRITETRUST_PUB : 0;

	file = pgpGetFileRefName(rdb->fileRef);

	tmp = pgpFileNameContract (file);
#if MACINTOSH
	bak = pgpFileNameExtend (tmp, " (backup)");
#else
	bak = pgpFileNameExtend (tmp, ".bak");
#endif
	bakFileRef = pgpCopyFileRef(rdb->fileRef);
	pgpSetFileRefName(&bakFileRef, bak);
	pgpFree(bak);
	pgpFree(tmp);
	pgpFree(file);
	fp2 = tmpkeyfile (rdb->fileRef, &tmpFileRef, kPGPFileOpenStdUpdateFlags,
						rdb->bprivate);
	if (!fp2) {
		pgpFreeFileRef (tmpFileRef);
		pgpFreeFileRef (bakFileRef);
		return PGPERR_NO_FILE;
	}
	/* Write out new data to tmpfile2 */
	pfp2 = pgpFileWriteOpen (fp2, NULL);
	error = ringSetWrite (set, pfp2, &rfile2, PGPVERSION_3, flags);
	pgpFileFlush (pfp2);
	if (error) {
		pgpFileClose (pfp2);
		pgpDeleteFile (tmpFileRef);
		pgpFreeFileRef (tmpFileRef);
		pgpFreeFileRef (bakFileRef);
		return error;
	}

	/* Must close set in order to be able to cleanly close rfile1 */
	ringSetDestroy(set);
	rdb->rsmut = NULL;
	if (ringFileClose (rfile1) != 0) {
		/* Have a problem here, it should have closed */
		/* Probably due to a pesky RingSet leak! */
		/* Actually happens a lot with pgp3, it doesn't care */
/*		fprintf (stderr, "Warning: %s didn't close!\n", file); */
		pgpAssert (0);
	}
	/* Close the PgpFile regardless */
	pgpFileClose (pfp1);

	/*
	* Convert temp PgpFile from write mode to read mode now, it will
	* serve as the backing store for the next step.
	*/
	error = pgpFileWrite2Read (pfp2);
	pgpAssert (error == 0);

	/* Now rename the old file */
	pgpDeleteFile (bakFileRef);
 error = pgpRenameFile (rdb->fileRef, bakFileRef);

#if 0
/*
	* The software is operating in a mode where this rename often fails.
	* If another process has the keyring file open under Windows, the file
	* can't be renamed. What happens in that case is we proceed with the
	* write and then the app posts a message to the other app(s) telling
	* it to re-read the keyring file. So we will not call that an error
	* at this point.
*/
	if (error) {
		/* Failed to rename, we cannot proceed. Get orig file open again */
		do_recover (rdb, rfile2);
		ringFileClose (rfile2);
		pgpFileClose (pfp2);
		pgpDeleteFile (tmpFileRef);
		pgpFreeFileRef (tmpFileRef);
		pgpFreeFileRef (bakFileRef);
		return PGPERR_NO_FILE;
	}
#endif

	/* Output data again, this time to original filename */
	fp3 = pgpFileRefStdIOOpen (rdb->fileRef, kPGPFileOpenStdUpdateFlags,
								rdb->bprivate ? kPGPFileTypePrivRing
											: kPGPFileTypePubRing, NULL);
	pfp3 = pgpFileWriteOpen (fp3, NULL);
	if (!pfp3) {
			/*
		* Failure to open output file. Should not happen since we just
		* had one and renamed it to .bak. Try to recover anyway.
		* Rename .bak file back to original and re-open it.
			*/
		pgpDeleteFile (rdb->fileRef);
		error = pgpRenameFile (bakFileRef, rdb->fileRef);
		pgpAssert (error == PGPERR_OK); /* Else we are lost */
		do_recover (rdb, rfile2);
		ringFileClose (rfile2);
		pgpFileClose (pfp2);
		pgpDeleteFile (tmpFileRef);
		pgpFreeFileRef (tmpFileRef);
		pgpFreeFileRef (bakFileRef);
		return PGPERR_NO_FILE;
	}
	error = ringSetWrite (ringFileSet(rfile2), pfp3, &rfile3,
		PGPVERSION_3, flags);
	pgpFileFlush (pfp3);
	if (error) {
		/* Try to recover by restoring .bak file */
		pgpFileClose (pfp3);
		pgpDeleteFile (rdb->fileRef);
		err1 = pgpRenameFile (bakFileRef, rdb->fileRef);
		pgpAssert (err1 == PGPERR_OK);	/* Else we are lost */
		do_recover (rdb, rfile2);
		ringFileClose (rfile2);
		pgpFileClose (pfp2);
		pgpDeleteFile (tmpFileRef);
		pgpFreeFileRef (tmpFileRef);
		pgpFreeFileRef (bakFileRef);
		return error;
}

	/* Delete tmp file now */
	err1 = ringFileClose (rfile2);
	pgpAssert (err1==0);
	pgpFileClose (pfp2);
	pgpDeleteFile (tmpFileRef);
	pgpFreeFileRef (tmpFileRef);

	/*
	* Convert final PgpFile from write mode to read mode now, it will
	* serve as the backing store from now on.
	*/
	error = pgpFileWrite2Read (pfp3);
	pgpAssert (error == PGPERR_OK);

	pgpFreeFileRef (bakFileRef);

	rdb->rfile = rfile3;
	rdb->pfile = pfp3;
	return PGPERR_OK;
}

/*
* Open the specified ringfile. If writeable flag is clear, it will be
* forced read-only, otherwise whether it is writeable will depend on
* permissions. Return a filled-in rdb structure.
* Note that "private" is not a concept of the underlying library, but
* is used in the add function to decide which objects to accept.
*/
static PGPError
do_open (PGPFileRef const *fileRef1, int trusted, int private, int writeable,
	RingPool *ringpool, RingDB *rdb)
{
		PGPFileRef	*fileRef;
		FILE	*f;
		PgpFile	*pfile;
		RingFile	*rfile;
	RingSet const *rsimmut;
	RingSet *rsmut;
	int error;

	rsmut = ringSetCreate (ringpool);
	if (!rsmut) {
		return PGPERR_NOMEM;
	}

	fileRef = pgpCopyFileRef(fileRef1);

	f = pgpFileRefStdIOOpen(fileRef, writeable ? kPGPFileOpenReadWritePerm
												: kPGPFileOpenReadPerm,
							kPGPFileTypeNone, &error);
	if (!f) {
		/* Try creating file if it did not exist */
		f = pgpFileRefStdIOOpen(fileRef, kPGPFileOpenStdUpdateFlags,
								private ? kPGPFileTypePrivRing
									: kPGPFileTypePubRing, &error);
		if (!f) {
			ringSetDestroy (rsmut);
			pgpFreeFileRef(fileRef);
			return error;
		}
	}

	pfile = pgpFileReadOpen (f, NULL, NULL);
	if (!pfile) {
		pgpStdIOClose (f);
		ringSetDestroy (rsmut);
		pgpFreeFileRef(fileRef);
		return PGPERR_NOMEM;
	}

	rfile = ringFileOpen (ringpool, pfile, trusted, &error);
	if (!rfile) {
		pgpFileClose (pfile);
		ringSetDestroy (rsmut);
		pgpFreeFileRef(fileRef);
		return error;
	}

	rsimmut = ringFileSet (rfile);
	ringSetAddSet (rsmut, rsimmut);

		rdb->fileRef	= fileRef;
		rdb->pfile	= pfile;
		rdb->rfile	= rfile;
		rdb->rsimmut	= rsimmut;
		rdb->rsmut	= rsmut;
	rdb->bwriteable = writeable;
	rdb->bdirty = ringFileIsDirty (rfile);
	rdb->btrusted = trusted;
	rdb->bprivate = private;

	return PGPERR_OK;
}


/* Close the open keyfile object */
static PGPError
do_close (RingDB *rdb)
{
	if (rdb->rsfrozen) {
		ringSetDestroy (rdb->rsfrozen);
		rdb->rsfrozen = 0;
	}
	rdb->rsimmut = 0;
	ringSetDestroy (rdb->rsmut);
	rdb->rsmut = 0;
		ringFileClose (rdb->rfile);
	rdb->rfile = 0;
	pgpFileClose (rdb->pfile);
	rdb->pfile = 0;
	if (rdb->bfreebuf && rdb->membuf){
		pgpMemFree (rdb->membuf);
	}
	return PGPERR_OK;
}




/******************** Virtual Functions ************************/


static Boolean
rdbIsMutable (PGPKeyDB *kdb) {
	RingDB *rdb = (RingDB *)kdb->private;

	return rdb->bwriteable;
}

static Boolean
rdbObjIsMutable (PGPKeyDB *kdb, RingObject *testObj)
{	
	RingDB *rdb = (RingDB *)kdb->private;

	return rdb->bwriteable && ringSetIsMember (rdb->rsmut, testObj);
}

static Boolean
rdbIsDirty (PGPKeyDB *kdb) {
	RingDB *rdb = (RingDB *)kdb->private;

	return rdb->bdirty;
}

/* Mark as dirty */
static void
rdbDirty (RingDB *rdb)
{
	rdb->bdirty = TRUE;
	if (rdb->rsfrozen) {
		ringSetDestroy (rdb->rsfrozen);
		rdb->rsfrozen = 0;
	}
}

static RingSet *
rdbGetRingSet (PGPKeyDB *kdb) {
	RingDB *rdb = (RingDB *)kdb->private;
	RingSet *rset;

	if (!(rset = rdb->rsfrozen)) {
		rset = ringSetCreate (ringSetPool (rdb->rsmut));
		ringSetAddSet (rset, rdb->rsmut);
		ringSetFreeze (rset);
		rdb->rsfrozen = rset;
	}
	return rset;
}

static PGPError
rdbAdd (PGPKeyDB *kdb, RingSet *toAdd) {
	RingDB *rdb = (RingDB *)kdb->private;
	RingIterator *riAdd;		/* Iterator over toAdd set */
	RingObject *robj;	 	/* Object we are adding */
	int level;		/* Level of iterator in hierarchy */
	int				type;		 /* Type of robj */
	int added;		/* Number of objects added */
	int skipped;	/* Number of objects skipped */
	int				skipparts; /* True if skipping, obj not for us */

	if (!rdb->bwriteable)
		return PGPERR_KEYDB_KEYDBREADONLY;

	riAdd = ringIterCreate (toAdd);
	if (!riAdd) {
		return PGPERR_NOMEM;
}

added = 0;
skipped = 0;
skipparts = 0;

	while ((level = ringIterNextObjectAnywhere(riAdd)) > 0) {
		robj = ringIterCurrentObject (riAdd, level);
		type = ringObjectType (robj);
		if (type==RINGTYPE_KEY) {
				/*
			* If key lacks secret object, put it only on the public
			* ring, and if key comes only from a source where it is a
			* secret key, put it only on the secret ring
				*/
			if (rdb->bprivate) {
				skipparts = !ringKeyIsSec (toAdd, robj);
			} else {
				skipparts = ringKeyIsSecOnly (toAdd, robj);
			}
		}
		/* Signatures only go to public keyrings, secrets to private */
		if (skipparts ||
			((type==RINGTYPE_SIG)&&rdb->bprivate) ||
			((type==RINGTYPE_SEC)&&!rdb->bprivate)) {
			skipped += 1;
		} else {
			ringSetAddObject (rdb->rsmut, robj);
			added += 1;
		}
	}
	ringIterDestroy (riAdd);

	rdbDirty (rdb);
	return PGPERR_OK;
}


static PGPError
rdbRemove (PGPKeyDB *kdb, RingObject *toRemove)
{
	RingDB *rdb = (RingDB *)kdb->private;

	if (!rdb->bwriteable)
		return PGPERR_KEYDB_KEYDBREADONLY;

	if (ringSetIsMember (rdb->rsmut, toRemove))
		rdbDirty (rdb);
	return ringSetRemObject (rdb->rsmut, toRemove);
}

/*
* Note that unions don't pass this call down, they take care of it
* themselves
*/
static PGPError
rdbChanged (PGPKeyDB *kdb, RingSet *changedkeys)
{
	RingDB *rdb = (RingDB *)kdb->private;

	if (!rdb->bwriteable)
		return PGPERR_KEYDB_KEYDBREADONLY;

	rdbDirty (rdb);
	return pgpReSortKeys (kdb, changedkeys);
}

static PGPError
rdbCommit (PGPKeyDB *kdb) {
	RingDB *rdb = (RingDB *)kdb->private;
	RingPool	*ringpool = ringSetPool (rdb->rsmut);
	PGPError error;

	if (!rdb->bwriteable)
		return PGPERR_OK;

	/* Don't check for trust changed if untrusted file */
	if (!rdb->bdirty && (!rdb->btrusted ||
				(rdb->btrusted && !ringFileIsTrustChanged(rdb->rfile))))
		return PGPERR_OK;

	/* Must use frozen set for writing */

	ringSetFreeze (rdb->rsmut);
	if (rdb->rsfrozen) {
		ringSetDestroy (rdb->rsfrozen);
		rdb->rsfrozen = 0;
	}
	error = do_write (rdb);

	if (error) {
		/*
		* Sometimes on error we have fixed rdb->rsmut, but possibly it may
		* still be frozen as above. To be safe we will always make a copy
		* here so it is writeable in the future.
*/
		RingSet *rset = ringSetCreate (ringpool);
		pgpAssert (rset);
		pgpAssert (rdb->rsmut);
		ringSetAddSet (rset, rdb->rsmut);
		ringSetDestroy (rdb->rsmut);
		rdb->rsmut = rset;
		return error;
	}

	rdb->rsimmut = ringFileSet (rdb->rfile);
	rdb->rsmut = ringSetCreate (ringpool);
	ringSetAddSet (rdb->rsmut, rdb->rsimmut);
	rdb->bdirty = 0;

return PGPERR_OK;
}


static PGPError
rdbRevert (PGPKeyDB *kdb) {
	RingDB *rdb = (RingDB *)kdb->private;

	if (!rdb->bdirty)
		return PGPERR_OK;
	rdb->bdirty = 0;
	if (rdb->rsmut) {
		ringSetDestroy (rdb->rsmut);
		rdb->rsmut = ringSetCreate (ringSetPool(rdb->rsimmut));
		if (!rdb->rsmut) {
			return PGPERR_NOMEM;
		}
		ringSetAddSet (rdb->rsmut, rdb->rsimmut);
	}
	if (rdb->rsfrozen) {
		ringSetDestroy (rdb->rsfrozen);
		rdb->rsfrozen = 0;
	}
	return PGPERR_OK;
}

static PGPError
rdbReload (PGPKeyDB *kdb) {
	RingDB *rdb = (RingDB *)kdb->private;
	PGPFileRef *fileRef;
	PGPError err;

	if (rdb->membuf)
		return PGPERR_OK;
	if ((fileRef = pgpCopyFileRef(rdb->fileRef)) == NULL)
		return PGPERR_NOMEM;
	err = do_close (rdb);
	if (err) {
		pgpFreeFileRef(fileRef);
		return err;
	}
	err = do_open (fileRef, rdb->btrusted, rdb->bprivate,
		rdb->bwriteable, ringSetPool (rdb->rsimmut), rdb);
	rdb->bdirty = 0;
	if (rdb->rsfrozen) {
		ringSetDestroy (rdb->rsfrozen);
		rdb->rsfrozen = 0;
	}
	pgpFreeFileRef(fileRef);
	return err;
}

static void
rdbDestroy (PGPKeyDB *kdb)
{
	RingDB *rdb = (RingDB *)kdb->private;

	do_close (rdb);
	pgpFreeFileRef (rdb->fileRef);
	pgpMemFree (rdb);
}


/**************************** Constructor **************************/


PGPKeyDB *
pgpCreateFileKeyDB (PGPFileRef const *fileRef, int trusted, int private,
	int writeable, RingPool *ringpool, PGPError *error)
{
	PGPKeyDB *kdb;
	RingDB *rdb;

	*error = 0;
	kdb = pgpKeyDBCreateInternal ();
	if (!kdb) {
		*error = PGPERR_NOMEM;
		return NULL;
	}
	rdb = (RingDB *) pgpMemAlloc (sizeof (RingDB));
	if (!rdb) {
		*error = PGPERR_NOMEM;
		pgpKeyDBDestroyInternal (kdb);
		return NULL;
	}

	memset (rdb, 0, sizeof (*rdb));

	*error = do_open (fileRef, trusted, private, writeable, ringpool, rdb);
	if (*error) {
		pgpMemFree (rdb);
		pgpKeyDBDestroyInternal (kdb);
		return NULL;
}

	kdb->private 	 	= rdb;
	kdb->magic				= PGPKDBFILEMAGIC;
	kdb->isMutable			= rdbIsMutable;
	kdb->objIsMutable	 	= rdbObjIsMutable;
	kdb->isDirty 	 	= rdbIsDirty;
	kdb->getRingSet			= rdbGetRingSet;
	kdb->add 	 	= rdbAdd;
	kdb->remove 		= rdbRemove;
	kdb->changed			= rdbChanged;
	kdb->commit 		= rdbCommit;
	kdb->revert 		= rdbRevert;
	kdb->reload 		= rdbReload;
  kdb->destroy			= rdbDestroy;

	pgpKeyDBInitInternal(kdb);

  return kdb;
}

/*
* Create a File type KeyDB from a memory buffer. This will be an
* immutable KeyDB since we can't save our changes anywhere.
*/
PGPKeyDB *
pgpCreateMemFileKeyDB (byte *buf, size_t length, RingPool *ringpool,
	PGPError *error)
{
	PGPKeyDB *kdb;
	RingDB *rdb;

	*error = 0;
	kdb = pgpKeyDBCreateInternal ();
	if (!kdb) {
 	*error = PGPERR_NOMEM;
		return NULL;
	}
	rdb = (RingDB *) pgpMemAlloc (sizeof (RingDB));
	if (!rdb) {
		*error = PGPERR_NOMEM;
		pgpKeyDBDestroyInternal (kdb);
  	return NULL;
	}

	memset (rdb, 0, sizeof (*rdb));

 rdb->rsmut = ringSetCreate (ringpool);
	if (!rdb->rsmut) {
		pgpMemFree (rdb);
		pgpKeyDBDestroyInternal (kdb);
		*error = PGPERR_NOMEM;
		return NULL;
	}

	rdb->pfile = pgpFileMemReadOpen (buf, length);
	if (!rdb->pfile) {
		ringSetDestroy (rdb->rsmut);
		pgpMemFree (rdb);
		pgpKeyDBDestroyInternal (kdb);
		*error = PGPERR_NOMEM;
		return NULL;
	}

	rdb->rfile = ringFileOpen (ringpool, rdb->pfile, 0, error);
	if (!rdb->rfile) {
		pgpFileClose (rdb->pfile);
		ringSetDestroy (rdb->rsmut);
		pgpMemFree (rdb);
		pgpKeyDBDestroyInternal (kdb);
		return NULL;
	}

	rdb->rsimmut = ringFileSet (rdb->rfile);
	ringSetAddSet (rdb->rsmut, rdb->rsimmut);

	rdb->membuf		= buf;
	rdb->bfreebuf	 = 1;
	rdb->fileRef	= pgpNewFileRefFromFullPath("");

	kdb->private 	 	= rdb;
	kdb->magic				= PGPKDBFILEMAGIC;
	kdb->isMutable			= rdbIsMutable;
	kdb->objIsMutable	 	= rdbObjIsMutable;
	kdb->isDirty 	 	= rdbIsDirty;
	kdb->getRingSet			= rdbGetRingSet;
	kdb->add 	 	= rdbAdd;
	kdb->remove 		= rdbRemove;
	kdb->changed			= rdbChanged;
	kdb->commit 		= rdbCommit;
  kdb->revert 		= rdbRevert;
	kdb->reload 		= rdbReload;
	kdb->destroy			= rdbDestroy;

	pgpKeyDBInitInternal(kdb);

 return kdb;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
