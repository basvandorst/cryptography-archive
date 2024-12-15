/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGAScroller.cp,v 1.1 1997/06/30 10:33:04 wprice Exp $
____________________________________________________________________________*/
#include "CGAScroller.h"

CGAScroller::CGAScroller(LStream	*inStream)
		:	LActiveScroller(inStream)
{
}

CGAScroller::~CGAScroller()
{
}

void
CGAScroller::DrawSelf()
{
	Rect	frame;
	CalcLocalFrameRect(frame);
	::PenNormal();
	ApplyForeAndBackColors();
	::MoveTo(frame.right, frame.top);
	::LineTo(frame.left, frame.top);
	::LineTo(frame.left, frame.bottom);
	//::FrameRect(&frame);
	
	if (mVerticalBar != nil) {
		MoveTo(frame.right - 16, frame.top);
		LineTo(frame.right - 16, frame.bottom - 1);
	}
	
	if (mHorizontalBar != nil) {
		MoveTo(frame.left, frame.bottom - 16);
		LineTo(frame.right - 1, frame.bottom - 16);
	}
	
		// When inactive, ScrollBars are hidden. Just outline
		// the ScrollBar locations with one pixel borders.
	
	if (!IsActive()) {
		if (mVerticalBar != nil) {
			mVerticalBar->CalcPortFrameRect(frame);
			PortToLocalPoint(topLeft(frame));
			PortToLocalPoint(botRight(frame));
			::FrameRect(&frame);
			::InsetRect(&frame, 1, 1);
			::EraseRect(&frame);
		}

		if (mHorizontalBar != nil) {
			mHorizontalBar->CalcPortFrameRect(frame);
			PortToLocalPoint(topLeft(frame));
			PortToLocalPoint(botRight(frame));
			::FrameRect(&frame);
			::InsetRect(&frame, 1, 1);
			::EraseRect(&frame);
		}
	}
}

