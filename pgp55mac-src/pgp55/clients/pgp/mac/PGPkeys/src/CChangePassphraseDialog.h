/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CChangePassphraseDialog.h,v 1.3 1997/08/10 10:47:58 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpKeys.h"

class CPassphraseEdit;
class CProgressBar;

const MessageT	kChangePassphraseSuccessful	=	'PASd';

class CChangePassphraseDialog		:		public LGADialogBox,
											public LListener,
											public LBroadcaster
{
public:
	enum { class_ID = 'PASd' };
					CChangePassphraseDialog();
					CChangePassphraseDialog(LStream *inStream);
					~CChangePassphraseDialog();
	void			ListenToMessage(MessageT inMessage, void *ioParam);
	void			FinishCreateSelf();
	
	void			SetKey(PGPKeyRef key);
private:
	Boolean			AcceptBadPassphrase();
	
	CPassphraseEdit	*mOldPassphraseEditField,
					*mNewPassphraseEditField,
					*mConfirmationEditField;
	LGACheckbox		*mHideTypingCheckbox;
	CProgressBar	*mPassQualityProgressBar;
	PGPKeyRef		mKey;
	
	enum	{	
				kOKButton						= 'bOK ',
				kCancelButton					= 'bCan',
				kOldPassphraseField 			= 'eOld',
				kNewPassphraseField 			= 'ePas',
				kConfirmPassphraseField			= 'eCon',
				kHideTypingCheckbox				= 'xHid',
				kPassQualityProgressBar			= 'pbPQ',
				kRecommendedQualityProgressBar	= 'pbRQ',
				kRecommendedQualityCaption		= 'cRQ1'
			};
	enum	{
				kStringListID		= 1007,
				kPassNoMatchID		= 1,
				kBadPassID,
				kSuccessID,
				kChangeErrorID,
				kBadPassphraseStrIndex,
				kAcceptBadPassphraseButtonStrIndex,
				kRejectBadPassphraseButtonStrIndex,
				kBadQualityErrorStrIndex,
				kBadLength1ErrorStrIndex,
				kBadLength2ErrorStrIndex
			};
};

