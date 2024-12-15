/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: CServerStatusDialog.cp,v 1.5 1999/03/10 02:39:41 heller Exp $____________________________________________________________________________*/#include <UDrawingState.h>#include <PP_Messages.h>#include "CServerStatusDialog.h"const PaneIDT	caption_Server	= 1000;const PaneIDT	caption_Status	= 1001;CServerStatusDialog::CServerStatusDialog(	LStream *	inStream)		: CClientKSGrafPortView(inStream){}CServerStatusDialog::~CServerStatusDialog(){}	voidCServerStatusDialog::SetServerCaption(	ConstStringPtr	inServer){	StColorPortState	theSavedPort(UQDGlobals::GetCurrentPort());		OutOfFocus(nil);	FindPaneByID(caption_Server)->SetDescriptor(inServer);}	voidCServerStatusDialog::SetStatusCaption(	ConstStringPtr	inStatus){	StColorPortState	theSavedPort(UQDGlobals::GetCurrentPort());		OutOfFocus(nil);	FindPaneByID(caption_Status)->SetDescriptor(inStatus);}CClientKSGrafPortView::CClientKSGrafPortView(LStream *inStream)	: CPGPModalGrafPortView(inStream){}CClientKSGrafPortView::~CClientKSGrafPortView(){}	voidCClientKSGrafPortView::DoIdle(const EventRecord &inMacEvent){	(void) inMacEvent;		// Force ModalDialog to exit.	SetDismissMessage( msg_OK );}