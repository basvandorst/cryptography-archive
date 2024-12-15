/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLsdksrv.c - wrapper functions for sdkui keyserver calls
	

	$Id: CLsdksrv.c,v 1.20 1998/08/11 14:43:15 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"

// external globals
extern HINSTANCE g_hInst;

//	___________________________________________________
//
//	all keyserver queries are funneled through here

static PGPError
sQueryServerInternal (
	HWND					hwndParent,
	PGPContextRef			context,
	PGPtlsContextRef		tlsContext,
	PGPOptionListRef		optionSearch,
	UINT					uServer,
	LPSTR					pszUserID,
	PGPKeySetRef			keysetMain,
	PGPKeySetRef*			pkeysetResult)
{
	PGPError				err				= kPGPError_NoErr;
	PGPKeyServerEntry*		pentryList		= NULL;
	PGPKeyServerSpec*		pserverList		= NULL;
	PGPInt32				iNumEntries		= 0;
	PCLIENTSERVERSTRUCT		pcss			= NULL;
	PGPMemoryMgrRef			memMgr;
	PGPPrefRef				clientPrefsRef;

	PGPValidatePtr (pkeysetResult);
	PGPValidatePtr (context);
	PGPValidatePtr (pszUserID);
		
	memMgr=PGPGetContextMemoryMgr(context);
	err=PGPclOpenClientPrefs (memMgr,&clientPrefsRef);

	if(IsntPGPError(err))
	{
		err=CLInitKeyServerPrefs(uServer,NULL,
			hwndParent,context,keysetMain,clientPrefsRef,pszUserID,
			&pcss,&pentryList,&pserverList,&iNumEntries);

		PGPclCloseClientPrefs (clientPrefsRef, FALSE);
	}

	if (IsntPGPError (err)) 
	{
		err = PGPSearchKeyServerDialog (context, iNumEntries,
			pserverList, tlsContext, 
			FALSE, pkeysetResult, optionSearch,
			PGPOUIParentWindowHandle (context, hwndParent),
			PGPOLastOption (context));
	}

	CLUninitKeyServerPrefs(uServer,
		pcss,pentryList,pserverList,iNumEntries);
	
	return err;
}

//	___________________________________________________
//
//	all keyserver sends are funneled through here

static PGPError
sSendToServerInternal (
		HWND					hwndParent,
		PGPContextRef			context,
		PGPtlsContextRef		tlsContext,
		UINT					uServer,
		PGPKeyServerEntry*		pkeyserver,
		LPSTR					pszUserID,
		PGPKeySetRef			keysetMain,
		PGPKeySetRef			keysetToSend)
{
	PGPError				err				= kPGPError_NoErr;
	PGPKeyServerEntry*		pentryList		= NULL;
	PGPKeyServerSpec*		pserverList		= NULL;
	PGPInt32				iNumEntries		= 0;
	PCLIENTSERVERSTRUCT		pcss			= NULL;
	PGPKeySetRef			keysetFailed	= kInvalidPGPKeySetRef;
	PGPMemoryMgrRef			memMgr;
	PGPPrefRef				clientPrefsRef;

	PGPValidatePtr (keysetToSend);
	PGPValidatePtr (context);
	PGPValidatePtr (pszUserID);
		
	memMgr=PGPGetContextMemoryMgr(context);
	err=PGPclOpenClientPrefs (memMgr,&clientPrefsRef);

	if(IsntPGPError(err))
	{
		err=CLInitKeyServerPrefs(uServer,pkeyserver,
			hwndParent,context,keysetMain,clientPrefsRef,pszUserID,
			&pcss,&pentryList,&pserverList,&iNumEntries);

		PGPclCloseClientPrefs (clientPrefsRef, FALSE);
	}

	if (IsntPGPError (err)) 
	{
		err = PGPSendToKeyServerDialog (context, &(pserverList[0]),
			tlsContext, keysetToSend, &keysetFailed, 
			PGPOUIParentWindowHandle (context, hwndParent),
			PGPOLastOption (context));

		if (PGPKeySetRefIsValid (keysetFailed))
			PGPFreeKeySet (keysetFailed);
	}

	CLUninitKeyServerPrefs(uServer,
		pcss,pentryList,pserverList,iNumEntries);
	
	return err;
}

//	___________________________________________________
//
//	send keyset to keyserver

PGPError PGPclExport 
PGPclSendKeysToServer (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		UINT				uServer,
		PGPKeyServerEntry*	pkeyserver,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef		keysetToSend)
{
	PGPError				err				= kPGPError_NoErr;
	PGPKeySetRef			keyset			= kInvalidPGPKeySetRef;
	PGPKeyListRef			keylist			= kInvalidPGPKeyListRef;
	PGPKeyIterRef			keyiter			= kInvalidPGPKeyIterRef;

	PGPValidatePtr (keysetToSend);
	PGPValidatePtr (context);
	PGPValidatePtr (pkeyserver);
	
	// derive server from userid
	if (uServer == PGPCL_USERIDBASEDSERVER) {
		PGPKeyRef		key			= kInvalidPGPKeyRef;

		CHAR			szUserID[kPGPMaxUserIDSize+1];
		PGPSize			size;

		err = PGPOrderKeySet (keysetToSend, 
								kPGPAnyOrdering, &keylist); CKERR;
		err = PGPNewKeyIter (keylist, &keyiter); CKERR;

		while (	IsntPGPError (PGPKeyIterNext (keyiter, &key)) && 
				PGPKeyRefIsValid (key))
		{
			err = PGPNewSingletonKeySet (key, &keyset); CKERR;

			err = PGPGetPrimaryUserIDNameBuffer (key, kPGPMaxUserIDSize,
							szUserID, &size); CKERR;

			err = sSendToServerInternal (hwndParent,
											context,
											tlsContext,
											uServer,
											pkeyserver,
											szUserID,
											keysetMain,
											keyset); CKERR;
		}
	}

	// use indicated or caller-supplied server
	else {
		err = sSendToServerInternal (hwndParent,
									context,
									tlsContext,
									uServer,
									pkeyserver,
									"",
									keysetMain,
									keysetToSend);
	}

done :	
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPKeySetRefIsValid (keyset))
		PGPFreeKeySet (keyset);

	return err;
}

//	___________________________________________________
//
//	search keyserver(s) for userid

PGPError PGPclExport 
PGPclSearchServerForUserID (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		LPSTR				szUserID,
		UINT				uServer,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef*		pkeysetResult) 
{
	PGPError				err				= kPGPError_NoErr;
	PGPKeySetRef			keyset			= kInvalidPGPKeySetRef;
	PGPOptionListRef		optionSearch	= kInvalidPGPOptionListRef;
	PGPFilterRef			filter			= kInvalidPGPFilterRef;
	
	// initialize return keyset
	PGPValidatePtr (pkeysetResult);
	*pkeysetResult = kInvalidPGPKeySetRef;

	err = PGPNewUserIDStringFilter (context, szUserID, 
					kPGPMatchSubString, &filter);

	if (IsntPGPError (err) && PGPFilterRefIsValid (filter)) {

		optionSearch = PGPOUIKeyServerSearchFilter (context, filter);

		if (PGPOptionListRefIsValid (optionSearch)) {

			err = sQueryServerInternal (hwndParent,
										context,
										tlsContext,
										optionSearch,
										uServer,
										szUserID,
										keysetMain,
										&keyset);

			if (IsntPGPError (err)) 
			{	
				*pkeysetResult = keyset;
				keyset = kInvalidPGPKeySetRef;
			}
		}
	}

	if (PGPKeySetRefIsValid (keyset))
		PGPFreeKeySet (keyset);
	if (PGPFilterRefIsValid (filter))
		PGPFreeFilter (filter);


	return err;
}

//	___________________________________________________
//
//	Update keyset from specified keyserver

PGPError PGPclExport 
PGPclUpdateKeySetFromServer (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		PGPKeySetRef		keysetToUpdate,
		UINT				uServer,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef*		pkeysetResult)
{
	PGPError				err				= kPGPError_NoErr;
	PGPKeySetRef			keysetCollect	= kInvalidPGPKeySetRef;
	PGPOptionListRef		optionSearch	= kInvalidPGPOptionListRef;
	PGPKeyListRef			keylist			= kInvalidPGPKeyListRef;
	PGPKeyIterRef			keyiter			= kInvalidPGPKeyIterRef;
	PGPKeySetRef			keyset			= kInvalidPGPKeySetRef;

	PGPKeyID				keyid;

	// initialize return keyset
	PGPValidatePtr (pkeysetResult);
	*pkeysetResult = kInvalidPGPKeySetRef;

	if (uServer == PGPCL_USERIDBASEDSERVER) {
		PGPKeyRef		key			= kInvalidPGPKeyRef;

		CHAR			szUserID[kPGPMaxUserIDSize+1];
		PGPSize			size;

		err = PGPNewKeySet (context, &keysetCollect); CKERR;
		err = PGPOrderKeySet (keysetToUpdate, 
								kPGPAnyOrdering, &keylist); CKERR;
		err = PGPNewKeyIter (keylist, &keyiter); CKERR;

		while (	IsntPGPError (err) &&
				IsntPGPError (PGPKeyIterNext (keyiter, &key)) && 
				PGPKeyRefIsValid (key))
		{
			err = PGPGetPrimaryUserIDNameBuffer (key, kPGPMaxUserIDSize,
							szUserID, &size); CKERR;

			err = PGPGetKeyIDFromKey (key, &keyid); CKERR;

			optionSearch = PGPOUIKeyServerSearchKeyIDList (context,
								1, &keyid);

			if (PGPOptionListRefIsValid (optionSearch)) {

				err = sQueryServerInternal (hwndParent,
											context,
											tlsContext,
											optionSearch,
											uServer,
											szUserID,
											keysetMain,
											&keyset); 

				if (IsPGPError (PGPclErrorBox (hwndParent, err)))
					err = kPGPError_NoErr;

				if (IsntPGPError (err))
				{
					if (PGPKeySetRefIsValid (keyset))
					{	
						err = PGPAddKeys (keyset, keysetCollect);
						if (IsntPGPError (PGPclErrorBox (hwndParent, err))) 
						{
							err = PGPCommitKeyRingChanges (keysetCollect);
							PGPclErrorBox (hwndParent, err);
						}
					}
				}

				if (PGPKeySetRefIsValid (keyset))
				{
					PGPFreeKeySet (keyset);
					keyset = kInvalidPGPKeySetRef;
				}
			}
		}

		if (PGPKeySetRefIsValid (keysetCollect)) 
		{	
			*pkeysetResult = keysetCollect;
			keysetCollect = kInvalidPGPKeySetRef;
		}
	}

	else { 
		optionSearch = PGPOUIKeyServerSearchKeySet (context, keysetToUpdate);

		if (PGPOptionListRefIsValid (optionSearch)) {

			err = sQueryServerInternal (hwndParent,
										context,
										tlsContext,
										optionSearch,
										uServer,
										"",
										keysetMain,
										&keysetCollect);

			if (IsntPGPError (err)) 
			{	
				*pkeysetResult = keysetCollect;
				keysetCollect = kInvalidPGPKeySetRef;
			}
		}
	}

done :
	if (PGPKeyListRefIsValid (keylist))
		PGPFreeKeyList (keylist);
	if (PGPKeyIterRefIsValid (keyiter))
		PGPFreeKeyIter (keyiter);
	if (PGPKeySetRefIsValid (keyset))
		PGPFreeKeySet (keyset);
	if (PGPKeySetRefIsValid (keysetCollect))
		PGPFreeKeySet (keysetCollect);

	return err;
}

//	___________________________________________________
//
//	search keyserver for keys with these keyids

PGPError PGPclExport 
PGPclSearchServerForKeyIDs (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		PGPKeyID*			pkeyidList,
		INT					iNumKeyIDs,
		UINT				uServer,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef*		pkeysetResult)
{	
	PGPError				err				= kPGPError_NoErr;
	PGPKeySetRef			keyset			= kInvalidPGPKeySetRef;
	PGPOptionListRef		optionSearch	= kInvalidPGPOptionListRef;

	// initialize return keyset
	PGPValidatePtr (pkeysetResult);
	*pkeysetResult = kInvalidPGPKeySetRef;

	optionSearch = PGPOUIKeyServerSearchKeyIDList (context,
						iNumKeyIDs, pkeyidList );

	if (PGPOptionListRefIsValid (optionSearch)) {

		err = sQueryServerInternal (hwndParent,
									context,
									tlsContext,
									optionSearch,
									uServer,
									"",
									keysetMain,
									&keyset);

		if (IsntPGPError (err)) 
		{	
			*pkeysetResult = keyset;
			keyset = kInvalidPGPKeySetRef;
		}
	}

	if (PGPKeySetRefIsValid (keyset))
		PGPFreeKeySet (keyset);

	return err;
}

//	___________________________________________________
//
//	search specified keyserver using specified filter

PGPError PGPclExport 
PGPclSearchServerWithFilter (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwndParent, 
		PGPFilterRef		filter,
		UINT				uServer,
		PGPKeySetRef		keysetMain,
		PGPKeySetRef*		pkeysetResult) 
{
	PGPError				err				= kPGPError_NoErr;
	PGPKeySetRef			keyset			= kInvalidPGPKeySetRef;
	PGPOptionListRef		optionSearch	= kInvalidPGPOptionListRef;
	
	// initialize return keyset
	PGPValidatePtr (pkeysetResult);
	*pkeysetResult = kInvalidPGPKeySetRef;

	optionSearch = PGPOUIKeyServerSearchFilter (context, filter);
	if (PGPOptionListRefIsValid (optionSearch))
	{
		err = sQueryServerInternal (	hwndParent,
										context,
										tlsContext,
										optionSearch,
										uServer,
										"",
										keysetMain,
										&keyset); 
		if (IsntPGPError (err)) 
		{	
			*pkeysetResult = keyset;
			keyset = kInvalidPGPKeySetRef;
		}
	}

	if (PGPKeySetRefIsValid (keyset))
		PGPFreeKeySet (keyset);

	return err;
}


