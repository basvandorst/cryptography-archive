/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPutScrapPatcher.cp,v 1.2.10.1 1997/12/05 22:14:54 mhw Exp $
____________________________________________________________________________*/

#include <SetupA4.h>

#include "CPutScrapPatcher.h"



// Define statics
CPutScrapPatcher *	CPutScrapPatcher::sPutScrapPatcher = nil;

CPutScrapPatcher::CPutScrapPatcher()
{
	// Current version will only install one patch
	if (sPutScrapPatcher == nil) {
		Patch(_PutScrap, kProcInfo, (ProcPtr) PutScrapCallback);
		sPutScrapPatcher = this;
	}
}



CPutScrapPatcher::~CPutScrapPatcher()
{
}



	long
CPutScrapPatcher::NewPutScrap(
	long	length,
	ResType	theType,
	void *	source)
{
	// default just calls through to the original
	return OldPutScrap(	length,
						theType,
						source);
}



	long
CPutScrapPatcher::OldPutScrap(
	long	length,
	ResType	theType,
	void *	source)
{
#if GENERATINGCFM
	return (long) ::CallUniversalProc(	mOldRoutine,
										kProcInfo,
										length,
										theType,
										source);
#else
	return ((PutScrapProc) mOldRoutine)(	length,
											theType,
											source);
#endif
}



	pascal long
CPutScrapPatcher::PutScrapCallback(
	long	length,
	ResType	theType,
	void *	source)
{
	EnterCallback();

	long	result = 0;
	
	try {
		result = sPutScrapPatcher->NewPutScrap(	length,
												theType,
												source);
	}
	
	catch(...) {
	}
	
	ExitCallback();
	return result;
}
