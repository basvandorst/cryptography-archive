/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	Mac-specific functions to plug into the PGPMemoryMgr:
		pgpCreateStandardMemoryMgr
		pgpDisposeStandardMemoryMgrUserValue
	
	$Id: pgpMemoryMgrMac.c,v 1.5 1999/03/10 02:53:21 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pgpMemoryMgrPriv.h"

#include "MacEnvirons.h"




	static void
sLockMemory(
	void *			mem,
	PGPSize			numBytes )
{
	if ( VirtualMemoryIsOn()  )
	{
		HoldMemory( mem, numBytes );
	}
}

	static void
sUnlockMemory(
	void *	mem,
	PGPSize	numBytes )
{
	if ( VirtualMemoryIsOn()  )
	{
		UnholdMemory( mem, numBytes );
	}
}




/*____________________________________________________________________________
	The default memory allocator. Note that memory mgr will be
	NULL when allocating the memory mgr itself.
____________________________________________________________________________*/
	static void *
sMacMemoryAllocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		userValue,
	PGPSize 			allocationSize,
	PGPMemoryMgrFlags	flags )
{
	(void) mgr;
	(void) flags;
	(void) userValue;

	return( NewPtr( allocationSize ) );
}



/*____________________________________________________________________________
	This is the default PFL memory deallocator. Note that mgr will
	be NULL when deallocating the PFLContext structure itself.
____________________________________________________________________________*/
	static PGPError
sMacMemoryDeallocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		userValue,
	void *				allocation,
	PGPSize				allocationSize )
{
	(void) mgr;
	(void) allocationSize;
	(void) userValue;

	DisposePtr( (Ptr)allocation );
	
	return( kPGPError_NoErr );
}



/*____________________________________________________________________________
	This is the default PFL memory reallocator. 
____________________________________________________________________________*/
	static PGPError
sMacMemoryReallocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		userValue,
	void **				allocation,
	PGPSize 			newAllocationSize,
	PGPMemoryMgrFlags	flags,
	PGPSize				existingSize )
{
	PGPError	err	= kPGPError_NoErr;
	Ptr			ptr	= (Ptr)*allocation;
	
	/* we aren't required to support NULL ptr semantics */
	pgpAssert( IsntNull( *allocation ) );
	
	(void) mgr;
	(void) flags;
	(void) existingSize;
	(void) userValue;

	SetPtrSize( ptr, newAllocationSize );
	if ( GetPtrSize( ptr ) != newAllocationSize )
	{
		void *	newPtr;
		
		/* get a new pointer and copy old to it */
		newPtr	= NewPtr( newAllocationSize );
		if ( IsntNull( newPtr ) )
		{
			pgpCopyMemory( ptr, newPtr,
				pgpMin( existingSize, newAllocationSize ) );
			pgpDebugWhackMemory( ptr, existingSize );
			DisposePtr( ptr );
			ptr	= (Ptr)newPtr;
		}
		else
		{
			err	= kPGPError_OutOfMemory;
		}
	}
	
	*allocation	= ptr;
	
	return( err );
}



	static void *
sMacSecureMemoryAllocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		userValue,
	PGPSize 			allocationSize,
	PGPMemoryMgrFlags	flags,
	PGPBoolean *		isNonPageable )
{
	void *		ptr	= NULL;
	
	pgpAssert( IsntNull( isNonPageable ) );
	(void) mgr;
	(void) flags;
	(void) userValue;

	*isNonPageable	= TRUE;
	
	ptr	= NewPtrSys( allocationSize );
	if ( IsntNull( ptr ) )
	{
		sLockMemory( ptr, allocationSize );
	}
	
	return( ptr );
}




/*____________________________________________________________________________
	The default secure memory deallocator.
____________________________________________________________________________*/
	static PGPError
sMacSecureMemoryDeallocationProc(
	PGPMemoryMgrRef	mgr,
	PGPUserValue	userValue,
	void *			allocation,
	PGPSize			allocationSize,
	PGPBoolean		wasLocked )
{
	(void) mgr;
	(void) allocationSize;
	(void) userValue;

	pgpAssert( wasLocked );
	if ( wasLocked )
	{
		sUnlockMemory( allocation, allocationSize );
	}
	
	DisposePtr( (Ptr)allocation );
	
	return( kPGPError_NoErr );
}



	PGPError
pgpCreateStandardMemoryMgr( PGPMemoryMgrRef *newMemoryMgr )
{
	PGPError				err	= kPGPError_NoErr;
	PGPNewMemoryMgrStruct	custom;
	
	PGPValidatePtr( newMemoryMgr );
	*newMemoryMgr	= NULL;
	
	pgpClearMemory( &custom, sizeof( custom ) );
	custom.customValue	= NULL;
	
	custom.sizeofStruct		= sizeof( custom );
	custom.allocProc		= sMacMemoryAllocationProc;
	custom.reallocProc		= sMacMemoryReallocationProc;
	custom.deallocProc		= sMacMemoryDeallocationProc;
	
	custom.secureAllocProc		= sMacSecureMemoryAllocationProc;
	custom.secureDeallocProc	= sMacSecureMemoryDeallocationProc;
	
	err	= PGPNewMemoryMgrCustom( &custom, newMemoryMgr );
	
	return( err );
}




	PGPError
pgpDisposeStandardMemoryMgrUserValue( PGPUserValue userValue )
{
	/* we don't maintain any state */
	(void)userValue;
	return( kPGPError_NoErr );
}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
