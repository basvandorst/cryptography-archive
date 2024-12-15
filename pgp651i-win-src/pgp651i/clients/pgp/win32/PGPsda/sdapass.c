/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: sdapass.c,v 1.3 1999/03/31 23:23:55 wjb Exp $
____________________________________________________________________________*/
#include "windows.h"
#include "resource.h"
#include "pgpErrors.h"

extern HINSTANCE g_hinst;

// global variable structure for re-entrancy
typedef struct _GPP
{
	LPSTR				pszPassPhrase;
	LPSTR				pszPassPhraseConf;
	LPSTR				szDummy;
	WNDPROC				wpOrigPhrase1Proc;  
	WNDPROC				wpOrigPhrase2Proc;  
	INT					iNextTabControl;
	BOOL				bHideText;
	HWND				hwndOptions;
	char				*szPrompt;
} GPP;

void FreePassphrases(GPP *gpp)
{
	if(gpp->pszPassPhrase)
	{
		free(gpp->pszPassPhrase);
		gpp->pszPassPhrase=NULL;
	}

	if(gpp->szDummy)
	{
		free(gpp->szDummy);
		gpp->szDummy=NULL;
	}
}

// SetCapsLockMessageState shows or hides the caps lock message as needed.

void SetCapsLockMessageState(HWND hdlg)
{
	if (GetKeyState(VK_CAPITAL) & 1)
	{
		ShowWindow(GetDlgItem(hdlg,IDC_CAPSWARNING),SW_SHOW);
	}
	else
	{
		ShowWindow(GetDlgItem(hdlg,IDC_CAPSWARNING),SW_HIDE);
	}
}

CommonPhraseMsgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
    switch (uMsg) 
	{
		case WM_KEYUP:
		{
			SetCapsLockMessageState(GetParent(hwnd));
			break;
		}

//		case WM_RBUTTONDOWN :
//		case WM_CONTEXTMENU :
		case WM_LBUTTONDBLCLK :
		case WM_MOUSEMOVE :
		case WM_COPY :
		case WM_CUT :
		case WM_PASTE :
		case WM_CLEAR :
			return TRUE;

		case WM_LBUTTONDOWN :
			if (GetKeyState (VK_SHIFT) & 0x8000) return TRUE;
			break;

		case WM_PAINT :
		{
			GPP *gpp;

			gpp=(GPP *)GetWindowLong (GetParent(hwnd), GWL_USERDATA);

			if (wParam) 
			{
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (gpp->bHideText) 
					SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				else 
					SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOWTEXT));
			}
			break; 
		}

		case WM_KEYDOWN :
			if (GetKeyState (VK_SHIFT) & 0x8000) 
			{
				switch (wParam) 
				{
					case VK_HOME :
					case VK_END :
					case VK_UP :
					case VK_DOWN :
					case VK_LEFT :
					case VK_RIGHT :
					case VK_NEXT :
					case VK_PRIOR :
						return TRUE;
				}
			}
			break;

		case WM_SETFOCUS :
			SendMessage (hwnd, EM_SETSEL, 0xFFFF, 0xFFFF);
			break;
	}
    return FALSE; 
} 

//	______________________________________
//
//  Passphrase edit box subclass procedure

LRESULT APIENTRY 
PhraseSubclassProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	LRESULT				lResult;
	BOOL				OKactive;
	GPP					 *gpp;

	gpp=(GPP *)GetWindowLong (GetParent(hWnd), GWL_USERDATA);

	if (CommonPhraseMsgProc (hWnd, uMsg, wParam, lParam)) return 0;

	switch (uMsg) 
	{
		case WM_GETTEXT :
		{
			if (!gpp->pszPassPhrase) return 0;
			lParam = (LPARAM)gpp->pszPassPhrase;
			break;
		}

		case WM_CHAR :
		{
			if (wParam == VK_TAB) 
			{
				if (GetKeyState (VK_SHIFT) & 0x8000) 
					SetFocus (GetDlgItem (GetParent (hWnd), IDC_HIDETYPING));
				else 
					SetFocus (GetDlgItem (GetParent (hWnd), gpp->iNextTabControl));
			}
			else 
			{
				OKactive=TRUE;

				lResult = CallWindowProc (gpp->wpOrigPhrase1Proc, 
					hWnd, uMsg, wParam, lParam); 

				return lResult;
			}
			break;
		}
	}
    return CallWindowProc (gpp->wpOrigPhrase1Proc, 
		hWnd, uMsg, wParam, lParam); 
} 

//	__________________
//
//	Wipe edit box clean

VOID 
WipeEditBox (
		GPP *gpp,
		HWND hDlg, 
		UINT uID) 
{
	CHAR*	p;
	INT		i;

	i = SendDlgItemMessage (hDlg, uID, WM_GETTEXTLENGTH, 0, 0);
	if (i > 0) {
		p = (char *)malloc (i+1);
		if (p) {
			FillMemory (p, i, ' ');
			SendDlgItemMessage (hDlg, uID, WM_SETTEXT, 0, (LPARAM)p);
			free (p);
		}
	}
}

void ClearPassphrases(HWND hDlg,GPP *gpp)
{
	HWND hwndPhrase1;

	if(gpp->pszPassPhraseConf)
	{
		free(gpp->pszPassPhraseConf);
		gpp->pszPassPhraseConf=NULL;
	}

	if(gpp->szDummy)
	{
		free(gpp->szDummy);
		gpp->szDummy=NULL;
	}

	hwndPhrase1=GetDlgItem(hDlg,IDC_PHRASE1);

	if(hwndPhrase1)
	{
		WipeEditBox (gpp,hDlg, IDC_PHRASE1);
		SetWindowText (hwndPhrase1, "");
	}

	SetFocus (hwndPhrase1);
}

BOOL CALLBACK 
DoCommonCalls (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	GPP *gpp;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{

			SetWindowLong (hDlg, GWL_USERDATA, lParam);
			gpp=(GPP *)lParam;

			gpp->bHideText = TRUE;
			CheckDlgButton (hDlg, IDC_HIDETYPING, BST_CHECKED);

			SetCapsLockMessageState(hDlg);

			SetForegroundWindow (hDlg);

			// Force focus to passphrase box
			SetFocus(GetDlgItem(hDlg, IDC_PHRASE1));
			break;
		}

		case WM_KEYUP:
		{
			SetCapsLockMessageState(hDlg);
			break;
		}

		case WM_QUIT:
		case WM_CLOSE:
		case WM_DESTROY: 
		{
			HWND hwndPhrase1;

			gpp=(GPP *)GetWindowLong (hDlg, GWL_USERDATA);

			ClearPassphrases(hDlg,gpp);

			hwndPhrase1=GetDlgItem(hDlg, IDC_PHRASE1);

			if(hwndPhrase1)
			{
				SetWindowLong (hwndPhrase1,
					   GWL_WNDPROC, 
					   (LONG)gpp->wpOrigPhrase1Proc);
			}

			EndDialog(hDlg,kPGPError_UserAbort);
			break;
		}

		case WM_CTLCOLOREDIT:
		{
			HWND hwndPhrase1;

			gpp=(GPP *)GetWindowLong (hDlg, GWL_USERDATA);

			hwndPhrase1=GetDlgItem(hDlg, IDC_PHRASE1);

			if(lParam==0)
				break;

			if ((HWND)lParam == hwndPhrase1)
			{
				SetBkColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				if (gpp->bHideText) 
					SetTextColor ((HDC)wParam, GetSysColor (COLOR_WINDOW));
				else 
					SetTextColor ((HDC)wParam, 
							  GetSysColor (COLOR_WINDOWTEXT));
				return (BOOL)CreateSolidBrush (GetSysColor (COLOR_WINDOW));
			}
			break;
		}

		case WM_COMMAND:
		{
			gpp=(GPP *)GetWindowLong (hDlg, GWL_USERDATA);

			switch(LOWORD (wParam)) 
			{
				case IDCANCEL:
					EndDialog (hDlg, kPGPError_UserAbort);
					break;

				case IDC_HIDETYPING :
				{
					HWND hwndPhrase1;
	
					hwndPhrase1=GetDlgItem(hDlg, IDC_PHRASE1);

					if (IsDlgButtonChecked (hDlg, IDC_HIDETYPING)
							== BST_CHECKED) 
						gpp->bHideText = TRUE;
					else 
						gpp->bHideText = FALSE;

					if(hwndPhrase1!=NULL)
						InvalidateRect (hwndPhrase1, NULL, TRUE);
					break;
				}
			}
			break;	
		}
	}

	return FALSE;
}

BOOL CALLBACK 
pgpPassphraseDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	GPP				*gpp;
	INT				i;
	BOOL			Common;

	Common=DoCommonCalls (hDlg,uMsg,wParam,lParam); 

	if(Common)
		return Common;

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{
			gpp=(GPP *)GetWindowLong (hDlg, GWL_USERDATA);
	
			gpp->iNextTabControl = IDOK;

			gpp->wpOrigPhrase1Proc = (WNDPROC) SetWindowLong (
				GetDlgItem (hDlg, IDC_PHRASE1), 
				GWL_WNDPROC, 
				(LONG) PhraseSubclassProc); 

			SetWindowText(GetDlgItem(hDlg,IDC_PROMPTSTRING),gpp->szPrompt);

			return FALSE;
		}

		case WM_COMMAND:
		{
			gpp=(GPP *)GetWindowLong (hDlg, GWL_USERDATA);

	
			switch(LOWORD (wParam)) 
			{
				case IDOK: 
				{
					FreePassphrases(gpp);

					i = SendDlgItemMessage (hDlg, IDC_PHRASE1, 
						WM_GETTEXTLENGTH, 0, 0) +1;

					gpp->szDummy = (char *)malloc (i);

					if(gpp->szDummy)
					{
						gpp->pszPassPhrase = (char *)malloc (i);

						if (gpp->pszPassPhrase) 
						{
							GetDlgItemText (hDlg, IDC_PHRASE1, gpp->szDummy, i);

							ClearPassphrases(hDlg,gpp);
							EndDialog (hDlg, kPGPError_NoErr);
							break;
						}
					}
						
					// Couldn't allocate passphrases
					ClearPassphrases(hDlg,gpp);
					FreePassphrases(gpp);
					EndDialog (hDlg, kPGPError_OutOfMemory);
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

// Just a simple decryption
	PGPError
SDAPassphraseDialog(HWND hwnd,char *szPrompt,char **ppszPassPhrase)
{
	PGPError err;
	GPP	gpp;

	memset(&gpp,0x00,sizeof(GPP));

	gpp.szPrompt=szPrompt;
			
	err = DialogBoxParam (g_hinst, 
		MAKEINTRESOURCE (IDD_PASSPHRASE), 
		hwnd,
		(DLGPROC)pgpPassphraseDlgProc, (LPARAM)&gpp);
		
	*ppszPassPhrase=gpp.pszPassPhrase;

	return(err);
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

