/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyGenWizardDialog.h,v 1.9 1997/09/12 22:00:49 heller Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpKeys.h"
#include "pgpEncode.h"

class CKeyGenWizardView;
class CKeyTable;

class CKeyGenWizardDialog	:	public LGADialogBox,
								public LListener
{
public:
	enum { class_ID = 'dKGW' };
	
								CKeyGenWizardDialog(LStream *inStream);
	virtual						~CKeyGenWizardDialog();

	virtual void				FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											Char16 &outMark,
											Str255 outName);
	virtual	void				ListenToMessage(MessageT inMessage,
												void *ioParam);
	
	virtual void				SetKeyTable(CKeyTable *keyTable);

	static Boolean				IsGoodPassphrase(
									ConstStr255Param	passphrase,
									ConstStr255Param	confirmation,
									Boolean				&confirmMatch,
									Boolean				&enforceLength,
									Boolean				&goodLength,
									Boolean				&goodQuality);
protected:

	virtual	void				FinishCreateSelf(void);

private:

	// It is assumed that this enum is kept in user presentation order.
	typedef enum KeyGenerationView
	{
		kViewInvalid			= 1,
		kViewIntroduction,
		kViewNameAndEmailAddress,
		kViewKeyType,
		kViewKeySize,
		kViewKeyExpiration,
		kViewKeyMRK,
		kViewSignCorp,
		kViewKeyPassphrase,
		kViewKeyBadPassphrase,
		kViewGetRandomData,
		kViewGeneratingKey,
		kViewSendToServer,
		kViewConclusion,
		kViewExit			// Not a real view
	} KeyGenerationView;


	KeyGenerationView			mCurrentView;
	CKeyGenWizardView			*mCurrentViewObj;
	CKeyTable					*mParentKeyTable;
	PGPKeyRef					mGeneratedKey;	// invalid until post-gen
	PGPKeyRef					mMRKKey;
	PGPKeyRef					mCorpKey;
	Boolean						mFinished;
	LGAPushButton				*mNextButton;
	LGAPushButton				*mPreviousButton;
	LGAPushButton				*mCancelButton;
	Str255						mName;
	Str255						mEmailAddress;
	UInt32						mEncryptKeySize;
	UInt32						mSigningKeySize;
	PGPPublicKeyAlgorithm		mAlgorithm;
	UInt32						mExpirationDays;
	Str255						mPassphrase,
								mConfirmation;
	Boolean						mPassphraseMatched,
								mPassphraseGoodLength,
								mPassphraseGoodQuality,
								mPassphraseEnforceLength,
								mAllowRSAKeygen;

	static Boolean				mGeneratingKey;
	static PGPError				mKeyGenerationError;
	
	void						AdjustControlsForView(
									KeyGenerationView theView);
	Boolean						FinishedKeyGeneration(void)
									{ return( mFinished ); };
	Boolean						GenerateKey(void);
	void						GotoNextView(void);
	void						GotoPreviousView(void);
	Boolean						LeaveCurrentView(KeyGenerationView newView);
	void						SwitchToView(	KeyGenerationView newView,
												Boolean increment);
	void						EnterView(KeyGenerationView newView);
	Boolean						ValidateCurrentView(void);
	
	static PGPError				GenerateKeyCallback(
												PGPContextRef	context,
												PGPEvent		*event,
												PGPUserValue	userValue);
};

