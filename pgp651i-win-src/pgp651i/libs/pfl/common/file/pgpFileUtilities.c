/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	

	$Id: pgpFileUtilities.c,v 1.17 1999/03/10 02:51:04 heller Exp $
____________________________________________________________________________*/

#include "pgpPFLConfig.h"

#include <string.h>
#include <stdio.h>

#if HAVE_UNISTD_H
#include <unistd.h>			/* Declares truncate() */
#endif

#include "pgpDebug.h"
#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pgpFileUtilities.h"
#include "pgpStdFileIO.h"

#define PGP_USE_FILE_SPEC_PRIV
#include "pgpFileSpecPriv.h"

#ifdef PGP_WIN32
#include <io.h>
#include <fcntl.h>

#endif	/* PGP_WIN32 */


	static PGPError
pgpOpenFileSpecAsFILE(
	PFLFileSpecRef	spec,
	const char *	openMode,
	FILE **			fileOut )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( fileOut );
	*fileOut	= NULL;
	PFLValidateFileSpec( spec );
	PGPValidatePtr( openMode );
	
	err	= pgpPlatformOpenFileSpecAsFILE( spec, openMode, fileOut);
	
	return( err );
}

	PGPError
PFLLockFILE(
	FILE *				file,
	PFLFileOpenFlags	flags )
{
	PGPValidatePtr( file );

	return pgpPlatformLockFILE( file, flags );
}

	PGPError
PGPOpenFileSpec(
	PFLFileSpecRef		spec,
	PFLFileOpenFlags	flags,
	PGPFileIORef *		outRef )
{
	PGPError		err	= kPGPError_NoErr;
	const char *	modeString	= NULL;
	FILE *			stdioFILE	= NULL;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PFLValidateFileSpec( spec );
	PGPValidateParam( ( flags & ~( kPFLFileOpenFlags_ReadWrite 
								 | kPFLFileOpenFlags_LockFile ) ) == 0 );
	
	if ( flags & kPFLFileOpenFlags_ReadWrite )
		modeString	= "r+b";
	else
		modeString	= "rb";
	
	err	= pgpOpenFileSpecAsFILE( spec, modeString, &stdioFILE );
	if ( IsntPGPError( err ) )
	{
		if ( flags & kPFLFileOpenFlags_LockFile )
			err = pgpPlatformLockFILE( stdioFILE, flags );
		if ( IsntPGPError( err ) )
		{
			PGPStdFileIORef		ref;
			PGPMemoryMgrRef		memoryMgr;
			
			memoryMgr	= PFLGetFileSpecMemoryMgr( spec );
			err	= PGPNewStdFileIO( memoryMgr, stdioFILE, TRUE, &ref );
			
			*outRef	= (PGPFileIORef)ref;
		}
	}
	
	return( err );
}



#if PGP_MACINTOSH	/* [ */

#include "MacErrors.h"
	static PGPError
sPGPSetFileSize(
	PFLConstFileSpecRef	fileSpec,
	PGPFileOffset		newSize )
{
	PGPError	err	= kPGPError_NoErr;
	FSSpec		macSpec;
	
	/* caller has already validated other things */
	/* Macs with current file system & API only support < 2GB files */
	PGPValidateParam( newSize < ( 2UL * 1024UL * 1024UL * 1024UL) - 512 );
	
	err	= PFLGetFSSpecFromFileSpec( fileSpec, &macSpec );
	if ( IsntPGPError( err ) )
	{
		short		fileRef;
		OSStatus	macErr;
	
		macErr	= FSpOpenDF( &macSpec, fsRdWrPerm, &fileRef );
		if ( macErr == noErr )
		{
			macErr	= SetEOF( fileRef, (long)newSize );
			FSClose( fileRef );
		}
		
		err	= MacErrorToPGPError( macErr );
	}
	
	
	return( err );
}

#endif	/* ] */

#if PGP_WIN32	/* [ */

#if defined( __MWERKS__ )
PGP_BEGIN_C_DECLARATIONS
/* metrowerks doesn't seem to have this prototype */
int		_chsize( int fileHandle, long newSize );
PGP_END_C_DECLARATIONS
#endif

	static PGPError
sPGPSetFileSize(
	PFLConstFileSpecRef	fileSpec,
	PGPFileOffset		newSize )
{
	PGPError	err = kPGPError_NoErr;
	char *		fullFilePath	= NULL;

	/* caller has already validated other things */
	PGPValidateParam(newSize <= LONG_MAX);

	err = PFLGetFullPathFromFileSpec(fileSpec, &fullFilePath);
	if ( IsntPGPError(err) )
	{
		int			result		= -1;
		int			fileHandle	= -1;
	
		fileHandle = _open(fullFilePath, _O_BINARY | O_WRONLY);

		if (fileHandle == -1)
			err = kPGPError_FileOpFailed;
		else
		{
			result = _chsize(fileHandle, (long) newSize);
			_close(fileHandle);

			if (result == -1)
				err = kPGPError_FileOpFailed;
		}
		
		PGPFreeData( fullFilePath);
	}

	return err;
}

#endif	/* ] PGP_WIN32 */



#if PGP_UNIX	/* [ */

	static PGPError
sPGPSetFileSize(
	PFLConstFileSpecRef	fileSpec,
	PGPFileOffset		newSize )
{
	PGPError	err = kPGPError_NoErr;
	char *		fullFilePath	= NULL;

	/* caller has already validated params */

	err = PFLGetFullPathFromFileSpec(fileSpec, &fullFilePath);
	if ( IsntPGPError(err) )
	{
		int			result		= -1;
	
		result = truncate( fullFilePath, (size_t)newSize );

		if( result == -1 )
			err = kPGPError_FileOpFailed;
		
		PGPFreeData(fullFilePath);
	}

	return err;
}

#endif	/* ] PGP_UNIX */



	PGPError
PGPSetFileSize(
	PFLConstFileSpecRef	fileSpec,
	PGPFileOffset		newSize )
{
	PGPError	err	= kPGPError_NoErr;
	
	PFLValidateFileSpec( fileSpec );
	PGPValidateParam( newSize >= 0 );

	/* call platform specific version */
	err	= sPGPSetFileSize( fileSpec, newSize );	
	
	return( err );
}
















/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
