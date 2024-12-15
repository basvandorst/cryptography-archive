/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacFonts.cp,v 1.1 1997/07/03 12:22:42 wprice Exp $
____________________________________________________________________________*/
#include "MacFonts.h"

	void
USystemFontUtils::FetchSystemFont(Int16 &outFont, Int16 &outSize)
{
	outFont = LMGetSysFontFam();
	outSize = LMGetSysFontSize();
}

	void
USystemFontUtils::ResetSystemFont()
{
	GrafPtr	savePort;
	GrafPtr wMgrPort;

	::GetWMgrPort(&wMgrPort);
	::GetPort(&savePort);
	::SetPort(wMgrPort);
	::TextFont(0);
	::TextSize(0);
	::SetPort(savePort);
}

	void
USystemFontUtils::SetSystemFont(Int16 inFont, Int16 inSize)
{
	if (LMGetSysFontFam() != inFont || LMGetSysFontSize() != inSize)
	{
		LMSetSysFontFam(inFont);
		LMSetSysFontSize(inSize);
		LMSetLastSPExtra(-1L);
	}
}

StSystemFontState::StSystemFontState(Int16 inNewFont, Int16 inNewSize)
{
	USystemFontUtils::ResetSystemFont();
	USystemFontUtils::FetchSystemFont(mSaveFont, mSaveSize);
	USystemFontUtils::SetSystemFont(inNewFont, inNewSize);
}

StSystemFontState::~StSystemFontState()
{
	USystemFontUtils::SetSystemFont(mSaveFont, mSaveSize);
}

