/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	File IO that uses a FILE * for its implementation

	$Id: pgpProxyIOPriv.h,v 1.2 1997/08/21 01:54:31 lloyd Exp $
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
