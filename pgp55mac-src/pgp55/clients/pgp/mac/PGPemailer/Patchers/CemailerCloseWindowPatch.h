/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerCloseWindowPatch.h,v 1.2.10.1 1997/12/05 22:13:52 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CCloseWindowPatcher.h"
#include "PGPemailerGlobals.h"

class CemailerCloseWindowPatch	:	public	CCloseWindowPatcher {
public:
									CemailerCloseWindowPatch(
											SGlobals & inGlobals);
	virtual							~CemailerCloseWindowPatch();

protected:
	SGlobals &					mGlobals;
	
	virtual void				NewCloseWindow(WindowRef theWindow);
};
