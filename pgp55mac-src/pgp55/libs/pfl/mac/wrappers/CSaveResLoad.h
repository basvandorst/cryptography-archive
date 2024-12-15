/*____________________________________________________________________________
	CSaveResLoad.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSaveResLoad.h,v 1.4 1997/09/18 01:35:03 lloyd Exp $
____________________________________________________________________________*/

#pragma once

#include <LowMem.h>

class CSaveResLoad
{
protected:
	SInt8	mSaveResLoad;
	
public:
			CSaveResLoad( void )		{ mSaveResLoad	= LMGetResLoad(); }
	virtual ~CSaveResLoad()				{ LMSetResLoad( mSaveResLoad ); }
};

