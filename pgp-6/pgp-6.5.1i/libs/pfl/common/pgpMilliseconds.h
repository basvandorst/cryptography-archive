/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: pgpMilliseconds.h,v 1.2 1998/11/03 13:14:49 pbj Exp $
____________________________________________________________________________*/
#ifndef Included_pgpMilliseconds_h	/* [ */
#define Included_pgpMilliseconds_h

#include "pgpBase.h"

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


PGPUInt32	PGPGetMilliseconds();


#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpMilliseconds_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
