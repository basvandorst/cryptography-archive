/*____________________________________________________________________________
	(C) Copyright 1996 by David K. Heller
	All rights reserved.

	David K. Heller
	730 E. Evelyn Avenue, #438
	Sunnyvale, CA  94086
	(408) 732-8960

	Note: This code is has been supplied to PGP, Inc. by
	David Heller without restriction

	$Id: CProgressBar.cp,v 1.2.10.1 1997/12/07 04:27:06 mhw Exp $
____________________________________________________________________________*/


#include <UDrawingUtils.h>

#include "CProgressBar.h"

static RGBColor	sRGBBlackColor 	= {0, 0, 0};
static RGBColor	sRGBWhiteColor 	= {0xFFFF, 0xFFFF, 0xFFFF};

// Remove dependency on qd:
static Pattern	sBlackPattern	= 
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};



CProgressBar::CProgressBar(LStream *inStream) : LPane(inStream)
{
	InitProgressBar();
}

CProgressBar::CProgressBar() : LPane()
{
	InitProgressBar();
}

CProgressBar::~CProgressBar()
{
}



	void
CProgressBar::InitProgressBar(void)
{
	mPercentComplete	= 0;
	
	mForeColor.red		= 0x4444;		// Dark gray
	mForeColor.green	= 0x4444;
	mForeColor.blue		= 0x4444;
	
	mBackColor.red		= 0xCCCC;		// Blue/purple
	mBackColor.green	= 0xCCCC;
	mBackColor.blue		= 0xFFFF;
}


	void
CProgressBar::DrawSelf(void)
{
	Rect	barRect;
	Rect	backFillRect;
	Rect	foreFillRect;
	
	pgpAssert( mPercentComplete <= 100 );
	
	FocusDraw();
	PenNormal();
	
	CalcLocalFrameRect( barRect );
	FrameRect( &barRect );	
	InsetRect( &barRect, 1, 1 );
	
	foreFillRect 		= barRect;
	foreFillRect.right 	= foreFillRect.left +
		 ( ( ( barRect.right - barRect.left ) * mPercentComplete ) / 100 );
	
	backFillRect		= barRect;
	backFillRect.left	= foreFillRect.right;
	
	{
		StDeviceLoop	devLoop( barRect );		
		SInt16			depth;
		RGBColor		saveForeColor;
		RGBColor		saveBackColor;
		
		GetForeColor( &saveForeColor );

		GetBackColor( &saveBackColor );
		RGBBackColor( &sRGBWhiteColor );
		
		while( devLoop.NextDepth( depth ) )
		{	
			if( foreFillRect.left != foreFillRect.right )
			{
				if( depth >= 8 )
				{
					RGBForeColor( &mForeColor );
				}
				else
				{
					RGBForeColor( &sRGBBlackColor );
				}
				
				FillRect( &foreFillRect, &sBlackPattern );
			}

			if( backFillRect.left != backFillRect.right )
			{
				if( depth >= 8 )
				{
					RGBForeColor( &mBackColor );
				}
				else
				{
					RGBForeColor( &sRGBWhiteColor );
				}
				
				FillRect( &backFillRect, &sBlackPattern );
			}
		}
	
		RGBForeColor( &saveForeColor );
		RGBBackColor( &saveBackColor );
	}
}


	void
CProgressBar::SetPercentComplete(UInt32 percentComplete)
{
	pgpAssert( percentComplete <= 100 );
	
	if( percentComplete > 100 )
		percentComplete = 100;
		
	if( percentComplete != mPercentComplete )
	{
		mPercentComplete = percentComplete;
		Draw( nil );
	}
}

//	Sprecify the foreground and/or background colors of a Window
//
//	Specify nil for inForeColor and/or inBackColor to leave that
//	color trait unchanged

	void
CProgressBar::SetForeAndBackColors(
	const RGBColor	*foreColor,
	const RGBColor	*backColor)
{
	if ( foreColor != nil )
		mForeColor = *foreColor;
	
	if( backColor != nil )
		mBackColor = *backColor;
}
