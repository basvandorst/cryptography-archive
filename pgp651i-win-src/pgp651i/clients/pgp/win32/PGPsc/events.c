/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: events.c,v 1.87.4.1 1999/06/03 03:43:07 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include "..\shared\VerificationBlock.h"
#include "..\shared\Prefs.h"
/*
BOOL SyncOnVerify(PGPMemoryMgrRef memoryMgr)
{
	PGPBoolean	bSync		= FALSE;
	PGPPrefRef	prefRef		= NULL;

	PGPclOpenClientPrefs(memoryMgr, &prefRef);
	PGPGetPrefBoolean(prefRef, kPGPPrefKeyServerSyncOnVerify, &bSync);
	PGPclCloseClientPrefs(prefRef, FALSE);

	return (BOOL) bSync;
}
*/
/* Generic event handler */
PGPError
myEvents(
	PGPContextRef context,
	PGPEvent *event,
	PGPUserValue userValue
	)
{
	MYSTATE *s;
	(void) context;

	s = (MYSTATE *)userValue;

	if(SCGetProgressCancel(s->hPrgDlg))
		return kPGPError_UserAbort;

	switch( event->type ) 
	{
	case kPGPEvent_NullEvent:
		{	PGPEventNullData *d = &event->data.nullData;

			if(d->bytesTotal!=0)
			{
				return SCSetProgressBar(s->hPrgDlg,
					(DWORD)(d->bytesWritten*100/d->bytesTotal),FALSE);
			}
		}
		break;
	case kPGPEvent_ErrorEvent:
		{	PGPEventErrorData *d = &event->data.errorData;

			if((d->error!=kPGPError_BadPassphrase)&&
				(d->error!=kPGPError_UserAbort))
				PGPclEncDecErrorBox (s->hwndWorking,d->error); 
		}
		break;
	case kPGPEvent_WarningEvent:
		{	PGPEventWarningData *d = &event->data.warningData;

		}
		break;
	case kPGPEvent_EntropyEvent:
		{	PGPEventEntropyData *d = &event->data.entropyData;
			PGPError err;

			err=PGPclRandom(context, s->hwndWorking, 
				d->entropyBitsNeeded);

			if(err==kPGPError_UserAbort)
				return kPGPError_UserAbort;
		}
		break;
	case kPGPEvent_PassphraseEvent:
		{
			PGPEventPassphraseData *d = &event->data.passphraseData;
			BOOL UserCancel;
			char DecryptPrompt[40];
			char StrRes[500];
			char szPassTitle[100];

			// If multiple passphrase events, clear previous
			if(s->PassPhrase)
			{
				PGPclFreeCachedPhrase (s->PassPhrase);
				s->PassPhrase=NULL;
			}

			if(s->PassKey)
			{
				memset(s->PassKey,0x00,s->PassKeyLen);
				PGPFreeData(s->PassKey);
				s->PassKey=NULL;
				s->PassKeyLen=0;
			}

			// Don't cache conventional passphrases
			if(d->fConventional)
			{
				PGPError PhraseErr;

				if(s->PassCount > 0)
				{
					LoadString(g_hinst, IDS_WRONG_PHRASE, DecryptPrompt, sizeof(DecryptPrompt));
				}
				else 
				{
					LoadString(g_hinst, IDS_ENTER_PHRASE, DecryptPrompt, sizeof(DecryptPrompt));
					LoadString (g_hinst, IDS_DECRYPTIONCOLON, StrRes, sizeof(StrRes));
					lstrcat (DecryptPrompt, StrRes);
				}

				PhraseErr=PGPclGetPhrase (context,
					s->KeySet,
					s->hwndWorking,
					DecryptPrompt,
					&(s->PassPhrase), 
					NULL,
					NULL, 
					0,
					NULL,
					NULL,
					PGPCL_DECRYPTION,
					NULL,NULL,
					0,0,s->tlsContext,NULL,NULL);

				UserCancel=IsPGPError(PhraseErr);

				s->PassCount++;
			}
			else
			// Go through caching otherwise
			{
				LoadString (g_hinst, IDS_ENTERPASSPHRASE, StrRes, sizeof(StrRes));
				strcpy(szPassTitle,s->szAppName);
				strcat(szPassTitle,StrRes);

				LoadString (g_hinst, IDS_PRIVATEKEYCOLON, StrRes, sizeof(StrRes));
				strcpy(DecryptPrompt,StrRes);

				UserCancel=GetDecryptPhrase(context,
					s->KeySet,
					s->hwndWorking, 
					&(s->PassPhrase),
					&(s->PassCount),
					DecryptPrompt,
					s->RecipientKeySet,
					s->RecipientKeyIDArray,
					s->dwKeyIDCount,
					&(s->PassKey),
					&(s->PassKeyLen),
					s->tlsContext,
					&(s->AddedKeys),
					szPassTitle);
			}
			
			if(UserCancel)
				return kPGPError_UserAbort;
	
			// Use passphrase first if available
			if(s->PassPhrase)
			{
				PGPAddJobOptions( event->job, 
					PGPOPassphraseBuffer(context,
						s->PassPhrase, strlen(s->PassPhrase) ),
					PGPOLastOption(context));
			}
			else if(s->PassKey)
			{
				PGPAddJobOptions( event->job, 
					PGPOPasskeyBuffer(context,
						s->PassKey,s->PassKeyLen),
					PGPOLastOption(context));
			}
		}
		break;
	case kPGPEvent_SignatureEvent:
		{	PGPEventSignatureData *d = &event->data.signatureData;
			OUTBUFFLIST *headobl,*footobl,*lastobl,*indexobl,*prevobl;

			if ((d->signingKey == NULL) &&
				SyncOnVerify(PGPGetContextMemoryMgr(context)))
			{
				PGPBoolean bGotKeys;

				PGPclLookupUnknownSigner(context, 
					s->KeySet,s->tlsContext, 
					s->hwndWorking, event, 
					d->signingKeyID, &bGotKeys);

				if (bGotKeys)
					return kPGPError_NoErr;
			}

			if(s->Operation!=MS_DECRYPTCLIPBOARD)
			{
				SigEvent(s->hwndWorking,context,d,(char *)s->verifyName);
			}
			else
			{
				prevobl=lastobl=0;
				indexobl=s->obl;

				while(indexobl!=0)
				{
					prevobl=lastobl;
					lastobl=indexobl;
					indexobl=indexobl->next;
				}

				// Trim off last entry
				if(prevobl==0)
				{
					s->obl=0;
				}
				else
				{
					prevobl->next=0;
				}

				// Create header
				headobl=MakeOutBuffItem(&(s->obl));

				// Add back in entry
				headobl->next=lastobl;

				// Creater footer
				footobl=MakeOutBuffItem(&(s->obl));

				CreateVerificationBlock(g_hinst, context, d,
					s->bVerEncrypted,
					&(headobl->pBuff),
					&(footobl->pBuff));

				headobl->dwBuffSize=strlen(headobl->pBuff);
				footobl->dwBuffSize=strlen(footobl->pBuff);
			}
		}
		break;
	case kPGPEvent_RecipientsEvent:
		{
			PGPEventRecipientsData *d = &event->data.recipientsData;
			PGPUInt32 i,memamt;
			
			// Save recipient key set for passphrase dialog
			PGPIncKeySetRefCount(d->recipientSet);
			s->RecipientKeySet=d->recipientSet;

			// Save unknown keyids
			if(d->keyCount>0)
			{
				s->dwKeyIDCount=d->keyCount;
				memamt=s->dwKeyIDCount*sizeof(PGPKeyID);
				s->RecipientKeyIDArray=(PGPKeyID *)malloc(memamt);
				memset(s->RecipientKeyIDArray,0x00,memamt);

				for(i=0;i<s->dwKeyIDCount;i++)
				{
					s->RecipientKeyIDArray[i]=d->keyIDArray[i];
				}
			}
		}
		break;
	case kPGPEvent_AnalyzeEvent:
		{	PGPEventAnalyzeData *d = &event->data.analyzeData;

			s->bVerEncrypted = (d->sectionType == kPGPAnalyze_Encrypted);

			if(d->sectionType==kPGPAnalyze_Unknown)
			{
				// If its tray, we want to keep it
				if(s->Operation!=MS_DECRYPTCLIPBOARD)
					return kPGPError_SkipSection;
			}
			else
			{
				s->FoundPGPData=TRUE;
			}
		}
		break;
	case kPGPEvent_DetachedSignatureEvent:
		{
			int UserCancel;

			UserCancel=GetOriginalFileRef(s->hwndWorking,context,
				s->fileName,(char *)s->verifyName,
				&(s->fileRef),s->hwndWorking);

			if(UserCancel)
				return kPGPError_UserAbort;

			PGPAddJobOptions(event->job,
				PGPODetachedSig(context,
					PGPOInputFile(context,s->fileRef),
					PGPOLastOption(context)),
				PGPOLastOption(context));
		}
		break;
	case kPGPEvent_OutputEvent:
		{	PGPEventOutputData *d = &event->data.outputData;
			int UserCancel;
			char inname[MAX_PATH];
			char guessName[MAX_PATH];
			char *suggestedName;
			BOOL Force;
			char StrRes[500];
			PGPBoolean FYEO;
			OUTBUFFLIST *nobl;

			FYEO=d->forYourEyesOnly;

			// Since we need an output event to get eyes data,
			// even buffers need to be assigned...
			if((s->Operation==MS_DECRYPTCLIPBOARD)||(FYEO))
			{
				nobl=MakeOutBuffItem(&(s->obl));

				nobl->FYEO=FYEO;

				PGPAddJobOptions(event->job,
					PGPOAllocatedOutputBuffer(context,&(nobl->pBuff), 
						MAX_BUFFER_SIZE ,&(nobl->dwBuffSize)),
					PGPOLastOption(context));

				break;
			}

			if(d->suggestedName!=0)
			{
				suggestedName=d->suggestedName;
				Force=FALSE;
			}
			else 
			{
				char *p;

				strcpy(guessName,JustFile(s->fileName));

				p = strrchr(guessName, '.');

				if(p!=0)
					*p=0;

				suggestedName=guessName;
				Force=FALSE; // Let guess be OK too.
			}

			strcpy(inname,s->fileName);

			AlterDecryptedFileName(inname,suggestedName);

			LoadString (g_hinst, IDS_ENTEROUTPUTFILENAME, StrRes, sizeof(StrRes));

			UserCancel=SaveOutputFile(context,
				s->hwndWorking, 
				StrRes,
				inname,
				&(s->fileRef),
				Force);

			if(UserCancel)
				return kPGPError_UserAbort;

			PGPAddJobOptions(event->job,
				PGPOOutputFile(context,s->fileRef),
				PGPOLastOption(context));
		}
		break;
	case kPGPEvent_BeginLexEvent:
		{	PGPEventBeginLexData *d = &event->data.beginLexData;
			s->sectionCount = d->sectionNumber;
		}
		break;
	case kPGPEvent_EndLexEvent:
		{	PGPEventEndLexData *d = &event->data.endLexData;

		}
		break;
		
	default:
		break;
	}
				
	return kPGPError_NoErr;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
