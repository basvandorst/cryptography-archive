/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPGPmenu.cp,v 1.14.8.1 1997/12/05 22:14:27 mhw Exp $
____________________________________________________________________________*/


#include <string.h>

#include <LActiveScroller.h>
#include <LGAPushButton.h>
#include <LGASecondaryBox.h>

#include "plugin.h"

#include "BETA.h"
#include "pgpLeaks.h"
#include "pgpMem.h"
#include "WarningAlert.h"
#include "MacStrings.h"
#include "pflPrefTypes.h"
#include "pflContext.h"
#include "pgpClientPrefs.h"
#include "PGPUtilities.h"
#include "pgpOpenPrefs.h"
#include "pgpSDKPrefs.h"
#include "PGPSharedEncryptDecrypt.h"
#include "pgpKeys.h"
#include "pgpMacCustomContextAlloc.h"

#include "CString.h"
#include "StSaveCurResFile.h"
#include "StSaveHeapZone.h"

#include "CmenuGetKeysPatch.h"
#include "CmenuMenuSelectPatch.h"
#include "CmenuSetMenuBarPatch.h"
#include "CmenuDrawMenuBarPatch.h"
#include "CmenuInsertMenuPatch.h"
#include "CmenuWaitNextEventPatch.h"
#include "CmenuPutScrapPatch.h"
#include "COutputDialog.h"
#include "CPGPmenu.h"

// Define the beta string
#if BETA
ConstStringPtr		kBetaHasExpired	=	"\pThis beta version of PGPmenu has"
										" expired and will now only decrypt"
										" and verify. Please contact the beta"
										" site or beta test coordinator for"
										" the latest version.";
#endif

// Define the demo string
#if PGP_DEMO
ConstStringPtr		kDemoHasExpired = 	"\pThank you for evaluating PGP. Your"
										" 30 day evaluation copy has expired."
										" You may continue to decrypt and"
										" verify messages for another 30"
										" days.";
#endif

// Globals
static SGlobals				sGlobals;
static UniversalProcPtr		sOldExitToShell =	nil;
static CPGPmenuPlug *		sPlugin			=	nil;
#if ! PGP_FINDERMENU
static PGPContextRef		sPGPContext		=	kPGPInvalidRef;
#endif

struct SMenuApps {
	PGPBoolean	privateScrap;
	PGPBoolean	useOutputDialog;
	OSType		creator;
	Str31		name;
};


// Constants
const SInt32	kSystem8 = 0x00000800;

// Termination routine declarations
#ifdef __cplusplus
	extern "C" {
#endif
	void __terminate(void);
#ifdef __cplusplus
	}
#endif



	pascal void
MyExitToShell()
{
	delete sPlugin;
	__terminate();
#if ! PGP_FINDERMENU

	if (sPGPContext != kPGPInvalidRef) {
		PGPFreeContext(sPGPContext);
	}
#endif
	::CallUniversalProc(	sOldExitToShell,
							kUPPExitToShellProcInfo);
}


	SInt32
main (
	ProcessInfoRec *	inProcInfoPtr,
	FSSpec *			inFSSpecPtr,
	Boolean				inPrivateScrap,
	Boolean				inUseOutputDialog)
{
	SInt32	result = noErr;
	
	// Set debug info
	SetDebugThrow_(debugAction_LowLevelDebugger);
	SetDebugSignal_(debugAction_LowLevelDebugger);
	
	try {
		// zero the globals
		pgpClearMemory(&sGlobals, sizeof(sGlobals));

#if ! PGP_FINDERMENU
		PGPError	pgpErr;
		
		pgpErr = pgpNewContextCustomMacAllocators( &sPGPContext);
		PGPThrowIfPGPErr_(pgpErr);
		
		sGlobals.privateScrap = inPrivateScrap;
		sGlobals.useOutputDialog = inUseOutputDialog;
		
		// Do we want to patch this app?
/*		if (CPGPmenuPlug::CaptureThisApp(inProcInfoPtr->processSignature,
		&sGlobals.privateScrap, &sGlobals.useOutputDialog)) {
*/
#endif
			// Create our plug object
			sPlugin = new CPGPmenuPlug(inProcInfoPtr, inFSSpecPtr);

			// Patch ExitToShell so that we properly unload our fragment
			UniversalProcPtr	newExitToShellUPP;

			sOldExitToShell = ::GetToolTrapAddress(_ExitToShell);
			newExitToShellUPP = (UniversalProcPtr) ::NewRoutineDescriptor(
									(ProcPtr) MyExitToShell,
									kUPPExitToShellProcInfo,
									GetCurrentArchitecture());
			if ((newExitToShellUPP == nil)
			|| (newExitToShellUPP == (UniversalProcPtr) MyExitToShell)) {
				PGPThrowOSErr_(kUnknownErr);
			}
			::SetToolTrapAddress(	(UniversalProcPtr) newExitToShellUPP,
									_ExitToShell);
/*#if ! PGP_FINDERMENU
		} else {
			result = kUnloadPlug;
			PGPFreeContext(sPGPContext);
		}
#endif
*/	}
	
	catch (CComboError & comboError) {
		result = kUnloadPlug;
	}
	
	catch (...) {
		result = kUnknownErr;
	}
	
	return result;
}



CPGPmenuPlug::CPGPmenuPlug(
	ProcessInfoRec *	inProcInfoPtr,	// Proc info for captured browser
	FSSpec *			inFSSpecPtr)	// FSSpec for this plugin
		: mGlobals(sGlobals)
{
	StSaveHeapZone		savedZone(::SystemZone());
	OSStatus			err = kNoErr;
	
	// Leaks stuff init
	pgpLeaksBeginSession("main");

// Handle beta and demo code
#if BETA || PGP_DEMO
	mGlobals.disableEncryptAndSign = false;
	
#if BETA
	if (BetaExpired()) {
		ConstStringPtr	theString = kBetaHasExpired;
#endif

#if PGP_DEMO
	PGPDemoState	theState = GetDemoState();
	
	if (theState == kDemoStateExpired) {
		PGPThrowOSErr_(kUnloadPlug);
	} else if (theState == kDemoStateEncryptionSigningDisabled) {
		ConstStringPtr theString = kDemoHasExpired;
#endif

		StringPtr	noteString = (StringPtr) ::NewPtr(theString[0] + 1);
		NMRecPtr	theNotificationRecP =
						(NMRecPtr) ::NewPtrClear(sizeof(NMRec));
		
		if ((noteString != nil) && (theNotificationRecP != nil)) {
			::BlockMoveData(	theString,
								noteString,
								theString[0] + 1);
			theNotificationRecP->qType = nmType;
			theNotificationRecP->nmSound = (Handle) -1;
			theNotificationRecP->nmStr = noteString;
			theNotificationRecP->nmResp = (NMUPP) -1;
			
			::NMInstall(theNotificationRecP);
		}
		mGlobals.disableEncryptAndSign = true;
	}
#endif

#if ! PGP_FINDERMENU
	// Save the PGPContextRef
	mGlobals.pgpContext = sPGPContext;
	PGPsdkLoadDefaultPrefs(sPGPContext);
#endif
	
	// Save away our FSSpec and ProcInfo
	mGlobals.processProcInfo = *inProcInfoPtr;
	mGlobals.processSpec = *inProcInfoPtr->processAppSpec;
	::BlockMoveData(	inProcInfoPtr->processName,
						mGlobals.processName,
						inProcInfoPtr->processName[0] + 1);
	mGlobals.processProcInfo.processAppSpec = &mGlobals.processSpec;
	mGlobals.processProcInfo.processName = mGlobals.processName;
	mGlobals.plugSpec = *inFSSpecPtr;
	
	// Find QDGlobals
	UQDGlobals::SetQDGlobals((QDGlobals*)(*((long*)::LMGetCurrentA5()) -
							(sizeof(QDGlobals) - sizeof(GrafPtr))));
	
	// Set the ColorQD environment variable
	Int32		qdVersion = gestaltOriginalQD;
	
	::Gestalt(gestaltQuickdrawVersion, &qdVersion);
	UEnvironment::SetFeature(	env_SupportsColor,
							 	(qdVersion > gestaltOriginalQD));
	
#if ! PGP_FINDERMENU
	// Register classes
	RegisterClass_(LGAPushButton);
	RegisterClass_(LActiveScroller);
	RegisterClass_(LTextEdit);
	RegisterClass_(COutputDialog);
	RegisterClass_(LColorEraseAttachment);
	
	CPGPEncoderDecoder::Init();
#endif
	
	// We've got all of the info we need from the app, so we now open our
	// resources
	LFile	thePlugFile(mGlobals.plugSpec);
	
	thePlugFile.OpenResourceFork(fsRdWrPerm);
	
	// Add patches
	new CmenuDrawMenuBarPatch(mGlobals);
	new CmenuInsertMenuPatch(mGlobals);
	new CmenuSetMenuBarPatch(mGlobals);
	new CmenuMenuSelectPatch(mGlobals);
	new CmenuWaitNextEventPatch(mGlobals);

#if ! PGP_FINDERMENU
	new CmenuPutScrapPatch(mGlobals);
	new CmenuGetKeysPatch(mGlobals);
#endif
	
	thePlugFile.CloseResourceFork();
};



CPGPmenuPlug::~CPGPmenuPlug()
{
#if ! PGP_FINDERMENU
	CPGPEncoderDecoder::Cleanup();
#endif
}



	void
CPGPmenuPlug::ShowError(
	PGPError	inErr)
{
	CString		theErrorString;
	
	PGPGetErrorString(	inErr,
						theErrorString.GetBufferSize() + 1,
						theErrorString);
	WarningAlert(	kWAStopAlertType,
					kWAOKStyle,
					STRx_Errors,
					kErrorStringID,
					CString(inErr),
					theErrorString);
}


#if ! PGP_FINDERMENU
	Boolean
CPGPmenuPlug::CaptureThisApp(
	OSType		inCreator,
	Boolean *	outPrivateScrap,
	Boolean *	outUseOutputDialog)
{
	Boolean			result = false;
	SMenuApps *		apps = nil;
	PGPPrefRef		prefRef = kPGPInvalidRef;
	PFLContextRef	pflContext = kPGPInvalidRef;
	
	try {
		PGPSize			len;
		UInt32			numApps;
		PGPError		pgpErr;

		pgpErr = PFLNewContext(&pflContext);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPOpenClientPrefs(pflContext, &prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPGetPrefData(	prefRef, 
									kPGPPrefPGPmenuMacAppSignatures, 
									&len, 
									&apps);
		PGPThrowIfPGPErr_(pgpErr);
		
		numApps = len / sizeof(SMenuApps);
		
		for (UInt32 x = 0; x < numApps; x++) {
			if (apps[x].creator == inCreator) {
				*outPrivateScrap = apps[x].privateScrap;
				*outUseOutputDialog = apps[x].useOutputDialog;
				result = true;
				break;
			}
		}
	}
	
	catch (...) {
	}
	
	if (apps != nil) {
		PGPDisposePrefData(	prefRef, 
							apps);

	}

	if (prefRef != kPGPInvalidRef) {
		PGPClosePrefFile(prefRef);
	}
	
	if (pflContext != kPGPInvalidRef) {
		PFLFreeContext(pflContext);
	}
	
	return result;
}



#endif
