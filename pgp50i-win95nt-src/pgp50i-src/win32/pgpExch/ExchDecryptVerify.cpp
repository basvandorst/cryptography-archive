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

#ifndef STATUS_SUCCESS
    #define STATUS_SUCCESS       0x0000000
#endif

#ifndef STATUS_FAILURE
    #define STATUS_FAILURE       0x0000001
#endif

#define KEYIDLENGTH 20
#define USERIDLENGTH 512

#define MAX_RETRIES 100

static unsigned long ReAllocAndCopy(char **Dest, char *Source);
static BOOL TranslateSignatureResult(char *SignatureResult,
                                        char *Signer,
                                        char *SignatureDate,
                                        int SignatureCode);


UINT DecryptVerifyBuffer(HWND hwnd,void* pInput,DWORD dwInSize,
                         void* pMessage, DWORD dwMsgSize,
                         void** ppOutput,DWORD* pOutSize)
{
    UINT ReturnValue = STATUS_FAILURE;
    UINT PassPhraseReturn = PGPPHRASE_OK;
    char *szPassPhrase = NULL;
    DWORD dwPhraseSize = 0;
    char Signer[256];
    char SignatureDate[64];
    char SignatureResultBuffer[512];
    DWORD RetryCount = 0;
    UINT ReceiveReturn = 0;
    UINT SignatureResult = 0;
    unsigned short NumPassphrasesRequested = 0;
    char TempErrorMessage[1024];
    char szTitle[100];

    assert(pInput);
    assert(ppOutput);

    // Check for expiration
    if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED))
    {
        PGPcomdlgErrorMessage(PGPCOMDLG_EXPIRED);
        return STATUS_FAILURE;
    }

    szPassPhrase = (char *) pgpAlloc(2);
    strcpy(szPassPhrase, " ");

    GetString(szTitle, sizeof(szTitle), IDS_TITLE);

    *ppOutput = NULL;

    *pOutSize = dwInSize * 2;

    if(*pOutSize < 65535)
    {
        *pOutSize = 65535;
    }

    do
    {
        if(*ppOutput)
        {
            pgpFree(*ppOutput);
        }

        if(ReceiveReturn == SIMPLEPGP_BADKEYPASSPHRASE)
        {
            /*Looks like they entered a wrong passphrase.
             *A more common scenario is that they actually HAVE a passphrase; we
             *try NO passphrase, first, in case that works.
             */
            if(szPassPhrase)
            {
                /*Free last bad passphrase, if any*/
                PGPFreePhrase(szPassPhrase);
                szPassPhrase = NULL;
            }

            if(!NumPassphrasesRequested)
                PassPhraseReturn = PGPGetCachedPhrase (hwnd,
                                       "Enter passphrase for private key:",
                                       FALSE,
                                       &szPassPhrase);
            else
                PassPhraseReturn = PGPGetCachedPhrase (hwnd,
                                       "Incorrect passphrase.  Enter again:",
                                       TRUE,
                                       &szPassPhrase);

            ++NumPassphrasesRequested;
        }
        else
            PassPhraseReturn = PGPPHRASE_OK;

        if (PassPhraseReturn == PGPCOMDLG_OK)
            PassPhraseReturn = PGPPHRASE_OK;

        *ppOutput = pgpAlloc((*pOutSize + 1) * sizeof(char));
        memset(*ppOutput, '\0', (*pOutSize + 1) * sizeof(char));

        if(*ppOutput && PassPhraseReturn == PGPPHRASE_OK)
        {
            //Try to decrypt:
            dwPhraseSize = strlen(szPassPhrase);
            ReceiveReturn = SimplePGPReceiveBuffer( hwnd,
                                            (char *) pInput,
                                            dwInSize,
                                            "Message",
                                            (char *) *ppOutput,
                                            (unsigned int *) pOutSize,
                                            szPassPhrase,
                                            dwPhraseSize,
                                            (int *) &SignatureResult,
                                            Signer,
                                            sizeof(Signer),
                                            (unsigned char *) SignatureDate,
                                            sizeof(SignatureDate),
                                            "", "");
            if(ReceiveReturn == 0)
            {
                ReturnValue = STATUS_SUCCESS;
                *((char *)*ppOutput + *pOutSize) = 0;
            }


            if ((ReceiveReturn == SIMPLEPGP_DETACHEDSIGNATUREFOUND)
                        && (pMessage))
            {
                ReceiveReturn = SimplePGPVerifyDetachedSignatureBuffer( hwnd,
                                            (char *) pMessage,
                                            dwMsgSize,
                                            (char *) pInput,
                                            dwInSize,
                                            (int *) &SignatureResult,
                                            Signer,
                                            sizeof(Signer),
                                            (unsigned char *) SignatureDate,
                                            sizeof(SignatureDate),
                                            "");
                *pOutSize = 0;
            }
        }

        if(ReceiveReturn != SIMPLEPGP_BADKEYPASSPHRASE)
            ++RetryCount;

    }while((ReceiveReturn == SIMPLEPGPRECEIVEBUFFER_OUTPUTBUFFERTOOSMALL ||
            ReceiveReturn == SIMPLEPGP_BADKEYPASSPHRASE) &&
            RetryCount < MAX_RETRIES &&
           PassPhraseReturn == PGPPHRASE_OK);

    if(szPassPhrase)
        PGPFreePhrase(szPassPhrase);

    if(PGPcomdlgErrorToString(ReceiveReturn, TempErrorMessage,
        sizeof(TempErrorMessage)))
    {
        MessageBox(hwnd, TempErrorMessage, szTitle, MB_ICONEXCLAMATION);
    }
    else
    {
        if(TranslateSignatureResult(SignatureResultBuffer,
                                     Signer,
                                     SignatureDate,
                                     SignatureResult))
        {
            MessageBox(hwnd, SignatureResultBuffer, szTitle, MB_OK);
        }
    }

    if (ReturnValue != STATUS_SUCCESS)
        ReturnValue = ReceiveReturn;

    return ReturnValue;
}


UINT DecryptVerifyFile(HWND hwnd,char *szInFile, char *szSigFile,
                       char **pszOutFile)
{
    UINT ReturnValue = STATUS_FAILURE;
    UINT PassPhraseReturn = PGPPHRASE_OK;
    char *szPassPhrase = NULL;
    DWORD dwPhraseSize = 0;
    char Signer[256];
    char SignatureDate[64];
    char SignatureResultBuffer[512];
    DWORD RetryCount = 0;
    UINT ReceiveReturn = 0;
    UINT SignatureResult = 0;
    unsigned short NumPassphrasesRequested = 0;
    char TempErrorMessage[1024];
    char szTitle[100];
    char* FirstDot = NULL;
    char* SecondDot = NULL;
    char *p, *DecryptionResultMsg = NULL;
    int FileStart = 0, FileExtensionStart = 0;
    PGPFileRef *InputFileRef = NULL, *OutputFileRef = NULL;
    char *szOutFile = NULL;

    assert(szInFile);
    assert(pszOutFile);

    // Check for expiration
    if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED))
    {
        PGPcomdlgErrorMessage(PGPCOMDLG_EXPIRED);
        return STATUS_FAILURE;
    }

    GetString(szTitle, sizeof(szTitle), IDS_TITLE);
    *pszOutFile = (char *) pgpAlloc(MAX_PATH * sizeof(char));
    szOutFile = *pszOutFile;

    szPassPhrase = (char *) pgpAlloc(2);
    strcpy(szPassPhrase, " ");

    if (!szSigFile)
    {
        FirstDot = strrchr(szInFile, '.');

        if(FirstDot)
        {
            SecondDot = strrchr(FirstDot - 1, '.');

            if(SecondDot)
            {
                *FirstDot = 0x00;
                strcpy(szOutFile, szInFile);
                *FirstDot = '.';
            }
            else
            {
                *FirstDot = '\0';
            }
        }

        if((p = strrchr(szOutFile, '\\')))
            FileStart = p - szOutFile + 1;

        if((p = strrchr(szOutFile, '.')))
            FileExtensionStart = p - szOutFile + 1;

        OPENFILENAME SaveFileName = {
                sizeof(SaveFileName),
                hwnd,
                NULL,
                "All Files (*.*)\0*.*\0\0",
                NULL,
                0,
                1,
                szOutFile,
                MAX_PATH,
                NULL,
                0,
                NULL,
                "Decrypt/Verify attachment and save as",
                OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
                FileStart,
                FileExtensionStart,
                "",
                NULL,
                NULL,
                NULL};


        if(!GetSaveFileName(&SaveFileName))
            return STATUS_FAILURE;
    }
    else
        strcpy(szOutFile, szInFile);

    do
    {
        if(ReceiveReturn == SIMPLEPGP_BADKEYPASSPHRASE)
        {
            /*Looks like they entered a wrong passphrase.
             *A more common scenario is that they actually HAVE a passphrase; we
             *try NO passphrase, first, in case that works.
             */
            if(szPassPhrase)
            {
                /*Free last bad passphrase, if any*/
                PGPFreePhrase(szPassPhrase);
                szPassPhrase = NULL;
            }

            if(!NumPassphrasesRequested)
                PassPhraseReturn = PGPGetCachedPhrase (hwnd,
                                       "Enter passphrase for private key:",
                                       FALSE,
                                       &szPassPhrase);
            else
                PassPhraseReturn = PGPGetCachedPhrase (hwnd,
                                       "Incorrect passphrase.  Enter again:",
                                       TRUE,
                                       &szPassPhrase);

            ++NumPassphrasesRequested;
        }
        else
            PassPhraseReturn = PGPPHRASE_OK;

        if(PassPhraseReturn == PGPPHRASE_OK)
        {
            //Try to decrypt:
            dwPhraseSize = strlen(szPassPhrase);
            if (!szSigFile)
                ReceiveReturn = SimplePGPReceiveFile(   hwnd,
                                            szInFile,
                                            *pszOutFile,
                                            szPassPhrase,
                                            dwPhraseSize,
                                            (int *) &SignatureResult,
                                            Signer,
                                            sizeof(Signer),
                                            (unsigned char *) SignatureDate,
                                            sizeof(SignatureDate),
                                            "", "");

            if (szSigFile)
                ReceiveReturn = SimplePGPVerifyDetachedSignatureFile(  hwnd,
                                            szInFile,
                                            szSigFile,
                                            (int *) &SignatureResult,
                                            Signer,
                                            sizeof(Signer),
                                            (unsigned char *) SignatureDate,
                                            sizeof(SignatureDate),
                                            "");

            if(ReceiveReturn == 0)
            {
                ReturnValue = STATUS_SUCCESS;
            }

        }

        if(ReceiveReturn != SIMPLEPGP_BADKEYPASSPHRASE)
            ++RetryCount;

    }while((ReceiveReturn == SIMPLEPGPRECEIVEBUFFER_OUTPUTBUFFERTOOSMALL ||
            ReceiveReturn == SIMPLEPGP_BADKEYPASSPHRASE) &&
            RetryCount < MAX_RETRIES &&
            PassPhraseReturn == PGPPHRASE_OK);

    if(szPassPhrase)
        PGPFreePhrase(szPassPhrase);

    if(PGPcomdlgErrorToString(ReceiveReturn, TempErrorMessage,
        sizeof(TempErrorMessage)))
    {
        MessageBox(hwnd, TempErrorMessage, szTitle, MB_ICONEXCLAMATION);
    }
    else
    {
        if(TranslateSignatureResult(SignatureResultBuffer,
                                     Signer,
                                     SignatureDate,
                                     SignatureResult))
        {
            MessageBox(hwnd, SignatureResultBuffer, szTitle, MB_OK);
        }
    }

    if (ReturnValue != STATUS_SUCCESS)
        ReturnValue = ReceiveReturn;

    return ReturnValue;
}


static BOOL TranslateSignatureResult(   char *SignatureResult,
                                        char *Signer,
                                        char *SignatureDate,
                                        int SignatureCode)
{
    char ConditionalMessage[255] = "\0"; //This is for the cases where we
                                         //sometimes add some more info.
    char szSigFormat[255];
    BOOL ReturnCode = FALSE;

    assert(SignatureResult);
    assert(Signer);
    assert(SignatureDate);

    *SignatureResult = '\0';

    switch(SignatureCode)
    {
        case SIGSTS_VERIFIED_UNTRUSTED:
            GetString(szSigFormat, sizeof(szSigFormat), IDS_SIG_UNCERTIFIED);
            wsprintf(ConditionalMessage, szSigFormat, Signer);
            //Fall through

       case SIGSTS_VERIFIED:
            GetString(szSigFormat, sizeof(szSigFormat), IDS_SIG_VERIFIED);
            wsprintf(SignatureResult,
                    szSigFormat,
                    Signer,
                    SignatureDate);
            strcat(SignatureResult, ConditionalMessage);
            break;

       case SIGSTS_NOTVERIFIED:
            GetString(szSigFormat, sizeof(szSigFormat), IDS_SIG_UNVERIFIED);
            strcpy(SignatureResult, szSigFormat);
            break;

       case SIGSTS_BADSIG:
            if(*Signer)
            {
                GetString(szSigFormat, sizeof(szSigFormat), IDS_SIG_BAD);
                wsprintf(SignatureResult, szSigFormat, Signer);
            }
            else
            {
                GetString(szSigFormat, sizeof(szSigFormat), IDS_SIG_NONE);
                strcpy(SignatureResult, szSigFormat);
            }

            if(*SignatureDate)
            {
                GetString(szSigFormat, sizeof(szSigFormat), IDS_SIG_DATE);
                wsprintf(ConditionalMessage,
                        szSigFormat,
                        SignatureDate);
                strcat(SignatureResult, ConditionalMessage);
            }
            break;
    }

    if(*SignatureResult)
    {
        ReturnCode = TRUE;
    }

    return(ReturnCode);
}


static unsigned long ReAllocAndCopy(char **Dest, char *Source)
{
    unsigned long ReturnCode = STATUS_SUCCESS;

    assert(Dest);
    assert(Source);

    if(!*Dest)
    {
        *Dest = (char *) malloc((strlen(Source) + 1) * sizeof(char));
    }
    else
    {
        *Dest = (char *) realloc(*Dest, (strlen(Source) + 1) * sizeof(char));
    }

    if(*Dest)
    {
        strcpy(*Dest, Source);
    }
    else
        ReturnCode = STATUS_FAILURE;

    return(ReturnCode);
}
