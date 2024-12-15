/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: COutputDialog.h,v 1.3.10.1 1997/12/05 22:14:27 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CModalDialogGrafPortView.h"

class COutputDialog	:	public	CModalDialogGrafPortView {
public:
	enum { class_ID = 'OutD' };

						COutputDialog(LStream * inStream);
	virtual				~COutputDialog();
	
	virtual Boolean		HandleKeyPress(const EventRecord & inKeyEvent);
	virtual void		ListenToMessage(MessageT inMessage, void * ioParam);
	
	void				SetText(Handle inTextH);
	Boolean				GetCopyToClipboard() { return mCopyToClipboard; }
	
protected:
	Boolean				mCopyToClipboard;
	
	virtual void		FinishCreateSelf();
};

	const ResIDT		kOutputDialogResID	=	11302;

