/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	

	$Id: pgpFileSpecWin32.c,v 1.2 1997/09/17 00:45:40 mhw Exp $
____________________________________________________________________________*/


#include "pgpPFLConfig.h"

#include <windows.h>
#include <stdio.h>

#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpPFLErrors.h"
#include "pgpFileSpec.h"
#include "pflContext.h"

struct PFLDirectoryIter
{
	PFLContextRef		context;
	PFLFileSpecRef		parentDir;
	HANDLE				fileSearch;
	WIN32_FIND_DATA		findData;
};



	PGPError
pgpPlatformNewDirectoryIter(
	PFLConstFileSpecRef		parentDir,
	PFLDirectoryIterRef *	outIter )
{
	PFLContextRef			context	= PFLGetFileSpecContext( parentDir );
	char *					path;
	char *					pattern;
	PFLDirectoryIterRef		newIter	= NULL;
	PGPError				err		= kPGPError_NoErr;

	*outIter = NULL;
	PGPValidateParam( PFLContextIsValid( context ) );

	err = PFLGetFullPathFromFileSpec( parentDir, &path );
	if ( IsntPGPError( err ) )
	{
		pattern = (char *)PFLContextMemAlloc( context,
											  strlen( path ) + 5,
											  0 );
		if ( IsntNull( pattern ) )
		{
			strcpy( pattern, path );
			strcat( pattern, "\\*.*" );
			newIter = (PFLDirectoryIterRef)PFLContextMemAlloc( context,
												sizeof( *newIter ), 0 );
			if ( IsntNull( newIter ) )
			{
				newIter->context = context;
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
			PFLContextMemFree( context, pattern );
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}
		PFLContextMemFree( context, path );
	}

	if ( IsntPGPError( err ) )
		*outIter = newIter;
	else if ( IsntNull( newIter ) )
		PFLContextMemFree( context, newIter );

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
	PFLContextMemFree( iter->context, iter );
	return err;
}



/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
