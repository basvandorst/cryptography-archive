/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGAFix.cp,v 1.1 1997/08/20 12:33:36 wprice Exp $
____________________________________________________________________________*/
#include "CGAFix.h"


CGAPopupFix::CGAPopupFix(LStream *inStream)
	: LGAPopup( inStream )
{
}

CGAPopupFix::~CGAPopupFix()
{
}

	void
CGAPopupFix::DrawSelf()
{
	// This function overrides a stupid bug in PowerPlant's
	// silly LGAPopup class which sets a new color without
	// saving the current port colors
	
	StColorState	savedColors;
	
	LGAPopup::DrawSelf();
}

CGACheckboxFix::CGACheckboxFix(LStream *inStream)
	: LGACheckbox( inStream )
{
}

CGACheckboxFix::~CGACheckboxFix()
{
}

	void
CGACheckboxFix::DrawSelf()
{
	// This function overrides a stupid bug in PowerPlant's
	// silly LGACheckbox class which sets a new color without
	// saving the current port colors
	
	StColorState	savedColors;
	
	LGACheckbox::DrawSelf();
}

