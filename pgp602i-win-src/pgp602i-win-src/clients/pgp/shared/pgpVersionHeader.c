/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpVersionHeader.c,v 1.47.2.24.2.1 1998/11/12 03:11:15 heller Exp $
____________________________________________________________________________*/
#include "pgpBuildFlags.h"
#include "pgpVersionHeader.h"

extern const char pgpVersionHeaderString[] =
#if PGP_BUSINESS_SECURITY
	"PGP 6.0.2i";
#elif PGP_PERSONAL_PRIVACY
	"PGP Personal Privacy 6.0.2i";
#elif PGP_FREEWARE
	"PGPfreeware 6.0.2i for non-commercial use";
#elif PGP_DEMO
	"PGP Personal Privacy 6.0.2i Trialware";
#else
	#error unknown build
#endif



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


