/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CRecessedCaption.cp,v 1.3 1997/09/17 19:36:40 wprice Exp $
____________________________________________________________________________*/
#include "CRecessedCaption.h"
#include <UAttachments.h>


	CRecessedCaption *
CRecessedCaption::CreateFromStream(
	LStream * inStream)
{
	return new CRecessedCaption(inStream);
}


CRecessedCaption::CRecessedCaption()
	// default Inherited ctor automatically called
{
}


CRecessedCaption::CRecessedCaption(
	const CRecessedCaption & inBorderCaption)
	: Inherited(inBorderCaption)
{
}


CRecessedCaption::CRecessedCaption(
	LStream * inStream)
	: Inherited(inStream)
{
	// read the fields used to configure the border
	//inStream->ReadData(&mBackColor, sizeof(RGBColor));
}


CRecessedCaption::CRecessedCaption(
	const SPaneInfo&	inPaneInfo,
	ConstStringPtr		inString,
	ResIDT				inTextTraitsID)
	: Inherited(inPaneInfo, inString, inTextTraitsID)
{
	//mBackColor = inBackColor;
}


	void
CRecessedCaption::FinishCreateSelf()
{
}


// NOTE: this follows the philosophy of drawing the border WITHIN the
// pane boundaries it saves a lot of hassles (e.g. with clipping)
	void
CRecessedCaption::DrawSelf()
{
	Rect localFrame;
	CalcLocalFrameRect (localFrame);
	ApplyForeAndBackColors();
	DrawBackground(	localFrame);
	DrawBox(		localFrame);
	DrawText(		localFrame);
}


	void
CRecessedCaption::DrawBackground(
	const Rect & localFrame)
{
	Rect frameCopy = localFrame;
	::InsetRect(&frameCopy, 1, 1);
	::EraseRect(&frameCopy);
}


	void
CRecessedCaption::DrawBox(
	const Rect & localFrame)
{
	StColorPenState	theColorPenState;
	StDeviceLoop	theLoop (localFrame);
	::PenNormal();

	Int16				depth;
	while (theLoop.NextDepth (depth)) 
	{
		if (depth >= 8)	// 256 colours
		{
			DrawGrayBox(localFrame);
		}
		else
		{
			::PenPat(&qd.gray);
			::FrameRect(&localFrame);
			::PenPat(&qd.black);
		}
	}
}
	

	void
CRecessedCaption::DrawGrayBox(
	const Rect & localFrame)
{
	Rect frameCopy = localFrame;
	
	// top left dark edges
	::RGBForeColor (&(UGAColorRamp::GetColor(7)));
	UGraphicsUtilities::TopLeftSide (&frameCopy, 0, 0, 1, 1);

	// bottom right light edges
	::RGBForeColor (&(UGAColorRamp::GetWhiteColor()));
	UGraphicsUtilities::BottomRightSide (&frameCopy, 1, 1, 0, 0);
}


// this is mostly copied from LCaption::DrawSelf
	void
CRecessedCaption::DrawText(
	const Rect & localFrame)
{
	Rect frame = localFrame;		// added
	::InsetRect(&frame, 4, 2);	// added
	frame.top--;					// added
	
	Int16	just = UTextTraits::SetPortTextTraits(mTxtrID);
	
	RGBColor	textColor;
	::GetForeColor(&textColor);
	
	ApplyForeAndBackColors();
	::RGBForeColor(&textColor);
	
	UTextDrawing::DrawWithJustification((Ptr)&mText[1], mText[0], frame, just);
}
