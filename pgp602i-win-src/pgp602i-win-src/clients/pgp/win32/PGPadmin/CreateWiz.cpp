/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: CreateWiz.cpp,v 1.66.8.1 1998/11/12 03:13:04 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "PGPadmin.h"
#include "resource.h"
#include "CreateWiz.h"
#include "FindInstaller.h"
#include "CopyInstaller.h"
#include "Utils.h"
#include "pflPrefs.h"
#include "pflPrefTypes.h"
#include "pgpMem.h"
#include "pgpFileSpec.h"
#include "pgpKeys.h"
#include "pgpAdminPrefs.h"
#include "pgpUtilities.h"
#include "pgpVersion.h"
#include "PGPcl.h"

extern "C" {
#include "pgpVersionHeader.h"
};

static pgpConfigInfo Config;

static HPALETTE 
CreateDIBPalette (LPBITMAPINFO lpbmi, 
				  LPINT lpiNumColors);

static HBITMAP 
LoadResourceBitmap (HINSTANCE hInstance, 
					LPSTR lpString,
					HPALETTE FAR* lphPalette);

#define DEFAULT_USERINSTALLDIR "C:\\Program Files\\Network Associates\\PGP60"

void CreateWiz(HWND hwndMain)
{
	PROPSHEETPAGE	pspWiz[NUM_WIZ_PAGES];
	PROPSHEETHEADER pshWiz;
	int				nIndex;
	int iNumBits,	iBitmap;
	HDC				hDC;
	char			szTitle[255];
	char			szMsg[255];
	char			szClientInstaller[MAX_PATH];
	char			szDisk[255];
	char			szPGPPath[MAX_PATH];
	HANDLE			hFind;
	WIN32_FIND_DATA findData;
	BOOL			bCancel;
	PGPError		err;

	// Make sure SDK isn't expired 

	err = PGPNewContext(kPGPsdkAPIVersion, &(Config.pgpContext));
	if (IsPGPError(err))
	{
		if (err == kPGPError_FeatureNotAvailable)
		{
			LoadString(g_hInstance, IDS_E_EXPIRED, szMsg, 254);
			LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
			MessageBox(hwndMain, szMsg, szTitle, MB_ICONWARNING);
		}
		else
			PGPclErrorBox(hwndMain, err);

		g_bGotReloadMsg = FALSE;
		return;
	}

	err = PGPclInitLibrary(Config.pgpContext);
	if (IsPGPError(err))
	{
		PGPclErrorBox (hwndMain, err);
		g_bGotReloadMsg = FALSE;
		return;
	}

	// Check for beta/demo expiration

	if (PGPclIsExpired(hwndMain) != kPGPError_NoErr)
	{
		g_bGotReloadMsg = FALSE;
		return;
	}
	
	// Check to make sure the ClientInstaller directory
	// is present in the PGP directory
	
	PGPclGetPGPPath(szPGPPath, MAX_PATH-1);
	LoadString(g_hInstance, IDS_CLIENTINSTALLER, szClientInstaller,
		MAX_PATH-1);
	LoadString(g_hInstance, IDS_INSTALLDISK, szDisk, 254);
	strcat(szPGPPath, szClientInstaller);
	strcpy(szClientInstaller, szPGPPath);
	strcat(szPGPPath, szDisk);

	bCancel = FALSE;
	LoadString(g_hInstance, IDS_E_CLIENTINSTALLER, szMsg, 254);
	LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
	do
	{
		hFind = FindFirstFile(szPGPPath, &findData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			MessageBox(hwndMain, szMsg, szTitle, MB_ICONEXCLAMATION);

			bCancel = DialogBoxParam(g_hInstance, 
						MAKEINTRESOURCE(IDD_FINDINSTALLER),
						hwndMain, (DLGPROC) FindInstallerDlgProc, 
						(LPARAM) szClientInstaller);

			strcpy(szPGPPath, szClientInstaller);
			strcat(szPGPPath, szDisk);
		}
	}
	while ((hFind == INVALID_HANDLE_VALUE) && (bCancel == FALSE));

	if (bCancel)
	{
		g_bGotReloadMsg = FALSE;
		if (Config.pgpContext)
			PGPFreeContext(Config.pgpContext);

		return;
	}
	else 
		FindClose(hFind);

	// Set defaults here:

	Config.memoryMgr = PGPGetContextMemoryMgr(Config.pgpContext);
	Config.hBitmap = NULL;
	Config.hPalette = NULL;
	Config.szLicenseNum = NULL;
	Config.bUseOutgoingADK = FALSE;
	Config.bUseIncomingADK = FALSE;
	Config.bUseDiskADK = FALSE;
	Config.szOutgoingADKID = NULL;
	Config.szOutgoingADK = NULL;
	Config.szIncomingADKID = NULL;
	Config.szIncomingADK = NULL;
	Config.szDiskADKID = NULL;
	Config.szDiskADK = NULL;
	Config.bEnforceIncomingADK = FALSE;
	Config.bEnforceOutgoingADK = FALSE;
	Config.bEnforceRemoteADK = TRUE;
	Config.bEnforceMinChars = FALSE;
	Config.nMinChars = 8;
	Config.bEnforceMinQuality = FALSE;
	Config.nMinQuality = 20;
	Config.szCorpKeyID = NULL;
	Config.szCorpKey = NULL;
	Config.corpKeyType = (PGPPublicKeyAlgorithm) 0;
	Config.outgoingADKKeyType = (PGPPublicKeyAlgorithm) 0;
	Config.diskADKKeyType = (PGPPublicKeyAlgorithm) 0;
	Config.bWarnNotCertByCorp = FALSE;
	Config.bAutoSignTrustCorp = FALSE;
	Config.bMetaIntroducerCorp = FALSE;
	Config.bAutoAddRevoker = FALSE;
	Config.szRevokerKeyID = NULL;
	Config.szRevokerKey = NULL;
	Config.revokerKeyType = (PGPPublicKeyAlgorithm) 0;
	Config.bAllowKeyGen = TRUE;
	Config.bAllowRSAKeyGen = FALSE;
	Config.nMinKeySize = 768;
	Config.bUpdateAllKeys = FALSE;
	Config.bUpdateTrustedIntroducers = FALSE;
	Config.nDaysUpdateAllKeys = 1;
	Config.nDaysUpdateTrustedIntroducers = 1;
	Config.bAllowConventionalEncryption = TRUE;
	Config.szComments = NULL;
	Config.nCommentLength = 0;
	Config.defaultKeySet = NULL;
	Config.szInstallDir = (char *) PGPNewData(Config.memoryMgr,
									strlen("C:\\")+1, 
									kPGPMemoryMgrFlags_Clear);
	strcpy(Config.szInstallDir, "C:\\");
	Config.bCopyClientPrefs = FALSE;
	Config.bPreselectInstall = FALSE;
	Config.szUserInstallDir = (char *) PGPNewData(Config.memoryMgr,
										strlen(DEFAULT_USERINSTALLDIR)+1, 
										kPGPMemoryMgrFlags_Clear);
	strcpy(Config.szUserInstallDir, DEFAULT_USERINSTALLDIR);
	Config.bInstallPrograms = TRUE;
	Config.bInstallDisk = TRUE;
	Config.bInstallEudora = TRUE;
	Config.bInstallOutlook = TRUE;
	Config.bInstallOutExpress = TRUE;
	Config.bInstallManual = TRUE;
	Config.bUninstallOld = TRUE;
	Config.bSave = FALSE;

	// Determine which bitmap will be displayed in the wizard

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1)
		iBitmap = IDB_ADMINWIZ1;
	else if (iNumBits <= 4) 
		iBitmap = IDB_ADMINWIZ4;
	else if (iNumBits <= 8) 
		iBitmap = IDB_ADMINWIZ8;
	else 
		iBitmap = IDB_ADMINWIZ24;

	Config.hBitmap = LoadResourceBitmap(g_hInstance, MAKEINTRESOURCE(iBitmap),
						&(Config.hPalette));

	// Set the values common to all pages

	for (nIndex=0; nIndex<NUM_WIZ_PAGES; nIndex++)
	{
		pspWiz[nIndex].dwSize		= sizeof(PROPSHEETPAGE);
		pspWiz[nIndex].dwFlags		= PSP_DEFAULT;
		pspWiz[nIndex].hInstance	= g_hInstance;
		pspWiz[nIndex].pszTemplate	= NULL;
		pspWiz[nIndex].hIcon		= NULL;
		pspWiz[nIndex].pszTitle		= NULL;
		pspWiz[nIndex].pfnDlgProc	= NULL;
		pspWiz[nIndex].lParam		= (LPARAM) &Config;
		pspWiz[nIndex].pfnCallback	= NULL;
		pspWiz[nIndex].pcRefParent	= NULL;
	}

	// Set up the license number page

	pspWiz[Wiz_License].pszTemplate	= MAKEINTRESOURCE(IDD_LICENSE);
	pspWiz[Wiz_License].pfnDlgProc	= (DLGPROC) LicenseDlgProc;
	
	// Set up the intro page

	pspWiz[Wiz_Intro].pszTemplate	= MAKEINTRESOURCE(IDD_INTRO);
	pspWiz[Wiz_Intro].pfnDlgProc	= (DLGPROC) IntroDlgProc;
	
	// Set up the additional decryption key intro page

	pspWiz[Wiz_ADKIntro].pszTemplate = MAKEINTRESOURCE(IDD_ADK);
	pspWiz[Wiz_ADKIntro].pfnDlgProc	= (DLGPROC) ADKIntroDlgProc;
	
	// Set up the incoming additional decryption key page

	pspWiz[Wiz_ADKIncoming].pszTemplate	= MAKEINTRESOURCE(IDD_ADK_INCOMING);
	pspWiz[Wiz_ADKIncoming].pfnDlgProc	= (DLGPROC) ADKIncomingDlgProc;
	
	// Set up the incoming additional decryption key selection page

	pspWiz[Wiz_ADKInSelect].pszTemplate =MAKEINTRESOURCE(IDD_ADK_INSEL);
	pspWiz[Wiz_ADKInSelect].pfnDlgProc  =(DLGPROC) ADKInSelectDlgProc;
	
	// Set up the outgoing additional decryption key page

	pspWiz[Wiz_ADKOutgoing].pszTemplate = 
										MAKEINTRESOURCE(IDD_ADK_OUTGOING);
	pspWiz[Wiz_ADKOutgoing].pfnDlgProc  = (DLGPROC) ADKOutgoingDlgProc;
	
	// Set up the outgoing additional decryption key selection page

	pspWiz[Wiz_ADKOutSelect].pszTemplate = 
										MAKEINTRESOURCE(IDD_ADK_OUTSEL);
	pspWiz[Wiz_ADKOutSelect].pfnDlgProc = (DLGPROC) ADKOutSelectDlgProc;
	
	// Set up the additional decryption key enforcement page

	pspWiz[Wiz_ADKEnforce].pszTemplate = 
										MAKEINTRESOURCE(IDD_ADK_ENFORCE);
	pspWiz[Wiz_ADKEnforce].pfnDlgProc = (DLGPROC) ADKEnforceDlgProc;
	
	// Set up the PGPdisk additional decryption key page

	pspWiz[Wiz_ADKDisk].pszTemplate = MAKEINTRESOURCE(IDD_ADK_DISK);
	pspWiz[Wiz_ADKDisk].pfnDlgProc  = (DLGPROC) ADKDiskDlgProc;
	
	// Set up the PGPdisk additional decryption key selection page

	pspWiz[Wiz_ADKDiskSelect].pszTemplate = 
										MAKEINTRESOURCE(IDD_ADK_DISKSEL);
	pspWiz[Wiz_ADKDiskSelect].pfnDlgProc = (DLGPROC) ADKDiskSelectDlgProc;
	
	// Set up the pass phrase page

	pspWiz[Wiz_PassPhrase].pszTemplate	= MAKEINTRESOURCE(IDD_PASSPHRASE);
	pspWiz[Wiz_PassPhrase].pfnDlgProc	= (DLGPROC) PassPhraseDlgProc;
	
	// Set up the corporate key page

	pspWiz[Wiz_CorpKey].pszTemplate = MAKEINTRESOURCE(IDD_CORPKEY);
	pspWiz[Wiz_CorpKey].pfnDlgProc	= (DLGPROC) CorpKeyDlgProc;
	
	// Set up the corporate key selection page

	pspWiz[Wiz_CorpKeySelect].pszTemplate	= MAKEINTRESOURCE(IDD_CORPKEYSEL);
	pspWiz[Wiz_CorpKeySelect].pfnDlgProc	= (DLGPROC) CorpKeySelectDlgProc;
	
	// Set up the revoker key page

	pspWiz[Wiz_Revoker].pszTemplate = MAKEINTRESOURCE(IDD_REVOKER);
	pspWiz[Wiz_Revoker].pfnDlgProc	= (DLGPROC) RevokerDlgProc;
	
	// Set up the revoker key selection page

	pspWiz[Wiz_RevokerSelect].pszTemplate	= MAKEINTRESOURCE(IDD_REVOKERSEL);
	pspWiz[Wiz_RevokerSelect].pfnDlgProc	= (DLGPROC) RevokerSelectDlgProc;
	
	// Set up the key generation page

	pspWiz[Wiz_KeyGen].pszTemplate	= MAKEINTRESOURCE(IDD_KEYGEN);
	pspWiz[Wiz_KeyGen].pfnDlgProc	= (DLGPROC) KeyGenDlgProc;
	
	// Set up the default key selection page

	pspWiz[Wiz_DefaultKeys].pszTemplate = MAKEINTRESOURCE(IDD_DEFKEYSEL);
	pspWiz[Wiz_DefaultKeys].pfnDlgProc	= (DLGPROC) DefaultKeysDlgProc;
	
	// Set up the server updates page

	pspWiz[Wiz_ServerUpdates].pszTemplate = 
										MAKEINTRESOURCE(IDD_SERVERUPDATES);
	pspWiz[Wiz_ServerUpdates].pfnDlgProc  = (DLGPROC) ServerUpdatesDlgProc;
	
	// Set up the miscellaneous page

	pspWiz[Wiz_Misc].pszTemplate	= MAKEINTRESOURCE(IDD_MISC);
	pspWiz[Wiz_Misc].pfnDlgProc		= (DLGPROC) MiscDlgProc;
	
	// Set up the review page

	pspWiz[Wiz_Review].pszTemplate	= MAKEINTRESOURCE(IDD_REVIEW);
	pspWiz[Wiz_Review].pfnDlgProc	= (DLGPROC) ReviewDlgProc;
	
	// Set up the client prefs page

	pspWiz[Wiz_ClientPrefs].pszTemplate	= MAKEINTRESOURCE(IDD_CLIENTPREFS);
	pspWiz[Wiz_ClientPrefs].pfnDlgProc	= (DLGPROC) ClientPrefsDlgProc;
	
	// Set up the install options page

	pspWiz[Wiz_InstallOptions].pszTemplate =
										MAKEINTRESOURCE(IDD_INSTALLOPTIONS);
	pspWiz[Wiz_InstallOptions].pfnDlgProc  = (DLGPROC) InstallOptionsDlgProc;
	
	// Set up the finishing page

	pspWiz[Wiz_Finish].pszTemplate	= MAKEINTRESOURCE(IDD_FINISH);
	pspWiz[Wiz_Finish].pfnDlgProc	= (DLGPROC) FinishDlgProc;
	
	// Create the header

	pshWiz.dwSize		= sizeof(PROPSHEETHEADER);
	pshWiz.dwFlags		= PSH_WIZARD | PSH_PROPSHEETPAGE;
	pshWiz.hwndParent	= hwndMain;
	pshWiz.hInstance	= g_hInstance;
	pshWiz.hIcon		= NULL;
	pshWiz.pszCaption	= NULL;
	pshWiz.nPages		= NUM_WIZ_PAGES;
	pshWiz.nStartPage	= Wiz_Start;
	pshWiz.ppsp			= pspWiz;
	pshWiz.pfnCallback	= NULL;

	// Execute the Wizard - doesn't return until Cancel or Save

	PropertySheet(&pshWiz);

	// Save settings

	if (Config.bSave)
	{
		PFLFileSpecRef  adminPrefFile;
		PGPPrefRef		prefRef;
		PGPKeyID		keyID;
		PGPByte			exportedKeyID[kPGPMaxExportedKeyIDSize];
		PGPSize			keyIDSize;
		char			szFile[MAX_PATH];
		char			szPrefFile[MAX_PATH];
		char			szOrigFile[MAX_PATH];
		char			szNewFile[MAX_PATH];
		char			szInstallDir[MAX_PATH];
		char			szDisk[255];

		// Add a directory to the install directory

		CreateDirectory(Config.szInstallDir, NULL);
		LoadString(g_hInstance, IDS_INSTALLDIR, szPrefFile, MAX_PATH-1);

		wsprintf(szInstallDir, szPrefFile, 
			Config.szInstallDir, PGPVERSIONSTRING);

		PGPFreeData(Config.szInstallDir);
		Config.szInstallDir = (char *) PGPNewData(Config.memoryMgr,
										strlen(szInstallDir)+1, 
										kPGPMemoryMgrFlags_Clear);

		strcpy(Config.szInstallDir, szInstallDir);
		CreateDirectory(Config.szInstallDir, NULL);

		LoadString(g_hInstance, IDS_INSTALLDISK, szDisk, 254);
		strcpy(szNewFile, Config.szInstallDir);
		strcat(szNewFile, szDisk);
		CreateDirectory(szNewFile, NULL);

		if (Config.bCopyClientPrefs)
		{
			// Copy default prefs to install directory
			
			LoadString(g_hInstance, IDS_CLIENTPREFFILE, szPrefFile, 
				MAX_PATH-1);
			
			PGPclGetPGPPath(szOrigFile, MAX_PATH-1);
			
			strcat(szNewFile, "\\");
			strcat(szOrigFile, szPrefFile);
			strcat(szNewFile, szPrefFile);
			
			CopyFile(szOrigFile, szNewFile, FALSE);
		}

		// Save the admin prefs

		LoadString(g_hInstance, IDS_ADMINPREFFILE, szPrefFile, MAX_PATH-1);

		strcpy(szFile, Config.szInstallDir);
		strcat(szFile, szDisk);
		strcat(szFile, "\\");
		PGPclGetPGPPath(szOrigFile, MAX_PATH-1);
		strcat(szFile, szPrefFile);
		strcat(szOrigFile, szPrefFile);

		err = PFLNewFileSpecFromFullPath(Config.memoryMgr, szFile,
				&adminPrefFile);
		if (IsPGPError(err))
		{
			PGPclErrorBox(hwndMain, err);
			goto error;
		}

		err = PFLFileSpecCreate(adminPrefFile);
		if (IsPGPError(err))
		{
			PFLFreeFileSpec(adminPrefFile);
			PGPclErrorBox(hwndMain, err);
			goto error;
		}

		err = PGPOpenPrefFile(adminPrefFile, NULL, 0, &prefRef);
		if (IsPGPError(err))
		{
			PFLFreeFileSpec(adminPrefFile);
			PGPclErrorBox(hwndMain, err);
			goto error;
		}

		PGPSetPrefBoolean(prefRef, kPGPPrefUseOutgoingADK, 
			Config.bUseOutgoingADK);
		PGPSetPrefBoolean(prefRef, kPGPPrefUseDHADK, Config.bUseIncomingADK);
		PGPSetPrefBoolean(prefRef, kPGPPrefUsePGPdiskADK, 
			Config.bUseDiskADK);
		PGPSetPrefBoolean(prefRef, kPGPPrefEnforceIncomingADK, 
			Config.bEnforceIncomingADK);
		PGPSetPrefBoolean(prefRef, kPGPPrefEnforceOutgoingADK, 
			Config.bEnforceOutgoingADK);
		PGPSetPrefBoolean(prefRef, kPGPPrefEnforceRemoteADKClass, 
			Config.bEnforceRemoteADK);
		PGPSetPrefBoolean(prefRef, kPGPPrefEnforceMinChars,
			Config.bEnforceMinChars);
		PGPSetPrefBoolean(prefRef, kPGPPrefEnforceMinQuality,
			Config.bEnforceMinQuality);
		PGPSetPrefBoolean(prefRef, kPGPPrefWarnNotCertByCorp,
			Config.bWarnNotCertByCorp);
		PGPSetPrefBoolean(prefRef, kPGPPrefAutoSignTrustCorp,
			Config.bAutoSignTrustCorp);
		PGPSetPrefBoolean(prefRef, kPGPPrefMetaIntroducerCorp,
			Config.bMetaIntroducerCorp);
		PGPSetPrefBoolean(prefRef, kPGPPrefAllowKeyGen,	Config.bAllowKeyGen);
		PGPSetPrefBoolean(prefRef, kPGPPrefAllowRSAKeyGen,
			Config.bAllowRSAKeyGen);
		PGPSetPrefBoolean(prefRef, kPGPPrefAllowConventionalEncryption,
			Config.bAllowConventionalEncryption);
		PGPSetPrefBoolean(prefRef, kPGPPrefUpdateAllKeys,
			Config.bUpdateAllKeys);
		PGPSetPrefBoolean(prefRef, kPGPPrefUpdateTrustedIntroducers,
			Config.bUpdateTrustedIntroducers);
		PGPSetPrefBoolean(prefRef, kPGPPrefAutoAddRevoker,
			Config.bAutoAddRevoker);

		PGPSetPrefNumber(prefRef, kPGPAdminPrefVersion, PGP_ADMINPREFVERSION);
		PGPSetPrefNumber(prefRef, kPGPPrefMinChars, Config.nMinChars);
		PGPSetPrefNumber(prefRef, kPGPPrefMinQuality, Config.nMinQuality);
		PGPSetPrefNumber(prefRef, kPGPPrefMinimumKeySize, Config.nMinKeySize);
		PGPSetPrefNumber(prefRef, kPGPPrefCorpKeyPublicKeyAlgorithm,
			Config.corpKeyType);
		PGPSetPrefNumber(prefRef, kPGPPrefOutADKPublicKeyAlgorithm,
			Config.outgoingADKKeyType);
		PGPSetPrefNumber(prefRef, kPGPPrefPGPdiskADKPublicKeyAlgorithm,
			Config.diskADKKeyType);
		PGPSetPrefNumber(prefRef, kPGPPrefDaysUpdateAllKeys,
			Config.nDaysUpdateAllKeys);
		PGPSetPrefNumber(prefRef, kPGPPrefDaysUpdateTrustedIntroducers,
			Config.nDaysUpdateTrustedIntroducers);
		PGPSetPrefNumber(prefRef, kPGPPrefRevokerPublicKeyAlgorithm,
			Config.revokerKeyType);

		if (Config.szLicenseNum != NULL)
			PGPSetPrefString(prefRef, kPGPPrefAdminCompanyName, 
				Config.szLicenseNum);
		else
			PGPSetPrefString(prefRef, kPGPPrefAdminCompanyName, "");

		if (Config.szComments != NULL)
			PGPSetPrefString(prefRef, kPGPPrefComments, Config.szComments);
		else
			PGPSetPrefString(prefRef, kPGPPrefComments,	"");

		if ((Config.szOutgoingADKID != NULL) && (Config.bUseOutgoingADK))
		{
			PGPGetKeyIDFromString(Config.szOutgoingADKID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(prefRef, kPGPPrefOutgoingADKID, keyIDSize, 
				exportedKeyID);
		}

		if ((Config.szIncomingADKID != NULL) && (Config.bUseIncomingADK))
		{
			PGPGetKeyIDFromString(Config.szIncomingADKID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(prefRef, kPGPPrefDHADKID, keyIDSize, 
				exportedKeyID);
		}
		
		if ((Config.szDiskADKID != NULL) && (Config.bUseDiskADK))
		{
			PGPGetKeyIDFromString(Config.szDiskADKID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(prefRef, kPGPPrefPGPdiskADKKeyID, keyIDSize, 
				exportedKeyID);
		}
		
		if ((Config.szCorpKeyID != NULL) && (Config.bAutoSignTrustCorp))
		{
			PGPGetKeyIDFromString(Config.szCorpKeyID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(prefRef, kPGPPrefCorpKeyID, keyIDSize, 
				exportedKeyID);
		}

		if ((Config.szRevokerKeyID != NULL) && (Config.bAutoAddRevoker))
		{
			PGPGetKeyIDFromString(Config.szRevokerKeyID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(prefRef, kPGPPrefRevokerKeyID, keyIDSize, 
				exportedKeyID);
		}

		// Save the default keys if any were specified

		if (Config.defaultKeySet != NULL)
		{
			char *				szKeyBlock;
			PGPSize				nKeyBlockSize;
			PGPOptionListRef	comments;
			char				szComment[255];

			if (GetCommentString(Config.memoryMgr, szComment, 
					sizeof(szComment)))
				PGPBuildOptionList(Config.pgpContext, &comments,
					PGPOCommentString(Config.pgpContext, szComment),
					PGPOLastOption(Config.pgpContext));
			else
				PGPBuildOptionList(Config.pgpContext, 
					&comments, 
					PGPOLastOption(Config.pgpContext));

			err = PGPExportKeySet(Config.defaultKeySet,
					PGPOAllocatedOutputBuffer(Config.pgpContext,
						(void **) &szKeyBlock, INT_MAX, &nKeyBlockSize),
					PGPOVersionString(Config.pgpContext, 
						pgpVersionHeaderString),
					comments,
					PGPOLastOption(Config.pgpContext));

			PGPFreeOptionList(comments);

			if (IsPGPError(err))
			{
				PGPclErrorBox(hwndMain, err);
				goto error;
			}

			PGPSetPrefString(prefRef, kPGPPrefDefaultKeys, szKeyBlock);
			PGPFreeData(szKeyBlock);
		}
		else
			PGPSetPrefString(prefRef, kPGPPrefDefaultKeys, "");

		// Save and close the admin prefs file

		PGPSavePrefFile(prefRef);
		PGPClosePrefFile(prefRef);
		PFLFreeFileSpec(adminPrefFile);
		CopyFile(szFile, szOrigFile, FALSE);

		// Now copy the Non-Admin installer

		if (!CopyInstaller(hwndMain, szClientInstaller, Config.szInstallDir))
		{
			char szLicNum[3];
			char szSetupIni[MAX_PATH];

			strcpy(szSetupIni, Config.szInstallDir);
			strcat(szSetupIni, szDisk);
			strcat(szSetupIni, "\\setup.ini");

			// Modify the setup.ini file to let the installer know if it
			// needs to ask for the license number or not

			if (Config.szLicenseNum != NULL)
				strcpy(szLicNum, "1");
			else
				strcpy(szLicNum, "0");

			WritePrivateProfileString("Startup", "CompanyName", 
				Config.szLicenseNum, szSetupIni);

			// Write the installation options if applicable

			if (Config.bPreselectInstall)
			{
				char szPrograms[3];
				char szDisk[3];
				char szEudora[3];
				char szExchangeOutlook[3];
				char szOutlookExpress[3];
				char szManual[3];
				char szUninstallOld[3];

				WritePrivateProfileString("Startup", "EasyInstall", "1",
					szSetupIni);

				WritePrivateProfileString("Startup", "InstallDir",
					Config.szUserInstallDir, szSetupIni);

				wsprintf(szPrograms, "%d", Config.bInstallPrograms);
				wsprintf(szDisk, "%d", Config.bInstallDisk);
				wsprintf(szEudora, "%d", Config.bInstallEudora);
				wsprintf(szExchangeOutlook, "%d", Config.bInstallOutlook);
				wsprintf(szOutlookExpress, "%d", Config.bInstallOutExpress);
				wsprintf(szManual, "%d", Config.bInstallManual);
				wsprintf(szUninstallOld, "%d", Config.bUninstallOld);

				WritePrivateProfileString("Startup", "ProgramFiles",
					szPrograms, szSetupIni);
				WritePrivateProfileString("Startup", "PGPdisk",
					szDisk, szSetupIni);
				WritePrivateProfileString("Startup", "EudoraPlugin",
					szEudora, szSetupIni);
				WritePrivateProfileString("Startup", "ExchangeOutlookPlugin",
					szExchangeOutlook, szSetupIni);
				WritePrivateProfileString("Startup", "OutlookExpressPlugin",
					szOutlookExpress, szSetupIni);
				WritePrivateProfileString("Startup", "UserManual",
					szManual, szSetupIni);
				WritePrivateProfileString("Startup", "UninstallOld",
					szUninstallOld, szSetupIni);
			}
			else
				WritePrivateProfileString("Startup", "EasyInstall", "0",
					szSetupIni);

			// We're done!
			
			LoadString(g_hInstance, IDS_DONE, szMsg, 254);
			LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
			MessageBox(hwndMain, szMsg, szTitle, MB_ICONINFORMATION);
		}
		else
		{
			// Copying failed
			
			LoadString(g_hInstance, IDS_FAILED, szMsg, 254);
			LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
			MessageBox(hwndMain, szMsg, szTitle, MB_ICONEXCLAMATION);
		}
	}
	else
	{
		char szPrefFile[MAX_PATH];
		char szOrigFile[MAX_PATH];

		// Delete user's prefs file that was going to be installed

		LoadString(g_hInstance, IDS_NEWPREFFILE, 
				szPrefFile, MAX_PATH-1);
		PGPclGetPGPPath(szOrigFile, MAX_PATH-1);
		strcat(szOrigFile, szPrefFile);
		DeleteFile(szOrigFile);
	}

error:
	// Free allocated memory and objects

	if (Config.szLicenseNum)
	{
		PGPFreeData(Config.szLicenseNum);
		Config.szLicenseNum = NULL;
	}

	if (Config.szComments)
	{
		PGPFreeData(Config.szComments);
		Config.szComments = NULL;
	}

	if (Config.szOutgoingADKID)
	{
		pgpFree(Config.szOutgoingADKID);
		Config.szOutgoingADKID = NULL;
	}

	if (Config.szOutgoingADK)
	{
		pgpFree(Config.szOutgoingADK);
		Config.szOutgoingADK = NULL;
	}

	if (Config.szIncomingADKID)
	{
		pgpFree(Config.szIncomingADKID);
		Config.szIncomingADKID = NULL;
	}

	if (Config.szIncomingADK)
	{
		pgpFree(Config.szIncomingADK);
		Config.szIncomingADK = NULL;
	}

	if (Config.szDiskADKID)
	{
		pgpFree(Config.szDiskADKID);
		Config.szDiskADKID = NULL;
	}

	if (Config.szDiskADK)
	{
		pgpFree(Config.szDiskADK);
		Config.szDiskADK = NULL;
	}

	if (Config.szCorpKeyID)
	{
		pgpFree(Config.szCorpKeyID);
		Config.szCorpKeyID = NULL;
	}

	if (Config.szCorpKey)
	{
		pgpFree(Config.szCorpKey);
		Config.szCorpKey = NULL;
	}

	if (Config.szRevokerKeyID)
	{
		pgpFree(Config.szRevokerKeyID);
		Config.szRevokerKeyID = NULL;
	}

	if (Config.szRevokerKey)
	{
		pgpFree(Config.szRevokerKey);
		Config.szRevokerKey = NULL;
	}

	if (Config.defaultKeySet != NULL)
	{
		PGPFreeKeySet(Config.defaultKeySet);
		Config.defaultKeySet = NULL;
	}

	if (Config.szInstallDir)
	{
		PGPFreeData(Config.szInstallDir);
		Config.szInstallDir = NULL;
	}

	if (Config.szUserInstallDir)
	{
		PGPFreeData(Config.szUserInstallDir);
		Config.szUserInstallDir = NULL;
	}

	PGPclCloseLibrary();
	PGPFreeContext(Config.pgpContext);
	DeleteObject(Config.hBitmap);

	return;
}


//-------------------------------------------------------------------|
// Load DIB bitmap and associated palette

static HPALETTE 
CreateDIBPalette (LPBITMAPINFO lpbmi, 
				  LPINT lpiNumColors) 
{
	LPBITMAPINFOHEADER lpbi;
	LPLOGPALETTE lpPal;
	HANDLE hLogPal;
	HPALETTE hPal = NULL;
	INT i;
 
	lpbi = (LPBITMAPINFOHEADER)lpbmi;
	if (lpbi->biBitCount <= 8) {
		*lpiNumColors = (1 << lpbi->biBitCount);
	}
	else
		*lpiNumColors = 0;  // No palette needed for 24 BPP DIB
 
	if (*lpiNumColors) {
		hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) +
                             sizeof (PALETTEENTRY) * (*lpiNumColors));
		lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
		lpPal->palVersion = 0x300;
		lpPal->palNumEntries = *lpiNumColors;
 
		for (i = 0;  i < *lpiNumColors;  i++) {
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}
		hPal = CreatePalette (lpPal);
		GlobalUnlock (hLogPal);
		GlobalFree (hLogPal);
   }
   return hPal;
}


static HBITMAP 
LoadResourceBitmap (HINSTANCE hInstance, 
					LPSTR lpString,
					HPALETTE FAR* lphPalette) 
{
	HRSRC  hRsrc;
	HGLOBAL hGlobal;
	HBITMAP hBitmapFinal = NULL;
	LPBITMAPINFOHEADER lpbi;
	HDC hdc;
    INT iNumColors;
 
	if (hRsrc = FindResource (hInstance, lpString, RT_BITMAP)) {
		hGlobal = LoadResource (hInstance, hRsrc);
		lpbi = (LPBITMAPINFOHEADER)LockResource (hGlobal);
 
		hdc = GetDC(NULL);
		*lphPalette =  CreateDIBPalette ((LPBITMAPINFO)lpbi, &iNumColors);
		if (*lphPalette) {
			SelectPalette (hdc,*lphPalette,FALSE);
			RealizePalette (hdc);
		}
 
		hBitmapFinal = CreateDIBitmap (hdc,
                   (LPBITMAPINFOHEADER)lpbi,
                   (LONG)CBM_INIT,
                   (LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
                   (LPBITMAPINFO)lpbi,
                   DIB_RGB_COLORS );
 
		ReleaseDC (NULL,hdc);
		UnlockResource (hGlobal);
		FreeResource (hGlobal);
	}
	return (hBitmapFinal);
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
