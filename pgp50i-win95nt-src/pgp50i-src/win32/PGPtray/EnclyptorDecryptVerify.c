/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)
#include <assert.h>

#include "..\include\config.h"
#include "..\include\pgprecip.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpcomdlg.h"
#include "..\include\spgp.h"
#include "..\include\pgpphrase.h"
#include "..\include\pgpmem.h"

#include "define.h"
#include "EnclyptorDecryptVerify.h"
#include "Clipboard.h"
#include "messagestrings.h"
#include "errs.h"

static unsigned long ReAllocAndCopy(char **Dest, char *Source);

UINT EnclyptorDecryptVerify(HWND hwnd,void* pInput,DWORD dwInSize,
							void** ppOutput,DWORD* pOutSize)
{
	UINT ReturnValue = STATUS_FAILURE;
	UINT PassPhraseReturn = PGPPHRASE_OK;
	char *EnteredOrCachedPhrase = NULL, EmptyPhrase[2] = "\0";
	char *PassPhraseToUse = EmptyPhrase;
	char Signer[256];
	char SignatureDate[64];
	char SignatureResultBuffer[512];
	DWORD RetryCount = 0;
	UINT ReceiveReturn = 0;
	UINT SignatureResult = 0;
	unsigned short NumPassphrasesRequested = 0;
	char TempErrorMessage[1024];

	assert(pInput);
	assert(ppOutput);

	// Check for expiration
	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED))
	{
		PGPcomdlgErrorMessage(PGPCOMDLG_EXPIRED);
		return STATUS_FAILURE;
	}

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

		if(ReceiveReturn == SIMPLEPGP_BADKEYPASSPHRASE ||
		   ReceiveReturn == SIMPLEPGP_BADCONVENTIONALPASSPHRASE)
		{
			/*Looks like they entered a wrong passphrase.
			 *A more common scenario is that they actually HAVE a passphrase; we
			 *try NO passphrase, first, in case that works.
			 */
			if(EnteredOrCachedPhrase)
			{
				/*Free last bad passphrase, if any*/
				PGPFreePhrase(EnteredOrCachedPhrase);
				EnteredOrCachedPhrase = NULL;
			}

			if(!NumPassphrasesRequested)
			{
				if (ReceiveReturn == SIMPLEPGP_BADKEYPASSPHRASE)
					PassPhraseReturn = PGPGetCachedPhrase (hwnd,
										"Enter passphrase for private key:",
										FALSE,
										&EnteredOrCachedPhrase);
				else
					PassPhraseReturn = PGPGetCachedPhrase (hwnd,
										"Enter decryption passphrase:",
										FALSE,
										&EnteredOrCachedPhrase);
			}
			else
				PassPhraseReturn = PGPGetCachedPhrase (hwnd,
								"The entered or cached phrase was incorrect.",
								TRUE,
								&EnteredOrCachedPhrase);

			PassPhraseToUse = EnteredOrCachedPhrase;
			++NumPassphrasesRequested;
		}
		else
			PassPhraseReturn = PGPPHRASE_OK;

		*ppOutput = pgpAlloc((*pOutSize + 1) * sizeof(char));
		memset(*ppOutput, '\0', (*pOutSize + 1) * sizeof(char));
		
		if(*ppOutput &&	PassPhraseReturn == PGPPHRASE_OK)
		{
			//Try to encrypt:

			ReceiveReturn = SimplePGPReceiveBuffer(
											hwnd,
											pInput,
											dwInSize,			
											"Message",
											*ppOutput,
											(unsigned int *) pOutSize,
											PassPhraseToUse,
											strlen(PassPhraseToUse),
											(int *) &SignatureResult,
											Signer,
											sizeof(Signer),
											(unsigned char *) SignatureDate,
											sizeof(SignatureDate),
											"", "");				
			if(ReceiveReturn == 0)
				ReturnValue = STATUS_SUCCESS;
		}

		if(ReceiveReturn != SIMPLEPGP_BADKEYPASSPHRASE &&
		   ReceiveReturn != SIMPLEPGP_BADCONVENTIONALPASSPHRASE)
			++RetryCount;

	}while((ReceiveReturn == SIMPLEPGPRECEIVEBUFFER_OUTPUTBUFFERTOOSMALL ||
			ReceiveReturn == SIMPLEPGP_BADKEYPASSPHRASE ||
			ReceiveReturn == SIMPLEPGP_BADCONVENTIONALPASSPHRASE) &&
			RetryCount < MAX_RETRIES &&
			PassPhraseReturn == PGPPHRASE_OK);

	if(EnteredOrCachedPhrase)
		PGPFreePhrase(EnteredOrCachedPhrase);

	if(PassPhraseReturn != PGPPHRASE_USERCANCEL) {
		if(PGPcomdlgErrorToString(ReceiveReturn, TempErrorMessage,
									sizeof(TempErrorMessage)))
		{
			DisplayDecryptionResults(hwnd, TempErrorMessage);
		}
		else
		{
			StoreClipboardData(hwnd, *ppOutput, *pOutSize);
			if(PGPcomdlgTranslateSignatureResult(SignatureResult,
											 Signer,
											 SignatureDate,
											 SignatureResultBuffer,
											 sizeof(SignatureResultBuffer)))
			{
				DisplayDecryptionResults(hwnd, SignatureResultBuffer);
			}
			else
			{
				DisplayDecryptionResults(hwnd, "Decryption Successful.");
			}
		}
	}

	return ReturnValue;
}
