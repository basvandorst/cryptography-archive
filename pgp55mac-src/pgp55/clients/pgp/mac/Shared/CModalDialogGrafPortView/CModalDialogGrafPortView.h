/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CModalDialogGrafPortView.h,v 1.4.10.1 1997/12/05 22:14:38 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include "LGrafPortView.h"
#include "LListener.h"
#include "CComboError.h"


class CModalDialogGrafPortView : public LGrafPortView,
								 public LListener {
public:
	static CModalDialogGrafPortView *	CreateCModalDialogGrafPortView(
											ResIDT inGrafPortView,
											LCommander * inSuperCommander);
											
										CModalDialogGrafPortView(
											LStream * inStream);
	virtual								~CModalDialogGrafPortView();
	
	virtual void						Run(ModalFilterProcPtr inFilterProc
												= nil);
	
	virtual void						ListenToMessage(MessageT inMessage,
											void * ioParam)
											{ (void) inMessage;
											(void) ioParam; }

	virtual void						Show();
	
	static pascal Boolean 				ModalDialogFilterProc(
											DialogPtr theDialog, 
											EventRecord * theEvent,
											SInt16 * itemHit);	
	
protected:
	ModalFilterProcPtr					mFilterProc;
	Boolean								mExitDialog;
	CComboError							mError;
	GrafPtr								mSavedPort;
	LArray								mUpdateWindows;
	
	virtual void						DrawSelf();
};
