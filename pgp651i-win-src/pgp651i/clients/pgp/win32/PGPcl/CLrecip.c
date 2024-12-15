/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLrecip.c - Glue code to allow clients to access
	            recipient dialog in PGPsdkUI

	$Id: CLrecip.c,v 1.47 1999/03/26 18:28:08 wjb Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpclx.h"
#include "pgpUserInterface.h"
extern HINSTANCE g_hInst;

#define SDKUITEST 0

/*
old RECIPIENTDIALOGSTRUCT for reference

typedef struct _recipientdialogstruct
{
	DWORD			Version;
	HWND			hwndParent;
	char *			szTitle;
	PGPContextRef	Context;
	PGPKeySetRef 	OriginalKeySetRef;
	PGPKeySetRef 	SelectedKeySetRef;
	char **			szRecipientArray;
	DWORD			dwNumRecipients;
	DWORD			dwOptions;
	DWORD			dwFlags;
	DWORD			dwDisableFlags;
} RECIPIENTDIALOGSTRUCT, *PRECIPIENTDIALOGSTRUCT;
*/

BOOL DefaultKeyCheck(HWND hwnd,
				PGPKeyRef DefaultKey,
				PGPKeySetRef SelectedKeySet)
{
    PGPKeyRef key;
	PGPError err;
 
	// We don't even know their default key!
	if(!PGPKeyRefIsValid(DefaultKey))
		return FALSE;

	err=PGPGetDefaultPrivateKey(SelectedKeySet,&key);

	if(IsPGPError(err))
		return FALSE;

	return TRUE;
}

BOOL RSACheck(HWND hwnd,
			   PGPKeySetRef SelectedKeySet)
{
    PGPKeyListRef list;
    PGPKeyIterRef iter;
    PGPKeyRef key;
	PGPBoolean FoundRSAKey;
	PGPInt32 algorithm;
  
    PGPOrderKeySet(SelectedKeySet,kPGPReverseUserIDOrdering,&list);
    PGPNewKeyIter(list,&iter);

	FoundRSAKey=FALSE;

    while (IsntPGPError(PGPKeyIterNext(iter,&key)))
    {
		PGPGetKeyNumber( key, kPGPKeyPropAlgID, &algorithm );
		if(algorithm == kPGPPublicKeyAlgorithm_RSA)
			FoundRSAKey=TRUE;
	}

	PGPFreeKeyIter(iter);
    PGPFreeKeyList(list);
 
	return FoundRSAKey;
}

BOOL CorpCheck(HWND hwnd,
			   PGPKeySetRef MainKeySet,
			   PGPKeySetRef SelectedKeySet,
			   PGPKeyRef CorpKey)
{
    PGPUserIDRef userID;
    PGPKeyListRef list;
    PGPKeyIterRef iter;
    PGPKeyRef key;
	char UserIDStr[kPGPMaxUserIDSize];
	int len;
	PGPBoolean SignedByCorp;
	PGPError err;
  	PGPSigRef sig;
	PGPKeyRef certifierKey;
 
    PGPOrderKeySet(SelectedKeySet,kPGPReverseUserIDOrdering,&list);
    PGPNewKeyIter(list,&iter);

    while (IsntPGPError(PGPKeyIterNext(iter,&key)))
    {
		SignedByCorp=FALSE;
		strcpy(UserIDStr,"");

        while (IsntPGPError(PGPKeyIterNextUserID(iter, &userID)))
        {
			if(!strcmp(UserIDStr,""))
			{
				PGPGetUserIDStringBuffer(userID, kPGPUserIDPropName,
    				kPGPMaxUserIDSize-1, UserIDStr, &len);
			}

			while(IsntPGPError(err=PGPKeyIterNextUIDSig(iter,&sig)))
			{
				err = PGPGetSigCertifierKey(
					sig, MainKeySet, &certifierKey);

				if(IsntPGPError(err))
				{
					if(certifierKey == CorpKey)
						SignedByCorp=TRUE;
				}
			}
		}
		
		if(!SignedByCorp)
		{
			UINT msgrst;
			char StrRes[500];

			LoadString (g_hInst, IDS_KEYNOTSIGNEDBYCORP, 
											StrRes, sizeof(StrRes));

			msgrst=MessageBox(hwnd,StrRes,UserIDStr,
				MB_YESNO|MB_ICONEXCLAMATION|MB_SETFOREGROUND);

			if(msgrst==IDNO)
			{
				PGPFreeKeyIter(iter);
				PGPFreeKeyList(list);

				return FALSE;
			}
		}
	}

	PGPFreeKeyIter(iter);
    PGPFreeKeyList(list);
 
	return TRUE;
}

typedef struct 
{
	// Admins Prefs
	PGPBoolean allowConvEncrypt;
	PGPBoolean WarnNotCorpSigned;
	PGPBoolean haveOutgoingADK;
	PGPBoolean enforceOutgoingADK;
	PGPBoolean enforceRemoteADKClass;
	PGPKeyRef corpKey;
	PGPKeyRef outgoingADK;

	// Client Prefs
	PGPBoolean bWarnOnADK;
	PGPBoolean showMarginalValidity;
	PGPBoolean marginalIsInvalid;
	PGPBoolean encryptToSelf;
	PGPBoolean synchUnknown;
	PGPKeyRef defaultKey;

	// KeyServer (Client) Prefs
	PGPKeyServerEntry *ksEntries;
	PGPUInt32 numKSEntries;
	PGPKeyServerSpec *serverList;
	PCLIENTSERVERSTRUCT	pcss;
} APREF;

void NoDefaultKey(HWND hwnd)
{
	PGPclMessageBox(hwnd,IDS_CAPTIONERROR,IDS_NOVALIDDEFAULTKEY,
		MB_OK|MB_ICONEXCLAMATION);
}

void NoOutADKKey(HWND hwnd)
{
	PGPclMessageBox(hwnd,IDS_CAPTIONERROR,IDS_NOOUTADK,
		MB_OK|MB_ICONSTOP);
}

void NoCorpKey(HWND hwnd)
{
	PGPclMessageBox(hwnd,IDS_CAPTIONERROR,IDS_NOCORPSIGNKEY,
		MB_OK|MB_ICONSTOP);
}

PGPError GrabRecipClientPrefs(HWND hwndParent,
							  PGPContextRef context,
							  PGPKeySetRef keyset,
							  APREF *apref)
{
	PGPMemoryMgrRef	memMgr;
	PGPPrefRef	clientPrefsRef;
	PGPError	err;

	memMgr=PGPGetContextMemoryMgr(context);

	err=PGPclOpenClientPrefs (memMgr,&clientPrefsRef);

	if(IsntPGPError(err))
	{
		err=CLInitKeyServerPrefs(
			PGPCL_DEFAULTSERVER,NULL,
			hwndParent,
			context,
			keyset,
			clientPrefsRef,"",
			&(apref->pcss),
			&(apref->ksEntries),
			&(apref->serverList),
			&(apref->numKSEntries));

		PGPGetPrefBoolean( clientPrefsRef, kPGPPrefWarnOnADK, 
				&(apref->bWarnOnADK));
		PGPGetPrefBoolean( clientPrefsRef, kPGPPrefDisplayMarginalValidity,
				&(apref->showMarginalValidity));
		PGPGetPrefBoolean( clientPrefsRef, kPGPPrefMarginalIsInvalid,
				&(apref->marginalIsInvalid));
		PGPGetPrefBoolean( clientPrefsRef, kPGPPrefEncryptToSelf,
				&(apref->encryptToSelf));
		PGPGetPrefBoolean( clientPrefsRef, kPGPPrefKeyServerSyncUnknownKeys,
				&(apref->synchUnknown));
		PGPclCloseClientPrefs (clientPrefsRef, FALSE);
	}

	if(IsntPGPError(err))
	{
		err=PGPGetDefaultPrivateKey(keyset,&(apref->defaultKey));

		if(IsPGPError(err))
		{
			// Warning message here
			apref->defaultKey=NULL;
			err=kPGPError_NoErr;
		}
	}

	return err;
}

PGPError GrabRecipAdminPrefs(HWND hwndParent,
							 PGPContextRef context,
							 PGPKeySetRef keyset,
							 APREF *apref)
{
	PGPByte		*adKeyID;
	PGPSize		adKeyIDSize;
	PGPUInt32	adKeyAlgorithm;
	PGPByte		*corpKeyID;
	PGPSize		corpKeyIDSize;
	PGPUInt32	corpKeyAlgorithm;
	PGPMemoryMgrRef	memMgr;
	PGPPrefRef	adminPrefsRef;
	PGPError	err;

	memMgr=PGPGetContextMemoryMgr(context);

//>>>??? #if PGP_BUSINESS_SECURITY	
	err = PGPclOpenAdminPrefs (memMgr,&adminPrefsRef, PGPclIsAdminInstall());
//>>>??? #endif	// PGP_BUSINESS_SECURITY

	if(IsntPGPError(err))
	{
		// Allow Conventional
		PGPGetPrefBoolean( adminPrefsRef,
			kPGPPrefAllowConventionalEncryption,
			&(apref->allowConvEncrypt));

		// Warn if recipient not signed by corp key
		PGPGetPrefBoolean( adminPrefsRef,
			kPGPPrefWarnNotCertByCorp,
			&(apref->WarnNotCorpSigned));

		// Use an outgoing ADK
		PGPGetPrefBoolean( adminPrefsRef, 
			kPGPPrefUseOutgoingADK,
			&(apref->haveOutgoingADK));

		// Enforce this outgoing ADK
		PGPGetPrefBoolean( adminPrefsRef, 
			kPGPPrefEnforceOutgoingADK,
			&(apref->enforceOutgoingADK));

		// Enforce remote ADKs
		PGPGetPrefBoolean( adminPrefsRef,
			kPGPPrefEnforceRemoteADKClass,
			&(apref->enforceRemoteADKClass));

		// Get corp key ref
		PGPGetPrefData(adminPrefsRef, 
			kPGPPrefCorpKeyID, 
			&corpKeyIDSize, 
			&corpKeyID);

		PGPGetPrefNumber( adminPrefsRef,
			kPGPPrefCorpKeyPublicKeyAlgorithm, 
			&corpKeyAlgorithm);

		// Get outgoing adk key
		PGPGetPrefData( adminPrefsRef, 
			kPGPPrefOutgoingADKID,
			&adKeyIDSize, &adKeyID );

		PGPGetPrefNumber( adminPrefsRef,
			kPGPPrefOutADKPublicKeyAlgorithm,
			&adKeyAlgorithm );

		PGPclCloseAdminPrefs (adminPrefsRef, FALSE);
	}

	if((apref->haveOutgoingADK)&&(IsntPGPError(err)))
	{
		err=PGPclGetKeyFromKeyID (
			context,
			keyset,
			adKeyID,
			adKeyAlgorithm,
			&(apref->outgoingADK));

		if(IsPGPError(err))
		{
			// Warning message here
			apref->outgoingADK=NULL;
			err=kPGPError_NoErr;
		}
	}
	
	if(adKeyID)
		PGPDisposePrefData( adminPrefsRef, adKeyID);

	if((apref->WarnNotCorpSigned)&&(IsntPGPError(err)))
	{
		err=PGPclGetKeyFromKeyID (
			context,
			keyset,
			corpKeyID,
			corpKeyAlgorithm,
			&(apref->corpKey));

		if(IsPGPError(err))
		{
			// Warning message here
			apref->corpKey=NULL;
			err=kPGPError_NoErr;
		}
	}

	if(corpKeyID)
		PGPDisposePrefData( adminPrefsRef, corpKeyID);

	return err;
}

UINT PGPclExport PGPclRecipientDialog(PRECIPIENTDIALOGSTRUCT prds)
{
	PGPOptionListRef			rdoptionList;
	PGPOptionListRef			optionList;
	PGPUInt32					conventionalEncrypt,
								textOutput,
								wipeOriginal,
								fyeo,
								sda;
	Boolean						haveFileOptions;
	PGPclGROUPFILE				*pGroup;
	PGPKeySetRef				foundKeys;
	PGPUInt32					numKeys;
	APREF						apref;
	PGPUInt32					arrEnforcement;
	PGPError					err;
	char						StrRes1[100],StrRes2[300],StrRes3[100];

	memset(&apref,0x00,sizeof(APREF));
	apref.allowConvEncrypt = TRUE;

	pGroup=NULL;
	err=kPGPError_NoErr;
	foundKeys=kPGPInvalidRef;
	rdoptionList = kInvalidPGPOptionListRef;
	optionList = kInvalidPGPOptionListRef;
	haveFileOptions=FALSE;
	arrEnforcement = kPGPARREnforcement_Warn;
	textOutput=conventionalEncrypt=wipeOriginal=fyeo=sda=0;

	// Create Null Option List for recipient dialog
	err = PGPBuildOptionList( prds->Context, &rdoptionList,
		PGPOLastOption( prds->Context ) );

// ******************BUSINESS SECURITY***********************
#if PGP_BUSINESS_SECURITY 

	if(IsntPGPError(err))
	{
		err=GrabRecipAdminPrefs(prds->hwndParent,
					prds->Context,
					prds->OriginalKeySetRef,
					&apref);
	}

	if(IsntPGPError(err))
	{
		arrEnforcement = kPGPARREnforcement_Warn;

		if(apref.enforceRemoteADKClass)
			arrEnforcement=kPGPARREnforcement_Strict;

		if(apref.WarnNotCorpSigned)
		{
			if(apref.corpKey==NULL)
			{
// Warn if we can't find it AND exit since required.
				NoCorpKey(prds->hwndParent);
				return FALSE;
			}
		}

		if(apref.haveOutgoingADK)
		{
			if(PGPKeyRefIsValid(apref.outgoingADK))
			{
				PGPRecipientSpec	spec;
		
				pgpClearMemory( &spec, sizeof( spec ) );
				
				spec.type	= kPGPRecipientSpecType_Key;
				spec.locked	= apref.enforceOutgoingADK;
				spec.u.key	= apref.outgoingADK;
			
				err = PGPAppendOptionList( rdoptionList,
					PGPOUIDefaultRecipients( prds->Context, 1, &spec ),
					PGPOLastOption( prds->Context ) );
			}
			else
			{
				if(apref.enforceOutgoingADK)
				{
					// HaveOutgoing, Enforced, but no key? No way!
					NoOutADKKey(prds->hwndParent);
					err = kPGPError_UserAbort;
				}
				else
				{
					err = kPGPError_NoErr;
				}
			}
		}
	}
#endif	
// ******************END BUSINESS SECURITY*******************

	// Get client prefs
	if(IsntPGPError(err))
	{
		err=GrabRecipClientPrefs(prds->hwndParent,
			prds->Context,
			prds->OriginalKeySetRef,
			&apref);
	}

	// Add recipients
	if((IsntPGPError(err))&&
		(prds->szRecipientArray!=NULL)&&
		(prds->dwNumRecipients!=0))
	{
		PGPRecipientSpec	*spec;
		int memamt,i;

		memamt=sizeof(PGPRecipientSpec)*prds->dwNumRecipients;
		spec=(PGPRecipientSpec *)malloc(memamt);
		memset(spec,0x00,memamt);

		for(i=0;i<(int)prds->dwNumRecipients;i++)
		{
			spec[i].type=kPGPRecipientSpecType_UserID;
			strcpy(spec[i].u.userIDStr,prds->szRecipientArray[i]);
		}
					
		err = PGPAppendOptionList( rdoptionList,
			PGPOUIDefaultRecipients( prds->Context,
										prds->dwNumRecipients,spec),
			PGPOLastOption( prds->Context ) );
			
		free(spec);
	}

	// Open groups file
	if( IsntPGPError( err ) )
	{
		err=PGPclOpenGroupFile (prds->Context,&pGroup);
			
		if(IsntPGPError(err))
		{
			err = PGPAppendOptionList( rdoptionList,
				PGPOUIRecipientGroups( prds->Context, pGroup->groupset ),
				PGPOLastOption( prds->Context ) );
		}
	}

	// AddDefaultKey
	if( IsntPGPError( err ) && apref.encryptToSelf )
	{
		if(PGPKeyRefIsValid(apref.defaultKey))
		{
			PGPRecipientSpec	spec;
				
			pgpClearMemory( &spec, sizeof( spec ) );
					
			spec.type	= kPGPRecipientSpecType_Key;
			spec.u.key	= apref.defaultKey;
						
			err = PGPAppendOptionList( rdoptionList,
				PGPOUIDefaultRecipients( prds->Context, 1, &spec ),
				PGPOLastOption( prds->Context ) );
		}
		else
		{
			NoDefaultKey(prds->hwndParent);
		}
	}

	// Make options control
	if( IsntPGPError( err ) )
	{
		err = PGPBuildOptionList( prds->Context, &optionList,
			PGPOLastOption( prds->Context ) );

		if(prds->dwOptions&PGPCL_ASCIIARMOR)
			textOutput=1;

		if((prds->dwOptions&PGPCL_PASSONLY)&&(apref.allowConvEncrypt))
			conventionalEncrypt=1;

		if(prds->dwOptions&PGPCL_WIPEORIG)
			wipeOriginal=1;

		if(prds->dwOptions&PGPCL_FYEO)
			fyeo=1;

		if(prds->dwOptions&PGPCL_SDA)
			sda=1;

#if SDKUITEST
		// Just some code here to stress test the sdkUI options....
		{
			static char one[]="one";
			static char two[]="two";
			char *list[2];
			static char descript[]=
				"She smiled at a sorrowful stranger. "
				"The smile seemed to make him feel better. "
				"He remembered past kindnesses of a friend "
				"And wrote him a thank you letter.";

			list[0]=one;
			list[1]=two;

			err = PGPAppendOptionList( optionList,
					PGPOUIPopupList( prds->Context, 900, one, descript, 2,
						list,0, &sda,
						PGPOUIPopupList( prds->Context, 901, two, descript, 2,
							list,0, &sda,
							PGPOLastOption( prds->Context ) ),
						PGPOLastOption( prds->Context ) ),
					PGPOLastOption( prds->Context ) );

			err = PGPAppendOptionList( optionList,
					PGPOUIPopupList( prds->Context, 902, one, descript, 2,
						list,0, &sda,
						PGPOUIPopupList( prds->Context, 903, two, descript, 2,
							list,0, &sda,
							PGPOLastOption( prds->Context ) ),
						PGPOLastOption( prds->Context ) ),
					PGPOLastOption( prds->Context ) );
		}
#endif

		if(!(prds->dwDisableFlags&PGPCL_DISABLE_ASCIIARMOR))
		{
			haveFileOptions=TRUE;
			LoadString (g_hInst, IDS_TEXTOUTPUT, StrRes1, sizeof(StrRes1));

			err = PGPAppendOptionList( optionList,
						PGPOUICheckbox( prds->Context, 801,
							StrRes1,NULL,
							textOutput, &textOutput,
							PGPOLastOption( prds->Context ) ),
						PGPOLastOption( prds->Context ) );
		}

		if(!(prds->dwDisableFlags&PGPCL_DISABLE_WIPEORIG))
		{
			haveFileOptions=TRUE;
			LoadString (g_hInst, IDS_WIPEORIGINAL, StrRes3, sizeof(StrRes3));

			err = PGPAppendOptionList( optionList,
						PGPOUICheckbox( prds->Context, 803,
							StrRes3,NULL,
							wipeOriginal, &wipeOriginal,
							PGPOLastOption( prds->Context ) ),
						PGPOLastOption( prds->Context ) );
		}

		if(!(prds->dwDisableFlags&PGPCL_DISABLE_FYEO))
		{
			haveFileOptions=TRUE;
			LoadString (g_hInst, IDS_FYEO, StrRes3, sizeof(StrRes3));

			err = PGPAppendOptionList( optionList,
						PGPOUICheckbox( prds->Context, 805,
							StrRes3,NULL,
							fyeo, &fyeo, PGPOLastOption( prds->Context ) ),
						PGPOLastOption( prds->Context ) );
		}

		if((!(prds->dwDisableFlags&PGPCL_DISABLE_PASSONLY))&&
			(apref.allowConvEncrypt))
		{
			PGPOptionListRef			ConvOptionList;

			// Create NULL suboption list for conventional encryption
			err = PGPBuildOptionList( prds->Context, &ConvOptionList,
				PGPOLastOption( prds->Context ) );

			// If SDA is specified, make it a suboption
			if(!(prds->dwDisableFlags&PGPCL_DISABLE_SDA))
			{
				haveFileOptions=TRUE;
				LoadString (g_hInst, IDS_SDA, StrRes3, sizeof(StrRes3));

				err = PGPAppendOptionList( ConvOptionList,
						PGPOUICheckbox( prds->Context, 806,
							StrRes3,NULL,
							sda, &sda, PGPOLastOption( prds->Context ) ),
						PGPOLastOption( prds->Context ) );
			}

			haveFileOptions=TRUE;
			LoadString (g_hInst, IDS_CONVENCRYPTION, 
										StrRes2, sizeof(StrRes2));

			err = PGPAppendOptionList( optionList,
						PGPOUICheckbox( prds->Context, 802,
							StrRes2,NULL,
							conventionalEncrypt, &conventionalEncrypt,
							ConvOptionList,
							PGPOLastOption( prds->Context ) ),
						PGPOLastOption( prds->Context ) );

			PGPFreeOptionList(ConvOptionList);
		}
	}

	if( IsntPGPError( err ) && ( haveFileOptions ))
	{
		err = PGPAppendOptionList( rdoptionList,
				PGPOUIDialogOptions( prds->Context,
					optionList,
					PGPOLastOption( prds->Context ) ),
				PGPOLastOption( prds->Context ) );
	}

	// Call recipient dialog
	if( IsntPGPError( err ))
	{
/*		Test for the options dialog. Leave commented out.
		err=PGPOptionsDialog(prds->Context,
			optionList,
			PGPOLastOption( prds->Context ));
*/
		err=PGPRecipientDialog(prds->Context,
			prds->OriginalKeySetRef,
			(unsigned char)(prds->dwDisableFlags&PGPCL_DISABLE_AUTOMODE),
			&(prds->SelectedKeySetRef),
			rdoptionList,
			PGPOUIParentWindowHandle(prds->Context,prds->hwndParent),
			PGPOUIWindowTitle(prds->Context,prds->szTitle),
			PGPOUIDisplayMarginalValidity( prds->Context, 
				apref.showMarginalValidity ),
			PGPOUIIgnoreMarginalValidity( prds->Context, 
				apref.marginalIsInvalid ),
			PGPOUIKeyServerUpdateParams(prds->Context,
				apref.numKSEntries,
				apref.serverList, prds->tlsContext,
				apref.synchUnknown,&foundKeys,
				PGPOLastOption( prds->Context ) ),
			PGPOUIEnforceAdditionalRecipientRequests( prds->Context,
				arrEnforcement, apref.bWarnOnADK ),
			PGPOLastOption( prds->Context ) );

		// If we had options, read the results and send em back
		if( IsntPGPError( err ) && haveFileOptions )
		{
			prds->dwOptions&=(~PGPCL_ASCIIARMOR);
			
			if( textOutput != 0 )
				prds->dwOptions |= PGPCL_ASCIIARMOR;

			prds->dwOptions&=(~PGPCL_PASSONLY);

			if( conventionalEncrypt != 0 )
				prds->dwOptions |= PGPCL_PASSONLY;

			prds->dwOptions&=(~PGPCL_WIPEORIG);

			if( wipeOriginal != 0 )
				prds->dwOptions |= PGPCL_WIPEORIG;

			prds->dwOptions&=(~PGPCL_FYEO);

			if( fyeo != 0 )
				prds->dwOptions |= PGPCL_FYEO;

			prds->dwOptions&=(~PGPCL_SDA);

			if( sda != 0 )
				prds->dwOptions |= PGPCL_SDA;
		}
	}

	CLUninitKeyServerPrefs(PGPCL_DEFAULTSERVER,
		apref.pcss,apref.ksEntries,
		apref.serverList,apref.numKSEntries);

	if(optionList)
		PGPFreeOptionList( optionList );

	if(rdoptionList)
		PGPFreeOptionList( rdoptionList );

	if(pGroup)
		PGPclCloseGroupFile(pGroup);

	prds->AddedKeys=foundKeys;

	// Do some last minute checks
	if(IsntPGPError(err))
	{
		PGPCountKeys( prds->SelectedKeySetRef, &numKeys );

		if((numKeys==0)&&(!conventionalEncrypt)&&(!sda))
		{
			PGPclMessageBox(prds->hwndParent,
				IDS_CAPTIONERROR,IDS_ATLEASTONEORCONV,
				MB_OK|MB_ICONSTOP);
			return FALSE;
		}

		// Check to make sure they have encrypted to
		// themselves if WIPEORIG is checked.
		if((prds->dwOptions&PGPCL_WIPEORIG)&&
			((prds->dwOptions&PGPCL_PASSONLY)==0)&&
			((prds->dwOptions&PGPCL_SDA)==0))
		{
			if(!DefaultKeyCheck(prds->hwndParent,
				apref.defaultKey,
				prds->SelectedKeySetRef))
			{
				int msgrst;
	
				LoadString (g_hInst, IDS_CAPTION, 
					StrRes1, sizeof(StrRes1));
				LoadString (g_hInst, IDS_WIPEWODEFAULT, 
					StrRes2, sizeof(StrRes2));

				msgrst=MessageBox(prds->hwndParent,StrRes2,StrRes1,
					MB_YESNO|MB_ICONEXCLAMATION|MB_SETFOREGROUND);

				if(msgrst==IDNO)
					return FALSE;
			}
		}

		prds->dwFlags&=(~PGPCL_RSAENCRYPT);

		// So passphrase dialog can check for DSS/RSA combo
		if(RSACheck(prds->hwndParent,
			prds->SelectedKeySetRef))
		{
			prds->dwFlags |= PGPCL_RSAENCRYPT;
		}

// ******************BUSINESS SECURITY***********************
#if PGP_BUSINESS_SECURITY
		if((apref.WarnNotCorpSigned)&&(!conventionalEncrypt))
		{
			if(!CorpCheck(prds->hwndParent,
				prds->OriginalKeySetRef,
				prds->SelectedKeySetRef,
				apref.corpKey))
				err=kPGPError_UserAbort;
		}
#endif	
// ******************END BUSINESS SECURITY*******************
	}

	if(IsPGPError(err))
		return FALSE;

	return TRUE;
}

