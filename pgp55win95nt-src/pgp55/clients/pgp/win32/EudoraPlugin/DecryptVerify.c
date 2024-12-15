/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: DecryptVerify.c,v 1.22 1997/10/22 23:05:48 elrod Exp $
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
#include "pgpSDKPrefs.h"


// Shared Headers 
#include "PGPcmdlg.h"
#include "PGPphras.h"
#include "PGPkm.h"
#include "SigEvent.h"

// Project Headers
#include "MyPrefs.h" 
#include "DisplayMessage.h"
#include "resource.h"

// Global Variables
extern PGPContextRef	g_pgpContext;
extern HWND				g_hwndEudoraMainWindow;


PGPError DecodeEventHandler(PGPContextRef context, 
							PGPEvent *event, 
							PGPUserValue userValue);

typedef struct _DecodeEventData
{
	HWND			hwnd;
	PGPKeySetRef	pubKeySet;
	BOOL			bAutoAddKeys;
	PGPAnalyzeType	sectionType;
	PGPKeySetRef	recipients;
	PGPUInt32		keyCount;
	PGPKeyID		*keyIDArray;
} DecodeEventData;

PGPError 
DecryptVerify(	HWND				hwndParent,		// Parent Window
				PGPOptionListRef	pgpOptions		// Encoding Options
)
{
	PGPError		error		= kPGPError_NoErr;	
	PGPKeySetRef	keyset		= kPGPInvalidRef;	
	PGPKeySetRef	addKeySet	= kPGPInvalidRef;	
	DecodeEventData	decodeData;

	assert(pgpOptions);

	// refresh the prefs
	PGPsdkLoadDefaultPrefs(g_pgpContext);

	// Check for expiration
	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
	{
		return kPGPError_Win32_Expired;
	}
 
	error = PGPOpenDefaultKeyRings(g_pgpContext, 0, &keyset);

	if( IsntPGPError( error ) )
	{
		PGPUInt32	numKeys;

		error = PGPNewKeySet( g_pgpContext, &addKeySet);

		if(IsntPGPError(error))
		{
			memset(&decodeData, 0x00, sizeof(decodeData));

			decodeData.hwnd			= hwndParent;
			decodeData.pubKeySet	= keyset;
			decodeData.bAutoAddKeys = FALSE;
			decodeData.recipients	= kPGPInvalidRef;
			decodeData.sectionType	= kPGPAnalyze_Unknown;

			error = PGPDecode(g_pgpContext,
				pgpOptions,
				PGPOEventHandler(g_pgpContext, 
					DecodeEventHandler, 
					&decodeData),
				PGPOKeySetRef(g_pgpContext, keyset),
				PGPOImportKeysTo(g_pgpContext,addKeySet),
				PGPOLastOption(g_pgpContext));

			if( IsntPGPError( error ) ) 
			{
				error = PGPCountKeys( addKeySet, &numKeys);
			
				// make sure there are keys in this Set
				if( IsntPGPError( error ) && numKeys > 0 ) 
				{
					PGPkmQueryAddKeys(	g_pgpContext, 
										g_hwndEudoraMainWindow, 
										addKeySet, 
										NULL);	
				}
			}

			PGPFreeKeySet (addKeySet);
		}

		PGPFreeKeySet(keyset);
	}

	return error;
}

PGPError DecodeEventHandler(PGPContextRef context, 
							PGPEvent *event, 
							PGPUserValue userValue)
{
	HWND			hwnd			= NULL;
	char*			szPassPhrase	= NULL;
	static BOOL		bAlreadyAsked	= FALSE;
	char *			szPrompt		= "Please enter your passphrase:";
	PGPKeySetRef	pubKeySet		= kPGPInvalidRef;
	PGPKeySetRef	encryptKeySet	= kPGPInvalidRef;
	PGPKeySetRef	recipients		= kPGPInvalidRef;
	PGPKeyID		*keyIDArray		= kPGPInvalidRef;
	PGPUInt32		keyCount		= 0;
	BOOL			bAutoAddKeys	= FALSE;
	DecodeEventData	*userData		= NULL;
	PGPError		err				= kPGPError_NoErr;

	assert(PGPRefIsValid(context));
	assert(event != NULL);

	userData		= (DecodeEventData *) userValue;

	hwnd			= userData->hwnd;
	pubKeySet		= userData->pubKeySet;
	bAutoAddKeys	= userData->bAutoAddKeys;
	recipients		= userData->recipients;
	keyCount		= userData->keyCount;
	keyIDArray		= userData->keyIDArray;


	switch (event->type)
	{
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
			{
				userData->recipients = NULL;
			}

			userData->keyCount = eventData->keyCount;

			if (eventData->keyCount > 0)
			{
				UINT i = 0;

				userData->keyIDArray =	(PGPKeyID*) 
										calloc(sizeof(PGPKeyID),
												eventData->keyCount);

				for (i=0; i<eventData->keyCount; i++)
				{
					userData->keyIDArray[i] = eventData->keyIDArray[i];
				}
			}
			else
			{
				userData->keyIDArray = NULL;
			}

			break;
		}

		case kPGPEvent_PassphraseEvent:
		{
			if (bAlreadyAsked)
			{
				szPrompt = "Passphrase did not match. Please try again:";
			}

			if(!event->data.passphraseData.fConventional)
			{
				encryptKeySet = event->data.passphraseData.keyset;
			}

			err = PGPGetCachedPhrase(	context, 
										hwnd, szPrompt, 
										bAlreadyAsked, 
										&szPassPhrase, 
										recipients,
										keyIDArray,
										keyCount);

			if( IsntPGPError (err) )
			{
				bAlreadyAsked = TRUE;

				err = PGPAddJobOptions(event->job,
					PGPOPassphrase(context, szPassPhrase),
					PGPOLastOption(context));
			}

			if (szPassPhrase)
			{
				PGPFreePhrase(szPassPhrase);
				szPassPhrase = NULL;
			}

			break;
		}

		case kPGPEvent_AnalyzeEvent:
		{
			if (event->data.analyzeData.sectionType != kPGPAnalyze_Unknown)
			{
				userData->sectionType = 
				event->data.analyzeData.sectionType;
			}
			break;
		}

		case kPGPEvent_SignatureEvent:
		{
			SigEvent(	&event->data.signatureData,
						"Eudora Plugin");

			break;
		}

/*
		case kPGPEvent_KeyFoundEvent:
		{
			if (TRUE)
			{
				PGPError err;

				err = PGPkmQueryAddKeys(context, g_hwndEudoraMainWindow, 
						event->data.keyFoundData.keySet, NULL);
			}

			break;
		}
*/

		case kPGPEvent_FinalEvent:
		{
			bAlreadyAsked = FALSE;

			if( PGPRefIsValid(userData->recipients) )
			{
				PGPFreeKeySet(userData->recipients);
			}

			if (userData->keyIDArray != NULL)
			{
				free(userData->keyIDArray);
			}

			break;
		}
	}

	return err;
}
