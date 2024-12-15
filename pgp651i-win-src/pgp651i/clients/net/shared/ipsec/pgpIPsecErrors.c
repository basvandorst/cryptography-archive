/*____________________________________________________________________________
	pgpIkeAlerts.c
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
		

	$Id: pgpIPsecErrors.c,v 1.5 1999/03/10 02:52:00 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpPubTypes.h"
#include "pgpErrors.h"
#include "pgpPFLErrors.h"
#include "pgpIPsecErrors.h"
#include "pgpMem.h"


typedef struct ErrorEntry
{
	PGPInt32 const			number;
	const char * const		string;
} ErrorEntry;

#define EN(number, string)		{ number, string }


static const ErrorEntry sErrors[] =
{

EN( kPGPIPsecError_PacketAlreadyReceived,	
							"packet already received"),
EN( kPGPIPsecError_PacketReceivedTooLate,	
							"packet received too late"),
EN( kPGPIPsecError_PacketAuthenticationFailed,	
							"packet authentication failed"),
EN( kPGPIPsecError_NoIPsecHeaderFound,	
							"no ipsec header found in packet"),
EN( kPGPIPsecError_CompressionAlgDoesntMatchCPI,	
							"compression algorithm doesn't match CPI"),
EN( kPGPIPsecError_NoSAFound,	
							"no SA found for packet"),
EN( kPGPIPsecError_NoSPIFound,	
							"no SPI found for packet"),
};
#define kPGPIPsecErrors_NumErrorTableEntries		\
	( sizeof( sErrors ) / sizeof( sErrors[ 0 ] ) )

#undef EN


	static PGPError 
sGetErrorString(
	PGPError			theError,
	PGPSize				bufferSize,
	char *				theString )
{
	PGPUInt32		idx;
	char			temp[ 256 ];
	char const *	errStr	= NULL;
	PGPSize			len	= 0;
	PGPBoolean		bufferBigEnough	= FALSE;
	
	PGPValidateParam( bufferSize >= 1 );
	PGPValidatePtr( theString );
	*theString	= '\0';

	if ((theError < kPGPIPsecError_FirstError) || 
		(theError > kPGPIPsecError_LastError))
		return kPGPError_BadParams;
	
	for( idx = 0; idx < kPGPIPsecErrors_NumErrorTableEntries; ++idx )
	{
		const ErrorEntry *	entry;
		
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
		sprintf( temp, "IPsec error #%ld", (long)theError );
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


	PGPError 
PGPGetIPsecErrorString(
	PGPError			theError,
	PGPSize				bufferSize,
	char *				theString )
{
	return sGetErrorString (
				theError, bufferSize, theString);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/