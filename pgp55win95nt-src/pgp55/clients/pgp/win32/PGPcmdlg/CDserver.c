/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMserver.c - handle communication with keyserver
	

	$Id: CDserver.c,v 1.33.4.1 1997/11/28 14:12:30 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpcdlgx.h"
#include "resource.h"
#include "pgpkeyserver.h"
#include "pgpkeyserverprefs.h"
#include "pgpkeyservertypes.h"

#include "pgpBuildFlags.h"


#define LEDTIMER		111L
#define LEDTIMERPERIOD	100L

#define NUMLEDS 10
#define LEDSPACING 2

#if PGP_BUSINESS_SECURITY && PGP_ADMIN_BUILD
#define	SERVER_ACCESS_TYPE	kPGPKSAccess_Administrator
#else
#define	SERVER_ACCESS_TYPE	kPGPKSAccess_Normal
#endif


// external globals
extern HINSTANCE g_hInst;

typedef struct {
	PGPContextRef			context;
	HWND					hwnd;
	LPTHREAD_START_ROUTINE	lpThread;
	BOOL					bSearchInProgress;
	BOOL					bCancel;
	BOOL					bThreadFree;
	BOOL					bAlreadyAsked;
	INT						iStatusValue;
	INT						iStatusDirection;
	PGPError				err;
	PGPKeySetRef			keysetSigning;
	PGPKeySetRef			keysetIn;
	PGPKeySetRef			keysetOut;
	PGPKeyID*				pkeyidList;
	INT						iNumKeyIDs;
	PGPKeyServerKeySpace	space;
	LPSTR					pszUserID;
	CHAR					szServer[128];
} SERVERTHREADSTRUCT, *PSERVERTHREADSTRUCT;


// local globals
 


//	___________________________________________________
//
//	Get single key from keyserver

PGPError ServerEventHandler(PGPContextRef context,
							PGPEvent *event, 
							PGPUserValue userValue)
{
	PSERVERTHREADSTRUCT psts = (PSERVERTHREADSTRUCT)userValue;
	PGPError err = kPGPError_NoErr;
	BOOL bUserCancel = FALSE;

	if (psts->bCancel) {
		return (kPGPError_UserAbort);
	}

	switch (event->type) {

	case kPGPEvent_KeyServerEvent:
		{
			INT						ids		= 0;
			PGPEventKeyServerData	data	= event->data.keyServerData;

			PGPcdSERVEREVENT		serverevent;

			memset(&serverevent, 0x00, sizeof(event));
	
			serverevent.nmhdr.hwndFrom = (HWND)psts->hwnd;
			serverevent.nmhdr.idFrom = 0;
			serverevent.nmhdr.code = PGPCOMDLG_SERVERPROGRESS;
			serverevent.cancel = FALSE;
			serverevent.step = PGPCOMDLG_SERVERINFINITE;
			serverevent.total = PGPCOMDLG_SERVERINFINITE;
	
			switch (data.state) {
			case kPGPKeyServerStateConnect : ids = IDS_CONNECTING;	break;
			case kPGPKeyServerStateWait :	 ids = IDS_WAITING;		break;
			case kPGPKeyServerStateReceive : ids = IDS_RECEIVING;	break;
			case kPGPKeyServerStateSend :	 ids = IDS_SENDING;		break;
			}

			if (ids) {
				LoadString (g_hInst, ids, serverevent.szmessage,
							sizeof(serverevent.szmessage));
				lstrcat (serverevent.szmessage, psts->szServer);
			}

			SendMessage (psts->hwnd, WM_NOTIFY, 
								(WPARAM)(psts->hwnd), (LPARAM)&serverevent);
			bUserCancel = serverevent.cancel;

			break;
		}

	case kPGPEvent_KeyServerSignEvent:
		{
			LPSTR		pszPhrase		= NULL;
			CHAR		sz[64];
			PGPKeyRef	keySigning;
			INT			iLen;

			if (!psts->bCancel) {
				LoadString (g_hInst, IDS_PHRASEPROMPT, sz, sizeof(sz));

				err = PGPGetSignCachedPhrase (psts->context, psts->hwnd,
						sz, psts->bAlreadyAsked, &pszPhrase, 
						psts->keysetSigning, &keySigning, NULL, NULL, 0);
				psts->bAlreadyAsked = TRUE;
						
				if (IsntPGPError(err)) {
					iLen = lstrlen (pszPhrase);				
					err = PGPAddJobOptions (event->job,
							PGPOSignWithKey (
								psts->context, 
								keySigning, 
								PGPOPassphraseBuffer (
									psts->context, 
									pszPhrase, 
									iLen),
								PGPOLastOption (psts->context)),
							PGPOClearSign (psts->context, TRUE),
							PGPOLastOption (psts->context));
				}

				// burn and free phrase
				if (pszPhrase) PGPFreePhrase (pszPhrase);
			}
		}
	}

	if (psts->bCancel || bUserCancel) {
		err = kPGPError_UserAbort;
	}

	return err;
}

//	___________________________________________________
//
//	Get single key from keyserver

PGPError 
SearchForSingleKey (PSERVERTHREADSTRUCT	psts,
					PGPPrefRef		prefref,
					LPSTR			szUserID,
					PGPKeyID*		pkeyid,
					PGPKeySetRef*	pkeyset)
{
	PGPKeyServerEntry*		keyserverPath	= NULL;
	PGPUInt32				serverCount		= 0;
	PGPUInt32				index			= 0;
	PGPFilterRef			filter			= kPGPInvalidRef; 
	PGPError				err				= kPGPError_NoErr;
	CHAR					szServer[256];
	CHAR					szDomain[256];
	PGPUInt32				u;
	PGPcdSERVEREVENT		event;

	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = psts->hwnd;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCOMDLG_SERVERPROGRESS;
	event.pData = NULL;
	event.cancel = FALSE;
	event.step = PGPCOMDLG_SERVERINFINITE;
	event.total = PGPCOMDLG_SERVERINFINITE;
	
	*pkeyset = kPGPInvalidRef;

	if (szUserID) {
		szDomain[0] = '\0';
		PGPFindEmailDomain (szUserID, szDomain);
		err = PGPCreateKeyServerPath (prefref, szDomain, &keyserverPath,
									&serverCount);
	}
	else {
		u = sizeof(szServer);
		err = PGPGetDefaultKeyServer (prefref, &u, szServer);
		serverCount = 1;
	}


	if (IsntPGPError (err)) {
		if (pkeyid)
			err = PGPNewKeyIDFilter (psts->context, pkeyid, &filter);
		else
			err = PGPNewUserIDStringFilter (psts->context, szUserID, 
									kPGPMatchSubString,
									&filter);
		if (IsntPGPError (err)) {

			for (index=0; index<serverCount; index++) {
				PGPKeyServerRef server;

				if (keyserverPath) {
					lstrcpyn (psts->szServer, 
							  keyserverPath[index].serverURL,
							  sizeof(psts->szServer));
				}
				else {
					lstrcpyn (psts->szServer, 
							  szServer,
							  sizeof(psts->szServer));
				}

				// send notification of what's going on
				LoadString (g_hInst, IDS_LOOKINGFORSERVER, event.szmessage,
								sizeof(event.szmessage));
				lstrcat (event.szmessage, psts->szServer);
				SendMessage (psts->hwnd, WM_NOTIFY, (WPARAM)(psts->hwnd), 
								(LPARAM)&event);

				// open server and do search
				err = PGPNewKeyServerFromURL (psts->context, 
											  psts->szServer,
											  SERVER_ACCESS_TYPE,
											  psts->space,
											  &server);

				if (IsntPGPError (err)) {

					err = PGPKeyServerOpen (server);

					if (IsntPGPError (err)) {
						err = PGPQueryKeyServer (server, filter, 
											 ServerEventHandler, psts,
											 pkeyset, NULL);
						PGPKeyServerClose (server);
					}

					PGPFreeKeyServer (server);

					if (psts->bCancel) {
						err = kPGPError_UserAbort;
						break;
					}

					if (IsntPGPError (err)) {
						err = PGPCountKeys(*pkeyset, &u);
						if (u > 0) break;
						else {
							PGPFreeKeySet (*pkeyset);
							*pkeyset = kPGPInvalidRef;
						}
					}
				}
			}

			PGPFreeFilter (filter);
		}

		if (keyserverPath) PGPDisposeKeyServerPath (keyserverPath);
	}

	return err;
}


//	___________________________________________________
//
//	Send single key to first keyserver in keyserver path

PGPError 
SendSingleKey (PSERVERTHREADSTRUCT	psts,
					PGPPrefRef		prefref,
					LPSTR			szUserID,
					PGPKeyRef		key)
{
	PGPKeyServerEntry*		keyserverPath	= NULL;
	PGPUInt32				serverCount		= 0;
	PGPError				err				= kPGPError_NoErr;
	PGPKeySetRef			keysetFailed	= kPGPInvalidRef;	
	CHAR					szDomain[256];
	PGPUInt32				u;
	PGPcdSERVEREVENT		event;
	PGPKeySetRef			keyset;

	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = psts->hwnd;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCOMDLG_SERVERPROGRESS;
	event.pData = NULL;
	event.cancel = FALSE;
	event.step = PGPCOMDLG_SERVERINFINITE;
	event.total = PGPCOMDLG_SERVERINFINITE;
	
	if (szUserID) {
		szDomain[0] = '\0';
		PGPFindEmailDomain (szUserID, szDomain);
		err = PGPCreateKeyServerPath (prefref, szDomain, &keyserverPath,
									&serverCount);
		lstrcpy (psts->szServer, keyserverPath[0].serverURL);
		PGPDisposeKeyServerPath (keyserverPath);
	}
	else {
		u = sizeof(psts->szServer);
		err = PGPGetDefaultKeyServer (prefref, &u, psts->szServer);
		serverCount = 1;
	}

	if (IsntPGPError (err)) {
		PGPKeyServerRef server;

		// send notification of what's going on
		LoadString (g_hInst, IDS_LOOKINGFORSERVER, event.szmessage,
								sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
		SendMessage (psts->hwnd, WM_NOTIFY, (WPARAM)(psts->hwnd), 
								(LPARAM)&event);

		// open server and send key
		err = PGPNewKeyServerFromURL (psts->context, 
									  psts->szServer,
									  SERVER_ACCESS_TYPE,
									  psts->space,
									  &server);

		if (IsntPGPError (err)) {
			err = PGPNewSingletonKeySet (key, &keyset);

			if (IsntPGPError (err)) {
				err = PGPKeyServerOpen (server);
				if (IsntPGPError (err)) {
					err = PGPUploadToKeyServer (server, keyset, 
						ServerEventHandler, psts, &keysetFailed);
					if (keysetFailed) PGPFreeKeySet (keysetFailed);
					PGPKeyServerClose (server);
				}
				PGPFreeKeySet (keyset);
			}
		}
	}

	return err;
}


//	___________________________________________________
//
//	Send keyset to keyserver thread routine

DWORD WINAPI 
SendKeySetThreadRoutine (LPVOID lpvoid)
{
	PSERVERTHREADSTRUCT		psts		= (PSERVERTHREADSTRUCT)lpvoid;
	PGPPrefRef				prefref		= kPGPInvalidRef;
	HWND					hwndParent	= psts->hwnd;
	PGPError				err			= kPGPError_NoErr;
	PGPKeyRef				key			= kPGPInvalidRef;

	PGPcdSERVEREVENT		event;
	PGPKeySetRef			keysetFailed= kPGPInvalidRef;
	UINT					u;
	PGPKeyListRef			keylist;
	PGPKeyIterRef			keyiter;
	PGPKeyServerRef			server;
	CHAR					szUserID[kPGPMaxUserIDSize];
	BOOL					bThreadFree;
	
	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = hwndParent;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCOMDLG_SERVERPROGRESS;
	event.pData = NULL;
	event.cancel = FALSE;
	event.step = PGPCOMDLG_SERVERINFINITE;
	event.total = PGPCOMDLG_SERVERINFINITE;

	err = PGPKeyServerInit ();
	if (IsntPGPError (err)) {

		err = PGPcomdlgOpenClientPrefs (&prefref);
		if (IsntPGPError (err)) {

			// no server specified, send keys one-by-one to server
			// which is determined on basis of email domain
			if (!(psts->szServer[0])) {
				PGPOrderKeySet (psts->keysetIn, kPGPAnyOrdering, &keylist);
				PGPNewKeyIter (keylist, &keyiter);
				PGPKeyIterNext (keyiter, &key);

				while (PGPRefIsValid (key)) {	
					PGPGetPrimaryUserIDNameBuffer (key, 
										sizeof(szUserID), szUserID, &u);
					err = SendSingleKey (psts, prefref, szUserID, key);
///					if (IsPGPError (err)) ...
					PGPKeyIterNext (keyiter, &key);
				}

				PGPFreeKeyIter (keyiter);
				PGPFreeKeyList (keylist);
			}

			// server has been explicitly specified
			else {
				LoadString (g_hInst, IDS_LOOKINGFORSERVER, event.szmessage,
						sizeof(event.szmessage));
				lstrcat (event.szmessage, psts->szServer);
				SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
								(LPARAM)&event);
				err = PGPNewKeyServerFromURL (psts->context, 
											  psts->szServer,
											  SERVER_ACCESS_TYPE,
											  psts->space,
											  &server);
				if (IsntPGPError (err)) {

					err = PGPKeyServerOpen (server);
					if (IsntPGPError (err)) {
						err = PGPUploadToKeyServer (server, psts->keysetIn, 
							ServerEventHandler, psts, &keysetFailed);
						if (keysetFailed) PGPFreeKeySet (keysetFailed);
						PGPKeyServerClose (server);
					}

					PGPFreeKeyServer (server);
				}
			}

			PGPcomdlgCloseClientPrefs (prefref, FALSE);
		}

		PGPKeyServerCleanup ();
	}

	// let window know that we're done
	psts->err = err;
	event.szmessage[0] = '\0';
	if (err == kPGPError_UserAbort) {
		event.nmhdr.code = PGPCOMDLG_SERVERABORT;
		LoadString (g_hInst, IDS_SENDSERVERABORT, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else if (IsPGPError (err)) {
		event.nmhdr.code = PGPCOMDLG_SERVERERROR;
		LoadString (g_hInst, IDS_SENDSERVERERROR, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else {
		event.nmhdr.code = PGPCOMDLG_SERVERDONE;
		LoadString (g_hInst, IDS_SENDSERVEROK, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}

	bThreadFree = psts->bThreadFree;
	SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
								(LPARAM)&event);

	// if thread should free data structure, do it
	if (bThreadFree) {
		if (psts->keysetIn) PGPFreeKeySet (psts->keysetIn);
		HeapFree (GetProcessHeap(), 0, psts);
	}

	return 0x10;
}

//	___________________________________________________
//
//	Delete keyset from keyserver thread routine

DWORD WINAPI 
DeleteKeySetThreadRoutine (LPVOID lpvoid)
{
	PSERVERTHREADSTRUCT		psts		= (PSERVERTHREADSTRUCT)lpvoid;
	PGPPrefRef				prefref		= kPGPInvalidRef;
	HWND					hwndParent	= psts->hwnd;
	PGPError				err			= kPGPError_NoErr;
	PGPKeyRef				key			= kPGPInvalidRef;

	PGPcdSERVEREVENT		event;
	PGPKeySetRef			keysetFailed= kPGPInvalidRef;
	PGPKeyServerRef			server;
	BOOL					bThreadFree;
	
	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = hwndParent;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCOMDLG_SERVERPROGRESS;
	event.pData = NULL;
	event.cancel = FALSE;
	event.step = PGPCOMDLG_SERVERINFINITE;
	event.total = PGPCOMDLG_SERVERINFINITE;

	err = PGPKeyServerInit ();
	if (IsntPGPError (err)) {

		LoadString (g_hInst, IDS_LOOKINGFORSERVER, event.szmessage,
				sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
		SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
				(LPARAM)&event);

		err = PGPNewKeyServerFromURL (psts->context, 
									  psts->szServer,
									  SERVER_ACCESS_TYPE,
									  psts->space,
									  &server);

		if (IsntPGPError (err)) {
			err = PGPKeyServerOpen (server);
			if (IsntPGPError (err)) {
				err = PGPDeleteFromKeyServer (server, psts->keysetIn, 
					ServerEventHandler, psts, &keysetFailed);
				if (keysetFailed) PGPFreeKeySet (keysetFailed);
				PGPKeyServerClose (server);
			}
			PGPFreeKeyServer (server);
		}

		PGPKeyServerCleanup ();
	}

	// let window know that we're done
	psts->err = err;
	event.szmessage[0] = '\0';
	if (err == kPGPError_UserAbort) {
		event.nmhdr.code = PGPCOMDLG_SERVERABORT;
		LoadString (g_hInst, IDS_DELETESERVERABORT, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else if (IsPGPError (err)) {
		event.nmhdr.code = PGPCOMDLG_SERVERERROR;
		LoadString (g_hInst, IDS_DELETESERVERERROR, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else {
		event.nmhdr.code = PGPCOMDLG_SERVERDONE;
		LoadString (g_hInst, IDS_DELETESERVEROK, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}

	bThreadFree = psts->bThreadFree;
	SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
								(LPARAM)&event);

	// if thread should free data structure, do it
	if (bThreadFree) {
		if (psts->keysetIn) PGPFreeKeySet (psts->keysetIn);
		HeapFree (GetProcessHeap(), 0, psts);
	}

	return 0;
}

//	___________________________________________________
//
//	Disable keyset on keyserver thread routine

DWORD WINAPI 
DisableKeySetThreadRoutine (LPVOID lpvoid)
{
	PSERVERTHREADSTRUCT		psts		= (PSERVERTHREADSTRUCT)lpvoid;
	PGPPrefRef				prefref		= kPGPInvalidRef;
	HWND					hwndParent	= psts->hwnd;
	PGPError				err			= kPGPError_NoErr;
	PGPKeyRef				key			= kPGPInvalidRef;

	PGPcdSERVEREVENT		event;
	PGPKeySetRef			keysetFailed= kPGPInvalidRef;
	PGPKeyServerRef			server;
	BOOL					bThreadFree;

	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = hwndParent;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCOMDLG_SERVERPROGRESS;
	event.pData = NULL;
	event.cancel = FALSE;
	event.step = PGPCOMDLG_SERVERINFINITE;
	event.total = PGPCOMDLG_SERVERINFINITE;

	err = PGPKeyServerInit ();
	if (IsntPGPError (err)) {


		LoadString (g_hInst, IDS_LOOKINGFORSERVER, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
		SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
					(LPARAM)&event);

		err = PGPNewKeyServerFromURL (psts->context, 
									  psts->szServer,
									  SERVER_ACCESS_TYPE,
									  psts->space,
									  &server);

		if (IsntPGPError (err)) {
			err = PGPKeyServerOpen (server);
			if (IsntPGPError (err)) {
				err = PGPDisableFromKeyServer (server, psts->keysetIn, 
					ServerEventHandler, psts, &keysetFailed);
				if (keysetFailed) PGPFreeKeySet (keysetFailed);
				PGPKeyServerClose (server);
			}
			PGPFreeKeyServer (server);
		}

		PGPKeyServerCleanup ();
	}

	// let window know that we're done
	psts->err = err;
	event.szmessage[0] = '\0';
	if (err == kPGPError_UserAbort) {
		event.nmhdr.code = PGPCOMDLG_SERVERABORT;
		LoadString (g_hInst, IDS_DISABLESERVERABORT, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else if (IsPGPError (err)) {
		event.nmhdr.code = PGPCOMDLG_SERVERERROR;
		LoadString (g_hInst, IDS_DISABLESERVERERROR, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else {
		event.nmhdr.code = PGPCOMDLG_SERVERDONE;
		LoadString (g_hInst, IDS_DISABLESERVEROK, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}

	bThreadFree = psts->bThreadFree;
	SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
								(LPARAM)&event);

	// if thread should free data structure, do it
	if (bThreadFree) {
		if (psts->keysetIn) PGPFreeKeySet (psts->keysetIn);
		HeapFree (GetProcessHeap(), 0, psts);
	}

	return 0;
}

//	___________________________________________________
//
//	Update selected keys from keyserver thread routine

DWORD WINAPI 
UpdateKeySetThreadRoutine (LPVOID lpvoid)
{
	PSERVERTHREADSTRUCT		psts		= (PSERVERTHREADSTRUCT)lpvoid;
	PGPPrefRef				prefref		= kPGPInvalidRef;
	HWND					hwndParent	= psts->hwnd;
	PGPError				err			= kPGPError_NoErr;
	PGPKeyRef				key			= kPGPInvalidRef;
	BOOL					bDownloaded = FALSE;

	PGPcdSERVEREVENT		event;
	PGPKeySetRef			keyset, keysetDownloaded;
	PGPKeyListRef			keylist;
	PGPKeyIterRef			keyiter;
	PGPKeyID				keyid;
	CHAR					szUserID[kPGPMaxUserIDSize];
	UINT					u;
	BOOL					bThreadFree;
	
	err = PGPKeyServerInit ();
	if (IsntPGPError (err)) {

		err = PGPcomdlgOpenClientPrefs (&prefref);
		if (IsntPGPError (err)) {

			PGPNewKeySet (psts->context, &keysetDownloaded);
			if (PGPRefIsValid (keysetDownloaded)) {

				PGPOrderKeySet (psts->keysetIn, kPGPAnyOrdering, &keylist);
				PGPNewKeyIter (keylist, &keyiter);
				PGPKeyIterNext (keyiter, &key);

				while (PGPRefIsValid (key)) {	
					PGPGetKeyIDFromKey (key, &keyid);
					PGPGetPrimaryUserIDNameBuffer (key, 
											sizeof(szUserID), szUserID, &u);
					err = SearchForSingleKey (psts, prefref,
						szUserID, &keyid, &keyset);
					if (IsntPGPError (err) && keyset) {
						PGPAddKeys (keyset, keysetDownloaded);
						PGPCommitKeyRingChanges (keysetDownloaded);
						PGPFreeKeySet (keyset);
						bDownloaded = TRUE;
					}
					PGPKeyIterNext (keyiter, &key);
				}

				PGPFreeKeyIter (keyiter);
				PGPFreeKeyList (keylist);
			}
			PGPcomdlgCloseClientPrefs (prefref, FALSE);
		}

		PGPKeyServerCleanup ();
	}

	// let window know that we're done
	psts->err = err;

	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = hwndParent;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCOMDLG_SERVERPROGRESS;
	event.pData = NULL;
	event.cancel = FALSE;
	event.step = PGPCOMDLG_SERVERINFINITE;
	event.total = PGPCOMDLG_SERVERINFINITE;
	event.pData = NULL;
	event.szmessage[0] = '\0';
	if (err == kPGPError_UserAbort) {
		PGPFreeKeySet (keysetDownloaded);
		event.nmhdr.code = PGPCOMDLG_SERVERABORT;
		LoadString (g_hInst, IDS_SEARCHABORT, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else if (IsPGPError (err)) {
		PGPFreeKeySet (keysetDownloaded);
		event.nmhdr.code = PGPCOMDLG_SERVERERROR;
		LoadString (g_hInst, IDS_SEARCHERROR, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else {
		event.nmhdr.code = PGPCOMDLG_SERVERDONE;
		event.pData = keysetDownloaded;
		LoadString (g_hInst, IDS_SEARCHOK, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}

	bThreadFree = psts->bThreadFree;
	SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
								(LPARAM)&event);

	// if thread should free data structure, do it
	if (bThreadFree) {
		HeapFree (GetProcessHeap(), 0, psts);
	}

	return 0;
}

//	___________________________________________________
//
//	Update selected keys from keyserver thread routine

DWORD WINAPI 
UpdateKeyIDsThreadRoutine (LPVOID lpvoid)
{
	PSERVERTHREADSTRUCT		psts		= (PSERVERTHREADSTRUCT)lpvoid;
	PGPPrefRef				prefref		= kPGPInvalidRef;
	HWND					hwndParent	= psts->hwnd;
	PGPError				err			= kPGPError_NoErr;
	PGPKeyRef				key			= kPGPInvalidRef;
	BOOL					bDownloaded = FALSE;

	PGPKeyID				keyid;
	PGPcdSERVEREVENT		event;
	PGPKeySetRef			keyset, keysetDownloaded;
	INT						iKeyID;
	BOOL					bThreadFree;
	
	err = PGPKeyServerInit ();
	if (IsntPGPError (err)) {

		err = PGPcomdlgOpenClientPrefs (&prefref);
		if (IsntPGPError (err)) {

			PGPNewKeySet (psts->context, &keysetDownloaded);

			if (PGPRefIsValid (keysetDownloaded)) {

				iKeyID = 0;
				while ((iKeyID < psts->iNumKeyIDs) && (!psts->bCancel)) {	
					keyid = psts->pkeyidList[iKeyID];
					err = SearchForSingleKey (psts, prefref, 
												NULL, &keyid, &keyset);
					if (keyset != kPGPInvalidRef) {
						PGPAddKeys (keyset, keysetDownloaded);
						PGPCommitKeyRingChanges (keysetDownloaded);
						PGPFreeKeySet (keyset);
						bDownloaded = TRUE;
					}
					++iKeyID;
				}

			}
			PGPcomdlgCloseClientPrefs (prefref, FALSE);
		}

		PGPKeyServerCleanup ();
	}

	// let window know that we're done
	psts->err = err;

	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = hwndParent;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCOMDLG_SERVERPROGRESS;
	event.pData = NULL;
	event.cancel = FALSE;
	event.step = PGPCOMDLG_SERVERINFINITE;
	event.total = PGPCOMDLG_SERVERINFINITE;
	event.pData = NULL;
	event.szmessage[0] = '\0';
	if (err == kPGPError_UserAbort) {
		PGPFreeKeySet (keysetDownloaded);
		event.nmhdr.code = PGPCOMDLG_SERVERABORT;
		LoadString (g_hInst, IDS_SEARCHABORT, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else if (IsPGPError (err)) {
		PGPFreeKeySet (keysetDownloaded);
		event.nmhdr.code = PGPCOMDLG_SERVERERROR;
		LoadString (g_hInst, IDS_SEARCHERROR, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else {
		event.nmhdr.code = PGPCOMDLG_SERVERDONE;
		event.pData = keysetDownloaded;
		LoadString (g_hInst, IDS_SEARCHOK, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}

	bThreadFree = psts->bThreadFree;
	SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
								(LPARAM)&event);

	// if thread should free data structure, do it
	if (bThreadFree) {
		HeapFree (GetProcessHeap(), 0, psts);
	}

	return 0;
}

//	___________________________________________________
//
//	Update selected keys from keyserver thread routine

DWORD WINAPI 
SearchForUserIDThreadRoutine (LPVOID lpvoid)
{
	PSERVERTHREADSTRUCT		psts		= (PSERVERTHREADSTRUCT)lpvoid;
	PGPPrefRef				prefref		= kPGPInvalidRef;
	HWND					hwndParent	= psts->hwnd;
	PGPError				err			= kPGPError_NoErr;
	PGPKeyRef				key			= kPGPInvalidRef;
	BOOL					bDownloaded = FALSE;

	PGPcdSERVEREVENT		event;
	PGPKeySetRef			keyset, keysetDownloaded;
	BOOL					bThreadFree;
	
	err = PGPKeyServerInit ();
	if (IsntPGPError (err)) {

		err = PGPcomdlgOpenClientPrefs (&prefref);
		if (IsntPGPError (err)) {

			PGPNewKeySet (psts->context, &keysetDownloaded);

			if (PGPRefIsValid (keysetDownloaded)) {

				err = SearchForSingleKey (psts, prefref,
							psts->pszUserID, kPGPInvalidRef, &keyset);
				if (keyset) {
					PGPAddKeys (keyset, keysetDownloaded);
					PGPCommitKeyRingChanges (keysetDownloaded);
					PGPFreeKeySet (keyset);
					bDownloaded = TRUE;
				}
			}
			PGPcomdlgCloseClientPrefs (prefref, FALSE);
		}

		PGPKeyServerCleanup ();
	}

	// let window know that we're done
	psts->err = err;

	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = hwndParent;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCOMDLG_SERVERPROGRESS;
	event.pData = NULL;
	event.step = PGPCOMDLG_SERVERINFINITE;
	event.cancel = FALSE;
	event.total = PGPCOMDLG_SERVERINFINITE;
	event.pData = NULL;
	event.szmessage[0] = '\0';
	if (err == kPGPError_UserAbort) {
		PGPFreeKeySet (keysetDownloaded);
		event.nmhdr.code = PGPCOMDLG_SERVERABORT;
		LoadString (g_hInst, IDS_SEARCHABORT, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else if (IsPGPError (err)) {
		PGPFreeKeySet (keysetDownloaded);
		event.nmhdr.code = PGPCOMDLG_SERVERERROR;
		LoadString (g_hInst, IDS_SEARCHERROR, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}
	else {
		event.nmhdr.code = PGPCOMDLG_SERVERDONE;
		event.pData = keysetDownloaded;
		LoadString (g_hInst, IDS_SEARCHOK, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->szServer);
	}

	bThreadFree = psts->bThreadFree;
	SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
								(LPARAM)&event);

	// if thread should free data structure, do it
	if (bThreadFree) {
		HeapFree (GetProcessHeap(), 0, psts);
	}

	return 0;
}

//	____________________________________
//
//  Draw the "LED" progress indicator

VOID
DrawStatus (HWND hwnd,
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
//  Server progress dialog procedure

UINT CALLBACK 
ServerProgressDlgProc (
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
			DrawStatus (GetDlgItem (hDlg, IDC_PROGRESS), psts);
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

				case PGPCOMDLG_SERVERPROGRESS :
				{
					PPGPcdSERVEREVENT pEvent = (PPGPcdSERVEREVENT)lParam;
					if (!(psts->bSearchInProgress)) {
						psts->bSearchInProgress = TRUE;
						psts->iStatusValue = 0;
						if (pEvent->step == PGPCOMDLG_SERVERINFINITE) {
							psts->iStatusDirection = 1;
							SetTimer (hDlg, LEDTIMER, LEDTIMERPERIOD, NULL);
						}
						else {
							psts->iStatusDirection = 0;
							psts->iStatusValue = 0;
						}
					}
					else {
						if (pEvent->step != PGPCOMDLG_SERVERINFINITE) {
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

				case PGPCOMDLG_SERVERDONE : 
				case PGPCOMDLG_SERVERABORT :
				case PGPCOMDLG_SERVERERROR :
				{
					PPGPcdSERVEREVENT	pEvent = (PPGPcdSERVEREVENT)lParam;
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
//	send keyset to keyserver

PGPError PGPcdExport 
PGPcomdlgSendKeysToServerNotify (PGPContextRef context,
						   HWND hwndToNotify, 
						   LPSTR szServerURL,
						   PGPKeySetRef keysetSigning,
						   PGPKeySetRef keysetToSend)
{
	PSERVERTHREADSTRUCT psts;
	DWORD				dwThreadID;

	psts = (PSERVERTHREADSTRUCT)HeapAlloc (GetProcessHeap (), 
						0, sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		PGPNewKeySet (context, &(psts->keysetIn));
		PGPAddKeys (keysetToSend, psts->keysetIn);
		PGPCommitKeyRingChanges (psts->keysetIn);

		psts->context			= context;
		psts->keysetSigning		= keysetSigning;
		psts->hwnd				= hwndToNotify;
		psts->lpThread			= SendKeySetThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= TRUE;
		psts->bAlreadyAsked		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= kPGPKSKeySpaceDefault,
		psts->pkeyidList		= NULL;
		psts->iNumKeyIDs		= 0;
		psts->pszUserID			= NULL;

		if (szServerURL) lstrcpy (psts->szServer, szServerURL);
		else psts->szServer[0] = '\0';

		CreateThread (NULL, 0, SendKeySetThreadRoutine, 
							(void*)psts, 0, &dwThreadID);
	}

	return kPGPError_NoErr;
}


//	___________________________________________________
//
//	send keyset to keyserver

PGPError PGPcdExport 
PGPcomdlgSendKeysToServer (PGPContextRef context,
						   HWND hwndParent, 
						   LPSTR szServerURL,
						   PGPKeySetRef keysetSigning,
						   PGPKeySetRef keysetToSend)
{
	PSERVERTHREADSTRUCT	psts;
	PGPError			err;


	psts = (PSERVERTHREADSTRUCT)HeapAlloc (GetProcessHeap (), 
						0, sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		PGPNewKeySet (context, &(psts->keysetIn));
		PGPAddKeys (keysetToSend, psts->keysetIn);
		PGPCommitKeyRingChanges (psts->keysetIn);
	
		psts->context			= context;
		psts->keysetSigning		= keysetSigning;
		psts->lpThread			= SendKeySetThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->bAlreadyAsked		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= kPGPKSKeySpaceDefault,
		psts->pkeyidList		= NULL;
		psts->iNumKeyIDs		= 0;
		psts->pszUserID			= NULL;

		if (szServerURL) lstrcpy (psts->szServer, szServerURL);
		else psts->szServer[0] = '\0';

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, ServerProgressDlgProc, (LPARAM)psts);

		if (!psts->bThreadFree) {
			if (psts->keysetIn) PGPFreeKeySet (psts->keysetIn);
			HeapFree (GetProcessHeap (), 0, psts);
		}
	}

	return err;
}


//	___________________________________________________
//
//	send keyset to keyserver

PGPError PGPcdExport 
PGPcomdlgDisableKeysOnServer (PGPContextRef context,
						   HWND hwndParent, 
						   LPSTR szServerURL,
						   PGPKeyServerKeySpace space,
						   PGPKeySetRef keysetSigning,
						   PGPKeySetRef keysetToSend)
{ 
	PSERVERTHREADSTRUCT	psts;
	PGPError			err;


	psts = (PSERVERTHREADSTRUCT)HeapAlloc (GetProcessHeap (), 
						0, sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		PGPNewKeySet (context, &(psts->keysetIn));
		PGPAddKeys (keysetToSend, psts->keysetIn);
		PGPCommitKeyRingChanges (psts->keysetIn);
	
		psts->context			= context;
		psts->keysetSigning		= keysetSigning;
		psts->lpThread			= DisableKeySetThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->bAlreadyAsked		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= space;
		psts->pkeyidList		= NULL;
		psts->iNumKeyIDs		= 0;
		psts->pszUserID			= NULL;

		if (szServerURL) lstrcpy (psts->szServer, szServerURL);
		else psts->szServer[0] = '\0';

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, ServerProgressDlgProc, (LPARAM)psts);

		if (!psts->bThreadFree) {
			if (psts->keysetIn) PGPFreeKeySet (psts->keysetIn);
			HeapFree (GetProcessHeap (), 0, psts);
		}
	}

	return err;
}


//	___________________________________________________
//
//	send keyset to keyserver

PGPError PGPcdExport 
PGPcomdlgDeleteKeysFromServer (PGPContextRef context,
						   HWND hwndParent, 
						   LPSTR szServerURL,
						   PGPKeyServerKeySpace space,
						   PGPKeySetRef keysetSigning,
						   PGPKeySetRef keysetToSend)
{
	PSERVERTHREADSTRUCT	psts;
	PGPError			err;


	psts = (PSERVERTHREADSTRUCT)HeapAlloc (GetProcessHeap (), 
						0, sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		PGPNewKeySet (context, &(psts->keysetIn));
		PGPAddKeys (keysetToSend, psts->keysetIn);
		PGPCommitKeyRingChanges (psts->keysetIn);
	
		psts->context			= context;
		psts->keysetSigning		= keysetSigning;
		psts->lpThread			= DeleteKeySetThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->bAlreadyAsked		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= space;
		psts->pkeyidList		= NULL;
		psts->iNumKeyIDs		= 0;
		psts->pszUserID			= NULL;

		if (szServerURL) lstrcpy (psts->szServer, szServerURL);
		else psts->szServer[0] = '\0';

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, ServerProgressDlgProc, (LPARAM)psts);

		if (!psts->bThreadFree) {
			if (psts->keysetIn) PGPFreeKeySet (psts->keysetIn);
			HeapFree (GetProcessHeap (), 0, psts);
		}
	}

	return err;
}


//	___________________________________________________
//
//	Update keyset from keyserver

PGPError PGPcdExport 
PGPcomdlgUpdateKeySetFromServer (PGPContextRef context,
							     HWND hwndParent, 
							     PGPKeySetRef keysetToUpdate,
								 PGPKeySetRef* pkeysetUpdated)
{
	PSERVERTHREADSTRUCT	psts;
	PGPError			err;

	psts = (PSERVERTHREADSTRUCT)HeapAlloc (GetProcessHeap (), 
						0, sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		psts->context			= context;
		psts->keysetSigning		= NULL;
		psts->lpThread			= UpdateKeySetThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->bAlreadyAsked		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->keysetIn			= keysetToUpdate;
		psts->space				= kPGPKSKeySpaceDefault,
		psts->pkeyidList		= NULL;
		psts->iNumKeyIDs		= 0;
		psts->pszUserID			= NULL;

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, ServerProgressDlgProc, (LPARAM)psts);

		*pkeysetUpdated = psts->keysetOut;

		if (!psts->bThreadFree) {
			HeapFree (GetProcessHeap (), 0, psts);
		}
	}

	return err;
}


//	___________________________________________________
//
//	Update keyset from keyserver

PGPError PGPcdExport 
PGPcomdlgSearchServerForKeyIDs (PGPContextRef context,
							     HWND hwndParent, 
							     PGPKeyID* pkeyidList,
								 INT iNumKeyIDs,
								 PGPKeySetRef* pkeysetFound)
{	
	PSERVERTHREADSTRUCT	psts;
	PGPError			err;

	psts = (PSERVERTHREADSTRUCT)HeapAlloc (GetProcessHeap (), 
						0, sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		psts->context			= context;
		psts->keysetSigning		= NULL;
		psts->lpThread			= UpdateKeyIDsThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->bAlreadyAsked		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->keysetIn			= NULL;
		psts->space				= kPGPKSKeySpaceDefault,
		psts->pkeyidList		= pkeyidList;
		psts->iNumKeyIDs		= iNumKeyIDs;
		psts->pszUserID			= NULL;

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, ServerProgressDlgProc, (LPARAM)psts);

		*pkeysetFound = psts->keysetOut;

		if (!psts->bThreadFree) {
			HeapFree (GetProcessHeap (), 0, psts);
		}
	}

	return err;
}


//	___________________________________________________
//
//	search keyserver(s) for userid

PGPError PGPcdExport 
PGPcomdlgSearchServerForUserID (PGPContextRef context,
								HWND hwndParent, 
								LPSTR szUserID,
								PGPKeySetRef* pkeysetFound) 
{
	PSERVERTHREADSTRUCT	psts;
	PGPError			err;

	psts = (PSERVERTHREADSTRUCT)HeapAlloc (GetProcessHeap (), 
						0, sizeof(SERVERTHREADSTRUCT));

	if (psts) {
		psts->context			= context;
		psts->keysetSigning		= NULL;
		psts->lpThread			= SearchForUserIDThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->bAlreadyAsked		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->keysetIn			= NULL;
		psts->space				= kPGPKSKeySpaceDefault,
		psts->pkeyidList		= NULL;
		psts->iNumKeyIDs		= 0;
		psts->pszUserID			= szUserID;

		err = DialogBoxParam (g_hInst, MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, ServerProgressDlgProc, (LPARAM)psts);

		*pkeysetFound = psts->keysetOut;

		if (!psts->bThreadFree) {
			HeapFree (GetProcessHeap (), 0, psts);
		}
	}

	return err;
}


//	___________________________________________________
//
//	search keyserver(s) for userid

PGPError PGPcdExport 
PGPcomdlgFillServerComboBox (HWND hwndComboBox)
{
	PGPUInt32			uNumServers	= 0;
	INT					iDefault	= 0;

	PGPKeyServerEntry*	keyserverList;
	PGPPrefRef			prefref;
	PGPUInt32			u;
	PGPError			err;
	INT					idx;
	CHAR				sz[64];

	err = PGPcomdlgOpenClientPrefs (&prefref);
	if (IsPGPError (err)) return err;

	PGPGetKeyServerPrefs (prefref, &keyserverList, &uNumServers);

	if (uNumServers) {
		for (u=0; u<uNumServers; u++) {
			if (IsKeyServerListed (keyserverList[u].flags)) {
				idx = SendMessage (hwndComboBox, CB_ADDSTRING, 0, 
							(LPARAM)(keyserverList[u].serverURL));
				if (IsKeyServerDefault (keyserverList[u].flags))
					iDefault = idx;
			}
		}
		SendMessage (hwndComboBox, CB_SETCURSEL, iDefault, 0);
	}
	else {
		LoadString (g_hInst, IDS_NOSERVERS, sz, sizeof(sz));
		SendMessage (hwndComboBox, CB_ADDSTRING, 0, (LPARAM)sz);
		EnableWindow (hwndComboBox, FALSE);
	}

	if (keyserverList) PGPDisposePrefData (prefref, keyserverList);
	PGPcomdlgCloseClientPrefs (prefref, FALSE);

	return kPGPError_NoErr;
}


