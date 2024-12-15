/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ExchEncryptSign.cpp,v 1.41 1997/10/11 01:08:05 lloyd Exp $
____________________________________________________________________________*/
#include <windows.h>

#include "stdinc.h"
#include "pgpExch.h"
#include "resource.h"
#include "UIutils.h"
#include "ExchPrefs.h"

#include "pgpConfig.h"
#include "pgpEncode.h"
#include "pgpMem.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "pgpSDKPrefs.h"

#include "pgpVersionHeader.h"
#include "PGPRecip.h"
#include "PGPcmdlg.h"
#include "PGPphras.h"

#include "BlockUtils.h"
#include "Working.h"

static int nProgressCount = 0;

PGPError EncryptSignEvent(PGPContextRef context, 
						  PGPEvent *event, 
						  PGPUserValue userValue);

BOOL WrapBuffer(char **pszOutText,
				char *szInText,
				PGPUInt16 wrapLength)
{
	BOOL RetVal = FALSE;
	PGPError err;
	char *cmdlgBuffer;

	err=PGPcomdlgWrapBuffer(
					szInText,
					wrapLength,
					&cmdlgBuffer);

	if(IsntPGPError (err))
	{
		int memamt,length;

		length=strlen(cmdlgBuffer);
		memamt=length+1;

		*pszOutText=(char *)malloc(memamt);
		memcpy(*pszOutText,cmdlgBuffer,length);
		(*pszOutText)[length]=0;
		PGPcomdlgFreeWrapBuffer(cmdlgBuffer);
		RetVal = TRUE;
	}

	return RetVal;
}


PGPError EncryptSignBuffer(HWND hwnd, PGPContextRef context, void *pInput,
						   DWORD dwInSize, RECIPIENTDIALOGSTRUCT *prds,
						   PGPOptionListRef *pSignOptions,
						   void **ppOutput, PGPSize *pOutSize, BOOL bEncrypt, 
						   BOOL bSign, BOOL bAttachment, BOOL bUseMIME)
{
	PGPError			err				= kPGPError_NoErr;
	PGPKeyRef			signKey			= NULL;
	PGPKeySetRef		pubKeySet		= NULL;
	PGPKeySetRef		recipKeySet		= NULL;
	PGPOptionListRef	options			= NULL;
	PGPOptionListRef	tempOptions		= NULL;
	PGPCipherAlgorithm  prefAlg			= kPGPCipherAlgorithm_CAST5;
	PGPCipherAlgorithm	allowedAlgs[3];
	void *				pFinalInput		= NULL;
	long				lWrapWidth		= 0;
	BOOL				bInputWrapped	= FALSE;
	char *				szPassphrase	= NULL;
	char *				szConvPass		= NULL;
	char *				szPrompt		= NULL;
	int					nPassphraseLen	= 0;
	int					nConvPassLen	= 0;
	int					nNumAlgs		= 0;
	BOOL				bGotPassphrase	= FALSE;
	BOOL				bGotConvPass	= FALSE;
	HWND				hwndWorking		= NULL;
	char 				szComment[255];

	pgpAssert(pInput != NULL);
	pgpAssert(prds != NULL);
	pgpAssert(pSignOptions != NULL);
	pgpAssert(ppOutput != NULL);
	pgpAssert(pOutSize != NULL);
	pgpAssert(PGPRefIsValid(context));

	UpdateWindow(hwnd);

	// Check for demo expiration

	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
		return kPGPError_UserAbort;

	if (prds->OriginalKeySetRef == NULL)
	{
		err = PGPsdkLoadDefaultPrefs(context);
		if (IsPGPError(err))
		{
			UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
			goto EncryptSignBufferError;
		}

		err = PGPOpenDefaultKeyRings(context, (PGPKeyRingOpenFlags) 0, 
				&pubKeySet);
		prds->OriginalKeySetRef = pubKeySet;
	}
	else
		pubKeySet = prds->OriginalKeySetRef;

	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto EncryptSignBufferError;
	}
	
	err = PGPGetDefaultPrivateKey(pubKeySet, &signKey);
	if (err == kPGPError_ItemNotFound)
	{
		PGPKeyListRef	pubKeyList = NULL;
		PGPKeyIterRef	pubKeyIter = NULL;
		
		PGPOrderKeySet(pubKeySet, kPGPTrustOrdering, &pubKeyList);
		PGPNewKeyIter(pubKeyList, &pubKeyIter);
		PGPKeyIterNext(pubKeyIter, &signKey);
		PGPFreeKeyIter(pubKeyIter);
		PGPFreeKeyList(pubKeyList);
	}
	else if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto EncryptSignBufferError;
	}

	pFinalInput = pInput;
	if (!bAttachment)
	{
		if (ByDefaultWordWrap(&lWrapWidth))
		{
			pFinalInput = NULL;
			bInputWrapped = WrapBuffer((char **) &pFinalInput, 
								(char *) pInput, (short) lWrapWidth);
			dwInSize = strlen((char *) pFinalInput);
		}
	}

	err = kPGPError_BadPassphrase;
	while (err == kPGPError_BadPassphrase)
	{
		if (IsntNull(szPassphrase))
		{
			PGPFreePhrase(szPassphrase);
			szPassphrase = NULL;
		}

		if (IsntNull(szConvPass))
		{
			PGPcomdlgFreePhrase(szConvPass);
			szConvPass = NULL;
		}

		err = PGPBuildOptionList(context, &options, PGPOLastOption(context));
		if (IsPGPError(err))
		{
			UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
			goto EncryptSignBufferError;
		}

		if (GetCommentString(szComment, 254))
		{
			err = PGPBuildOptionList(context, &tempOptions,
					options,
					PGPOCommentString(context, szComment),
					PGPOLastOption(context));
			
			if (IsPGPError(err))
			{
				UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
				goto EncryptSignBufferError;
			}

			PGPFreeOptionList(options);
			options = tempOptions;
		}

		if (bEncrypt)
		{
			if (prds->dwOptions & PGPCOMDLG_PASSONLY)
			{
				if (!bGotConvPass)
				{
					szPrompt = "Enter conventional passphrase:";
					err = PGPcomdlgGetPhrase(context, hwnd, szPrompt, 
						&szConvPass, NULL, NULL, 0, NULL, NULL, 
						PGPCOMDLG_ENCRYPTION);
					if (err == kPGPError_UserAbort)
						goto EncryptSignBufferError;
					
					nConvPassLen = strlen(szConvPass);
					bGotConvPass = TRUE;
				}
				
				GetPreferredAlgorithm(&prefAlg);

				err = PGPBuildOptionList(context, &tempOptions,
						options,
						PGPOConventionalEncrypt(context,
							PGPOPassphraseBuffer(context,
								szConvPass, 
								nConvPassLen),
							PGPOLastOption(context)),
						PGPOCipherAlgorithm(context, prefAlg),
						PGPOLastOption(context));

				if (IsPGPError(err))
				{
					UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
						err);
					goto EncryptSignBufferError;
				}
			}
			else
			{
				GetAllowedAlgorithms(allowedAlgs, &nNumAlgs);
				recipKeySet = prds->SelectedKeySetRef;

				err = PGPBuildOptionList(context, &tempOptions,
						options,
						PGPOPreferredAlgorithms(context,
							allowedAlgs, nNumAlgs ),
						PGPOEncryptToKeySet(context, recipKeySet),
						PGPOFailBelowValidity(context, kPGPValidity_Invalid),
						PGPOWarnBelowValidity(context, kPGPValidity_Invalid),
						PGPOLastOption(context));

				if (IsPGPError(err))
				{
					UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
						err);
					goto EncryptSignBufferError;
				}
			}
			
			PGPFreeOptionList(options);
			options = tempOptions;
		}
		
		if (bSign)
		{
			if (*pSignOptions == NULL)
			{
				if (bGotPassphrase)
					szPrompt = "Passphrase did not match. Please try again:";
				else
					szPrompt = "Please enter your passphrase:";
				
				err = PGPGetSignCachedPhrase(context, hwnd, 
					szPrompt, bGotPassphrase, &szPassphrase, 
					pubKeySet, &signKey, NULL, NULL, prds->dwFlags);
				
				if (IsPGPError(err))
				{
					if (err != kPGPError_UserAbort)
						UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
							err);
					goto EncryptSignBufferError;
				}
				
				bGotPassphrase = TRUE;
				nPassphraseLen = strlen(szPassphrase);
				
				err = PGPBuildOptionList(context, pSignOptions,
						PGPOSignWithKey(context, 
							signKey, 
							PGPOPassphraseBuffer(context,
								szPassphrase, 
								nPassphraseLen),
							PGPOLastOption(context)),
						PGPOLastOption(context));
				
				if (IsPGPError(err))
				{
					if (err != kPGPError_UserAbort)
						UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
							err);
					goto EncryptSignBufferError;
				}
			}

			err = PGPBuildOptionList(context, &tempOptions,
					options,
					PGPOClearSign(context, !bEncrypt && !bAttachment),
					PGPOLastOption(context));
			
			if (IsPGPError(err))
			{
				if (err != kPGPError_UserAbort)
					UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
						err);
				goto EncryptSignBufferError;
			}

			PGPFreeOptionList(options);
			options = tempOptions;
		}
		
		if (bEncrypt && !bSign)
			hwndWorking = WorkingDlgProcThread(GetModuleHandle("pgpExch.dll"), NULL,
							"PGP Encrypting", "");
		else if (!bEncrypt && bSign)
			hwndWorking = WorkingDlgProcThread(GetModuleHandle("pgpExch.dll"), NULL,
							"PGP Signing", "");
		else
			hwndWorking = WorkingDlgProcThread(GetModuleHandle("pgpExch.dll"), NULL,
							"PGP Encrypting and Signing", "");

		if (*pSignOptions == NULL)
			PGPBuildOptionList(context, pSignOptions, 
				PGPOLastOption(context));

		err = PGPEncode(context,
				PGPOInputBuffer(context, pFinalInput, dwInSize),
				PGPOAllocatedOutputBuffer(context, 
					ppOutput, 
					INT_MAX, 
					pOutSize),
				PGPOEventHandler(context, EncryptSignEvent, hwndWorking),
				options,
				*pSignOptions,
				PGPOSendNullEvents(context, 100),
				PGPODataIsASCII(context, !bAttachment),
				PGPOArmorOutput(context, bEncrypt || bAttachment),
				PGPOVersionString(context, pgpVersionHeaderString),
				PGPOAskUserForEntropy(context, TRUE),
				PGPOLastOption(context));

		DestroyWindow(hwndWorking);

		if (options != NULL)
		{
			PGPFreeOptionList(options);
			options = NULL;
		}

		if (err == kPGPError_BadPassphrase)
		{
			PGPFreeOptionList(*pSignOptions);
			*pSignOptions = NULL;
		}
	}

	if (IsPGPError(err) && (err != kPGPError_UserAbort))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto EncryptSignBufferError;
	}

EncryptSignBufferError:

	if (bInputWrapped)
		free(pFinalInput);

	if (szPassphrase != NULL)
	{
		PGPFreePhrase(szPassphrase);
		szPassphrase = NULL;
	}

	if (szConvPass != NULL)
	{
		PGPcomdlgFreePhrase(szConvPass);
		szConvPass = NULL;
	}

	if (options != NULL)
		PGPFreeOptionList(options);

/*	
	if (prds->OriginalKeySetRef == NULL)
		PGPFreeKeySet(pubKeySet);
*/

	return err;
}


PGPError EncryptSignFile(HWND hwnd, PGPContextRef context, char *szInFile, 
						 RECIPIENTDIALOGSTRUCT *prds, 
						 PGPOptionListRef *pSignOptions, char **pszOutFile, 
						 BOOL bEncrypt, BOOL bSign, BOOL bUseMIME)
{
	PGPError			err				= kPGPError_NoErr;
	PGPKeyRef			signKey			= NULL;
	PGPKeySetRef		pubKeySet		= NULL;
	PGPKeySetRef		recipKeySet		= NULL;
	PGPOptionListRef	options			= NULL;
	PGPOptionListRef	tempOptions		= NULL;
	PGPFileSpecRef		inputFile		= NULL;
	PGPFileSpecRef		outputFile		= NULL;
	PGPCipherAlgorithm  prefAlg			= kPGPCipherAlgorithm_CAST5;
	PGPCipherAlgorithm	allowedAlgs[3];
	char *				szPassphrase	= NULL;
	char *				szConvPass		= NULL;
	char *				szPrompt		= NULL;
	int					nPassphraseLen	= 0;
	int					nConvPassLen	= 0;
	int					nNumAlgs		= 0;
	BOOL				bGotPassphrase	= FALSE;
	BOOL				bGotConvPass	= FALSE;

	pgpAssert(szInFile != NULL);
	pgpAssert(prds != NULL);
	pgpAssert(pSignOptions != NULL);
	pgpAssert(*pszOutFile != NULL);
	pgpAssert(PGPRefIsValid(context));

	UpdateWindow(hwnd);

	// Check for demo expiration

	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
		return kPGPError_UserAbort;

	if (prds->OriginalKeySetRef == NULL)
	{
		err = PGPsdkLoadDefaultPrefs(context);
		if (IsPGPError(err))
		{
			UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
			goto EncryptSignFileError;
		}

		err = PGPOpenDefaultKeyRings(context, (PGPKeyRingOpenFlags) 0, 
				&pubKeySet);
		prds->OriginalKeySetRef = pubKeySet;
	}
	else
		pubKeySet = prds->OriginalKeySetRef;

	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto EncryptSignFileError;
	}

	err = PGPNewFileSpecFromFullPath(context, szInFile, &inputFile);
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto EncryptSignFileError;
	}

	err = PGPNewFileSpecFromFullPath(context, *pszOutFile, &outputFile);
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto EncryptSignFileError;
	}

	err = PGPGetDefaultPrivateKey(pubKeySet, &signKey);
	if (err == kPGPError_ItemNotFound)
	{
		PGPKeyListRef	pubKeyList = NULL;
		PGPKeyIterRef	pubKeyIter = NULL;
		
		PGPOrderKeySet(pubKeySet, kPGPTrustOrdering, &pubKeyList);
		PGPNewKeyIter(pubKeyList, &pubKeyIter);
		PGPKeyIterNext(pubKeyIter, &signKey);
		PGPFreeKeyIter(pubKeyIter);
		PGPFreeKeyList(pubKeyList);
	}
	else if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto EncryptSignFileError;
	}

	err = kPGPError_BadPassphrase;
	while (err == kPGPError_BadPassphrase)
	{
		if (IsntNull(szPassphrase))
		{
			PGPFreePhrase(szPassphrase);
			szPassphrase = NULL;
		}
		
		if (IsntNull(szConvPass))
		{
			PGPcomdlgFreePhrase(szConvPass);
			szConvPass = NULL;
		}

		err = PGPBuildOptionList(context, &options, 
				PGPOLastOption(context));
		
		if (IsPGPError(err))
		{
			UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
			goto EncryptSignFileError;
		}

		if (bEncrypt)
		{
			if (prds->dwOptions & PGPCOMDLG_PASSONLY)
			{
				if (!bGotConvPass)
				{
					szPrompt = "Enter conventional passphrase:";
					err = PGPcomdlgGetPhrase(context, hwnd, szPrompt, 
						&szConvPass, NULL, NULL, 0, NULL, NULL, 
						PGPCOMDLG_ENCRYPTION);
					if (err == kPGPError_UserAbort)
						goto EncryptSignFileError;
					
					nConvPassLen = strlen(szConvPass);
					bGotConvPass = TRUE;
				}
				
				GetPreferredAlgorithm(&prefAlg);

				err = PGPBuildOptionList(context, &tempOptions, 
						options,
						PGPOConventionalEncrypt(context,
							PGPOPassphraseBuffer(context,
								szConvPass, 
								nConvPassLen),
							PGPOLastOption(context)),
						PGPOCipherAlgorithm(context, prefAlg),
						PGPOLastOption(context));
				
				if (IsPGPError(err))
				{
					UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
						err);
					goto EncryptSignFileError;
				}
			}
			else
			{
				GetAllowedAlgorithms(allowedAlgs, &nNumAlgs);
				recipKeySet = prds->SelectedKeySetRef;

				err = PGPBuildOptionList(context, &tempOptions,
						options,
						PGPOPreferredAlgorithms(context,
							allowedAlgs, nNumAlgs ),
						PGPOEncryptToKeySet(context, recipKeySet),
						PGPOFailBelowValidity(context, kPGPValidity_Invalid),
						PGPOWarnBelowValidity(context, kPGPValidity_Invalid),
						PGPOLastOption(context));
				
				if (IsPGPError(err))
				{
					UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
						err);
					goto EncryptSignFileError;
				}
			}
			
			PGPFreeOptionList(options);
			options = tempOptions;
		}
		
		if (bSign && (*pSignOptions == NULL)) 
		{
			if (bGotPassphrase)
				szPrompt = "Passphrase did not match. Please try again:";
			else
				szPrompt = "Please enter your passphrase:";
			
			err = PGPGetSignCachedPhrase(context, hwnd, 
					szPrompt, bGotPassphrase, &szPassphrase, 
					pubKeySet, &signKey, NULL, NULL, prds->dwFlags);
			
			if (IsPGPError(err))
			{
				if (err != kPGPError_UserAbort)
					UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
						err);
				goto EncryptSignFileError;
			}
			
			bGotPassphrase = TRUE;
			nPassphraseLen = strlen(szPassphrase);
			
			err = PGPBuildOptionList(context, pSignOptions, 
					PGPOSignWithKey(context, 
						signKey, 
						PGPOPassphraseBuffer(context,
							szPassphrase, 
							nPassphraseLen),
						PGPOLastOption(context)),
					PGPOLastOption(context));
				
			if (IsPGPError(err))
			{
				UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
				goto EncryptSignFileError;
			}
		}
		
		if (*pSignOptions == NULL)
			PGPBuildOptionList(context, pSignOptions, 
				PGPOLastOption(context));

		err = PGPEncode(context,
				PGPOInputFile(context, inputFile),
				PGPOOutputFile(context, outputFile),
				PGPOEventHandler(context, EncryptSignEvent, hwnd),
				options,
				*pSignOptions,
				PGPOAskUserForEntropy(context, TRUE),
				PGPOLastOption(context));

		if (options != NULL)
		{
			PGPFreeOptionList(options);
			options = NULL;
		}

		if (err == kPGPError_BadPassphrase)
		{
			PGPFreeOptionList(*pSignOptions);
			*pSignOptions = NULL;
		}
	}

	if (IsPGPError(err) && (err != kPGPError_UserAbort))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto EncryptSignFileError;
	}

EncryptSignFileError:

	if (szPassphrase != NULL)
	{
		PGPFreePhrase(szPassphrase);
		szPassphrase = NULL;
	}

	if (szConvPass != NULL)
	{
		PGPcomdlgFreePhrase(szConvPass);
		szConvPass = NULL;
	}

	if (options != NULL)
		PGPFreeOptionList(options);

/*	
	if (prds->OriginalKeySetRef == NULL)
		PGPFreeKeySet(pubKeySet);
*/

	return err;
}


PGPError EncryptSignEvent(PGPContextRef context, 
						  PGPEvent *event, 
						  PGPUserValue userValue)
{
	HWND			hwnd			= NULL;
	PGPError		err				= kPGPError_NoErr;

	pgpAssert(PGPRefIsValid(context));
	pgpAssert(event != NULL);

	hwnd = (HWND) userValue;

	switch (event->type)
	{
	case kPGPEvent_WarningEvent:
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
			event->data.warningData.warning);
		break;

	case kPGPEvent_EntropyEvent:
		err = PGPcomdlgRandom(context, hwnd, 
				event->data.entropyData.entropyBitsNeeded);
		break;

	case kPGPEvent_NullEvent:
		{	
			PGPEventNullData *d = &event->data.nullData;
			BOOL bCancel;

			bCancel = WorkingCallback (	(HWND)userValue, 
										(unsigned long)d->bytesWritten, 
										(unsigned long)d->bytesTotal) ;

			if(bCancel)
			{
				return kPGPError_UserAbort;
			}
		}
		break;
	}

	return err;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
