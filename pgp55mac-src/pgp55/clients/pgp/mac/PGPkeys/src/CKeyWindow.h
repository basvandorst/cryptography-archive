/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyWindow.h,v 1.1 1997/06/30 10:33:10 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpKeys.h"

class CKeyView;

class CKeyWindow		:		public LWindow
{
public:
	enum { class_ID = 'keyW' };
								CKeyWindow();
								CKeyWindow(LStream *inStream);
								~CKeyWindow();
	void						FinishCreateSelf();
	void						DrawSelf();
	CKeyView					*GetKeyView()
									{	return mKeyView;	}
	
	virtual void				ResizeFrameBy(
									Int16		inWidthDelta,
									Int16		inHeightDelta,
									Boolean		inRefresh);
	
private:
	CKeyView					*mKeyView;
};

