/*____________________________________________________________________________
	Copyright (C) 6.5.1 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpVersionHeader.c,v 1.137.2.28.2.10 1999/07/01 21:28:25 build Exp $
____________________________________________________________________________*/
#include "pgpBuildFlags.h"
#include "pgpVersionHeader.h"

extern const char pgpVersionHeaderString[] =
#if PGP_BUSINESS_SECURITY
	"PGP 6.5.1i";
#elif PGP_PERSONAL_PRIVACY
	"PGP Personal Privacy 6.5.1i";
#elif PGP_FREEWARE
	"PGP 6.5.1i for non-commercial use <http://www.pgpi.com/>";
#elif PGP_DEMO
	"PGP Personal Privacy 6.5.1i Trialware";
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


