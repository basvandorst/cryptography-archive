/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyGenWizardDialog.cp,v 1.53.4.2 1997/11/20 19:36:11 wprice Exp $
____________________________________________________________________________*/
#include "CKeyGenWizardDialog.h"

#include "CPassphraseEdit.h"
#include "CPGPStDialogHandler.h"
#include "CSecureMemory.h"
#include "CWarningAlert.h"
#include <LGALittleArrows.h>

#include "ChoiceDialogBox.h"
#include "CInfinityBar.h"
#include "CKeyWindow.h"
#include "CProgressBar.h"
#include "CGADurationEditField.h"
#include "CPGPKeys.h"
#include "CKeyTable.h"
#include "MacEnvirons.h"
#include "MacStrings.h"

#include <fp.h>
#include <string.h>

#include "pgpMem.h"
#include "pgpRandomPool.h"
#include "pgpErrors.h"
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pgpUserInterface.h"

const Int16			kRecommendedMinPassLength	=	8;

const ResIDT		kStringListResID	= 1015;

const ResIDT		view_Introduction	= 200;
const ResIDT		view_NameAndAddress	= 201;
const ResIDT		view_KeyType		= 202;
const ResIDT		view_KeySize		= 203;
const ResIDT		view_KeyExpiration	= 204;
const ResIDT		view_KeyPassphrase	= 205;
const ResIDT		view_BadPassphrase	= 211;
const ResIDT		view_GetRandomData	= 206;
const ResIDT		view_GeneratingKey	= 207;
const ResIDT		view_MRK			= 208;
const ResIDT		view_Corp			= 209;
const ResIDT		view_SendToServer	= 212;
const ResIDT		view_Conclusion		= 210;

const MessageT		kNextViewMessage			= 'Next';
const MessageT		kPreviousViewMessage		= 'Prev';
const MessageT		kAllowViewSwitchMessage		= 'ASwi';

const PaneIDT		kNextButtonPaneID			= 'bNxt';
const PaneIDT		kPreviousButtonPaneID		= 'bPrv';
const PaneIDT		kCancelButtonPaneID			= 'bCan';
const PaneIDT		kGeneratingKeyProgressID	= 'Ther';

const UInt32		kMinKeySize				= 512;
const UInt32		kRSAMaxKeySize			= 2048;
const UInt32		kDSAElGamalMaxKeySize	= 4096;

const UInt32		kMaxKeyExpirationDays	= 10950;

enum
{
	kNextButtonTitleStrIndex	= 1,
	kDoneButtonTitleStrIndex,
	kNameRequiredStrIndex,
	kVerifyNoEmailAddressStrIndex,
	kKeyRangeCaptionFormatStrIndex,
	kKeySizeOutOfRangeStrIndex,
	kKeyExpirationOutOfRangeStrIndex,
	kKeyGenerationErrorStrIndex,
	kNoMRKErrorStrIndex,
	kNoCorpKeyErrorStrIndex,
	k768WarningStrIndex,
	kAdminKeySizeErrorStrIndex
};

Boolean		CKeyGenWizardDialog::mGeneratingKey;
PGPError	CKeyGenWizardDialog::mKeyGenerationError;

#pragma mark --- CKeyGenWizardView ---

class CKeyGenWizardView : 	public LView,
							public LCommander
{
public:

	enum { class_ID = 'vKGW' };
	
				CKeyGenWizardView(LStream *inStream);
	virtual		~CKeyGenWizardView();

};

CKeyGenWizardView::CKeyGenWizardView(LStream *inStream)
	: LView(inStream)
{
}

CKeyGenWizardView::~CKeyGenWizardView()
{
}

#pragma mark --- CNameAndAddressView ---

class CNameAndAddressView : public CKeyGenWizardView
{
public:

	enum { class_ID = 'vNAA' };
	
					CNameAndAddressView(LStream *inStream);
	virtual			~CNameAndAddressView();

	void			GetParams(StringPtr name, StringPtr address);
	void			SetParams(ConstStr255Param name, ConstStr255Param address);
	
	void			GetEmailAddress(StringPtr address);
	void			SetEmailAddress(ConstStr255Param address);
	
private:

	enum
	{
		kNameEditFieldPaneID			= 'Name',
		kEmailAddressEditFieldPaneID	= 'Mail'
	};	
};

CNameAndAddressView::CNameAndAddressView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CNameAndAddressView::~CNameAndAddressView()
{
}

	void
CNameAndAddressView::GetParams(
	StringPtr name,
	StringPtr address)
{
	LGAEditField	*editFieldObj;
	
	editFieldObj = (LGAEditField *) FindPaneByID( kNameEditFieldPaneID );
	pgpAssertAddrValid( editFieldObj, LGAEditField );
	
	editFieldObj->GetDescriptor( name );

	editFieldObj = (LGAEditField *) FindPaneByID(
										kEmailAddressEditFieldPaneID );
	pgpAssertAddrValid( editFieldObj, LGAEditField );
	
	editFieldObj->GetDescriptor( address );
}

	void
CNameAndAddressView::SetParams(
	ConstStr255Param name,
	ConstStr255Param address)
{
	LGAEditField	*editFieldObj;
	
	editFieldObj = (LGAEditField *) FindPaneByID(
										kEmailAddressEditFieldPaneID );
	pgpAssertAddrValid( editFieldObj, LGAEditField );
	
	editFieldObj->SetDescriptor( address );

	editFieldObj = (LGAEditField *) FindPaneByID( kNameEditFieldPaneID );
	pgpAssertAddrValid( editFieldObj, LGAEditField );
	
	editFieldObj->SetDescriptor( name );
	editFieldObj->SelectAll();
	LCommander::SwitchTarget( editFieldObj );
}

#pragma mark --- CKeyTypeView ---

class CKeyTypeView : public CKeyGenWizardView
{
public:

	enum { class_ID = 'vKTp' };
	
							CKeyTypeView(LStream *inStream);
	virtual					~CKeyTypeView();
	virtual void			FinishCreateSelf();
	
	PGPPublicKeyAlgorithm	GetAlgorithm(void);
	void					SetAlgorithm(UInt32 algorithm);
	
private:

	enum
	{
		kDSAElGamalButtonPaneID	= 'bDSA',
		kRSAButtonPaneID		= 'bRSA',
		kButtonGroupPaneID		= 'vGrp',
		kNoRSAKeyGenCaptionID	= 'cRKG'
	};	
};

CKeyTypeView::CKeyTypeView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CKeyTypeView::~CKeyTypeView()
{
}

	void
CKeyTypeView::FinishCreateSelf()
{
	Boolean		allowRSA = TRUE;
#if PGP_BUSINESS_SECURITY
	PGPError	err;

	err = PGPGetPrefBoolean(gAdminPrefRef, kPGPPrefAllowRSAKeyGen, &allowRSA);
	pgpAssertNoErr(err);
#endif
	if(PGP_FREEWARE || !allowRSA || NO_RSA_KEYGEN)
		((LGARadioButton *)FindPaneByID(kRSAButtonPaneID))->Disable();
}

	void
CKeyTypeView::SetAlgorithm(UInt32 algorithm)
{
	PaneIDT	paneID = PaneIDT_Undefined;
	
	switch( algorithm )
	{
		case kPGPPublicKeyAlgorithm_DSA:
			paneID = kDSAElGamalButtonPaneID;
			break;
		
		case kPGPPublicKeyAlgorithm_RSA:
			paneID = kRSAButtonPaneID;
			break;

		default:
			pgpDebugMsg( "SetAlgorithm(): Unknown algorithm" );
			break;
	}
	
	if( paneID != PaneIDT_Undefined )
	{
		LGARadioButton	*buttonObj;
			
		buttonObj = (LGARadioButton *) FindPaneByID( paneID );
		pgpAssertAddrValid( buttonObj, LGARadioButton );
		
		buttonObj->SetValue( 1 );
	}
}

	PGPPublicKeyAlgorithm
CKeyTypeView::GetAlgorithm(void)
{
	LGARadioButton			*buttonObj;
	PGPPublicKeyAlgorithm	algorithm;
	
	algorithm = kPGPPublicKeyAlgorithm_DSA;
	
	buttonObj = (LGARadioButton *) FindPaneByID( kRSAButtonPaneID );
	pgpAssertAddrValid( buttonObj, LGARadioButton );
	
	if( buttonObj->GetValue() > 0 )
	{
		algorithm = kPGPPublicKeyAlgorithm_RSA;
	}
	
	return( algorithm );
}

#pragma mark --- CKeySizeView ---

class CKeySizeView : public CKeyGenWizardView
{
public:

	enum { class_ID = 'vKSz' };
	
					CKeySizeView(LStream *inStream);
	virtual			~CKeySizeView();

	UInt32			GetKeySize(void);
	void			SetParams(UInt32 algorithm, UInt32 keySize);
	
protected:

	virtual	void	FinishCreateSelf(void);
	
private:

	enum
	{
		k768BitsButtonPaneID			= 768,
		k1024BitsButtonPaneID			= 1024,
		k1536BitsButtonPaneID			= 1536,
		k2048BitsButtonPaneID			= 2048,
		k3072BitsButtonPaneID			= 3072,
		kCustomBitsButtonPaneID			= 'bCus',
		kCustomBitsEditFieldPaneID		= 'eCus',
		kCustomBitsRangeCaptionPaneID	= 'cRng',
		kRSALimitNoteCaptionPaneID		= 'cNot'
	};
	
	typedef struct ButtonInfo
	{
		UInt32		Keys;
		PaneIDT		paneID;
	};
	
	static ButtonInfo	sButtonInfo[];
	
	LGARadioButton		*mCustomBitsButton;
	LGAEditField		*mCustomBitsEditField;
};

CKeySizeView::ButtonInfo	CKeySizeView::sButtonInfo[] = {
								{ 768, k768BitsButtonPaneID },
								{ 1024, k1024BitsButtonPaneID },
								{ 1536, k1536BitsButtonPaneID },
								{ 2048, k2048BitsButtonPaneID },
								{ 3072, k3072BitsButtonPaneID } };

CKeySizeView::CKeySizeView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CKeySizeView::~CKeySizeView(void)
{
}

	void
CKeySizeView::FinishCreateSelf(void)
{
	CKeyGenWizardView::FinishCreateSelf();
	
	mCustomBitsButton = (LGARadioButton *) FindPaneByID(
												kCustomBitsButtonPaneID );
	pgpAssertAddrValid( mCustomBitsButton, LGARadioButton );

	mCustomBitsEditField = (LGAEditField *) FindPaneByID(
												kCustomBitsEditFieldPaneID );
	pgpAssertAddrValid( mCustomBitsEditField, LGAEditField );
}

	void
CKeySizeView::SetParams(UInt32 algorithm, UInt32 keySize)
{
	Boolean			foundButton = false;
	UInt32			numButtons;
	LGARadioButton	*buttonObj;
	LCaption		*captionObj;
	Str255			caption;
	UInt32			maxKeySize;
	Str32			numStr;
	
	pgpAssert( keySize >= 512 && keySize <= 4096 );
	
	if( algorithm == kPGPPublicKeyAlgorithm_RSA )
	{
		maxKeySize = kRSAMaxKeySize;
		
		buttonObj = (LGARadioButton *) FindPaneByID( k3072BitsButtonPaneID );
		pgpAssertAddrValid( buttonObj, LGARadioButton );
		
		buttonObj->Disable();
		
		captionObj = (LCaption *) FindPaneByID( kRSALimitNoteCaptionPaneID );
		pgpAssertAddrValid( buttonObj, LCaption );
		
		captionObj->Show();
	}
	else
	{
		maxKeySize = kDSAElGamalMaxKeySize;
	}
	
	if( keySize > maxKeySize )
		keySize = maxKeySize;
		
	GetIndString( caption, kStringListResID, kKeyRangeCaptionFormatStrIndex );
	NumToString( maxKeySize, numStr );
	PrintPString( caption, caption, numStr );
	
	captionObj = (LCaption *) FindPaneByID( kCustomBitsRangeCaptionPaneID );
	pgpAssertAddrValid( captionObj, LCaption );
	
	captionObj->SetDescriptor( caption );
	
	numButtons = sizeof( sButtonInfo ) / sizeof( sButtonInfo[0] );
	for( UInt32 buttonIndex = 0; buttonIndex < numButtons; buttonIndex++ )
	{
		if( sButtonInfo[buttonIndex].Keys == keySize )
		{
			buttonObj = (LGARadioButton *) FindPaneByID(
										sButtonInfo[buttonIndex].paneID );
			pgpAssertAddrValid( buttonObj, LGARadioButton );
			
			buttonObj->SetValue( 1 );
			foundButton = true;
			
			break;
		}
	}
	
	if( ! foundButton )
	{
		mCustomBitsButton->SetValue( 1 );
		mCustomBitsEditField->SetValue( keySize );
	}

	mCustomBitsEditField->SelectAll();
	LCommander::SwitchTarget( mCustomBitsEditField );
}

	UInt32
CKeySizeView::GetKeySize(void)
{
	UInt32	Keys;
	Boolean	foundButton = false;
	UInt32	numButtons;
	
	numButtons = sizeof( sButtonInfo ) / sizeof( sButtonInfo[0] );
	for( UInt32 buttonIndex = 0; buttonIndex < numButtons; buttonIndex++ )
	{
		LGARadioButton	*buttonObj;
		
		buttonObj = (LGARadioButton *) FindPaneByID(
											sButtonInfo[buttonIndex].paneID );
		pgpAssertAddrValid( buttonObj, LGARadioButton );
		
		if( buttonObj->GetValue() > 0 )
		{
			Keys 	= sButtonInfo[buttonIndex].Keys;
			foundButton = true;
			break;
		}
	}
	
	if( ! foundButton )
	{
		pgpAssert( mCustomBitsButton->GetValue() > 0 );
		
		Keys = mCustomBitsEditField->GetValue();
	}
	
	return( Keys );
}

#pragma mark --- CKeyExpirationView ---

class CKeyExpirationView :	public CKeyGenWizardView,
							public LListener
{
public:

	enum { class_ID = 'vKEx' };
	
					CKeyExpirationView(LStream *inStream);
	virtual			~CKeyExpirationView();

	Boolean			GetExpiration(UInt32 *expirationDays);
	void			SetExpiration(UInt32 Keys);
	virtual	void	ListenToMessage(MessageT inMessage, void *ioParam);
	
private:

	Int32						mLastArrowValue;

	enum
	{
		kNeverButtonPaneID		= 'bNvr',
		kDaysButtonPaneID		= 'bExp',
		kDateDurationFieldID	= 'eDur',
		kDateArrowsID			= 'lArr'
	};
};

CKeyExpirationView::CKeyExpirationView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CKeyExpirationView::~CKeyExpirationView(void)
{
}

	void
CKeyExpirationView::SetExpiration(UInt32 days)
{
	LGARadioButton			*buttonObj;
	CGADurationEditField	*durationObj;
	LGALittleArrows			*arrowsObj;
	UInt32					timeNow;
		
	durationObj = (CGADurationEditField *)
					FindPaneByID( kDateDurationFieldID );
	pgpAssertAddrValid( durationObj, CGADurationEditField );
	durationObj->SetDurationType(kDateDurationType);
	arrowsObj = (LGALittleArrows *) FindPaneByID(kDateArrowsID);
	pgpAssertAddrValid( arrowsObj, LGALittleArrows );
	mLastArrowValue = arrowsObj->GetValue();
	arrowsObj->AddListener(this);

	GetDateTime(&timeNow);
	timeNow += days * 86400;
	durationObj->SetDurationValue( timeNow );
	
	if( days == 0 )
	{
		buttonObj = (LGARadioButton *) FindPaneByID( kNeverButtonPaneID );
		pgpAssertAddrValid( buttonObj, LGARadioButton );
		
		buttonObj->SetValue( 1 );
	}
	else
	{		
		buttonObj = (LGARadioButton *) FindPaneByID( kDaysButtonPaneID );
		pgpAssertAddrValid( buttonObj, LGARadioButton );
		
		buttonObj->SetValue( 1 );
	}
}

	Boolean
CKeyExpirationView::GetExpiration(UInt32 *expirationDays)
{
	LGARadioButton	*buttonObj;
	Boolean			validExpiration;
	UInt32			expireDate,
					timeNow;
	
	validExpiration = true;
	
	buttonObj = (LGARadioButton *) FindPaneByID( kNeverButtonPaneID );
	pgpAssertAddrValid( buttonObj, LGARadioButton );
	
	if( buttonObj->GetValue() > 0 )
	{
		*expirationDays = 0;
	}
	else
	{
		CGADurationEditField	*durationObj;
			
		durationObj = (CGADurationEditField *)
						FindPaneByID( kDateDurationFieldID );
		pgpAssertAddrValid( durationObj, CGADurationEditField );
	
		expireDate = durationObj->GetDurationValue();
		GetDateTime(&timeNow);
		*expirationDays = ((expireDate - timeNow) / 86400) + 1;
		if( *expirationDays < 1 || *expirationDays > kMaxKeyExpirationDays )
		{
			validExpiration = false;
		}
	}
	
	return( validExpiration );
}

	void
CKeyExpirationView::ListenToMessage(MessageT inMessage, void *ioParam)
{
	Int32					newValue;
	CGADurationEditField	*durationObj;
	
	switch( inMessage )
	{
		case kDateArrowsID:
			durationObj = (CGADurationEditField *)
							FindPaneByID( kDateDurationFieldID );
			pgpAssertAddrValid( durationObj, CGADurationEditField );
			newValue = *(Int32 *)ioParam;
			if(newValue < mLastArrowValue)
				durationObj->AdjustValue(kDownAdjust);
			else
				durationObj->AdjustValue(kUpAdjust);
			mLastArrowValue = newValue;
			break;
	}
}

#pragma mark --- CMRKView ---

class CMRKView 		:		public CKeyGenWizardView
{
public:

	enum { class_ID = 'vKMR' };
	
					CMRKView(LStream *inStream);
	virtual			~CMRKView();

	void			SetMRK(PGPKeyRef mrkKey);
	
private:

	enum
	{
		kMRKPaneID				= 'cMRK'
	};
};

CMRKView::CMRKView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CMRKView::~CMRKView(void)
{
}

	void
CMRKView::SetMRK(PGPKeyRef mrkKey)
{
	PGPError		err;
	Str255			str;
	PGPSize			len;

	err = PGPGetPrimaryUserIDNameBuffer(mrkKey,
		sizeof(str) - 1, (char *)&str[1], &len);
	pgpAssertNoErr(err);
	str[0] = len;
	((LCaption *)FindPaneByID(kMRKPaneID))->SetDescriptor(str);
}

#pragma mark --- CCorpView ---

class CCorpView 		:		public CKeyGenWizardView
{
public:

	enum { class_ID = 'vCor' };
	
					CCorpView(LStream *inStream);
	virtual			~CCorpView();

	void			SetCorpKey(PGPKeyRef corpKey);
	
private:

	enum
	{
		kCorpPaneID				= 'cCor'
	};
};

CCorpView::CCorpView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CCorpView::~CCorpView(void)
{
}

	void
CCorpView::SetCorpKey(PGPKeyRef corpKey)
{
	PGPError		err;
	Str255			str;
	PGPSize			len;

	err = PGPGetPrimaryUserIDNameBuffer(corpKey,
				sizeof(str) - 1, (char *)&str[1], &len);
	pgpAssertNoErr(err);
	str[0] = len;
	((LCaption *)FindPaneByID(kCorpPaneID))->SetDescriptor(str);
}

#pragma mark --- CSendToServerView ---

class CSendToServerView 	:		public CKeyGenWizardView
{
public:

	enum { class_ID = 'vStS' };
	
					CSendToServerView(LStream *inStream);
	virtual			~CSendToServerView();
	
	void			DoSendToServer(PGPKeyRef	generatedKey);

private:
	enum
	{
		kSendToServerBoxPaneID		= 'xSnd'
	};
};

CSendToServerView::CSendToServerView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CSendToServerView::~CSendToServerView(void)
{
}

	void
CSendToServerView::DoSendToServer(PGPKeyRef	generatedKey)
{
	LGACheckbox	*checkboxObj;

	checkboxObj = (LGACheckbox *) FindPaneByID( kSendToServerBoxPaneID );
	pgpAssertAddrValid( checkboxObj, LGACheckbox );
	
	if(checkboxObj->GetValue() && PGPRefIsValid(generatedKey))
	{
		PGPError err;
		
		err = PGPSendKeyToServer(gPGPContext, generatedKey, 0);
		if(IsPGPError(err) && (err != kPGPError_UserAbort))
			ReportPGPError(err);
	}
}

#pragma mark --- CBadPassphraseView ---

class CBadPassphraseView 	:		public CKeyGenWizardView
{
public:

	enum { class_ID = 'vBPh' };
	
					CBadPassphraseView(LStream *inStream);
	virtual			~CBadPassphraseView();
	
	void			SetPassphrase(	ConstStr255Param passphrase,
									ConstStr255Param confirmation);

private:
	enum
	{
		kConfirmationBoxPaneID		= 'bCon',
		kMinLengthBoxPaneID			= 'bMLe',
		kMinQualityBoxPaneID		= 'bMQu',
		kMinLengthCaption1PaneID	= 'cML1',
		kMinLengthCaption2PaneID	= 'cML2',
		kMinLengthCaption3PaneID	= 'cML3',
		kMinQualityCaptionPaneID	= 'cPQ1',
		kMinQualityBarPaneID		= 'cPQ2',
		kBadConfirmPaneID			= 'cCP1'
	};
};

CBadPassphraseView::CBadPassphraseView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CBadPassphraseView::~CBadPassphraseView(void)
{
}

	void
CBadPassphraseView::SetPassphrase(
	ConstStr255Param passphrase,
	ConstStr255Param confirmation)
{
	Boolean		match,
				enforceLength,
				goodLength,
				goodQuality;
	
	CKeyGenWizardDialog::IsGoodPassphrase(
		passphrase, confirmation, match,
		enforceLength, goodLength, goodQuality);
	if(match)
		((LGAPrimaryBox *)FindPaneByID(kConfirmationBoxPaneID))->Hide();
	if(goodLength)
		((LGAPrimaryBox *)FindPaneByID(kMinLengthBoxPaneID))->Hide();
	else
	{
		Str255		lengthString;
		PGPUInt32	minLength;

		minLength = kRecommendedMinPassLength;
#if PGP_BUSINESS_SECURITY
		if(enforceLength)
		{
			PGPError	err;
			
			err = PGPGetPrefNumber(
					gAdminPrefRef, kPGPPrefMinChars, &minLength);
			pgpAssertNoErr(err);
		}
#endif
		::NumToString(minLength, lengthString);
		((LCaption *)FindPaneByID(kMinLengthCaption2PaneID))->
			SetDescriptor(lengthString);
	}
	if(goodQuality)
		((LGAPrimaryBox *)FindPaneByID(kMinQualityBoxPaneID))->Hide();
	else
	{
#if PGP_BUSINESS_SECURITY
		PGPUInt32	minQuality;
		PGPError	err;
		
		err = PGPGetPrefNumber(
				gAdminPrefRef, kPGPPrefMinQuality, &minQuality);
		pgpAssertNoErr(err);
		if(IsntPGPError(err))
		{
			((CProgressBar *)FindPaneByID(kMinQualityBarPaneID))->
				SetPercentComplete(minQuality);
		}
#endif
	}
}

#pragma mark --- CKeyPassphraseView ---

class CKeyPassphraseView :  public CKeyGenWizardView,
							public LListener
{
public:

	enum { class_ID = 'vKPP' };
	
					CKeyPassphraseView(LStream *inStream);
	virtual			~CKeyPassphraseView();

	void			ClearPassphrase(void);
	void			GetPassphrase(	StringPtr	passphrase,
									StringPtr	confirmation);
	
protected:

	virtual	void	FinishCreateSelf(void);
	virtual	void	ListenToMessage(MessageT inMessage, void *ioParam);
	
private:

	enum
	{
		kPassphraseEditFieldPaneID		= 'ePas',
		kConfirmationEditFieldPaneID	= 'eCon',
		kPassphraseQualityBarPaneID		= 'pbPQ',
		kHideTypingCheckboxPaneID		= 'xHid',
		kRecommendedQualityBarPaneID	= 'pbRQ',
		kRecommendedQualityCaptionPaneID= 'cRQ1',
		kMinPassphraseLenCaptionPaneID	= 'cMin'
	};

	CPassphraseEdit	*mPassphraseEditField;
	CPassphraseEdit	*mConfirmationEditField;
	CProgressBar	*mPassQualityProgressBar;
	LGACheckbox		*mHideTypingCheckbox;
};

CKeyPassphraseView::CKeyPassphraseView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CKeyPassphraseView::~CKeyPassphraseView(void)
{
}

	void
CKeyPassphraseView::GetPassphrase(
	StringPtr	passphrase,
	StringPtr	confirmation)
{
	mPassphraseEditField->GetDescriptor( passphrase );
	mConfirmationEditField->GetDescriptor( confirmation );
}

	void
CKeyPassphraseView::ClearPassphrase(void)
{
	mPassphraseEditField->ClearPassphrase();
	mConfirmationEditField->ClearPassphrase();
	mPassQualityProgressBar->SetPercentComplete( 0 );
	
	LCommander::SwitchTarget( mPassphraseEditField );
}

	void
CKeyPassphraseView::FinishCreateSelf(void)
{	
	PGPUInt32	minLength = kRecommendedMinPassLength;
	Str255		lengthString;
	
	CKeyGenWizardView::FinishCreateSelf();
	
	mPassphraseEditField 	= (CPassphraseEdit *)
		FindPaneByID( kPassphraseEditFieldPaneID );
	mConfirmationEditField 	= (CPassphraseEdit *)
		FindPaneByID( kConfirmationEditFieldPaneID );
	mPassQualityProgressBar = (CProgressBar *)
		FindPaneByID( kPassphraseQualityBarPaneID );
	mHideTypingCheckbox 	= (LGACheckbox *)
		FindPaneByID( kHideTypingCheckboxPaneID );

	pgpAssertAddrValid( mPassphraseEditField, VoidAlign );
	pgpAssertAddrValid( mConfirmationEditField, VoidAlign );
	pgpAssertAddrValid( mPassQualityProgressBar, VoidAlign );
	pgpAssertAddrValid( mHideTypingCheckbox, LGACheckbox );

	mPassphraseEditField->AddListener(this);
	mHideTypingCheckbox->AddListener( this );
	
#if PGP_BUSINESS_SECURITY
	{
		Boolean		enforceMinQuality,
					enforceMinLength;
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
				
				((LCaption *)FindPaneByID(kRecommendedQualityCaptionPaneID))->
					Show();
				recQualityBar =
					(CProgressBar *)FindPaneByID(kRecommendedQualityBarPaneID);
				recQualityBar->Show();
				recQualityBar->SetPercentComplete(minQuality);
			}
		}
		err = PGPGetPrefBoolean(
				gAdminPrefRef, kPGPPrefEnforceMinChars, &enforceMinLength);
		pgpAssertNoErr(err);
		if(IsntPGPError(err) && enforceMinLength)
		{
			err = PGPGetPrefNumber(
					gAdminPrefRef, kPGPPrefMinChars, &minLength);
			pgpAssertNoErr(err);
		}
	}
#endif
	::NumToString(minLength, lengthString);
	((LCaption *)FindPaneByID(kMinPassphraseLenCaptionPaneID))->
		SetDescriptor(lengthString);
	LCommander::SwitchTarget( mPassphraseEditField );	
}

	void
CKeyPassphraseView::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch( inMessage )
	{
		case kPassphraseEditFieldPaneID:
		{
			CSecureCString256	passphrase;
			PGPUInt16			quality;
			
			mPassphraseEditField->GetDescriptor( (uchar *) passphrase.mString );
			PToCString( (uchar *) passphrase.mString, passphrase.mString );
			
			quality = PGPCalcPassphraseQuality( passphrase );
			mPassQualityProgressBar->SetPercentComplete( quality );
			break;
		}

		case kHideTypingCheckboxPaneID:
		{
			Boolean	hide;
			
			hide = ( mHideTypingCheckbox->GetValue() == 1 );
			
			mPassphraseEditField->SetHideTyping( hide );
			mConfirmationEditField->SetHideTyping( hide );
			
			break;
		}
	}
}

#pragma mark --- CGetRandomDataView ---

class CGetRandomDataView :  public CKeyGenWizardView,
							public LBroadcaster,
							public LPeriodical
{
public:

	enum { class_ID = 'vGRD' };
	
						CGetRandomDataView(LStream *inStream);
	virtual				~CGetRandomDataView();

	virtual	Boolean		HandleKeyPress(const EventRecord& inKeyEvent);
	void				SetParams(
							PGPPublicKeyAlgorithm	algorithm,
							UInt32					encryptKeySize,
							UInt32					signingKeySize);
	virtual	void		SpendTime(const EventRecord &inMacEvent);

protected:

	virtual	void		FinishCreateSelf(void);

private:

	SInt32			mNeededEntropy;
	SInt32			mStartEntropy;
	Boolean			mCollectingData;
	UInt32			mEncryptKeySize;
	UInt32			mAlgorithm;
	CProgressBar	*mProgressBar;
	UInt32			mEntropyCollected;
	Point			mLastMouseLoc;
	
	enum
	{
		kMinimumUserEntropy = 300,
		
		kProgressBarPaneID	= 'bPrg'
	};
};

CGetRandomDataView::CGetRandomDataView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
	mCollectingData 	= true;
	mEntropyCollected	= 0;
}

CGetRandomDataView::~CGetRandomDataView()
{
}

	void
CGetRandomDataView::FinishCreateSelf(void)
{
	CKeyGenWizardView::FinishCreateSelf();
	
	mProgressBar = (CProgressBar *) FindPaneByID( kProgressBarPaneID );
	pgpAssertAddrValid( mProgressBar, CProgressBar );
	StartRepeating();

	SwitchTarget( this );
	StartIdling();
}

	void
CGetRandomDataView::SpendTime(const EventRecord &inMacEvent)
{
	if( mCollectingData )
	{
		PGPUInt32	curEntropy;		
		GrafPtr		savePort;
		GrafPtr		wMgrPort;
		Point		mouseLoc;
		UInt32		complete;

		::GetPort(&savePort);
		::GetWMgrPort(&wMgrPort);	
		
		::SetPort(wMgrPort);
		::GetMouse(&mouseLoc);
		::SetPort(savePort);
		
		if((mouseLoc.v != mLastMouseLoc.v) ||
			(mouseLoc.h != mLastMouseLoc.h))
		{
			mEntropyCollected +=
				PGPGlobalRandomPoolAddMouse(mouseLoc.h, mouseLoc.v);
			mLastMouseLoc = mouseLoc;
		}

		curEntropy = PGPGlobalRandomPoolGetEntropy();
		if( curEntropy >= PGPGlobalRandomPoolGetSize() )
		{
			// Collect extra random bits even if the pool is full
			if(mStartEntropy + mEntropyCollected > curEntropy)
				curEntropy = mStartEntropy + mEntropyCollected;
		}
		
		pgpAssertAddrValid( mProgressBar, CProgressBar );
		complete = ( curEntropy - mStartEntropy ) *
								100 / ( mNeededEntropy - mStartEntropy );
		if(complete > 100)
			complete = 100;
		mProgressBar->SetPercentComplete(complete);

		if( curEntropy >= mNeededEntropy )
		{
			mProgressBar->SetPercentComplete( 100 );
			mCollectingData = false;
			
			BroadcastMessage( kAllowViewSwitchMessage, nil );
		}
	}
}

	Boolean
CGetRandomDataView::HandleKeyPress(const EventRecord &inKeyEvent)
{
	Boolean	keyHandled;
	Char16	theChar = inKeyEvent.message & charCodeMask;
	
	if( mCollectingData )
	{	
		mEntropyCollected += PGPGlobalRandomPoolAddKeystroke(theChar);
		keyHandled = true;
	}
	else
	{
		keyHandled = CKeyGenWizardView::HandleKeyPress( inKeyEvent );
	}

	return keyHandled;
}

	void
CGetRandomDataView::SetParams(
	PGPPublicKeyAlgorithm	algorithm,
	UInt32					encryptKeySize,
	UInt32					signingKeySize)
{
	PGPError	err;
	PGPBoolean	fastGen;
	
	err = PGPGetPrefBoolean(gPrefRef, kPGPPrefFastKeyGen, &fastGen);
	pgpAssertNoErr(err);
	mStartEntropy = PGPGlobalRandomPoolGetEntropy();
	
	mNeededEntropy = PGPGetKeyEntropyNeeded(
			gPGPContext,
			PGPOKeyGenParams(gPGPContext, algorithm, signingKeySize),
			PGPOKeyGenFast(gPGPContext, fastGen),
			PGPOLastOption(gPGPContext));
	if( algorithm == kPGPPublicKeyAlgorithm_DSA )
	{
		mNeededEntropy += PGPGetKeyEntropyNeeded(
			gPGPContext,
			PGPOKeyGenParams(gPGPContext,
							kPGPPublicKeyAlgorithm_ElGamal, encryptKeySize),
			PGPOKeyGenFast(gPGPContext, fastGen),
			PGPOLastOption(gPGPContext));
	}
	
	// Force collection of some data.
	if( mNeededEntropy - mStartEntropy < (SInt32) kMinimumUserEntropy )
		mNeededEntropy = mStartEntropy + kMinimumUserEntropy;

	mNeededEntropy = min( mNeededEntropy, PGPGlobalRandomPoolGetSize() );
}

#pragma mark --- CGeneratingKeyView ---

class CGeneratingKeyView : public CKeyGenWizardView
{
public:

	enum { class_ID = 'vGen' };
	
				CGeneratingKeyView(LStream *inStream);
	virtual		~CGeneratingKeyView();

};

CGeneratingKeyView::CGeneratingKeyView(LStream *inStream)
	: CKeyGenWizardView(inStream)
{
}

CGeneratingKeyView::~CGeneratingKeyView()
{
}

#pragma mark --- CKeyGenWizardDialog ---

CKeyGenWizardDialog::CKeyGenWizardDialog(LStream *inStream)
	: LGADialogBox(inStream)
{
	mCurrentView 		= kViewInvalid;
	mFinished			= false;
	mGeneratedKey		= kPGPInvalidRef;
	mCurrentViewObj		= nil;
	mEmailAddress[0]	= 0;
	mEncryptKeySize		= 2048;
	mSigningKeySize		= 1024;
	mAllowRSAKeygen		= TRUE;
	mAlgorithm			= kPGPPublicKeyAlgorithm_DSA;
	mExpirationDays		= 0;
	mGeneratingKey		= false;
	mParentKeyTable		= nil;
	
	GetMachineOwnerName( mName );
}

CKeyGenWizardDialog::~CKeyGenWizardDialog()
{
	// Clear items for security reasons.
	pgpClearMemory( mName, sizeof( mName ) );
	pgpClearMemory( mEmailAddress, sizeof( mEmailAddress ) );
	pgpClearMemory( mPassphrase, sizeof( mPassphrase ) );
	pgpClearMemory( mConfirmation, sizeof( mConfirmation ) );
}

	void
CKeyGenWizardDialog::FinishCreateSelf()
{
	LGADialogBox::FinishCreateSelf();
	
	RegisterClass_( CKeyGenWizardView );
	RegisterClass_( CNameAndAddressView );
	RegisterClass_( CKeyTypeView );
	RegisterClass_( CKeySizeView );
	RegisterClass_( CKeyExpirationView );
	RegisterClass_( CMRKView );
	RegisterClass_( CCorpView );
	RegisterClass_( CKeyPassphraseView );
	RegisterClass_( CGetRandomDataView );
	RegisterClass_( CGeneratingKeyView );
	
	mNextButton = (LGAPushButton *) FindPaneByID( kNextButtonPaneID );
	pgpAssertAddrValid( mNextButton, VoidAlign );

	mPreviousButton = (LGAPushButton *) FindPaneByID( kPreviousButtonPaneID );
	pgpAssertAddrValid( mPreviousButton, VoidAlign );

	mCancelButton = (LGAPushButton *) FindPaneByID( kCancelButtonPaneID );
	pgpAssertAddrValid( mCancelButton, VoidAlign );

	// mNextButton already added by the modal dialog handler.
	mPreviousButton->AddListener( this );
	
	mMRKKey = NULL;
	mCorpKey = NULL;
#if PGP_BUSINESS_SECURITY
	{
		PGPError		err;
		Boolean			useMRK,
						autoCorp;
		PGPKeySetRef	keySet;
		PGPKeyID		keyID;
		PGPSize			len;
		PGPByte			*keyIDRaw;
			
		
		keySet = CPGPKeys::TheApp()->GetKeySet();
		// Set MRK on this key?
		err = PGPGetPrefBoolean(gAdminPrefRef, kPGPPrefUseDHADK, &useMRK);
		pgpAssertNoErr(err);
		if(useMRK)
		{
			err = PGPGetPrefData(	gAdminPrefRef, kPGPPrefDHADKID,
									&len, &keyIDRaw);
			pgpAssertNoErr(err);
			if(IsntPGPError(err))
			{
				err = PGPImportKeyID( keyIDRaw, &keyID);
				pgpAssertNoErr(err);
				if(IsntPGPError(err))
				{
					err = PGPGetKeyByKeyID(	keySet,
											&keyID,
											kPGPPublicKeyAlgorithm_DSA,
											&mMRKKey);
					pgpAssertNoErr(err);
				}
				err = PGPDisposePrefData(gAdminPrefRef, keyIDRaw);
				pgpAssertNoErr(err);
			}
		}
		// Should we auto-sign/trust the Corporate Key?
		err = PGPGetPrefBoolean(gAdminPrefRef,
								kPGPPrefAutoSignTrustCorp, &autoCorp);
		pgpAssertNoErr(err);
		if(autoCorp)
		{
			err = PGPGetPrefData(	gAdminPrefRef, kPGPPrefCorpKeyID,
									&len, &keyIDRaw);
			pgpAssertNoErr(err);
			if(IsntPGPError(err))
			{
				PGPUInt32	corpKeyAlgorithm;
				
				err = PGPImportKeyID( keyIDRaw, &keyID);
				pgpAssertNoErr(err);
				if(IsntPGPError(err))
				{
					err = PGPGetPrefNumber(gAdminPrefRef,
								kPGPPrefCorpKeyPublicKeyAlgorithm,
								&corpKeyAlgorithm );
					pgpAssertNoErr(err);
					
					err = PGPGetKeyByKeyID(	keySet,
									&keyID,
									(PGPPublicKeyAlgorithm)corpKeyAlgorithm,
									&mCorpKey);
					pgpAssertNoErr(err);
				}
				err = PGPDisposePrefData(gAdminPrefRef, keyIDRaw);
				pgpAssertNoErr(err);
			}
		}
		if(useMRK && !mMRKKey)
		{
			CWarningAlert::Display(	kWAStopAlertType, kWAOKStyle, 
								kStringListResID, kNoMRKErrorStrIndex );
			mFinished = true;
		}
		else if(autoCorp && !mCorpKey)
		{
			CWarningAlert::Display(	kWAStopAlertType, kWAOKStyle, 
								kStringListResID, kNoCorpKeyErrorStrIndex );
			mFinished = true;
		}
		err = PGPGetPrefBoolean(gAdminPrefRef, kPGPPrefAllowRSAKeyGen,
								&mAllowRSAKeygen);
		pgpAssertNoErr(err);
	}
#endif
	if(PGP_FREEWARE || NO_RSA_KEYGEN)
		mAllowRSAKeygen = FALSE;
	
	SwitchToView( kViewIntroduction, FALSE );
}

	void
CKeyGenWizardDialog::SetKeyTable(CKeyTable *keyTable)
{
	pgpAssertAddrValid(keyTable, CKeyGenWizardDialog *);
	mParentKeyTable = keyTable;
}

	void
CKeyGenWizardDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch( inMessage )
	{
		case kNextViewMessage:
		{
			GotoNextView();
			
			if( FinishedKeyGeneration() )
			{
				DoClose();
			}
			break;
		}
			
		case kPreviousViewMessage:
			GotoPreviousView();
			break;
			
		case msg_Cancel:
		{
			if( mGeneratingKey )
			{
				// This is the error returned if the user cancels.
				mKeyGenerationError = kPGPError_OutOfMemory;
			}
			else
			{
				DoClose();
			}
			
			break;
		}
		
		case kAllowViewSwitchMessage:
			mNextButton->Enable();
			mPreviousButton->Enable();
			break;
	}
}

	void
CKeyGenWizardDialog::AdjustControlsForView(KeyGenerationView theView)
{
	short	titleIndex = kNextButtonTitleStrIndex;
	Str255	buttonTitle;
	
	mNextButton->Enable();
	mPreviousButton->Enable();

	switch( theView )
	{
		case kViewIntroduction:
			mPreviousButton->Disable();
			break;
		case kViewGetRandomData:
		case kViewGeneratingKey:
			mPreviousButton->Disable();
			mNextButton->Disable();
			break;
		case kViewKeyBadPassphrase:
			if(mPassphraseEnforceLength || !mPassphraseGoodQuality ||
					!mPassphraseMatched)
				mNextButton->Disable();
			break;
		case kViewConclusion:
			titleIndex = kDoneButtonTitleStrIndex;
		case kViewSendToServer:
			mPreviousButton->Disable();
			mCancelButton->Disable();
			break;
	}
	
	GetIndString( buttonTitle, kStringListResID, titleIndex );
	mNextButton->SetDescriptor( buttonTitle );
}

	Boolean
CKeyGenWizardDialog::IsGoodPassphrase(
	ConstStr255Param	passphrase,
	ConstStr255Param	confirmation,
	Boolean				&confirmMatch,
	Boolean				&enforceLength,
	Boolean				&goodLength,
	Boolean				&goodQuality)
{
	goodLength = TRUE;
	goodQuality = TRUE;
	confirmMatch = TRUE;
	enforceLength = FALSE;
	if(!PStringsAreEqual(passphrase, confirmation))
		confirmMatch = FALSE;
#if	PGP_BUSINESS_SECURITY
	{
		PGPBoolean	enforceMinLength,
					enforceMinQuality;
		PGPUInt32	minLength,
					minQuality;
		PGPError	err;
		
		err = PGPGetPrefBoolean(
				gAdminPrefRef, kPGPPrefEnforceMinChars, &enforceMinLength);
		pgpAssertNoErr(err);
		if(IsntPGPError(err) && enforceMinLength)
		{
			err = PGPGetPrefNumber(
					gAdminPrefRef, kPGPPrefMinChars, &minLength);
			pgpAssertNoErr(err);
			if(passphrase[0] < minLength)
				goodLength = FALSE;
			enforceLength = TRUE;
		}
		err = PGPGetPrefBoolean(
				gAdminPrefRef, kPGPPrefEnforceMinQuality, &enforceMinQuality);
		pgpAssertNoErr(err);
		if(IsntPGPError(err) && enforceMinQuality)
		{
			CSecureCString256	cPassphrase;
			
			err = PGPGetPrefNumber(
					gAdminPrefRef, kPGPPrefMinQuality, &minQuality);
			pgpAssertNoErr(err);
			
			PToCString( passphrase, cPassphrase );
			if( PGPCalcPassphraseQuality(cPassphrase) < minQuality)
				goodQuality = FALSE;
		}
	}
#endif	// PGP_BUSINESS_SECURITY
	if(passphrase[0] < kRecommendedMinPassLength)
		goodLength = FALSE;
	return (confirmMatch && goodLength && goodQuality);
}

	Boolean
CKeyGenWizardDialog::LeaveCurrentView(KeyGenerationView newView)
{
	Boolean	okToSwitch = true;
	
	switch( mCurrentView )
	{
		case kViewNameAndEmailAddress:
		{
			CNameAndAddressView	*viewObj = (CNameAndAddressView *)
				mCurrentViewObj;
			
			viewObj->GetParams( mName, mEmailAddress );
			
			// Only validate if switching to the next pane
			if( ( newView == (KeyGenerationView) ((UInt32)
				kViewNameAndEmailAddress + 1 ) ) ||
				( newView == (KeyGenerationView) ((UInt32)
				kViewNameAndEmailAddress + 2 ) ) )
			{
				if( mName[0] == 0 )
				{
					SysBeep(1);
					
					CWarningAlert::Display( kWACautionAlertType, kWAOKStyle, 
								kStringListResID, kNameRequiredStrIndex );
					okToSwitch = false;
				}
				else if( mEmailAddress[0] == 0 )
				{
					SysBeep(1);
					
					if( CWarningAlert::Display(	kWACautionAlertType,
											kWAOKCancelStyle, 
											kStringListResID,
											kVerifyNoEmailAddressStrIndex )
											!= msg_OK )
					{
						okToSwitch = false;
					}
				}
			}
			
			break;
		}
		
		case kViewKeyType:
		{
			CKeyTypeView	*viewObj = (CKeyTypeView *) mCurrentViewObj;
			
			mAlgorithm = viewObj->GetAlgorithm();
			break;
		}
		
		case kViewKeySize:
		{
			CKeySizeView	*viewObj = (CKeySizeView *) mCurrentViewObj;
			UInt32			maxKeySize;
			
			if( mAlgorithm == kPGPPublicKeyAlgorithm_DSA )
			{
				maxKeySize = kDSAElGamalMaxKeySize;
			}
			else
			{
				maxKeySize = kRSAMaxKeySize;
			}
			
			mEncryptKeySize = viewObj->GetKeySize();
			if( mEncryptKeySize < kMinKeySize || mEncryptKeySize > maxKeySize )
			{
				Str32	numStr;
				
				NumToString( maxKeySize, numStr );
				
				SysBeep(1);
				
				(void)CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
					kStringListResID, kKeySizeOutOfRangeStrIndex, numStr );
				
				okToSwitch = false;
			}
			else
			{
#if PGP_BUSINESS_SECURITY
				PGPUInt32	minAdminKeySize;
				PGPError	err;
				
				err = PGPGetPrefNumber(gAdminPrefRef,
									kPGPPrefMinimumKeySize, &minAdminKeySize);
				pgpAssertNoErr(err);
				if(mEncryptKeySize < minAdminKeySize)
				{
					Str255	adminKeySizeStr;
					
					::NumToString(minAdminKeySize, adminKeySizeStr);
					(void)CWarningAlert::Display(kWACautionAlertType,
									kWAOKStyle,
									kStringListResID,
									kAdminKeySizeErrorStrIndex,
									adminKeySizeStr );
					okToSwitch = false;
				}
#endif
				if(mEncryptKeySize < 768)
				{
					CWarningAlert::Display(	kWANoteAlertType,
										kWAOKStyle, 
										kStringListResID,
										k768WarningStrIndex );
				}
				if( mAlgorithm == kPGPPublicKeyAlgorithm_DSA )
				{
					if( mEncryptKeySize < 1024 )
					{
						mSigningKeySize = mEncryptKeySize;
					}
					else
						mSigningKeySize = 1024;
				}
				else
				{
					mSigningKeySize = mEncryptKeySize;
				}
			}

			break;
		}

		case kViewKeyExpiration:
		{
			CKeyExpirationView	*viewObj = (CKeyExpirationView *)
													mCurrentViewObj;
			
			if( ! viewObj->GetExpiration( &mExpirationDays ) )
			{
				SysBeep(1);
				
				(void) CWarningAlert::Display( kWACautionAlertType, kWAOKStyle, 
										kStringListResID,
										kKeyExpirationOutOfRangeStrIndex );
				okToSwitch = false;
			}
			
			break;
		}
		
		case kViewSendToServer:
		{
			CSendToServerView	*viewObj = (CSendToServerView *)
													mCurrentViewObj;

			viewObj->DoSendToServer(mGeneratedKey);
			break;
		}
	}

	if( okToSwitch && IsntNull( mCurrentViewObj ) )
	{
		Rect	viewRect;
		
		mCurrentViewObj->CalcPortFrameRect( viewRect );
		delete mCurrentViewObj;				// delete it
		mCurrentViewObj = nil;
		
		FocusDraw();
		ApplyForeAndBackColors();
		EraseRect( &viewRect );
	}
	
	return( okToSwitch );
}

	void
CKeyGenWizardDialog::EnterView(KeyGenerationView newView)
{
	ResIDT	newViewResID;
	
	switch( newView )
	{
		case kViewIntroduction:
			newViewResID = view_Introduction;
			break;

		case kViewNameAndEmailAddress:
			newViewResID = view_NameAndAddress;
			break;

		case kViewKeyType:
			newViewResID = view_KeyType;
			break;

		case kViewKeySize:
			newViewResID = view_KeySize;
			break;
			
		case kViewKeyExpiration:
			newViewResID = view_KeyExpiration;
			break;

		case kViewKeyMRK:
			newViewResID = view_MRK;
			break;

		case kViewSignCorp:
			newViewResID = view_Corp;
			break;

		case kViewKeyPassphrase:
			newViewResID = view_KeyPassphrase;
			break;

		case kViewKeyBadPassphrase:
			newViewResID = view_BadPassphrase;
			break;

		case kViewGetRandomData:
			newViewResID = view_GetRandomData;
			break;

		case kViewGeneratingKey:
			newViewResID = view_GeneratingKey;
			break;
			
		case kViewSendToServer:
			newViewResID = view_SendToServer;
			break;

		case kViewConclusion:
			newViewResID = view_Conclusion;
			break;

		default:
			newViewResID = 0;
			break;
	}
	
	if( newViewResID != 0 )
	{
		LCommander::SetDefaultCommander( this );
		LPane::SetDefaultView( this );
		
		mCurrentViewObj = (CKeyGenWizardView *)
							UReanimator::ReadObjects( 'PPob', newViewResID );
		mCurrentViewObj->FinishCreate();
		mCurrentViewObj->SetSuperCommander( this );
		
		switch( newView )
		{
			case kViewNameAndEmailAddress:
			{
				CNameAndAddressView	*viewObj =
						(CNameAndAddressView *) mCurrentViewObj;
				
				viewObj->SetParams( mName, mEmailAddress );
				break;
			}

			case kViewKeyType:
			{
				CKeyTypeView	*viewObj = (CKeyTypeView *) mCurrentViewObj;
				
				viewObj->SetAlgorithm( mAlgorithm );
				break;
			}

			case kViewKeySize:
			{
				CKeySizeView	*viewObj = (CKeySizeView *) mCurrentViewObj;
				
				viewObj->SetParams( mAlgorithm, mEncryptKeySize );
				break;
			}

			case kViewKeyExpiration:
			{
				CKeyExpirationView	*viewObj =
					(CKeyExpirationView *) mCurrentViewObj;
				
				viewObj->SetExpiration( mExpirationDays );
				break;
			}

			case kViewKeyMRK:
			{
				CMRKView	*viewObj =
					(CMRKView *) mCurrentViewObj;
				
				viewObj->SetMRK(mMRKKey);
				break;
			}

			case kViewSignCorp:
			{
				CCorpView	*viewObj =
					(CCorpView *) mCurrentViewObj;
				
				viewObj->SetCorpKey(mCorpKey);
				break;
			}
			
			case kViewKeyBadPassphrase:
			{
				CBadPassphraseView	*viewObj =
					(CBadPassphraseView *) mCurrentViewObj;
				
				viewObj->SetPassphrase(mPassphrase, mConfirmation);
				break;
			}
			
			case kViewGetRandomData:
			{
				CGetRandomDataView	*viewObj =
					(CGetRandomDataView *) mCurrentViewObj;
				
				viewObj->SetParams( mAlgorithm, mEncryptKeySize,
									mSigningKeySize );
				viewObj->AddListener( this );
				break;
			}
		}
		
		mCurrentViewObj->Show();
		mCurrentViewObj->Draw( nil );
		mCurrentViewObj->DontRefresh();
	}
	
	mCurrentView = newView;
}


	void
CKeyGenWizardDialog::SwitchToView(	KeyGenerationView newView,
									Boolean increment)
{
	Boolean directional = FALSE;
	
	if(newView == kViewInvalid)
	{
		directional = TRUE;
		if(increment)
			newView = (KeyGenerationView)((UInt32)mCurrentView + 1);
		else
			newView = (KeyGenerationView)((UInt32)mCurrentView - 1);
	}
	if( newView == kViewExit )
	{
		mFinished = true;
	}
	else
	{
		if(directional)
		{
			if(newView == kViewKeyType && !mAllowRSAKeygen)
			{
				if(increment)
					newView = (KeyGenerationView)((UInt32)newView + 1);
				else
					newView = (KeyGenerationView)((UInt32)newView - 1);
			}
			if(newView == kViewKeyMRK && (!mMRKKey ||
				(mAlgorithm == kPGPPublicKeyAlgorithm_RSA)))
			{
				if(increment)
					newView = (KeyGenerationView)((UInt32)newView + 1);
				else
					newView = (KeyGenerationView)((UInt32)newView - 1);
			}
			if(newView == kViewSignCorp && !mCorpKey)
			{
				if(increment)
					newView = (KeyGenerationView)((UInt32)newView + 1);
				else
					newView = (KeyGenerationView)((UInt32)newView - 1);
			}
			if(newView == kViewKeyMRK && !mMRKKey)
			{
				if(increment)
					newView = (KeyGenerationView)((UInt32)newView + 1);
				else
					newView = (KeyGenerationView)((UInt32)newView - 1);
			}
			if(newView == kViewKeyBadPassphrase)
			{
				if(!increment)
					newView = (KeyGenerationView)((UInt32)newView - 1);
				else
				{
					CKeyPassphraseView	*viewObj = (CKeyPassphraseView *)
															mCurrentViewObj;
					
					viewObj->GetPassphrase(mPassphrase, mConfirmation);
					if(IsGoodPassphrase(mPassphrase, mConfirmation,
										mPassphraseMatched,
										mPassphraseEnforceLength,
										mPassphraseGoodLength,
										mPassphraseGoodQuality))
					{
						newView = (KeyGenerationView)((UInt32)newView + 1);
					}
				}
			}
			if(newView == kViewSendToServer)
			{
				if(!gServerCallsPresent)
					newView = (KeyGenerationView)((UInt32)newView + 1);
			}
		}
		
		// Skip the random input collection pane if the pool is full.
		if( newView == kViewGetRandomData )
		{
			if( PGPGlobalRandomPoolGetEntropy() ==
				PGPGlobalRandomPoolGetSize() )
			{
				newView = (KeyGenerationView)
					((UInt32) kViewGetRandomData + 1);
			}
		}
		
		if( LeaveCurrentView( newView ) )
		{
			AdjustControlsForView( newView );
			EnterView( newView );
			
			// Special case the state machine for generating key
			if( newView == kViewGeneratingKey )
			{
				Boolean	goodKey;
				
				goodKey = GenerateKey();
				if( goodKey )
				{
					GotoNextView();
				}
				else
				{
					SwitchToView( kViewExit, FALSE );
				}
			}
		}
	}
}

	void
CKeyGenWizardDialog::GotoNextView(void)
{
	if( mCurrentView != kViewConclusion )
	{
		SwitchToView(kViewInvalid, TRUE);
	}
	else
	{
		mFinished = true;
	}
}

	void
CKeyGenWizardDialog::GotoPreviousView(void)
{
	pgpAssert( mCurrentView != kViewInvalid );
	
	if( mCurrentView != kViewIntroduction )
	{
		SwitchToView(kViewInvalid, FALSE);
	}
}

	void
CKeyGenWizardDialog::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	outEnabled = false;
}

	PGPError
CKeyGenWizardDialog::GenerateKeyCallback(PGPContextRef context,
						PGPEvent *event, PGPUserValue userValue)
{
	ulong				timeNow;
	CKeyGenWizardDialog	*dialogObj = (CKeyGenWizardDialog *) userValue;
	static ulong		lastProgressTime;
	static ulong		lastEventTime;
	
	if(event->type == kPGPEvent_KeyGenEvent)
	{
		timeNow = LMGetTicks();
		if( timeNow - lastProgressTime > 10 )
		{
			CInfinityBar	*genThermometer;

			genThermometer = (CInfinityBar *)
				dialogObj->FindPaneByID( kGeneratingKeyProgressID );
			pgpAssertAddrValid( genThermometer, VoidAlign );
			
			genThermometer->IncrementState();
			
			lastProgressTime = timeNow;
		}
		
		if( timeNow - lastEventTime > 30 )
		{
			lastEventTime = timeNow;
			
			CPGPKeys::TheApp()->ProcessNextEvent();
		}
	}
	return( mKeyGenerationError );
}

	Boolean
CKeyGenWizardDialog::GenerateKey(void)
{
	PGPError			err;
	PGPKeyRef			key = kPGPInvalidRef;
	char				userIDStr[256];
	CSecureCString256	passphraseC;
	Boolean				good = false;
	PGPBoolean			fastGen;
	PGPUInt32			numAllowed;
	PGPCipherAlgorithm	*allowedAlgs;
	PGPSize				dataSize;
	
	PToCString( mName, userIDStr );
	if( mEmailAddress[0] != 0 )
	{
		char	emailAddress[256];
		
		PToCString( mEmailAddress, emailAddress );
		
		strcat( userIDStr, " <" );
		strcat( userIDStr, emailAddress );
		strcat( userIDStr, ">" );
	}
	
	PToCString( mPassphrase, passphraseC);
	
	mGeneratingKey 		= true;
	mKeyGenerationError	= noErr;
	
	// Force dimming of menu bar.
	LCommander::SetUpdateCommandStatus( TRUE );
	LEventDispatcher::GetCurrentEventDispatcher()->UpdateMenus();
		
	err = PGPGetPrefBoolean(gPrefRef, kPGPPrefFastKeyGen, &fastGen);
	pgpAssertNoErr(err);

	err = PGPGetPrefData(gPrefRef, kPGPPrefAllowedAlgorithmsList,
				&dataSize, &allowedAlgs);
	pgpAssertNoErr(err);
	numAllowed = dataSize / sizeof(PGPCipherAlgorithm);
	
#if PGP_BUSINESS_SECURITY
	if(mMRKKey)
	{
		Boolean 		enforce = FALSE;
		PGPKeySetRef	mrkKeySet;
		
		err = PGPNewSingletonKeySet(mMRKKey, &mrkKeySet);
		pgpAssertNoErr(err);
		err = PGPGetPrefBoolean(gAdminPrefRef,
								kPGPPrefEnforceIncomingADK, &enforce);
		pgpAssertNoErr(err);
		err = PGPGenerateKey(	gPGPContext, &key,
								PGPOKeyGenParams(gPGPContext, 
									mAlgorithm, mSigningKeySize),
								PGPOKeyGenName(gPGPContext, 
									userIDStr, strlen(userIDStr)),
								PGPOKeySetRef(gPGPContext, 
									CPGPKeys::TheApp()->GetKeySet()),
								PGPOPassphrase(gPGPContext, passphraseC),
								PGPOKeyGenFast(gPGPContext, fastGen),
								PGPOExpiration(gPGPContext, mExpirationDays),
								PGPOPreferredAlgorithms(
									gPGPContext, allowedAlgs, numAllowed ),
								PGPOEventHandler(gPGPContext, 
									GenerateKeyCallback, this),
								PGPOAdditionalRecipientRequestKeySet(
									gPGPContext, 
									mrkKeySet, enforce ? 0x80 : 0),
								PGPOLastOption(gPGPContext)
							);
		err = PGPFreeKeySet(mrkKeySet);
		pgpAssertNoErr(err);
	}
	else
#endif
	{
		err = PGPGenerateKey(	gPGPContext, &key,
								PGPOKeyGenParams(gPGPContext,
									mAlgorithm, mSigningKeySize),
								PGPOKeyGenName(gPGPContext, 
									userIDStr, strlen(userIDStr)),
								PGPOKeySetRef(gPGPContext, 
									CPGPKeys::TheApp()->GetKeySet()),
								PGPOPassphrase(gPGPContext, passphraseC),
								PGPOKeyGenFast(gPGPContext, fastGen),
								PGPOExpiration(gPGPContext, mExpirationDays),
								PGPOPreferredAlgorithms(
									gPGPContext, allowedAlgs, numAllowed ),
								PGPOEventHandler(gPGPContext,
									GenerateKeyCallback, this),
								PGPOLastOption(gPGPContext)
							);
	}
	
	if(IsntNull(allowedAlgs))
		PGPDisposePrefData(gPrefRef, allowedAlgs);
	
	mGeneratingKey = false;
	
	// who knows why the lib returns nomem if
	// the user aborts the generatekey....
	// the world will never know...
	if(err != kPGPError_OutOfMemory)
	{
		pgpAssert(IsntErr(err));
		if( err )
		{
			CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
						kStringListResID, kKeyGenerationErrorStrIndex );
		}
	}
	
	if(IsntPGPError(err))
	{
		if( mAlgorithm == kPGPPublicKeyAlgorithm_DSA)
		{
			PGPSubKeyRef subKey;
			
			mGeneratingKey 		= true;
			mKeyGenerationError	= noErr;
			
			err = PGPGenerateSubKey(gPGPContext, &subKey,
									PGPOKeyGenMasterKey(gPGPContext, key),
									PGPOKeyGenParams(gPGPContext, 
										kPGPPublicKeyAlgorithm_ElGamal,
										mEncryptKeySize),
									PGPOPassphrase(gPGPContext, passphraseC),
									PGPOExpiration(gPGPContext,
										mExpirationDays),
									PGPOKeyGenFast(gPGPContext, fastGen),
									PGPOEventHandler(gPGPContext, 
										GenerateKeyCallback,
										this),
									PGPOLastOption(gPGPContext)
								);
			mGeneratingKey = false;
			
			if(IsPGPError(err))
			{
				PGPKeySetRef genKeySet;
				
				if(err != kPGPError_OutOfMemory)
				{
					pgpAssert(IsntErr(err));
					if(err)
					{
						CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
									kStringListResID,
									kKeyGenerationErrorStrIndex);
					}				
				}
				// Make sure DSS key doesn't sit around naked without
				// its ElGamal buddy
				
				err = PGPNewSingletonKeySet(key, &genKeySet);
				pgpAssertNoErr(err);
				err = PGPRemoveKeys(  genKeySet,
						CPGPKeys::TheApp()->GetKeySet() );
				pgpAssertNoErr(err);
				PGPFreeKeySet(genKeySet);
			}
		}
		
		if(IsntPGPError(err))
		{
#if	PGP_BUSINESS_SECURITY
			// Shall we sign and trust the Corporate key?
			if(mCorpKey)
			{
				PGPUserIDRef	userID;
				PGPBoolean		metaCorp = FALSE;

				err = PGPGetPrefBoolean(
					gAdminPrefRef, kPGPPrefMetaIntroducerCorp,
					&metaCorp);
				err = PGPGetPrimaryUserID(mCorpKey, &userID);
				pgpAssertNoErr(err);
				CPGPKeys::TheApp()->GetMinimumRandomData();
				err = PGPSignUserID(userID, key,
					PGPOPassphrase(gPGPContext, passphraseC),
					PGPOExpiration(gPGPContext, 0),
					PGPOExportable(gPGPContext, FALSE),
					PGPOSigTrust(gPGPContext,
						metaCorp ? 2 : 0, kPGPKeyTrust_Complete),
					PGPOLastOption(gPGPContext));
				pgpAssertNoErr(err);
				err = PGPSetKeyTrust(mCorpKey, kPGPKeyTrust_Complete);
				// Valid errors can occur here which do not appear
				// to have reasonable result codes, so we ignore errors
				// in this case.  For instance, an error will be returned
				// if the corporate key is on the keyring and is implicitly
				// trusted.
			}
#endif
			mParentKeyTable->ResyncTable(FALSE, TRUE);
			mParentKeyTable->RedrawTable();
			CPGPKeys::TheApp()->CreatedKey();
			CPGPKeys::TheApp()->CommitDefaultKeyrings();
			
			good = true;
		}
		if(good)
			mGeneratedKey = key;
	}

	pgpClearMemory( userIDStr, sizeof( userIDStr ) );
	pgpClearMemory( passphraseC, sizeof( passphraseC ) );
	
	return( good );
}

