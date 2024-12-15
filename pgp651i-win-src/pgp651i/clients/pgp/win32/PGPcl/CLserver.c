/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLserver.c -	handle those types of communication with keyserver
					which are not handled by SDKuiLib
	

	$Id: CLserver.c,v 1.69 1999/03/27 00:05:41 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"
#include "pgpkmx.h"
#include "pgpKeyServer.h"

// constant definitions
#define	MAX_AV			10

#define LEDTIMER		111L
#define LEDTIMERPERIOD	100L

#define NUMLEDS			10
#define LEDSPACING		2

#define SENDGROUPS		1
#define GETGROUPS		2
#define SENDKEY			3
#define	DISABLEKEY		4
#define DELETEKEY		5

#define UPDATEREVOCATIONS	1
#define REQUESTCERTIFICATE	2
#define RETRIEVECERTIFICATE	3


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
	PGPKeyRef				keyIn;
	PGPKeySetRef			keysetIn;
	PGPKeySetRef			keysetOut;
	PGPKeySetRef			keysetMain;
	PGPKeyServerKeySpace	space;
	PGPKeyServerEntry		ksentry;
	PGPKeyServerRef			server;
	PGPGroupSetRef			groupset;
	PGPAttributeValue*		pAVlist;
	PGPUInt32				numAVs;
	LPSTR					pszPassPhrase;
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
			case kPGPKeyServerState_Opening : 
										ids = IDS_CONNECTING;	break;
			case kPGPKeyServerState_Querying :	 
										ids = IDS_QUERYING;		break;
			case kPGPKeyServerState_ReceivingResults :	 
										ids = IDS_RECEIVING;	break;
			case kPGPKeyServerState_ProcessingResults :
										ids = IDS_PROCESSING;	break;
			case kPGPKeyServerState_Uploading :	
										ids = IDS_EXCHANGING;	break;
			case kPGPKeyServerState_Deleting : 
										ids = IDS_DELETING;		break;
			case kPGPKeyServerState_Disabling : 
										ids = IDS_DISABLING;	break;
			case kPGPKeyServerState_Closing : 
										ids = IDS_CLOSING;		break;
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

	PGPKeyServerAccessType		ksaccess	= kPGPKeyServerAccessType_Normal;

	PGPclSERVEREVENT				event;
	BOOL							bThreadFree;
	PGPKeyServerThreadStorageRef	previousStorage;

#if PGP_BUSINESS_SECURITY
	if (PGPclIsAdminInstall ())
		ksaccess = kPGPKeyServerAccessType_Administrator;
#endif // PGP_BUSINESS_SECURITY
	
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
							kInvalidPGPSigRef, kInvalidPGPKeySetRef,
							PGPOPassphrase (psts->context, pszPhrase),
							PGPOLastOption (psts->context));
					else
						err = PGPtlsSetLocalPrivateKey (tls, keyAuth,
							kInvalidPGPSigRef, kInvalidPGPKeySetRef,
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


//	___________________________________________________
//
//	get the first key from a keyset

static PGPError
sGetFirstKeyInSet(
	PGPKeySetRef 	keyset,
	PGPKeyRef*		pkey)
{
	PGPError		err;
	PGPKeyListRef	keylist;
	
	*pkey = kInvalidPGPKeyRef;
	
	err = PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
	if (IsntPGPError (err))
	{
		PGPKeyIterRef	keyiter;
		
		err = PGPNewKeyIter (keylist, &keyiter);
		if (IsntPGPError (err))
		{
			err = PGPKeyIterNext (keyiter, pkey);
			if (err == kPGPError_EndOfIteration)
				err = kPGPError_ItemNotFound;
			
			PGPFreeKeyIter (keyiter);
		}
		
		PGPFreeKeyList (keylist);
	}

	return err;
}


//	___________________________________________________
//
//	thread routine to handle all CA server operations

static DWORD WINAPI 
sCAThreadRoutine (LPVOID lpvoid)
{
	PSERVERTHREADSTRUCT			psts			= (PSERVERTHREADSTRUCT)lpvoid;
	PGPPrefRef					prefref			= kPGPInvalidRef;
	HWND						hwndParent		= psts->hwnd;
	PGPError					err				= kPGPError_NoErr;
	PGPKeyRef					key				= kInvalidPGPKeyRef;
	PGPByte*					pPasskey		= NULL;
	LPSTR						pszPhrase		= NULL;
	PGPSize						sizePasskey		= 0;
	PGPUInt32					numAVItems		= 0;

	PGPKeyServerAccessType		ksaccess	= kPGPKeyServerAccessType_Normal;

	PGPclSERVEREVENT				event;
	BOOL							bThreadFree;
	PGPKeyServerThreadStorageRef	previousStorage;
	PGPContextRef					ctx;

	memset (&event, 0x00, sizeof(event));
	event.nmhdr.hwndFrom = hwndParent;
	event.nmhdr.idFrom = 0;
	event.nmhdr.code = PGPCL_SERVERPROGRESS;
	event.pData = psts->keysetOut;
	event.cancel = FALSE;
	event.step = PGPCL_SERVERINFINITE;
	event.total = PGPCL_SERVERINFINITE;

	err = PGPKeyServerInit ();
	if (IsntPGPError (err)) 
	{
		PGPKeyServerCreateThreadStorage (&previousStorage);

		// send message to update status text
		LoadString (g_hInst, IDS_LOOKINGFORSERVER, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->ksentry.serverDNS);
		SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
							(LPARAM)&event);

		ctx = psts->context;
		err = PGPNewKeyServer (ctx, 
					psts->ksentry.protocol,	//using protocol for class info
					&psts->server,
					PGPONetURL (ctx, psts->ksentry.serverDNS),
					PGPOKeyServerKeySpace (ctx, psts->space),
					PGPOKeyServerAccessType (ctx, ksaccess),
					PGPOLastOption (ctx));

		if (IsntPGPError (err)) 
		{
			PGPtlsSessionRef	tls		= kInvalidPGPtlsSessionRef;

			PGPSetKeyServerEventHandler (psts->server, 
							sServerEventHandler, psts);

			err = PGPNewTLSSession (psts->tlsContext, &tls);

			if (IsntPGPError (err) &&
				(PGPtlsSessionRefIsValid (tls)) &&
				(psts->uOperation == REQUESTCERTIFICATE))
			{
				sGetFirstKeyInSet (psts->keysetIn, &key);

				if (IsntPGPError (err) &&
					(psts->pszPassPhrase == NULL))
				{
					err = KMGetKeyPhrase (
							psts->context,		// in context
							psts->tlsContext,	// in tlscontext
							psts->hwnd,			// in hwnd of parent
							NULL,				// in prompt
							psts->keysetMain,	// in keyset
							key,				// in key
							&pszPhrase,			// out phrase
							&pPasskey,			// out passkey buffer
							&sizePasskey);		// out passkey length

					if (IsntPGPError (err))
						psts->pszPassPhrase = pszPhrase;
				}
			}

			// perform the CA server operation
			if (IsntPGPError (err)) 
			{
				err = PGPKeyServerOpen (psts->server, tls);
				if (IsntPGPError (err)) 
				{
					PVOID		pBuffer		= NULL;
					PGPSize		size;

					switch (psts->uOperation) {
					case REQUESTCERTIFICATE :
					{
						PGPUInt32			uExportFormat;
						PGPUInt32			uOutputFormat;
						PVOID				pRequest;
						PGPSize				sizeRequest;

						switch (psts->ksentry.protocol) {
						case kPGPKeyServerClass_NetToolsCA :
							uExportFormat = 
								kPGPExportFormat_NetToolsCAV1_CertReq;
							uOutputFormat = 
								kPGPOutputFormat_NetToolsCAV1_CertReqInPKCS7;
							break;

						case kPGPKeyServerClass_Verisign :
							uExportFormat = 
								kPGPExportFormat_VerisignV1_CertReq;
							uOutputFormat = 
								kPGPOutputFormat_VerisignV1_CertReqInPKCS7;
							break;

						case kPGPKeyServerClass_Entrust :
							uExportFormat = 
								kPGPExportFormat_EntrustV1_CertReq;
							uOutputFormat = 
								kPGPOutputFormat_EntrustV1_CertReqInPKCS7;
							break;
						}

						err = PGPExportKeySet (psts->keysetIn,
								PGPOAllocatedOutputBuffer (psts->context, 
										&pBuffer, MAX_PGPSize, &size),
								PGPOExportFormat (psts->context, 
										uExportFormat),
								psts->pszPassPhrase ?
									PGPOPassphrase (psts->context, 
											psts->pszPassPhrase) :
									PGPOPasskeyBuffer (psts->context, 
											pPasskey, sizePasskey),
								PGPOAttributeValue (psts->context, 
										psts->pAVlist, psts->numAVs),
								PGPOLastOption (psts->context));

						if (IsntPGPError (err)) 
						{
							err = PGPEncode (psts->context,
									PGPOInputBuffer (psts->context, 
											pBuffer, size),
									PGPOOutputFormat (psts->context, 
											uOutputFormat),
									PGPOAllocatedOutputBuffer (psts->context,
											&pRequest, MAX_PGPSize, 
											&sizeRequest),
									PGPOSignWithKey (psts->context, key,
											psts->pszPassPhrase ?
											PGPOPassphrase (psts->context, 
													psts->pszPassPhrase) :
											PGPOPasskeyBuffer (psts->context,
													pPasskey, sizePasskey),
											PGPOLastOption (psts->context)),
									PGPOLastOption (psts->context));

							if (IsntPGPError (err))
							{
								err = PGPSendCertificateRequest (
										psts->server,
										PGPOKeyServerCAKey (psts->context, 
												psts->keyIn),
										PGPOKeyServerRequestKey (
												psts->context, key),
										PGPOInputBuffer (psts->context, 
											pRequest, sizeRequest),
										PGPOLastOption (psts->context));

								PGPFreeData (pRequest);
							}
							PGPFreeData (pBuffer);
						}
						break;
					}

					case RETRIEVECERTIFICATE :
					{
						PGPFilterRef	filter		= kInvalidPGPFilterRef;
						PGPByte			md5HashBuffer[128];
						PGPUInt32		uInputFormat;

						pBuffer = NULL;

						switch (psts->ksentry.protocol) {
						case kPGPKeyServerClass_NetToolsCA :
							uInputFormat = 
								kPGPInputFormat_NetToolsCAV1_DataInPKCS7;
							break;
						case kPGPKeyServerClass_Verisign :
							uInputFormat = 
									kPGPInputFormat_VerisignV1_DataInPKCS7;
							break;
						case kPGPKeyServerClass_Entrust :
							uInputFormat = 
									kPGPInputFormat_EntrustV1_DataInPKCS7;
							break;
						}

						switch (psts->ksentry.protocol) {

						case kPGPKeyServerClass_NetToolsCA :
							err = PGPGetKeyPropertyBuffer (psts->keyIn,
								kPGPKeyPropX509MD5Hash, sizeof(md5HashBuffer),
								md5HashBuffer, &size);	

							if (IsntPGPError (err))
							{
								err = PGPNewKeyPropertyBufferFilter (
										psts->context,
										kPGPKeyPropX509MD5Hash, 
										md5HashBuffer, size,
										kPGPMatchEqual, 
										&filter);

								if (IsntPGPError (err))
								{
									err = PGPRetrieveCertificate ( 
											psts->server, 
											PGPOKeyServerSearchFilter (
													psts->context,
													filter),
											PGPOAllocatedOutputBuffer (
													psts->context,
													&pBuffer,
													MAX_PGPSize, 
													&size),
											PGPOLastOption (psts->context));
								}
							}
							break;

						case kPGPKeyServerClass_Verisign :
						case kPGPKeyServerClass_Entrust :
							err = KMGetKeyPhrase (
									psts->context,		// in context
									psts->tlsContext,	// in tlscontext
									psts->hwnd,			// in hwnd of parent
									NULL,				// in prompt
									psts->keysetMain,	// in keyset
									psts->keyIn,		// in key
									&pszPhrase,			// out phrase
									&pPasskey,			// out passkey buffer
									&sizePasskey);		// out passkey length

							if (IsntPGPError (err))
							{
								psts->pszPassPhrase = pszPhrase;

								err = PGPRetrieveCertificate ( 
										psts->server, 
    									PGPOSignWithKey (
											psts->context,
											psts->keyIn,
											pszPhrase ?
											PGPOPassphrase (psts->context, 
												pszPhrase) :
											PGPOPasskeyBuffer (psts->context,
												pPasskey, sizePasskey),
											PGPOLastOption (
												psts->context)),
										PGPOKeyServerSearchKey (
											psts->context,
											psts->keyIn),
										PGPOAllocatedOutputBuffer (
											psts->context,
											&pBuffer,
											MAX_PGPSize, 
											&size),
										PGPOLastOption (psts->context));
							}
							break;
						}

						if (IsntPGPError (err))
						{
							err = PGPDecode (psts->context,
									PGPOInputBuffer (psts->context, 
											pBuffer, size),
									PGPODiscardOutput (psts->context,
											TRUE),
									PGPOImportKeysTo (psts->context,
											psts->keysetOut),
									PGPOInputFormat (psts->context, 
											uInputFormat),
									PGPOLastOption (psts->context));

						}
						if (pBuffer)
							PGPFreeData (pBuffer);
						if (PGPFilterRefIsValid (filter))
							PGPFreeFilter (filter);
						break;
					}

					case UPDATEREVOCATIONS :
					{
						PGPUInt32		uInputFormat;
						PGPKeyRef		keySigning;

						switch (psts->ksentry.protocol) {
						case kPGPKeyServerClass_NetToolsCA :
							uInputFormat = 
									kPGPInputFormat_NetToolsCAV1_DataInPKCS7;
							break;

						case kPGPKeyServerClass_Verisign :
							uInputFormat = 
									kPGPInputFormat_VerisignV1_DataInPKCS7;
							break;

						case kPGPKeyServerClass_Entrust :
							uInputFormat = 
									kPGPInputFormat_EntrustV1_DataInPKCS7;
							break;
						}

						switch (psts->ksentry.protocol) {
						case kPGPKeyServerClass_NetToolsCA :
							err = PGPRetrieveCertificateRevocationList (
								psts->server,
								PGPOKeyServerCAKey (psts->context, 
										psts->keyIn), 
								PGPOAllocatedOutputBuffer (psts->context, 
										&pBuffer, MAX_PGPSize, &size),
								PGPOLastOption (psts->context));
							break;

						case kPGPKeyServerClass_Verisign :
						case kPGPKeyServerClass_Entrust :

							keySigning = kInvalidPGPKeyRef;
							err = KMGetSigningKeyPhrase (
									psts->context,		// in context
									psts->tlsContext,	// in tlscontext
									psts->hwnd,			// in hwnd of parent
									NULL,				// in prompt
									psts->keysetMain,	// in keyset
									TRUE,				// in no split keys
									&keySigning,		// out key to use
									&pszPhrase,			// out phrase
									&pPasskey,			// out passkey buffer
									&sizePasskey);		// out passkey length

							if (IsntPGPError (err)) 
							{
								psts->pszPassPhrase = pszPhrase;

								err = PGPRetrieveCertificateRevocationList (
									psts->server,
									PGPOKeyServerCAKey (psts->context, 
											psts->keyIn), 
									PGPOKeySetRef (psts->context,
											psts->keysetMain),
    								PGPOSignWithKey (
										psts->context,
										keySigning,
										pszPhrase ?
										PGPOPassphrase (psts->context, 
											pszPhrase) :
										PGPOPasskeyBuffer (psts->context,
											pPasskey, sizePasskey),
										PGPOLastOption (
											psts->context)),
									PGPOAllocatedOutputBuffer (psts->context, 
											&pBuffer, MAX_PGPSize, &size),
									PGPOLastOption (psts->context));
							}

							break;
						}

						if (IsntPGPError (err) && 
							(pBuffer != NULL))
						{
							err = PGPDecode (psts->context,
									PGPOInputBuffer (psts->context, 
											pBuffer, size),
									PGPODiscardOutput (psts->context, TRUE),
									PGPOImportKeysTo (psts->context, 
											psts->keysetMain),
									PGPOInputFormat (psts->context, 
											uInputFormat),
									PGPOLastOption (psts->context));
					
							PGPFreeData (pBuffer);
						}
						break;
					}
					}

					// burn and free phrase
					if (IsntNull (pszPhrase))
					{
						PGPclFreePhrase (pszPhrase);
						psts->pszPassPhrase = NULL;
					}
					if (pPasskey)
						PGPFreeData (pPasskey);

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

//done :
	// let window know that we're done
	psts->err = err;
	event.szmessage[0] = '\0';
	if (err == kPGPError_UserAbort) 
	{
		event.nmhdr.code = PGPCL_SERVERABORT;
		LoadString (g_hInst, IDS_SERVERABORT, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->ksentry.serverDNS);
	}
	else if (IsPGPError (err)) 
	{
		event.nmhdr.code = PGPCL_SERVERERROR;
		LoadString (g_hInst, IDS_SERVERERROR, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->ksentry.serverDNS);
	}
	else 
	{
		event.nmhdr.code = PGPCL_SERVERDONE;
		LoadString (g_hInst, IDS_SERVEROK, event.szmessage,
					sizeof(event.szmessage));
		lstrcat (event.szmessage, psts->ksentry.serverDNS);
	}

	bThreadFree = psts->bThreadFree;
	SendMessage (hwndParent, WM_NOTIFY, (WPARAM)hwndParent, 
								(LPARAM)&event);

	// if thread should free data structure, do it
	if (bThreadFree) 
	{
		if (PGPKeySetRefIsValid (psts->keysetIn)) 
			PGPFreeKeySet (psts->keysetIn);
		PGPclFreeCACertRequestAVList (psts->pAVlist, psts->numAVs);
		PGPFreeData (psts);
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


//	___________________________________________________
//
//	request a certificate from a CA server

PGPError PGPclExport 
PGPclSendCertificateRequestToServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain,
		PGPUserIDRef			userid,
		PGPKeySetRef			keysetKey)
{
	PGPError			err				= kPGPError_NoErr;
	PGPBoolean			bCheckForCRL	= FALSE;
	PGPPrefRef			prefref			= kInvalidPGPPrefRef;
	PSERVERTHREADSTRUCT	psts			= NULL;
	PGPKeyServerEntry*	pentry			= NULL;

	PGPKeyRef			keyRootCA;
	PGPSigRef			sigRootCA;

	err = PGPclOpenClientPrefs (
				PGPGetContextMemoryMgr (context), &prefref);  CKERR;
	err = PGPclGetRootCACertPrefs (context, prefref, keysetMain,
				&keyRootCA, &sigRootCA); CKERR;

	psts = PGPNewData (PGPGetContextMemoryMgr (context),
				sizeof(SERVERTHREADSTRUCT), kPGPMemoryMgrFlags_Clear);
	if (psts) 
	{
		psts->keyIn				= keyRootCA;
		psts->keysetIn			= keysetKey;
		psts->pszPassPhrase		= NULL;
		psts->keysetOut			= kInvalidPGPKeySetRef;
		psts->uOperation		= REQUESTCERTIFICATE;
		psts->context			= context;
		psts->tlsContext		= tlsContext;
		psts->keysetMain		= keysetMain;
		psts->lpThread			= sCAThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= kPGPKeyServerKeySpace_Default;
		psts->groupset			= kInvalidPGPGroupSetRef;

		err = CLGetCAServerPrefs (context, prefref, &pentry); CKERR;
		if (pentry[0].serverDNS[0] == '\0')
		{
			err = kPGPError_Win32_NoCAServerPrefs;
			goto done;
		}
		pgpCopyMemory (&pentry[0], 
					&psts->ksentry, sizeof(PGPKeyServerEntry));

		{
			PGPKeyRef	key;

			if (!PGPUserIDRefIsValid (userid)) 
			{
				sGetFirstKeyInSet (keysetKey, &key);
				PGPGetPrimaryUserID (key, &userid);
			}

			err = PGPclGetCACertRequestAVList (
					hwndParent, context, FALSE, userid, 
					pentry->protocol, 
					&(psts->pAVlist), &(psts->numAVs)); CKERR;
		}

		err = DialogBoxParam (g_hInst, 
					MAKEINTRESOURCE(IDD_SERVERPROGRESS),
					hwndParent, sServerProgressDlgProc, (LPARAM)psts);

		if (IsntPGPError (err))
		{
			CLSetCAServerPrefs (context, prefref, &psts->ksentry);
		}
	}

done :
	if (psts)
	{
		if (!psts->bThreadFree) 
		{
			PGPclFreeCACertRequestAVList (psts->pAVlist, psts->numAVs);
			PGPFreeData (psts);
		}
	}
	if (PGPPrefRefIsValid (prefref))
		PGPclCloseClientPrefs (prefref, IsntPGPError (err));
	if (IsntNull (pentry))
		PGPFreeData (pentry);

	return err;
}

//	___________________________________________________
//
//	request a certificate from a CA server

PGPError PGPclExport 
PGPclSendCertificateRequestToServerNotify (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndToNotify, 
		PGPKeySetRef			keysetMain,
		PGPUserIDRef			userid,
		PGPKeySetRef			keysetKey,
		LPSTR					pszPassPhrase)
{
	PGPError			err				= kPGPError_NoErr;
	PGPBoolean			bCheckForCRL	= FALSE;
	PGPPrefRef			prefref			= kInvalidPGPPrefRef;
	PSERVERTHREADSTRUCT	psts			= NULL;
	PGPKeyServerEntry*	pentry			= NULL;

	PGPKeyRef			keyRootCA;
	PGPSigRef			sigRootCA;
	DWORD				dwThreadID;

	err = PGPclOpenClientPrefs (
				PGPGetContextMemoryMgr (context), &prefref);  CKERR;
	err = PGPclGetRootCACertPrefs (context, prefref, keysetMain,
				&keyRootCA, &sigRootCA); CKERR;

	psts = PGPNewData (PGPGetContextMemoryMgr (context),
				sizeof(SERVERTHREADSTRUCT), kPGPMemoryMgrFlags_Clear);
	if (psts) 
	{
		psts->keyIn				= keyRootCA;
		psts->keysetIn			= keysetKey;
		psts->pszPassPhrase		= pszPassPhrase;
		psts->keysetOut			= kInvalidPGPKeySetRef;
		psts->uOperation		= REQUESTCERTIFICATE;
		psts->context			= context;
		psts->tlsContext		= tlsContext;
		psts->keysetMain		= keysetMain;
		psts->hwnd				= hwndToNotify;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;  // see below!
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= kPGPKeyServerKeySpace_Default;
		psts->groupset			= kInvalidPGPGroupSetRef;

		err = CLGetCAServerPrefs (context, prefref, &pentry); CKERR;
		if (pentry[0].serverDNS[0] == '\0')
		{
			err = kPGPError_Win32_NoCAServerPrefs;
			goto done;
		}
		pgpCopyMemory (&pentry[0], 
					&psts->ksentry, sizeof(PGPKeyServerEntry));

		{
			PGPKeyRef	key;

			if (!PGPUserIDRefIsValid (userid)) 
			{
				sGetFirstKeyInSet (keysetKey, &key);
				PGPGetPrimaryUserID (key, &userid);
			}

			err = PGPclGetCACertRequestAVList (
					hwndToNotify, context, FALSE, userid, 
					pentry->protocol, 
					&(psts->pAVlist), &(psts->numAVs)); CKERR;
		}

		psts->bThreadFree = TRUE;
		CreateThread (NULL, 0, sCAThreadRoutine, 
										(void*)psts, 0, &dwThreadID);
	}

done :
	if (psts)
	{
		if (!psts->bThreadFree) 
		{
			if (PGPKeySetRefIsValid (psts->keysetIn))
				PGPFreeKeySet (psts->keysetIn);
			PGPFreeData (psts);
		}
	}
	if (PGPPrefRefIsValid (prefref))
		PGPclCloseClientPrefs (prefref, IsntPGPError (err));
	if (IsntNull (pentry))
		PGPFreeData (pentry);

	return err;
}

//	___________________________________________________
//
//	retrieve a previously-requested certificate from a CA server

PGPError PGPclExport 
PGPclRetrieveCertificateFromServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain,
		PGPKeySetRef			keysetKey,
		PGPUserIDRef			userid,
		PGPKeySetRef*			pkeysetCert)
{
	PGPError			err				= kPGPError_NoErr;
	PGPBoolean			bCheckForCRL	= FALSE;
	PGPPrefRef			prefref			= kInvalidPGPPrefRef;
	PSERVERTHREADSTRUCT	psts			= NULL;
	PGPKeyServerEntry*	pentry			= NULL;

	err = PGPclOpenClientPrefs (
				PGPGetContextMemoryMgr (context), &prefref);  CKERR;

	psts = PGPNewData (PGPGetContextMemoryMgr (context),
				sizeof(SERVERTHREADSTRUCT), kPGPMemoryMgrFlags_Clear);
	if (psts) 
	{		
		psts->keysetIn			= kInvalidPGPKeySetRef;
		psts->pszPassPhrase		= NULL;
		psts->uOperation		= RETRIEVECERTIFICATE;
		psts->context			= context;
		psts->tlsContext		= tlsContext;
		psts->keysetMain		= keysetMain;
		psts->lpThread			= sCAThreadRoutine;
		psts->bSearchInProgress	= FALSE;
		psts->bCancel			= FALSE;
		psts->bThreadFree		= FALSE;
		psts->iStatusValue		= -1;
		psts->iStatusDirection	= 1;
		psts->space				= kPGPKeyServerKeySpace_Default;
		psts->groupset			= kInvalidPGPGroupSetRef;

		err = sGetFirstKeyInSet (keysetKey, &psts->keyIn);  CKERR;
		err = PGPNewKeySet (context, &psts->keysetOut); CKERR;

		err = CLGetCAServerPrefs (context, prefref, &pentry); CKERR;
		if (pentry[0].serverDNS[0] == '\0')
		{
			err = kPGPError_Win32_NoCAServerPrefs;
			goto done;
		}
		pgpCopyMemory (&pentry[0], 
					&psts->ksentry, sizeof(PGPKeyServerEntry));

		err = DialogBoxParam (g_hInst, 
					MAKEINTRESOURCE(IDD_SERVERPROGRESS),
					hwndParent, sServerProgressDlgProc, (LPARAM)psts);

		if (IsPGPError (err))
		{
			PGPFreeKeySet (psts->keysetOut);
			psts->keysetOut = kInvalidPGPKeySetRef;
		}
		else
		{
			CLSetCAServerPrefs (context, prefref, &psts->ksentry);
			if (pkeysetCert)
			{
				*pkeysetCert = psts->keysetOut;
				psts->keysetOut = kInvalidPGPKeySetRef;
			}
		}
	}

done :
	if (psts)
	{
		if (!psts->bThreadFree) 
		{
			if (PGPKeySetRefIsValid (psts->keysetOut))
				PGPFreeKeySet (psts->keysetOut);
			PGPFreeData (psts);
		}
	}
	if (PGPPrefRefIsValid (prefref))
		PGPclCloseClientPrefs (prefref, IsntPGPError (err));
	if (IsntNull (pentry))
		PGPFreeData (pentry);

	return err;
}


//	___________________________________________________
//
//	retrieve certificate revocation list for root CA

PGPError PGPclExport 
PGPclGetCertificateRevocationsFromServer (
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		HWND					hwndParent, 
		PGPKeySetRef			keysetMain)
{
	PGPError			err				= kPGPError_NoErr;
	PGPBoolean			bCheckForCRL	= FALSE;
	PGPPrefRef			prefref			= kInvalidPGPPrefRef;
	PSERVERTHREADSTRUCT	psts			= NULL;
	PGPKeyServerEntry*	pentry			= NULL;

	PGPKeyRef			keyRootCA;
	PGPSigRef			sigRootCA;
	PGPBoolean			bHasCRL;
	PGPTime				timeExpiration;

	err = PGPclOpenClientPrefs (
				PGPGetContextMemoryMgr (context), &prefref);  CKERR;
	err = PGPclGetRootCACertPrefs (context, prefref, keysetMain,
				&keyRootCA, &sigRootCA); CKERR;
	err = PGPGetKeyBoolean (
				keyRootCA, kPGPKeyPropHasCRL, &bHasCRL); CKERR;

	if (bHasCRL)
	{	
		err = PGPGetKeyTime (keyRootCA,
					kPGPKeyPropCRLNextUpdate, &timeExpiration); CKERR;

		if (timeExpiration <= PGPGetTime())
			bCheckForCRL = TRUE;
	}
	else
		bCheckForCRL = TRUE;

	if (bCheckForCRL)
	{
		psts = PGPNewData (PGPGetContextMemoryMgr (context),
					sizeof(SERVERTHREADSTRUCT), kPGPMemoryMgrFlags_Clear);

		if (psts) 
		{		
			psts->keyIn				= keyRootCA;
			psts->keysetIn			= kInvalidPGPKeySetRef;
			psts->pszPassPhrase		= NULL;
			psts->uOperation		= UPDATEREVOCATIONS;
			psts->context			= context;
			psts->tlsContext		= tlsContext;
			psts->keysetMain		= keysetMain;
			psts->lpThread			= sCAThreadRoutine;
			psts->bSearchInProgress	= FALSE;
			psts->bCancel			= FALSE;
			psts->bThreadFree		= FALSE;
			psts->iStatusValue		= -1;
			psts->iStatusDirection	= 1;
			psts->space				= kPGPKeyServerKeySpace_Default;
			psts->groupset			= kInvalidPGPGroupSetRef;

			err = CLGetCAServerPrefs (context, prefref, &pentry); CKERR;
			if (pentry[1].serverDNS[0] == '\0')
			{
				err = kPGPError_Win32_NoCAServerPrefs;
				goto done;
			}
			pgpCopyMemory (&pentry[1], 
						&psts->ksentry, sizeof(PGPKeyServerEntry));

			err = DialogBoxParam (g_hInst, 
						MAKEINTRESOURCE(IDD_SERVERPROGRESS),
						hwndParent, sServerProgressDlgProc, (LPARAM)psts);

			if (IsPGPError (err))
			{
				psts->keysetOut = kInvalidPGPKeySetRef;
			}
			else
			{
				CLSetCARevocationServerPrefs (
						context, prefref, &psts->ksentry);
			}
		}
	}
	else
		err = kPGPError_Win32_NoNewCRL;

done :
	if (psts)
	{
		if (!psts->bThreadFree) 
		{
			PGPFreeData (psts);
		}
	}
	if (PGPPrefRefIsValid (prefref))
		PGPclCloseClientPrefs (prefref, IsntPGPError (err));
	if (IsntNull (pentry))
		PGPFreeData (pentry);

	return err;
}


