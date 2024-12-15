/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CPGPGetPassphraseGrafPort.h,v 1.10 1997/10/10 00:59:07 heller Exp $
____________________________________________________________________________*/

#pragma once

#include "CPGPLibGrafPortView.h"
#include "pgpUserInterface.h"
#include "pgpPubTypes.h"

class CPassphraseEdit;
class CPrivateKeysPopup;
class LGAPopup;
class LGACheckbox;
class CUserIDTable;

class CPGPGetPassphraseGrafPort	:	public CPGPLibGrafPortView
{
public:
	enum { class_ID = 'GtPh' };
						CPGPGetPassphraseGrafPort(LStream *inStream);
	virtual				~CPGPGetPassphraseGrafPort();

	Boolean				BuildKeyList(PGPKeySetRef allKeys);
	void				GetPassphrase(char passphrase[256]);
	PGPGetPassphraseSettings	GetSettings(void);
	PGPKeyRef			GetTargetKey(void);
	virtual	void		ListenToMessage(MessageT inMessage, void *ioParam);
	virtual	Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	void				SetPrompt(ConstStr255Param prompt);
	void				SetSettings(PGPPrefRef clientPrefsRef,
								PGPGetPassphraseSettings defaultSettings,
								PGPGetPassphraseOptions dialogOptions);
	void				SetRecipients(PGPKeySetRef recipientSet,
								PGPUInt32 numMissingRecipients);
	void				SetTargetKey(PGPKeyRef signingKey);
	
	void				ResetTarget();

protected:

	virtual void		FinishCreateSelf(void);
		
private:
	
	LGACheckbox			*mTextOutputCheckbox;
	LGAPopup			*mMacBinaryPopup;
	LGACheckbox			*mDetachedSigCheckbox;

	CPassphraseEdit		*mPassphraseEdit;
	LGACheckbox			*mHideTypingCheckbox;
	Boolean				mHaveFileOptions;
	CPrivateKeysPopup	*mKeysPopup;
	CUserIDTable		*mRecipientTable;
};

