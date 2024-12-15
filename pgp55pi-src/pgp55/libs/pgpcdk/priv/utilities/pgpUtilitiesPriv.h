/*____________________________________________________________________________
	pgpUtilities.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	This file contains miscellaneous utility functions needed for the PGPsdk.

	$Id: pgpUtilitiesPriv.h,v 1.13 1997/10/09 23:54:19 lloyd Exp $
____________________________________________________________________________*/

#ifndef Included_pgpUtilitiesPriv_h	/* [ */
#define Included_pgpUtilitiesPriv_h

#include "pgpUtilities.h"
#include "pflContext.h"

PGP_BEGIN_C_DECLARATIONS
					
void  * 	pgpNewData( PGPContextRef context,
					PGPSize allocationSize, PGPMemoryFlags flags);

/* Same as PGPNewData(), but copies data into the newly allocated block. */
void  * 	pgpNewDataCopy( PGPContextRef context,
					PGPSize allocationSize, PGPMemoryFlags flags,
					const void *dataToCopy);


PGPError	pgpGetPrefsSpec( PFLContextRef context, PFLFileSpecRef*outRef );


PGPError	pgpsdkCleanupForce( void );
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
