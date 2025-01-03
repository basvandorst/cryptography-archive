/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecErrors.h,v 1.8 1999/03/10 02:52:01 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIPsecErrors_h	/* [ */
#define Included_pgpIPsecErrors_h

#include "pgpErrors.h"

enum PGPIPsecError_
{
	kPGPIPsecError_FirstError					= -5000,
	kPGPIPsecError_LastError					= -4000,

	kPGPIPsecError_PacketAlreadyReceived		= -4999,
	kPGPIPsecError_PacketReceivedTooLate		= -4998,
	kPGPIPsecError_PacketAuthenticationFailed	= -4997,
	kPGPIPsecError_NoIPsecHeaderFound			= -4996,
	kPGPIPsecError_CompressionAlgDoesntMatchCPI	= -4995,
	kPGPIPsecError_NoSAFound					= -4994,
	kPGPIPsecError_NoSPIFound					= -4993
};

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


	PGPError 
PGPGetIPsecErrorString(
	PGPError			theError,
	PGPSize				bufferSize,
	char *				theString );


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpIPsecErrors_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
