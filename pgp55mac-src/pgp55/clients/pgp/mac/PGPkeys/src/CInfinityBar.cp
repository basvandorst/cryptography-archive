/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CInfinityBar.cp,v 1.2 1997/09/06 12:38:19 wprice Exp $
____________________________________________________________________________*/
#include "CInfinityBar.h"

CInfinityBar::CInfinityBar(LStream *inStream):
				LView(inStream)
{
	mState = kMaxState;
}

	void	
CInfinityBar::DrawSelf(void)
{
	Rect		frame;
	Rect		iconFrame;
		
	::PenNormal();
	CalcLocalFrameRect(frame);
	
	ApplyForeAndBackColors();
	iconFrame.left 		= frame.left 		- mState;
	iconFrame.right		= iconFrame.left 	+ kIconWidth;
	iconFrame.top		= frame.top;
	iconFrame.bottom	= iconFrame.top		+ kIconHeight;
	
	while(iconFrame.left < frame.right)
	{
		::PlotIconID(&iconFrame, atNone, ttNone, kIconID);
		iconFrame.left	+= kIconWidth;
		iconFrame.right	+= kIconWidth;
	}	
	
	::FrameRect(&frame);
}

	void	
CInfinityBar::IncrementState(void)
{
	mState--;
	if(mState <= 0)
		mState = kMaxState;
	FocusDraw();
	Draw( nil );
}

