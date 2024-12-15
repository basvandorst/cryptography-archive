/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIkeAlerts.c,v 1.9 1999/05/05 06:22:01 wprice Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpPubTypes.h"
#include "pgpIke.h"
#include "pgpMem.h"

#include "pgpIKEAlerts.h"


typedef struct AlertEntry
{
	PGPInt32 const			number;
	const char * const		string;
} AlertEntry;

#define EN(number, string)		{ number, string }


static const AlertEntry sAlerts[] =
{

EN( kPGPike_AL_None,					"None"),
EN( kPGPike_AL_InvalidPayload,			"Invalid Payload"),
EN( kPGPike_AL_DOIUnsupported,			"Unsupported DOI"),
EN( kPGPike_AL_SituationUnsupported,	"Unsupported Situation"),
EN( kPGPike_AL_InvalidCookie,			"Invalid Cookie"),
EN( kPGPike_AL_InvalidMajorVersion,		"Invalid Major Version"),
EN( kPGPike_AL_InvalidMinorVersion,		"Invalid Minor Version"),
EN( kPGPike_AL_InvalidExchange,			"Invalid Exchange"),
EN( kPGPike_AL_InvalidFlags,			"Invalid Flags"),
EN( kPGPike_AL_InvalidMessageID,		"Invalid Message ID"),
EN( kPGPike_AL_InvalidProtocolID,		"Invalid Protocol ID"),
EN( kPGPike_AL_InvalidSPI,				"Invalid SPI"),
EN( kPGPike_AL_InvalidTransform,		"Invalid Transform"),
EN( kPGPike_AL_InvalidAttribute,		"Invalid Attribute"),
EN( kPGPike_AL_NoProposalChoice,		"No Common Proposals"),
EN( kPGPike_AL_BadProposal,				"Bad Proposal"),
EN( kPGPike_AL_PayloadMalformed,		"Malformed Payload"),
EN( kPGPike_AL_InvalidKey,				"Invalid Key"),
EN( kPGPike_AL_InvalidID,				"Invalid ID"),
EN( kPGPike_AL_InvalidCertEncoding,		"Invalid Certificate Encoding"),
EN( kPGPike_AL_InvalidCert,				"Invalid Certificate"),
EN( kPGPike_AL_UnsupportedCert,			"Certificate Rejected"),
EN( kPGPike_AL_InvalidCertAuthority,	"Invalid Certificate Authority"),
EN( kPGPike_AL_InvalidHash,				"Invalid Hash"),
EN( kPGPike_AL_AuthenticationFailed,	"Authentication Failed"),
EN( kPGPike_AL_InvalidSignature,		"Invalid Signature"),
EN( kPGPike_AL_AddressNotification,		"IP Address Error"),
EN( kPGPike_AL_NotifySALifetime,		"SA Lifetime Error"),
EN( kPGPike_AL_CertUnavailable,			"Unavailable Certificate"),
EN( kPGPike_AL_UnsupportedExchange,		"Unsupported Exchange"),
EN( kPGPike_AL_UnequalPayloadLengths,	"Unequal Payload Lengths"),

};
#define kPGPAlerts_NumAlertTableEntries		\
	( sizeof( sAlerts ) / sizeof( sAlerts[ 0 ] ) )


static const AlertEntry sInternalAlerts[] =
{

EN( kPGPike_IA_None,					"none"),
EN( kPGPike_IA_ResponseTimeout,			"Response Timeout"),
EN( kPGPike_IA_NoProposals,				"No Proposals"),
EN( kPGPike_IA_NewPhase1SA,				"IKE SA Created"),
EN( kPGPike_IA_NewPhase2SA,				"IPSEC SA Created"),
EN( kPGPike_IA_DeadPhase1SA,			"IKE SA Died"),
EN( kPGPike_IA_DeadPhase2SA,			"IPSEC SA Died"),
EN( kPGPike_IA_TooManyExchanges,		"Packet Ignored: Denial of Service"),

};
#define kPGPAlerts_NumInternalAlertTableEntries		\
	( sizeof( sInternalAlerts ) / sizeof( sInternalAlerts[ 0 ] ) )

#undef EN


	static PGPError 
sGetAlertString(
	const AlertEntry	sAlertTable[],
	PGPikeAlert			theAlert,
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
	
	for( idx = 0; idx < kPGPAlerts_NumAlertTableEntries; ++idx )
	{
		const AlertEntry *	entry;
		
		entry	= &sAlertTable[ idx ];
		if ( entry->number == theAlert )
		{
			errStr	= entry->string;
			break;
		}
	}
	
	if ( IsNull( errStr ) )
	{
		/* Produce something for missing errors */
		sprintf( temp, "IKE alert #%ld", (long)theAlert );
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
PGPGetIkeAlertString(
	PGPikeAlert			theAlert,
	PGPSize				bufferSize,
	char *				theString )
{
	return sGetAlertString (
				sAlerts, theAlert, bufferSize, theString);
}


	PGPError 
PGPGetIkeInternalAlertString(
	PGPikeInternalAlert	theAlert,
	PGPSize				bufferSize,
	char *				theString )
{ 
	return sGetAlertString (
				sInternalAlerts, (PGPikeAlert)theAlert,
				bufferSize, theString);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
