/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ExchDecryptVerify.cpp,v 1.28 1997/10/08 20:07:58 dgal Exp $
____________________________________________________________________________*/
#include "stdinc.h"
#include <time.h>
#include "pgpExch.h"
#include "resource.h"
#include "Working.h"
#include "UIutils.h"

#include "pgpConfig.h"
#include "pgpEncode.h"
#include "pgpMem.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "pgpSDKPrefs.h"
#include "PGPcmdlg.h"
#include "PGPphras.h"
#include "PGPRecip.h"
#include "PGPkm.h"
#include "SigEvent.h"

typedef struct _DecodeEventData
{
	HWND			hwnd;
	HWND			hwndWorking;
	PGPKeySetRef	pubKeySet;
	BOOL			bAutoAddKeys;
	PGPAnalyzeType	sectionType;
	PGPKeySetRef	recipients;
	PGPUInt32		keyCount;
	PGPKeyID		*keyIDArray;
} DecodeEventData;

PGPError DecryptVerifyEvent(PGPContextRef context, 
							PGPEvent *event, 
							PGPUserValue userValue);


PGPError DecryptVerifyBuffer(HWND hwnd,	PGPContextRef context, void *pInput,
							 DWORD dwInSize, void *pAttachment, 
							 DWORD dwAttachmentSize, BOOL bAutoAddKeys,
							 void **ppOutput, PGPSize *pOutSize)
{
	PGPError			err			= kPGPError_NoErr;
	PGPKeySetRef		pubKeySet	= NULL;
	PGPKeySetRef		newKeySet	= NULL;
	PGPOptionListRef	options		= NULL;
	PGPOptionListRef	tempOptions	= NULL;
	PGPUInt32			nNumKeys	= 0;
	HWND				hwndWorking	= NULL;
	DecodeEventData		decodeData;

	pgpAssert(pInput != NULL);
	pgpAssert(ppOutput != NULL);
	pgpAssert(pOutSize != NULL);
	pgpAssert(PGPRefIsValid(context));

	UpdateWindow(hwnd);

	// Garbage characters are 16 and 39 (0x10 and 0x27)

	err = PGPBuildOptionList(context, &options, PGPOLastOption(context));
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto DecryptVerifyBufferError;
	}
	
	err = PGPsdkLoadDefaultPrefs(context);
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto DecryptVerifyBufferError;
	}

	err = PGPOpenDefaultKeyRings(context, (PGPKeyRingOpenFlags)0, &pubKeySet);
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto DecryptVerifyBufferError;
	}
	
	decodeData.hwnd			= hwnd;
	decodeData.pubKeySet	= pubKeySet;
	decodeData.bAutoAddKeys = bAutoAddKeys;
	decodeData.sectionType	= kPGPAnalyze_Unknown;
	decodeData.recipients	= NULL;
	decodeData.keyCount		= 0;
	decodeData.keyIDArray	= NULL;

	if ((pAttachment != NULL) && (dwAttachmentSize > 0))
	{
		err = PGPBuildOptionList(context, &tempOptions,
				options,
				PGPODetachedSig(context,
					PGPOInputBuffer(context, 
						pAttachment, 
						dwAttachmentSize),
					PGPOLastOption(context)),
				PGPOLastOption(context));
		
		if (IsPGPError(err))
		{
			UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
			goto DecryptVerifyBufferError;
		}
	
		PGPFreeOptionList(options);
		options = tempOptions;
	}

	PGPNewKeySet(context, &newKeySet);

	hwndWorking = WorkingDlgProcThread(GetModuleHandle("pgpExch.dll"), NULL,
					"Decoding data", "");
	decodeData.hwndWorking = hwndWorking;

	err = PGPDecode(context,
			PGPOInputBuffer(context, pInput, dwInSize),
			PGPOAllocatedOutputBuffer(context, ppOutput, INT_MAX, pOutSize),
			PGPOEventHandler(context, DecryptVerifyEvent, &decodeData),
			options,
			PGPOSendNullEvents(context, 100),
			PGPOImportKeysTo(context, newKeySet),
			PGPOKeySetRef(context, pubKeySet),
			PGPOLastOption(context));

	DestroyWindow(hwndWorking);

	if (IsPGPError(err) && (err != kPGPError_UserAbort))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto DecryptVerifyBufferError;
	}

	PGPCountKeys(newKeySet, &nNumKeys);
	if (nNumKeys > 0)
		PGPkmQueryAddKeys(context, hwnd, newKeySet, NULL);

	if (*ppOutput != NULL)
		((char *) *ppOutput)[*pOutSize] = 0;

DecryptVerifyBufferError:

	if (decodeData.recipients != NULL)
		PGPFreeKeySet(decodeData.recipients);

	if (decodeData.keyIDArray != NULL)
		free(decodeData.keyIDArray);

	if (options != NULL)
		PGPFreeOptionList(options);

	if (newKeySet != NULL)
		PGPFreeKeySet(newKeySet);

	if (pubKeySet != NULL)
		PGPFreeKeySet(pubKeySet);

	return err;
}


PGPError DecryptVerifyFile(HWND hwnd, PGPContextRef context, char *szInFile, 
						   char *szDataFile, BOOL bAutoAddKeys, 
						   char **pszOutFile)
{
	PGPError			err			= kPGPError_NoErr;
	PGPKeySetRef		pubKeySet	= NULL;
	PGPKeySetRef		newKeySet	= NULL;
	PGPOptionListRef	options		= NULL;
	PGPOptionListRef	tempOptions	= NULL;
	DecodeEventData		decodeData;
	PGPFileSpecRef		inputFile	= NULL;
	PGPFileSpecRef		outputFile	= NULL;
	PGPFileSpecRef		finalFile	= NULL;
	PGPFileSpecRef		dataFile	= NULL;
	PGPUInt32			nNumKeys	= 0;
	PGPUInt32			macCreator	= 0;
	PGPUInt32			macType		= 0;

	pgpAssert(szInFile != NULL);
	pgpAssert(*pszOutFile != NULL);
	pgpAssert(PGPRefIsValid(context));

	UpdateWindow(hwnd);

	err = PGPNewFileSpecFromFullPath(context, szInFile, &inputFile);
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto DecryptVerifyFileError;
	}

	if (szDataFile != NULL)
	{
		err = PGPNewFileSpecFromFullPath(context, szDataFile, &dataFile);
		if (IsPGPError(err))
		{
			UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
			goto DecryptVerifyFileError;
		}
	}

	err = PGPBuildOptionList(context, &options, PGPOLastOption(context));
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto DecryptVerifyFileError;
	}

	err = PGPsdkLoadDefaultPrefs(context);
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto DecryptVerifyFileError;
	}

	err = PGPOpenDefaultKeyRings(context, (PGPKeyRingOpenFlags)0, &pubKeySet);
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto DecryptVerifyFileError;
	}
	
	decodeData.hwnd			= hwnd;
	decodeData.pubKeySet	= pubKeySet;
	decodeData.bAutoAddKeys = bAutoAddKeys;
	decodeData.sectionType	= kPGPAnalyze_Unknown;
	decodeData.recipients	= NULL;
	decodeData.keyCount		= 0;
	decodeData.keyIDArray	= NULL;
	
	PGPNewKeySet(context, &newKeySet);

	err = PGPDecode(context,
			PGPOInputFile(context, inputFile),
			PGPOOutputFile(context, outputFile),
			PGPOEventHandler(context, DecryptVerifyEvent, &decodeData),
			PGPODetachedSig(context,
				PGPOInputFile(context, dataFile),
				PGPOLastOption(context)),
			PGPOImportKeysTo(context, newKeySet),
			PGPOKeySetRef(context, pubKeySet),
			PGPOLastOption(context));

	PGPCountKeys(newKeySet, &nNumKeys);
	if (nNumKeys > 0)
		PGPkmQueryAddKeys(context, hwnd, newKeySet, NULL);

	if (IsPGPError(err) && (err != kPGPError_UserAbort))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto DecryptVerifyFileError;
	}
	else
	{
		PGPMacBinaryToLocal(outputFile, &finalFile, &macCreator, &macType);
		PGPGetFullPathFromFileSpec(finalFile, pszOutFile);
	}

DecryptVerifyFileError:

	if (decodeData.recipients != NULL)
		PGPFreeKeySet(decodeData.recipients);

	if (decodeData.keyIDArray != NULL)
		free(decodeData.keyIDArray);

	if (options != NULL)
		PGPFreeOptionList(options);

	if (newKeySet != NULL)
		PGPFreeKeySet(newKeySet);

	if (pubKeySet != NULL)
		PGPFreeKeySet(pubKeySet);
	
	return err;
}


PGPError DecryptVerifyEvent(PGPContextRef context, 
							PGPEvent *event, 
							PGPUserValue userValue)
{
	HWND			hwnd			= NULL;
	char *			szPassPhrase	= NULL;
	static BOOL		bAlreadyAsked	= FALSE;
	char *			szPrompt		= NULL;
	PGPKeySetRef	pubKeySet		= NULL;
	PGPKeySetRef	recipients		= NULL;
	BOOL			bAutoAddKeys	= FALSE;
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
	pubKeySet		= userData->pubKeySet;
	bAutoAddKeys	= userData->bAutoAddKeys;
	recipients		= userData->recipients;
	keyCount		= userData->keyCount;
	keyIDArray		= userData->keyIDArray;

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

	case kPGPEvent_RecipientsEvent:
		{
			PGPUInt32				numKeys;
			PGPEventRecipientsData	*eventData; 

			eventData = &(event->data.recipientsData);

			PGPCountKeys(eventData->recipientSet, &numKeys);
			if (numKeys > 0)
			{
				PGPIncKeySetRefCount(eventData->recipientSet);
				userData->recipients = eventData->recipientSet;
			}
			else
				userData->recipients = NULL;

			userData->keyCount = eventData->keyCount;
			if (eventData->keyCount > 0)
			{
				userData->keyIDArray =	(PGPKeyID *) 
										calloc(sizeof(PGPKeyID),
												eventData->keyCount);

				for (UINT i=0; i<eventData->keyCount; i++)
					userData->keyIDArray[i] = eventData->keyIDArray[i];
			}
			else
				userData->keyIDArray = NULL;
		}
		break;

	case kPGPEvent_PassphraseEvent:
		if (bAlreadyAsked)
			szPrompt = "Passphrase did not match. Please try again:";
		else
			szPrompt = "Please enter your passphrase:";

		err = PGPGetCachedPhrase(context, hwnd, szPrompt, bAlreadyAsked, 
				&szPassPhrase, recipients, keyIDArray, keyCount);

		switch (err)
		{
		case kPGPError_NoErr:
			bAlreadyAsked = TRUE;
			break;

		default:
			return err;
		}

		err = PGPAddJobOptions(event->job,
				PGPOPassphrase(context, szPassPhrase),
				PGPOLastOption(context));

		if (IsntNull(szPassPhrase))
		{
			PGPFreePhrase(szPassPhrase);
			szPassPhrase = NULL;
		}

		break;

	case kPGPEvent_AnalyzeEvent:
		if (event->data.analyzeData.sectionType != kPGPAnalyze_Unknown)
			((DecodeEventData *) userValue)->sectionType = 
				event->data.analyzeData.sectionType;
		break;

	case kPGPEvent_SignatureEvent:
		{
			PGPEventSignatureData *d = &event->data.signatureData;
			char szName[255];
			char szWarning[255];
			char szTitle[255];

			UIGetString(szTitle, 254, IDS_TITLE);

			if (d->keyRevoked)
			{
				UIGetString(szWarning, 254, IDS_SIGKEYREVOKED);
				MessageBox(hwnd, szWarning, szTitle, MB_ICONEXCLAMATION);
			}
			else if (d->keyDisabled)
			{
				UIGetString(szWarning, 254, IDS_SIGKEYDISABLED);
				MessageBox(hwnd, szWarning, szTitle, MB_ICONEXCLAMATION);
			}
			else if (d->keyExpired)
			{
				UIGetString(szWarning, 254, IDS_SIGKEYEXPIRED);
				MessageBox(hwnd, szWarning, szTitle, MB_ICONEXCLAMATION);
			}

			UIGetString(szName, 254, IDS_LOGNAME);
		    SigEvent(d, szName);
		}
		break;

	case kPGPEvent_FinalEvent:
		if (IsntNull(szPassPhrase))
		{
			PGPFreePhrase(szPassPhrase);
			szPassPhrase = NULL;
		}
		bAlreadyAsked = FALSE;
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
