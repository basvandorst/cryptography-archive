/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "CPGPtoolsWindow.h"

class CPGPtoolsMainWindow	: public CPGPtoolsWindow
{
public:
	enum { class_ID = 'MWin' };

						CPGPtoolsMainWindow(LStream *inStream);
						~CPGPtoolsMainWindow(void);

	virtual void		DoSetZoom(Boolean inZoomToStdState);
	void				UpdateButtonsState(void);
	
protected:

	virtual void		FinishCreateSelf(void);
	
	Boolean				mIsSmallSize;
};

