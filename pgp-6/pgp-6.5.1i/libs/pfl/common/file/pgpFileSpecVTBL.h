/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpFileSpecVTBL.h,v 1.3 1999/03/10 02:51:02 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpFileSpecVTBL_h	/* [ */
#define Included_pgpFileSpecVTBL_h

#include "pgpFileSpecPriv.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif




const PGPFileSpecVTBL *	pgpGetFileSpecVTBLForType( PFLFileSpecType type );



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpFileSpecVTBL_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
