/*____________________________________________________________________________
	pgpCBCPriv.h
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	

	$Id: pgpCBCPriv.h,v 1.2.22.1 1998/11/12 03:21:18 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpCBCPriv_h	/* [ */
#define Included_pgpCBCPriv_h

#include "pgpOpaqueStructs.h"
#include "pgpSymmetricCipher.h"
#include "pgpCBC.h"


PGP_BEGIN_C_DECLARATIONS


#define PGP_CBC_MAXBLOCKSIZE 20



/*____________________________________________________________________________
	internal glue routine follow; use is discouraged
____________________________________________________________________________*/

PGPSize		pgpCBCGetKeySize( PGPCBCContextRef ref );
PGPSize		pgpCBCGetBlockSize( PGPCBCContextRef ref );



PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpCBCPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
