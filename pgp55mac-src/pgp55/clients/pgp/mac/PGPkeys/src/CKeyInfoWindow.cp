/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyInfoWindow.cp,v 1.25 1997/10/22 07:29:46 wprice Exp $
____________________________________________________________________________*/
#include "CKeyInfoWindow.h"
#include "CKeyWindow.h"
#include "CKeyTable.h"
#include "CPGPKeys.h"
#include "CFingerprintField.h"
#include "CChangePassphraseDialog.h"
#include "CWarningAlert.h"
#include "MacStrings.h"

#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"

const ResIDT		kKeyInfoWindowID	= 140;
const ResIDT		kChangePassDialogID	= 136;
const short			kNumTrustValues		=	3;
const short			kNumValidityValues	=	3;
const Int16			kUnsupportedStringID=	4;

short			CKeyInfoWindow::sNumKeyInfoWindows	= 0;
CKeyInfoWindow	*CKeyInfoWindow::sKeyInfoWindows[kMaxKeyInfoWindows];


/*
Pass NIL for the table to OpenKeyInfo to disable redrawing the
table on close and disable all editable control in the key info
window.  This is useful for editing keyserver search keysets
for instance where you can't modify the trust, etc...
*/
	Boolean
CKeyInfoWindow::OpenKeyInfo(PGPKey *key, CKeyTable *table)
{
	CKeyInfoWindow *keyInfoWindow;
	short kiIndex;
	
	if(sNumKeyInfoWindows < kMaxKeyInfoWindows)
	{
		for(kiIndex = 0;kiIndex < sNumKeyInfoWindows;kiIndex++)
		{
			if(sKeyInfoWindows[kiIndex]->GetKey() == key)
			{
				sKeyInfoWindows[kiIndex]->Select();
				return TRUE;
			}
		}
		keyInfoWindow = (CKeyInfoWindow *)
			LWindow::CreateWindow(kKeyInfoWindowID, CPGPKeys::TheApp());
		sKeyInfoWindows[sNumKeyInfoWindows] = keyInfoWindow;
		keyInfoWindow->SetInfo(key, table);
		sNumKeyInfoWindows++;
		return TRUE;
	}
	else
	{
		CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
						kKeyInfoStringListID, kTooManyWindowsID);
		return FALSE;
	}
}

	void
CKeyInfoWindow::CloseAll()
{	
	while(sNumKeyInfoWindows > 0)
	{
		pgpAssertAddrValid(sKeyInfoWindows[0], VoidAlign);
		delete sKeyInfoWindows[0];
	}
	pgpAssert(sNumKeyInfoWindows == 0);
}

	PGPKeyRef
CKeyInfoWindow::GetKey()
{
	return mKey;
}

CKeyInfoWindow::CKeyInfoWindow()
{
}

CKeyInfoWindow::CKeyInfoWindow(LStream *inStream)
	:	LWindow(inStream)
{
}

CKeyInfoWindow::~CKeyInfoWindow()
{
	short		windowIndex,
				windowID;
	PGPError	err;
	uchar		trust;
	PGPValidity	validity;
	PGPBoolean	expired,
				revoked,
				axiomatic;
	
	err = PGPGetKeyBoolean(mKey, kPGPKeyPropIsExpired, &expired);
	pgpAssertNoErr(err);
	err = PGPGetKeyBoolean(mKey, kPGPKeyPropIsRevoked, &revoked);
	pgpAssertNoErr(err);
	if(CPGPKeys::TheApp()->IsKeyringWritable() && !expired && !revoked)
	{
		if(mEnabledCheckbox->GetValue())
		{
			err = PGPEnableKey(mKey);
			pgpAssert(!err);
		}
		else
		{
			err = PGPDisableKey(mKey);
			pgpAssert(!err);
		}
		axiomatic = mAxiomaticCheckbox->GetValue();
		if(axiomatic && !mAxiomatic)
		{
			err = PGPSetKeyAxiomatic(mKey, FALSE, "");
			pgpAssertNoErr(err);
		}
		else if(!axiomatic && mAxiomatic)
		{
			err = PGPUnsetKeyAxiomatic(mKey);
			pgpAssertNoErr(err);
		}
		else if(mTrustSlider->IsEnabled())
		{
			err = PGPGetPrimaryUserIDValidity(mKey, &validity);
			pgpAssert(!err);
			switch(mTrustSlider->GetValue())
			{
				default:
				case 0:
					trust = kPGPKeyTrust_Never;
					break;
				case 1:
					trust = kPGPKeyTrust_Marginal;
					break;
				case 2:
					trust = kPGPKeyTrust_Complete;
					break;
				case 3:
					trust = kPGPKeyTrust_Complete;
					break;
			}
			err = PGPSetKeyTrust(mKey, trust);
			pgpAssert(!err);
			if(mTrustChanged && ((validity == kPGPValidity_Invalid) ||
				(validity == kPGPValidity_Unknown)))
			{
				CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
								kKeyInfoStringListID, kInvalidKeySetTrustID);
			}
		}
		pgpAssert((mChanged && IsntNull(mParentTable)) || !mChanged);
		if(mChanged && mParentTable)
		{
			CPGPKeys::TheApp()->CommitDefaultKeyrings();
			mParentTable->RedrawTable();
		}
	}

	// Remove this Key Info window from the list
	for(windowID = 0;windowID < sNumKeyInfoWindows;windowID++)
		if(sKeyInfoWindows[windowID] == this)
		{
			sKeyInfoWindows[windowID] = NULL;
			break;
		}
	
	for(windowIndex = windowID + 1;
		windowIndex < sNumKeyInfoWindows;
		windowIndex++)
		sKeyInfoWindows[windowIndex-1] = sKeyInfoWindows[windowIndex];
	sNumKeyInfoWindows--;
}

	void
CKeyInfoWindow::FinishCreateSelf()
{
	LWindow::FinishCreateSelf();
	
	mChanged = FALSE;
	mTrustChanged = FALSE;
	mAxiomatic = FALSE;
	mKeyIDCaption =			(LCaption *)	FindPaneByID(kKeyIDCaption);
	pgpAssertAddrValid(mKeyIDCaption,		VoidAlign);
	mKeyTypeCaption =		(LCaption *)	FindPaneByID(kKeyTypeCaption);
	pgpAssertAddrValid(mKeyTypeCaption,		VoidAlign);
	mCreationCaption =		(LCaption *)	FindPaneByID(kCreationCaption);
	pgpAssertAddrValid(mCreationCaption,	VoidAlign);
	mSizeCaption =			(LCaption *)	FindPaneByID(kSizeCaption);
	pgpAssertAddrValid(mSizeCaption,		VoidAlign);
	mCipherCaption =		(LCaption *)	FindPaneByID(kCipherCaption);
	pgpAssertAddrValid(mCipherCaption,		VoidAlign);
	mExpiresCaption =		(LCaption *)	FindPaneByID(kExpiresCaption);
	pgpAssertAddrValid(mExpiresCaption,		VoidAlign);
	mEnabledCheckbox =		(LGACheckbox *)	FindPaneByID(kEnabledCheckbox);
	pgpAssertAddrValid(mEnabledCheckbox,	VoidAlign);
	mTrustCaption =			(LCaption *)	FindPaneByID(kTrustCaption);
	pgpAssertAddrValid(mTrustCaption,		VoidAlign);
	mTrustSlider =			(LControl *)	FindPaneByID(kTrustSlider);
	pgpAssertAddrValid(mTrustSlider,		VoidAlign);
	mValidityCaption =		(LCaption *)	FindPaneByID(kValidityCaption);
	pgpAssertAddrValid(mValidityCaption,	VoidAlign);
	mValidityBox =			(LControl *)	FindPaneByID(kValidityBox);
	pgpAssertAddrValid(mValidityBox,		VoidAlign);
	mPassphraseButton =		(LGAPushButton *)FindPaneByID(kPassphraseButton);
	pgpAssertAddrValid(mPassphraseButton,	VoidAlign);
	mAxiomaticCheckbox =	(LGACheckbox *)	FindPaneByID(kAxiomaticCheckbox);
	pgpAssertAddrValid(mAxiomaticCheckbox,	VoidAlign);
	mFingerprintCaption =	(CFingerprintField *)
										FindPaneByID(kFingerprintCaption);
	pgpAssertAddrValid(mFingerprintCaption,	VoidAlign);
}

	void
CKeyInfoWindow::SetInfo(PGPKey *key, CKeyTable *table)
{
	PGPError		err;
	uchar			str[128],
					str2[128],
					*p = NULL;
	Str255			pstr;
	size_t			len;
	PGPBoolean		disabled,
					expired,
					revoked,
					secret;
	Boolean			writable;
	PGPInt32		keyStatus;
	Int16			trust;
	PGPKeyRef		mrKey;
	PGPSubKeyRef	subKey;
	PGPKeyID		keyID;
	PGPByte			mrClass;
	PGPTime			creation,
					expiration;
	Rect			windBounds;
	PGPInt32		bits,
					algorithm;
	
	mKey			= key;
	mParentTable	= table;
	err = PGPGetKeyBoolean(key, kPGPKeyPropIsSecret, &secret);
	pgpAssertNoErr(err);
	mSecret = secret;
	err = PGPGetKeyBoolean(key, kPGPKeyPropIsExpired, &expired);
	pgpAssertNoErr(err);
	err = PGPGetKeyBoolean(mKey, kPGPKeyPropIsRevoked, &revoked);
	pgpAssertNoErr(err);
	writable = table->IsWritable();
	
	// Setup message recovery agent identification
	err = PGPGetIndexedAdditionalRecipientRequest(key, table->GetKeySet(), 0,
									&mrKey, &mrClass);
	if(err == kPGPError_AdditionalRecipientRequestKeyNotFound)
	{
		Str255			unknownKeyString,
						keyIDPString;
		PGPKeyID		mrkeyid;
		
		keyIDPString[0] = 0;
		err = PGPGetIndexedAdditionalRecipientRequestKeyID(key, 0,
									&mrkeyid, &mrClass);
		pgpAssertNoErr(err);
		if(IsntPGPError(err))
		{
			char		keyIDString[ kPGPMaxKeyIDStringSize ];
			
			err = PGPGetKeyIDString( &mrkeyid, kPGPKeyIDString_Abbreviated,
							keyIDString);
			pgpAssertNoErr(err);
			if(IsntPGPError(err))
			{
				CToPString(keyIDString, keyIDPString);
			}
		}
		::GetIndString(unknownKeyString, kKeyInfoStringListID, kUnknownMRKID);
		AppendPString(keyIDPString, unknownKeyString);
		((CRecessedCaption *)FindPaneByID(kAgentRCaption))->
			SetDescriptor(unknownKeyString);
		CalcStandardBounds(windBounds);
		DoSetBounds(windBounds);
	}
	else if(IsntPGPError(err) && PGPRefIsValid(mrKey))
	{
		err = PGPGetPrimaryUserIDNameBuffer(mrKey,
			sizeof( pstr ) - 1, (char *)&pstr[1], &len);
		pgpAssert(!err);
		pstr[0] = len;
		((CRecessedCaption *)FindPaneByID(kAgentRCaption))->
			SetDescriptor(pstr);
		CalcStandardBounds(windBounds);
		DoSetBounds(windBounds);
	}
	else
		((LGAPrimaryBox *)FindPaneByID(kRecoveryBox))->Hide();
	
	// Stuff the fields
	err = PGPGetPrimaryUserIDNameBuffer(key,
		sizeof( pstr ) - 1, (char *)&pstr[1], &len);
	pgpAssert(!err);
	pstr[0] = len;
	SetDescriptor(pstr);
	
	// Set the Passphrase button status
	if(!mSecret || !writable)
	{
		mPassphraseButton->Hide();
		mAxiomaticCheckbox->Hide();
	}
	mPassphraseButton->AddListener(this);
	mAxiomaticCheckbox->AddListener(this);

	// Get the KeyID
	err = PGPGetKeyIDFromKey(key, &keyID );
	pgpAssertNoErr(err);
	if(IsntPGPError(err))
	{
		char	keyIDString[ kPGPMaxKeyIDStringSize ];
		err = PGPGetKeyIDString( &keyID,
					kPGPKeyIDString_Abbreviated, keyIDString);
		pgpAssertNoErr(err);
		CToPString(keyIDString, pstr);
		if(pstr[0] > 10)
			pstr[0] = 10;
		mKeyIDCaption->SetDescriptor(pstr);
	}
	
	// Get the Fingerprint
	GetFingerprintString(pstr, key);
	mFingerprintCaption->SetDescriptor(pstr);
	
	// Get Enabled status
	err = PGPGetKeyBoolean(key, kPGPKeyPropIsDisabled, &disabled);
	pgpAssert(!err);
	mEnabledCheckbox->SetValue(!disabled);
	mEnabledCheckbox->AddListener(this);
	
	// Get the Trust
	err = PGPGetKeyNumber(key, kPGPKeyPropTrust, &keyStatus);
	pgpAssert(!err);
	switch(keyStatus & kPGPKeyTrust_Mask)
	{
		case kPGPKeyTrust_Undefined:
		case kPGPKeyTrust_Unknown:
		case kPGPKeyTrust_Never:
		default:
			trust = 0;
			break;
		case kPGPKeyTrust_Marginal:
			trust = 1;
			break;
		case kPGPKeyTrust_Ultimate:
			mAxiomatic = TRUE;
		case kPGPKeyTrust_Complete:
			trust = 2;
			break;
	}
	SetTrustCaption(mTrustCaption, trust);
	mTrustSlider->SetMaxValue(kNumTrustValues - 1);
	mTrustSlider->SetValue(trust);
	mTrustSlider->AddListener(this);
	
	// Get the Validity
	err = PGPGetKeyNumber(key, kPGPKeyPropValidity, &keyStatus);
	pgpAssert(!err);
	switch(keyStatus)
	{
		case kPGPValidity_Unknown:
		case kPGPValidity_Invalid:
		default:
			trust = 0;
			break;
		case kPGPValidity_Marginal:
			trust = 1;
			break;
		case kPGPValidity_Complete:
			trust = 2;
			break;
	}
	SetValidityCaption(mValidityCaption, trust);
	mValidityBox->SetMaxValue(kNumValidityValues - 1);
	mValidityBox->SetValue(trust);
	if(mAxiomatic)
		mTrustSlider->Disable();
	
	if(mAxiomatic)
		mAxiomaticCheckbox->SetValue(1);
		
	// Get the Creation Date
	err = PGPGetKeyTime(key, kPGPKeyPropCreation, &creation);
	pgpAssert(!err);
	creation = PGPTimeToMacTime(creation);
	::DateString(creation, shortDate, pstr, NULL);
	mCreationCaption->SetDescriptor(pstr);
 		
	// Get the Expiration Date
	err = PGPGetKeyTime(key, kPGPKeyPropExpiration, &expiration);
	pgpAssert(!err);
	if(IsntErr(err) && expiration)
	{
		expiration = PGPTimeToMacTime(expiration);
		::DateString(expiration, shortDate, pstr, NULL);
	}
	else
		::GetIndString(pstr, kKeyInfoStringListID, kNeverExpiresID);
	mExpiresCaption->SetDescriptor(pstr);
	
	// Get the Algorithm ID
	err = PGPGetKeyNumber(key, kPGPKeyPropAlgID, &algorithm);
	pgpAssert(!err);
	switch(algorithm)
	{
		case kPGPPublicKeyAlgorithm_RSA:
		case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
		case kPGPPublicKeyAlgorithm_RSASignOnly:
			::GetIndString(pstr, kKeyInfoStringListID, kRSATypeID);
			break;
		case kPGPPublicKeyAlgorithm_DSA:
		case kPGPPublicKeyAlgorithm_ElGamal:	// Technically not
			::GetIndString(pstr, kKeyInfoStringListID, kDSATypeID);
			break;
		default:
			pgpDebugPStr("\pUnknown algorithm");
			break;
	}
	mKeyTypeCaption->SetDescriptor(pstr);
	
	// Get the Key Size
	err = PGPGetKeyNumber(key, kPGPKeyPropBits, &bits);
	pgpAssertNoErr(err);
	::NumToString(bits, str);
	if(algorithm == kPGPPublicKeyAlgorithm_DSA)
	{
		PGPKeyIterRef	subIter;
		PGPKeyListRef	listRef;
		
		listRef = table->GetKeyList();
		err = PGPNewKeyIter(listRef, &subIter);
		pgpAssertNoErr(err);
		PGPKeyIterSeek(subIter, key);
		err = PGPKeyIterNextSubKey(subIter, &subKey);
		if(IsntPGPError(err) && IsntNull(subKey))
		{
			err = PGPGetSubKeyNumber(subKey, kPGPKeyPropBits,
										&bits);
			pgpAssertNoErr(err);
			::NumToString(bits, str2);
			AppendPString("\p/", str2);
			AppendPString(str, str2);
			CopyPString(str2, str);
		}
		PGPFreeKeyIter(subIter);
	}
	mSizeCaption->SetDescriptor(str);
	
	// Get the Preferred Cipher
	{
		PGPByte		propBuff[128];	
		
		err = PGPGetKeyPropertyBuffer(key, kPGPKeyPropPreferredAlgorithms,
								sizeof( propBuff ), propBuff, &len);
		pgpAssertNoErr(err);
		if ( IsntPGPError( err ) )
		{
			if(len >= sizeof(PGPCipherAlgorithm))
			{
				UInt32	prefAlg = *(PGPCipherAlgorithm const *)&propBuff[0];
				
				pgpAssert((prefAlg >= kPGPCipherAlgorithm_None) &&
							(prefAlg <= kPGPCipherAlgorithm_CAST5));
				::GetIndString(str, kAlgorithmStringsID, prefAlg);
			}
			else
				::GetIndString(str, kAlgorithmStringsID, kIDEAString);
			mCipherCaption->SetDescriptor(str);
		}
	}

	if(expired || revoked || mAxiomatic || !writable)
	{
		mEnabledCheckbox->Hide();
		mTrustSlider->Disable();
	}
	if(expired || revoked || !writable)
	{
		mPassphraseButton->Hide();
		mAxiomaticCheckbox->Hide();
	}
	Show();
}

	void
CKeyInfoWindow::GetFingerprintString(Str255 pstr, PGPKeyRef key)
{
	static char hexDigit[] = "0123456789ABCDEF";
	PGPError	err;
	PGPSize		len;
	PGPByte		fingerprint[ 256 ];
	Int16		strIndex;
	uchar		*p;
	
	err = PGPGetKeyPropertyBuffer( key, kPGPKeyPropFingerprint,
			sizeof( fingerprint ), fingerprint, &len);
	pgpAssertNoErr(err);
	if ( IsntPGPError( err ) )
	{
		if(len == 20)
		{
			pstr[0] = 50;
			p = pstr + 1;
			for(strIndex = 0 ; strIndex < 20 ; strIndex++)
			{
				*p++ = hexDigit[fingerprint[strIndex]>>4];
				*p++ = hexDigit[fingerprint[strIndex]&0xF];
				if((strIndex == 1) || (strIndex == 3) || (strIndex == 5)
						|| (strIndex == 7) || (strIndex == 11) ||
						(strIndex == 13)
						|| (strIndex == 13) || (strIndex == 15) ||
						(strIndex == 17))
					*p++ = ' ';
				else if(strIndex == 9)
				{
					*p++ = ' ';
					*p++ = ' ';
				}
			}
		}
		else
		{
			pstr[0] = 40;
			p = pstr + 1;
			for(strIndex = 0 ; strIndex < 16 ; strIndex++)
			{
				*p++ = hexDigit[fingerprint[strIndex]>>4];
				*p++ = hexDigit[fingerprint[strIndex]&0xF];
				if((strIndex == 1) || (strIndex == 3) || (strIndex == 5)
					|| (strIndex == 9) || (strIndex == 11) || (strIndex == 13))
					*p++ = ' ';
				else if(strIndex == 7)
				{
					*p++ = ' ';
					*p++ = ' ';
				}
			}
		}
	}
}

	void
CKeyInfoWindow::SetTrustCaption(
	LCaption	*inCaption,
	Int16		inTrustLevel)
{
	Str255 theTrustString;
	
	::GetIndString(theTrustString, kTrustStringListID, inTrustLevel + 1);
	inCaption->SetDescriptor(theTrustString);
	inCaption->Draw(nil);				// update immediately
	inCaption->DontRefresh(false);		// don't draw a second time
}

	void
CKeyInfoWindow::SetValidityCaption(
	LCaption	*inCaption,
	Int16		inValidityLevel)
{
	Str255 theValidityString;
	
	::GetIndString(theValidityString, kValidityStringListID,
					inValidityLevel + 1);
	inCaption->SetDescriptor(theValidityString);
	inCaption->Draw(nil);				// update immediately
	inCaption->DontRefresh(false);		// don't draw a second time
}

	void
CKeyInfoWindow::DrawSelf()
{
	LWindow::DrawSelf();
}

	void
CKeyInfoWindow::ListenToMessage(MessageT inMessage, void *ioParam)
{
	Int32 value;
	
	value = *(Int32 *)ioParam;
	switch(inMessage)
	{
		case kTrustSlider:
			SetTrustCaption(mTrustCaption, value);
			mTrustChanged = TRUE;
			mChanged = TRUE;
			break;
		case kEnabledCheckbox:
			mChanged = TRUE;
			break;
		case kPassphraseButton:{
			CChangePassphraseDialog *mCPDialog;
			
			mCPDialog = (CChangePassphraseDialog *)
				LWindow::CreateWindow(kChangePassDialogID, this);
			mCPDialog->AddListener(this);
			mCPDialog->SetKey(mKey);
			mChanged = TRUE;
			break;}
		case kAxiomaticCheckbox:
			mChanged = TRUE;
			if(value != 0)
			{
				mTrustSlider->Disable();
				mTrustSlider->SetValue(kNumTrustValues - 1);
				mValidityBox->SetValue(kNumValidityValues - 1);
				SetValidityCaption(mValidityCaption, kNumValidityValues - 1);
				mEnabledCheckbox->Hide();
			}
			else
			{
				mTrustSlider->Enable();
				mTrustSlider->SetValue(0);
				mValidityBox->SetValue(0);
				SetValidityCaption(mValidityCaption, 0);
				mEnabledCheckbox->Show();
			}
			mTrustSlider->Draw(nil);
			mValidityBox->Draw(nil);
			break;
		case kChangePassphraseSuccessful:
			mAxiomatic = TRUE;
			mAxiomaticCheckbox->SetValue(1);
			break;
	}
}

	Boolean
CKeyInfoWindow::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true;

	switch(inCommand) 
	{
		case cmd_Close:
			delete this;
			break;
		default:
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

	void
CKeyInfoWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	switch (inCommand) 
	{
		case cmd_Close:
			outEnabled = true;
			break;
		default:
			LCommander::FindCommandStatus(inCommand, outEnabled,
										outUsesMark, outMark, outName);
			break;
	}
}

