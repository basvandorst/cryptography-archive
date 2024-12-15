/*
 * RecKeyDB.c  Routines needed to fetch and match keys
 *
 * Most of the calls to PGPKeyDB lib are isolated here. The keys are
 * read in from disk, and matches are attempted.
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

#include "precomp.h"

BOOL AddLinkedListtoListView(HWND hwndList,PUSERKEYINFO Head)
{
    PUSERKEYINFO pui;
    int image,result;

    pui=Head;

    while(pui!=0)
    {

        if(pui->Algorithm==PGP_PKALG_RSA)
        {
            image=(RecGbl(GetParent(hwndList)))->g_ValRSA;
            if(pui->Flags&PUIF_MULTIPLEFOUND)
                image=(RecGbl(GetParent(hwndList)))->g_MultRSA;
        }
        else
        {
            image=(RecGbl(GetParent(hwndList)))->g_ValDSA;
            if(pui->Flags&PUIF_MULTIPLEFOUND)
                 image=(RecGbl(GetParent(hwndList)))->g_MultDSA;
        }

        if((pui->Flags&PUIF_NOTFOUND)&&((pui->Flags&PUIF_MULTIPLEFOUND)==0))
        {
            image=(RecGbl(GetParent(hwndList)))->g_Unknown;
        }

        result=AddListViewItem(hwndList, image, pui->UserId, (LPARAM)pui);

        pui=pui->next;
    }

//    vErrorOut(fg_yellow,"Added all\n");
    return TRUE;
}

UINT KMConvertFromPGPValidity (UINT uPGPValidity) {
    switch (uPGPValidity) {
    case PGP_VALIDITY_UNKNOWN :
    case PGP_VALIDITY_INVALID :
//    case PGP_VALIDITY_UNTRUSTED :
         return 0;
    case PGP_VALIDITY_MARGINAL :
         return 1;
    case PGP_VALIDITY_COMPLETE :
        return 2;
    }
}

UINT KMConvertFromPGPTrust (UINT uPGPTrust) {
    switch (uPGPTrust & PGP_KEYTRUST_MASK) {
    case PGP_KEYTRUST_UNDEFINED    :
    case PGP_KEYTRUST_UNKNOWN :
    case PGP_KEYTRUST_NEVER :
        return 0;
    case PGP_KEYTRUST_MARGINAL :
        return 1;
    case PGP_KEYTRUST_COMPLETE :
        return 2;
    case PGP_KEYTRUST_ULTIMATE :
        return 3;
    }
}

int FillInPUI(PGPKey *key,PGPUserID *userID,PUSERKEYINFO pui)
{
    size_t len;
    char szKeyId[KEYIDLENGTH];
    unsigned char bAxiomatic;

    pui->Flags=0;
    pui->refCount=0;

    len = USERIDLENGTH;
    pgpGetUserIDString(userID, kPGPUserIDPropName, pui->UserId, &len);
    pui->UserId[len] = 0x00;

    len = KEYIDLENGTH;
    pgpGetKeyString(key, kPGPKeyPropKeyId, szKeyId, &len);
    KMConvertStringKeyID (szKeyId);
    wsprintf(pui->KeyId, "0x%s",szKeyId);

    pgpGetUserIDNumber(userID, kPGPUserIDPropValidity, &(pui->Validity));
    pui->Validity= KMConvertFromPGPValidity(pui->Validity);

    pgpGetKeyNumber(key,kPGPKeyPropTrustVal , &(pui->Trust));
    pui->Trust= KMConvertFromPGPTrust (pui->Trust);

    pgpGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &bAxiomatic);

    if (bAxiomatic)
    {
        pui->Trust =
            KMConvertFromPGPTrust (PGP_KEYTRUST_ULTIMATE) + 1;
        pui->Validity =
            KMConvertFromPGPValidity (PGP_VALIDITY_COMPLETE) + 1;
    }

    pgpGetKeyNumber (key, kPGPKeyPropAlgId, &(pui->Algorithm));

//  (fg_white, "Reading in: %s %s\n", pui->KeyId,pui->UserId);

    return TRUE;
}

PUSERKEYINFO AddLink(PUSERKEYINFO head,PUSERKEYINFO pui)
{
     pui->next=head;

     return pui;
}

void GetUser(char *RecipientList,char *User,int UserNumber)
{
    int index;
    int start;
    int end;
    int ListIndex;

    index=0;
    ListIndex=0;

    while(RecipientList[index]!=0)
    {
        if(ListIndex==UserNumber)
            break;

        if(RecipientList[index]=='\n')
            ListIndex++;

        index++;
    }

    start=index;

    while((RecipientList[index]!='\n')&&(RecipientList[index]!=0))
    {
         index++;
    }

    end=index;

    memcpy(User,&RecipientList[start],end-start);
    User[end-start]=0;
}

PUSERKEYINFO AddOneNotFound(char *RecipientList,PUSERKEYINFO *HeadUserLL,
							int index,int flags)
{
    char User[256];
	PUSERKEYINFO pui;

    GetUser(RecipientList,User,index);

    pui = (PUSERKEYINFO) malloc( sizeof(USERKEYINFO) );
    memset (pui,0x00,sizeof(USERKEYINFO));

    strcpy(pui->UserId,User);
    strcpy(pui->KeyId,"");
    strcpy(pui->szSize,"");
    pui->Validity=0;
   pui->Algorithm=PGP_PKALG_RSA;
   pui->Flags=flags;

   return AddLink(*HeadUserLL,pui);
}

int StickInUnfoundUserNames(char *RecipientList,int NumRecipients,
                            PUSERKEYINFO *RecipientCheck,
                            PUSERKEYINFO *HeadUserLL)
{
    int index,ReturnValue;

    if((RecipientList==0)||(NumRecipients==0))
        return FALSE;

    ReturnValue=TRUE;

    for(index=0;index<NumRecipients;index++)
    {
        if(RecipientCheck[index]==0)
        {
            *HeadUserLL=AddOneNotFound(RecipientList,HeadUserLL,index,
							     PUIF_NOTFOUND|PUIF_MATCHEDWITHRECIPIENT);

            ReturnValue=FALSE;  // Not all matched!
        }
    }

    return ReturnValue;
}

BOOL AddUsersToLists(PUSERKEYINFO *HeadUserLL,
                     DWORD NumRecipients, char* RecipientList,
                     BOOL EncSelfOn)
{
    BOOL ReturnValue;
    PGPKeySet *set;
    PGPKeyList *list;
    PGPKeyIter *iter;
    PGPKey *key,*defaultkey;
    PGPUserID *userID;
    PUSERKEYINFO pui;
    int RecipientsMatched;
    PUSERKEYINFO *RecipientCheck;
    PGPError OpenKeyRingError;
    unsigned char EncryptToSelf;
    unsigned char KeyRevoked;
    unsigned char KeyDisabled;
	unsigned char KeyExpired;
    unsigned char KeyCanEncrypt;
    unsigned char MarginallyValidWarning;
    int MultipleFound;
    BOOL EncryptToSelfSucceeded;
    UINT uAlg;
    UINT uKeyBits, uSubKeyBits;
    PGPSubKey* pSubKey;

    ReturnValue=TRUE;
    EncryptToSelfSucceeded=FALSE;

    *HeadUserLL=0;
    RecipientsMatched=0;
    MultipleFound=0;

    pgpLibInit();
    set = pgpOpenDefaultKeyRings(FALSE, &OpenKeyRingError);

    if(set==0)
    {
        MessageBox(NULL,"Could not open key rings. Please set your "
                         "public and and private\n"
                         "key ring files in the key files preferences.",
                         "PGP 5.0 Error",
                         MB_OK|MB_ICONEXCLAMATION);
        pgpLibCleanup();  // Really bad! Lets get out of here now!

        return FALSE;  // Could not open key rings!
    }


    pgpGetPrefBoolean (kPGPPrefMarginallyValidWarning,
        &MarginallyValidWarning);

    pgpGetPrefBoolean (kPGPPrefEncryptToSelf,&EncryptToSelf);
    defaultkey=pgpGetDefaultPrivateKey (set);

    RecipientCheck=
        (PUSERKEYINFO *)malloc(sizeof(PUSERKEYINFO)*NumRecipients);
    memset(RecipientCheck,0x00,sizeof(PUSERKEYINFO)*NumRecipients);

    list = pgpOrderKeySet(set,kPGPReverseUserIDOrdering);
    iter = pgpNewKeyIter(list);

    if((EncSelfOn==FALSE)||(defaultkey==0))
        EncryptToSelf=FALSE; // No default key? Accept for now ;-)

    while (key = pgpKeyIterNext(iter))
    {
         pgpGetKeyBoolean (key, kPGPKeyPropIsRevoked, &KeyRevoked);

         if(KeyRevoked)
             continue;

         pgpGetKeyBoolean (key, kPGPKeyPropIsDisabled, &KeyDisabled);

         if(KeyDisabled)
            continue;

 	 	 pgpGetKeyBoolean (key, kPGPKeyPropIsExpired, &KeyExpired);

         if(KeyExpired)
            continue;

 	 	 pgpGetKeyBoolean (key, kPGPKeyPropCanEncrypt, &KeyCanEncrypt);

         if(!KeyCanEncrypt)
            continue;

        uAlg=uKeyBits=uSubKeyBits=0;

        pgpGetKeyNumber (key, kPGPKeyPropAlgId, &uAlg);

        if((uAlg!=PGP_PKALG_RSA)&&(uAlg!=PGP_PKALG_DSA))
            continue;

        pgpGetKeyNumber (key, kPGPKeyPropBits, &uKeyBits);

        if(uAlg==PGP_PKALG_DSA)
        {
            pSubKey = pgpKeyIterNextSubKey (iter);
            if(pSubKey==0)
                continue;
            pgpGetSubKeyNumber (pSubKey, kPGPKeyPropBits, &uSubKeyBits);
        }

        while (userID = pgpKeyIterNextUserID(iter))
        {
            pui = (PUSERKEYINFO) malloc( sizeof(USERKEYINFO) );
            memset (pui,0x00,sizeof(USERKEYINFO));

            FillInPUI(key,userID,pui);

            if(pui->Algorithm==PGP_PKALG_RSA)
                sprintf(pui->szSize,"%i", uKeyBits);
            else
                sprintf(pui->szSize,"%i/%i", uKeyBits, uSubKeyBits);

            pui->mrk=GetMRKList(key,set);   // MRK ADDITION!!!!!

            if(key==defaultkey)
            {
                if((EncryptToSelf)&(!EncryptToSelfSucceeded))
                {
                    pui->Flags|=PUIF_MATCHEDWITHRECIPIENT;
					pui->Flags|=PUIF_DEFAULTKEY;
                    RecipientsMatched++;
                    EncryptToSelfSucceeded=TRUE;
                }
            }

            if(UserIsARecipient(pui,&MultipleFound,
                RecipientList, NumRecipients, RecipientCheck,
				HeadUserLL))
            {
                pui->Flags|=PUIF_MATCHEDWITHRECIPIENT;

                RecipientsMatched++;

                if(pui->Validity==0)
                    ReturnValue=FALSE;      // Pop dialog if invalid
                if((MarginallyValidWarning)&&(pui->Validity==1))
                    ReturnValue=FALSE;      // Pop dialog if marginal
            }
            *HeadUserLL=AddLink(*HeadUserLL,pui);
        }
    }

    if(RecipientsMatched==0)   // NO MATCHES!
        ReturnValue=FALSE;

    if(MultipleFound!=0)
        ReturnValue=FALSE;    // Some confusion here. Pop the dialog
    pgpFreeKeyIter(iter);
    pgpFreeKeyList(list);
    pgpFreeKeySet(set);

    pgpLibCleanup();

    if(!StickInUnfoundUserNames(RecipientList,NumRecipients,
                                RecipientCheck,HeadUserLL))
        ReturnValue=FALSE; // A USERNAME WASN'T FOUND!

    free(RecipientCheck);

    if(NumRecipients==0)
        ReturnValue=FALSE;  // NO RECIPIENTS!

    return ReturnValue;
}

//
// big endian to little endian , swap 4 bytes per longword,
// n is the size of the block to swap in bytes, has to be multiple of 4*/
//
void SwapEndian(char *lw, int n)
{
    int i;
    char keep[4];

    for(i=0;i<n;i+=4,lw+=4)
    {
        keep[0]=*lw++;keep[1]=*lw++;keep[2]=*lw++;keep[3]=*lw;
        *lw--=keep[0];*lw--=keep[1];*lw--=keep[2];*lw=keep[3];
    }
}

//----------------------------------------------------|
// Convert KeyID from "string" format to presentation format
// NB: sz must be at least 9 bytes long

void KMConvertStringKeyID (LPSTR sz)
{
    long first,second;
    long *firstptr,*secondptr;

    SwapEndian(sz,8); // 68000/Sparc big.. x86 little.. this is in big
                      // I thought we were in a Wintel world?

    firstptr=(long *)&sz[0];
    secondptr=(long *)&sz[4];

    first=*firstptr;
    second=*secondptr;

    wsprintf (sz, "%08lX%08lX",first,second);
}

int AbuttingChrsOK(char *cp,char *RecipientList,char *user)
{
    int userlength;
    char endbutt,startbutt;

    userlength=strlen(user);

    if(cp==RecipientList)  // Beginning of recipient list
    {
        endbutt=cp[userlength]; // character after username
        startbutt='\n';
    }
    else
    {
        startbutt=cp[-1];   // character before username
        endbutt=cp[userlength];  // character after username

    }

    if(((startbutt=='<')||(startbutt=='\n')||(startbutt==' '))&&
       ((endbutt=='>')||(endbutt=='\n')||(endbutt==' ')||(endbutt=='\0')))
        return TRUE;

    return FALSE;
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
                break;
            lastspace=username+1;
        }
        username=username+1;
    }
    while(*username!=0);

    if(endflag==TRUE)
    {
        memcpy(emailaddress,lastspace,(int)(username-lastspace));
        emailaddress[username-lastspace]=0;
//      vErrorOut(fg_yellow,"Email Address *%s*\n",emailaddress);
        return TRUE;
    }

    return FALSE;
}

int UserIsARecipient(PUSERKEYINFO pui,int *MultipleFound,
                     char* RecipientList, DWORD NumRecipients,
                     PUSERKEYINFO *RecipientCheck,
					 PUSERKEYINFO *HeadUserLL)
{
    int index;
    char* cp = NULL;
    char *cpindex;
    char emailaddress[256];
    BOOL WeGotMatch;
    char *RecipIndex;
    BOOL FoundAtLeastOne;

    if((RecipientList==0)||(NumRecipients==0))
        return FALSE;

    FoundAtLeastOne=FALSE;
    RecipIndex=RecipientList;

    do
    {
        WeGotMatch=FALSE;

        cp = strstri(RecipIndex, pui->UserId);


        if(cp!=0)
        {
            if(AbuttingChrsOK(cp,RecipientList,pui->UserId))
            {
                WeGotMatch=TRUE;
//                vErrorOut(fg_yellow,"Matched on whole\n");
            }
        }
        else
        {
            if(GetUsersEmail(pui->UserId,emailaddress))
            {
                cp=strstri(RecipIndex,emailaddress);
                if(cp!=0)
                {
                    if(AbuttingChrsOK(cp,RecipientList,emailaddress))
                    {
                        WeGotMatch=TRUE;
//                        vErrorOut(fg_yellow,"Matched on email\n");
                    }
                }
            }
        }

        if(WeGotMatch)
        {
            index=0;

            for(cpindex=RecipientList;cpindex<cp;cpindex++)
            {
                if(*cpindex=='\n')
                    index++;
            }

			if(RecipientCheck[index]==0)
			{
				FoundAtLeastOne=TRUE;
				RecipientCheck[index]=pui;
			}
			else
			{   // keyid don't match
				if(strcmp(RecipientCheck[index]->KeyId,pui->KeyId))
				{
					if((RecipientCheck[index]->Flags&PUIF_ALREADYINSERTEDMULTI)
						==0)
					{
					RecipientCheck[index]->Flags&=(~PUIF_MATCHEDWITHRECIPIENT);
					RecipientCheck[index]->Flags|=PUIF_ALREADYINSERTEDMULTI;
                    *MultipleFound=*MultipleFound+1;
		            *HeadUserLL=AddOneNotFound(RecipientList,HeadUserLL,index,
					    PUIF_MATCHEDWITHRECIPIENT|PUIF_NOTFOUND|
						PUIF_MULTIPLEFOUND);
					}

        //            RecipientCheck[index]->Flags|=PUIF_MULTIPLEFOUND;
 //                   pui->Flags|=PUIF_MULTIPLEFOUND;
 //                   FoundAtLeastOne=TRUE;  // Different ID but a match!
//                    RecipientCheck[index]=pui;
// Commented out here, since we only want to display one multiple
// user ID, not all of them.
				}
			}
         }
         RecipIndex=cp+1;

    } while(cp!=0);

    return FoundAtLeastOne;
}
