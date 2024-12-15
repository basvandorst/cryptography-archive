/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <UGraphicsUtilities.h>
#include <UWindows.h>

#include "CPGPtoolsWindow.h"

CPGPtoolsWindow::CPGPtoolsWindow(LStream *inStream)
	: LWindow( inStream )
{
}

CPGPtoolsWindow::~CPGPtoolsWindow(void)
{
}

	Boolean
CPGPtoolsWindow::PointIsOnVisibleDevice(Point globalPt)
{
	RgnHandle	myGrayRgn = NewRgn();
	Boolean		pointIsVisible;
	
	AssertHandleIsValid( myGrayRgn, "PointIsOnVisibleDevice" );
	
	CopyRgn( GetGrayRgn(), myGrayRgn );
	InsetRgn( myGrayRgn, 10, 10 );
	
	pointIsVisible = PtInRgn( globalPt, myGrayRgn );
	
	DisposeRgn( myGrayRgn );
	
	return( pointIsVisible );
}

	Boolean
CPGPtoolsWindow::WindowOnVisibleDevice(void)
{
	Rect		windowRect;
	Point		upperRight;
	Point		upperLeft;
	
	windowRect 	= UWindows::GetWindowContentRect( mMacWindowP );

	upperLeft.v		= windowRect.top;
	upperLeft.h		= windowRect.right;
	
	upperRight.v	= windowRect.top;
	upperRight.h	= windowRect.right;
	
	return( PointIsOnVisibleDevice( upperLeft ) ||
			PointIsOnVisibleDevice( upperRight ) );
}

//	Validates that the window will be visible and moveable before calling
//	the real DoSetPosition().

	void
CPGPtoolsWindow::DoSetPosition(Point inPosition) // Top left in global coords
{
	Rect		windowRect;
	Point		upperRight;
	
	windowRect 	= UWindows::GetWindowContentRect( mMacWindowP );

	upperRight		= inPosition;
	upperRight.h	= inPosition.h + UGraphicsUtilities::RectWidth(windowRect);
	
	if( PointIsOnVisibleDevice( inPosition ) ||
		PointIsOnVisibleDevice( upperRight ) )
	{
		LWindow::DoSetPosition( inPosition );
	}
}


//	Validates that the window will be visible and moveable before calling
//	the real DoSetBounds().

	void
CPGPtoolsWindow::DoSetBounds(const Rect &inBounds)	// Bounds in global coords
{
	Point	upperLeft;
	Point	upperRight;
	
	upperLeft.v = inBounds.top;
	upperLeft.h = inBounds.left;
	
	upperRight.v = inBounds.top;
	upperRight.h = inBounds.right;
	
	if( PointIsOnVisibleDevice( upperRight ) ||
		PointIsOnVisibleDevice( upperLeft ) )
	{
		LWindow::DoSetBounds( inBounds );
	}
}

// PowerPlant "leaks" AEDesc's when sending AppleEvents to itself, so we
// override this routine to call DoSetBounds directly since we dont care about
// recording AppleEvents.

	void
CPGPtoolsWindow::SendAESetBounds(
	Rect	*inBounds,					// New bounds in global coords
	Boolean	inExecuteAE)
{
	#pragma unused( inExecuteAE )
	
	DoSetBounds( *inBounds );
}

// PowerPlant "leaks" AEDesc's when sending AppleEvents to itself, so we
// override this routine to call DoSetZoom directly since we dont care about
// recording AppleEvents.

	void
CPGPtoolsWindow::SendAESetZoom(void)
{
	Rect	stdBounds;
	Boolean	zoomToStdState = ! CalcStandardBounds( stdBounds );

	DoSetZoom( zoomToStdState );
}