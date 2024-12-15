/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGADurationEditField.cp,v 1.3 1997/07/28 10:24:58 wprice Exp $
____________________________________________________________________________*/
#include "CGADurationEditField.h"


#include "MacStrings.h"

const Int16	kMaxYear			= 38;
const Int16 kMinYear			= 50;
const Int16 kMaxFullYear		= 2038;
const Int16 kMinFullYear		= 1950;

CGADurationEditField::CGADurationEditField()
	: LCaption()
{
	SetDurationType(kLengthDurationType);
	mSelectedSection = 0;
}

CGADurationEditField::CGADurationEditField(LStream * inStream)
	: LCaption(inStream)
{
	SetDurationType(kLengthDurationType);
	mSelectedSection = 0;
}

CGADurationEditField::~CGADurationEditField()
{
}

	void
CGADurationEditField::SetDurationType(DurationType type)
{
	mType = type;
	switch(mType)
	{
		case kDateDurationType:
			// Should initialize with today's date
			mValues[0] = 1;
			mValues[1] = 1;
			mValues[2] = 1997;
			mValues[3] = 0;
			break;
		case kLengthDurationType:
			// Initialize to 10 minutes
			mValues[0] = 0;
			mValues[1] = 10;
			mValues[2] = 0;
			mValues[3] = 0;
			break;
		case kTimeDurationType:
			// Initialize to 12:00:00 PM
			mValues[0] = 12;
			mValues[1] = 0;
			mValues[2] = 0;
			mValues[3] = 1;
			break;
	}
}

	void
CGADurationEditField::SetDurationValue(UInt32 duration)
{
	// duration is either a GetDateTime-style value
	// or a number of seconds duration (either way it
	// is a number of seconds actually, but for time
	// and date styles it has a base of midnight,
	// Jan 1, 1904
	DateTimeRec dtr;
	
	mValues[3] = 0;
	switch(mType)
	{
		case kDateDurationType:
			SecondsToDate(duration, &dtr);
			mValues[0] = dtr.month;
			mValues[1] = dtr.day;
			mValues[2] = dtr.year;
			break;
		case kLengthDurationType:
			mValues[0] = duration / 60 / 60;
			mValues[1] = duration / 60 - (mValues[0] * 60);
			mValues[2] = duration % 60;
			break;
		case kTimeDurationType:
			SecondsToDate(duration, &dtr);
			mValues[0] = (dtr.hour > 12) ? dtr.hour - 12 : dtr.hour;
			mValues[1] = dtr.minute;
			mValues[2] = dtr.second;
			mValues[3] = (dtr.hour > 12) ? 1 : 0;
			break;
	}
}

	UInt32
CGADurationEditField::GetDurationValue()
{
	UInt32	value;
	Int16	hour24;
	
	switch(mType)
	{
		case kDateDurationType:
			{
				DateTimeRec dtr;
				
				dtr.year		= mValues[2];
				dtr.month		= mValues[0];
				dtr.day			= mValues[1];
				dtr.hour		= 0;
				dtr.minute		= 0;
				dtr.second		= 0;
				dtr.dayOfWeek	= 0;
				DateToSeconds(&dtr, &value);
			}
			break;
		case kLengthDurationType:
			value = (mValues[0] * 60 * 60) +
					(mValues[1] * 60) +
					mValues[2];
			break;
		case kTimeDurationType:
			hour24 = mValues[0];
			if(mValues[3] && mValues[0] < 12)
				hour24 = mValues[0] + 12;
			else if(!mValues[3] && mValues[0] == 12)
				hour24 = 0;
			value = (hour24 * 60 * 60) +
					(mValues[1] * 60) +
					mValues[2];
			break;
	}
	return value;
}

	Boolean
CGADurationEditField::ObeyCommand(
	CommandT			inCommand,
	void *				ioParam)
{
	Boolean cmdHandled;

	cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
	return cmdHandled;
}

	void
CGADurationEditField::FindCommandStatus(
	CommandT			inCommand,
	Boolean &			outEnabled,
	Boolean &			outUsesMark,
	Char16 &			outMark,
	Str255				outName)
{
	LCommander::FindCommandStatus(inCommand, outEnabled,
									outUsesMark, outMark, outName);
}

	void
CGADurationEditField::FinishCreateSelf()
{
	LPane::FinishCreateSelf();
}

	void
CGADurationEditField::ClickSelf(
	const SMouseDownEvent & inMouseDown)
{
	Int16	oldSection;
	Int16	valueIndex;
	Rect	localFrame;
	
	LPane::ClickSelf(inMouseDown);
	oldSection = mSelectedSection;
	for(valueIndex = 0;valueIndex < 4;valueIndex++)
	{
		if(inMouseDown.whereLocal.h <= mHorizontalCutoffs[valueIndex])
		{
			mSelectedSection = valueIndex;
			break;
		}
	}
	if(valueIndex >= 3 && mType != kTimeDurationType)
		mSelectedSection = oldSection;
	if (!IsTarget())
		SwitchTarget(this);
	else
	{
		FocusDraw();
		CalcLocalFrameRect (localFrame);
		DrawText(localFrame);
	}
}

	void
CGADurationEditField::AdjustCursorSelf(
	Point				inPortPt,
	const EventRecord &	inMacEvent)
{
	LPane::AdjustCursorSelf(inPortPt, inMacEvent);
}

	void
CGADurationEditField::BeTarget()
{
	LCommander::BeTarget();
	Refresh();	
}

	void
CGADurationEditField::DontBeTarget()
{
	LCommander::DontBeTarget();
	Refresh();
}

	void
CGADurationEditField::DrawSelf()
{
	StColorPenState	theColorPenState;
	Rect localFrame;
	
	CalcLocalFrameRect (localFrame);
	::RGBBackColor (&(UGAColorRamp::GetWhiteColor()));
	DrawBackground(localFrame);
	DrawBox(localFrame);
	DrawText(localFrame);
}

	void
CGADurationEditField::DrawBackground(
	const Rect & localFrame)
{
	Rect frameCopy = localFrame;
	
	::InsetRect(&frameCopy, 1, 1);
	::EraseRect(&frameCopy);
}

	void
CGADurationEditField::DrawBox(
	const Rect & localFrame)
{
	StColorPenState	theColorPenState;
	StDeviceLoop	theLoop (localFrame);
	Rect			innerFrame;
	Int16			depth;
	
	::PenNormal();
	innerFrame = localFrame;
	::InsetRect(&innerFrame, 1, 1);
	while (theLoop.NextDepth (depth)) 
	{
		if (depth >= 8)
		{
			DrawGrayBox(localFrame);
			::ForeColor(blackColor);
			::FrameRect(&innerFrame);
		}
		else
			::FrameRect(&localFrame);
	}
}
	
	void
CGADurationEditField::DrawGrayBox(
	const Rect & localFrame)
{
	Rect frameCopy = localFrame;
	
	::RGBForeColor (&(UGAColorRamp::GetColor(7)));
	UGraphicsUtilities::TopLeftSide (&frameCopy, 0, 0, 1, 1);

	::RGBForeColor (&(UGAColorRamp::GetWhiteColor()));
	UGraphicsUtilities::BottomRightSide (&frameCopy, 1, 1, 0, 0);
}

	void
CGADurationEditField::DrawText(
	const Rect & localFrame)
{
	Rect		frame = localFrame,
				valFrame;
	Str255		valString;
	Int16		just;
	Int16		valueIndex,
				width;
	char		divChar;
	FontInfo	fInfo;
	
	::InsetRect(&frame, 4, 2);
	just = UTextTraits::SetPortTextTraits(mTxtrID);
	::ForeColor(blackColor);
	::BackColor(whiteColor);
	::GetFontInfo(&fInfo);
	valFrame = frame;
	valFrame.top = valFrame.bottom - fInfo.descent - fInfo.ascent;
	for(valueIndex = 0 ; valueIndex < 4 ; valueIndex++)
	{
		if(valueIndex == 3)
		{
			if(mType == kTimeDurationType)
			{
				if(mValues[3])
					CopyPString("\pPM", valString);
				else
					CopyPString("\pAM", valString);
			}
			else
				break;
		}
		if(valueIndex < 3)
		{
			::NumToString(mValues[valueIndex], valString);
			if(valString[0] == 1)
			{
				valString[0]++; valString[2] = valString[1];
				valString[1] = '0';
			}
		}
		width = StringWidth(valString);
		::MoveTo(valFrame.left, valFrame.bottom - fInfo.descent);
		::DrawString(valString);
		valFrame.right = mHorizontalCutoffs[valueIndex] =
				valFrame.left + width;
		if(IsTarget() && mSelectedSection == valueIndex)
		{
			UDrawingUtils::SetHiliteModeOn();
			::InvertRect(&valFrame);
		}
		valFrame.left += width;
		// Draw dividing character
		divChar = 0xCA;
		switch(mType)
		{
			case kDateDurationType:
				if(valueIndex < 2)
					divChar = '/';
				break;
			case kLengthDurationType:
				if(valueIndex < 2)
					divChar = ':';
				break;
			case kTimeDurationType:
				if(valueIndex < 2)
					divChar = ':';
				break;
		}
		valString[0] = 1; valString[1] = divChar;
		width = StringWidth(valString);
		::MoveTo(valFrame.left, valFrame.bottom - fInfo.descent);
		::DrawString(valString);
		valFrame.left += width;
	}
}

	Boolean
CGADurationEditField::ValidDay(Int16 month, Int16 day, Int16 year)
{
	Int16 maxDay;
	
	switch(month)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			maxDay = 31;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			maxDay = 30;
			break;
		case 2:
			if(!(year % 4) && ((year % 100) || !(year % 400)))
				maxDay = 29;	// its a leap year
			else
				maxDay = 28;
			break;
	}
	if(day > maxDay || day < 1)
		return FALSE;
	else
		return TRUE;
}

	Boolean
CGADurationEditField::HandleKeyPress(const EventRecord&	inKeyEvent)
{
	Boolean		keyHandled	= true;
	Int16		theKey		= inKeyEvent.message & charCodeMask;
	Rect		localFrame;
	
	if((inKeyEvent.modifiers & cmdKey) ||
		(theKey == char_Return) || (theKey == char_Enter))
		keyHandled = LCommander::HandleKeyPress(inKeyEvent);
	else
	{
		SetUpdateCommandStatus(TRUE);
		if(theKey == char_Tab)
		{
			if(mSelectedSection < ((mType == kTimeDurationType) ? 3 : 2))
				mSelectedSection++;
			else
			{
				// switch to next target....
				keyHandled = LCommander::HandleKeyPress(inKeyEvent);
			}
		}
		else if((theKey >= '0') && (theKey <= '9'))
		{
			Int16	val = theKey - '0',
					newVal,
					newVal1,
					newVal2,
					curVal;
			
			if(mSelectedSection == 3 && mType == kTimeDurationType)
				mValues[3] = !mValues[3];
			else
			{
				curVal = mValues[mSelectedSection];
				if(mType == kDateDurationType && mSelectedSection == 2)
				{
					if(curVal >= 2000)
						curVal -= 2000;
					else
						curVal -= 1900;
				}
				newVal1 = curVal * 10 + val;
				newVal2 = (curVal - ((curVal / 10) * 10)) * 10 + val;
				switch(mType)
				{
					case kDateDurationType:
						switch(mSelectedSection)
						{
							case 0:
								if(newVal1 >= 1 && newVal1 <= 12)
									newVal = newVal1;
								else if(newVal2 >= 1 && newVal2 <= 12)
									newVal = newVal2;
								else
									newVal = val;
								if(newVal < 1)
									newVal = 1;
								if(!ValidDay(newVal, mValues[1],
												mValues[2]))
									mValues[1] = 1;
								break;
							case 1:
								if(newVal1 >= 1 && newVal1 <= 31 &&
									ValidDay(mValues[0], newVal1,
												mValues[2]))
									newVal = newVal1;
								else if(newVal2 >= 1 && newVal2 <= 31 &&
									ValidDay(mValues[0], newVal2,
												mValues[2]))
									newVal = newVal2;
								else
									newVal = val;
								if(newVal < 1)
									newVal = 1;
								break;
							case 2:
								if(newVal1 <= kMaxYear)
									newVal1 += 2000;
								else
									newVal1 += 1900;
								if(newVal1 >= kMinFullYear &&
										newVal1 <= kMaxFullYear)
									newVal = newVal1;
								else
								{
									if(newVal2 <= kMaxYear)
										newVal2 += 2000;
									else
										newVal2 += 1900;
									if(newVal2 >= kMinFullYear &&
											newVal2 <= kMaxFullYear)
										newVal = newVal2;
									else if(val == 0)
										newVal = 2000;
									else
										newVal = mValues[mSelectedSection];
								}
								if(!ValidDay(mValues[0], mValues[1],
												newVal))
									mValues[1] = 1;
								break;
						}
						break;
					case kLengthDurationType:
						switch(mSelectedSection)
						{
							case 0:
								if(newVal1 >= 0 && newVal1 <= 99)
									newVal = newVal1;
								else if(newVal2 >= 0 && newVal2 <= 99)
									newVal = newVal2;
								else
									newVal = val;
								break;
							case 1:
							case 2:
								if(newVal1 >= 0 && newVal1 <= 59)
									newVal = newVal1;
								else if(newVal2 >= 0 && newVal2 <= 59)
									newVal = newVal2;
								else
									newVal = val;
								break;
						}
						break;
					case kTimeDurationType:
						switch(mSelectedSection)
						{
							case 0:
								if(newVal1 >= 1 && newVal1 <= 12)
									newVal = newVal1;
								else if(newVal2 >= 1 && newVal2 <= 12)
									newVal = newVal2;
								else
									newVal = val;
								if(newVal < 1)
									newVal = 1;
								break;
							case 1:
							case 2:
								if(newVal1 >= 0 && newVal1 <= 59)
									newVal = newVal1;
								else if(newVal2 >= 0 && newVal2 <= 59)
									newVal = newVal2;
								else
									newVal = val;
								break;
						}
						break;
				}
				mValues[mSelectedSection] = newVal;
			}
		}
		FocusDraw();
		CalcLocalFrameRect (localFrame);
		DrawText(localFrame);
	}
	return keyHandled;
}

	void
CGADurationEditField::AdjustValue(Int16 direction)
{
	Int16	newVal1,
			newVal;
	Rect	localFrame;
	
	if(mSelectedSection == 3 && mType == kTimeDurationType)
		mValues[3] = !mValues[3];
	else
	{
		newVal = mValues[mSelectedSection];
		newVal1 = mValues[mSelectedSection] + direction;
		switch(mType)
		{
			case kDateDurationType:
				switch(mSelectedSection)
				{
					case 0:
						if(newVal1 >= 1 && newVal1 <= 12)
							newVal = newVal1;
						if(!ValidDay(newVal, mValues[1], mValues[2]))
							mValues[1] = 1;
						break;
					case 1:
						if(newVal1 >= 1 && newVal1 <= 31)
							newVal = newVal1;
						if(!ValidDay(mValues[0], newVal, mValues[2]))
							newVal = 1;
						break;
					case 2:
						if(newVal1 >= kMinFullYear &&
								newVal1 <= kMaxFullYear)
							newVal = newVal1;
						if(!ValidDay(mValues[0], mValues[1], newVal))
							mValues[1] = 1;
						break;
				}
				break;
			case kLengthDurationType:
				switch(mSelectedSection)
				{
					case 0:
						if(newVal1 >= 0 && newVal1 <= 99)
							newVal = newVal1;
						break;
					case 1:
					case 2:
						if(newVal1 >= 0 && newVal1 <= 59)
							newVal = newVal1;
						break;
				}
				break;
			case kTimeDurationType:
				switch(mSelectedSection)
				{
					case 0:
						if(newVal1 >= 1 && newVal1 <= 12)
							newVal = newVal1;
						break;
					case 1:
					case 2:
						if(newVal1 >= 0 && newVal1 <= 59)
							newVal = newVal1;
						break;
				}
				break;
		}
		mValues[mSelectedSection] = newVal;
	}
	if (!IsTarget())
		SwitchTarget(this);
	else
	{
		FocusDraw();
		CalcLocalFrameRect(localFrame);
		DrawText(localFrame);
	}
}

