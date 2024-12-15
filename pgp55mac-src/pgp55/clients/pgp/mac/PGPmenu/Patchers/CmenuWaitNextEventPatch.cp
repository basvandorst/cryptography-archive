/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuWaitNextEventPatch.cp,v 1.2.10.1 1997/12/05 22:14:22 mhw Exp $
____________________________________________________________________________*/

#include "StSaveCurResFile.h"
#include "CCopyPasteHack.h"

#include "CmenuWaitNextEventPatch.h"



CmenuWaitNextEventPatch::CmenuWaitNextEventPatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals)
{
}



CmenuWaitNextEventPatch::~CmenuWaitNextEventPatch()
{
}



	void
CmenuWaitNextEventPatch::PreWaitNextEvent()
{
#if ! PGP_FINDERMENU
	if (CCopyPasteHack::GetCopyPasteHack() != nil) {
		StSaveCurResFile	saveResFile;
		LFile				thePlugFile(mGlobals.plugSpec);	
		
		thePlugFile.OpenResourceFork(fsRdPerm);
		CCopyPasteHack::GetCopyPasteHack()->PreWaitNextEvent(sEventParams);
		thePlugFile.CloseResourceFork();
	}
#endif
}



	void
CmenuWaitNextEventPatch::HandleKeyDown()
{
#if ! PGP_FINDERMENU
	if (CCopyPasteHack::GetCopyPasteHack() != nil) {
		StSaveCurResFile	saveResFile;
		LFile				thePlugFile(mGlobals.plugSpec);	
		
		thePlugFile.OpenResourceFork(fsRdPerm);
		CCopyPasteHack::GetCopyPasteHack()->HandleKeyDown(sEventParams);
		thePlugFile.CloseResourceFork();
	}
#endif
}
