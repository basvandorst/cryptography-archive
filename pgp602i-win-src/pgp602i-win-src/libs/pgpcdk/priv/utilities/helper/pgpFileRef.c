/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpFileRef.c,v 1.73.4.1.2.1 1998/11/12 03:23:18 heller Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_PATHS_H
#include <paths.h>
#endif

#if PGP_UNIX || PGP_WIN32
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#endif

#if __MWERKS__
#define HAVE_SHARE_H	0
#else
#define HAVE_SHARE_H	0
#endif

#if PGP_WIN32
#include <windows.h>
#include <io.h>
#if HAVE_SHARE_H
#include <share.h>
#endif
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

#if PGP_MACINTOSH
#include <unix.h>
#include <Errors.h>
#include <Folders.h>
#include <Script.h>
#endif

#include "pgpFileRef.h"
#include "pgpFileNames.h"
#include "pgpFileUtilities.h"
#include "pgpErrors.h"
#include "pgpTypes.h"
#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpContext.h"

#include "pgpFileSpec.h"


#if PGP_MACINTOSH

#include "MacFiles.h"
#include "MacStrings.h"
#include "pgpMacFile.h"
#include "pgpMacUtils.h"

#endif


#if !defined(_PATH_TMP) && PGP_UNIX && HAVE_MKSTEMP
#define _PATH_TMP	"/tmp/"
#endif

#if HAVE_MKSTEMP && defined(sparc)
extern int	mkstemp (char *__template);
#endif

#define pgpaPGPFileTypeValid(fileType)								\
		pgpaAssert(fileType > kPGPFileTypeNone						\
				&& fileType < kPGPFileTypeNumTypes)

/* These must be kept in sync with the PGPFileType decl in pgpFileRef.h */

#if PGP_UNIX	/* [ */

static struct UnixFileTypeEntry
{
	mode_t	mask;	/* umask value for permissions */
} sUnixFileTypeTable[kPGPFileTypeNumTypes] = {
	{ 0 },		/* kPGPFileTypeNone */
	{ 0077 },	/* kPGPFileTypePrefs */
	{ 0077 },	/* kPGPFileTypePubRing */
	{ 0077 },	/* kPGPFileTypePrivRing */
	{ 0077 },	/* kPGPFileTypeDetachedSig */
	{ 0077 },	/* kPGPFileTypeRandomSeed */
	{ 0077 },	/* kPGPFileTypeExportedKeys */
	{ 0077 },	/* kPGPFileTypeArmorFile */
	{ 0077 },	/* kPGPFileTypeEncryptedData */
	{ 0077 },	/* kPGPFileTypeGroups */
	{ 0077 },	/* kPGPFileTypeDecryptedText */
	{ 0077 },	/* kPGPFileTypeDecryptedBin */
	{ 0077 }	/* kPGPFileTypeSignedData */
};

#endif	/* ] PGP_UNIX */




	PGPMemoryMgrRef
pgpGetFileRefMemoryMgr( PFLConstFileSpecRef fileRef )
{
	return( 
		PFLGetFileSpecMemoryMgr( (PFLConstFileSpecRef)fileRef ) );
}



/*
 * pgpGetFileRefName returns a string allocated with pgpContextMemAlloc(),
 * which the caller is responsible for freeing.
 */
	char *
pgpGetFileRefName( PFLConstFileSpecRef	fileRef)
{
	char *	name	= NULL;
	PGPError	err	= kPGPError_NoErr;
	
	err	= PFLGetFileSpecName( fileRef, &name );
	
	return( name );
}


	PGPError
pgpSetFileRefName(
	PFLFileSpecRef	fileRef,
	char const *	newName)
{
	return( PFLSetFileSpecName( fileRef, newName ) );
}

	uchar *
pgpExportFileRef(
	PFLConstFileSpecRef		fileRef,
	size_t *				size)
{
	PGPError	err	= kPGPError_NoErr;
	PGPByte *	data	= NULL;
	PGPSize		dataSize;
	
	err	= PFLExportFileSpec( fileRef, &data, &dataSize );
	*size	= dataSize;
	
	return( data );
}

	PGPError
pgpImportFileRef(
	PGPContextRef		context,
	uchar const *		buffer,
	size_t				size,
	PFLFileSpecRef *		outRef )
{
	PGPError	err	= kPGPError_NoErr;
	
	err	= PFLImportFileSpec( PGPGetContextMemoryMgr( context ),
				buffer, size, outRef );
	
	return( err );
}


#if PGP_MACINTOSH	/* [ */

	PGPError
pgpCalcFileSize(
	PFLConstFileSpecRef		fileRef,
	PGPFileOpenFlags	flags,
	size_t *			fileSize)
{
	return pgpMacCalcFileSize(fileRef, flags, fileSize);
}

	PGPError
pgpCreateFile(
	PFLConstFileSpecRef		fileRef,
	PGPFileType			fileType)
{
	PGPError	err = kPGPError_NoErr;
	FSSpec		spec;
	OSErr		macErr;
	
	PFLValidateFileSpec( fileRef );
	
	err	= PFLGetFSSpecFromFileSpec(fileRef, &spec);
	if ( IsntPGPError( err ) )
	{
		macErr = FSpCreate( &spec,
					pgpGetMacFileCreatorFromPGPFileType( fileType ),
					pgpGetMacFileTypeFromPGPFileType( fileType ),
					smSystemScript );
		if (macErr != noErr)
			err = pgpErrorFromMacError(macErr, kPGPError_CantOpenFile);
	}
	return err;
}


	FILE *
pgpStdIOOpenTempFile(
	PGPContextRef		context,
	PFLFileSpecRef *	tempFileRef,
	PGPError *			errorCode)
{
	FSSpec		spec;
	FILE *		stdIOFile = NULL;
	PGPError	err = kPGPError_NoErr;
	OSErr		macErr;
	long		i;
	
	pgpa(pgpaAddrValid(tempFileRef, PFLFileSpecRef));
	
	*tempFileRef = NULL;
	if ((macErr = FindFolder(kOnSystemDisk, kTemporaryFolderType,
								kCreateFolder, &spec.vRefNum,
								&spec.parID)) != noErr)
		goto macError;
	
	for (i = 0; ; i++)
	{
		pgpFormatPStr(spec.name, sizeof(spec.name), 0, "PGP Temp %d", i);
		macErr = FSpCreate( &spec, '????', 'BINA', smSystemScript );
		if (macErr == noErr)
			break;
		else if (macErr != dupFNErr)
			goto macError;
	}
	
	err	= PFLNewFileSpecFromFSSpec( PGPGetContextMemoryMgr( context ),
		&spec, tempFileRef );
	if ( IsErr( err ) )
		goto error;
	
	*errorCode	= pgpFileRefStdIOOpen(*tempFileRef, kPGPFileOpenStdUpdateFlags,
						kPGPFileTypeDecryptedBin, &stdIOFile);
	return( stdIOFile );
	
macError:
	err = pgpErrorFromMacError(macErr, kPGPError_CantOpenFile);
error:
	if (*tempFileRef != NULL)
	{
		PFLFreeFileSpec(*tempFileRef);
		*tempFileRef = NULL;
	}
	if (errorCode != NULL)
	{
		pgpAssertAddrValid(errorCode, PGPError);
		*errorCode = err;
	}
	return NULL;
}

#else	/* ] PGP_MACINTOSH [ */

	PGPError
pgpCalcFileSize(
	PFLConstFileSpecRef		fileRef,
	PGPFileOpenFlags		flags,
	size_t *				fileSize)
{
	PGPError		err = kPGPError_NoErr;
	char *			fullPath = NULL;
	struct stat		statBuf;
	
	PFLValidateFileSpec( fileRef );
	PGPValidatePtr( fileSize );
	
	(void)flags;	/* Avoid warning */
	*fileSize = 0;	/* In case there's an error */
	
	err = PFLGetFullPathFromFileSpec(fileRef, &fullPath);
	if ( IsntPGPError( err ) )
	{
		/* XXX Maybe make this more portable.  Does this work on MSDOS? */
		if (stat(fullPath, &statBuf) != 0)
			err = kPGPError_CantOpenFile;
		else if ((S_IFMT & statBuf.st_mode) == S_IFREG)
			*fileSize = statBuf.st_size;
		/* XXX Maybe return a special err if it's not a regular file */
	}
	
		
	PGPFreeData( fullPath );
	return err;
}



	PGPError
pgpCreateFile(
	PFLConstFileSpecRef		fileRef,
	PGPFileType			fileType)
{
	PGPError	err = kPGPError_NoErr;
	char *		fullPath = NULL;
	
	PFLValidateFileSpec( fileRef );
	(void)fileType;
	
	err = PFLGetFullPathFromFileSpec(fileRef, &fullPath);
	if ( IsntPGPError( err ) )
	{
#if PGP_UNIX	/* [ */
		int			fileDes;
		
		/*
		 * XXX Beware that calling this routine could be a security flaw,
		 *     because there is a window between creating and opening
		 *     the file, in between which the file might have been moved
		 *     out of the way, thus possibly having you open a new file
		 *     with the wrong permissions.
		 */
		fileDes = open(fullPath,
						O_WRONLY | O_CREAT | O_EXCL,
						0666 & ~sUnixFileTypeTable[fileType].mask);
		if (fileDes > 0)
			close(fileDes);
		else
			err = kPGPError_CantOpenFile;	/* XXX Use better error */
#else	/* ] PGP_UNIX [ */
		FILE *		stdIOFile;
		
		stdIOFile = fopen(fullPath, "ab+");
		if (stdIOFile != NULL)
			fclose(stdIOFile);
		else
			err = kPGPError_CantOpenFile;	/* XXX Use better error */
#endif	/* ] PGP_UNIX */
	}
	PGPFreeData( fullPath);
	return err;
}




	FILE *
pgpStdIOOpenTempFile(
	PGPContextRef		context,
	PFLFileSpecRef *	tempFileRef,
	PGPError *			errorCode)
{
	char		fileName[L_tmpnam];
	PGPError	err = kPGPError_NoErr;
	FILE *		stdIOFile = NULL;
	
	pgpa(pgpaAddrValid(tempFileRef, PFLFileSpecRef));
	
	*tempFileRef = NULL;

#if defined(_PATH_TMP) && HAVE_MKSTEMP	/* [ */

	{
		int			fd = -1;

		strncpy(fileName, _PATH_TMP "ptmpXXXXXX", sizeof(fileName));
		fileName[sizeof(fileName) - 1] = '\0';
		if ((fd = mkstemp(fileName)) == -1 ||
				(stdIOFile = fdopen(fd, "w+b")) == NULL)
		{
			if (fd != -1)
				close(fd);
			goto fileError;
		}
		err	= PFLNewFileSpecFromFullPath( PGPGetContextMemoryMgr( context ),
				fileName, tempFileRef);
		if (IsPGPError( err ))
			goto error;
	}

#else	/* ] defined(_PATH_TMP) && HAVE_MKSTEMP [ */
	
	if (tmpnam(fileName) != fileName)
		goto fileError;

	if (IsPGPError(err = PFLNewFileSpecFromFullPath(
		PGPGetContextMemoryMgr( context ), fileName, tempFileRef)))
	{
		err = kPGPError_OutOfMemory;
		goto error;
	}
	
	/* XXX Perhaps check for an error and retry if necessary */
	*errorCode = pgpFileRefStdIOOpen(*tempFileRef,
			kPGPFileOpenStdUpdateFlags,
			kPGPFileTypeDecryptedBin, &stdIOFile);

#endif	/* ] defined(_PATH_TMP) && HAVE_MKSTEMP */

	if (stdIOFile == NULL)
	{
	fileError:
		err = kPGPError_CantOpenFile;
		goto error;
	}

	return stdIOFile;
	
error:
	if (*tempFileRef != NULL)
	{
		PFLFreeFileSpec(*tempFileRef);
		*tempFileRef = NULL;
	}
	if (errorCode != NULL)
	{
		pgpAssertAddrValid(errorCode, PGPError);
		*errorCode = err;
	}
	return NULL;
}

#endif	/* ] PGP_MACINTOSH */


/*____________________________________________________________________________
	A stupid API routine called in several places by one file.
	Get RID OF IT as soon as possible.
	
	- the file is renamed to have the name found in "newRef".  The rest of
	the path is ignored (so why pass it?)
	- you can't change either file ref
____________________________________________________________________________*/
	PGPError
pgpRenameFile(
	PFLConstFileSpecRef	oldRef,
	PFLConstFileSpecRef	newRef)
{
	PGPError	err = kPGPError_NoErr;
	char *		newName	= NULL;
	
	PFLValidateFileSpec( oldRef );
	PFLValidateFileSpec( newRef );
	
	err	= PFLGetFileSpecName( newRef, &newName);
	if ( IsntPGPError( err ) )
	{
		PFLFileSpecRef	tempRef;
		
		/* need to copy it because PFLFileSpecRename() changes the name 
		as well as renaming the file */
		err	= PFLCopyFileSpec( oldRef, &tempRef );
		if ( IsntPGPError( err ) )
		{
			err	= PFLFileSpecRename( tempRef, newName );
			(void)PFLFreeFileSpec( tempRef );
		}
		PGPFreeData( newName );
	}
	
	return err;
}




	PGPFile *
pgpFileRefOpen(
	PGPContextRef		context,
	PFLConstFileSpecRef	fileRef,
	PGPFileOpenFlags	flags,
	PGPFileType			fileType,
	PGPError *			errorCode)
{
	FILE *			stdIOFile;
	PGPMemoryMgrRef	memoryMgr;
	
	pgpAssertAddrValid( fileRef, PFLFileSpecRef );
	memoryMgr	= PFLGetFileSpecMemoryMgr( fileRef );

#if PGP_MACINTOSH	/* [ */
	switch (flags & (kPGPFileOpenReadWritePerm |
					kPGPFileOpenAppend | kPGPFileOpenTruncate |
					kPGPFileOpenCreate | kPGPFileOpenTextMode))
	{
		case kPGPFileOpenReadPerm:
			return pgpFileRefMacReadOpen( context, fileRef, flags, errorCode);
		case kPGPFileOpenStdWriteFlags:
			return pgpFileRefMacWriteOpen( context, fileRef, fileType, flags,
											errorCode);
		default:
			break;
	}
	pgpAssertMsg((flags & (kPGPFileOpenMaybeLocalEncode |
							kPGPFileOpenForceLocalEncode)) == 0,
				"This combination of flags not currently supported");
#endif	/* ] PGP_MACINTOSH */
	
	*errorCode = pgpFileRefStdIOOpen(fileRef, flags, fileType, &stdIOFile);
	if ( IsPGPError( *errorCode ) )
		return NULL;
	
	switch (flags & kPGPFileOpenReadWritePerm)
	{
		case kPGPFileOpenReadPerm:
			return pgpFileReadOpen( context, stdIOFile, NULL, NULL);
		case kPGPFileOpenWritePerm:
			return pgpFileWriteOpen( context, stdIOFile, NULL);
		default:
			pgpAssertMsg(0, "No support for simultaneous read/write yet");
			break;
	}
	fclose(stdIOFile);
	return NULL;
}

	PGPFileRead *
pgpFileRefReadCreate(
	PGPContextRef		context,
	PFLConstFileSpecRef	fileRef,
	PGPFileOpenFlags	flags,
	PGPError *			errorCode,
	PGPFileDataType	*	fileDataType)
{
	PGPFile *		pgpFile;
	PGPFileRead *	pgpFileRead;
	
	*fileDataType = kPGPFileDataType_Unknown;
	
	if ((pgpFile = pgpFileRefOpen( context, fileRef, flags,
								kPGPFileTypeNone, errorCode)) == NULL)
		return NULL;
	
	pgpFileRead = pgpPGPFileReadCreate( context, pgpFile, TRUE);
	
	if ( pgpFileRead == NULL)
	{
		pgpFileClose(pgpFile);
		return NULL;
	}
	
	*fileDataType = pgpFile->dataType;
	
	return pgpFileRead;
}

/* XXX Compatibility routines follow, intended to be phased out */

/*
 * pgpFileRefStdIOOpen is currently the heart of the implementation
 * for opening files, but this may change later.
 */

	PGPError
pgpFileRefStdIOOpen(
	PFLConstFileSpecRef	fileRef,
	PGPFileOpenFlags	flags,
	PGPFileType			fileType,
	FILE **				outFile )
{
	char		mode[5];
	FILE *		stdIOFile = NULL;
	PGPError	err = kPGPError_NoErr;
	const char *	kModeStr	= NULL;
	
	(void)fileType;
#if PGP_DEBUG
	if (flags & kPGPFileOpenCreate)
		pgpa(pgpaPGPFileTypeValid(fileType));
#endif
	*outFile	= NULL;
	
	switch (flags & ~(kPGPFileOpenTextMode |
					 kPGPFileOpenMaybeLocalEncode |
					 kPGPFileOpenForceLocalEncode |
					 kPGPFileOpenLocalEncodeHashOnly |
					 kPGPFileOpenNoMacBinCRCOkay |
					 kPGPFileOpenLockFile))
	{
		case kPGPFileOpenReadPerm:		kModeStr	= "r";	break;
		case kPGPFileOpenStdWriteFlags:	kModeStr	= "w";	break;
		case kPGPFileOpenStdAppendFlags:kModeStr	= "a";	break;
		case kPGPFileOpenReadWritePerm:	kModeStr	= "r+";	break;
		case kPGPFileOpenStdUpdateFlags:kModeStr	= "w+";	break;
		case kPGPFileOpenStdAppendFlags | kPGPFileOpenReadPerm:
										kModeStr	= "a+";	break;
		default:
			pgpAssertMsg(0, "Unsupported open mode");
			err = kPGPError_BadParams;
			goto error;
	}
	strcpy( mode, kModeStr );
	
	if (!(flags & kPGPFileOpenTextMode))
		strcat(mode, "b");
	
#if PGP_WIN32
	strcat (mode, "c");			/* Commit-to-disk mode, flushes writes out */
#endif

#if PGP_MACINTOSH	/* [ */
	{
		FSSpec		spec;
		short		refNum;
		SInt8		perm	= fsRdPerm;
		
		err	= PFLGetFSSpecFromFileSpec(fileRef, &spec);
		if ( IsntPGPError( err ) )
		{
			OSErr		macErr;
			
			if (flags & kPGPFileOpenCreate)
			{
				macErr = FSpCreate( &spec,
							pgpGetMacFileCreatorFromPGPFileType( fileType ),
							pgpGetMacFileTypeFromPGPFileType( fileType ),
							smSystemScript );
				if (macErr != noErr && macErr != dupFNErr)
				{
					err = pgpErrorFromMacError(macErr,
							kPGPError_CantOpenFile);
					goto error;
				}
			}
			
			switch (flags & kPGPFileOpenReadWritePerm)
			{
				case kPGPFileOpenReadPerm:		perm = fsRdPerm;	break;
				case kPGPFileOpenWritePerm:		perm = fsWrPerm;	break;
				case kPGPFileOpenReadWritePerm:	perm = fsRdWrPerm;	break;
				default:
					pgpAssertMsg(0, "Unsupported open mode");
					err = kPGPError_BadParams;
					goto error;
			}
			
			if ((macErr = FSpOpenDF(&spec, perm, &refNum)) != noErr)
			{
				err = pgpErrorFromMacError(macErr, kPGPError_CantOpenFile);
				goto error;
			}
			stdIOFile = fdopen(refNum, mode);
			if (stdIOFile == NULL)
				FSClose(refNum);
		}
	}
#elif PGP_WIN32		/* ] [ */
	{
		char *		fullPath = NULL;
		int			fileHandle	= -1;
		int			oFlags = 0;
	#if HAVE_SHARE_H
		int			shFlags = 0;
	#endif

		err = PFLGetFullPathFromFileSpec(fileRef, &fullPath);
		if ( IsntPGPError( err ) )
		{
			/* 'kPGPFileOpenReadWritePerm' is a mask */
			if ( ( flags & kPGPFileOpenReadWritePerm ) ==
					kPGPFileOpenReadWritePerm )
				oFlags |= _O_RDWR;
			else if ( flags & kPGPFileOpenReadPerm )
				oFlags |= _O_RDONLY;
			else if ( flags & kPGPFileOpenWritePerm )
				oFlags |= _O_WRONLY;
			if ( flags & kPGPFileOpenAppend )
				oFlags |= _O_APPEND;
			if ( flags & kPGPFileOpenTruncate )
				oFlags |= _O_TRUNC;
			if ( flags & kPGPFileOpenCreate )
				oFlags |= _O_CREAT;
			if ( flags & kPGPFileOpenTextMode )
				oFlags |= _O_TEXT;
			else
				oFlags |= _O_BINARY;

			/*
			 * XXX: In the future, we may decide to lock by default,
			 *      but for now we're playing it safe and keeping the
			 *      existing behavior.
			 */
		#if HAVE_SHARE_H
			if ( !( flags & kPGPFileOpenLockFile ) )
				shFlags |= _SH_DENYNO;
			else if ( flags & kPGPFileOpenWritePerm )
				shFlags |= _SH_DENYRW;
			else
				shFlags |= _SH_DENYWR;

			fileHandle = sopen( fullPath, oFlags, shFlags,
								_S_IREAD | _S_IWRITE );
		#else
			fileHandle	= open( fullPath, oFlags, _S_IREAD | _S_IWRITE );
		#endif
		
			/* Free the path buffer regardless */
			PGPFreeData( fullPath);

			/* Check for error in sopen */
			if ( fileHandle == -1 )
			{
				if ( errno == EACCES )
					err = kPGPError_FilePermissions;
				else if ( errno == ENOENT )
					err = kPGPError_FileNotFound;
				else
					err = kPGPError_CantOpenFile;
				goto error;
			}

			stdIOFile = fdopen( fileHandle, mode );
			if (stdIOFile == NULL)
				goto error;
		}
	}
#else	/* ] [ */
	{
		char *		fullPath = NULL;
#if PGP_UNIX
		mode_t		oldMask;
#endif
		err = PFLGetFullPathFromFileSpec(fileRef, &fullPath);
		if ( IsntPGPError( err ) )
		{
#if PGP_UNIX
			oldMask = umask(sUnixFileTypeTable[fileType].mask);
#endif
			stdIOFile = fopen(fullPath, mode);

			/* Free the path buffer regardless */
			PGPFreeData( fullPath );

			if (stdIOFile == NULL)
			{
				if (errno == EACCES
#ifdef EROFS
					|| errno == EROFS
#endif
						)
				{
					err = kPGPError_FilePermissions;
				}
				else if (errno == ENOENT)
				{
					err = kPGPError_FileNotFound;
				}
				else
				{
					err = kPGPError_CantOpenFile;
				}
			}
#if PGP_UNIX
			umask(oldMask);
#endif

			if (stdIOFile == NULL)
				goto error;
		}
	}
#endif	/* ] PGP_MACINTOSH */

	if ( IsntNull( stdIOFile ) && ( flags & kPGPFileOpenLockFile ) )
	{
		err = PFLLockFILE( stdIOFile,
						   ( kPFLFileOpenFlags_LockFile |
							 ( ( flags & kPGPFileOpenWritePerm )
								? kPFLFileOpenFlags_ReadWrite
								: kPFLFileOpenFlags_ReadOnly ) ) );
		if ( IsPGPError( err ) )
		{
			fclose( stdIOFile );
			stdIOFile = NULL;
		}
	}

		
error:
	if ( IsNull( stdIOFile ) || IsPGPError( err ) )
	{
		if ( IsntPGPError( err ) )
			err = kPGPError_CantOpenFile;
	}

	*outFile	= stdIOFile;
	
	/* we should have an error and no file, or no error and a file */
	pgpAssert( ( IsntPGPError( err ) && IsntNull( *outFile ) ) ||
		( IsPGPError( err ) && IsNull( *outFile ) ) );
	
	return err;
}


/*
 * Due to the way memory is handled on PGP_WIN32 systems with static linking,
 * fclose must be called from the same library which called fopen.
 * This function servers that purpose.  It should be called by external
 * libraries and apps to close FILE * handles returned by this module.
 */
	PGPError
pgpStdIOClose(
	FILE *			stdIOFile)
{
	if (fclose (stdIOFile)) {
		return kPGPError_FileOpFailed;
	}
	return kPGPError_NoErr;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
