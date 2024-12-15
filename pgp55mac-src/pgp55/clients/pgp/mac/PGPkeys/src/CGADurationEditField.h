/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGADurationEditField.h,v 1.2 1997/07/14 17:59:46 wprice Exp $
____________________________________________________________________________*/
#pragma once

typedef enum DurationType
{
	kDateDurationType = 0,
	kLengthDurationType,
	kTimeDurationType
} DurationType;

enum	{	kDownAdjust = -1, kUpAdjust = 1 };

class CGADurationEditField :	public LCaption,
								public LCommander
{
public:
	enum { class_ID = 'gDEF' };
						CGADurationEditField();
						CGADurationEditField(LStream * inStream);
						~CGADurationEditField();
						
	void				SetDurationType(DurationType type);
	void				SetDurationValue(UInt32 duration);
	UInt32				GetDurationValue();	// returns seconds
	virtual void		AdjustValue(Int16 direction);

	virtual Boolean		HandleKeyPress(const EventRecord&	inKeyEvent);
	virtual Boolean		ObeyCommand(
								CommandT			inCommand,
								void *				ioParam);
	virtual void		FindCommandStatus(
								CommandT			inCommand,
								Boolean &			outEnabled,
								Boolean &			outUsesMark,
								Char16 &			outMark,
								Str255				outName);
	virtual void		DrawSelf();
	virtual void		FinishCreateSelf();
	virtual void		ClickSelf(
							const SMouseDownEvent & inMouseDown);
	virtual void		AdjustCursorSelf(
							Point				inPortPt,
							const EventRecord &	inMacEvent);
	virtual void		BeTarget();
	virtual void		DontBeTarget();
	
protected:
	void				DrawBackground(const Rect & localFrame);
	void				DrawBox(const Rect & localFrame);
	void				DrawGrayBox(const Rect & localFrame);
	void				DrawText(const Rect & localFrame);
	Boolean				ValidDay(Int16 month, Int16 day, Int16 year);
private:
	
	DurationType		mType;
	Int16				mSelectedSection;
	Int16				mHorizontalCutoffs[4];
	Int16				mValues[4];
};

