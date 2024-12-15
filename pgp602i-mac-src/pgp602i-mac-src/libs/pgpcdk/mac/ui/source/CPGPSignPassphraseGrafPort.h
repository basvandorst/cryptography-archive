/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CPGPSignPassphraseGrafPort.h,v 1.6.10.1 1998/11/12 03:20:50 heller Exp $____________________________________________________________________________*/#pragma once#include "CPGPKeyPassphraseGrafPort.h"class CPrivateKeysPopup;class CPassphraseEdit;class LStaticText;class CPGPSigningPassphraseGrafPortView :			public CPGPKeySetPassphraseGrafPortView{public:	enum	{		class_ID				= 'SPGP',				kKeysPopupPaneID		= 'pKey',		kPassphraseEditPaneID	= 'ePwd',		kSplitMsgCaptionPaneID	= 'cSpl'	};						CPGPSigningPassphraseGrafPortView(LStream *inStream);	virtual				~CPGPSigningPassphraseGrafPortView();	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);	PGPError			SetOptions(PGPContextRef context, 							CPGPSigningPassphraseDialogOptions *options);							protected:	CPrivateKeysPopup	*mKeysPopup;	CPassphraseEdit		*mPassphraseEdit;	LStaticText			*mSplitMsgText;		virtual PGPKeyRef	FindKeyForPassphrase(PGPKeySetRef keySet,								const char *passphrase);	virtual void		FinishCreateSelf(void);	virtual PGPKeyRef	GetPassphraseKey(void);	virtual	MessageT	HandleMessage(MessageT theMessage);	virtual Boolean		VerifyPassphrase(void);	private:	const CPGPSigningPassphraseDialogOptions	*mOptions;	Boolean										mShowingSplitKey;		void				AdjustDisplay(void);};