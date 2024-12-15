/*____________________________________________________________________________
	pgpClientErrors.c
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpClientErrors.c,v 1.9 1999/03/10 02:47:24 heller Exp $
____________________________________________________________________________*/
#include <stdio.h>
#include <string.h>

#include "pgpClientErrors.h"
#include "pgpMem.h"


typedef struct ClientErrorEntry
{
	PGPError const		number;
	const char * const	string;
} ClientErrorEntry;

#define EN(number, string)		{ number, string }


static const ClientErrorEntry sErrors[] =
{
/* Share errors */
EN(kPGPClientError_NotEnoughSharesInObject,
   "not enough shares to split"),
EN(kPGPClientError_DifferentSplitKeys,
   "shares were not split from the same key"),
EN(kPGPClientError_DifferentSharePool,
   "shares were not split from the same share pool"),
EN(kPGPClientError_IdenticalShares,
   "identical shares cannot be combined"),

/* SKEP errors */
EN(kPGPClientError_IncorrectSKEPVersion,
   "client SKEP version different from server SKEP version"),
EN(kPGPClientError_RejectedSKEPAuthentication,
   "your key was not accepted for authentication by the remote system"),

/* Pref errors */
EN(kPGPClientError_AdminPrefsNotFound,
   "administrative preferences file not found"),

EN(kPGPClientError_UnsplitPrivateKeyNotFound,
   "unsplit private key could not be found to authenticate the connection")
};

#define kPGPClientErrors_NumErrorTableEntries		\
	( sizeof( sErrors ) / sizeof( sErrors[ 0 ] ) )

#undef EN


	
	PGPError 
PGPGetClientErrorString(
	PGPError	theError,
	PGPSize		bufferSize,
	char *		theString )
{
	PGPUInt32		idx;
	char			temp[ 256 ];
	char const *	errStr	= NULL;
	PGPSize			len	= 0;
	PGPBoolean		bufferBigEnough	= FALSE;
	
	if (theError >= kPGPPFLErrorBase && theError <= kPGPError_Last )
		return PGPGetErrorString(theError, bufferSize, theString);

	PGPValidateParam( bufferSize >= 1 );
	PGPValidatePtr( theString );
	*theString	= '\0';
	
	for( idx = 0; idx < kPGPClientErrors_NumErrorTableEntries; ++idx )
	{
		const ClientErrorEntry *	entry;
		
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
		sprintf( temp, "PGPClientError #%ld", (long)theError );
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