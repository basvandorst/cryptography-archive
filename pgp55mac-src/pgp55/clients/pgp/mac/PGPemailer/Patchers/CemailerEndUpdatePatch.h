/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerEndUpdatePatch.h,v 1.2.10.1 1997/12/05 22:13:54 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CEndUpdatePatcher.h"
#include "PGPemailerGlobals.h"

class CemailerEndUpdatePatch	:	public	CEndUpdatePatcher {
public:
									CemailerEndUpdatePatch(
											SGlobals & inGlobals);
	virtual							~CemailerEndUpdatePatch();

protected:
	SGlobals &					mGlobals;
	
	virtual void				NewEndUpdate(WindowRef theWindow);
};
