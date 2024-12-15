/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMColumn.c - manage column selection
	

	$Id: KMColumn.c,v 1.16 1997/10/22 20:47:48 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkmx.h"
#include "resource.h"
#include "pgpClientPrefs.h"

typedef struct {
	WORD wColumnField[NUMBERFIELDS];
	WORD wFieldWidth[NUMBERFIELDS];
	LONG lSortField;
} COLUMNPREFSSTRUCT, *PCOLUMNPREFSSTRUCT;


#define DEFAULTCOLWIDTHNAME  240
#define DEFAULTCOLWIDTHVALID  50
#define DEFAULTCOLWIDTHTRUST  50
#define DEFAULTCOLWIDTHSIZE   70
#define DEFAULTCOLWIDTHDESC	 200
#define MINWIDTH			  10
#define MAXWIDTH			1000


// external global variables
extern HINSTANCE g_hInst;

#define KMI_NAME			0
#define KMI_VALIDITY		1
#define KMI_SIZE			2
#define KMI_DESCRIPTION		3
#define KMI_KEYID			4
#define KMI_TRUST			5
#define KMI_CREATION		6
#define KMI_EXPIRATION		7
#define KMI_ADK				8
#define NUMBERFIELDS		9

static DWORD aColumnStringIds[] = {	
    KMI_NAME,			IDS_NAMEFIELD, 
    KMI_VALIDITY,		IDS_VALIDITYFIELD, 
    KMI_SIZE,			IDS_SIZEFIELD, 
    KMI_DESCRIPTION,	IDS_DESCRIPTIONFIELD, 
    KMI_KEYID,			IDS_KEYIDFIELD, 
    KMI_TRUST,			IDS_TRUSTFIELD, 
    KMI_CREATION,		IDS_CREATIONFIELD, 
    KMI_EXPIRATION,		IDS_EXPIRATIONFIELD, 
    KMI_ADK,			IDS_ADKFIELD, 
    0,0 
}; 

static DWORD aColumnHelpIds[] = {	
	IDC_AVAILABLELIST,	IDH_PGPKM_COLAVAILABLE,
	IDC_SELECTEDLIST,	IDH_PGPKM_COLSHOW,
	IDC_ADDFIELD,		IDH_PGPKM_COLADDFIELD,
	IDC_REMOVEFIELD,	IDH_PGPKM_COLREMOVEFIELD,
	IDC_MOVEUP,			IDH_PGPKM_COLMOVEUP,
	IDC_MOVEDOWN,		IDH_PGPKM_COLMOVEDOWN,
  0,0 
}; 

//	________________________________________
//
//	Get sort order information from registry

BOOL 
KMGetSortOrderFromRegistry (LONG* lField) 
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dwValueType, dwValue, dwSize=4;
	CHAR	szKey[512];

	*lField = kPGPUserIDOrdering;

	LoadString (g_hInst, IDS_REGISTRYKEY, szKey, sizeof(szKey));
	lResult = RegOpenKeyEx (HKEY_CURRENT_USER, szKey, 0, KEY_READ, &hKey);
	if (lResult == ERROR_SUCCESS) {
		lResult = RegQueryValueEx (hKey, "SortField", 0, &dwValueType, 
			(LPBYTE)&dwValue, &dwSize);
		if (lResult == ERROR_SUCCESS) *lField = dwValue;
		RegCloseKey (hKey);
		return TRUE;
	}
	return FALSE;
}

//	___________________________________________
//
//	Get widths of control columns from registry

VOID 
KMGetColumnPreferences (PKEYMAN pKM)
{
	PGPError			err;
	PGPPrefRef			prefref;
	PGPSize				size;
	PCOLUMNPREFSSTRUCT	pcps;
	INT					i;
	WORD				wField, wWidth;

#if PGP_BUSINESS_SECURITY
	pKM->wColumnField[0]				= KMI_NAME;
	pKM->wColumnField[1]				= KMI_VALIDITY;
	pKM->wColumnField[2]				= KMI_SIZE;
	pKM->wColumnField[3]				= KMI_DESCRIPTION;
	pKM->wColumnField[4]				= 0;
	pKM->wColumnField[5]				= 0;
	pKM->wColumnField[6]				= 0;
	pKM->wColumnField[7]				= 0;
	pKM->wColumnField[8]				= 0;

	pKM->wFieldWidth[KMI_NAME]			= DEFAULTCOLWIDTHNAME;
	pKM->wFieldWidth[KMI_VALIDITY]		= DEFAULTCOLWIDTHVALID;
	pKM->wFieldWidth[KMI_SIZE]			= DEFAULTCOLWIDTHSIZE;
	pKM->wFieldWidth[KMI_DESCRIPTION]	= DEFAULTCOLWIDTHDESC;
	pKM->wFieldWidth[KMI_KEYID]			= 0;
	pKM->wFieldWidth[KMI_TRUST]			= 0;
	pKM->wFieldWidth[KMI_CREATION]		= 0;
	pKM->wFieldWidth[KMI_EXPIRATION]	= 0;
	pKM->wFieldWidth[KMI_ADK]			= 0;
#else
	pKM->wColumnField[0]				= KMI_NAME;
	pKM->wColumnField[1]				= KMI_VALIDITY;
	pKM->wColumnField[2]				= KMI_TRUST;
	pKM->wColumnField[3]				= KMI_SIZE;
	pKM->wColumnField[4]				= KMI_DESCRIPTION;
	pKM->wColumnField[5]				= 0;
	pKM->wColumnField[6]				= 0;
	pKM->wColumnField[7]				= 0;
	pKM->wColumnField[8]				= 0;

	pKM->wFieldWidth[KMI_NAME]			= DEFAULTCOLWIDTHNAME;
	pKM->wFieldWidth[KMI_VALIDITY]		= DEFAULTCOLWIDTHVALID;
	pKM->wFieldWidth[KMI_SIZE]			= DEFAULTCOLWIDTHSIZE;
	pKM->wFieldWidth[KMI_DESCRIPTION]	= DEFAULTCOLWIDTHDESC;
	pKM->wFieldWidth[KMI_KEYID]			= 0;
	pKM->wFieldWidth[KMI_TRUST]			= DEFAULTCOLWIDTHTRUST;
	pKM->wFieldWidth[KMI_CREATION]		= 0;
	pKM->wFieldWidth[KMI_EXPIRATION]	= 0;
	pKM->wFieldWidth[KMI_ADK]			= 0;
#endif

	pKM->lKeyListSortField				= kPGPUserIDOrdering;

	err = PGPcomdlgOpenClientPrefs (&prefref);
	if (IsntPGPError (err)) {
		err = PGPGetPrefData (prefref, kPGPPrefPGPkeysWinColumnData,
							  &size, &pcps);

		if (IsntPGPError (err)) {
			if (size == sizeof(COLUMNPREFSSTRUCT)) {
				for (i=0; i<NUMBERFIELDS; i++) {
					wField = pcps->wColumnField[i];
					if ((wField >= 0) && (wField < NUMBERFIELDS))
						pKM->wColumnField[i] = wField;
					wWidth = pcps->wFieldWidth[i];
					if ((wWidth >= MINWIDTH) && (wWidth <= MAXWIDTH))
						pKM->wFieldWidth[i] = wWidth;
				}
				pKM->lKeyListSortField = pcps->lSortField;
			}
			PGPDisposePrefData (prefref, pcps);
		}
		PGPcomdlgCloseClientPrefs (prefref, FALSE);
	}
}

//	___________________________________________
// 
//	Put window position information in registry

VOID 
KMSetColumnPreferences (PKEYMAN pKM) 
{
	PGPError			err;
	PGPPrefRef			prefref;
	COLUMNPREFSSTRUCT	cps;
	INT					i, iField;


	err = PGPcomdlgOpenClientPrefs (&prefref);
	if (IsntPGPError (err)) {
		for (i=0; i<NUMBERFIELDS; i++) {
			cps.wColumnField[i] = pKM->wColumnField[i];

			iField = pKM->wColumnField[i];
			if ((i == 0) || (iField != 0)) pKM->wFieldWidth[iField] = 
					LOWORD (TreeList_GetColumnWidth (pKM->hWndTree, i));
			cps.wFieldWidth[i] = pKM->wFieldWidth[i];
		}

		cps.lSortField = pKM->lKeyListSortField;

		PGPSetPrefData (prefref, kPGPPrefPGPkeysWinColumnData,
							  sizeof(cps), &cps);

		PGPcomdlgCloseClientPrefs (prefref, TRUE);
	}
}

//	__________________________________
//
//  Initialize column/field list boxes 

VOID 
SetLists (
		HWND	hDlg, 
		PKEYMAN pKM) 
{
	INT		iCol, iField, iDisp, ids, idx;
	CHAR	sz[64];

	SendDlgItemMessage (hDlg, IDC_AVAILABLELIST, LB_RESETCONTENT, 0, 0);
	SendDlgItemMessage (hDlg, IDC_SELECTEDLIST, LB_RESETCONTENT, 0, 0);

	for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
		iField = pKM->wColumnField[iCol];
		if (iField) {
			ids = aColumnStringIds[(iField*2)+1];
			LoadString (g_hInst, ids, sz, sizeof(sz));
			idx = SendDlgItemMessage (hDlg, IDC_SELECTEDLIST, 
								LB_ADDSTRING, 0, (LPARAM)sz);
			SendDlgItemMessage (hDlg, IDC_SELECTEDLIST,
								LB_SETITEMDATA, idx, iField);
		}
	}

	for (iField=1; iField<NUMBERFIELDS; iField++) {
		iDisp = iField;
		for (iCol=0; iCol<NUMBERFIELDS; iCol++) {
			if (pKM->wColumnField[iCol] == iField) iDisp = 0;
		}
		if (iDisp != 0) {
			ids = aColumnStringIds[(iDisp*2)+1];
			LoadString (g_hInst, ids, sz, sizeof(sz));
			idx = SendDlgItemMessage (hDlg, IDC_AVAILABLELIST, 
								LB_ADDSTRING, 0, (LPARAM)sz);
			SendDlgItemMessage (hDlg, IDC_AVAILABLELIST,
								LB_SETITEMDATA, idx, iField);
		}
	}
}

//	__________________________________
//
//  Initialize column/field list boxes 

VOID 
SetMoveUpDownButtons (
		HWND hDlg,
		PKEYMAN	pkm)
{
	BOOL	bUp		= FALSE;
	BOOL	bDown	= FALSE;
	INT		idx;

	if (pkm->wColumnField[1] > 0) 
		EnableWindow (GetDlgItem (hDlg, IDC_REMOVEFIELD), TRUE);
	else 
		EnableWindow (GetDlgItem (hDlg, IDC_REMOVEFIELD), FALSE);

	if (pkm->wColumnField[2] > 0) {	
		idx = SendDlgItemMessage (hDlg, IDC_SELECTEDLIST, LB_GETCURSEL, 0, 0);
		if (idx >= 0) {
			if (idx > 0) bUp = TRUE;
			if (pkm->wColumnField[idx+2]) bDown = TRUE;
		}
	}

	if (!bUp || !bDown) SetFocus (GetDlgItem (hDlg, IDC_SELECTEDLIST));

	EnableWindow (GetDlgItem (hDlg, IDC_MOVEUP), bUp);
	EnableWindow (GetDlgItem (hDlg, IDC_MOVEDOWN), bDown);	
}

//	___________________________________________________
//
//  Select columns to display dialog message procedure

BOOL CALLBACK 
SelectColumnsDlgProc (
		HWND hDlg, 
		UINT uMsg, 
		WPARAM wParam, 
		LPARAM lParam) 
{
	PKEYMAN pkm;
	INT		idx, iField, iCol;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pkm = (PKEYMAN)lParam;
		SetLists (hDlg, pkm);
		EnableWindow (GetDlgItem (hDlg, IDC_ADDFIELD), FALSE);
		return TRUE;

	case WM_HELP: 
		pkm = (PKEYMAN)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, 
			pkm->szHelpFile, HELP_WM_HELP, (DWORD) (LPSTR) aColumnHelpIds); 
		break; 
 
	case WM_CONTEXTMENU: 
		pkm = (PKEYMAN)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp ((HWND) wParam, pkm->szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aColumnHelpIds); 
		break; 

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK :
			EndDialog (hDlg, 1);
			break;

		case IDCANCEL :
			EndDialog (hDlg, 0);
			break;

		case IDHELP :
			pkm = (PKEYMAN)GetWindowLong (hDlg, GWL_USERDATA);
			WinHelp (hDlg, pkm->szHelpFile, HELP_CONTEXT, 
						IDH_PGPKM_COLUMNDIALOG); 
			break;

		case IDC_ADDFIELD :
			pkm = (PKEYMAN)GetWindowLong (hDlg, GWL_USERDATA);
			idx = SendDlgItemMessage (hDlg, IDC_AVAILABLELIST,
								LB_GETCURSEL, 0, 0);
			iField = SendDlgItemMessage (hDlg, IDC_AVAILABLELIST,
								LB_GETITEMDATA, idx, 0);

			if (iField != LB_ERR) {
				for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
					if (pkm->wColumnField[iCol] == 0) {
						pkm->wColumnField[iCol] = iField;
						if ((pkm->wFieldWidth[iField] > 1000) ||
							(pkm->wFieldWidth[iField] <= 0)) 
							pkm->wFieldWidth[iField] = 60;
						break;
					}
				}
				SetLists (hDlg, pkm);
			}
			EnableWindow (GetDlgItem (hDlg, IDC_ADDFIELD), FALSE);
			break;

		case IDC_REMOVEFIELD :
			pkm = (PKEYMAN)GetWindowLong (hDlg, GWL_USERDATA);
			idx = SendDlgItemMessage (hDlg, IDC_SELECTEDLIST,
								LB_GETCURSEL, 0, 0);
			iField = SendDlgItemMessage (hDlg, IDC_SELECTEDLIST,
								LB_GETITEMDATA, idx, 0);

			for (iCol=1; iCol<NUMBERFIELDS; iCol++) {
				if (pkm->wColumnField[iCol] == iField) {
					for (idx=iCol+1; idx<NUMBERFIELDS; idx++) {
						pkm->wColumnField[idx-1] = pkm->wColumnField[idx];
					}
					pkm->wColumnField[NUMBERFIELDS-1] = 0;
					break;
				}
			}
			SetLists (hDlg, pkm);
			SetMoveUpDownButtons (hDlg, pkm);
			EnableWindow (GetDlgItem (hDlg, IDC_REMOVEFIELD), FALSE);
			break;

		case IDC_MOVEUP :
			pkm = (PKEYMAN)GetWindowLong (hDlg, GWL_USERDATA);
			idx = SendDlgItemMessage (hDlg, IDC_SELECTEDLIST,
								LB_GETCURSEL, 0, 0);
			iField = SendDlgItemMessage (hDlg, IDC_SELECTEDLIST,
								LB_GETITEMDATA, idx, 0);
			for (iCol=2; iCol<NUMBERFIELDS; iCol++) {
				if (iField == pkm->wColumnField[iCol]) {
					pkm->wColumnField[iCol] = pkm->wColumnField[iCol-1];
					pkm->wColumnField[iCol-1] = iField;
					idx--;
					break;
				}
			}
			SetLists (hDlg, pkm);
			SendDlgItemMessage (hDlg, IDC_SELECTEDLIST, LB_SETCURSEL, 
								idx, 0);
			SetMoveUpDownButtons (hDlg, pkm);
			break;

		case IDC_MOVEDOWN :
			pkm = (PKEYMAN)GetWindowLong (hDlg, GWL_USERDATA);
			idx = SendDlgItemMessage (hDlg, IDC_SELECTEDLIST,
								LB_GETCURSEL, 0, 0);
			iField = SendDlgItemMessage (hDlg, IDC_SELECTEDLIST,
								LB_GETITEMDATA, idx, 0);
			for (iCol=1; iCol<NUMBERFIELDS-1; iCol++) {
				if (iField == pkm->wColumnField[iCol]) {
					if (pkm->wColumnField[iCol+1] != 0) {
						pkm->wColumnField[iCol] = pkm->wColumnField[iCol+1];
						pkm->wColumnField[iCol+1] = iField;
						idx++;
					}
					break;
				}
			}
			SetLists (hDlg, pkm);
			SendDlgItemMessage (hDlg, IDC_SELECTEDLIST, LB_SETCURSEL, 
								idx, 0);
			SetMoveUpDownButtons (hDlg, pkm);
			break;

		case IDC_AVAILABLELIST :
			switch (HIWORD (wParam)) {
			case LBN_SETFOCUS :
				SendDlgItemMessage (hDlg, IDC_SELECTEDLIST, 
									LB_SETCURSEL, (WPARAM)-1, 0);
				EnableWindow (GetDlgItem (hDlg, IDC_MOVEUP), FALSE);
				EnableWindow (GetDlgItem (hDlg, IDC_MOVEDOWN), FALSE);
				EnableWindow (GetDlgItem (hDlg, IDC_REMOVEFIELD), FALSE);
				break;

			case LBN_SELCHANGE :
				EnableWindow (GetDlgItem (hDlg, IDC_ADDFIELD), TRUE);
				break;

			case LBN_DBLCLK :
				SendMessage (hDlg, WM_COMMAND, 
									MAKEWPARAM (IDC_ADDFIELD, 0), 0);
				break;
			}
			break;

		case IDC_SELECTEDLIST :
			pkm = (PKEYMAN)GetWindowLong (hDlg, GWL_USERDATA);
			switch (HIWORD (wParam)) {
			case LBN_SETFOCUS :
				SendDlgItemMessage (hDlg, IDC_AVAILABLELIST, 
									LB_SETCURSEL, (WPARAM)-1, 0);
				EnableWindow (GetDlgItem (hDlg, IDC_ADDFIELD), FALSE);
				break;

			case LBN_SELCHANGE :
				SetMoveUpDownButtons (hDlg, pkm);
				break;

			case LBN_DBLCLK :
				SendMessage (hDlg, WM_COMMAND, 
									MAKEWPARAM (IDC_REMOVEFIELD, 0), 0);
				break;
			}
			break;


		default :
			break;
		}
		break;

	}
	return FALSE;
}

//	_________________________
//
//	Select columns to display

VOID 
KMSelectColumns (PKEYMAN pKM, BOOL bDialog) 
{
	INT		i;
	WORD	wColumnField[NUMBERFIELDS];
	WORD	wFieldWidth[NUMBERFIELDS];


	if (bDialog) {
		// save current column settings
		for (i=0; i<NUMBERFIELDS; i++) {
			wColumnField[i] = pKM->wColumnField[i];
			wFieldWidth[i] = pKM->wFieldWidth[i];
		}

		if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_SELECTCOLUMNS),
			pKM->hWndParent, SelectColumnsDlgProc, (LPARAM)pKM)) {

			// user clicked "OK" => reinitialize treelist
			KMDeleteAllKeyProperties (pKM, FALSE);
			KMDeleteAllUserValues (pKM);
			KMSetFocus (pKM, NULL, FALSE);
			TreeList_DeleteTree (pKM->hWndTree, TRUE);
			TreeList_DeleteAllColumns (pKM->hWndTree);

			KMAddColumns (pKM);
			KMLoadKeyRingIntoTree (pKM, FALSE, FALSE, TRUE);
			InvalidateRect (pKM->hWndTree, NULL, TRUE);
			UpdateWindow (pKM->hWndTree);

		}

		// user cancelled => restore column settings
		else {
			for (i=0; i<NUMBERFIELDS; i++) {
				pKM->wColumnField[i] = wColumnField[i];
				pKM->wFieldWidth[i] = wFieldWidth[i];
			}
		}
	}

	KMSetColumnPreferences (pKM);
}
