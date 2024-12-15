/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//:KMKeyIO.c - implements various operations related to importing
//             and exporting keys
//
//	$Id: KMKeyIO.c,v 1.4 1997/05/20 18:30:17 pbj Exp $
//

#include <windows.h>
#include "pgpkeys.h"
#include "resource.h"

// external globals
extern HINSTANCE g_hInst;
extern HWND g_hWndMain;
extern HWND g_hWndTree;
extern PGPKeySet* g_pKeySetMain;
extern CHAR g_szbuf[G_BUFLEN];

// local globals
static PGPKeySet* pExportKeySet;


//----------------------------------------------------|
//  Import key from text file
//
//	hDrop	== NULL	=> prompt user for file name
//			!= NULL => hDrop is a handle passed in from
//						the WM_DROPFILE message
//

BOOL KMImportKey (HWND hWnd, HDROP hDrop) {
	OPENFILENAME OpenFileName;
	CHAR szFile[MAX_PATH];
	CHAR szFilter[256];
	CHAR* p;
	INT iIndex;
	BOOL bImport;
	PGPFileRef* pFileRef;
	PGPKeySet* pKeySet;

	if (hDrop) {
		iIndex = 0;
		bImport = FALSE;
		while (DragQueryFile (hDrop, iIndex, szFile, sizeof(szFile))) {
			pFileRef = pgpNewFileRefFromFullPath (szFile);
			if (pFileRef) {
				pKeySet = pgpImportKeyFile (pFileRef);
				if (pKeySet) {
					if (pgpCountKeys (pKeySet) > 0) {
						if (!PGPcomdlgErrorMessage (
#if 1	//>>> pre-existing code
								pgpAddKeys (g_pKeySetMain, pKeySet))) {
#else	//>>> PGPcomdlgQueryAddKeys changes ...
								PGPcomdlgQueryAddKeys (hWnd,
									g_pKeySetMain, pKeySet))) {
#endif	//<<< PGPcomdlgQueryAddKeys changes
							KMCommitKeyRingChanges (g_pKeySetMain);
							bImport = TRUE;
						}
					}
					pgpFreeKeySet (pKeySet);
				}
				pgpFreeFileRef (pFileRef);
			}
			iIndex++;
		}
		if (!bImport) {
			KMMessageBox (hWnd, IDS_CAPTION, IDS_NOIMPORTKEYS,
							MB_OK|MB_ICONEXCLAMATION);
		}
	}
	else {
		lstrcpy (szFile, "");
		LoadString (g_hInst, IDS_IMPORTFILTER, szFilter, sizeof(szFilter));
		while (p = strrchr (szFilter, '@')) *p = '\0';
		LoadString (g_hInst, IDS_IMPORTCAPTION, g_szbuf, G_BUFLEN);
		OpenFileName.lStructSize       = sizeof (OPENFILENAME);
		OpenFileName.hwndOwner         = hWnd;
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
		OpenFileName.lpstrTitle        = g_szbuf;
		OpenFileName.Flags			   = OFN_HIDEREADONLY|OFN_NOCHANGEDIR;
		OpenFileName.nFileOffset       = 0;
		OpenFileName.nFileExtension    = 0;
		OpenFileName.lpstrDefExt       = "";
		OpenFileName.lCustData         = 0;

		bImport = FALSE;
		if (GetOpenFileName (&OpenFileName)) {
			pFileRef = pgpNewFileRefFromFullPath (szFile);
			if (pFileRef) {
				pKeySet = pgpImportKeyFile (pFileRef);
				if (pKeySet) {
					if (pgpCountKeys (pKeySet) > 0) {
						if (KMCheckForSecretKeys (pKeySet))
							KMMessageBox (hWnd, IDS_CAPTION,
											IDS_IMPORTEDSECRETKEYS,
											MB_OK|MB_ICONEXCLAMATION);
						if (!PGPcomdlgErrorMessage (
#if 1	//>>> pre-existing code
								pgpAddKeys (g_pKeySetMain, pKeySet))) {
#else	//>>> PGPcomdlgQueryAddKeys changes ...
								PGPcomdlgQueryAddKeys (hWnd,
									g_pKeySetMain, pKeySet))) {
#endif	//<<< PGPcomdlgQueryAddKeys changes
							KMCommitKeyRingChanges (g_pKeySetMain);
							bImport = TRUE;
						}
					}
					pgpFreeKeySet (pKeySet);
				}
				pgpFreeFileRef (pFileRef);
			}
			if (!bImport) {
				KMMessageBox (hWnd, IDS_CAPTION, IDS_NOIMPORTKEYS,
								MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}

	if (bImport) {
		KMLoadKeyRingIntoTree (hWnd, g_hWndTree, FALSE, TRUE);
		InvalidateRect (g_hWndTree, NULL, TRUE);
	}

	return bImport;
}


//----------------------------------------------------|
//	routine called as a callback function from the
//  TreeList control to add single key to keyset for export
//
//	lptli	= pointer to TreeList item to add

BOOL AddKeyToExportKeySet (TL_TREEITEM* lptli) {
	PGPKeySet* pKeySet;

	pKeySet = pgpNewSingletonKeySet ((PGPKey*)(lptli->lParam));
	PGPcomdlgErrorMessage (pgpAddKeys (pExportKeySet, pKeySet));
	pgpFreeKeySet (pKeySet);

	return TRUE;
}

	
//----------------------------------------------------|
//  Export key(s) to text file

BOOL ExportKeys (LPSTR szFile) {
	PGPFileRef* pFileRef;

	if (KMMultipleSelected ()) {
		pExportKeySet = pgpNewKeySet ();
		if (pExportKeySet)
			TreeList_IterateSelected (g_hWndTree, AddKeyToExportKeySet);
	}
	else {
		pExportKeySet = pgpNewSingletonKeySet ((PGPKey*) KMFocusedObject ());
	}

	pFileRef = pgpNewFileRefFromFullPath (szFile);
	if (pFileRef) {
		if (PGPcomdlgErrorMessage (
				pgpExportKeyFile (pExportKeySet, pFileRef))) {
			pgpFreeKeySet (pExportKeySet);
			return FALSE;
		}
		pgpFreeFileRef (pFileRef);
	}

	pgpFreeKeySet (pExportKeySet);
	return TRUE;
}


//----------------------------------------------------|
//  Compute name of export file

VOID GetExportFileName (LPSTR sz, UINT uLen) {
	CHAR szDefExt[8];
	TL_TREEITEM tli;
	INT i;

	if (!KMMultipleSelected ()) {
		tli.hItem = KMFocusedItem ();
		tli.pszText = sz;
		tli.cchTextMax = uLen;
		tli.mask = TLIF_TEXT;
		if (TreeList_GetItem (g_hWndTree, &tli)) {
			i = strcspn (sz, "\\/:*?""<>|");
			sz[i] = '\0';
			if (i > 0) {
				if (sz[i-1] == ' ') sz[i-1] = '\0';
			}
			if (lstrlen (sz) < 2) LoadString (g_hInst, IDS_DROPFILENAME,
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
//  Export key to text file

BOOL KMExportKeys (HWND hWnd, LPSTR szFile) {
	OPENFILENAME OpenFileName;
	CHAR sz256[256];
	CHAR szDefExt[8];
	CHAR szFilter[256];
	CHAR* p;

	if (szFile) return (ExportKeys (szFile));
	else {
		OpenFileName.lStructSize       = sizeof (OPENFILENAME);
		OpenFileName.hwndOwner         = hWnd;
		OpenFileName.hInstance         = (HANDLE)g_hInst;
		LoadString (g_hInst, IDS_EXPORTFILTER, szFilter, sizeof(szFilter));
		while (p = strrchr (szFilter, '@')) *p = '\0';
		OpenFileName.lpstrFilter       = szFilter;
		OpenFileName.lpstrCustomFilter = (LPTSTR)NULL;
		OpenFileName.nMaxCustFilter    = 0L;
		OpenFileName.nFilterIndex      = 1L;
		GetExportFileName (sz256, sizeof (sz256));
		OpenFileName.lpstrFile         = sz256;
		OpenFileName.nMaxFile          = sizeof (sz256);
		OpenFileName.lpstrFileTitle    = NULL;
		OpenFileName.nMaxFileTitle     = 0;
		OpenFileName.lpstrInitialDir   = NULL;
		LoadString (g_hInst, IDS_EXPORTCAPTION, g_szbuf, G_BUFLEN);
		OpenFileName.lpstrTitle        = g_szbuf;
		OpenFileName.Flags			   = OFN_HIDEREADONLY|
										 OFN_OVERWRITEPROMPT|
										 OFN_NOREADONLYRETURN|
										 OFN_NOCHANGEDIR;
		OpenFileName.nFileOffset       = 0;
		OpenFileName.nFileExtension    = 0;
		LoadString (g_hInst, IDS_DEFEXPORTEXTENSION, szDefExt,
								sizeof(szDefExt));
		OpenFileName.lpstrDefExt       = szDefExt;
		OpenFileName.lCustData         = 0;

		if (GetSaveFileName (&OpenFileName)) return (ExportKeys (sz256));
	}

	return FALSE;
}


//----------------------------------------------------|
//  Drag selected key or keys out of application onto
//  desktop or into Explorer.  Uses OLE interface.

BOOL KMDragAndDrop (HWND hWnd) {
	size_t			slen;
	LPSTR			pExport;
	LPDATAOBJECT	lpdo;
	LPDROPSOURCE	lpds;
	DWORD			dwEffect;

	if (KMFocusedObjectType () != OBJECT_KEY) return FALSE;

	GetExportFileName (g_szbuf, G_BUFLEN);

	if (KMMultipleSelected ()) {
		pExportKeySet = pgpNewKeySet ();
		if (pExportKeySet)
			TreeList_IterateSelected (g_hWndTree, AddKeyToExportKeySet);
	}
	else {
		pExportKeySet = pgpNewSingletonKeySet ((PGPKey*) KMFocusedObject ());
	}

	slen = 0;
	if (!PGPcomdlgErrorMessage (pgpExportKeyBuffer (
				pExportKeySet, NULL, &slen))) {
		pExport = malloc (slen+1);
		if (pExport) {
			if (!PGPcomdlgErrorMessage (pgpExportKeyBuffer (
						pExportKeySet, pExport, &slen))) {
				pExport[slen] = '\0';
				lpdo = KMCreateDataObject (g_szbuf, pExport);
				lpds = KMCreateDropSource ();

				if (lpds) {
					if (lpdo) {
						DoDragDrop (lpdo, lpds, DROPEFFECT_COPY, &dwEffect);
						lpdo->lpVtbl->Release(lpdo);
					}
					lpds->lpVtbl->Release(lpds);
				}
			}
			free (pExport);
		}
	}
	pgpFreeKeySet (pExportKeySet);
}


//----------------------------------------------------|
//  Copy key(s) to clipboard

BOOL KMCopyKeys (HANDLE* phMem) {
	size_t slen;
	HANDLE hMem;
	BOOL bRetVal = FALSE;
	LPSTR pMem;

	if (KMMultipleSelected ()) {
		pExportKeySet = pgpNewKeySet ();
		if (pExportKeySet)
			TreeList_IterateSelected (g_hWndTree, AddKeyToExportKeySet);
	}
	else {
		pExportKeySet = pgpNewSingletonKeySet ((PGPKey*) KMFocusedObject ());
	}

	slen = 0;
	if (!PGPcomdlgErrorMessage (pgpExportKeyBuffer (
				pExportKeySet, NULL, &slen))) {
		hMem = GlobalAlloc (GMEM_MOVEABLE|GMEM_DDESHARE, slen+1);
		if (hMem) {
			pMem = GlobalLock (hMem);
			if (!PGPcomdlgErrorMessage (pgpExportKeyBuffer (
						pExportKeySet, pMem, &slen))) {
				pMem[slen] = '\0';
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
			}
			else GlobalFree (hMem);
		}
	}
	pgpFreeKeySet (pExportKeySet);

	return bRetVal;
}


//----------------------------------------------------|
//  Paste key(s) from clipboard

BOOL KMPasteKeys (HWND hWnd) {
	PGPKeySet* pKeySet;
	BOOL bImport;
	HANDLE hMem;
	LPSTR pMem;
	size_t sLen;
	HDROP hDrop;

	bImport = FALSE;
	if (OpenClipboard (NULL)) {
		hMem = GetClipboardData (CF_HDROP);
		if (hMem) {
			hDrop = GlobalLock (hMem);
			bImport = KMImportKey (hWnd, hDrop);
			GlobalUnlock (hMem);
			CloseClipboard ();
			return bImport;
		}
		hMem = GetClipboardData (CF_TEXT);
		if (hMem) {
			pMem = GlobalLock (hMem);
			if (pMem) {
				sLen = lstrlen (pMem);
				pKeySet = pgpImportKeyBuffer (pMem, sLen);
				if (pKeySet) {
					if (pgpCountKeys (pKeySet) > 0) {
						if (KMCheckForSecretKeys (pKeySet))
							KMMessageBox (hWnd, IDS_CAPTION,
											IDS_IMPORTEDSECRETKEYS,
											MB_OK|MB_ICONEXCLAMATION);
#if 1	//>>> pre-existing code
						if (!PGPcomdlgErrorMessage (pgpAddKeys (
										g_pKeySetMain, pKeySet))) {
#else	//>>> PGPcomdlgQueryAddKeys changes ...
						if (!PGPcomdlgErrorMessage (
								PGPcomdlgQueryAddKeys(
									hWnd, g_pKeySetMain, pKeySet))) {
#endif	//<<< PGPcomdlgQueryAddKeys changes
							KMCommitKeyRingChanges (g_pKeySetMain);
							bImport = TRUE;
						}
					}
					pgpFreeKeySet (pKeySet);
				}
				GlobalUnlock (hMem);
			}
		}
		if (!bImport) {
			KMMessageBox (hWnd, IDS_CAPTION, IDS_NOIMPORTKEYSPASTE,
				MB_OK|MB_ICONEXCLAMATION);
		}
		CloseClipboard ();
	}

	if (bImport) {
		KMLoadKeyRingIntoTree (hWnd, g_hWndTree, FALSE, TRUE);
		InvalidateRect (g_hWndTree, NULL, TRUE);
	}
	
	return bImport;
}


//----------------------------------------------------|
//  Drop text key(s)

BOOL KMDropKeys (HANDLE hMem) {
	PGPKeySet* pKeySet;
	BOOL bImport;
	LPSTR pMem;
	size_t sLen;

	bImport = FALSE;
	if (hMem) {
		pMem = GlobalLock (hMem);
		if (pMem) {
			sLen = lstrlen (pMem);
			pKeySet = pgpImportKeyBuffer (pMem, sLen);
			if (pKeySet) {
				if (pgpCountKeys (pKeySet) > 0) {
					if (KMCheckForSecretKeys (pKeySet))
						KMMessageBox (g_hWndMain, IDS_CAPTION,
										IDS_IMPORTEDSECRETKEYS,
										MB_OK|MB_ICONEXCLAMATION);
#if 1	//>>> pre-existing code
					if (!PGPcomdlgErrorMessage (pgpAddKeys (
									g_pKeySetMain, pKeySet))) {
#else	//>>> PGPcomdlgQueryAddKeys changes ...
					if (!PGPcomdlgErrorMessage (
							PGPcomdlgQueryAddKeys(
								g_hWndMain, g_pKeySetMain, pKeySet))) {
#endif	//<<< PGPcomdlgQueryAddKeys changes
						KMCommitKeyRingChanges (g_pKeySetMain);
						bImport = TRUE;
					}
				}
				pgpFreeKeySet (pKeySet);
			}
			GlobalUnlock (hMem);
		}
		if (!bImport) {
			KMMessageBox (g_hWndMain, IDS_CAPTION, IDS_NODRAGTEXTKEYS,
				MB_OK|MB_ICONEXCLAMATION);
		}
	}

	if (bImport) {
		KMLoadKeyRingIntoTree (g_hWndMain, g_hWndTree, FALSE, TRUE);
		InvalidateRect (g_hWndTree, NULL, TRUE);
	}
	
	return bImport;
}


//----------------------------------------------------|
//  Determine if there is text in the clipboard

BOOL KMDataToPaste (void) {
	if (IsClipboardFormatAvailable (CF_TEXT)) return TRUE;
	if (IsClipboardFormatAvailable (CF_HDROP)) return TRUE;
	return FALSE;
}
