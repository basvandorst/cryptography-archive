/*____________________________________________________________________________
	MacNew.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacNew.cp,v 1.8 1997/09/18 01:34:51 lloyd Exp $
____________________________________________________________________________*/

#include "MacNew.h"
#include "pgpMem.h"

#if USE_MAC_OPERATOR_NEW
#error use 'USE_PGP_OPERATOR_NEW'
#endif


#if USE_PGP_OPERATOR_NEW	// [

#include <new>


MacNewDummy::MacNewDummy()
	{
	}

/*____________________________________________________________________________
	With the modern memory manager, NewPtr() is substantially faster
	than MetroWerks operator new().
____________________________________________________________________________*/
	void *
operator new(size_t size)
	{
	void *ptr;
	
#if MEMORY_ALLOC_MUST_BE_APP
	pgpAssertMsg( GetZone() == ApplicationZone(),
		"operator new: MEMORY_ALLOC_MUST_BE_APP");
#elif MEMORY_ALLOC_MUST_BE_SYS
	pgpAssertMsg( GetZone() == SystemZone(),
		"operator new: MEMORY_ALLOC_MUST_BE_SYS");
#elif MEMORY_ALLOC_MUST_BE_APP_OR_SYS
	pgpAssertMsg( GetZone() == ApplicationZone() || GetZone() == SystemZone(),
		"operator new: MEMORY_ALLOC_MUST_BE_APP_OR_SYS");
#endif

	ptr	= pgpAlloc( size );
	
	return ptr;
	}
	
	
	void
operator delete(void *ptr)
	{
	if ( IsntNull( ptr ) )
		{
		pgpFree( ptr );
		}
	}



#endif	// ]