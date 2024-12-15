/*____________________________________________________________________________
	pgpNetServiceErrors.c
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
		

	$Id: pgpNetServiceErrors.c,v 1.7 1999/05/18 08:49:14 wprice Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpPubTypes.h"
#include "pgpMem.h"

#include "pgpNetServiceErrors.h"

typedef struct ErrorEntry
{
	PGPnetServiceError const	number;
	const char * const			string;
} ErrorEntry;

#define EN(number, string)		{ number, string }


static const ErrorEntry sErrors[] =
{

EN( kPGPnetSrvcError_NoErr,				"not an error"),
EN( kPGPnetSrvcError_NoAuthPassphrase,	"Unable to obtain passphrase for authentication key"),
EN( kPGPnetSrvcError_NoAuthKey,			"Unable to obtain authentication key"),
EN( kPGPnetSrvcError_NTon95,			"The IKE service installed on this machine is designed for Windows NT only"),
EN( kPGPnetSrvcError_ReadConfig,		"Unable to read PGPnet configuration"),
EN( kPGPnetSrvcError_NoNetwork,			"PGPnet unable to start networking"),
EN( kPGPnetSrvcError_NoDriver,			"The PGPnet driver is not installed"),
EN( kPGPnetSrvcError_DrvEvent,			"Unable to communicate with PGPnet driver"),
EN( kPGPnetSrvcError_DrvSharedMemory,	"Unable to communicate with PGPnet driver"),
EN( kPGPnetSrvcError_DrvCommunication,	"Unable to communicate with PGPnet driver"),
EN( kPGPnetSrvcError_SAFailed,			"Unable to establish Security Association with peer"),
EN( kPGPnetSrvcError_95onNT,			"The IKE service installed on this machine is designed for Windows 95/98 only"),
EN( kPGPnetSrvcError_AuthenticatedKey,	"Authenticated Key ID "),
EN( kPGPnetSrvcError_AuthenticatedCert,	"Authenticated X.509 certificate"),
EN( kPGPnetSrvcError_RevokedCert,		"Remote X.509 certificate is revoked"),
EN( kPGPnetSrvcError_ExpiredCert,		"Remote X.509 certificate is expired"),
EN( kPGPnetSrvcError_RevokedKey,		"Remote PGP key is revoked"),
EN( kPGPnetSrvcError_ExpiredKey,		"Remote PGP key is expired"),

};
#define kPGPnetServiceErrors_NumErrorTableEntries		\
	( sizeof( sErrors ) / sizeof( sErrors[ 0 ] ) )

#undef EN


	PGPError 
PGPnetGetServiceErrorString(
	PGPnetServiceError	theError,
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
	
	for( idx = 0; idx < kPGPnetServiceErrors_NumErrorTableEntries; ++idx )
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
		sprintf( temp, "PGPnet service error #%ld", (long)theError );
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
