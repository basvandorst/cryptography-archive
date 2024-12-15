/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CAdminDialog.cp,v 1.29.4.1.4.1 1997/12/05 22:13:36 mhw Exp $
____________________________________________________________________________*/

#include <LGACheckbox.h>
#include <LGAEditField.h>
#include <LGACaption.h>

#include "MacDesktop.h"
#include "MacFiles.h"
#include "MacResources.h"

#include "pflContext.h"
#include "pgpFileSpec.h"
#include "pflPrefTypes.h"
#include "pgpAdminPrefs.h"
#include "pgpUtilities.h"
#include "PGPOpenPrefs.h"
#include "pgpClientPrefs.h"
#include "CWarningAlert.h"

#include "CString.h"
#include "CTempFile.h"

#include "PGPadminGlobals.h"
#include "CKeyTable.h"
#include "CAdminDialog.h"

const ResIDT	RidL_Dialog									=	128;

const PaneIDT	pane_First									=	1999;
const PaneIDT	pane_Last									=	2017;

const PaneIDT	pane_SiteLicenseNumber						=	1999;
const PaneIDT	pane_InADK									=	2003;
const PaneIDT	pane_OutADK									=	2005;
const PaneIDT	pane_EnforceMessageRecovery					=	2006;
const PaneIDT	pane_PassphraseMinimums						=	2007;
const PaneIDT	pane_CorporateKey							=	2009;
const PaneIDT	pane_KeyGeneration							=	2010;
const PaneIDT	pane_Misc									=	2012;
const PaneIDT	pane_Summary								=	2013;
const PaneIDT	pane_BaseInstaller							=	2015;
const PaneIDT	pane_SaveInstaller							=	2016;

const PaneIDT	button_Next									=	1000;
const PaneIDT	button_Previous								=	1001;
const PaneIDT	button_Quit									=	1002;

const PaneIDT	checkbox_UseInADK							=	1003;
const PaneIDT	keytable_InADK								=	1004;
const PaneIDT	checkbox_UseOutADK							=	1005;
const PaneIDT	keytable_OutADK								=	1006;
const PaneIDT	checkbox_EnforceMinimumCharacters			=	1007;
const PaneIDT	editfield_EnforceMinimumCharacters			=	1008;
const PaneIDT	checkbox_EnforceMinimumQuality				=	1009;
const PaneIDT	editfield_EnforceMinimumQuality				=	1010;
const PaneIDT	checkbox_SignCorporateKey					=	1011;
const PaneIDT	checkbox_WarnNotSignedCorporateKey			=	1012;
const PaneIDT	keytable_CorporateKey						=	1013;
const PaneIDT	checkbox_AllowKeyGeneration					=	1014;
const PaneIDT	checkbox_AllowRSAKeyGeneration				=	1015;
const PaneIDT	keytable_DefaultPublicKeys					=	1016;
const PaneIDT	checkbox_AllowConventionalEncryption		=	1017;
const PaneIDT	editfield_CommentText						=	1018;
const PaneIDT	button_ChooseBaseInstaller					=	1019;
const PaneIDT	caption_BaseInstaller						=	1020;
const PaneIDT	checkbox_EnforceIncomingCADK				=	1021;
const PaneIDT	checkbox_CopyClientPrefs					=	1022;
const PaneIDT	checkbox_MetaIntroduceCorporateKey			=	1023;
const PaneIDT	editfield_MinimumKeySize					=	1024;
const PaneIDT	keytable_Summary							=	1025;
const PaneIDT	caption_SummaryInCADK						=	1026;
const PaneIDT	caption_SummaryInCADKID						=	1027;
const PaneIDT	caption_SummaryOutCADK						=	1028;
const PaneIDT	caption_SummaryOutCADKID					=	1029;
const PaneIDT	caption_SummaryEnforceIncomingCADK			=	1030;
const PaneIDT	caption_SummaryCorporateKey					=	1031;
const PaneIDT	caption_SummaryCorporateKeyID				=	1032;
const PaneIDT	caption_SummaryMetaIntroduceCorporateKey	=	1033;
const PaneIDT	caption_SummaryWarnNotSignedCorporateKey	=	1034;
const PaneIDT	caption_SummaryEnforceMinimumCharacters		=	1035;
const PaneIDT	caption_SummaryMinimumCharacters			=	1036;
const PaneIDT	caption_SummaryEnforceMinimumQuality		=	1037;
const PaneIDT	caption_SummaryMinimumQuality				=	1038;
const PaneIDT	caption_SummaryAllowKeyGeneration			=	1039;
const PaneIDT	caption_SummaryAllowRSAKeyGeneration		=	1040;
const PaneIDT	caption_SummaryAllowConventionalEncryption	=	1041;
const PaneIDT	caption_SummaryMessageHeaderComment			=	1042;
const PaneIDT	caption_SummaryMessageHeaderCommentText		=	1043;
const PaneIDT	caption_SummaryMinimumKeySize				=	1044;
const PaneIDT	caption_SummaryMinimumKeySizeValue			=	1045;
const PaneIDT	checkbox_EnforceOutgoingCADK				=	1046;
const PaneIDT	checkbox_EnforceRemoteCADK					=	1047;
const PaneIDT	caption_SummaryEnforceOutgoingCADK			=	1048;
const PaneIDT	caption_SummaryEnforceRemoteCADK			=	1049;
const PaneIDT	editfield_SiteLicenseNumber					=	1050;

const MessageT	msg_Next				=	button_Next;
const MessageT	msg_Previous			=	button_Previous;
const MessageT	msg_DHADKTable			=	keytable_InADK;
const MessageT	msg_OutADKTable			=	keytable_OutADK;
const MessageT	msg_EnforceMinimumCharacters	
										=	checkbox_EnforceMinimumCharacters;
const MessageT	msg_EnforceMinimumQuality
										=	checkbox_EnforceMinimumQuality;
const MessageT	msg_CorporateKeyTable	=	keytable_CorporateKey;
const MessageT	msg_AllowKeyGeneration			
										=	checkbox_AllowKeyGeneration;
const MessageT	msg_ChooseBaseInstaller	=	button_ChooseBaseInstaller;
const MessageT	msg_SignCorporateKey	=	checkbox_SignCorporateKey;

const SInt16	kMinimumCharacters						=	3;
const SInt16	kMaximumCharacters						=	255;
const SInt16	kMinimumQuality							=	20;
const SInt16	kMaximumQuality							=	100;
const SInt16	kMinimumKeySize							=	512;
const SInt16	kMaximumKeySize							=	4096;
const SInt16	kMaximumRSAKeySize						=	2048;

const OSType	kPGPInstallerType						=	'APPL';
const OSType	kPGPInstallerCreator					=	'VIS3';
const OSType	kPrefFileType							=	'pref';
const OSType	kPGPKeysCreator							=	'pgpK';

const ResType	kAdminResType							=	'ADMN';
const ResIDT	ADMN_AdminPrefs							=	5001;
const ResIDT	ADMN_ClientPrefs						=	5002;

const ResIDT	Txtr_AdminWizard						=	131;
const ResIDT	Txtr_AdminWizardBold					=	132;
const ResIDT	Txtr_AdminWizardBoldRight				=	137;
const ResIDT	Txtr_AdminWizardDisabled				=	138;
const ResIDT	Txtr_AdminWizardBoldDisabled			=	139;
const ResIDT	Txtr_AdminWizardBoldRightDisabled		=	140;

extern PGPContextRef	gPGPContext;


CAdminDialog::CAdminDialog(
	LStream *	inStream)
		: LGADialogBox(inStream), mDirty(false), mCurrentPane(pane_First)
{
}

CAdminDialog::~CAdminDialog()
{
}

	void
CAdminDialog::FinishCreateSelf()
{
	LGADialogBox::FinishCreateSelf();
	
	// Listen to controls
	UReanimator::LinkListenerToControls(this, this, RidL_Dialog);
	
	// Set up tables
	PGPError		pgpErr;
	PGPFilterRef	filter;
	CKeyTable *		theTable;
	
	pgpErr = PGPNewKeyEncryptAlgorithmFilter(gPGPContext, 
											kPGPPublicKeyAlgorithm_ElGamal,
											&filter);
	PGPThrowIfPGPErr_(pgpErr);
	theTable = (CKeyTable *) FindPaneByID(keytable_InADK);
	theTable->SetTableInfo(filter, false);
	theTable->AddListener(this);
	
	theTable = (CKeyTable *) FindPaneByID(keytable_OutADK);
	theTable->SetTableInfo(kPGPInvalidRef, false);
	theTable->AddListener(this);
	theTable = (CKeyTable *) FindPaneByID(keytable_CorporateKey);
	theTable->SetTableInfo(kPGPInvalidRef, false);
	theTable->AddListener(this);
	theTable = (CKeyTable *) FindPaneByID(keytable_DefaultPublicKeys);
	theTable->SetTableInfo(kPGPInvalidRef, true);
	
	// Disable RSA key generation if necessary
#if NO_RSA_KEYGEN
	FindPaneByID(checkbox_AllowRSAKeyGeneration)->Disable();
#endif

	// Set the target
	SwitchTarget((LGAEditField *) FindPaneByID(editfield_SiteLicenseNumber));
}

	void
CAdminDialog::ListenToMessage(
	MessageT	inMessage,
	void *		ioParam)
{
	switch (inMessage) {
		case msg_Cancel:
		{
			ObeyCommand(cmd_Quit, nil);
		}
		break;

		case msg_Next:
		{
			if (VerifyEditFieldValues()) {
				if (mCurrentPane != pane_SaveInstaller) {
					// Hide the current pane
					FindPaneByID(mCurrentPane++)->Hide();
					
					// Skip the unneeded panes
					if (((mCurrentPane == pane_InADK)
					  && (! ((LGACheckbox *)
					  		FindPaneByID(checkbox_UseInADK))->IsSelected()))
					|| ((mCurrentPane == pane_OutADK)
					  && (! ((LGACheckbox *)
					  		FindPaneByID(checkbox_UseOutADK))->IsSelected()))
					|| ((mCurrentPane == pane_CorporateKey)
					  && (! ((LGACheckbox *)
					  		FindPaneByID(checkbox_SignCorporateKey))->
					  		IsSelected()))) {
						mCurrentPane++;
					}
					if (mCurrentPane == pane_EnforceMessageRecovery) {
						if (((LGACheckbox *)
						FindPaneByID(checkbox_UseInADK))->IsSelected()) {
					  		FindPaneByID(checkbox_EnforceIncomingCADK)->
					  			Enable();
					  	} else {
					  		FindPaneByID(checkbox_EnforceIncomingCADK)->
					  			Disable();
					  	}
						if (((LGACheckbox *)
						FindPaneByID(checkbox_UseOutADK))->IsSelected()) {
					  		FindPaneByID(checkbox_EnforceOutgoingCADK)->
					  			Enable();
					  	} else {
					  		FindPaneByID(checkbox_EnforceOutgoingCADK)->
					  			Disable();
					  	}
					}
					
					// Update summary pane
					if (mCurrentPane == pane_Summary) {
						UpdateSummaryPane();
					}
										
					// Show the new pane
					FindPaneByID(mCurrentPane)->Show();
					
					// Adjust the buttons
					LPane *		buttonNext = FindPaneByID(button_Next);
					CKeyTable *	theTable = nil;
					
					switch (mCurrentPane) {
						case pane_InADK:
						{
							theTable = (CKeyTable *)
								FindPaneByID(keytable_InADK);
						}
						break;
						
						
						case pane_OutADK:
						{
							theTable = (CKeyTable *)
								FindPaneByID(keytable_OutADK);
						}
						break;


						case pane_CorporateKey:
						{
							theTable = (CKeyTable *)
								FindPaneByID(keytable_CorporateKey);
						}
						break;
						
						case pane_BaseInstaller:
						{
							CString	descriptor;
							
							FindPaneByID(caption_BaseInstaller)->
								GetDescriptor(descriptor);
							if (descriptor.GetLength() == 0) {
								buttonNext->Disable();
							} else {
								buttonNext->Enable();
							}
						}
						break;
					}
					if (theTable != nil) {
						if (theTable->GetFirstSelectedCell().IsNullCell()) {
							buttonNext->Disable();
						} else {
							buttonNext->Enable();
						}
					}
					FindPaneByID(button_Previous)->Enable();
					if (mCurrentPane == pane_SaveInstaller) {
						buttonNext->SetDescriptor(CString(STRx_AdminStrings,
														kSaveID));
					}
				} else {
					// Save the options
					Save();
				}
			}
		}
		break;


		case msg_Previous:
		{
			if (VerifyEditFieldValues()) {
				// Hide the current pane
				FindPaneByID(mCurrentPane--)->Hide();

				// Skip the unneeded panes
				if (mCurrentPane == pane_EnforceMessageRecovery) {
					if (((LGACheckbox *)
					FindPaneByID(checkbox_UseInADK))->IsSelected()) {
				  		FindPaneByID(checkbox_EnforceIncomingCADK)->
				  			Enable();
				  	} else {
				  		FindPaneByID(checkbox_EnforceIncomingCADK)->
				  			Disable();
				  	}
					if (((LGACheckbox *)
					FindPaneByID(checkbox_UseOutADK))->IsSelected()) {
				  		FindPaneByID(checkbox_EnforceOutgoingCADK)->
				  			Enable();
				  	} else {
				  		FindPaneByID(checkbox_EnforceOutgoingCADK)->
				  			Disable();
				  	}
				}

				if (((mCurrentPane == pane_InADK)
				  && (! ((LGACheckbox *) FindPaneByID(checkbox_UseInADK))->
				  	IsSelected()))
				|| ((mCurrentPane == pane_OutADK)
				  && (! ((LGACheckbox *) FindPaneByID(checkbox_UseOutADK))->
				  	IsSelected()))
				|| ((mCurrentPane == pane_CorporateKey)
				  && (! ((LGACheckbox *)
				  FindPaneByID(checkbox_SignCorporateKey))->IsSelected()))) {
					mCurrentPane--;
				}
				
				// Update summary pane
				if (mCurrentPane == pane_Summary) {
					UpdateSummaryPane();
				}

				if (mCurrentPane == pane_SiteLicenseNumber) {
					((LGAEditField *) FindPaneByID(editfield_SiteLicenseNumber))->
							SelectAll();
					SwitchTarget((LGAEditField *) FindPaneByID(
							editfield_SiteLicenseNumber));
				}
					
				// Show the new pane
				FindPaneByID(mCurrentPane)->Show();

				// Adjust the buttons
				LPane *	buttonNext = FindPaneByID(button_Next);
				
				if (mCurrentPane != pane_SaveInstaller) {
					buttonNext->SetDescriptor(CString(	STRx_AdminStrings,
														kNextID));
				}
				buttonNext->Enable();
				if (mCurrentPane == pane_First) {
					FindPaneByID(button_Previous)->Disable();
				}
				SetDefaultButton(button_Next);
			}
		}
		break;
		
		
		case msg_EnforceMinimumCharacters:
		{
			LGACheckbox *	theCheckbox = (LGACheckbox *)
							FindPaneByID(checkbox_EnforceMinimumCharacters);
			LGAEditField *	theEditField = (LGAEditField *)
							FindPaneByID(editfield_EnforceMinimumCharacters);
									
			if (theCheckbox->IsSelected()) {
				theEditField->Enable();
			} else {
				theEditField->Disable();
			}
		}
		break;
		
		
		case msg_EnforceMinimumQuality:
		{
			LGACheckbox *	theCheckbox = (LGACheckbox *)
								FindPaneByID(checkbox_EnforceMinimumQuality);
			LGAEditField *	theEditField = (LGAEditField *)
								FindPaneByID(editfield_EnforceMinimumQuality);
								
			if (theCheckbox->IsSelected()) {
				theEditField->Enable();
			} else {
				theEditField->Disable();
			}
		}
		break;
		
		
		case msg_DHADKTable:
		case msg_OutADKTable:
		case msg_CorporateKeyTable:
		{
			CKeyTable *	theTable;
			
			switch (inMessage) {
				case msg_DHADKTable:
				{
					theTable = (CKeyTable *) FindPaneByID(keytable_InADK);
				}
				break;
				
				
				case msg_OutADKTable:
				{
					theTable = (CKeyTable *) FindPaneByID(keytable_OutADK);
				}
				break;


				case msg_CorporateKeyTable:
				{
					theTable = (CKeyTable *)
						FindPaneByID(keytable_CorporateKey);
				}
				break;
			}
			if (theTable->GetFirstSelectedCell().IsNullCell()) {
				FindPaneByID(button_Next)->Disable();
			} else {
				FindPaneByID(button_Next)->Enable();
			}
		}
		break;
		
		
		case msg_AllowKeyGeneration:
		{
			LGACheckbox *	theCheckbox = (LGACheckbox *) FindPaneByID(
									checkbox_AllowKeyGeneration);
									
			if (theCheckbox->IsSelected()) {
				FindPaneByID(checkbox_AllowRSAKeyGeneration)->Enable();
				FindPaneByID(editfield_MinimumKeySize)->Enable();
			} else {
				FindPaneByID(checkbox_AllowRSAKeyGeneration)->Disable();
				FindPaneByID(editfield_MinimumKeySize)->Disable();
			}
		}
		break;
		
		case msg_SignCorporateKey:
		{
			if (((LGACheckbox *)
					FindPaneByID(checkbox_SignCorporateKey))->IsSelected()) {
				FindPaneByID(checkbox_WarnNotSignedCorporateKey)->Enable();
				FindPaneByID(checkbox_MetaIntroduceCorporateKey)->Enable();
			} else {
				FindPaneByID(checkbox_WarnNotSignedCorporateKey)->Disable();
				FindPaneByID(checkbox_MetaIntroduceCorporateKey)->Disable();
			}
		}
		break;
		
		case msg_ChooseBaseInstaller:
		{
			StandardFileReply	reply;
			UniversalProcPtr	fileFilterUPP;
			OSStatus			err;
		
			// Find the installer
			if (FindApplication(kPGPInstallerType, kPGPInstallerCreator,
				kFindAppOnAllVolumes, &mBaseInstallerSpec)) {
				::LMSetSFSaveDisk(-mBaseInstallerSpec.vRefNum);
				::LMSetCurDirStore(mBaseInstallerSpec.parID);
			}
			
			fileFilterUPP = NewFileFilterProc(InstallerFileFilter);
			::StandardGetFile(	fileFilterUPP,
								1,
								&kPGPInstallerType,
								&reply);
			DisposeRoutineDescriptor(fileFilterUPP);
			if (reply.sfGood) {
				CString			descriptor;
				LPane *			baseInstallerCaption =
									FindPaneByID(caption_BaseInstaller);
				
				mBaseInstallerSpec = reply.sfFile;
				err = FSpGetFullPath(&mBaseInstallerSpec, descriptor);
				PGPThrowIfOSErr_(err);
				baseInstallerCaption->SetDescriptor(descriptor);
				FindPaneByID(button_Next)->Enable();
			}
		}
		break;
	}
}

	Boolean
CAdminDialog::AttemptQuitSelf(
	SInt32	inSaveOption)
{
	if (mDirty) {
		::SysBeep(0);
	}
	return ! mDirty;
}



	Boolean
CAdminDialog::VerifyEditFieldValues()
{
	Boolean			result = true;
	LGAEditField *	theEditField;
	SInt16			theValue;
	SInt16			stringID = 0;
	SInt16			minimum;
	SInt16			maximum;
	
	// Test the fields
	switch (mCurrentPane) {
		case pane_PassphraseMinimums:
		{
			theEditField = (LGAEditField *) FindPaneByID(
								editfield_EnforceMinimumCharacters);
			theValue = theEditField->GetValue();
			if (theEditField->IsEnabled() &&
			((theValue < kMinimumCharacters) ||
			(theValue > kMaximumCharacters))) {
				result = false;
				stringID = kMinimumCharactersID;
				minimum = kMinimumCharacters;
				maximum = kMaximumCharacters;
			} else {
				theEditField = (LGAEditField *) FindPaneByID(
									editfield_EnforceMinimumQuality);
				theValue = theEditField->GetValue();
				if (theEditField->IsEnabled() &&
				((theValue < kMinimumQuality) ||
				(theValue > kMaximumQuality))) {
					result = false;
					stringID = kMinimumQualityID;
					minimum = kMinimumQuality;
					maximum = kMaximumQuality;
				}
			}
		} 
		break;
		
		
		case pane_KeyGeneration:
		{
			Boolean	haveRSAGeneration = ((LGACheckbox *)
					FindPaneByID(checkbox_AllowRSAKeyGeneration))->
					IsSelected();
					
			SInt16 maxKeySize = haveRSAGeneration ? kMaximumRSAKeySize :
									kMaximumKeySize;
			
			theEditField = (LGAEditField *) FindPaneByID(
								editfield_MinimumKeySize);
			theValue = theEditField->GetValue();
										
			if (theEditField->IsEnabled() &&
			((theValue < kMinimumKeySize) ||
			(theValue > maxKeySize)))
			{
				result = false;
				
				if( haveRSAGeneration )
				{
					stringID = kMinimumRSAKeySizeID;
				}
				else
				{
					stringID = kMinimumKeySizeID;
				}
				
				minimum = kMinimumKeySize;
				maximum = maxKeySize;
			}
		}
		break;
	}
	
	
	// Handle bad compare
	if (result == false ) {
		::SysBeep(0);
		CWarningAlert::Display(	kWAStopAlertType,
								kWAOKStyle,
								CString(STRx_AdminStrings,
										stringID),
								CString(minimum),
								CString(maximum));
		SwitchTarget(theEditField);
		theEditField->SelectAll();
	}
		
	return result;
}

	Boolean
CAdminDialog::Save()
{
	Boolean				result = false;
	StandardFileReply	reply;
	OSStatus			err;
	
	// Choose where to save the copy
	::StandardPutFile(	CString(STRx_AdminStrings, kSavePromptID),
						mBaseInstallerSpec.name,
						&reply);
	if (reply.sfGood) {
	
		// Create a copy of the file
		if (! FSpEqual(&mBaseInstallerSpec, &reply.sfFile)) {
			if (reply.sfReplacing) {
				err = ::FSpDelete(&reply.sfFile);
				PGPThrowIfOSErr_(err);
			}
			err = FSpCopyFiles(&mBaseInstallerSpec, &reply.sfFile);
			PGPThrowIfOSErr_(err);
		}
		
		// Open temp pref file
		CTempFile			tempFile(kPGPKeysCreator, kPrefFileType);
		PGPPrefRef			prefRef;
		PGPError			pgpErr;
		PFLContextRef		pflContext;
		PFLFileSpecRef		tempSpecRef;
		FSSpec				tempSpec;
		StHandleBlock		handle(0);
		StHandleBlock		keyHandle(0);
		CString				editfieldString;
		Boolean				isSelected;
		PGPKeyID			keyID;
		SInt32				algorithm;
		PGPByte				exportedID[ kPGPMaxExportedKeyIDSize ];
		PGPSize				size;
		CKeyTable *			keyTable;
		PFLFileSpecRef		prefFileSpec;
		
		pgpErr = PFLNewContext(&pflContext);
		PGPThrowIfPGPErr_(pgpErr);
		tempFile.GetSpecifier(tempSpec);
		pgpErr = PFLNewFileSpecFromFSSpec(	pflContext,
											&tempSpec,
											&tempSpecRef);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPOpenPrefFile(	tempSpecRef, 
						  			&prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		
		// Write prefs to file
		FindPaneByID(editfield_SiteLicenseNumber)->GetDescriptor(editfieldString);
		if (editfieldString.GetLength() != 0) {
			pgpErr = PGPSetPrefString(	prefRef,
										kPGPPrefSiteLicenseNumber,
										editfieldString);
			PGPThrowIfPGPErr_(pgpErr);
		}
		
		isSelected = ((LGACheckbox *) FindPaneByID(checkbox_UseInADK))->
						IsSelected();
		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefUseDHADK,
					isSelected);
		PGPThrowIfPGPErr_(pgpErr);
		if (isSelected) {
			try {
				keyTable = (CKeyTable *) FindPaneByID(keytable_InADK);
				keyTable->GetSelectedKeys(keyHandle);
				::HandAndHand(keyHandle, handle);
				PGPThrowIfMemError_();
				keyTable->GetFirstSelectedKeyID(&keyID);
				pgpErr = PGPExportKeyID(	&keyID,
											exportedID,
											&size);
				PGPThrowIfPGPErr_(pgpErr);
				pgpErr = PGPSetPrefData(	prefRef, 
											kPGPPrefDHADKID, 
								 			size, 
											exportedID);
				PGPThrowIfPGPErr_(pgpErr);
			}
			
			catch (...) {
				throw;
			}
		}

		isSelected = ((LGACheckbox *) FindPaneByID(checkbox_UseOutADK))->
						IsSelected();
		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefUseOutgoingADK,
					isSelected);
		PGPThrowIfPGPErr_(pgpErr);
		if (isSelected) {
			try {
				keyTable = (CKeyTable *) FindPaneByID(keytable_OutADK);
				keyTable->GetSelectedKeys(keyHandle);
				::HandAndHand(keyHandle, handle);
				PGPThrowIfMemError_();
				keyTable->GetFirstSelectedKeyID(&keyID,
												&algorithm);
				pgpErr = PGPExportKeyID(	&keyID,
											exportedID,
											&size);
				PGPThrowIfPGPErr_(pgpErr);
				pgpErr = PGPSetPrefData(	prefRef, 
											kPGPPrefOutgoingADKID, 
								 			size, 
											exportedID);
				PGPThrowIfPGPErr_(pgpErr);
				pgpErr = PGPSetPrefNumber(	prefRef,
											kPGPPrefOutADKPublicKeyAlgorithm,
											algorithm);
				PGPThrowIfPGPErr_(pgpErr);
			}
			
			catch (...) {
				throw;
			}
		}
		
		pgpErr = PGPSetPrefBoolean(	prefRef, kPGPPrefEnforceIncomingADK,
								((LGACheckbox *)
								FindPaneByID(
								checkbox_EnforceIncomingCADK))->
								IsSelected());
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPSetPrefBoolean(	prefRef, kPGPPrefEnforceOutgoingADK,
								((LGACheckbox *)
								FindPaneByID(
								checkbox_EnforceOutgoingCADK))->
								IsSelected());
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPSetPrefBoolean(	prefRef, kPGPPrefEnforceRemoteADKClass,
								((LGACheckbox *)
								FindPaneByID(
								checkbox_EnforceRemoteCADK))->
								IsSelected());
		PGPThrowIfPGPErr_(pgpErr);
		
		isSelected = ((LGACheckbox *)
			FindPaneByID(checkbox_EnforceMinimumCharacters))->IsSelected();
		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefEnforceMinChars,
					isSelected);
		PGPThrowIfPGPErr_(pgpErr);
		if (isSelected) {
			pgpErr = PGPSetPrefNumber(	prefRef,
						kPGPPrefMinChars,
						FindPaneByID(editfield_EnforceMinimumCharacters)->
						GetValue());
			PGPThrowIfPGPErr_(pgpErr);
		}

		isSelected = ((LGACheckbox *)
			FindPaneByID(checkbox_EnforceMinimumQuality))->IsSelected();
		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefEnforceMinQuality,
					isSelected);
		PGPThrowIfPGPErr_(pgpErr);
		if (isSelected) {
			pgpErr = PGPSetPrefNumber(	prefRef,
						kPGPPrefMinQuality,
						FindPaneByID(editfield_EnforceMinimumQuality)->
						GetValue());
			PGPThrowIfPGPErr_(pgpErr);
		}

		isSelected = ((LGACheckbox *)
					FindPaneByID(checkbox_SignCorporateKey))->IsSelected();
		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefAutoSignTrustCorp,
					isSelected);
		PGPThrowIfPGPErr_(pgpErr);

		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefWarnNotCertByCorp, isSelected &&
					((LGACheckbox *)
					FindPaneByID(checkbox_WarnNotSignedCorporateKey))->
					IsSelected());
		PGPThrowIfPGPErr_(pgpErr);

		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefMetaIntroducerCorp, isSelected &&
					((LGACheckbox *)
					FindPaneByID(checkbox_MetaIntroduceCorporateKey))->
					IsSelected());
		PGPThrowIfPGPErr_(pgpErr);

				
		keyTable = (CKeyTable *) FindPaneByID(keytable_CorporateKey);
		keyTable->GetSelectedKeys(keyHandle);
		if (::GetHandleSize(keyHandle) != 0) {
			try {
				::HandAndHand(keyHandle, handle);
				PGPThrowIfMemError_();
				keyTable->GetFirstSelectedKeyID(&keyID,
												&algorithm);
				pgpErr = PGPExportKeyID(	&keyID,
											exportedID,
											&size);
				PGPThrowIfPGPErr_(pgpErr);
				pgpErr = PGPSetPrefData(	prefRef, 
											kPGPPrefCorpKeyID, 
								 			size, 
											exportedID);
				PGPThrowIfPGPErr_(pgpErr);
				pgpErr = PGPSetPrefNumber(	prefRef,
											kPGPPrefCorpKeyPublicKeyAlgorithm,
											algorithm);
				PGPThrowIfPGPErr_(pgpErr);
			}
			
			catch (...) {
				throw;
			}
		}

		isSelected = ((LGACheckbox *)
			FindPaneByID(checkbox_AllowKeyGeneration))->IsSelected();
		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefAllowKeyGen,
					isSelected);
		PGPThrowIfPGPErr_(pgpErr);

		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefAllowRSAKeyGen, isSelected &&
					((LGACheckbox *)
					FindPaneByID(checkbox_AllowRSAKeyGeneration))->
					IsSelected());
		PGPThrowIfPGPErr_(pgpErr);
		if (isSelected) {
			pgpErr = PGPSetPrefNumber(	prefRef,
										kPGPPrefMinimumKeySize,
										FindPaneByID(
											editfield_MinimumKeySize)->
												GetValue());
			PGPThrowIfPGPErr_(pgpErr);
		}

		((CKeyTable *) FindPaneByID(keytable_DefaultPublicKeys))->
			GetSelectedKeys(keyHandle);
		::HandAndHand(handle, keyHandle);
		PGPThrowIfMemError_();
		if (::GetHandleSize(keyHandle) != 0) {
			::HLock(keyHandle);
			pgpErr = PGPSetPrefData(	prefRef, 
										kPGPPrefDefaultKeys, 
							 			::GetHandleSize(keyHandle), 
							 			*keyHandle);
			::HUnlock(keyHandle);
			PGPThrowIfPGPErr_(pgpErr);
		}

		pgpErr = PGPSetPrefBoolean(	prefRef,
					kPGPPrefAllowConventionalEncryption,
					((LGACheckbox *) FindPaneByID(
					checkbox_AllowConventionalEncryption))->IsSelected());
		PGPThrowIfPGPErr_(pgpErr);
		
		FindPaneByID(editfield_CommentText)->GetDescriptor(editfieldString);
		if (editfieldString.GetLength() != 0) {
			pgpErr = PGPSetPrefString(	prefRef,
										kPGPPrefComments,
										editfieldString);
			PGPThrowIfPGPErr_(pgpErr);
		}

		// Save the admin prefs file and copy it to the resource fork
		// of the installer
		pgpErr = PGPSavePrefFile(prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPClosePrefFile(prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		err = CopyFileToResource(	&tempSpec,
									&reply.sfFile,
									kAdminResType,
									ADMN_AdminPrefs);
		PGPThrowIfOSErr_(err);
		
		// Copy admin prefs to current machine's admin prefs
		FSSpec	adminPrefs;
		
		pgpErr = PGPOpenAdminPrefs(	pflContext, &prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPGetPrefFileSpec(	prefRef,
										&prefFileSpec);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPClosePrefFile(prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PFLGetFSSpecFromFileSpec(	prefFileSpec,
											&adminPrefs);
		PGPThrowIfPGPErr_(pgpErr);
		err = FSpCopyFiles(&tempSpec, &adminPrefs);
		PGPThrowIfOSErr_(err);							
		
		// Copy the current client prefs to the resource fork of the
		// installer
		if (((LGACheckbox *) FindPaneByID(checkbox_CopyClientPrefs))->
				IsSelected()) {
			FSSpec			clientPrefs;
			
			pgpErr = PGPOpenClientPrefs(	pflContext,
											&prefRef);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPGetPrefFileSpec(	prefRef,
											&prefFileSpec);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPClosePrefFile(prefRef);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PFLGetFSSpecFromFileSpec(	prefFileSpec,
												&clientPrefs);
			err = FSpCopyFiles(&clientPrefs, &tempSpec);
			PGPThrowIfOSErr_(err);
			pgpErr = PGPOpenPrefFile(	tempSpecRef, 
							  			&prefRef);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPSetPrefNumber(	prefRef,
										kPGPPrefDateOfLastSplashScreen,
										0);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPSetPrefString(	prefRef,
										kPGPPrefOwnerName,
										"");
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPSetPrefString(	prefRef,
										kPGPPrefCompanyName,
										"");
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPSetPrefString(	prefRef,
										kPGPPrefLicenseNumber,
										"");
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPSetPrefBoolean(	prefRef,
										kPGPPrefFirstKeyGenerated,
										false);
			PGPThrowIfPGPErr_(pgpErr);
			PGPRemovePref(	prefRef, 
							kPGPPrefPGPtoolsMacPrivateData);
			PGPRemovePref(	prefRef, 
							kPGPPrefPGPkeysMacMainWinPos);
			PGPRemovePref(	prefRef, 
							kPGPPrefPGPkeysMacColumnData);
			pgpErr = PGPSavePrefFile(prefRef);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPClosePrefFile(prefRef);
			PGPThrowIfPGPErr_(pgpErr);
			err = CopyFileToResource(	&tempSpec,
										&reply.sfFile,
										kAdminResType,
										ADMN_ClientPrefs);
			PGPThrowIfOSErr_(err);
		}
		// Hide the current pane
		FindPaneByID(mCurrentPane++)->Hide();
		FindPaneByID(mCurrentPane)->Show();
		FindPaneByID(button_Next)->Disable();
		SetDefaultButton(button_Quit);
		
		result = true;
	}
	
	return result;		
}

	OSStatus
CAdminDialog::CopyFileToResource(
	FSSpec *	inFileSpec,
	FSSpec *	inResourceFileSpec,
	ResType		inType,
	ResIDT		inID)
{
	OSStatus	result;
	SInt16		resourceForkRefNum = -1;
	SInt16		fileRefNum = -1;
	
	try {
		StHandleBlock	handle(0);
	
		// Open the files
		::SetResLoad(false);
		resourceForkRefNum = ::FSpOpenResFile(inResourceFileSpec, fsRdWrPerm);
		::SetResLoad(true);
		PGPThrowIfResError_();
		result = ::FSpOpenDF(inFileSpec, fsRdPerm, &fileRefNum);
		PGPThrowIfOSErr_(result);
		
		// Add the resource if it doesn't already exist
		if (! ResourceExists(inType, inID, true)) {
			::AddResource(	handle,
							inType,
							inID,
							"\p");
			PGPThrowIfResError_();
			::UpdateResFile(resourceForkRefNum);
			PGPThrowIfResError_();
			::DetachResource(handle);
			PGPThrowIfResError_();
		}
		::SetHandleSize(handle, 1024);
		::SetResLoad(false);
		
		Handle			theResource = ::GetResource(inType, inID);
		StHandleLocker	theLock(handle);
		SInt32			count = 1024;
		SInt32			offset = 0;
		SInt32			dataLength;
		
		::SetResLoad(true);
		result = ::GetEOF(fileRefNum, &dataLength);
		PGPThrowIfOSErr_(result);
		::SetResourceSize(theResource, dataLength);
		PGPThrowIfResError_();
		do {
			result = ::FSRead(fileRefNum, &count, *handle);
			
			if ((result == noErr) || (result == eofErr)) {
				::WritePartialResource(	theResource,
										offset,
										*handle,
										count);
				PGPThrowIfResError_();
				offset += count;
			} else {
				PGPThrowOSErr_(result);
			}
		} while (result == noErr);
		::UpdateResFile(resourceForkRefNum);
		PGPThrowIfResError_();
		result = noErr;
	}
	
	catch (CComboError & exception) {
		result = exception.err;
	}
	
	catch (...) {
		result = fnfErr;
	}
	
	if (resourceForkRefNum != -1) {
		::CloseResFile(resourceForkRefNum);
		::FlushVol(nil, inResourceFileSpec->vRefNum);
	}
	if (fileRefNum != -1) {
		::FSClose(fileRefNum);
	}
	
	return result;
}

	pascal Boolean
CAdminDialog::InstallerFileFilter(
	CInfoPBPtr	inPB,
	Ptr			inDataP)
{
	Boolean	result = true;
	
	if (cpbFileCreator(inPB) == kPGPInstallerCreator) {
		result = false;
	}
	
	return result;
}



	void
CAdminDialog::UpdateSummaryPane()
{
	CString	noneString(STRx_AdminStrings, kNoneID);
	CString	dataString;
	
	if (((LGACheckbox *) FindPaneByID(checkbox_UseInADK))->IsSelected()) {
		((LGACaption *) FindPaneByID(caption_SummaryInCADK))->
			SetTextTraitsID(Txtr_AdminWizardBold);
		((LGACaption *) FindPaneByID(caption_SummaryInCADKID))->
			SetTextTraitsID(Txtr_AdminWizard);
		((CKeyTable *) FindPaneByID(keytable_InADK))->
			GetFirstSelectedUserID(dataString);
		((LGACaption *) FindPaneByID(caption_SummaryInCADKID))->
			SetDescriptor(dataString);
	} else {
		((LGACaption *) FindPaneByID(caption_SummaryInCADK))->
			SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryInCADKID))->
			SetTextTraitsID(Txtr_AdminWizardDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryInCADKID))->
			SetDescriptor(noneString);
	}

	if (((LGACheckbox *) FindPaneByID(checkbox_UseOutADK))->IsSelected()) {
		((LGACaption *) FindPaneByID(caption_SummaryOutCADK))->
			SetTextTraitsID(Txtr_AdminWizardBold);
		((LGACaption *) FindPaneByID(caption_SummaryOutCADKID))->
			SetTextTraitsID(Txtr_AdminWizard);
		((CKeyTable *) FindPaneByID(keytable_OutADK))->
			GetFirstSelectedUserID(dataString);
		((LGACaption *) FindPaneByID(caption_SummaryOutCADKID))->
			SetDescriptor(dataString);
	} else {
		((LGACaption *) FindPaneByID(caption_SummaryOutCADK))->
			SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryOutCADKID))->
			SetTextTraitsID(Txtr_AdminWizardDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryOutCADKID))->
			SetDescriptor(noneString);
	}

	if (((LGACheckbox *) FindPaneByID(checkbox_EnforceIncomingCADK))->IsSelected()
	&& ((LGACheckbox *) FindPaneByID(checkbox_UseInADK))->IsSelected()) {
		((LGACaption *) FindPaneByID(caption_SummaryEnforceIncomingCADK))->
			SetTextTraitsID(Txtr_AdminWizardBold);
	} else {
		((LGACaption *) FindPaneByID(caption_SummaryEnforceIncomingCADK))->
			SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
	}
	
	if (((LGACheckbox *) FindPaneByID(checkbox_EnforceOutgoingCADK))->IsSelected()
	&& ((LGACheckbox *) FindPaneByID(checkbox_UseOutADK))->IsSelected()) {
		((LGACaption *) FindPaneByID(caption_SummaryEnforceOutgoingCADK))->
			SetTextTraitsID(Txtr_AdminWizardBold);
	} else {
		((LGACaption *) FindPaneByID(caption_SummaryEnforceOutgoingCADK))->
			SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
	}

	if (((LGACheckbox *) FindPaneByID(checkbox_EnforceRemoteCADK))->IsSelected()) {
		((LGACaption *) FindPaneByID(caption_SummaryEnforceRemoteCADK))->
			SetTextTraitsID(Txtr_AdminWizardBold);
	} else {
		((LGACaption *) FindPaneByID(caption_SummaryEnforceRemoteCADK))->
			SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
	}
	

	if (((LGACheckbox *) FindPaneByID(checkbox_SignCorporateKey))->
	IsSelected()) {
		((LGACaption *) FindPaneByID(caption_SummaryCorporateKey))->
			SetTextTraitsID(Txtr_AdminWizardBold);
		((LGACaption *) FindPaneByID(caption_SummaryCorporateKeyID))->
			SetTextTraitsID(Txtr_AdminWizard);
		((CKeyTable *) FindPaneByID(keytable_CorporateKey))->
			GetFirstSelectedUserID(dataString);
		((LGACaption *) FindPaneByID(caption_SummaryCorporateKeyID))->
			SetDescriptor(dataString);
		if (((LGACheckbox *)
		FindPaneByID(checkbox_WarnNotSignedCorporateKey))->IsSelected()) {
			((LGACaption *)
				FindPaneByID(caption_SummaryWarnNotSignedCorporateKey))->
					SetTextTraitsID(Txtr_AdminWizardBoldRight);
		} else {
			((LGACaption *)
				FindPaneByID(caption_SummaryWarnNotSignedCorporateKey))->
					SetTextTraitsID(Txtr_AdminWizardBoldRightDisabled);
		}
		if (((LGACheckbox *)
		FindPaneByID(checkbox_MetaIntroduceCorporateKey))->IsSelected()) {
			((LGACaption *)
				FindPaneByID(caption_SummaryMetaIntroduceCorporateKey))->
					SetTextTraitsID(Txtr_AdminWizardBold);
		} else {
			((LGACaption *)
				FindPaneByID(caption_SummaryMetaIntroduceCorporateKey))->
					SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
		}
	} else {
		((LGACaption *) FindPaneByID(caption_SummaryCorporateKey))->
			SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryCorporateKeyID))->
			SetTextTraitsID(Txtr_AdminWizardDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryCorporateKeyID))->
			SetDescriptor(noneString);
		((LGACaption *)
			FindPaneByID(caption_SummaryWarnNotSignedCorporateKey))->
				SetTextTraitsID(Txtr_AdminWizardBoldRightDisabled);
		((LGACaption *)
			FindPaneByID(caption_SummaryMetaIntroduceCorporateKey))->
				SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
	}

	if (((LGACheckbox *) FindPaneByID(checkbox_EnforceMinimumCharacters))->
	IsSelected()) {
		((LGACaption *) FindPaneByID(
			caption_SummaryEnforceMinimumCharacters))->
				SetTextTraitsID(Txtr_AdminWizardBold);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumCharacters))->
			SetTextTraitsID(Txtr_AdminWizard);
		((CKeyTable *) FindPaneByID(editfield_EnforceMinimumCharacters))->
			GetDescriptor(dataString);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumCharacters))->
			SetDescriptor(dataString);
	} else {
		((LGACaption *) FindPaneByID(
			caption_SummaryEnforceMinimumCharacters))->
				SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumCharacters))->
			SetTextTraitsID(Txtr_AdminWizardDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumCharacters))->
			SetDescriptor(noneString);
	}

	if (((LGACheckbox *) FindPaneByID(checkbox_EnforceMinimumQuality))->
	IsSelected()) {
		((LGACaption *) FindPaneByID(
			caption_SummaryEnforceMinimumQuality))->
				SetTextTraitsID(Txtr_AdminWizardBoldRight);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumQuality))->
			SetTextTraitsID(Txtr_AdminWizard);
		((CKeyTable *) FindPaneByID(editfield_EnforceMinimumQuality))->
			GetDescriptor(dataString);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumQuality))->
			SetDescriptor(dataString);
	} else {
		((LGACaption *) FindPaneByID(
			caption_SummaryEnforceMinimumQuality))->
				SetTextTraitsID(Txtr_AdminWizardBoldRightDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumQuality))->
			SetTextTraitsID(Txtr_AdminWizardDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumQuality))->
			SetDescriptor(noneString);
	}

	if (((LGACheckbox *) FindPaneByID(checkbox_AllowKeyGeneration))->
	IsSelected()) {
		((LGACaption *) FindPaneByID(caption_SummaryAllowKeyGeneration))->
			SetTextTraitsID(Txtr_AdminWizardBold);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumKeySize))->
			SetTextTraitsID(Txtr_AdminWizardBold);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumKeySizeValue))->
			SetTextTraitsID(Txtr_AdminWizard);
		((CKeyTable *) FindPaneByID(editfield_MinimumKeySize))->
			GetDescriptor(dataString);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumKeySizeValue))->
			SetDescriptor(dataString);
		if (((LGACheckbox *)
		FindPaneByID(checkbox_AllowRSAKeyGeneration))->IsSelected()) {
			((LGACaption *)
				FindPaneByID(caption_SummaryAllowRSAKeyGeneration))->
					SetTextTraitsID(Txtr_AdminWizardBold);
		} else {
			((LGACaption *)
				FindPaneByID(caption_SummaryAllowRSAKeyGeneration))->
					SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
		}
	} else {
		((LGACaption *) FindPaneByID(caption_SummaryAllowKeyGeneration))->
			SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumKeySize))->
			SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumKeySizeValue))->
			SetTextTraitsID(Txtr_AdminWizardDisabled);
		((LGACaption *) FindPaneByID(caption_SummaryMinimumKeySizeValue))->
			SetDescriptor(noneString);
		((LGACaption *)
			FindPaneByID(caption_SummaryAllowRSAKeyGeneration))->
				SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
	}

	if (((LGACheckbox *) FindPaneByID(checkbox_AllowConventionalEncryption))->
	IsSelected()) {
		((LGACaption *)
			FindPaneByID(caption_SummaryAllowConventionalEncryption))->
				SetTextTraitsID(Txtr_AdminWizardBold);
	} else {
		((LGACaption *)
			FindPaneByID(caption_SummaryAllowConventionalEncryption))->
				SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
	}

	FindPaneByID(editfield_CommentText)->GetDescriptor(dataString);
	if (dataString.GetLength() != 0) {
		((LGACaption *) FindPaneByID(caption_SummaryMessageHeaderComment))->
			SetTextTraitsID(Txtr_AdminWizardBold);
		((LGACaption *)
			FindPaneByID(caption_SummaryMessageHeaderCommentText))->
				SetTextTraitsID(Txtr_AdminWizard);
		((LGACaption *)
			FindPaneByID(caption_SummaryMessageHeaderCommentText))->
				SetDescriptor(dataString);
	} else {
		((LGACaption *) FindPaneByID(caption_SummaryMessageHeaderComment))->
			SetTextTraitsID(Txtr_AdminWizardBoldDisabled);
		((LGACaption *)
			FindPaneByID(caption_SummaryMessageHeaderCommentText))->
				SetTextTraitsID(Txtr_AdminWizardDisabled);
		((LGACaption *)
			FindPaneByID(caption_SummaryMessageHeaderCommentText))->
				SetDescriptor(noneString);
	}
	
	PGPKeySetRef	defaultPublicKeys = kPGPInvalidRef;

	try {		
		((CKeyTable *) FindPaneByID(keytable_DefaultPublicKeys))->
			GetSelectedKeys(&defaultPublicKeys);
		((CKeyTable *) FindPaneByID(keytable_Summary))->
			SetTableInfo(defaultPublicKeys);
		PGPFreeKeySet(defaultPublicKeys);
	}
	
	catch (...) {
		if (defaultPublicKeys != kPGPInvalidRef) {
			PGPFreeKeySet(defaultPublicKeys);
		}
		throw;
	}
}

