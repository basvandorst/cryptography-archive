//
//	UGrayscaleAppearance.cp
//
//	Utilities for supporting the Apple Grayscale Appearance for System 7.5
//
//
//	Copyright (C) 1996 by James Jennings. All rights reserved.
//

// To Check: are these being disposed of?

#include "UGrayscaleAppearance.h"

void	UGrayscaleAppearance::FrameRect( const Rect &inFrame, 
		EGAColor inTopLeft, EGAColor inBottomRight, EGAColor inCorners)
{	// Draw a frame that gives the "beveled" look.
	// inTopLeft is the color of the top and left edges.
	// inBottomRight is the color of the bottom and right edges.
	// inCorners is the color of the pixels in the bottom-left and
	// top-right corners.
	
	// width and height of the interior of the frame
	Int16 width = inFrame.right - inFrame.left - 2;
	Int16 height = inFrame.bottom - inFrame.top - 2;
	
	Assert_( width >= 0 && height >= 0 );
	
	::PenSize( 1, 1 );
	
	::MoveTo( inFrame.left, inFrame.bottom - 1 );
	SetForeColor( inCorners );
	::Line( 0, 0 );
	
	SetForeColor( inTopLeft );
	::Move( 0, -1 );
	::Line( 0, - height );
	::Line( width, 0 );
	
	SetForeColor( inCorners );
	::Move( 1, 0 );
	::Line( 0, 0 );
	
	SetForeColor( inBottomRight );
	::Move( 0, 1 );
	::Line( 0, height );
	::Line( - width, 0 );
	
}
