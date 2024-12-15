/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMkeygenwiz.c - handle wizard for creating new keys
//
//	$Id: PGPkeygenwiz.c,v 1.14 1997/05/16 14:30:16 pbj Exp $
//
//	This code attempts to thwart some common situations
//	which could compromise the security of the passphrase.
//	For the most part, this code has been removed or comment-
//	out.
//	See KMChange.c for a description of methods used here.
//
#include <windows.h>
#include <process.h>
#include <commctrl.h>

#include <assert.h>

#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpcomdlg.h"
#include "..\include\pgpkeyserversupport.h"
#include "..\include\pgpmem.h"
#include "..\include\pgpversion.h"
#include "..\pgpkeyserver\winids.h"
#include "..\pgpkeyserver\getkeys.h"

#include "resource.h"
#include "pgpkeys.h"

#define MAX_FULL_NAME_LEN		128
#define MAX_EMAIL_LEN			128

#define MIN_EXPIRATION			1
#define MAX_EXPIRATION			2000

#define	MIN_KEY_SIZE			512
#define MAX_RSA_KEY_SIZE		2048
#define MAX_DSA_KEY_SIZE		4096

#define DEFAULT_KEY_SIZE		2048
#define DEFAULT_SUBKEY_SIZE		1024
#define DEFAULT_KEY_TYPE		PGP_PKALG_DSA

#define TIMERPERIOD				1000L
#define AVI_TIMER				2345L
#define AVI_RUNTIME				11000L	// duration of AVI in ms

// external globals
extern HINSTANCE g_hInst;
extern PGPKeySet* g_pKeySetMain;
extern g_bKeyServerSupportLoaded;
extern CHAR g_szHelpFile[MAX_PATH];		//string containing name of help file

// local globals
static HHOOK hHookDebug;
static HHOOK hHookKeyboard;
static HWND hWndCollectEntropy = NULL;
static BOOL bNeedsCentering;
static WNDPROC wpOrigPhrase1Proc;
static WNDPROC wpOrigPhrase2Proc;
static BOOL bHideTyping;
static LPSTR szPhrase1;
static LPSTR szPhrase2;
static LONG lPhrase1Len;
static LONG lPhrase2Len;
//static HWND hWndFocus;

// local prototypes
LRESULT WINAPI KeyWizardDefaultDlgProc (HWND hdlg, UINT Msg,
										WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardNameDlgProc (HWND hdlg, UINT Msg,
									 WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardSizeDlgProc (HWND hdlg, UINT Msg,
									 WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardTypeDlgProc (HWND hdlg, UINT Msg,
									 WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardExpirationDlgProc (HWND hdlg, UINT Msg,
										   WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardPassphraseDlgProc (HWND hdlg, UINT Msg,
										   WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardRandobitsDlgProc (HWND hdlg, UINT Msg,
										  WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardGenerationDlgProc (HWND hdlg, UINT Msg,
										   WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardPreSendDlgProc(HWND hdlg, UINT Msg,
									   WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardSendToServerDlgProc(HWND hdlg, UINT Msg,
											WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardDoneDlgProc (HWND hdlg, UINT Msg,
									 WPARAM wParam, LPARAM lParam);
LRESULT WINAPI KeyWizardSignOldDlgProc (HWND hdlg, UINT Msg,
										WPARAM wParam, LPARAM lParam);
static void PaintWizardBitmap(HWND hdlg, BOOL IsFirstPaint,
							  UINT BitmapResource);
static BOOL ValidatePassPhrase(HWND hwnd, char *PassPhrase);
static unsigned long GetDSAKeySize (unsigned long Requested);
void KeyCreationThread(void *pArgs);
static HBITMAP LoadResourceBitmap (HINSTANCE hInstance, LPSTR lpString,
                           HPALETTE FAR* lphPalette);
static HPALETTE CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpiNumColors);
static unsigned long CreatePGPUserID(char **UserID, char *FullName,
									 char *Email);
UINT KeyGenWizard (HWND hWnd, INT startsheet);
static int KeyWizWorkingCallback(void *pArg, INT Phase);
static void GetOldKey(PGPKey** ppOldKey, char *UserID);
void StartPutKeyInfoThread(void *pThreadArgs);
void DisplayKSError(HWND hwnd, unsigned long ReturnCode);

// typedefs
typedef struct _UserInfo
{
	char *FullName;
	char *Email;
	char *UserID;
	UINT KeyType;
	unsigned long KeySize;
	unsigned long SubKeySize;
	UINT ExpireDays;
	char *PassPhrase;
	long RandomBitsNeeded;
	long OriginalEntropy;
	PGPKey* pKey;
	PGPKey* pOldKey;
	WPARAM FinalResult;
	BOOL FinishSelected;
	BOOL CancelPending;
	BOOL InGeneration;
	unsigned long WorkingPhase;
	WDPLT PutKeyArgs;
	BOOL PutComplete;
	char *KeyBuffer;
	BOOL DoSend;
}USERINFO, *PUSERINFO;

typedef struct _PageInfo
{
	PROPSHEETPAGE psp;
	PUSERINFO pUserInfo;
	unsigned short WizFlags;
	INT	BitmapResource;
	HWND hwnd;
	INT HelpID;
	BOOL PageActive;
}PAGEINFO, *PPAGEINFO;


//----------------------------------------------------|
//  Thread to handle wizard

void NewKeyWizThread(void *pArg)
{
	HWND hwnd;

	hwnd = (HWND) pArg;

	KeyGenWizard(hwnd, 0);
}

//----------------------------------------------------|
//  Hook procedure for WH_KEYBOARD hook --
//  This is just a dummy routine to allow the
//  the Debug hook to trap this event.

LRESULT CALLBACK WizKeyboardHookProc (INT iCode,
									  WPARAM wParam, LPARAM lParam) {

	if (hWndCollectEntropy) {
		pgpRandPoolKeystroke (wParam);
		PostMessage (hWndCollectEntropy, WM_TIMER, 0, 0);
		return 1;		
	}
	else return CallNextHookEx (hHookKeyboard, iCode, wParam, lParam);
}


//----------------------------------------------------|
//  Hook procedure for WH_DEBUG hook -- this procedure is
//  called before another hook is about to be called.

LRESULT CALLBACK WizDebugHookProc (INT iCode, WPARAM wParam, LPARAM lParam) {

    if (iCode < 0)  // do not process message
        return CallNextHookEx (hHookDebug, iCode, wParam, lParam);

//	if (wParam != WH_KEYBOARD)
//      return CallNextHookEx (hHookDebug, iCode, wParam, lParam);

	return CallNextHookEx (hHookDebug, iCode, wParam, lParam);
}


//----------------------------------------------------|
//  Check if message is dangerous to pass to passphrase edit box

BOOL WizCommonNewKeyPhraseMsgProc (HWND hWnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
	case WM_RBUTTONDOWN :
	case WM_LBUTTONDBLCLK :
	case WM_MOUSEMOVE :
	case WM_COPY :
	case WM_CUT :
	case WM_PASTE :
	case WM_CLEAR :
	case WM_CONTEXTMENU :
		return TRUE;

	case WM_ENABLE :
		if (!wParam) return TRUE;
		break;

	case WM_PAINT :
		if (wParam) {
			SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			if (bHideTyping)
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
			else
				SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
		}
		break;

	case WM_SETFOCUS :
		SendMessage (hWnd, EM_SETSEL, 0xFFFF, 0xFFFF);
		break;

	case WM_KILLFOCUS :
		break;
	}
    return FALSE;
}


//----------------------------------------------------|
//  New passphrase 1 edit box subclass procedure

LRESULT APIENTRY WizPhrase1SubclassProc (HWND hWnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {

	if (WizCommonNewKeyPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = lPhrase1Len;
		lParam = (LPARAM)szPhrase1;
		break;

	case WM_CHAR :
		switch (wParam) {
		case VK_TAB :
			if (GetKeyState (VK_SHIFT) & 0x8000)
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_DUMMYSTOP));
			else
				SetFocus (GetDlgItem (GetParent (hWnd),
											IDC_EDIT_PASSPHRASE2));
			break;

		case VK_RETURN :
			{
				HWND hGrandParent = GetParent (GetParent (hWnd));
				PropSheet_PressButton (hGrandParent, PSBTN_NEXT);
			}
			break;
		}
		break;
	}
    return CallWindowProc(wpOrigPhrase1Proc, hWnd, uMsg, wParam, lParam);
}


//----------------------------------------------------|
//  New passphrase 2 edit box subclass procedure

LRESULT APIENTRY WizPhrase2SubclassProc (HWND hWnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {

	if (WizCommonNewKeyPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = lPhrase2Len;
		lParam = (LPARAM)szPhrase2;
		break;

	case WM_CHAR :
		switch (wParam) {
		case VK_TAB :
			if (GetKeyState (VK_SHIFT) & 0x8000)
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_EDIT_PASSPHRASE));
			else
				SetFocus (GetDlgItem (GetParent (hWnd),
										IDC_CHECK_HIDE_TYPING));
			break;

		case VK_RETURN :
			{
				HWND hGrandParent = GetParent (GetParent (hWnd));
				PropSheet_PressButton (hGrandParent, PSBTN_NEXT);
			}
			break;
		}
		break;
	}
    return CallWindowProc(wpOrigPhrase2Proc, hWnd, uMsg, wParam, lParam);
}


//----------------------------------------------------|
//  Signing passphrase edit box subclass procedure

LRESULT APIENTRY WizPhrase3SubclassProc (HWND hWnd, UINT uMsg,
								   WPARAM wParam, LPARAM lParam) {

	if (WizCommonNewKeyPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;
	switch (uMsg) {
	case WM_GETTEXT :
		wParam = lPhrase1Len;
		lParam = (LPARAM)szPhrase1;
		break;

	case WM_CHAR :
		switch (wParam) {
		case VK_TAB :
			if (GetKeyState (VK_SHIFT) & 0x8000)
				SetFocus (GetDlgItem (GetParent (hWnd), IDC_CHECK_SIGN_KEY));
			else
				SetFocus (GetDlgItem (GetParent (hWnd),
											IDC_CHECK_HIDE_TYPING));
			break;

		case VK_RETURN :
			{
				HWND hGrandParent = GetParent (GetParent (hWnd));
				PropSheet_PressButton (hGrandParent, PSBTN_NEXT);
			}
			break;
		}
		break;
	}
    return CallWindowProc(wpOrigPhrase1Proc, hWnd, uMsg, wParam, lParam);
}


//----------------------------------------------------|
//  Create wizard data structures and call PropertySheet
//	to actually create wizard

UINT KeyGenWizard (HWND hWnd, INT startsheet)
{
	DWORD dwThreadID;
    PAGEINFO page[12];
    PROPSHEETHEADER psh;
	INT iNumBits, iBitmap;
	HDC hDC;
	UINT uNumPages = 0, PageNum;
	UINT ReturnValue = PGPCOMDLG_OK;
	USERINFO UserInfo = {NULL,
						 NULL,
						 NULL,
						 DEFAULT_KEY_TYPE,
						 DEFAULT_KEY_SIZE,
						 DEFAULT_SUBKEY_SIZE,
						 0,
						 NULL,
						 0,
						 0,
						 NULL,
						 NULL,
						 0,
						 FALSE,
						 FALSE,
						 FALSE,
						 0,
						 {NULL, NULL, FALSE, NULL, NULL,
											SUCCESS, FALSE, FALSE},
						 FALSE,
						 NULL,
						 FALSE};

	uNumPages = (sizeof(page) / sizeof(PAGEINFO));

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 1) iBitmap = IDB_KEYWIZ1;
	else if (iNumBits <= 4) iBitmap = IDB_KEYWIZ4;
	else if (iNumBits <= 8) iBitmap = IDB_KEYWIZ8;
	else iBitmap = IDB_KEYWIZ24;

	for(PageNum = 0; PageNum < uNumPages; ++PageNum)
	{
		page[PageNum].psp.dwSize = sizeof(PAGEINFO);
		page[PageNum].psp.dwFlags = PSP_HASHELP;
		page[PageNum].psp.hInstance = g_hInst;
		page[PageNum].psp.pszIcon = NULL;
		page[PageNum].psp.pfnDlgProc = KeyWizardDefaultDlgProc;
		page[PageNum].psp.pszTitle = "Foo";
		page[PageNum].psp.lParam = (LPARAM) NULL;
		page[PageNum].psp.pfnCallback = NULL;
		page[PageNum].pUserInfo = &UserInfo;
		page[PageNum].WizFlags = PSWIZB_BACK | PSWIZB_NEXT;
		page[PageNum].BitmapResource = iBitmap;
		page[PageNum].HelpID = IDH_PGPKEYWIZ_ABOUT;
		page[PageNum].PageActive = FALSE;
	}

	page[0].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_ABOUT);
	page[0].psp.pfnDlgProc = KeyWizardDefaultDlgProc;
	page[0].WizFlags = PSWIZB_NEXT;
	page[0].HelpID = IDH_PGPKEYWIZ_ABOUT;

	page[1].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_NAME);
	page[1].psp.pfnDlgProc = KeyWizardNameDlgProc;
	page[1].HelpID = IDH_PGPKEYWIZ_NAME;

	page[2].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_TYPE);
	page[2].psp.pfnDlgProc = KeyWizardTypeDlgProc;
	page[2].HelpID = IDH_PGPKEYWIZ_TYPE;

	page[3].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_SIZE);
	page[3].psp.pfnDlgProc = KeyWizardSizeDlgProc;
	page[3].HelpID = IDH_PGPKEYWIZ_SIZE;

	page[4].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_EXPIRATION);
	page[4].psp.pfnDlgProc = KeyWizardExpirationDlgProc;
	page[4].HelpID = IDH_PGPKEYWIZ_EXPIRATION;

	page[5].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_PASSPHRASE);
	page[5].psp.pfnDlgProc = KeyWizardPassphraseDlgProc;
	page[5].HelpID = IDH_PGPKEYWIZ_PASSPHRASE;

	page[6].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_RANDOBITS);
	page[6].psp.pfnDlgProc = KeyWizardRandobitsDlgProc;
	page[6].WizFlags = PSWIZB_BACK;
	page[6].HelpID = IDH_PGPKEYWIZ_RANDOBITS;

	page[7].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_GENERATION);
	page[7].psp.pfnDlgProc = KeyWizardGenerationDlgProc;
	page[7].WizFlags = 0;
	page[7].HelpID = IDH_PGPKEYWIZ_GENERATION;

	page[8].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_SIGN_OLD);
	page[8].psp.pfnDlgProc = KeyWizardSignOldDlgProc;
	page[8].WizFlags = PSWIZB_NEXT;
	page[8].HelpID = IDH_PGPKEYWIZ_SIGNOLD;

	page[9].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_PRESEND);
	page[9].psp.pfnDlgProc = KeyWizardPreSendDlgProc;
	page[9].WizFlags = PSWIZB_NEXT;
	page[9].HelpID = IDH_PGPKEYWIZ_PRESEND;

	page[10].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_SEND);
	page[10].psp.pfnDlgProc = KeyWizardSendToServerDlgProc;
	page[10].WizFlags = 0;
	page[10].HelpID = IDH_PGPKEYWIZ_SEND;

	page[11].psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYWIZ_DONE);
	page[11].psp.pfnDlgProc = KeyWizardDoneDlgProc;
	page[11].WizFlags = PSWIZB_FINISH;
	page[11].HelpID = IDH_PGPKEYWIZ_DONE;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE | PSH_WIZARD;
	psh.hwndParent = hWnd;
	psh.hInstance = g_hInst;
	psh.pszIcon = NULL;
	psh.pszCaption = NULL;
	psh.nPages = uNumPages;
	psh.nStartPage = startsheet;
	psh.ppsp = (LPCPROPSHEETPAGE) &page;
	psh.pfnCallback = NULL;

	hWndCollectEntropy = NULL;
	bNeedsCentering = TRUE;

	dwThreadID = GetCurrentThreadId ();
	hHookKeyboard =
		SetWindowsHookEx (WH_KEYBOARD, WizKeyboardHookProc, NULL, dwThreadID);
	hHookDebug =
		SetWindowsHookEx (WH_DEBUG, WizDebugHookProc, NULL, dwThreadID);

	PropertySheet(&psh);

	UnhookWindowsHookEx (hHookDebug);
	UnhookWindowsHookEx (hHookKeyboard);

	if(UserInfo.FullName)
		pgpFree(UserInfo.FullName);

	if(UserInfo.Email)
		pgpFree(UserInfo.Email);

	if(UserInfo.UserID)
		pgpFree(UserInfo.UserID);

	if(UserInfo.PassPhrase)
	{
		memset(UserInfo.PassPhrase, '\0', strlen(UserInfo.PassPhrase));
		pgpFree(UserInfo.PassPhrase);
	}

	if(UserInfo.KeyBuffer)
		pgpFree(UserInfo.KeyBuffer);

	if(!UserInfo.FinishSelected)
	{
		PGPKeySet* pKeySet;
		if (UserInfo.pKey) {
			pKeySet = pgpNewSingletonKeySet(UserInfo.pKey);
			PGPcomdlgErrorMessage(pgpRemoveKeys(g_pKeySetMain, pKeySet));
			pgpFreeKeySet(pKeySet);
		}

		UserInfo.FinalResult = 0;
	}

	SendMessage(hWnd, KM_M_CREATEDONE, UserInfo.FinalResult,
							(LPARAM) UserInfo.pKey);

	return(0);
}


//----------------------------------------------------|
//	Dialog procedure for getting name and email address
//	from user

LRESULT WINAPI KeyWizardNameDlgProc (HWND hdlg, UINT Msg,
									 WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
	PAGEINFO *pPage;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage(hdlg, IDC_FULL_NAME, EM_SETLIMITTEXT,
				0, MAX_FULL_NAME_LEN);
			SendDlgItemMessage(hdlg, IDC_EMAIL, EM_SETLIMITTEXT,
				0, MAX_EMAIL_LEN);
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;
			if(pnmh->code == PSN_WIZNEXT)
			{
				unsigned long TextLen;
				BOOL Continue = TRUE;

				pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
				assert(pPage);

				if(pPage)
				{
					if(pPage->pUserInfo->FullName)
					{
						pgpFree(pPage->pUserInfo->FullName);
						pPage->pUserInfo->FullName = NULL;
					}

					if(pPage->pUserInfo->Email)
					{
						pgpFree(pPage->pUserInfo->Email);
						pPage->pUserInfo->Email = NULL;
					}

					TextLen = SendDlgItemMessage(hdlg, IDC_FULL_NAME,
						WM_GETTEXTLENGTH, 0, 0) + 1;
					pPage->pUserInfo->FullName =
						pgpAlloc((TextLen) * sizeof(char));
					if(pPage->pUserInfo->FullName)
						GetDlgItemText(hdlg, IDC_FULL_NAME,
										pPage->pUserInfo->FullName, TextLen);

					if(TextLen == 1)
					{
						if((KMMessageBox(hdlg, IDS_KW_TITLE,
										 IDS_KW_NO_FULL_NAME,
										 MB_YESNO|MB_ICONWARNING|
										 MB_DEFBUTTON2) != IDYES))
						{
							Continue = FALSE;
							SetFocus (GetDlgItem (hdlg, IDC_FULL_NAME));
						}
					}
					
					if(Continue)
					{
						TextLen = SendDlgItemMessage(hdlg, IDC_EMAIL,
												WM_GETTEXTLENGTH, 0, 0) + 1;
						pPage->pUserInfo->Email =
							pgpAlloc((TextLen) * sizeof(char));
						if(pPage->pUserInfo->Email)
							GetDlgItemText(hdlg, IDC_EMAIL,
											pPage->pUserInfo->Email, TextLen);

						if(TextLen == 1)
						{
							if((KMMessageBox(hdlg, IDS_KW_TITLE,
									IDS_KW_NO_EMAIL,
									MB_YESNO|MB_ICONWARNING|
									MB_DEFBUTTON2) != IDYES))
							{
								Continue = FALSE;
								SetFocus (GetDlgItem (hdlg, IDC_EMAIL));
							}
						}
					}

					if(!Continue)
					{
						SetWindowLong(hdlg, DWL_MSGRESULT, -1L);
						ReturnCode = TRUE;
						CallDefaultProc = FALSE;
					}
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for getting type of key (RSA/DSA)
//	from user

LRESULT WINAPI KeyWizardTypeDlgProc (HWND hdlg, UINT Msg,
									 WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
	PAGEINFO *pPage = NULL;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
			CallDefaultProc = FALSE;
			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);
			if(pPage)
			{
#ifdef PGP_FREEWARE
				pPage->pUserInfo->KeyType = PGP_PKALG_DSA;
				EnableWindow (GetDlgItem (hdlg, IDC_RADIO_RSA), FALSE);
				ShowWindow (GetDlgItem (hdlg, IDC_FREEWARETEXT), SW_SHOW);
#endif // PGP_FREEWARE
				switch(pPage->pUserInfo->KeyType)
				{
					case PGP_PKALG_RSA:
					{
						SendDlgItemMessage (hdlg, IDC_RADIO_RSA,
							BM_SETCHECK, BST_CHECKED, 0);
						break;
					}

					case PGP_PKALG_DSA:
					{
						SendDlgItemMessage (hdlg, IDC_RADIO_ELGAMAL,
							BM_SETCHECK, BST_CHECKED, 0);
						break;
					}
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;
			switch (pnmh->code) {
				case PSN_KILLACTIVE :
				{
					UINT KeyType = PGP_PKALG_DSA + PGP_PKALG_RSA;

					if(SendDlgItemMessage(hdlg, IDC_RADIO_RSA,
							BM_GETCHECK, 0, 0) == BST_CHECKED)
						KeyType = PGP_PKALG_RSA;
					else if(SendDlgItemMessage(hdlg, IDC_RADIO_ELGAMAL,
								BM_GETCHECK, 0, 0) == BST_CHECKED)
						KeyType = PGP_PKALG_DSA;

					if(KeyType != PGP_PKALG_RSA &&
					   KeyType != PGP_PKALG_DSA) /*Should never happen*/
					{
						KMMessageBox (hdlg, IDS_KW_TITLE,
										IDS_KW_INVALID_KEY_TYPE,
										MB_OK|MB_ICONERROR);
						SetWindowLong(hdlg, DWL_MSGRESULT, -1L);
						ReturnCode = TRUE;
						CallDefaultProc = FALSE;
					}
					else
					{
						pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
						assert(pPage);
						pPage->pUserInfo->KeyType = KeyType;
					}
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for getting size of key (number of bits)
//	from user

LRESULT WINAPI KeyWizardSizeDlgProc (HWND hdlg, UINT Msg,
									 WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
	PAGEINFO *pPage;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
			CallDefaultProc = FALSE;
			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);
			SendDlgItemMessage (hdlg, IDC_CUSTOM_BITS, EM_LIMITTEXT, 4, 0);
			if(pPage)
			{
				SendDlgItemMessage (hdlg, IDC_CUSTOM_BITS, WM_ENABLE,
												FALSE, 0);
				switch(pPage->pUserInfo->KeySize)
				{
					case 768:
					{
						SendDlgItemMessage (hdlg, IDC_RADIO_768,
							BM_SETCHECK, BST_CHECKED, 0);
						break;
					}

					case 1024:
					{
						SendDlgItemMessage (hdlg, IDC_RADIO_1024,
							BM_SETCHECK, BST_CHECKED, 0);
						break;
					}

					case 1536:
					{
						SendDlgItemMessage (hdlg, IDC_RADIO_1536,
							BM_SETCHECK, BST_CHECKED, 0);
						break;
					}

					case 2048:
					{
						SendDlgItemMessage (hdlg, IDC_RADIO_2048,
							BM_SETCHECK, BST_CHECKED, 0);
						break;
					}

					case 3072:
					{
						SendDlgItemMessage (hdlg, IDC_RADIO_3072,
							BM_SETCHECK, BST_CHECKED, 0);
						break;
					}

					default:
					{
						SendDlgItemMessage (hdlg, IDC_RADIO_CUSTOM,
							BM_SETCHECK, BST_CHECKED, 0);
						SetDlgItemInt(hdlg, IDC_CUSTOM_BITS,
							pPage->pUserInfo->KeySize, FALSE);
						SendDlgItemMessage (hdlg, IDC_CUSTOM_BITS,
							WM_ENABLE, TRUE, 0);
						break;
					}
				}
			}
			break;
		}

		case WM_COMMAND:
		{
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
				{
					if(LOWORD(wParam) == IDC_RADIO_CUSTOM)
					{
						SendDlgItemMessage (hdlg, IDC_CUSTOM_BITS,
							WM_ENABLE, TRUE, 0);
					}
					else
					{
						SendDlgItemMessage (hdlg, IDC_CUSTOM_BITS,
							WM_ENABLE, FALSE, 0);
					}
					break;
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);

			pnmh = (LPNMHDR) lParam;
			switch(pnmh->code)
			{
				case PSN_KILLACTIVE:
				{
					unsigned long MaxKeySize;
					unsigned long KeySize = 0;

					if(SendDlgItemMessage(hdlg, IDC_RADIO_768,
							BM_GETCHECK, 0, 0) == BST_CHECKED)
						KeySize = 768;
					else if(SendDlgItemMessage(hdlg, IDC_RADIO_1024,
							BM_GETCHECK, 0, 0) == BST_CHECKED)
						KeySize = 1024;
					else if(SendDlgItemMessage(hdlg, IDC_RADIO_1536,
							BM_GETCHECK, 0, 0) == BST_CHECKED)
						KeySize = 1536;
					else if(SendDlgItemMessage(hdlg, IDC_RADIO_2048,
							BM_GETCHECK, 0, 0) == BST_CHECKED)
						KeySize = 2048;
					else if(SendDlgItemMessage(hdlg, IDC_RADIO_3072,
							BM_GETCHECK, 0, 0) == BST_CHECKED)
						KeySize = 3072;
					else if(SendDlgItemMessage(hdlg, IDC_RADIO_CUSTOM,
							BM_GETCHECK, 0, 0) == BST_CHECKED)
					{
						KeySize = GetDlgItemInt(hdlg, IDC_CUSTOM_BITS,
							NULL, FALSE);
					}

					if(pPage->pUserInfo->KeyType == PGP_PKALG_RSA)
						MaxKeySize = MAX_RSA_KEY_SIZE;
					else
						MaxKeySize = MAX_DSA_KEY_SIZE;

					if(KeySize < MIN_KEY_SIZE || KeySize > MaxKeySize)
					{
						char Error[1024], Title[1024], Temp[1024];
						LoadString(g_hInst, IDS_KW_INVALID_KEY_SIZE,
							Temp, sizeof(Temp));
						LoadString(g_hInst, IDS_KW_TITLE,
							Title, sizeof(Title));
						sprintf(Error, Temp, MIN_KEY_SIZE, MaxKeySize);
						MessageBox(hdlg, Error, Title, MB_OK | MB_ICONERROR);
						SetWindowLong(hdlg, DWL_MSGRESULT, -1L);
						ReturnCode = TRUE;
						CallDefaultProc = FALSE;
					}
					else
					{
						if(pPage->pUserInfo->KeyType == PGP_PKALG_DSA)
						{
							pPage->pUserInfo->KeySize =
								GetDSAKeySize(KeySize);
							pPage->pUserInfo->SubKeySize =
								KeySize;
						}
						else
						{
							pPage->pUserInfo->KeySize = KeySize;
						}
					}
					break;
				}

				case PSN_SETACTIVE:
				{
					CHAR szText[64];
					if(pPage->pUserInfo->KeyType == PGP_PKALG_DSA)
					{
						ShowWindow(GetDlgItem(hdlg, IDC_RADIO_3072), SW_SHOW);
						ShowWindow(GetDlgItem(hdlg, IDC_STATIC_SMALL_GUIDE),
											SW_SHOW);
						LoadString (g_hInst, IDS_KW_DSACUSTOM,
							szText, sizeof(szText));
						SetDlgItemText(hdlg, IDC_RADIO_CUSTOM, szText);
						LoadString (g_hInst, IDS_KW_DSA1536,
							szText, sizeof(szText));
						SetDlgItemText(hdlg, IDC_RADIO_1536, szText);
						LoadString (g_hInst, IDS_KW_DSA2048,
							szText, sizeof(szText));
						SetDlgItemText(hdlg, IDC_RADIO_2048, szText);
						LoadString (g_hInst, IDS_KW_DSA3072,
							szText, sizeof(szText));
						SetDlgItemText(hdlg, IDC_RADIO_3072, szText);
					}
					else
					{
						ShowWindow(GetDlgItem(hdlg, IDC_RADIO_3072), SW_HIDE);
						ShowWindow(GetDlgItem(hdlg, IDC_STATIC_SMALL_GUIDE),
											SW_HIDE);
						LoadString (g_hInst, IDS_KW_RSACUSTOM,
							szText, sizeof(szText));
						SetDlgItemText(hdlg, IDC_RADIO_CUSTOM, szText);
						LoadString (g_hInst, IDS_KW_RSA1536,
							szText, sizeof(szText));
						SetDlgItemText(hdlg, IDC_RADIO_1536, szText);
						LoadString (g_hInst, IDS_KW_RSA2048,
							szText, sizeof(szText));
						SetDlgItemText(hdlg, IDC_RADIO_2048, szText);
					}
					break;
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for getting expiration info of key
//	from user

LRESULT WINAPI KeyWizardExpirationDlgProc (HWND hdlg, UINT Msg,
										   WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
	BOOL InRange = TRUE;
	PAGEINFO *pPage;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
			CallDefaultProc = FALSE;

			SendDlgItemMessage(hdlg, IDC_EDIT_CUSTOM_DAYS,
									EM_SETLIMITTEXT, 4, 0);

			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);
			if(pPage)
			{
				if(!pPage->pUserInfo->ExpireDays)
				{
					SendDlgItemMessage (hdlg, IDC_RADIO_NEVER,
						BM_SETCHECK, BST_CHECKED, 0);
					SendDlgItemMessage (hdlg, IDC_EDIT_CUSTOM_DAYS,
						WM_ENABLE, FALSE, 0);
				}
				else
				{
					SendDlgItemMessage (hdlg, IDC_RADIO_CUSTOM_DAYS,
						BM_SETCHECK, BST_CHECKED, 0);
					SetDlgItemInt(hdlg, IDC_EDIT_CUSTOM_DAYS,
						pPage->pUserInfo->ExpireDays, FALSE);
					SendDlgItemMessage (hdlg, IDC_EDIT_CUSTOM_DAYS,
						WM_ENABLE, TRUE, 0);
				}
			}

			break;
		}

		case WM_COMMAND:
		{
			switch(HIWORD(wParam))
			{
				case BN_CLICKED:
				{
					if(LOWORD(wParam) == IDC_RADIO_CUSTOM_DAYS)
					{
						SendDlgItemMessage (hdlg, IDC_EDIT_CUSTOM_DAYS,
							WM_ENABLE, TRUE, 0);
					}
					else
					{
						SendDlgItemMessage (hdlg, IDC_EDIT_CUSTOM_DAYS,
							WM_ENABLE, FALSE, 0);
					}
					break;
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;
			if(pnmh->code == PSN_WIZNEXT)
			{
				UINT ExpireDays;

				if(SendDlgItemMessage(hdlg, IDC_RADIO_NEVER,
						BM_GETCHECK, 0, 0) == BST_CHECKED)
					ExpireDays = 0;
				else if(SendDlgItemMessage(hdlg, IDC_RADIO_CUSTOM_DAYS,
						BM_GETCHECK, 0, 0) == BST_CHECKED) {
					ExpireDays = GetDlgItemInt(hdlg, IDC_EDIT_CUSTOM_DAYS,
												NULL, FALSE);
				
					if ((ExpireDays<MIN_EXPIRATION) ||
						(ExpireDays>MAX_EXPIRATION))
					{
						char Error[1024], Title[1024], Temp[1024];
						LoadString(g_hInst, IDS_KW_INVALID_EXPIRATION,
									Temp, sizeof(Temp));
						LoadString(g_hInst, IDS_KW_TITLE,
									Title, sizeof(Title));
						sprintf(Error, Temp, MIN_EXPIRATION, MAX_EXPIRATION);
						MessageBox(hdlg, Error, Title, MB_OK | MB_ICONERROR);
						SetWindowLong(hdlg, DWL_MSGRESULT, -1L);
						ReturnCode = TRUE;
						CallDefaultProc = FALSE;
						InRange = FALSE;
					}
				}
				if (InRange)
				{
					pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
					assert(pPage);
					pPage->pUserInfo->ExpireDays = ExpireDays;
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for getting passphrase
//	from user

LRESULT WINAPI KeyWizardPassphraseDlgProc (HWND hdlg, UINT Msg,
										   WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
	PAGEINFO *pPage;
	static HBRUSH hBrushEdit;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
			CallDefaultProc = FALSE;

			wpOrigPhrase1Proc = (WNDPROC) SetWindowLong(GetDlgItem(hdlg,
							IDC_EDIT_PASSPHRASE),
							GWL_WNDPROC,
							(LONG) WizPhrase1SubclassProc);
			wpOrigPhrase2Proc = (WNDPROC) SetWindowLong(GetDlgItem(hdlg,
							IDC_EDIT_PASSPHRASE2),
							GWL_WNDPROC,
							(LONG) WizPhrase2SubclassProc);

			hBrushEdit = CreateSolidBrush (GetSysColor (COLOR_WINDOW));

			break;
		}

		case WM_CTLCOLOREDIT:
			if (((HWND)lParam == GetDlgItem (hdlg, IDC_EDIT_PASSPHRASE)) ||
				((HWND)lParam == GetDlgItem (hdlg, IDC_EDIT_PASSPHRASE2))) {
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (bHideTyping)
					SetTextColor ((HDC)wParam, GetSysColor(COLOR_WINDOW));
				else
					SetTextColor ((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
				return (BOOL)hBrushEdit;
			}
			break;

		case WM_DESTROY:
            SetWindowLong(GetDlgItem(hdlg, IDC_EDIT_PASSPHRASE),
							GWL_WNDPROC, (LONG)wpOrigPhrase1Proc);
	        SetWindowLong(GetDlgItem(hdlg, IDC_EDIT_PASSPHRASE2),
							GWL_WNDPROC, (LONG)wpOrigPhrase2Proc);
			DeleteObject (hBrushEdit);
			KMWipeEditBox (hdlg, IDC_EDIT_PASSPHRASE);
			KMWipeEditBox (hdlg, IDC_EDIT_PASSPHRASE2);
            break;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_CHECK_HIDE_TYPING:

					if (IsDlgButtonChecked (hdlg, IDC_CHECK_HIDE_TYPING) ==
							BST_CHECKED)
						bHideTyping = TRUE;
					else
						bHideTyping = FALSE;

					InvalidateRect(GetDlgItem(hdlg, IDC_EDIT_PASSPHRASE),
						NULL, TRUE);
					InvalidateRect(GetDlgItem(hdlg, IDC_EDIT_PASSPHRASE2),
						NULL, TRUE);
					break;
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;
			if(pnmh->code == PSN_SETACTIVE)
			{
				bHideTyping = TRUE;
				CheckDlgButton (hdlg, IDC_CHECK_HIDE_TYPING, BST_CHECKED);
			}
			else if(pnmh->code == PSN_WIZNEXT)
			{
				pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
				assert(pPage);

				if(pPage)
				{
					CHAR szDummy[4];

					lPhrase1Len = SendDlgItemMessage (hdlg,
						IDC_EDIT_PASSPHRASE, WM_GETTEXTLENGTH, 0, 0) +1;
					szPhrase1 = KMAlloc (lPhrase1Len);
					if (szPhrase1)
						GetDlgItemText (hdlg, IDC_EDIT_PASSPHRASE,
										szDummy, sizeof(szDummy));

					lPhrase2Len = SendDlgItemMessage (hdlg,
						IDC_EDIT_PASSPHRASE2, WM_GETTEXTLENGTH, 0, 0) +1;
					szPhrase2 = KMAlloc (lPhrase2Len);
					if (szPhrase2)
						GetDlgItemText (hdlg, IDC_EDIT_PASSPHRASE2,
										szDummy, sizeof(szDummy));

					if(szPhrase1 && szPhrase2)
					{
						if (strcmp (szPhrase1, szPhrase2) == 0)
						{
							if(ValidatePassPhrase(hdlg, szPhrase1))
							{
								pPage->pUserInfo->PassPhrase =
									pgpAlloc((lPhrase1Len) * sizeof(char));

								strcpy(pPage->pUserInfo->PassPhrase,
									szPhrase1);
							}
							else
							{
								SetDlgItemText(hdlg,IDC_EDIT_PASSPHRASE,"");
								SetDlgItemText(hdlg,IDC_EDIT_PASSPHRASE2,"");
								SetFocus (GetDlgItem (hdlg,
													IDC_EDIT_PASSPHRASE));
								SetWindowLong(hdlg, DWL_MSGRESULT, -1L);
								ReturnCode = TRUE;
								CallDefaultProc = FALSE;
							}
						}
						else
						{
							KMMessageBox(hdlg, IDS_KW_TITLE,
								IDS_KW_MISMATCHED_PHRASES,
								MB_OK | MB_ICONERROR);
							SetDlgItemText(hdlg, IDC_EDIT_PASSPHRASE, "");
							SetDlgItemText(hdlg, IDC_EDIT_PASSPHRASE2, "");
							SetFocus (GetDlgItem (hdlg, IDC_EDIT_PASSPHRASE));
							SetWindowLong(hdlg, DWL_MSGRESULT, -1L);
							ReturnCode = TRUE;
							CallDefaultProc = FALSE;
						}
						KMFree(szPhrase1);
						KMFree(szPhrase2);
					}
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for getting entropy
//	from user

LRESULT WINAPI KeyWizardRandobitsDlgProc (HWND hdlg, UINT Msg,
										  WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
	PAGEINFO *pPage;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
			CallDefaultProc = FALSE;
			break;
		}

		case WM_MOUSEMOVE:
		case WM_TIMER:
		{
			int PercentComplete = 0;
			float ftot, fsofar;

			ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
			CallDefaultProc = FALSE;

			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);

			if(pPage->PageActive && pPage->pUserInfo->RandomBitsNeeded)
			{
				fsofar = (float) pgpRandPoolEntropy();
				ftot = (float) pPage->pUserInfo->RandomBitsNeeded;
				fsofar -= (float) pPage->pUserInfo->OriginalEntropy;
				ftot -= (float) pPage->pUserInfo->OriginalEntropy;
				PercentComplete = (int) ((fsofar / ftot) * 100.0);

				if(ftot <= fsofar)
				{
					pPage->pUserInfo->RandomBitsNeeded = 0;
					pPage->WizFlags = PSWIZB_NEXT | PSWIZB_BACK;
					SendMessage(GetParent(hdlg), PSM_SETWIZBUTTONS,
										0, pPage->WizFlags);
				}
				SendDlgItemMessage (hdlg, IDC_PROGRESS, PBM_SETPOS,
										PercentComplete, 0);
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;
			if(pnmh->code == PSN_KILLACTIVE)
			{
				pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
				assert(pPage);
				if(pPage->pUserInfo->RandomBitsNeeded >
					(long) pgpRandPoolEntropy())
				{
					KMMessageBox(hdlg, IDS_KW_TITLE, IDS_KW_NEED_MORE_BITS,
						MB_OK);
					SetWindowLong(hdlg, DWL_MSGRESULT, -1L);
					ReturnCode = TRUE;
					CallDefaultProc = FALSE;
					hWndCollectEntropy = NULL;
				}
			}
			else
			{
				if(pnmh->code == PSN_SETACTIVE)
				{
					long TotalBitsNeeded, RandPoolSize;
					hWndCollectEntropy = hdlg;
					/*Check to see if there are random bits needed.*/
					pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
					assert(pPage);
					if(pPage)
					{
						TotalBitsNeeded =
							pgpKeyEntropyNeeded(
								(byte) pPage->pUserInfo->KeyType,
								(unsigned) pPage->pUserInfo->KeySize);
						if(pPage->pUserInfo->KeyType == PGP_PKALG_DSA)
						{
							TotalBitsNeeded += pgpKeyEntropyNeeded(
												PGP_PKALG_ELGAMAL,
												pPage->pUserInfo->SubKeySize);
						}
						RandPoolSize = pgpRandPoolSize();

						pPage->pUserInfo->RandomBitsNeeded =
								min(TotalBitsNeeded, RandPoolSize);
					}
					if(!pPage ||
						(pPage &&
						((long)(long)pPage->pUserInfo->RandomBitsNeeded -
							(long) pgpRandPoolEntropy()) > 0))
					{
						pPage->pUserInfo->OriginalEntropy =
												pgpRandPoolEntropy();

						SendDlgItemMessage (hdlg, IDC_PROGRESS,
							PBM_SETRANGE, 0, MAKELPARAM(0, 100));
						SendDlgItemMessage (hdlg, IDC_PROGRESS,
							PBM_SETPOS, 0, 0);
					}
					else
					{
						SetWindowLong(hdlg, DWL_MSGRESULT, -1L); /*Skip*/
						CallDefaultProc = FALSE;
						ReturnCode = TRUE;
						hWndCollectEntropy = NULL;
					}
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for actually generating the key(s)

LRESULT WINAPI KeyWizardGenerationDlgProc (HWND hdlg, UINT Msg,
										   WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
    PAGEINFO *pPage = NULL;
	DWORD dwThreadID;
	char PhaseString[128];
	static BOOL bAVIFinished;
	static BOOL bAVIStarted;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			HANDLE hThread;

			ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
			CallDefaultProc = FALSE;
			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);
			pPage->pUserInfo->WorkingPhase = IDS_KEYGENPHASE1;
			LoadString(g_hInst, pPage->pUserInfo->WorkingPhase,
					   PhaseString, sizeof(PhaseString));
			SetDlgItemText (hdlg, IDC_PHASE, PhaseString);

			pPage->hwnd = hdlg;

			pPage->pUserInfo->InGeneration = TRUE;

			/*Kick off generation proc, here*/
			hThread = (HANDLE) _beginthreadex (NULL, 0,
				(LPTHREAD_START_ROUTINE)KeyCreationThread,
				(void *) pPage, 0, &dwThreadID);
			break;
		}

		case KM_M_CHANGEPHASE:
		{
			CallDefaultProc = FALSE;
			ReturnCode = TRUE;
			LoadString(g_hInst, lParam, PhaseString, sizeof(PhaseString));
			SetDlgItemText(hdlg, IDC_PHASE, PhaseString);
			break;
		}

		case WM_TIMER :
		{
			if (wParam == AVI_TIMER) {
				KillTimer (hdlg, AVI_TIMER);
				ReturnCode = TRUE;
				CallDefaultProc = FALSE;
				if (bAVIStarted) {
					bAVIFinished = TRUE;
					pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
					if (!pPage->pUserInfo->InGeneration)
						PostMessage (hdlg, KM_M_GENERATION_COMPLETE, 0, 0);
				}
				else {
					char szFile[32];
					char AnimationFile[_MAX_PATH], *p;

					strcpy(AnimationFile, g_szHelpFile);

					p = strrchr(AnimationFile, '\\');
					if(!p)
						p = AnimationFile;
					else
						++p;

					*p = '\0';
					LoadString (g_hInst, IDS_ANIMATIONFILE,
						szFile, sizeof(szFile));
					strcat(AnimationFile, szFile);

					Animate_Open(GetDlgItem(hdlg, IDC_WORKINGAVI),
						AnimationFile);
					Animate_Play(GetDlgItem(hdlg, IDC_WORKINGAVI),
						0, -1, -1);

					SetTimer (hdlg, AVI_TIMER, AVI_RUNTIME, NULL);
					bAVIStarted = TRUE;
				}
			}
			break;
		}

		case KM_M_GENERATION_COMPLETE:
		{
			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);

			if (pPage->PageActive) {
				if (bAVIFinished)
					Animate_Close (GetDlgItem (hdlg, IDC_WORKINGAVI));
	
				if (pPage->pUserInfo->CancelPending) {
					LoadString (g_hInst, IDS_KW_CANCELING,
						PhaseString, sizeof(PhaseString));
				}
				else {
					LoadString (g_hInst, IDS_KW_COMPLETE,
						PhaseString, sizeof(PhaseString));
				}
				SetDlgItemText (hdlg, IDC_PHASE, PhaseString);
				pPage->WizFlags = PSWIZB_NEXT;
				SendMessage (GetParent(hdlg),
					PSM_SETWIZBUTTONS, 0, pPage->WizFlags);
			}
						
			CallDefaultProc = FALSE;
			ReturnCode = TRUE;
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);

			pnmh = (LPNMHDR) lParam;
			switch(pnmh->code)
			{
				case PSN_SETACTIVE:
				{
					bAVIFinished = FALSE;
					bAVIStarted = FALSE;
					SetTimer (hdlg, AVI_TIMER, 100, NULL);	// delay a few ms
															// before drawing
															// AVI
					break;
				}

				case PSN_KILLACTIVE:
				{
					LoadString(g_hInst, IDS_KW_CANCELING,
						PhaseString, sizeof(PhaseString));
					SetDlgItemText(hdlg, IDC_PHASE, PhaseString);
					Animate_Close(GetDlgItem(hdlg, IDC_WORKINGAVI));
					break;
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for querying user about signing new
//  key with old key

LRESULT WINAPI KeyWizardSignOldDlgProc (HWND hdlg, UINT Msg,
										WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
	PAGEINFO *pPage;
	static HBRUSH hBrushEdit;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
			CallDefaultProc = FALSE;

			wpOrigPhrase1Proc = (WNDPROC) SetWindowLong(GetDlgItem(hdlg,
										IDC_EDIT_PASSPHRASE),
										GWL_WNDPROC,
										(LONG) WizPhrase3SubclassProc);

			hBrushEdit = CreateSolidBrush (GetSysColor (COLOR_WINDOW));

			break;
		}

		case WM_CTLCOLOREDIT:
			if (((HWND)lParam == GetDlgItem (hdlg, IDC_EDIT_PASSPHRASE)) ||
				((HWND)lParam == GetDlgItem (hdlg, IDC_EDIT_PASSPHRASE2))) {
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (bHideTyping)
					SetTextColor ((HDC)wParam, GetSysColor(COLOR_WINDOW));
				else
					SetTextColor ((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
				return (BOOL)hBrushEdit;
			}
			break;

		case WM_DESTROY:
             SetWindowLong(GetDlgItem(hdlg, IDC_EDIT_PASSPHRASE),
							GWL_WNDPROC, (LONG)wpOrigPhrase1Proc);
			DeleteObject (hBrushEdit);
			KMWipeEditBox (hdlg, IDC_EDIT_PASSPHRASE);
            break;


		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_CHECK_HIDE_TYPING:
				{
					if (IsDlgButtonChecked (hdlg, IDC_CHECK_HIDE_TYPING) ==
							BST_CHECKED)
						bHideTyping = TRUE;
					else
						bHideTyping = FALSE;

					InvalidateRect(GetDlgItem(hdlg, IDC_EDIT_PASSPHRASE),
										NULL, TRUE);
					break;
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);

			pnmh = (LPNMHDR) lParam;
			switch(pnmh->code)
			{
				case PSN_KILLACTIVE:
				{
					if(pPage &&
						SendDlgItemMessage (hdlg, IDC_CHECK_SIGN_KEY,
								BM_GETCHECK, 0, 0) == BST_CHECKED)
					{
						BOOL AllOk = FALSE;
						PGPUserID* pUserID;
						CHAR szDummy[4];

						lPhrase1Len = SendDlgItemMessage (hdlg,
										IDC_EDIT_PASSPHRASE,
										WM_GETTEXTLENGTH, 0, 0) +1;
						szPhrase1 = KMAlloc (lPhrase1Len);
						if (szPhrase1)
							GetDlgItemText (hdlg, IDC_EDIT_PASSPHRASE,
											szDummy, sizeof(szDummy));

						if(szPhrase1)
						{
							pUserID = pgpGetPrimaryUserID(
											pPage->pUserInfo->pKey);

							if(pUserID)
							{
								INT iError = PGPERR_KEYDB_BADPASSPHRASE;

								iError = pgpCertifyUserID(pUserID,
												pPage->pUserInfo->pOldKey,
												szPhrase1);
								if(iError == PGPERR_KEYDB_BADPASSPHRASE)
								{
									KMMessageBox(hdlg, IDS_KW_TITLE,
												IDS_KW_BAD_PASSPHRASE,
												MB_OK | MB_ICONERROR);
								}
								else
								{
									if(iError != PGPERR_OK)
										PGPcomdlgErrorMessage(iError);
									else
										AllOk = TRUE;
								}
							}						
							KMFree (szPhrase1);
						}
						if(!AllOk)
						{
							SetWindowLong(hdlg, DWL_MSGRESULT, -1L);
							ReturnCode = TRUE;
							CallDefaultProc = FALSE;
						}
					}
					break;
				}

				case PSN_SETACTIVE:
				{
					if(!pPage->pUserInfo->pOldKey)
					{
						SetWindowLong(hdlg, DWL_MSGRESULT, -1L); /*Skip*/
						CallDefaultProc = FALSE;
						ReturnCode = TRUE;
					}
					else
					{
						bHideTyping = TRUE;
						CheckDlgButton (hdlg, IDC_CHECK_HIDE_TYPING,
							BST_CHECKED);
						SendDlgItemMessage (hdlg, IDC_CHECK_SIGN_KEY,
							BM_SETCHECK, BST_CHECKED, 0);
					}

					break;
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for querying about sending to keyserver

LRESULT WINAPI KeyWizardPreSendDlgProc (HWND hdlg, UINT Msg,
										WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
    PAGEINFO *pPage = NULL;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage (hdlg, IDC_CHECK_SEND,
				BM_SETCHECK, BST_UNCHECKED, 0);
			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_CHECK_SEND:
				{
					pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
					assert(pPage);
					if(IsDlgButtonChecked(hdlg, IDC_CHECK_SEND) ==
							BST_CHECKED)
						pPage->pUserInfo->DoSend = TRUE;
					else
 						pPage->pUserInfo->DoSend = FALSE;
					break;
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);

			pnmh = (LPNMHDR) lParam;
			switch(pnmh->code)
			{
				case PSN_SETACTIVE:
				{
					if(!g_bKeyServerSupportLoaded)
					{
						SetWindowLong(hdlg, DWL_MSGRESULT, -1L); /*Skip*/
						CallDefaultProc = FALSE;
						ReturnCode = TRUE;
					}
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for displaying keyserver communication status

LRESULT WINAPI KeyWizardSendToServerDlgProc(HWND hdlg, UINT Msg,
											WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
    PAGEINFO *pPage = NULL;

	switch(Msg)
	{
		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);

			pnmh = (LPNMHDR) lParam;
			switch(pnmh->code)
			{
				case PSN_SETACTIVE:
				{
					if(!pPage->pUserInfo->DoSend ||
						!g_bKeyServerSupportLoaded)
					{
						SetWindowLong(hdlg, DWL_MSGRESULT, -1L); /*Skip*/
						CallDefaultProc = FALSE;
						ReturnCode = TRUE;
					}
					else
					{
						PGPKeySet *pKeySet = NULL;
						unsigned long BufSize = 0;
						UINT ExportReturnCode, RetryCount = 0;

						pKeySet =
							pgpNewSingletonKeySet(pPage->pUserInfo->pKey);
						SendDlgItemMessage (hdlg, IDC_PROGRESS,
							PBM_SETRANGE, 0, MAKELPARAM(0, 100));
						SendDlgItemMessage (hdlg, IDC_PROGRESS,
							PBM_SETPOS, 0, 0);

						pPage->pUserInfo->KeyBuffer = NULL;

						do
						{
							if(pPage->pUserInfo->KeyBuffer)
								pgpFree(pPage->pUserInfo->KeyBuffer);

							BufSize += 1;

							pPage->pUserInfo->KeyBuffer =
								pgpAlloc(BufSize * sizeof(char));

							ExportReturnCode = pgpExportKeyBuffer(pKeySet,
										pPage->pUserInfo->KeyBuffer,
										&BufSize);

							++RetryCount;
							*(pPage->pUserInfo->KeyBuffer + (BufSize - 1)) =
								'\0';
						} while(ExportReturnCode == SUCCESS &&
							   !strstr(pPage->pUserInfo->KeyBuffer,
							   "-----END PGP PUBLIC KEY BLOCK-----") &&
							   RetryCount < 10);

						if(pKeySet)
							pgpFreeKeySet(pKeySet);

						pPage->pUserInfo->PutKeyArgs.hWorkingDlg = hdlg;
						pPage->pUserInfo->PutKeyArgs.hParent = NULL;
						pPage->pUserInfo->PutKeyArgs.CancelPending = FALSE;
						pPage->pUserInfo->PutKeyArgs.UserId =
							pPage->pUserInfo->Email;
						pPage->pUserInfo->PutKeyArgs.KeyBuffer =
							&(pPage->pUserInfo->KeyBuffer);
						pPage->pUserInfo->PutKeyArgs.ReturnCode = SUCCESS;
						pPage->pUserInfo->PutKeyArgs.IsGet = FALSE;
						StartPutKeyInfoThread(&(
							pPage->pUserInfo->PutKeyArgs));
					}
					break;
				}

				case PSN_QUERYCANCEL:
				{
					if (!pPage->pUserInfo->PutComplete)
					{
						pPage->pUserInfo->CancelPending = TRUE;
						pPage->pUserInfo->PutKeyArgs.CancelPending = TRUE;
						SetWindowLong(hdlg, DWL_MSGRESULT, -1L);
						CallDefaultProc = FALSE;
						ReturnCode = TRUE;
					}
					break;
				}
			}
			break;
		}

		case PGPM_PROGRESS_UPDATE:
		{
			ReturnCode = TRUE;
			CallDefaultProc = FALSE;
			SetDlgItemText(hdlg, IDC_PROGRESS_TEXT, (char *) wParam);
			SendDlgItemMessage(hdlg, IDC_PROGRESS,
				PBM_SETPOS, (WPARAM) lParam, 0);
			break;
		}

		case PGPM_DISMISS_PROGRESS_DLG:
		{
			CHAR szText[32];
			SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETPOS, 100, 0);
			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);

			switch (pPage->pUserInfo->PutKeyArgs.ReturnCode) {
			case SUCCESS :
				LoadString (g_hInst, IDS_KW_COMPLETE, szText, sizeof(szText));
				SetDlgItemText(hdlg, IDC_PROGRESS_TEXT, szText);
				EnableWindow (GetDlgItem (GetParent (hdlg), IDCANCEL), FALSE);
				pPage->pUserInfo->PutComplete = TRUE;
				break;

			case ERR_USER_CANCEL :
				LoadString (g_hInst, IDS_KW_CANCELED,
					szText, sizeof(szText));
				SetDlgItemText(hdlg, IDC_PROGRESS_TEXT, szText);
				pPage->pUserInfo->PutComplete = TRUE;
				break;

			default :
				DisplayKSError(hdlg, pPage->pUserInfo->PutKeyArgs.ReturnCode);
				pPage->pUserInfo->PutComplete = TRUE;
				break;
			}

			PostMessage(GetParent(hdlg), PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);
			ReturnCode = TRUE;
			CallDefaultProc = FALSE;
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for "Finish" dialog

LRESULT WINAPI KeyWizardDoneDlgProc (HWND hdlg, UINT Msg,
									 WPARAM wParam, LPARAM lParam)
{
	BOOL CallDefaultProc = TRUE;
	BOOL ReturnCode = FALSE;
    PAGEINFO *pPage = NULL;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			assert(pPage);

			pnmh = (LPNMHDR) lParam;
			switch(pnmh->code)
			{
				case PSN_SETACTIVE:
				{
					if ((!pPage->pUserInfo->DoSend) ||
						(pPage->pUserInfo->PutKeyArgs.ReturnCode != SUCCESS))
					{
						CHAR szText[256];
						LoadString (g_hInst, IDS_KW_SENDLATER,
							szText, sizeof(szText));
						SetDlgItemText(hdlg, IDC_STATIC_KS_TEXT, szText);
					}
					else {
						EnableWindow (GetDlgItem (GetParent (hdlg), IDCANCEL),
										FALSE);
					}

					pPage->WizFlags = PSWIZB_FINISH;
				}
			}
			break;
		}
	}

	if(CallDefaultProc)
	{
		ReturnCode = KeyWizardDefaultDlgProc(hdlg, Msg, wParam, lParam);
	}
	return(ReturnCode);
}


//----------------------------------------------------|
//	Dialog procedure for handling beginning introductory
//  dialog, as well as providing default message handling
//  for other dialogs.

LRESULT WINAPI KeyWizardDefaultDlgProc (HWND hdlg, UINT Msg,
										WPARAM wParam, LPARAM lParam)
{
	BOOL ReturnCode = FALSE;
	static BOOL IsFirstPaint = TRUE;
    PAGEINFO *pPage = NULL;
	RECT rc;

	switch(Msg)
	{
		case WM_INITDIALOG:
		{
			// center dialog on screen
			if (bNeedsCentering) {
				GetWindowRect (GetParent (hdlg), &rc);
				SetWindowPos (GetParent (hdlg), NULL,
					(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))/2,
					(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))/3,
					0, 0, SWP_NOSIZE | SWP_NOZORDER);
				bNeedsCentering = FALSE;
			}

			if(lParam)
			{
				pPage = (PAGEINFO *) lParam;
				SetWindowLong(hdlg, GWL_USERDATA, lParam);
			}

			SetTimer(hdlg, 1, TIMERPERIOD, NULL);

			IsFirstPaint = TRUE;
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;
			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			if(pPage)
			{
				switch(pnmh->code)
				{
					case PSN_SETACTIVE:
					{
						pPage->PageActive = TRUE;
						PostMessage(GetParent(hdlg),
							PSM_SETWIZBUTTONS, 0, pPage->WizFlags);
						SetWindowLong(hdlg, DWL_MSGRESULT, 0L);
						ReturnCode = TRUE;
						break;
					}

					case PSN_KILLACTIVE:
					{
						pPage->PageActive = FALSE;
						SetWindowLong(hdlg, DWL_MSGRESULT, 0L);
						ReturnCode = TRUE;
						break;
					}

					case PSN_WIZFINISH:
					{
						pPage->PageActive = FALSE;
						pPage->pUserInfo->FinishSelected = TRUE;
						SetWindowLong(hdlg, DWL_MSGRESULT, 0L);
						ReturnCode = TRUE;
						break;
					}

					case PSN_HELP:
					{
						WinHelp (hdlg, g_szHelpFile, HELP_CONTEXT,
										pPage->HelpID);
						break;
					}

					case PSN_QUERYCANCEL:
					{
				/*If we're generating a key, don't let the user press
				 *cancel without asking.  If he says, "yes, I want to cancel,"
				 *then we'll still reject the message, but set CancelPending
				 *to TRUE, so that the next time the library comes around, we
				 *can nuke the thread.  The thread will then clear the
				 *InGeneration flag and re-send us the cancel message.
				 */
						if(pPage->pUserInfo->InGeneration)
						{
							if(!pPage->pUserInfo->CancelPending &&
								(KMMessageBox(hdlg, IDS_KW_TITLE,
									IDS_KW_CONFIRM_CANCEL,
									MB_YESNO | MB_ICONQUESTION) == IDYES))
							{
								pPage->pUserInfo->CancelPending = TRUE;
								PostMessage (hdlg, KM_M_GENERATION_COMPLETE,
									0, 0L);
							}
							SetWindowLong(hdlg, DWL_MSGRESULT, 1L);
							ReturnCode = TRUE;
						}
						else {
							SetWindowLong(hdlg, DWL_MSGRESULT, 0L);
							ReturnCode = TRUE;
						}
						break;
					}
				}

			}
			break;
		}

		case WM_PAINT:
		{
			pPage = (PAGEINFO *) GetWindowLong(hdlg, GWL_USERDATA);
			if(pPage)
			{
				PaintWizardBitmap(hdlg, IsFirstPaint, pPage->BitmapResource);
			}
			IsFirstPaint = FALSE;
			break;
		}

		case WM_MOUSEMOVE :
		{
			pgpRandPoolMouse (LOWORD (lParam), HIWORD (lParam));
			break;
		}
	
		case WM_TIMER:
		{
			LARGE_INTEGER newperf;
			SYSTEMTIME stime;

			if(QueryPerformanceCounter(&newperf))
			{
				pgpRandPoolKeystroke(newperf.LowPart);
			}
			else
			{
				GetSystemTime(&stime);
				pgpRandPoolKeystroke(stime.wMilliseconds);
			}
			break;
		}

		case WM_DESTROY:
		case WM_CLOSE:
		{
			KillTimer(hdlg, 1);
			break;
		}

	}

	return(ReturnCode);
}


//----------------------------------------------------|
//	Blit bitmap into dialog window

static void PaintWizardBitmap(HWND hdlg, BOOL IsFirstPaint,
							  UINT BitmapResource)
{
	HDC hDC, hMemDC;
	PAINTSTRUCT ps;
	HBITMAP hBitmapOld;
	BITMAP bm;
	static HBITMAP hBitmap;
	static HPALETTE hPalette;
	static UINT uTimeOut;
	static UINT uXsize, uYsize;
	static UINT uXpos, uYpos;

	if(IsFirstPaint)
	{
		hBitmap = LoadResourceBitmap (g_hInst,
									  MAKEINTRESOURCE (BitmapResource),
									  &hPalette);
		GetObject (hBitmap, sizeof(BITMAP), (LPSTR)&bm);

		uXsize = bm.bmWidth;
		uYsize = bm.bmHeight;

		uXpos = 0;
		uYpos = 13;
	}

	hDC = BeginPaint (hdlg, &ps);
	hMemDC = CreateCompatibleDC (hDC);
	if (hPalette)
	{
		SelectPalette (hDC, hPalette, FALSE);
		RealizePalette (hDC);
		SelectPalette (hMemDC, hPalette, FALSE);
		RealizePalette (hMemDC);
	}
	hBitmapOld = SelectObject (hMemDC, hBitmap);
	BitBlt (hDC, uXpos, uYpos, uXsize, uYsize, hMemDC, 0, 0, SRCCOPY);
	SelectObject (hMemDC, hBitmapOld);
	DeleteDC (hMemDC);
	EndPaint (hdlg, &ps);
}

//----------------------------------------------------|
//	test passphrase for validity

static BOOL ValidatePassPhrase(HWND hwnd, char *PassPhrase)
{
	BOOL PhraseIsBad = FALSE;
	BOOL UsePhrase = TRUE;

	if(!PassPhrase)
	{
		PhraseIsBad = TRUE;
	}
	else
	{
		if(strlen(PassPhrase) < 8)
			PhraseIsBad = TRUE;
	}

	if(PhraseIsBad)
	{
		UsePhrase = KMUseBadPassPhrase(hwnd);
	}
	
	return(UsePhrase);	
}


//----------------------------------------------------|
//	return DSA key size on basis of requested ElGamal key size

static unsigned long GetDSAKeySize (unsigned long Requested)
{
	unsigned long ActualBits = Requested;

	if(Requested > 1024) ActualBits = 1024;

	return(ActualBits);
}


//----------------------------------------------------|
//	thread for actually creating key

void KeyCreationThread(void *pArgs)
{
    PAGEINFO *pPage = NULL;
	INT iGenerationResult;
	BOOL NewDefaultKey = FALSE;

	assert(pArgs);

	pPage = (PAGEINFO *) pArgs;

	if(!pgpGetDefaultPrivateKey(g_pKeySetMain))
		NewDefaultKey = TRUE;

	iGenerationResult = CreatePGPUserID(&pPage->pUserInfo->UserID,
									   pPage->pUserInfo->FullName,
									   pPage->pUserInfo->Email);
	GetOldKey(&pPage->pUserInfo->pOldKey, pPage->pUserInfo->UserID);
	if(iGenerationResult == PGPERR_OK)
	{
		iGenerationResult = pgpGenerateKey(g_pKeySetMain,
									   (byte) pPage->pUserInfo->KeyType,
									   (unsigned) pPage->pUserInfo->KeySize,
									   (word16) pPage->pUserInfo->ExpireDays,
									  pPage->pUserInfo->UserID,
									  strlen(pPage->pUserInfo->UserID),
									  pPage->pUserInfo->PassPhrase,
									  KeyWizWorkingCallback,
									  (void *) pPage,
									  &(pPage->pUserInfo->pKey));

		if(iGenerationResult == PGPERR_OK)
		{
			if(pPage->pUserInfo->KeyType == PGP_PKALG_DSA ||
			    pPage->pUserInfo->KeyType == PGP_PKALG_ELGAMAL)
			{
				iGenerationResult = pgpGenerateSubKey(pPage->pUserInfo->pKey,
									 (unsigned) pPage->pUserInfo->SubKeySize,
									 (word16) pPage->pUserInfo->ExpireDays,
									 pPage->pUserInfo->PassPhrase,
									 pPage->pUserInfo->PassPhrase,
									 KeyWizWorkingCallback,
									 (void *) pPage);

			}
		}
	}

	if(iGenerationResult == PGPERR_OK && !pPage->pUserInfo->CancelPending)
	{
		pPage->pUserInfo->FinalResult = KCD_NEWKEY;
		if(NewDefaultKey)
			pPage->pUserInfo->FinalResult |= KCD_NEWDEFKEY;
	}
	else
	{
		if(!pPage->pUserInfo->CancelPending)
			PGPcomdlgErrorMessage(iGenerationResult);
	}

	pPage->pUserInfo->InGeneration = FALSE;
	if(pPage->pUserInfo->CancelPending)
	{
		PropSheet_PressButton(GetParent(pPage->hwnd), PSBTN_CANCEL);
	}
	else
	{
		PostMessage(pPage->hwnd, KM_M_GENERATION_COMPLETE, 0, 0);
	}
}


//-------------------------------------------------------------------|
// Load DIB bitmap and associated palette

static HBITMAP LoadResourceBitmap (HINSTANCE hInstance, LPSTR lpString,
                             HPALETTE FAR* lphPalette) {
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


static HPALETTE CreateDIBPalette (LPBITMAPINFO lpbmi, LPINT lpiNumColors) {
	LPBITMAPINFOHEADER lpbi;
	LPLOGPALETTE lpPal;
	HANDLE hLogPal;
	HPALETTE hPal = NULL;
	INT i;

	lpbi = (LPBITMAPINFOHEADER)lpbmi;
	if (lpbi->biBitCount <= 8)
		*lpiNumColors = (1 << lpbi->biBitCount);
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


//----------------------------------------------------|
//	create standard PGP userid from name and email address

static unsigned long CreatePGPUserID(char **UserID, char *FullName,
									 char *Email)
{
	INT ReturnCode = PGPERR_OK;
	unsigned long UserIDLen = 0;

	assert(UserID);
	assert(FullName);
	assert(Email);

	/*+4 is:  1 for the \0, one for the space, two for the broquets.*/
	UserIDLen = strlen(FullName) + strlen(Email) + 4;

	*UserID = pgpAlloc(sizeof(char) * UserIDLen);
	if(*UserID)
	{
		sprintf(*UserID, "%s <%s>", FullName, Email);
		assert(strlen(*UserID) == UserIDLen - 1);
	}
	else
		ReturnCode = PGPERR_NOMEM;

	return(ReturnCode);
}


//----------------------------------------------------|
//	callback routine called by library key generation routine
//  every so often with status of keygen.  Returning a nonzero
//  value cancels the key generation.

static int KeyWizWorkingCallback(void *pArg, INT Phase)
{
	int ReturnCode = 0;
    PAGEINFO *pPage = NULL;
	unsigned long OriginalPhase;
	
	assert(pArg);

	pPage = pArg;

	if(pPage)
	{
		OriginalPhase = pPage->pUserInfo->WorkingPhase;
		if(!pPage->pUserInfo->CancelPending)
		{
			if(Phase == ' ')
			{
				if(pPage->pUserInfo->WorkingPhase == IDS_KEYGENPHASE1)
					pPage->pUserInfo->WorkingPhase = IDS_KEYGENPHASE2;
				else
					pPage->pUserInfo->WorkingPhase = IDS_KEYGENPHASE1;
			}
			if(OriginalPhase != pPage->pUserInfo->WorkingPhase)
				PostMessage(pPage->hwnd,
							KM_M_CHANGEPHASE,
							0,
							(LPARAM) pPage->pUserInfo->WorkingPhase);
		}
		else /*Let the kernel know we're canceling*/
			ReturnCode = -1;

	}

	return(ReturnCode);
}


//----------------------------------------------------|
//	find key with specified user id, if it exists

static void GetOldKey(PGPKey** ppOldKey, char *UserID)
{
	PGPKeyList* pKeyList;
	PGPKeyIter* pKeyIter;
	PGPKeySet* pKeySet;
	PGPKey* pKey;
	Boolean bSecret;

	*ppOldKey = NULL;

	pKeySet = pgpFilterKeySetUserID (g_pKeySetMain, UserID, strlen(UserID));

	if(pKeySet)
	{
		pKeyList = pgpOrderKeySet(pKeySet, kPGPCreationOrdering);
		if(pKeyList)
		{
			pKeyIter = pgpNewKeyIter(pKeyList);
			if(pKeyIter)
			{
				pKey = pgpKeyIterNext(pKeyIter);

				while(pKey && !*ppOldKey)
				{
					pgpGetKeyBoolean (pKey, kPGPKeyPropIsSecret, &bSecret);
					if (bSecret)
					{
						*ppOldKey = pKey;
					}
					pKey = pgpKeyIterNext (pKeyIter);
				}
				pgpFreeKeyIter (pKeyIter);
			}
			pgpFreeKeyList (pKeyList);
		}
		pgpFreeKeySet(pKeySet);
	}
}


//----------------------------------------------------|
//  Create thread to handle dialog box

void KMCreateKey (HWND hParent) {
	DWORD dwNewKeyThreadID;

	EnableWindow (hParent, FALSE);

	_beginthreadex (NULL, 0, (LPTHREAD_START_ROUTINE)NewKeyWizThread,
		hParent, 0, &dwNewKeyThreadID);
}
