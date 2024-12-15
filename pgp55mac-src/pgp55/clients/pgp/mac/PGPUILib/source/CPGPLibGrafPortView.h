/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CPGPLibGrafPortView.h,v 1.5 1997/10/30 20:53:01 heller Exp $
____________________________________________________________________________*/

#pragma once

#include <LGrafPortView.h>
#include <LListener.h>

#include "PGPPubTypes.h"

class LGAPushButton;
class LArray;
struct SavedWindowBitsInfo;

class CPGPLibGrafPortView : public LGrafPortView,
							public LListener
{
public:
	enum { class_ID = 'Ppvw' };

						CPGPLibGrafPortView(LStream *inStream);
	virtual				~CPGPLibGrafPortView(void);
						
	virtual void		DoIdle(const EventRecord &inMacEvent);
	virtual void		Draw(RgnHandle	inSuperDrawRgnH);
	virtual void		FinishCreateSelf(void);
	DialogRef			GetDialog(void);
	virtual void		GlobalToPortPoint(Point &ioPoint) const;
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void		PortToGlobalPoint(Point &ioPoint) const;
	void				RememberInvalidWindow(WindowRef theWindow);
	virtual	void 		UpdatePort(void);
	
	MessageT			GetDismissMessage(void)
									{ return( mDismissMessage ); };
	void				SetDismissMessage(MessageT message)
									{ mDismissMessage = message; };

	static CPGPLibGrafPortView 	*CreateDialog(ResIDT dialogResID);
	static pascal Boolean		ModalFilterProc(DialogPtr theDialog,
										EventRecord *theEvent,
										short *itemHit);
	
protected:

	MessageT			mDismissMessage;
	LGAPushButton		*mOKButton;
	LGAPushButton		*mCancelButton;
	LArray				*mInvalidWindowList;
	Boolean				mWindowIsMoveable;
	Boolean				mWindowIsResizeable;
	Rect				mMinMaxSize;
	Boolean				mSaveRestoreWindowBits;
	SavedWindowBitsInfo	*mSavedWindowBitsList;
	SDimension16		mStandardSize;
	Rect				mUserBounds;
	Boolean				mMoveOnlyUserZoom;
	
	Boolean				ClickInDrag(const EventRecord &inMacEvent);
	Boolean				ClickInGrow(const EventRecord &inMacEvent);
	Boolean				ClickInZoom(const EventRecord &inMacEvent,
								Int16 inZoomDirection);
	void				SaveAllWindowBits(void);
	void				SaveWindowBits(WindowRef theWindow);
	void				RestoreWindowBits(WindowRef theWindow);
	Boolean				CalcStandardBounds(Rect &outStdBounds) const;
	void				CalcStandardBoundsForScreen(
								const Rect &inScreenBounds,
								Rect &outStdBounds) const;
	void				DoSetZoom(Boolean inZoomToStdState);
	void				ResizeWindowTo(Int16 newWidth, Int16 newHeight);
	
private:

	CPGPLibGrafPortView	*mNextView;
};
