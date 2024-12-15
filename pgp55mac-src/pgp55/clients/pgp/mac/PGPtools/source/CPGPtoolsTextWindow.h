/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "CPGPtoolsWindow.h"

class CPGPtoolsTextWindow	: public CPGPtoolsWindow
{
public:
	enum { class_ID = 'TWin' };

						CPGPtoolsTextWindow(LStream *inStream);
						~CPGPtoolsTextWindow(void);
	
	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam);
	OSStatus			SetText(LogicalAddress theText, UInt32 textLength);
	
protected:

	virtual	void		FinishCreateSelf(void);
};

