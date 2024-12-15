/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	File IO that uses a FILE * for its implementation

	$Id: pgpProxyIOPriv.h,v 1.2.34.1 1998/11/12 03:18:28 heller Exp $
____________________________________________________________________________*/

#include "pgpProxyIO.h"
#include "pgpIOPriv.h"

PGP_BEGIN_C_DECLARATIONS


/* the virtual function table for a PGPFileIO */
typedef struct PGPProxyIOVtbl
{
	PGPIOVtbl	parentVTBL;
} PGPProxyIOVtbl;


PGPProxyIOVtbl const *	pgpProxyIOGetClassVTBL( void );


/* used to initialize */
typedef struct PGPProxyIOData
{
	PGPBoolean	ownsBaseIO;
	PGPIORef	baseIO;
} PGPProxyIOData;

struct PGPProxyIO
{
	PGPIO			parent;
	
	PGPUInt32		proxyIOMagic;
	PGPBoolean		ownsBaseIO;
	/* this object does all the work */
	PGPIO *			baseIO;
};
#define kPGPProxyIOMagic		0x50524f58	/* PROX */



PGP_END_C_DECLARATIONS

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
