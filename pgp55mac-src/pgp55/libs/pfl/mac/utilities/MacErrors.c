/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$Id: MacErrors.c,v 1.11 1997/09/18 01:34:45 lloyd Exp $
____________________________________________________________________________*/

#include "pgpPFLErrors.h"
#include "pgpMem.h"
#include "MacErrors.h"

enum
{
	paramErrStrIndex	= 1,
	dskFulErrStrIndex,
	nsvErrStrIndex,
	ioErrStrIndex,
	bdNamErrStrIndex,
	fnOpnErrStrIndex,
	eofErrStrIndex,
	posErrStrIndex,
	fnfErrStrIndex,
	wPrErrStrIndex,
	fLckdErrStrIndex,
	vLckdErrStrIndex,
	fBsyErrStrIndex,
	dupFNErrStrIndex,
	permErrStrIndex,
	wrPermErrStrIndex,
	memFullErrStrIndex
};

typedef struct OSErrorMapping
{
	OSStatus	error;
	short		strIndex;

} OSErrorMapping;

static OSErrorMapping	sErrorMappings[] = {
	{ paramErr, 	paramErrStrIndex 	},
	{ dskFulErr, 	dskFulErrStrIndex 	},
	{ nsvErr, 		nsvErrStrIndex 		},
	{ ioErr, 		ioErrStrIndex 		},
	{ bdNamErr,	 	bdNamErrStrIndex 	},
	{ fnOpnErr, 	fnOpnErrStrIndex 	},
	{ eofErr,		eofErrStrIndex 		},
	{ posErr,		posErrStrIndex 		},
	{ fnfErr,		fnfErrStrIndex 		},
	{ wPrErr,		wPrErrStrIndex 		},
	{ fLckdErr,		fLckdErrStrIndex 	},
	{ vLckdErr,		vLckdErrStrIndex 	},
	{ fBsyErr,		fBsyErrStrIndex 	},
	{ dupFNErr,		dupFNErrStrIndex 	},
	{ permErr,		permErrStrIndex 	},
	{ wrPermErr,	wrPermErrStrIndex 	},
	{ memFullErr,	memFullErrStrIndex	} };


/*____________________________________________________________________________
	Get the textual representation of a system error. Returns TRUE if a
	specific error string was found.
______________________________________________________________________________
*/

	Boolean
GetOSErrorString(
	OSStatus 	error,
	StringPtr 	errorStr)
{
	short	strIndex;
	Boolean	foundString;
	UInt32	numMappings;
	UInt32	mappingIndex;

	pgpAssertAddrValid( errorStr, uchar );
	pgpAssert( error != noErr && error != userCanceledErr );
	pgpAssert( IsntNull( GetResource( 'STR#', kOSErrorStringListResID ) ) );

	errorStr[0] = 0;
	strIndex	= 0;
	numMappings = sizeof( sErrorMappings ) / sizeof( sErrorMappings[0] );
	foundString	= FALSE;

	for( mappingIndex = 0; mappingIndex < numMappings; mappingIndex++ )
	{
		if( sErrorMappings[mappingIndex].error == error )
		{
			strIndex = sErrorMappings[mappingIndex].strIndex;
			break;
		}
	}

	if( strIndex != 0 )
	{
		GetIndString( errorStr, kOSErrorStringListResID, strIndex );
		pgpAssert( errorStr[0] != 0 );

		foundString = TRUE;
	}

	if( errorStr[0] == 0 )
	{
		NumToString( error, errorStr );
		foundString = FALSE;
	}

	return( foundString );
}



/*____________________________________________________________________________
	This table maps a PGPError to a mac error.

	The table could be used in either direction, though it may be well-defined
	in  all cases which error should be used 
____________________________________________________________________________*/
typedef struct
{
	PGPError	pgpErr;
	OSStatus	macErr;
} ErrorMappingEntry;

static const ErrorMappingEntry	sErrorMap[] =
{
	{ kPGPError_NoErr,				noErr },
	{ kPGPError_FileNotFound,		fnfErr },
	{ kPGPError_OutOfMemory,		memFullErr },
	{ kPGPError_DiskFull,			dskFulErr },
	{ kPGPError_FileLocked,			fLckdErr },
	{ kPGPError_FileLocked,			opWrErr },
	{ kPGPError_FilePermissions,	permErr },
	{ kPGPError_FilePermissions,	afpAccessDenied },
	{ kPGPError_EOF,				eofErr },
	{ kPGPError_UserAbort,			userCanceledErr },
	{ kPGPError_BadParams,			paramErr },
};
#define kNumErrorMappings	( sizeof( sErrorMap ) / sizeof( sErrorMap[ 0 ] ) )

	PGPError
MacErrorToPGPError( OSStatus macErr )
{
	PGPError					pgpErr;
	const ErrorMappingEntry *	map	= sErrorMap;
	PGPUInt32					mapIndex;
	
	/* by default, if we don't find one */
	pgpErr	= kPGPError_UnknownError;
	
	for( mapIndex = 0; mapIndex < kNumErrorMappings; ++mapIndex)
	{
		ErrorMappingEntry const	*	entry;
		
		entry	= &map[ mapIndex ];
		
		if ( entry->macErr == macErr )
		{
			pgpErr	= entry->pgpErr;
			break;
		}
	}
	
	return( pgpErr );
}


	OSStatus
PGPErrorToMacError( PGPError	pgpErr )
{
	OSStatus					macErr;
	const ErrorMappingEntry *	map	= sErrorMap;
	PGPUInt32					mapIndex;
	
	/* by default, if we don't find one */
	macErr	= -1;
	
	for( mapIndex = 0; mapIndex < kNumErrorMappings; ++mapIndex )
	{
		ErrorMappingEntry const	*	entry;
		
		entry	= &map[ mapIndex ];
		
		if ( entry->pgpErr == pgpErr )
		{
			macErr	= entry->macErr;
			break;
		}
	}
	
	return( macErr );
}








