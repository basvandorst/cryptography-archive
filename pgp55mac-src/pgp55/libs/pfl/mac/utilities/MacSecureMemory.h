/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: MacSecureMemory.h,v 1.2 1997/10/16 19:30:05 heller Exp $
____________________________________________________________________________*/
#ifndef Included_MacSecureMemory_h	/* [ */
#define Included_MacSecureMemory_h


#include "pgpMem.h"
#include "MacEnvirons.h"


/*____________________________________________________________________________
	Class to automatically clear a block of memory upon destruction/exit scope
____________________________________________________________________________*/

class CMemoryWiper
{
protected:
	void *	mMemoryToWipe;
	size_t	mNumBytes;
	
public:

	CMemoryWiper( void *memory, size_t numBytes )
			{ mMemoryToWipe	= memory;  mNumBytes = numBytes; }
			
	~CMemoryWiper()
			{ pgpClearMemory( mMemoryToWipe, mNumBytes ); }
};

/*____________________________________________________________________________
	Can be used exactly like a char[256]
____________________________________________________________________________*/
class CSecureCString256
{
public:
	char	mString[ 256 ];
	
	CSecureCString256( void )
	{
		if ( VirtualMemoryIsOn() )
		{
			HoldMemory( mString, sizeof( mString ) );
		}
		mString[ 0 ]	= '\0';
	}
	
	~CSecureCString256( void )
	{
		pgpClearMemory( mString, sizeof( mString ) );
		if ( VirtualMemoryIsOn() )
		{
			UnholdMemory( mString, sizeof( mString ) );
		}
	}
	
	operator char *( void )
	{
		return( mString );
	}
};

/*____________________________________________________________________________
	Can be used exactly like a Str255
____________________________________________________________________________*/
class CSecurePString255
{
public:
	Str255	mString;
	
	CSecurePString255( void )
	{
		if ( VirtualMemoryIsOn() )
		{
			HoldMemory( mString, sizeof( mString ) );
		}
		mString[ 0 ] = 0;
	}
	
	~CSecurePString255( void )
	{
		pgpClearMemory( mString, sizeof( mString ) );
		if ( VirtualMemoryIsOn() )
		{
			UnholdMemory( mString, sizeof( mString ) );
		}
	}
	
	operator unsigned char *( void )
	{
		return( mString );
	}
};


#endif /* ] Included_MacSecureMemory_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
