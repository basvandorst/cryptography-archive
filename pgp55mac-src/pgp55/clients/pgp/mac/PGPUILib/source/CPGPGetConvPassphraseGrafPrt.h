/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CPGPGetConvPassphraseGrafPrt.h,v 1.2 1997/09/20 01:36:21 heller Exp $
____________________________________________________________________________*/

#pragma once

#include "CPGPLibGrafPortView.h"
#include "pgpPubTypes.h"

class CPassphraseEdit;
class LGACheckbox;
class CProgressBar;

class CPGPGetConvPassphraseGrafPort	:	public CPGPLibGrafPortView
{
public:
	enum { class_ID = 'GCPh' };
						CPGPGetConvPassphraseGrafPort(LStream *inStream);
	virtual				~CPGPGetConvPassphraseGrafPort();

	void				ClearPassphrase(void);
	void				GetPassphrase(char passphrase[256],
									char confirmation[256]);
	virtual	void		ListenToMessage(MessageT inMessage, void *ioParam);
	virtual	Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	void				SetPrompt(ConstStr255Param prompt);

protected:

	virtual void		FinishCreateSelf(void);
		
private:
	
	CPassphraseEdit		*mPassphraseEdit;
	CPassphraseEdit		*mConfirmationEdit;
	LGACheckbox			*mHideTypingCheckbox;
	CProgressBar		*mPassQualityProgressBar;
	
	void				AdjustButtons(void);
};

