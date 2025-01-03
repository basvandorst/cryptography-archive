/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	Generic/Unix functions to plug into the PGPMemoryMgr:
		pgpCreateStandardMemoryMgr
		pgpDisposeStandardMemoryMgrUserValue
		
	$Id: pgpMemoryMgrStd.c,v 1.7 1999/03/10 02:53:24 heller Exp $
____________________________________________________________________________*/
#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pgpMemoryMgr.h"
#include "pgpMemoryMgrPriv.h"

#if PGP_MACINTOSH
#include "MacEnvirons.h"
#endif

#if 0
#if PGP_MACINTOSH || PGP_WIN32
#error use platform-specific variants
#endif
#endif


/*____________________________________________________________________________
	IMPORANT NOTE:
	
	Even though we provide sLockMemory() and sUnlockMemory() for mac and win32
	it is expected that the generic implementation will not be used since 
	there are specific implementations for those platforms.
	
____________________________________________________________________________*/
#if PGP_MACINTOSH	/* [ */

	static void
sLockMemory(
	void *			mem,
	PGPSize			numBytes,
	PGPBoolean *	isNonPageable )
{
	*isNonPageable	= TRUE;
	
	if ( VirtualMemoryIsOn()  )
	{
		HoldMemory( mem, numBytes );
	}
}

	static void
sUnlockMemory(
	void *		mem,
	PGPSize		numBytes,
	PGPBoolean	nonPageable )
{
	if ( nonPageable && VirtualMemoryIsOn()  )
	{
		UnholdMemory( mem, numBytes );
	}
}

#elif PGP_WIN32 	/* ] [ */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

	static void
sLockMemory(
	void *			mem,
	PGPSize			numBytes,
	PGPBoolean *	isNonPageable )
{
	*isNonPageable	= FALSE;
	/* this doesn't actually prevent paging in all cases */
	VirtualLock ( mem, numBytes );
}

	static void
sUnlockMemory(
	void *		mem,
	PGPSize		numBytes.
	PGPBoolean	nonPageable )
{
	/* we always call VirtualLock() so always call VirtualUnlock() */
	(void)wasLocked;
	
	VirtualUnlock ( mem, numBytes );
}

#elif PGP_UNIX 	/* ] [ */


/* Code borrowed from Peter Gutmann's cryptlib for secure memory locking */

#if defined( __osf__ )
  #include <sys/mman.h>
#elif defined( sun )
  #include <sys/types.h>
  #include <sys/mman.h>
#else
  int mlock( void *address, size_t length );
  int munlock( void *address, size_t length );
#endif /* Unix-variant-specific includes */

/*____________________________________________________________________________
	Unix version
____________________________________________________________________________*/

	static void
sLockMemory(
	void *			mem,
	PGPSize			numBytes,
	PGPBoolean *	isNonPageable )
{
	pgpAssert( IsntNull( isNonPageable ) );

	(void)mem;
	(void)numBytes;
	/* mlock call taken from Peter Gutmann's cryptlib */
	/* Under many Unix variants the SYSV/POSIX mlock() call can be
	   used, but only by the superuser.  OSF/1 has mlock(), but
	   this is defined to the nonexistant memlk() so we need to
	   special-case it out.  Aches and PHUX don't even pretend to
	   have mlock().  Many systems also have plock(), but this is
	   pretty crude since it locks all data, and also has various
	   other shortcomings.  Finally, PHUX has datalock(), which is
	   just a plock() variant */
#if !( defined( __osf__ ) || defined( _AIX ) || defined( __hpux ) || \
defined( _M_XENIX ) )
	*isNonPageable = (mlock( mem, numBytes ) == 0);
#else
	*isNonPageable = FALSE;
#endif /* !( __osf__ || _AIX || __hpux || _M_XENIX ) */

}


/*____________________________________________________________________________
	Unix version
____________________________________________________________________________*/
	static void
sUnlockMemory(
	void *		mem,
	PGPSize		numBytes,
	PGPBoolean	nonPageable )
{
	(void)mem;
	(void)numBytes;
	
	if ( nonPageable )
	{
		(void)munlock( mem,  numBytes );
	}
}



#else

#error unknown platform

#endif	/* ] */




/*____________________________________________________________________________
	The default memory allocator. Note that memory mgr will be
	NULL when allocating the memory mgr itself.
____________________________________________________________________________*/
	static void *
sGenericMemoryAllocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	PGPSize 			allocationSize,
	PGPMemoryMgrFlags	flags )
{
	(void) mgr;
	(void) flags;
	(void) customValue;

	return( malloc( allocationSize ) );
}



/*____________________________________________________________________________
	This is the default PFL memory deallocator. Note that mgr will
	be NULL when deallocating the PFLContext structure itself.
____________________________________________________________________________*/
	static PGPError
sGenericMemoryDeallocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	void *				allocation,
	PGPSize				allocationSize )
{
	(void) mgr;
	(void) allocationSize;
	(void) customValue;

	free( allocation );
	
	return( kPGPError_NoErr );
}


/*____________________________________________________________________________
	Generic memory reallocator. 
____________________________________________________________________________*/
	static PGPError
sGenericMemoryReallocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	void **				allocation,
	PGPSize 			newAllocationSize,
	PGPMemoryMgrFlags	flags,
	PGPSize				existingSize )
{
	PGPError	err;
	void *		newPtr	= NULL;
	void *		oldPtr	= *allocation;
	
	(void) mgr;
	(void) flags;
	(void) existingSize;
	(void) customValue;

	newPtr = realloc( oldPtr, newAllocationSize );
	if ( IsNull( newPtr ) )
	{
		err	= kPGPError_OutOfMemory;
	}
	else
	{
		err	= kPGPError_NoErr;
		*allocation	= newPtr;
	}
	
	return( err );
}




	static void *
sGenericSecureMemoryAllocationProc(
	PGPMemoryMgrRef		mgr,
	PGPUserValue		customValue,
	PGPSize 			allocationSize,
	PGPMemoryMgrFlags	flags,
	PGPBoolean *		isSecure )
{
	void *		ptr	= NULL;
	
	(void) mgr;
	(void) flags;
	(void) customValue;

	*isSecure	= FALSE;
	
	ptr	= sGenericMemoryAllocationProc( mgr,
				customValue, allocationSize, flags );
	if ( IsntNull( ptr ) )
	{
		sLockMemory( ptr, allocationSize, isSecure );
	}
	
	return( ptr );
}




/*____________________________________________________________________________
	Generic secure memory deallocator.
____________________________________________________________________________*/
	static PGPError
sGenericSecureMemoryDeallocationProc(
	PGPMemoryMgrRef	mgr,
	PGPUserValue	customValue,
	void *			allocation,
	PGPSize			allocationSize,
	PGPBoolean		wasLocked )
{
	PGPError	err	= kPGPError_NoErr;
	
	(void) mgr;
	(void) allocationSize;
	(void) customValue;

	if ( wasLocked )
	{
		sUnlockMemory( allocation, allocationSize, wasLocked );
	}
	
	err	= sGenericMemoryDeallocationProc( mgr, customValue,
				allocation, allocationSize);
	
	return( err );
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
	custom.allocProc		= sGenericMemoryAllocationProc;
	custom.reallocProc		= sGenericMemoryReallocationProc;
	custom.deallocProc		= sGenericMemoryDeallocationProc;
	
	custom.secureAllocProc		= sGenericSecureMemoryAllocationProc;
	custom.secureDeallocProc	= sGenericSecureMemoryDeallocationProc;
	
	err	= PGPNewMemoryMgrCustom( &custom, newMemoryMgr );
	
	return( err );
}


	PGPError
pgpDisposeStandardMemoryMgrUserValue( PGPUserValue customValue )
{
	/* we don't maintain any state */
	(void)customValue;
	return( kPGPError_NoErr );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
