/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CFocusBorder.cp,v 1.2 1997/10/30 20:52:59 heller Exp $
____________________________________________________________________________*/

#include "pgpMem.h"

#include "CFocusBorder.h"
#include "PGPUILibDialogs.h"

CFocusBorder::CFocusBorder(LStream	*inStream)
		: LGAFocusBorder(inStream)
{
}

CFocusBorder::~CFocusBorder(void)
{
}

	void
CFocusBorder::DrawFocusBorder(Int16 inBitDepth)
{
	if( IsntNull( GetInsetSubPane() ) )
	{
		Rect			frame;
		StColorPenState	theColorPenState;
		
		theColorPenState.Normalize();
		
		FocusDraw();
		CalcLocalFrameRect( frame );
		
		PenSize( 3, 3 );

		frame.right 	-= 2;
		frame.bottom 	-= 2;
		
		if( inBitDepth == 1 )
		{
			Rect	drawRect = frame;
		
			PenSize( 2, 2 );
			InsetRect( &drawRect, 1, 1 );

			drawRect.right 	+= 2;
			drawRect.bottom += 2;
			
			FrameRect( &drawRect );
		}
		else
		{
			BevelBorder( frame );
		}

		ApplyForeAndBackColors();
	}
}

	RgnHandle
CFocusBorder::GetFocusRegion(void)
{
	return( GetBorderRegion() );
}
