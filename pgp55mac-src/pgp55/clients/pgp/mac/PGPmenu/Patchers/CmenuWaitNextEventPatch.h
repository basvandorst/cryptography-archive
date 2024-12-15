/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuWaitNextEventPatch.h,v 1.1.10.1 1997/12/05 22:14:22 mhw Exp $
____________________________________________________________________________*/

#pragma once 


#include "PGPmenuGlobals.h"
#include "CWaitNextEventDispatcher.h"

class CmenuWaitNextEventPatch : public CWaitNextEventDispatcher {
public:
					CmenuWaitNextEventPatch(SGlobals & inGlobals);
	virtual			~CmenuWaitNextEventPatch();
	
protected:
	SGlobals &		mGlobals;
	
	virtual void	PreWaitNextEvent();
	virtual void	HandleKeyDown();
};
