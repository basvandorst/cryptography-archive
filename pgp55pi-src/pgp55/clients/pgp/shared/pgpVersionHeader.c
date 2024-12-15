/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpVersionHeader.c,v 1.2.4.1 1997/11/25 04:42:59 build Exp $
____________________________________________________________________________*/
#include "pgpBuildFlags.h"

char const pgpVersionHeaderString[] =
#if PGP_BUSINESS_SECURITY
	"PGP for Business Security 5.5.3";
#elif PGP_PERSONAL_PRIVACY
	"PGP for Personal Privacy 5.5.3";
#elif PGP_FREEWARE
	"PGPfreeware 5.5.3 for non-commercial use <http://www.pgp.com>";
#elif PGP_DEMO
	"PGP for Personal Privacy 5.5.3 Trialware";
#endif



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
