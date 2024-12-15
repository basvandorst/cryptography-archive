/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDkeyDB.c,v 1.36 1997/10/21 15:42:34 wjb Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"

/*
 * RecKeyDB.c  Routines needed to fetch and match keys
 *
 * Most of the calls to PGPKeyDB lib are isolated here. The keys are
 * read in from disk, and matches are attempted.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */

BOOL AddLinkedListtoListView(HWND hwndList,PUSERKEYINFO Head)
{
    PUSERKEYINFO pui;
    int result;
	DWORD image;

    pui=Head;

    while(pui!=0)
    {
        if(pui->Algorithm==kPGPPublicKeyAlgorithm_RSA)
            image=IDX_RSAUSERID;
        
		if(pui->Algorithm==kPGPPublicKeyAlgorithm_DSA)
            image=IDX_DSAUSERID;

        if(pui->Flags&PUIF_NOTFOUND)
            image=IDX_INVALIDUSERID;

		if(pui->Flags&PUIF_GROUP)
			image=IDX_GROUP;

		pui->icon=image;
        result=AddAnItem(hwndList,pui);
        pui=pui->next;
    }

    return TRUE;
}

UINT KMConvertFromPGPValidity (UINT uPGPValidity) {
    switch (uPGPValidity) {
    case kPGPValidity_Unknown :
    case kPGPValidity_Invalid :
        return 0;
    case kPGPValidity_Marginal :
        return 1;
    case kPGPValidity_Complete :
        return 2;
    }
}

UINT KMConvertFromPGPTrust (UINT uPGPTrust) {
    switch (uPGPTrust & kPGPKeyTrust_Mask) {
    case kPGPKeyTrust_Undefined    :
    case kPGPKeyTrust_Unknown :
    case kPGPKeyTrust_Never :
        return 0;
    case kPGPKeyTrust_Marginal :
        return 1;
    case kPGPKeyTrust_Complete :
        return 2;
    case kPGPKeyTrust_Ultimate :
        return 3;
    }
}

PUSERKEYINFO AddLink(PUSERKEYINFO head,PUSERKEYINFO pui)
{
    pui->next=head;

    return pui;
}

PUSERKEYINFO AddOneNotFound(char *recipient,
							PUSERKEYINFO *HeadUserLL,
							int flags)
{
	PUSERKEYINFO pui;

    pui = (PUSERKEYINFO) malloc( sizeof(USERKEYINFO) );
    memset (pui,0x00,sizeof(USERKEYINFO));

    strcpy(pui->UserId,recipient);
    pui->KeyRef=0;
    strcpy(pui->szSize,"No Match");
    pui->Validity=0;
    pui->Algorithm=kPGPPublicKeyAlgorithm_Invalid;
    pui->Flags=flags;

    return AddLink(*HeadUserLL,pui);
}

BOOL IsUserIDSignedByCorpKey(PGPKeySetRef mAllKeys,
							 PGPKeyIterRef keyIterator,
							 PGPKeyRef corporateKeyRef)
{
	PGPSigRef sig;
	PGPError err;
	PGPKeyRef certifierKey;

	while(IsntPGPError(err=PGPKeyIterNextUIDSig(keyIterator,&sig)))
	{
		err = PGPGetSigCertifierKey(
			sig, mAllKeys, &certifierKey);

		if(IsntPGPError(err))
		{
			if(certifierKey == corporateKeyRef)
				return TRUE;
		}
	}
	
	return FALSE;
}

void NoDefaultKey(HWND hwnd)
{
	MessageBox(hwnd,
		"Could not find valid Default Key! Please\n"
		"check your settings in PGPkeys.",
		"Recipient Dialog",
		MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND);
}

void NoOutADKKey(HWND hwnd)
{
	MessageBox(hwnd,
		"The Outgoing Additional Decryption Key\n"
		"configured for this PGP could not be found.\n"
		"Please contact your security administrator.",
		"Recipient Dialog",
		MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
}

void NoCorpKey(HWND hwnd)
{
	MessageBox(hwnd,
		"Could not find valid Corporate Signing Key!\n"
		"Contact your security administrator.",
		"Recipient Dialog",
		MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
}

UINT LoadKeys(HWND hwndLoad,PUSERKEYINFO *HeadUserLL,
			PRECIPIENTDIALOGSTRUCT prds,
			PPREFS Prefs)
{
    PGPKeyListRef list;
    PGPKeyIterRef iter;
    PGPKeyRef key;
	PGPSubKeyRef pSubKey;
   
	PGPKeyRef OutADKKey,CorpKey,DefaultKey;
	BOOL CorpKeyOK,OutADKKeyOK,DefaultKeyOK;

	PUSERKEYINFO pui;
    PGPUserIDRef userID;
    UINT uAlg,uKeyBits,uSubKeyBits;
	PGPBoolean KeyRevoked,KeyDisabled,KeyExpired,KeyCanEncrypt,bAxiomatic;
	PGPError err;
    size_t len;

    *HeadUserLL=0;
	OutADKKey=CorpKey=DefaultKey=0;
	CorpKeyOK=OutADKKeyOK=DefaultKeyOK=FALSE;

	// Get default key KeyRef
	if(Prefs->bEncToSelf)
	{
		err=PGPGetDefaultPrivateKey (prds->OriginalKeySetRef, &DefaultKey);
	
		// Just warn if we can't find it. No big deal,
		if(IsPGPError(err))
			NoDefaultKey(hwndLoad);
	}
		
	// Get outgoing ADK key KeyRef
	if(Prefs->bUseOutgoingADK)
	{
		err=PGPcomdlgGetKeyFromKeyID (prds->Context,
			  prds->OriginalKeySetRef,
			  Prefs->outADKKeyID,
			  (PGPPublicKeyAlgorithm)Prefs->outADKKeyAlgorithm,
			  &OutADKKey);

		if(IsPGPError(err))
		{
			// Warn if we can't find it. Exit if required.
			NoOutADKKey(hwndLoad);

			if(Prefs->bEnforceOutgoingADK)
				return FALSE;
		}
	}

	// Get corporate key KeyRef
	if(Prefs->bWarnNotCorpSigned)
	{
		err=PGPcomdlgGetKeyFromKeyID (prds->Context,
			  prds->OriginalKeySetRef,
			  Prefs->corpKeyID,
			  (PGPPublicKeyAlgorithm) Prefs->corpKeyAlgorithm,
			  &CorpKey);

		if(IsPGPError(err))
		{
			// Warn if we can't find it AND exit since required.
			NoCorpKey(hwndLoad);
			return FALSE;
		}
	}
	
    PGPOrderKeySet(prds->OriginalKeySetRef,kPGPReverseUserIDOrdering,&list);
    PGPNewKeyIter(list,&iter);

    while (IsntPGPError(PGPKeyIterNext(iter,&key)))
    {
		PGPGetKeyBoolean (key, kPGPKeyPropIsRevoked, &KeyRevoked);
		if(KeyRevoked)
			continue;

		PGPGetKeyBoolean (key, kPGPKeyPropIsDisabled, &KeyDisabled);
		if(KeyDisabled)
			continue;

		PGPGetKeyBoolean (key, kPGPKeyPropIsExpired, &KeyExpired);
		if(KeyExpired)
			continue;

        uAlg=uKeyBits=uSubKeyBits=0;
        PGPGetKeyNumber (key, kPGPKeyPropAlgID, &uAlg);

        if((uAlg!=kPGPPublicKeyAlgorithm_RSA)&&
			(uAlg!= kPGPPublicKeyAlgorithm_RSAEncryptOnly)&&
			(uAlg!=kPGPPublicKeyAlgorithm_DSA))
            continue;

        PGPGetKeyNumber (key, kPGPKeyPropBits, &uKeyBits);
        if(uAlg==kPGPPublicKeyAlgorithm_DSA)
        {
            PGPKeyIterNextSubKey (iter, &pSubKey);
            if(pSubKey==0)
                continue;
            PGPGetSubKeyNumber (pSubKey, kPGPKeyPropBits, &uSubKeyBits);
        }
 
		// Check CorpKey (doesn't have to encrypt though)
		if(CorpKey==key)
			CorpKeyOK=TRUE;

		PGPGetKeyBoolean (key, kPGPKeyPropCanEncrypt, &KeyCanEncrypt);
		if(!KeyCanEncrypt)
			continue;

        while (IsntPGPError(PGPKeyIterNextUserID(iter, &userID)))
        {
            pui = (PUSERKEYINFO) malloc( sizeof(USERKEYINFO) );
            memset (pui,0x00,sizeof(USERKEYINFO));
		
			pui->KeyRef=key;

			PGPGetUserIDStringBuffer(userID, kPGPUserIDPropName,
    			kPGPMaxUserIDSize-1, pui->UserId, &len);

			PGPGetUserIDNumber(userID, kPGPUserIDPropValidity, 
				&(pui->Validity));
			pui->Validity= KMConvertFromPGPValidity(pui->Validity);

			PGPGetKeyNumber(key, kPGPKeyPropTrust, &(pui->Trust));
			pui->Trust= KMConvertFromPGPTrust (pui->Trust);

			PGPGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &bAxiomatic);

			if (bAxiomatic) 
			{
				pui->Trust = 
					KMConvertFromPGPTrust (kPGPKeyTrust_Ultimate) + 1;        
				pui->Validity = 
					KMConvertFromPGPValidity (kPGPValidity_Complete) + 1;
			}

			pui->Algorithm=uAlg;

            if(pui->Algorithm==kPGPPublicKeyAlgorithm_RSA)
			{
				pui->Flags|=PUIF_HASRSA;
                sprintf(pui->szSize,"%i", uKeyBits);
			}
            else
			{
				pui->Flags|=PUIF_HASDSA;
                sprintf(pui->szSize,"%i/%i", uSubKeyBits, uKeyBits);
			}

			// Retreive ADK's.

			pui->ADK=GetADKList(pui,key,prds->OriginalKeySetRef,pui->ADK);

			if(Prefs->bWarnNotCorpSigned)
			{
				if(IsUserIDSignedByCorpKey(prds->OriginalKeySetRef,
					iter,
					CorpKey))
					pui->Flags|=PUIF_SIGNEDBYCORP;
			}

			// Check DefaultKey and Outgoing ADK key (do encrypt)
			if((DefaultKey==key)&&(DefaultKeyOK==FALSE))
			{
				pui->Flags|=PUIF_SELECTED;
				pui->Flags|=PUIF_SYSTEMKEY;
				DefaultKeyOK=TRUE;
			}

			if((OutADKKey==key)&&(OutADKKeyOK==FALSE))
			{
				pui->Flags|=PUIF_SELECTED;
				pui->Flags|=PUIF_SYSTEMKEY;
				OutADKKeyOK=TRUE;
			}

            *HeadUserLL=AddLink(*HeadUserLL,pui); 
        }
    }

    PGPFreeKeyIter(iter);
    PGPFreeKeyList(list);
 
	if((CorpKey!=0)&&(!CorpKeyOK))
	{
		NoCorpKey(hwndLoad);
		return FALSE;
	}

	if((OutADKKey!=0)&&(!OutADKKeyOK))
	{
		// Warn them first.
		NoOutADKKey(hwndLoad);
		// If we enforce, we can't continue
		if(Prefs->bEnforceOutgoingADK)
			return FALSE;
	}

	if((DefaultKey!=0)&&(!DefaultKeyOK))
		NoDefaultKey(hwndLoad);

	Prefs->CorpKey=CorpKey;
	Prefs->RSAADKKey=OutADKKey;

    return TRUE;
}

PUSERKEYINFO KeyIsEncryptable(PUSERKEYINFO headpui,PGPKeyRef key)
{
    while(headpui!=0)
    {
        if(headpui->KeyRef==key)
            return headpui;
        headpui=headpui->next;
    }

	return 0;
}

	static PGPValidity
RDMinValidity(
	PGPValidity	validity1,
	PGPValidity	validity2 )
{
	PGPValidity	validity;
	
	if ( validity1 == kPGPValidity_Unknown ||
		validity2 == kPGPValidity_Unknown )
	{
		/* lowest we can get */
		validity	= kPGPValidity_Invalid;
		return( validity );
	}
	
	/* asserts above guarantee we can use a simple MIN here */

	if ( validity1 < validity2 )
		validity	= validity1;
	else
		validity	= validity2;
	
	return( validity );
}

PGPError RDGroupItemToKey(
	PGPGroupItem const *	item,
	PGPKeySetRef			keySet,
	PGPKeyRef *				outKey )
{
	PGPError		err	= kPGPError_NoErr;
	PGPContextRef	context;
	PGPKeyRef		key	= kPGPInvalidRef;
	PGPKeyID		keyID;
	
	context	= PGPGetKeySetContext( keySet );
	
	/* get the key ID into a PGPsdk form */
	err	= PGPImportKeyID(item->u.key.exportedKeyID,&keyID );
	if ( IsntPGPError( err ) )
	{
		err	= PGPGetKeyByKeyID( keySet, &keyID,
				(PGPPublicKeyAlgorithm)item->u.key.algorithm, &key );
	}
	
	*outKey	= key;
	
	return( err );
}

	PGPError
RDGetGroupValidityAndADKs(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPKeySetRef	keySet,
	PUSERKEYINFO	HeadPUI,
	PGPUInt32 *		numKeysFound,
	PGPUInt32 *		numKeysTotal,
	PUSERKEYINFO	pui)
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroupItemIterRef	iter;
	PUSERKEYINFO OwnerPUI;

	pui->Validity	= kPGPValidity_Invalid;
	*numKeysFound=*numKeysTotal=0;

	err	= PGPNewGroupItemIter( set, id,
			kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Keys,
			&iter );
	if ( IsntPGPError( err )  )
	{
		PGPGroupItem	item;
		PGPContextRef	context;
		PGPBoolean		examinedAKey	= FALSE;
		PGPValidity		lowestValidity	= kPGPValidity_Complete;

		context	= PGPGetKeySetContext( keySet );
		
		while( IsntPGPError( err = PGPGroupItemIterNext( iter, &item ) ) )
		{
			PGPKeyRef	key;
			
			*numKeysTotal=*numKeysTotal+1;

			err	= RDGroupItemToKey( &item, keySet, &key );

			if ( IsntPGPError( err) )
			{
				if(OwnerPUI=KeyIsEncryptable(HeadPUI,key))
				{
					PGPValidity		validity;
					PGPUInt32		uAlg;

					examinedAKey	= TRUE;

					PGPGetPrimaryUserIDValidity( key, &validity );
					lowestValidity=RDMinValidity(validity,
						lowestValidity );

					pui->ADK=GetADKList(OwnerPUI,key,keySet,pui->ADK);

					PGPGetKeyNumber (key, kPGPKeyPropAlgID, &uAlg);

					if(uAlg==kPGPPublicKeyAlgorithm_RSA)
					{
						pui->Flags|=PUIF_HASRSA;
					}
					else
					{
						pui->Flags|=PUIF_HASDSA;
					}

					*numKeysFound=*numKeysFound+1;
					key	= NULL;
				}
			}
			else
			{
				if ( err == kPGPError_ItemNotFound )
				{
					/* just can't find the key; that's OK */
					err	= kPGPError_NoErr;
				}
			}
			
			if ( IsPGPError( err ) )
				break;
		}
		if ( err == kPGPError_EndOfIteration )
			err	= kPGPError_NoErr;
		
		/* if we examined keys, use the validity we found, otherwise
			consider it invalid */
		if ( examinedAKey )
			pui->Validity	= lowestValidity;
		else
			pui->Validity	= kPGPValidity_Invalid;

		if(*numKeysFound!=*numKeysTotal)
			pui->Validity	= kPGPValidity_Invalid;

		PGPFreeGroupItemIter( iter );
	}
	
	return( err );
}




UINT LoadGroups(PUSERKEYINFO *HeadUserLL,
			PRECIPIENTDIALOGSTRUCT prds,
			PPREFS Prefs)
{
	PGPUInt32			numGroups,
						groupIndex;
	PGPGroupID			groupID;
	PGPError			err;
	PGPGroupInfo		groupInfo;
	PGPUInt32			numKeysFound,numKeysTotal;
	PUSERKEYINFO pui;

	if(Prefs->Group==0)
		return FALSE;

	err = PGPCountGroupsInSet(Prefs->Group->groupset, &numGroups);
	for(groupIndex = 0; groupIndex < numGroups ; groupIndex++)
	{
		err = PGPGetIndGroupID(Prefs->Group->groupset, groupIndex, &groupID);
		err = PGPGetGroupInfo(Prefs->Group->groupset, groupID, &groupInfo);

	    pui = (PUSERKEYINFO) malloc( sizeof(USERKEYINFO) );
		memset (pui,0x00,sizeof(USERKEYINFO));

		pui->GroupSet=Prefs->Group->groupset;
		pui->GroupID=groupID;

		strcpy(pui->UserId,groupInfo.name);
		pui->KeyRef=0;

// Get Group's ADK keys
		RDGetGroupValidityAndADKs(Prefs->Group->groupset,
			groupID,
			prds->OriginalKeySetRef,
			*HeadUserLL,
			&numKeysFound,
			&numKeysTotal,
			pui);

		if(numKeysFound==numKeysTotal)
			sprintf(pui->szSize,"%d keys",numKeysTotal);
		else
			sprintf(pui->szSize,"%d of %d keys",numKeysFound,
				numKeysTotal);

		pui->NumGroupMembers=numKeysTotal;

		pui->Validity=KMConvertFromPGPValidity(pui->Validity);

		pui->Algorithm=kPGPPublicKeyAlgorithm_Invalid;
		pui->Flags|=PUIF_GROUP;

		*HeadUserLL=AddLink(*HeadUserLL,pui);
	}

	return TRUE;
}

typedef struct _MATCHLIST
{
	PUSERKEYINFO pui;
	struct _MATCHLIST *next;
} MATCHLIST;

MATCHLIST *AddMatch(MATCHLIST *head,PUSERKEYINFO pui)
{
	MATCHLIST *match;

    match = (MATCHLIST *) malloc( sizeof(MATCHLIST) );
    memset (match, 0x00,sizeof(MATCHLIST));

	match->pui=pui;
    match->next=head;

    return match;
}

int NumMatches(MATCHLIST *match)
{
	int count;

	count=0;

	while(match!=0)
	{
		count++;
		match=match->next;
	}

	return count;
}

void FreeMatch(MATCHLIST *match)
{
	MATCHLIST *FreeAtLast;

	while(match!=0)
	{
		FreeAtLast=match;
		match=match->next;
		free(FreeAtLast);
	}
}

int GetUsersEmail(char *username,char *emailaddress)
{
    BOOL endflag;
    char *lastspace;

    lastspace=username;
    endflag=FALSE;

    do
    {
        if(*username=='@')
        {
            endflag=TRUE;
        }
        else if((*username==' ')||(*username=='<')||(*username=='>'))
        {
            if(endflag==TRUE)
			{
			//	username=username-1;
                break;
			}
            lastspace=username+1;
        }
        username=username+1;
    }
    while(*username!=0);

    if(endflag==TRUE)
    {
        memcpy(emailaddress,lastspace,(int)(username-lastspace));
        emailaddress[username-lastspace]=0;
        return TRUE;
    }

    return FALSE;
}

int BuildMatchList(PUSERKEYINFO pui,
				   PRECIPIENTDIALOGSTRUCT prds,
				   MATCHLIST **MatchList)
{
    char email1[256],email2[256];
    BOOL WeGotMatch;
    int RecipIndex;
    BOOL FoundAtLeastOne;

    if((prds->dwNumRecipients==0)||(prds->szRecipientArray==0))
        return FALSE;

    FoundAtLeastOne=FALSE;
 
    for(RecipIndex=0;RecipIndex<(int)prds->dwNumRecipients;RecipIndex++)
    {
        WeGotMatch=FALSE;

		// For a complete lowercase match
		if(!stricmp(prds->szRecipientArray[RecipIndex],pui->UserId))
		{
			WeGotMatch=TRUE;
        }
        else
        {
			// For an email address match
            if(GetUsersEmail(pui->UserId,email1))
			{
				if(GetUsersEmail(prds->szRecipientArray[RecipIndex],email2))
				{
					if(!stricmp(email1,email2))
                        WeGotMatch=TRUE;
				}
			}
        }

        if(WeGotMatch)
        {
     
			if(MatchList[RecipIndex]==0)
			{
				MatchList[RecipIndex]=
					AddMatch(MatchList[RecipIndex],pui);
			}
			else
			{   // keyid don't match
				if(MatchList[RecipIndex]->pui->KeyRef!=pui->KeyRef) 
				{
					MatchList[RecipIndex]=
						AddMatch(MatchList[RecipIndex],pui);
 				}
				else if(pui->Flags&PUIF_GROUP)
				{
					// Since Groups have 0's for KeyRefs
					MatchList[RecipIndex]=
						AddMatch(MatchList[RecipIndex],pui);
				}
			}
        }
	}

    return FoundAtLeastOne;
}

UINT MatchKeys(PUSERKEYINFO pui,
			PRECIPIENTDIALOGSTRUCT prds,
			MATCHLIST **MatchList)
{
	while(pui!=0)
	{
		BuildMatchList(pui,prds,MatchList);
		pui=pui->next;
 	}

	return TRUE;
}

UINT MarkKeys(PUSERKEYINFO *HeadUserLL,
			PRECIPIENTDIALOGSTRUCT prds,
			MATCHLIST **MatchList,
			UINT *ReturnValue)
{
	unsigned int index,multiplecount;
	MATCHLIST *match,*FreeAtLast;

	for(index=0;index<prds->dwNumRecipients;index++)
	{
		match=MatchList[index];

		if(match==0)
		{
			*ReturnValue|=ADDUSER_KEYSNOTFOUND;
			*HeadUserLL=AddOneNotFound(prds->szRecipientArray[index],HeadUserLL,
				PUIF_NOTFOUND|PUIF_SELECTED);
		}

		multiplecount=0;

		// If first match is RSA and second is DSA, free first/use second
		if((NumMatches(match)==2)&&
			((match->pui->Algorithm==kPGPPublicKeyAlgorithm_RSA)&&
			(match->next->pui->Algorithm==kPGPPublicKeyAlgorithm_DSA)))
		{
			FreeAtLast=match;
			match=match->next;
			MatchList[index]=match;
			free(FreeAtLast);
		}

		// If first match is DSA and second is RSA, free second/use first
		if((NumMatches(match)==2)&&
			((match->pui->Algorithm==kPGPPublicKeyAlgorithm_DSA)&&
			(match->next->pui->Algorithm==kPGPPublicKeyAlgorithm_RSA)))
		{
			FreeAtLast=match->next;
			match->next=0;
			free(FreeAtLast);
		}

		while(match!=0)
		{
			match->pui->Flags|=PUIF_SELECTED;

			match=match->next;
			multiplecount++;
		}

		if(multiplecount>1)
		{
			*ReturnValue|=ADDUSER_MULTIPLEMATCH;
		}
	}

	return TRUE;
}


int MoveSelectedKeysAndADKs(HWND hdlg,
							PUSERKEYINFO headpui,
							BOOL RecToUser)
{
	PUSERKEYINFO pui;

	pui=headpui;

    while(pui!=0)
    {
        if(pui->Flags&PUIF_SELECTED)
        {
            DragUserX(hdlg,headpui,pui,RecToUser);
        }
        pui=pui->next;
    }

    return TRUE;
}

UINT VerfifyKeys(PUSERKEYINFO pui,
			PPREFS Prefs,
			UINT *ReturnValue)
{
	while(pui!=0)
	{
		if((pui->Flags&PUIF_MOVEME)&&
			((pui->Flags&PUIF_NOTFOUND)==0))
		{
			// Flag if we've included ADKs
			if((Prefs->bWarnOnADK)
				&&((pui->Flags&PUIF_MANUALLYDRAGGED)==0))
				*ReturnValue|=ADDUSER_ADKSINCLUDED;
			// Flag if somebody not signed by corp key
			if((Prefs->bWarnNotCorpSigned)
				&&((pui->Flags&PUIF_SIGNEDBYCORP)==0))
				*ReturnValue|=ADDUSER_KEYSNOTCORPSIGNED;
			// Flag if a key isn't valid
			if(pui->Validity==0)
				*ReturnValue|=ADDUSER_KEYSNOTVALID;
			// Flag if marginals are considered invalid
			if((Prefs->bMarginalInvalid)&&(pui->Validity==1))
				*ReturnValue|=ADDUSER_KEYSNOTVALID; 
		}

		pui=pui->next;
 	}

	return TRUE;
}


UINT AddUsersToLists(HWND hwndLoad,PUSERKEYINFO *HeadUserLL,
					 PRECIPIENTDIALOGSTRUCT prds,
					 PPREFS Prefs,
					 UINT *ReturnValue)
{
	MATCHLIST **MatchList;
	unsigned int index;
	int memamt;
	BOOL Abort;

	if(!LoadKeys(hwndLoad,HeadUserLL,prds,Prefs))
		return FALSE;

	LoadGroups(HeadUserLL,prds,Prefs);

	memamt=sizeof(MATCHLIST *)*prds->dwNumRecipients;

    MatchList=(MATCHLIST **)malloc(memamt);
    memset(MatchList,0x00,memamt);

	MatchKeys(*HeadUserLL,prds,MatchList);

	*ReturnValue=0;
	MarkKeys(HeadUserLL,prds,MatchList,ReturnValue);

	for(index=0;index<prds->dwNumRecipients;index++)
	{
		FreeMatch(MatchList[index]);
	}

	free(MatchList);

	Abort=VetoSelectedKeys(NULL,Prefs,*HeadUserLL,FALSE);

	if(Abort)
		*ReturnValue|=ADDUSER_ADKMISSING;

	MoveSelectedKeysAndADKs(NULL,*HeadUserLL,FALSE);

	VerfifyKeys(*HeadUserLL,Prefs,ReturnValue);

	if((prds->dwNumRecipients==0)||(prds->szRecipientArray==0))
		*ReturnValue|=ADDUSER_NORECIPIENTSTOSEARCH;

	return TRUE;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
