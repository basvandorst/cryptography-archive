/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMKeyIO.c - implements various operations related to importing 
				and exporting keys  
	

	$Id: KMKeyIO.c,v 1.24 1997/10/21 13:36:38 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpkmx.h"
#include <commdlg.h>

#include "resource.h"

#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"

typedef struct {
	FARPROC			lpfnCallback;
	PGPContextRef	Context;
	PGPKeySetRef	ExportKeySet;
} EXPORTSTRUCT;

#include "pgpVersionHeader.c"

// external globals
extern HINSTANCE g_hInst;

// local globals
static BOOL bDraggingOut = FALSE;   //NB: valid across keymanagers!
									//prevents bonus selective import dialog

// prototypes
BOOL IsDeleteEnabled (PKEYMAN pKM);


//	_______________________________________________
//
//  Import key from text file
//
//	hDrop	== NULL	=> prompt user for file name
//			!= NULL => hDrop is a handle passed in from 
//						the WM_DROPFILE message
//

BOOL 
KMImportKey (
		PKEYMAN pKM, 
		HDROP	hDrop) 
{
	OPENFILENAME	OpenFileName;
	CHAR			szFile[MAX_PATH];
	CHAR			szFilter[256];
	CHAR			szTitle[64];
	CHAR*			p;
	INT				iIndex;
	BOOL			bImport;
	BOOL			bKeys;
	PGPFileSpecRef	FileRef;
	PGPKeySetRef	KeySet;

	if (hDrop) {
		iIndex = 0;
		bImport = FALSE;
		bKeys = FALSE;
		while (DragQueryFile (hDrop, iIndex, szFile, sizeof(szFile))) {
			PGPNewFileSpecFromFullPath (pKM->Context, szFile, &FileRef);
			if (FileRef) {
				PGPImportKeySet (pKM->Context, &KeySet, 
								PGPOInputFile (pKM->Context, FileRef),
								PGPOLastOption (pKM->Context));
				if (KeySet) {
					PGPUInt32	numKeys;
					PGPCountKeys (KeySet, &numKeys);
					if (numKeys > 0) {
						bKeys = TRUE;
						if (bDraggingOut) {
							if (!PGPcomdlgErrorMessage (
								PGPAddKeys (KeySet, pKM->KeySetDisp))) {
								KMCommitKeyRingChanges (pKM);
								bImport = TRUE;
							}
						}
						else {
							if (PGPkmQueryAddKeys (pKM->Context, 
									pKM->hWndParent, 
									KeySet, 
									pKM->KeySetDisp) == kPGPError_NoErr) {
								KMCommitKeyRingChanges (pKM);
								bImport = TRUE;
							}
						}
					}
					PGPFreeKeySet (KeySet);
				}
				PGPFreeFileSpec (FileRef);
			}
			iIndex++;
		}
		if (!bKeys) {
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_NOIMPORTKEYS, 
							MB_OK|MB_ICONEXCLAMATION);
		}
	}
	else {
		lstrcpy (szFile, "");
		LoadString (g_hInst, IDS_IMPORTFILTER, szFilter, sizeof(szFilter));
		while (p = strrchr (szFilter, '@')) *p = '\0';
		LoadString (g_hInst, IDS_IMPORTCAPTION, szTitle, sizeof(szTitle));
		OpenFileName.lStructSize       = sizeof (OPENFILENAME);
		OpenFileName.hwndOwner         = pKM->hWndParent;
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
		bKeys = FALSE;
		if (GetOpenFileName (&OpenFileName)) {
			PGPNewFileSpecFromFullPath (pKM->Context, szFile, &FileRef);
			if (FileRef) {
				PGPImportKeySet (pKM->Context, &KeySet, 
								PGPOInputFile (pKM->Context, FileRef),
								PGPOLastOption (pKM->Context));
				if (KeySet) {
					PGPUInt32	numKeys;
					
					PGPCountKeys (KeySet, &numKeys);
					if ( numKeys > 0) {
						bKeys = TRUE;
						if (KMCheckForSecretKeys (KeySet))
							KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
											IDS_IMPORTEDSECRETKEYS, 
											MB_OK|MB_ICONEXCLAMATION);
						if (PGPkmQueryAddKeys (pKM->Context, 
												pKM->hWndParent,
												KeySet, pKM->KeySetDisp) == 
													kPGPError_NoErr) {
							KMCommitKeyRingChanges (pKM);
							bImport = TRUE;
						}
					}
					PGPFreeKeySet (KeySet);
				}
				PGPFreeFileSpec (FileRef);
			}
			if (!bKeys) {
				KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_NOIMPORTKEYS, 
								MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}

	if (bImport) {
		KMLoadKeyRingIntoTree (pKM, FALSE, TRUE, FALSE);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
	}

	return bImport;
}

//	_______________________________________________
//
//	routine called as a callback function from the 
//  TreeList control to add single key to keyset for export
//
//	lptli	= pointer to TreeList item to add

BOOL CALLBACK 
AddKeyToExportKeySet (
		TL_TREEITEM*	lptli, 
		LPARAM			lParam) 
{
	PGPKeySetRef	KeySet;
	EXPORTSTRUCT*	pes = (EXPORTSTRUCT*)lParam;

	PGPNewSingletonKeySet ((PGPKeyRef)(lptli->lParam), &KeySet);
	PGPcomdlgErrorMessage (PGPAddKeys (KeySet, pes->ExportKeySet));
	PGPCommitKeyRingChanges (pes->ExportKeySet);
	PGPFreeKeySet (KeySet);

	return TRUE;
}

//	_______________________________________________
//
//  Get appropriate comment string

VOID 
GetCommentString (
		LPSTR	sz, 
		UINT	uLen) 
{
	PGPPrefRef	prefref;

	sz[0] = 0;

#if PGP_BUSINESS_SECURITY
	PGPcomdlgOpenAdminPrefs (&prefref, FALSE);
	PGPGetPrefStringBuffer (prefref, kPGPPrefComments, uLen, sz);
	PGPcomdlgCloseAdminPrefs (prefref, FALSE);
#endif	// PGP_BUSINESS_SECURITY

	if (sz[0] == 0) {
		PGPcomdlgOpenClientPrefs (&prefref);
		PGPGetPrefStringBuffer (prefref, kPGPPrefComment, uLen, sz);
		PGPcomdlgCloseClientPrefs (prefref, FALSE);
	}
}

//	_______________________________________________
//
//  Export key(s) to text file

BOOL 
ExportKeys (
		PKEYMAN			pKM, 
		PGPKeySetRef	keyset,
		LPSTR			szFile,
		PGPBoolean		bExportPrivateKeys) 
{
	BOOL			bRetVal				= FALSE;
	PGPFileSpecRef	FileRef;
	CHAR			szComment[256];

	PGPNewFileSpecFromFullPath (pKM->Context, szFile, &FileRef);
	if (FileRef) {	
		GetCommentString (szComment, sizeof(szComment));
		if (!PGPcomdlgErrorMessage (
				PGPExportKeySet (keyset, 
								 PGPOOutputFile (pKM->Context, FileRef),
								 PGPOVersionString (pKM->Context, 
												pgpVersionHeaderString),
								 PGPOCommentString (pKM->Context, szComment),
								 PGPOExportPrivateKeys (pKM->Context,
												bExportPrivateKeys),
								 PGPOLastOption (pKM->Context)))) {
			bRetVal = TRUE;
		}
		PGPFreeFileSpec (FileRef);
	}

	return bRetVal;
}

//	___________________________
//
//  Compute name of export file

VOID 
GetExportFileName (
		PKEYMAN pKM, 
		LPSTR	sz, 
		UINT	uLen) 
{
	CHAR		szDefExt[8];
	TL_TREEITEM tli;
	INT			i;

	if (!KMMultipleSelected (pKM)) {
		tli.hItem = KMFocusedItem (pKM);
		tli.pszText = sz;
		tli.cchTextMax = uLen;
		tli.mask = TLIF_TEXT;
		if (TreeList_GetItem (pKM->hWndTree, &tli)) {
			i = strcspn (sz, "\\/:*?""<>|");
			sz[i] = '\0';
			if (i > 0) {
				if (sz[i-1] == ' ') sz[i-1] = '\0';
			}
			if (lstrlen(sz) < 2) LoadString (g_hInst, IDS_DROPFILENAME, 
											 sz, uLen);
		}
		else LoadString (g_hInst, IDS_DROPFILENAME, sz, uLen);
	}
	else LoadString (g_hInst, IDS_DROPFILENAME, sz, uLen);

	lstrcat (sz, ".");
	LoadString (g_hInst, IDS_DEFEXPORTEXTENSION, szDefExt, sizeof(szDefExt));
	lstrcat (sz, szDefExt);

}

//----------------------------------------------------|
// Delete All dialog message procedure

BOOL CALLBACK 
ExportPrivKeyDlgProc (HWND hWndDlg, 
				  UINT uMsg, 
				  WPARAM wParam, 
				  LPARAM lParam) 
{
	OPENFILENAME*	pofn;
	PGPBoolean*		pbExport;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hWndDlg, GWL_USERDATA, lParam);
		pofn = (OPENFILENAME*)lParam;
		pbExport = (PGPBoolean*)pofn->lCustData;
		if (!(*pbExport)) 
			EnableWindow (GetDlgItem (hWndDlg, IDC_EXPORTPRIVATEKEYS), FALSE);
		*pbExport = FALSE;
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDC_EXPORTPRIVATEKEYS :
			pofn = (OPENFILENAME*)GetWindowLong (hWndDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hWndDlg, IDC_EXPORTPRIVATEKEYS) ==
															BST_CHECKED) {
				pbExport = (PGPBoolean*)pofn->lCustData;
				*pbExport = TRUE;
			}
			else {
				pbExport = (PGPBoolean*)pofn->lCustData;
				*pbExport = FALSE;
			}
			return TRUE;

		}
		return FALSE;
	}
	return FALSE;
}

//	_______________________________________________
//
//  Export key to text file

BOOL 
KMExportKeys (
		PKEYMAN pKM, 
		LPSTR	szFile) 
{
	PGPBoolean		bExportPrivKeys		= FALSE;
	OPENFILENAME	OpenFileName;
	EXPORTSTRUCT	es;
	CHAR			sz256[256];
	CHAR			szDefExt[8];
	CHAR			szFilter[256];
	CHAR			szTitle[64];
	CHAR*			p;
	BOOL			bRetVal;

	es.Context = pKM->Context;
	es.ExportKeySet = NULL;

	if (KMMultipleSelected (pKM)) {
		PGPNewKeySet (pKM->Context, &(es.ExportKeySet));
		if (es.ExportKeySet) {
			es.lpfnCallback = AddKeyToExportKeySet;
			TreeList_IterateSelected (pKM->hWndTree, &es);
		}
	}
	else {
		PGPNewSingletonKeySet((PGPKeyRef)KMFocusedObject (pKM), 
								&(es.ExportKeySet));
	}

	if (szFile) bRetVal = ExportKeys (pKM, es.ExportKeySet, szFile, FALSE);
	else {
		OpenFileName.lStructSize       = sizeof (OPENFILENAME);
		OpenFileName.hwndOwner         = pKM->hWndParent;
		OpenFileName.hInstance         = (HANDLE)g_hInst;
		LoadString (g_hInst, IDS_EXPORTFILTER, szFilter, sizeof(szFilter));
		while (p = strrchr (szFilter, '@')) *p = '\0';
		OpenFileName.lpstrFilter       = szFilter;
		OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
		OpenFileName.nMaxCustFilter    = 0L;
		OpenFileName.nFilterIndex      = 1L;
		GetExportFileName (pKM, sz256, sizeof (sz256));
		OpenFileName.lpstrFile         = sz256;
		OpenFileName.nMaxFile          = sizeof (sz256);
		OpenFileName.lpstrFileTitle    = NULL;
		OpenFileName.nMaxFileTitle     = 0;
		OpenFileName.lpstrInitialDir   = NULL;
		LoadString (g_hInst, IDS_EXPORTCAPTION, szTitle, sizeof(szTitle));
		OpenFileName.lpstrTitle        = szTitle;
		OpenFileName.Flags			   = OFN_EXPLORER|
										 OFN_ENABLETEMPLATE|
										 OFN_ENABLEHOOK|
										 OFN_HIDEREADONLY|
										 OFN_OVERWRITEPROMPT|
										 OFN_NOREADONLYRETURN|
										 OFN_NOCHANGEDIR;
		OpenFileName.nFileOffset       = 0;
		OpenFileName.nFileExtension    = 0;
		LoadString (g_hInst, IDS_DEFEXPORTEXTENSION, szDefExt, 
								sizeof(szDefExt));
		OpenFileName.lpstrDefExt       = szDefExt;
		OpenFileName.lCustData         = (DWORD)&bExportPrivKeys;
		OpenFileName.lpfnHook          = ExportPrivKeyDlgProc; 
		OpenFileName.lpTemplateName    = MAKEINTRESOURCE (IDD_EXPORTPRIVKEY);

		bExportPrivKeys = KMCheckForSecretKeys (es.ExportKeySet);
		if (GetSaveFileName (&OpenFileName)) {
			bRetVal = 
				ExportKeys (pKM, es.ExportKeySet, sz256, bExportPrivKeys);
		}
	}

	if (es.ExportKeySet) PGPFreeKeySet (es.ExportKeySet);

	return FALSE;
}

//	_______________________________________________
//
//  Drag selected key or keys out of application onto 
//  desktop or into Explorer.  Uses OLE interface.

BOOL 
KMDragAndDrop (PKEYMAN pKM) 
{
	size_t			slen;
	LPSTR			pExport;
	LPDATAOBJECT	lpdo;
	LPDROPSOURCE	lpds;
	DWORD			dwEffect = 0;
	EXPORTSTRUCT	es;
	CHAR			szFile[MAX_PATH];
	BOOL			bOKtoDelete = FALSE;
	CHAR			szComment[256];

	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;

	GetExportFileName (pKM, szFile, sizeof(szFile));

	es.Context = pKM->Context;

	if (KMMultipleSelected (pKM)) {
		PGPNewKeySet (pKM->Context, &(es.ExportKeySet));
		if (es.ExportKeySet) {
			es.lpfnCallback = AddKeyToExportKeySet;
			TreeList_IterateSelected (pKM->hWndTree, &es);
		}
	}
	else {
		PGPNewSingletonKeySet((PGPKeyRef)KMFocusedObject (pKM),
								&(es.ExportKeySet));
	}

	slen = 0;
	GetCommentString (szComment, sizeof(szComment));
	if (!PGPcomdlgErrorMessage (
		PGPExportKeySet (es.ExportKeySet, 
						PGPOAllocatedOutputBuffer (pKM->Context,
								&pExport, 0x40000000, &slen),
						PGPOVersionString (pKM->Context, 
								pgpVersionHeaderString),
						PGPOCommentString (pKM->Context, szComment),
						PGPOLastOption (pKM->Context)))) {
		if (pExport) {
			pExport[slen-2] = '\0';
			lpdo = KMCreateDataObject (szFile, pExport);
			lpds = KMCreateDropSource ();

			if (lpds) {
				if (lpdo) {
					bDraggingOut = TRUE;
					DoDragDrop (lpdo, lpds, 
						DROPEFFECT_COPY|DROPEFFECT_MOVE, &dwEffect); 
					bOKtoDelete = KMOKToDeleteDataObject (lpdo);
					lpdo->lpVtbl->Release(lpdo);
					bDraggingOut = FALSE;
				}
				lpds->lpVtbl->Release(lpds);
			}
			PGPFreeData (pExport);
		}
	}
	PGPFreeKeySet (es.ExportKeySet);

	if (dwEffect == DROPEFFECT_MOVE) {
		if (bOKtoDelete) {
			if (IsDeleteEnabled (pKM)) {
				KMDeleteObject (pKM);
			}
		}
	}
}

//	_______________________________________________
//
//  Copy key(s) to clipboard

BOOL 
KMCopyKeys (
		PKEYMAN pKM, 
		HANDLE* phMem) 
{
	size_t			slen;
	HANDLE			hMem;
	BOOL			bRetVal = FALSE;
	LPSTR			pMem;
	LPSTR			pMemG;
	EXPORTSTRUCT	es;
	CHAR			szComment[256];

	es.Context = pKM->Context;

	if (KMMultipleSelected (pKM)) {
		PGPNewKeySet (pKM->Context, &(es.ExportKeySet));
		if (es.ExportKeySet) {
			es.lpfnCallback = AddKeyToExportKeySet;
			TreeList_IterateSelected (pKM->hWndTree, &es);
		}
	}
	else {
		PGPNewSingletonKeySet((PGPKeyRef)KMFocusedObject (pKM),
								&(es.ExportKeySet));
	}

	slen = 0;
	GetCommentString (szComment, sizeof(szComment));
	if (!PGPcomdlgErrorMessage (
		PGPExportKeySet (es.ExportKeySet, 
						PGPOAllocatedOutputBuffer (pKM->Context,
								&pMem, 0x40000000, &slen),
						PGPOVersionString (pKM->Context, 
								pgpVersionHeaderString),
						PGPOCommentString (pKM->Context, szComment),
						PGPOLastOption (pKM->Context)))) {
		hMem = GlobalAlloc (GMEM_MOVEABLE|GMEM_DDESHARE, slen+1);
		if (hMem) {
			pMemG = GlobalLock (hMem);
			if (pMem) {
				pMem[slen-2] = '\0';
				memcpy (pMemG, pMem, slen+1);
				GlobalUnlock (hMem);
				if (phMem) {
					*phMem = hMem;
					bRetVal = TRUE;
				}
				else {
					if (OpenClipboard (NULL)) {
						EmptyClipboard ();
						SetClipboardData (CF_TEXT, hMem);
						CloseClipboard ();
						bRetVal = TRUE;
					}
				}
				PGPFreeData (pMem);
			}
			else GlobalFree (hMem);
		}
	}
	PGPFreeKeySet (es.ExportKeySet);

	return bRetVal;
}

//	_______________________________________________
//
//  Paste key(s) from clipboard

BOOL 
KMPasteKeys (PKEYMAN pKM) 
{
	PGPKeySetRef	KeySet;
	BOOL			bImport;
	BOOL			bKeys;
	HANDLE			hMem;
	LPSTR			pMem;
	size_t			sLen;
	HDROP			hDrop;

	bImport = FALSE;
	bKeys = FALSE;
	if (OpenClipboard (NULL)) {
		hMem = GetClipboardData (CF_HDROP);
		if (hMem) {
			hDrop = GlobalLock (hMem);
			bImport = KMImportKey (pKM, hDrop);
			GlobalUnlock (hMem);
			CloseClipboard ();
			return bImport;
		}
		hMem = GetClipboardData (CF_TEXT);
		if (hMem) {
			pMem = GlobalLock (hMem);
			if (pMem) {
				sLen = lstrlen (pMem);
				PGPImportKeySet (pKM->Context, &KeySet, 
								PGPOInputBuffer (pKM->Context, pMem, sLen),
								PGPOLastOption (pKM->Context));
				if (KeySet) {
					PGPUInt32	numKeys;
					(void)PGPCountKeys (KeySet, &numKeys );
					
					if (numKeys > 0) {
						bKeys = TRUE;
						if (KMCheckForSecretKeys (KeySet))
							KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
											IDS_IMPORTEDSECRETKEYS, 
											MB_OK|MB_ICONEXCLAMATION);
						if (PGPkmQueryAddKeys (pKM->Context, 
									pKM->hWndParent, KeySet, 
									pKM->KeySetDisp) == kPGPError_NoErr) {
							KMCommitKeyRingChanges (pKM);
							bImport = TRUE;
						}
					}
					PGPFreeKeySet (KeySet);
				}
				GlobalUnlock (hMem);
			}
		}
		if (!bKeys) {
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
				IDS_NOIMPORTKEYSPASTE, MB_OK|MB_ICONEXCLAMATION);
		}
		CloseClipboard ();
	}

	if (bImport) {
		KMLoadKeyRingIntoTree (pKM, FALSE, TRUE, FALSE);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
	}
	
	return bImport;
}

//	_______________________________________________
//
//  Drop text key(s)

BOOL 
KMDropKeys (
		PKEYMAN pKM, 
		HANDLE	hMem) 
{
	PGPKeySetRef	KeySet;
	BOOL			bImport;
	BOOL			bKeys;
	LPSTR			pMem;
	size_t			sLen;

	bImport = FALSE;
	bKeys = FALSE;
	if (hMem) {
		pMem = GlobalLock (hMem);
		if (pMem) {
			sLen = lstrlen (pMem);
			PGPImportKeySet (pKM->Context, &KeySet, 
							PGPOInputBuffer (pKM->Context, pMem, sLen),
							PGPOLastOption (pKM->Context));
			if (KeySet) {
				PGPUInt32	numKeys;
				(void)PGPCountKeys (KeySet, &numKeys );
				
				if ( numKeys > 0) {
					bKeys = TRUE;
					if (KMCheckForSecretKeys (KeySet))
						KMMessageBox (pKM->hWndParent, IDS_CAPTION, 
										IDS_IMPORTEDSECRETKEYS, 
										MB_OK|MB_ICONEXCLAMATION);
					if (bDraggingOut) {
						if (!PGPcomdlgErrorMessage (
							PGPAddKeys (KeySet, pKM->KeySetDisp))) {
							KMCommitKeyRingChanges (pKM);
							bImport = TRUE;
						}
					}
					else {
						if (PGPkmQueryAddKeys (pKM->Context, 
								pKM->hWndParent, 
								KeySet, 
								pKM->KeySetDisp) == kPGPError_NoErr) {
							KMCommitKeyRingChanges (pKM);
							bImport = TRUE;
						}
					}
				}
				PGPFreeKeySet (KeySet);
			}
			GlobalUnlock (hMem);
		}
		if (!bKeys) {
			KMMessageBox (pKM->hWndParent, IDS_CAPTION, IDS_NODRAGTEXTKEYS, 
				MB_OK|MB_ICONEXCLAMATION);
		}
	}

	if (bImport) {
		KMLoadKeyRingIntoTree (pKM, FALSE, TRUE, FALSE);
		InvalidateRect (pKM->hWndTree, NULL, TRUE);
	}
	
	return bImport;
}

//	___________________________________________
//
//  Determine if there is text in the clipboard

BOOL 
KMDataToPaste (VOID) 
{
	if (IsClipboardFormatAvailable (CF_TEXT)) return TRUE;
	if (IsClipboardFormatAvailable (CF_HDROP)) return TRUE;
	return FALSE;
}


