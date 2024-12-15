/*____________________________________________________________________________
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpPFLErrors.c,v 1.2 1999/04/02 09:20:04 jason Exp $
____________________________________________________________________________*/
#include <stdio.h>
#include <string.h>

#include "pgpPFLErrors.h"
#include "pgpMem.h"


typedef struct PFLErrorEntry
{
	PGPError const		number;
	const char * const	string;
} PFLErrorEntry;

#define EN(number, string)		{ number, string }


static const PFLErrorEntry sErrors[] =
{
EN( kPGPError_NoErr,					"not an error"),
EN( kPGPError_BadParams,				"bad parameters"),
EN( kPGPError_OutOfMemory,				"out of memory"),
EN( kPGPError_BufferTooSmall,			"buffer too small"),

EN( kPGPError_FileNotFound,				"file not found"),
EN( kPGPError_CantOpenFile,				"can't open file"),
EN( kPGPError_FilePermissions,			"file permissions"),
EN( kPGPError_FileLocked,				"file locked"),
EN( kPGPError_IllegalFileOp,			"illegal file operation"),
EN( kPGPError_FileOpFailed,				"file operation error"),
EN( kPGPError_ReadFailed,				"read failed"),
EN( kPGPError_WriteFailed,				"write failed"),
EN( kPGPError_EOF,						"end of file"),

EN( kPGPError_UserAbort,				"user cancelled"),
EN( kPGPError_UnknownRequest,			"unrecognized request"),
EN( kPGPError_LazyProgrammer,			"unknown error"),
EN( kPGPError_ItemNotFound,				"item not found"),
EN( kPGPError_ItemAlreadyExists,		"item already exists"),
EN( kPGPError_AssertFailed,				"assert failed"),
EN( kPGPError_BadMemAddress,			"bad memory address"),
EN( kPGPError_UnknownError,				"unknown error"),

EN( kPGPError_PrefNotFound,				"preference not found"),
EN( kPGPError_EndOfIteration,			"end of iteration"),
EN( kPGPError_ImproperInitialization,	"improper initialization"),
EN( kPGPError_CorruptData,				"corrupt data"),
EN( kPGPError_FeatureNotAvailable,		"feature not available"),

EN( kPGPError_DiskFull,					"disk full"),
EN( kPGPError_DiskLocked,				"disk locked"),
};

#define kPFLErrors_NumErrorTableEntries		\
	( sizeof( sErrors ) / sizeof( sErrors[ 0 ] ) )

#undef EN


	
	PGPError 
PGPGetPFLErrorString(
	PGPError	theError,
	PGPSize		bufferSize,
	char *		theString )
{
	PGPUInt32		idx;
	char			temp[ 256 ];
	char const *	errStr	= NULL;
	PGPSize			len	= 0;
	PGPBoolean		bufferBigEnough	= FALSE;
	
	PGPValidateParam( bufferSize >= 1 );
	PGPValidatePtr( theString );
	*theString	= '\0';
	
	for( idx = 0; idx < kPFLErrors_NumErrorTableEntries; ++idx )
	{
		const PFLErrorEntry *	entry;
		
		entry	= &sErrors[ idx ];
		if ( entry->number == theError )
		{
			errStr	= entry->string;
			break;
		}
	}
	
	if ( IsNull( errStr ) )
	{
		/* Produce something for missing errors */
		sprintf( temp, "PFLError #%ld", (long)theError );
		errStr	= temp;
	}
	
	len	= strlen( errStr );
	bufferBigEnough	= ( len + 1 <= bufferSize );
	if ( bufferBigEnough )
	{
		strcpy( theString, errStr );
	}
	else
	{
		pgpCopyMemory( errStr, theString, bufferSize - 1 );
		theString[ bufferSize - 1 ]	= '\0';
	}
	
	return( bufferBigEnough	? kPGPError_NoErr : kPGPError_BufferTooSmall );
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/