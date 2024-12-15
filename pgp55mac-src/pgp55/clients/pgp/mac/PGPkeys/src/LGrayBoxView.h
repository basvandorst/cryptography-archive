/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: LGrayBoxView.h,v 1.1 1997/06/30 10:33:15 wprice Exp $
____________________________________________________________________________*/
#pragma once

class LGrayBoxView : public LView
{
private:
	typedef LView Inherited;
public:
	enum { class_ID = 'GyBV' };
	static LGrayBoxView* CreateFromStream ( LStream * inStreamP );
	LGrayBoxView( LStream * inStreamP );
private:

	virtual void DrawSelf();
	
	// corners and lines from top left, around the rect clockwise
	// these colors are PowerPlant Grayscale class color indices
	// -1 means use the window background color
	Int16	mTopLeftColorIndex;
	Int16	mTopLineColorIndex;
	Int16	mTopRightColorIndex;
	Int16	mRightLineColorIndex;
	Int16	mBottomRightColorIndex;
	Int16	mBottomLineColorIndex;
	Int16	mBottomLeftColorIndex;
	Int16	mLeftLineColorIndex;
};

