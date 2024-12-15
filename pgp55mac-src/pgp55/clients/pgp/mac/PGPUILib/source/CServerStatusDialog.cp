/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CServerStatusDialog.cp,v 1.2.10.1 1997/12/05 22:13:33 mhw Exp $
____________________________________________________________________________*/

#include <LGACaption.h>
#include <LGAPushButton.h>

#include "CSharedProgressBar.h"
#include "CServerStatusDialog.h"


const PaneIDT	caption_Server		=	1000;
const PaneIDT	caption_Status		=	1001;
const PaneIDT	view_ProgressBar	=	1002;


CServerStatusDialog::CServerStatusDialog(
	LStream *	inStream)
		: CPGPLibGrafPortView(inStream)
{
}

CServerStatusDialog::~CServerStatusDialog()
{
}

	void
CServerStatusDialog::FinishCreateSelf()
{
	CPGPLibGrafPortView::FinishCreateSelf();
	StartRepeating();
}

	void
CServerStatusDialog::SpendTime(
	const EventRecord &	inMacEvent)
{
	#pragma unused(inMacEvent)
	
	StColorPortState	theSavedPort(UQDGlobals::GetCurrentPort());
	
	OutOfFocus(nil);
	((CSharedProgressBar *) FindPaneByID(view_ProgressBar))->
			SetPercentComplete(CSharedProgressBar::kInfinitePercent);
}

	void
CServerStatusDialog::SetServerCaption(
	ConstStringPtr	inServer)
{
	StColorPortState	theSavedPort(UQDGlobals::GetCurrentPort());
	
	OutOfFocus(nil);
	FindPaneByID(caption_Server)->SetDescriptor(inServer);
}

	void
CServerStatusDialog::SetStatusCaption(
	ConstStringPtr	inStatus)
{
	StColorPortState	theSavedPort(UQDGlobals::GetCurrentPort());
	
	OutOfFocus(nil);
	FindPaneByID(caption_Status)->SetDescriptor(inStatus);
}

