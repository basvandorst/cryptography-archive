/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$Id: PGPSplash.r,v 1.4 1997/10/28 20:45:37 heller Exp $
____________________________________________________________________________*/

#include "PGPBuildFlags.h"

#if PGP_FREEWARE
	Include "PGPSplashFree.rsrc";
#elif PGP_BUSINESS_SECURITY
	Include "PGPSplashBS.rsrc";
#elif PGP_PERSONAL_PRIVACY
	Include "PGPSplashPP.rsrc";
#else
	#error Cannot include correct splash screen
#endif

#if NO_RSA_OPERATIONS
	Include "PGPSplashCreditsDH.rsrc";
#else
	Include "PGPSplashCredits.rsrc";
#endif

