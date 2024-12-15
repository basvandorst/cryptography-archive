/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: StSuspendAESpecialHandler.cp,v 1.2.10.1 1997/12/05 22:15:25 mhw Exp $
____________________________________________________________________________*/

#include "StSuspendAESpecialHandler.h"



StSuspendAESpecialHandler::StSuspendAESpecialHandler(
	AEKeyword	inFunctionClass,
	Boolean		inIsSysHandler)
		: mFunctionClass(inFunctionClass), mHandler(nil),
		  mIsSysHandler(inIsSysHandler)
{
	Suspend();
}



StSuspendAESpecialHandler::~StSuspendAESpecialHandler()
{
	Resume();
}
	


	void
StSuspendAESpecialHandler::Suspend()
{
	::AEGetSpecialHandler(	mFunctionClass,
							&mHandler,
							mIsSysHandler);
	if (mHandler != nil) {
		::AERemoveSpecialHandler(	mFunctionClass,
									mHandler,
									mIsSysHandler);
	}
}



	void
StSuspendAESpecialHandler::Resume()
{
	if (mHandler != nil) {
		::AEInstallSpecialHandler(	mFunctionClass,
									mHandler,
									mIsSysHandler);
	}
}
