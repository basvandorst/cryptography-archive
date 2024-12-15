/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pflPrefs.h,v 1.9 1997/09/30 20:00:16 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pflPrefs_h	/* [ */
#define Included_pflPrefs_h

#include "pgpPFLConfig.h"
#include "pgpBase.h"
#include "pflTypes.h"

typedef struct PGPPref *	PGPPrefRef;
typedef PGPInt32			PGPPrefIndex;

#define	kInvalidPGPPrefRef			((PGPPrefRef) NULL)
#define PGPPrefRefIsValid( ref )	( (ref) != kInvalidPGPPrefRef )

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


PGPError PGPOpenPrefFile(PFLFileSpecRef prefFileSpec, 
							PGPPrefRef *prefRef);

PGPError PGPSavePrefFile(PGPPrefRef prefRef);

PGPError PGPClosePrefFile(PGPPrefRef prefRef);

PGPError PGPGetPrefFileSpec(PGPPrefRef prefRef,
								PFLFileSpecRef *prefFileSpec);


PGPError PGPGetPrefData(PGPPrefRef prefRef,
							 PGPPrefIndex prefIndex, 
							 PGPSize *dataLength, 
							 void **inBuffer);

PGPError PGPSetPrefData(PGPPrefRef prefRef, 
							 PGPPrefIndex prefIndex, 
							 PGPSize dataLength, 
							 const void *outBuffer);

PGPError PGPRemovePref(PGPPrefRef prefRef, 
							PGPPrefIndex prefIndex);

PGPError PGPDisposePrefData(PGPPrefRef prefRef, 
								 void *dataBuffer);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pflPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
