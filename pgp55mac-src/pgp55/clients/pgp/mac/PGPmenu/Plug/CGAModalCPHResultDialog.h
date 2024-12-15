/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CGAModalCPHResultDialog.h,v 1.2.10.1 1997/12/05 22:14:26 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "CPGPEncoderDecoder.h"
#include "CModalDialogGrafPortView.h"

class CGAModalCPHResultDialog	:	public	CModalDialogGrafPortView {
public:
	enum { class_ID = 'CPHG' };

						CGAModalCPHResultDialog(LStream * inStream);
	virtual				~CGAModalCPHResultDialog();
	
	virtual Boolean		HandleKeyPress(const EventRecord & inKeyEvent);
	virtual void		ListenToMessage(MessageT inMessage, void * ioParam);
	
	void				SetStatusMessage(SStatusMessage & inStatusMessage);
	
protected:
	Boolean				mValidSignature;
	UInt16				mValidity;
	
	virtual void		FinishCreateSelf();
	
	virtual void		DrawSelf();
};

	const ResIDT		kModalCPHResultsDialogResID	=	1101;

