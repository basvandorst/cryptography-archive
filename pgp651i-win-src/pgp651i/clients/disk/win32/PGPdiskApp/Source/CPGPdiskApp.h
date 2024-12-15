//////////////////////////////////////////////////////////////////////////////
// CPGPdiskApp.h
//
// Declaration of class CPGPdiskApp.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPGPdiskApp.h,v 1.24 1999/02/26 04:09:57 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CPGPdiskApp_h	// [
#define Included_CPGPdiskApp_h

#include "DualErr.h"
#include "LinkResolution.h"
#include "PGPdiskContainer.h"
#include "SecureString.h"
#include "SharedMemory.h"
#include "Win32Utils.h"
#include "WindowsVersion.h"

#include "CPGPdiskAppACI.h"
#include "CPGPdiskAppWinutils.h"


////////
// Types
////////

class CMainDialog;

// AppInstanceInfo stores information about a currently running instance of
// the PGPdisk application.

typedef struct AppInstanceInfo 
{
	PGPBoolean	commandLineMode;
	HWND		mainDialogHwnd;

	AppCommandInfo	ACI;

} AppInstanceInfo, *PAppInstanceInfo;


////////////////////
// Class CPGPdiskApp
////////////////////

// Class CPGPdiskApp is the MFC-required class representing the PGPdisk
// application itself.

class CPGPdiskApp : public CWinApp
{
public:
	// Public data members

	PAppInstanceInfo	mPAII;					// shared info about app
	CMainDialog			*mMainDialog;			// the main dialog
	PGPdiskContainer	mPGPdisks;				// PGPdisks container

	PGPBoolean			mCommandLineMode;		// have a command line?

	CString				mUserText;				// user name
	CString				mOrgText;				// organization name

	PGPBoolean			mAutoUnmount;			// auto-unmount enabled?
	PGPBoolean			mHotKeyEnabled;			// unmount hotkey enabled?
	PGPUInt16			mHotKeyCode;			// hot key codes
	PGPBoolean			mUnmountOnSleep;		// unmount on sleep?
	PGPBoolean			mNoSleepIfUnmountFail;	// no sleep unmount fails?
	PGPUInt32			mUnmountTimeout;		// mins b4 auto-unmount

	PGPUInt32			mPlatformMaxKbSize;		// max size PGPdisk
	PGPUInt32			mPlatformMinKbSize;		// min size PGPdisk

					CPGPdiskApp();

	// From CPGPdiskApp.cpp

	void			AbortPGPdiskApp();
	void			ExitPGPdiskApp();

	void			CallShowAboutBox();
	void			CallShowHelp();
	void			CallShowPrefs();

	DualErr			SetDriverPrefsFromApp();
	DualErr			CreatePGPdiskResidentApp();
	PGPBoolean		UpdatePGPdiskResident();

	PGPBoolean		FindAndShowNonCommandLinePGPdiskApp();
	PGPBoolean		FindAndShowSimilarPGPdiskApp(AppCommandInfo *pACI);

	void			SetSharedMemoryCommandInfo(AppCommandInfo *pACI);
	void			ClearSharedMemoryCommandInfo();

	// From CPGPdiskAppACI.cpp

	void			DispatchAppCommandInfo(PAppCommandInfo pACI);

	// From CPGPdiskAppPassphrase.cpp

	void			AskUserAddPassphrase();
	void			AskUserChangePassphrase();
	void			AskUserRemovePassphrase(PGPBoolean removeAll = FALSE);
	void			AskUserAddRemovePublicKeys();

	PGPUInt16		CalcPassphraseQuality(SecureString *passphrase);

	// From CPGPdiskAppRegistry.cpp

	DualErr			GetPersonalizationInfo();

	DualErr			GetRegistryPrefs();
	DualErr			SetRegistryPrefs();

	DualErr			SetRegistryPaths();

	// From CPGPdiskAppVolumes.cpp

	void			AskUserMountPGPdisk();
	void			AskUserUnmountPGPdisk();
	void			AskUserNewPGPdisk();
	void			AskUserGlobalConvertPGPdisks();

	DualErr			UpdateAppPGPdiskInfo();

	DualErr			ConvertPGPdiskPriorToMount(LPCSTR path, 
						PGPBoolean *pUserSaysNo);
	DualErr			ConvertPGPdiskFromGlobalWizard(LPCSTR path, 
						PGPBoolean *pUserSaysCancel);
	DualErr			ConvertPGPdisk(LPCSTR path);

	// From CPGPdiskAppWinutils.cpp

	PGPBoolean		AreAnyDriveLettersFree();

	PGPBoolean		IsDirectoryValid(LPCSTR path);
	PGPBoolean		IsFileValid(LPCSTR path);
	PGPBoolean		IsPathValid(LPCSTR path);
	PGPBoolean		IsVolumeValid(PGPUInt8 drive);

	PGPBoolean		IsVolumeFormatted(PGPUInt8 drive);

	PGPBoolean		IsDirectoryReadOnly(LPCSTR path);
	PGPBoolean		IsFileReadOnly(LPCSTR path);

	PGPBoolean		IsFileInUseByReader(LPCSTR path);
	PGPBoolean		IsFileInUseByWriter(LPCSTR path);
	PGPBoolean		IsFileInUse(LPCSTR path);

	DualErr			BruteForceTestIfEnoughSpace(LPCSTR dir, 
						PGPUInt32 kbLength, PGPBoolean *isEnoughSpace);
	DualErr			HowMuchFreeSpace(LPCSTR path, PGPUInt64 *bytesFree);

	static int		ExceptionFilter(int exception);
	DualErr			ShowWindowsFormatDialog(PGPUInt8 drive);

	void			TweakOnTopAttribute(CWnd *pWnd);

	BOOL			GetBitmapAndPalette(UINT nIDResource, CBitmap *bitmap, 
						CPalette *pal);

	void			PaintRegistrationInfo(CDC *pDC, PGPUInt32 x, PGPUInt32 y, 
						COLORREF textColor, CWnd *pWnd);
	void			PaintVersionInfo(CDC *pDC, PGPUInt32 x, PGPUInt32 y, 
						COLORREF textColor, CWnd *pWnd);

	void			GetTextRect(LPCSTR text, CWnd *pWnd, RECT *pRect);

	void			TruncateDisplayString(LPCSTR inString, 
						CString *outString, CDC *pDC, 
						PGPInt32 maxPixelLength);
	void			FitStringToWindow(LPCSTR inString, CString *outString, 
						CWnd *pWnd);
	void			FormatFitStringToWindow(LPCSTR inStr, LPCSTR subStr, 
						CString *outString, CWnd *pWnd);

	//{{AFX_VIRTUAL(CPGPdiskApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPGPdiskApp)
	//}}AFX_MSG

private:
	// Private data members

	SharedMemory mSharedAppInfoMem;		// shared app info memory


	// From CPGPdiskApp.cpp

	void					EnforceUniquePGPdiskGUI();
	void					CheckWindowsVersion();

#if PGPDISK_BETAVERSION
	void					CheckBetaTimeout();
#elif PGPDISK_DEMOVERSION
	void					CheckDemoTimeout();
#endif // PGPDISK_BETAVERSION

	void					BringThisPGPdiskAppToFront(HWND mainDialogHwnd);

	DualErr					InitRegistryKeys();

	DualErr					CreateMainDialog();
	void					DeleteMainDialog();

	DualErr					CreateAppInfoStruct();
	DualErr					DeleteAppInfoStruct();

	DualErr					ShowPGPdiskAboutBox();
	void					ShowPGPdiskHelp();
	DualErr					ShowPGPdiskPrefs();
	void					ShowSplashScreen();

	// From CPGPdiskAppACI.cpp

	PGPBoolean				AreAppCommandsSimilar(PAppCommandInfo pACI1, 
								PAppCommandInfo pACI2);

#if PGPDISK_BETAVERSION
	DualErr					CheckACIBetaWarnings(PAppCommandInfo pACI);
#elif PGPDISK_DEMOVERSION
	DualErr					CheckACIDemoWarnings(PAppCommandInfo pACI);
#endif // PGPDISK_BETAVERSION

	DualErr					CanonicalizeAppCommandInfo(PAppCommandInfo pACI);	

	DualErr					PerformPGPdiskConversionCheck(
								PAppCommandInfo pACI);

	// From CPGPdiskAppPassphrase.cpp

	DualErr					AddPGPdiskPassphrase(LPCSTR path);
	DualErr					ChangePGPdiskPassphrase(LPCSTR path);

	DualErr					RemovePGPdiskPassphrase(LPCSTR path, 
								PGPBoolean removeAll = FALSE);

	DualErr					AddRemovePGPdiskPublicKeys(LPCSTR path);

#if PGPDISK_WIPE_FUNCS

	DualErr					WipePassesOnAllPGPdisks(
								PGPBoolean doSilentWiping = FALSE);
	DualErr					WipePassesOnThisPGPdisk(LPCSTR path, 
								PGPBoolean doSilentWiping = FALSE);

#endif // PGPDISK_WIPE_FUNCS

	// From CPGPdiskAppVolumes.cpp

	DualErr					GlobalConvertPGPdisks();
	DualErr					CreatePGPdisk(LPCSTR defaultPath = "");
	DualErr					MountPGPdisk(LPCSTR path, 
								PGPBoolean forceReadOnly = FALSE, 
								PGPBoolean useDialog = TRUE, 
								PGPBoolean needMaster = FALSE, 
								PGPUInt8 drive = kInvalidDrive, 
								SecureString *passphrase = NULL);
	DualErr					UnmountPGPdisk(PGPUInt8 drive, 
								PGPBoolean isThisEmergency = FALSE);
	DualErr					UnmountAllPGPdisks(
								PGPBoolean isThisEmergency = FALSE);

	// From CPGPdiskAppWinutils.cpp

	HPALETTE				CreateDIBPalette(LPBITMAPINFO lpbmi, 
								LPINT lpiNumColors);

	DECLARE_MESSAGE_MAP()
};

#endif	// ] Included_CPGPdiskApp_h
