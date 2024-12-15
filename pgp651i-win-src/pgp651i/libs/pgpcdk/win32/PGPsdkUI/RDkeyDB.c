/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: RDkeyDB.c,v 1.17 1999/03/31 23:22:46 wjb Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"

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

	return 0;
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

	return 0;
}

PUSERKEYINFO AddLink(PUSERKEYINFO head,PUSERKEYINFO pui)
{
    pui->next=head;

    return pui;
}

UINT AddToTable(HWND hwndTable,
				PGPGroupSetRef mGroupSet,
				PGPRecipientUser *pru,
				PUSERKEYINFO *UserLinkedList)
{
	PUSERKEYINFO pui;
    PGPKeyRef key;
	DWORD image;

	pui=(PUSERKEYINFO)malloc(sizeof(USERKEYINFO));
	memset(pui,0x00,sizeof(USERKEYINFO));

	switch(pru->kind)
	{
		case kPGPRecipientUserKind_Key:
		{
			PGPBoolean bAxiomatic;
			PGPRecipientKey *rKey;

			rKey=pru->userInfo.key;
			key=rKey->keyRef;

			PGPGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &bAxiomatic);
			PGPGetKeyNumber(key, kPGPKeyPropTrust, &(pui->Trust));

			PGPGetRecipientUserName(pru,pui->UserId);

			pui->Validity=pru->validity;

			pui->Validity= KMConvertFromPGPValidity(pui->Validity);
			pui->Trust= KMConvertFromPGPTrust (pui->Trust);

			if (bAxiomatic) 
			{
				pui->Trust = 
					KMConvertFromPGPTrust (kPGPKeyTrust_Ultimate) + 1;        
				pui->Validity = 
					KMConvertFromPGPValidity (kPGPValidity_Complete) + 1;
			}

			pui->Algorithm=rKey->algorithm;

            if(rKey->algorithm==kPGPPublicKeyAlgorithm_RSA)
			{
                sprintf(pui->szSize,"%i", rKey->keyBits);
			}
            else
			{
                sprintf(pui->szSize,"%i/%i", rKey->subKeyBits, rKey->keyBits);
			}

			break;
		}

		case kPGPRecipientUserKind_Group:
		{
			PGPGroupInfo groupInfo;
			char StrRes[500];

			PGPGetGroupInfo(mGroupSet,
				pru->groupInfo.groupID, &groupInfo);

			strcpy(pui->UserId,groupInfo.name);

			if(pru->groupInfo.numMissingKeys==0)
			{
				LoadString(gPGPsdkUILibInst, IDS_NUMKEYS, StrRes, sizeof(StrRes));

				sprintf(pui->szSize,"%d keys",pru->groupInfo.numKeys);
			}
			else
			{
				LoadString(gPGPsdkUILibInst, IDS_NUMOFNUMKEYS, StrRes, sizeof(StrRes));

				sprintf(pui->szSize,"%d of %d keys",
					pru->groupInfo.numKeys-pru->groupInfo.numMissingKeys,
					pru->groupInfo.numKeys);
			}

			pui->Validity=pru->validity;
			pui->Validity=KMConvertFromPGPValidity(pui->Validity);
			pui->Trust=0;
			pui->Algorithm=kPGPPublicKeyAlgorithm_Invalid;
			break;
		}

		case kPGPRecipientUserKind_MissingRecipient:
		{
			PGPGetRecipientUserName(pru,pui->UserId);
			strcpy(pui->szSize,"No Match");
			pui->Validity=0;
			pui->Trust=0;
			pui->Algorithm=kPGPPublicKeyAlgorithm_Invalid;
			break;
		}

		default:
		{
			free(pui);
			return FALSE;
		}
	}

	pui->pru=pru;

	if(pui->Algorithm==kPGPPublicKeyAlgorithm_RSA)
		image=IDX_RSAUSERID;
        
	if(pui->Algorithm==kPGPPublicKeyAlgorithm_DSA)
		image=IDX_DSAUSERID;

	if(pru->kind==kPGPRecipientUserKind_MissingRecipient)
		image=IDX_INVALIDUSERID;

	if(pru->kind==kPGPRecipientUserKind_Group)
		image=IDX_GROUP;

	pui->icon=image;
	AddAnItem(hwndTable,pui);

	*UserLinkedList=AddLink(*UserLinkedList,pui);

    return TRUE;
}

// BuildTable
//
// This routine takes our PGPRecipientsList and inserts them
// into the correct listview.

	PGPError
BuildTable(
	HWND						hwndTable,
	PGPGroupSetRef				mGroupSet,
	PUSERKEYINFO				*UserLinkedList,
	PGPRecipientsList			*mRecipients,
	PGPRecipientUserLocation 	location)
{
	PGPError			err = kPGPError_NoErr;
	PGPRecipientKey		*curKey;
	PGPRecipientUser	*curGroup;
	PGPRecipientUser	*curUser;
		
	curKey = mRecipients->keys;
	while( IsntNull( curKey ) )
	{
		curUser = curKey->users;
		while( IsntNull( curUser ) )
		{
			if( curUser->location == location )
			{
				AddToTable(hwndTable,mGroupSet,curUser,UserLinkedList);
			}
			
			curUser = curUser->nextUser;
		}
	
		curKey = curKey->nextKey;
	}
	
	curGroup = mRecipients->groups;
	while( IsntNull( curGroup ) )
	{
		pgpAssert( curGroup->kind == kPGPRecipientUserKind_Group );
		
		if( curGroup->location == location )
		{
			AddToTable(hwndTable,mGroupSet,curGroup,UserLinkedList);
		}

		curGroup = curGroup->nextUser;
	}
	
	curUser = mRecipients->missingRecipients;

	while( IsntNull( curUser ) )
	{
		pgpAssert( curUser->kind ==
				kPGPRecipientUserKind_MissingRecipient );

		// Always in recipients list
		if( curUser->location == location )
		{
			AddToTable(hwndTable,mGroupSet,curUser,UserLinkedList);
		}
				
		curUser = curUser->nextUser;
	}
	return( err );
}
	
void BuildTables(PRECGBL prg)
{
	if(prg->gUserLinkedList)
	{
		ListView_DeleteAllItems(prg->hwndRecipients);
		ListView_DeleteAllItems(prg->hwndUserIDs);

		FreeLinkedLists(prg->gUserLinkedList);
		prg->gUserLinkedList=NULL;
	}

	BuildTable(prg->hwndUserIDs,
		prg->mGroupSet,
		&(prg->gUserLinkedList),
		&(prg->mRecipients),
		kPGPRecipientUserLocation_UserList);

	SortEm(prg->hwndUserIDs);

	ListView_SetItemState(prg->hwndUserIDs,0,LVIS_FOCUSED,LVIS_FOCUSED);

	BuildTable(prg->hwndRecipients,
		prg->mGroupSet,
		&(prg->gUserLinkedList),
		&(prg->mRecipients),
		kPGPRecipientUserLocation_RecipientList);

	SortEm(prg->hwndRecipients);

	ListView_SetItemState(prg->hwndRecipients,0,LVIS_FOCUSED,LVIS_FOCUSED);
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
