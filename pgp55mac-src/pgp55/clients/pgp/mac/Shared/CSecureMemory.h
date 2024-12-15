/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSecureMemory.h,v 1.6 1997/10/16 19:30:51 heller Exp $
____________________________________________________________________________*/
#ifndef Included_CSecureMemory_h	/* [ */
#define Included_CSecureMemory_h


#include "pgpMem.h"
#include "pgpUtilities.h"

#include "MacSecureMemory.h"

/*____________________________________________________________________________
	Allocates a buffer of the specified size which is held in memory.
____________________________________________________________________________*/
class CSecureMemory
{
public:
	void *		mMemory;
	PGPSize		mSize;
	
	CSecureMemory( PGPContextRef context, PGPSize size )
	{
		mSize	= size;
		mMemory	= PGPNewSecureData( context, size, NULL );
		pgpAssert( IsntNull( mMemory ) );
	}
	
	~CSecureMemory( void )
	{
		if ( IsntNull( mMemory ) )
		{
			PGPFreeData( mMemory );
			mMemory	= NULL;
		}
	}
};


#endif /* ] Included_CSecureMemory_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
