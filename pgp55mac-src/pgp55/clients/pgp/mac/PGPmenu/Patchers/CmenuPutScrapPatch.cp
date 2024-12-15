/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuPutScrapPatch.cp,v 1.2.10.1 1997/12/05 22:14:19 mhw Exp $
____________________________________________________________________________*/

#include "CCopyPasteHack.h"
#include "StSaveCurResFile.h"

#include "CmenuPutScrapPatch.h"



CmenuPutScrapPatch::CmenuPutScrapPatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals)
{
}



CmenuPutScrapPatch::~CmenuPutScrapPatch()
{
}



	long
CmenuPutScrapPatch::NewPutScrap(
	long	length,
	ResType	theType,
	void *	source)
{
	SInt32			result;
	Boolean			callThrough = true;
	
	try {
		if (CCopyPasteHack::GetCopyPasteHack() != nil) {
			StSaveCurResFile	saveResFile;
			LFile				thePlugFile(mGlobals.plugSpec);	
			
			thePlugFile.OpenResourceFork(fsRdPerm);
			if (CCopyPasteHack::GetCopyPasteHack()->PutScrap(length,
			theType, source)) {
				result = noErr;
				callThrough = false;
			}
			thePlugFile.CloseResourceFork();
		}
	}
	
	catch (...) {
	}
	
	if (callThrough) {
		result = OldPutScrap(	length,
								theType,
								source);
	}
	
	return result;
}
