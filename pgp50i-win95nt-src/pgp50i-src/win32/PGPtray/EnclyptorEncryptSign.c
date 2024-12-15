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
#include "..\include\PGPRecip.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpcomdlg.h"
#include "..\include\spgp.h"
#include "..\include\pgpphrase.h"
#include "..\include\pgpMem.h"

#include "define.h"
#include "enclyptorkeydb.h"
#include "resource.h"

extern HINSTANCE g_hinst;

BOOL GetRecipientsInfo(HWND hwnd,
					   RECIPIENTDIALOGSTRUCT **prds,
					   DWORD PGPFlags,
					   DWORD DisableFlags);
BOOL GetSignPhrase(HWND hwnd,
				   char **EnteredOrCachedPhrase,
				   unsigned long *PassPhraseLen,
				   unsigned short *PassPhraseCount,
				   char *szSignKey,
				   UINT uFlags);

UINT EnclyptorEncryptSign(HWND hwnd,void* pInput,DWORD dwInSize,
				void** ppOutput,DWORD* pOutSize, BOOL Encrypt, BOOL Sign)
{
	UINT ReturnValue = STATUS_FAILURE, LibReturn = PGPERR_OK;
	PRECIPIENTDIALOGSTRUCT prds = NULL;
	char *EnteredOrCachedPhrase = NULL;
	unsigned long PassPhraseLen = 0;
	unsigned short PassPhraseCount = 0;
	char szSignKey[KEYIDLENGTH + 1] = "\0";
	BOOL UserCancel = FALSE;

	assert(pInput);
	assert(ppOutput);
	assert(Sign || Encrypt);

	// Check for expiration
	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
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

	if(Encrypt)
	{
		UserCancel = GetRecipientsInfo(hwnd,
									   &prds,
									   PGP_ENCRYPT | PGP_ASCII_ARMOR,
									   DISABLE_ASCII_ARMOR);
	}

	if(!UserCancel)
	{
		DWORD RetryCount = 0;
		do
		{
			if(Sign)
			{
				UINT uPhraseFlags = 0;
				if (prds && (prds->dwPGPFlags & PGP_ATLEASTONERSAKEY))
					uPhraseFlags |= PGPCOMDLG_RSAENCRYPT;
				UserCancel = GetSignPhrase(hwnd,
										   &EnteredOrCachedPhrase,
										   &PassPhraseLen,
										   &PassPhraseCount,
										   szSignKey,
										   uPhraseFlags);
			}
			else
			{
				PassPhraseLen = 0;
				EnteredOrCachedPhrase = NULL;
			}

			if(!UserCancel)
			{
				if(*ppOutput)
				{
					pgpFree(*ppOutput);
				}

				*ppOutput = pgpAlloc(*pOutSize * sizeof(char));
				memset(*ppOutput, '\0', *pOutSize * sizeof(char));

				if(*ppOutput)
				{
					if(Encrypt)
					{
						LibReturn = SimplePGPEncryptBuffer(	
									hwnd,
									pInput,
									dwInSize,			
									"Message",
									*ppOutput,
									(unsigned int *) pOutSize,
									Sign,			// Sign it?

									// Armor it?
									(prds->dwPGPFlags &  PGP_ASCII_ARMOR),

									TRUE,			// Text Mode = On
									FALSE,			// Not IDEA only
									TRUE,			// Use untrusted keys
									prds->szRecipients,
									szSignKey,
									strlen(szSignKey),	// strlen(Signer Id)
									EnteredOrCachedPhrase,
									PassPhraseLen,
									NULL, //IDEA password
									0, "", "");
					}
					else
					{
						LibReturn = SimplePGPSignBuffer (	
									hwnd,			// handle
									pInput,			// input
									dwInSize,		// input size	
									"Message",		// name of buffer
									*ppOutput,		// ouput
									(unsigned int *) pOutSize,	// output size
									TRUE,			// Armor it?
									TRUE,			// TextMode = On
									FALSE,			// Signature Only
									TRUE,			// clear sign
									szSignKey,		// Signer Id (char*)
									strlen(szSignKey),	// strlen(Signer Id)
									EnteredOrCachedPhrase,
									PassPhraseLen,
									"");
					}

					switch(LibReturn)
					{
						case 0:
							ReturnValue = STATUS_SUCCESS;
							*pOutSize += 1; /*Make room for a NULL*/
							break;

						case SIMPLEPGP_RNGNOTSEEDED:
							if(PGPcomdlgRandom (hwnd,
												SimplePGPRandomNeeded(hwnd))
										== PGPCOMDLG_CANCEL)
								UserCancel = TRUE;
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
				
	if(EnteredOrCachedPhrase)
		PGPFreePhrase(EnteredOrCachedPhrase);

	if(prds)
	{
		if(prds->szRecipients)
			pgpFree(prds->szRecipients);

		pgpFree(prds);
	}

	return ReturnValue;
}

BOOL GetRecipientsInfo(HWND hwnd,
					   RECIPIENTDIALOGSTRUCT **prds,
					   DWORD PGPFlags,
					   DWORD DisableFlags)
{
	BOOL UserCancel = FALSE;
	UINT RecipientReturn = RCPT_DLG_ERROR_SUCCESS;

	assert(prds);

	*prds = (PRECIPIENTDIALOGSTRUCT) pgpAlloc(sizeof(RECIPIENTDIALOGSTRUCT));
	if(*prds)
	{
		do
		{
			memset(*prds, '\0', sizeof(RECIPIENTDIALOGSTRUCT) );

			(*prds)->sVersion = RCPT_DLG_VERSION;
			(*prds)->hwndParent = hwnd;
			(*prds)->szTitle = "PGP - Key Selection Dialog";
			(*prds)->dwPGPFlags = PGPFlags;
			(*prds)->dwDisableFlags = DisableFlags;

			RecipientReturn = PGPRecipientDialog(*prds);

			if(RecipientReturn == RCPT_DLG_ERROR_SUCCESS &&
						!(*prds)->dwNumRecipients)
				MessageBox(NULL,	"In order to Encrypt you must choose\n"
									"at least one recipient.", "No Recipients",
									MB_OK|MB_ICONWARNING);
		}while(RecipientReturn == RCPT_DLG_ERROR_SUCCESS &&
			    !(*prds)->dwNumRecipients);

		if(RecipientReturn != RCPT_DLG_ERROR_SUCCESS)
			UserCancel = TRUE;
	}

	return(UserCancel);
}

BOOL GetSignPhrase(HWND hwnd,
				   char **EnteredOrCachedPhrase,
				   unsigned long *PassPhraseLen,
				   unsigned short *PassPhraseCount,
				   char *szSignKey,
				   UINT uFlags)
{
	BOOL UserCancel = FALSE;
	BOOL ForceEntry = FALSE;
	char PhraseTitle[256];

	assert(EnteredOrCachedPhrase);
	assert(PassPhraseLen);
	assert(szSignKey);

	if(*EnteredOrCachedPhrase)
	{
		PGPFreePhrase(*EnteredOrCachedPhrase);
		*EnteredOrCachedPhrase = NULL;
	}

	if(PassPhraseCount && *PassPhraseCount > 0)
	{
		LoadString(g_hinst, IDS_WRONG_PHRASE, PhraseTitle, sizeof(PhraseTitle));
		ForceEntry = TRUE;
	}
	else
		LoadString(g_hinst, IDS_ENTER_PHRASE, PhraseTitle, sizeof(PhraseTitle));

	if(PGPGetSignCachedPhrase (hwnd,
							   PhraseTitle,
							   ForceEntry,
							   EnteredOrCachedPhrase,
							   szSignKey,
							   NULL,
							   NULL,
							   uFlags) != PGPPHRASE_OK)
	{
		UserCancel = TRUE;
	}
	else
	{
		*PassPhraseLen = (unsigned long) strlen(*EnteredOrCachedPhrase);
	}

	if(PassPhraseCount)
		++(*PassPhraseCount);

	return(UserCancel);
}
