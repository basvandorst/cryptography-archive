/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMKeyIO.c - implements various operations related to importing 
				and exporting keys  
	

	$Id: KMKeyIO.c,v 1.16 1998/08/13 02:46:10 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// pgp header files
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pgpVersionHeader.c"

// system header files
#include <commdlg.h>

// typedefs
typedef struct {
	PGPBoolean	bExportPrivKeys;
	PGPBoolean	bCompatible;
	LPSTR		pszHelpFile;
} EXPORTPRIVKEYSTRUCT, *PEXPORTPRIVKEYSTRUCT;

// external globals
extern HINSTANCE g_hInst;

// local globals
static BOOL bDraggingOut = FALSE;   //NB: valid across keymanagers!
									//prevents bonus selective import dialog

static DWORD aExportKeyIds[] = {			// Help IDs
    IDC_EXPORTPRIVATEKEYS,		IDH_PGPKM_EXPORTPRIVATEKEYS, 
    IDC_INCLUDEEXTENSIONS,		IDH_PGPKM_INCLUDEEXTENSIONS, 
    0,0 
}; 

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
							if (IsntPGPError (PGPclErrorBox (NULL, 
								PGPAddKeys (KeySet, pKM->KeySetDisp)))) {
								KMCommitKeyRingChanges (pKM);
								bImport = TRUE;
							}
						}
						else {
							if (PGPclQueryAddKeys (pKM->Context,
									pKM->tlsContext,
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
						if (PGPclQueryAddKeys (pKM->Context, 
												pKM->tlsContext,
												pKM->hWndParent,
												KeySet, 
												pKM->KeySetDisp) == 
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
//  Get appropriate comment string

static VOID 
sGetCommentString (
		PGPMemoryMgrRef	memMgr,
		LPSTR			sz, 
		UINT			uLen) 
{
	PGPPrefRef	prefref;

	sz[0] = 0;

#if PGP_BUSINESS_SECURITY
	PGPclOpenAdminPrefs (memMgr, &prefref, FALSE);
	PGPGetPrefStringBuffer (prefref, kPGPPrefComments, uLen, sz);
	PGPclCloseAdminPrefs (prefref, FALSE);
#endif	// PGP_BUSINESS_SECURITY

	if (sz[0] == 0) {
		PGPclOpenClientPrefs (memMgr, &prefref);
		PGPGetPrefStringBuffer (prefref, kPGPPrefComment, uLen, sz);
		PGPclCloseClientPrefs (prefref, FALSE);
	}
}

//	_______________________________________________
//
//  Get export format based on prefs

static PGPExportFormat 
sGetExportFormat (PGPMemoryMgrRef	memMgr)
{
	PGPBoolean	bCompatible		= TRUE;
	PGPPrefRef	prefref;

	PGPclOpenClientPrefs (memMgr, &prefref);
	PGPGetPrefBoolean (prefref, kPGPPrefExportKeysCompatible, &bCompatible);
	PGPclCloseClientPrefs (prefref, FALSE);

	if (bCompatible) 
		return kPGPExportFormat_Basic;
	else
		return kPGPExportFormat_Complete;
}

//	_______________________________________________
//
//  Export key(s) to text file

static BOOL 
sExportKeys (
		PKEYMAN			pKM, 
		PGPKeySetRef	keyset,
		LPSTR			szFile,
		PGPBoolean		bExportPrivateKeys,
		PGPBoolean		bCompatible) 
{
	BOOL			bRetVal				= FALSE;
	PGPFileSpecRef	FileRef;
	CHAR			szComment[256];
	PGPExportFormat	exportformat;

	PGPNewFileSpecFromFullPath (pKM->Context, szFile, &FileRef);
	if (FileRef) {	
		sGetCommentString (PGPGetContextMemoryMgr (pKM->Context), 
										szComment, sizeof(szComment));

		if (bCompatible)
			exportformat = kPGPExportFormat_Basic;
		else
			exportformat = kPGPExportFormat_Complete;

		if (IsntPGPError (PGPclErrorBox (NULL,
				PGPExportKeySet (keyset, 
								 PGPOOutputFile (pKM->Context, FileRef),
								 PGPOExportFormat (pKM->Context, 
												exportformat),
								 PGPOVersionString (pKM->Context, 
												pgpVersionHeaderString),
								 PGPOCommentString (pKM->Context, szComment),
								 PGPOExportPrivateKeys (pKM->Context,
												bExportPrivateKeys),
								 PGPOLastOption (pKM->Context))))) {
			bRetVal = TRUE;
		}
		PGPFreeFileSpec (FileRef);
	}

	return bRetVal;
}

//	___________________________
//
//  Compute name of export file

static VOID 
sGetExportFileName (
		PKEYMAN pKM, 
		LPSTR	sz, 
		UINT	uLen) 
{
	CHAR		szDefExt[8];
	PGPKeyRef	key;
	INT			i;

	if (!KMMultipleSelected (pKM)) {
		key = KMFocusedObject (pKM);
		PGPGetPrimaryUserIDNameBuffer (key, uLen, sz, &i);

		i = strcspn (sz, "\\/:*?""<>|");
		sz[i] = '\0';
		if (i > 0) {
			if (sz[i-1] == ' ') sz[i-1] = '\0';
		}
		if (lstrlen(sz) < 2) LoadString (g_hInst, IDS_DROPFILENAME, 
										 sz, uLen);
	}
	else LoadString (g_hInst, IDS_DROPFILENAME, sz, uLen);

	lstrcat (sz, ".");
	LoadString (g_hInst, IDS_DEFEXPORTEXTENSION, szDefExt, sizeof(szDefExt));
	lstrcat (sz, szDefExt);
}

//----------------------------------------------------|
// Export Private Key dialog message procedure

static BOOL CALLBACK 
sExportPrivKeyDlgProc (
		HWND	hWndDlg, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	OPENFILENAME*			pofn;
	PEXPORTPRIVKEYSTRUCT	pepks;

	switch (uMsg) {

	case WM_INITDIALOG :
		pofn = (OPENFILENAME*)lParam;
		SetWindowLong (hWndDlg, GWL_USERDATA, pofn->lCustData);
		pepks = (PEXPORTPRIVKEYSTRUCT)pofn->lCustData;
		if (!pepks->bExportPrivKeys) 
			EnableWindow (GetDlgItem (hWndDlg, IDC_EXPORTPRIVATEKEYS), FALSE);
		pepks->bExportPrivKeys = FALSE;
		if (!pepks->bCompatible)
			CheckDlgButton (hWndDlg, IDC_INCLUDEEXTENSIONS, BST_CHECKED);
		break;

	case WM_HELP :
		pepks = (PEXPORTPRIVKEYSTRUCT)GetWindowLong (hWndDlg, GWL_USERDATA);
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, pepks->pszHelpFile, 
            HELP_WM_HELP, (DWORD) (LPSTR) aExportKeyIds); 
		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDC_EXPORTPRIVATEKEYS :
			pepks = 
				(PEXPORTPRIVKEYSTRUCT)GetWindowLong (hWndDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hWndDlg, IDC_EXPORTPRIVATEKEYS) ==
															BST_CHECKED) {
				pepks->bExportPrivKeys = TRUE;
			}
			else {
				pepks->bExportPrivKeys = FALSE;
			}
			return TRUE;

		case IDC_INCLUDEEXTENSIONS :
			pepks = 
				(PEXPORTPRIVKEYSTRUCT)GetWindowLong (hWndDlg, GWL_USERDATA);
			if (IsDlgButtonChecked (hWndDlg, IDC_INCLUDEEXTENSIONS) ==
															BST_CHECKED) {
				pepks->bCompatible = FALSE;
			}
			else {
				pepks->bCompatible = TRUE;
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
	PGPKeySetRef		keysetExport		= kInvalidPGPKeySetRef;
	PGPError			err					= kPGPError_NoErr;

	OPENFILENAME		OpenFileName;
	EXPORTPRIVKEYSTRUCT	epks;
	CHAR				sz256[256];
	CHAR				szDefExt[8];
	CHAR				szFilter[256];
	CHAR				szTitle[64];
	CHAR*				p;
	BOOL				bRetVal;
	PGPPrefRef			prefref;

	KMGetSelectedKeys (pKM, &keysetExport, NULL);
	if (!PGPKeySetRefIsValid (keysetExport))
		return FALSE;

	if (szFile) {
		PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), 
							&prefref);
		PGPGetPrefBoolean (prefref, kPGPPrefExportKeysCompatible, 
							&epks.bCompatible);
		PGPclCloseClientPrefs (prefref, FALSE);

		bRetVal = sExportKeys (pKM, keysetExport, szFile, 
							FALSE, epks.bCompatible);
	}
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
		sGetExportFileName (pKM, sz256, sizeof (sz256));
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

		OpenFileName.lCustData         = (DWORD)&epks;
		OpenFileName.lpfnHook          = sExportPrivKeyDlgProc; 
		OpenFileName.lpTemplateName    = 
						MAKEINTRESOURCE (IDD_EXPORTDLGEXTENSIONS);

		epks.pszHelpFile = pKM->szHelpFile;
		epks.bExportPrivKeys = KMCheckForSecretKeys (keysetExport);
		PGPclOpenClientPrefs (PGPGetContextMemoryMgr (pKM->Context), 
							&prefref);
		PGPGetPrefBoolean (prefref, kPGPPrefExportKeysCompatible, 
							&epks.bCompatible);
		PGPclCloseClientPrefs (prefref, FALSE);

		if (GetSaveFileName (&OpenFileName)) {
			bRetVal = 
				sExportKeys (pKM, keysetExport, sz256, 
						epks.bExportPrivKeys, epks.bCompatible);
		}
	}

	if (PGPKeySetRefIsValid (keysetExport)) 
		PGPFreeKeySet (keysetExport);

	return TRUE;
}

//	_______________________________________________
//
//	called by OLE when DropTarget asks for data

LPSTR 
KMDropKeysGetData (
		PKEYMAN	pKM,
		BOOL	bForceComplete)
{
	PGPKeySetRef	keysetDrop		= kInvalidPGPKeySetRef;
	PGPError		err				= kPGPError_NoErr;
	DWORD			dwEffect		= 0;
	BOOL			bOKtoDelete		= FALSE;
	PGPSize			slen			= 0;
	LPSTR			pExport			= NULL;

	CHAR			szComment[256];
	HCURSOR			hcursorOld;
	PGPExportFormat exportformat;

	hcursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));

	err = KMGetSelectedKeys (pKM, &keysetDrop, NULL);
	if (!PGPKeySetRefIsValid (keysetDrop))
		return NULL;

	// get comment string
	slen = 0;
	sGetCommentString (PGPGetContextMemoryMgr (pKM->Context), 
										szComment, sizeof(szComment));

	// determine export format
	if (bForceComplete)
		exportformat = kPGPExportFormat_Complete;
	else
		exportformat = 
			sGetExportFormat (PGPGetContextMemoryMgr (pKM->Context));

	// export to buffer
	if (IsntPGPError (PGPclErrorBox (NULL,
		PGPExportKeySet (keysetDrop, 
						PGPOAllocatedOutputBuffer (pKM->Context,
								&pExport, 0x40000000, &slen),
						PGPOExportFormat (pKM->Context, exportformat),
						PGPOVersionString (pKM->Context, 
								pgpVersionHeaderString),
						PGPOCommentString (pKM->Context, szComment),
						PGPOLastOption (pKM->Context))))) {
		if (pExport) 
			pExport[slen-2] = '\0';
	}

	if (PGPKeySetRefIsValid (keysetDrop))
		PGPFreeKeySet (keysetDrop);

	SetCursor (hcursorOld);

	return pExport;		
}


//	_______________________________________________
//
//	called by OLE when DropTarget asks for data

VOID 
KMDropKeysFreeData (LPSTR psz) 
{
	if (IsntNull (psz))
		PGPFreeData (psz);
}


//	_______________________________________________
//
//  Drag selected key or keys out of application onto 
//  desktop or into Explorer.  Uses OLE interface.

BOOL 
KMDragAndDrop (PKEYMAN pKM) 
{
	LPDATAOBJECT	lpdo;
	LPDROPSOURCE	lpds;
	DWORD			dwEffect = 0;
	CHAR			szFile[MAX_PATH];
	BOOL			bOKtoDelete = FALSE;

	if (KMFocusedObjectType (pKM) != OBJECT_KEY) return FALSE;

	sGetExportFileName (pKM, szFile, sizeof(szFile));

	lpdo = KMCreateDataObject (pKM, szFile);
	lpds = KMCreateDropSource (pKM, pKM->hWndParent, pKM->hWndTree);

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
	PGPKeySetRef	keysetCopy			= kInvalidPGPKeySetRef;
	PGPError		err					= kPGPError_NoErr;
	BOOL			bRetVal				= FALSE;
	size_t			slen;
	HANDLE			hMem;
	LPSTR			pMem;
	LPSTR			pMemG;
	CHAR			szComment[256];
	PGPExportFormat	exportformat;

	err = KMGetSelectedKeys (pKM, &keysetCopy, NULL);
	if (!PGPKeySetRefIsValid (keysetCopy))
		return FALSE;

	slen = 0;
	sGetCommentString (PGPGetContextMemoryMgr (pKM->Context), 
										szComment, sizeof(szComment));
	exportformat = 
			sGetExportFormat (PGPGetContextMemoryMgr (pKM->Context));

	if (IsntPGPError (PGPclErrorBox (NULL,
		PGPExportKeySet (keysetCopy, 
						PGPOAllocatedOutputBuffer (pKM->Context,
								&pMem, 0x40000000, &slen),
						PGPOExportFormat (pKM->Context, exportformat),
						PGPOVersionString (pKM->Context, 
								pgpVersionHeaderString),
						PGPOCommentString (pKM->Context, szComment),
						PGPOLastOption (pKM->Context))))) {
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

	if (PGPKeySetRefIsValid (keysetCopy))
		PGPFreeKeySet (keysetCopy);

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
						if (PGPclQueryAddKeys (pKM->Context, 
									pKM->tlsContext,
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
					if (bDraggingOut) {
						if (IsntPGPError (PGPclErrorBox (NULL,
							PGPAddKeys (KeySet, pKM->KeySetDisp)))) {
							KMCommitKeyRingChanges (pKM);
							bImport = TRUE;
						}
					}
					else {
						if (PGPclQueryAddKeys (pKM->Context, 
								pKM->tlsContext,
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


