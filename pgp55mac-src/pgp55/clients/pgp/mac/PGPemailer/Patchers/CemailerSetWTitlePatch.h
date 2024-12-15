/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CemailerSetWTitlePatch.h,v 1.2.10.1 1997/12/05 22:13:55 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CSetWTitlePatcher.h"
#include "PGPemailerGlobals.h"

class CemailerSetWTitlePatch	:	public	CSetWTitlePatcher {
public:
						CemailerSetWTitlePatch(SGlobals & inGlobals);
	virtual				~CemailerSetWTitlePatch();
	
protected:
	SGlobals &			mGlobals;
	Str255				mOutgoingMessageString;
	Str255				mIncomingMessageString;
	
	virtual void		NewSetWTitle(WindowRef theWindow,
							ConstStr255Param title);
};
