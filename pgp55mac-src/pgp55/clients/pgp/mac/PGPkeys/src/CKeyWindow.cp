/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyWindow.cp,v 1.5 1997/10/14 10:12:39 wprice Exp $
____________________________________________________________________________*/
#include "CKeyWindow.h"
#include "CKeyView.h"
#include "CPGPKeys.h"

#include "pgpErrors.h"
#include "pgpClientPrefs.h"

const Int16	kMinimumWindowHeight	=	150;


CKeyWindow::CKeyWindow()
{
}

CKeyWindow::CKeyWindow(LStream *inStream)
	:	LWindow(inStream)
{
}

CKeyWindow::~CKeyWindow()
{
	Rect		windowPos;
	PGPError	err;
	size_t		length;	
	
	length = sizeof(Rect);
	windowPos = UWindows::GetWindowContentRect(mMacWindowP);
	
	if(windowPos.bottom - windowPos.top >= kMinimumWindowHeight)
	{
		err = PGPSetPrefData(	gPrefRef,
								kPGPPrefPGPkeysMacMainWinPos,
								length,
								&windowPos);
		pgpAssertNoErr(err);
	}
}

	void
CKeyWindow::FinishCreateSelf()
{
	PGPError	err;
	Rect		*windPosPtr,
				windowPos,
				monitorIntersectRect,
				windowTitleBarRect;
	size_t		length;
	
	LWindow::FinishCreateSelf();
		
	LCommander::SetDefaultCommander( this );
	LPane::SetDefaultView( this );

	mKeyView = (CKeyView *)
		UReanimator::ReadObjects( 'PPob', kKeyViewResID );
	pgpAssertAddrValid(mKeyView, CKeyView *);
	
	mKeyView->FinishCreate();

	length = sizeof(Rect);
	err = PGPGetPrefData(	gPrefRef,
							kPGPPrefPGPkeysMacMainWinPos,
							&length,
							&windPosPtr);
	if(IsntPGPError(err))
	{
		windowPos = *windPosPtr;
		PGPDisposePrefData(gPrefRef, windPosPtr);
		windowTitleBarRect = windowPos;
		windowTitleBarRect.bottom = windowTitleBarRect.top + 20;
		// sanity check
		if((windowPos.left > 0) && (windowPos.right > 0)
			&& (windowPos.top > 0) && (windowPos.bottom > 0)
			&& SectRect(&windowTitleBarRect, &qd.screenBits.bounds,
						&monitorIntersectRect))
		{
			DoSetBounds(windowPos);
		}
	}
	
	Show();
}

	void
CKeyWindow::DrawSelf()
{
	Rect	frame;
	
	FocusDraw();
	CalcLocalFrameRect(frame);
	LWindow::DrawSelf();
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
	::MoveTo(frame.right - 1, frame.top + 1);
	::LineTo(frame.right - 1, frame.bottom - 16);
	::LineTo(frame.right - 16, frame.bottom - 16);
	::LineTo(frame.right - 16, frame.bottom - 1);
	::LineTo(frame.left + 1, frame.bottom - 1);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::MoveTo(frame.left, frame.bottom);
	::LineTo(frame.left, frame.top);
	::LineTo(frame.right, frame.top);
}

	void
CKeyWindow::ResizeFrameBy(
	Int16		inWidthDelta,
	Int16		inHeightDelta,
	Boolean		inRefresh)
{
	Rect	frame;

	LWindow::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	
	// following corrects a drawing problem with resizing that
	// was caused by our 3D border effect
	CalcPortFrameRect(frame);
	InvalPortRect(&frame);
}
	
