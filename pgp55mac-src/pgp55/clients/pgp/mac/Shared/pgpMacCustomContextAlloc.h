/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpMacCustomContextAlloc.h,v 1.3 1997/09/11 18:32:25 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpMacCustomContextAlloc_h	/* [ */
#define Included_pgpMacCustomContextAlloc_h


#include "pgpUtilities.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


PGPError	pgpNewContextCustomMacAllocators( PGPContextRef *context );



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpMacCustomContextAlloc_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
