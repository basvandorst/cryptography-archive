//: KMUser.c - miscellaneous user interface routines
//
//	$Id: KMUser.c,v 1.5 1997/09/12 17:41:39 pbj Exp $
//

#include "pgpkmx.h"
#include "resource.h"

// external global variables
extern HINSTANCE g_hInst;

// local global variables
static DWORD aSignConfirmIds[] = {			// Help IDs
    0,0 
}; 

//----------------------------------------------------|
// Message box routine
//
//	iCaption and iMessage are string table resource IDs.

LRESULT KMMessageBox (HWND hWnd, INT iCaption, INT iMessage,
					  ULONG ulFlags) {
	CHAR szCaption [128];
	CHAR szMessage [512];

	LoadString (g_hInst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hInst, iMessage, szMessage, sizeof(szMessage));

	ulFlags |= MB_SETFOREGROUND;
	return (MessageBox (hWnd, szMessage, szCaption, ulFlags));
}


//----------------------------------------------------|
// Signature confirmation dialog message procedure

BOOL CALLBACK SignConfirmDlgProc(HWND hWndDlg, UINT uMsg, 
								 WPARAM wParam, LPARAM lParam) {
	PKEYMAN pKM;

	switch (uMsg) {
	case WM_INITDIALOG :
		SetWindowLong (hWndDlg, GWL_USERDATA, lParam);
		CheckDlgButton (hWndDlg, IDC_SENDSIG, BST_UNCHECKED);
		break;

	case WM_HELP: 
		pKM = (PKEYMAN)GetWindowLong (hWndDlg, GWL_USERDATA);
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, pKM->szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aSignConfirmIds); 
	    break; 
 
	case WM_CONTEXTMENU: 
		pKM = (PKEYMAN)GetWindowLong (hWndDlg, GWL_USERDATA);
	    WinHelp ((HWND) wParam, pKM->szHelpFile, HELP_CONTEXTMENU, 
	        (DWORD) (LPVOID) aSignConfirmIds); 
	    break; 

	case WM_COMMAND :
		switch (LOWORD(wParam)) {
		case IDCANCEL :
			EndDialog (hWndDlg, 0);
			break;

		case IDOK :
			if (IsDlgButtonChecked (hWndDlg, IDC_SENDSIG) == BST_CHECKED) 
				EndDialog (hWndDlg, 2);
			else 
				EndDialog (hWndDlg, 1);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
// Ask user for signature confirmation

BOOL KMSignConfirm (PKEYMAN pKM, BOOL* pbSendToServer) {
	INT iError;

	iError = DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_SIGNCONFIRM), 
						pKM->hWndParent, SignConfirmDlgProc, (LPARAM)pKM);

	switch (iError) {
	case 0 :
		*pbSendToServer = FALSE;
		return FALSE;

	case 1 :
		*pbSendToServer = FALSE;
		return TRUE;

	case 2 :
		*pbSendToServer = TRUE;
		return TRUE;
	}	
}


//----------------------------------------------------|
// construct userid from name and address edit box contents

BOOL KMConstructUserID (HWND hDlg, UINT uNameIDC, 
						UINT uAddrIDC, LPSTR* pszUserID) {
	UINT uNameLen, uAddrLen, uUserIDLen;
	CHAR* p;

	uNameLen = SendDlgItemMessage (hDlg, uNameIDC, WM_GETTEXTLENGTH, 0, 0);
	if (uNameLen == 0) {
		KMMessageBox (hDlg, IDS_CAPTION, IDS_NOUSERNAME, 
			MB_OK|MB_ICONEXCLAMATION);
		SetFocus (GetDlgItem (hDlg, uNameIDC));
		return FALSE;
	}

	uAddrLen = SendDlgItemMessage (hDlg, uAddrIDC, WM_GETTEXTLENGTH, 0, 0);
	if (uAddrLen == 0) {
		if (KMMessageBox (hDlg, IDS_CAPTION, IDS_NOUSERADDR, 
			MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDNO) {
			SetFocus (GetDlgItem (hDlg, uAddrIDC));
			return FALSE;
		}
	}

	uUserIDLen = uNameLen + uAddrLen + 4;
	*pszUserID = KMAlloc (uUserIDLen * sizeof(char));
	if (!*pszUserID) return FALSE;

	p = *pszUserID;
	GetDlgItemText (hDlg, uNameIDC, p, uUserIDLen);

	if (uAddrLen > 0) {
		p += uNameLen;
		lstrcpy (p, " <");

		p += 2;
		GetDlgItemText (hDlg, uAddrIDC, p, uUserIDLen);

		p += uAddrLen;
		lstrcpy (p, ">");
	}
}


//----------------------------------------------------|
// Wipe edit box clean

VOID KMWipeEditBox (HWND hDlg, UINT uID) {
	CHAR* p;
	INT i;

	i = SendDlgItemMessage (hDlg, uID, WM_GETTEXTLENGTH, 0, 0);
	if (i > 0) {
		p = KMAlloc (i+1);
		if (p) {
			FillMemory (p, i, ' ');
			SendDlgItemMessage (hDlg, uID, WM_SETTEXT, 0, (LPARAM)p);
			KMFree (p);
		}
	}
}


