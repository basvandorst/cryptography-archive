/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include <LTextEdit.h>

class CDragTextEdit : public LTextEdit
{
public:
	enum { class_ID = 'DTxt' };

						CDragTextEdit(LStream *inStream);
	virtual				~CDragTextEdit(void);

	virtual void		Click(SMouseDownEvent &inMouseDown);

protected:

	virtual void		AdjustCursorSelf(Point inPortPt,
									const EventRecord &inMacEvent);
	Boolean				PortPtInTextSelection(Point inPortPt);
};