/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpFileIO.h,v 1.3 1997/09/11 05:38:27 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpFileIO_h	/* [ */
#define Included_pgpFileIO_h

#include "pflContext.h"
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
