/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPKeys.cp,v 1.57.4.4.4.1 1997/12/05 22:14:07 mhw Exp $
____________________________________________________________________________*/
#include "CPGPKeys.h"
#include "BETA.h"
#include "PGPDemo.h"
#include "CWarningAlert.h"
#include "MacDebugPatches.h"
#include "PGPKeysMenuBar.h"
#include "CKeyWindow.h"
#include "CSearchWindow.h"
#include "CSearchPanel.h"
#include "CGroupsWindow.h"
#include "CGADurationEditField.h"
#include "CKeyView.h"
#include "CKeyInfoWindow.h"
#include "CFingerprintField.h"
#include "CAGASlider.h"
#include "CRectThermometer.h"
#include "CKeyTableLabels.h"
#include "CRecessedCaption.h"
#include "LGrayBoxView.h"
#include "CKeyTable.h"
#include "CGAScroller.h"
#include "CPicture.h"
#include "CPopupList.h"
#include "CKeyserverTable.h"
#include "CPassphraseEdit.h"
#include "CPGPPreferenceDialog.h"
#include "CChangePassphraseDialog.h"
#include "CSignDialog.h"
#include "CVersionCaption.h"
#include "CPGPStDialogHandler.h"
#include "CInfinityBar.h"
#include "CKeyGenWizardDialog.h"
#include "CGAProgressDialog.h"
#include "CProgressBar.h"
#include "CWhiteList.h"
#include "BackupHandler.h"
#include "pgpRandomPool.h"
#include "MacErrors.h"
#include "MacInternet.h"
#include "MacStrings.h"
#include "MacFiles.h"
#include "PGPOpenPrefs.h"
#include "pgpMacCustomContextAlloc.h"

// PGPsdk includes
#include "pgpUtilities.h"
#include "pgpErrors.h"
#include "pgpRandomPool.h"
#include "pgpMem.h"
#include "pgpSDKPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pgpRandomPool.h"
#include "pgpKeyServer.h"
#include "pgpUserInterface.h"

#include <string.h>

ResIDT				gApplicationResFile;
PGPContextRef		gPGPContext;
PFLContextRef		gPFLContext;
PGPPrefRef			gPrefRef;
PGPPrefRef			gAdminPrefRef;
Boolean				gServerCallsPresent;
FSSpec				gAppFSSpec;

const ResIDT		kKeyWindow							= 129;
const ResIDT		dialog_Preferences					= 142;
const ResIDT		kWindowAbout						= 131;
const ResIDT		kNoKeyFilesDialog					= 147;
const ResIDT		kRandomDataDialog					= 156;
const ResIDT		kReadOnlyErrorDialog				= 157;
const ResIDT		kAboutMainPictureResID				= 129;
const ResIDT		kAboutCreditsPictureResID			= 130;
const short			kSplashDisplayTime					= 30;
const ulong			kSecondsInADay						= 86400;
const long			ae_ImportKey						= 4000;
#if CANC_PGP_FREEWARE
const ResID			kWindowFreewareUpgrade				= 7000;
#endif


enum	{
			kStringListID			= 1017,
			kNoAdminPrefsStringID	= 1,
			kErrorPrefaceStringID,
			kAboutInfoButtonTitleStrIndex,
			kAboutCreditsButtonTitleStrIndex,
			kPGPWebSiteURLStrIndex,
			kCantLaunchWebBrowserBecauseStrIndex,
			kUnrecoverableKeyringErrorStrIndex,
			kUnrecoverablePrefsErrorStrIndex,
			kPGPRegisterURLStrIndex,
			kPGPRegisterOnlineItemStrIndex,
			kNoGroupsSupportStrIndex,
			kPGPUpgradeItemStrIndex,
			kPGPBuyNowURLStrIndex,
			kURLRegExtensionStrIndex
		};

static void RealMain(void)
{
	// GrowZone stuff disabled, not working,
	// possible PowerPlant bug?
	//LGrowZone* growZone = new LGrowZone(20000);

	// set these globals before we do anything else
	gApplicationResFile = ::CurResFile();

	CPGPKeys*	theApp = new CPGPKeys;
	theApp->Run();
	delete theApp;
	
	//delete growZone;
}

void main(void)
{
	ProcessInfoRec		processInfo;
	ProcessSerialNumber	currentProcess;
	
	SetDebugThrow_(debugAction_SourceDebugger);
	SetDebugSignal_(debugAction_SourceDebugger);

	InitializeHeap(3);
	UQDGlobals::InitializeToolbox(&qd);
	DebugPatches_PatchDisposeTraps();
	
	GetCurrentProcess(&currentProcess);
	processInfo.processAppSpec = &gAppFSSpec;
	processInfo.processInfoLength = sizeof(ProcessInfoRec);
	processInfo.processName = NULL;
	GetProcessInformation(&currentProcess, &processInfo);
	
	// make sure we do not leak the Comparator classes
	LComparator::GetComparator();
	LLongComparator::GetComparator();

	pgpLeaksBeginSession("main");
	//pgpLeaksSuspend();
		RealMain();
	
	pgpLeaksEndSession();
}

CPGPKeys	*CPGPKeys::sTheApp = 0;

CPGPKeys::CPGPKeys(void):
	mMenuBar(0),
	mModelDirector(0),
	mCreatedKey(false)
{
	sTheApp			= this;
	mKeyWindow		= NULL;
	mSearchWindow	= NULL;
	mGroupsWindow	= NULL;
	
	RegisterAllPPClasses();
	RegisterGALibraryClasses();
	
	RegisterClass_(LActiveScroller);
	RegisterClass_(LTableView);

	RegisterClass_(CKeyWindow);
	RegisterClass_(CKeyView);
	RegisterClass_(CSearchWindow);
	RegisterClass_(CSearchPanel);
	RegisterClass_(CGroupsWindow);
	RegisterClass_(CGroupsTable);
	RegisterClass_(CGroupTableLabels);
	RegisterClass_(CKeyTableLabels);
	RegisterClass_(CKeyTable);
	RegisterClass_(CGAScroller);
	RegisterClass_(CVersionCaption);
	RegisterClass_(CPicture);
	RegisterClass_(CGADurationEditField);

	RegisterClass_(CPGPPreferenceDialog);
	RegisterClass_(CPassphraseEdit);
	RegisterClass_(LIconPane);
	RegisterClass_(CRecessedCaption);

	RegisterClass_(CKeyGenWizardDialog);
	
	RegisterClass_(CKeyInfoWindow);
	RegisterClass_(CFingerprintField);
	RegisterClass_(CAGASlider);
	RegisterClass_(CRectThermometer);
	RegisterClass_(LGrayBoxView);
	RegisterClass_(CChangePassphraseDialog);
	RegisterClass_(CSignDialog);
	RegisterClass_(CKeyserverTable);
	
	RegisterClass_(CGAProgressDialog);
	RegisterClass_(CProgressBar);
	RegisterClass_(CInfinityBar);
	RegisterClass_(CWhiteList);
	RegisterClass_(CPopupList);
	
	mSplashScreen	= NULL;
	mCheckedLicense = FALSE;
	mLastMouseLoc.h	= mLastMouseLoc.v = -1;
	mSearching		= FALSE;
	mAppStarted		= FALSE;
}

	void
CPGPKeys::CleanUpPPLeaks()
{
	OSErr	theErr;
	
	pgpAssertAddrValid(&UAppleEventsMgr::sAnyType, AEDesc);
	theErr = ::AEDisposeDesc(&UAppleEventsMgr::sAnyType);
	pgpAssert(theErr == noErr);
	
	URegistrar::DisposeClassTable();
	UScreenPort::Dispose();
}

CPGPKeys::~CPGPKeys(void)
{
	StopIdling();
	if(IsntNull(mGroupsWindow))
		delete mGroupsWindow;
	mGroupsWindow = NULL;
	if(IsntNull(mSearchWindow))
		delete mSearchWindow;
	mSearchWindow = NULL;
	if(IsntNull(mKeyWindow))
		delete mKeyWindow;
	mKeyWindow = NULL;
	
#if	PGP_BUSINESS_SECURITY
	PGPClosePrefFile(gAdminPrefRef);
#endif
	PGPSavePrefFile(gPrefRef);
	PGPClosePrefFile(gPrefRef);
	PFLFreeContext(gPFLContext);
	PGPFreeContext(gPGPContext);
	
	pgpAssertAddrValid(mMenuBar, VoidAlign);
	if(IsntNull(mMenuBar))
		delete mMenuBar;
		
	pgpAssertAddrValid(mModelDirector, LModelDirector);
	if(IsntNull(mModelDirector))
		delete mModelDirector;
					
	CAGASlider::Purge();
	CleanUpPPLeaks();
}

	void
CPGPKeys::ResetKeyDB()
{
	PGPError	err;
	short		keyFilesErr;
	
	if(IsntNull(mGroupsWindow))
		delete mGroupsWindow;
	mDefaultKeysView->CloseKeys();
	// Out with the old
	
	// In with the new
	err = 0;
	err = PGPOpenDefaultKeyRings(gPGPContext,
			kPGPKeyRingOpenFlags_Mutable, &mKeySet);
	if(!PGPRefIsValid(mKeySet) && (err != kPGPError_FileLocked))
	{
		if(err == kPGPError_FilePermissions)
		{
			mWritableKeyring = FALSE;
			err = PGPOpenDefaultKeyRings(gPGPContext,
					(PGPKeyRingOpenFlags)0, &mKeySet);
		}
		if(!PGPRefIsValid(mKeySet) && ((err == kPGPError_CantOpenFile) ||
								(err == kPGPError_FileNotFound)))
		{
		tryOpenKeysAgain:
			if((keyFilesErr = AskMakeNewKeyFiles()) != 0)
			{
				if(keyFilesErr == 1)
					CPGPPreferenceDialog::ResetDefaultKeyringsPath();
				mWritableKeyring = TRUE;
				err = PGPOpenDefaultKeyRings(gPGPContext,
						(PGPKeyRingOpenFlags)
						(kPGPKeyRingOpenFlags_Mutable |
						kPGPKeyRingOpenFlags_Create), &mKeySet);
			}
			else
			{
				PGPFreeContext(gPGPContext);
				::ExitToShell();
			}
		}
	}
	if(!PGPRefIsValid(mKeySet) && IsErr(err))
	{
		// Fatal keyring error
		::SysBeep(1);
		CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
			kStringListID, kUnrecoverableKeyringErrorStrIndex);
		goto tryOpenKeysAgain;
	}
	// Reset the list
	mDefaultKeysView->SetKeyDBInfo(	mKeySet, mWritableKeyring, true);
}

	short
CPGPKeys::AskMakeNewKeyFiles()
{
	CPGPStDialogHandler	askKeyFilesDialog(kNoKeyFilesDialog, this);
	const CommandT		kQuitButton				= 'quit';
	const CommandT		kNewKeyFilesButton		= 'nuke';
	const CommandT		kSelectKeyFilesButton	= 'sele';
	MessageT			dialogMessage;
	
	do
	{
		dialogMessage = askKeyFilesDialog.DoDialog();
	} while(!dialogMessage);
	if(dialogMessage == kNewKeyFilesButton)
		return 1;
	else if(dialogMessage == kSelectKeyFilesButton)
	{
		FSSpec	publicSpec,
				privateSpec;
		
		if(CPGPPreferenceDialog::SelectKeyFile(&publicSpec, kPGPLTPublic))
			return 0;
		if(CPGPPreferenceDialog::SelectKeyFile(&privateSpec, kPGPLTPrivate))
			return 0;
		CPGPPreferenceDialog::SetLibSpecs(&publicSpec, &privateSpec, NULL);
		return 2;
	}
	else
		return 0;
}

	void
CPGPKeys::GetMinimumRandomData()
{
	if(!PGPGlobalRandomPoolHasMinimumEntropy())
	{
		CPGPStDialogHandler	randomDialog(kRandomDataDialog, this);
		PGPUInt32			minEntropy;
		
		minEntropy = PGPGlobalRandomPoolGetMinimumEntropy();
		do
		{
			randomDialog.DoDialog();
		} while(PGPGlobalRandomPoolGetEntropy() < minEntropy);
	}
}


#if PGP_DEBUG
#include "PowerPlantLeaks.h"
	static void
InitPowerplantLeaks()
{
	CForceInitLPeriodical	initLPeriodical;
}

#else
#define InitPowerplantLeaks()	{/*nothing*/}
#endif

	void
CPGPKeys::StartUp(void)
{
	PGPError		err;
	Int16			keyFilesErr;
	
	if( mAppStarted )
		return;		/* Already did this */
		
	pgpLeaksSuspend();
	new UMainThread;
	
	InitPowerplantLeaks();
	
	mHelpMenuRegisterItemNumber = -1;
	{
		OSErr		osErr;
		MenuHandle	helpMenuHandle;
		
		//Install Register... command on Help menu
		
		osErr = HMGetHelpMenuHandle(&helpMenuHandle);
		if(IsntErr(osErr))
		{
			Str255	itemStr;
			
#if PGP_FREEWARE
			GetIndString(itemStr, kStringListID,
				kPGPUpgradeItemStrIndex);
#else
			GetIndString(itemStr, kStringListID,
				kPGPRegisterOnlineItemStrIndex);
#endif
			AppendMenu(helpMenuHandle, itemStr);
			mHelpMenuRegisterItemNumber = CountMItems(helpMenuHandle);
		}
	}
	
	err = pgpNewContextCustomMacAllocators(&gPGPContext);
	//err = PGPNewContext(kPGPsdkAPIVersion, &gPGPContext);
	pgpAssertNoErr(err);

	err = PGPsdkLoadDefaultPrefs(gPGPContext);
	pgpAssertNoErr(err);
	
	err = PFLNewContext(&gPFLContext);
	pgpAssertNoErr(err);
	err = PGPOpenClientPrefs(gPFLContext, &gPrefRef);
	if(IsPGPError(err))
	{
		::SysBeep(1);
		CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
			kStringListID, kUnrecoverablePrefsErrorStrIndex);
		pgpAssertNoErr(err);
		::ExitToShell();
	}
	gAdminPrefRef = kPGPInvalidRef;
#if	PGP_BUSINESS_SECURITY
	err = PGPOpenAdminPrefs(gPFLContext, &gAdminPrefRef );
	if(IsPGPError(err))
	{
		CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
						kStringListID, kNoAdminPrefsStringID);
		pgpAssertNoErr(err);
		::ExitToShell();
	}
#endif
	
	if(!CFM_AddressIsResolved_(::PGPKeyServerInit))
		gServerCallsPresent = FALSE;
	else
		gServerCallsPresent = TRUE;
	
	// Personalize if necessary
	
	if(!mCheckedLicense)
	{
		Str255		str;
		char		companyStr[128];
		char		nameStr[128];
		char		licenseStr[128];
		char		cstr[128];
		ulong		len;

		mCheckedLicense = TRUE;
		
		len = 128;
		err = PGPGetPrefStringBuffer(gPrefRef, kPGPPrefCompanyName,
								len, companyStr);
		pgpAssert(IsntErr(err));
		err = PGPGetPrefStringBuffer(gPrefRef, kPGPPrefOwnerName,
								len, nameStr);
		pgpAssert(IsntErr(err));
		err = PGPGetPrefStringBuffer(gPrefRef, kPGPPrefLicenseNumber,
								len, licenseStr);
		pgpAssert(IsntErr(err));
		
		if(nameStr[0] == '\0')
		{
			const short			kWindowRegister		= 139;
			const MessageT		kOwnerNameID		= 'eNam';
			const MessageT		kOwnerCompanyID		= 'eCom';
			const MessageT		kOwnerLicenseID		= 'eLic';
			const PaneIDT		kOwnerLicenseCID	= 'cLic';

			CPGPStDialogHandler	regDialog(kWindowRegister, this);
			LWindow				*regLDialog;
			MessageT			message;
			
			regLDialog = regDialog.GetDialog();
#if PGP_NO_LICENSE_NUMBER
			((LGAEditField *)regLDialog->FindPaneByID(kOwnerLicenseID))->
				Hide();
			((LGACaption *)regLDialog->FindPaneByID(kOwnerLicenseCID))->
				Hide();
#endif
			do
			{
				message = regDialog.DoDialog();
			} while((message != msg_OK) && (message != msg_Cancel));
			if(message == msg_OK)
			{
				((LGAEditField *)regLDialog->FindPaneByID(kOwnerNameID))->
					GetDescriptor(str);
				PToCString(str, cstr);
				err = PGPSetPrefString(gPrefRef, kPGPPrefOwnerName, cstr);
				pgpAssertNoErr(err);
				((LGAEditField *)regLDialog->FindPaneByID(kOwnerCompanyID))->
					GetDescriptor(str);
				PToCString(str, cstr);
				err = PGPSetPrefString(gPrefRef, kPGPPrefCompanyName, cstr);
				pgpAssertNoErr(err);
#if !PGP_NO_LICENSE_NUMBER
				((LGAEditField *)regLDialog->FindPaneByID(kOwnerLicenseID))->
					GetDescriptor(str);
				PToCString(str, cstr);
				err = PGPSetPrefString(gPrefRef, kPGPPrefLicenseNumber,
										cstr);
				pgpAssertNoErr(err);
#endif
				PGPSavePrefFile(gPrefRef);
				pgpAssertNoErr(err);
			}
		}
	}

	ShowSplashScreen();
	mWritableKeyring = TRUE;
	err = PGPOpenDefaultKeyRings(gPGPContext,
				kPGPKeyRingOpenFlags_Mutable, &mKeySet);
	if(!PGPRefIsValid(mKeySet) && (err != kPGPError_FileLocked))
	{
		if(err == kPGPError_FilePermissions)
		{
			mWritableKeyring = FALSE;
			err = PGPOpenDefaultKeyRings(gPGPContext,
					(PGPKeyRingOpenFlags)0, &mKeySet);
		}
		if(!PGPRefIsValid(mKeySet) &&
			( err == kPGPError_CantOpenFile ||
				err == kPGPError_FileNotFound ))
		{
		tryOpenKeysAgain:
			if((keyFilesErr = AskMakeNewKeyFiles()) != 0)
			{
				if(keyFilesErr == 1)
					CPGPPreferenceDialog::ResetDefaultKeyringsPath();
				mWritableKeyring = TRUE;
				err = PGPOpenDefaultKeyRings(gPGPContext,
						(PGPKeyRingOpenFlags)
						(kPGPKeyRingOpenFlags_Mutable |
						kPGPKeyRingOpenFlags_Create), &mKeySet);
			}
			else
			{
				// User has requested Quit
				PGPFreeContext(gPGPContext);
				::ExitToShell();
			}
		}
	}
	if(!PGPRefIsValid(mKeySet) && IsPGPError(err))
	{
		// Fatal no keyrings error
		::SysBeep(1);
		CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
			kStringListID, kUnrecoverableKeyringErrorStrIndex);
		goto tryOpenKeysAgain;
	}
#if BETA
	if(BetaExpired())
	{
		::Alert(205, NULL);	// Beta expiration
		::ExitToShell();
	}
#endif

#if PGP_DEMO
	if( ShowDemoDialog( TRUE ) == kDemoStateExpired )
	{
		::ExitToShell();
	}
#endif

	mKeyWindow = (CKeyWindow *)
		LWindow::CreateWindow(kKeyWindow, this);
	mDefaultKeysView = mKeyWindow->GetKeyView();
	mDefaultKeysView->SetKeyDBInfo(mKeySet, mWritableKeyring, true);
	
	StartIdling();
	
	pgpLeaksResume();
	
	mAppStarted = TRUE;
}

	void
CPGPKeys::ListenToMessage(	MessageT inMessage,
								void */*ioParam*/)
{
	switch(inMessage)
	{
		case kSearchWindowClosed:
			mSearchWindow = NULL;
			break;
		case kSearchInProgress:
			mSearching = TRUE;
			SetUpdateCommandStatus(true);
			break;
		case kSearchComplete:
			mSearching = FALSE;
			SetUpdateCommandStatus(true);
			break;
		case kGroupsWindowClosed:
			mGroupsWindow = NULL;
			break;
	}
}

	void
CPGPKeys::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	switch (inCommand) 
	{
		case cmd_KSFindKey:
			if(gServerCallsPresent)
				outEnabled = true;
			break;
		case cmd_ShowGroups:
			outEnabled = true;
			break;
		case cmd_Preferences:	
			outEnabled = true;
			break;
		case cmd_Quit:
			if(mSearching)
			{
				outEnabled = false;
				break;
			}
		default:
			LDocApplication::FindCommandStatus(inCommand, outEnabled,
												outUsesMark, outMark, outName);
			break;
	}
}

#if CANC_PGP_FREEWARE	/* [ */

	void
CPGPKeys::ShowFreewareUpgradeDialog(Boolean skipDateCheck)
{
	PFLContextRef	pflContext;
	PGPError		err;
	
	err	= PFLNewContext( &pflContext );
	if ( IsntPGPError( err ) )
	{
		PGPPrefRef		prefRef;

		err = PGPOpenClientPrefs( pflContext, &prefRef );
		if( IsntPGPError( err ) )
		{
			PGPUInt32	lastDialogTime;
			UInt32		now;

			err = PGPGetPrefNumber( prefRef, kPGPPrefDateOfLastFreewareNotice,
							&lastDialogTime );
			pgpAssertNoErr( err );
			
			GetDateTime( &now );
			
			if( skipDateCheck || IsErr( err ) ||
				now - lastDialogTime > ( 60L * 60L * 24L ) ||
										/* One day in secs */
				lastDialogTime > now )	/* Clock has been set back */
			{
				FSSpec	libFileSpec;
				
				if( IsntErr( PGPGetUILibFSSpec( &libFileSpec ) ) )
				{
					short	saveResFile;
					short	libFileRefNum;
					
					saveResFile = CurResFile();
					
					libFileRefNum = FSpOpenResFile( &libFileSpec, fsRdPerm );
					if( libFileRefNum > 0 )
					{
						UseResFile( libFileRefNum );
						
						{
							CPGPStDialogHandler	theDialog(
												kWindowFreewareUpgrade, NULL );
							MessageT			message = msg_Nothing;
								
							const PaneIDT		kLaterButtonPaneID	= 'bLtr';
							const PaneIDT		kBuyNowButtonPaneID	= 'bBuy';

							const MessageT		kLaterMessage 	=
													kLaterButtonPaneID;
							const MessageT		kBuyNowMessage 	=
													kBuyNowButtonPaneID;
							
							theDialog.GetDialog()->Show();
							
							while( TRUE )
							{
								message = theDialog.DoDialog();
								
								if( message == kLaterMessage ||
									message == kBuyNowMessage )
								{
									break;
								}
							}
							
							if( message == kBuyNowMessage )
							{
								Str255		url;
								OSStatus	status;
								
								GetIndString( url, kStringListID,
												kPGPBuyNowURLStrIndex );
								
								status = OpenURL( url );
								if( IsErr( status ) )
								{
									Str255	errorStr;

									SysBeep( 1 );
									::NumToString(status, errorStr);
									CWarningAlert::Display( kWACautionAlertType,
										kWAOKStyle,
										kStringListID,
										kCantLaunchWebBrowserBecauseStrIndex,
										errorStr );
								}
							}
						}
						
						CloseResFile( libFileRefNum );
						UseResFile( saveResFile );
					}
				}

				err = PGPSetPrefNumber( prefRef,
							kPGPPrefDateOfLastFreewareNotice, now );
				pgpAssert( IsntErr( err ) );
				
				err = PGPSavePrefFile( prefRef );
				pgpAssert( IsntErr( err ) );
			}

			(void) PGPClosePrefFile( prefRef );
		}
		
		PFLFreeContext( pflContext );
	}
}

#endif	/* ] PGP_FREEWARE */

	Boolean
CPGPKeys::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true;
	ResIDT		theMenuID;
	Int16		theMenuItem;

	/* Handle case of earlier than expected commands */
	if( ! mAppStarted )
		StartUp();
		
	if(IsSyntheticCommand(inCommand, theMenuID, theMenuItem))
	{
		switch(theMenuID)
		{
			case kSubstituteHelpMenuID:
			{
				if(theMenuItem == mHelpMenuRegisterItemNumber)
				{
					Str255		url,
								licenseStr,
								licenseNumStr;
					char		licenseCStr[128];
					OSStatus	status;
					PGPError	err;

#if PGP_FREEWARE
					GetIndString( url, kStringListID,
									kPGPBuyNowURLStrIndex );
#else
					GetIndString( url, kStringListID,
									kPGPRegisterURLStrIndex );
#endif
					licenseCStr[0] = '\0';
#if	PGP_BUSINESS_SECURITY
					err = PGPGetPrefStringBuffer(
							gAdminPrefRef, kPGPPrefSiteLicenseNumber, 128,
							licenseCStr);
#endif
					if(licenseCStr[0] == '\0')
					{
						err = PGPGetPrefStringBuffer(
							gPrefRef, kPGPPrefLicenseNumber, 128,
							licenseCStr);
					}
					if(licenseCStr[0] != '\0')
					{
						CToPString(licenseCStr, licenseNumStr);
						GetIndString( licenseStr, kStringListID,
									kURLRegExtensionStrIndex );
						AppendPString(licenseNumStr, licenseStr);
						AppendPString(licenseStr, url);
					}
					status = OpenURL( url );
					if( IsErr( status ) )
					{
						Str255	errorStr;

						SysBeep( 1 );
						::NumToString(status, errorStr);
						CWarningAlert::Display( kWACautionAlertType,
							kWAOKStyle,
							kStringListID,
							kCantLaunchWebBrowserBecauseStrIndex,
							errorStr );
					}
				}
				break;
			}
			default:
				cmdHandled = LDocApplication::ObeyCommand(inCommand, ioParam);
				break;
		}
	}
	else switch (inCommand) 
	{
		case cmd_Preferences:
			{
				CPGPPreferenceDialog *prefDlog;
				
				prefDlog = (CPGPPreferenceDialog *)
					LWindow::CreateWindow(	dialog_Preferences, this);
				prefDlog->AddListener(mDefaultKeysView->GetKeyTable());
			}
			break;
		case cmd_KSFindKey:
			if(IsNull(mSearchWindow))
			{
				mSearchWindow = (CSearchWindow *)
					LWindow::CreateWindow(kSearchWindowID, this);
				mSearchWindow->AddListener(this);
			}
			else
				mSearchWindow->Select();
			break;
		case cmd_ShowGroups:
			if(IsNull(mGroupsWindow))
			{
#if CANC_PGP_FREEWARE
				CWarningAlert::Display(kWANoteAlertType, kWAOKStyle,
					kStringListID, kNoGroupsSupportStrIndex);
#endif
				mGroupsWindow = (CGroupsWindow *)
					LWindow::CreateWindow(kGroupsWindowID,
					CPGPKeys::TheApp());
				mGroupsWindow->AddListener(this);
			}
			else
				mGroupsWindow->Select();
			break;
		case cmd_Quit:
			if(TryingToQuit())
				SendAEQuit();
			break;
		default:
			cmdHandled = LDocApplication::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

	Boolean
CPGPKeys::TryingToQuit(void)
{
	if(mCreatedKey)
	{
		BackupHandler	handler;
		return handler.Run();
	}
	else
		return true;
}

	void
CPGPKeys::CreatedKey()
{
	mCreatedKey = true;
}

	void
CPGPKeys::MakeLicenseStr(Str255 outLicenseStr)
{
	PGPError	err;
	char		companyStr[128];
	char		nameStr[128];
	char		licenseStr[128];
	char		str[256];
	ulong		len;
	
	len = 128;
	err = PGPGetPrefStringBuffer(gPrefRef, kPGPPrefCompanyName,
								len, companyStr);
	pgpAssertNoErr(err);
	err = PGPGetPrefStringBuffer(gPrefRef, kPGPPrefOwnerName,
								len, nameStr);
	pgpAssertNoErr(err);
	err = PGPGetPrefStringBuffer(gPrefRef, kPGPPrefLicenseNumber,
								len, licenseStr);
	pgpAssertNoErr(err);
	strcpy(str, nameStr);
	strcat(str, "\r");
	strcat(str, companyStr);
	strcat(str, "\r");
	strcat(str, licenseStr);
	c2pstr(str);
	CopyPString((uchar *)str, outLicenseStr);
}

	void
CPGPKeys::ShowAboutBox()
{
	Boolean	didShowDialog = FALSE;
	FSSpec	libraryFileSpec;
	
	// Open the library resource file at the top of the resource chain
	// so we get our pictures.
	if( IsntErr( PGPGetUILibFSSpec( &libraryFileSpec ) ) )
	{
		short	saveResFile;
		short	libFileRefNum;
		
		saveResFile = CurResFile();
		
		libFileRefNum = FSpOpenResFile( &libraryFileSpec, fsRdPerm );
		if( libFileRefNum > 0 )
		{
			UseResFile( libFileRefNum );
			
			{
				CPGPStDialogHandler	aboutDialog(kWindowAbout, this);
				LWindow			*theDialog;
				MessageT		message;
				Boolean			credits = FALSE;
				Str255			str;
				
				const MessageT		kPGPButtonMessage		= 'bPGP';
				const MessageT		kCreditsButtonMessage	= 'bCre';
				const MessageT		kOKButtonMessage		= 'bOK ';
				
				const PaneIDT		kCreditsButtonPaneID	= 'bCre';
				const PaneIDT		kAboutPicturePaneID		= 'APIC';
				const PaneIDT		kCreditsPicturePaneID	= 'CPIC';
				const PaneIDT		kLicenseCaptionID		= 'cLic';
				
				theDialog = aboutDialog.GetDialog();
				didShowDialog = TRUE;

				MakeLicenseStr(str);
				((LCaption *)theDialog->FindPaneByID(kLicenseCaptionID))->
					SetDescriptor(str);
				
				while((message = aboutDialog.DoDialog()) != kOKButtonMessage )
				{
					if( message == kPGPButtonMessage )
					{
						Str255		url;
						OSStatus	status;
						
						GetIndString( url, kStringListID,
										kPGPWebSiteURLStrIndex );
						
						status = OpenURL( url );
						if( IsErr( status ) )
						{
							Str255	errorStr;

							SysBeep( 1 );
							::NumToString(status, errorStr);
							CWarningAlert::Display( kWACautionAlertType,
								kWAOKStyle,
								kStringListID,
								kCantLaunchWebBrowserBecauseStrIndex,
								errorStr );
						}
					}
					else if ( message == kCreditsButtonMessage )
					{
						Str255	buttonTitle;
						
						if( credits )
						{
							theDialog->FindPaneByID( kAboutPicturePaneID )->
								Show();
							theDialog->FindPaneByID( kCreditsPicturePaneID )->
								Hide();
							
							GetIndString( buttonTitle, kStringListID,
								kAboutCreditsButtonTitleStrIndex );
						}
						else
						{
							theDialog->FindPaneByID( kCreditsPicturePaneID )->
								Show();
							theDialog->FindPaneByID( kAboutPicturePaneID )->
								Hide();
							
							GetIndString( buttonTitle, kStringListID,
								kAboutInfoButtonTitleStrIndex );
						}
						
						((LControl *)theDialog->
							FindPaneByID(kCreditsButtonPaneID))->
							SetDescriptor( buttonTitle );

						credits = ! credits;
					}
				}
			}
			
			CloseResFile( libFileRefNum );
			UseResFile( saveResFile );
		}
	}
	
	if( ! didShowDialog )
		SysBeep( 1 );
}

	void
CPGPKeys::ShowSplashScreen()
{
	FSSpec		libraryFileSpec;
	PGPError	err;
	ulong		timeNow;
	PGPUInt32	lastSplashed;
	
	GetDateTime(&timeNow);
	err = PGPGetPrefNumber(gPrefRef, kPGPPrefDateOfLastSplashScreen,
							&lastSplashed);
	pgpAssertNoErr(err);
	if(timeNow - lastSplashed < kSecondsInADay)		// One day
		return;
	
	// Open the libarary resource file at the top of the resource chain
	// so we get our pictures.
	if( IsntErr( PGPGetUILibFSSpec( &libraryFileSpec ) ) )
	{
		short	saveResFile;
		short	libFileRefNum;
		
		saveResFile = CurResFile();
		
		libFileRefNum = FSpOpenResFile( &libraryFileSpec, fsRdPerm );
		if( libFileRefNum > 0 )
		{
			UseResFile( libFileRefNum );
			
			{
				const ResIDT		kWindowSplash			= 146;
				const MessageT		kLicenseCaptionID		= 'cLic';
				LCaption			*licenseCaption;
				Str255				licenseStr;
				
				mSplashScreen = (LGADialogBox *)
					LWindow::CreateWindow(kWindowSplash, this);
				licenseCaption = ((LCaption *)
					mSplashScreen->FindPaneByID(kLicenseCaptionID));
				MakeLicenseStr(licenseStr);
				licenseCaption->SetDescriptor(licenseStr);
				mSplashScreen->UpdatePort();
				mSplashDisplayTime = LMGetTicks();
				err = PGPSetPrefNumber(gPrefRef,
						kPGPPrefDateOfLastSplashScreen, timeNow);
				pgpAssert(IsntErr(err));
			}
			
			CloseResFile( libFileRefNum );
			UseResFile( saveResFile );
		}
	}
}

	void
CPGPKeys::EndSplashScreen()
{
	if(mSplashScreen)
	{
		while((mSplashDisplayTime + kSplashDisplayTime > LMGetTicks()) &&
				!Button())
			;
		delete mSplashScreen;
		mSplashScreen = NULL;
		::FlushEvents(mDownMask, 0);
	}
}

	void
CPGPKeys::MakeMenuBar()
{
	mMenuBar = new PGPKeysMenuBar(MBAR_Initial);
}

	void
CPGPKeys::MakeModelDirector()
{
	pgpLeaksSuspend();
	mModelDirector = new LModelDirector(this);
	pgpLeaksResume();
}

	void
CPGPKeys::SpendTime(const EventRecord &/*inMacEvent*/)
{
	GrafPtr		savePort;
	GrafPtr		wMgrPort;
	Point		mouseLoc;

	if(IsntNull(mSplashScreen))
	{
		EndSplashScreen();
#if CANC_PGP_FREEWARE
		ShowFreewareUpgradeDialog(FALSE);
#endif
	}

	::GetPort(&savePort);
	::GetWMgrPort(&wMgrPort);	
	
	::SetPort(wMgrPort);
	::GetMouse(&mouseLoc);
	::SetPort(savePort);
	
	if((mouseLoc.v != mLastMouseLoc.v) ||
		(mouseLoc.h != mLastMouseLoc.h))
	{
		PGPGlobalRandomPoolAddMouse(mouseLoc.h, mouseLoc.v);
		mLastMouseLoc = mouseLoc;
	}
}

	PGPKeyListRef
CPGPKeys::GetKeyList()
{
	return mKeyList;
}

	void
CPGPKeys::SetKeyList(PGPKeyListRef keyList)
{
	mKeyList = keyList;
}

	PGPKeySet *
CPGPKeys::GetKeySet()
{
	return mKeySet;
}

	void
CPGPKeys::CommitDefaultKeyrings()
{
	PGPError	err;
	
	err = PGPCommitKeyRingChanges(mKeySet);
	if(IsPGPError(err))
	{
		if((err == kPGPError_ItemIsReadOnly) &&
			!mWritableKeyring)
		{
			PGPBoolean	warnRO;
			
			err = PGPGetPrefBoolean(gPrefRef,
				kPGPPrefWarnOnReadOnlyKeyRings, &warnRO);
			pgpAssertNoErr(err);
			if(warnRO)
			{
				CPGPStDialogHandler	roErrorDialog(kReadOnlyErrorDialog, this);
				const PaneIDT		kNoWarnCheckbox	= 'xWar';
				MessageT			dialogMessage;
				LWindow				*roErrorLWindow;
				
				do
				{
					dialogMessage = roErrorDialog.DoDialog();
				} while(!dialogMessage);
				roErrorLWindow = roErrorDialog.GetDialog();
				if(((LGACheckbox *)roErrorLWindow->
					FindPaneByID(kNoWarnCheckbox))->
					GetValue())
				{
					err = PGPSetPrefBoolean(gPrefRef,
						kPGPPrefWarnOnReadOnlyKeyRings, FALSE);
					pgpAssertNoErr(err);
				}
			}
		}
		else
			ReportPGPError(err);
	}
}

	void
CPGPKeys::InvalidateCaches()
{
	if(IsntNull(mGroupsWindow))
	{
		mGroupsWindow->InvalidateCaches();
	}
}

	void
CPGPKeys::OpenDocument(
	FSSpec	*inMacFSSpec)
{
	if( ! mAppStarted )
		StartUp();
}

	void
ReportPGPError(PGPError err)
{
	if(IsPGPError(err))
	{
		char	str[256];
		Str255	pStr;
		
		PGPGetErrorString(err, sizeof( str ), str);
		CToPString( str, pStr);
		
		CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
			kStringListID, kErrorPrefaceStringID, pStr);
	}
}

	void
CPGPKeys::HandleAppleEvent(
	const AppleEvent	&inAppleEvent,
	AppleEvent			&outAEReply,
	AEDesc				&outResult,
	long				inAENumber)
{
	switch( inAENumber )
	{
		case ae_ImportKey:
		{
			StAEDescriptor	descKeys(	inAppleEvent,
										keyDirectObject);
			
			if(mWritableKeyring)
			{
				mDefaultKeysView->
					ImportKeysFromHandle(descKeys.mDesc.dataHandle);
			}
			break;
		}


		default:
			LDocApplication::HandleAppleEvent(inAppleEvent, outAEReply,
								outResult, inAENumber);
			break;
	}
	
}

