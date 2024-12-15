/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CreateWiz.cpp,v 1.44 1997/10/16 18:32:01 dgal Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "resource.h"
#include "PGPadmin.h"
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
#include "PGPcmdlg.h"
#include "pgpVersionHeader.h"

static pgpConfigInfo Config;

static HPALETTE 
CreateDIBPalette (LPBITMAPINFO lpbmi, 
				  LPINT lpiNumColors);

static HBITMAP 
LoadResourceBitmap (HINSTANCE hInstance, 
					LPSTR lpString,
					HPALETTE FAR* lphPalette);


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

	// Check for beta/demo expiration

	if (PGPcomdlgIsExpired(hwndMain))
		return;
	
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
			PGPcomdlgErrorMessage(err);

		return;
	}

	// Check to make sure the ClientInstaller directory
	// is present in the PGP directory
	
	PGPcomdlgGetPGPPath(szPGPPath, MAX_PATH-1);
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

	PFLNewContext(&(Config.pflContext));

	Config.hBitmap = NULL;
	Config.hPalette = NULL;
	Config.szLicenseNum = NULL;
	Config.bUseOutgoingADK = FALSE;
	Config.bUseIncomingADK = FALSE;
	Config.szOutgoingADKID = NULL;
	Config.szOutgoingADK = NULL;
	Config.szIncomingADKID = NULL;
	Config.szIncomingADK = NULL;
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
	Config.bWarnNotCertByCorp = FALSE;
	Config.bAutoSignTrustCorp = FALSE;
	Config.bMetaIntroducerCorp = FALSE;
	Config.bAllowKeyGen = TRUE;
	Config.bAllowRSAKeyGen = FALSE;
	Config.nMinKeySize = 512;
	Config.bAllowConventionalEncryption = TRUE;
	Config.szComments = NULL;
	Config.nCommentLength = 0;
	Config.defaultKeySet = NULL;
	Config.szInstallDir = (char *) PFLContextMemAlloc(Config.pflContext,
									strlen("C:\\")+1, kPFLMemoryFlags_Clear);
	strcpy(Config.szInstallDir, "C:\\");
	Config.bCopyClientPrefs = FALSE;
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
	
	// Set up the pass phrase page

	pspWiz[Wiz_PassPhrase].pszTemplate	= MAKEINTRESOURCE(IDD_PASSPHRASE);
	pspWiz[Wiz_PassPhrase].pfnDlgProc	= (DLGPROC) PassPhraseDlgProc;
	
	// Set up the corporate key page

	pspWiz[Wiz_CorpKey].pszTemplate = MAKEINTRESOURCE(IDD_CORPKEY);
	pspWiz[Wiz_CorpKey].pfnDlgProc	= (DLGPROC) CorpKeyDlgProc;
	
	// Set up the corporate key selection page

	pspWiz[Wiz_CorpKeySelect].pszTemplate	= MAKEINTRESOURCE(IDD_CORPKEYSEL);
	pspWiz[Wiz_CorpKeySelect].pfnDlgProc	= (DLGPROC) CorpKeySelectDlgProc;
	
	// Set up the key generation page

	pspWiz[Wiz_KeyGen].pszTemplate	= MAKEINTRESOURCE(IDD_KEYGEN);
	pspWiz[Wiz_KeyGen].pfnDlgProc	= (DLGPROC) KeyGenDlgProc;
	
	// Set up the default key selection page

	pspWiz[Wiz_DefaultKeys].pszTemplate = MAKEINTRESOURCE(IDD_DEFKEYSEL);
	pspWiz[Wiz_DefaultKeys].pfnDlgProc	= (DLGPROC) DefaultKeysDlgProc;
	
	// Set up the miscellaneous page

	pspWiz[Wiz_Misc].pszTemplate	= MAKEINTRESOURCE(IDD_MISC);
	pspWiz[Wiz_Misc].pfnDlgProc		= (DLGPROC) MiscDlgProc;
	
	// Set up the review page

	pspWiz[Wiz_Review].pszTemplate	= MAKEINTRESOURCE(IDD_REVIEW);
	pspWiz[Wiz_Review].pfnDlgProc	= (DLGPROC) ReviewDlgProc;
	
	// Set up the client prefs page

	pspWiz[Wiz_ClientPrefs].pszTemplate	= MAKEINTRESOURCE(IDD_CLIENTPREFS);
	pspWiz[Wiz_ClientPrefs].pfnDlgProc	= (DLGPROC) ClientPrefsDlgProc;
	
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
	pshWiz.nStartPage	= Wiz_License;
	pshWiz.ppsp			= pspWiz;
	pshWiz.pfnCallback	= NULL;

	// Execute the Wizard - doesn't return until Cancel or Save

	PropertySheet(&pshWiz);

	// Save settings

	if (Config.bSave)
	{
		PFLFileSpecRef  adminPrefFile;
		PGPPrefRef		prefRef;
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

		PFLContextMemFree(Config.pflContext, Config.szInstallDir);
		Config.szInstallDir = (char *) PFLContextMemAlloc(Config.pflContext,
										strlen(szInstallDir)+1, 
										kPFLMemoryFlags_Clear);

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
			
			PGPcomdlgGetPGPPath(szOrigFile, MAX_PATH-1);
			
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
		PGPcomdlgGetPGPPath(szOrigFile, MAX_PATH-1);
		strcat(szFile, szPrefFile);
		strcat(szOrigFile, szPrefFile);

		err = PFLNewFileSpecFromFullPath(Config.pflContext, szFile,
				&adminPrefFile);
		if (IsPGPError(err))
		{
			PGPcomdlgErrorMessage(err);
			goto error;
		}

		err = PFLFileSpecCreate(adminPrefFile);
		if (IsPGPError(err))
		{
			PFLFreeFileSpec(adminPrefFile);
			PGPcomdlgErrorMessage(err);
			goto error;
		}

		err = PGPOpenPrefFile(adminPrefFile, &prefRef);
		if (IsPGPError(err))
		{
			PFLFreeFileSpec(adminPrefFile);
			PGPcomdlgErrorMessage(err);
			goto error;
		}

		PGPSetPrefBoolean(prefRef, kPGPPrefUseOutgoingADK, 
			Config.bUseOutgoingADK);
		PGPSetPrefBoolean(prefRef, kPGPPrefUseDHADK, Config.bUseIncomingADK);
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

		PGPSetPrefNumber(prefRef, kPGPAdminPrefVersion, PGP_ADMINPREFVERSION);
		PGPSetPrefNumber(prefRef, kPGPPrefMinChars, Config.nMinChars);
		PGPSetPrefNumber(prefRef, kPGPPrefMinQuality, Config.nMinQuality);
		PGPSetPrefNumber(prefRef, kPGPPrefMinimumKeySize, Config.nMinKeySize);
		PGPSetPrefNumber(prefRef, kPGPPrefCorpKeyPublicKeyAlgorithm,
			Config.corpKeyType);
		PGPSetPrefNumber(prefRef, kPGPPrefOutADKPublicKeyAlgorithm,
			Config.outgoingADKKeyType);

		if (Config.szLicenseNum != NULL)
			PGPSetPrefString(prefRef, kPGPPrefSiteLicenseNumber, 
				Config.szLicenseNum);
		else
			PGPSetPrefString(prefRef, kPGPPrefSiteLicenseNumber, "");

		if (Config.szComments != NULL)
			PGPSetPrefString(prefRef, kPGPPrefComments, Config.szComments);
		else
			PGPSetPrefString(prefRef, kPGPPrefComments,	"");

		if ((Config.szOutgoingADKID != NULL) && (Config.bUseOutgoingADK))
			PGPSetPrefString(prefRef, kPGPPrefOutgoingADKID, 
				Config.szOutgoingADKID);
		else
			PGPSetPrefString(prefRef, kPGPPrefOutgoingADKID, "");

		if ((Config.szIncomingADKID != NULL) && (Config.bUseIncomingADK))
			PGPSetPrefString(prefRef, kPGPPrefDHADKID, Config.szIncomingADKID);
		else
			PGPSetPrefString(prefRef, kPGPPrefDHADKID, "");
		
		if ((Config.szCorpKeyID != NULL) && (Config.bAutoSignTrustCorp))
			PGPSetPrefString(prefRef, kPGPPrefCorpKeyID, Config.szCorpKeyID);
		else
			PGPSetPrefString(prefRef, kPGPPrefCorpKeyID, "");

		// Save the default keys if any were specified

		if (Config.defaultKeySet != NULL)
		{
			char *				szKeyBlock;
			PGPSize				nKeyBlockSize;
			PGPOptionListRef	comments;
			char				szComment[255];

			if (GetCommentString(szComment, sizeof(szComment)))
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
				PGPcomdlgErrorMessage(err);
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

			WritePrivateProfileString("Startup", "LicNum", szLicNum, 
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
		PGPcomdlgGetPGPPath(szOrigFile, MAX_PATH-1);
		strcat(szOrigFile, szPrefFile);
		DeleteFile(szOrigFile);
	}

error:
	// Free allocated memory and objects

	if (Config.szLicenseNum)
	{
		PFLContextMemFree(Config.pflContext, Config.szLicenseNum);
		Config.szLicenseNum = NULL;
	}

	if (Config.szComments)
	{
		PFLContextMemFree(Config.pflContext, Config.szComments);
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

	if (Config.defaultKeySet != NULL)
	{
		PGPFreeKeySet(Config.defaultKeySet);
		Config.defaultKeySet = NULL;
	}

	if (Config.szInstallDir)
	{
		PFLContextMemFree(Config.pflContext, Config.szInstallDir);
		Config.szInstallDir = NULL;
	}

	PFLFreeContext(Config.pflContext);
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
