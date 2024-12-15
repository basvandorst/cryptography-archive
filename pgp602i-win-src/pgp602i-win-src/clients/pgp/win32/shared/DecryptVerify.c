/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: DecryptVerify.c,v 1.9.2.5.2.1 1998/11/12 03:14:09 heller Exp $
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
#include "pgpSC.h"

// Shared Headers 
#include "PGPcl.h"
#include "SigEvent.h"
#include "Working.h"
#include "Prefs.h"
#include "DecryptVerify.h"
#include "VerificationBlock.h"
#include "WorkingResource.h"
#include "SharedStrings.h"

typedef struct _VerificationBlock VerificationBlock;

struct _VerificationBlock
{
	VerificationBlock *		next;
	VerificationBlock *		previous;
	char *					szBlockBegin;
	char *					szBlockEnd;
	void *					pOutput;
	PGPSize					outSize;
	PGPBoolean				bEncrypted;
	PGPBoolean				FYEO;
};

typedef struct _DecodeEventData
{
	HINSTANCE			hInst;
	HWND				hwnd;
	HWND				hwndWorking;
	PGPtlsContextRef	tlsContext;
	char *				szName;
	PGPKeySetRef		pubKeySet;
	PGPKeySetRef		recipients;
	PGPUInt32			keyCount;
	PGPKeyID			*keyIDArray;
	VerificationBlock *	pVerBlock;
} DecodeEventData;

static PGPError DecryptVerify(HINSTANCE hInst, HWND hwnd, 
							  PGPContextRef context, 
							  PGPtlsContextRef tlsContext, 
							  char *szName, char *szModule,
							  PGPOptionListRef options, BOOL bMIME,
							  VerificationBlock *pVerBlock);

PGPError DecodeEventHandler(PGPContextRef context, 
							PGPEvent *event, 
							PGPUserValue userValue);

static void DisplayErrorCode(char *szFile, 
							 int nLine, 
							 char *szModule, 
							 int nCode);

static char * GetNextLine(char *szStart,
						  char *szOutput,
						  size_t nOutputSize);


PGPError DecryptVerifyBuffer(HINSTANCE hInst, HWND hwnd, 
							 PGPContextRef context, 
							 PGPtlsContextRef tlsContext, 
							 char *szName, char *szModule,
							 void *pInput, DWORD dwInSize, 
							 BOOL bMIME, void **ppOutput, PGPSize *pOutSize,
							 BOOL *FYEO)
{
	PGPError			err			= kPGPError_NoErr;
	PGPOptionListRef	options		= NULL;
	VerificationBlock *	pVerBlock	= NULL;
	VerificationBlock * pTempBlock	= NULL;
	PGPMemoryMgrRef		memoryMgr	= NULL;
	VerificationBlock * pVerIndex	= NULL;
	OUTBUFFLIST	      * obl			= NULL;
	OUTBUFFLIST       * nobl		= NULL;

	pgpAssert(pInput != NULL);
	pgpAssert(ppOutput != NULL);
	pgpAssert(pOutSize != NULL);
	pgpAssert(szName != NULL);
	pgpAssert(szModule != NULL);
	pgpAssert(PGPRefIsValid(context));
	pgpAssert(PGPRefIsValid(tlsContext));

	memoryMgr = PGPGetContextMemoryMgr(context);

	err = PGPBuildOptionList(context, &options, 
			PGPOInputBuffer(context, pInput, dwInSize),
			PGPOLastOption(context));

	if (IsPGPError(err))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto DecryptVerifyBufferError;
	}

	pVerBlock = 
		(VerificationBlock *) PGPNewData(memoryMgr,
									sizeof(VerificationBlock),
									kPGPMemoryMgrFlags_Clear);

	pVerBlock->next = NULL;
	pVerBlock->szBlockBegin = NULL;
	pVerBlock->szBlockEnd = NULL;
	pVerBlock->pOutput = NULL;
	pVerBlock->outSize = 0;

	err = DecryptVerify(hInst, hwnd, context, tlsContext, szName, szModule, 
			options, bMIME, pVerBlock);

	*ppOutput = NULL;
	*pOutSize = 0;

	pVerIndex=pVerBlock;

	// Convert pVerBlock to OUTBUFFLIST
	do
	{
		if (pVerIndex->szBlockBegin != NULL)
		{
			nobl=MakeOutBuffItem(&obl);
			nobl->pBuff=pVerIndex->szBlockBegin;
			nobl->dwBuffSize=strlen(pVerIndex->szBlockBegin);
		}

		if (pVerIndex->pOutput != NULL)
		{
			nobl=MakeOutBuffItem(&obl);
			nobl->pBuff=pVerIndex->pOutput;
			nobl->dwBuffSize=strlen(pVerIndex->pOutput);
			nobl->FYEO=pVerIndex->FYEO;
		}

		if (pVerIndex->szBlockEnd != NULL)
		{
			nobl=MakeOutBuffItem(&obl);
			nobl->pBuff=pVerIndex->szBlockEnd;
			nobl->dwBuffSize=strlen(pVerIndex->szBlockEnd);
		}

		pTempBlock = pVerIndex;
		pVerIndex=pVerIndex->next;
		PGPFreeData(pTempBlock);
	}
	while (pVerIndex != NULL);

	// Concatinate them to ppOutput
	ConcatOutBuffList((void *)context,
		obl,
		(char **)ppOutput,
		pOutSize,
		FYEO);
/*
	do
	{
		if (pVerBlock->szBlockBegin != NULL)
		{
			PGPReallocData(memoryMgr, ppOutput, *pOutSize + 
				strlen(pVerBlock->szBlockBegin) + 
				strlen((char *) pVerBlock->pOutput) + 
				strlen(pVerBlock->szBlockEnd) + 1,
				kPGPMemoryMgrFlags_None);

			((char *) *ppOutput)[*pOutSize] = 0;
			strcat((char *) *ppOutput, pVerBlock->szBlockBegin);
			strcat((char *) *ppOutput, (char *) pVerBlock->pOutput);
			strcat((char *) *ppOutput, pVerBlock->szBlockEnd);
			*pOutSize = strlen((char *) *ppOutput) + 1;

			PGPFreeData(pVerBlock->szBlockBegin);
			PGPFreeData(pVerBlock->pOutput);
			PGPFreeData(pVerBlock->szBlockEnd);
		}
		else if (pVerBlock->pOutput != NULL)
		{
			PGPReallocData(memoryMgr, ppOutput, *pOutSize + 
				strlen((char *) pVerBlock->pOutput) + 1,
				kPGPMemoryMgrFlags_None);

			((char *) *ppOutput)[*pOutSize] = 0;
			strcat((char *) *ppOutput, (char *) pVerBlock->pOutput);
			*pOutSize = strlen((char *) *ppOutput) + 1;

			PGPFreeData(pVerBlock->pOutput);
		}
		
		pTempBlock = pVerBlock;
		pVerBlock = pTempBlock->next;
		PGPFreeData(pTempBlock);
	}
	while (pVerBlock != NULL);
*/
DecryptVerifyBufferError:

	if (options != NULL)
		PGPFreeOptionList(options);

	return err;
}


PGPError DecryptVerifyFile(HINSTANCE hInst, HWND hwnd, PGPContextRef context, 
						   PGPtlsContextRef tlsContext, 
						   char *szName, char *szModule,
						   char *szInFile, BOOL bMIME, BOOL bBinary,
						   char **pszOutFile, void **ppOutput, 
						   PGPSize *pOutSize, BOOL *FYEO)
{
	PGPError			err			= kPGPError_NoErr;
	PGPOptionListRef	options		= NULL;
	PGPFileSpecRef		inputFile	= NULL;
	PGPFileSpecRef		outputFile	= NULL;
	PGPFileSpecRef		finalFile	= NULL;
	PGPFileSpecRef		dataFile	= NULL;
	PGPUInt32			macCreator	= 0;
	PGPUInt32			macType		= 0;
	VerificationBlock *	pVerBlock	= NULL;
	VerificationBlock * pTempBlock	= NULL;
	char *				szExtension = NULL;
	char *				szOutFile	= NULL;
	OUTBUFFLIST	      * obl			= NULL;
	OUTBUFFLIST       * nobl		= NULL;

	pgpAssert(szInFile != NULL);
	pgpAssert(pszOutFile != NULL);
	pgpAssert(szName != NULL);
	pgpAssert(szModule != NULL);
	pgpAssert(PGPRefIsValid(context));
	pgpAssert(PGPRefIsValid(tlsContext));

	if (ppOutput != NULL)
		*ppOutput = NULL;

	if (pOutSize != NULL)
		*pOutSize = 0;

	err = PGPNewFileSpecFromFullPath(context, szInFile, &inputFile);
	if (IsPGPError(err))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto DecryptVerifyFileError;
	}

	szOutFile = (char *) calloc(sizeof(char), strlen(szInFile)+5);
	if (szOutFile == NULL)
	{
		err = kPGPError_OutOfMemory;
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto DecryptVerifyFileError;
	}

	strcpy(szOutFile, szInFile);
 	szExtension = strrchr(szOutFile, '.');
	if (szExtension != NULL)
	{
		if (!strcmp(szExtension, ".asc") || !strcmp(szExtension, ".pgp"))
			*szExtension = '\0';
		else
			strcat(szOutFile, ".tmp");
	}
	else
		strcat(szOutFile, ".tmp");

	if (bBinary)
	{
		err = PGPNewFileSpecFromFullPath(context, szOutFile, &outputFile);
		if (IsPGPError(err))
		{
			DisplayErrorCode(__FILE__, __LINE__, szModule, err);
			goto DecryptVerifyFileError;
		}
		
		err = PGPBuildOptionList(context, &options, 
				PGPOInputFile(context, inputFile),
				PGPOOutputFile(context, outputFile),
				PGPOLastOption(context));
	}
	else
	{
		pVerBlock = 
			(VerificationBlock *) PGPNewData(PGPGetContextMemoryMgr(context),
									sizeof(VerificationBlock),
									kPGPMemoryMgrFlags_Clear);

		pVerBlock->next = NULL;
		pVerBlock->szBlockBegin = NULL;
		pVerBlock->szBlockEnd = NULL;
		pVerBlock->pOutput = NULL;
		pVerBlock->outSize = 0;

		err = PGPBuildOptionList(context, &options, 
				PGPOInputFile(context, inputFile),
				PGPOLastOption(context));
	}

		
	if (IsPGPError(err))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto DecryptVerifyFileError;
	}

	err = DecryptVerify(hInst, hwnd, context, tlsContext, szName, szModule, 
			options, bMIME, pVerBlock);

	if (!bBinary)
	{
		HANDLE hFile;
		DWORD dwWritten;
		BOOL bFixed = FALSE;

		hFile = CreateFile(szOutFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL, NULL);

		if (pVerBlock->FYEO)
		{
			// Convert pVerBlock to OUTBUFFLIST
			do
			{
				if (pVerBlock->szBlockBegin != NULL)
				{
					nobl=MakeOutBuffItem(&obl);
					nobl->pBuff=pVerBlock->szBlockBegin;
					nobl->dwBuffSize=strlen(pVerBlock->szBlockBegin);
				}
				
				if (pVerBlock->pOutput != NULL)
				{
					nobl=MakeOutBuffItem(&obl);
					nobl->pBuff=pVerBlock->pOutput;
					nobl->dwBuffSize=strlen(pVerBlock->pOutput);
					nobl->FYEO=pVerBlock->FYEO;
				}
				
				if (pVerBlock->szBlockEnd != NULL)
				{
					nobl=MakeOutBuffItem(&obl);
					nobl->pBuff=pVerBlock->szBlockEnd;
					nobl->dwBuffSize=strlen(pVerBlock->szBlockEnd);
				}
				
				pTempBlock = pVerBlock;
				pVerBlock=pVerBlock->next;
				PGPFreeData(pTempBlock);
			}
			while (pVerBlock != NULL);
			
			// Concatinate them to ppOutput
			ConcatOutBuffList((void *)context,
				obl,
				(char **)ppOutput,
				pOutSize,
				FYEO);
		}
		else do
		{
			// Fix for Eudora 4.0.1 bug where "boundary=..." is on
			// a separate line

			if ((pVerBlock->pOutput != NULL) && !bFixed && bMIME)
			{
				char szLine[256];
				char szLastLine[256];
				char *szLinePtr;
				char *szLastLinePtr;
				
				szLine[0] = 0;
				szLastLine[0] = 0;
				
				szLinePtr = (char *) pVerBlock->pOutput;
				while (szLinePtr != NULL)
				{
					strcpy(szLastLine, szLine);
					szLastLinePtr = szLinePtr;
					szLinePtr = GetNextLine(szLinePtr, szLine, 
									sizeof(szLine));
					if ((strstr(szLine, "boundary=") != NULL) &&
						(strstr(szLastLine, "Content-Type: multipart/mixed;")
						!= NULL))
					{
						szLastLinePtr -= 2;
						szLastLinePtr[0] = ' ';
						szLastLinePtr[1] = ' ';
					}
				}
				
				bFixed = TRUE;
			}

			if (pVerBlock->szBlockBegin != NULL)
			{
				if (!bMIME)
				{
					WriteFile(hFile, pVerBlock->szBlockBegin, 
						strlen(pVerBlock->szBlockBegin), &dwWritten, 
						NULL);
						
					WriteFile(hFile, pVerBlock->pOutput, 
						strlen((char *) pVerBlock->pOutput), &dwWritten, 
						NULL);
					
					WriteFile(hFile, pVerBlock->szBlockEnd, 
						strlen(pVerBlock->szBlockEnd), &dwWritten, NULL);
					
					PGPFreeData(pVerBlock->szBlockBegin);
					PGPFreeData(pVerBlock->pOutput);
					PGPFreeData(pVerBlock->szBlockEnd);
				}
				else
				{
					char szLine[256];
					char szLastLine[256];
					char szBoundary[256];
					char szContentType[256];
					char *szLinePtr;
					char *szFinalBoundary = NULL;
					char *szFirstText = NULL;
					DWORD dwBegin;
					DWORD dwEnd;
					
					szLine[0] = 0;
					szLastLine[0] = 0;
					szBoundary[0] = 0;
					
					szLinePtr = (char *) pVerBlock->pOutput;
					while ((szLinePtr != NULL) && (szBoundary[0] == 0))
					{
						strcpy(szLastLine, szLine);
						szLinePtr = GetNextLine(szLinePtr, szLine, 
										sizeof(szLine));
						if (strstr(szLine, "Content-Type: text") 
								!= NULL)
						{
							strcpy(szBoundary, szLastLine);
							strcpy(szContentType, szLine);
							if (szFirstText == NULL)
								szFirstText = GetNextLine(szLinePtr, NULL, 0);
						}
					}
					
					while (szLinePtr != NULL)
					{
						GetNextLine(szLinePtr, szLine, sizeof(szLine));
						if (strstr(szLine, szBoundary))
							szFinalBoundary = szLinePtr;
						szLinePtr = GetNextLine(szLinePtr, NULL, 0);
					}
					
					dwBegin = szFirstText - ((char *) pVerBlock->pOutput);
					if (szFinalBoundary != NULL)
						dwEnd = szFinalBoundary - 
								((char *) pVerBlock->pOutput);
					else
						dwEnd = strlen((char *) pVerBlock->pOutput);
					
					WriteFile(hFile, pVerBlock->pOutput, dwBegin, &dwWritten, 
						NULL);

					if (strstr(szContentType, "text/html"))
					{
						char szTemp[] = "<html><pre>\r\n";

						WriteFile(hFile, szTemp, strlen(szTemp), 
							&dwWritten, NULL);
					}

					WriteFile(hFile, pVerBlock->szBlockBegin, 
						strlen(pVerBlock->szBlockBegin), &dwWritten, NULL);

					if (strstr(szContentType, "text/html"))
					{
						char szTemp[] = "</html>\r\n";

						WriteFile(hFile, szTemp, strlen(szTemp), 
							&dwWritten, NULL);
					}

					WriteFile(hFile, ((char *) pVerBlock->pOutput) + dwBegin,
						dwEnd - dwBegin, &dwWritten, NULL);
					
					if (szFinalBoundary != NULL)
					{
						WriteFile(hFile, szBoundary, strlen(szBoundary),
							&dwWritten, NULL);
						WriteFile(hFile, "\r\n", 2, &dwWritten, NULL);
						WriteFile(hFile, szContentType, strlen(szContentType),
							&dwWritten, NULL);
						WriteFile(hFile, "\r\n", 2, &dwWritten, NULL);
					}

					if (strstr(szContentType, "text/html"))
					{
						char szTemp[] = "<html><pre>\r\n";

						WriteFile(hFile, szTemp, strlen(szTemp), 
							&dwWritten, NULL);
					}

					WriteFile(hFile, pVerBlock->szBlockEnd, 
						strlen(pVerBlock->szBlockEnd), &dwWritten, NULL);

					if (strstr(szContentType, "text/html"))
					{
						char szTemp[] = "</html>\r\n";

						WriteFile(hFile, szTemp, strlen(szTemp), 
							&dwWritten, NULL);
					}

					WriteFile(hFile, ((char *) pVerBlock->pOutput) + dwEnd,
						strlen((char *) pVerBlock->pOutput) - dwEnd, 
						&dwWritten, NULL);
					
					PGPFreeData(pVerBlock->szBlockBegin);
					PGPFreeData(pVerBlock->pOutput);
					PGPFreeData(pVerBlock->szBlockEnd);
				}
			}
			else if (pVerBlock->pOutput != NULL)
			{
				WriteFile(hFile, pVerBlock->pOutput, 
					strlen((char *) pVerBlock->pOutput), &dwWritten, 
					NULL);
				
				PGPFreeData(pVerBlock->pOutput);
			}
				
			pTempBlock = pVerBlock;
			pVerBlock = pTempBlock->next;
			PGPFreeData(pTempBlock);
		}
		while (pVerBlock != NULL);
			
		CloseHandle(hFile);

		*pszOutFile = (char *) PGPNewData(PGPGetContextMemoryMgr(context),
									strlen(szOutFile)+1,
									kPGPMemoryMgrFlags_Clear);
		strcpy(*pszOutFile, szOutFile);
	}
	else
	{
		PGPMacBinaryToLocal(outputFile, &finalFile, &macCreator, &macType);
		if (finalFile != NULL)
			PGPGetFullPathFromFileSpec(finalFile, pszOutFile);
		else
		{
			*pszOutFile = (char *) PGPNewData(PGPGetContextMemoryMgr(context),
										strlen(szOutFile)+1,
										kPGPMemoryMgrFlags_Clear);
			strcpy(*pszOutFile, szOutFile);
		}
	}

DecryptVerifyFileError:

	if (finalFile != NULL)
		PGPFreeFileSpec(finalFile);

	if (inputFile != NULL)
		PGPFreeFileSpec(inputFile);

	if (outputFile != NULL)
		PGPFreeFileSpec(outputFile);

	if (options != NULL)
		PGPFreeOptionList(options);

	return err;
}


PGPError DecryptVerify(HINSTANCE hInst, HWND hwnd, PGPContextRef context, 
					   PGPtlsContextRef tlsContext, 
					   char *szName, char *szModule,
					   PGPOptionListRef options, BOOL bMIME,
					   VerificationBlock *pVerBlock)
{
	PGPError			err			= kPGPError_NoErr;
	PGPKeySetRef		pubKeySet	= NULL;
	PGPKeySetRef		newKeySet	= NULL;
	PGPOptionListRef	tempOptions	= NULL;
	PGPUInt32			nNumKeys	= 0;
	HWND				hwndWorking	= NULL;
	char				szWorkingTitle[256];
	DecodeEventData		decodeData;

	UpdateWindow(hwnd);

	decodeData.hInst		= hInst;
	decodeData.hwnd			= hwnd;
	decodeData.tlsContext	= tlsContext;
	decodeData.szName		= szName;
	decodeData.recipients	= NULL;
	decodeData.keyCount		= 0;
	decodeData.keyIDArray	= NULL;
	decodeData.pVerBlock	= pVerBlock;

	err = PGPsdkLoadDefaultPrefs(context);
	if (IsPGPError(err))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto DecryptVerifyError;
	}

	err = PGPOpenDefaultKeyRings(context, (PGPKeyRingOpenFlags)0, &pubKeySet);
	if (IsPGPError(err))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto DecryptVerifyError;
	}

	decodeData.pubKeySet = pubKeySet;

	PGPNewKeySet(context, &newKeySet);

	LoadString(hInst, IDS_WORKINGDECRYPT, szWorkingTitle, 
		sizeof(szWorkingTitle));

	hwndWorking = WorkingDlgProcThread(GetModuleHandle(szModule), hInst, NULL,
					szWorkingTitle, "");
	decodeData.hwndWorking = hwndWorking;

	err = PGPDecode(context,
			options,
			PGPOPassThroughIfUnrecognized(context, (PGPBoolean) !bMIME),
			PGPOPassThroughKeys(context, TRUE),
			PGPOEventHandler(context, DecodeEventHandler, &decodeData),
			PGPOSendNullEvents(context, 100),
			PGPOImportKeysTo(context, newKeySet),
			PGPOKeySetRef(context, pubKeySet),
			PGPOLastOption(context));

	DestroyWindow(hwndWorking);

	if (IsPGPError(err) && (err != kPGPError_UserAbort))
	{
		DisplayErrorCode(__FILE__, __LINE__, szModule, err);
		goto DecryptVerifyError;
	}

	PGPCountKeys(newKeySet, &nNumKeys);
	if (nNumKeys > 0)
		PGPclQueryAddKeys(context, tlsContext, hwnd, newKeySet, NULL);

DecryptVerifyError:

	if (decodeData.recipients != NULL)
		PGPFreeKeySet(decodeData.recipients);

	if (decodeData.keyIDArray != NULL)
		free(decodeData.keyIDArray);

	if (newKeySet != NULL)
		PGPFreeKeySet(newKeySet);

	if (pubKeySet != NULL)
		PGPFreeKeySet(pubKeySet);

	return err;
}


PGPError DecodeEventHandler(PGPContextRef context, 
							PGPEvent *event, 
							PGPUserValue userValue)
{
	HWND			hwnd			= NULL;
	char *			szPassPhrase	= NULL;
	PGPByte *		pPasskey		= NULL;
	PGPUInt32		nPasskeyLength	= 0;
	static BOOL		bAlreadyAsked	= FALSE;
	char *			szName			= NULL;
	PGPMemoryMgrRef	memoryMgr		= NULL;
	PGPKeySetRef	pubKeySet		= NULL;
	PGPKeySetRef	addedKeys		= NULL;
	PGPKeySetRef	recipients		= NULL;
	PGPUInt32		keyCount		= 0;
	PGPKeyID		*keyIDArray		= NULL;
	DecodeEventData	*userData		= NULL;
	HWND			hwndWorking		= NULL;
	PGPError		err				= kPGPError_NoErr;

	pgpAssert(PGPRefIsValid(context));
	pgpAssert(event != NULL);

	userData = (DecodeEventData *) userValue;

	hwnd			= userData->hwnd;
	hwndWorking		= userData->hwndWorking;
	szName			= userData->szName;
	pubKeySet		= userData->pubKeySet;
	recipients		= userData->recipients;
	keyCount		= userData->keyCount;
	keyIDArray		= userData->keyIDArray;

	memoryMgr = PGPGetContextMemoryMgr(context);

	switch (event->type)
	{
	case kPGPEvent_NullEvent:
		{	
			PGPEventNullData *d = &event->data.nullData;
			BOOL bCancel;

			bCancel = WorkingCallback (	hwndWorking, 
										(unsigned long)d->bytesWritten, 
										(unsigned long)d->bytesTotal) ;

			if(bCancel)
			{
				return kPGPError_UserAbort;
			}
		}
		break;

	case kPGPEvent_BeginLexEvent:
		if (userData->pVerBlock != NULL)
		{
			userData->pVerBlock->bEncrypted = FALSE;
		}
		break;

	case kPGPEvent_OutputEvent:
		if (userData->pVerBlock != NULL)
		{
			PGPEventOutputData *d = &event->data.outputData;

			// Added FYEO member for tempest viewer
			userData->pVerBlock->FYEO=d->forYourEyesOnly;

			PGPAddJobOptions(event->job,
				PGPOAllocatedOutputBuffer(context, 
					&(userData->pVerBlock->pOutput), 
					INT_MAX, 
					&(userData->pVerBlock->outSize)),
				PGPOLastOption(context));

			userData->pVerBlock->next = (VerificationBlock *) 
				PGPNewData(memoryMgr,
					sizeof(VerificationBlock),
					kPGPMemoryMgrFlags_Clear);

			userData->pVerBlock->next->previous = userData->pVerBlock;
			userData->pVerBlock = userData->pVerBlock->next;
		}
		break;

	case kPGPEvent_RecipientsEvent:
		{
			PGPEventRecipientsData	*eventData; 

			eventData = &(event->data.recipientsData);

			PGPIncKeySetRefCount(eventData->recipientSet);
			userData->recipients = eventData->recipientSet;
			userData->keyCount = eventData->keyCount;

			if (eventData->keyCount > 0)
			{
				UINT i;

				userData->keyIDArray =	(PGPKeyID *) 
										calloc(sizeof(PGPKeyID),
												eventData->keyCount);

				for (i=0; i<eventData->keyCount; i++)
					userData->keyIDArray[i] = eventData->keyIDArray[i];
			}
			else
				userData->keyIDArray = NULL;
		}
		break;

	case kPGPEvent_PassphraseEvent:
		{
			char szPrompt[256];
			PGPEventPassphraseData *d = &event->data.passphraseData;

			if (bAlreadyAsked)
				LoadString(userData->hInst, IDS_PASSPHRASEREENTER, szPrompt, 
					sizeof(szPrompt));
			else
				LoadString(userData->hInst, IDS_PASSPHRASEPROMPT, szPrompt, 
					sizeof(szPrompt));

			// Don't cache conventional passphrases
			if(d->fConventional)
			{
				err=PGPclGetPhrase (context,
					pubKeySet,
					hwnd,
					szPrompt,
					&szPassPhrase, 
					NULL,
					NULL, 
					0,
					NULL,
					NULL,
					PGPCL_DECRYPTION,
					NULL,NULL,
					1,0,userData->tlsContext,NULL);
			}
			else
			{
				err = PGPclGetCachedDecryptionPhrase(context, 
						userData->tlsContext, pubKeySet, hwnd, szPrompt, 
						bAlreadyAsked, &szPassPhrase, recipients, keyIDArray,
						keyCount, &pPasskey, &nPasskeyLength,  &addedKeys);
			}
			
			if (addedKeys != NULL)
			{
				PGPUInt32 numKeys;
				
				PGPCountKeys(addedKeys, &numKeys);
				if (numKeys > 0)
					PGPclQueryAddKeys(context, userData->tlsContext, hwnd, 
						addedKeys, NULL);
				
				PGPFreeKeySet(addedKeys);
				addedKeys = NULL;
			}
			
			switch (err)
			{
			case kPGPError_NoErr:
				bAlreadyAsked = TRUE;
				break;
				
			default:
				return err;
			}
			
			if (IsntNull(szPassPhrase))
			{
				err = PGPAddJobOptions(event->job,
						PGPOPassphrase(context, szPassPhrase),
						PGPOLastOption(context));
				
				PGPclFreeCachedPhrase(szPassPhrase);
				szPassPhrase = NULL;
			}
			else if (IsntNull(pPasskey))
			{
				err = PGPAddJobOptions(event->job, 
						PGPOPasskeyBuffer(context, pPasskey, nPasskeyLength),
						PGPOLastOption(context));
			}
			
			if (IsntNull(pPasskey))
			{
				PGPFreeData(pPasskey);
				pPasskey = NULL;
				nPasskeyLength = 0;
			}
		}
		break;

	case kPGPEvent_AnalyzeEvent:
		if (userData->pVerBlock != NULL)
		{
			if (event->data.analyzeData.sectionType == kPGPAnalyze_Encrypted)
				userData->pVerBlock->bEncrypted = TRUE;
		}
		break;

	case kPGPEvent_SignatureEvent:
		{
			PGPEventSignatureData *d = &event->data.signatureData;

			if (IsNull(d->signingKey) && 
				SyncOnVerify(memoryMgr))
			{
				PGPBoolean bGotKeys;

				PGPclLookupUnknownSigner(context, pubKeySet, 
					userData->tlsContext, hwnd, event, d->signingKeyID, 
					&bGotKeys);

				if (bGotKeys)
					return kPGPError_NoErr;
			}

			if (userData->pVerBlock == NULL)
				SigEvent(hwnd, context, d, szName);
			else
				CreateVerificationBlock(userData->hInst, context, d,
					userData->pVerBlock->previous->bEncrypted,
					&(userData->pVerBlock->previous->szBlockBegin),
					&(userData->pVerBlock->previous->szBlockEnd));
		}
		break;

	case kPGPEvent_FinalEvent:
		if (IsntNull(szPassPhrase))
		{
			PGPclFreeCachedPhrase(szPassPhrase);
			szPassPhrase = NULL;
		}
		if (IsntNull(pPasskey))
		{
			PGPFreeData(pPasskey);
			pPasskey = NULL;
			nPasskeyLength = 0;
		}
		bAlreadyAsked = FALSE;
		break;
	}

	return err;
}


void DisplayErrorCode(char *szFile, int nLine, char *szModule, int nCode)
{
	char szErrorMsg[255];

	if (nCode == kPGPError_BadPacket)
		nCode = kPGPError_CorruptData;

	PGPclEncDecErrorToString(nCode, szErrorMsg, 254);

#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
	_CrtDbgReport(_CRT_ERROR, szFile, nLine, szModule, szErrorMsg);
#endif

	MessageBox(NULL, szErrorMsg, szModule, MB_ICONEXCLAMATION);
	return;
}


static char * GetNextLine(char *szStart,
						  char *szOutput,
						  size_t nOutputSize)
{
	char *szEnd = NULL;
	BOOL bStop = FALSE;

	if (szStart == NULL)
		return NULL;

	szEnd = strchr(szStart, '\n');
	if (szEnd == NULL)
	{
		szEnd = strchr(szStart, '\0');
		bStop = TRUE;
	}

	if ((szOutput != NULL) && (nOutputSize > 0))
	{
		size_t nLineSize;
		size_t nBuffSize;

		nLineSize = szEnd - szStart;
		if (nLineSize == 0)
			szOutput[0] = 0;
		else
		{
			if (nLineSize > nOutputSize)
				nBuffSize = nOutputSize;
			else
				nBuffSize = nLineSize;

			strncpy(szOutput, szStart, nBuffSize);
			szOutput[nBuffSize-1] = 0;
		}
	}

	if (bStop)
		return NULL;
	else
		return szEnd+1;
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
