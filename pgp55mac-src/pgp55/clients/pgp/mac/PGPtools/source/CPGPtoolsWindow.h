/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

// Base class for all application windows

#pragma once

#include <LWindow.h>

class CPGPtoolsWindow : public LWindow
{
public:
	enum 				{ class_ID = 'AWin' };

						CPGPtoolsWindow(LStream *inStream);
	virtual				~CPGPtoolsWindow(void);

	virtual void		DoSetBounds(const Rect &inBounds);
	virtual void		DoSetPosition(Point inPosition);
	virtual void		SendAESetBounds(Rect *inBounds, Boolean inExecuteAE);
	virtual void		SendAESetZoom(void);

protected:

	virtual	Boolean		WindowOnVisibleDevice(void);
	
private:

	virtual	Boolean		PointIsOnVisibleDevice(Point globalPt);
};