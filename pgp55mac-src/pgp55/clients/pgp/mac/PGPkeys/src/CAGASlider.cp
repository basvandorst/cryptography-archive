/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CAGASlider.cp,v 1.2.8.1 1997/12/05 22:14:04 mhw Exp $
____________________________________________________________________________*/

//
//	CAGASlider.cp
//
//	Apple Grayscale Appearance Slider class for PowerPlant
//	Subclassed from LControl
//
//	version 1.2 - September 1, 1996
//
//	Copyright (C) 1996 by James Jennings. All rights reserved.
//

#include "CAGASlider.h"
#include <UGWorld.h>

// Note that the flag AGA_VERSION won't change the indicator/thumb. 
// You have to replace the PICT 1000 resource for that.
//#define AGA_VERSION 1	// Apple's specs as of April 96
#define AGA_VERSION 2	// Apple's specs as of Sept 96

// constants that control the geometry of the slider
static const Int16 kEndMargin = 10;		// where the indicator stops.
static const Int16 kFlatMargin = 4;		// margin on the flat side on an
										// indicator
static const Int16 kPointMargin = 6;	// margin on the pointy side--not
										// including tickmarks
static const Int16 kTrackWidth = 5;		// width of the indicator's track
static const Int16 kOffsideCutoff = 24;	// abort by dragging this far from
										// control
static const Int16 kTrackToTick = 10;	// center of track to start of tick
										// mark
static const Int16 kTickLen = 6;		// length of tick mark
static const Int16 kIndSize = 16;		// size of indicator image
static const Int16 kDepthCutoff = 4;// 4 or 8. The screen depth to start
									// grayscale drawing.

// If there are no tick marks, the slider is 16 pixels wide.
// If there are tick marks, the slider is approximately...
//const Int16 kSliderWidth = kFlatMargin+kTrackWidth+kTrackToTick+kTickLen;
// = 25 pixels

// Class variables for holding the indicator image data.
LGWorld	*	CAGASlider::sColorData = 0;
LGWorld	*	CAGASlider::sBlackAndWhiteData = 0;
LGWorld	*	CAGASlider::sMaskData = 0;

// Indicator image related constants.
const Rect 		kPICTDataSize = { 0, 0, 6*kIndSize, 4*kIndSize };
const ResIDT	k8BitPICTid = 1000;
const ResIDT	k1BitPICTid = 1001;
const ResIDT	kMaskPICTid = 1002;

CAGASlider*	CAGASlider::CreateFromStream(LStream *inStream)
{
	return new CAGASlider(inStream);
}

CAGASlider::CAGASlider(LStream *inStream)
	: LControl(inStream), mState(ind_Enabled), mContinuousBroadcast(true),
	  mIsTracking(false), mTrackingValue(0), mMinIsBottomOrRight(false)
{
	Int32 directionCode;
	inStream->ReadBlock(&directionCode, sizeof(directionCode));
	
	// Convert the entered code to the enumerated type
	switch (directionCode) {
	case 'hori':	mDirection = ind_Horizontal;	break;
	case 'vert':	mDirection = ind_Vertical;		break;
	case 'nort':	mDirection = ind_North;			break;
	case 'sout':	mDirection = ind_South;			break;
	case 'east':	mDirection = ind_East;			break;
	case 'west':	mDirection = ind_West;			break;
	default:
		SDimension16 theSize;
		GetFrameSize(theSize);
		if (theSize.width > theSize.height) mDirection = ind_Horizontal;
		else mDirection = ind_Vertical;
		break;
	}
	inStream->ReadBlock(&mNumberOfTicks, sizeof(mNumberOfTicks));
	inStream->ReadBlock(&mContinuousBroadcast, sizeof(mContinuousBroadcast));
	inStream->ReadBlock(&mMinIsBottomOrRight, sizeof(mMinIsBottomOrRight));
}

CAGASlider::CAGASlider(const CAGASlider &inSlider)	// copy constructor
	:	LControl(inSlider), mState(ind_Enabled), mIsTracking(false),
		mTrackingValue(0), 
	  mDirection(inSlider.mDirection), 
	  mNumberOfTicks(inSlider.mNumberOfTicks),
	  mContinuousBroadcast(inSlider.mContinuousBroadcast), 
	  mMinIsBottomOrRight(inSlider.mMinIsBottomOrRight)
{
}

CAGASlider::~CAGASlider()
{

}

void CAGASlider::Purge()
{	
	if (sColorData != nil) {
		delete sColorData;
		sColorData = nil;
	}
	
	if (sBlackAndWhiteData != nil) {
		delete sBlackAndWhiteData;
		sBlackAndWhiteData = nil;
	}
	
	if (sMaskData != nil) {
		delete sMaskData;
		sMaskData = nil;
	}
}

void CAGASlider::DrawSelf(void)
{
	Rect theFrame, r;
	CalcLocalFrameRect(theFrame);
	
	StDeviceLoop devices(theFrame);
	Int16 depth;

	while (devices.NextDepth(depth)) {
	
		Boolean useGrays = (depth >= kDepthCutoff);
		
		r = (*devices.GetCurrentDevice())->gdRect;
		::GlobalToLocal(&topLeft(r));
		::GlobalToLocal(&botRight(r));
		if (!::SectRect(&theFrame, &r, &r)) continue;
		
		StOffscreenGWorld offscreen(r);
		
		DrawSelfBasic(useGrays);
			
	}
}

void CAGASlider::DrawSelfBasic(Boolean useGrays)
{
	StColorPenState::Normalize();
	Rect theFrame;
	CalcLocalFrameRect(theFrame);
	
	EIndicatorState theState = (IsEnabled() ? mState : ind_Disabled);
	
	EraseRect(theFrame);
	
	DrawTrack(theState, useGrays);
	DrawTickMarks(mNumberOfTicks, theState, useGrays);
	DrawIndicator(GetValue(), theState, useGrays);
	
	if (mIsTracking && (GetValue() != mTrackingValue))
		DrawIndicator(mTrackingValue, ind_Ghost, useGrays);
}

void CAGASlider::DrawTrack(EIndicatorState inState, Boolean inUseGrays)
{
	StColorPenState::Normalize();
	
	Rect r;
	GetTrackRect(r);
	
	const Int16 kRoundness = 4;
	
	// Colors depend on inState
	EGAColor black, gray;
	if (inState == ind_Disabled) {
		gray = ga_4;
		black = ga_8;
	} else {
		gray = ga_5;
		black = ga_Black;
	}

	if (inUseGrays) {
#if (AGA_VERSION==2)
		if (inState != ind_Disabled) {
#endif
			r.bottom++;
			r.right++;
			SetForeColor(ga_White);
			::PaintRoundRect(&r, kRoundness, kRoundness);
			
			::OffsetRect(&r, -1, -1);
			SetForeColor(gray);
			::PaintRoundRect(&r, kRoundness, kRoundness);
			
			r.top++;
			r.left++;
#if (AGA_VERSION==2)
		}
#endif
		SetForeColor(black);
	} else {
		if (inState == ind_Disabled) 
			::PenPat(&UQDGlobals::GetQDGlobals()->gray);
	}
	::FrameRoundRect(&r, kRoundness, kRoundness);
}

void CAGASlider::DrawIndicator(Int32 inValue, EIndicatorState inState,
								Boolean inUseGrays)
{
	Rect r;
	ValueToIndicatorRect(inValue, r);
	
	DrawIcon(r, mDirection, inState, inUseGrays);
}

void	CAGASlider::DrawIcon(Rect inWhere, EIndicatorDirection inDirection, 
							EIndicatorState inState, Boolean inUseGrays)
{	// draw the indicator out of a PICT
	// PlotIconID() needs a 16x16 rectangle.
	inWhere.bottom = inWhere.top + kIndSize;
	inWhere.right = inWhere.left + kIndSize;

	if (inUseGrays) InitColorData();
	else InitBlackAndWhiteData();
	
	GWorldPtr imageWorld = (inUseGrays ?
			sColorData : sBlackAndWhiteData)->GetMacGWorld();
	GWorldPtr maskWorld = sMaskData->GetMacGWorld();
	Assert_(imageWorld != nil && maskWorld != nil);
	
	PixMapHandle image = ::GetGWorldPixMap(imageWorld);
	PixMapHandle mask  = ::GetGWorldPixMap(maskWorld);
	ThrowIfNil_(image);
	ThrowIfNil_(mask);
	
	Boolean notPurged;
	notPurged = ::LockPixels(image);
	notPurged = ::LockPixels(mask);
	
	Rect r = {0,0,kIndSize,kIndSize};
	Rect maskRect = r;
	::OffsetRect(&r, inState * kIndSize, inDirection * kIndSize);
	::OffsetRect(&maskRect, 0, inDirection * kIndSize);
	
	GrafPtr thePort;
	::GetPort(& thePort);
	
	::ForeColor(blackColor);
	::BackColor(whiteColor);
	//	::CopyMask(srcBits, maskBits, dstBits, srcRect, maskRect, dstRect);
	::CopyMask((BitMapPtr)*image, (BitMapPtr)*mask, &(thePort->portBits), 
					&r, &maskRect, &inWhere);
	
	::UnlockPixels(image);
	::UnlockPixels(mask);
}

void	CAGASlider::InitColorData()
{
	InitMaskData();
	
	if (sColorData != nil) return;
	
	sColorData = new LGWorld(kPICTDataSize, 8);
	ThrowIfNil_(sColorData);
	
	InitPICTDataHelper(sColorData, k8BitPICTid, kPICTDataSize);
}

void	CAGASlider::InitBlackAndWhiteData()
{
	InitMaskData();
	
	if (sBlackAndWhiteData != nil) return;
	
	sBlackAndWhiteData = new LGWorld(kPICTDataSize, 1);
	ThrowIfNil_(sBlackAndWhiteData);
	
	InitPICTDataHelper(sBlackAndWhiteData, k1BitPICTid, kPICTDataSize);
}

void	CAGASlider::InitMaskData()
{
	if (sMaskData != nil) return;
	
	Rect r = kPICTDataSize;
	r.right = r.left + kIndSize;
	sMaskData = new LGWorld(r, 1);
	ThrowIfNil_(sMaskData);
	
	InitPICTDataHelper(sMaskData, kMaskPICTid, r);
}

void	CAGASlider::InitPICTDataHelper(LGWorld *inWorld,
				const ResIDT inPICT, const Rect inRect)
{
	PicHandle aPicH = ::GetPicture(inPICT);
	ThrowIfNil_(aPicH);
	
	inWorld->BeginDrawing();
	::DrawPicture(aPicH, &inRect);
	inWorld->EndDrawing();
	
	::ReleaseResource((Handle)aPicH);
}

void	CAGASlider::DrawTickMarks(UInt16 inNumber, EIndicatorState inState,
					Boolean inUseGrays)
{
	if (inNumber == 0) return;
	if (inNumber == 1) inNumber = mMaxValue - mMinValue + 1;
	if (inNumber <= 1) return;
	// Only "pointy" indicators have tickmarks.
	if (mDirection == ind_Horizontal || mDirection == ind_Vertical) return;
	
	Rect theTrack;
	GetTrackRect(theTrack);
	
	Rect firstTick;
	Point p = ValueToPosition(mMinValue);
	Point q = ValueToPosition(mMaxValue);
	switch (mDirection) {
	case ind_North:
		p.v -= kTrackToTick + kTickLen + 1;
		q.v -= kTrackToTick + kTickLen + 1;
		::SetRect(&firstTick, p.h, p.v, p.h + 1, p.v + kTickLen);
		break;
	case ind_South:
		p.v += kTrackToTick;
		q.v += kTrackToTick;
		::SetRect(&firstTick, p.h, p.v, p.h + 1, p.v + kTickLen);
		break;
	case ind_East:
		p.h += kTrackToTick;
		q.h += kTrackToTick;
		::SetRect(&firstTick, p.h, p.v, p.h + kTickLen, p.v + 1);
		break;
	case ind_West:
		p.h -= kTrackToTick + kTickLen + 1;
		q.h -= kTrackToTick + kTickLen + 1;
		::SetRect(&firstTick, p.h, p.v, p.h + kTickLen, p.v + 1);
		break;
	case ind_Horizontal:
	case ind_Vertical:
	default:
		SignalPStr_("\pBad Slider Class Member");
	}
	
	Int32 intervals = inNumber - 1;
	Assert_(intervals > 0);
	for(Int32 i = 0; i <= intervals; i++) {
	
		Int16 deltah = (q.h - p.h) * i / intervals;
		Int16 deltav = (q.v - p.v) * i / intervals;
		Rect r = firstTick;
		::OffsetRect(&r, deltah, deltav);
		
		if (inUseGrays) {
		
			// Colors depend on inState
			EGAColor black, gray;
			if (inState == ind_Disabled) {
				gray = ga_4;
				black = ga_8;
			} else {
				gray = ga_7;
				black = ga_Black;
			}
			
			PaintRect(r, black);

#if AGA_VERSION==2
			if (inState != ind_Disabled) {
				::InsetRect(&r, -1, -1);
				FrameRect(r, ga_White, gray, ga_Background);
			}
#else
			::InsetRect(&r, -1, -1);
			FrameRect(r, ga_White, gray, ga_Background);
#endif		
			
		} else {
		
			if (inState == ind_Disabled) 
				::PenPat(&UQDGlobals::GetQDGlobals()->gray);
			::PaintRect(&r);
			
		}
	}
}

CAGASlider::EOrientation CAGASlider::GetOrientation(void)
{
	EOrientation theResult;
	switch (mDirection) {
	case ind_Horizontal:
	case ind_North:
	case ind_South:
		theResult = orient_Horizontal;
		break;
	case ind_Vertical:
	case ind_East:
	case ind_West:
		theResult = orient_Vertical;
		break;
	default:
		SignalPStr_("\pBad Slider Class Member");
	}
	return theResult;
}

void CAGASlider::ValueToIndicatorRect(Int32 inValue, Rect &outRect)
{
	Point center = ValueToPosition(inValue);
	
	switch (mDirection) {
	case ind_Horizontal:
		//	SetRect() arguments are: Rect*, left, top, right, bottom
		::SetRect(&outRect, center.h - 6, center.v - 8, center.h + 7,
					center.v + 8);
		break;
	case ind_North:
		::SetRect(&outRect, center.h - 7, center.v -10, center.h + 8,
					center.v + 6);
		break;
	case ind_South:
		::SetRect(&outRect, center.h - 7, center.v - 7, center.h + 8,
					center.v + 9);
		break;
	case ind_Vertical:
		::SetRect(&outRect, center.h - 8, center.v - 6, center.h + 8,
					center.v + 7);
		break;
	case ind_East:
		::SetRect(&outRect, center.h - 7, center.v - 7, center.h + 9,
					center.v + 8);
		break;
	case ind_West:
		::SetRect(&outRect, center.h -10, center.v - 7, center.h + 6,
					center.v + 8);
		break;
	default:
		SignalPStr_("\pBad Slider Class Member");
	}
}

Point	CAGASlider::ValueToPosition(Int32 inValue)
{
	Point p;	// the result
	
	Assert_(mMinValue <= inValue && inValue <= mMaxValue);
	
	Int32 valueRange = mMaxValue - mMinValue;
	Int16 pixelRange;
	
	// Handle special case to avoid a divide by zero later.
	if (valueRange <= 0) {
		inValue = mMinValue;
		valueRange = 1;
	}
	
	// Flip the direction of increasing value, if asked.
	Int32 startValue;
	if (mMinIsBottomOrRight) {
		startValue = mMaxValue;
		valueRange = - valueRange;
	} else {
		startValue = mMinValue;
	}
	
	Rect theTrack;
	GetTrackRect(theTrack);
	
	// Non-pointy indicators (which have no tick marks) aren't as 
	// wide, so we adjust the placement.
	Int16 margin = kEndMargin;
	if (mDirection == ind_Horizontal || mDirection == ind_Vertical) margin--;
	
	switch(GetOrientation()) {
	case orient_Horizontal:
		theTrack.right--;	// aesthetic adjustments
		theTrack.bottom++;
		pixelRange = theTrack.right - theTrack.left - 2*margin;
		p.h = theTrack.left + margin + 
				pixelRange * (inValue - startValue) / valueRange;
		p.v = (theTrack.top + theTrack.bottom)/2;
		break;
	case orient_Vertical:
		theTrack.bottom--;	// aesthetic adjustments
		theTrack.right++;
		pixelRange = theTrack.bottom - theTrack.top - 2*margin;
		p.v = theTrack.top + margin + 
				pixelRange * (inValue - startValue) / valueRange;
		p.h = (theTrack.right + theTrack.left)/2;
		break;
	default:
		SignalPStr_("\pBad Slider Orientation");
		break;
	}
	
	return p;
}

Int32	CAGASlider::PositionToValue(Point inPosition)
{
	// Handle special case to avoid a divide by zero later.
	if (mMaxValue <= mMinValue) return mMinValue;
	
	Rect theTrack;
	GetTrackRect(theTrack);
	
	Point offside = {0,0};
	
	if (inPosition.v < theTrack.top)
		offside.v = theTrack.top - inPosition.v;
	if (inPosition.v > theTrack.bottom)
		offside.v = inPosition.v - theTrack.bottom;
	if (inPosition.h < theTrack.left)
		offside.h = theTrack.left - inPosition.h;
	if (inPosition.h > theTrack.right)
		offside.h = inPosition.h - theTrack.right;
	
	if ((offside.h > kOffsideCutoff) || (offside.v > kOffsideCutoff))
		return GetValue();	

	Int32 offset, range, result;
	
	switch(GetOrientation()) {
	case orient_Horizontal:
		offset = inPosition.h - theTrack.left - kEndMargin;
		range = theTrack.right - theTrack.left - 2*kEndMargin;
		Assert_(range > 0);
		break;
	case orient_Vertical:
		offset = inPosition.v - theTrack.top - kEndMargin;
		range = theTrack.bottom - theTrack.top - 2*kEndMargin;
		Assert_(range > 0);
		break;
	default:
		SignalPStr_("\pBad Slider Orientation");
		break;
	}
	
	if (mMinIsBottomOrRight)
		offset = range - offset;
	
	offset += range / (mMaxValue - mMinValue) / 2;
	
	// calculate result
	result = mMinValue + (mMaxValue - mMinValue) * offset / range;
	
	// range clipping
	if (result < mMinValue) result = mMinValue;
	if (result > mMaxValue) result = mMaxValue;
	
	return result;
}

void	CAGASlider::GetTrackRect(Rect &outRect)
{	// Calculate the rectange to draw the track into.
	
	CalcLocalFrameRect(outRect);
	
	SDimension16 theSize;
	GetFrameSize(theSize);

	// Center the track in the frame
	switch (GetOrientation()) {
	
	case orient_Horizontal:
		
		switch (mDirection) {
		case ind_Horizontal:
			outRect.top += (theSize.height - kTrackWidth)/2;
			break;
		case ind_North:
			outRect.top = outRect.bottom - kFlatMargin - kTrackWidth;
			break;
		case ind_South:
			outRect.top += kFlatMargin;
			break;
		}
		outRect.bottom = outRect.top + kTrackWidth;
		outRect.left++;
		outRect.right--;
		break;
		
	case orient_Vertical:
	
		switch (mDirection) {
		case ind_Vertical:
			outRect.left += (theSize.width - kTrackWidth)/2;
			break;
		case ind_East:
			outRect.left += kFlatMargin;
			break;
		case ind_West:
			outRect.left = outRect.right - kFlatMargin - kTrackWidth;
			break;
		}
		outRect.right = outRect.left + kTrackWidth;
		outRect.top++;
		outRect.bottom--;
		break;
	}
}

Int16	CAGASlider::FindHotSpot(Point inPoint)	// Rewrite w/o icons !!!
{
	// default is None.
	Int16 result = hot_None;
	
	// Is it in the track?
	Rect r;
	GetTrackRect(r);
	::InsetRect(&r, -2, -2);
	
	if (::PtInRect(inPoint, &r)) result = hot_Track;
	
	// Is it in the indicator? If so, override track.
	ValueToIndicatorRect(GetValue(), r);
	
	// code which relys on the PICT mask
	if (::PtInRect(inPoint, &r)) {
	
		Point p;
		p.h = inPoint.h - r.left;
		p.v = inPoint.v - r.top + mDirection * kIndSize;
		
		InitMaskData();
		PixMapHandle pm = ::GetGWorldPixMap(sMaskData->GetMacGWorld());
		Assert_(pm != nil);
		
		Ptr aPtr = ::GetPixBaseAddr(pm);
		Int16 rowBytes = (*pm)->rowBytes & 0x3FFF;
		aPtr += rowBytes * p.v + (p.h/8);
		
		if (*aPtr & (1<<(7 - (p.h%8)))) 
			result = hot_Indicator;
	}
	
	return result;
}

Boolean	CAGASlider::PointInHotSpot(Point inPoint, Int16 inHotSpot)
{
	if (inHotSpot == 0) return false;
	
	return (FindHotSpot(inPoint) == inHotSpot);
}

void	CAGASlider::HotSpotAction(Int16 inHotSpot,
			Boolean inCurrInside, Boolean inPrevInside)
{	// Not used? Keep it for SimulateHotSpotClick().
	if (inHotSpot != 1) return;	// sanity check
	
	if (inCurrInside != inPrevInside) {
		if (inCurrInside) mState = ind_Pressed;
		else mState = ind_Enabled;
		Draw(nil);
	}
}

void	CAGASlider::HotSpotResult(Int16 inHotSpot)
{
	if (inHotSpot != 1) return;
	
	// done dragging, un-press the indicator
	mState = ind_Enabled;
	Draw(nil);
}

Boolean	CAGASlider::TrackHotSpot(Int16 inHotSpot, Point inPoint)
{
	Assert_ (inHotSpot == hot_Indicator || inHotSpot == hot_Track);
	
	// "Press" the indicator
	mState = ind_Pressed;	// should these be "St" objects?
	mIsTracking = true;
	mTrackingValue = GetValue();
	Draw(nil);
	
	// Track the mouse while it is down
	Point	currPt = inPoint;
	while (StillDown()) {
	
		::GetMouse(&currPt);
		
		// calculate ghost position
		Int32 currValue = PositionToValue(currPt);
		
		if (currValue != mTrackingValue) {
		
			mTrackingValue = currValue;
			
			if (mContinuousBroadcast) {
			
				if (mValueMessage != msg_Nothing) {
					Int32	value = mTrackingValue;
					BroadcastMessage(mValueMessage, (void*) &value);
				}
				
			}
			Draw(nil);
		}
	}
	
	mState = ind_Enabled;
									// Check if MouseUp occurred in HotSpot
	EventRecord	macEvent;			// Get location from MouseUp event
	if (::GetOSEvent(mUpMask, &macEvent)) {
		currPt = macEvent.where;
		::GlobalToLocal(&currPt);
		mTrackingValue = PositionToValue(currPt);
	}
	
	SetValue(mTrackingValue);	// SetValue() calls BroadcastValueMessage()
	Draw(nil);
	
	mIsTracking = false;
	
	return true;	// tell caller that mouse was released in hot-spot
}
