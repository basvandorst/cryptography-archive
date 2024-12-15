/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerSizeWindowPatch.h,v 1.2.10.1 1997/12/05 22:13:56 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CSizeWindowPatcher.h"
#include "PGPemailerGlobals.h"

class CemailerSizeWindowPatch	:	public	CSizeWindowPatcher {
public:
								CemailerSizeWindowPatch(SGlobals & inGlobals);
	virtual						~CemailerSizeWindowPatch();

protected:
	SGlobals &					mGlobals;
	
	virtual void				NewSizeWindow(WindowRef theWindow, short w,
									short h, Boolean fUpdate);
};
