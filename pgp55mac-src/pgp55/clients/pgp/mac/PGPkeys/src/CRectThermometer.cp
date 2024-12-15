/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CRectThermometer.cp,v 1.1 1997/06/30 10:33:12 wprice Exp $
____________________________________________________________________________*/
#include "CRectThermometer.h"

CRectThermometer::CRectThermometer(
	LStream * inStreamP) :
	Inherited(inStreamP)
{
}

	CRectThermometer *
CRectThermometer::CreateFromStream(
	LStream * inStreamP)
{
	return new CRectThermometer(inStreamP);
}

CRectThermometer::~CRectThermometer()
{
}

	void
CRectThermometer::DrawSelf(
	void)
{
	::PenNormal();
	Rect localFrame;
	CalcLocalFrameRect (localFrame);
	::FrameRect(&localFrame);
	::InsetRect(&localFrame, 1, 1);
	
	if (	(mValue >= mMinValue) &&
			(mValue <= mMaxValue) &&
			(mMaxValue > mMinValue))
	{
		Int32 valueRange		= mMaxValue - mMinValue;
		Int32 normalizedValue	= mValue - mMinValue;
		Int32 frameWidth		= localFrame.right	- localFrame.left;
		Int32 darkWidth			= frameWidth * normalizedValue / valueRange;

		Rect valueRect	= localFrame;
		valueRect.right	= valueRect.left + darkWidth;

		Rect blankRect	= localFrame;
		blankRect.left	= valueRect.right;
		
		StColorPenState	theColorPenState;
		StDeviceLoop	theLoop (localFrame);
		Int16			depth;
		while (theLoop.NextDepth (depth)) 
		{
			if (depth >= 8)	// 256 colours
			{
				::RGBForeColor(&UGAColorRamp::GetColor(9));
				::PaintRect(&valueRect);
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Purple1));
				::PaintRect(&blankRect);
			}
			else
			{
				::RGBForeColor(&UGAColorRamp::GetBlackColor());
				::PaintRect(&valueRect);
				::RGBForeColor(&UGAColorRamp::GetWhiteColor());
				::PaintRect(&blankRect);
			}
		}	
	}
}
