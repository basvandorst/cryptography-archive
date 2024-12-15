/*____________________________________________________________________________
	pgpCFBPriv.h
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	

	$Id: pgpCFBPriv.h,v 1.3.12.1 1998/11/12 03:21:19 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpCFBPriv_h	/* [ */
#define Included_pgpCFBPriv_h

#include "pgpOpaqueStructs.h"
#include "pgpSymmetricCipher.h"
#include "pgpCFB.h"


PGP_BEGIN_C_DECLARATIONS


#define PGP_CFB_MAXBLOCKSIZE 20



/* Clear a PGPCFBContext of its sensitive data */
void 							pgpCFBWipe( PGPCFBContextRef ref );



/*____________________________________________________________________________
	internal glue routine follow; use is discouraged
____________________________________________________________________________*/

PGPSize		pgpCFBGetKeySize( PGPCFBContextRef ref );
PGPSize		pgpCFBGetBlockSize( PGPCFBContextRef ref );

/* Allocate a new PGPCFBContext structure */
PGPCFBContextRef		pgpCFBCreate( PGPMemoryMgrRef memoryMgr,
									PGPCipherVTBL const *c );



PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpCFBPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/