/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <string.h>

#include <LActiveScroller.h>
#include <LCaption.h>
#include <LGABox.h>
#include <LGACaption.h>
#include <LGACmdIconButton.h>
#include <LGAIconButton.h>
#include <LGAPushButton.h>
#include <LGrowZone.h>
#include <LIconPane.h>
#include <LMenu.h>
#include <LMenuBar.h>

#include <UAppleEventsMgr.h>
#include <UDesktop.h>
#include <UDrawingState.h>
#include <UMemoryMgr.h>
#include <UModalDialogs.h>
#include <URegistrar.h>
#include <UWindows.h>

#include "CVersionCaption.h"
#include "CWarningAlert.h"
#include "MacDesktop.h"
#include "MacErrors.h"
#include "MacEvents.h"
#include "MacEnvirons.h"
#include "MacFiles.h"
#include "MacInternet.h"
#include "MacProcesses.h"
#include "MacStrings.h"
#include "pgpMem.h"
#include "PowerPlantLeaks.h"
#include "CWrappers.h"

#include "BETA.h"
#include "PGPDemo.h"
#include "PGPOpenPrefs.h"
#include "PGPtools.h"
#include "PGPtoolsEncryptDecrypt.h"
#include "PGPtoolsPreferences.h"
#include "PGPtoolsResources.h"
#include "PGPtoolsUtils.h"
#include "PGPSharedEncryptDecrypt.h"
#include "pgpFeatures.h"
#include "pgpUserInterface.h"
#include "pgpUtilities.h"

#include "CDragTextEdit.h"
#include "CDropReceiver.h"
#include "CPGPStDialogHandler.h"
#include "CPGPtools.h"
#include "CPGPtoolsClipboardTask.h"
#include "CPGPtoolsMainWindow.h"
#include "CPGPtoolsFileTask.h"
#include "CPGPtoolsWipeTask.h"
#include "CPGPtoolsTaskList.h"
#include "CPGPtoolsTaskProgressDialog.h"
#include "CPGPtoolsTextWindow.h"
#include "CPicture.h"
#include "CProgressBar.h"
#include "CResultsTable.h"
#include "CResultsWindow.h"

#include "pgpClientPrefs.h"
#include "pgpLeaks.h"
#include "pgpKeys.h"
#include "pgpUtilities.h"
#include "pgpRandomPool.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpMacCustomContextAlloc.h"

CPGPtools		*gApplication;

Boolean CPGPtools::mAppCanEncrypt		= TRUE;
Boolean CPGPtools::mAppCanSign			= TRUE;
Boolean CPGPtools::mAppCanDecryptVerify	= TRUE;

// Note: Changing these definitions requires changing the command values
// in the PPob resource for the buttons of the main window.

const CommandT	cmd_PGPEncrypt			= 'Encr';
const CommandT	cmd_PGPSign				= 'Sign';
const CommandT	cmd_PGPEncryptSign		= 'EncS';
const CommandT	cmd_PGPDecryptVerify	= 'Decr';
const CommandT	cmd_PGPKeys				= 'Keys';
const CommandT	cmd_PGPWipe				= 'Wipe';

const CommandT	cmd_ShowHideResults		= 'Resu';

//	Validate that that application can run on this machine. Return TRUE if we
//	can run.

const ushort	kMinSystemVersion = 0x0710;

const ResIDT	kMinSystemVersionAlertResID	= 2100;
const ResIDT	kNeed68020AlertResID		= 2101;
const ResIDT	kBetaExpiredAlertResID		= 2102;


/* these are found in the "aedt" resource in PGPtools.rsrc */
const long		ae_Sign						= 10000;
const long		ae_Encrypt					= 10001;
const long		ae_EncryptAndSign			= 10002;
const long		ae_DecryptVerify			= 10003;
const long		ae_OptionSign				= 10004;
const long		ae_OptionEncrypt			= 10005;
const long		ae_OptionEncryptAndSign		= 10006;
const long		ae_OptionDecryptVerify		= 10007;
const long		ae_SecureWipe				= 10008;
const long		ae_OpenedFromMenu			= 10009;

const OSType	kFinderType					= 'FNDR';
const OSType	kFinderCreator				= 'MACS';

#if PGP_DEBUG
const CommandT	cmd_DebugTest				= 10001;
#endif

	static Boolean
VerifyEnvironment(void)
{
	ushort	systemVersion;
	
	systemVersion = GetSystemVersion();

	if( systemVersion < kMinSystemVersion )
	{
		Str255	versionString;
		
		VersionToString( kMinSystemVersion, versionString );
		ParamText( versionString, "\p", "\p", "\p" );
		StopAlert( kMinSystemVersionAlertResID, nil );
		return( FALSE );
	}

#if BETA
	if ( BetaExpired() )
	{
		NoteAlert( kBetaExpiredAlertResID, nil );
		// keep going
	}
#endif

#if PGP_DEMO
	if( ShowDemoDialog( TRUE ) == kDemoStateExpired )
	{
		return( FALSE );
	}
#endif

	return( TRUE );
}

	static void
RealMain( void  )
{
	LGrowZone		*myGrowZoneObj;
	
	MacLeaks_Suspend();	
	
	myGrowZoneObj = new LGrowZone(20000);
	
	MacLeaks_Resume();
	
	CPGPtools theApp;
	theApp.Run();
	
	if( myGrowZoneObj != nil )
		delete( myGrowZoneObj );
}

// Disable 68020 code generation for the 68020 check itself
#if GENERATING68K
	#pragma code68020 off
#endif

	void
main(void)
{
	long	gestaltResult;
	
	// Check for a 68020 or later processor and bail immediately if none is
	// available.
	if( Gestalt( gestaltProcessorType, &gestaltResult ) == noErr &&
		gestaltResult >= gestalt68020 )
	{
		// OK
	}
	else
	{
		// Need to initialize the toolbox here. Cannot call
		// UQDGlobals::InitializeToolbox() because
		// we cannot control the code generation there.
		
		MaxApplZone();
		InitGraf( (Ptr) &qd.thePort );
		InitFonts();
		InitWindows();
		InitMenus();
		TEInit();
		InitDialogs( nil );
		
		StopAlert( kNeed68020AlertResID, nil );
		return;
	}

	//The following calls determine the Debug state for PowerPlant,
	//and should be set appropriately.
	
	//debugAction_Nothing
	//debugAction_Alert
	//debugAction_LowLevelDebugger
	//debugAction_SourceDebugger

#if PGP_DEBUG
	gDebugThrow = debugAction_SourceDebugger;
	gDebugSignal = debugAction_SourceDebugger;
#endif

	InitializeHeap(3);		// Number of Master Pointer blocks to allocate
	UQDGlobals::InitializeToolbox(&qd);
	UnloadScrap();
	
	DebugPatches_PatchDisposeTraps();

	if( VerifyEnvironment() )
	{
		#if USE_MAC_DEBUG_LEAKS
		{ CForceInitLPeriodical	temp; }
		#endif
		
		pgpLeaksBeginSession("main");
			RealMain();
			URegistrar::DisposeClassTable();
		pgpLeaksEndSession();
	}
}

#if GENERATING68K
	#pragma code68020 reset
#endif

CPGPtools::CPGPtools() :
		LApplication()
{
	PGPFlags	featureFlags;
	
	mAutoQuit				= FALSE;
	mUserLaunchedAppOnly	= FALSE;
	mMainWindow 			= nil;
	gApplication			= this;
	mApplicationResFile		= CurResFile();
	mAppCanEncrypt			= TRUE;
	mAppCanSign				= TRUE;
	mAppCanDecryptVerify	= TRUE;
	
	/* contexts exist only while processing */
	mContext				= kInvalidPGPContextRef;

#if BETA
	if( BetaExpired() )
	{
		mAppCanEncrypt	= FALSE;
		mAppCanSign		= FALSE;
	}
#endif

#if PGP_DEMO
	if( mAppCanEncrypt || mAppCanSign )
	{
		if( GetDemoState() ==
			kDemoStateEncryptionSigningDisabled )
		{
			mAppCanEncrypt 	= FALSE;
			mAppCanSign 	= FALSE;
		}
	}
#endif

	// Check the SDK to see if we can encrypt/sign
	
	if( IsntPGPError( PGPGetFeatureFlags( kPGPFeatures_GeneralSelector,
			&featureFlags ) ) )
	{
		if( ! PGPFeatureExists( featureFlags, kPGPFeatureMask_CanEncrypt ) )
		{
			mAppCanEncrypt = FALSE;
		}

		if( ! PGPFeatureExists( featureFlags, kPGPFeatureMask_CanSign ) )
		{
			mAppCanSign = FALSE;
		}

		if( ( ! PGPFeatureExists( featureFlags, kPGPFeatureMask_CanDecrypt)) ||
			( ! PGPFeatureExists( featureFlags, kPGPFeatureMask_CanVerify)) )
		{
			mAppCanDecryptVerify = FALSE;
		}
	}
	
	// Standard PowerPlant classes
	RegisterClass_(LActiveScroller);
	RegisterClass_(LCaption);
	RegisterClass_(LIconPane);
	RegisterClass_(LPicture);
	RegisterClass_(LTextEdit);
	RegisterClass_(LView);
	RegisterClass_(LWindow);

	// Extra PowerPlant class registrations
	RegisterClass_(LGABox);
	RegisterClass_(LGACaption);
	RegisterClass_(LGADialogBox);
	RegisterClass_(LGAPushButton);
	RegisterClass_(LGAIconButton);
	RegisterClass_(LGAIconSuite);
	RegisterClass_(LGACmdIconButton);
	
	// Custom class declarations
	RegisterClass_(CDragTextEdit);
	RegisterClass_(CVersionCaption);
	RegisterClass_(CPGPtoolsMainWindow);
	RegisterClass_(CPGPtoolsTaskProgressDialog);
	RegisterClass_(CPGPtoolsTextWindow);
	RegisterClass_(CPicture);
	RegisterClass_(CProgressBar);
	RegisterClass_(CResultsTable);
	RegisterClass_(CResultsWindow);
	
	// We do not want the default behavior of RegisterClass__ for this class
	URegistrar::RegisterClass(CDropReceiver::class_ID,
				(ClassCreatorFunc) CDropReceiver::CreateFromStream);

	/* make sure we can get a context. Also load prefs */
	if ( IsntPGPError( CreateContext( FALSE ) ) )
	{
		DisposeContext();
	}
}


CPGPtools::~CPGPtools()
{
	SavePreferences( );
	
	DisposeContext();
}

	Boolean
CPGPtools::AppCanDoOperation(PGPtoolsOperation operation)
{
	Boolean	canDo = FALSE;
	
	switch( operation )
	{
		case kPGPtoolsInvalidOperation:
			canDo = FALSE;
			break;
			
		case kPGPtoolsEncryptOperation:
			canDo = mAppCanEncrypt;
			break;

		case kPGPtoolsSignOperation:
			canDo = mAppCanSign;
			break;

		case kPGPtoolsEncryptSignOperation:
			canDo = (mAppCanEncrypt && mAppCanSign);
			break;

		case kPGPtoolsDecryptVerifyOperation:
			canDo = mAppCanDecryptVerify;
			break;
			
		case kPGPtoolsWipeOperation:
			canDo = TRUE;
			break;
			
		default:
			pgpDebugMsg( "CPGPtools::AppCanDoOperation: Unknown op" );
			canDo = FALSE;
			break;
	}
	
	return( canDo );	
}

	PGPError
CPGPtools::CreateContext(Boolean verifyContext)
{
	PGPError	err;

	pgpAssert( IsNull( mContext ) );
	
	err = pgpNewContextCustomMacAllocators( &mContext );
	if( IsntPGPError( err ) )
	{
		err = LoadPreferences( mContext );
	}
	
	if( verifyContext )
	{
		if ( IsntPGPError( err ) )
		{
			UInt32		numPublicKeys = 0;
			UInt32		numPrivateKeys = 0;

			err = CountKeysInDefaultKeyring( mContext, &numPublicKeys,
						&numPrivateKeys );
			if( IsntPGPError( err )  )
			{
				short	strIndex = 0;
				
				if( numPublicKeys == 0 )
				{
					if( numPrivateKeys == 0 )
					{
						strIndex = kNoKeysFoundStrIndex;
					}
					else
					{
						strIndex = kNoPublicKeysFoundStrIndex;
					}
				}
				else if( numPrivateKeys == 0 )
				{
					strIndex = kNoPrivateKeysFoundStrIndex;
				}		
			
				if( strIndex != 0 )
				{	
					SysBeep( 1 );
					CWarningAlert::Display( kWACautionAlertType, kWAOKStyle,
							kErrorStringListResID, strIndex );
				}
			}
			else
			{
				short	strIndex;
				Str255	errorString;
				char	cError[ 256 ];
					
				DisposeContext();
				
				if( err == kPGPError_FileNotFound )
				{
					strIndex = kNoKeyringsFoundStrIndex;
				}
				else
				{
					strIndex = kLibraryInitErrorStrIndex;
				}
				
				PGPGetErrorString( err, sizeof( cError ), cError );
				CToPString( cError, errorString );
				
				SysBeep( 1 );
				CWarningAlert::Display( kWAStopAlertType, kWAOKStyle,
					kErrorStringListResID, strIndex, errorString );
			}
		}
		else
		{
			/* couldn't create context */
			Str255	errorStr;
			
			pgpAssert( IsNull( mContext ) );
			
			/* can't use the context to get an error string because
			we couldn't initialize it */
			PGPGetErrorString( err, sizeof( errorStr ),
					(char *) errorStr );
			CToPString( (char *) errorStr, errorStr );
			CWarningAlert::Display( kWAStopAlertType, kWAOKStyle,
				kErrorStringListResID, kLibraryInitErrorStrIndex,
				errorStr );
		}
	}
	
	return( err );
}


	PGPError
CPGPtools::DisposeContext()
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( IsntNull( mContext ) )
	{
		err	= PGPFreeContext( mContext );
		mContext	= NULL;
	}
	
	pgpAssertNoErr( err );
	return( err );
}


	void
CPGPtools::Initialize(void)
{
	StartIdling();
	
	mMainWindow = (CPGPtoolsMainWindow *) LWindow::CreateWindow(window_Main,
							this);
	pgpAssertAddrValid( mMainWindow, VoidAlign );

	if( IsntNull( mMainWindow ) )
	{
		if( gPreferences.mainWindowLocation.v != 0 ||
			gPreferences.mainWindowLocation.h != 0 )
		{
			mMainWindow->DoSetPosition( gPreferences.mainWindowLocation );
		}
		
	}
	
	mResultsWindow = (CResultsWindow *) LWindow::CreateWindow(
			window_Results, this);
	pgpAssertAddrValid( mResultsWindow, VoidAlign );
	
	if( IsntNull( mResultsWindow ) )
	{
		if( ! EmptyRect( &gPreferences.resultsWindowBounds ) )
		{
			mResultsWindow->DoSetBounds( gPreferences.resultsWindowBounds );
		}
	}

#if PGP_DEBUG
	InstallDebugMenu();
#endif
}

// Allow for more environment checking after the app is fully initialized.
// If the function returns FALSE, the app will quit.

	Boolean
CPGPtools::SecondaryVerifyEnvironment(void)
{
	CComboError	err;
	Boolean		canRun = TRUE;
	
#if PGP_BUSINESS_SECURITY
	#if ! PGP_ADMIN_BUILD

	// Check for the presence of an Admin Prefs file. If not found,
	// disable encryption/signing.
	
	{
		PFLContextRef	context;
		
		if( IsntPGPError( PFLNewContext( &context ) ) )
		{
			PGPPrefRef	prefsRef;
			
			if( IsntPGPError( PGPOpenAdminPrefs( context, &prefsRef ) ) )
			{
				PGPClosePrefFile( prefsRef );
			}
			else
			{
				SysBeep( 1 );
				CWarningAlert::Display( kWACautionAlertType, kWAOKStyle,
								kErrorStringListResID,
								kNoAdminPrefsFileFoundStrIndex );
								
				canRun = FALSE;
			}
			
			PFLFreeContext( context );
		}
	}
	
	#endif
#endif

	if( canRun )
	{
		/* make sure we can get a context */
		if ( IsntPGPError( CreateContext( FALSE) ) )
		{
			DisposeContext();
		}
		else
		{
			canRun = FALSE;
		}
	}

	return( canRun );
}

	Boolean
CPGPtools::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand)
	{
		case cmd_PGPEncrypt:
			CreateAndEnqueueTaskPrompt( kPGPtoolsEncryptOperation );
			break;
			
		case cmd_PGPSign:
			CreateAndEnqueueTaskPrompt( kPGPtoolsSignOperation );
			break;
			
		case cmd_PGPEncryptSign:
			CreateAndEnqueueTaskPrompt( kPGPtoolsEncryptSignOperation );
			break;

		case cmd_PGPDecryptVerify:
			CreateAndEnqueueTaskPrompt( kPGPtoolsDecryptVerifyOperation );
			break;

		case cmd_PGPWipe:
			CreateAndEnqueueTaskPrompt( kPGPtoolsWipeOperation );
			break;
		
		case cmd_PGPKeys:
			LaunchPGPkeys();
			break;
			
		case cmd_ShowHideResults:
		{
			if( mResultsWindow->IsVisible() )
			{
				mResultsWindow->Hide();
			}
			else
			{
				mResultsWindow->Select();
				mResultsWindow->Show();
			}
			
			break;
		}

#if PGP_DEBUG
		case cmd_DebugTest:
			DebugTest();
			break;
#endif
		
		default:
			cmdHandled = LApplication::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

	void
CPGPtools::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	switch (inCommand)
	{
		case cmd_PGPEncrypt:
			outEnabled = AppCanDoOperation( kPGPtoolsEncryptOperation );
			break;

		case cmd_PGPSign:
			outEnabled = AppCanDoOperation( kPGPtoolsSignOperation );
			break;

		case cmd_PGPEncryptSign:
			outEnabled = AppCanDoOperation( kPGPtoolsEncryptSignOperation );
			break;
			
		case cmd_PGPDecryptVerify:
			outEnabled = AppCanDoOperation( kPGPtoolsDecryptVerifyOperation );
			break;
			
		case cmd_PGPWipe:
			outEnabled = AppCanDoOperation( kPGPtoolsWipeOperation );
			break;

		case cmd_PGPKeys:
			outEnabled = TRUE;
			break;

		case cmd_ShowHideResults:
		{
			short	strIndex;
			
			if( mResultsWindow->IsVisible() )
			{
				strIndex = kHideResultsStrIndex;
			}
			else
			{
				strIndex = kShowResultsStrIndex;
			}

			outEnabled = TRUE;
			GetIndString( outName, kPGPtoolsMiscStringsResID, strIndex );
				
			break;
		}
			
#if PGP_DEBUG
		case cmd_DebugTest:
			outEnabled = TRUE;
			break;
#endif

		default:
			LApplication::FindCommandStatus(inCommand, outEnabled,
											outUsesMark, outMark, outName);
			break;
	}
}

	static void
MakeLicenseStr(StringPtr outLicenseStr)
{
	PFLContextRef	pflContext;
	PGPError		err;
	
	outLicenseStr[0] = 0;
	
	err	= PFLNewContext( &pflContext );
	if ( IsntPGPError( err ) )
	{
		PGPPrefRef		prefRef;

		err = PGPOpenClientPrefs( pflContext, &prefRef );
		if( IsntPGPError( err ) )
		{
			char	companyStr[128];
			char	nameStr[128];
			char	licenseStr[128];
			char	str[256];

			err = PGPGetPrefStringBuffer( prefRef, kPGPPrefCompanyName,
						sizeof( companyStr ) - 1, companyStr );
			pgpAssert( IsntErr( err ) );
			
			err = PGPGetPrefStringBuffer( prefRef, kPGPPrefOwnerName,
						sizeof( nameStr ) - 1, nameStr );
			pgpAssert( IsntErr( err ) );

			err = PGPGetPrefStringBuffer( prefRef, kPGPPrefLicenseNumber,
						sizeof( licenseStr ) - 1, licenseStr );
			pgpAssert( IsntErr( err ) );

			CopyCString( nameStr, str );
			strcat( str, "\r");
			strcat( str, companyStr );
			strcat( str, "\r" );
			strcat( str, licenseStr );
			CToPString( str, outLicenseStr );

			(void) PGPClosePrefFile( prefRef );
		}
		
		PFLFreeContext( pflContext );
	}
}

	void
CPGPtools::ShowAboutBox()
{
	Boolean		didShowDialog = FALSE;
	FSSpec		libraryFileSpec;
	CToolsError	err;
	
	// Open the libarary resource file at the top of the resource chain so
	// we get our pictures.
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
				CPGPStDialogHandler	aboutDialog(window_About, this);
				LWindow				*theDialog;
				MessageT			message;
				Boolean				credits = FALSE;
				Str255				licenseStr;
				
				const MessageT		kPGPButtonMessage		= 'bPGP';
				const MessageT		kCreditsButtonMessage	= 'bCre';
				const MessageT		kOKButtonMessage		= 'bOK ';
				
				const PaneIDT		kCreditsButtonPaneID	= 'bCre';
				const PaneIDT		kAboutPicturePaneID		= 'APIC';
				const PaneIDT		kCreditsPicturePaneID	= 'CPIC';
				const PaneIDT		kLicenseCaptionID		= 'cLic';

				theDialog = aboutDialog.GetDialog();
				
				didShowDialog = true;
				
				MakeLicenseStr( licenseStr );
				((LCaption *)theDialog->FindPaneByID(kLicenseCaptionID))->
						SetDescriptor( licenseStr );

				while( (message = aboutDialog.DoDialog()) != kOKButtonMessage)
				{
					if(message == kPGPButtonMessage )
					{
						Str255		url;
						
						GetIndString( url, kPGPtoolsMiscStringsResID,
									kPGPWebSiteURLStrIndex );
						
						err.err = OpenURL( url );
						if( err.IsError() )
						{
							Str255	errorStr;
							
							GetErrorString( err, errorStr );
							
							SysBeep( 1 );
							CWarningAlert::Display( kWACautionAlertType,
								kWAOKStyle,
								kErrorStringListResID,
								kCantLaunchWebBrowserBecauseStrIndex,
								errorStr );
						}
					}
					else if(message == kCreditsButtonPaneID)
					{
						Str255	buttonTitle;
						
						if( credits )
						{
							theDialog->FindPaneByID( kAboutPicturePaneID )->
										Show();
							theDialog->FindPaneByID( kCreditsPicturePaneID )->
										Hide();
							
							GetIndString( buttonTitle,
										kPGPtoolsMiscStringsResID,
										kAboutCreditsButtonTitleStrIndex );
						}
						else
						{
							theDialog->FindPaneByID( kCreditsPicturePaneID )->
										Show();
							theDialog->FindPaneByID( kAboutPicturePaneID )->
										Hide();
							
							GetIndString( buttonTitle,
										kPGPtoolsMiscStringsResID,
										kAboutInfoButtonTitleStrIndex );
						}
						
						((LControl *)theDialog->FindPaneByID(
									kCreditsButtonPaneID))->SetDescriptor(
									buttonTitle );

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


/*____________________________________________________________________________
	Create and enqueue a file task for the specified operation.
____________________________________________________________________________*/
	CToolsError
CPGPtools::CreateAndEnqueueFileTaskList(
	PGPtoolsOperation	operation,
	ushort				taskModifiers,
	const FSSpec *		inputSpecs,
	PGPUInt16			numSpecs )
{
	CToolsError			err;
	CPGPtoolsTaskList *	taskList	= NULL;
	
	taskList = new CPGPtoolsTaskList( operation );
	if( IsntNull( taskList ) )
	{
		taskList->SetTaskListHasFileTasks( TRUE );

		for( PGPUInt16 specIndex = 0; specIndex < numSpecs; ++specIndex )
		{
			const FSSpec *		srcSpec;
			CPGPtoolsTask *		task;
			
			srcSpec	= &inputSpecs[ specIndex ];
			AssertSpecIsValid( srcSpec, "CPGPtools::CreateAndEnqueueTask" );
			
			if ( operation == kPGPtoolsWipeOperation )
			{
				task = new CPGPtoolsWipeTask( srcSpec );
			}
			else
			{
				task = new CPGPtoolsFileTask( operation, srcSpec );
			}
			
			if( IsntNull( task ) )
			{
				task->SetTaskModifiers( taskModifiers );
				taskList->AppendTask( task );
			}
			else
			{
				err.err	= memFullErr;
				break;
			}
		}
		if ( err.IsntError( ) )
		{
			EnqueueTaskList( taskList );
		}
		else
		{
			delete	taskList;
			taskList	= NULL;
		}
	}
	else
	{
		err.err	= memFullErr;
	}
	return( err );
}


/*____________________________________________________________________________
	Create and enqueue a clipboard task for the specified operation.
____________________________________________________________________________*/
	CToolsError
CPGPtools::CreateAndEnqueueClipboardTask(
	PGPtoolsOperation	operation,
	ushort				taskModifiers )
{
	CToolsError					err;
	CPGPtoolsTaskList *			taskList	= NULL;
	CPGPtoolsClipboardTask *	task	= NULL;
	
	taskList	= new CPGPtoolsTaskList( operation );
	task		= new CPGPtoolsClipboardTask( operation );
	if( IsntNull( taskList ) && IsntNull( task ) )
	{
		taskList->SetTaskListHasFileTasks( FALSE );
		task->SetTaskModifiers( taskModifiers );
		taskList->AppendTask( task );

		EnqueueTaskList( taskList );
	}
	else
	{
		delete taskList;
		delete task;
		err.err	= memFullErr;
	}
	
	return( err );
}
	
/*____________________________________________________________________________
	Prompt the user for items to operate on, then create and enqueue a task
	for it.
____________________________________________________________________________*/
	void
CPGPtools::CreateAndEnqueueTaskPrompt(PGPtoolsOperation operation)
{
	FSSpec		srcFileSpec;
	CToolsError	err;
	Boolean		useClipboard;
	Str255		openButtonTitle;
	ushort		taskModifiers;
	
	taskModifiers	= GetAllModifiers();
	
	GetIndString(openButtonTitle, kPGPtoolsMiscStringsResID, kSelectStrIndex );
	
	err.err = GetFileOrClipboard( operation, openButtonTitle,
						&srcFileSpec, &useClipboard );
	
	if ( err.IsntError() )
	{
		if ( useClipboard )
		{
			err	= CreateAndEnqueueClipboardTask( operation, taskModifiers);
		}
		else
		{
			err	= CreateAndEnqueueFileTaskList( operation,
				taskModifiers, &srcFileSpec, 1 );	
		}
	}
	
	ReportTaskError( err );
}


	void
CPGPtools::ReportTaskError( CToolsError	err)
{
	if( ShouldReportError( err ) )
	{
		Str255	errorStr;
		
		GetErrorString( err, errorStr );
		
		SysBeep( 1 );
		CWarningAlert::Display( kWAStopAlertType, kWAOKStyle,
			kErrorStringListResID, kOperationCouldNotBeCompletedStrIndex,
			errorStr );
	}
}


	void
CPGPtools::ProcessNextTask()
{
	CPGPtoolsTaskList	*taskList;

	if( IsntPGPError( CreateContext( TRUE ) ) )
	{
		pgpAssert( IsntNull( mContext ) );

		taskList = (CPGPtoolsTaskList *) mTaskListQueue.NextGet();
		if( IsntNull( taskList ) )
		{
			taskList->ProcessTasks( mContext );
			delete( taskList );
		}
	
		/* If we were launched by PGPmenu, we may want to 
		quit now */
		if ( mAutoQuit && mTaskListQueue.IsEmpty())
		{ 
			Boolean	verifiedSigs;
			
			/* quit now, unless we have signatures the user
				will want to look at */
			verifiedSigs	= IsntNull( mResultsWindow ) &&
								mResultsWindow->HaveSignatures();
			if ( ! verifiedSigs )
			{
				ObeyCommand(cmd_Quit, NULL );
			}
			else
			{
				mMainWindow->Show();
				/* we have to stay around, so show toolbar */
				mAutoQuit	= FALSE;
			}
		}
		DisposeContext();
	}
	else
	{
		/* Clear out the task queue */
		taskList = (CPGPtoolsTaskList *) mTaskListQueue.NextGet();
		while( IsntNull( taskList ) )
		{
			delete( taskList );
			
			taskList = (CPGPtoolsTaskList *) mTaskListQueue.NextGet();
		}
	}
}



	void
CPGPtools::SpendTime(const EventRecord &inMacEvent)
{
	static Boolean	sBusy = FALSE;
	
	(void)inMacEvent;

	// A reentrancy check is needed because displaying a dialog
	// from this function can cause this LPeriodical to execute again.
	if( ! sBusy )
	{
		static Boolean	sFirstTime = TRUE;

		sBusy = TRUE;
		
		if( sFirstTime )
		{
			sFirstTime = FALSE;
			
			if( ! SecondaryVerifyEnvironment() )
				DoQuit();
		}
		
		// Process items in our deferred processing queue.
		
		if( ! mTaskListQueue.IsEmpty() )
		{
			// We have tasks to process. If we aren't the front process, set
			// ourselves to be so and try again later. It is necessary to
			// wait a few iterations of the main event loop so that
			// PowerPlant has settled down a bit after the context switch
			
			ProcessSerialNumber	appPSN;
			ProcessSerialNumber	frontPSN;
			Boolean				appIsInFront;
			static ushort		sWaitCount = 0;
			
			// Bring application to the front
			GetCurrentProcess( &appPSN );
			GetFrontProcess( &frontPSN );
			
			appIsInFront = TRUE;
			(void) SameProcess( &appPSN, &frontPSN, &appIsInFront );
			if( appIsInFront && sWaitCount == 0 )
			{
				(void)ProcessNextTask();
			}
			else if( sWaitCount == 0 )
			{
				// Wait three iterations of the main event loop after
				// calling  SetFrontProcess before processing the task list
				// items.
				sWaitCount = 3;
				SetFrontProcess( &appPSN );
			}
			else
			{
				--sWaitCount;
			}
		}
		else
		{
			GrafPtr	savePort;
			GrafPtr	wMgrPort;
			Point	mouseLoc;
			
			GetPort( &savePort );
			GetPort( &wMgrPort );
			SetPort( wMgrPort );
			
			GetMouse( &mouseLoc );
			
			if( mouseLoc.h != mLastMouseLocation.h ||
				mouseLoc.v != mLastMouseLocation.v )
			{
				(void) PGPGlobalRandomPoolAddMouse( mouseLoc.h, mouseLoc.v );
				mLastMouseLocation = mouseLoc;
			}
			
			SetPort( savePort );
		}
		
		sBusy = FALSE;
	}
}

	void
CPGPtools::EnqueueTaskList(CPGPtoolsTaskList *taskList)
{
	pgpAssert( IsntNull( taskList ) );	
	mTaskListQueue.NextPut( taskList );
}

	void
CPGPtools::Run()
{
	LApplication::Run();
	
	if( IsntNull( mMainWindow ) )
	{
		Rect	contentRect;
		
		contentRect = UWindows::GetWindowContentRect(
					mMainWindow->GetMacPort() );
		
		gPreferences.mainWindowLocation.v = contentRect.top;
		gPreferences.mainWindowLocation.h = contentRect.left;
	}

	if( IsntNull( mResultsWindow ) )
	{
		gPreferences.resultsWindowBounds = UWindows::GetWindowContentRect(
					mResultsWindow->GetMacPort() );
	}
}

	void
CPGPtools::StartUp(void)
{
	LApplication::StartUp();
	
	/* Disable idling to prevent reentrancy if CreateContext */
	/* displays a dialog */
	
	StopIdling();
	
	/* Notify user of keyring problems, if any */
	if( IsntPGPError( CreateContext( TRUE ) ) )
	{
		DisposeContext();
	}
	
	StartIdling();
}

//	Respond to 'odoc' AppleEvent events

	void
CPGPtools::HandleAppleEvent(
	const AppleEvent	&inAppleEvent,
	AppleEvent			&outAEReply,
	AEDesc				&outResult,
	long				inAENumber)
{
	OSErr	err;

#if 0
/* simulate opening from PGPmenu */
if ( inAENumber == ae_OpenApp &&
	(GetAllModifiers() & alphaLock) != 0 )
{
	pgpDebugPStr( "\psimulating ae_OpenedFromMenu" );
	inAENumber	= ae_OpenedFromMenu;
}
#endif

	switch( inAENumber )
	{
		case ae_OpenDoc:
		{
			if ( ! mUserLaunchedAppOnly )
				mAutoQuit	= TRUE; 
				
			err = DoOpenDocAppleEvent( inAppleEvent );
			ThrowIfOSErr_(err);
			break;
		}
		
		case ae_OpenedFromMenu:
			/* special event sent by PGPmenu to tell us it
			launched us */
			mAutoQuit	= TRUE;
			/* don't show main window */
			break;
			
		case ae_OpenApp:
			mUserLaunchedAppOnly	= TRUE;
			LApplication::HandleAppleEvent(inAppleEvent, outAEReply,
					outResult, inAENumber);
			mMainWindow->Show();
			break;
			
		case ae_Sign:
		case ae_Encrypt:
		case ae_EncryptAndSign:
		case ae_DecryptVerify:
		case ae_OptionSign:
		case ae_OptionEncrypt:
		case ae_OptionEncryptAndSign:
		case ae_OptionDecryptVerify:
		case ae_SecureWipe:
		{
			TArray<FSSpec>	theSpecsArray;
			
			GetFinderSelection( &theSpecsArray );
			RemoveChildAndDuplicateSpecsFromList( &theSpecsArray );
			ProcessFinderSelection( inAENumber, theSpecsArray );
			break;
		}
		
			
		default:
			LApplication::HandleAppleEvent(inAppleEvent, outAEReply,
								outResult, inAENumber);
			break;
	}
	
}

	void
CPGPtools::GetFinderSelection(
	TArray<FSSpec> *	outSpecsArray)
{
				
	try
	{
		StAEDescriptor		theReply;
		OSErr				err = noErr;

		// Send the appleevent
		{
			StAEDescriptor		theGetSelectionEvent;
			ProcessSerialNumber	finderPSN;
			StAEDescriptor		theAddress;

			// Create the AppleEvent
			ThrowIfNot_( FindProcess( kFinderType, kFinderCreator, &finderPSN,
					nil, nil, nil ) );
			err = ::AECreateDesc(	typeProcessSerialNumber,
									(Ptr) &finderPSN,
									sizeof( finderPSN ),
									theAddress);
			ThrowIfOSErr_( err );
			err = ::AECreateAppleEvent(	kAECoreSuite,
										kAEGetData,
										theAddress,
										kAutoGenerateReturnID,
										kAnyTransactionID,
										theGetSelectionEvent);
			ThrowIfOSErr_( err );

			// Create the specifier
			StAEDescriptor	nullDescriptor;
			StAEDescriptor	selectionProperty( (DescType) pUserSelection );
			StAEDescriptor	selectionSpec;
			
			err = ::CreateObjSpecifier(	cProperty,
										nullDescriptor,
										formPropertyID,
										selectionProperty,
										false,
										selectionSpec);
			ThrowIfOSErr_(err);

			// Add the direct object to the appleevent
			err = ::AEPutParamDesc(	theGetSelectionEvent,
									keyDirectObject,
									selectionSpec);
			ThrowIfOSErr_(err);
			
			StAEDescriptor	asFSSType((DescType) typeFSS);
			
			err = ::AEPutParamDesc(	theGetSelectionEvent,
									keyAERequestedType,
									asFSSType);
			ThrowIfOSErr_(err);
			
			UAppleEventsMgr::SendAppleEventWithReply(	theGetSelectionEvent,
														theReply,
														false);
		}				
		
		// Check for errors
		StAEDescriptor	errorCode(theReply, keyErrorNumber);
		
		if (errorCode.mDesc.descriptorType != typeNull)
		{
			UExtractFromAEDesc::TheInt16(errorCode, err);
			
			ThrowIfOSErr_(err);
		}
		
		// Walk the list adding all of the FSSpecs to the outFileSpecsH handle
		AEKeyword		keyWord;
		SInt32			numItems = 0;
		StAEDescriptor	theSelectionList(theReply, keyDirectObject);
		FSSpec			theSpec;
		
		ThrowIf_(theSelectionList.mDesc.descriptorType == typeNull);
		
		if (theSelectionList.mDesc.descriptorType == typeFSS)
		{
			Boolean	targetIsFolder;
			Boolean	targetWasAlias;
				
			pgpCopyMemory(	*theSelectionList.mDesc.dataHandle,
				&theSpec, sizeof(FSSpec));
				
			if( IsntErr( ResolveAliasFile( &theSpec, TRUE,
						&targetIsFolder, &targetWasAlias ) ) )
			{
				outSpecsArray->InsertItemsAt( 1, LArray::index_Last,
							&theSpec);
			}
		}
		else
		{
			err = ::AECountItems(theSelectionList, & numItems);
			ThrowIfOSErr_(err);
			for (SInt32 i = 1; i <= numItems; i++)
			{
				// Grab the item
				StAEDescriptor	item;
				Boolean			targetIsFolder;
				Boolean			targetWasAlias;
				
				err = ::AEGetNthDesc(	theSelectionList,
										i,
										typeFSS,
										&keyWord,
										item);
				ThrowIfOSErr_(err);
				
				// Insert it into the array
				pgpCopyMemory(	*item.mDesc.dataHandle, &theSpec,
							sizeof(theSpec));

				if( IsntErr( ResolveAliasFile( &theSpec, TRUE,
							&targetIsFolder, &targetWasAlias ) ) )
				{
					outSpecsArray->InsertItemsAt( 1, LArray::index_Last,
								&theSpec);
				}
			}
		}
		
		// Check for missing parms and dispose of the reply
		UAppleEventsMgr::CheckForMissedParams(theReply);
	}
	
	catch (...)
	{
	}
}


/*____________________________________________________________________________
	
____________________________________________________________________________*/
	static void
AddAllTrashFolders( TArray<FSSpec> *array )
{
	PGPUInt32		volIndex;
	short			vRefNums[ 100 ];
	unsigned short	numVRefNums;
	OSStatus		err	= noErr;
	
	numVRefNums	= sizeof( vRefNums ) / sizeof( vRefNums[ 0 ] );
	GetAllVRefNums( vRefNums, numVRefNums, &numVRefNums );
	for( volIndex = 0; volIndex < numVRefNums; ++volIndex )
	{
		short	vRefNum	= vRefNums[ volIndex ];
		short	trashVRefNum;
		long	trashDirID;
		
		err	= FindFolder( vRefNum, kTrashFolderType, kDontCreateFolder,
				&trashVRefNum, &trashDirID);
		if ( IsntErr( err ) )
		{
			FSSpec	spec;
			
			err	= FSMakeFSSpec( vRefNum, trashDirID, "\p", &spec );
			AssertNoErr( err, "AddAllTrashFolders" );
			if ( IsntErr( err ) )
			{
				array->InsertItemsAt( 1, LArray::index_Last, &spec);
			}
		}
	}
}

	void
CPGPtools::ProcessFinderSelection(
	long			inEventCode,
	TArray<FSSpec> &inSpecsArray)
{
	PGPtoolsOperation	op = kPGPtoolsInvalidOperation;
	ushort				modifiers	= 0;
	OSStatus			err;
	
	switch( inEventCode )
	{
		default:
			pgpDebugMsg( "\punknown operation" );
			op	= kPGPtoolsInvalidOperation;
			break;
			
		case ae_OptionSign:
			modifiers	|= optionKey;
		case ae_Sign:
			op	= kPGPtoolsSignOperation;
			break;
		
		case ae_OptionEncrypt:
			modifiers	|= optionKey;
		case ae_Encrypt:
			op = kPGPtoolsEncryptOperation;
			break;
		
		case ae_OptionEncryptAndSign:
			modifiers	|= optionKey;
		case ae_EncryptAndSign:
			op	= kPGPtoolsEncryptSignOperation;
			break;
		
		case ae_DecryptVerify:
			modifiers	|= optionKey;
		case ae_OptionDecryptVerify:
			op	= kPGPtoolsDecryptVerifyOperation;
			break;
		
		case ae_SecureWipe:
			op	= kPGPtoolsWipeOperation;
			break;
	}
	
	if ( AppCanDoOperation( op ) )
	{
		PGPUInt32		numSpecs;
		Boolean			isTrash	= FALSE;
		
		numSpecs	= inSpecsArray.GetCount();
	
		/* check to see if it's the trash folder */
		if ( numSpecs == 1 )
		{
			FSSpec			spec;
			CInfoPBRec		cpb;
			
			/* LArray starts with index 1 */
			inSpecsArray.FetchItemAt( 1, &spec);
			err	= FSpGetCatInfo( &spec, &cpb );
			if ( IsntErr( err ) && cpbIsFolder( &cpb ) &&
				FSIsTrashFolder( spec.vRefNum, cpb.dirInfo.ioDrDirID ) )
			{
				isTrash	= TRUE;
			}
		}
		
		if ( isTrash )
		{
			/* create specs for all trash folders */
			AddAllTrashFolders( &inSpecsArray );
			numSpecs	= inSpecsArray.GetCount();
		}

		Handle			itemsHandle	= inSpecsArray.GetItemsHandle();
		StHandleLocker	locker( itemsHandle );
		
		CreateAndEnqueueFileTaskList( op,
			modifiers,
			(const FSSpec *)*itemsHandle, numSpecs );
	}
}


	OSErr
CPGPtools::ProcessDragAndDrop(
	const FSSpec 	*specList,
	UInt32			numSpecs)
{
	OSErr	err = noErr;
	
	UInt32	numEncrypted		= 0;
	UInt32	numSigned			= 0;
	UInt32	numDetachedSigned	= 0;
	UInt32	numUnknown			= 0;
	
	for( UInt32 specIndex = 0; specIndex < numSpecs; specIndex++ )
	{
		PGPAnalyzeType	dataKind = kPGPAnalyze_Encrypted;
		
		if( IsErr( err ) )
			break;
	
		switch( dataKind )
		{
			case kPGPAnalyze_Encrypted:
				++numEncrypted;
				break;
			
			case kPGPAnalyze_Signed:
				++numSigned;
				break;

			case kPGPAnalyze_DetachedSignature:
				++numDetachedSigned;
				break;
			
			default:
				++numUnknown;
				break;
		}
	}
	
	if( IsntErr( err ) )
	{
		PGPtoolsOperation	operation;
			
		if( numUnknown == 0 )
		{
			// All of the files are deterministic. Go for decryption
			
			operation = kPGPtoolsDecryptVerifyOperation;
		}
		else
		{
			// We have unknown/unencrypted files. If we have none of our files,
			// then go for encryption, otherwise prompt for the operation
		
			if( numEncrypted == 0 &&
				numSigned == 0 &&
				numDetachedSigned == 0 )
			{
				operation = kPGPtoolsEncryptOperation;
			}
			else
			{
				operation = kPGPtoolsEncryptOperation;
			}	
		}
		
		if( IsntErr( err ) && AppCanDoOperation( operation ) )
		{
			CPGPtoolsTaskList	*taskList;
			
			taskList = new CPGPtoolsTaskList( operation );
			if( IsntNull( taskList ) )
			{
				PGPUInt32	specIndex;
				
				taskList->SetTaskListHasFileTasks( TRUE );

				for( specIndex = 0; specIndex < numSpecs; specIndex++ )
				{
					CPGPtoolsFileTask		*fileTask;
					
					fileTask = new CPGPtoolsFileTask( operation,
									&specList[specIndex] );
					if( IsntNull( fileTask ) )
					{
						taskList->AppendTask( fileTask );
					}
				}
				
				EnqueueTaskList( taskList );
			}
			else
			{
				err = memFullErr;
			}
		}
	}

Exit:

	return( err );
}

	OSErr
CPGPtools::DoOpenDocAppleEvent(const AppleEvent &inAppleEvent)
{
	AEDescList	docList;
	OSErr		err;
	
	err = AEGetParamDesc( &inAppleEvent, keyDirectObject, typeAEList,
				&docList );
	if( IsntErr( err ) )
	{
		Int32		numDocs;

		err = AECountItems( &docList, &numDocs );
		if( IsntErr( err ) )
		{
			FSSpec	*specList;
			
			// Loop through all items in the list, extract descriptor for
			// the document, and coerce descriptor data into a FSSpec
			
			specList = (FSSpec *) pgpAlloc( numDocs * sizeof( *specList ) );
			if( IsntNull( specList ) )
			{
				UInt32		numSpecs	= 0;
				CPGPFree	freeSpecs( specList );

				for (Int32 i = 1; i <= numDocs; i++)
				{
					AEKeyword	theKey;
					DescType	theType;
					Size		theSize;
				
					err = AEGetNthPtr( &docList, i, typeFSS, &theKey,
								&theType, &specList[ numSpecs ],
								sizeof(FSSpec), &theSize );
					if( IsErr( err ) )
						break;
					++numSpecs;
				}
				
				if( IsntErr( err ) && numSpecs != 0 )
				{
					err = ProcessDragAndDrop( specList, numSpecs );
				}
			}
			else
			{
				err = memFullErr;
			}
		}
		
		AEDisposeDesc( &docList );
	}
	
	return( err );
}

const OSType	kPGPkeysCreator	= 'pgpK';
	
	void
CPGPtools::LaunchPGPkeys(void)
{
	ProcessSerialNumber	keysPSN;
	CToolsError			err;
	
	// Check for already running app
	
	if( FindProcess( 'APPL', kPGPkeysCreator, &keysPSN, nil, nil, nil ) )
	{
		SetFrontProcess( &keysPSN );
	}
	else
	{
		FSSpec	toolsSpec;
		FSSpec	keysSpec;
		Boolean	foundKeys = false;
		
		if( IsntErr( GetSpecFromRefNum( mApplicationResFile, &toolsSpec ) ) )
		{
			foundKeys = FindApplicationOnVolume( toolsSpec.vRefNum, 'APPL',
								kPGPkeysCreator, &keysSpec );
		}
		
		if( ! foundKeys )
		{
			foundKeys = FindApplication( 'APPL', kPGPkeysCreator,
					kFindAppOnAllVolumes, &keysSpec );
		}
		
		if( foundKeys )
		{
			LaunchParamBlockRec	launchPB;

			pgpClearMemory( &launchPB, sizeof( launchPB ) );
		
			launchPB.launchBlockID			= extendedBlock;
			launchPB.launchEPBLength		= extendedBlockLen;
			launchPB.launchFileFlags		= 0;
			launchPB.launchControlFlags		= launchContinue |
												launchNoFileFlags;
			launchPB.launchAppSpec			= &keysSpec;
			launchPB.launchAppParameters	= nil;
			
			err.err = LaunchApplication( &launchPB );
		}
		else
		{
			err.err = fnfErr;
		}
	}
	
	if( err.IsError() )
	{
		Str255	errorStr;
		
		GetErrorString( err, errorStr );

		SysBeep( 1 );
		CWarningAlert::Display( kWACautionAlertType, kWAOKStyle,
			kErrorStringListResID, kCantLaunchPGPkeysBecauseStrIndex,
			errorStr );
	}
}


	Boolean
CPGPtools::AttemptQuitSelf(
	Int32	inSaveOption)
{
	Boolean	doQuit	= TRUE;
	
	(void)inSaveOption;
	
	return doQuit;
}


#if PGP_DEBUG	// [

#define	kDebugMenuID	10001	// Must not conflict with regular menus

	void
CPGPtools::InstallDebugMenu(void)
{
	LMenu	*debugMenu;
	
	// Need to suspend leaking because PowerPlant menu objects leak like crazy
	MacLeaks_Suspend();
	
	debugMenu = new( LMenu )( kDebugMenuID, "\pDEBUG" );
	if( debugMenu != nil )
	{
		debugMenu->InsertCommand( "\pTest du jour", cmd_DebugTest, 9999 );
		
		LMenuBar::GetCurrentMenuBar()->InstallMenu( debugMenu, 0 );
	}
	else
	{
		pgpDebugMsg( "InstallDebugMenu: Failed to create menu" );
	}

	MacLeaks_Resume();
}


#if 0	// KEEP [
#include "pgpLineEndIO.h"
#include "pgpIOUtilities.h"
#include "pgpFileUtilities.h"


	static void
TestLineEndIO(
	const FSSpec *		 fsSpec,
	ConvertLineEndType	lineEndType
	)
{
	PGPError		err;
	FSSpec			tempSpec;
	PFLFileSpecRef	inSpec	= kInvalidPGPKeySetRef;
	PFLFileSpecRef	outSpec	= kInvalidPGPKeySetRef;
	PGPIORef		inRef;
	PGPIORef		outRef;
	PGPLineEndIORef	convertRef;
	PFLContextRef	context;
	
	err	= PFLNewContext( &context );
	
	tempSpec	= *fsSpec;
	
	PFLNewFileSpecFromFSSpec( context, &tempSpec, &inSpec );
	switch( lineEndType )
	{
		case kCRLFLineEndType:
			AppendPString( "\p.CRLF", tempSpec.name );	break;
		case kCRLineEndType:
			AppendPString( "\p.CR", tempSpec.name );	break;
		case kLFLineEndType:
			AppendPString( "\p.LF", tempSpec.name );	break;
	}
	
	
	(void)FSpDelete( &tempSpec );
	(void)FSpCreate( &tempSpec, 'CWIE', 'TEXT', smSystemScript);
	PFLNewFileSpecFromFSSpec( context, &tempSpec, &outSpec );
	err	= PGPOpenFileSpec( inSpec, kPFLFileOpenFlags_ReadOnly,
			(PGPFileIORef *)&inRef );
	err	= PGPOpenFileSpec( outSpec, kPFLFileOpenFlags_ReadWrite,
			(PGPFileIORef *)&outRef );
	
	err	= PGPNewLineEndIO( outRef, FALSE, lineEndType, &convertRef );
	if (IsntPGPError( err ) )
	{
		PGPFileOffset	byteCount;
		
		PGPIOGetEOF( inRef, &byteCount );
		err	= PGPCopyIO( inRef, byteCount, (PGPIORef)convertRef );
	}
	
	PFLFreeFileSpec( inSpec );
	PFLFreeFileSpec( outSpec );
	PGPFreeIO( inRef );
	PGPFreeIO( (PGPIORef)convertRef );
	PGPFreeIO( outRef );
	PFLFreeContext( context );
}

#endif	// ] KEEP


#include "pgpGroups.h"
#include "pgpGroupsUtil.h"


	void
CPGPtools::DebugTest(void)
{
#if 0
	if( IsntPGPError( CreateContext( TRUE ) ) )
	{
		PGPError		err;
		PGPKeySetRef 	keySet;
		
		err = PGPOpenDefaultKeyRings( mContext, 0, &keySet );
		if( IsntPGPError( err ) )
		{
			PGPKeySetRef				recipients;
			PGPRecipientSettings		settings;
			PGPRecipientOptions			options;
			PGPKeySpec					keySpecs[10];
			PGPKeySetRef				newKeys;
			
			settings 	= kPGPRecipientSettingsDefault;
//			options		= kPGPRecipientOptionsAlwaysShowDialog;
			options		= 0;
			
			strcpy( keySpecs[0].userIDStr, "Employee 5" );
			strcpy( keySpecs[1].userIDStr, "Employee 4" );
			
			UDesktop::Deactivate();
			err = PGPRecipientDialog( mContext, keySet, 2,
					&keySpecs[0], options, settings, &settings,
					&recipients, nil, &newKeys );
			UDesktop::Activate();

			pgpAssertNoErr( err );
			
			if( IsntPGPError( err ) )
			{
				PGPUInt32	numKeys;
				Str255	str;
				
				(void)PGPCountKeys( recipients, &numKeys);
				
				PGPFreeKeySet( recipients );
				NumToString( numKeys, str );
				
			}
			
			if( PGPKeySetRefIsValid( keySet ) )
				PGPFreeKeySet( keySet );
			
			if( PGPKeySetRefIsValid( newKeys ) )
				PGPFreeKeySet( newKeys );
		}
		
		DisposeContext();
	}
#endif

#if 0
	if( IsntPGPError( CreateContext( TRUE ) ) )
	{
		PGPError		err;
		
		UDesktop::Deactivate();
		err = PGPOptionsDialog( mContext );
		UDesktop::Activate();

		pgpAssertNoErr( err );
		
		DisposeContext();
	}
#endif

}

#endif	// ] PGP_DEBUG

