/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	GMNewGrp.c - create new recipient group
	

	$Id: GMNewGrp.c,v 1.14 1998/08/11 15:20:03 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpgmx.h"

// typedefs 
typedef struct {
	PGROUPMAN		pGM;
	PGPGroupSetRef	groupset;
	PGPGroupID		groupid;
	BOOL			bNewGroup;
	CHAR			szGroupName[kPGPMaxGroupNameLength];
	CHAR			szGroupDesc[kPGPMaxGroupDescriptionLength];
} NEWGROUPSTRUCT;

// external globals
extern HINSTANCE g_hInst;

// local globals
static DWORD aIds[] = {			// Help IDs
	IDC_NEWGROUPNAME,	IDH_PGPPKGRP_GROUPNAME, 
	IDC_NEWGROUPDESC,	IDH_PGPPKGRP_GROUPDESC,
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
sNewGroupDlgProc (
		HWND	hDlg, 
		UINT	uMsg, 
		WPARAM	wParam,
		LPARAM	lParam) 
{

	NEWGROUPSTRUCT* pngs;
	PGPError		err;

	switch(uMsg) {

	case WM_INITDIALOG:
		pngs = (NEWGROUPSTRUCT*)lParam;
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		SendDlgItemMessage (hDlg, IDC_NEWGROUPNAME, EM_SETLIMITTEXT,
								kPGPMaxGroupNameLength, 0);
		SendDlgItemMessage (hDlg, IDC_NEWGROUPDESC, EM_SETLIMITTEXT,
								kPGPMaxGroupDescriptionLength, 0);
		if (!pngs->bNewGroup) {
			CHAR szTitle[64];
			LoadString (g_hInst, IDS_GROUPPROPS, szTitle, sizeof(szTitle));
			SetWindowText (hDlg, szTitle);
			SetDlgItemText (hDlg, IDC_NEWGROUPNAME, pngs->szGroupName);
			SetDlgItemText (hDlg, IDC_NEWGROUPDESC, pngs->szGroupDesc);
		}
		return TRUE;

    case WM_HELP: 
		pngs = (NEWGROUPSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pngs->pGM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
        break; 
 
    case WM_CONTEXTMENU: 
 		pngs = (NEWGROUPSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
		WinHelp ((HWND) wParam, pngs->pGM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aIds); 
        break; 

	case WM_COMMAND:
		switch(LOWORD (wParam)) {
		case IDOK: 
			pngs = (NEWGROUPSTRUCT*)GetWindowLong (hDlg, GWL_USERDATA);
			if (!GetDlgItemText (hDlg, IDC_NEWGROUPNAME, pngs->szGroupName,
				sizeof (pngs->szGroupName))) {
				PGPgmMessageBox (hDlg, IDS_CAPTION, IDS_NEEDGROUPNAME,
					MB_OK|MB_ICONEXCLAMATION);
				return TRUE;
			}

			GetDlgItemText (hDlg, IDC_NEWGROUPDESC, pngs->szGroupDesc,
							sizeof (pngs->szGroupDesc));

			if (pngs->bNewGroup) {
				err = PGPNewGroup (pngs->pGM->groupsetMain, 
					pngs->szGroupName, pngs->szGroupDesc, &(pngs->groupid));
			}
			else {
				err = PGPSetGroupName (pngs->pGM->groupsetMain, 
										pngs->groupid, pngs->szGroupName);
				if (IsntPGPError (err)) {
					err = PGPSetGroupDescription (pngs->pGM->groupsetMain, 
								pngs->groupid, pngs->szGroupDesc);
				}
			}

			if (IsPGPError (err)) {
				PGPclErrorBox (NULL, err);
				EndDialog (hDlg, 0);
				return TRUE;
			}

			EndDialog (hDlg, 1);
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
//  Update recipient group
//	This routine is called when the user chooses to edit
//	recipient group properties

BOOL 
GMGroupProperties (PGROUPMAN pGM)
{
	BOOL			bRetVal			= FALSE;
	PGPGroupItem	groupitem;
	PGPGroupInfo	groupinfo;
	PGPUInt32		iIndex;
	TL_TREEITEM		tli;
	NEWGROUPSTRUCT	ngs;

	// initialize struct
	ngs.pGM = pGM;
	ngs.bNewGroup = FALSE;

	tli.hItem = GMFocusedItem (pGM);
	tli.mask = TLIF_IMAGE|TLIF_PARAM;
	TreeList_GetItem (pGM->hWndTree, &tli);

	// if selected item is a key, get its parent
	if (tli.iImage != IDX_GROUP) {
		tli.hItem = GMFocusedItem (pGM);
		tli.mask = TLIF_PARENTHANDLE;
		TreeList_GetItem (pGM->hWndTree, &tli);

		tli.mask = TLIF_IMAGE|TLIF_PARAM;
		TreeList_GetItem (pGM->hWndTree, &tli);
	}

	// if selected item is a group then get the groupid
	if (tli.iImage == IDX_GROUP) {
		ngs.groupid = HIWORD(tli.lParam);
		if (ngs.groupid) {
			iIndex = LOWORD(tli.lParam);
			PGPGetIndGroupItem (pGM->groupsetMain, ngs.groupid, iIndex,
									&groupitem);
			ngs.groupid = groupitem.u.group.id; 
		}
		else ngs.groupid = LOWORD(tli.lParam);

		PGPGetGroupInfo (pGM->groupsetMain, ngs.groupid, &groupinfo);
		lstrcpy (ngs.szGroupName, groupinfo.name);
		lstrcpy (ngs.szGroupDesc, groupinfo.description);

		// get new info from user
		if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_NEWGROUP), 
					pGM->hWndParent, sNewGroupDlgProc, (LPARAM)&ngs)) {
			GMSortGroupSet (pGM);
			GMCommitGroupChanges (pGM, TRUE);
			PGPgmLoadGroups ((HGROUPMAN)pGM);
			InvalidateRect (pGM->hWndTree, NULL, FALSE);
			UpdateWindow (pGM->hWndTree);
			bRetVal = TRUE;			
		}
	}

	return bRetVal;
}

//	___________________________________________________
//
//  Create new recipient group
//	This routine is called when the user chooses to create
//	a new recipient group

PGPError PGPgmExport
PGPgmNewGroup (HGROUPMAN hGroupMan) 
{
	PGROUPMAN		pGM				= (PGROUPMAN)hGroupMan;
	BOOL			bRetVal			= FALSE;
	NEWGROUPSTRUCT	ngs;

	// initialize struct
	ngs.pGM = pGM;
	ngs.bNewGroup = TRUE;

	// get new group info from user
	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_NEWGROUP), 
				pGM->hWndParent, sNewGroupDlgProc, (LPARAM)&ngs)) {
		GMCommitGroupChanges (pGM, TRUE);
//
			TreeList_DeleteTree (pGM->hWndTree, FALSE);
			InvalidateRect (pGM->hWndTree, NULL, TRUE);
			UpdateWindow (pGM->hWndTree);
			GMSortGroupSet (pGM);
			GMLoadGroupsIntoTree (pGM, TRUE, FALSE, FALSE);
			InvalidateRect (pGM->hWndTree, NULL, TRUE);
//
//		GMLoadGroupsIntoTree (pGM, FALSE, FALSE, FALSE); 
//		InvalidateRect (pGM->hWndTree, NULL, FALSE);
		UpdateWindow (pGM->hWndTree);
		bRetVal = TRUE;			
	}

	return bRetVal;
}

/*
			TreeList_DeleteTree (pGM->hWndTree, FALSE);
			InvalidateRect (pGM->hWndTree, NULL, TRUE);
			UpdateWindow (pGM->hWndTree);
			GMSortGroupSet (pGM);
			GMLoadGroupsIntoTree (pGM, TRUE, FALSE, FALSE);
			InvalidateRect (pGM->hWndTree, NULL, TRUE);
*/