/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CPGPemailer.cp,v 1.12.8.1 1997/12/05 22:13:59 mhw Exp $
____________________________________________________________________________*/

#include <string.h>

#include "plugin.h"

#include <LToggleButton.h>

#include "BETA.h"
#include "pgpLeaks.h"
#include "pgpMem.h"
#include "PGPUtilities.h"
#include "pgpOpenPrefs.h"
#include "pgpMacCustomContextAlloc.h"
#include "pgpSDKPrefs.h"

#include "MacFiles.h"

//#include "Utilities.h"
#include "UPGPException.h"
#include "StSaveCurResFile.h"
#include "CemailerSizeWindowPatch.h"
#include "CemailerCloseWindowPatch.h"
#include "CemailerWaitNextEventPatch.h"
#include "CemailerEndUpdatePatch.h"
#include "CemailerSetWTitlePatch.h"
#include "CPGPemailerView.h"
#include "CPGPemailer.h"

#ifdef powerc
// Define our entry point
ProcInfoType __procinfo = uppPlugInMainProcInfo;
#endif

// Define the beta string
#if BETA
ConstStringPtr		kBetaHasExpired	=	"\pThis beta version of PGPemailer"
										" has expired and will now only"
										" decrypt and verify. Please contact"
										" the beta site or beta test"
										" coordinator for the latest"
										" version.";
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
static UniversalProcPtr		sOldExitToShell =	nil;
static CPGPemailerPlug *	sPlugin			=	nil;
static PGPContextRef		sPGPContext		=	kPGPInvalidRef;


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
	if (sPGPContext != kPGPInvalidRef) {
		PGPFreeContext(sPGPContext);
	}
	::CallUniversalProc(	sOldExitToShell,
							kUPPExitToShellProcInfo);
}



	SInt32
main (
	ProcessInfoRec *	inProcInfoPtr,
	FSSpec *			inFSSpecPtr)
{
	// Set debug info
	SetDebugThrow_(debugAction_LowLevelDebugger);
	SetDebugSignal_(debugAction_LowLevelDebugger);
	
	// Leaks stuff init
//	pgpLeaksBeginSession("main");

	try {
		PGPError	pgpErr;
		
		pgpErr = pgpNewContextCustomMacAllocators( &sPGPContext );
		PGPThrowIfPGPErr_(pgpErr);
		PGPsdkLoadDefaultPrefs(sPGPContext);


		// Create our plug object
		sPlugin = new CPGPemailerPlug(inProcInfoPtr, inFSSpecPtr);

		// Patch ExitToShell so that we properly unload our fragment
		UniversalProcPtr	newExitToShellUPP;

		sOldExitToShell = ::GetToolTrapAddress(_ExitToShell);
		newExitToShellUPP = (UniversalProcPtr) :: NewRoutineDescriptor(
													(ProcPtr) MyExitToShell,
													kUPPExitToShellProcInfo,
													GetCurrentArchitecture());
		if ((newExitToShellUPP == nil)
		|| (newExitToShellUPP == (UniversalProcPtr) MyExitToShell)) {
			PGPThrowOSErr_(kUnknownErr);
		}
		::SetToolTrapAddress(	(UniversalProcPtr) newExitToShellUPP,
								_ExitToShell);
	
	}
	
	catch (CComboError & comboError) {
		return kUnloadPlug;
	}
	
	catch (...) {
		return kUnknownErr;
	}
	
	return kNoErr;
}



CPGPemailerPlug::CPGPemailerPlug(
	ProcessInfoRec *	inProcInfoPtr,	// Proc info for captured browser
	FSSpec *			inFSSpecPtr)	// FSSpec for this plugin
{
	OSStatus		err = kNoErr;

// Handle beta code
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

	// Save the PGPContextRef
	mGlobals.pgpContext = sPGPContext;
	
	// Set the curresfile back to app
	err = FSpFindOpenForkRef(	inProcInfoPtr->processAppSpec,
								true,
								&mGlobals.appResForkRef);
	PGPThrowIfOSErr_(err);
	
	StSaveCurResFile	setToAppRes(mGlobals.appResForkRef);
	
	// Test 'vers' info
	StResource	theVers('vers', 1);

	mGlobals.versInfo = (**((VersRecHndl) theVers.mResourceH)).numericVersion;
	PGPThrowIf_(mGlobals.versInfo.majorRev != 2);

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
	UQDGlobals::SetQDGlobals((QDGlobals*)(*((long*)::LMGetCurrentA5())
					- (sizeof(QDGlobals) - sizeof(GrafPtr))));
	
	// Set the ColorQD environment variable
	Int32		qdVersion = gestaltOriginalQD;
	
	::Gestalt(gestaltQuickdrawVersion, &qdVersion);
	UEnvironment::SetFeature(	env_SupportsColor,
							 	(qdVersion > gestaltOriginalQD));

	// Register panes
	URegistrar::RegisterClass(	CPGPemailerView::class_ID,
			(ClassCreatorFunc) CPGPemailerView::CreateCPGPemailerViewStream);
	URegistrar::RegisterClass(LButton::class_ID,
							(ClassCreatorFunc) LButton::CreateButtonStream);
	URegistrar::RegisterClass(LView::class_ID,
							(ClassCreatorFunc) LView::CreateViewStream);
	
	// Initialize PGPEncoderDecoder
	CPGPEncoderDecoder::Init();
	
	// We've got all of the info we need from the browser, so we now open our
	// resources
	LFile	thePlugFile(mGlobals.plugSpec);
	
	thePlugFile.OpenResourceFork(fsRdWrPerm);
	
	// Add patches
	new CemailerSetWTitlePatch(mGlobals);
	new CemailerWaitNextEventPatch(mGlobals);
	new CemailerEndUpdatePatch(mGlobals);
	new CemailerCloseWindowPatch(mGlobals);
	new CemailerSizeWindowPatch(mGlobals);

	thePlugFile.CloseResourceFork();
};



CPGPemailerPlug::~CPGPemailerPlug()
{
	// Cleanup after ourselves
	CPGPEncoderDecoder::Cleanup();
}
