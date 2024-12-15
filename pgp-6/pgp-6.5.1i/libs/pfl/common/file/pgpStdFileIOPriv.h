/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	File IO that uses a FILE * for its implementation

	$Id: pgpStdFileIOPriv.h,v 1.3 1999/03/10 02:58:49 heller Exp $
____________________________________________________________________________*/

#include "pgpStdFileIO.h"
#include "pgpFileIOPriv.h"
#include <stdio.h>

PGP_BEGIN_C_DECLARATIONS


/* the virtual function table for a PGPFileIO */
typedef struct PGPStdFileIOVtbl
{
	PGPFileIOVtbl	parentVTBL;
} PGPStdFileIOVtbl;


PGPStdFileIOVtbl const *	pgpStdFileIOGetClassVTBL( void );


/* used to initialize */
typedef struct PGPStdFileIOData
{
	PGPBoolean		autoClose;
	FILE *			stdioFILE;
} PGPStdFileIOData;

struct PGPStdFileIO
{
	PGPFileIO		parent;
	
	PGPUInt32		stdFileIOMagic;
	FILE *			stdioFILE;
	PGPBoolean		autoClose;
};
#define kPGPStdFileIOMagic		0x5354494f



PGP_END_C_DECLARATIONS

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
