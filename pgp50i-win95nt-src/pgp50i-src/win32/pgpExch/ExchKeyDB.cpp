/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include "stdinc.h"

#include <config.h>
#include <pgpTypes.h>
#include <pgpKeyDB.h>
#include <pgpMem.h>
#include <assert.h>
#include <stdio.h>
#include "ExchKeyDB.h"

#ifndef STATUS_SUCCESS
    #define STATUS_SUCCESS       0x0000000
#endif

#ifndef STATUS_FAILURE
    #define STATUS_FAILURE       0x0000001
#endif


unsigned long GetDefaultSigningKey(char* szKeyId, size_t length)
{
    unsigned long ReturnValue = STATUS_FAILURE;
    PGPKeySet *set;
    PGPKey *key;
    size_t len = length;
    PGPError error;

    assert(szKeyId);

    error = pgpLibInit();

    if(error == PGPERR_OK)
    {

        set = pgpOpenDefaultKeyRings(FALSE, &error);

        if(set)
        {
            key = pgpGetDefaultPrivateKey(set);

            if(key)
            {
                error = pgpGetKeyString(key, kPGPKeyPropKeyId, szKeyId, &len);

                if(error == PGPERR_OK)
                {
                    KMConvertStringKeyID (szKeyId);
                    ReturnValue = STATUS_SUCCESS;
                }
            }

            pgpFreeKeySet(set);
        }

        pgpLibCleanup();
    }

    return ReturnValue;
    }

void KMConvertStringKeyID (char* sz)
{
    unsigned long i, uL;

    uL = 0;

    for (i=0; i<4; i++)
    {
        uL <<= 8;
        uL |= ((unsigned long)sz[i+4] & 0xFF);
    }
    sprintf (sz, "%08lX", uL);
}

BOOL UsePGPMime()
{
    Boolean UseMime = FALSE;
    PGPError error;

    error = pgpLibInit();

    if(error == PGPERR_OK)
    {
        pgpGetPrefBoolean (kPGPPrefMailEncryptPGPMIME, &UseMime);
        pgpLibCleanup();
    }

    return (BOOL)UseMime;
}


BOOL UsePGPMimeForSigning()
{
    Boolean UseMime = FALSE;
    PGPError error;

    error = pgpLibInit();

    if(error == PGPERR_OK)
    {
        pgpGetPrefBoolean (kPGPPrefMailSignPGPMIME, &UseMime);
        pgpLibCleanup();
    }

    return (BOOL)UseMime;
 }

BOOL ByDefaultEncrypt()
{
    Boolean Encrypt = FALSE;
    PGPError error;

    error = pgpLibInit();

    if(error == PGPERR_OK)
    {
        pgpGetPrefBoolean (kPGPPrefMailEncryptDefault, &Encrypt);
        pgpLibCleanup();
    }

    return (BOOL)Encrypt;
}


BOOL ByDefaultSign()
{
    Boolean Sign = FALSE;
    PGPError error;

    error = pgpLibInit();

    if(error == PGPERR_OK)
    {
        pgpGetPrefBoolean (kPGPPrefMailSignDefault, &Sign);
        pgpLibCleanup();
    }

    return (BOOL)Sign;
}


BOOL ByDefaultWordWrap(long* pThreshold)
{
    Boolean Wrap = FALSE;
    PGPError error;

    *pThreshold = 0;

    error = pgpLibInit();

    if(error == PGPERR_OK)
    {
        pgpGetPrefBoolean (kPGPPrefMailWordWrapEnable, &Wrap);

        pgpGetPrefNumber (kPGPPrefMailWordWrapWidth, pThreshold);

        pgpLibCleanup();
    }


    return (BOOL)Wrap;
}
