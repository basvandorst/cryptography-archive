/*____________________________________________________________________________
	Copyright (C) 1996 Pretty Good Privacy, Inc.
	All rights reserved.
____________________________________________________________________________*/

#include "MacInterrupts.h"

/*____________________________________________________________________________
	This is deliberately not an inline because as an inline MetroWerks
	optimizes out the memory access.
____________________________________________________________________________*/
	Boolean
TestAndSet( volatile Boolean *value )
	{
	Boolean	result;
	
	EnterCriticalSection();
	result	= *value;
	*value	= TRUE;
	ExitCriticalSection();
	
	return( result );
	}





