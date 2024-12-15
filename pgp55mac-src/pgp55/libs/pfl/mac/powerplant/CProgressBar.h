/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CProgressBar.h,v 1.2 1997/07/25 19:32:28 heller Exp $
____________________________________________________________________________*/

#pragma once

#include <LPane.h>

class CProgressBar : public LPane
{
public:

	enum { class_ID = 'PBar' };

						CProgressBar();
						CProgressBar(LStream *inStream);
	
	virtual				~CProgressBar();

	virtual void		SetForeAndBackColors(const RGBColor *inForeColor,
									const RGBColor *inBackColor);
	void				SetPercentComplete(UInt32 percentComplete);

protected:

	RGBColor		mForeColor;
	RGBColor		mBackColor;
	UInt32			mPercentComplete;
	
	void			InitProgressBar(void);
	virtual	void	DrawSelf(void);
};
