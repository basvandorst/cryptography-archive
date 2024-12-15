/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: events.c,v 1.48 1997/10/08 19:04:53 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

static char unknownName[]="Unknown.txt";

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

	switch( event->type ) 
	{
	case kPGPEvent_NullEvent:
		{	PGPEventNullData *d = &event->data.nullData;
			BOOL CancelOperation;

			CancelOperation=WorkingCallback (s->hwndWorking , 
				(unsigned long)d->bytesWritten, (unsigned long)d->bytesTotal) ;

			if(CancelOperation)
				return kPGPError_UserAbort;
		}
		break;
	case kPGPEvent_ErrorEvent:
		{	PGPEventErrorData *d = &event->data.errorData;

			if((d->error!=kPGPError_BadPassphrase)&&
				(d->error!=kPGPError_UserAbort))
				PGPcomdlgErrorMessage (d->error); 
		}
		break;
	case kPGPEvent_WarningEvent:
		{	PGPEventWarningData *d = &event->data.warningData;

		}
		break;
	case kPGPEvent_EntropyEvent:
		{	PGPEventEntropyData *d = &event->data.entropyData;

			PGPcomdlgRandom(context, s->hwnd, 
				d->entropyBitsNeeded);
		}
		break;
	case kPGPEvent_PassphraseEvent:
		{
			PGPEventPassphraseData *d = &event->data.passphraseData;
			unsigned long passlen;
			BOOL UserCancel;
			char DecryptPrompt[40];

			if(d->fConventional)
				strcpy(DecryptPrompt," decryption :");
			else
				strcpy(DecryptPrompt," your private key :");

			UserCancel=GetDecryptPhrase(context,s->hwnd, 
				&(s->PassPhrase),
				&passlen,
				&(s->PassCount),
				DecryptPrompt,
				s->RecipientKeySet,
				s->RecipientKeyIDArray,
				s->dwKeyIDCount);

			ProcessWorkingDlg(s->hwndWorking);
			
			if(UserCancel)
				return kPGPError_UserAbort;

			PGPAddJobOptions( event->job, 
				PGPOPassphraseBuffer(context,
					s->PassPhrase, passlen ),
				PGPOLastOption(context));
		}
		break;
	case kPGPEvent_SignatureEvent:
		{	PGPEventSignatureData *d = &event->data.signatureData;

			SigEvent(d,s->fileName);
		}
		break;
	case kPGPEvent_RecipientsEvent:
		{
			PGPEventRecipientsData *d = &event->data.recipientsData;
			PGPUInt32	numKeys;
			unsigned int i,memamt;
			
			(void)PGPCountKeys(d->recipientSet, &numKeys );
		
			s->RecipientKeySet=0;

			if( numKeys > 0 ) 
			{
				PGPIncKeySetRefCount(d->recipientSet);
				s->RecipientKeySet=d->recipientSet;
			}

			if(d->keyCount>0)
			{
				s->dwKeyIDCount=d->keyCount;
				memamt=s->dwKeyIDCount*sizeof(PGPKeyID);
				s->RecipientKeyIDArray=(PGPKeyID *)malloc(memamt);
				memset(s->RecipientKeyIDArray,0x00,memamt);

				for(i=0;i<s->dwKeyIDCount;i++)
				{
					memcpy(&(s->RecipientKeyIDArray[i]),
						&(d->keyIDArray[i]),
						sizeof(PGPKeyID));

//					PGPCopyKeyID(d->keyIDArray[i],
//						&(s->RecipientKeyIDArray[i]));
				}
			}
		}
		break;
	case kPGPEvent_AnalyzeEvent:
		{	PGPEventAnalyzeData *d = &event->data.analyzeData;

			if(d->sectionType==kPGPAnalyze_Unknown)
				return kPGPError_SkipSection;
			s->FoundPGPData=TRUE;
		}
		break;
	case kPGPEvent_DetachedSignatureEvent:
		{
			int UserCancel;

			UserCancel=GetOriginalFileRef(s->hwnd,context,
				s->fileName,&(s->fileRef),s->hwndWorking);

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
				Force=TRUE;
			}

			strcpy(inname,s->fileName);

			AlterDecryptedFileName(inname,suggestedName);

			UserCancel=SaveOutputFile(context,
				s->hwnd, 
				"Enter output filename",
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
