/*____________________________________________________________________________
	pgpRandomPoolPriv.h
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	Random pool and dummy random pool, used globally.

	$Id: pgpRandomPoolPriv.h,v 1.5.30.1 1998/11/12 03:21:31 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpRandomPoolPriv_h
#define Included_pgpRandomPoolPriv_h

#include "pgpRandomPool.h"
#include "pgpRandomContext.h"


PGP_BEGIN_C_DECLARATIONS


PGPError		pgpInitGlobalRandomPool( void );
void			pgpInitGlobalRandomPoolContext( PGPRandomContext *rc );

/* one could create multiple instances of the global random pool */
PGPRandomVTBL const *	pgpGetGlobalRandomPoolVTBL( void );
PGPRandomVTBL const *	pgpGetGlobalDummyRandomPoolVTBL( void );

/*
 * Return the number of bits added to the rand pool over its lifetime.
 * This does not count any seed bits that were read from the file.
 */
PGPUInt32 				pgpGlobalRandomPoolGetInflow( void );


PGP_END_C_DECLARATIONS

#endif /* Included_pgpRandomPoolPriv_h */
