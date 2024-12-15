/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	GMTree.h - create and fill group manager control
	

	$Id: GMOps.c,v 1.17 1997/10/21 13:36:30 pbj Exp $
____________________________________________________________________________*/

#include "pgpgmx.h"
#include <process.h>
#include <commdlg.h>

#include "resource.h"

#include "pgpFileSpec.h"
#include "pgpGroupsUtil.h"

#define BITMAP_WIDTH 16
#define BITMAP_HEIGHT 16

#define VALIDITYTHRESHOLD 2
#define AXIOMATICTHRESHOLD 3

typedef struct {
	FARPROC lpfnCallback;
	PGPContextRef Context;
	PGROUPMAN pGM;
	LPSTR pszPrompt;
	BOOL bItemModified;
	BOOL bItemNotDeleted;
	BOOL bDeleteAll;
	HTLITEM hPostDeleteFocusItem;
} DELETESTRUCT;

typedef struct {
	FARPROC lpfnCallback;
	PGROUPMAN pGM;
	BOOL bFirst;
} LOCATESTRUCT;

typedef struct {
	FARPROC lpfnCallback;
	PGROUPMAN pGM;
	PGPGroupID groupidDest;
	BOOL bAdded;
} ADDSTRUCT;

static DWORD aDeleteAllIds[] = {			// Help IDs
    0,0 
}; 

// external globals
extern HINSTANCE g_hInst;


//----------------------------------------------------|
// Delete All dialog message procedure

BOOL CALLBACK 
DeleteAllDlgProc (HWND hWndDlg, 
				  UINT uMsg, 
				  WPARAM wParam, 
				  LPARAM lParam) 
{
	DELETESTRUCT* pds;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hWndDlg, GWL_USERDATA, lParam);
		pds = (DELETESTRUCT*)lParam;
		SetDlgItemText (hWndDlg, IDC_STRING, pds->pszPrompt);
		break;

    case WM_HELP: 
		pds = (DELETESTRUCT*)GetWindowLong (hWndDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pds->pGM->szHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aDeleteAllIds); 
        break; 
 
    case WM_CONTEXTMENU:
		pds = (DELETESTRUCT*)GetWindowLong (hWndDlg, GWL_USERDATA);
        WinHelp ((HWND) wParam, pds->pGM->szHelpFile, HELP_CONTEXTMENU, 
            (DWORD) (LPVOID) aDeleteAllIds); 
        break; 

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL:
			pds = (DELETESTRUCT*)GetWindowLong (hWndDlg, GWL_USERDATA);
			pds->bItemNotDeleted = TRUE;
			EndDialog (hWndDlg, IDCANCEL);
			break;

		case IDOK:
		case IDYES:
			EndDialog (hWndDlg, IDYES);
			break;

		case IDNO:
			EndDialog (hWndDlg, IDNO);
			break;

		case IDC_YESTOALL :
			pds = (DELETESTRUCT*)GetWindowLong (hWndDlg, GWL_USERDATA);
			pds->bDeleteAll = TRUE;
			EndDialog (hWndDlg, IDYES);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------|
//  Ask user for delete confirmation

INT 
DeleteConfirm (TL_TREEITEM* lptli, 
			   INT iPromptID, 
			   DELETESTRUCT* pds) 
{
	CHAR sz256[256];
	CHAR sz512[512];
	INT iRetVal;

	if (pds->bDeleteAll) return IDYES;

	LoadString (g_hInst, iPromptID, sz256, 256); 
	wsprintf (sz512, sz256, lptli->pszText);

	if (GMMultipleSelected (pds->pGM)) {
		pds->pszPrompt = sz512;
		iRetVal = DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_DELETEALL), 
			pds->pGM->hWndParent, DeleteAllDlgProc, (LPARAM)pds);
		if (!pds->bItemNotDeleted && (iRetVal == IDNO)) {
			pds->bItemNotDeleted = TRUE;
			pds->hPostDeleteFocusItem = lptli->hItem;
		}
	}
	else {
		LoadString (g_hInst, IDS_DELCONFCAPTION, sz256, 256);
		iRetVal = MessageBox (pds->pGM->hWndParent, sz512, sz256,
				MB_YESNO|MB_TASKMODAL|MB_DEFBUTTON2|MB_ICONWARNING);
	}

	return iRetVal;
}


//----------------------------------------------------|
//  Get handle of nearby item
//
//	lptli	= pointer to TreeList item

HTLITEM 
GetAdjacentItem (HWND hWndTree, 
				 TL_TREEITEM* lptli) 
{
	TL_TREEITEM tli;

	tli.hItem = lptli->hItem;
	tli.mask = TLIF_NEXTHANDLE;
	TreeList_GetItem (hWndTree, &tli);
	if (!tli.hItem) {
		tli.hItem = lptli->hItem;
		tli.mask = TLIF_PREVHANDLE;
		TreeList_GetItem (hWndTree, &tli);
		if (!tli.hItem) {
			tli.hItem = lptli->hItem;
			tli.mask = TLIF_PARENTHANDLE;
			TreeList_GetItem (hWndTree, &tli);
		}
	}

	return tli.hItem;
}

	
//----------------------------------------------------|
//  Delete a single object
//	routine called as a callback function from the TreeList control to 
//	delete a single item.
//
//	lptli	= pointer to TreeList item to delete

BOOL CALLBACK 
DeleteSingleObject (TL_TREEITEM* lptli, 
					LPARAM lParam) 
{
	DELETESTRUCT*	pds			= (DELETESTRUCT*)lParam;
	INT				iConfirm;
	PGPUInt32		index;
	PGPGroupID		id;

	switch (lptli->iImage) {

	// is it a group ?
	case IDX_GROUP :
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFGROUP, pds);
		if (iConfirm == IDYES) {
			id = HIWORD(lptli->lParam);
			index = LOWORD(lptli->lParam);
			// if hiword is non-zero, then it's a copy of a group
			if (id) {
				PGPDeleteIndItemFromGroup (pds->pGM->groupsetMain, 
						id, index);
			}

			// if hiword is zero, then it's a root group
			else {
				id = LOWORD(lptli->lParam);
				PGPDeleteGroup (pds->pGM->groupsetMain, id);
			}

			pds->bItemModified = TRUE;
			if (!pds->bItemNotDeleted) 
				pds->hPostDeleteFocusItem = 
						GetAdjacentItem (pds->pGM->hWndTree, lptli); 
			TreeList_DeleteItem (pds->pGM->hWndTree, lptli);
		}
		if (iConfirm == IDCANCEL) return FALSE; 
		else return TRUE;

	// otherwise it's a key 
	default:
		iConfirm = DeleteConfirm (lptli, IDS_DELCONFKEY, pds);
		if (iConfirm == IDYES) {
			id = HIWORD(lptli->lParam);
			index = LOWORD(lptli->lParam);
			if (id) {
				PGPDeleteIndItemFromGroup (pds->pGM->groupsetMain, id, index);
				pds->bItemModified = TRUE;
				if (!pds->bItemNotDeleted) 
					pds->hPostDeleteFocusItem = 
							GetAdjacentItem (pds->pGM->hWndTree, lptli); 
				TreeList_DeleteItem (pds->pGM->hWndTree, lptli);
			}
		}
		if (iConfirm == IDCANCEL) return FALSE; 
		else return TRUE;
	}
}

//	___________________________________________________
//
//	delete group or keys

BOOL 
GMDeleteObject (PGROUPMAN pGM)
{
	TL_TREEITEM		tli;
	DELETESTRUCT	ds;

	ds.lpfnCallback = DeleteSingleObject;
	ds.Context = pGM->context;
	ds.pGM = pGM;
	ds.bItemModified = FALSE;
	ds.bDeleteAll = FALSE;
	ds.bItemNotDeleted = FALSE;
	ds.hPostDeleteFocusItem = NULL;

	TreeList_IterateSelected (pGM->hWndTree, &ds);

	if (ds.bItemModified) {
		GMCommitGroupChanges (pGM, TRUE);
		GMLoadGroupsIntoTree (pGM, FALSE, FALSE, FALSE);
		if (ds.bItemNotDeleted) {
			if (ds.hPostDeleteFocusItem) {
				tli.hItem = ds.hPostDeleteFocusItem;
				TreeList_Select (pGM->hWndTree, &tli, FALSE);
			}
			else GMSetFocus (pGM, NULL, FALSE);
		}
		else {
			if (ds.hPostDeleteFocusItem) {
				tli.hItem = ds.hPostDeleteFocusItem;
				TreeList_Select (pGM->hWndTree, &tli, TRUE);
				tli.stateMask = TLIS_SELECTED;
				tli.state = 0;
				tli.mask = TLIF_STATE;
				TreeList_SetItem (pGM->hWndTree, &tli);
			}
			else GMSetFocus (pGM, NULL, FALSE);
		}
		InvalidateRect (pGM->hWndTree, NULL, TRUE);
	}
	return (ds.bItemModified);
}

//----------------------------------------------------|
//  Add a single object
//	routine called as a callback function from the TreeList control to 
//	add a single item.
//
//	lptli	= pointer to TreeList item to delete

BOOL CALLBACK 
AddSingleObject (TL_TREEITEM* lptli, 
					LPARAM lParam) 
{
	ADDSTRUCT*		pas			= (ADDSTRUCT*)lParam;
	PGPUInt32		index;
	PGPGroupID		id;
	PGPGroupItem	groupitem;
	PGPError		err;

	switch (lptli->iImage) {

	// is it a group ?
	case IDX_GROUP :
		id = HIWORD(lptli->lParam);
		if (!id) id = LOWORD(lptli->lParam);
		if (id) {
			if (id != pas->groupidDest) {
				groupitem.type = kPGPGroupItem_Group;
				groupitem.userValue = 0;
				groupitem.u.group.id = id;
				err = PGPAddItemToGroup (pas->pGM->groupsetMain, 
								&groupitem, pas->groupidDest);
				if (IsntPGPError (err)) pas->bAdded = TRUE;
			}
		}
		return TRUE;

	// otherwise it's a key 
	default:
		id = HIWORD(lptli->lParam);
		index = LOWORD(lptli->lParam);
		if (id) {
			PGPGetIndGroupItem (pas->pGM->groupsetMain, 
											id, index, &groupitem);
			groupitem.userValue = 0;
			err = PGPAddItemToGroup (pas->pGM->groupsetMain, &groupitem, 
													pas->groupidDest);
			if (IsntPGPError (err)) pas->bAdded = TRUE;
		}
		return TRUE;
	}
}

//	_______________________________________________
//
//  add selected items to focused group

BOOL 
GMAddSelectedToFocused (PGROUPMAN pGM) 
{
	PGPGroupID		groupidDest;
	TL_TREEITEM		tli;
	ADDSTRUCT		as;

	tli.hItem = GMFocusedItem (pGM);
	tli.mask = TLIF_IMAGE|TLIF_PARAM;
	TreeList_GetItem (pGM->hWndTree, &tli);

	// if selected item is a group then get the groupid
	if (tli.iImage == IDX_GROUP) {
		groupidDest = HIWORD(tli.lParam);
		if (!groupidDest) groupidDest = LOWORD(tli.lParam);
	}

	// otherwise it's a key, get the parent groupid
	else {
		groupidDest = HIWORD(tli.lParam);
	}

	if (groupidDest) {
		as.lpfnCallback = AddSingleObject;
		as.pGM = pGM;
		as.groupidDest = groupidDest;
		as.bAdded = FALSE;

		TreeList_IterateSelected (pGM->hWndTree, &as);
	}
	
	if (as.bAdded) {
		GMSortGroupSet (pGM);
		GMCommitGroupChanges (pGM, TRUE);
		GMLoadGroupsIntoTree (pGM, FALSE, TRUE, FALSE);
		InvalidateRect (pGM->hWndTree, NULL, TRUE);
		UpdateWindow (pGM->hWndTree);
	}

	return TRUE;
}

//	_______________________________________________
//
//  add keys to selected group

BOOL 
GMAddKeysToGroup (
		PGROUPMAN		pGM, 
		PGPKeySetRef	keyset) 
{
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPKeyID		keyid;
	PGPKeyRef		key;
	PGPGroupID		groupid;
	PGPGroupItem	groupitem;
	PGPByte			exportedKeyID[kPGPMaxExportedKeyIDSize];
	PGPSize			size;
	TL_TREEITEM		tli;

	tli.hItem = GMFocusedItem (pGM);
	tli.mask = TLIF_IMAGE|TLIF_PARAM;
	TreeList_GetItem (pGM->hWndTree, &tli);

	// if selected item is a group then get the groupid
	if (tli.iImage == IDX_GROUP) {
		groupid = HIWORD(tli.lParam);
		if (!groupid) groupid = LOWORD(tli.lParam);
	}

	// otherwise it's a key, get the parent groupid
	else {
		groupid = HIWORD(tli.lParam);
	}

	if (groupid) {
		PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
		PGPNewKeyIter (keylist, &keyiter);

		PGPKeyIterNext (keyiter, &key);

		while (key) {
			groupitem.type = kPGPGroupItem_KeyID;
			groupitem.userValue = 0;
			PGPGetKeyNumber (key, kPGPKeyPropAlgID, 
									&groupitem.u.key.algorithm);
			PGPGetKeyIDFromKey (key, &keyid);
			PGPExportKeyID (&keyid, exportedKeyID, &size);
			groupitem.u.key.length = size;
			memcpy (groupitem.u.key.exportedKeyID, exportedKeyID, size);

			PGPAddItemToGroup (pGM->groupsetMain, &groupitem, groupid);

			PGPKeyIterNext (keyiter, &key);
		}
	
		PGPFreeKeyIter (keyiter);
		PGPFreeKeyList (keylist);
	}
	
	return TRUE;
}

//	_______________________________________________
//
//  Drop text key(s)

BOOL 
GMDropKeys (
		PGROUPMAN	pGM, 
		HANDLE		hMem) 
{
	PGPKeySetRef	keyset;
	PGPError		err;
	BOOL			bAdded;
	LPSTR			pMem;
	size_t			sLen;

	bAdded = FALSE;
	if (hMem) {
		pMem = GlobalLock (hMem);
		if (pMem) {
			sLen = lstrlen (pMem);
			err =PGPImportKeySet (pGM->context, &keyset, 
							PGPOInputBuffer (pGM->context, pMem, sLen),
							PGPOLastOption (pGM->context));
			if (!PGPcomdlgErrorMessage (err) && PGPRefIsValid (keyset)) {
				PGPUInt32	numKeys;
				PGPCountKeys (keyset, &numKeys);
				
				if (numKeys > 0) {
					GMAddKeysToGroup (pGM, keyset);

					TreeList_Select (pGM->hWndTree, NULL, TRUE);
					GMSetFocus (pGM, NULL, FALSE);

					bAdded = TRUE;
				}

				PGPFreeKeySet (keyset);
			}
			GlobalUnlock (hMem);
		}
	}

	if (bAdded) {
		GMSortGroupSet (pGM);
		GMCommitGroupChanges (pGM, TRUE);
		GMLoadGroupsIntoTree (pGM, FALSE, TRUE, FALSE);
		InvalidateRect (pGM->hWndTree, NULL, TRUE);
		UpdateWindow (pGM->hWndTree);
	}
	
	return bAdded;
}

//	_______________________________________________
//
//  Paste key(s) from clipboard

BOOL 
GMPasteKeys (PGROUPMAN pGM) 
{
	BOOL			bAdded;
	HANDLE			hMem;

	bAdded = FALSE;

	if (OpenClipboard (NULL)) {
		hMem = GetClipboardData (CF_TEXT);
		if (hMem) {
			bAdded = GMDropKeys (pGM, hMem);
		}
		CloseClipboard ();
	}

	if (!bAdded) {
		PGPgmMessageBox (pGM->hWndParent, IDS_CAPTION, 
					IDS_NOPASTEKEYS, MB_OK|MB_ICONEXCLAMATION);

	}

	return bAdded;
}

//----------------------------------------------------|
//  Locate (ie. select) a single object
//	routine called as a callback function from the TreeList control to 
//	delete a single item.
//
//	lptli	= pointer to TreeList item to delete

BOOL CALLBACK 
LocateSingleObject (TL_TREEITEM* lptli, 
					LPARAM lParam) 
{
	LOCATESTRUCT*	pls			= (LOCATESTRUCT*)lParam;
	PGPKeyID		keyid;
	PGPKeyRef		key;
	PGPUInt32		index;
	PGPGroupID		id;
	PGPGroupItem	groupitem;
	PGPError		err;

	switch (lptli->iImage) {

	// is it a group ?
	case IDX_GROUP :
		return TRUE;

	// otherwise it's a key 
	default:
		id = HIWORD(lptli->lParam);
		index = LOWORD(lptli->lParam);
		if (id) {
			PGPGetIndGroupItem (pls->pGM->groupsetMain, 
											id, index, &groupitem);

			err = PGPImportKeyID (groupitem.u.key.exportedKeyID, &keyid);

			if (IsntPGPError (err)) {
				err = PGPGetKeyByKeyID (pls->pGM->keysetMain, &keyid, 
							groupitem.u.key.algorithm, &key);

				if (IsntPGPError (err)) {
					PGPkmSelectKey (pls->pGM->hKM, key, pls->bFirst);
					pls->bFirst = FALSE;
				}
			}

			return TRUE;
		}
	}
}

//	___________________________________________________
//
//	select keys in main keys window

BOOL  
GMLocateKeys (PGROUPMAN pGM)
{
	LOCATESTRUCT	ls;

	ls.lpfnCallback = LocateSingleObject;
	ls.pGM = pGM;
	ls.bFirst = TRUE;

	pGM->bLocatingKeys = TRUE;
	TreeList_IterateSelected (pGM->hWndTree, &ls);
	pGM->bLocatingKeys = FALSE;

	return TRUE;
}


//	_______________________________________________
//
//  Import groups from group file
//
//	hDrop	== NULL	=> prompt user for file name
//			!= NULL => hDrop is a handle passed in from 
//						the WM_DROPFILE message
//

BOOL 
GMImportGroups (
		PGROUPMAN	pGM, 
		HDROP		hDrop) 
{
	OPENFILENAME	OpenFileName;
	CHAR			szFile[MAX_PATH];
	CHAR			szFilter[256];
	CHAR			szTitle[64];
	CHAR*			p;
	INT				iIndex;
	BOOL			bImport;
	PFLContextRef	pflcontext;
	PFLFileSpecRef	filespec;
	PGPGroupSetRef	groupset;
	PGPError		err;

	// file was dropped
	if (hDrop) {
		iIndex = 0;
		bImport = FALSE;
		while (DragQueryFile (hDrop, iIndex, szFile, sizeof(szFile))) {
			PFLNewContext (&pflcontext);
			if (pflcontext) {
				PFLNewFileSpecFromFullPath (pflcontext, szFile, &filespec);
				if (filespec) {
					err = PGPNewGroupSetFromFile (filespec, &groupset);
					if (IsntPGPError (err)) {
						err = PGPMergeGroupSets (groupset, pGM->groupsetMain);
						if (IsntPGPError (err)) bImport = TRUE;
					}
					PFLFreeFileSpec (filespec);
				}
				PFLFreeContext (pflcontext);
			}
			iIndex++;
		}
		if (!bImport) {
			PGPgmMessageBox (pGM->hWndParent, IDS_CAPTION, 
							IDS_NOIMPORTGROUPS, MB_OK|MB_ICONEXCLAMATION);
		}
	}

	// file wasn't dropped, prompt user for file name
	else {
		lstrcpy (szFile, "");
		LoadString (g_hInst, IDS_GROUPIMPORTFILTER, 
									szFilter, sizeof(szFilter));
		while (p = strrchr (szFilter, '@')) *p = '\0';
		LoadString (g_hInst, IDS_GROUPIMPORTCAPTION, 
									szTitle, sizeof(szTitle));
		OpenFileName.lStructSize       = sizeof (OPENFILENAME);
		OpenFileName.hwndOwner         = pGM->hWndParent;
		OpenFileName.hInstance         = (HANDLE)g_hInst;
		OpenFileName.lpstrFilter       = szFilter;
		OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
		OpenFileName.nMaxCustFilter    = 0L;
		OpenFileName.nFilterIndex      = 1L;
		OpenFileName.lpstrFile         = szFile;
		OpenFileName.nMaxFile          = sizeof (szFile);
		OpenFileName.lpstrFileTitle    = NULL;
		OpenFileName.nMaxFileTitle     = 0;
		OpenFileName.lpstrInitialDir   = NULL;
		OpenFileName.lpstrTitle        = szTitle;
		OpenFileName.Flags			   = OFN_HIDEREADONLY|OFN_NOCHANGEDIR;
		OpenFileName.nFileOffset       = 0;
		OpenFileName.nFileExtension    = 0;
		OpenFileName.lpstrDefExt       = "";
		OpenFileName.lCustData         = 0;

		bImport = FALSE;
		if (GetOpenFileName (&OpenFileName)) {
			PFLNewContext (&pflcontext);
			if (pflcontext) {
				PFLNewFileSpecFromFullPath (pflcontext, szFile, &filespec);
				if (filespec) {
					err = PGPNewGroupSetFromFile (filespec, &groupset);
					if (IsntPGPError (err)) {
						err = PGPMergeGroupSets (groupset, pGM->groupsetMain);
						if (IsntPGPError (err)) bImport = TRUE;
					}
					PFLFreeFileSpec (filespec);
				}
				PFLFreeContext (pflcontext);
			}
			if (!bImport) {
				PGPgmMessageBox (pGM->hWndParent, IDS_CAPTION, 
							IDS_NOIMPORTGROUPS, MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}

	if (bImport) {
		GMSortGroupSet (pGM);
		GMCommitGroupChanges (pGM, TRUE);
		GMLoadGroupsIntoTree (pGM, FALSE, TRUE, FALSE);
		InvalidateRect (pGM->hWndTree, NULL, TRUE);
		UpdateWindow (pGM->hWndTree);
	}

	return bImport;
}


