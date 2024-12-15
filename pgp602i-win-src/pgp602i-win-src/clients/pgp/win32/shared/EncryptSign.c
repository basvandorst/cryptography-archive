/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: EncryptSign.c,v 1.8.2.2.2.1 1998/11/12 03:14:10 heller Exp $
____________________________________________________________________________*/
// System Headers
#include <windows.h> 
#include <windowsx.h>
#include <assert.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpWerr.h"
#include "pgpUtilities.h"
#include "pgpMem.h"
#include "pgpSDKPrefs.h"

// Shared Headers
#include "pgpVersionHeader.h"
#include "PGPcl.h"
#include "BlockUtils.h"
#include "Working.h"
#include "Prefs.h"
#include "EncryptSign.h"
#include "WorkingResource.h"
#include "SharedStrings.h"

static int nProgressCount = 0;

static PGPError EncryptSign(HINSTANCE hInst, HWND hwnd, PGPContextRef context, 
							PGPtlsContextRef tlsContext, 
							char *szName, char *szModule,
							RECIPIENTDIALOGSTRUCT *prds,
							PGPOptionListRef ioOptions,
							PGPOptionListRef mimeOptions,
							PGPOptionListRef *pSignOptions,
							BOOL bEncrypt, BOOL bSign, BOOL bBinary);

PGPError EncodeEventHandler(PGPContextRef context, 
							PGPEvent *event, 
							PGPUserValue userValue);

static void DisplayErrorCode(char *szFile, 
							 int nLine, 
							 char *szModule, 
							 int nCode);

BOOL WrapBuffer(char **pszOutText,
				char *szInText,
				PGPUInt16 wrapLength)
{
	BOOL RetVal = FALSE;
	PGPError err;
	char *cmdlgBuffer;

	err=PGPclWrapBuffer(
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
		PGPclFreeWrapBuffer(cmdlgBuffer);
		RetVal = TRUE;
	}

	return RetVal;
}


PGPError EncryptSignBuffer(HINSTANCE hInst, HWND hwnd, PGPContextRef context, 
						   PGPtlsContextRef tlsContext, 
						   char *szName, char *szModule,
						   void *pInput, DWORD dwInSize, 
						   RECIPIENTDIALOGSTRUCT *prds,
						   PGPOptionListRef mimeOptions,
						   PGPOptionListRef *pSignOptions,
						   void **ppOutput, PGPSize *pOutSize, BOOL bEncrypt, 
						   BOOL bSign, BOOL bBinary)
{
	PGPError			err				= kPGPError_NoErr;
	PGPMemoryMgrRef		memoryMgr		= NULL;
	PGPOptionListRef	options			= NULL;
	void *				pFinalInput		= NULL;
	long				lWrapWidth		= 0;
	BOOL				bInputWrapped	= FALSE;

	pgpAssert(pInput != NULL);
	pgpAssert(prds != NULL);
	pgpAssert(prds->OriginalKeySetRef != NULL);
	pgpAssert(pSignOptions != NULL);
	pgpAssert(ppOutput != NULL);
	pgpAssert(pOutSize != NULL);
	pgpAssert(PGPRefIsValid(context));

	memoryMgr = PGPGetContextMemoryMgr(context);
	pFinalInput = pInput;

	if (!bBinary)
	{
		if (ByDefaultWordWrap(memoryMgr, &lWrapWidth))
		{
			pFinalInput = NULL;
			bInputWrapped = WrapBuffer((char **) &pFinalInput, 
								(char *) pInput, (short) lWrapWidth);
			dwInSize = strlen((char *) pFinalInput);
		}
	}

	err = PGPBuildOptionList(context, &options, 
			PGPOInputBuffer(context, pFinalInput, dwInSize),
			PGPOAllocatedOutputBuffer(context, 
				ppOutput, 
				INT_MAX, 
				pOutSize),
			PGPOLastOption(context));

	if (IsPGPError(err))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto EncryptSignBufferError;
	}

	err = EncryptSign(hInst, hwnd, context, tlsContext, szName, szModule, 
			prds, options, mimeOptions, pSignOptions, bEncrypt, bSign, 
			bBinary);

EncryptSignBufferError:

	if (bInputWrapped)
		free(pFinalInput);

	if (options != NULL)
		PGPFreeOptionList(options);

	return err;
}


PGPError EncryptSignFile(HINSTANCE hInst, HWND hwnd, PGPContextRef context, 
						 PGPtlsContextRef tlsContext, 
						 char *szName, char *szModule, char *szInFile, 
						 RECIPIENTDIALOGSTRUCT *prds, 
						 PGPOptionListRef mimeOptions,
						 PGPOptionListRef *pSignOptions, char *szOutFile, 
						 BOOL bEncrypt, BOOL bSign, BOOL bBinary)
{
	PGPError			err				= kPGPError_NoErr;
	PGPOptionListRef	options			= NULL;
	PGPFileSpecRef		inputFile		= NULL;
	PGPFileSpecRef		outputFile		= NULL;

	pgpAssert(szInFile != NULL);
	pgpAssert(prds != NULL);
	pgpAssert(prds->OriginalKeySetRef != NULL);
	pgpAssert(pSignOptions != NULL);
	pgpAssert(szOutFile != NULL);
	pgpAssert(PGPRefIsValid(context));

	err = PGPNewFileSpecFromFullPath(context, szInFile, &inputFile);
	if (IsPGPError(err))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto EncryptSignFileError;
	}

	err = PGPNewFileSpecFromFullPath(context, szOutFile, &outputFile);
	if (IsPGPError(err))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto EncryptSignFileError;
	}

	err = PGPBuildOptionList(context, &options, 
			PGPOInputFile(context, inputFile),
			PGPOOutputFile(context, outputFile),
			PGPOLastOption(context));

	if (IsPGPError(err))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto EncryptSignFileError;
	}

	err = EncryptSign(hInst, hwnd, context, tlsContext, szName, szModule, 
			prds, options, mimeOptions, pSignOptions, bEncrypt, bSign, 
			bBinary);

EncryptSignFileError:

	if (options != NULL)
		PGPFreeOptionList(options);

	if (inputFile != NULL)
		PGPFreeFileSpec(inputFile);

	if (outputFile != NULL)
		PGPFreeFileSpec(outputFile);

	return err;
}


PGPError EncryptSign(HINSTANCE hInst, HWND hwnd, PGPContextRef context, 
					 PGPtlsContextRef tlsContext, 
					 char *szName, char *szModule,
					 RECIPIENTDIALOGSTRUCT *prds,
					 PGPOptionListRef ioOptions,
					 PGPOptionListRef mimeOptions,
					 PGPOptionListRef *pSignOptions,
					 BOOL bEncrypt, BOOL bSign, 
					 BOOL bBinary)
{
	PGPError			err				= kPGPError_NoErr;
	PGPMemoryMgrRef		memoryMgr		= NULL;
	PGPKeyRef			signKey			= NULL;
	PGPKeySetRef		pubKeySet		= NULL;
	PGPKeySetRef		addedKeys		= NULL;
	PGPKeySetRef		recipKeySet		= NULL;
	PGPOptionListRef	options			= NULL;
	PGPOptionListRef	tempOptions		= NULL;
	PGPCipherAlgorithm  prefAlg			= kPGPCipherAlgorithm_CAST5;
	PGPCipherAlgorithm	allowedAlgs[3];
	char *				szPassphrase	= NULL;
	char *				szConvPass		= NULL;
	PGPByte *			pPasskey		= NULL;
	PGPUInt32			nPasskeyLength	= 0;
	int					nPassphraseLen	= 0;
	int					nConvPassLen	= 0;
	int					nNumAlgs		= 0;
	BOOL				bGotPassphrase	= FALSE;
	BOOL				bGotConvPass	= FALSE;
	HWND				hwndWorking		= NULL;
	char 				szComment[256];
	char 				szWorkingTitle[256];

	UpdateWindow(hwnd);
	memoryMgr = PGPGetContextMemoryMgr(context);

	// Check for demo expiration

	if (PGPclEvalExpired(hwnd, PGPCL_ENCRYPTSIGNEXPIRED) != kPGPError_NoErr)
		return kPGPError_UserAbort;

	pubKeySet = prds->OriginalKeySetRef;
	
	err = PGPGetDefaultPrivateKey(pubKeySet, &signKey);
	if (IsPGPError(err))
	{
		PGPKeyListRef	pubKeyList = NULL;
		PGPKeyIterRef	pubKeyIter = NULL;
		
		PGPOrderKeySet(pubKeySet, kPGPTrustOrdering, &pubKeyList);
		PGPNewKeyIter(pubKeyList, &pubKeyIter);
		PGPKeyIterNext(pubKeyIter, &signKey);
		PGPFreeKeyIter(pubKeyIter);
		PGPFreeKeyList(pubKeyList);

		err = kPGPError_NoErr;
	}

	err = kPGPError_BadPassphrase;
	while (err == kPGPError_BadPassphrase)
	{
		if (IsntNull(szPassphrase))
		{
			PGPclFreeCachedPhrase(szPassphrase);
			szPassphrase = NULL;
		}

		if (IsntNull(szConvPass))
		{
			PGPclFreePhrase(szConvPass);
			szConvPass = NULL;
		}

		if (IsNull(mimeOptions))
			err = PGPBuildOptionList(context, &options, 
					PGPOLastOption(context));
		else
			err = PGPBuildOptionList(context, &options, 
					mimeOptions,
					PGPOLastOption(context));

		if (IsPGPError(err))
		{
			DisplayErrorCode(__FILE__, __LINE__, szModule, err);
			goto EncryptSignError;
		}

		if (GetCommentString(memoryMgr, szComment, 254))
		{
			err = PGPBuildOptionList(context, &tempOptions,
					options,
					PGPOCommentString(context, szComment),
					PGPOLastOption(context));
			
			if (IsPGPError(err))
			{
				DisplayErrorCode(__FILE__, __LINE__, szModule, err);
				goto EncryptSignError;
			}

			PGPFreeOptionList(options);
			options = tempOptions;
		}

		if (bEncrypt)
		{
			if (prds->dwOptions & PGPCL_ASCIIARMOR)
			{
				err = PGPBuildOptionList(context, &tempOptions,
						PGPOArmorOutput(context, TRUE),
						options,
						PGPOLastOption(context));

				if (IsPGPError(err))
				{
					DisplayErrorCode(__FILE__, __LINE__, szModule, 
						err);
					goto EncryptSignError;
				}

				PGPFreeOptionList(options);
				options = tempOptions;
			}

			if (prds->dwOptions & PGPCL_PASSONLY)
			{
				if (!bGotConvPass)
				{
					char szPrompt[256];

					LoadString(hInst, IDS_CONVPASSPHRASE, szPrompt, 
						sizeof(szPrompt));

					err = PGPclGetPhrase(context, pubKeySet, hwnd, szPrompt, 
						&szConvPass, NULL, NULL, 0, NULL, NULL, 
						PGPCL_ENCRYPTION, NULL, NULL,1,0,NULL,NULL);
					
					// wjb changed to 1 for min passphrase length
					if (err == kPGPError_UserAbort)
						goto EncryptSignError;
					
					nConvPassLen = strlen(szConvPass);
					bGotConvPass = TRUE;
				}
				
				GetPreferredAlgorithm(memoryMgr, &prefAlg);

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
					DisplayErrorCode(__FILE__, __LINE__, szModule, 
						err);
					goto EncryptSignError;
				}
			}
			else
			{
				GetAllowedAlgorithms(memoryMgr, allowedAlgs, &nNumAlgs);
				recipKeySet = prds->SelectedKeySetRef;

				err = PGPBuildOptionList(context, &tempOptions,
						options,
						PGPOPreferredAlgorithms(context,
							allowedAlgs, nNumAlgs ),
						PGPOEncryptToKeySet(context, recipKeySet),
						PGPOFailBelowValidity(context, kPGPValidity_Unknown),
						PGPOWarnBelowValidity(context, kPGPValidity_Unknown),
						PGPOLastOption(context));

				if (IsPGPError(err))
				{
					DisplayErrorCode(__FILE__, __LINE__, szModule, 
						err);
					goto EncryptSignError;
				}
			}
			
			PGPFreeOptionList(options);
			options = tempOptions;
		}
		
		if (bSign)
		{
			if (*pSignOptions == NULL)
			{
				char szPrompt[256];

				if (bGotPassphrase)
					LoadString(hInst, IDS_PASSPHRASEREENTER, szPrompt, 
						sizeof(szPrompt));
				else
					LoadString(hInst, IDS_PASSPHRASEPROMPT, szPrompt, 
						sizeof(szPrompt));
				
				err = PGPclGetCachedSigningPhrase(context, tlsContext, hwnd, 
						szPrompt, bGotPassphrase, &szPassphrase, 
						pubKeySet, &signKey, NULL, NULL, prds->dwFlags,
						&pPasskey, &nPasskeyLength, &addedKeys);
				
				if (addedKeys != NULL)
				{
					PGPUInt32 numKeys;

					PGPCountKeys(addedKeys, &numKeys);
					if (numKeys > 0)
						PGPclQueryAddKeys(context, tlsContext, hwnd, 
							addedKeys, NULL);

					PGPFreeKeySet(addedKeys);
					addedKeys = NULL;
				}

				if (IsPGPError(err))
				{
					if (err != kPGPError_UserAbort)
						DisplayErrorCode(__FILE__, __LINE__, szModule, 
							err);
					goto EncryptSignError;
				}
				
				bGotPassphrase = TRUE;
				if (IsntNull(szPassphrase))
				{
					nPassphraseLen = strlen(szPassphrase);
				
					err = PGPBuildOptionList(context, pSignOptions,
							PGPOSignWithKey(context, 
								signKey, 
								PGPOPassphraseBuffer(context,
									szPassphrase, 
									nPassphraseLen),
								PGPOLastOption(context)),
							PGPOLastOption(context));
				}
				else if (IsntNull(pPasskey))
				{
					err = PGPBuildOptionList(context, pSignOptions,
							PGPOSignWithKey(context, 
								signKey, 
								PGPOPasskeyBuffer(context,
									pPasskey, 
									nPasskeyLength),
								PGPOLastOption(context)),
							PGPOLastOption(context));
				}
				
				if (IsPGPError(err))
				{
					if (err != kPGPError_UserAbort)
						DisplayErrorCode(__FILE__, __LINE__, szModule, 
							err);
					goto EncryptSignError;
				}
			}

			err = PGPBuildOptionList(context, &tempOptions,
					options,
					PGPOClearSign(context, 
						(PGPBoolean) (!bEncrypt && !bBinary)),
					PGPOLastOption(context));
			
			if (IsPGPError(err))
			{
				if (err != kPGPError_UserAbort)
					DisplayErrorCode(__FILE__, __LINE__, szModule, 
						err);
				goto EncryptSignError;
			}

			PGPFreeOptionList(options);
			options = tempOptions;
		}
		
		if (bEncrypt && !bSign)
			LoadString(hInst, IDS_WORKINGENCRYPT, szWorkingTitle, 
				sizeof(szWorkingTitle));
		else if (!bEncrypt && bSign)
			LoadString(hInst, IDS_WORKINGSIGN, szWorkingTitle, 
				sizeof(szWorkingTitle));
		else
			LoadString(hInst, IDS_WORKINGENCRYPTSIGN, szWorkingTitle, 
				sizeof(szWorkingTitle));

		hwndWorking = WorkingDlgProcThread(GetModuleHandle(szModule), 
						hInst, NULL, szWorkingTitle, "");

		if (*pSignOptions == NULL)
			PGPBuildOptionList(context, pSignOptions, 
				PGPOLastOption(context));

		err = PGPEncode(context,
				ioOptions,
				PGPOEventHandler(context, EncodeEventHandler, hwndWorking),
				options,
				*pSignOptions,
				PGPOSendNullEvents(context, 100),
				PGPODataIsASCII(context, (PGPBoolean) !bBinary),
				PGPOVersionString(context, pgpVersionHeaderString),
				PGPOAskUserForEntropy(context, TRUE),
				PGPOForYourEyesOnly(context,
					(PGPBoolean)((prds->dwOptions & PGPCL_FYEO)==PGPCL_FYEO)),
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
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto EncryptSignError;
	}

EncryptSignError:

	if (szPassphrase != NULL)
	{
		PGPclFreeCachedPhrase(szPassphrase);
		szPassphrase = NULL;
	}

	if (pPasskey != NULL)
	{
		PGPFreeData(pPasskey);
		pPasskey = NULL;
	}

	if (szConvPass != NULL)
	{
		PGPclFreePhrase(szConvPass);
		szConvPass = NULL;
	}

	if (options != NULL)
		PGPFreeOptionList(options);

	return err;
}


PGPError EncodeEventHandler(PGPContextRef context, 
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
	case kPGPEvent_EntropyEvent:
		err = PGPclRandom(context, hwnd, 
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


void DisplayErrorCode(char *szFile, int nLine, char *szModule, int nCode)
{
	char szErrorMsg[255];

	PGPclEncDecErrorToString(nCode, szErrorMsg, 254);

#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
	_CrtDbgReport(_CRT_ERROR, szFile, nLine, szModule, szErrorMsg);
#endif

	MessageBox(NULL, szErrorMsg, szModule, MB_ICONEXCLAMATION);
	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
