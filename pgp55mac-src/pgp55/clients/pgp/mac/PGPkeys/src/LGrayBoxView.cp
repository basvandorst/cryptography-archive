/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: LGrayBoxView.cp,v 1.2 1997/09/17 19:36:44 wprice Exp $
____________________________________________________________________________*/
#include "LGrayBoxView.h"


LGrayBoxView::LGrayBoxView(
	LStream * inStreamP ) :
		Inherited( inStreamP )
{
	inStreamP->ReadData( &mTopLeftColorIndex,		sizeof( Int16 ) );
	inStreamP->ReadData( &mTopLineColorIndex,		sizeof( Int16 ) );
	inStreamP->ReadData( &mTopRightColorIndex,		sizeof( Int16 ) );
	inStreamP->ReadData( &mRightLineColorIndex,		sizeof( Int16 ) );
	inStreamP->ReadData( &mBottomRightColorIndex,	sizeof( Int16 ) );
	inStreamP->ReadData( &mBottomLineColorIndex,	sizeof( Int16 ) );
	inStreamP->ReadData( &mBottomLeftColorIndex,	sizeof( Int16 ) );
	inStreamP->ReadData( &mLeftLineColorIndex,		sizeof( Int16 ) );
}


	void
LGrayBoxView::DrawSelf()
{
	Rect		frame;
	Int16		currentColorIndex = -100;
	
	CalcLocalFrameRect( frame );
	::PenNormal();
	
	// draw eight components sequentially
	// too lazy to combine endpoints into lines
	// start with top left corner and proceed clockwise
	
	if ( mTopLeftColorIndex >= 0 )
	{
		if ( currentColorIndex != mTopLeftColorIndex )
		{
			currentColorIndex = mTopLeftColorIndex;
			::RGBForeColor(&(UGAColorRamp::GetColor(currentColorIndex)));	
		}
		::MoveTo( frame.left,		frame.top );
		::LineTo( frame.left,		frame.top );
	}
			
	if ( mTopLineColorIndex >= 0 )
	{
		if ( currentColorIndex != mTopLineColorIndex )
		{
			currentColorIndex = mTopLineColorIndex;
			::RGBForeColor(&(UGAColorRamp::GetColor(currentColorIndex)));	
		}
		::MoveTo( frame.left + 1,	frame.top );
		::LineTo( frame.right - 2,	frame.top );
	}

	if ( mTopRightColorIndex >= 0 )
	{
		if ( currentColorIndex != mTopRightColorIndex )
		{
			currentColorIndex = mTopRightColorIndex;
			::RGBForeColor(&(UGAColorRamp::GetColor(currentColorIndex)));	
		}
		::MoveTo( frame.right - 1,	frame.top );
		::LineTo( frame.right - 1,	frame.top );
	}
			
	if ( mRightLineColorIndex >= 0 )
	{
		if ( currentColorIndex != mRightLineColorIndex )
		{
			currentColorIndex = mRightLineColorIndex;
			::RGBForeColor(&(UGAColorRamp::GetColor(currentColorIndex)));	
		}
		::MoveTo( frame.right - 1,	frame.top + 1 );
		::LineTo( frame.right - 1,	frame.bottom - 2 );
	}

	if ( mBottomRightColorIndex >= 0 )
	{
		if ( currentColorIndex != mBottomRightColorIndex )
		{
			currentColorIndex = mBottomRightColorIndex;
			::RGBForeColor(&(UGAColorRamp::GetColor(currentColorIndex)));	
		}
		::MoveTo( frame.right - 1,	frame.bottom - 1 );
		::LineTo( frame.right - 1,	frame.bottom - 1 );
	}

	if ( mBottomLineColorIndex >= 0 )
	{
		if ( currentColorIndex != mBottomLineColorIndex )
		{
			currentColorIndex = mBottomLineColorIndex;
			::RGBForeColor(&(UGAColorRamp::GetColor(currentColorIndex)));	
		}
		::MoveTo( frame.right - 2,	frame.bottom - 1 );
		::LineTo( frame.left + 1,	frame.bottom - 1 );
	}

	if ( mBottomLeftColorIndex >= 0 )
	{
		if ( currentColorIndex != mBottomLeftColorIndex )
		{
			currentColorIndex = mBottomLeftColorIndex;
			::RGBForeColor(&(UGAColorRamp::GetColor(currentColorIndex)));	
		}
		::MoveTo( frame.left,		frame.bottom - 1 );
		::LineTo( frame.left,		frame.bottom - 1 );
	}

	if ( mLeftLineColorIndex >= 0 )
	{
		if ( currentColorIndex != mLeftLineColorIndex )
		{
			currentColorIndex = mLeftLineColorIndex;
			::RGBForeColor(&(UGAColorRamp::GetColor(currentColorIndex)));	
		}
		::MoveTo( frame.left,		frame.bottom - 2 );
		::LineTo( frame.left,		frame.top + 1 );
	}
}
