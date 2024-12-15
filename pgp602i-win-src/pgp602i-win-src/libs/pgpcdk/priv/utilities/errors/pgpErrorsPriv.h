/*____________________________________________________________________________
	pgpErrorsPriv.h
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	Error codes for all PGP errors can be found in this file.

	$Id: pgpErrorsPriv.h,v 1.9.24.1 1998/11/12 03:23:17 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpErrorsPriv_h	/* [ */
#define Included_pgpErrorsPriv_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS


char const  *pgperrString(PGPError	error);



PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpErrorsPriv_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
