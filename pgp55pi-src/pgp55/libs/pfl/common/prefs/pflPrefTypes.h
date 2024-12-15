/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pflPrefTypes.h,v 1.9 1997/09/11 05:38:35 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pflPrefTypes_h	/* [ */
#define Included_pflPrefTypes_h

#include "pgpBase.h"
#include "pgpPFLErrors.h"
#include "pflPrefs.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



PGPError PGPGetPrefBoolean(PGPPrefRef prefRef,
								PGPPrefIndex prefIndex,
								PGPBoolean *data);

PGPError PGPSetPrefBoolean(PGPPrefRef prefRef,
								PGPPrefIndex prefIndex,
								PGPBoolean data);

PGPError PGPGetPrefNumber(PGPPrefRef prefRef,
							   PGPPrefIndex prefIndex,
							   PGPUInt32 *data);

PGPError PGPSetPrefNumber(PGPPrefRef prefRef,
							   PGPPrefIndex prefIndex,
							   PGPUInt32 data);

PGPError PGPGetPrefStringAlloc(PGPPrefRef prefRef,
									PGPPrefIndex prefIndex,
									char **string);

PGPError PGPGetPrefStringBuffer(PGPPrefRef prefRef,
									PGPPrefIndex prefIndex,
									PGPSize maxSize,
									char *string);

PGPError PGPSetPrefString(PGPPrefRef prefRef,
							   PGPPrefIndex prefIndex,
							   const char *string);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pflPrefTypes_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
