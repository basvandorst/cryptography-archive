/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/


#include <UDrawingUtils.h>

#include "CSharedProgressBar.h"
#include "pgpMem.h"

static RGBColor	sRGBBlackColor 	= {0, 0, 0};
static RGBColor	sRGBWhiteColor 	= {0xFFFF, 0xFFFF, 0xFFFF};

// Remove dependency on qd:
static Pattern	sBlackPattern	= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
									0xFF};

CSharedProgressBar::CSharedProgressBar(LStream *inStream) : LView(inStream)
{
	InitProgressBar();
}

CSharedProgressBar::CSharedProgressBar() : LView()
{
	InitProgressBar();
}

CSharedProgressBar::~CSharedProgressBar()
{
}

	void
CSharedProgressBar::InitProgressBar(void)
{
	mPercentComplete	= 0;
	mState				= 0;
	
	mForeColor.red		= 0x4444;		// Dark gray
	mForeColor.green	= 0x4444;
	mForeColor.blue		= 0x4444;
	
	mBackColor.red		= 0xCCCC;		// Blue/purple
	mBackColor.green	= 0xCCCC;
	mBackColor.blue		= 0xFFFF;
}

	void
CSharedProgressBar::DrawSelf(void)
{
#if PGP_DEBUG
	/* Verify that the barber pole icon resources can be found */
	if( IsNull( GetResource( kMini1BitMask, kIconID ) ) )
	{
		pgpDebugMsg( "CSharedProgressBar::DrawSelf(): Unable to find "
		"needed resources" );
	}
#endif

	if (mPercentComplete > 100 ) {
		Rect		frame;
		Rect		iconFrame;
			
		::PenNormal();
		CalcLocalFrameRect(frame);
		
		ApplyForeAndBackColors();
		::FrameRect(&frame);

		iconFrame.left 		= frame.left 		+ mState - kIconWidth;
		iconFrame.right		= iconFrame.left 	+ kIconWidth;
		iconFrame.top		= frame.top;
		iconFrame.bottom	= iconFrame.top		+ kIconHeight;
		
		::InsetRect(&frame, 1, 1);
		StClipRgnState	savedClip(frame);
		
	
		while(iconFrame.left < frame.right)
		{
			::PlotIconID(&iconFrame, atNone, ttNone, kIconID);
			iconFrame.left	+= kIconWidth;
			iconFrame.right	+= kIconWidth;
		}	
		
	} else {
		Rect	barRect;
		Rect	backFillRect;
		Rect	foreFillRect;
		
		FocusDraw();
		PenNormal();
		
		CalcLocalFrameRect( barRect );
		FrameRect( &barRect );	
		InsetRect( &barRect, 1, 1 );
		
		foreFillRect 		= barRect;
		foreFillRect.right 	= foreFillRect.left +  ( ( ( barRect.right -
									barRect.left ) *
									mPercentComplete ) / 100 );
		
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
}

	void
CSharedProgressBar::SetPercentComplete(UInt32 percentComplete)
{
	if( percentComplete > 100 )
		IncrementState();
		
	if(( percentComplete != mPercentComplete ) || (percentComplete > 100))
	{
		mPercentComplete = percentComplete;
		Draw( nil );
	}
}

//	Specify the foreground and/or background colors of a Window
//
//	Specify nil for inForeColor and/or inBackColor to leave that
//	color trait unchanged

	void
CSharedProgressBar::SetForeAndBackColors(
	const RGBColor	*foreColor,
	const RGBColor	*backColor)
{
	if ( foreColor != nil )
		mForeColor = *foreColor;
	
	if( backColor != nil )
		mBackColor = *backColor;
}

	void	
CSharedProgressBar::IncrementState(void)
{
	mState++;
	if(mState >= kMaxState)
		mState = 0;
	Draw(nil);
}
