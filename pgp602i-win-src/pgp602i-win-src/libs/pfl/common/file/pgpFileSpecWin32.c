/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	

	$Id: pgpFileSpecWin32.c,v 1.6.16.1 1998/11/12 03:18:16 heller Exp $
____________________________________________________________________________*/


#include "pgpPFLConfig.h"

#include <windows.h>
#include <stdio.h>

#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpPFLErrors.h"
#define PGP_USE_FILE_SPEC_PRIV
#include "pgpFileSpecPriv.h"
#include "pgpMemoryMgr.h"

struct PFLDirectoryIter
{
	PGPMemoryMgrRef		memoryMgr;
	PFLFileSpecRef		parentDir;
	HANDLE				fileSearch;
	WIN32_FIND_DATA		findData;
};



	PGPError
pgpPlatformNewDirectoryIter(
	PFLConstFileSpecRef		parentDir,
	PFLDirectoryIterRef *	outIter )
{
	PGPMemoryMgrRef			memoryMgr	= PFLGetFileSpecMemoryMgr( parentDir );
	char *					path;
	char *					pattern;
	PFLDirectoryIterRef		newIter	= NULL;
	PGPError				err		= kPGPError_NoErr;

	*outIter = NULL;
	PGPValidateMemoryMgr( memoryMgr );

	err = PFLGetFullPathFromFileSpec( parentDir, &path );
	if ( IsntPGPError( err ) )
	{
		pattern = (char *)PGPNewData( memoryMgr,
								 strlen( path ) + 5,  0 );
		if ( IsntNull( pattern ) )
		{
			strcpy( pattern, path );
			strcat( pattern, "\\*.*" );
			newIter = (PFLDirectoryIterRef)PGPNewData( memoryMgr,
												sizeof( *newIter ), 0 );
			if ( IsntNull( newIter ) )
			{
				newIter->memoryMgr = memoryMgr;
				newIter->fileSearch = FindFirstFile( pattern,
													 &newIter->findData );

				if ( newIter->fileSearch == INVALID_HANDLE_VALUE &&
						GetLastError() != 0 )
				{
					err = kPGPError_FileOpFailed;	/* XXX Better error code? */
				}
				else
				{
					err = PFLCopyFileSpec( parentDir, &newIter->parentDir );
				}
			}
			else
			{
				err = kPGPError_OutOfMemory;
			}
			PGPFreeData( pattern );
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}
		PGPFreeData( path );
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

	*outRef = NULL;

	if ( iter->fileSearch != INVALID_HANDLE_VALUE )
	{
		err = PFLComposeFileSpec( iter->parentDir,
								  iter->findData.cFileName, outRef );
		if ( !FindNextFile( iter->fileSearch, &iter->findData ) )
			iter->fileSearch = INVALID_HANDLE_VALUE;
	}
	else
	{
		err = kPGPError_EndOfIteration;
	}

	return err;
}

	PGPError
pgpPlatformFreeDirectoryIter(
	PFLDirectoryIterRef		iter )
{
	PGPError	err = kPGPError_NoErr;

	PGPValidatePtr( iter );

	PFLFreeFileSpec( iter->parentDir );
	FindClose( iter->fileSearch ); 
	PGPFreeData( iter );
	return err;
}



/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
