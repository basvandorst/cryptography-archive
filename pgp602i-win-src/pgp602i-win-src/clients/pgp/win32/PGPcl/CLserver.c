/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLserver.c -	handle those types of communication with keyserver
					which are not handled by SDKuiLib
	

	$Id: CLserver.c,v 1.42 1998/08/12 17:32:16 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"
#include "pgpkmx.h"

// constant definitions
#define LEDTIMER		111L
#define LEDTIMERPERIOD	100L

#define NUMLEDS			10
#define LEDSPACING		2

#define SENDGROUPS		1
#define GETGROUPS		2
#define SENDKEY			3
#define	DISABLEKEY		4
#define DELETEKEY		5

#if PGP_BUSINESS_SECURITY && PGP_ADMIN_BUILD
#define	SERVER_ACCESS_TYPE	kPGPKeyServerAccessType_Administrator
#else
#define	SERVER_ACCESS_TYPE	kPGPKeyServerAccessType_Normal
#endif

// external globals
extern HINSTANCE g_hInst;

// typedefs
typedef struct {
	PGPContextRef			context;
	PGPtlsContextRef		tlsContext;
	HWND					hwnd;
	LPTHREAD_START_ROUTINE	lpThread;
	BOOL					bSearchInProgress;
	BOOL					bCancel;
	BOOL					bThreadFree;
	INT						iStatusValue;
	INT						iStatusDirection;
	UINT					uOperation;
	PGPError				err;
	PGPKeySetRef			keysetIn;
	PGPKeySetRef			keysetOut;
	PGPKeySetRef			keysetMain;
	PGPKeyServerKeySpace	space;
	PGPKeyServerEntry		ksentry;
	PGPKeyServerRef			server;
	PGPGroupSetRef			groupset;
} SERVERTHREADSTRUCT, *PSERVERTHREADSTRUCT;


//	___________________________________________________
//
//	update keyserver entry info in prefs

PGPError PGPclExport 
PGPclSyncKeyserverPrefs (
		PGPContextRef		context,
		PGPKeyServerEntry*	keyserver)
{
	PGPKeyServerEntry*	keyserverList		= NULL;
	PGPPrefRef			prefRef;
	PGPUInt32			keyserverCount;
	PGPError			err;

	// load keyserverprefs
	err = PGPclOpenClientPrefs (
				PGPGetContextMemoryMgr (context),
				&prefRef);

	if (IsntPGPError (err)) {
		err = PGPGetKeyServerPrefs (prefRef,
									&keyserverList,
									&keyserverCount);

		if (IsntPGPError (err)) {
			PGPUInt32	index;

			for (index = 0; index < keyserverCount; index++) {
				if (KeyServersAreEqual (keyserverList[index], *keyserver))
				{
					keyserverList[index].authAlg = keyserver->authAlg;
					lstrcpy (keyserverList[index].authKeyIDString, 
							keyserver->authKeyIDString);
					break;
				}
			}

			err = PGPSetKeyServerPrefs (prefRef,
									keyserverList,
									keyserverCount);

		}

		PGPclCloseClientPrefs (prefRef, TRUE);
	}

	if (keyserverList) 
		PGPDisposePrefData (prefRef, keyserverList);

	return err;
}

//	___________________________________________________
//
//	event handler for keyserver functions

static PGPError 
sServerEventHandler(
		PGPContextRef	context,
		PGPEvent*		pevent, 
		PGPUserValue	userValue)
{
	PSERVERTHREADSTRUCT		psts		= (PSERVERTHREADSTRUCT)userValue;
	PGPError				err			= kPGPError_NoErr;
	BOOL					bUserCancel = FALSE;
	BOOL					bSecure		= FALSE;

	if (psts->bCancel) {
		return kPGPError_UserAbort;
	}

	switch (pevent->type) {

	case kPGPEvent_KeyServerEvent:
		{
			INT						ids		= 0;
			PGPEventKeyServerData	data	= pevent->data.keyServerData;

			PGPclSERVEREVENT		serverevent;

			memset(&serverevent, 0x00, sizeof(PGPclSERVEREVENT));
	
			serverevent.nmhdr.hwndFrom = (HWND)psts->hwnd;
			serverevent.nmhdr.idFrom = 0;
			serverevent.nmhdr.code = PGPCL_SERVERPROGRESS;
			serverevent.cancel = FALSE;
			serverevent.step = PGPCL_SERVERINFINITE;
			serverevent.total = PGPCL_SERVERINFINITE;
	
			switch (data.state) {
			case kPGPKeyServerState_Opening : ids = IDS_CONNECTING;	break;
			case kPGPKeyServerState_Querying :	 ids = IDS_QUERYING;break;
			case kPGPKeyServerState_ProcessingResults :
											ids = IDS_PROCESSING;	break;
			case kPGPKeyServerState_Uploading :	
											ids = IDS_EXCHANGING;	break;
			case kPGPKeyServerState_Deleting : ids = IDS_DELETING;	break;
			case kPGPKeyServerState_Disabling : ids = IDS_DISABLING;break;
			case kPGPKeyServerState_Closing : ids = IDS_CLOSING;	break;
			}

			if (ids) {
				LoadString (g_hInst, ids, serverevent.szmessage,
							sizeof(serverevent.szmessage));
				lstrcat (serverevent.szmessage, psts->ksentry.serverDNS);
			}

			SendMessage (psts->hwnd, WM_NOTIFY, 
								(WPARAM)(psts->hwnd), (LPARAM)&serverevent);
			bUserCancel = serverevent.cancel;

			break;
		}

	case kPGPEvent_KeyServerSignEvent:
		{
			PGPKeyRef		keySigning	= kInvalidPGPKeyRef;
			PGPByte*		pPasskey	= NULL;
			PGPSize			sizePasskey;
			CHAR			sz[64];

			if (!psts->bCancel) {
				LoadString (g_hInst, IDS_PHRASEPROMPT, sz, sizeof(sz));

				err = KMGetSigningKeyPhrase (
					psts->context,		// in context
					NULL,				// in tlscontext
					psts->hwnd,			// in hwnd of parent
					sz,					// in prompt
					psts->keysetMain,	// in keyset
					FALSE,				// in reject split keys
					&keySigning,		// in/out signing key
					NULL,				// out phrase
					&pPasskey,			// out passkey buffer
					&sizePasskey);		// out passkey length

				if (IsntPGPError(err)) {
					err = PGPAddJobOptions (pevent->job,
							PGPOSignWithKey (
								psts->context, 
								keySigning, 
								(pPasskey) ?
									PGPOPasskeyBuffer (psts->context, 
										pPasskey, sizePasskey) :
									PGPONullOption (psts->context),
								PGPOLastOption (psts->context)),
							PGPOClearSign (psts->context, TRUE),
							PGPOLastOption (psts->context));
				}

				// burn and free passkey
				if (pPasskey)
					PGPFreeData (pPasskey);
			}
			break;
		}

	case kPGPEvent_KeyServerTLSEvent:
		err = CLHandleTLSEvent (context, pevent, psts->hwnd, 
							psts->keysetMain, &psts->ksentry, &bSecure);
		break;
	}

	if (psts->bCancel || bUserCancel) {
		err = kPGPError_UserAbort;
	}

	return err;
}

//	___________________________________________________
//
//	thread routine to handle all keyserver operations

static DWORD WINAPI 
sKeyserverThreadRoutine (LPVOID lpvoid)
{
	PSERVERTHREADSTRUCT			psts			= (PSERVERTHREADSTRUCT)lpvoid;
	PGPPrefRef					prefref			= kPGPInvalidRef;
	HWND						hwndParent		= psts->hwnd;
	PGPError					err				= kPGPError_NoErr;
	PGPKeyRef					key				= kInvalidPGPKeyRef;
	PGPKeySetRef				keysetFailed	= kInvalidPGPKeySetRef;
	PGPKeyServerAccessType		ksaccess		= SERVER_ACCESS_TYPE;

	PGPclSERVEREVENT				event;
	BOOL							bThreadFree;
	PGPKeyServerThreadStorageRef	previousStorage;
	
	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = hwndParent;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCL_SERVERPROGRESS;
	event.pData = NULL;
	event.cancel = FALSE;
	event.step = PGPCL_SERVERINFINITE;
	event.total = PGPCL_SERVERINFINITE;

	err = PGPKeyServerInit ();
	if (IsntPGPError (err)) {
		PGPKeyServerCreateThreadStorage(&previousStorage);

		// check for cases where we use root server exclusively
		if ((psts->uOperation == GETGROUPS) ||
			(psts->uOperation == SENDGROUPS) ||
			(psts->uOperation == SENDKEY))
		{
			err = PGPclOpenClientPrefs (
					PGPGetContextMemoryMgr (psts->context), 
					&prefref); CKERR;
			err = PGPGetRootKeyServer (prefref, &psts->ksentry); 
			PGPclCloseClientPrefs (prefref, FALSE);
			if (err == kPGPError_ItemNotFound)
			{
				PGPclMessageBox (hwndParent, IDS_CAPTION, 
						IDS_NOROOTSERVER, MB_OK|MB_ICONEXCLAMATION);
				err = kPGPError_UserAbort;
				goto done;
			}
		}

		// send message to update status text
		LoadString (g_hInst, IDS_LOOKINGFORSERVER, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->ksentry.serverDNS);
		SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
							(LPARAM)&event);

		// we need to set keyserver access type to admin to allow 
		// users to delete their own keys.
		if (psts->uOperation == DELETEKEY)
			ksaccess = kPGPKeyServerAccessType_Administrator;

		err = PGPNewKeyServerFromHostName (psts->context, 
										  psts->ksentry.serverDNS,
										  psts->ksentry.serverPort,
										  psts->ksentry.protocol,
										  ksaccess,
										  psts->space,
										  &psts->server);
		if (IsntPGPError (err)) {
			PGPtlsSessionRef tls = kInvalidPGPtlsSessionRef;

			PGPSetKeyServerEventHandler (psts->server, 
							sServerEventHandler, psts);

			if ((psts->ksentry.protocol == 
								kPGPKeyServerType_LDAPS) ||
				(psts->ksentry.protocol ==
								kPGPKeyServerType_HTTPS))
			{
				err = PGPNewTLSSession (psts->tlsContext, &tls);

				if (IsPGPError (err)) {
					// warn beyond the icon?
					tls = kInvalidPGPtlsSessionRef;
				}
			}

			// if disabling or deleting using TLS, then get local key
			if (((psts->uOperation == DISABLEKEY) ||
				 (psts->uOperation == DELETEKEY) ||
				 (psts->uOperation == SENDGROUPS)) &&
				 PGPtlsSessionRefIsValid (tls)) 
			{
				PGPKeyRef	keyAuth			= kInvalidPGPKeyRef;
				PGPByte*	pPasskey		= NULL;
				LPSTR		pszPhrase		= NULL;
				PGPSize		sizePasskey;
				CHAR		sz[64];

				LoadString (g_hInst, IDS_PHRASEPROMPT, sz, sizeof(sz));
			
				err = KMGetSigningKeyPhrase (
					psts->context,		// in context
					NULL,				// in tlscontext
					psts->hwnd,			// in hwnd of parent
					sz,					// in prompt
					psts->keysetMain,	// in keyset
					FALSE,				// in reject split keys
					&keyAuth,			// in/out signing key
					&pszPhrase,			// out phrase
					&pPasskey,			// out passkey buffer
					&sizePasskey);		// out passkey length

				if (IsntPGPError (err)) {
					if (pszPhrase)
						err = PGPtlsSetLocalPrivateKey (tls, keyAuth,
							PGPOPassphrase (psts->context, pszPhrase),
							PGPOLastOption (psts->context));
					else
						err = PGPtlsSetLocalPrivateKey (tls, keyAuth,
							(pPasskey) ?
								PGPOPasskeyBuffer (psts->context, 
									pPasskey, sizePasskey) :
								PGPONullOption (psts->context),
							PGPOLastOption (psts->context));
				}

				// burn and free phrase
				if (IsntNull (pszPhrase))
					PGPclFreePhrase (pszPhrase);
				if (pPasskey)
					PGPFreeData (pPasskey);
			}

			// perform the keyserver operation
			if (IsntPGPError (err)) {
				err = PGPKeyServerOpen (psts->server, tls);
				if (IsntPGPError (err)) {

					switch (psts->uOperation) {
					case GETGROUPS :
						err = PGPRetrieveGroupsFromServer (psts->server, 
										&psts->groupset); 
						break;

					case SENDGROUPS :
						err = PGPSendGroupsToServer (psts->server, 
										psts->groupset); 
						break;

					case SENDKEY :
						err = PGPUploadToKeyServer (psts->server, 
										psts->keysetIn, 
										&keysetFailed);
						if (PGPKeySetRefIsValid (keysetFailed)) 
							PGPFreeKeySet (keysetFailed);
						break;

					case DISABLEKEY :
						err = PGPDisableFromKeyServer (psts->server, 
										psts->keysetIn, 
										&keysetFailed);
						if (PGPKeySetRefIsValid (keysetFailed)) 
							PGPFreeKeySet (keysetFailed);
						break;

					case DELETEKEY :
						err = PGPDeleteFromKeyServer (psts->server, 
										psts->keysetIn, 
										&keysetFailed);
						if (PGPKeySetRefIsValid (keysetFailed)) 
							PGPFreeKeySet (keysetFailed);
						break;

					}
					PGPKeyServerClose (psts->server);
				}
			}

			PGPFreeKeyServer (psts->server);
			psts->server = kInvalidPGPKeyServerRef;

			if (PGPtlsSessionRefIsValid (tls)) 
				PGPFreeTLSSession (tls);
		}

		PGPKeyServerDisposeThreadStorage(previousStorage);

		PGPKeyServerCleanup ();
	}

done :
	// let window know that we're done
	psts->err = err;
	event.szmessage[0] = '\0';
	if (err == kPGPError_UserAbort) {
		event.nmhdr.code = PGPCL_SERVERABORT;
		LoadString (g_hInst, IDS_SERVERABORT, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->ksentry.serverDNS);
	}
	else if (IsPGPError (err)) {
		event.nmhdr.code = PGPCL_SERVERERROR;
		LoadString (g_hInst, IDS_SERVERERROR, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->ksentry.serverDNS);
	}
	else {
		event.nmhdr.code = PGPCL_SERVERDONE;
		LoadString (g_hInst, IDS_SERVEROK, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->ksentry.serverDNS);
	}

	bThreadFree = psts->bThreadFree;
	SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
								(LPARAM)&event);

	// if thread should free data structure, do it
	if (bThreadFree) {
		if (psts->keysetIn) PGPFreeKeySet (psts->keysetIn);
		free (psts);
	}

	return 0;
}


//	____________________________________
//
//  Draw the "LED" progress indicator

static VOID
sDrawStatus (
		HWND				hwnd,
		PSERVERTHREADSTRUCT psts) 
{
	HBRUSH			hBrushLit, hBrushUnlit, hBrushOld;
	HPEN			hPen, hPenOld;
	INT				i;
	INT				itop, ibot, ileft, iright, iwidth;
	PAINTSTRUCT		ps;
	HDC				hdc;
	RECT			rc;

	if (psts->iStatusValue < -1) return;

	hdc = BeginPaint (hwnd, &ps);

	// draw 3D shadow
	GetClientRect (hwnd, &rc);
	itop = rc.top+1;
	ibot = rc.bottom-2;

	iwidth = (rc.right-rc.left) / NUMLEDS;
	iwidth -= LEDSPACING;

	ileft = rc.left + 4;
	for (i=0; i<NUMLEDS; i++) {
		iright = ileft + iwidth;

		MoveToEx (hdc, ileft, ibot, NULL);
		LineTo (hdc, iright, ibot);
		LineTo (hdc, iright, itop);

		ileft += iwidth + LEDSPACING;
	}

	hPen = CreatePen (PS_SOLID, 0, RGB (128, 128, 128));
	hPenOld = SelectObject (hdc, hPen);
	hBrushLit = CreateSolidBrush (RGB (0, 255, 0));
	hBrushUnlit = CreateSolidBrush (RGB (0, 128, 0));

	ileft = rc.left + 4;

	// draw "Knight Rider" LEDs
	if (psts->iStatusDirection) {
		hBrushOld = SelectObject (hdc, hBrushUnlit);
		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i == psts->iStatusValue) {
				SelectObject (hdc, hBrushLit);
				Rectangle (hdc, ileft, itop, iright, ibot);
				SelectObject (hdc, hBrushUnlit);
			}
			else  {
				Rectangle (hdc, ileft, itop, iright, ibot);
			}
	
			ileft += iwidth + LEDSPACING;
		}
	}

	// draw "progress bar" LEDs
	else { 
		if (psts->iStatusValue >= 0) 
			hBrushOld = SelectObject (hdc, hBrushLit);
		else
			hBrushOld = SelectObject (hdc, hBrushUnlit);

		for (i=0; i<NUMLEDS; i++) {
			iright = ileft + iwidth;
	
			if (i > psts->iStatusValue) {
				SelectObject (hdc, hBrushUnlit);
			}
			Rectangle (hdc, ileft, itop, iright, ibot);
	
			ileft += iwidth + LEDSPACING;
		}
	}

	SelectObject (hdc, hBrushOld);
	SelectObject (hdc, hPenOld);
	DeleteObject (hPen);
	DeleteObject (hBrushLit);
	DeleteObject (hBrushUnlit);

	EndPaint (hwnd, &ps);
}

//	______________________________________________________
//
//  Server progress dialog procedure (used for disable and delete)

static UINT CALLBACK 
sServerProgressDlgProc (
		 HWND	hDlg, 
		 UINT	uMsg, 
		 WPARAM wParam,
		 LPARAM lParam) 
{

	switch (uMsg) {

	case WM_INITDIALOG:
		{
			PSERVERTHREADSTRUCT psts = (PSERVERTHREADSTRUCT)lParam;
			DWORD				dwThreadID;

			SetWindowLong (hDlg, GWL_USERDATA, lParam);

			psts->hwnd = hDlg;
			CreateThread (NULL, 0, psts->lpThread, 
							(void*)psts, 0, &dwThreadID);
			return TRUE;
		}

	case WM_PAINT :
		{
			PSERVERTHREADSTRUCT psts = 
				(PSERVERTHREADSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			sDrawStatus (GetDlgItem (hDlg, IDC_PROGRESS), psts);
			break;
		}

	case WM_TIMER :
		{
			PSERVERTHREADSTRUCT psts = 
				(PSERVERTHREADSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
			psts->iStatusValue += psts->iStatusDirection;
			if (psts->iStatusValue <= 0) {
				psts->iStatusValue = 0;
				psts->iStatusDirection = 1;
			}
			else if (psts->iStatusValue >= NUMLEDS-1) {
				psts->iStatusValue = NUMLEDS-1;
				psts->iStatusDirection = -1;
			}
			InvalidateRect (hDlg, NULL, FALSE);
			break;
		}

	case WM_NOTIFY :
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			PSERVERTHREADSTRUCT psts = 
				(PSERVERTHREADSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);

			if (pnmh->hwndFrom != hDlg) break;

			switch (pnmh->code) {

				case PGPCL_SERVERPROGRESS :
				{
					PPGPclSERVEREVENT pEvent = (PPGPclSERVEREVENT)lParam;
					if (!(psts->bSearchInProgress)) {
						psts->bSearchInProgress = TRUE;
						psts->iStatusValue = 0;
						if (pEvent->step == PGPCL_SERVERINFINITE) {
							psts->iStatusDirection = 1;
							SetTimer (hDlg, LEDTIMER, LEDTIMERPERIOD, NULL);
						}
						else {
							psts->iStatusDirection = 0;
							psts->iStatusValue = 0;
						}
					}
					else {
						if (pEvent->step != PGPCL_SERVERINFINITE) {
							psts->iStatusDirection = 0;
							psts->iStatusValue = (pEvent->step * 9) /
													pEvent->total;
							InvalidateRect (hDlg, NULL, FALSE);
						}
					}
					SetDlgItemText (hDlg, IDC_PROGRESSTEXT, 
											pEvent->szmessage);
					return FALSE;
				}

				case PGPCL_SERVERDONE : 
				case PGPCL_SERVERABORT :
				case PGPCL_SERVERERROR :
				{
					PPGPclSERVEREVENT	pEvent = (PPGPclSERVEREVENT)lParam;
					PGPError			err;

					KillTimer (hDlg, LEDTIMER);
					psts->keysetOut = (PGPKeySetRef)(pEvent->pData);
					err = psts->err;

					EndDialog (hDlg, err);
					return FALSE;
				}
			}
			break;
		}


	case WM_COMMAND :
		switch(LOWORD (wParam)) {
		case IDCANCEL :
			{
				PSERVERTHREADSTRUCT psts = 
					(PSERVERTHREADSTRUCT)GetWindowLong (hDlg, GWL_USERDATA);
				KillTimer (hDlg, LEDTIMER);
				psts->bThreadFree = TRUE;
				psts->bCancel = TRUE;
				if (psts->server) {
					PGPError err;

					err = PGPCancelKeyServerCall (psts->server);
					PGPclErrorBox (hDlg, err);
				}
				EndDialog (hDlg, kPGPError_UserAbort);
				break;
			}

		}
		return TRUE;
	}

	return FALSE;
}


//	___________________________________________________
//
//	update all groups from keyserver

PGPError PGPclExport 
PGPclGetGroupsFromRootServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain,
		PGPGroupSetRef*			pgroupsetDownloaded)
{
	PGPError			err		= kPGPError_NoErr;

	PSERVERTHREADSTRUCT psts;

	psts = (PSERVERTHREADSTRUCT)malloc (sizeof(SERVERTHREADSTRUCT));
	memset(psts,0x00,sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		psts->uOperation		= GETGROUPS;
		psts->context			= context;
		psts->tlsContext		= tlsContext;
		psts->keysetMain		= keysetMain;
		psts->hwnd				= hwndParent;
		psts->lpThread			= sKeyserverThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= kPGPKeyServerKeySpace_Default;
		psts->groupset			= kInvalidPGPGroupSetRef;

		ZeroMemory (&psts->ksentry, sizeof(PGPKeyServerEntry));

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, sServerProgressDlgProc, (LPARAM)psts);

		if (IsntPGPError (err)) {
			if (pgroupsetDownloaded)
				*pgroupsetDownloaded = psts->groupset;
		}

		if (!psts->bThreadFree) {
			free (psts);
		}
	}

	return err;
}


//	___________________________________________________
//
//	send all groups to keyserver

PGPError PGPclExport 
PGPclSendGroupsToRootServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain,
		PGPGroupSetRef			groupsetToSend)
{
	PGPError			err		= kPGPError_NoErr;

	PSERVERTHREADSTRUCT psts;

	psts = (PSERVERTHREADSTRUCT)malloc (sizeof(SERVERTHREADSTRUCT));
	memset(psts,0x00,sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		psts->uOperation		= SENDGROUPS;
		psts->context			= context;
		psts->tlsContext		= tlsContext;
		psts->keysetMain		= keysetMain;
		psts->hwnd				= hwndParent;
		psts->lpThread			= sKeyserverThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= kPGPKeyServerKeySpace_Default;
		psts->groupset			= groupsetToSend;

		ZeroMemory (&psts->ksentry, sizeof(PGPKeyServerEntry));

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, sServerProgressDlgProc, (LPARAM)psts);

		if (!psts->bThreadFree) {
			free (psts);
		}
	}

	return err;
}


//	___________________________________________________
//
//	send keyset to keyserver

PGPError PGPclExport 
PGPclSendKeysToRootServerNotify (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndToNotify, 
		PGPKeySetRef		keysetMain,
		PGPKeySetRef		keysetToSend)
{
	PSERVERTHREADSTRUCT psts;
	DWORD				dwThreadID;

	psts = (PSERVERTHREADSTRUCT)malloc (sizeof(SERVERTHREADSTRUCT));
	memset(psts,0x00,sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		PGPNewKeySet (context, &(psts->keysetIn));
		PGPAddKeys (keysetToSend, psts->keysetIn);
		PGPCommitKeyRingChanges (psts->keysetIn);

		psts->uOperation		= SENDKEY;
		psts->context			= context;
		psts->tlsContext		= tlsContext;
		psts->keysetMain		= keysetMain;
		psts->hwnd				= hwndToNotify;
		psts->lpThread			= NULL;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= TRUE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= kPGPKeyServerKeySpace_Default,
		psts->groupset			= kInvalidPGPGroupSetRef;

		ZeroMemory (&psts->ksentry, sizeof(PGPKeyServerEntry));

		CreateThread (NULL, 0, sKeyserverThreadRoutine, 
										(void*)psts, 0, &dwThreadID);
	}

	return kPGPError_NoErr;
}


//	___________________________________________________
//
//	disable keyset on keyserver

PGPError PGPclExport 
PGPclDisableKeysOnServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeyServerEntry*		pkeyserver,
		PGPKeyServerKeySpace	space,
		PGPKeySetRef			keysetMain,
		PGPKeySetRef			keysetToSend)
{ 
	PSERVERTHREADSTRUCT	psts;
	PGPError			err;


	psts = (PSERVERTHREADSTRUCT)malloc (sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		PGPNewKeySet (context, &(psts->keysetIn));
		PGPAddKeys (keysetToSend, psts->keysetIn);
		PGPCommitKeyRingChanges (psts->keysetIn);
	
		psts->uOperation		= DISABLEKEY;
		psts->context			= context;
		psts->tlsContext		= tlsContext;
		psts->keysetMain		= keysetMain;
		psts->lpThread			= sKeyserverThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= space;
		psts->groupset			= kInvalidPGPGroupSetRef;

		CopyMemory (&psts->ksentry, pkeyserver, sizeof(PGPKeyServerEntry));

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, sServerProgressDlgProc, (LPARAM)psts);

		if (!psts->bThreadFree) {
			if (psts->keysetIn) PGPFreeKeySet (psts->keysetIn);
			free (psts);
		}
	}

	return err;
}


//	___________________________________________________
//
//	delete keyset from keyserver

PGPError PGPclExport 
PGPclDeleteKeysFromServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeyServerEntry*		pkeyserver,
		PGPKeyServerKeySpace	space,
		PGPKeySetRef			keysetMain,
		PGPKeySetRef			keysetToSend)
{
	PSERVERTHREADSTRUCT	psts;
	PGPError			err;


	psts = (PSERVERTHREADSTRUCT)malloc (sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		PGPNewKeySet (context, &(psts->keysetIn));
		PGPAddKeys (keysetToSend, psts->keysetIn);
		PGPCommitKeyRingChanges (psts->keysetIn);
	
		psts->uOperation		= DELETEKEY;
		psts->context			= context;
		psts->tlsContext		= tlsContext;
		psts->keysetMain		= keysetMain;
		psts->lpThread			= sKeyserverThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= space;
		psts->groupset			= kInvalidPGPGroupSetRef;

		CopyMemory (&psts->ksentry, pkeyserver, sizeof(PGPKeyServerEntry));

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, sServerProgressDlgProc, (LPARAM)psts);

		if (!psts->bThreadFree) {
			if (psts->keysetIn) PGPFreeKeySet (psts->keysetIn);
			free (psts);
		}
	}

	return err;
}


