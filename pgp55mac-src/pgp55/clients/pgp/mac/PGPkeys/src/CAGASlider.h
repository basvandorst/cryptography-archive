/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CAGASlider.h,v 1.2.8.1 1997/12/05 22:14:04 mhw Exp $
____________________________________________________________________________*/

//
//	CAGASlider.h
//
//	Apple Grayscale Appearance Slider class for PowerPlant
//	Subclassed from LControl
//
//	version 1.2 - September 1, 1996
//
//	Copyright (C) 1996 by James Jennings. All rights reserved.
//

#pragma once

#include <LControl.h>
#include "UGrayscaleAppearance.h"

class CAGASlider : public LControl, UGrayscaleAppearance {
public:
	enum { class_ID = 'Slid' };
	static CAGASlider*	CreateFromStream(LStream *inStream);
	
						CAGASlider(LStream *inStream);
						CAGASlider(const CAGASlider &inSlider);
	virtual				~CAGASlider();
	
	static	void		Purge();	// Release indicator images
	
	// Enumerations for controling display states.
typedef	enum {
		hot_None = 0,
		hot_Indicator,
		hot_Track		// don't confuse 'the track' with 'mouse tracking'.
		} EHotSpots;
typedef	enum {
		ind_Enabled = 0,
		ind_Pressed,
		ind_Ghost,
		ind_Disabled
		} EIndicatorState;
typedef	enum {
		ind_Horizontal = 0,
		ind_North,
		ind_South,
		ind_Vertical,
		ind_East,
		ind_West
		} EIndicatorDirection;
	
protected:
	EIndicatorDirection	mDirection;
	EIndicatorState		mState;
	Uint16				mNumberOfTicks;
	Boolean				mContinuousBroadcast;
	Boolean				mMinIsBottomOrRight;
	
	Boolean				mIsTracking;
	Int32				mTrackingValue;
	
	virtual void		DrawSelf();
	virtual void		DrawSelfBasic( Boolean useGrays );
	virtual void		DrawTrack( EIndicatorState inState,
									Boolean inUseGrays = true );
	virtual void		DrawIndicator( Int32 inValue,
								EIndicatorState inState = ind_Enabled, 
								Boolean inUseGrays = true );
	virtual void		DrawTickMarks( UInt16 inNumber,
								EIndicatorState inState, 
								Boolean inUseGrays = true );
	
	virtual void		DrawIcon( Rect inWhere,
							EIndicatorDirection inDirection, 
							EIndicatorState inState, Boolean inUseGrays );
	virtual void		InitColorData();
	virtual void		InitBlackAndWhiteData();
	virtual void		InitMaskData();
	virtual void		InitPICTDataHelper( LGWorld *inWorld,
								const ResIDT inPICT, 
								const Rect inRect );
	static	LGWorld	*	sColorData;
	static	LGWorld	*	sBlackAndWhiteData;
	static	LGWorld	*	sMaskData;
	
	enum EOrientation { orient_Horizontal, orient_Vertical };
	virtual EOrientation GetOrientation();
	
	virtual void		ValueToIndicatorRect(Int32 inValue, Rect & outRect);
	virtual Point		ValueToPosition( Int32 inValue );
	virtual Int32		PositionToValue( Point inPosition );
	virtual void		GetTrackRect( Rect &outRect );

	virtual Int16		FindHotSpot(Point inPoint);
	virtual Boolean		PointInHotSpot(Point inPoint, Int16 inHotSpot);
	virtual Boolean		TrackHotSpot(Int16 inHotSpot, Point inPoint);
	virtual void		HotSpotAction(Int16 inHotSpot,
							Boolean inCurrInside, Boolean inPrevInside);
	virtual void		HotSpotResult(Int16 inHotSpot);
};

