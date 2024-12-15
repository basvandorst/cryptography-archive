/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpFileIO.h,v 1.4.16.1 1998/11/12 03:18:07 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpFileIO_h	/* [ */
#define Included_pgpFileIO_h

#include "pgpMemoryMgr.h"
#include "pgpFileSpec.h"

/* base class */
#include "pgpIO.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


typedef struct PGPFileIO	PGPFileIO;
typedef PGPFileIO *			PGPFileIORef;




PGPBoolean	PGPFileIOIsValid( PGPFileIORef ref );
#define PGPValidateFileIO( ref )	PGPValidateParam( PGPFileIOIsValid( ref ) )




#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpFileIO_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
