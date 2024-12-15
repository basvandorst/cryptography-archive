/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPatcher.cp,v 1.3.10.1 1997/12/05 22:14:51 mhw Exp $
____________________________________________________________________________*/

#include "CPatcher.h"



CPatcher::CPatcher()
	:	mOldRoutine(nil)
{
}


CPatcher::~CPatcher()
{
}



	void
CPatcher::Patch(
	SInt16			inTrapNum,
	ProcInfoType	inProcInfo,
	ProcPtr			inProc)
{
	UniversalProcPtr	theNewRoutine;

#if	GENERATINGCFM
	theNewRoutine =
		(UniversalProcPtr) ::NewRoutineDescriptor(	(ProcPtr) inProc,
													inProcInfo,
													GetCurrentArchitecture());
	PGPThrowIfNil_(theNewRoutine);
#else
	theNewRoutine = (UniversalProcPtr) inProc;
#endif

	if (inTrapNum & 0x0800){
		mOldRoutine = ::GetToolTrapAddress(inTrapNum);
		::SetToolTrapAddress(	(UniversalProcPtr) theNewRoutine,
								inTrapNum);
	} else {
		mOldRoutine = ::GetOSTrapAddress(inTrapNum);
		::SetOSTrapAddress(	(UniversalProcPtr) theNewRoutine,
							inTrapNum);
	}
}
