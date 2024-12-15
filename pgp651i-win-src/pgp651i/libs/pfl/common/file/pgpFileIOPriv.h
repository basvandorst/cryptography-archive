/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	abstract base class for file IO.

	$Id: pgpFileIOPriv.h,v 1.4 1999/03/10 02:50:34 heller Exp $
____________________________________________________________________________*/

#include "pgpFileIO.h"
#include "pgpIOPriv.h"
#include "pgpFileSpec.h"

PGP_BEGIN_C_DECLARATIONS


/* the virtual function table for a PGPFileIO */
typedef struct PGPFileIOVtbl
{
	PGPIOVtbl			parentVTBL;
} PGPFileIOVtbl;


PGPFileIOVtbl const *	pgpFileIOGetClassVTBL( void );


struct PGPFileIO
{
	PGPIO			parent;
	
	PGPUInt32		fileIOMagic;
	PGPBoolean		autoClose;
};
#define kPGPFileIOMagic		0x464c494f



PGP_END_C_DECLARATIONS

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/