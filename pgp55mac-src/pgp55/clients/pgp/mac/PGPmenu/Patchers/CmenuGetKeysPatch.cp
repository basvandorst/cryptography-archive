/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CmenuGetKeysPatch.cp,v 1.2.10.1 1997/12/05 22:14:15 mhw Exp $
____________________________________________________________________________*/

#include "CCopyPasteHack.h"

#include "CmenuGetKeysPatch.h"



CmenuGetKeysPatch::CmenuGetKeysPatch(
	SGlobals &	inGlobals)
		: mGlobals(inGlobals)
{
}



CmenuGetKeysPatch::~CmenuGetKeysPatch()
{
}



	void
CmenuGetKeysPatch::NewGetKeys(
	KeyMap	theKeys)
{
	OldGetKeys(theKeys);
	
	try {
		if (CCopyPasteHack::GetCopyPasteHack() != nil) {
			CCopyPasteHack::GetCopyPasteHack()->GetKeys(theKeys);
		}
	}
	
	catch (...) {
	}
}
