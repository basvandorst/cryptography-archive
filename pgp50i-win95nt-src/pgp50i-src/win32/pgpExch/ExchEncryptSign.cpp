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
#include "pgpMem.h"
#include "PGPRecip.h"
#include "pgpcomdlg.h"
#include "spgp.h"
#include "pgpphrase.h"

#include "ExchKeyDB.h"
#include "BlockUtils.h"

#ifndef STATUS_SUCCESS
    #define STATUS_SUCCESS          0x0000000
#endif

#ifndef STATUS_FAILURE
    #define STATUS_FAILURE          0x0000001
#endif

#define KEYIDLENGTH 20
#define USERIDLENGTH 512

#define MAX_RETRIES 100

BOOL GetRecipientsInfo(HWND hwnd,
                       RECIPIENTDIALOGSTRUCT *prds,
                       DWORD PGPFlags,
                       DWORD DisableFlags);

BOOL GetSignPhrase(HWND hwnd,
                   char **EnteredOrCachedPhrase,
                   unsigned long *PassPhraseLen,
                   unsigned short *PassPhraseCount,
                   char *szSignKey, ULONG *pulHash, UINT uPhraseFlags);


UINT EncryptSignBuffer(HWND hwnd,void* pInput,DWORD dwInSize,
                       void *pRecDlgStruct,void** ppOutput,
                       DWORD* pOutSize,BOOL bEncrypt,BOOL bSign,
                       BOOL bAttachment,BOOL bUseMIME)
{
    UINT ReturnValue = STATUS_FAILURE;
    UINT LibReturn = 0;
    PRECIPIENTDIALOGSTRUCT prds = NULL;
    char *szPassPhrase = NULL;
    DWORD dwPhraseSize = 0;
    unsigned short PassPhraseCount = 0;
    char szSignKey[KEYIDLENGTH + 1];
    BOOL UserCancel = FALSE;
    ULONG ulHash = 0;
    UINT uPhraseFlags = 0;
    void *pWrappedInput = NULL;
    long Threshold = -1;

    assert(pInput);
    assert(ppOutput);
    assert(bSign || bEncrypt);

    // Check for expiration
    if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
    {
        PGPcomdlgErrorMessage(PGPCOMDLG_EXPIRED);
        return STATUS_FAILURE;
    }

    *ppOutput = NULL;

    *pOutSize = dwInSize * 2;

    if(*pOutSize < 64000)
    {
        *pOutSize = 64000;
    }

    szPassPhrase = (char *) pgpAlloc(2);
    strcpy(szPassPhrase, " ");
    memset(szSignKey, 0, KEYIDLENGTH);

    prds = (PRECIPIENTDIALOGSTRUCT) pRecDlgStruct;
    if(bEncrypt && prds)
    {
        UserCancel = GetRecipientsInfo(hwnd,
                                       prds,
                                       PGP_ENCRYPT | PGP_ASCII_ARMOR,
                                       DISABLE_ASCII_ARMOR);

    if (prds->dwPGPFlags & PGP_ATLEASTONERSAKEY)
        uPhraseFlags |= PGPCOMDLG_RSAENCRYPT;
    }

    if(!UserCancel)
    {
        DWORD RetryCount = 0;
        do
        {
            if(bSign && (LibReturn == SIMPLEPGP_BADKEYPASSPHRASE))
            {
                UserCancel = GetSignPhrase(hwnd,
                                           &szPassPhrase,
                                           &dwPhraseSize,
                                           &PassPhraseCount,
                                           szSignKey, &ulHash, uPhraseFlags);
         }

            if(!UserCancel)
            {
                if (ByDefaultWordWrap(&Threshold) && !bAttachment)
                {
                    ReturnValue = WrapBuffer((char **) &pWrappedInput,
                                                (char *) pInput,
                                                (USHORT)Threshold);
                    if (ReturnValue || !pWrappedInput)
                    {
                        pgpFree(szPassPhrase);
                        return ReturnValue;
                    }
                }
                else
                {
                pWrappedInput = pInput;
                Threshold = -1;
                }

                if(*ppOutput)
                {
                pgpFree(*ppOutput);
                }

                *ppOutput = pgpAlloc((*pOutSize + 1) * sizeof(char));
                memset(*ppOutput, '\0', (*pOutSize + 1) * sizeof(char));

                if(*ppOutput)
                {
                    if(bEncrypt)
                    {
                        LibReturn = SimplePGPEncryptBuffer(
                                hwnd,
                                (char *) pWrappedInput,
                                dwInSize,
                                "",
                                (char *) *ppOutput,
                                (unsigned int *) pOutSize,
                                bSign,       // Sign it?
                                (prds->dwPGPFlags
                                        & PGP_ASCII_ARMOR), // Armor it?
                                !bAttachment,  // Text Mode if not attachment
                                FALSE,       // Not IDEA only
                                TRUE,        // Use untrusted keys
                                prds->szRecipients,
                                szSignKey,
                                strlen(szSignKey), // strlen(Signer Id)
                                szPassPhrase,
                                dwPhraseSize,
                                NULL, //IDEA password
                                0, "", "");
/*
                        if (bUseMIME)
                        {
                            char szDelimiter[DELIMITERLENGTH];
                            emsMIMEtypeP pMimeType;

$$                          if (!ConvertToEncryptedPGPMIME((char **) ppOutput,
                                                    pOutSize, szDelimiter))
                                if (!BuildEncryptedPGPMIMEType(&pMimeType,
                                                                szDelimiter))
                                    AddMIMEParam(pMimeType, "PGPFormat",
                                                 "PGPMIME-encrypted");
                        }
*/
                    }
                    else
                    {
                        LibReturn = SimplePGPSignBuffer (
                                hwnd,           // handle
                                (char *) pWrappedInput,// input
                                dwInSize,       // input size	
                                "",             // name of buffer
                                (char *)*ppOutput, // ouput
                                (unsigned int *) pOutSize,    // output size
                                TRUE,   // Armor it?
                                !bAttachment,   // TextMode if not attachment
                                FALSE,  // Detached signature?
                                !bAttachment,   // clear sign if not attachment
                                szSignKey,      // Signer Id (char*)
                                strlen(szSignKey),  // strlen(Signer Id)
                                szPassPhrase,
                                dwPhraseSize,
                                "");
/*
                        if (bUseMIME)
                        {
                            char szDelimiter[DELIMITERLENGTH];
                            emsMIMEtypeP pMimeType;

$$                          if (!ConvertToSignedPGPMIME((char *) pInput,
                                            (char **) ppOutput, pOutSize,
                                            szDelimiter, ulHash))
                                if (!BuildEncryptedPGPMIMEType(&pMimeType,
                                                                szDelimiter))
                                    AddMIMEParam(pMimeType, "PGPFormat",
                                                 "PGPMIME-encrypted");
                        }
*/
                    }

                    switch(LibReturn)
                    {
                        case 0:
                            ReturnValue = STATUS_SUCCESS;
                            *((char *)*ppOutput + *pOutSize) = 0;
                            break;

                        case SIMPLEPGP_RNGNOTSEEDED:
                            PGPcomdlgRandom (hwnd, SimplePGPRandomNeeded(hwnd));
                            break;

                        case SIMPLEPGP_BADKEYPASSPHRASE:
                            break;  /*Do nothing*/

                        default:
                            PGPcomdlgErrorMessage(LibReturn);
                            ++RetryCount;
                            break;
                    }
                }
            }
        }while((LibReturn == SIMPLEPGPENCRYPTBUFFER_OUTPUTBUFFERTOOSMALL ||
                LibReturn == SIMPLEPGPSIGNBUFFER_OUTPUTBUFFERTOOSMALL ||
                LibReturn == SIMPLEPGP_RNGNOTSEEDED ||
                LibReturn == SIMPLEPGP_BADKEYPASSPHRASE) &&
                !UserCancel &&
                RetryCount < MAX_RETRIES);
    }

    if(szPassPhrase)
        PGPFreePhrase(szPassPhrase);

    if ((Threshold > 0) && (ReturnValue == STATUS_SUCCESS))
        pgpFree(pWrappedInput);

/*
    if(prds)
    {
        if(prds->szRecipients)
            pgpFree(prds->szRecipients);

        pgpFree(prds);
    }
*/

    return ReturnValue;
}


UINT EncryptSignFile(HWND hwnd,char *szInFile,void *pRecDlgStruct,
                     char **pszOutFile,BOOL bEncrypt,BOOL bSign,
                     BOOL bAttachment,BOOL bUseMIME)
{
    UINT ReturnValue = STATUS_FAILURE;
    UINT LibReturn = 0;;
    PRECIPIENTDIALOGSTRUCT prds = NULL;
    char *szPassPhrase = NULL;
    DWORD dwPhraseSize = 0;
    unsigned short PassPhraseCount = 0;
    char szSignKey[KEYIDLENGTH + 1];
    BOOL UserCancel = FALSE;
    ULONG ulHash = 0;
    UINT uPhraseFlags = 0;

    assert(szInFile);
    assert(pszOutFile);
    assert(bSign || bEncrypt);

    // Check for expiration
    if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
    {
        PGPcomdlgErrorMessage(PGPCOMDLG_EXPIRED);
        return STATUS_FAILURE;
    }

    szPassPhrase = (char *) pgpAlloc(2);
    strcpy(szPassPhrase, " ");
    memset(szSignKey, 0, KEYIDLENGTH);

    prds = (PRECIPIENTDIALOGSTRUCT) pRecDlgStruct;
    if(bEncrypt && prds)
    {
        UserCancel = GetRecipientsInfo(hwnd,
                                       prds,
                                       PGP_ENCRYPT | PGP_ASCII_ARMOR,
                                       DISABLE_ASCII_ARMOR);

        if (prds->dwPGPFlags & PGP_ATLEASTONERSAKEY)
            uPhraseFlags |= PGPCOMDLG_RSAENCRYPT;
    }

    if(!UserCancel)
    {
        DWORD RetryCount = 0;
        do
        {
            if(bSign && (LibReturn == SIMPLEPGP_BADKEYPASSPHRASE))
            {
                UserCancel = GetSignPhrase(hwnd,
                                           &szPassPhrase,
                                           &dwPhraseSize,
                                           &PassPhraseCount,
                                           szSignKey, &ulHash, uPhraseFlags);
            }

            if(!UserCancel)
            {
                if(bEncrypt)
                {
                    LibReturn = SimplePGPEncryptFile(
                        hwnd,
                        szInFile,
                        *pszOutFile,
                        bSign,       // Sign it?
                        FALSE,       // Wipe cleartext?
                        (prds->dwPGPFlags & PGP_ASCII_ARMOR),  // Armor it?
                        FALSE,       // Text Mode = Off
                        FALSE,       // Not IDEA only
                        TRUE,        // Use untrusted keys
                        prds->szRecipients,
                        szSignKey,
                        strlen(szSignKey), // strlen(Signer Id)
                        szPassPhrase,
                        dwPhraseSize,
                        NULL, //IDEA password
                        0, NULL, NULL);

/*
                        if (bEncryptMIME)
                        {
                            char szDelimiter[DELIMITERLENGTH];
                            emsMIMEtypeP pMimeType;

$$                          if (!ConvertToEncryptedPGPMIME((char **) ppOutput,
                                                          pOutSize, szDelimiter))
                            if (!BuildEncryptedPGPMIMEType(&pMimeType,
                                                            szDelimiter))
                            AddMIMEParam(pMimeType, "PGPFormat",
                                        "PGPMIME-encrypted");
                            }
*/
                }
                else
                {
                    LibReturn = SimplePGPSignFile(
                        hwnd,        // handle
                        szInFile,     // input
                        *pszOutFile,   // ouput
                        TRUE,  // Armor it?
                        FALSE,       // TextMode = Off
                        FALSE, // Signature Only
                        FALSE, // clear sign
                        szSignKey,    // Signer Id (char*)
                        strlen(szSignKey), // strlen(Signer Id)
                        szPassPhrase,
                        dwPhraseSize,
                        "");

/*
                        if (bSignMIME)
                        {
                            char szDelimiter[DELIMITERLENGTH];
                            emsMIMEtypeP pMimeType;

$$                          if (!ConvertToSignedPGPMIME((char *) pInput,
                                          (char **) ppOutput, pOutSize,
                                          szDelimiter, ulHash))
                            if (!BuildEncryptedPGPMIMEType(&pMimeType,
                                                            szDelimiter))
                            AddMIMEParam(pMimeType, "PGPFormat",
                                            "PGPMIME-encrypted");
                        }
*/
                }

                switch(LibReturn)
                {
                case 0:
                    ReturnValue = STATUS_SUCCESS;
                    break;

                case SIMPLEPGP_RNGNOTSEEDED:
                    PGPcomdlgRandom (hwnd, SimplePGPRandomNeeded(hwnd));
                    break;

                case SIMPLEPGP_BADKEYPASSPHRASE:
                    break;  /*Do nothing*/

                default:
                    PGPcomdlgErrorMessage(LibReturn);
                    ++RetryCount;
                    break;
                }
            }
        }while((LibReturn == SIMPLEPGPENCRYPTBUFFER_OUTPUTBUFFERTOOSMALL ||
            LibReturn == SIMPLEPGPSIGNBUFFER_OUTPUTBUFFERTOOSMALL ||
            LibReturn == SIMPLEPGP_RNGNOTSEEDED ||
            LibReturn == SIMPLEPGP_BADKEYPASSPHRASE) &&
            !UserCancel &&
            RetryCount < MAX_RETRIES);
    }

    if(szPassPhrase)
        PGPFreePhrase(szPassPhrase);

/*
    if(prds)
    {
        if(prds->szRecipients)
            pgpFree(prds->szRecipients);

        pgpFree(prds);
    }
*/

    return ReturnValue;
}


BOOL GetRecipientsInfo(HWND hwnd,
                       RECIPIENTDIALOGSTRUCT *prds,
                       DWORD PGPFlags,
                       DWORD DisableFlags)
{
    BOOL UserCancel = FALSE;
    UINT RecipientReturn = RCPT_DLG_ERROR_SUCCESS;

    assert(prds);

    if(!(prds->sVersion))
    {
        do
        {
            prds->sVersion = RCPT_DLG_VERSION;
            prds->hwndParent = hwnd;
            prds->szTitle = "PGP - Key Selection Dialog";
            prds->dwPGPFlags = PGPFlags;
            prds->dwDisableFlags = DisableFlags;

            RecipientReturn = PGPRecipientDialog(prds);

            if(RecipientReturn == RCPT_DLG_ERROR_SUCCESS &&
                        !(prds)->dwNumRecipients)
                MessageBox(hwnd,    "In order to Encrypt you must choose\n"
                                    "at least one recipient.", "No Recipients",
                                    MB_OK|MB_ICONWARNING);
        }while(RecipientReturn == RCPT_DLG_ERROR_SUCCESS &&
            !(prds)->dwNumRecipients);

        if(RecipientReturn != RCPT_DLG_ERROR_SUCCESS)
            UserCancel = TRUE;
    }
    else
    {
        prds->dwPGPFlags = PGPFlags;
    }

    return(UserCancel);
}

BOOL GetSignPhrase(HWND hwnd,
                   char **pszPassPhrase,
                   DWORD *pdwPhraseSize,
                   unsigned short *PassPhraseCount,
                   char *szSignKey, ULONG *pulHash, UINT uPhraseFlags)
{
    BOOL UserCancel = FALSE;
    BOOL ForceEntry = FALSE;
    char PhraseTitle[256];

    assert(pszPassPhrase);
    assert(pdwPhraseSize);
    assert(szSignKey);
    assert(pulHash);

    if(*pszPassPhrase)
    {
        PGPFreePhrase(*pszPassPhrase);
        *pszPassPhrase = NULL;
        *pdwPhraseSize = 0;
    }

    if(PassPhraseCount && *PassPhraseCount > 0)
    {
        GetString(PhraseTitle, sizeof(PhraseTitle), IDS_WRONG_PHRASE);
        ForceEntry = TRUE;
    }
    else
        GetString(PhraseTitle, sizeof(PhraseTitle), IDS_ENTER_PHRASE);


    if(PGPGetSignCachedPhrase (hwnd,
                               PhraseTitle,
                               ForceEntry,
                               pszPassPhrase,
                               szSignKey,
                               pulHash,
                               NULL,
                               uPhraseFlags) != PGPPHRASE_OK)
    {
        UserCancel = TRUE;
    }
    else
    {
        *pdwPhraseSize = strlen(*pszPassPhrase);
    }
    if(PassPhraseCount)
        ++(*PassPhraseCount);

    return(UserCancel);
}
