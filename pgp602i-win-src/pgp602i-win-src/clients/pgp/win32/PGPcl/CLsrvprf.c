/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLsrvprf.c - does the drudge work of setting up keyservers
	

	$Id: CLsrvprf.c,v 1.8 1998/08/11 14:43:19 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"

// external globals
extern HINSTANCE g_hInst;

//	___________________________________________________
//
//	handler for keyserver events that we care about (TLS events)

PGPError 
CLHandleTLSEvent (
		PGPContextRef		context,
		PGPEvent*			pevent,
		HWND				hwnd,
		PGPKeySetRef		keyset,
		PGPKeyServerEntry*	pentry,
		BOOL*				pbSecure)
{
	PGPError				err			= kPGPError_NoErr;

	PGPtlsSessionRef		tls;
	PGPKeyRef				keyAuth;
	PGPPublicKeyAlgorithm	algAuth;
	PGPKeyID				keyidAuth;
	PGPKeyID				keyidExpected;
	CHAR					szUserID[kPGPMaxUserIDSize +1];
	PGPSize					size;

	*pbSecure = FALSE;

	if (pevent->type == kPGPEvent_KeyServerTLSEvent)
	{
		PGPEventKeyServerTLSData data = pevent->data.keyServerTLSData;

		if (data.state == kPGPKeyServerState_TLSConnectionSecured)
		{	
			CHAR	szServer[256];

			PGPGetKeyServerURL (pentry, szServer);

			tls = pevent->data.keyServerTLSData.tlsSession;
			err = PGPtlsGetRemoteAuthenticatedKey (tls, &keyAuth); CKERR;

			err = PGPGetPrimaryUserIDNameBuffer (keyAuth, 
						sizeof(szUserID), szUserID, &size); CKERR;
			
			err = PGPGetKeyIDFromKey (keyAuth, &keyidAuth); CKERR;

			err = PGPGetKeyNumber (keyAuth, 
							kPGPKeyPropAlgID, &algAuth); CKERR;

			if (pentry->authAlg == kPGPPublicKeyAlgorithm_Invalid)
			{
				
				err = PGPGetKeyIDString (&keyidAuth, 
								kPGPKeyIDString_Full,
								pentry->authKeyIDString); CKERR;

				pentry->authAlg = algAuth;
				PGPclSyncKeyserverPrefs (context, pentry);

				err = PGPclConfirmRemoteAuthentication (
											context,
											hwnd,
											szServer,
											keyAuth,
											keyset,
											PGPCL_AUTHNEWKEY);
				if (IsntPGPError (err)) 
				{
					*pbSecure = TRUE; // ???
				}
			}
			else				
			{
				err = PGPGetKeyIDFromString (
							pentry->authKeyIDString,
							&keyidExpected); CKERR;

				if (PGPCompareKeyIDs (&keyidExpected, &keyidAuth))
				{
					err = PGPclConfirmRemoteAuthentication (
											context,
											hwnd,
											szServer,
											keyAuth,
											keyset,
											PGPCL_AUTHUNEXPECTEDKEY);
					// if user approves
					if (IsntPGPError (err))
					{
						err = PGPGetKeyIDString (&keyidAuth, 
									kPGPKeyIDString_Full,
									pentry->authKeyIDString); CKERR;

						pentry->authAlg = algAuth;
						PGPclSyncKeyserverPrefs (context, pentry);

						*pbSecure = TRUE; // ???
					}
				}
				else
				{
					err = PGPclConfirmRemoteAuthentication (
											context,
											hwnd,
											szServer,
											keyAuth,
											keyset,
											PGPCL_AUTHEXPECTEDKEY);
					// if user approves
					if (IsntPGPError (err))
					{
						*pbSecure = TRUE;
					}
				}
			}
		}		
	}

done :

	return err;
}


//	___________________________________________________
//
//	handler for keyserver events that we care about (TLS events)

static PGPError 
sServerEventHandler (
		PGPContextRef	context,
		PGPEvent*		pevent, 
		PGPUserValue	uservalue)
{
	PCLIENTSERVERSTRUCT	pcss	= (PCLIENTSERVERSTRUCT)uservalue;
	PGPError			err		= kPGPError_NoErr;
	BOOL				bSecure;

	switch (pevent->type) {
	case kPGPEvent_KeyServerTLSEvent:
		err = CLHandleTLSEvent (context, pevent, pcss->hwnd, 
							pcss->keysetMain, pcss->pentry, &bSecure);
		break;
	}

	return err;
}

//	___________________________________________________
//
//	Init keyservers

PGPError CLInitKeyServerPrefs(
		PGPUInt32			uServer,
		PGPKeyServerEntry*	pkeyserver,
		HWND				hwndParent,
		PGPContextRef		context,
		PGPKeySetRef		keysetMain,
		PGPPrefRef			clientPrefsRef,
		CHAR*				pszUserID,
		PCLIENTSERVERSTRUCT	*ppcss,
		PGPKeyServerEntry	**ksEntries,
		PGPKeyServerSpec	**serverList,
		PGPUInt32*			numKSEntries)
{
	PGPError				err				= kPGPError_NoErr;
	CHAR					szDomain[kMaxServerNameLength+1];
	PGPKeyServerEntry		entryRoot;

	PGPValidatePtr (keysetMain);
	PGPValidatePtr (context);
	PGPValidatePtr (pszUserID);

	switch (uServer) 
	{
		case PGPCL_SPECIFIEDSERVER :
		{
			(*ksEntries) = 
				(PGPKeyServerEntry *)pgpAlloc(sizeof(PGPKeyServerEntry));
			memcpy (*ksEntries, pkeyserver, sizeof(PGPKeyServerEntry));
			(*numKSEntries) = 1;
			break;
		}

		case PGPCL_USERIDBASEDSERVER :
		{
			szDomain[0] = '\0';
			if (pszUserID) 
				PGPFindEmailDomain (pszUserID, szDomain);
			err = PGPCreateKeyServerPath (clientPrefsRef, szDomain,
				ksEntries, &(*numKSEntries));
		}
		break;

		case PGPCL_DEFAULTSERVER :
		{
			err = PGPCreateKeyServerPath (clientPrefsRef, "",
				ksEntries, &(*numKSEntries));
			break;
		}

		case PGPCL_ROOTSERVER : 
		{
			err = PGPGetRootKeyServer (clientPrefsRef, &entryRoot); 

			if( err == kPGPError_ItemNotFound )
			{
				PGPclMessageBox (hwndParent, IDS_CAPTION, IDS_NOROOTSERVER,
					MB_OK|MB_ICONEXCLAMATION);
				return kPGPError_UserAbort;
			}
			(*ksEntries) = 
				(PGPKeyServerEntry *)pgpAlloc(sizeof(PGPKeyServerEntry));
			memcpy (*ksEntries, &entryRoot, sizeof(PGPKeyServerEntry));
			(*numKSEntries) = 1;
			break;
		}
	}

	if ((IsntPGPError (err)) && 
		(IsntNull (*ksEntries)) && 
		((*numKSEntries) > 0))
	{
		*ppcss = clAlloc ((*numKSEntries) * sizeof(CLIENTSERVERSTRUCT));

		if (*ppcss!=0)
		{
			*serverList = (PGPKeyServerSpec*)PGPNewData(
				PGPGetContextMemoryMgr (context),
				(*numKSEntries) * sizeof(PGPKeyServerSpec),
				kPGPMemoryMgrFlags_Clear);

			if (IsntNull (*serverList))
			{
				PGPUInt32						iIndex;
				PGPKeyServerThreadStorageRef	previousStorage;

				PGPKeyServerInit();

				PGPKeyServerCreateThreadStorage(&previousStorage);

				for (iIndex = 0; iIndex < (*numKSEntries); iIndex++)
				{
					err = PGPNewKeyServerFromHostName (context,
						(*ksEntries)[iIndex].serverDNS,
						(*ksEntries)[iIndex].serverPort,
						(*ksEntries)[iIndex].protocol,
						kPGPKeyServerAccessType_Normal,
						kPGPKeyServerKeySpace_Normal,
						&(*serverList)[iIndex].server);

					if (IsPGPError (err))
						break;

					(*serverList)[iIndex].serverName = 
						(*ksEntries)[iIndex].serverDNS;

					(*ppcss)[iIndex].hwnd		= hwndParent;
					(*ppcss)[iIndex].keysetMain	= keysetMain;
					(*ppcss)[iIndex].pentry		= &(*ksEntries)[iIndex];

					err = PGPSetKeyServerEventHandler (
								(*serverList)[iIndex].server, 
								sServerEventHandler, 
								(PGPUserValue)&(*ppcss)[iIndex]);
				}

				PGPKeyServerDisposeThreadStorage(previousStorage);
			}
		}
	}

	return err;
}

//	___________________________________________________
//
//	Uninit keyservers

PGPError CLUninitKeyServerPrefs(
		PGPUInt32			uServer,
		PCLIENTSERVERSTRUCT	pcss,
		PGPKeyServerEntry	*ksEntries,
		PGPKeyServerSpec	*serverList,
		PGPUInt32			numKSEntries)
{
	PGPUInt32						iIndex;
	PGPError						err;
	PGPKeyServerThreadStorageRef	previousStorage;

	err=kPGPError_NoErr;

	PGPKeyServerCreateThreadStorage(&previousStorage);

	for (iIndex = 0; iIndex < numKSEntries; iIndex++)
	{
		if (PGPKeyServerRefIsValid (serverList[iIndex].server))
			PGPFreeKeyServer (serverList[iIndex].server);
	}
	
	if (IsntNull (serverList))
		PGPFreeData (serverList);

	if (IsntNull (ksEntries))
		PGPDisposeKeyServerPath (ksEntries);

	if (IsntNull (pcss))
		clFree(pcss);

	PGPKeyServerDisposeThreadStorage(previousStorage);

	PGPKeyServerCleanup ();

	return err;
}

