/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIKEAlerts.h,v 1.1 1998/12/28 07:53:49 wprice Exp $
____________________________________________________________________________*/

#ifndef Included_pgpIKEAlerts_h
#define Included_pgpIKEAlerts_h

// includes
#include "pgpPubTypes.h"
#include "pgpIKE.h"
#include "pflTypes.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

// prototypes

PGPError PGPGetIkeAlertString(
	PGPikeAlert			theAlert,
	PGPSize				bufferSize,
	char *				theString );

PGPError PGPGetIkeInternalAlertString(
	PGPikeInternalAlert	theAlert,
	PGPSize				bufferSize,
	char *				theString );


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif //Included_pgpIKEAlerts_h

