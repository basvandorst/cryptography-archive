/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: PGPadmin.h,v 1.25.8.1 1998/11/12 03:13:11 heller Exp $
____________________________________________________________________________*/
#ifndef Included_PGPadmin_h	/* [ */
#define Included_PGPadmin_h

extern HINSTANCE g_hInstance;
extern HWND g_hCurrentDlgWnd;
extern BOOL g_bGotReloadMsg;

#include "pgpBase.h"
#include "pgpKeys.h"

// Wizard pages

typedef enum _AdminWizPage
{
	Wiz_Start=0,
	Wiz_License=Wiz_Start,
	Wiz_Intro,
	Wiz_ADKIntro,
	Wiz_ADKIncoming,
	Wiz_ADKInSelect,
	Wiz_ADKOutgoing,
	Wiz_ADKOutSelect,
	Wiz_ADKEnforce,
	Wiz_ADKDisk,
	Wiz_ADKDiskSelect,
	Wiz_PassPhrase,
	Wiz_CorpKey,
	Wiz_CorpKeySelect,
	Wiz_Revoker,
	Wiz_RevokerSelect,
	Wiz_KeyGen,
	Wiz_DefaultKeys,
	Wiz_ServerUpdates,
	Wiz_Misc,
	Wiz_Review,
	Wiz_ClientPrefs,
	Wiz_InstallOptions,
	Wiz_Finish,
	NUM_WIZ_PAGES
} AdminWizPage;

// Configuration information

typedef struct _pgpConfigInfo
{
	HBITMAP					hBitmap;
	HPALETTE				hPalette;
	PGPContextRef			pgpContext;
	PGPMemoryMgrRef			memoryMgr;
	char *					szLicenseNum;
	PGPBoolean				bUseOutgoingADK;
	PGPBoolean				bUseIncomingADK;
	PGPBoolean				bUseDiskADK;
	char *	 				szOutgoingADKID;
	char *					szOutgoingADK;
	char *	 				szIncomingADKID;
	char *					szIncomingADK;
	char *					szDiskADKID;
	char *					szDiskADK;
	PGPBoolean				bEnforceIncomingADK;
	PGPBoolean				bEnforceOutgoingADK;
	PGPBoolean				bEnforceRemoteADK;
	PGPBoolean				bEnforceMinChars;
	PGPUInt32				nMinChars;
	PGPBoolean				bEnforceMinQuality;
	PGPUInt32				nMinQuality;
	char *	 				szCorpKeyID;
	char *					szCorpKey;
	PGPPublicKeyAlgorithm	corpKeyType;
	PGPPublicKeyAlgorithm	outgoingADKKeyType;
	PGPPublicKeyAlgorithm	diskADKKeyType;
	PGPBoolean				bWarnNotCertByCorp;
	PGPBoolean				bAutoSignTrustCorp;
	PGPBoolean				bMetaIntroducerCorp;
	PGPBoolean				bAutoAddRevoker;
	char *					szRevokerKeyID;
	char *					szRevokerKey;
	PGPPublicKeyAlgorithm	revokerKeyType;
	PGPBoolean				bAllowKeyGen;
	PGPBoolean				bAllowRSAKeyGen;
	PGPUInt32				nMinKeySize;
	PGPBoolean				bUpdateAllKeys;
	PGPBoolean				bUpdateTrustedIntroducers;
	PGPUInt32				nDaysUpdateAllKeys;
	PGPUInt32				nDaysUpdateTrustedIntroducers;
	PGPBoolean				bAllowConventionalEncryption;
	char *					szComments;
	PGPUInt32				nCommentLength;
	PGPKeySetRef			userKeySet;
	PGPKeySetRef			defaultKeySet;
/*
	char *					szKeySetFile;
	char *					szPrefFile;
*/
	char *					szInstallDir;
	PGPBoolean				bCopyClientPrefs;
	PGPBoolean				bPreselectInstall;
	char *					szUserInstallDir;
	PGPBoolean				bInstallPrograms;
	PGPBoolean				bInstallDisk;
	PGPBoolean				bInstallEudora;
	PGPBoolean				bInstallOutlook;
	PGPBoolean				bInstallOutExpress;
	PGPBoolean				bInstallManual;
	PGPBoolean				bUninstallOld;
	PGPBoolean				bSave;
} pgpConfigInfo;

// Global variables

extern HWND g_hInstance;

// Wizard page dialog procedures

BOOL CALLBACK LicenseDlgProc(HWND hwndDlg, 
							 UINT uMsg,
							 WPARAM wParam, 
							 LPARAM lParam);

BOOL CALLBACK IntroDlgProc(HWND hwndDlg, 
						   UINT uMsg,
						   WPARAM wParam, 
						   LPARAM lParam);

BOOL CALLBACK ADKIntroDlgProc(HWND hwndDlg,
								 UINT uMsg,
								 WPARAM wParam,
								 LPARAM lParam);

BOOL CALLBACK ADKIncomingDlgProc(HWND hwndDlg,
								 UINT uMsg, 
								 WPARAM wParam, 
								 LPARAM lParam);

BOOL CALLBACK ADKInSelectDlgProc(HWND hwndDlg, 
								 UINT uMsg, 
								 WPARAM wParam, 
								 LPARAM lParam);

BOOL CALLBACK ADKOutgoingDlgProc(HWND hwndDlg, 
									UINT uMsg, 
									WPARAM wParam, 
									LPARAM lParam);

BOOL CALLBACK ADKOutSelectDlgProc(HWND hwndDlg, 
									 UINT uMsg, 
									 WPARAM wParam, 
									 LPARAM lParam);

BOOL CALLBACK ADKEnforceDlgProc(HWND hwndDlg,
								   UINT uMsg,
								   WPARAM wParam,
								   LPARAM lParam);

BOOL CALLBACK ADKDiskDlgProc(HWND hwndDlg, 
							 UINT uMsg, 
							 WPARAM wParam, 
							 LPARAM lParam);

BOOL CALLBACK ADKDiskSelectDlgProc(HWND hwndDlg, 
								   UINT uMsg, 
								   WPARAM wParam, 
								   LPARAM lParam);

BOOL CALLBACK PassPhraseDlgProc(HWND hwndDlg, 
								UINT uMsg, 
								WPARAM wParam, 
								LPARAM lParam);

BOOL CALLBACK CorpKeyDlgProc(HWND hwndDlg, 
							 UINT uMsg, 
							 WPARAM wParam, 
							 LPARAM lParam);

BOOL CALLBACK CorpKeySelectDlgProc(HWND hwndDlg, 
								   UINT uMsg, 
								   WPARAM wParam, 
								   LPARAM lParam);

BOOL CALLBACK RevokerDlgProc(HWND hwndDlg, 
							 UINT uMsg, 
							 WPARAM wParam, 
							 LPARAM lParam);

BOOL CALLBACK RevokerSelectDlgProc(HWND hwndDlg, 
								   UINT uMsg, 
								   WPARAM wParam, 
								   LPARAM lParam);

BOOL CALLBACK KeyGenDlgProc(HWND hwndDlg, 
							UINT uMsg, 
							WPARAM wParam, 
							LPARAM lParam);

BOOL CALLBACK DefaultKeysDlgProc(HWND hwndDlg, 
								 UINT uMsg, 
								 WPARAM wParam, 
								 LPARAM lParam);

BOOL CALLBACK ServerUpdatesDlgProc(HWND hwndDlg, 
								   UINT uMsg, 
								   WPARAM wParam, 
								   LPARAM lParam);

BOOL CALLBACK MiscDlgProc(HWND hwndDlg, 
						  UINT uMsg, 
						  WPARAM wParam, 
						  LPARAM lParam);

BOOL CALLBACK ReviewDlgProc(HWND hwndDlg, 
							UINT uMsg,
							WPARAM wParam, 
							LPARAM lParam);

BOOL CALLBACK ClientPrefsDlgProc(HWND hwndDlg, 
								 UINT uMsg,
								 WPARAM wParam, 
								 LPARAM lParam);

BOOL CALLBACK InstallOptionsDlgProc(HWND hwndDlg, 
									UINT uMsg, 
									WPARAM wParam, 
									LPARAM lParam);

BOOL CALLBACK FinishDlgProc(HWND hwndDlg, 
							UINT uMsg, 
							WPARAM wParam, 
							LPARAM lParam);

#endif /* ] Included_PGPadmin_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
