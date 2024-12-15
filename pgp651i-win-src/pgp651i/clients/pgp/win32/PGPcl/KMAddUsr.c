/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMAddUsr.c - add userID to key
	

	$Id: KMAddUsr.c,v 1.26 1998/08/12 18:31:21 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// pgp header files
#include "PGPClientPrefs.h"

// system header files
#include <commdlg.h>

// constant definitions
#define MAX_FULL_NAME_LEN		126
#define MAX_EMAIL_LEN			126

// typedefs
typedef struct {
	PKEYMAN		pKM;
	LPSTR		pszUserID;
	PGPBoolean	bSyncWithServer;
} ADDNAMESTRUCT, *PADDNAMESTRUCT;

typedef struct {
	PKEYMAN		pKM;
	WNDPROC		wpOrigPhotoIDProc;
	HBITMAP		hbitmapPhotoID;
	HPALETTE	hpalettePhotoID;
	INT			iwidthPhotoID;
	INT			iheightPhotoID;
	LPBYTE		pPhotoBuffer;
	PGPSize		iPhotoBufferLength;
	PGPBoolean	bSyncWithServer;
} ADDPHOTOSTRUCT, *PADDPHOTOSTRUCT;

// external globals
extern HINSTANCE g_hInst;

// local globals
static DWORD aNewUserIds[] = {			// Help IDs
    IDC_NEWUSERNAME,	IDH_PGPKM_NEWUSERID, 
	IDC_NEWEMAILADDR,	IDH_PGPKM_NEWEMAILADDR,
    0,0 
}; 

static DWORD aNewPhotoIds[] = {			// Help IDs
	IDC_PHOTOID,		IDH_PGPKM_NEWPHOTOID,
    IDC_SELECTFILE,		IDH_PGPKM_BROWSENEWPHOTOID, 
    0,0 
}; 


//	___________________________________________________
//
//  Dialog Message procedure
//	When user asks to add a userID to a key, a dialog 
//	appears asking for the new userID to be typed in.
//	This is the message processing procedure for that
//	dialog.

static BOOL CALLBACK 
sAddUserDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{

	PADDNAMESTRUCT pans;

	switch(uMsg) {

	case WM_INITDIALOG:
		pans = (PADDNAMESTRUCT)lParam;
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		SendDlgItemMessage (hDlg, IDC_NEWUSERNAME, EM_SETLIMITTEXT, 
				MAX_FULL_NAME_LEN, 0);
		SendDlgItemMessage (hDlg, IDC_NEWEMAILADDR, EM_SETLIMITTEXT, 
				MAX_EMAIL_LEN, 0);
		return TRUE;

    case WM_HELP: 
		pans = (PADDNAMESTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pans->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aNewUserIds); 
        break; 
 
    case WM_CONTEXTMENU: 
 		pans = (PADDNAMESTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp ((HWND) wParam, pans->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aNewUserIds); 
        break; 

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
		case IDOK: 
			pans = (PADDNAMESTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			if (KMConstructUserID (hDlg, IDC_NEWUSERNAME, IDC_NEWEMAILADDR, 
					&(pans->pszUserID))) {
				EndDialog (hDlg, 1);
			}
			return TRUE;

		case IDCANCEL:
			EndDialog (hDlg, 0);
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

//	___________________________________________________
//
//  Add User to key
//	This routine is called when the user chooses to add
//	a userID to an existing secret key.

BOOL 
KMAddUserToKey (PKEYMAN pKM) 
{
	PGPByte*		pPasskey		= NULL;
	BOOL			bRetVal			= TRUE;
	PGPError		err;
	PGPKeyRef		key;
	PGPSize			sizePasskey;
	ADDNAMESTRUCT	ans;
	CHAR			sz[256];
	PGPPrefRef		prefref;

	// get selected key
	key = (PGPKeyRef)KMFocusedObject (pKM);

	// initialize struct
	ans.pKM = pKM;
	PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), &prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnAdd, 
						&(ans.bSyncWithServer));
	PGPclCloseClientPrefs (prefref, FALSE);

	// get new userid from user
	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_NEWUSERID), 
					pKM->hWndParent, sAddUserDlgProc, (LPARAM)&ans)) {

		// get valid passphrase, if required
		LoadString (g_hInst, IDS_SELKEYPASSPHRASE, sz, sizeof(sz)); 
		err = KMGetKeyPhrase (pKM->Context, pKM->tlsContext,
						pKM->hWndParent, sz,
						pKM->KeySetMain, key,
						NULL, &pPasskey, &sizePasskey);
		PGPclErrorBox (NULL, err);

		// now we have a valid passphrase, if required
		if (IsntPGPError (err)) {

			// update from server
			if (ans.bSyncWithServer) {
				if (!KMGetFromServerInternal (pKM, FALSE, FALSE, FALSE)) {
					if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
									IDS_QUERYCONTINUEADDING, 	
									MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
						bRetVal = FALSE;
					}
				}
			}
		
			if (bRetVal) {
				// make sure we have enough entropy
				PGPclRandom (pKM->Context, pKM->hWndParent, 0);

				if (pPasskey) {
					err = PGPAddUserID (key, ans.pszUserID, 
							PGPOPasskeyBuffer (pKM->Context, 
											pPasskey, sizePasskey),
							PGPOLastOption (pKM->Context));
				}
				else {
					err = PGPAddUserID (key, ans.pszUserID, 
							PGPOLastOption (pKM->Context));
				}
					 						
				if (IsntPGPError (PGPclErrorBox (NULL, err))) {
					KMCommitKeyRingChanges (pKM);
					KMUpdateKeyInTree (pKM, key, FALSE);
					InvalidateRect (pKM->hWndTree, NULL, TRUE);

					// send to server
					if (ans.bSyncWithServer) {
						KMSendToServer (pKM, PGPCL_DEFAULTSERVER);
					}
				}
				else bRetVal = FALSE;
			}
		}
		else bRetVal = FALSE;

		KMFree (ans.pszUserID);
	}
	else bRetVal = FALSE;

	if (pPasskey) {
		KMFreePasskey (pPasskey, sizePasskey);
		pPasskey = NULL;
	}

	return bRetVal;
}


//----------------------------------------------------|
//  display photo userID with appropriate overwriting

static VOID 
sPaintPhotoID (
		HWND		hWnd,
		HBITMAP		hbitmapID,
		HPALETTE	hpaletteID,
		INT			iwidthBM,
		INT			iheightBM)
{
	HPALETTE		hpaletteOld		= NULL;
	HDC				hdc;
	HDC				hdcMem;
	PAINTSTRUCT		ps;
	RECT			rc;
	INT				icent;
	INT				ileft, itop, iwidth, iheight;

	hdc = BeginPaint (hWnd, &ps);

	if (hbitmapID) {
		GetWindowRect (GetDlgItem (hWnd, IDC_PHOTOID), &rc);
		MapWindowPoints (NULL, hWnd, (LPPOINT)&rc, 2);

		// check if bitmap needs shrinking
		if ((iheightBM > (rc.bottom-rc.top-2)) ||
			(iwidthBM  > (rc.right-rc.left-2))) 
		{
			if (iheightBM > (iwidthBM * 1.25)) {
				itop = rc.top +1;
				iheight = rc.bottom-rc.top -2;
				icent = (rc.right+rc.left) / 2;
				iwidth = ((rc.bottom-rc.top) * iwidthBM) / iheightBM;
				ileft = icent -(iwidth/2);
			}
			else {
				ileft = rc.left +1;
				iwidth = rc.right-rc.left -2;
				icent = (rc.bottom+rc.top) / 2;
				iheight = ((rc.right-rc.left) * iheightBM) / iwidthBM;
				itop = icent - (iheight/2);
			}
		}
		// otherwise draw it at its real size
		else {
			iwidth = iwidthBM;
			iheight = iheightBM;
			icent = (rc.right+rc.left) / 2;
			ileft = icent - (iwidth/2);
			icent = (rc.bottom+rc.top) / 2;
			itop = icent - (iheight/2);
		}

		hdcMem = CreateCompatibleDC (hdc);

		if (hpaletteID) {
			hpaletteOld = SelectPalette (hdc, hpaletteID, FALSE);
			RealizePalette (hdc);
		}

		SetStretchBltMode (hdc, COLORONCOLOR);
		SelectObject (hdcMem, hbitmapID);
		StretchBlt (hdc, ileft, itop, iwidth, iheight,
					hdcMem, 0, 0, iwidthBM, iheightBM, SRCCOPY);

		if (hpaletteOld) {
			SelectPalette (hdc, hpaletteOld, TRUE);
			RealizePalette (hdc);
		}

		DeleteDC (hdcMem);
	}

	EndPaint (hWnd, &ps);
} 


//----------------------------------------------------|
//  update system palette

static BOOL
sUpdatePalette (
		HWND			hwnd,
		PADDPHOTOSTRUCT	paps)
{
	BOOL		bretval		= FALSE;
	HDC			hdc;
	HPALETTE	hpaletteOld;

	if (paps->hpalettePhotoID == NULL) return FALSE;

	hdc = GetDC (hwnd);

	hpaletteOld = SelectPalette (hdc, paps->hpalettePhotoID, FALSE);
	if (RealizePalette (hdc)) {
		InvalidateRect (hwnd, NULL, TRUE); 
		bretval = TRUE;
	}

	SelectPalette (hdc, hpaletteOld, TRUE);
	RealizePalette (hdc);
	ReleaseDC (hwnd, hdc);

	return bretval;
}


//----------------------------------------------------|
//  PhotoID subclass procedure

static LRESULT APIENTRY 
sPhotoIDSubclassProc (
		HWND	hWnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PADDPHOTOSTRUCT paps;
	paps = (PADDPHOTOSTRUCT)GetWindowLong (GetParent (hWnd), GWL_USERDATA);

    switch (uMsg) {

	case WM_CONTEXTMENU :
		{
			HMENU	hMC;
			HMENU	hMenuTrackPopup;

			hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUNEWPHOTOID));
			if (paps->hbitmapPhotoID)
				EnableMenuItem (hMC, IDM_COPYBITMAP, 
										MF_BYCOMMAND|MF_ENABLED);
			else
				EnableMenuItem (hMC, IDM_COPYBITMAP, 
										MF_BYCOMMAND|MF_GRAYED);

			if (IsClipboardFormatAvailable (CF_BITMAP) ||
				 IsClipboardFormatAvailable (CF_HDROP))
				EnableMenuItem (hMC, IDM_PASTEBITMAP, 
										MF_BYCOMMAND|MF_ENABLED);
			else
				EnableMenuItem (hMC, IDM_PASTEBITMAP, 
										MF_BYCOMMAND|MF_GRAYED);

			hMenuTrackPopup = GetSubMenu (hMC, 0);

			TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN|TPM_RIGHTBUTTON,
				LOWORD(lParam), HIWORD(lParam), 0, GetParent (hWnd), NULL);

			DestroyMenu (hMC);
		}
		break;

	case WM_DROPFILES :
		{
			CHAR				szFile[MAX_PATH];
			INT					iLen;
			LPBITMAPINFO		lpbmi;
			LPBYTE				pPhoto;
			INT					isize;
			PGPError			err;
			HWND				hwndParent;

			iLen = DragQueryFile ((HDROP)wParam, 0, szFile, sizeof(szFile));
			hwndParent = GetParent (hWnd);

			err = KMReadPhotoFromFile (szFile, &pPhoto, &isize);
			if (IsntPGPError (PGPclErrorBox (hWnd, err))) {
				SetActiveWindow (GetParent (hwndParent));
				if (paps->pPhotoBuffer) 
					KMFree (paps->pPhotoBuffer);
				if (paps->hbitmapPhotoID) 
					DeleteObject (paps->hbitmapPhotoID);
				if (paps->hpalettePhotoID) 
					DeleteObject (paps->hpalettePhotoID);

				paps->pPhotoBuffer = pPhoto;
				paps->iPhotoBufferLength = isize;
				EnableWindow (GetDlgItem (hwndParent, IDOK), TRUE); 

				err = KMDIBfromPhoto (pPhoto, isize, TRUE, &lpbmi); 

				if (IsntPGPError (err)) {
					KMGetDIBSize (lpbmi, &(paps->iwidthPhotoID),
						 &(paps->iheightPhotoID));

					paps->hbitmapPhotoID = KMDDBfromDIB (lpbmi, 
							&(paps->hpalettePhotoID)); 

					KMFree (lpbmi);

					InvalidateRect (hwndParent, NULL, TRUE);
				}
			} 

			DragFinish ((HDROP)wParam);
		}
		break;

	default :
		return CallWindowProc (paps->wpOrigPhotoIDProc, hWnd, uMsg, 
								wParam, lParam); 
	}
	return TRUE;
} 


//	___________________________________________________
//
//  select BMP file 

static VOID 
sSelectBitmapFile (
		HWND			hwnd,
		PADDPHOTOSTRUCT	paps)
{
	LPBYTE			pPhoto;
	INT				isize;
	CHAR			szFile[MAX_PATH];
	CHAR			szTitle[128];
	CHAR			szFilter[128];
	CHAR*			p;
	PGPError		err;
	OPENFILENAME	ofn;
	LPBITMAPINFO	lpbmi;

	szFile[0] = '\0';

	LoadString (g_hInst, IDS_BITMAPFILTER, szFilter, sizeof(szFilter));
	while (p = strrchr (szFilter, '@')) *p = '\0';

	LoadString (g_hInst, IDS_OPENBITMAPTITLE, szTitle, sizeof(szTitle));

	ofn.lStructSize       = sizeof (OPENFILENAME);
	ofn.hwndOwner         = hwnd;
	ofn.hInstance         = (HANDLE)g_hInst;
	ofn.lpstrFilter       = szFilter;
	ofn.lpstrCustomFilter = (LPTSTR)NULL;
	ofn.nMaxCustFilter    = 0L;
	ofn.nFilterIndex      = 1L;
	ofn.lpstrFile         = szFile;
	ofn.nMaxFile          = sizeof (szFile);
	ofn.lpstrFileTitle    = szTitle;
	ofn.nMaxFileTitle     = 0;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = szTitle;
	ofn.Flags			  = OFN_HIDEREADONLY|OFN_NOCHANGEDIR;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = NULL;
	ofn.lCustData         = 0;

	if (!GetOpenFileName (&ofn)) return;

	err = KMReadPhotoFromFile (szFile, &pPhoto, &isize);

	if (IsntPGPError (PGPclErrorBox (hwnd, err))) {
		if (paps->pPhotoBuffer) 
			KMFree (paps->pPhotoBuffer);
		if (paps->hbitmapPhotoID) 
			DeleteObject (paps->hbitmapPhotoID);
		if (paps->hpalettePhotoID) 
			DeleteObject (paps->hpalettePhotoID);

		paps->pPhotoBuffer = pPhoto;
		paps->iPhotoBufferLength = isize;
		EnableWindow (GetDlgItem (hwnd, IDOK), TRUE);

		err = KMDIBfromPhoto (pPhoto, isize, TRUE, &lpbmi); 

		if (IsntPGPError (err)) {
			KMGetDIBSize (lpbmi, &(paps->iwidthPhotoID),
				 &(paps->iheightPhotoID));

			paps->hbitmapPhotoID = KMDDBfromDIB (lpbmi, 
					&(paps->hpalettePhotoID)); 

			KMFree (lpbmi);

			InvalidateRect (hwnd, NULL, TRUE);
		}
	} 
}


//	___________________________________________________
//
//  Add PhotoID Dialog Message procedure

static BOOL CALLBACK 
sAddPhotoDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{
	PADDPHOTOSTRUCT paps;
	LPBITMAPINFO	lpbmi;

	switch(uMsg) {

	case WM_INITDIALOG:
		paps = (PADDPHOTOSTRUCT)lParam;
		SetWindowLong (hDlg, GWL_USERDATA, lParam);

		// subclass photoID control to handle dropping, dragging
		paps->wpOrigPhotoIDProc = 
			(WNDPROC) SetWindowLong(GetDlgItem (hDlg, IDC_PHOTOID), 
						GWL_WNDPROC, (LONG) sPhotoIDSubclassProc); 

		return TRUE;

    case WM_HELP: 
		paps = (PADDPHOTOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, paps->pKM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aNewPhotoIds); 
        break; 
 
    case WM_CONTEXTMENU: 
 		paps = (PADDPHOTOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp ((HWND) wParam, paps->pKM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aNewPhotoIds); 
        break; 

	case WM_DESTROY :
 		paps = (PADDPHOTOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		SetWindowLong(GetDlgItem(hDlg, IDC_PHOTOID), 
							GWL_WNDPROC, (LONG)paps->wpOrigPhotoIDProc); 
		break;

	case WM_PAINT :
		paps = (PADDPHOTOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		sPaintPhotoID (hDlg, 
						paps->hbitmapPhotoID, paps->hpalettePhotoID,
						paps->iwidthPhotoID, paps->iheightPhotoID);
		break;

	case WM_PALETTECHANGED :
		if ((HWND)wParam != hDlg) {
			paps = (PADDPHOTOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			sUpdatePalette (hDlg, paps);
		}
		break;

	case WM_QUERYNEWPALETTE :
		paps = (PADDPHOTOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		return (sUpdatePalette (hDlg, paps));

	case WM_COMMAND:
		paps = (PADDPHOTOSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
		switch(LOWORD (wParam)) {
		case IDOK: 
			EndDialog (hDlg, 1);
			return TRUE;

		case IDCANCEL:
			EndDialog (hDlg, 0);
			return TRUE;

		case IDC_SELECTFILE :
			sSelectBitmapFile (hDlg, paps);
			break;

		case IDM_COPYBITMAP :
			KMCopyPhotoToClipboard (hDlg, paps->pPhotoBuffer, 
									paps->iPhotoBufferLength);
			break;

		case IDM_PASTEBITMAP :
			{
				LPBYTE		pPhoto				= NULL;
				INT			isize				= 0;
				PGPError	err					= kPGPError_UserAbort;
				INT			iLen;
				CHAR		szFile[MAX_PATH];
				HANDLE		hMem;
				HDROP		hDrop;

				if (IsClipboardFormatAvailable (CF_DIB)) {
					err = KMPastePhotoFromClipboard (hDlg, &pPhoto, &isize);
				}

				if (IsPGPError (err)) {
					if (IsClipboardFormatAvailable (CF_HDROP)) {
						if (OpenClipboard (hDlg)) { 
							hMem = GetClipboardData (CF_HDROP); 
							if (hMem) {
								hDrop = GlobalLock (hMem);
								iLen = DragQueryFile (hDrop, 0, 
												szFile, sizeof(szFile));
	
								err = KMReadPhotoFromFile (
												szFile, &pPhoto, &isize);
	
								GlobalUnlock (hMem);
							}
							CloseClipboard ();
						}
					}
				}

				if (IsntPGPError (PGPclErrorBox (hDlg, err)) &&
					IsntPGPError (err)) 
				{
					if (paps->pPhotoBuffer) 
						KMFree (paps->pPhotoBuffer);
					if (paps->hbitmapPhotoID) 
						DeleteObject (paps->hbitmapPhotoID);
					if (paps->hpalettePhotoID) 
						DeleteObject (paps->hpalettePhotoID);

					paps->pPhotoBuffer = pPhoto;
					paps->iPhotoBufferLength = isize;
					EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);

					err = KMDIBfromPhoto (pPhoto, isize, TRUE, &lpbmi); 

					if (IsntPGPError (err)) {
						KMGetDIBSize (lpbmi, &(paps->iwidthPhotoID),
							 &(paps->iheightPhotoID));

						paps->hbitmapPhotoID = KMDDBfromDIB (lpbmi, 
								&(paps->hpalettePhotoID)); 

						KMFree (lpbmi);

						InvalidateRect (hDlg, NULL, TRUE);
					}
				} 
			}
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//	___________________________________________________
//
//  Add Photo UserID to key
//	This routine is called when the user chooses to add
//	a PhotoID to an existing secret key.

BOOL 
KMAddPhotoToKey (PKEYMAN pKM) 
{
	PGPByte*		pPasskey		= NULL;
	BOOL			bRetVal			= TRUE;
	PGPError		err;
	PGPKeyRef		key;
	PGPSize			sizePasskey;
	ADDPHOTOSTRUCT	aps;
	CHAR			sz[256];
	PGPPrefRef		prefref;
	HCURSOR			hcursorOld;

	// get selected key
	key = (PGPKeyRef)KMFocusedObject (pKM);

	// check if there is a pre-existing photoid
	if (KMExistingPhotoID (pKM, key)) {
		KMMessageBox (pKM->hWndParent, IDS_CAPTION,
			IDS_PHOTOIDALREADYEXISTS, MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	// initialize struct
	aps.pKM = pKM;
	aps.pPhotoBuffer = NULL;
	aps.hbitmapPhotoID = NULL;
	aps.hpalettePhotoID = NULL;
	PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), &prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefKeyServerSyncOnAdd, 
						&(aps.bSyncWithServer));
	PGPclCloseClientPrefs (prefref, FALSE);

	// get new photoid from user
	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_NEWPHOTOID), 
					pKM->hWndParent, sAddPhotoDlgProc, (LPARAM)&aps)) {

		// get valid passphrase
		LoadString (g_hInst, IDS_SELKEYPASSPHRASE, sz, sizeof(sz)); 
		err = KMGetKeyPhrase (pKM->Context, pKM->tlsContext,
						pKM->hWndParent, sz,
						pKM->KeySetMain, key,
						NULL, &pPasskey, &sizePasskey);
		PGPclErrorBox (NULL, err);

		// now we have a valid passphrase, if required
		if (IsntPGPError (err)) {

			// update from server
			if (aps.bSyncWithServer) {
				if (!KMGetFromServerInternal (pKM, FALSE, FALSE, FALSE)) {
					if (KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
									IDS_QUERYCONTINUEADDING, 	
									MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
						bRetVal = FALSE;
					}
				}
			}
		
			if (bRetVal) {
				// make sure we have enough entropy
				PGPclRandom (pKM->Context, pKM->hWndParent, 0);

				hcursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));
				if (pPasskey) {
					err = PGPAddAttributeUserID (
										key,
										kPGPAttribute_Image,
										aps.pPhotoBuffer,
										aps.iPhotoBufferLength,
										PGPOPasskeyBuffer (pKM->Context, 
											pPasskey, sizePasskey),
										PGPOLastOption (pKM->Context));
				}
				else {
					err = PGPAddAttributeUserID (
										key,
										kPGPAttribute_Image,
										aps.pPhotoBuffer,
										aps.iPhotoBufferLength,
										PGPOLastOption (pKM->Context));
				}
				SetCursor (hcursorOld);
						
				if (IsntPGPError (PGPclErrorBox (NULL, err))) {
					KMCommitKeyRingChanges (pKM);
					KMUpdateKeyInTree (pKM, key, FALSE);
					InvalidateRect (pKM->hWndTree, NULL, TRUE);

					// send to server
					if (aps.bSyncWithServer) {
						KMSendToServer (pKM, PGPCL_DEFAULTSERVER);
					}
				}
				else bRetVal = FALSE;
			}
		}
		else bRetVal = FALSE;
	}
	else bRetVal = FALSE;

	// free passkey
	if (pPasskey) {
		KMFreePasskey (pPasskey, sizePasskey);
		pPasskey = NULL;
	}

	// free photoID objects
	if (aps.hbitmapPhotoID)
		DeleteObject (aps.hbitmapPhotoID);
	if (aps.hpalettePhotoID) 
		DeleteObject (aps.hpalettePhotoID);
	if (aps.pPhotoBuffer) 
		KMFree (aps.pPhotoBuffer);


	return bRetVal;
}
