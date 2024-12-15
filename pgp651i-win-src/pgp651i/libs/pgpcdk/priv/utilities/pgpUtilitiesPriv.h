/*____________________________________________________________________________
	pgpUtilities.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	This file contains miscellaneous utility functions needed for the PGPsdk.

	$Id: pgpUtilitiesPriv.h,v 1.17 1999/03/10 02:59:39 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpUtilitiesPriv_h	/* [ */
#define Included_pgpUtilitiesPriv_h

#include "pgpUtilities.h"
#include "pflTypes.h"
#include "pgpMemoryMgr.h"

PGP_BEGIN_C_DECLARATIONS
					
PGPError	pgpGetPrefsSpec( PGPMemoryMgrRef context, PFLFileSpecRef*outRef );


PGPError	pgpForceSDKCleanup( void );
PGPBoolean	pgpsdkIsInited( void );

#define pgpValidatePGPContextRef( context ) \
	PGPValidateParam( pgpContextIsValid( context ) )

PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpUtilitiesPriv_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
