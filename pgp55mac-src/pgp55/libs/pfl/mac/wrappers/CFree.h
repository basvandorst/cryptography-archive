/*____________________________________________________________________________
	CFree.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CFree.h,v 1.3 1997/09/18 01:35:01 lloyd Exp $
____________________________________________________________________________*/
#pragma once


class CFree
{
protected:
	void *	mMemory;
	
public:
			CFree( void * memory )		{ mMemory	= memory; }
	virtual ~CFree()
	{
		/* subclass should provide destructor which frees */
	}
			
	 void	Forget( void )				{ mMemory	= nil;}
};

