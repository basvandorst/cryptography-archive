/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	KMShare.c - code for splitting secret keys. 
	

	$Id: KMShare.c,v 1.47.2.1 1998/10/21 20:48:41 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpkmx.h"

// system header files
#include "shlobj.h"

// pgp header files
#include "pgpFileUtilities.h"
#include "pgpShareFile.h"

// constant definitions
#define INITIAL_SHAREKEY_COLUMNWIDTH	250
#define MAX_TOTAL_SHARES				255
#define MAX_SHARES						99
#define MAX_SHARES_LEN					2

// typedefs
typedef struct {
	INT		iNumSteps;
} SPLITKEYPROGRESSSTRUCT;

// external globals  
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];

// local global
static PSPLITKEYSTRUCT psksList = NULL;

static DWORD aSplitIds[] = {			// Help IDs
	IDC_KEYTOSPLIT,			IDH_PGPCLSPLIT_KEYTOSPLIT, 
	IDC_SHAREHOLDERS,		IDH_PGPCLSPLIT_SHAREHOLDERS, 
	IDC_CURRENTSHAREHOLDER,	IDH_PGPCLSPLIT_SELECTEDSHAREHOLDER, 
	IDC_SHARES,				IDH_PGPCLSPLIT_SELECTEDSHARES, 
	IDC_REMOVESHAREHOLDER,	IDH_PGPCLSPLIT_REMOVESHARES, 
	IDC_ADDSHAREHOLDER,		IDH_PGPCLSPLIT_ADDSHARES, 
	IDC_TOTALSHARES,		IDH_PGPCLSPLIT_TOTALSHARES, 
	IDC_THRESHOLD,			IDH_PGPCLSPLIT_THRESHOLD, 
	IDOK,					IDH_PGPCLSPLIT_SPLIT, 
    0,0 
}; 

static DWORD aNewShareIds[] = {			// Help IDs
    IDC_NEWSHAREHOLDER,	IDH_PGPCLSPLIT_NEWSHAREHOLDER, 
    0,0 
}; 

//	_______________________________________________
//
//  add shareholder to listview control

static BOOL 
sAddShareHolder (
		PSPLITKEYSTRUCT		psks,
		PSHAREHOLDERSTRUCT	pshs, 
		INT					iImage, 
		HWND				hwndList)
{
	LV_ITEM		lvI;
	INT			iItem;	
	CHAR		sz[16];

	// see if we're over the limit
	if ((psks->uTotalShares + pshs->uShares) > MAX_TOTAL_SHARES) {
		MessageBeep (MB_ICONASTERISK);
		return FALSE;
	}

	// figure item index to use
	iItem = ListView_GetItemCount (hwndList);

	// insert listview item
	lvI.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_STATE|LVIF_PARAM;
	lvI.state = 0;      
	lvI.stateMask = 0;
	lvI.iImage = iImage;

	lvI.iItem = iItem;
	lvI.iSubItem = 0;
	lvI.pszText	= pshs->szUserID; 
	lvI.cchTextMax = 0;
	lvI.lParam = (LPARAM)pshs;

	iItem = ListView_InsertItem (hwndList, &lvI);
	if (iItem == -1) {
		return FALSE;
	}
	else {
		// add strings for Shares column
		wsprintf (sz, "%i", pshs->uShares);
		ListView_SetItemText (hwndList, iItem, 1, sz);
		return TRUE;
	}
}


//	_______________________________________________
//
//  destroy shareholder data structures from listview control

static BOOL 
sIsKeyIDAlreadyInList (
		PGPKeyID*				pkeyid, 
		PGPPublicKeyAlgorithm	keyalg,
		PSPLITKEYSTRUCT			psks)
{
	INT					iNumItems;
	INT					iIndex;
	LV_ITEM				lvI;
	PSHAREHOLDERSTRUCT	pshs;

	iNumItems = ListView_GetItemCount (psks->hwndList);
	for (iIndex=0; iIndex<iNumItems; iIndex++) {
		lvI.mask = LVIF_PARAM;
		lvI.iItem = iIndex;
		lvI.iSubItem = 0;
		ListView_GetItem (psks->hwndList, &lvI);

		pshs = (PSHAREHOLDERSTRUCT)lvI.lParam;
		if (pshs->keyalg == keyalg) {
			if (PGPCompareKeyIDs (&pshs->keyid, pkeyid) == 0) {
				return TRUE;
			}
		}
	}

	return FALSE;
}


//	_______________________________________________
//
//  Drop text key(s)

BOOL 
KMSplitDropKeys (
		PSPLITKEYSTRUCT psks, 
		HANDLE	hMem) 
{
	PGPKeySetRef			keyset		= NULL;
	PGPKeyListRef			keylist		= NULL;
	PGPKeyIterRef			keyiter		= NULL;
	LPSTR					pMem		= NULL;

	PGPError				err;
	PGPKeyRef				key;
	PGPKeyID				keyid;
	BOOL					bKeys;
	PGPBoolean				bKeyIsUsable;
	size_t					sLen;
	PSHAREHOLDERSTRUCT		pshs;
	PGPPublicKeyAlgorithm	alg;

	bKeys = FALSE;

	if (hMem) {
		pMem = GlobalLock (hMem);
		if (pMem) {

			sLen = lstrlen (pMem);
			err = PGPImportKeySet (psks->pKM->Context, &keyset, 
							PGPOInputBuffer (psks->pKM->Context, pMem, sLen),
							PGPOLastOption (psks->pKM->Context));
			if (IsPGPError (err)) goto SplitDropCleanup;

			err = PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
			if (IsPGPError (err)) goto SplitDropCleanup;

			err = PGPNewKeyIter (keylist, &keyiter);
			if (IsPGPError (err)) goto SplitDropCleanup;

			PGPKeyIterNext (keyiter, &key);

			while (key) {
				bKeyIsUsable = FALSE;
				PGPGetKeyNumber (key, kPGPKeyPropAlgID, &alg);
				PGPGetKeyIDFromKey (key, &keyid);
				
				// key must either not be RSA or RSA ops must be enabled
				PGPGetKeyBoolean (key, kPGPKeyPropCanEncrypt, &bKeyIsUsable);

				// key must not be the same one that is being split
				if (alg == psks->keyalgToSplit) {
					if (PGPCompareKeyIDs (&keyid, &(psks->keyidToSplit))==0)
						bKeyIsUsable = FALSE;
				}

				// key must not already be in list
				if (sIsKeyIDAlreadyInList (&keyid, alg, psks))
					bKeyIsUsable = FALSE;

				if (bKeyIsUsable) {

					bKeys = TRUE;
					pshs = KMAlloc (sizeof(SHAREHOLDERSTRUCT));
					if (pshs) {
						PGPSize		size;

						pshs->bPublicKey = TRUE;
						pshs->pszPassphrase = NULL;
						pshs->uShares = 1;

						PGPGetPrimaryUserIDNameBuffer (key, 
							sizeof(pshs->szUserID), pshs->szUserID, &size);
						PGPGetKeyIDFromKey (key, &(pshs->keyid));
						PGPGetKeyNumber (key, 
							kPGPKeyPropAlgID, &(pshs->keyalg));

						if (sAddShareHolder (psks, pshs, 
								KMDetermineUserIDIcon (key, NULL, NULL), 
								psks->hwndList)) {
							psks->uTotalShares += pshs->uShares;
							SetDlgItemInt (psks->hwndDlg, IDC_TOTALSHARES, 
									psks->uTotalShares, FALSE);
							SendMessage (psks->hwndDlg, WM_COMMAND, 
									MAKEWPARAM(IDC_THRESHOLD, EN_CHANGE), 0);
						}
						else {
							KMFree (pshs);
						}
					}
				}

				PGPKeyIterNext (keyiter, &key);
			}

SplitDropCleanup :
			if (keyiter)
				PGPFreeKeyIter (keyiter);

			if (keylist)
				PGPFreeKeyList (keylist);

			if (keyset)
				PGPFreeKeySet (keyset);

			if (pMem)
				GlobalUnlock (hMem);
		}
	}
	
	return bKeys;
}


//	_______________________________________________
//
//  Initialize ListView

static VOID 
sInitKeyList (PSPLITKEYSTRUCT psks) 
{

	LV_COLUMN	lvC; 
	CHAR		sz[256];
	HBITMAP		hBmp;
	HDC			hDC;
	INT			iNumBits;

	// create image list
	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		psks->hIml = ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (psks->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		psks->hIml = ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
										NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (psks->hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}

	ListView_SetImageList (psks->hwndList, psks->hIml, LVSIL_SMALL);

	lvC.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT; 
	lvC.pszText = sz;

	LoadString (g_hInst, IDS_USERID, sz, sizeof(sz));
	lvC.cx = INITIAL_SHAREKEY_COLUMNWIDTH;   
	lvC.iSubItem = 0;
	if (ListView_InsertColumn (psks->hwndList, 0, &lvC) == -1) return;

	LoadString (g_hInst, IDS_SHARES, sz, sizeof(sz));
	lvC.cx = 50;   
	lvC.iSubItem = 1;
	if (ListView_InsertColumn (psks->hwndList, 1, &lvC) == -1) return;
}

//	_______________________________________________
//
//  remove shareholder from listview control

static VOID 
sRemoveShareHolderFromList (HWND hDlg, PSPLITKEYSTRUCT psks)
{
	INT					iIndex;
	LV_ITEM				lvI;
	PSHAREHOLDERSTRUCT	pshs;

	iIndex = ListView_GetNextItem (psks->hwndList, -1, LVNI_SELECTED);

	if (iIndex > -1) {
		lvI.mask = LVIF_PARAM;
		lvI.iItem = iIndex;
		lvI.iSubItem = 0;
		ListView_GetItem (psks->hwndList, &lvI);

		// update total shares
		pshs = (PSHAREHOLDERSTRUCT)lvI.lParam;
		psks->uTotalShares -= pshs->uShares;
		SetDlgItemInt (hDlg, IDC_TOTALSHARES, psks->uTotalShares, FALSE);
		SendMessage (hDlg, WM_COMMAND, 
						MAKEWPARAM (IDC_THRESHOLD, EN_CHANGE), 0);

		KMFreePhrase (pshs->pszPassphrase);
		KMFree (pshs);

		ListView_DeleteItem (psks->hwndList, iIndex);
	}
}


//	_______________________________________________
//
// add share holder dialog message procedure

static BOOL CALLBACK 
sAddShareHolderDlgProc (HWND hDlg, 
					   UINT uMsg, 
					   WPARAM wParam, 
					   LPARAM lParam) 
{
	PSPLITKEYSTRUCT		psks;
	PSHAREHOLDERSTRUCT	pshs;

	switch (uMsg) {

	case WM_INITDIALOG :
		// save address of struct
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		return TRUE;

	case WM_HELP: 
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aNewShareIds); 
	    break; 

	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
		    (DWORD) (LPVOID) aNewShareIds); 
		break; 

	case WM_COMMAND :
		switch (LOWORD(wParam)) {
		case IDCANCEL :
			EndDialog (hDlg, 0);
			break;

		case IDOK :
			psks = (PSPLITKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			pshs = KMAlloc (sizeof(SHAREHOLDERSTRUCT));
			if (pshs) {
				CHAR		szPrompt[64];
				PGPError	err;

				pshs->bPublicKey = FALSE;
				pshs->keyalg = 0;
				pshs->pszPassphrase = NULL;
				pshs->uShares = 1;
				GetDlgItemText (hDlg, IDC_NEWSHAREHOLDER, 
								pshs->szUserID, sizeof(pshs->szUserID));

				LoadString (g_hInst, IDS_SHAREPHRASEPROMPT, 
										szPrompt, sizeof (szPrompt));
				err = KMGetConfirmationPhrase (psks->pKM->Context, hDlg, 
								szPrompt, psks->pKM->KeySetMain,
								1, 0, &(pshs->pszPassphrase));
				// wjb changed min length to 1 for passphrase

				if (IsntPGPError (err)) {
					if (sAddShareHolder (
							psks, pshs, IDX_HEAD, psks->hwndList)) {
						psks->uTotalShares += pshs->uShares;
						EndDialog (hDlg, 1);
					}
					else {
						KMFreePhrase (pshs->pszPassphrase);
						KMFree (pshs);
						EndDialog (hDlg, 0);
					}
				}
			}
			else 
				EndDialog (hDlg, 0);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//	_______________________________________________
//
//  pop dialog for adding conventional encryption shareholder

static VOID 
sAddShareHolderToList (HWND hDlg, PSPLITKEYSTRUCT psks)
{
	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_ADDSHAREHOLDER), 
		hDlg, sAddShareHolderDlgProc, (LPARAM)psks)) {

		SetDlgItemInt (hDlg, IDC_TOTALSHARES, psks->uTotalShares, FALSE);
		SendMessage (hDlg, WM_COMMAND, 
						MAKEWPARAM (IDC_THRESHOLD, EN_CHANGE), 0);
	}
}


//	_______________________________________________
//
//  destroy shareholder data structures from listview control

static VOID 
sDestroyShareHolders (PSPLITKEYSTRUCT psks)
{
	INT					iNumItems;
	INT					iIndex;
	LV_ITEM				lvI;
	PSHAREHOLDERSTRUCT	pshs;

	iNumItems = ListView_GetItemCount (psks->hwndList);
	for (iIndex=0; iIndex<iNumItems; iIndex++) {
		lvI.mask = LVIF_PARAM;
		lvI.iItem = iIndex;
		lvI.iSubItem = 0;
		ListView_GetItem (psks->hwndList, &lvI);

		pshs = (PSHAREHOLDERSTRUCT)lvI.lParam;
		KMFreePhrase (pshs->pszPassphrase);

		KMFree (pshs);
	}
}


//	_______________________________________________
//
//  destroy dialog struct and remove from list

static VOID 
sDestroySplitKeyStruct (PSPLITKEYSTRUCT psksToDestroy)
{
	PSPLITKEYSTRUCT* ppsks;

	ppsks = psksToDestroy->pHeadOfList;

	while (*ppsks) {
		if (*ppsks == psksToDestroy) {
			*ppsks = psksToDestroy->next;
			KMFree (psksToDestroy);
			return;
		}
		ppsks = &((*ppsks)->next);
	}
}


//	_______________________________________________
//
//  Compute name of share file

static PGPError
sCreateFilePathFromUserName (
		LPSTR	pszFolder,
		LPSTR	pszUserID,
		UINT	uNumShares,
		LPSTR	pszModifier,
		LPSTR	pszPath, 
		INT		iLen) 
{
	CHAR		sz[kPGPMaxUserIDSize];
	CHAR		szDefName[16];
	CHAR		szDefExt[8];
	CHAR		szShares[16];
	INT			iMinRequiredLen;
	INT			i;

	// prepare number of shares substring
	if (uNumShares == 1) {
		LoadString (g_hInst, IDS_ONESHARE, szShares, sizeof(szShares));
	}
	else {
		LoadString (g_hInst, IDS_NUMSHARES, sz, sizeof(sz));
		wsprintf (szShares, sz, uNumShares);
	}

	// get default file name and extension
	LoadString (g_hInst, IDS_DEFSHARENAME, szDefName, sizeof(szDefName));
	lstrcat (szDefName, pszModifier);
	LoadString (g_hInst, IDS_DEFSHAREEXTENSION, szDefExt, sizeof(szDefExt));

	// check length of destination buffer
	iMinRequiredLen = 
		lstrlen (pszFolder) + lstrlen (szDefExt) + lstrlen (szDefName) +1;
	if (iMinRequiredLen >= iLen) 
		return kPGPError_CantOpenFile;

	// put folder into path
	lstrcpy (pszPath, pszFolder);
	iLen -= lstrlen (pszPath);
	if (pszPath[lstrlen(pszPath)-1] != '\\') {
		lstrcat (pszPath, "\\");
		iLen -= 1;
	}

	// look for invalid characters and truncate
	lstrcpy (sz, pszUserID);
	i = strcspn (sz, "\\/:*?""<>|");
	sz[i] = '\0';

	// remove trailing spaces
	while ((i > 0) && (sz[i-1] == ' ')) {
		i--;
		sz[i] = '\0';
	}

	// check if we've truncated too much
	if (lstrlen (sz) < 2) 
		lstrcpy (sz, szDefName);

	// check if name is too long
	iLen -= (lstrlen (szDefExt) +1);
	if ((lstrlen(sz) + lstrlen(szShares) + lstrlen(pszModifier)) >= iLen) {
		if ((lstrlen (sz) + lstrlen (pszModifier)) >= iLen) {
			if (lstrlen (sz) >= iLen) {
				sz[iLen-1] = '\0';
			}
			lstrcat (pszPath, sz);
			lstrcat (pszPath, ".");
			lstrcat (pszPath, szDefExt);

		}
		else {
			lstrcat (pszPath, sz);
			lstrcat (pszPath, pszModifier);
			lstrcat (pszPath, ".");
			lstrcat (pszPath, szDefExt);
		}
	}
	else {
		// construct full path
		lstrcat (pszPath, sz);
		lstrcat (pszPath, szShares);
		lstrcat (pszPath, pszModifier);
		lstrcat (pszPath, ".");
		lstrcat (pszPath, szDefExt);
	}

	return kPGPError_NoErr;
}


//	_______________________________________________
//
//  split the key 

static PGPError 
sSaveSharesToFile (
		PSHAREHOLDERSTRUCT	pshs, 
		PGPContextRef		context,
		PGPShareRef			sharesTotal,
		PGPKeySetRef		keyset,
		LPSTR				pszFolder)
{
	PFLFileSpecRef		filespec		= NULL;
	PGPShareFileRef		sharefile		= NULL;
	PGPShareRef			sharesHolder	= NULL;
	PGPOptionListRef	encodeOptions	= NULL;
	PGPError			err				= kPGPError_NoErr;
	INT					iModifier		= 0;

	CHAR				szPath[MAX_PATH];
	CHAR				szModifier[MAX_SHARES_LEN+1];
	CHAR				sz1[32];
	CHAR				sz2[kPGPMaxUserIDSize + 32];
	PGPKeyRef			key;

	// create file name and filespec
	err = sCreateFilePathFromUserName (pszFolder, pshs->szUserID, 
						pshs->uShares, NULL, szPath, sizeof(szPath));
	if (IsPGPError (err)) goto SaveFileCleanup;

	// check for pre-existence of file
	while (GetFileAttributes (szPath) != 0xFFFFFFFF) {
		iModifier++;
		if (iModifier > MAX_SHARES) {
			err = kPGPError_CantOpenFile;
			goto SaveFileCleanup;
		}
		wsprintf (szModifier, " %i", iModifier);
		err = sCreateFilePathFromUserName (pszFolder, pshs->szUserID,
						pshs->uShares, szModifier, szPath, sizeof(szPath));
		if (IsPGPError (err)) goto SaveFileCleanup;
	}	

	err = PFLNewFileSpecFromFullPath (PGPGetContextMemoryMgr (context), 
		szPath, &filespec);
	if (IsPGPError (err)) goto SaveFileCleanup;
	
	err = PFLFileSpecCreate (filespec);
	if (IsPGPError (err)) goto SaveFileCleanup;

	err = PGPNewShareFile (filespec, &sharefile);
	if (IsPGPError (err)) goto SaveFileCleanup;

	err = PGPSetShareFileUserID (sharefile, pshs->szUserID);
	if (IsPGPError (err)) goto SaveFileCleanup;

	err = PGPSplitShares (sharesTotal, pshs->uShares, &sharesHolder);
	if (IsPGPError (err)) goto SaveFileCleanup;

	// if this shareholder has public key, use it
	if (pshs->bPublicKey) {
		err = PGPSetShareFileOwnerKeyID (sharefile, pshs->keyid);
		if (IsPGPError (err)) goto SaveFileCleanup;

		err = PGPGetKeyByKeyID (keyset, &(pshs->keyid), pshs->keyalg, &key);
		if (IsPGPError (err)) {
			LoadString (g_hInst, IDS_CAPTIONERROR, sz1, sizeof(sz1));
			LoadString (g_hInst, IDS_SHAREKEYGONE, sz2, sizeof(sz2));
			lstrcat (sz2, pshs->szUserID);
			MessageBox (NULL, sz2, sz1, MB_OK|MB_ICONERROR);
			err = kPGPError_UserAbort;
			goto SaveFileCleanup;
		}

		err = PGPBuildOptionList (context, &encodeOptions,
			PGPOEncryptToKey (context, key),
			PGPOLastOption (context));
		if (IsPGPError (err)) goto SaveFileCleanup;
	}

	// there is no public key for this shareholder
	else {
		err = PGPBuildOptionList (context, &encodeOptions,
			PGPOConventionalEncrypt (context,
				PGPOPassphrase (context, pshs->pszPassphrase),
				PGPOLastOption (context)),
			PGPOLastOption (context));
		if (IsPGPError (err)) goto SaveFileCleanup;
	}

	err = PGPCopySharesToFile (context, sharefile, 
									encodeOptions, sharesHolder);
	if (IsPGPError (err)) goto SaveFileCleanup;

	err = PGPSaveShareFile (sharefile);

SaveFileCleanup:

	if (encodeOptions != NULL)
		PGPFreeOptionList (encodeOptions);

	if (sharesHolder != NULL)
		PGPFreeShares (sharesHolder);

	if (sharefile != NULL)
		PGPFreeShareFile (sharefile);

	if (filespec != NULL)
		PFLFreeFileSpec (filespec);

	return err;
}


//	_______________________________________________
//
//	Split key progress dialog message procedure

static BOOL CALLBACK 
sSplitKeyProgressDlgProc (HWND hDlg, 
					   UINT uMsg, 
					   WPARAM wParam, 
					   LPARAM lParam) 
{
	SPLITKEYPROGRESSSTRUCT*	pskps;

	switch (uMsg) {

	case WM_INITDIALOG :
		// save address of struct
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pskps = (SPLITKEYPROGRESSSTRUCT*)lParam;
		SendDlgItemMessage (hDlg, IDC_PROGRESSBAR, PBM_SETRANGE,
							0, MAKELPARAM(0, pskps->iNumSteps));
		SendDlgItemMessage (hDlg, IDC_PROGRESSBAR, PBM_SETPOS,
							0, 0);
		return TRUE;

	case WM_APP :
		SendDlgItemMessage (hDlg, IDC_PROGRESSBAR, PBM_SETPOS,
							wParam, 0);
		SetDlgItemText (hDlg, IDC_PROGRESSTEXT, (LPSTR)lParam);
		break;

	}
	return FALSE;
}


//	___________________________________________________
//
//  Change passphrase of key and all subkeys

static PGPError 
sChangeKeyPhrase (
		PGPContextRef	context,
		PGPKeySetRef	keyset,
		PGPKeyRef		key, 
		LPSTR			szOld, 
		PGPByte*		pPasskeyOld,
		PGPSize			sizePasskeyOld,
		PGPByte*		pPasskey,
		PGPSize			sizePasskey) 
{
	UINT			u;
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPSubKeyRef	subkey;
	PGPError		err;

	if (szOld) {
		err = PGPChangePassphrase (key, 
				PGPOPassphrase (context, szOld), 
				PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
				PGPOLastOption (context));
	}
	else if (sizePasskeyOld > 0) {
		err = PGPChangePassphrase (key, 
				PGPOPasskeyBuffer (context, pPasskeyOld, sizePasskeyOld), 
				PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
				PGPOLastOption (context));
	}
	else {
		err = PGPChangePassphrase (key, 
				PGPOPassphrase (context, ""), 
				PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
				PGPOLastOption (context));
	}
	if (IsPGPError (err)) return err;

	PGPGetKeyNumber (key, kPGPKeyPropAlgID, &u);
	switch (u) {
	case kPGPPublicKeyAlgorithm_RSA :
		break;

	case kPGPPublicKeyAlgorithm_DSA :
		PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
		PGPNewKeyIter (keylist, &keyiter);
		PGPKeyIterSeek (keyiter, key);
		PGPKeyIterNextSubKey (keyiter, &subkey);
		while (subkey) {
			if (szOld) {
				err = PGPChangeSubKeyPassphrase (subkey, 
						PGPOPassphrase (context, szOld),
						PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
						PGPOLastOption (context));
			}
			else if (sizePasskeyOld > 0) {
				err = PGPChangeSubKeyPassphrase (subkey, 
						PGPOPasskeyBuffer (context, 
											pPasskeyOld, sizePasskeyOld),
						PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
						PGPOLastOption (context));
			}
			else {
				err = PGPChangeSubKeyPassphrase (subkey, 
						PGPOPassphrase (context, ""),
						PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
						PGPOLastOption (context));
			}
			PGPKeyIterNextSubKey (keyiter, &subkey);
		}
		PGPFreeKeyIter (keyiter);
		PGPFreeKeyList (keylist);
		break;

	default :
		break;
	}

	return err;
}

//	_______________________________________________
//
//  split the key 

static BOOL 
sSplitKey (PSPLITKEYSTRUCT psks)
{
	PGPShareRef			shares				= NULL;
	PGPSize				sizePasskey			= 0;
	PGPByte*			pPasskey			= NULL;
	LPSTR				pszPhraseKeyToSplit	= NULL;
	PGPByte*			pPasskeyToSplit		= NULL;
	PGPSize				sizePasskeyToSplit	= 0;
	CHAR				szEmptyString[]		= {""};
	HWND				hwndProgress		= NULL;
	BOOL				bRetVal				= TRUE;

	PGPError				err;
	PGPKeyRef				keyToSplit;
	BROWSEINFO				bi;
	LPITEMIDLIST			pidl;
	LPMALLOC				pMalloc;
	CHAR					szFolder[MAX_PATH];
	CHAR					sz[kPGPMaxUserIDSize + 32];
	INT						iItem;
	INT						iNumItems;
	LV_ITEM					lvI;
	PSHAREHOLDERSTRUCT		pshs;
	HCURSOR					hcursorOld;
	SPLITKEYPROGRESSSTRUCT	skps;

	// get keyref from keyring
	err = PGPGetKeyByKeyID (psks->pKM->KeySetMain, &(psks->keyidToSplit),
							psks->keyalgToSplit, &keyToSplit);
	if (IsPGPError (err)) {
		KMMessageBox (psks->hwndDlg, IDS_CAPTIONERROR, IDS_SPLITKEYGONE, 
						MB_OK|MB_ICONERROR);
		bRetVal = FALSE;
		goto SplitKeyCleanup;
	}

	// get task allocator
	if (SHGetMalloc(&pMalloc) != NOERROR) return FALSE;

	// get prompt string
	LoadString (g_hInst, IDS_SAVESHARES, sz, sizeof(sz));

	// initialize structure
	bi.hwndOwner = psks->hwndDlg;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szFolder;
	bi.lpszTitle = sz;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;

	// allow user to browse
	pidl = SHBrowseForFolder (&bi);
	if (pidl == NULL) return FALSE;
	SHGetPathFromIDList (pidl, szFolder);
	pMalloc->lpVtbl->Free(pMalloc, pidl);

	// get passphrase of key to split
	LoadString (g_hInst, IDS_SPLITKEYPHRASEPROMPT, sz, sizeof(sz));
	err = KMGetKeyPhrase (psks->pKM->Context, psks->pKM->tlsContext,
			psks->hwndDlg, sz, 
			psks->pKM->KeySetMain, keyToSplit,
			&pszPhraseKeyToSplit, &pPasskeyToSplit, 
			&sizePasskeyToSplit);
	PGPclErrorBox (NULL, err);

	if (IsPGPError (err)) {
		bRetVal = FALSE;
		goto SplitKeyCleanup;
	}

	// make sure that this is what user wants to do
	if (KMMessageBox (psks->hwndDlg, IDS_CAPTION, IDS_SPLITKEYCONFIRMATION,
						MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
		bRetVal = FALSE;
		goto SplitKeyCleanup;
	}

	// post progress dialog
	iNumItems = ListView_GetItemCount (psks->hwndList);
	skps.iNumSteps = iNumItems +2;
	hwndProgress = CreateDialogParam (g_hInst, 
						MAKEINTRESOURCE(IDD_SPLITKEYPROGRESS),
						psks->hwndDlg, sSplitKeyProgressDlgProc,
						(LPARAM)&skps);
	LoadString (g_hInst, IDS_CREATINGSHARES, sz, sizeof(sz));
	SendMessage (hwndProgress, WM_APP, 1, (LPARAM)sz);

	// create the shares
	err = PGPCreateShares (	psks->pKM->Context, 
							keyToSplit, 
							psks->uThreshold, 
							psks->uTotalShares, 
							&shares);

	if (IsPGPError (PGPclErrorBox (NULL, err))) {
		bRetVal = FALSE;
		goto SplitKeyCleanup;
	}

	// get the passkey from the shares
	err = PGPGetPasskeyFromShares (shares, &pPasskey, &sizePasskey);
	if (IsntPGPError (PGPclErrorBox (NULL, err))) {

		hcursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));

		// save share file for each item in listview
		for (iItem=0; iItem<iNumItems; iItem++) {
			lvI.mask = LVIF_PARAM;
			lvI.iItem = iItem;
			lvI.iSubItem = 0;
			ListView_GetItem(psks->hwndList, &lvI);

			// update progress dialog
			pshs = (PSHAREHOLDERSTRUCT)lvI.lParam;
			LoadString (g_hInst, IDS_SAVINGSHARES, sz, sizeof(sz));
			lstrcat (sz, pshs->szUserID);
			SendMessage (hwndProgress, WM_APP, iItem+2, (LPARAM)sz);

			err = sSaveSharesToFile (pshs, psks->pKM->Context, shares,
										psks->pKM->KeySetMain, szFolder);
			if (IsPGPError (err)) break;
		}

		// update progress dialog
		LoadString (g_hInst, IDS_SPLITTINGKEY, sz, sizeof(sz));
		SendMessage (hwndProgress, WM_APP, iNumItems+2, (LPARAM)sz);

		SetCursor (hcursorOld);
		PGPclErrorBox (NULL, err);

		if (IsPGPError (err)) {
///			delete files;
		}
		else {
			err = sChangeKeyPhrase (psks->pKM->Context, 
					psks->pKM->KeySetMain, keyToSplit, 
					pszPhraseKeyToSplit, pPasskeyToSplit, sizePasskeyToSplit,
					pPasskey, sizePasskey);
			PGPclErrorBox (NULL, err);
			KMCommitKeyRingChanges (psks->pKM); 
			KMUpdateKeyInTree (psks->pKM, keyToSplit, FALSE);
			KMSelectKey (psks->pKM, keyToSplit, TRUE);
		}
	}

	// cleanup
SplitKeyCleanup :
	if (hwndProgress)
		DestroyWindow (hwndProgress);

	if (pszPhraseKeyToSplit) 
		KMFreePhrase (pszPhraseKeyToSplit);

	if (pPasskeyToSplit)
		KMFreePasskey (pPasskeyToSplit, sizePasskeyToSplit);

	if (pPasskey)
		PGPFreeData (pPasskey);

	if (shares != NULL)
		PGPFreeShares (shares);

	return (bRetVal);
}


//	_______________________________________________
//
//	Split key dialog message procedure

static BOOL CALLBACK 
sSplitKeyDlgProc (HWND hDlg, 
				 UINT uMsg, 
				 WPARAM wParam, 
				 LPARAM lParam) 
{
	PSPLITKEYSTRUCT		psks;
	CHAR				sz[8];
	NMHDR*				pnmh;
	UINT				u;

	switch (uMsg) {

	case WM_INITDIALOG :
	{
		CHAR	szTitle[kPGPMaxUserIDSize + 32];

		// save address of struct
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		psks = (PSPLITKEYSTRUCT)lParam;
		psks->hwndDlg = hDlg;

		// if we have a function to call to add hwnd to list, then call it
		if (psks->pKM->lpfnHwndListFunc) 
			(psks->pKM->lpfnHwndListFunc)(hDlg, TRUE, NULL, NULL);

		// initialize shareholder list
		psks->hwndList = GetDlgItem (hDlg, IDC_SHAREHOLDERS);
		sInitKeyList (psks);

		// initialize name of key to split
		SetDlgItemText (hDlg, IDC_KEYTOSPLIT, psks->szUserIDToSplit);
		LoadString (g_hInst, IDS_SPLITKEYTITLE, szTitle, sizeof(szTitle));
		lstrcat (szTitle, psks->szUserIDToSplit);
		SetWindowText (hDlg, szTitle);

		// limit number of shares
		SendDlgItemMessage (hDlg, IDC_SHARES, EM_SETLIMITTEXT, 
				(WPARAM)MAX_SHARES_LEN, 0);
		SendDlgItemMessage (hDlg, IDC_THRESHOLD, EM_SETLIMITTEXT, 
				(WPARAM)MAX_SHARES_LEN, 0);

		// initialize spin controls 
		SendDlgItemMessage (hDlg, IDC_SHARESSPIN, UDM_SETRANGE,
				0, (LPARAM)MAKELONG (MAX_SHARES, 1));
		SetDlgItemText (hDlg, IDC_SHARES, "");
		EnableWindow (GetDlgItem (hDlg, IDC_SHARES), FALSE);

		SendDlgItemMessage (hDlg, IDC_THRESHOLDSPIN, UDM_SETRANGE,
				0, (LPARAM)MAKELONG (MAX_SHARES, 1));
		SendDlgItemMessage (hDlg, IDC_THRESHOLDSPIN, UDM_SETPOS,
				0, (LPARAM)MAKELONG (psks->uThreshold, 0));

		// initialize total number of shares
		SetDlgItemInt (hDlg, IDC_TOTALSHARES, psks->uTotalShares, FALSE);

		// "split key" and "remove" buttons initially disabled
		EnableWindow (GetDlgItem (hDlg, IDC_REMOVESHAREHOLDER), FALSE);
		EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);

		// initialize drag/drop
		psks->pDropTarget = KMCreateDropTarget (psks->hwndList, NULL, psks); 
		CoLockObjectExternal ((IUnknown*)psks->pDropTarget, TRUE, TRUE);
		RegisterDragDrop (psks->hwndList, psks->pDropTarget);
		KMEnableDropTarget (psks->pDropTarget, TRUE);

		return TRUE;
	}

	case WM_HELP: 
	{
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aSplitIds); 
	    break; 
	}

	case WM_CONTEXTMENU: 
	{
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
		    (DWORD) (LPVOID) aSplitIds); 
		break; 
	}

	case WM_NOTIFY :
		pnmh = (NMHDR*)lParam;
		switch (pnmh->code) {

		case NM_CLICK :
		case LVN_KEYDOWN :
			if (pnmh->idFrom == IDC_SHAREHOLDERS) {
				INT		iIndex;

				psks = (PSPLITKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
				iIndex = 
					ListView_GetNextItem (psks->hwndList, -1, LVNI_SELECTED);
				if (iIndex > -1) {
					LV_ITEM				lvI;
					PSHAREHOLDERSTRUCT	pshs;

					lvI.mask = LVIF_PARAM;
					lvI.iItem = iIndex;
					lvI.iSubItem = 0;
					ListView_GetItem(psks->hwndList, &lvI);

					pshs = (PSHAREHOLDERSTRUCT)lvI.lParam;
					psks->pshsCurrent = pshs;
					psks->iIndexCurrent = iIndex;

					SetDlgItemText (hDlg, IDC_CURRENTSHAREHOLDER, 
														pshs->szUserID);
					SendDlgItemMessage (hDlg, IDC_SHARESSPIN, UDM_SETPOS,
								0, (LPARAM)MAKELONG (pshs->uShares, 0));

					EnableWindow (GetDlgItem (hDlg, IDC_SHARES), TRUE);
					EnableWindow (GetDlgItem (hDlg, IDC_SHARESSPIN), TRUE);
					EnableWindow (
						GetDlgItem (hDlg, IDC_REMOVESHAREHOLDER), TRUE);
				}
				else {
					psks->iIndexCurrent = -1;
					psks->pshsCurrent = NULL;
					SetDlgItemText (hDlg, IDC_SHARES, "");
					SetDlgItemText (hDlg, IDC_CURRENTSHAREHOLDER, "");
					EnableWindow (GetDlgItem (hDlg, IDC_SHARES), FALSE);
					EnableWindow (
						GetDlgItem (hDlg, IDC_REMOVESHAREHOLDER), FALSE);
				}
			}
			break;
		}
		break;

	case WM_DESTROY :
		psks = (PSPLITKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

		// terminate drag/drop
		RevokeDragDrop (psks->hwndList);
		KMReleaseDropTarget (psks->pDropTarget);  
		CoLockObjectExternal ((IUnknown*)psks->pDropTarget, FALSE, TRUE);

		// call function to remove hwnd from list
		if (psks->pKM->lpfnHwndListFunc) 
			(psks->pKM->lpfnHwndListFunc)(hDlg, FALSE, NULL, NULL);

		// destroy data objects
		ImageList_Destroy (psks->hIml);
		sDestroyShareHolders (psks);
		sDestroySplitKeyStruct (psks);
		break;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {
		case IDCANCEL :
			DestroyWindow (hDlg);
			break;

		case IDOK :
			psks = (PSPLITKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			EnableWindow (psks->pKM->hWndParent, FALSE);
			if (sSplitKey (psks)) 
			{
				EnableWindow (psks->pKM->hWndParent, TRUE);
				DestroyWindow (hDlg);
			}
			else
				EnableWindow (psks->pKM->hWndParent, TRUE);
			break;

		case IDHELP :
			psks = (PSPLITKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			WinHelp (hDlg, psks->pKM->szHelpFile, HELP_CONTEXT, 
						IDH_PGPCLSPLIT_SPLITDIALOG); 
			break;

		case IDC_ADDSHAREHOLDER :
			psks = (PSPLITKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			EnableWindow (psks->pKM->hWndParent, FALSE);
			sAddShareHolderToList (hDlg, psks);
			EnableWindow (psks->pKM->hWndParent, TRUE);
			SetFocus (GetDlgItem (hDlg, IDC_ADDSHAREHOLDER));
			break;

		case IDC_REMOVESHAREHOLDER :
			psks = (PSPLITKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			sRemoveShareHolderFromList (hDlg, psks);
			psks->iIndexCurrent = -1;
			psks->pshsCurrent = NULL;
			SetDlgItemText (hDlg, IDC_SHARES, "");
			SetDlgItemText (hDlg, IDC_CURRENTSHAREHOLDER, "");
			EnableWindow (GetDlgItem (hDlg, IDC_SHARES), FALSE);
			EnableWindow (GetDlgItem (hDlg, IDC_REMOVESHAREHOLDER), FALSE);
			break;

		case IDC_THRESHOLD :
			switch (HIWORD(wParam)) {
			case EN_CHANGE :
				psks = (PSPLITKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
				if (psks) {
					GetDlgItemText (hDlg, IDC_THRESHOLD, sz, sizeof(sz));
					if (lstrcmp (sz, "0")) {
						UINT u = 
							GetDlgItemInt (hDlg, IDC_THRESHOLD, NULL, FALSE);
						if (u != 0) {
							psks->uThreshold = u;
							if (psks->uTotalShares >= psks->uThreshold)
								EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
							else
								EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);
						}
					}
				}
				break;

			case EN_KILLFOCUS :
				u = GetDlgItemInt (hDlg, IDC_THRESHOLD, NULL, FALSE);
				if (u == 0) u = 1;
				SetDlgItemInt (hDlg, IDC_THRESHOLD, u, FALSE);
				break;
			}
			break;

		case IDC_SHARES :
			psks = (PSPLITKEYSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			switch (HIWORD(wParam)) {
			case EN_CHANGE :
				if (psks) {
					PSHAREHOLDERSTRUCT	pshs;
					GetDlgItemText (hDlg, IDC_SHARES, sz, sizeof(sz));
					pshs = psks->pshsCurrent;
					if (pshs && (lstrcmp (sz, "0"))) {
						UINT u = 
							GetDlgItemInt (hDlg, IDC_SHARES, NULL, FALSE);
						if (u != 0) {
							psks->uTotalShares -= pshs->uShares;
							pshs->uShares = u;
							psks->uTotalShares += pshs->uShares;
							SetDlgItemInt (hDlg, IDC_TOTALSHARES, 
											psks->uTotalShares, FALSE);

							wsprintf (sz, "%i", pshs->uShares);
							ListView_SetItemText (psks->hwndList, 
										psks->iIndexCurrent, 1, sz);

							if ((psks->uTotalShares >= psks->uThreshold) &&
								(psks->uTotalShares <= MAX_TOTAL_SHARES))
								EnableWindow (GetDlgItem (hDlg, IDOK), TRUE);
							else
								EnableWindow (GetDlgItem (hDlg, IDOK), FALSE);
						}
					}
				}
				break;

			case EN_KILLFOCUS :
				u = GetDlgItemInt (hDlg, IDC_SHARES, NULL, FALSE);
				if (u == 0) {
					u = 1;
					MessageBeep (MB_ICONASTERISK);
				}
				else if (psks->uTotalShares-psks->pshsCurrent->uShares+u > 
														MAX_TOTAL_SHARES) {
					u = MAX_TOTAL_SHARES - (psks->uTotalShares - 
												psks->pshsCurrent->uShares);
					MessageBeep (MB_ICONASTERISK);
				}

				SetDlgItemInt (hDlg, IDC_SHARES, u, FALSE);
				break;
			}
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//	_______________________________________________
//
//  Split selected key

BOOL KMSplitKey (PKEYMAN pKM, PGPKeyRef key) 
{
	PSPLITKEYSTRUCT			psks;
	BOOL					bOK;
	PGPKeyID				keyidToSplit;
	PGPPublicKeyAlgorithm	keyalgToSplit;
	HWND					hwnd;

	// find existing dialog struct for this key
	PGPGetKeyIDFromKey (key, &keyidToSplit);
	PGPGetKeyNumber (key, kPGPKeyPropAlgID, &keyalgToSplit);
	psks = pKM->pSplitKeyDialogList;
	while (psks) {
		if (PGPCompareKeyIDs (&keyidToSplit, &(psks->keyidToSplit)) == 0) {
			if (keyalgToSplit == psks->keyalgToSplit) 
				break;
		}
		psks = psks->next;
	}

	// if dialog already exists, move to foreground
	if (psks) {
		SetForegroundWindow (psks->hwndDlg);
		bOK = TRUE;
	}

	// otherwise create new dialog
	else {
		psks = KMAlloc (sizeof (SPLITKEYSTRUCT));
		if (psks) {
			// initialize struct
			psks->pKM = pKM;
			psks->pHeadOfList = &(pKM->pSplitKeyDialogList);
			PGPGetKeyIDFromKey (key, &(psks->keyidToSplit));
			psks->keyalgToSplit = keyalgToSplit;
			KMGetKeyName (key, psks->szUserIDToSplit, kPGPMaxUserIDSize);
			psks->uTotalShares = 0;
			psks->uThreshold = 2;
			psks->iIndexCurrent = -1;
			psks->pshsCurrent = NULL;
			psks->pDropTarget = NULL; 

			// create modeless dialog
			hwnd = CreateDialogParam (g_hInst, 
					MAKEINTRESOURCE (IDD_SPLITKEY),
					NULL, sSplitKeyDlgProc, (LPARAM)psks);

			// make it "floating"
			SetWindowPos (hwnd, HWND_TOPMOST, 
							0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);

			// add dialog struct to list
			psks->next = pKM->pSplitKeyDialogList;
			pKM->pSplitKeyDialogList = psks;
			bOK = TRUE;
		}
		else 
			bOK = FALSE;
	}

	return bOK;
}


//	_______________________________________________
//
//  Do any split key dialogs exist?

BOOL KMExistSplitKeyDialog (PKEYMAN pKM) 
{
	if (pKM->pSplitKeyDialogList)
		return TRUE;
	else
		return FALSE;
}
