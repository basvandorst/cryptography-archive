/*____________________________________________________________________________
	pgpContext.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	This file contains the definition of the PGPContext data structure and
	prototypes for private context manipulation functions. See pgpUtilities.h
	for public PGPContext manipulation functions

	$Id: pgpContext.h,v 1.34 1999/03/10 02:49:27 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpContext_h	/* [ */
#define Included_pgpContext_h

#include "pgpMemoryMgr.h"
#include "pgpOpaqueStructs.h"
#include "pgpUtilities.h"
#include "pgpPFLErrors.h"
#include "pflPrefs.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

PGPBoolean 	pgpContextIsValid( PGPContextRef context );
#define PGPValidateContext( context )	\
	PGPValidateParam( pgpContextIsValid( context ) )

PGPError 	pgpNewContextInternal( PGPUInt32 sdkVersion,
						PGPContextRef *newContext);
PGPError 	pgpNewContextCustomInternal(
				PGPUInt32 sdkVersion,
				PGPNewContextStruct const *	custom,
				PGPContextRef * newContext);
	
PGPEnv *	pgpContextGetEnvironment(PGPContextRef context);

PGPRandomContext *	pgpContextGetGlobalPoolRandomContext(
						PGPContextRef context);
PGPRandomContext *	pgpContextGetDummyRandomContext(PGPContextRef context);
PGPRandomContext *	pgpContextGetX9_17RandomContext(PGPContextRef context);
								
RingPool  *		pgpContextGetRingPool(PGPContextRef context);
void				pgpContextSetRingPool(PGPContextRef context,
								RingPool *ringPool);
PGPKeySetRef		pgpContextGetDefaultKeySet(PGPContextRef context);
PGPBoolean			pgpContextArePrefsChanged(PGPContextRef context);
void				pgpContextSetPrefsChanged(PGPContextRef context,
								PGPBoolean randomSeeded);

void				pgpContextSetPrefs( PGPContextRef context,
						PGPPrefRef prefs );
PGPPrefRef			pgpContextGetPrefs( PGPContextRef context );

/* memory allocation and deallocation */
void  *		pgpContextMemAlloc( PGPContextRef context,
					PGPSize allocationSize, PGPMemoryMgrFlags flags );
void  *		PGPContextMemAllocCopy( PGPContextRef context,
					PGPSize allocationSize, PGPMemoryMgrFlags flags,
					const void *dataToCopy);
PGPError 	pgpContextMemRealloc( PGPContextRef context,
					void **allocation, PGPSize newAllocationSize,
					PGPMemoryMgrFlags flags );
PGPError 	pgpContextMemFree( PGPContextRef context,
					void *allocation );

					
#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpContext_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
