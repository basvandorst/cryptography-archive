/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: StSuspendAESpecialHandler.h,v 1.2.10.1 1997/12/05 22:15:26 mhw Exp $
____________________________________________________________________________*/

#pragma once 

class StSuspendAESpecialHandler {
public:
						StSuspendAESpecialHandler(
							AEKeyword inFunctionClass,
							Boolean inIsSysHandler = false);
	virtual				~StSuspendAESpecialHandler();
	
protected:
	AEKeyword			mFunctionClass;
	UniversalProcPtr	mHandler;
	Boolean				mIsSysHandler;

	void				Suspend();
	void				Resume();
};
