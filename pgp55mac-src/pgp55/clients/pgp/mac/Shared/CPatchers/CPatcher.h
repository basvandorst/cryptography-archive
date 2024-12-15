/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPatcher.h,v 1.2.10.1 1997/12/05 22:14:52 mhw Exp $
____________________________________________________________________________*/

#pragma once 

class CPatcher {
public:
						CPatcher();
	virtual				~CPatcher() = 0;
	
	virtual void		Patch(SInt16 inTrapNum, ProcInfoType inProcInfo,
							ProcPtr inProc);
	
protected:
	UniversalProcPtr	mOldRoutine;
	
/*	Subclasses need to define the following:
	static Subclass *	sPatcherObject;
	
	virtual somereturn		VirtualCallback();
	virtual someReturn		CallOriginal();
	static someReturn		TheCallback(someParams)
								{sPatcherObject->virtualCallback; }
*/	
};
