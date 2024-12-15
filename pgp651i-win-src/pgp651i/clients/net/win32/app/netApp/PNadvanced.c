/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPadvanced.c - PGPnet advanced options window message procedure
	

	$Id: PNadvanced.c,v 1.30.4.1.2.1 1999/06/29 18:46:09 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "PGPnetApp.h"
#include "PGPnetHelp.h"

#include "pgpBuildFlags.h"

#include "pgpFileSpec.h"
#include "pgpIke.h"


#define OFFSET_FIRST		2
#define OFFSET_OTHER		6

#define MAXPROPOSALS		16

#define	IKEAUTHCOLWIDTH		100
#define	IKEHASHCOLWIDTH		60
#define	IKECIPHERCOLWIDTH	70
#define	IKEDHCOLWIDTH		60

#define	IKECOL_AUTH			0
#define	IKECOL_HASH			1
#define	IKECOL_CIPHER		2
#define	IKECOL_DH			3

#define	IPSECAHCOLWIDTH		60
#define	IPSECESPCOLWIDTH	130
#define	IPSECCOMPCOLWIDTH	60

#define	IPSECCOL_AH			0
#define	IPSECCOL_ESP		1
#define	IPSECCOL_COMP		2

extern HINSTANCE			g_hinst;
extern PGPContextRef		g_context;
extern APPOPTIONSSTRUCT		g_AOS;
extern CHAR					g_szHelpFile[];
extern HWND					g_hwndOptions;

static INT	g_iSelectedIkeItem;
static INT	g_iSelectedIpsecItem;
static INT	g_iFocusedControl;

static DWORD aAdvIds[] = {			// Help IDs
    IDC_ALLOWCAST,				IDH_PNOPTADV_ALLOWCAST, 
	IDC_ALLOW3DES,				IDH_PNOPTADV_ALLOW3DES,
	IDC_ALLOWDES,				IDH_PNOPTADV_ALLOWDES,
	IDC_ALLOWNONECIPHER,		IDH_PNOPTADV_ALLOWNULLCIPHER,
	IDC_ALLOWSHA1,				IDH_PNOPTADV_ALLOWSHA,
	IDC_ALLOWMD5,				IDH_PNOPTADV_ALLOWMD5,
	IDC_ALLOWNONEHASH,			IDH_PNOPTADV_ALLOWNULLHASH,
#if PGPNET_ALLOW_DH768
	IDC_ALLOW768,				IDH_PNOPTADV_ALLOW768,
#endif //PGPNET_ALLOW_DH768
	IDC_ALLOW1024,				IDH_PNOPTADV_ALLOW1024,
	IDC_ALLOW1536,				IDH_PNOPTADV_ALLOW1536,
	IDC_ALLOWLZS,				IDH_PNOPTADV_ALLOWLZS,
	IDC_ALLOWDEFLATE,			IDH_PNOPTADV_ALLOWDEFLATE,
	IDC_DEFAULTS,				IDH_PNOPTADV_DEFAULTSETTINGS,
	IDC_IKEPROPOSALLIST,		IDH_PNOPTADV_IKEPROPOSALS,
	IDC_IPSECPROPOSALLIST,		IDH_PNOPTADV_IPSECPROPOSALS,
	IDC_NEWPROPOSAL,			IDH_PNOPTADV_NEWPROPOSAL,
	IDC_EDITPROPOSAL,			IDH_PNOPTADV_EDITPROPOSAL,
	IDC_REMOVEPROPOSAL,			IDH_PNOPTADV_REMOVEPROPOSAL,
	IDC_PROPOSALUP,				IDH_PNOPTADV_MOVEPROPOSALUP,
	IDC_PROPOSALDOWN,			IDH_PNOPTADV_MOVEPROPOSALDOWN,
	IDC_PERFECTSECRECY,			IDH_PNOPTADV_PERFECTFWDSECRECY,
	0,0
};

static DWORD aIkeIds[] = {			// Help IDs
    IDC_IKEAUTHCOMBO,			IDH_PNNEWIKE_AUTHENTICATION, 
    IDC_IKEHASHCOMBO,			IDH_PNNEWIKE_HASH, 
    IDC_IKECIPHERCOMBO,			IDH_PNNEWIKE_CIPHER, 
    IDC_IKEDHCOMBO,				IDH_PNNEWIKE_DIFFIEHELLMAN, 
	0,0
};

static DWORD aIpsecIds[] = {		// Help IDs
    IDC_ENABLEAH,				IDH_PNNEWIPSEC_AHENABLE, 
    IDC_IPSECAHCOMBO,			IDH_PNNEWIPSEC_AHHASH, 
    IDC_ENABLEESP,				IDH_PNNEWIPSEC_ESPENABLE, 
    IDC_IPSECESPHASHCOMBO,		IDH_PNNEWIPSEC_ESPHASH, 
    IDC_IPSECESPCIPHERCOMBO,	IDH_PNNEWIPSEC_ESPCIPHER, 
    IDC_ENABLECOMP,				IDH_PNNEWIPSEC_COMPENABLE, 
    IDC_IPSECCOMPCOMBO,			IDH_PNNEWIPSEC_COMPALGORITHM, 
	0,0
};

//	____________________________________
//
//	select the specified item

static VOID
sSelectItem (
		HWND	hwnd,
		INT		iControl,
		INT		iItem)
{
	HWND	hwndList;

	hwndList = GetDlgItem (hwnd, iControl);

	ListView_SetItemState (hwndList, iItem, LVIS_SELECTED, LVIS_SELECTED);
	ListView_EnsureVisible (hwndList, iItem, FALSE);
}

//	____________________________________
//
//	initialize IKE proposal list view

static VOID
sInitAllowedAlgorithms (
		HWND			hwnd,
		PGPikeMTPref*	pIkeAlgorithmPrefs)
{
	UINT	uState;

	// ciphers
	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.cast5 ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOWCAST, uState);

	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.tripleDES ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOW3DES, uState);

	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.singleDES ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOWDES, uState);

	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.espNULL ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOWNONECIPHER, uState);

	// hashes
	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.sha1 ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOWSHA1, uState);

	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.md5 ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOWMD5, uState);

	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.noAuth ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOWNONEHASH, uState);

	// compression
	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.lzs ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOWLZS, uState);

	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.deflate ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOWDEFLATE, uState);

	// Diffie-Hellman
#if PGPNET_ALLOW_DH768
	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.modpOne768 ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOW768, uState);
#endif //PGPNET_ALLOW_DH768

	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.modpTwo1024 ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOW1024, uState);

	uState = pIkeAlgorithmPrefs->u.allowedAlgorithms.modpFive1536 ?
				BST_CHECKED : BST_UNCHECKED;
	CheckDlgButton (hwnd, IDC_ALLOW1536, uState);
}


//	____________________________________
//
//	retrieve allowed algorithm prefs from checkbox controls

static VOID
sGetAllowedAlgorithms (
		HWND			hwnd,
		PGPikeMTPref*	pIkeAlgorithmPrefs)
{
	// ciphers
	pIkeAlgorithmPrefs->u.allowedAlgorithms.cast5 =
		(IsDlgButtonChecked (hwnd, IDC_ALLOWCAST) == BST_CHECKED);
	pIkeAlgorithmPrefs->u.allowedAlgorithms.tripleDES =
		(IsDlgButtonChecked (hwnd, IDC_ALLOW3DES) == BST_CHECKED);
#if PGPNET_ALLOW_DES56
	pIkeAlgorithmPrefs->u.allowedAlgorithms.singleDES =
		(IsDlgButtonChecked (hwnd, IDC_ALLOWDES) == BST_CHECKED);
#else
	pIkeAlgorithmPrefs->u.allowedAlgorithms.singleDES = FALSE;
#endif //PGPNET_ALLOW_DES56
	pIkeAlgorithmPrefs->u.allowedAlgorithms.espNULL =
		(IsDlgButtonChecked (hwnd, IDC_ALLOWNONECIPHER) == BST_CHECKED);

	// hashes
	pIkeAlgorithmPrefs->u.allowedAlgorithms.sha1 =
		(IsDlgButtonChecked (hwnd, IDC_ALLOWSHA1) == BST_CHECKED);
	pIkeAlgorithmPrefs->u.allowedAlgorithms.md5 =
		(IsDlgButtonChecked (hwnd, IDC_ALLOWMD5) == BST_CHECKED);
	pIkeAlgorithmPrefs->u.allowedAlgorithms.noAuth =
		(IsDlgButtonChecked (hwnd, IDC_ALLOWNONEHASH) == BST_CHECKED);

	// compression
	pIkeAlgorithmPrefs->u.allowedAlgorithms.lzs =
		(IsDlgButtonChecked (hwnd, IDC_ALLOWLZS) == BST_CHECKED);
	pIkeAlgorithmPrefs->u.allowedAlgorithms.deflate =
		(IsDlgButtonChecked (hwnd, IDC_ALLOWDEFLATE) == BST_CHECKED);

	// Diffie-Hellman
#if PGPNET_ALLOW_DH768
	pIkeAlgorithmPrefs->u.allowedAlgorithms.modpOne768 =
		(IsDlgButtonChecked (hwnd, IDC_ALLOW768) == BST_CHECKED);
#endif //PGPNET_ALLOW_DH768
	pIkeAlgorithmPrefs->u.allowedAlgorithms.modpTwo1024 =
		(IsDlgButtonChecked (hwnd, IDC_ALLOW1024) == BST_CHECKED);
	pIkeAlgorithmPrefs->u.allowedAlgorithms.modpFive1536 =
		(IsDlgButtonChecked (hwnd, IDC_ALLOW1536) == BST_CHECKED);
}


//	____________________________________
//
//	initialize IPSEC groupID combobox

static VOID
sLoadIpsecGroupID (
		HWND			hwnd,
		PGPikeGroupID	groupid)
{
	INT		iIndex;

	switch (groupid) {
#if PGPNET_ALLOW_DH768
		case kPGPike_GR_None :		iIndex = 0; break;
		case kPGPike_GR_MODPOne :	iIndex = 1; break;
		case kPGPike_GR_MODPTwo :	iIndex = 2; break;
		case kPGPike_GR_MODPFive :	iIndex = 3; break;
		default :					iIndex = 0; break;	
#else
		case kPGPike_GR_None :		iIndex = 0; break;
		case kPGPike_GR_MODPTwo :	iIndex = 1; break;
		case kPGPike_GR_MODPFive :	iIndex = 2; break;
		default :					iIndex = 0; break;	
#endif //PGPNET_ALLOW_DH768
	}

	SendMessage (GetDlgItem (hwnd, IDC_PERFECTSECRECY), 
			CB_SETCURSEL, (WPARAM)iIndex, 0);
}

//	____________________________________
//
//	retrieve group ID pref from combobox control

static VOID
sGetIpsecGroupID (
		HWND			hwnd,
		PGPikeGroupID*	pIkeIpsecGroupID)
{
	INT		iIndex;

	iIndex = SendMessage (
			GetDlgItem (hwnd, IDC_PERFECTSECRECY), CB_GETCURSEL, 0, 0);

	switch (iIndex) {
#if PGPNET_ALLOW_DH768
		case 0 :	*pIkeIpsecGroupID = kPGPike_GR_None;		break;
		case 1 :	*pIkeIpsecGroupID = kPGPike_GR_MODPOne;		break;
		case 2 :	*pIkeIpsecGroupID = kPGPike_GR_MODPTwo;		break;
		case 3 :	*pIkeIpsecGroupID = kPGPike_GR_MODPFive;	break;
		default :	*pIkeIpsecGroupID = kPGPike_GR_None;		break;
#else
		case 0 :	*pIkeIpsecGroupID = kPGPike_GR_None;		break;
		case 1 :	*pIkeIpsecGroupID = kPGPike_GR_MODPTwo;		break;
		case 2 :	*pIkeIpsecGroupID = kPGPike_GR_MODPFive;	break;
		default :	*pIkeIpsecGroupID = kPGPike_GR_None;		break;
#endif //PGPNET_ALLOW_DH768
	}
}


//	____________________________________
//
//	initialize IKE proposal list view

static VOID
sInitIkeProposalList (
		HWND	hwnd)
{
	HWND			hwndList;
	LV_COLUMN		lvc;
	CHAR			sz[64];

	hwndList = GetDlgItem (hwnd, IDC_IKEPROPOSALLIST);

	lvc.mask = LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
	lvc.pszText = sz;

	LoadString (g_hinst, IDS_IKEAUTHCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = IKEAUTHCOLWIDTH;
	lvc.iSubItem = IKECOL_AUTH;
	ListView_InsertColumn (hwndList, 0, &lvc);

	LoadString (g_hinst, IDS_IKEHASHCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = IKEHASHCOLWIDTH;
	lvc.iSubItem = IKECOL_HASH;
	ListView_InsertColumn (hwndList, 1, &lvc);

	LoadString (g_hinst, IDS_IKECIPHERCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = IKECIPHERCOLWIDTH;
	lvc.iSubItem = IKECOL_CIPHER;
	ListView_InsertColumn (hwndList, 2, &lvc);

	LoadString (g_hinst, IDS_IKEDHCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = IKEDHCOLWIDTH;
	lvc.iSubItem = IKECOL_DH;
	ListView_InsertColumn (hwndList, 3, &lvc);
}

//	____________________________________
//
//	load IKE proposal list view with entries

static VOID
sLoadIkeProposalList (
		HWND			hwnd,
		PGPikeMTPref*	pIkeIkeProposalPrefs)
{
	HWND				hwndList;
	CHAR				sz[64];
	UINT				u, uNumProposals;
	LV_ITEM				lvi;
	PGPikeTransform*	pIke;
	INT					ids;

	hwndList = GetDlgItem (hwnd, IDC_IKEPROPOSALLIST);
	ListView_DeleteAllItems (hwndList);

	uNumProposals = pIkeIkeProposalPrefs->u.ikeProposals.numTransforms;

	lvi.mask = LVIF_TEXT;
	lvi.pszText = sz;
	lvi.stateMask = 0;

	for (u=0; u<uNumProposals; u++)
	{
		pIke = &(pIkeIkeProposalPrefs->u.ikeProposals.t[u]);
		lvi.iItem = u;

		// authentication string
		lvi.iSubItem = IKECOL_AUTH;
		switch (pIke->authMethod) {
			case kPGPike_AM_None :			ids = IDS_NONE;			break;
			case kPGPike_AM_PreSharedKey :	ids = IDS_SHAREDKEY;	break;
			case kPGPike_AM_DSS_Sig :		ids = IDS_DSSSIGNATURE;	break;
			case kPGPike_AM_RSA_Sig :		ids = IDS_RSASIGNATURE;	break;
			default :						ids = IDS_UNKNOWN;		break;
		}
		LoadString (g_hinst, ids, sz, sizeof(sz));
		ListView_InsertItem (hwndList, &lvi);

		// hash string
		lvi.iSubItem = IKECOL_HASH;
		switch (pIke->hash) {
			case kPGPike_HA_None :			ids = IDS_NONE;			break;
			case kPGPike_HA_MD5 :			ids = IDS_MD5;			break;
			case kPGPike_HA_SHA1 :			ids = IDS_SHA;			break;
			default :						ids = IDS_UNKNOWN;		break;
		}
		LoadString (g_hinst, ids, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		// IPCOMP string
		lvi.iSubItem = IKECOL_CIPHER;
		switch (pIke->cipher) {
			case kPGPike_SC_None :			ids = IDS_NONE;			break;
			case kPGPike_SC_DES_CBC :		ids = IDS_DES56;		break;
			case kPGPike_SC_3DES_CBC :		ids = IDS_3DES;			break;
			case kPGPike_SC_CAST_CBC :		ids = IDS_CAST;			break;
			default :						ids = IDS_UNKNOWN;		break;
		}
		LoadString (g_hinst, ids, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);

		// DH string
		lvi.iSubItem = IKECOL_DH;
		switch (pIke->groupID) {
			case kPGPike_GR_None :			ids = IDS_NONE;			break;
			case kPGPike_GR_MODPOne :		ids = IDS_768;			break;
			case kPGPike_GR_MODPTwo :		ids = IDS_1024;			break;
			case kPGPike_GR_MODPFive :		ids = IDS_1536;			break;
			default :						ids = IDS_UNKNOWN;		break;
		}
		LoadString (g_hinst, ids, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);
	}
}

//	____________________________________
//
//	initialize IKE proposal list view

static VOID
sInitIpsecProposalList (
		HWND	hwnd)
{
	HWND			hwndList;
	LV_COLUMN		lvc;
	CHAR			sz[64];

	hwndList = GetDlgItem (hwnd, IDC_IPSECPROPOSALLIST);

	lvc.mask = LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
	lvc.pszText = sz;

	LoadString (g_hinst, IDS_IPSECAHCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = IPSECAHCOLWIDTH;
	lvc.iSubItem = IPSECCOL_AH;
	ListView_InsertColumn (hwndList, 0, &lvc);

	LoadString (g_hinst, IDS_IPSECESPCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = IPSECESPCOLWIDTH;
	lvc.iSubItem = IPSECCOL_ESP;
	ListView_InsertColumn (hwndList, 1, &lvc);

	LoadString (g_hinst, IDS_IPSECCOMPCOLNAME, sz, sizeof(sz));
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = IPSECCOMPCOLWIDTH;
	lvc.iSubItem = IPSECCOL_COMP;
	ListView_InsertColumn (hwndList, 2, &lvc);
}


//	____________________________________
//
//	load IPSEC proposal list view with entries

static VOID
sLoadIpsecProposalList (
		HWND			hwnd,
		PGPikeMTPref*	pIkeIpsecProposalPrefs)
{
	HWND				hwndList;
	CHAR				sz[64];
	UINT				u, uNumProposals;
	LV_ITEM				lvi;
	PGPipsecTransform*	pIpsec;
	INT					ids;

	hwndList = GetDlgItem (hwnd, IDC_IPSECPROPOSALLIST);
	ListView_DeleteAllItems (hwndList);

	uNumProposals = pIkeIpsecProposalPrefs->u.ipsecProposals.numTransforms;

	lvi.mask = LVIF_TEXT;
	lvi.pszText = sz;
	lvi.stateMask = 0;

	for (u=0; u<uNumProposals; u++)
	{
		pIpsec = &(pIkeIpsecProposalPrefs->u.ipsecProposals.t[u]);
		lvi.iItem = u;

		// AH authentication string
		lvi.iSubItem = IPSECCOL_AH;
		switch (pIpsec->ah.authAttr) {
			case kPGPike_AA_None :			ids = IDS_NONE;			break;
			case kPGPike_AA_HMAC_MD5 :		ids = IDS_MD5;			break;
			case kPGPike_AA_HMAC_SHA :		ids = IDS_SHA;			break;
			default :						ids = IDS_UNKNOWN;		break;
		}
		LoadString (g_hinst, ids, sz, sizeof(sz));
		ListView_InsertItem (hwndList, &lvi);

		// ESP hash string
		lvi.iSubItem = IPSECCOL_ESP;
		switch (pIpsec->esp.authAttr) {
			case kPGPike_AA_None :			ids = IDS_NONE;			break;
			case kPGPike_AA_HMAC_MD5	:	ids	= IDS_MD5;			break;
			case kPGPike_AA_HMAC_SHA :		ids = IDS_SHA;			break;
			default :						ids = IDS_UNKNOWN;		break;
		}
		LoadString (g_hinst, ids, sz, sizeof(sz));

		// ESP cipher string
		lvi.iSubItem = IPSECCOL_ESP;
		switch (pIpsec->esp.cipher) {
			case kPGPike_ET_NULL :			ids = IDS_NONE;			break;
			case kPGPike_ET_DES :			ids	= IDS_DES56;		break;
			case kPGPike_ET_DES_IV64 :		ids	= IDS_DES56;		break;
			case kPGPike_ET_3DES :			ids = IDS_3DES;			break;
			case kPGPike_ET_CAST :			ids = IDS_CAST;			break;
			default :						ids = IDS_UNKNOWN;		break;
		}
		if (ids != IDS_NONE)
			PNCommaCat (sz, ids);
		ListView_SetItem (hwndList, &lvi);

		// IPCOMP string
		lvi.iSubItem = IPSECCOL_COMP;
		switch (pIpsec->ipcomp.compAlg) {
			case kPGPike_IC_None :			ids = IDS_NONE;			break;
			case kPGPike_IC_Deflate :		ids = IDS_DEFLATE;		break;
			case kPGPike_IC_LZS :			ids = IDS_LZS;			break;
			default :						ids = IDS_UNKNOWN;		break;
		}
		LoadString (g_hinst, ids, sz, sizeof(sz));
		ListView_SetItem (hwndList, &lvi);
	}
}

//	____________________________________
//
//	dialog proc of IKE proposal dialog

static BOOL CALLBACK
sIkeProposalDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	PGPikeTransform*	pIke;

	switch (uMsg) {
	case WM_INITDIALOG :
	{
		CHAR	sz[32];
		HWND	hwndCombo;
		UINT	u;

		SetWindowLong (hwnd, GWL_USERDATA, lParam);
		pIke = (PGPikeTransform*)GetWindowLong (hwnd, GWL_USERDATA);

		// load combo boxes with text strings
		hwndCombo = GetDlgItem (hwnd, IDC_IKEAUTHCOMBO);
		LoadString (g_hinst, IDS_SHAREDKEY, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_DSSSIGNATURE, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_RSASIGNATURE, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);

		hwndCombo = GetDlgItem (hwnd, IDC_IKEHASHCOMBO);
		LoadString (g_hinst, IDS_SHA, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_MD5, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);

		hwndCombo = GetDlgItem (hwnd, IDC_IKECIPHERCOMBO);
		LoadString (g_hinst, IDS_CAST, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_3DES, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
#if PGPNET_ALLOW_DES56
		LoadString (g_hinst, IDS_DES56, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
#endif //PGPNET_ALLOW_DES56

		hwndCombo = GetDlgItem (hwnd, IDC_IKEDHCOMBO);
#if PGPNET_ALLOW_DH768
		LoadString (g_hinst, IDS_768, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
#endif //PGPNET_ALLOW_DH768
		LoadString (g_hinst, IDS_1024, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_1536, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);

		// initialize combo boxes
		switch (pIke->authMethod) {
			case kPGPike_AM_PreSharedKey :	u = 0;	break;
			case kPGPike_AM_DSS_Sig :		u = 1;	break;
			case kPGPike_AM_RSA_Sig :		u = 2;	break;
			default :						u = 0;	break;
		}
		SendDlgItemMessage (hwnd, IDC_IKEAUTHCOMBO, CB_SETCURSEL, u, 0);

		switch (pIke->hash) {
			case kPGPike_HA_SHA1 :			u = 0;	break;
			case kPGPike_HA_MD5 :			u = 1;	break;
			default :						u = 0;	break;
		}
		SendDlgItemMessage (hwnd, IDC_IKEHASHCOMBO, CB_SETCURSEL, u, 0);

		switch (pIke->cipher) {
			case kPGPike_SC_CAST_CBC :		u = 0;	break;
			case kPGPike_SC_3DES_CBC :		u = 1;	break;
#if PGPNET_ALLOW_DES56
			case kPGPike_SC_DES_CBC :		u = 2;	break;
#endif //PGPNET_ALLOW_DES56
			default :						u = 0;	break;
		}
		SendDlgItemMessage (hwnd, IDC_IKECIPHERCOMBO, CB_SETCURSEL, u, 0);

		switch (pIke->groupID) {
#if PGPNET_ALLOW_DH768
			case kPGPike_GR_MODPOne :		u = 0;	break;
			case kPGPike_GR_MODPTwo :		u = 1;	break;
			case kPGPike_GR_MODPFive :		u = 2;	break;
			default :						u = 1;	break;
#else
			case kPGPike_GR_MODPTwo :		u = 0;	break;
			case kPGPike_GR_MODPFive :		u = 1;	break;
			default :						u = 0;	break;
#endif //PGPNET_ALLOW_DH768
		}
		SendDlgItemMessage (hwnd, IDC_IKEDHCOMBO, CB_SETCURSEL, u, 0);

		return FALSE;
	}

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPVOID) aIkeIds); 
		break; 
 
	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aIkeIds); 
		break; 

	case WM_COMMAND :
		pIke = (PGPikeTransform*)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD (wParam)) {

		case IDOK :
		{
			UINT	u;

			u = SendDlgItemMessage (
						hwnd, IDC_IKEAUTHCOMBO, CB_GETCURSEL, 0, 0);
			switch (u) {
			case 0	:	pIke->authMethod = kPGPike_AM_PreSharedKey;	break;
			case 1	:	pIke->authMethod = kPGPike_AM_DSS_Sig;		break;
			case 2	:	pIke->authMethod = kPGPike_AM_RSA_Sig;		break;
			}

			u = SendDlgItemMessage (
						hwnd, IDC_IKEHASHCOMBO, CB_GETCURSEL, 0, 0);
			switch (u) {
			case 0	:	pIke->hash = kPGPike_HA_SHA1;			break;
			case 1	:	pIke->hash = kPGPike_HA_MD5;			break;
			}

			u = SendDlgItemMessage (
						hwnd, IDC_IKECIPHERCOMBO, CB_GETCURSEL, 0, 0);
			switch (u) {
			case 0	:	pIke->cipher = kPGPike_SC_CAST_CBC;		break;
			case 1	:	pIke->cipher = kPGPike_SC_3DES_CBC;		break;
			case 2	:	pIke->cipher = kPGPike_SC_DES_CBC;		break;
			}

			u = SendDlgItemMessage (
						hwnd, IDC_IKEDHCOMBO, CB_GETCURSEL, 0, 0);
			switch (u) {
#if PGPNET_ALLOW_DH768
			case 0	:	pIke->groupID = kPGPike_GR_MODPOne;		break;
			case 1	:	pIke->groupID = kPGPike_GR_MODPTwo;		break;
			case 2	:	pIke->groupID = kPGPike_GR_MODPFive;	break;
#else
			case 0	:	pIke->groupID = kPGPike_GR_MODPTwo;		break;
			case 1	:	pIke->groupID = kPGPike_GR_MODPFive;	break;
#endif //PGPNET_ALLOW_DH768
			}

			if ((pIke->authMethod == kPGPike_AM_DSS_Sig) &&
				(pIke->hash != kPGPike_HA_SHA1))
			{
				PNMessageBox (hwnd, IDS_CAPTION, IDS_DSSNONSHAWARN,
										MB_OK|MB_ICONEXCLAMATION);
				break;
			}

			if (pIke->cipher == kPGPike_SC_DES_CBC)
			{
				if (PNMessageBox (hwnd, IDS_CAPTION, IDS_DESWARN,
					MB_OKCANCEL|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDOK)
					EndDialog (hwnd, 1);
			}
			else
				EndDialog (hwnd, 1);

			break;
		}

		case IDCANCEL :
			EndDialog (hwnd, 0);
			break;
		}
		break;
	}

	return FALSE;
}


//	_____________________________________________________
//
//  display IKE proposal dialog

static VOID
sNewIkeProposalDialog (
		PGPContextRef	context,
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	PGPikeTransform		ike;

	ike.authMethod		= kPGPike_AM_PreSharedKey;
	ike.hash			= kPGPike_HA_SHA1;
	ike.cipher			= kPGPike_SC_CAST_CBC;
	ike.modpGroup		= TRUE;
	ike.groupID			= kPGPike_GR_MODPTwo;

	if (DialogBoxParam (g_hinst, MAKEINTRESOURCE(IDD_IKEPROPOSAL), 
				hwnd, sIkeProposalDlgProc, (LPARAM)&ike))
	{
		PGPikeTransform*	pIke;
		PGPSize				size;
		UINT				uNumProposals;
		
		uNumProposals = ++(pos->pnconfig.IkeIkeProposalPrefs.
										u.ikeProposals.numTransforms);

		size = uNumProposals * sizeof(PGPikeTransform);

		pIke = PGPNewData (PGPGetContextMemoryMgr (context),
										size, kPGPMemoryMgrFlags_Clear);
		if (pIke)
		{
			pos->pnconfig.IkeIkeProposalPrefs.u.ikeProposals.numTransforms = 
											uNumProposals;

			pgpCopyMemory (&ike, pIke, sizeof(PGPikeTransform));
			pgpCopyMemory (
				pos->pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t,
				&pIke[1], (uNumProposals-1)*sizeof(PGPikeTransform));

			PGPFreeData (pos->pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t);
			pos->pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t = pIke;

			sLoadIkeProposalList (hwnd, &pos->pnconfig.IkeIkeProposalPrefs);

			sSelectItem (hwnd, IDC_IKEPROPOSALLIST, 0);
			g_iSelectedIkeItem = 0;
		}
	}
}


//	____________________________________
//
//	dialog proc of IPSEC proposal dialog

static BOOL CALLBACK
sIpsecProposalDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	PGPipsecTransform*	pIpsec;

	switch (uMsg) {
	case WM_INITDIALOG :
	{
		CHAR	sz[32];
		HWND	hwndCombo;
		UINT	u;

		SetWindowLong (hwnd, GWL_USERDATA, lParam);
		pIpsec = (PGPipsecTransform*)GetWindowLong (hwnd, GWL_USERDATA);

		// load combo box strings
		hwndCombo = GetDlgItem (hwnd, IDC_IPSECAHCOMBO);
		LoadString (g_hinst, IDS_SHA, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_MD5, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);

		hwndCombo = GetDlgItem (hwnd, IDC_IPSECESPCIPHERCOMBO);
		LoadString (g_hinst, IDS_NONE, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_CAST, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_3DES, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
#if PGPNET_ALLOW_DES56
		LoadString (g_hinst, IDS_DES56, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
#endif //PGPNET_ALLOW_DES56

		hwndCombo = GetDlgItem (hwnd, IDC_IPSECESPHASHCOMBO);
		LoadString (g_hinst, IDS_NONE, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_SHA, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_MD5, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);

		hwndCombo = GetDlgItem (hwnd, IDC_IPSECCOMPCOMBO);
		LoadString (g_hinst, IDS_LZS, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_DEFLATE, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);

		// initialize check boxes
		if (pIpsec->useAH)
		{
			CheckDlgButton (hwnd, IDC_ENABLEAH, BST_CHECKED);
			EnableWindow (GetDlgItem (hwnd, IDC_IPSECAHCOMBO), TRUE);
		}

		if (pIpsec->useESP)
		{
			CheckDlgButton (hwnd, IDC_ENABLEESP, BST_CHECKED);
			EnableWindow (GetDlgItem (hwnd, IDC_IPSECESPCIPHERCOMBO), TRUE);
			EnableWindow (GetDlgItem (hwnd, IDC_IPSECESPHASHCOMBO), TRUE);
		}

		if (pIpsec->useIPCOMP)
		{
			CheckDlgButton (hwnd, IDC_ENABLECOMP, BST_CHECKED);
			EnableWindow (GetDlgItem (hwnd, IDC_IPSECCOMPCOMBO), TRUE);
		}

		// initialize combo boxes
		switch (pIpsec->ah.authAttr) {
			case kPGPike_AA_HMAC_SHA :		u = 0;	break;
			case kPGPike_AA_HMAC_MD5 :		u = 1;	break;
			default :						u = 0;	break;
		}
		SendDlgItemMessage (hwnd, IDC_IPSECAHCOMBO, CB_SETCURSEL, u, 0);

		switch (pIpsec->esp.cipher) {
			case kPGPike_ET_NULL :			u = 0;	break;
			case kPGPike_ET_CAST :			u = 1;	break;
			case kPGPike_ET_3DES :			u = 2;	break;
			case kPGPike_ET_DES :			u = 3;	break;
			case kPGPike_ET_DES_IV64 :		u = 3;	break;
			default :						u = 0;	break;
		}
		SendDlgItemMessage (
					hwnd, IDC_IPSECESPCIPHERCOMBO, CB_SETCURSEL, u, 0);

		switch (pIpsec->esp.authAttr) {
			case kPGPike_AA_None :			u = 0;	break;
			case kPGPike_AA_HMAC_SHA :		u = 1;	break;
			case kPGPike_AA_HMAC_MD5 :		u = 2;	break;
			default :						u = 0;	break;
		}
		SendDlgItemMessage (hwnd, IDC_IPSECESPHASHCOMBO, CB_SETCURSEL, u, 0);

		switch (pIpsec->ipcomp.compAlg) {
			case kPGPike_IC_LZS :			u = 0;	break;
			case kPGPike_IC_Deflate :		u = 1;	break;
			default :						u = 0;	break;
		}
		SendDlgItemMessage (hwnd, IDC_IPSECCOMPCOMBO, CB_SETCURSEL, u, 0);

		return FALSE;
	}

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPVOID) aIpsecIds); 
		break; 
 
	case WM_CONTEXTMENU: 
		WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
			(DWORD) (LPVOID) aIpsecIds); 
		break; 

	case WM_COMMAND :
		pIpsec = (PGPipsecTransform*)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD (wParam)) {

		case IDC_ENABLEAH :
			if (IsDlgButtonChecked (hwnd, IDC_ENABLEAH) == BST_CHECKED)
				EnableWindow (GetDlgItem (hwnd, IDC_IPSECAHCOMBO), TRUE);
			else
				EnableWindow (GetDlgItem (hwnd, IDC_IPSECAHCOMBO), FALSE);
			break;

		case IDC_ENABLEESP :
			if (IsDlgButtonChecked (hwnd, IDC_ENABLEESP) == BST_CHECKED)
			{
				EnableWindow (GetDlgItem (
								hwnd, IDC_IPSECESPHASHCOMBO), TRUE);
				EnableWindow (GetDlgItem (
								hwnd, IDC_IPSECESPCIPHERCOMBO), TRUE);
			}
			else
			{
				EnableWindow (GetDlgItem (
								hwnd, IDC_IPSECESPHASHCOMBO), FALSE);
				EnableWindow (GetDlgItem (
								hwnd, IDC_IPSECESPCIPHERCOMBO), FALSE);
			}
			break;

		case IDC_ENABLECOMP :
			if (IsDlgButtonChecked (hwnd, IDC_ENABLECOMP) == BST_CHECKED)
				EnableWindow (GetDlgItem (hwnd, IDC_IPSECCOMPCOMBO), TRUE);
			else
				EnableWindow (GetDlgItem (hwnd, IDC_IPSECCOMPCOMBO), FALSE);
			break;

		case IDCANCEL :
			EndDialog (hwnd, 0);
			break;

		case IDOK :
		{
			UINT	u;

			if (IsDlgButtonChecked (hwnd, IDC_ENABLEAH) == BST_CHECKED)
			{
				pIpsec->useAH = TRUE;

				u = SendDlgItemMessage (
							hwnd, IDC_IPSECAHCOMBO, CB_GETCURSEL, 0, 0);
				switch (u) {
				case 0	:	
					pIpsec->ah.authAlg = kPGPike_AH_SHA;	
					pIpsec->ah.authAttr = kPGPike_AA_HMAC_SHA;	
					break;

				case 1	:	
					pIpsec->ah.authAlg = kPGPike_AH_MD5;	
					pIpsec->ah.authAttr = kPGPike_AA_HMAC_MD5;	
					break;
				}
			}
			else
			{
				pIpsec->useAH = FALSE;
				pIpsec->ah.authAlg = kPGPike_AH_None;
				pIpsec->ah.authAttr = kPGPike_AA_None;
			}

			if (IsDlgButtonChecked (hwnd, IDC_ENABLEESP) == BST_CHECKED)
			{
				pIpsec->useESP = TRUE;

				u = SendDlgItemMessage (
						hwnd, IDC_IPSECESPCIPHERCOMBO, CB_GETCURSEL, 0, 0);
				switch (u) {
				case 0	:	pIpsec->esp.cipher = kPGPike_ET_NULL;		break;
				case 1	:	pIpsec->esp.cipher = kPGPike_ET_CAST;		break;
				case 2	:	pIpsec->esp.cipher = kPGPike_ET_3DES;		break;
				case 3	:	pIpsec->esp.cipher = kPGPike_ET_DES;		break;
				}

				u = SendDlgItemMessage (
						hwnd, IDC_IPSECESPHASHCOMBO, CB_GETCURSEL, 0, 0);
				switch (u) {
				case 0	:	pIpsec->esp.authAttr = kPGPike_AA_None;		break;
				case 1	:	pIpsec->esp.authAttr = kPGPike_AA_HMAC_SHA;	break;
				case 2	:	pIpsec->esp.authAttr = kPGPike_AA_HMAC_MD5;	break;
				}
			}
			else
			{
				pIpsec->useESP = FALSE;
				pIpsec->esp.cipher = kPGPike_ET_NULL;
				pIpsec->esp.authAttr = kPGPike_AA_None;
			}

			if (IsDlgButtonChecked (hwnd, IDC_ENABLECOMP) == BST_CHECKED)
			{
				pIpsec->useIPCOMP = TRUE;

				u = SendDlgItemMessage (
						hwnd, IDC_IPSECCOMPCOMBO, CB_GETCURSEL, 0, 0);
				switch (u) {
				case 0	:	pIpsec->ipcomp.compAlg = kPGPike_IC_LZS;	break;
				case 1	:	pIpsec->ipcomp.compAlg = kPGPike_IC_Deflate;break;
				}
			}
			else
			{
				pIpsec->useIPCOMP = FALSE;
				pIpsec->ipcomp.compAlg = kPGPike_IC_None;
			}

			// check for valid combinations
			if ((!pIpsec->useAH) &&
				(!pIpsec->useESP))
			{
				PNMessageBox (hwnd, IDS_CAPTION, IDS_NEEDAHORESP,
								MB_OK|MB_ICONSTOP);
				break;
			}

			if ((pIpsec->useESP) &&
				(pIpsec->esp.cipher == kPGPike_ET_NULL) &&
				(pIpsec->esp.authAttr == kPGPike_AA_None))
			{
				PNMessageBox (hwnd, IDS_CAPTION, IDS_ESPNEEDSALGORITHM,
								MB_OK|MB_ICONSTOP);
				break;
			}

			// warn user about DES
			if ((pIpsec->esp.cipher == kPGPike_ET_DES_IV64) ||
				(pIpsec->esp.cipher == kPGPike_ET_DES))
			{
				if (PNMessageBox (hwnd, IDS_CAPTION, IDS_DESWARN,
					MB_OKCANCEL|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDOK)
					EndDialog (hwnd, 1);
			}
			else
				EndDialog (hwnd, 1);

			break;
		}
		}
		break;
	}

	return FALSE;
}


//	_____________________________________________________
//
//  display IPSEC proposal dialog

static VOID
sNewIpsecProposalDialog (
		PGPContextRef	context,
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	PGPipsecTransform	ipsec;

	ipsec.useAH				= TRUE;
	ipsec.ah.authAlg		= kPGPike_AH_MD5;
	ipsec.ah.authAttr		= kPGPike_AA_HMAC_SHA;
	ipsec.useESP			= TRUE;
	ipsec.esp.cipher		= kPGPike_ET_CAST;
	ipsec.esp.authAttr		= kPGPike_AA_HMAC_SHA;
	ipsec.useIPCOMP			= FALSE;
	ipsec.ipcomp.compAlg	= kPGPike_IC_None;
	ipsec.groupID			= kPGPike_GR_MODPTwo;

	if (DialogBoxParam (g_hinst, MAKEINTRESOURCE(IDD_IPSECPROPOSAL), 
				hwnd, sIpsecProposalDlgProc, (LPARAM)&ipsec))
	{
		PGPipsecTransform*	pIpsec;
		PGPSize				size;
		UINT				uNumProposals;
		
		uNumProposals = ++(pos->pnconfig.IkeIpsecProposalPrefs.
										u.ipsecProposals.numTransforms);

		size = uNumProposals * sizeof(PGPipsecTransform);

		pIpsec = PGPNewData (PGPGetContextMemoryMgr (context),
										size, kPGPMemoryMgrFlags_Clear);
		if (pIpsec)
		{
			pos->pnconfig.IkeIpsecProposalPrefs.
						u.ipsecProposals.numTransforms = uNumProposals;

			pgpCopyMemory (&ipsec, pIpsec, sizeof(PGPipsecTransform));
			pgpCopyMemory (
				pos->pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t,
				&pIpsec[1], (uNumProposals-1)*sizeof(PGPipsecTransform));

			PGPFreeData (
				pos->pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t);
			pos->pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t = pIpsec;

			sLoadIpsecProposalList (
					hwnd, &pos->pnconfig.IkeIpsecProposalPrefs);

			sSelectItem (hwnd, IDC_IPSECPROPOSALLIST, 0);
			g_iSelectedIpsecItem = 0;
		}
	}
}


//	_____________________________________________________
//
//  post "New" button context menu 

static VOID
sNewProposalContextMenu (
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	HMENU			hMC;
	HMENU			hMenuTrackPopup;
	RECT			rc;
	INT				iNumItems;

	hMC = LoadMenu (g_hinst, MAKEINTRESOURCE (IDR_MENUNEWPROPOSAL));
	hMenuTrackPopup = GetSubMenu (hMC, 0);

	iNumItems = 
		pos->pnconfig.IkeIkeProposalPrefs.u.ikeProposals.numTransforms;

	if (iNumItems >= MAXPROPOSALS)
		EnableMenuItem (hMenuTrackPopup, IDM_NEWIKEPROPOSAL, 
							MF_BYCOMMAND | MF_GRAYED);

	iNumItems = 
		pos->pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.numTransforms;

	if (iNumItems >= MAXPROPOSALS)
		EnableMenuItem (hMenuTrackPopup, IDM_NEWIPSECPROPOSAL, 
							MF_BYCOMMAND | MF_GRAYED);

	GetWindowRect (GetDlgItem (hwnd, IDC_NEWPROPOSAL), &rc);
	TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					rc.left, rc.bottom, 0, hwnd, NULL);

	DestroyMenu (hMC);
}


//	_____________________________________________________
//
//  set proposal button states 

static VOID
sEnableProposalButtons (
		HWND			hwnd,
		BOOL			bEnable,
		POPTIONSSTRUCT	pos)
{
	INT		iSelectedItem	= -1;
	HWND	hwndCtrl;
	INT		iNumItems;

	if (bEnable)
	{
		EnableWindow (GetDlgItem (hwnd, IDC_EDITPROPOSAL), TRUE);

		if (g_iFocusedControl == IDC_IKEPROPOSALLIST)
		{
			iNumItems = pos->pnconfig.IkeIkeProposalPrefs.
									u.ikeProposals.numTransforms;
			iSelectedItem = g_iSelectedIkeItem;
		}
		else if (g_iFocusedControl == IDC_IPSECPROPOSALLIST)
		{
			iNumItems = pos->pnconfig.IkeIpsecProposalPrefs.
									u.ipsecProposals.numTransforms;
			iSelectedItem = g_iSelectedIpsecItem;
		}

		if (iSelectedItem >= 0)
		{
			EnableWindow (GetDlgItem (hwnd, IDC_REMOVEPROPOSAL), 
								(iNumItems > 1));

			hwndCtrl = GetDlgItem (hwnd, IDC_PROPOSALUP);
			if (iSelectedItem > 0)
				EnableWindow (hwndCtrl, TRUE);
			else 
			{
				if (GetFocus () == hwndCtrl)
				{
					SetFocus (GetDlgItem (hwnd, IDC_PROPOSALDOWN));
					SendMessage (hwnd, DM_SETDEFID, IDC_PROPOSALDOWN, 0);
				}
				EnableWindow (hwndCtrl, FALSE);
			}

			hwndCtrl = GetDlgItem (hwnd, IDC_PROPOSALDOWN);
			if (iSelectedItem < iNumItems-1)
				EnableWindow (hwndCtrl, TRUE);
			else
			{
				if (GetFocus () == hwndCtrl)
				{
					SetFocus (GetDlgItem (hwnd, IDC_PROPOSALUP));
					SendMessage (hwnd, DM_SETDEFID, IDC_PROPOSALUP, 0);
				}
				EnableWindow (hwndCtrl, FALSE);
			}
		}
	}
	else
	{
		EnableWindow (GetDlgItem (hwnd, IDC_EDITPROPOSAL), FALSE);
		EnableWindow (GetDlgItem (hwnd, IDC_REMOVEPROPOSAL), FALSE);
		EnableWindow (GetDlgItem (hwnd, IDC_PROPOSALUP), FALSE);
		EnableWindow (GetDlgItem (hwnd, IDC_PROPOSALDOWN), FALSE);
	}
}


//	_____________________________________________________
//
//  edit the selected proposal

static VOID
sEditSelectedProposal (
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	if (g_iFocusedControl == IDC_IKEPROPOSALLIST)
	{
		PGPikeTransform		ike;
		PGPikeTransform*	pCurrentIke;

		pCurrentIke = &pos->pnconfig.IkeIkeProposalPrefs.
							u.ikeProposals.t[g_iSelectedIkeItem];
		pgpCopyMemory (pCurrentIke, &ike, sizeof(PGPikeTransform));

		if (DialogBoxParam (g_hinst, MAKEINTRESOURCE(IDD_IKEPROPOSAL), 
				hwnd, sIkeProposalDlgProc, (LPARAM)&ike))
		{
			pgpCopyMemory (&ike, pCurrentIke, sizeof(PGPikeTransform));
			sLoadIkeProposalList (hwnd, &pos->pnconfig.IkeIkeProposalPrefs);
		}

		sSelectItem (hwnd, g_iFocusedControl, g_iSelectedIkeItem);
	}

	else if (g_iFocusedControl == IDC_IPSECPROPOSALLIST)
	{
		PGPipsecTransform		ipsec;
		PGPipsecTransform*		pCurrentIpsec;

		pCurrentIpsec = &pos->pnconfig.IkeIpsecProposalPrefs.
								u.ipsecProposals.t[g_iSelectedIpsecItem];
		pgpCopyMemory (pCurrentIpsec, &ipsec, sizeof(PGPipsecTransform));

		if (DialogBoxParam (g_hinst, MAKEINTRESOURCE(IDD_IPSECPROPOSAL), 
				hwnd, sIpsecProposalDlgProc, (LPARAM)&ipsec))
		{
			pgpCopyMemory (&ipsec, pCurrentIpsec, sizeof(PGPipsecTransform));
			sLoadIpsecProposalList (
				hwnd, &pos->pnconfig.IkeIpsecProposalPrefs);
		}

		sSelectItem (hwnd, g_iFocusedControl, g_iSelectedIpsecItem);
	}
}


//	_____________________________________________________
//
//  remove the selected proposal

static VOID
sRemoveSelectedProposal (
		HWND			hwnd,
		POPTIONSSTRUCT	pos)
{
	UINT	u, uNumProposals;

	if (g_iFocusedControl == IDC_IKEPROPOSALLIST)
	{
		PGPikeTransform*	pIkeSrc;
		PGPikeTransform*	pIkeDst;

		uNumProposals = pos->pnconfig.IkeIkeProposalPrefs.
							u.ikeProposals.numTransforms;

		pIkeDst = &pos->pnconfig.IkeIkeProposalPrefs.
							u.ikeProposals.t[g_iSelectedIkeItem];

		for (u=g_iSelectedIkeItem; u<uNumProposals-1; u++)
		{
			pIkeSrc = 
				&pos->pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t[u+1];

			pgpCopyMemory (pIkeSrc, pIkeDst, sizeof(PGPikeTransform));
			pIkeDst = pIkeSrc;
		}

		pos->pnconfig.IkeIkeProposalPrefs.u.ikeProposals.numTransforms--;
		sLoadIkeProposalList (hwnd, &pos->pnconfig.IkeIkeProposalPrefs);

		g_iSelectedIkeItem = -1;
		sEnableProposalButtons (hwnd, FALSE, NULL);
	}

	else if (g_iFocusedControl == IDC_IPSECPROPOSALLIST)
	{
		PGPipsecTransform*		pIpsecSrc;
		PGPipsecTransform*		pIpsecDst;

		uNumProposals = pos->pnconfig.IkeIpsecProposalPrefs.
							u.ipsecProposals.numTransforms;

		pIpsecDst = &pos->pnconfig.IkeIpsecProposalPrefs.
							u.ipsecProposals.t[g_iSelectedIpsecItem];

		for (u=g_iSelectedIpsecItem; u<uNumProposals-1; u++)
		{
			pIpsecSrc = &pos->pnconfig.IkeIpsecProposalPrefs.
							u.ipsecProposals.t[u+1];

			pgpCopyMemory (pIpsecSrc, pIpsecDst, sizeof(PGPipsecTransform));
			pIpsecDst = pIpsecSrc;
		}

		pos->pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.numTransforms--;
		sLoadIpsecProposalList (hwnd, &pos->pnconfig.IkeIpsecProposalPrefs);

		g_iSelectedIpsecItem = -1;
		sEnableProposalButtons (hwnd, FALSE, NULL);
	}
}


//	_____________________________________________________
//
//  set proposal button states 

static VOID
sMoveSelectedProposal (
		HWND			hwnd,
		POPTIONSSTRUCT	pos,
		INT				iDir)
{
	if (g_iFocusedControl == IDC_IKEPROPOSALLIST)
	{
		PGPikeTransform		ike;
		PGPikeTransform*	pIkeSrc;
		PGPikeTransform*	pIkeDst;

		pIkeSrc = &pos->pnconfig.IkeIkeProposalPrefs.
							u.ikeProposals.t[g_iSelectedIkeItem];
		pgpCopyMemory (pIkeSrc, &ike, sizeof(PGPikeTransform));

		pIkeDst = &pos->pnconfig.IkeIkeProposalPrefs.
							u.ikeProposals.t[g_iSelectedIkeItem-iDir];
		pgpCopyMemory (pIkeDst, pIkeSrc, sizeof(PGPikeTransform));
		pgpCopyMemory (&ike, pIkeDst, sizeof(PGPikeTransform));

		g_iSelectedIkeItem -= iDir;
		sLoadIkeProposalList (hwnd, &pos->pnconfig.IkeIkeProposalPrefs);
		sEnableProposalButtons (hwnd, TRUE, pos);

		sSelectItem (hwnd, g_iFocusedControl, g_iSelectedIkeItem);
	}

	else if (g_iFocusedControl == IDC_IPSECPROPOSALLIST)
	{
		PGPipsecTransform		ipsec;
		PGPipsecTransform*		pIpsecSrc;
		PGPipsecTransform*		pIpsecDst;

		pIpsecSrc = &pos->pnconfig.IkeIpsecProposalPrefs.
							u.ipsecProposals.t[g_iSelectedIpsecItem];
		pgpCopyMemory (pIpsecSrc, &ipsec, sizeof(PGPipsecTransform));

		pIpsecDst = &pos->pnconfig.IkeIpsecProposalPrefs.
							u.ipsecProposals.t[g_iSelectedIpsecItem-iDir];

		pgpCopyMemory (pIpsecDst, pIpsecSrc, sizeof(PGPipsecTransform));
		pgpCopyMemory (&ipsec, pIpsecDst, sizeof(PGPipsecTransform));

		g_iSelectedIpsecItem -= iDir;
		sLoadIpsecProposalList (hwnd, &pos->pnconfig.IkeIpsecProposalPrefs);
		sEnableProposalButtons (hwnd, TRUE, pos);

		sSelectItem (hwnd, g_iFocusedControl, g_iSelectedIpsecItem);
	}
}


//	_____________________________________________________
//
//  process host list context menu 

static VOID
sContextMenu (
		HWND			hwnd, 
		POPTIONSSTRUCT	pos,
		INT				iX, 
		INT				iY)
{
	INT				iSelectedItem		= -1;
	INT				iNumItems;

	HMENU			hMC;
	HMENU			hMenuTrackPopup;
	MENUITEMINFO	mii;
	LV_ITEM			lvi;

	mii.cbSize		= sizeof(MENUITEMINFO);
	mii.fMask		= MIIM_ID;

	lvi.mask		= LVIF_STATE;
	lvi.iSubItem	= 0;
	lvi.stateMask	= LVIS_SELECTED;

	hMC = LoadMenu (g_hinst, MAKEINTRESOURCE (IDR_MENUPROPOSALCONTEXT));
	hMenuTrackPopup = GetSubMenu (hMC, 0);

	if (g_iFocusedControl == IDC_IKEPROPOSALLIST)
	{
		mii.wID = IDM_NEWIKEPROPOSAL;
		iNumItems = pos->pnconfig.IkeIkeProposalPrefs.
							u.ikeProposals.numTransforms;

		lvi.iItem = g_iSelectedIkeItem;
		ListView_GetItem (GetDlgItem (hwnd, IDC_IKEPROPOSALLIST), &lvi);
		if (lvi.state == LVIS_SELECTED)
			iSelectedItem = g_iSelectedIkeItem;
	}
	else if (g_iFocusedControl == IDC_IPSECPROPOSALLIST)
	{
		mii.wID = IDM_NEWIPSECPROPOSAL;
		iNumItems = pos->pnconfig.IkeIpsecProposalPrefs.
							u.ipsecProposals.numTransforms;

		lvi.iItem = g_iSelectedIpsecItem;
		ListView_GetItem (GetDlgItem (hwnd, IDC_IPSECPROPOSALLIST), &lvi);
		if (lvi.state == LVIS_SELECTED)
			iSelectedItem = g_iSelectedIpsecItem;
	}

	if (iNumItems < MAXPROPOSALS) 
	{
		EnableMenuItem (hMenuTrackPopup, IDC_NEWPROPOSAL, 
								MF_BYCOMMAND|MF_ENABLED);
		SetMenuItemInfo (hMenuTrackPopup, IDC_NEWPROPOSAL, FALSE, &mii);
	}

	if (iSelectedItem >= 0)
	{
		EnableMenuItem (hMenuTrackPopup, IDC_EDITPROPOSAL, 
									MF_BYCOMMAND|MF_ENABLED);

		if (iNumItems > 1)
			EnableMenuItem (hMenuTrackPopup, IDC_REMOVEPROPOSAL, 
									MF_BYCOMMAND|MF_ENABLED);

		if (iSelectedItem > 0)
			EnableMenuItem (hMenuTrackPopup, IDC_PROPOSALUP, 
									MF_BYCOMMAND|MF_ENABLED);

		if (iSelectedItem < iNumItems-1)
			EnableMenuItem (hMenuTrackPopup, IDC_PROPOSALDOWN, 
									MF_BYCOMMAND|MF_ENABLED);
	}

	TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					iX, iY, 0, hwnd, NULL);

	DestroyMenu (hMC);
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

		ListView_GetItemText (pdis->hwndItem, pdis->itemID, iCol,
						sz, sizeof(sz));
		if (lstrlen (sz) == 0)
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


//	____________________________________
//
//	load default advanced prefs by opening a dummy file
//	and reading the prefs.

static PGPError
sLoadDefaultAdvancedPrefs (
		PGPContextRef	context,
		PPNCONFIG		ppnconfig)
{
	PFLFileSpecRef		fileref		= NULL;
	PGPPrefRef			prefref		= kInvalidPGPPrefRef;
	PGPError			err;

	err = PFLGetTempFileSpec (PGPGetContextMemoryMgr (context), 
			NULL, &fileref); CKERR;

	err = PGPOpenPrefFile (fileref, netDefaults, 
			netDefaultsSize, &prefref); CKERR;

	err = PGPnetGetIkeAlgorithmPrefs (context, prefref, ppnconfig); CKERR;

	if (ppnconfig->IkeIkeProposalPrefs.u.ikeProposals.t)
	{
		PGPFreeData (ppnconfig->IkeIkeProposalPrefs.u.ikeProposals.t);
		ppnconfig->IkeIkeProposalPrefs.u.ikeProposals.t = NULL;
	}
	err = PGPnetGetIkeProposalPrefs (context, prefref, ppnconfig); CKERR;

	if (ppnconfig->IkeIpsecProposalPrefs.u.ipsecProposals.t)
	{
		PGPFreeData (ppnconfig->IkeIpsecProposalPrefs.u.ipsecProposals.t);
		ppnconfig->IkeIpsecProposalPrefs.u.ipsecProposals.t = NULL;
	}
	err = PGPnetGetIpsecProposalPrefs (context, prefref, ppnconfig); CKERR;

done :
	if (PGPPrefRefIsValid (prefref)) 
		PGPClosePrefFile (prefref);
	if (IsntNull (fileref)) 
		PFLFreeFileSpec (fileref);

	return err;
}



//	____________________________________
//
//	dialog proc of advanced options dialog

BOOL CALLBACK
PNAdvancedOptionsDlgProc (
		HWND	hwnd, 
		UINT	uMsg, 
		WPARAM	wParam, 
		LPARAM	lParam)
{
	POPTIONSSTRUCT	pos;

	switch (uMsg)
	{
	case WM_INITDIALOG :
	{
		HWND	hwndCombo;
		CHAR	sz[64];
#if !PGPNET_ALLOW_DES56
		RECT	rc;
#endif //PGPNET_ALLOW_DES56

		SetWindowLong (hwnd, GWL_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		g_hwndOptions = hwnd;

		g_iSelectedIkeItem = 0;
		g_iSelectedIpsecItem = 0;
		g_iFocusedControl = 0;

		hwndCombo = GetDlgItem (hwnd, IDC_PERFECTSECRECY);
		LoadString (g_hinst, IDS_NONE, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
#if PGPNET_ALLOW_DH768
		LoadString (g_hinst, IDS_768, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
#endif //PGPNET_ALLOW_DH768
		LoadString (g_hinst, IDS_1024, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);
		LoadString (g_hinst, IDS_1536, sz, sizeof(sz));
		SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM)sz);

		// hide DES(56) checkbox if not allowed
#if !PGPNET_ALLOW_DES56
		GetWindowRect (GetDlgItem (hwnd, IDC_ALLOWDES), &rc);
		MapWindowPoints (NULL, hwnd, (LPPOINT)&rc, 2);
		ShowWindow (GetDlgItem (hwnd, IDC_ALLOWDES), SW_HIDE);
		EnableWindow (GetDlgItem (hwnd, IDC_ALLOWDES), FALSE);
		SetWindowPos (GetDlgItem (hwnd, IDC_ALLOWNONECIPHER), NULL, 
						rc.left, rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
#endif //PGPNET_ALLOW_DES56

		sInitAllowedAlgorithms (hwnd, &pos->pnconfig.IkeAlgorithmPrefs);
		sInitIkeProposalList (hwnd);
		sLoadIkeProposalList (hwnd, &pos->pnconfig.IkeIkeProposalPrefs);
		sInitIpsecProposalList (hwnd);
		sLoadIpsecProposalList (hwnd, &pos->pnconfig.IkeIpsecProposalPrefs);
		sLoadIpsecGroupID (hwnd, pos->pnconfig.IkeIpsecGroupID);

		return FALSE;
	}

	case WM_DESTROY :
		break;

	case WM_NOTIFY :
	{
		LPNMHDR pnmhdr = (LPNMHDR)lParam;
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		if ((pnmhdr->idFrom == IDC_IKEPROPOSALLIST) ||
			(pnmhdr->idFrom == IDC_IPSECPROPOSALLIST))
		{
			switch (pnmhdr->code) {
			case NM_SETFOCUS :
				g_iFocusedControl = pnmhdr->idFrom;

				switch (pnmhdr->idFrom) {
				case IDC_IKEPROPOSALLIST :
					ListView_SetItemState (pnmhdr->hwndFrom,
						g_iSelectedIkeItem, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState (
						GetDlgItem (hwnd, IDC_IPSECPROPOSALLIST),
						g_iSelectedIpsecItem, 0, LVIS_SELECTED);
					break;

				case IDC_IPSECPROPOSALLIST :
					ListView_SetItemState (pnmhdr->hwndFrom,
						g_iSelectedIpsecItem, LVIS_SELECTED, LVIS_SELECTED);
					ListView_SetItemState (
						GetDlgItem (hwnd, IDC_IKEPROPOSALLIST),
						g_iSelectedIkeItem, 0, LVIS_SELECTED);
					break;
				}

				sEnableProposalButtons (hwnd, TRUE, pos);
				break;

			case LVN_ITEMCHANGED :
			{
				NM_LISTVIEW* pnmlv = (NM_LISTVIEW*)lParam;

				if (pnmlv->iItem >= 0) 
				{
					if (pnmlv->uNewState & LVIS_SELECTED)
					{
						g_iFocusedControl = pnmhdr->idFrom;
						if (pnmhdr->idFrom == IDC_IKEPROPOSALLIST)
						{
							g_iSelectedIkeItem = pnmlv->iItem;
							ListView_SetItemState (
									GetDlgItem (hwnd, IDC_IPSECPROPOSALLIST),
									g_iSelectedIpsecItem, 0, LVIS_SELECTED);
						}
						else {
							g_iSelectedIpsecItem = pnmlv->iItem;
							ListView_SetItemState (
									GetDlgItem (hwnd, IDC_IKEPROPOSALLIST),
									g_iSelectedIkeItem, 0, LVIS_SELECTED);
						}
						sEnableProposalButtons (hwnd, TRUE, pos);
					}
					else
					{
						sEnableProposalButtons (hwnd, FALSE, pos);
					}
				}
				break;
			}

			case LVN_KEYDOWN :
				if (((LV_KEYDOWN*)lParam)->wVKey == VK_DELETE)
				{
					if (IsWindowEnabled (
							GetDlgItem (hwnd, IDC_REMOVEPROPOSAL)))
					{
						PostMessage (hwnd, WM_COMMAND, IDC_REMOVEPROPOSAL, 0);
					}
				}
				break;
			}
			break;
		}

		switch (((NMHDR FAR *) lParam)->code) {
		case PSN_SETACTIVE :
			break;

		case PSN_HELP :
			WinHelp (hwnd, g_szHelpFile, HELP_CONTEXT, IDH_PNOPTADV_HELP); 
			break;

		case PSN_APPLY :
			pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

			sGetAllowedAlgorithms (hwnd, &g_AOS.pnconfig.IkeAlgorithmPrefs);
			sGetIpsecGroupID (hwnd, &g_AOS.pnconfig.IkeIpsecGroupID);

			g_AOS.pnconfig.IkeIkeProposalPrefs.
									u.ikeProposals.numTransforms =
					pos->pnconfig.IkeIkeProposalPrefs.
									u.ikeProposals.numTransforms;
			PGPFreeData (
				g_AOS.pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t);
			g_AOS.pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t = 
					pos->pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t;
			pos->pnconfig.IkeIkeProposalPrefs.u.ikeProposals.t = NULL;

			g_AOS.pnconfig.IkeIpsecProposalPrefs.
									u.ipsecProposals.numTransforms =
					pos->pnconfig.IkeIpsecProposalPrefs.
									u.ipsecProposals.numTransforms;
			PGPFreeData (
				g_AOS.pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t);
			g_AOS.pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t = 
					pos->pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t;
			pos->pnconfig.IkeIpsecProposalPrefs.u.ipsecProposals.t = NULL;

			SetWindowLong (hwnd, DWL_MSGRESULT, PSNRET_NOERROR);
			return TRUE;

		case PSN_RESET :
			pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
			pos->bUserCancel = TRUE;
			break;
		}
		break;
	}

	case WM_DRAWITEM :
		sDrawItem ((LPDRAWITEMSTRUCT)lParam);
		break;

	case WM_HELP: 
		WinHelp (((LPHELPINFO) lParam)->hItemHandle, g_szHelpFile, 
			HELP_WM_HELP, (DWORD) (LPVOID) aAdvIds); 
		break; 
 
	case WM_CONTEXTMENU :
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);
		if ((HWND)wParam == GetDlgItem (hwnd, IDC_IKEPROPOSALLIST) ||
			(HWND)wParam == GetDlgItem (hwnd, IDC_IPSECPROPOSALLIST))
			sContextMenu (hwnd, pos, LOWORD(lParam), HIWORD(lParam));
		else
			WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
				(DWORD) (LPVOID) aAdvIds); 
		break;

	case WM_COMMAND :
		pos = (POPTIONSSTRUCT)GetWindowLong (hwnd, GWL_USERDATA);

		switch (LOWORD (wParam)) {

		case IDC_DEFAULTS :
			sLoadDefaultAdvancedPrefs (g_context, &pos->pnconfig);
			sInitAllowedAlgorithms (hwnd, 
					&pos->pnconfig.IkeAlgorithmPrefs);
			sLoadIkeProposalList (hwnd, 
					&pos->pnconfig.IkeIkeProposalPrefs);
			sLoadIpsecProposalList (hwnd, 
					&pos->pnconfig.IkeIpsecProposalPrefs);
			sLoadIpsecGroupID (hwnd, pos->pnconfig.IkeIpsecGroupID);
			break;

		case IDC_ALLOWDES :
			if (IsDlgButtonChecked (hwnd, IDC_ALLOWDES) == BST_CHECKED)
			{
				if (PNMessageBox (hwnd, IDS_CAPTION, IDS_DESWARN,
					MB_OKCANCEL|MB_DEFBUTTON2|MB_ICONEXCLAMATION) != IDOK)
					CheckDlgButton (hwnd, IDC_ALLOWDES, BST_UNCHECKED);
			}
			break;

		case IDC_NEWPROPOSAL :
			sNewProposalContextMenu (hwnd, pos);
			break;

		case IDM_NEWIKEPROPOSAL :
			sNewIkeProposalDialog (g_context, hwnd, pos);
			break;

		case IDM_NEWIPSECPROPOSAL :
			sNewIpsecProposalDialog (g_context, hwnd, pos);
			break;

		case IDC_EDITPROPOSAL :
			sEditSelectedProposal (hwnd, pos);
			break;

		case IDC_REMOVEPROPOSAL :
			sRemoveSelectedProposal (hwnd, pos);
			break;

		case IDC_PROPOSALUP :
			sMoveSelectedProposal (hwnd, pos, 1);
			break;

		case IDC_PROPOSALDOWN :
			sMoveSelectedProposal (hwnd, pos, -1);
			break;

		case IDM_PROPOSALHELP :
			if (g_iFocusedControl == IDC_IKEPROPOSALLIST)
				WinHelp (GetDlgItem (hwnd, IDC_IKEPROPOSALLIST), 
						g_szHelpFile, HELP_WM_HELP, 
						(DWORD) (LPVOID) aAdvIds); 
			else if (g_iFocusedControl == IDC_IPSECPROPOSALLIST)
				WinHelp (GetDlgItem (hwnd, IDC_IPSECPROPOSALLIST), 
						g_szHelpFile, HELP_WM_HELP, 
						(DWORD) (LPVOID) aAdvIds); 
			break;
		}
		return 0;
	} 

	return FALSE;
}


