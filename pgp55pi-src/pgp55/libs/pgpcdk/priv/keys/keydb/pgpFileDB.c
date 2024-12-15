/*
 * pgpFileDB.c
 * Manage PGPKeyDB's based on RingFiles
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
 * All rights reserved
 *
 * $Id: pgpFileDB.c,v 1.67 1997/10/09 20:56:38 mhw Exp $
 */
#include <stdio.h>
#include <string.h>

#include "pgpConfig.h"

#if HAVE_FCNTL_H
#include <fcntl.h>		/* For O_CREAT, O_EXCL */
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifndef W_OK
#define W_OK 2
#endif

#include "pgpKDBInt.h"
#include "pgpFileRef.h"
#include "pgpFileSpec.h"
#include "pgpFileUtilities.h"
#include "pgpStrings.h"
#include "pgpFileNames.h"
#include "pgpDebug.h"
#include "pgpFile.h"
#include "pgpRngRead.h"
#include "pgpContext.h"

/* Private data for RingFile type of PGPKeyDBRef */
typedef struct RingDB {
	PGPContextRef	context;
	RingFile *		ringFile;		/* RingFile for keys */
	PGPFile *		pgpFile;		/* PGPFile for keys */
	PFLFileSpecRef	mainFileRef;	/* Main keyring filename */
	PFLFileSpecRef	readFileRef;	/* Filename of keyring backing store */
	FILE *			mainStdFile;	/* Main keyring FILE (for locking only) */
	RingSet *		mutableSet;		/* Mutable ringset */
								/* mutableSet will be NULL if !bwriteable */
	RingSet const *	immutableSet;	/* Immutable ringset */
	RingSet *		frozenSet;		/* Frozen copy of mutableSet */
	PGPBoolean		bwriteable;		/* True if a writeable database */
	PGPBoolean		bdirty;			/* True if mutableSet has been changed */
	PGPBoolean		bprivate;		/* True if a private keyring */
	PGPBoolean		btrusted;		/* True if should get trust packets */
	PGPBoolean		bmembuf;		/* True if memory-buffer based */
	PGPBoolean		btempfile;		/* True if readFileRef is a temp file */
	PGPKeyRingOpenFlags	openFlags;	/* Flags used to open keyring */

	DEBUG_STRUCT_CONSTRUCTOR( RingDB )
} RingDB;


/***************************** File IO ****************************/

/*
 * Readers and writers can safely access the same key ring simultaneously.
 * The only constraint is that at most one writer can access the file at
 * the same time.
 * 
 * This wasn't possible before because the bulk of the key data is loaded
 * on demand using an in-memory index which is built on open. This requires
 * that the file used for backing store never change. Our solution is to
 * never use the main ring file for backing store, but instead use a static
 * copy of the file.
 * 
 * But copying the file is a lot of overhead to suffer every time the key
 * ring is opened. So we leave the copy lying around so the next user will
 * be spared the overhead, amortizing the cost over several accesses.
 * 
 * These static copies are also useful as backups, and that's how they're
 * named on disk. Every time a key ring is closed, old backups are purged.
 * Currently, all but the most recent 4 backups are removed.
 */

	static const char *
GetSeparator(
	PGPSize		maxFileLength )
{
#if PGP_MACINTOSH
	(void)maxFileLength;	/* Avoid warning */
	return " Backup ";
#else
	if ( maxFileLength >= 18 )
		return "-bak-";
	else
		return "-";
#endif
}

	static PGPError
FindMatchingBackup(
	PFLFileSpecRef		mainFileRef,
	PFLFileSpecRef *	readFileRef )
{
	PGPError			err = kPGPError_NoErr;
	PFLContextRef		context = PFLGetFileSpecContext( mainFileRef );
	PGPSize				maxFileLength;
	char *				mainFileName = NULL;
	PFLFileInfo			mainFileInfo;
	PFLDirectoryIterRef	dirIter = kPGPInvalidRef;
	PFLFileSpecRef		fileRef = kPGPInvalidRef;
	char *				fileName = NULL;
	const char *		separator;
	PFLFileInfo			fileInfo;
	PFLFileSpecRef		newestFileRef = kPGPInvalidRef;
	time_t				newestFileModTime = 0;	/* Avoid warning */
	PGPBoolean			valid;
	PGPUInt32			number;

	*readFileRef = NULL;

	err = PFLGetFileSpecName( mainFileRef, &mainFileName );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = PFLGetMaxFileSpecNameLength( mainFileRef, &maxFileLength );
	if ( IsPGPError( err ) )
		goto cleanup;

	separator = GetSeparator( maxFileLength );

	err = PFLGetFileInfo( mainFileRef, &mainFileInfo );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = PFLGetParentDirectory( mainFileRef, &fileRef );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = PFLNewDirectoryIter( fileRef, &dirIter );
	if ( IsPGPError( err ) )
		goto cleanup;

	PFLFreeFileSpec( fileRef );
	fileRef = NULL;

	while ( IsntPGPError( err = PFLNextFileInDirectory( dirIter, &fileRef ) ) )
	{
		err = PFLGetFileSpecName( fileRef, &fileName );
		if ( IsPGPError( err ) )
			goto cleanup;

		err = PGPVerifyNumberFileName( mainFileName, separator, fileName,
					maxFileLength + 1, &valid, &number );
		if ( IsPGPError( err ) )
			goto cleanup;

		if ( valid )
		{
			err = PFLGetFileInfo( fileRef, &fileInfo );
			if ( IsPGPError( err ) )
				goto cleanup;

			/*
			 * XXX: We may want to rethink this heuristic.
			 *      In particular, what happens if a backup file
			 *      exists which is newer than the current time?
			 *      Also, it would be nice to check file meta
			 *      info (like creator/type on the Mac) to make
			 *      sure it matches.
			 */
			if ( ( fileInfo.flags & kPGPFileInfo_IsPlainFile ) &&
				 fileInfo.dataLength == mainFileInfo.dataLength &&
				 fileInfo.modificationTime >= mainFileInfo.modificationTime &&
				 ( newestFileRef == kPGPInvalidRef ||
				   newestFileModTime < fileInfo.modificationTime ) )
			{
				if ( newestFileRef != kPGPInvalidRef )
					PFLFreeFileSpec( newestFileRef );
				newestFileRef = fileRef;
				fileRef = kPGPInvalidRef;
				newestFileModTime = fileInfo.modificationTime;
			}
		}

		if ( fileRef != kPGPInvalidRef )
		{
			PFLFreeFileSpec( fileRef );
			fileRef = NULL;
		}

		PFLContextMemFree( context, fileName );
		fileName = NULL;
	}
	if ( err != kPGPError_EndOfIteration )
		goto cleanup;

	if ( newestFileRef == kPGPInvalidRef )
	{
		err = kPGPError_FileNotFound;
		goto cleanup;
	}

	/* XXX: Maybe check file contents here, just to be sure? */

	err = kPGPError_NoErr;
	*readFileRef = newestFileRef;
	newestFileRef = kPGPInvalidRef;	/* We're no longer responsible for this */
	
cleanup:
	if ( fileRef != kPGPInvalidRef )
		PFLFreeFileSpec( fileRef );
	if ( newestFileRef != kPGPInvalidRef )
		PFLFreeFileSpec( newestFileRef );
	if ( dirIter != kPGPInvalidRef )
		PFLFreeDirectoryIter( dirIter );
	if ( IsntNull( mainFileName ) )
		PFLContextMemFree( context, mainFileName );
	if ( IsntNull( fileName ) )
		PFLContextMemFree( context, fileName );
	return err;
}

	static PGPError
CopyStdFile(
	PFLContextRef	context,
	FILE *			origFile,
	FILE *			destFile )
{
	int			stdErr;
	PGPError	err = kPGPError_NoErr;
	char *		buffer = NULL;
	PGPSize		bufferSize = 16 * 1024L;
	PGPSize		length;

	rewind( origFile );
	rewind( destFile );

	buffer = (char *)PFLContextMemAlloc( context, bufferSize, 0 );
	if ( IsNull( buffer ) )
	{
		err = kPGPError_OutOfMemory;
		goto cleanup;
	}

	while ( ( length = fread( buffer, 1, bufferSize, origFile ) ) > 0 )
	{
		if ( length != fwrite( buffer, 1, length, destFile ) )
		{
			stdErr = ferror( destFile );
			if (
					0
#ifdef ENOSPC
					|| stdErr == ENOSPC
#endif
#ifdef EFBIG
					|| stdErr == EFBIG
#endif
					)
			{
				err = kPGPError_DiskFull;
			}
			else
			{
				err = kPGPError_WriteFailed;
			}
			goto cleanup;
		}
	}
	if ( !feof( origFile ) )
	{
		err = kPGPError_ReadFailed;
		goto cleanup;
	}

cleanup:
	if ( IsntNull( buffer ) )
		PFLContextMemFree( context, buffer );
	return err;
}

	static PGPError
MakeMatchingBackup(
	PFLFileSpecRef		mainFileRef,
	FILE *				mainStdFile,
	PGPFileType			fileType,
	PFLFileSpecRef *	readFileRef )
{
	PGPError			err = kPGPError_NoErr;
	PFLContextRef		context = PFLGetFileSpecContext( mainFileRef );
	PFLFileSpecRef		fileRef = kPGPInvalidRef;
	PFLFileSpecRef		tempFileRef = kPGPInvalidRef;
	char *				mainFileName = NULL;
	char *				fileName = NULL;
	PGPSize				maxFileLength;
	const char *		separator;
	PGPUInt32			number;
	PGPBoolean			exists;
	PGPBoolean			weOpenedMainFile = FALSE;
	FILE *				stdFile = NULL;

	*readFileRef = NULL;

	/* First copy the ring to a temp file in the backup directory */

	err = PFLFileSpecGetUniqueSpec( mainFileRef, &tempFileRef );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = pgpFileRefStdIOOpen( tempFileRef, kPGPFileOpenStdWriteFlags,
							   fileType, &stdFile );
	if ( IsPGPError( err ) )
		goto cleanup;

	if ( IsNull( mainStdFile ) )
	{
		err = pgpFileRefStdIOOpen( mainFileRef, kPGPFileOpenReadPerm,
								   kPGPFileTypeNone, &mainStdFile );
		if ( IsPGPError( err ) )
			goto cleanup;

		weOpenedMainFile = TRUE;
	}

	err = CopyStdFile( context, mainStdFile, stdFile );
	if ( IsPGPError( err ) )
		goto cleanup;
	
	pgpStdIOClose( stdFile );
	stdFile = NULL;


	/* Next, we find a new backup name to rename to */

	err = PFLGetMaxFileSpecNameLength( mainFileRef, &maxFileLength );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = PFLCopyFileSpec( mainFileRef, &fileRef );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = PFLGetFileSpecName( mainFileRef, &mainFileName );
	if ( IsPGPError( err ) )
		goto cleanup;

	separator = GetSeparator( maxFileLength );

	fileName = (char *)PFLContextMemAlloc( context, maxFileLength + 1, 0 );
	if ( IsNull( fileName ) )
	{
		err = kPGPError_OutOfMemory;
		goto cleanup;
	}

	for (number = 1; ; number++ )
	{
		err = PGPNewNumberFileName( mainFileName, separator, number,
									maxFileLength + 1, fileName );
		if ( IsPGPError( err ) )
			goto cleanup;

		err = PFLSetFileSpecName( fileRef, fileName );
		if ( IsPGPError( err ) )
			goto cleanup;

		err = PFLFileSpecExists( fileRef, &exists );
		if ( IsPGPError( err ) )
			goto cleanup;

		if ( !exists )
		{
			err = PFLFileSpecRename( tempFileRef, fileName );
			if ( IsntPGPError( err ) )
				break;
			else if ( err != kPGPError_ItemNotFound )
				goto cleanup;
		}
	}

	*readFileRef = fileRef;
	fileRef = NULL;		/* We're no longer responsible for it */

cleanup:
	if ( tempFileRef != kPGPInvalidRef )
		PFLFreeFileSpec( tempFileRef );
	if ( fileRef != kPGPInvalidRef )
		PFLFreeFileSpec( fileRef );
	if ( IsntNull( mainFileName ) )
		PFLContextMemFree( context, mainFileName );
	if ( IsntNull( fileName ) )
		PFLContextMemFree( context, fileName );
	if ( IsntNull( stdFile ) )
		pgpStdIOClose( stdFile );
	if ( weOpenedMainFile && IsntNull( mainStdFile ) )
		pgpStdIOClose( mainStdFile );
	return err;
}

	static PGPError
MakeTempCopy(
	PFLFileSpecRef			mainFileRef,
	FILE *					mainStdFile,
	PGPFileType				fileType,
	PFLFileSpecRef *		readFileRef )
{
	PGPError			err = kPGPError_NoErr;
	PFLContextRef		context = PFLGetFileSpecContext( mainFileRef );
	PFLFileSpecRef		fileRef = kPGPInvalidRef;
	FILE *				stdFile = NULL;
	PGPBoolean			weOpenedMainFile = FALSE;

	err = PFLGetTempFileSpec( context, kPGPInvalidRef, &fileRef );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = pgpFileRefStdIOOpen( fileRef, kPGPFileOpenStdWriteFlags,
							   fileType, &stdFile );
	if ( IsPGPError( err ) )
		goto cleanup;

	if ( IsNull( mainStdFile ) )
	{
		err = pgpFileRefStdIOOpen( mainFileRef, kPGPFileOpenReadPerm,
								   kPGPFileTypeNone, &mainStdFile );
		if ( IsPGPError( err ) )
			goto cleanup;

		weOpenedMainFile = TRUE;
	}

	err = CopyStdFile( context, mainStdFile, stdFile );
	if ( IsPGPError( err ) )
		goto cleanup;

	*readFileRef = fileRef;
	fileRef = NULL;		/* We're no longer responsible for it */

cleanup:
	if ( fileRef != kPGPInvalidRef )
		PFLFreeFileSpec( fileRef );
	if ( IsntNull( stdFile ) )
		pgpStdIOClose( stdFile );
	if ( weOpenedMainFile && IsntNull( mainStdFile ) )
		pgpStdIOClose( mainStdFile );
	return err;
}

/* This struct is only used by PurgeOldBackups() */
typedef struct
{
	PFLFileSpecRef	fileRef;
	time_t			modTime;
} PGPFileTimeInfo;

	static PGPError
PurgeOldBackups(
	PFLFileSpecRef		mainFileRef )
{
	PGPError			err = kPGPError_NoErr;
	PFLContextRef		context = PFLGetFileSpecContext( mainFileRef );
	const char *		separator;
	PGPSize				maxFileLength;
	char *				mainFileName = NULL;
	PFLFileInfo			mainFileInfo;
	PFLDirectoryIterRef	dirIter = kPGPInvalidRef;
	PFLFileSpecRef		fileRef = kPGPInvalidRef;
	char *				fileName = NULL;
	PFLFileInfo			fileInfo;
	PGPFileTimeInfo		newestFiles[4];
	int					maxNewest = sizeof(newestFiles) /
									sizeof(newestFiles[0]);
	PGPFileTimeInfo		olderFiles[2][2];
	long				olderFileAges[2] = { 60 * 60L, 24 * 60 * 60L };
	int					maxOlder = sizeof(olderFiles) / sizeof(olderFiles[0]);
	PFLFileSpecRef		filesToDelete[32];
	int					maxFilesToDelete = sizeof(filesToDelete) /
										   sizeof(filesToDelete[0]);
	int					numFilesToDelete = 0;
	int					i;
	time_t				now = time( NULL );
	PGPBoolean			valid;
	PGPBoolean			deleteThisFile;
	PGPUInt32			number;

	pgpAssert( maxOlder == sizeof(olderFileAges) / sizeof(olderFileAges[0]) );

	for (i = 0; i < maxNewest; i++)
		newestFiles[i].fileRef = kPGPInvalidRef;
	for (i = 0; i < maxOlder; i++)
		olderFiles[i][0].fileRef = olderFiles[i][1].fileRef = kPGPInvalidRef;
	
	err = PFLGetFileSpecName( mainFileRef, &mainFileName );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = PFLGetMaxFileSpecNameLength( mainFileRef, &maxFileLength );
	if ( IsPGPError( err ) )
		goto cleanup;

	separator = GetSeparator( maxFileLength );

	err = PFLGetFileInfo( mainFileRef, &mainFileInfo );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = PFLGetParentDirectory( mainFileRef, &fileRef );
	if ( IsPGPError( err ) )
		goto cleanup;

	err = PFLNewDirectoryIter( fileRef, &dirIter );
	if ( IsPGPError( err ) )
		goto cleanup;

	PFLFreeFileSpec( fileRef );
	fileRef = NULL;

	while ( IsntPGPError( err = PFLNextFileInDirectory( dirIter, &fileRef ) ) )
	{
		err = PFLGetFileSpecName( fileRef, &fileName );
		if ( IsPGPError( err ) )
			goto cleanup;

		err = PGPVerifyNumberFileName( mainFileName, separator, fileName,
					maxFileLength + 1, &valid, &number );
		if ( IsPGPError( err ) )
			goto cleanup;

		if ( valid )
		{
			err = PFLGetFileInfo( fileRef, &fileInfo );
			if ( IsPGPError( err ) )
				goto cleanup;

			deleteThisFile = TRUE;

			if ( fileInfo.modificationTime <= now )
			{
				i = 0;
				while ( i < maxNewest &&
						newestFiles[i].fileRef != kPGPInvalidRef &&
						newestFiles[i].modTime > fileInfo.modificationTime )
					i++;
				if ( i < maxNewest )
				{
					if ( newestFiles[ maxNewest - 1 ].fileRef
							!= kPGPInvalidRef )
					{
						if ( numFilesToDelete >= maxFilesToDelete )
							goto overflow;
						filesToDelete[ numFilesToDelete++ ] =
								newestFiles[ maxNewest - 1 ].fileRef;
					}
					pgpCopyMemory( newestFiles + i, newestFiles + i + 1,
								   ( maxNewest - i - 1 )
									   * sizeof( newestFiles[0] ) );

					newestFiles[i].modTime = fileInfo.modificationTime;
					err = PFLCopyFileSpec( fileRef, 
							&newestFiles[ i ].fileRef );
					if ( IsPGPError( err ) )
						goto cleanup;
					deleteThisFile = FALSE;
				}
			}
			if ( deleteThisFile )
			{
				if ( numFilesToDelete >= maxFilesToDelete )
					goto overflow;
				err = PFLCopyFileSpec( fileRef,
						&filesToDelete[ numFilesToDelete ] );
				if ( IsPGPError( err ) )
					goto cleanup;
				numFilesToDelete++;
			}
		}

		if ( fileRef != kPGPInvalidRef )
		{
			PFLFreeFileSpec( fileRef );
			fileRef = NULL;
		}

		PFLContextMemFree( context, fileName );
		fileName = NULL;
	}
	if ( err != kPGPError_EndOfIteration )
		goto cleanup;

overflow:	/* Overflowing is currently ignored, we'll delete the rest later */
	err = kPGPError_NoErr;
	goto cleanup;

cleanup:
	if ( fileRef != kPGPInvalidRef )
		PFLFreeFileSpec( fileRef );
	if ( dirIter != kPGPInvalidRef )
		PFLFreeDirectoryIter( dirIter );
	if ( IsntNull( mainFileName ) )
		PFLContextMemFree( context, mainFileName );
	if ( IsntNull( fileName ) )
		PFLContextMemFree( context, fileName );
	for ( i = 0; i < maxNewest; i++ )
		if ( newestFiles[i].fileRef != kPGPInvalidRef )
			PFLFreeFileSpec( newestFiles[i].fileRef );
	for ( i = 0; i < numFilesToDelete; i++ )
	{
		if ( filesToDelete[i] != kPGPInvalidRef )
		{
			PFLFileSpecDelete( filesToDelete[i] );
			PFLFreeFileSpec( filesToDelete[i] );
		}
	}
	return err;
}

/* 
 * Open the specified ringfile.  If writeable flag is clear, it will be
 * forced read-only, otherwise whether it is writeable will depend on
 * permissions.  Returns a filled-in rdb structure.
 * Note that "private" is not a concept of the underlying library, but
 * is used in the add function to decide which objects to accept.
 */
	static PGPError
OpenKeyRing (PFLConstFileSpecRef fileRef, PGPKeyRingOpenFlags openFlags,
	RingPool *ringpool, RingDB *rdb)
{
	PFLFileSpecRef	mainFileRef = kPGPInvalidRef;	/* Main ring */
	PFLFileSpecRef	readFileRef = kPGPInvalidRef;	/* Latest backup */
	FILE *			mainStdFile	= NULL;	/* Stdio file of main ring */
	FILE *			readStdFile	= NULL;	/* Stdio file of latest backup */
	PGPFile *		pgpFile		= NULL;
	RingFile *		ringFile	= NULL;
	RingSet const *	immutableSet = NULL;
	RingSet *		mutableSet	= NULL;
	PGPError		err			= kPGPError_NoErr;
	PGPContextRef	cdkContext	= pgpGetFileRefContext( fileRef );

	PGPBoolean	trusted		= !!( openFlags & kPGPKeyRingOpenFlags_Trusted );
	PGPBoolean	priv		= !!( openFlags & kPGPKeyRingOpenFlags_Private );
	PGPBoolean	writeable	= !!( openFlags & kPGPKeyRingOpenFlags_Mutable );
	PGPFileType	fileType = priv ? kPGPFileTypePrivRing : kPGPFileTypePubRing;

	err = PFLCopyFileSpec(fileRef, &mainFileRef);

	if ( writeable )
	{
		PGPFileOpenFlags	fileOpenFlags;
		
		mutableSet = ringSetCreate (ringpool);
		if ( IsNull( mutableSet ) )
		{
			err = kPGPError_OutOfMemory;
			goto cleanup;
		}

		/* Lock the main keyring file to ensure exclusive access */
		fileOpenFlags = kPGPFileOpenReadWritePerm | kPGPFileOpenLockFile;

		err = pgpFileRefStdIOOpen( mainFileRef, fileOpenFlags, fileType,
								   &mainStdFile );

		if ( IsPGPError( err ) )
		{
			if ( err == kPGPError_FileNotFound &&
				 ( openFlags & kPGPKeyRingOpenFlags_Create ) )
			{
				err = pgpCreateFile( mainFileRef, fileType );
				if ( IsPGPError( err ) )
					goto cleanup;

				err = pgpFileRefStdIOOpen( mainFileRef, fileOpenFlags,
										   fileType, &mainStdFile );
			}
			if ( IsPGPError( err ) )
				goto cleanup;
		}
	}
	else
	{
		/* Immutable keydb's don't need mutableSet */
		mutableSet = NULL;
	}

	err = FindMatchingBackup( mainFileRef, &readFileRef );
	if ( IsPGPError( err ) )
	{
		err = MakeMatchingBackup( mainFileRef, mainStdFile, fileType,
								  &readFileRef );
		if ( err == kPGPError_FileLocked )
			goto cleanup;
	}

	if ( IsPGPError( err ) || ( !writeable && PFL_PLATFORM_UNSAFE_DELETE ) )
	{
		/*
		 * If we failed to create a matching backup file, or
		 * if deletes aren't safe on this platform and we're not a writer,
		 * then we need to make a local copy of the keyring as backing store.
		 */
		err = MakeTempCopy( mainFileRef, mainStdFile, fileType, &readFileRef );
		if ( IsPGPError( err ) )
			goto cleanup;
		rdb->btempfile = TRUE;
	}
	else
	{
		rdb->btempfile = FALSE;
	}

	/* Open the read-only file which will be used for backing store */
	err = pgpFileRefStdIOOpen( readFileRef, kPGPFileOpenReadPerm,
							   kPGPFileTypeNone, &readStdFile );
	if ( IsPGPError( err ) )
		goto cleanup;

	pgpFile = pgpFileReadOpen ( cdkContext, readStdFile, NULL, NULL );
	if ( IsNull( pgpFile ) )
	{
		err = kPGPError_OutOfMemory;	/* XXX: Return better error */
		goto cleanup;
	}
	readStdFile = NULL;		/* pgpFile is now responsible for closing it */

	ringFile = ringFileOpen( ringpool, pgpFile, trusted, &err );
	if ( IsNull( ringFile ) )
		goto cleanup;

	immutableSet = ringFileSet( ringFile );
	if ( IsntNull( mutableSet ) )
		ringSetAddSet( mutableSet, immutableSet );

	rdb->mainFileRef	= mainFileRef;
	rdb->mainStdFile	= mainStdFile;
	rdb->readFileRef	= readFileRef;
	rdb->pgpFile		= pgpFile;
	rdb->ringFile		= ringFile;
	rdb->immutableSet	= immutableSet;
	rdb->mutableSet		= mutableSet;
	rdb->openFlags		= openFlags;
	rdb->bwriteable		= writeable;
	rdb->bdirty			= ringFileIsDirty( ringFile );
	rdb->btrusted		= trusted;
	rdb->bprivate		= priv;

	return kPGPError_NoErr;

cleanup:
	if ( IsntNull( mutableSet ) )
		ringSetDestroy( mutableSet );
	if ( IsntNull( ringFile ) )
		ringFileClose( ringFile );
	if ( IsntNull( pgpFile ) )
		pgpFileClose( pgpFile );
	if ( IsntNull( readStdFile ) )
		pgpStdIOClose( readStdFile );
	if ( readFileRef != kPGPInvalidRef )
		PFLFreeFileSpec( readFileRef );
	if ( IsntNull( mainStdFile ) )
		pgpStdIOClose( mainStdFile );
	if ( mainFileRef != kPGPInvalidRef )
		PFLFreeFileSpec( mainFileRef );
	return err;
}

	static PGPError
WriteKeyRing( RingDB *	rdb )
{
	PGPContextRef	cdkContext	= rdb->context;
	PFLContextRef	pflContext	= pgpContextGetPFLContext( cdkContext );
	PGPError		err			= kPGPError_NoErr;

	PGPFile *		oldPGPFile	= rdb->pgpFile;
	RingFile *		oldRingFile	= rdb->ringFile;

	PFLFileSpecRef	tempFileRef	= kPGPInvalidRef;
	FILE *			tempStdFile	= NULL;		/* Stdio file for temp file */
	PGPFile *		tempPGPFile	= NULL;		/* PGPFile for temp file */

	PFLFileSpecRef	newFileRef	= kPGPInvalidRef;
	FILE *			newStdFile	= NULL;		/* Stdio file for new backup */
	PGPFile *		newPGPFile	= NULL;		/* PGPFile for new backup file */
	RingFile *		newRingFile	= NULL;		/* RingFile for new backup file */

	char *			mainFileName = NULL;
	PGPFileType		fileType = rdb->bprivate ? kPGPFileTypePrivRing
											 : kPGPFileTypePubRing;
	int             flags	= ( rdb->btrusted && !rdb->bprivate )
									? PGP_WRITETRUST_PUB : 0;


	/* Find a unique temporary file name */
	err = PFLFileSpecGetUniqueSpec( rdb->mainFileRef, &tempFileRef );
	if ( IsPGPError( err ) )
		goto cleanup;

	/* Create and open the temp file */
	err = pgpFileRefStdIOOpen( tempFileRef,
							   kPGPFileOpenStdUpdateFlags
								   | kPGPFileOpenLockFile,
							   fileType,
							   &tempStdFile );
	if ( IsPGPError( err ) )
		goto cleanup;

	/* Create a PGPFile object for the temp file */
	tempPGPFile = pgpFileWriteOpen( cdkContext, tempStdFile, NULL );
	if ( IsNull( tempPGPFile ) )
	{
		pgpStdIOClose( tempStdFile );
		err = kPGPError_OutOfMemory;	/* XXX: Return better error */
		goto cleanup;
	}

	/* Write the key ring to the temp file, creating newRingFile */
	err = ringSetWrite( rdb->mutableSet, tempPGPFile, &newRingFile,
						PGPVERSION_3, flags );
	if ( IsPGPError( err ) )
		goto cleanup;

	/* Make sure the data is written to disk */
	err = pgpFileFlush( tempPGPFile );
	if ( IsPGPError( err ) )
		goto cleanup;


	/*
	 * Make a new read-only backup from the newly written temp file
	 */
	err = MakeMatchingBackup( rdb->mainFileRef, tempStdFile, fileType,
							  &newFileRef );
	if ( IsPGPError( err ) )
		goto cleanup;

	/* Open the new backup file which will be used for backing store */
	err = pgpFileRefStdIOOpen( newFileRef, kPGPFileOpenReadPerm,
							   kPGPFileTypeNone, &newStdFile );
	if ( IsPGPError( err ) )
		goto cleanup;

	/* Create a PGPFile object for the new backup file */
	newPGPFile = pgpFileReadOpen( cdkContext, newStdFile, NULL, NULL );
	if ( IsNull( newPGPFile ) )
	{
		pgpStdIOClose( newStdFile );
		err = kPGPError_OutOfMemory;	/* XXX: Return better error */
		goto cleanup;
	}

	/*
	 * Switch the ringfile's backing store
	 * from the temp file to the new backup file.
	 */
	ringFileSwitchFile( newRingFile, newPGPFile );

	pgpFileClose( tempPGPFile );
	tempPGPFile = NULL;


	/*
	 * Delete the main ring file and move the temp file into its place.
	 *
	 * XXX: Race condition here, we must release the lock momentarily
	 */
	{
		/* We'll need this for renaming */
		err = PFLGetFileSpecName( rdb->mainFileRef, &mainFileName );
		if ( IsPGPError( err ) )
			goto cleanup;

		/* Close the main ring file, releasing our lock */
		if ( IsntNull( rdb->mainStdFile ) )
		{
			err = pgpStdIOClose( rdb->mainStdFile );
			if ( IsPGPError( err ) )
				goto cleanup;
			rdb->mainStdFile = NULL;
		}

		/* Try to delete the main ring file, ignoring errors */
		(void)PFLFileSpecDelete( rdb->mainFileRef );

		/* Move the temp file into the main ring's place */
		err = PFLFileSpecRename( tempFileRef, mainFileName );
		if ( IsPGPError( err ) )
			goto cleanup;

		/* Relock the main keyring file to ensure exclusive access */
		err = pgpFileRefStdIOOpen( rdb->mainFileRef,
								   kPGPFileOpenReadWritePerm |
									   kPGPFileOpenLockFile,
								   fileType,
								   &rdb->mainStdFile );
		if ( IsPGPError( err ) )
			goto cleanup;

		/* XXX: We should swap fileIDs here on the Mac */
	}


	/* We must close mutableSet in order to cleanly close oldRingFile */
	ringSetDestroy( rdb->mutableSet );
	rdb->mutableSet = NULL;

	/* Now close the old backup or temp file */
	err = ringFileClose( oldRingFile );
	pgpFileClose( oldPGPFile );		/* Close the PGPFile regardless */
	if ( IsPGPError( err ) )
	{
		/*
		 * We shouldn't get here.  Probably due to a pesky RingSet leak!
		 */
		pgpDebugMsg( "Failed to close RingSet" );
	}

	/* If it was a local temp file, delete it immediately */
	if ( rdb->btempfile )
	{
		PFLFileSpecDelete( rdb->readFileRef );
		rdb->btempfile = FALSE;
	}

	PFLContextMemFree( pflContext, mainFileName );

	PFLFreeFileSpec( rdb->readFileRef );
	rdb->readFileRef = newFileRef;
	newFileRef = NULL;

	PFLFreeFileSpec( tempFileRef );
	tempFileRef = NULL;

	rdb->ringFile = newRingFile;
	rdb->pgpFile = newPGPFile;

	PurgeOldBackups( rdb->mainFileRef );

	return kPGPError_NoErr;

cleanup:
	if ( IsNull( rdb->mainStdFile ) )
	{
		/* If we lost our exclusive lock, we must forfeit write access */
		rdb->bwriteable = FALSE;
		rdb->openFlags &= ~kPGPKeyRingOpenFlags_Mutable;
	}

	if ( IsntNull( newRingFile ) )
		ringFileClose( newRingFile );
	if ( IsntNull( newPGPFile ) )
		pgpFileClose( newPGPFile );
	if ( newFileRef != kPGPInvalidRef )
		PFLFreeFileSpec( newFileRef );

	if ( IsntNull( tempPGPFile ) )
		pgpFileClose( tempPGPFile );
	if ( IsntNull( tempStdFile ) )
		PFLFileSpecDelete( tempFileRef );
	if ( tempFileRef != kPGPInvalidRef )
		PFLFreeFileSpec( tempFileRef );

	if ( IsntNull( mainFileName ) )
		PFLContextMemFree( pflContext, mainFileName );

	return err;
}

	static PGPError
CloseKeyRing( RingDB *	rdb )
{
	PGPContextRef	context;

	pgpAssertAddrValid( rdb, RingDB );
	context	= rdb->context;

	if (rdb->frozenSet)
	{
		ringSetDestroy (rdb->frozenSet);
		rdb->frozenSet = 0;
	}
	rdb->immutableSet = 0;
	if (rdb->mutableSet)
		ringSetDestroy (rdb->mutableSet);
	rdb->mutableSet = 0;
	ringFileClose (rdb->ringFile);
	rdb->ringFile = 0;
	pgpFileClose (rdb->pgpFile);
	rdb->pgpFile = 0;

	if ( IsntNull( rdb->mainStdFile ) )
		pgpStdIOClose( rdb->mainStdFile );

	if ( rdb->mainFileRef != kPGPInvalidRef )
	{
		if ( rdb->bwriteable || !PFL_PLATFORM_UNSAFE_DELETE )
			PurgeOldBackups( rdb->mainFileRef );
		PFLFreeFileSpec( rdb->mainFileRef );
		rdb->mainFileRef = kPGPInvalidRef;
	}
	if ( rdb->readFileRef != kPGPInvalidRef )
	{
		if ( rdb->btempfile )
			PFLFileSpecDelete( rdb->readFileRef );
		PFLFreeFileSpec( rdb->readFileRef );
		rdb->readFileRef = kPGPInvalidRef;
	}
	
	return kPGPError_NoErr;
}




/********************  Virtual Functions  ************************/


	static PGPBoolean
rdbIsMutable (PGPKeyDBRef kdb) {
	RingDB         *rdb = (RingDB *)kdb->priv;

	return rdb->bwriteable;
}

	static PGPBoolean
rdbObjIsMutable (PGPKeyDBRef kdb, RingObject *testObj)
{	
	RingDB         *rdb = (RingDB *)kdb->priv;

	return rdb->bwriteable && ringSetIsMember (rdb->mutableSet, testObj);
}

	static PGPBoolean
rdbIsDirty (PGPKeyDBRef kdb)
{
	RingDB         *rdb = (RingDB *)kdb->priv;

	return rdb->bdirty;
}

/* Mark as dirty */
	static void
rdbDirty (RingDB *rdb)
{
	rdb->bdirty = TRUE;
	if (rdb->frozenSet) {
		ringSetDestroy (rdb->frozenSet);
		rdb->frozenSet = 0;
	}
}

	static const RingSet *
rdbGetRingSet (PGPKeyDBRef kdb)
{
	RingDB         *rdb = (RingDB *)kdb->priv;
	RingSet        *rset;

	if (!rdb->bwriteable) {
		return rdb->immutableSet;
	} else if (!(rset = rdb->frozenSet)) {
		rset = ringSetCreate (ringSetPool (rdb->mutableSet));
		ringSetAddSet (rset, rdb->mutableSet);
		ringSetFreeze (rset);
		rdb->frozenSet = rset;
	}
	return rset;
}

	static PGPError
rdbAdd (PGPKeyDBRef kdb, RingSet *toAdd)
{
	RingDB         *rdb = (RingDB *)kdb->priv;
	RingIterator   *riAdd;		/* Iterator over toAdd set */
	RingObject     *robj;		/* Object we are adding */
	int             level;		/* Level of iterator in hierarchy */
	int				type;		/* Type of robj */
	int             added;		/* Number of objects added */
	int             skipped;	/* Number of objects skipped */
	PGPBoolean		skipparts;  /* True if skipping, obj not for us */

	if (!rdb->bwriteable)
		return kPGPError_ItemIsReadOnly;

	riAdd = ringIterCreate (toAdd);
	if (!riAdd) {
		return ringSetError(toAdd)->error;
	}

	added = 0;
	skipped = 0;
	skipparts = FALSE;

	while ((level = ringIterNextObjectAnywhere(riAdd)) > 0) {
		robj = ringIterCurrentObject (riAdd, level);
		type = ringObjectType (robj);
		if (type==RINGTYPE_KEY) {
			/*
			 * If key lacks secret object, put it only on the public
			 * ring, and if key comes only from a source where it is a
			 * secret key, put it only on the secret ring.  This is
			 * important to prevent propagation of the 2.6.2 "version bug",
			 * where changing passphrase on a V2 secret key turns it into
			 * V2_6.  We must not let that get into the public keyring.
			 */
			if (rdb->bprivate) {
				skipparts = !ringKeyIsSec (toAdd, robj);
			} else {
				skipparts = ringKeyIsSecOnly (toAdd, robj);
				/* Go ahead and add to pubring if key has a signature */
				if (skipparts) {
					unsigned levelx;
					RingObject *robjx;
					int typex;
					while ((levelx = ringIterNextObjectAnywhere(riAdd)) > 0) {
						robjx = ringIterCurrentObject (riAdd, levelx);
						typex = ringObjectType (robjx);
						if (typex == RINGTYPE_KEY)
							break;
						if (typex == RINGTYPE_SIG) {
							skipparts = FALSE;
							break;
						}
					}
					ringIterSeekTo (riAdd, robj);
				}
			}
		}
		if (skipparts) {
			skipped += 1;
		} else if ((type==RINGTYPE_SEC) && !rdb->bprivate) {
			/* Secrets only go to private keyrings */
			skipped += 1;
		} else if ((type==RINGTYPE_SIG) && rdb->bprivate &&
				   !ringSigIsSelfSig( toAdd, robj )) {
			/*
			 * Signatures only go to public keyrings, normally.  The
			 * exception is your own self-sigs.  If you lose the one
			 * on your subkey, people won't be able to encrypt to you.
			 * If you lose the one on your username, you'll lose expiration
			 * or preferred algorithms.
			 */
			skipped += 1;
		} else {
			ringSetAddObject (rdb->mutableSet, robj);
			added += 1;
		}
	}
	ringIterDestroy (riAdd);

	rdbDirty (rdb);
	return kPGPError_NoErr;
}


	static PGPError
rdbRemove (PGPKeyDBRef kdb, RingObject *toRemove)
{
	RingDB         *rdb = (RingDB *)kdb->priv;

	if (!rdb->bwriteable)
		return kPGPError_ItemIsReadOnly;

	if (ringSetIsMember (rdb->mutableSet, toRemove))
		rdbDirty (rdb);
	return (PGPError)ringSetRemObject (rdb->mutableSet, toRemove);
}

/*
 * Note that unions don't pass this call down, they take care of it
 * themselves
 */
	static PGPError
rdbChanged (PGPKeyDBRef kdb, RingSet *changedkeys)
{
	RingDB         *rdb = (RingDB *)kdb->priv;

	if (!rdb->bwriteable)
		return kPGPError_ItemIsReadOnly;

	rdbDirty (rdb);
	return pgpReSortKeys (kdb, changedkeys);
}

	static PGPError
rdbCommit (PGPKeyDBRef kdb)
{
	RingDB         *rdb = (RingDB *)kdb->priv;
	RingPool	   *ringpool = ringSetPool (rdb->immutableSet);
	PGPError        error;

	if (!rdb->bwriteable)
		return kPGPError_NoErr;

	/*  Don't check for trust changed if untrusted file */
	if (!rdb->bdirty &&	(!rdb->btrusted || 
				 (rdb->btrusted && !ringFileIsTrustChanged(rdb->ringFile))))
		return kPGPError_NoErr;

	/* Must use frozen set for writing */

	ringSetFreeze (rdb->mutableSet);
	if (rdb->frozenSet) {
		ringSetDestroy (rdb->frozenSet);
		rdb->frozenSet = 0;
	}
	error = WriteKeyRing (rdb);

	if (error) {
		/*
		 * Sometimes on error we have fixed rdb->mutableSet, but possibly it
		 * may still be frozen as above.  To be safe we will always make a
		 * copy here so it is writeable in the future.
		 */
		RingSet *rset = ringSetCreate (ringpool);
		pgpAssert (rset);
		pgpAssert (rdb->mutableSet);
		ringSetAddSet (rset, rdb->mutableSet);
		ringSetDestroy (rdb->mutableSet);
		rdb->mutableSet = rset;
		return error;
	}

	rdb->immutableSet = ringFileSet (rdb->ringFile);
	rdb->mutableSet = ringSetCreate (ringpool);
	ringSetAddSet (rdb->mutableSet, rdb->immutableSet);
	rdb->bdirty = 0;

	return kPGPError_NoErr;
}


	static PGPError
rdbRevert (PGPKeyDBRef kdb)
{
	RingDB         *rdb = (RingDB *)kdb->priv;

	if (!rdb->bdirty)
		return kPGPError_NoErr;
	rdb->bdirty = 0;
	if (rdb->mutableSet) {
		ringSetDestroy (rdb->mutableSet);
		rdb->mutableSet = ringSetCreate (ringSetPool(rdb->immutableSet));
		if (!rdb->mutableSet) {
			return ringSetError( rdb->immutableSet )->error;
		}
		ringSetAddSet (rdb->mutableSet, rdb->immutableSet);
	}
	if (rdb->frozenSet) {
		ringSetDestroy (rdb->frozenSet);
		rdb->frozenSet = 0;
	}
	return kPGPError_NoErr;
}

	static PGPError
rdbReload (PGPKeyDBRef kdb)
{
	RingDB         *rdb = (RingDB *)kdb->priv;
	PFLFileSpecRef     	fileRef;
	PGPError        err	= kPGPError_NoErr;

	if (rdb->bmembuf)
		return kPGPError_NoErr;
	if ( IsPGPError(err = PFLCopyFileSpec(rdb->mainFileRef, &fileRef)))
		return kPGPError_OutOfMemory;
	err = CloseKeyRing (rdb);
	if (err) {
		PFLFreeFileSpec(fileRef);
		return err;
	}
	err = OpenKeyRing (fileRef, rdb->openFlags,
					   ringSetPool (rdb->immutableSet), rdb);
	rdb->bdirty = 0;
	if (rdb->frozenSet) {
		ringSetDestroy (rdb->frozenSet);
		rdb->frozenSet = 0;
	}
	PFLFreeFileSpec(fileRef);
	return err;
}

	static void
rdbDestroy (PGPKeyDBRef kdb)
{
	RingDB         *rdb = (RingDB *)kdb->priv;
	PGPContextRef	context;

	pgpAssertAddrValid( kdb, RingDB );
	context	= rdb->context;

	CloseKeyRing (rdb);
	kdb->typeMagic	= ~ kdb->typeMagic;		/* mark as invalid */
	kdb->fixedMagic	= ~ kdb->fixedMagic;	/* mark as invalid */
	pgpContextMemFree( context, rdb);
}


/****************************  Constructor  **************************/

	PGPKeyDBRef 
pgpCreateFileKeyDB (PGPContextRef context, PFLConstFileSpecRef fileRef,
	PGPKeyRingOpenFlags openFlags, RingPool *ringpool,
	PGPError *error)
{
	PGPKeyDBRef    kdb;
	RingDB         *rdb;

	*error = kPGPError_NoErr;
	kdb = pgpKeyDBCreateInternal (context);
	if (!kdb) {
		*error = kPGPError_OutOfMemory;
		return NULL;
	}
	rdb = (RingDB *) pgpContextMemAlloc( context,
		sizeof (RingDB), kPGPMemoryFlags_Clear);
	if (!rdb) {
		*error = kPGPError_OutOfMemory;
		pgpKeyDBDestroyInternal (kdb);
		return NULL;
	}

	rdb->context	= context;
	
	*error = OpenKeyRing (fileRef, openFlags, ringpool, rdb);
	if (*error) {
		pgpContextMemFree( context, rdb);
		pgpKeyDBDestroyInternal (kdb);
		return NULL;
	}

	kdb->priv    		= rdb;
	kdb->typeMagic			= PGPKDBFILEMAGIC;
	kdb->fixedMagic			= kPGPKeyDBMagic;
	kdb->isMutable			= rdbIsMutable;
	kdb->objIsMutable		= rdbObjIsMutable;
	kdb->isDirty    		= rdbIsDirty;
	kdb->getRingSet			= rdbGetRingSet;
	kdb->add        		= rdbAdd;
	kdb->remove     		= rdbRemove;
	kdb->changed			= rdbChanged;
	kdb->commit     		= rdbCommit;
	kdb->revert     		= rdbRevert;
	kdb->reload     		= rdbReload;
	kdb->destroy			= rdbDestroy;

	pgpKeyDBInitInternal(kdb);

	return kdb;
}

/*
 * Create a File type KeyDB from a memory buffer.  This will be an
 * immutable KeyDB since we can't save our changes anywhere.
 * We make a copy of the buffer passed in, so caller can free it.
 */
	PGPKeyDBRef 
pgpCreateMemFileKeyDB (
	PGPContextRef	context,
	PGPByte *		buf,
	size_t 			length,
	RingPool *		ringpool,
	PGPError *		error)
{
	PGPKeyDBRef		kdb;
	RingDB         *rdb;

	*error = kPGPError_NoErr;
	kdb = pgpKeyDBCreateInternal (context);
	if (!kdb) {
		*error = kPGPError_OutOfMemory;
		return NULL;
	}
	rdb = (RingDB *) pgpContextMemAlloc( context,
		sizeof (RingDB), kPGPMemoryFlags_Clear);
	if (!rdb) {
		*error = kPGPError_OutOfMemory;
		pgpKeyDBDestroyInternal (kdb);
		return NULL;
	}

	rdb->context	= context;

	/* Memfile keydb's are immutable */
	rdb->mutableSet = NULL;

	rdb->pgpFile = pgpFileMemOpen ( context, buf, length);
	if (!rdb->pgpFile) {
		pgpContextMemFree( context, rdb);
		pgpKeyDBDestroyInternal (kdb);
		*error = kPGPError_OutOfMemory;
		return NULL;
	}

	rdb->ringFile = ringFileOpen (ringpool, rdb->pgpFile, 0, error);
	if (!rdb->ringFile) {
		pgpFileClose (rdb->pgpFile);
		pgpContextMemFree( context, rdb);
		pgpKeyDBDestroyInternal (kdb);
		*error	= ringPoolError(ringpool)->error;
		if (!*error)
			*error = kPGPError_OutOfMemory;
		return NULL;
	}

	rdb->immutableSet = ringFileSet (rdb->ringFile);
	rdb->bmembuf = TRUE;
	rdb->mainStdFile = NULL;
	rdb->mainFileRef = rdb->readFileRef = NULL;

	kdb->priv    			= rdb;
	kdb->typeMagic			= PGPKDBFILEMAGIC;
	kdb->fixedMagic			= kPGPKeyDBMagic;
	kdb->isMutable			= rdbIsMutable;
	kdb->objIsMutable		= rdbObjIsMutable;
	kdb->isDirty    		= rdbIsDirty;
	kdb->getRingSet			= rdbGetRingSet;
	kdb->add        		= rdbAdd;
	kdb->remove     		= rdbRemove;
	kdb->changed			= rdbChanged;
	kdb->commit     		= rdbCommit;
	kdb->revert     		= rdbRevert;
	kdb->reload     		= rdbReload;
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