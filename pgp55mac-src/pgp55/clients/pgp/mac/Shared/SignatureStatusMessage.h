/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/
#pragma once


#include "pgpEncode.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


typedef struct
{
	PGPEventSignatureData	sigData;
	/* used if key could not be found */
	char					keyIDString[ 64 ];
} SignatureStatusInfo;

void GetSignatureStatusMessage(	SignatureStatusInfo const *	info,
								StringPtr	msg );



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS