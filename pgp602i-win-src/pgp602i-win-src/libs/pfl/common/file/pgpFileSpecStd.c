/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	

	$Id: pgpFileSpecStd.c,v 1.47.12.1 1998/11/12 03:18:13 heller Exp $
____________________________________________________________________________*/


#include "pgpPFLConfig.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#if HAVE_PATHS_H
#include <paths.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_DIRENT_H && !PGP_WIN32
#include <dirent.h>
#define USE_DIRENT	1
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "pgpDebug.h"
#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pgpMemoryMgr.h"

#define PGP_USE_FILE_SPEC_PRIV		1
#include "pgpFileSpecStdPriv.h"


typedef struct MyData
{
	char		path[ 1 ];
} MyData;
#define GetMyData( ref )			( (MyData *)((ref)->data) )
#define SetMyData( ref, newData )	( ((ref)->data)	= (MyData *)newData )
#define MyDataSize( ref )			( strlen( GetMyData(ref)->path ) + 1)

#if USE_DIRENT
struct PFLDirectoryIter
{
	PGPMemoryMgrRef	memoryMgr;
	DIR *			dirRef;
	PFLFileSpecRef	parentDir;
};
#endif


static PGPError	sExportProc( PFLConstFileSpecRef ref,
					PGPByte **data, PGPSize *dataSize );
static PGPError	sImportProc( PFLFileSpecRef ref,
					PGPByte const *data, PGPSize dataSize );
static PGPError	sGetNameProc( PFLConstFileSpecRef ref, char name[ 256 ] );
static PGPError	sSetNameProc( PFLFileSpecRef ref, char const *name );
static PGPError	sSetMetaInfoProc( PFLFileSpecRef ref, void const *info );
static PGPError	sExistsProc( PFLFileSpecRef ref, PGPBoolean *exists);
static PGPError	sCreateProc( PFLFileSpecRef ref );
static PGPError	sDeleteProc( PFLConstFileSpecRef ref );
static PGPError	sRenameProc( PFLFileSpecRef ref, const char *newName);
static PGPError	sGetMaxNameLengthProc( PFLConstFileSpecRef ref,
					PGPSize * maxNameLength);
static PGPError	sParentDirProc( PFLConstFileSpecRef fileFromDir,
					PFLFileSpecRef *outParent);
static PGPError	sComposeProc( PFLConstFileSpecRef parent, char const *name,
					PFLFileSpecRef *outRef );


static const PGPFileSpecVTBL	sVTBLStd =
{
	sExportProc,
	sImportProc,
	sGetNameProc,
	sSetNameProc,
	
	sSetMetaInfoProc,
	sExistsProc,
	sCreateProc,
	sDeleteProc,
	sRenameProc,
	sGetMaxNameLengthProc,
	sParentDirProc,
	sComposeProc
};
	
	
	PGPFileSpecVTBL const *
GetFileSpecVTBLStd()
{
	return( &sVTBLStd );
}


/*
 * kDirectorySeparators is a string of possible directory-separation characters
 * The first one is the preferred one, which goes in between
 * PGPPATH and the file name if PGPPATH is not terminated with a
 * directory separator.
 *
 * PATHSEP is a the directory separator within a PATH of directories.
 * Only one character per platform, please
 */

#if defined(PGP_MSDOS) || defined(PGP_ATARI) || defined(PGP_WIN32)
static char const kDirectorySeparators[] = "\\/";
#define PATHSEP ';'
#define SINGLE_EXT 1

#elif defined(PGP_UNIX)
static char const kDirectorySeparators[] = "/";
#define PATHSEP ':'

#elif defined(PGP_AMIGA)
static char const kDirectorySeparators[] = "/:";
#define PATHSEP ';'

#elif defined(PGP_VMS)
static char const kDirectorySeparators[] = "]:";	/* Any more? */
#define PATHSEP ';'
#define SINGLE_EXT 1

#elif defined(PGP_MACINTOSH)
static char const kDirectorySeparators[] = ":";
#define PATHSEP ';'

#else
#error Unknown operating system - need one of\
 UNIX, MSDOS, AMIGA, ATARI, VMS, or MACINTOSH.
#endif


	static char const *
pgpFileNameTail( char const *path )
{
	char const *	lastTail = path;
	char const *	separator;
	
	for (separator = kDirectorySeparators; *separator != '\0'; ++separator)
	{
		char const *	tail;
	
		tail = strrchr(lastTail, *separator);
		if ( IsntNull( tail ) )
			lastTail = tail + 1;
	}
	return( lastTail );
}





#if ! PGP_MACINTOSH /* [ */

	PGPError
pgpPlatformOpenFileSpecAsFILE(
	PFLFileSpecRef	spec,
	const char *	openMode,
	FILE **			fileOut )
{
	PGPError		err	= kPGPError_NoErr;
	FILE *			stdioFILE	= NULL;
	char *			fullPath;
	
	PGPValidatePtr( fileOut );
	*fileOut	= NULL;
	PFLValidateFileSpec( spec );
	PGPValidatePtr( openMode );
	PGPValidateParam( spec->type == kPFLFileSpecFullPathType );
	
	
	err	= PFLGetFullPathFromFileSpec( spec, &fullPath );
	if ( IsntPGPError( err ) )
	{
		stdioFILE	= fopen( fullPath, openMode );
		PGPFreeData( fullPath );

		if ( IsNull( stdioFILE ) )
		{
			if (errno == EACCES
#ifdef EROFS
				|| errno == EROFS
#endif
				)
			{
				err	= kPGPError_FilePermissions;
			}
			else if (errno == ENOENT)
			{
				err	= kPGPError_FileNotFound;
			}
			else
			{
				err	= kPGPError_FileOpFailed;
			}
		}
	}
	
	*fileOut	= stdioFILE;
	
	return( err );
}



	PGPError 
PFLNewFileSpecFromFullPath(
	PGPMemoryMgrRef		memoryMgr,
	char const *		path,
	PFLFileSpecRef *	outRef )
{
	PFLFileSpecRef	newFileRef	= NULL;
	PGPError		err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateMemoryMgr( memoryMgr );
	
	err	= pgpNewFileSpec( memoryMgr, kPFLFileSpecFullPathType,
			strlen(path) + 1, &newFileRef );
	if ( IsntPGPError( err ) )
	{
		MyData *	myData	= GetMyData( newFileRef );
		
		strcpy( myData->path, path );
	}
	
	*outRef	= newFileRef;
	return err;
}


	PGPError 
PFLGetFullPathFromFileSpec(
	PFLConstFileSpecRef		fileRef,
	char **					fullPathPtr)
{
	PGPError		err = kPGPError_NoErr;
	char const *	srcPath;
	char *			newPath	= NULL;
	MyData *		myData	= GetMyData( fileRef );

	PGPValidatePtr( fullPathPtr );
	*fullPathPtr	= NULL;
	PFLValidateFileSpec( fileRef );
	PGPValidateParam( fileRef->type == kPFLFileSpecFullPathType );
	
	srcPath	= myData->path;

	newPath = (char *)PGPNewData( fileRef->memoryMgr,
				strlen( myData->path ) + 1, 0);
	
	if ( IsntNull( newPath ) )
		strcpy( newPath, myData->path );
	else
		err	= kPGPError_OutOfMemory;
	
	*fullPathPtr	= newPath;
	
	return err;
}


	PGPError
pgpPlatformGetTempFileSpec(
	PGPMemoryMgrRef			memoryMgr,
	PFLConstFileSpecRef		optionalRef,
	PFLFileSpecRef *		outRef )
{
	char			fileName[ L_tmpnam ];
	PGPError		err = kPGPError_NoErr;
	PFLFileSpecRef	tempRef = NULL;

	(void)optionalRef;
	
#if defined(_PATH_TMP) && HAVE_MKTEMP	/* [ */
	{
		strncpy(fileName, _PATH_TMP "ptmpXXXXXX", sizeof(fileName));
		fileName[ sizeof(fileName) - 1 ] = '\0';
		if ( IsNull( mktemp(fileName) ) )
		{
			err	= kPGPError_FileOpFailed;
		}
			
		if ( IsntPGPError( err ) )
		{
			err	= PFLNewFileSpecFromFullPath( memoryMgr, fileName, &tempRef);
		}
	}

#else	/* ] defined(_PATH_TMP) && HAVE_MKTEMP [ */
	
	if ( tmpnam(fileName) != fileName)
	{
		err	= kPGPError_FileOpFailed;
	}
	
	err = PFLNewFileSpecFromFullPath( memoryMgr, fileName, &tempRef);
	
#endif	/* ] defined(_PATH_TMP) && HAVE_MKTEMP */
	
	*outRef	= tempRef;
	
	return err;
}

#define PGP_ISDIR(m)	( ( ( m ) & S_IFDIR ) != 0 )
#define PGP_ISREG(m)	( ( ( m ) & S_IFREG ) != 0 )

	PGPError
pgpPlatformGetFileInfo(
	PFLConstFileSpecRef	spec,
	PFLFileInfo *		outInfo )
{
	PGPError		err		= kPGPError_NoErr;
	const char *	path	= GetMyData( spec )->path;
	struct stat		statInfo;

	PGPValidateParam( spec->type == kPFLFileSpecFullPathType );
	if ( stat( path, &statInfo ) != 0 )
	{
		err = kPGPError_FileOpFailed;
	}
	else
	{
		outInfo->flags = 0;
		if ( PGP_ISDIR( statInfo.st_mode ) )
			outInfo->flags |= kPGPFileInfo_IsDirectory;
		if ( PGP_ISREG( statInfo.st_mode ) )
		{
			outInfo->flags |= kPGPFileInfo_IsPlainFile;
			outInfo->dataLength = statInfo.st_size;
		}
		else
		{
			outInfo->dataLength = 0;
		}
		outInfo->modificationTime = statInfo.st_mtime;
	}
	
	return err;
}

	PGPError
pgpPlatformLockFILE(
	FILE *				file,
	PFLFileOpenFlags	flags )
{
	PGPError		err		= kPGPError_NoErr;
#if HAVE_FCNTL_H && defined(F_SETLK)
	int				fileDesc = fileno( file );
	struct flock	lockInfo;

	if ( flags & kPFLFileOpenFlags_ReadWrite )
		lockInfo.l_type = F_WRLCK;
	else
		lockInfo.l_type = F_RDLCK;
	lockInfo.l_start	= 0;
	lockInfo.l_whence	= SEEK_SET;
	lockInfo.l_len		= 0;

	if ( fcntl( fileDesc, F_SETLK, &lockInfo ) == -1 )
	{
		if ( errno == EACCES || errno == EAGAIN )
			err = kPGPError_FileLocked;
		else if ( errno == ENOENT )
			err = kPGPError_FileNotFound;
		else
			err = kPGPError_FileOpFailed;
	}
#elif PGP_MACINTOSH || PGP_WIN32
	/* Locking on MacOS and Win32 is done during open */
	(void)file;
	(void)flags;
#else
#error File locking needs to be implemented for this platform!
#endif

	return err;
}

#endif	/* ] PGP_MACINTOSH */

#if USE_DIRENT	/* [ */

	PGPError
pgpPlatformNewDirectoryIter(
	PFLConstFileSpecRef		parentDir,
	PFLDirectoryIterRef *	outIter )
{
	PGPMemoryMgrRef			memoryMgr		= parentDir->memoryMgr;
	const char *			path		= GetMyData( parentDir )->path;
	PFLDirectoryIterRef		newIter		= NULL;
	PGPError				err			= kPGPError_NoErr;

	*outIter = NULL;
	PGPValidateParam( parentDir->type == kPFLFileSpecFullPathType );
	PGPValidateParam( PGPMemoryMgrIsValid( memoryMgr ) );

	newIter = (PFLDirectoryIterRef)PGPNewData( memoryMgr,
										sizeof( *newIter ), 0 );
	if ( IsntNull( newIter ) )
	{
		newIter->memoryMgr = memoryMgr;
		newIter->dirRef = opendir( path );
		if ( IsNull( newIter->dirRef ) )
			err = kPGPError_FileOpFailed;	/* XXX Better error code? */
		else
			err = PFLCopyFileSpec( parentDir, &newIter->parentDir );
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}
	
	if ( IsntPGPError( err ) )
		*outIter = newIter;
	else if ( IsntNull( newIter ) )
		PGPFreeData( newIter );

	return err;
}

	PGPError
pgpPlatformNextFileInDirectory(
	PFLDirectoryIterRef		iter,
	PFLFileSpecRef *		outRef )
{
	PGPError			err = kPGPError_NoErr;
	struct dirent *		entry;

	*outRef = NULL;
	PGPValidatePtr( iter->dirRef );

	entry = readdir( iter->dirRef );
	if ( IsntNull( entry ) )
		err = PFLComposeFileSpec( iter->parentDir, entry->d_name, outRef );
	else
		err = kPGPError_EndOfIteration;

	return err;
}

	PGPError
pgpPlatformFreeDirectoryIter(
	PFLDirectoryIterRef		iter )
{
	PGPError	err = kPGPError_NoErr;

	PGPValidatePtr( iter );

	if ( closedir( iter->dirRef ) != 0 )
		err = kPGPError_FileOpFailed;	/* XXX Better error code? */
	PFLFreeFileSpec( iter->parentDir );
	PGPFreeData( iter );
	return err;
}

#endif	/* ] USE_DIRENT */


/*____________________________________________________________________________
	Export a C string which is the entire path.
____________________________________________________________________________*/
	static PGPError
sExportProc(
	PFLConstFileSpecRef	ref,
	PGPByte **			dataOut,
	PGPSize *			dataSizeOut )
{
	PGPUInt32			dataSize;
	PGPError			err	= kPGPError_NoErr;
	char *				exported	= NULL;
	char const *		srcName;
	
	srcName		= GetMyData( ref )->path;
	dataSize	= strlen( srcName ) + 1;	/* yes, include the NULL char */
	
	exported	= (char *)PGPNewData( ref->memoryMgr, dataSize, 0);
	if ( IsntNull( exported ) )
	{
		strcpy( exported, srcName );
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	pgpAssert( dataSize == strlen( srcName ) + 1 );
	*dataOut		= (PGPByte *)exported;
	*dataSizeOut	= dataSize;
	
	return( err);
}


/*____________________________________________________________________________
	The imported data should be a NULL-terminated C string
____________________________________________________________________________*/
	static PGPError
sImportProc(
	PFLFileSpecRef		ref,
	PGPByte const *		importData,
	PGPSize				importDataSize )
{
	PGPError		err		= kPGPError_NoErr;
	char const *	importedPath;
	MyData *		myData	= NULL;

	importedPath	= (char const *)importData;
	/* data should be a C string (NULL-terminated)  */
	PGPValidateParam( importedPath[ importDataSize - 1 ] == '\0' );
	pgpAssert( strlen( importedPath ) == importDataSize - 1 );
	
	err	= PGPReallocData( ref->memoryMgr, &ref->data, importDataSize, 0 );
	
	if ( IsntPGPError( err ) )
	{
		myData	= GetMyData( ref );
		ref->dataSize	= importDataSize;
		
		pgpAssert( ref->dataSize == strlen( importedPath ) + 1 );
		strcpy( myData->path, importedPath );
	}
	
	return( err);
}


	static PGPError
sGetNameProc(
	PFLConstFileSpecRef	ref,
	char				name[ 256 ] )
{
	char const *	tail	= NULL;
	PGPError		err		= kPGPError_NoErr;
	
	PFLValidateFileSpec( ref );
	PGPValidatePtr( ref->data );
	
	tail	= pgpFileNameTail( (const char *)ref->data );
	if ( IsntNull( tail) && strlen( tail ) <= 256 )
	{
		strcpy( name, tail );
	}
	else
	{
		err	= kPGPError_BadParams;
	}
	
	return( err );
}


	static PGPError
sSetNameProc(
	PFLFileSpecRef	ref,
	char const *	newName )
{
	PGPUInt32	nameLength;
	char *		path	= NULL;
	PGPUInt32	nameOffset;
	PGPUInt32	newSize;
	PGPError	err	= kPGPError_NoErr;
	void *		temp;
	
	PFLValidateFileSpec( ref);
	PGPValidatePtr( ref->data );
	PGPValidatePtr( newName );
	nameLength	= strlen( newName );
	PGPValidateParam( nameLength != 0 );
	
	path		= (char *)ref->data;
	nameOffset	= pgpFileNameTail( path ) - path;

	newSize	= nameOffset + nameLength + 1;
	temp	= path;
	err = PGPReallocData(ref->memoryMgr, &temp, newSize, 0);
	path	= (char *)temp;

	if ( IsntPGPError( err ) )
	{
		ref->data		= (PGPByte *)path;
		strcpy( path + nameOffset, newName);
		ref->dataSize	= newSize;
	}
	
	return( err );

}


	static PGPError
sGetMaxNameLengthProc(
	PFLConstFileSpecRef	ref,
	PGPSize *		maxNameLength )
{
	const MyData *	myData	= GetMyData( ref );
	
	PGPValidatePtr( myData );
	
	/* at some point, this will need to be done programmatically by
	examining the file system in use which the file is  on */
	/* obviously, 8.3 file systems have a max of 8 + 1 + 3 */
	*maxNameLength	= 255;
	
	return( kPGPError_NoErr );
}


	static PGPError
sSetMetaInfoProc(
	PFLFileSpecRef	ref,
	void const *	infoIn )
{
	PGPError	err	= kPGPError_NoErr;
	
	(void)ref;
	(void)infoIn;
	
	return( err );
}

	static PGPError
sParentDirProc(
	PFLConstFileSpecRef		fileFromDir,
	PFLFileSpecRef *		outParent )
{
	PGPMemoryMgrRef	memoryMgr		= fileFromDir->memoryMgr;
	PFLFileSpecRef	newFileRef	= NULL;
	PGPError		err			= kPGPError_NoErr;
	char *			path		= NULL;
	PGPUInt32		nameOffset;
	
	PGPValidatePtr( outParent );
	*outParent	= NULL;
	PGPValidateParam( PGPMemoryMgrIsValid( memoryMgr ) );

	path		= (char *)fileFromDir->data;
	nameOffset	= pgpFileNameTail( path ) - path;

	if (nameOffset <= 1)	/* There is no parent directory */
		return kPGPError_FileNotFound;	/* XXX Reasonable error code? */
	
	err	= pgpNewFileSpec( memoryMgr, kPFLFileSpecFullPathType,
				nameOffset, &newFileRef );
	if ( IsntPGPError( err ) )
	{
		MyData *	myData	= GetMyData( newFileRef );
		
		pgpCopyMemory( path, myData->path, nameOffset - 1 );
		myData->path[nameOffset - 1] = '\0';
	}
	
	*outParent	= newFileRef;
	return err;
}

	static PGPError
sComposeProc(
	PFLConstFileSpecRef	parent,
	const char *		fileName,
	PFLFileSpecRef *	outRef )
{
	PGPMemoryMgrRef	memoryMgr		= parent->memoryMgr;
	PFLFileSpecRef	newFileRef	= NULL;
	PGPError		err			= kPGPError_NoErr;
	char *			path		= NULL;
	PGPSize			pathLength;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateParam( PGPMemoryMgrIsValid( memoryMgr ) );

	path	= (char *)parent->data;
	pathLength	= strlen( path );
	if ( pathLength > 0 && IsntNull( strchr( kDirectorySeparators,
											 path[ pathLength - 1 ] ) ) )
	{
		pathLength--;
	}

	err	= pgpNewFileSpec( memoryMgr, kPFLFileSpecFullPathType,
				pathLength + strlen( fileName ) + 2, &newFileRef );
	if ( IsntPGPError( err ) )
	{
		MyData *	myData	= GetMyData( newFileRef );
		
		pgpCopyMemory( path, myData->path, pathLength );
		myData->path[ pathLength ] = kDirectorySeparators[0];
		strcpy( myData->path + pathLength + 1, fileName );
	}
	
	*outRef	= newFileRef;
	return err;
}


	static PGPError
sExistsProc(
	PFLFileSpecRef	ref,
	PGPBoolean *	exists)
{
	PGPError		err	= kPGPError_NoErr;
	MyData const *	myData = GetMyData( ref );
	FILE *			stdIOFile;

	/* Open the file read-only. If file doesn't exit, fopen fails */

	stdIOFile = fopen( myData->path, "rb" );
	if ( IsntNull( stdIOFile ) )
	{
		fclose(stdIOFile);
		*exists = TRUE;
	}
#ifdef EACCES
	else if ( errno == EACCES )
	{
#if PGP_WIN32
		err = kPGPError_CantOpenFile;
#endif
		*exists = TRUE;
	}
#endif
#ifdef ENOENT
	else if ( errno == ENOENT )
		*exists = FALSE;
#endif
#ifdef EAGAIN
	else if ( errno == EAGAIN )
		*exists = FALSE;
#endif
	else
	{
		pgpDebugMsg( "pgpFileSpecStd.c:sExistsProc(): unknown errno" );
		err = kPGPError_CantOpenFile;
	}

	return( err );
}


	static PGPError
sCreateProc( PFLFileSpecRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	char const *	path	= GetMyData( ref )->path;
	
#if PGP_UNIX	/* [ */
	int			fileDes;
	
	/*
	 * XXX Beware that calling this routine could be a security flaw,
	 *     because there is a window between creating and opening
	 *     the file, in between which the file might have been moved
	 *     out of the way, thus possibly having you open a new file
	 *     with the wrong permissions.
	 */
	fileDes = open( path, O_WRONLY | O_CREAT | O_EXCL, 0600 );
	if (fileDes > 0)
		close( fileDes );
	else
		err = kPGPError_FileOpFailed;
#else	/* ] PGP_UNIX [ */
	FILE *		stdIOFile;
	
	stdIOFile = fopen( path, "ab+");
	if ( IsntNull( stdIOFile ) )
		fclose(stdIOFile);
	else
		err = kPGPError_FileOpFailed;
#endif	/* ] PGP_UNIX */

	return( err );
}


	static PGPError
sDeleteProc( PFLConstFileSpecRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	const char *	path	= GetMyData( ref )->path;
	
	if ( remove( path ) != 0 )
		err = kPGPError_FileOpFailed;
	
	return( err );
}


/*____________________________________________________________________________
	Rename the file AND change the name in the file spec itself.
	
	If the file cannot be renamed, the file spec is not changed.
____________________________________________________________________________*/

	static PGPError
sRenameProc(
	PFLFileSpecRef	ref,
	const char *	newName)
{
	PGPError		err	= kPGPError_NoErr;
	MyData *		oldDataCopy	= NULL;
	PGPMemoryMgrRef	memoryMgr	= ref->memoryMgr;
	
	/* save the old data in case the rename fails */
	/* also needed for rename (see below) */
	oldDataCopy	= (MyData *)PGPNewData( memoryMgr,
					MyDataSize( ref ), 0);
	if ( IsntNull( oldDataCopy ) )
	{
		pgpCopyMemory( GetMyData( ref ),
			oldDataCopy, MyDataSize( ref ) );

		/* this changes the current data */
		err	= sSetNameProc( ref, newName );
		
		if ( rename( oldDataCopy->path, GetMyData( ref )->path ) != 0)
		{
			err = kPGPError_FileOpFailed;		/* XXX Improve error */
			/* put old path back in place of new one */
			PGPFreeData( GetMyData( ref ) );
			SetMyData( ref, oldDataCopy );
		}
		else
		{
			PGPFreeData( oldDataCopy );
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}







/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
