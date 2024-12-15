/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PNmisc.c - PGPnet miscellaneous routines
	

	$Id: PNmisc.c,v 1.39.4.1 1999/06/07 17:21:22 pbj Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <winsock.h>

#include "resource.h"
#include "PGPnetApp.h"

#include "PGPcl.h"
#include "pgpNetIPC.h"
#include "pgpNetPaths.h"
#include "pgpTrayIPC.h"

#include "pgpOptionList.h"
#include "pgpUserInterface.h"

#define MIN_SECRET_LENGTH	1
#define MIN_SECRET_QUALITY	0

#define IPC_TIMEOUT		2500

extern HINSTANCE		g_hinst;
extern HWND				g_hwndStatusDlg;
extern PGPContextRef	g_context;
extern BOOL				g_bWinsock;
extern APPOPTIONSSTRUCT	g_AOS;

//	___________________________________________________
//
//	Message box routine using string table resource IDs

LPARAM
PNMessageBox (
		 HWND	hWnd, 
		 INT	iCaption, 
		 INT	iMessage,
		 ULONG	ulFlags) 
{
	CHAR szCaption [128];
	CHAR szMessage [512];

	LoadString (g_hinst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hinst, iMessage, szMessage, sizeof(szMessage));

	ulFlags |= MB_SETFOREGROUND;
	return (MessageBox (hWnd, szMessage, szCaption, ulFlags));
}


//	____________________________________
//
//	cat a string resource onto string, using comma if necessary

VOID
PNCommaCat (
		LPSTR	psz,
		INT		ids)
{
	CHAR	szTemp[32];;

	// if string is not empty, cat comma, space onto it
	if (psz[0])
	{
		LoadString (g_hinst, IDS_COMMASPACE, szTemp, sizeof(szTemp));
		lstrcat (psz, szTemp);
	}

	LoadString (g_hinst, ids, szTemp, sizeof(szTemp));
	lstrcat (psz, szTemp);
}


//	____________________________________
//
//	put up the hourglass cursor while reloading keyrings

BOOL
PNLoadKeyRings (
		HWND			hwnd,
		PGPKeySetRef*	pkeyset)
{
	HCURSOR		hcursorOld;
	PGPError	err;

	hcursorOld = SetCursor (LoadCursor (NULL, IDC_WAIT));

	if (PGPKeySetRefIsValid (*pkeyset))
		PGPFreeKeySet (*pkeyset);

	err = PGPOpenDefaultKeyRings (g_context, 0, pkeyset); 
	SetCursor (hcursorOld);

	if (IsPGPError (err))
	{
		PNMessageBox (hwnd, IDS_CAPTION, IDS_CANNOTOPENKEYRINGS,
								MB_OK|MB_ICONSTOP);
		*pkeyset = kInvalidPGPKeySetRef;
		return FALSE;
	}
	else
		return TRUE;
}


//	________________________
//
//	Query service for status

UINT 
PNGetServiceStatus (
		HWND	hwndCaller)
{
	static HWND		hwndService		=	(HWND)0xFFFFFFFF;
	HWND			hwnd;
	DWORD			dwResult;

	hwnd = FindWindow (PGPNET_SERVICECOMMWINDOWNAME, 
									PGPNET_SERVICECOMMWINDOWNAME);
	if (hwnd)
	{
		if (SendMessageTimeout (hwnd, PGPNET_M_APPMESSAGE, 
				(WPARAM)PGPNET_QUERYDRIVERSTATUS, (LPARAM)hwndCaller,
				SMTO_NORMAL, IPC_TIMEOUT, &dwResult))
		{
			if (hwnd != hwndService)
			{
				// service has come alive => tell it we want SAs
				if (hwndService != (HWND)0xFFFFFFFF)
				{
					Sleep (100);
					SendMessageTimeout (hwnd, PGPNET_M_APPMESSAGE, 
						(WPARAM)PGPNET_ENABLESTATUSMESSAGES, 
						(LPARAM)hwndCaller,
						SMTO_NORMAL, IPC_TIMEOUT, &dwResult);
				}
				hwndService = hwnd;
			}
			return dwResult;
		}
		else
		{
			if (hwndService)
			{
				// service was running, now it's not => clear SA list
				hwndService = NULL;
				PostMessage (g_hwndStatusDlg, PGPNET_M_CLEARSALIST, 0, 0);
			}
			return PGPNET_SERVICENOTRESPONDING;
		}
	}
	else
	{
		if (hwndService)
		{
			// service was running, now it's not => clear SA list
			hwndService = NULL;
			PostMessage (g_hwndStatusDlg, PGPNET_M_CLEARSALIST, 0, 0);
		}
		return PGPNET_SERVICENOTAVAILABLE;
	}
}


//	________________________
//
//	Send message to service, if running

BOOL 
PNSendServiceMessage (
		UINT	uMsg,
		WPARAM	wParam,
		LPARAM	lParam)
{
	HWND	hwnd;
	DWORD	dwResult;

	hwnd = FindWindow (PGPNET_SERVICECOMMWINDOWNAME, 
									PGPNET_SERVICECOMMWINDOWNAME);
	if (hwnd)
	{
		if (SendMessageTimeout (hwnd, uMsg, wParam, lParam, 
							SMTO_NORMAL, IPC_TIMEOUT, &dwResult))
			return TRUE;
		else
			return FALSE;
	}
	else
		return FALSE;
}

//	________________________
//
//	Send message to tray app, if running

BOOL 
PNSendTrayAppMessage (
		UINT	uMsg,
		WPARAM	wParam,
		LPARAM	lParam)
{
	HWND	hwnd;
	DWORD	dwResult;

	hwnd = FindWindow (PGPTRAY_WINDOWNAME, PGPTRAY_WINDOWNAME);
	if (hwnd)
	{
		SendMessageTimeout (hwnd, uMsg, wParam, lParam, 
							SMTO_NORMAL, IPC_TIMEOUT, &dwResult);
		return TRUE;
	}
	else
		return FALSE;
}

//	________________________
//
//	Convert tm to SystemTime

VOID 
sTimeToSystemTime (
	struct tm*	ptm, 
	SYSTEMTIME* pst) 
{
	pst->wYear = ptm->tm_year + 1900;
	pst->wMonth = ptm->tm_mon + 1;
	pst->wDay = ptm->tm_mday;
	pst->wDayOfWeek = ptm->tm_wday;
	pst->wHour = ptm->tm_hour;
	pst->wMinute = ptm->tm_min;
	pst->wSecond = ptm->tm_sec;
	pst->wMilliseconds = 0;
}

//	______________________________________________________
//
//	Convert time to string format based on system settings

VOID
PNConvertTimeToString (
		PGPTime		Time, 
		LPSTR		sz, 
		INT			iLen) 
{
	SYSTEMTIME	systemtime;
	time_t		ttTime;
	INT			iBytes;
	struct tm*	ptm;

	ttTime = PGPGetStdTimeFromPGPTime (Time);
	ptm = localtime (&ttTime);

	if (ptm) {
		sTimeToSystemTime (ptm, &systemtime);
		iBytes = GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, 
						&systemtime, NULL, sz, iLen);
		lstrcat (sz, " ");
		GetTimeFormat (LOCALE_USER_DEFAULT, 0, 
						&systemtime, NULL, &sz[iBytes], iLen-iBytes);
	}
	else 
		LoadString (g_hinst, IDS_INVALIDDATE, sz, iLen);
}


//	______________________________________________________
//
//	get the IP and hostname of this computer

VOID
PNGetLocalHostInfo (
		DWORD*		pdwAddress,
		LPSTR		psz, 
		INT			iLen) 
{
	if (!psz || (iLen == 0))
		return;

	psz[0] = '\0';

	if (pdwAddress)
		*pdwAddress = 0;

	if (!g_bWinsock)
		return;

	if (gethostname (psz, iLen) != SOCKET_ERROR)
	{
		struct hostent* phe = gethostbyname (psz);

		if (phe != NULL)
		{
			// local ip address found
			struct in_addr ia;
			ia.S_un.S_addr = 
				((struct in_addr*)(phe->h_addr_list[0]))->s_addr;
			if (pdwAddress)
			{
				*pdwAddress = ia.S_un.S_addr;
				lstrcpyn (psz, inet_ntoa (ia), iLen);
			}
			else
			{
				lstrcpyn (psz, phe->h_name, iLen);
			}
		}
	}
}


//	______________________________________________________
//
//	get the IP of the specified host

BOOL
PNGetRemoteHostIP (
		LPSTR		psz, 
		DWORD*		pdwAddress)
{
	struct hostent* phe;

	if (!psz || !pdwAddress)
		return FALSE;

	*pdwAddress = 0;

	if (!g_bWinsock)
		return FALSE;

	phe = gethostbyname (psz);

	if (phe != NULL)
	{
		// ip address found
		struct in_addr ia;
		ia.S_un.S_addr = 
			((struct in_addr*)(phe->h_addr_list[0]))->s_addr;

		*pdwAddress = ia.S_un.S_addr;

		return TRUE;
	}
	else
		return FALSE;
}


//	____________________________________
//
//	get confirmed passphrase for use as shared secret

BOOL
PNGetSharedSecret (
		HWND	hwnd,
		LPSTR	pszSecret,
		UINT	uMaxSecretLength)
{
	PGPError			err				= kPGPError_NoErr;
	CHAR				szPrompt[128];
	LPSTR				psz;
	PGPOptionListRef	optionlist;

	if (g_AOS.pnconfig.bWarnReSharedSecret)
	{
		PNMessageBox (hwnd, IDS_CAPTION, IDS_SHAREDSECRETWARNING,
				MB_OK|MB_ICONEXCLAMATION);

		g_AOS.pnconfig.bWarnReSharedSecret = FALSE;
		PNSaveConfiguration (hwnd, &g_AOS.pnconfig, FALSE);
	}

	LoadString (g_hinst, IDS_SHAREDSECRETPROMPT, szPrompt, sizeof(szPrompt));

	err = PGPBuildOptionList (g_context, &optionlist,
			PGPOUIDialogPrompt (g_context, szPrompt),
			PGPOUIOutputPassphrase (g_context, &psz),
			PGPOUIParentWindowHandle (g_context, hwnd),
			PGPOUIMinimumPassphraseLength (g_context, MIN_SECRET_LENGTH),
			PGPOUIMinimumPassphraseQuality (g_context, MIN_SECRET_QUALITY),
			PGPOLastOption (g_context));

	err = PGPConfirmationPassphraseDialog (g_context, 
					optionlist, PGPOLastOption (g_context));
	PGPFreeOptionList (optionlist);

	if (IsntPGPError (err))
	{
		if (psz) 
		{
			lstrcpyn (pszSecret, psz, uMaxSecretLength+1);
			PGPFreeData (psz);
		}
		return TRUE;
	}
	else 
		return FALSE;

}


//	_____________________________________
//
//	putup error messagebox (if necessary)

PGPError 
PNErrorBox (
		HWND			hwnd,
		PGPError		errIn) 
{
	return PGPclErrorBox (hwnd, errIn);
}

//	______________________________________________
//
//  create a shortcut file

static HRESULT 
sCreateFileLink (
		LPCSTR	pszPathFile, 
		LPSTR	pszPathLink, 
		LPSTR	pszDescription) 
{
	HRESULT		hres; 
    IShellLink*	psl;     

	// Get a pointer to the IShellLink interface. 
	hres = CoInitialize (NULL);
	hres = CoCreateInstance (&CLSID_ShellLink, NULL, 
						CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl); 
	if (SUCCEEDED (hres)) 
	{         
		IPersistFile* ppf;  

		psl->lpVtbl->SetPath (psl, pszPathFile); 
        psl->lpVtbl->SetDescription (psl, pszDescription);  

		hres = psl->lpVtbl->QueryInterface (psl, &IID_IPersistFile, &ppf);          
		if (SUCCEEDED (hres)) 
		{ 
			WORD wsz[MAX_PATH];
			MultiByteToWideChar (CP_ACP, 0, pszPathLink, -1, wsz, MAX_PATH);

			hres = ppf->lpVtbl->Save (ppf, wsz, TRUE); 
			ppf->lpVtbl->Release (ppf);         
		} 
        psl->lpVtbl->Release (psl);     
	}     

	CoUninitialize ();
	return hres; 
} 


//	___________________________________________________
//
//  Set the flags based on if we have good authentication keys

VOID
PNSetAuthKeyFlags (
		PGPContextRef	context,
		PPNCONFIG		ppnconfig,
		PBOOL			pbPGPKey,
		PBOOL			pbX509Key)
{
	PGPKeyRef	keyPGP;
	PGPKeyRef	keyX509;
	PGPSigRef	sigX509;
	PGPBoolean	b;

	*pbPGPKey = FALSE;
	*pbX509Key = FALSE;

	PGPnetGetConfiguredAuthKeys (g_context, &g_AOS.pnconfig, 
					g_AOS.keysetMain, &keyPGP, &keyX509, &sigX509);

	if (ppnconfig->uPGPAuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		if (PGPKeyRefIsValid (keyPGP))
		{
			PGPGetKeyBoolean (keyPGP, kPGPKeyPropCanSign, &b);
			if (b)
			{
				PGPGetKeyBoolean (keyPGP, kPGPKeyPropIsSecretShared, &b);
				if (!b)
					*pbPGPKey = TRUE;
			}
		}
	}

	if (ppnconfig->uX509AuthKeyAlg != kPGPPublicKeyAlgorithm_Invalid)
	{
		if (PGPKeyRefIsValid (keyX509) &&
			PGPSigRefIsValid (sigX509))
		{
			PGPGetKeyBoolean (keyX509, kPGPKeyPropCanSign, &b);
			if (b)
			{
				PGPGetKeyBoolean (keyX509, kPGPKeyPropIsSecretShared, &b);
				if (!b)
					*pbX509Key = TRUE;
			}
		}
	}
}


//	___________________________________________________
//
//  Determine the appropriate icon for a key, based on
//	its properties

INT 
PNDetermineKeyIcon (
		PGPKeyRef	Key, 
		BOOL*		pbItalics) 
{

	PGPBoolean bRevoked, bSecret, bDisabled, bExpired, bSplit;
	PGPBoolean b, bRSADisabled;
	PGPUInt32 iIdx, iAlg;

	bRSADisabled = FALSE;
	PGPGetKeyBoolean (Key, kPGPKeyPropIsRevoked, &bRevoked);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsDisabled, &bDisabled);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsExpired, &bExpired);
	PGPGetKeyBoolean (Key, kPGPKeyPropIsSecretShared, &bSplit);
	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, &iAlg);

	if (iAlg == kPGPPublicKeyAlgorithm_RSA) {
		if (bSecret) {
			PGPGetKeyBoolean (Key, kPGPKeyPropCanSign, &b);
			if (!b) bRSADisabled = TRUE;
			if (bRevoked) iIdx = IDX_RSASECREVKEY;
			else if (bExpired) iIdx = IDX_RSASECEXPKEY;
			else if (bDisabled) iIdx = IDX_RSASECDISKEY;
			else if (bSplit) iIdx = IDX_RSASECSHRKEY;
			else iIdx = IDX_RSASECKEY;
		}
		else {
			PGPGetKeyBoolean (Key, kPGPKeyPropCanEncrypt, &b);
			if (!b) bRSADisabled = TRUE;
			if (bRevoked) iIdx = IDX_RSAPUBREVKEY;
			else if (bExpired) iIdx = IDX_RSAPUBEXPKEY;
			else if (bDisabled) iIdx = IDX_RSAPUBDISKEY;
			else iIdx = IDX_RSAPUBKEY;
		}
	}
	// DSA/ElGamal
	else {
		if (bSecret) {
			if (bRevoked) iIdx = IDX_DSASECREVKEY;
			else if (bExpired) iIdx = IDX_DSASECEXPKEY;
			else if (bDisabled) iIdx = IDX_DSASECDISKEY;
			else if (bSplit) iIdx = IDX_DSASECSHRKEY;
			else iIdx = IDX_DSASECKEY;
		}
		else {
			if (bRevoked) iIdx = IDX_DSAPUBREVKEY;
			else if (bExpired) iIdx = IDX_DSAPUBEXPKEY;
			else if (bDisabled) iIdx = IDX_DSAPUBDISKEY;
			else iIdx = IDX_DSAPUBKEY;
		}
	}

	if (pbItalics) 
		*pbItalics = bRevoked || bExpired || bDisabled || bRSADisabled;
	return iIdx;
}


//	___________________________________________________
//
//  Determine the appropriate icon for a cert, based on
//	its properties

INT 
PNDetermineCertIcon (
		PGPSigRef	cert, 
		BOOL*		pbItalics) 
{
	PGPBoolean	bRevoked, bVerified, bTried, bExpired;
	PGPBoolean	bNotCorrupt, bExportable, bX509;
	PGPUInt32	uTrustLevel;
	INT			idx;

	PGPGetSigBoolean (cert, kPGPSigPropIsRevoked, &bRevoked);
	PGPGetSigBoolean (cert, kPGPSigPropIsExpired, &bExpired);
	PGPGetSigBoolean (cert, kPGPSigPropIsVerified, &bVerified);
	PGPGetSigBoolean (cert, kPGPSigPropIsTried, &bTried);
	PGPGetSigBoolean (cert, kPGPSigPropIsNotCorrupt, &bNotCorrupt);
	PGPGetSigBoolean (cert, kPGPSigPropIsExportable, &bExportable);
	PGPGetSigBoolean (cert, kPGPSigPropIsX509, &bX509);
	PGPGetSigNumber  (cert, kPGPSigPropTrustLevel, &uTrustLevel);

	if (bX509) {
		if (bRevoked)
			idx = IDX_X509REVCERT;
		else if (bExpired)
			idx = IDX_X509EXPCERT;
		else
			idx = IDX_X509CERT;
	}
	else if (bRevoked) 
		idx = IDX_REVCERT;
	else if (bExpired) 
		idx = IDX_EXPCERT;
	else if (bVerified) {
		if (bExportable) {
			if (uTrustLevel == 1) 
				idx = IDX_TRUSTEDCERT;
			else 
				idx = IDX_EXPORTCERT;
		}
		else {
			if (uTrustLevel == 2) 
				idx = IDX_METACERT;
			else 
				idx = IDX_CERT;
		}
	}
	else if (bTried) 
		idx = IDX_BADCERT;
	else if (bNotCorrupt) {
		if (bExportable) 
			idx = IDX_EXPORTCERT;
		else 
			idx = IDX_CERT;
	}
	else 
		idx = IDX_BADCERT;

	if ((idx == IDX_BADCERT) || (idx == IDX_REVCERT) || (idx == IDX_EXPCERT)) 
	{
		if (pbItalics)
			*pbItalics = TRUE;
	}

	return idx;
}


//	___________________________________________________
//
//  save the configuration data and signal service

VOID 
PNSaveConfiguration (
		HWND			hwnd,
		PPNCONFIG		pPNConfig,
		BOOL			bKeyringsUpdated) 
{
	UINT uStatus;

	PGPnetSaveConfiguration (g_context, pPNConfig);
	if (bKeyringsUpdated)
	{
		PNSendServiceMessage (PGPNET_M_APPMESSAGE,
					PGPNET_CONFIGANDKEYRINGSUPDATED, (LPARAM)hwnd);
	}
	else
	{
		PNSendServiceMessage (PGPNET_M_APPMESSAGE,
					PGPNET_CONFIGUPDATED, (LPARAM)hwnd);
	}

	uStatus = PNDisplayStatusBarText ();
	if ((uStatus != PGPNET_DRIVERENABLED) && 
		(uStatus != PGPNET_DRIVERDISABLED) &&
		(hwnd != NULL))
	{
		PNMessageBox (hwnd, IDS_CAPTION, IDS_OPTIONNOSERVICE, 
						MB_OK|MB_ICONEXCLAMATION);
	}
}

//	____________________________________
//
//	save On/Off pref to file

VOID
PNSaveOnOffButtons (
		HWND			hwnd,
		PGPContextRef	context)
{
	PGPPrefRef			prefref			= kInvalidPGPPrefRef;
	PGPError			err;
	UINT				uStatus;

	if (IsDlgButtonChecked (hwnd, IDC_PGPNETON) == BST_CHECKED)
		g_AOS.pnconfig.bPGPnetEnabled = TRUE;
	else
		g_AOS.pnconfig.bPGPnetEnabled = FALSE;

	err = PGPnetOpenPrefs (PGPGetContextMemoryMgr (context), &prefref);
	if (IsntPGPError (err))
	{
		// save on/off value to pref file 
		PGPSetPrefBoolean (prefref, kPGPNetPrefEnablePGPnet, 
				g_AOS.pnconfig.bPGPnetEnabled); 
		PGPnetClosePrefs (prefref, TRUE);

		PNSendServiceMessage (
					PGPNET_M_APPMESSAGE, PGPNET_CONFIGUPDATED, (LPARAM)hwnd);

		uStatus = PNDisplayStatusBarText ();
		if ((uStatus != PGPNET_DRIVERENABLED) &&
			(uStatus != PGPNET_DRIVERDISABLED))
		{
			PNMessageBox (hwnd, IDS_CAPTION, IDS_OPTIONNOSERVICE, 
							MB_OK|MB_ICONEXCLAMATION);
		}

		// signal tray app to reload from registry
		PNSendTrayAppMessage (
					PGPNET_M_APPMESSAGE, PGPNET_CONFIGUPDATED, (LPARAM)hwnd);
	}
}


