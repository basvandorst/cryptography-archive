/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CPassphraseDialog.h,v 1.7.8.1 1998/11/12 03:05:15 heller Exp $____________________________________________________________________________*/#pragma once#include <LPeriodical.h>#include "CPGPDiskDialog.h"const MessageT	msg_HideTyping = 1050;class CPassphraseEdit;class CPassphraseDialog : public CPGPDiskDialog, public LPeriodical{public:	enum { class_ID = 'PPhr' };							CPassphraseDialog();	virtual				~CPassphraseDialog();						CPassphraseDialog(LStream *inStream);	Boolean				GetPassphrase(StringPtr passphrase);	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);	void				SetPrompt(ConstStr255Param prompt);	virtual	void		SpendTime(const EventRecord &inMacEvent);	protected:	enum	{		kOKButtonPaneID				= 5001,		kCancelButtonPaneID			= 5002,		kDescriptionCaptionPaneID	= 5003,		kCapsLockDownCaptionPaneID	= 5004,		kPassphraseEditFieldPaneID	= 5005,		kHideTypingCheckboxPaneID	= 5006	};	CPassphraseEdit 	*mPassphraseFieldObj;	Boolean				mCapsLockMessageVisible;		virtual void		FinishCreateSelf(void);	virtual	void		PassphraseHasChanged(void);	virtual	void		SetHideTyping(Boolean hideTyping);};