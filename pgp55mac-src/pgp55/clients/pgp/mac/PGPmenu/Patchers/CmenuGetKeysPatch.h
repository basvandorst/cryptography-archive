/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuGetKeysPatch.h,v 1.1.10.1 1997/12/05 22:14:16 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "PGPmenuGlobals.h"
#include "CGetKeysPatcher.h"

class CmenuGetKeysPatch : public CGetKeysPatcher {
public:
						CmenuGetKeysPatch(SGlobals & inGlobals);
	virtual				~CmenuGetKeysPatch();
	
protected:
	SGlobals &			mGlobals;
	
	virtual void		NewGetKeys(KeyMap theKeys);
};
