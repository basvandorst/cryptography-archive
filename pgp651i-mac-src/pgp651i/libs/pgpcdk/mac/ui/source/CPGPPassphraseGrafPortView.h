/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CPGPPassphraseGrafPortView.h,v 1.11 1999/03/10 02:37:24 heller Exp $____________________________________________________________________________*/#pragma once#include "pgpDialogs.h"#include "pgpUserInterface.h"#include "pgpPubTypes.h"#include "CPGPUIGrafPortView.h"class CPassphraseEdit;class LCheckBox;class LPushButton;class LStaticText;class CPGPPassphraseGrafPortView : public CPGPUIGrafPortView{public:	enum	{		class_ID = 'PGPV',				kPassphraseEditFieldPaneID		= 'ePwd',		kHideTypingCheckboxPaneID		= 'HTyp',		kHideTypingPositioningPaneID	= 'TypP',		kPromptTextPaneID				= 'cTxt',		kCapsLockDownTextPaneID			= 'cCLD',		kOptionsButtonPaneID			= 'bOpt',				msg_HideTyping				= kHideTypingCheckboxPaneID,		msg_Options					= kOptionsButtonPaneID,		msg_ChangedPassphrase		= kPassphraseEditFieldPaneID	};						CPGPPassphraseGrafPortView(LStream *inStream);	virtual				~CPGPPassphraseGrafPortView();	virtual void		DoIdle(const EventRecord &inMacEvent);	virtual void		ClearPassphrase(void);	virtual	void		ListenToMessage(MessageT inMessage, void *ioParam);	virtual	Boolean		HandleKeyPress(const EventRecord &inKeyEvent);	virtual void		SetDescriptor(ConstStringPtr prompt);	PGPError			SetOptions(PGPContextRef context, 							CPGPPassphraseDialogOptions *options);	protected:	CPassphraseEdit		*mPassphraseEdit;	LCheckBox			*mHideTypingCheckbox;	LPushButton			*mOptionsButton;	LStaticText			*mCapsLockMessage;	PGPBoolean			mShowingCapsLockMessage;		virtual	void		AdjustButtons(void);	virtual void		FinishCreateSelf(void);	virtual	MessageT	HandleMessage(MessageT theMessage);	virtual void		DoOptionsDialog(void);	private:		const CPGPPassphraseDialogOptions 	*mOptions;};