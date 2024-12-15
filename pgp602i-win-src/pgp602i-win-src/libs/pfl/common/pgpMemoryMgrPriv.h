/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpMemoryMgrPriv.h,v 1.2.12.1 1998/11/12 03:17:58 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpMemoryMgrPriv_h	/* [ */
#define Included_pgpMemoryMgrPriv_h

#include "pgpMemoryMgr.h"

PGP_BEGIN_C_DECLARATIONS


PGPError	pgpCreateStandardMemoryMgr( PGPMemoryMgrRef *newMemoryMgr );
PGPError	pgpDisposeStandardMemoryMgrUserValue( PGPUserValue userValue );
void		pgpFreeDefaultMemoryMgrList(void);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpMemoryMgrPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/