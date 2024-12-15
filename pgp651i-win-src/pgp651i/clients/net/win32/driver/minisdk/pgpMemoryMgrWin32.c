/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	Win32-specific functions to plug into the PGPMemoryMgr:
		pgpCreateStandardMemoryMgr
		pgpDisposeStandardMemoryMgrUserValue
	
	$Id: pgpMemoryMgrWin32.c,v 1.4 1999/03/10 02:53:39 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLErrors.h"
#include "pgpMem.h"
#include "pgpMemoryMgrPriv.h"

#ifndef USERLAND_TEST
# include <ndis.h>
#endif

static NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress = 
								NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);

	static void *
sNdisMemoryAllocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	PGPSize 			allocationSize,
	PGPMemoryMgrFlags	flags )
{
	void *allocation = NULL;

	(void) mgr;
	(void) flags;
	(void) customValue;

	NdisAllocateMemory(&allocation, allocationSize, 0, 
		HighestAcceptableAddress);

	return allocation;
}


	static PGPError
sNdisMemoryDeallocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	void *				allocation,
	PGPSize				allocationSize )
{
	(void) mgr;
	(void) allocationSize;
	(void) customValue;

	NdisFreeMemory(allocation, allocationSize, 0);
	return( kPGPError_NoErr );
}


	static PGPError
sNdisMemoryReallocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	void **				allocation,
	PGPSize 			newAllocationSize,
	PGPMemoryMgrFlags	flags,
	PGPSize				existingSize )
{
	PGPError	err;
	void *		oldPtr	= *allocation;
	void *		newPtr	= NULL;
	PGPSize		copySize;
	
	(void) mgr;
	(void) flags;
	(void) customValue;

	NdisAllocateMemory(&newPtr, newAllocationSize, 0, 
		HighestAcceptableAddress);

	if ( IsNull( newPtr ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	else
	{
		err	= kPGPError_NoErr;
		if (existingSize < newAllocationSize)
			copySize = existingSize;
		else
			copySize = newAllocationSize;
		
		NdisMoveMemory(newPtr, oldPtr, copySize);
		NdisFreeMemory(oldPtr, existingSize, 0);
	}
	
	*allocation = newPtr;
	return( err );
}


	static void *
sNdisSecureMemoryAllocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue 		customValue,
	PGPSize 			allocationSize,
	PGPMemoryMgrFlags	flags,
	PGPBoolean *		isNonPageable )
{
	*isNonPageable	= FALSE;
	
	return sNdisMemoryAllocationProc(mgr, customValue, allocationSize, flags);
}


	static PGPError
sNdisSecureMemoryDeallocationProc(
	PGPMemoryMgrRef	mgr,
	PGPUserValue 	customValue,
	void *			allocation,
	PGPSize			allocationSize,
	PGPBoolean		nonPageable)
{
	(void) nonPageable;
	
	return sNdisMemoryDeallocationProc(mgr, customValue, allocation, 
		allocationSize);
}


	PGPError
pgpCreateStandardMemoryMgr( PGPMemoryMgrRef *newMemoryMgr )
{
	PGPError				err	= kPGPError_NoErr;
	PGPNewMemoryMgrStruct	custom;
	
	PGPValidatePtr( newMemoryMgr );
	*newMemoryMgr	= NULL;
	
	pgpClearMemory( &custom, sizeof( custom ) );
	
	custom.customValue		= 0;
	custom.sizeofStruct		= sizeof( custom );

	custom.allocProc		= sNdisMemoryAllocationProc;
	custom.reallocProc		= sNdisMemoryReallocationProc;
	custom.deallocProc		= sNdisMemoryDeallocationProc;
	
	custom.secureAllocProc		= sNdisSecureMemoryAllocationProc;
	custom.secureDeallocProc	= sNdisSecureMemoryDeallocationProc;
	
	err	= PGPNewMemoryMgrCustom( &custom, newMemoryMgr );
	
	return( err );
}


	PGPError
pgpDisposeStandardMemoryMgrUserValue( PGPUserValue customValue )
{
	return( kPGPError_NoErr );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
