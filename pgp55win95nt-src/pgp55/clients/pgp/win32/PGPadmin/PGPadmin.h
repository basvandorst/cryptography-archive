/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPadmin.h,v 1.17 1997/10/16 18:32:06 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_PGPadmin_h	/* [ */
#define Included_PGPadmin_h

extern HINSTANCE g_hInstance;
extern HWND g_hCurrentDlgWnd;
extern BOOL g_bGotReloadMsg;

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pflContext.h"

// Wizard pages

typedef enum _AdminWizPage
{
	Wiz_License=0,
	Wiz_Intro,
	Wiz_ADKIntro,
	Wiz_ADKIncoming,
	Wiz_ADKInSelect,
	Wiz_ADKOutgoing,
	Wiz_ADKOutSelect,
	Wiz_ADKEnforce,
	Wiz_PassPhrase,
	Wiz_CorpKey,
	Wiz_CorpKeySelect,
	Wiz_KeyGen,
	Wiz_DefaultKeys,
	Wiz_Misc,
	Wiz_Review,
	Wiz_ClientPrefs,
	Wiz_Finish,
	NUM_WIZ_PAGES
} AdminWizPage;

// Configuration information

typedef struct _pgpConfigInfo
{
	HBITMAP					hBitmap;
	HPALETTE				hPalette;
	PFLContextRef			pflContext;
	PGPContextRef			pgpContext;
	char *					szLicenseNum;
	PGPBoolean				bUseOutgoingADK;
	PGPBoolean				bUseIncomingADK;
	char *	 				szOutgoingADKID;
	char *					szOutgoingADK;
	char *	 				szIncomingADKID;
	char *					szIncomingADK;
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
	PGPBoolean				bWarnNotCertByCorp;
	PGPBoolean				bAutoSignTrustCorp;
	PGPBoolean				bMetaIntroducerCorp;
	PGPBoolean				bAllowKeyGen;
	PGPBoolean				bAllowRSAKeyGen;
	PGPUInt32				nMinKeySize;
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

BOOL CALLBACK KeyGenDlgProc(HWND hwndDlg, 
							UINT uMsg, 
							WPARAM wParam, 
							LPARAM lParam);

BOOL CALLBACK DefaultKeysDlgProc(HWND hwndDlg, 
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
