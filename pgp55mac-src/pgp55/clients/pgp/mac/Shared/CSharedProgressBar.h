/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include <LView.h>

class CSharedProgressBar : public LView
{
public:

	enum { class_ID = 'Prog' };
	enum { kInfinitePercent = 200 };

						CSharedProgressBar();
						CSharedProgressBar(LStream *inStream);
	
	virtual				~CSharedProgressBar();

	virtual void		SetForeAndBackColors(const RGBColor *inForeColor,
									const RGBColor *inBackColor);
	void				SetPercentComplete(UInt32 percentComplete);

protected:

	RGBColor		mForeColor;
	RGBColor		mBackColor;
	UInt32			mPercentComplete;
	UInt32			mState;
	
	void			InitProgressBar(void);
	virtual	void	DrawSelf(void);

	void			IncrementState(void);

	enum	{
				kMaxState	= 16,
				kIconWidth	= 16,
				kIconHeight	= 12,
				kIconID		= 4000
			};
};
