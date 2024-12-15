/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpGroupsPriv.h,v 1.2 1999/03/10 02:51:20 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpGroupsPriv_h	/* [ */
#define Included_pgpGroupsPriv_h

#include "pgpGroups.h"

#define PGPValidateGroupSet( set )	\
		PGPValidateParam( PGPGroupSetIsValid( set ) );

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPBoolean	PGPGroupSetIsValid( PGPGroupSetRef set );

void		TestGroups(void);
PGPError	TestGroupsUtil(void);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpGroupsPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/








