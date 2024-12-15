/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPOpenPrefs.h,v 1.9 1997/09/29 10:24:38 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpUtilities.h"
#include "pflPrefs.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



PGPError PGPOpenClientPrefs(	PFLContextRef	pgpContext,
								PGPPrefRef		*prefRef);
								
PGPError PGPOpenAdminPrefs(		PFLContextRef	pgpContext,
								PGPPrefRef		*prefRef );

PGPError PGPGetGroupsFileFSSpec(FSSpec			*fileSpec);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

