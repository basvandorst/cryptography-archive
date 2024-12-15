/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: CreateWiz.cpp,v 1.85.4.1 1999/06/16 23:12:13 wjb Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "PGPadmin.h"
#include "resource.h"
#include "CreateWiz.h"
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
#include "pgpVersion.h"
#include "PGPsea.h"
#include "pgpClientLib.h"

extern "C" {
#include "pgpNetPaths.h"
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

BOOL CALLBACK WaitProc(HWND hwndDlg, 
					   UINT uMsg, 
					   WPARAM wParam, 
					   LPARAM lParam);

#define DEFAULT_USERINSTALLDIR "C:\\Program Files\\Network Associates\\PGP"

// AddPrefsToSEA
//
// Given the filename of the SEA, add the
// buffers containing the preference files
// to the SEA. 
//
// Note, each call of AddPrefsToSEA erases any
// previous prefs written to the SEA. Also, a 
// NULL pointer for a buffer means that no prefs
// file should be included in the SEA for that 
// module. 
//
// Return value of TRUE means success. FALSE 
// means there was an error manipulating the SEA.
BOOL AddPrefsToSEA(char *filename,
				   char *AdminBuffer,
				   DWORD AdminSize,
				   char *ClientBuffer,
				   DWORD ClientSize,
				   char *NetBuffer,
				   DWORD NetSize,
				   char *SetupBuffer,
				   DWORD SetupSize)
{
	SDAHEADER SDAHeader;
	FILE *fsea;
	DWORD offset;

	fsea=fopen(filename,"r+b");

	if(fsea==0)
		return FALSE; // Couldn't open file

	fseek(fsea, -(int)(sizeof(SDAHEADER)), SEEK_END);
	fread(&SDAHeader,1,sizeof(SDAHEADER),fsea);

	if(memcmp(SDAHeader.szPGPSDA,"PGPSEA",6)!=0)
	{
		// Not an SEA file
		fclose(fsea);
		return FALSE;
	}

	// Erase old pref entries in header
	memset(&SDAHeader.AdminOffset,0x00,8*sizeof(DWORD));

	// Prefs go after the compressed data
	offset=SDAHeader.offset+SDAHeader.CompLength;

	// Truncate whatever cruft was already there
	fflush(fsea);
	_chsize(_fileno(fsea),offset);
	fflush(fsea);

	// Start at the end of the file
	fseek(fsea,offset,SEEK_SET);

	if(AdminBuffer)
	{
		SDAHeader.AdminOffset=offset;
		SDAHeader.AdminSize=AdminSize;

		fwrite(AdminBuffer,1,SDAHeader.AdminSize,fsea);

		offset=offset+SDAHeader.AdminSize;
	}

	if(ClientBuffer)
	{
		SDAHeader.ClientOffset=offset;
		SDAHeader.ClientSize=ClientSize;

		fwrite(ClientBuffer,1,SDAHeader.ClientSize,fsea);

		offset=offset+SDAHeader.ClientSize;
	}

	if(NetBuffer)
	{
		SDAHeader.NetOffset=offset;
		SDAHeader.NetSize=NetSize;

		fwrite(NetBuffer,1,SDAHeader.NetSize,fsea);

		offset=offset+SDAHeader.NetSize;
	}

	if(SetupBuffer)
	{
		SDAHeader.SetupOffset=offset;
		SDAHeader.SetupSize=SetupSize;

		fwrite(SetupBuffer,1,SDAHeader.SetupSize,fsea);

		offset=offset+SDAHeader.SetupSize;
	}

	// Finally, finish up by writing out new header
	fwrite(&SDAHeader,1,sizeof(SDAHEADER),fsea);

	fclose(fsea);
	return TRUE;
}

void CreateWiz(HWND hwndMain)
{
	PROPSHEETPAGE	pspWiz[NUM_WIZ_PAGES];
	PROPSHEETHEADER pshWiz;
	int				nIndex;
	int iNumBits,	iBitmap;
	HDC				hDC;
	char			szTitle[255];
	char			szMsg[255];
	PGPPrefRef		adminPrefs;
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
		PGPFreeContext(Config.pgpContext);
		return;
	}

	Config.memoryMgr = PGPGetContextMemoryMgr(Config.pgpContext);

	// Check for beta/demo expiration

	if (PGPclIsExpired(hwndMain) != kPGPError_NoErr)
	{
		PGPclCloseLibrary();
		PGPFreeContext(Config.pgpContext);
		g_bGotReloadMsg = FALSE;
		return;
	}

	// Make sure we can actually save the prefs, i.e. user is running
	// with Administrator privileges

	err = PGPclOpenAdminPrefs(Config.memoryMgr, &adminPrefs, TRUE);
	if (IsntPGPError(err))
		err = PGPclCloseAdminPrefs(adminPrefs, TRUE);
	else
	{
		PGPclCloseLibrary();
		PGPFreeContext(Config.pgpContext);
		g_bGotReloadMsg = FALSE;
		return;
	}
	
	if (IsPGPError(err))
	{
		LoadString(g_hInstance, IDS_E_CANTSAVE, szMsg, 254);
		LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
		MessageBox(hwndMain, szMsg, szTitle, MB_ICONWARNING);

		PGPclCloseLibrary();
		PGPFreeContext(Config.pgpContext);
		g_bGotReloadMsg = FALSE;
		return;
	}

	// Set defaults here:

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
	Config.bKeyGenCertRequest = FALSE;
	Config.bAllowManualCertRequest = TRUE;
	Config.bAutoUpdateCRL = FALSE;
	Config.nCAType = kPGPKeyServerClass_Invalid;
	Config.pAVList = NULL;
	Config.nNumAVs = 0;
	Config.bAllowKeyGen = TRUE;
	Config.bAllowRSAKeyGen = FALSE;
	Config.nMinKeySize = 1024;
	Config.bUpdateAllKeys = FALSE;
	Config.bUpdateTrustedIntroducers = FALSE;
	Config.nDaysUpdateAllKeys = 1;
	Config.nDaysUpdateTrustedIntroducers = 1;
	Config.bAllowConventionalEncryption = TRUE;
	Config.szComments = NULL;
	Config.nCommentLength = 0;
	Config.defaultKeySet = NULL;
	Config.szAdminInstaller = NULL;
	Config.szClientInstaller = NULL;
	Config.bCopyClientPrefs = FALSE;
	Config.bPreselectInstall = FALSE;
	Config.szUserInstallDir = (char *) PGPNewData(Config.memoryMgr,
										strlen(DEFAULT_USERINSTALLDIR)+1, 
										kPGPMemoryMgrFlags_Clear);
	strcpy(Config.szUserInstallDir, DEFAULT_USERINSTALLDIR);
	Config.bInstallPrograms = TRUE;
	Config.bInstallCmdLine = TRUE;
	Config.bInstallNet = TRUE;
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
	
	// Set up the X.509 certificate settings page

	pspWiz[Wiz_X509].pszTemplate	= MAKEINTRESOURCE(IDD_X509);
	pspWiz[Wiz_X509].pfnDlgProc		= (DLGPROC) X509DlgProc;
	
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
	
	// Set up the installer page

	pspWiz[Wiz_Installer].pszTemplate	= MAKEINTRESOURCE(IDD_INSTALLER);
	pspWiz[Wiz_Installer].pfnDlgProc	= (DLGPROC) InstallerDlgProc;
	
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
		PGPPrefRef		clientPrefs;
		PGPPrefRef		netPrefs;
		PGPKeyID		keyID;
		PGPByte			exportedKeyID[kPGPMaxExportedKeyIDSize];
		PGPSize			keyIDSize;
		char			szSetupIni[MAX_PATH];
		char *			szAdminBuffer = NULL;
		char *			szClientBuffer = NULL;
		char *			szNetBuffer = NULL;
		char *			szSetupBuffer = NULL;
		PGPSize			nAdminBufferSize = 0;
		PGPSize			nClientBufferSize = 0;
		PGPSize			nNetBufferSize = 0;
		PGPSize			nSetupBufferSize = 0;
		HWND			hwndDlg;
		HANDLE			hFile;

		// Show the "wait" dialog box while this is being executed

		hwndDlg = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_WAIT), 
					hwndMain, (DLGPROC) WaitProc);
		ShowWindow(hwndDlg, SW_SHOW);

		// Save the admin prefs

		PGPclOpenAdminPrefs(Config.memoryMgr, &adminPrefs, TRUE);

		PGPSetPrefBoolean(adminPrefs, kPGPPrefUseOutgoingADK, 
			Config.bUseOutgoingADK);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefUseDHADK, 
			Config.bUseIncomingADK);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefUsePGPdiskADK, 
			Config.bUseDiskADK);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefEnforceIncomingADK, 
			Config.bEnforceIncomingADK);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefEnforceOutgoingADK, 
			Config.bEnforceOutgoingADK);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefEnforceRemoteADKClass, 
			Config.bEnforceRemoteADK);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefEnforceMinChars,
			Config.bEnforceMinChars);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefEnforceMinQuality,
			Config.bEnforceMinQuality);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefWarnNotCertByCorp,
			Config.bWarnNotCertByCorp);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefAutoSignTrustCorp,
			Config.bAutoSignTrustCorp);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefMetaIntroducerCorp,
			Config.bMetaIntroducerCorp);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefAllowKeyGen,	
			Config.bAllowKeyGen);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefAllowRSAKeyGen,
			Config.bAllowRSAKeyGen);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefAllowConventionalEncryption,
			Config.bAllowConventionalEncryption);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefUpdateAllKeys,
			Config.bUpdateAllKeys);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefUpdateTrustedIntroducers,
			Config.bUpdateTrustedIntroducers);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefAutoAddRevoker,
			Config.bAutoAddRevoker);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefKeyGenX509CertRequest,
			Config.bKeyGenCertRequest);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefAllowManualX509CertRequest,
			Config.bAllowManualCertRequest);
		PGPSetPrefBoolean(adminPrefs, kPGPPrefAutoUpdateX509CRL,
			Config.bAutoUpdateCRL);

		PGPSetPrefNumber(adminPrefs, kPGPAdminPrefVersion, 
			PGP_ADMINPREFVERSION);
		PGPSetPrefNumber(adminPrefs, kPGPPrefMinChars, Config.nMinChars);
		PGPSetPrefNumber(adminPrefs, kPGPPrefMinQuality, Config.nMinQuality);
		PGPSetPrefNumber(adminPrefs, kPGPPrefMinimumKeySize, 
			Config.nMinKeySize);
		PGPSetPrefNumber(adminPrefs, kPGPPrefCorpKeyPublicKeyAlgorithm,
			Config.corpKeyType);
		PGPSetPrefNumber(adminPrefs, kPGPPrefOutADKPublicKeyAlgorithm,
			Config.outgoingADKKeyType);
		PGPSetPrefNumber(adminPrefs, kPGPPrefPGPdiskADKPublicKeyAlgorithm,
			Config.diskADKKeyType);
		PGPSetPrefNumber(adminPrefs, kPGPPrefDaysUpdateAllKeys,
			Config.nDaysUpdateAllKeys);
		PGPSetPrefNumber(adminPrefs, kPGPPrefDaysUpdateTrustedIntroducers,
			Config.nDaysUpdateTrustedIntroducers);
		PGPSetPrefNumber(adminPrefs, kPGPPrefRevokerPublicKeyAlgorithm,
			Config.revokerKeyType);

		if (Config.szLicenseNum != NULL)
			PGPSetPrefString(adminPrefs, kPGPPrefAdminCompanyName, 
				Config.szLicenseNum);
		else
			PGPSetPrefString(adminPrefs, kPGPPrefAdminCompanyName, "");

		if (Config.szComments != NULL)
			PGPSetPrefString(adminPrefs, kPGPPrefComments, Config.szComments);
		else
			PGPSetPrefString(adminPrefs, kPGPPrefComments,	"");

		if ((Config.szOutgoingADKID != NULL) && (Config.bUseOutgoingADK))
		{
			PGPGetKeyIDFromString(Config.szOutgoingADKID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(adminPrefs, kPGPPrefOutgoingADKID, keyIDSize, 
				exportedKeyID);
		}

		if ((Config.szIncomingADKID != NULL) && (Config.bUseIncomingADK))
		{
			PGPGetKeyIDFromString(Config.szIncomingADKID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(adminPrefs, kPGPPrefDHADKID, keyIDSize, 
				exportedKeyID);
		}
		
		if ((Config.szDiskADKID != NULL) && (Config.bUseDiskADK))
		{
			PGPGetKeyIDFromString(Config.szDiskADKID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(adminPrefs, kPGPPrefPGPdiskADKKeyID, keyIDSize, 
				exportedKeyID);
		}
		
		if ((Config.szCorpKeyID != NULL) && (Config.bAutoSignTrustCorp))
		{
			PGPGetKeyIDFromString(Config.szCorpKeyID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(adminPrefs, kPGPPrefCorpKeyID, keyIDSize, 
				exportedKeyID);
		}

		if ((Config.szRevokerKeyID != NULL) && (Config.bAutoAddRevoker))
		{
			PGPGetKeyIDFromString(Config.szRevokerKeyID, &keyID);
			PGPExportKeyID(&keyID, exportedKeyID, &keyIDSize);
			PGPSetPrefData(adminPrefs, kPGPPrefRevokerKeyID, keyIDSize, 
				exportedKeyID);
		}

		// Save the AV list, if specified

		if (Config.nCAType != kPGPKeyServerClass_Invalid)
		{
			PGPByte *data;
			PGPSize dataSize;

			PGPSetPrefNumber(adminPrefs, kPGPPrefAdminCAType, Config.nCAType);

			if ((Config.pAVList != NULL) && (Config.nNumAVs > 0))
			{
				PGPAVPairsToData(Config.memoryMgr, Config.pAVList, 
					Config.nNumAVs, &data, &dataSize);
				PGPSetPrefData(adminPrefs, kPGPPrefExtraAVPairs, dataSize,
					data);
				PGPFreeData(data);
			}
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

			PGPSetPrefString(adminPrefs, kPGPPrefDefaultKeys, szKeyBlock);
			PGPFreeData(szKeyBlock);
		}
		else
			PGPSetPrefString(adminPrefs, kPGPPrefDefaultKeys, "");

		// Store the admin prefs to a buffer

		PGPExportPrefFileToBuffer(adminPrefs, &nAdminBufferSize,
			(void **) &szAdminBuffer);

		// Save and close the admin prefs file

		PGPclCloseAdminPrefs(adminPrefs, TRUE);

		// Load client, PGPnet, and PGPdisk prefs into a buffer, if needed
			
		if (Config.bCopyClientPrefs)
		{
			char szNetPrefFile[256];
			PFLFileSpecRef netPrefFile;

			PGPclOpenClientPrefs(Config.memoryMgr, &clientPrefs);
			PGPExportPrefFileToBuffer(clientPrefs, &nClientBufferSize,
				(void **) &szClientBuffer);
			PGPclCloseClientPrefs(clientPrefs, FALSE);

			if (IsntPGPError(PGPnetGetPrefsFullPath(szNetPrefFile, 255)))
			{
				err = PFLNewFileSpecFromFullPath(Config.memoryMgr, 
						szNetPrefFile, &netPrefFile);
				
				if (IsPGPError(err))
				{
					PGPclErrorBox(hwndMain, err);
					goto error;
				}
				
				err = PFLFileSpecCreate(netPrefFile);
				if (IsPGPError(err))
				{
					PFLFreeFileSpec(netPrefFile);
					PGPclErrorBox(hwndMain, err);
					goto error;
				}

				PGPOpenPrefFile(netPrefFile, NULL, 0, &netPrefs);
				PGPExportPrefFileToBuffer(netPrefs, &nNetBufferSize,
					(void **) &szNetBuffer);
				PGPClosePrefFile(netPrefs);
				PFLFreeFileSpec(netPrefFile);
			}
		}

		// Now create the client installer

		if (GetTempPath(MAX_PATH-1, szSetupIni))
		{
			char szLicNum[3];
			char szVersion[256];
			DWORD dwBytesRead;
			
			strcat(szSetupIni, "setup.ini");

			wsprintf(szVersion, "%s %s", PGPPRODUCTNAME, PGPVERSIONSTRING);
			WritePrivateProfileString("Startup", "AppName", 
				szVersion, szSetupIni);

			WritePrivateProfileString("Startup", "FreeDiskSpace", 
				"181", szSetupIni);

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
				char szCmdLine[3];
				char szNet[3];
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
				wsprintf(szCmdLine, "%d", Config.bInstallCmdLine);
				wsprintf(szNet, "%d", Config.bInstallNet);
				wsprintf(szDisk, "%d", Config.bInstallDisk);
				wsprintf(szEudora, "%d", Config.bInstallEudora);
				wsprintf(szExchangeOutlook, "%d", Config.bInstallOutlook);
				wsprintf(szOutlookExpress, "%d", Config.bInstallOutExpress);
				wsprintf(szManual, "%d", Config.bInstallManual);
				wsprintf(szUninstallOld, "%d", Config.bUninstallOld);

				WritePrivateProfileString("Startup", "ProgramFiles",
					szPrograms, szSetupIni);
				WritePrivateProfileString("Startup", "NTcmdln",
					szCmdLine, szSetupIni);
				WritePrivateProfileString("Startup", "PGPnet",
					szNet, szSetupIni);
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

			// On windows 95, we must flush the ini file by using three
			// nulls -wjb
			WritePrivateProfileString(NULL,NULL,NULL,
					szSetupIni);

			hFile = CreateFile(szSetupIni, GENERIC_READ, 0, NULL, OPEN_ALWAYS,
						FILE_ATTRIBUTE_NORMAL, NULL);
			nSetupBufferSize = GetFileSize(hFile, NULL);

			szSetupBuffer = (char *) PGPNewData(Config.memoryMgr,
										nSetupBufferSize,
										kPGPMemoryMgrFlags_Clear);

			ReadFile(hFile, szSetupBuffer, nSetupBufferSize, 
				&dwBytesRead, NULL);
			CloseHandle(hFile);
			DeleteFile(szSetupIni);
		}

		if (CopyFile(Config.szAdminInstaller, Config.szClientInstaller, 
			FALSE))
		{
			if (SetFileAttributes(Config.szClientInstaller, 
					FILE_ATTRIBUTE_NORMAL))
			{
				AddPrefsToSEA(Config.szClientInstaller, szAdminBuffer, 
					nAdminBufferSize, szClientBuffer, nClientBufferSize,
					szNetBuffer, nNetBufferSize, szSetupBuffer, 
					nSetupBufferSize);

				// We're done!
			
				DestroyWindow(hwndDlg);
				LoadString(g_hInstance, IDS_DONE, szMsg, 254);
				LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
				MessageBox(hwndMain, szMsg, szTitle, MB_ICONINFORMATION);
			}
			else
			{
				// Setting attributes failed
			
				DestroyWindow(hwndDlg);
				LoadString(g_hInstance, IDS_FAILED, szMsg, 254);
				LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
				MessageBox(hwndMain, szMsg, szTitle, MB_ICONEXCLAMATION);
			}
		}
		else
		{
			// Copying file failed
			
			DestroyWindow(hwndDlg);
			LoadString(g_hInstance, IDS_FAILED, szMsg, 254);
			LoadString(g_hInstance, IDS_TITLE, szTitle, 254);
			MessageBox(hwndMain, szMsg, szTitle, MB_ICONEXCLAMATION);
		}
		
		if (szAdminBuffer != NULL)
			PGPFreeData(szAdminBuffer);
		if (szClientBuffer != NULL)
			PGPFreeData(szClientBuffer);
		if (szNetBuffer != NULL)
			PGPFreeData(szNetBuffer);
		if (szSetupBuffer != NULL)
			PGPFreeData(szSetupBuffer);
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

	if (Config.szAdminInstaller)
	{
		PGPFreeData(Config.szAdminInstaller);
		Config.szAdminInstaller = NULL;
	}

	if (Config.szClientInstaller)
	{
		PGPFreeData(Config.szClientInstaller);
		Config.szClientInstaller = NULL;
	}

	if (Config.szUserInstallDir)
	{
		PGPFreeData(Config.szUserInstallDir);
		Config.szUserInstallDir = NULL;
	}

	if (Config.pAVList)
	{
		PGPclFreeCACertRequestAVList(Config.pAVList, Config.nNumAVs);
		Config.pAVList = NULL;
		Config.nNumAVs = 0;
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


BOOL CALLBACK WaitProc(HWND hwndDlg, 
					   UINT uMsg, 
					   WPARAM wParam, 
					   LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT rc;

			// center dialog on screen
			GetWindowRect(GetParent(hwndDlg), &rc);
			SetWindowPos(GetParent(hwndDlg), NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);

			break;
		}
	}

	return 0;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
