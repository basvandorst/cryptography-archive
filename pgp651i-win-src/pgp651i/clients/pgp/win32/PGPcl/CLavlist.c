/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLavlist.c -	get AVlist either from prefs or from dialog
	

	$Id: CLavlist.c,v 1.17 1999/05/13 22:54:21 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"
#include "pgpkmx.h"
#include "pgpClientLib.h"

// compiler definitions
#define PGP_VERISIGN_NAMES	0	// <- 1 forces VeriSign requests to use 
								// F,M,L names instead of CommonName

// constant definitions
#define	MAX_AV							20
#define INITIAL_ATTRIBUTE_COLUMNWIDTH	120
#define SCROLLBAR_WIDTH					16

#define OFFSET_FIRST		2
#define OFFSET_OTHER		6

// external globals
extern HINSTANCE g_hInst;
extern CHAR g_szHelpFile[MAX_PATH];

// typedefs
typedef struct {
	PGPContextRef		context;
	PGPKeyServerClass	serverclass;
	PGPAttributeValue*	selectedpair;
	PGPAttributeValue*	pAVListStart;
	PGPAttributeValue*	pAVListEnd;
} AVLISTSTRUCT, *PAVLISTSTRUCT;

// local globals
static DWORD aAVlistIds[] = {			// Help IDs
	IDC_CATYPE,		IDH_PGPCLAV_CATYPE,
	IDC_AVLIST,		IDH_PGPCLAV_AVLIST,
	IDC_NEWAV,		IDH_PGPCLAV_ADDAV,
	IDC_EDITAV,		IDH_PGPCLAV_EDITAV,
	IDC_REMOVEAV,	IDH_PGPCLAV_REMOVEAV,
	0,0 
}; 

static DWORD aAVeditIds[] = {			// Help IDs
	IDC_ATTRIBUTELIST,	IDH_PGPCLAV_ATTRIBUTE,
	IDC_VALUE,			IDH_PGPCLAV_VALUE,
	0,0 
}; 



//	___________________________________________________
//
//	fill the AV list with CommonName and Email attributes

static PGPError
sAppendClassAVPairs (
		PGPContextRef		context,
		HWND				hwnd,
		PGPKeyServerClass	serverclass,
		PGPAttributeValue**	ppAVcur)
{
	PGPError	err					= kPGPError_NoErr;
	LPSTR		pszPhrase			= NULL;

	CHAR		sz[256];
	PGPSize		size;

	switch (serverclass) {
	case kPGPKeyServerClass_NetToolsCA :
		return err;

	case kPGPKeyServerClass_Verisign :
		// add challenge attribute
		PGPclMessageBox (hwnd, IDS_CAPTION, IDS_CHALLENGEWARNING, 
							MB_OK|MB_ICONEXCLAMATION);
		LoadString (g_hInst, IDS_CHALLENGEPROMPT, sz, sizeof(sz));
		err = KMGetConfirmationPhrase (context, hwnd, sz, 
					kInvalidPGPKeySetRef, 1, 0, &pszPhrase);
		if (IsntPGPError (err))
		{
			size = lstrlen (pszPhrase);
			(*ppAVcur)->value.pointervalue = PGPNewData (
					PGPGetContextMemoryMgr (context), size+1, 
					kPGPMemoryMgrFlags_Clear);
			if ((*ppAVcur)->value.pointervalue)
			{
				(*ppAVcur)->attribute = kPGPAVAttribute_Challenge;
				(*ppAVcur)->size = size;
				pgpCopyMemory (
						pszPhrase, (*ppAVcur)->value.pointervalue, size);
				++(*ppAVcur);
			}
		}
		if (pszPhrase)
			KMFreePhrase (pszPhrase);

		return err;

	case kPGPKeyServerClass_Entrust :
		return err;

	default :
		return err;
	}
}


//	___________________________________________________
//
//	fill the AV list with attributes common for all requests

static PGPError
sCreateCommonAVPairs (
		PGPContextRef		context,
		PGPKeyServerClass	serverclass,
		PGPUserIDRef		userid,
		PGPAttributeValue**	ppAVcur)
{
	PGPError			err			= kPGPError_NoErr;
	PGPByte*			pName;
	PGPSize				size;

	// first compute those attributes based on the userid
	if (PGPUserIDRefIsValid (userid))
	{
		// get the "common name" attribute from the userid
		err = PGPGetUserIDStringBuffer (
				userid, kPGPUserIDPropCommonName, 0, NULL, &size);

		if (size > 0)
		{
			pName = PGPNewData (PGPGetContextMemoryMgr (context), 
								size+1, kPGPMemoryMgrFlags_Clear);

			if (pName)
			{
				err = PGPGetUserIDStringBuffer (
						userid, kPGPUserIDPropCommonName, size, 
						pName, &size); 

				if (IsntPGPError (err))
				{
#if PGP_VERISIGN_NAMES
					if (serverclass == kPGPKeyServerClass_Verisign)
					{
						CHAR* pFirstName;
						CHAR* pMiddleName;
						CHAR* pLastName;

						PGPParseCommonName (PGPGetContextMemoryMgr (context),
							pName, &pFirstName, &pMiddleName, &pLastName);

						if (pFirstName)
						{
							(*ppAVcur)->attribute = 
										kPGPAVAttribute_MailFirstName;
							(*ppAVcur)->size = lstrlen (pFirstName);
							(*ppAVcur)->value.pointervalue = pFirstName;
							++(*ppAVcur);
						}

						if (pMiddleName)
						{
							// note: middlename is really middle initial
							(*ppAVcur)->attribute = 
										kPGPAVAttribute_MailMiddleName;
							(*ppAVcur)->size = 1;
							(*ppAVcur)->value.pointervalue = pMiddleName;
							pMiddleName[1] = '\0';
							++(*ppAVcur);
						}

						if (pLastName)
						{
							(*ppAVcur)->attribute = 
										kPGPAVAttribute_MailLastName;
							(*ppAVcur)->size = lstrlen (pLastName);
							(*ppAVcur)->value.pointervalue = pLastName;
							++(*ppAVcur);
						}

						PGPFreeData (pName);
					}
					else
#endif //PGP_VERISIGN_NAMES
					{
						(*ppAVcur)->attribute = kPGPAVAttribute_CommonName;
						(*ppAVcur)->size = size;
						(*ppAVcur)->value.pointervalue = pName;
						++(*ppAVcur);
					}
				}
			}
			else
				err = kPGPError_OutOfMemory;
		}
		else
			err = kPGPError_InvalidProperty;
		CKERR;

		// get the "email" attribute from the userid
		err = PGPGetUserIDStringBuffer (
				userid, kPGPUserIDPropEmailAddress, 0, NULL, &size);

		if (size > 0)
		{
			(*ppAVcur)->value.pointervalue = PGPNewData (
					PGPGetContextMemoryMgr (context), size+1, 
					kPGPMemoryMgrFlags_Clear);

			if ((*ppAVcur)->value.pointervalue)
			{
				err = PGPGetUserIDStringBuffer (
						userid, kPGPUserIDPropEmailAddress, size, 
						(*ppAVcur)->value.pointervalue, &size); 

				if (IsntPGPError (err))
				{
					(*ppAVcur)->attribute = kPGPAVAttribute_Email;
					(*ppAVcur)->size = size;
					++(*ppAVcur);
				}
			}
			else
				err = kPGPError_OutOfMemory;
		}
		else
			err = kPGPError_InvalidProperty;
		CKERR;
	}

done:
	return err;
}


//	___________________________________________________
//
//	fill the AV list with default attributes for the specified server class

static PGPError
sCreateDefaultAVPairs (
		PGPContextRef		context,
		PGPKeyServerClass	serverclass,
		PGPAttributeValue**	ppAVcur)
{
	PGPError	err				= kPGPError_NoErr;
	CHAR		vs_certtype[]	= "end-user";
	PGPSize		size;

	// add class-specific attributes
	switch (serverclass) {
	case kPGPKeyServerClass_NetToolsCA :
		break;

	case kPGPKeyServerClass_Verisign :
		size = lstrlen (vs_certtype);
		(*ppAVcur)->value.pointervalue = PGPNewData (
					PGPGetContextMemoryMgr (context), size+1, 
					kPGPMemoryMgrFlags_Clear);
		if ((*ppAVcur)->value.pointervalue)
		{
			(*ppAVcur)->attribute = kPGPAVAttribute_CertType;
			(*ppAVcur)->size = size;
			pgpCopyMemory (
					vs_certtype, (*ppAVcur)->value.pointervalue, size);
			++(*ppAVcur);
		}

		(*ppAVcur)->attribute = kPGPAVAttribute_OrganizationName;
		(*ppAVcur)->size = 0;
		(*ppAVcur)->value.pointervalue = NULL;
		++(*ppAVcur);

		(*ppAVcur)->attribute = kPGPAVAttribute_OrganizationalUnitName;
		(*ppAVcur)->size = 0;
		(*ppAVcur)->value.pointervalue = NULL;
		++(*ppAVcur);

		break;

	case kPGPKeyServerClass_Entrust :
		break;

	default :
		break;
	}

	return err;
}


//	___________________________________________________
//
//	get the display string for the attribute of an AV pair

PGPError PGPclExport
PGPclGetAVListAttributeString (
		PGPAVAttribute	attr,
		LPSTR			psz,
		UINT			uLen)
{
	UINT	uids	= 0;

	switch (attr) {

	case kPGPAVAttribute_CommonName : 
		uids = IDS_AVCOMMONNAME;	
		break;
	case kPGPAVAttribute_Email : 
		uids = IDS_AVEMAIL;	
		break;
	case kPGPAVAttribute_OrganizationName : 
		uids = IDS_AVORGNAME;	
		break;
	case kPGPAVAttribute_OrganizationalUnitName :
		uids = IDS_AVORGUNITNAME;	
		break;
	case kPGPAVAttribute_Country :
		uids = IDS_AVCOUNTRY;	
		break;
	case kPGPAVAttribute_Locality :
		uids = IDS_AVLOCALITY;	
		break;
	case kPGPAVAttribute_State :
		uids = IDS_AVSTATE;	
		break;
	case kPGPAVAttribute_StreetAddress :
		uids = IDS_AVSTREET;	
		break;
	case kPGPAVAttribute_PostalCode :
		uids = IDS_AVPOSTAL;	
		break;
	case kPGPAVAttribute_POBOX :
		uids = IDS_AVPOBOX;	
		break;
	case kPGPAVAttribute_TelephoneNumber :
		uids = IDS_AVTELEPHONE;	
		break;
	case kPGPAVAttribute_Initials :
		uids = IDS_AVINITIALS;	
		break;
	case kPGPAVAttribute_DNSName :
		uids = IDS_AVDNSNAME;	
		break;
	case kPGPAVAttribute_IPAddress :
		uids = IDS_AVIPADDRESS;	
		break;

	// Verisign-specific
	case kPGPAVAttribute_MailFirstName :
		uids = IDS_AVFIRSTNAME;	
		break;
	case kPGPAVAttribute_MailMiddleName :
		uids = IDS_AVMIDDLENAME;	
		break;
	case kPGPAVAttribute_MailLastName :
		uids = IDS_AVLASTNAME;	
		break;		
	case kPGPAVAttribute_CertType :
		uids = IDS_AVCERTTYPE;
		break;
	}

	psz[0] = '\0';
	if (uids)
	{
		LoadString (g_hInst, uids, psz, uLen);
		return kPGPError_NoErr;
	}
	else
		return kPGPError_ItemNotFound;
}

//	___________________________________________________
//
//	get the display string for the value of an AV pair

static VOID
sGetValueString (
		PGPAttributeValue*	pAV,
		LPSTR				psz,
		UINT				uLen)
{
	PGPSize		size;

	if (pAV->value.pointervalue)
	{
		size = min(pAV->size +1, uLen);
		lstrcpyn (psz, pAV->value.pointervalue, size);
		psz[size] = '\0';
	}
	else
		psz[0] = '\0';
}


//	___________________________________________________
//
//	insert the attribute into the list

static VOID
sInsertAttribute (
		HWND			hwnd,
		PAVLISTSTRUCT	pavls,
		PGPAVAttribute	attr)
{
	PGPAttributeValue*	pAV;
	CHAR				sz[128];
	INT					index;

	if (pavls)
	{
		for (pAV=pavls->pAVListStart; pAV<pavls->pAVListEnd; pAV++)
		{
			if (pAV->attribute == attr)
				return;
		}
	}

	PGPclGetAVListAttributeString (attr, sz, sizeof(sz));
	index = SendDlgItemMessage (hwnd, IDC_ATTRIBUTELIST, 
							CB_ADDSTRING, 0, (LPARAM)sz);
	SendDlgItemMessage (hwnd, IDC_ATTRIBUTELIST, 
							CB_SETITEMDATA, (WPARAM)index, (LPARAM)attr);
}


//	___________________________________________________
//
//	Initialize the AV list listview control

static BOOL
sInitAVPairDialog (
		HWND			hwnd,
		PAVLISTSTRUCT	pavls)
{
	BOOL				bReturn		= TRUE;
	PGPAVAttribute		attr		= 0;
	CHAR				sz[256];

	switch (pavls->serverclass) {
	case kPGPKeyServerClass_Invalid :
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_CommonName);
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_MailFirstName);
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_MailMiddleName);
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_MailLastName);
		break;

	case kPGPKeyServerClass_Verisign :
#if PGP_VERISIGN_NAMES
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_MailFirstName);
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_MailMiddleName);
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_MailLastName);
#else
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_CommonName);
#endif //PGP_VERISIGN_NAMES
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_CertType);
		break;

	default :
		sInsertAttribute (hwnd, pavls, kPGPAVAttribute_CommonName);
		break;
	}

	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_Email);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_OrganizationName);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_OrganizationalUnitName);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_Country);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_Locality);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_State);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_StreetAddress);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_PostalCode);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_POBOX);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_TelephoneNumber);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_Initials);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_DNSName);
	sInsertAttribute (hwnd, pavls, kPGPAVAttribute_IPAddress);
	
	if (pavls->selectedpair == NULL)
	{
		LoadString (g_hInst, IDS_NEWATTRIBUTETITLE, sz, sizeof(sz));
		SetWindowText (hwnd, sz);

		if (!SendDlgItemMessage (hwnd, IDC_ATTRIBUTELIST, CB_GETCOUNT, 0, 0))
		{
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_NOMOREATTRIBUTES, 
								MB_OK|MB_ICONEXCLAMATION);
			bReturn = FALSE;
		}
		else
		{
			SendDlgItemMessage (
					hwnd, IDC_ATTRIBUTELIST, CB_SETCURSEL, 0, 0);
			attr = SendDlgItemMessage (
					hwnd, IDC_ATTRIBUTELIST, CB_GETITEMDATA, 0, 0);
		}

		SetFocus (GetDlgItem (hwnd, IDC_ATTRIBUTELIST));
	}
	else
	{
		INT	index;

		attr = pavls->selectedpair->attribute;
		sInsertAttribute (hwnd, NULL, attr);
		PGPclGetAVListAttributeString (attr, sz, sizeof(sz));
		index = SendDlgItemMessage (hwnd, IDC_ATTRIBUTELIST, 
							CB_FINDSTRINGEXACT, 0, (LPARAM)sz);
		SendDlgItemMessage (hwnd, IDC_ATTRIBUTELIST, CB_SETCURSEL, index, 0);
		sGetValueString (pavls->selectedpair, sz, sizeof(sz));
		SetDlgItemText (hwnd, IDC_VALUE, sz);
		SendDlgItemMessage (hwnd, IDC_VALUE, EM_SETSEL, 0, -1);
		SetFocus (GetDlgItem (hwnd, IDC_VALUE));
	}

	if (attr == kPGPAVAttribute_MailMiddleName)
		SendDlgItemMessage (
				hwnd, IDC_VALUE, EM_SETLIMITTEXT, 1, 0);
	else
		SendDlgItemMessage (
				hwnd, IDC_VALUE, EM_SETLIMITTEXT, 0, 0);

	return bReturn;
}


//	___________________________________________________
//
//	New AV pair dialog procedure

static BOOL CALLBACK 
sAVPairDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PAVLISTSTRUCT		pavls;
	PGPSize				size;
	INT					index;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hwnd, GWL_USERDATA, lParam);
		pavls = (PAVLISTSTRUCT)lParam;

		if (!sInitAVPairDialog (hwnd, pavls))
			EndDialog (hwnd, 0);

		return FALSE;

	case WM_HELP: 
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aAVeditIds); 
	    break; 

	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
		    (DWORD) (LPVOID) aAVeditIds); 
		break; 

	case WM_COMMAND :
		pavls = (PAVLISTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD(wParam)) {

		case IDC_ATTRIBUTELIST :
			if (HIWORD (wParam) == CBN_SELCHANGE)
			{
				PGPAVAttribute		attr;

				SetDlgItemText (hwnd, IDC_VALUE, "");
				SetFocus (GetDlgItem (hwnd, IDC_VALUE));

				index = SendDlgItemMessage (
						hwnd, IDC_ATTRIBUTELIST, CB_GETCURSEL, 0, 0);
				attr = SendDlgItemMessage (
						hwnd, IDC_ATTRIBUTELIST, CB_GETITEMDATA, index, 0);

				if (attr == kPGPAVAttribute_MailMiddleName)
					SendDlgItemMessage (
							hwnd, IDC_VALUE, EM_SETLIMITTEXT, 1, 0);
				else
					SendDlgItemMessage (
							hwnd, IDC_VALUE, EM_SETLIMITTEXT, 0, 0);
			}
			break;

		case IDCANCEL :
			EndDialog (hwnd, 0);
			break;

		case IDOK :
			size = GetWindowTextLength (GetDlgItem (hwnd, IDC_VALUE));
			if (size == 0)
			{
				PGPclMessageBox (hwnd, IDS_CAPTION, IDS_MUSTENTERVALUE,
										MB_OK|MB_ICONSTOP);
				break;
			}

			if (pavls->selectedpair)
			{
				if (pavls->selectedpair->value.pointervalue)
					PGPFreeData (pavls->selectedpair->value.pointervalue);
			}
			else
			{
				pavls->selectedpair = pavls->pAVListEnd;
				index = SendDlgItemMessage (
						hwnd, IDC_ATTRIBUTELIST, CB_GETCURSEL, 0, 0);
				pavls->selectedpair->attribute = SendDlgItemMessage (
						hwnd, IDC_ATTRIBUTELIST, CB_GETITEMDATA, index, 0);
				(pavls->pAVListEnd)++;
			}

			pavls->selectedpair->value.pointervalue = PGPNewData (
							PGPGetContextMemoryMgr (pavls->context), 
							size+1, kPGPMemoryMgrFlags_Clear);

			if (pavls->selectedpair->value.pointervalue)
			{
				pavls->selectedpair->size = size;
				GetWindowText (GetDlgItem (hwnd, IDC_VALUE), 
						pavls->selectedpair->value.pointervalue, size+1);
			}
			else
				pavls->selectedpair->size = 0;

			EndDialog (hwnd, 1);
			break;
		}
		return TRUE;
	}
	return FALSE;
}


//	___________________________________________________
//
//	Initialize the AV list listview control

static VOID
sInitAVListControls (
		HWND			hwnd,
		PAVLISTSTRUCT	pavls)
{
	LV_COLUMN		lvc;
	RECT			rc;
	CHAR			sz[32];
	HWND			hwndList;

	// initialize text string
	switch (pavls->serverclass) {
	case kPGPKeyServerClass_NetToolsCA :
		LoadString (g_hInst, IDS_CATYPENETTOOLS, sz, sizeof(sz));
		break;

	case kPGPKeyServerClass_Verisign :
		LoadString (g_hInst, IDS_CATYPEVERISIGN, sz, sizeof(sz));
		break;

	case kPGPKeyServerClass_Entrust :
		LoadString (g_hInst, IDS_CATYPEENTRUST, sz, sizeof(sz));
		break;

	default :
		LoadString (g_hInst, IDS_CATYPEUNSPECIFIED, sz, sizeof(sz));
		break;
	}
	SetDlgItemText (hwnd, IDC_CATYPE, sz);

	// add columns
	hwndList = GetDlgItem (hwnd, IDC_AVLIST);
	GetClientRect (hwndList, &rc);
	lvc.mask = LVCF_WIDTH|LVCF_TEXT;
	lvc.pszText = sz;

	LoadString (g_hInst, IDS_ATTRIBUTE, sz, sizeof(sz));
	lvc.cx = INITIAL_ATTRIBUTE_COLUMNWIDTH;   
	lvc.iSubItem = 0;
	if (ListView_InsertColumn (hwndList, 0, &lvc) == -1) return;

	LoadString (g_hInst, IDS_VALUE, sz, sizeof(sz));
	lvc.cx = rc.right - rc.left 
					-INITIAL_ATTRIBUTE_COLUMNWIDTH -SCROLLBAR_WIDTH;
	lvc.iSubItem = 1;
	if (ListView_InsertColumn (hwndList, 1, &lvc) == -1) return;

}


//	___________________________________________________
//
//	Load the AV list listview control with AV pairs

static VOID
sLoadAVList (
		HWND			hwnd,
		PAVLISTSTRUCT	pavls)
{
	CHAR				sz[256];
	LV_ITEM				lvi;
	HWND				hwndList;
	PGPAttributeValue*	pAV;

	hwndList = GetDlgItem (hwnd, IDC_AVLIST);

	ListView_DeleteAllItems (hwndList);

	lvi.iItem = 0;
	lvi.pszText = sz;

	for (pAV=pavls->pAVListStart; pAV<pavls->pAVListEnd; pAV++)
	{
		lvi.mask = LVIF_TEXT|LVIF_PARAM;
		lvi.iSubItem = 0;
		PGPclGetAVListAttributeString (pAV->attribute, sz, sizeof(sz));
		lvi.lParam = (LPARAM)pAV;
		ListView_InsertItem (hwndList, &lvi);

		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = 1;
		sz[0] = '\0';
		sGetValueString (pAV, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		lvi.iItem++;
	}
}


//	___________________________________________________
//
//	remove the selected AV pair from the list

static VOID
sRemoveSelectedAVPair (
		PAVLISTSTRUCT	pavls)
{
	PGPAttributeValue*	pAV;

	if (pavls->selectedpair->value.pointervalue)
		PGPFreeData (pavls->selectedpair->value.pointervalue);

	for (pAV=pavls->selectedpair; pAV<pavls->pAVListEnd; pAV++)
	{
		pgpCopyMemory ((pAV+1), pAV, sizeof(PGPAttributeValue));
	}

	(pavls->pAVListEnd)--;
}


//	_____________________________________________________
//
//  draw listview item 

static VOID
sDrawItem (
		LPDRAWITEMSTRUCT	pdis)
{
	RECT			rc;
	RECT			rcAll;
	RECT			rcLabel;
	RECT			rcDraw;

	BOOL			bFocus = (GetFocus() == pdis->hwndItem);
	BOOL			bSelected;

	COLORREF		clrTextSave, clrBkSave;

	static CHAR		sz[128];

	LV_COLUMN		lvc;
	LV_ITEM			lvi;
	UINT			nJustify = DT_LEFT;
	INT				iCol;

	// get item data
	lvi.mask		= LVIF_TEXT | LVIF_STATE;
	lvi.iItem		= pdis->itemID;
	lvi.iSubItem	= 0;
	lvi.pszText		= sz;
	lvi.cchTextMax	= sizeof(sz);
	lvi.stateMask	= 0xFFFF;     // get all state flags
	ListView_GetItem (pdis->hwndItem, &lvi);

	bSelected = lvi.state & LVIS_SELECTED;

	ListView_GetItemRect (pdis->hwndItem, pdis->itemID, &rcAll, LVIR_BOUNDS);
	ListView_GetItemRect (pdis->hwndItem, pdis->itemID, &rcLabel, LVIR_LABEL);
	rcAll.left = rcLabel.left;

	// set colors if item is selected
	if (bSelected && bFocus)
	{
		clrTextSave = SetTextColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		clrBkSave = SetBkColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHT));

		FillRect (pdis->hDC, &rcAll, (HBRUSH)(COLOR_HIGHLIGHT+1));
	}
	else if (bSelected && !bFocus)
	{
		clrTextSave = SetTextColor(pdis->hDC, GetSysColor(COLOR_BTNTEXT));
		clrBkSave = SetBkColor(pdis->hDC, GetSysColor(COLOR_BTNFACE));

		FillRect (pdis->hDC, &rcAll, (HBRUSH)(COLOR_BTNFACE+1));
	}
	else
		FillRect (pdis->hDC, &rcAll, (HBRUSH)(COLOR_WINDOW+1));

	// draw item label
	rcDraw = rcLabel;
	rcDraw.left += OFFSET_FIRST;
	rcDraw.right -= OFFSET_FIRST;

	DrawText (pdis->hDC, sz, -1, &rcDraw,
		DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER |
		DT_END_ELLIPSIS);

	// draw labels for extra columns
	rc = rcLabel;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;
	for (iCol = 1; ListView_GetColumn (pdis->hwndItem, iCol, &lvc); iCol++)
	{
		rc.left = rc.right;
		rc.right += lvc.cx;

		sz[0] = '\0';
		ListView_GetItemText (pdis->hwndItem, pdis->itemID, iCol,
						sz, sizeof(sz));
		if (sz[0] == '\0')
			continue;

		switch (lvc.fmt & LVCFMT_JUSTIFYMASK)
		{
		case LVCFMT_RIGHT:
			nJustify = DT_RIGHT;
			break;
		case LVCFMT_CENTER:
			nJustify = DT_CENTER;
			break;
		default:
			break;
		}

		rcDraw = rc;
		rcDraw.left += OFFSET_OTHER;
		rcDraw.right -= OFFSET_OTHER;

		DrawText (pdis->hDC, sz, -1, &rcDraw,
			nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER |
			DT_END_ELLIPSIS);
	}

	// draw focus rectangle if item has focus
	if (lvi.state & LVIS_FOCUSED && bFocus && bSelected)
		DrawFocusRect (pdis->hDC, &rcAll);

	// set original colors if item was selected
	if (bSelected)
	{
		SetTextColor (pdis->hDC, clrTextSave);
		SetBkColor (pdis->hDC, clrBkSave);
	}
}


//	___________________________________________________
//
//	Check if all attributes have values assigned

static BOOL
sAllAttributesHaveValues (
		HWND			hwnd,
		PAVLISTSTRUCT	pavls)
{
	PGPAttributeValue*	pAV;

	for (pAV=pavls->pAVListStart; pAV<pavls->pAVListEnd; pAV++)
	{
		if (pAV->value.pointervalue == NULL)
			return FALSE;
	}

	return TRUE;
}


//	_____________________________________________________
//
//  context menu for AVlist listview

static VOID
sAVlistContextMenu (
		HWND			hwnd,
		PAVLISTSTRUCT	pavls,
		INT				iX, 
		INT				iY)
{
	HMENU	hMC;
	HMENU	hMenuTrackPopup;

	hMC = LoadMenu (g_hInst, MAKEINTRESOURCE (IDR_MENUAVLIST));
	if (pavls->selectedpair) 
	{
		EnableMenuItem (hMC, IDC_EDITAV, MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem (hMC, IDC_REMOVEAV, MF_BYCOMMAND|MF_ENABLED);
	}

	hMenuTrackPopup = GetSubMenu (hMC, 0);

	TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					iX, iY, 0, hwnd, NULL);

	DestroyMenu (hMC);
}


//	___________________________________________________
//
//	AV list dialog procedure

static BOOL CALLBACK 
sAVListDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam) 
{
	PAVLISTSTRUCT		pavls;

	switch (uMsg) {

	case WM_INITDIALOG :
		SetWindowLong (hwnd, GWL_USERDATA, lParam);
		pavls = (PAVLISTSTRUCT)lParam;

		sInitAVListControls (hwnd, pavls);
		sLoadAVList (hwnd, pavls);

		return TRUE;

	case WM_HELP: 
	    WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
	        HELP_WM_HELP, (DWORD) (LPSTR) aAVlistIds); 
	    break; 

	case WM_CONTEXTMENU :
		pavls = (PAVLISTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if ((HWND)wParam == GetDlgItem (hwnd, IDC_AVLIST))
			sAVlistContextMenu (hwnd, pavls, LOWORD(lParam), HIWORD(lParam));
		else
			WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
				(DWORD) (LPVOID) aAVlistIds); 
		break;

	case WM_DRAWITEM :
		sDrawItem ((LPDRAWITEMSTRUCT)lParam);
		break;

	case WM_NOTIFY :
	{
		LPNMHDR pnmhdr = (LPNMHDR)lParam;
		pavls = (PAVLISTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		if (pnmhdr->idFrom == IDC_AVLIST)
		{
			switch (pnmhdr->code) {
			case LVN_ITEMCHANGED :
			{
				NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)lParam;

				if (pnmlv->iItem >= 0) 
				{
					BOOL bEnable = FALSE;

					if (pnmlv->uNewState & LVIS_SELECTED)
					{
						pavls->selectedpair = 
									(PGPAttributeValue*)pnmlv->lParam;
						bEnable = TRUE;
					}
					else
					{
						pavls->selectedpair = NULL;
					}
					EnableWindow (GetDlgItem (hwnd, IDC_EDITAV), bEnable);
					EnableWindow (GetDlgItem (hwnd, IDC_REMOVEAV), bEnable);
				}
				break;
			}

			case NM_DBLCLK :
				if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_AVPAIR), 
							hwnd, sAVPairDlgProc, (LPARAM)pavls))
					sLoadAVList (hwnd, pavls);
				break;
			}
		}
		break;
	}

	case WM_COMMAND :
		pavls = (PAVLISTSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD(wParam)) {

		case IDCANCEL :
			EndDialog (hwnd, 0);
			break;

		case IDOK :
			if (sAllAttributesHaveValues (hwnd, pavls))
				EndDialog (hwnd, 1);
			else
				PGPclMessageBox (hwnd, IDS_CAPTION, 
						IDS_MISSINGATTRIBUTEVALUE, MB_OK|MB_ICONSTOP);
			break;

		case IDC_NEWAV :
			pavls->selectedpair = NULL;
			EnableWindow (GetDlgItem (hwnd, IDC_EDITAV), FALSE);
			EnableWindow (GetDlgItem (hwnd, IDC_REMOVEAV), FALSE);
			if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_AVPAIR), 
						hwnd, sAVPairDlgProc, (LPARAM)pavls))
				sLoadAVList (hwnd, pavls);
			break;

		case IDC_EDITAV :
			if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_AVPAIR), 
						hwnd, sAVPairDlgProc, (LPARAM)pavls))
				sLoadAVList (hwnd, pavls);
			break;

		case IDC_REMOVEAV :
			sRemoveSelectedAVPair (pavls);
			pavls->selectedpair = NULL;
			EnableWindow (GetDlgItem (hwnd, IDC_EDITAV), FALSE);
			EnableWindow (GetDlgItem (hwnd, IDC_REMOVEAV), FALSE);
			sLoadAVList (hwnd, pavls);
			break;

		}
		return TRUE;
	}
	return FALSE;
}


//	___________________________________________________
//
//	fill the AV list from the user dialog

static PGPError
sGetAVPairsFromUser (
	HWND				hwnd,
	PGPContextRef		context,
	PGPKeyServerClass	serverclass,
	PGPAttributeValue*	pAVListStart,
	PGPAttributeValue**	ppAVList)
{
	AVLISTSTRUCT	avls;

	avls.context		= context;
	avls.selectedpair	= NULL;
	avls.serverclass	= serverclass;
	avls.pAVListStart	= pAVListStart;
	avls.pAVListEnd		= *ppAVList;

	if (DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_AVLIST), hwnd, 
						sAVListDlgProc, (LPARAM)&avls))
	{
		*ppAVList = avls.pAVListEnd;
		return kPGPError_NoErr;
	}
	else
	{
		*ppAVList = avls.pAVListEnd;
		return kPGPError_UserAbort;
	}
}


//	___________________________________________________
//
//	fill the AV list from the prefs file

static PGPError
sGetAVPairsFromPrefs (
	PGPContextRef		context,
	PGPPrefRef			prefref,
	PGPAttributeValue**	ppAVcur)
{
	PGPError			err;
	PGPSize				size;
	PGPByte*			data;
	PGPUInt32			numAVs;
	PGPAttributeValue*	pAVlist;
	PGPUInt32			u;

	err = PGPGetPrefData (prefref, kPGPPrefExtraAVPairs, &size, &data);
	if (IsntPGPError (err))
	{
		err = PGPDataToAVPairs (PGPGetContextMemoryMgr (context),
					data, size, &pAVlist, &numAVs);
		PGPDisposePrefData (prefref, data);

		for (u=0; u<numAVs; u++)
		{
			(*ppAVcur)->attribute			= pAVlist[u].attribute;
			(*ppAVcur)->size				= pAVlist[u].size;
			(*ppAVcur)->value.pointervalue	= pAVlist[u].value.pointervalue;
			++(*ppAVcur);
		}

		PGPFreeData (pAVlist);
	}
	return err;
}


//	___________________________________________________
//
//	Get CA cert request AV list either from prefs or from user

PGPError PGPclExport
PGPclGetCACertRequestAVList (
		HWND					hwnd,
		PGPContextRef			context,
		PGPBoolean				bForceDlg,
		PGPUserIDRef			userid,
		PGPKeyServerClass		serverclass,
		PGPAttributeValue**		ppAVlist,
		PGPUInt32*				pNumAVs)
{
	PGPUInt32			numAVs				= 0;
	PGPPrefRef			prefref				= kInvalidPGPPrefRef;

	PGPAttributeValue*	pAVcur;
	PGPError			err;
	PGPAttributeValue	tempAVlist[MAX_AV];

	pAVcur = &tempAVlist[0];

	err = sCreateCommonAVPairs (context, serverclass, userid, &pAVcur); 

#if PGP_BUSINESS_SECURITY
	if (!bForceDlg)
	{
		err = PGPclOpenAdminPrefs (PGPGetContextMemoryMgr (context), 
				&prefref, PGPclIsAdminInstall()); CKERR;
		err = sGetAVPairsFromPrefs (context, prefref, &pAVcur);
	}

	if (IsPGPError (err) || bForceDlg)
#endif
	{
		err = sCreateDefaultAVPairs (
				context, serverclass, &pAVcur); CKERR;
		err = sGetAVPairsFromUser (
				hwnd, context, serverclass, 
				tempAVlist, &pAVcur);
	}

done:
	if (PGPPrefRefIsValid (prefref)) 
		PGPclCloseAdminPrefs (prefref, FALSE);

	if (IsntPGPError (err))
	{
		// is userid is passed in, this is a real request and we
		// should append any class-specific pairs
		if (PGPUserIDRefIsValid (userid))
		{
			err = sAppendClassAVPairs (context, hwnd, serverclass, &pAVcur);
		}
	}

	numAVs = pAVcur - &tempAVlist[0];

	if (IsntPGPError (err))
	{
		PGPSize		size;

		size = numAVs * sizeof(PGPAttributeValue);

		*ppAVlist = PGPNewData (
			PGPGetContextMemoryMgr (context), size, 0);

		if (*ppAVlist)
		{
			pgpCopyMemory (tempAVlist, *ppAVlist, size);
			*pNumAVs = numAVs;
		}
		else 
			err = kPGPError_OutOfMemory;
	}

	if (IsPGPError (err))
	{
		PGPUInt32	u;

		for (u=0; u<numAVs; u++)
		{
			if (tempAVlist[u].value.pointervalue)
				PGPFreeData (tempAVlist[u].value.pointervalue);
		}
	}

	return err;
}


//	___________________________________________________
//
//	Free the CA cert request AV list returned by PGPclGetCACertRequestAVList

PGPError PGPclExport
PGPclFreeCACertRequestAVList (
		PGPAttributeValue*		pAVlist,
		PGPUInt32				NumAVs)
{
	PGPUInt32		u;

	if (pAVlist) 
	{
		for (u=0; u<NumAVs; u++)
		{
			if ((pAVlist[u].attribute >= kPGPAVAttributeFirstPointer) &&
				(pAVlist[u].attribute < kPGPAVAttributeFirstBoolean))
			{
				if (pAVlist[u].value.pointervalue)
					PGPFreeData (pAVlist[u].value.pointervalue);
			}
		}

		PGPFreeData (pAVlist);
	}

	return kPGPError_NoErr;
}
