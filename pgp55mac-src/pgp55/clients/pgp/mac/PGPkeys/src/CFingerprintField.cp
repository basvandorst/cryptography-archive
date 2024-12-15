/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CFingerprintField.cp,v 1.2 1997/09/17 19:36:35 wprice Exp $
____________________________________________________________________________*/
#include "CFingerprintField.h"
#include <string.h>


	CFingerprintField *
CFingerprintField::CreateFromStream(
	LStream * inStream)
{
	return new CFingerprintField(inStream);
}

CFingerprintField::CFingerprintField()
	// default Inherited ctor automatically called
{
}

CFingerprintField::CFingerprintField(
	const CFingerprintField & inOriginal)
	: Inherited(inOriginal)
{
}

CFingerprintField::CFingerprintField(
	LStream * inStream)
	: Inherited(inStream)
{
}

	Boolean
CFingerprintField::ObeyCommand(
	CommandT			inCommand,
	void *				ioParam)
{
	Boolean cmdHandled;
	if ((inCommand == cmd_Copy) && (IsTarget()))
	{
		::ZeroScrap();
		::PutScrap(mText[ 0 ], 'TEXT', &mText[ 1 ]);
		cmdHandled = true;
	}
	else
	{
		cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
	}
	return cmdHandled;
}

	void
CFingerprintField::FindCommandStatus(
	CommandT			inCommand,
	Boolean &			outEnabled,
	Boolean &			outUsesMark,
	Char16 &			outMark,
	Str255				outName)
{
	if ((inCommand == cmd_Copy) && (IsTarget()))
	{
		outEnabled = true;
		outUsesMark = false;
	}
	else
	{
		LCommander::FindCommandStatus(inCommand, outEnabled, outUsesMark,
										outMark, outName);
	}
}

	void
CFingerprintField::FinishCreateSelf()
{
	Inherited::FinishCreateSelf();
}

	void
CFingerprintField::ClickSelf(
	const SMouseDownEvent & inMouseDown)
{
	Inherited::ClickSelf(inMouseDown);
	// clicking in a CFingerprintField makes it the Target.
	if (!IsTarget())
	{
		SwitchTarget(this);
	}
}

	void
CFingerprintField::AdjustCursorSelf(
	Point				inPortPt,
	const EventRecord &	inMacEvent)
{
	Inherited::AdjustCursorSelf(inPortPt, inMacEvent);
}

	void
CFingerprintField::BeTarget()
{
	LCommander::BeTarget();
	Refresh();	
}

	void
CFingerprintField::DontBeTarget()
{
	LCommander::DontBeTarget();
	Refresh();
}

	void
CFingerprintField::DrawSelf()
{
	// background color is white for all drawing in this pane
	StColorPenState	theColorPenState;
	::RGBBackColor (&(UGAColorRamp::GetWhiteColor()));
	Inherited::DrawSelf();
}

	void
CFingerprintField::DrawBackground(
	const Rect & localFrame)
{
	Rect frameCopy = localFrame;
	::InsetRect(&frameCopy, 2, 2);
	::EraseRect(&frameCopy);
}

	void
CFingerprintField::DrawBox(
	const Rect & localFrame)
{
	StColorPenState	theColorPenState;
	Rect frameCopy = localFrame;
	::PenNormal();

	// draw the surrounding shadow if on a color screen
	{
		StDeviceLoop	theLoop (localFrame);

		Int16				depth;
		while (theLoop.NextDepth (depth)) 
		{
			if (depth >= 8)	// 256 colors
			{
				DrawGrayBox(localFrame);
			}
		}
	}
	
	// always draw the black box regardless of depth
	::PenNormal();
	::InsetRect(&frameCopy, 1, 1);
	::RGBForeColor (&(UGAColorRamp::GetBlackColor()));
	::FrameRect(&frameCopy);
}

	void
CFingerprintField::DrawText(
	const Rect & localFrame)
{
	Rect frameCopy = localFrame;
	// three pixels around text are shadow, black box, white box.
	::InsetRect(&frameCopy, 3, 3);
	
	frameCopy.top--;	// needed to match spec appearance

	{
		StColorPenState	theColorPenState;	//	needed to restore before
											//	target-hiliting below
		Int16 just = UTextTraits::SetPortTextTraits(mTxtrID);
		
		RGBColor	textColor;
		::GetForeColor(&textColor);
		
		ApplyForeAndBackColors();
		::RGBForeColor(&textColor);

		UTextDrawing::DrawWithJustification((Ptr)&mText[1], mText[0],
											frameCopy, just);
	}
	
	if (IsTarget())
	{
		UDrawingUtils::SetHiliteModeOn();
		::InvertRect(&frameCopy);
	}
}

