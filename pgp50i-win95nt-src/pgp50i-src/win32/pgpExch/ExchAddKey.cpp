/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>

#include "stdinc.h"
#include "pgpexch.h"
#include "ui.h"
#include "assert.h"
#include "resource.h"

#include "config.h"
#include "pgpkeydb.h"
#include "pgpcomdlg.h"

#ifndef STATUS_SUCCESS
    #define STATUS_SUCCESS       0x0000000
#endif

#ifndef STATUS_FAILURE
    #define STATUS_FAILURE       0x0000001
#endif

static void BuildNewKeyList(PGPKeySet *NewKeySet,
                            char *szUserList, size_t ListSize);

UINT AddKeyBuffer(HWND hwnd, void* pInput, DWORD dwInSize)
{
    UINT ReturnValue = STATUS_FAILURE;
    PGPKeySet *RingSet;
    PGPKeySet *NewKeySet;
    PGPError error;
    char szMessage[1024];
    char szTitle[255];

    assert(pInput);

    memset(szMessage, 0, 1024);
    pgpLibInit();

    RingSet = pgpOpenDefaultKeyRings(TRUE, NULL);

    if(RingSet)
    {
        NewKeySet = pgpImportKeyBuffer((BYTE*)pInput, dwInSize);

        if(NewKeySet)
        {
            error = pgpAddKeys(RingSet, NewKeySet);

            if(error == PGPERR_OK)
            {
                error = pgpCommitKeyRingChanges(RingSet);

                if(error == PGPERR_OK)
                {
                    ReturnValue = STATUS_SUCCESS;
                    PGPcomdlgNotifyKeyringChanges(GetCurrentProcessId());
                    GetString(szTitle, sizeof(szTitle), IDS_TITLE);
                    GetString(szMessage, sizeof(szMessage), IDS_ADDKEY_NEWKEYS);
                    BuildNewKeyList(NewKeySet, szMessage, sizeof(szMessage));
                    MessageBox(hwnd, szMessage, szTitle, MB_OK);
                }
                else
                    PGPcomdlgErrorMessage(error);
            }
            else
                PGPcomdlgErrorMessage(error);

            pgpFreeKeySet(NewKeySet);
        }

      pgpFreeKeySet(RingSet);
    }

    pgpLibCleanup();

    return ReturnValue;
}


UINT AddKeyFile(HWND hwnd, char *szInFile)
{
    UINT ReturnValue = STATUS_FAILURE;
    PGPKeySet *RingSet;
    PGPKeySet *NewKeySet;
    PGPFileRef* fileRef;
    PGPError error;
    char szMessage[1024];
    char szTitle[255];

    assert(szInFile);

    pgpLibInit();

    RingSet = pgpOpenDefaultKeyRings(TRUE, NULL);

    if(RingSet)
    {
        fileRef = pgpNewFileRefFromFullPath(szInFile);

        if(fileRef)
        {
            NewKeySet = pgpImportKeyFile(fileRef);

            if(NewKeySet)
            {
                error = pgpAddKeys(RingSet, NewKeySet);

                if(error == PGPERR_OK)
                {
                    error = pgpCommitKeyRingChanges(RingSet);

                    if(error == PGPERR_OK)
                    {
                        ReturnValue = STATUS_SUCCESS;
                        PGPcomdlgNotifyKeyringChanges(GetCurrentProcessId());
                        GetString(szTitle, sizeof(szTitle), IDS_TITLE);
                        GetString(szMessage, sizeof(szMessage),
                                    IDS_ADDKEY_NEWKEYS);
                        BuildNewKeyList(NewKeySet, szMessage,
                                    sizeof(szMessage));
                        MessageBox(hwnd, szMessage, szTitle, MB_OK);
                    }
                    else
                        PGPcomdlgErrorMessage(error);
                }
                else
                    PGPcomdlgErrorMessage(error);

                pgpFreeKeySet(NewKeySet);
            }

            pgpFreeFileRef(fileRef);
        }

        pgpFreeKeySet(RingSet);
    }

    pgpLibCleanup();

    return ReturnValue;
}


static void BuildNewKeyList(PGPKeySet *NewKeySet,
                            char *szUserList, size_t ListSize)
{
    PGPKeyList *NewKeyList;
    PGPKeyIter *NewKeyIter;
    PGPKey *NewKey;
    char szUserID[255];
    size_t UserIDLen;

    NewKeyList = pgpOrderKeySet(NewKeySet, kPGPUserIDOrdering);
    NewKeyIter = pgpNewKeyIter(NewKeyList);

    NewKey = pgpKeyIterNext(NewKeyIter);
    while (NewKey)
    {
        UserIDLen = sizeof(szUserID)-2;
        pgpGetPrimaryUserIDName(NewKey, szUserID, &UserIDLen);
        szUserID[UserIDLen] = '\n';
        szUserID[UserIDLen+1] = 0;
        if ((strlen(szUserList)+strlen(szUserID)) > ListSize)
            break;
        strcat(szUserList, szUserID);
        NewKey = pgpKeyIterNext(NewKeyIter);
    }

    pgpFreeKeyIter(NewKeyIter);
    pgpFreeKeyList(NewKeyList);

    return;
}
