/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerWaitNextEventPatch.h,v 1.2.10.1 1997/12/05 22:13:57 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CWaitNextEventDispatcher.h"
#include "PGPemailerGlobals.h"

class CemailerWaitNextEventPatch : public CWaitNextEventDispatcher {
public:
					CemailerWaitNextEventPatch(SGlobals & inGlobals);
	virtual			~CemailerWaitNextEventPatch();
	
protected:
	Boolean			mInitialized;
	SGlobals &		mGlobals;
	
	void			InitializePlug();
	
	virtual void	DispatchEvent();

	virtual void	HandleMouseDown();
};
