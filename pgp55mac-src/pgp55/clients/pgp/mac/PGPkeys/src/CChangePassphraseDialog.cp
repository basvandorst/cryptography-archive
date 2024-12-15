/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CChangePassphraseDialog.cp,v 1.9 1997/09/17 19:36:34 wprice Exp $
____________________________________________________________________________*/
#include "CChangePassphraseDialog.h"
#include "CPassphraseEdit.h"
#include "CProgressBar.h"
#include "ChoiceDialogBox.h"
#include "CKeyGenWizardDialog.h"
#include "CWarningAlert.h"
#include "CPGPKeys.h"
#include "CSecureMemory.h"
#include "MacStrings.h"

#include "pgpAdminPrefs.h"
#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpFeatures.h"
#include "pgpErrors.h"
#include "pgpUserInterface.h"

CChangePassphraseDialog::CChangePassphraseDialog()
{
}

CChangePassphraseDialog::CChangePassphraseDialog(LStream *inStream)
	: LGADialogBox(inStream)
{
}

CChangePassphraseDialog::~CChangePassphraseDialog()
{
}

	void
CChangePassphraseDialog::FinishCreateSelf()
{
	LGADialogBox::FinishCreateSelf();
	
	mOldPassphraseEditField =	(CPassphraseEdit *)
		FindPaneByID(kOldPassphraseField);
	pgpAssertAddrValid(mOldPassphraseEditField,		VoidAlign);
	mConfirmationEditField =	(CPassphraseEdit *)
		FindPaneByID(kConfirmPassphraseField);
	pgpAssertAddrValid(mConfirmationEditField,		VoidAlign);
	mNewPassphraseEditField =	(CPassphraseEdit *)
		FindPaneByID(kNewPassphraseField);
	pgpAssertAddrValid(mNewPassphraseEditField,		VoidAlign);
	mNewPassphraseEditField->AddListener(this);
	mHideTypingCheckbox = 		(LGACheckbox *)
		FindPaneByID(kHideTypingCheckbox);
	pgpAssertAddrValid(mHideTypingCheckbox,			LGACheckbox);
	mPassQualityProgressBar = 	(CProgressBar *)
		FindPaneByID(kPassQualityProgressBar);
	pgpAssertAddrValid(mPassQualityProgressBar,		VoidAlign);
	
	mHideTypingCheckbox->AddListener(this);
	
#if PGP_BUSINESS_SECURITY
	{
		Boolean		enforceMinQuality;
		PGPUInt32	minQuality;
		PGPError	err;
		
		err = PGPGetPrefBoolean(
				gAdminPrefRef, kPGPPrefEnforceMinQuality, &enforceMinQuality);
		pgpAssertNoErr(err);
		if(IsntPGPError(err) && enforceMinQuality)
		{
			err = PGPGetPrefNumber(
					gAdminPrefRef, kPGPPrefMinQuality, &minQuality);
			pgpAssertNoErr(err);
			if(IsntPGPError(err))
			{
				CProgressBar	*recQualityBar;
				
				((LCaption *)FindPaneByID(kRecommendedQualityCaption))->
					Show();
				recQualityBar =
				(CProgressBar *)FindPaneByID(kRecommendedQualityProgressBar);
				recQualityBar->Show();
				recQualityBar->SetPercentComplete(minQuality);
			}
		}
	}
#endif
}

	void
CChangePassphraseDialog::SetKey(PGPKeyRef key)
{
	mKey = key;
	
	Show();
}

	Boolean
CChangePassphraseDialog::AcceptBadPassphrase()
{
	Str255	okButtonTitle, cancelButtonTitle, errorString;
	
	::GetIndString(errorString, kStringListID,
		kBadPassphraseStrIndex );
	pgpAssert(errorString[ 0 ] > 0);
	::GetIndString(okButtonTitle, kStringListID,
		kRejectBadPassphraseButtonStrIndex);
	pgpAssert(okButtonTitle[ 0 ] > 0);
	::GetIndString(cancelButtonTitle, kStringListID,
		kAcceptBadPassphraseButtonStrIndex );
	pgpAssert(cancelButtonTitle[ 0 ] > 0);
	
	ChoiceDialogBox::ChoiceHandler().SetOKButtonTitle(okButtonTitle);
	ChoiceDialogBox::ChoiceHandler().SetCancelButtonTitle(cancelButtonTitle);
	if(ChoiceDialogBox::ChoiceHandler().Ask(errorString))
		return FALSE;
	else
		return TRUE;
}

	void
CChangePassphraseDialog::ListenToMessage(	MessageT inMessage,
											void */*ioParam*/)
{
	Boolean				hide,
						badPass;
	PGPInt32			algorithm;
	CSecurePString255	oldPass,
						confPass,
						newPass;
	
	CSecureCString256	oldPassC;
	CSecureCString256	newPassC;
	PGPError			err;
	
	switch(inMessage)
	{
		case kOKButton:
		{
			Boolean		confirmMatch;
			Boolean		enforceLength;
			Boolean		goodLength;
			Boolean		goodQuality;
			
			Hide();
			badPass = FALSE;
			mOldPassphraseEditField->GetDescriptor(oldPass);
			mConfirmationEditField->GetDescriptor(confPass);
			mNewPassphraseEditField->GetDescriptor(newPass);
			if(CKeyGenWizardDialog::IsGoodPassphrase(newPass, confPass,
					confirmMatch, enforceLength, goodLength, goodQuality))
			{
			goodPassphrase:	
				PToCString(oldPass, oldPassC);
				PToCString(newPass, newPassC);
				err = PGPChangePassphrase(mKey, oldPassC, newPassC);
				if(!err)
				{
					err = PGPGetKeyNumber(mKey, kPGPKeyPropAlgID,
											&algorithm);
					pgpAssert(!err);
					if(algorithm == kPGPPublicKeyAlgorithm_DSA)
					{
						PGPKeyIterRef		keyIter;
						PGPSubKeyRef		subKey;
						
						err = PGPNewKeyIter(
							CPGPKeys::TheApp()->GetKeyList(), &keyIter);
						pgpAssertAddrValid(keyIter, VoidAlign);
						PGPKeyIterSeek(keyIter, mKey);
						while(!err &&
							IsntPGPError(PGPKeyIterNextSubKey(keyIter,
										&subKey)) &&
							IsntNull(subKey))
						{
							err = PGPChangeSubKeyPassphrase(subKey,
												oldPassC, newPassC);
						}
						PGPFreeKeyIter(keyIter);
					}
				}
				if(err == kPGPError_BadPassphrase)
				{
					CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
									kStringListID, kBadPassID);
					badPass = TRUE;
				}
				else
				{
					pgpAssert(IsntErr(err));
					if(IsntErr(err))
					{
						BroadcastMessage(kChangePassphraseSuccessful,nil);
						CWarningAlert::Display(kWANoteAlertType, kWAOKStyle,
								kStringListID, kSuccessID);
					}
					if(err)
					{
						CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
								kStringListID, kChangeErrorID);
					}
				}
			}
			else
			{
				if(!confirmMatch)
				{
					CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
									kStringListID, kPassNoMatchID);
				}
				else if(!goodLength)
				{
					if(!enforceLength && AcceptBadPassphrase())
						goto goodPassphrase;
					else if(enforceLength)
					{
						Str255		errorStr1,
									errorStr2,
									lengthStr;
						PGPUInt32	minLength;
						
						err = PGPGetPrefNumber(
								gAdminPrefRef, kPGPPrefMinChars, &minLength);
						pgpAssertNoErr(err);
						::NumToString(minLength, lengthStr);
						::GetIndString(errorStr1, kStringListID,
							kBadLength1ErrorStrIndex);
						::GetIndString(errorStr2, kStringListID,
							kBadLength2ErrorStrIndex);
						AppendPString(lengthStr, errorStr1);
						AppendPString(errorStr2, errorStr1);
						CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
											errorStr1);
					}
				}
				else if(!goodQuality)
				{
					CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
									kStringListID, kBadQualityErrorStrIndex);
				}
				badPass = TRUE;
			}

			if(badPass)
			{
				mOldPassphraseEditField->ClearPassphrase();
				mNewPassphraseEditField->ClearPassphrase();
				mConfirmationEditField->ClearPassphrase();
				mPassQualityProgressBar->SetPercentComplete(0);
				SwitchTarget(mOldPassphraseEditField);
				Show();
				break;
			}
		}
		case kCancelButton:
			delete this;
			break;
		case kHideTypingCheckbox:
			hide = (mHideTypingCheckbox->GetValue() == 1);
			mOldPassphraseEditField->SetHideTyping(hide);
			mNewPassphraseEditField->SetHideTyping(hide);
			mConfirmationEditField->SetHideTyping(hide);
			break;
		case kNewPassphraseField:{
			CSecureCString256	passphrase;
			PGPUInt16			quality;
			
			mNewPassphraseEditField->GetDescriptor( (uchar *)
								passphrase.mString);
			PToCString( (uchar *) passphrase.mString, passphrase );
			quality = PGPCalcPassphraseQuality(passphrase);
			mPassQualityProgressBar->SetPercentComplete(quality);
			break;}
	}
}

