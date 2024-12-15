/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies. 
	All rights reserved.

	$Id: pgpDiskWiper.h,v 1.3 1999/03/10 02:50:11 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpDiskWiper_h	/* [ */
#define Included_pgpDiskWiper_h

#include "pgpPubTypes.h"

#define kPGPNumPatterns 26

/* need to integrate this error */
#define kPGPError_NoMorePatterns 1

typedef struct PGPDiskWipe * PGPDiskWipeRef;

#define	kInvalidPGPDiskWipeRef			((PGPDiskWipeRef) NULL)
#define PGPDiskWipeRefIsValid( ref )	( (ref) != PGPDiskWipeRef )

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


PGPError 
PGPCreateDiskWiper(	PGPContextRef context, 
					PGPDiskWipeRef *wipeRef,
					PGPInt32 numPasses);
						
PGPError 
PGPGetDiskWipeBuffer(	PGPDiskWipeRef wipeRef,
						PGPInt32 buffer[256]);
											
PGPError 
PGPDestroyDiskWiper(PGPDiskWipeRef wipeRef);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpDiskWiper_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
