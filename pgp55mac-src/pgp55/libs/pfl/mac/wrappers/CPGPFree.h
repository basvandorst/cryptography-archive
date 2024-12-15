/*____________________________________________________________________________
	CPGPFree.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPFree.h,v 1.4 1997/09/18 01:35:02 lloyd Exp $
____________________________________________________________________________*/
#include "CFree.h"


class CPGPFree : public CFree
{
public:			
				CPGPFree( void *memory ) : CFree( memory ) {}

	virtual		~CPGPFree( void )
	{
		if ( mMemory != NULL ) pgpFree( mMemory );
	}
};


