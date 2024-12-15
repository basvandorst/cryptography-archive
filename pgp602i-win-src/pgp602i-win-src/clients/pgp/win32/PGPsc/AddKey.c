/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: AddKey.c,v 1.41.8.1 1998/11/12 03:13:23 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

PGPError
AddEvents(
	PGPContextRef context,
	PGPEvent *event,
	PGPUserValue userValue
	)
{
	MYSTATE *s;
	(void) context;

	s = (MYSTATE *)userValue;

	if(s->ws.CancelPending)
		return kPGPError_UserAbort;

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
				PGPclEncDecErrorBox(s->hwndWorking,d->error);
		}
		break;
	case kPGPEvent_WarningEvent:
		{	PGPEventWarningData *d = &event->data.warningData;

		}
		break;
	case kPGPEvent_AnalyzeEvent:
		{	PGPEventAnalyzeData *d = &event->data.analyzeData;

			if(d->sectionType!=kPGPAnalyze_Key)
				return kPGPError_SkipSection;

			s->FoundPGPData=TRUE;
		}
		break;
	default:
		break;
	}
				
	return kPGPError_NoErr;
}

PGPError GenericAddKey(MYSTATE *ms,
					PGPOptionListRef opts,
					char *OperationTarget)
{
	PGPKeySetRef AddKeySet;
	PGPContextRef	context;
	char StrRes[500];
	PGPError err;

	err=kPGPError_NoErr;

	context=ms->context;

	ms->fileName=OperationTarget;

	LoadString (g_hinst, IDS_ADDINGFILE, StrRes, sizeof(StrRes));
	sprintf(ms->ws.TitleText,StrRes,JustFile(ms->fileName));
	strcpy(ms->ws.StartText,"");
	LoadString (g_hinst, IDS_FINISHEDREADINGKEYS, ms->ws.EndText, sizeof(ms->ws.EndText));

	SendMessage(ms->hwndWorking, PGPM_PROGRESS_UPDATE,
		(WPARAM)PGPSC_WORK_NEWFILE,0);

	err=PGPImportKeySet(context,&AddKeySet,
		opts,
		PGPOSendNullEvents(context,75),
		PGPOEventHandler(context,AddEvents,ms),
		PGPOLastOption(context));

	WorkingCallback (ms->hwndWorking, 
		ms->ws.total,ms->ws.total);

	if(IsntPGPError(err)) 
	{
		PGPUInt32	numKeys;

		PGPCountKeys( AddKeySet, &numKeys);
		if ( numKeys > 0) 
		{	
			err=PGPclQueryAddKeys (context, 
					ms->tlsContext,ms->hwndWorking,AddKeySet,NULL);
		}
		PGPFreeKeySet (AddKeySet);
	}

	return err;
}

PGPError AddKeyFileListStub (MYSTATE *ms) 
{
	PGPContextRef context;
	PGPFileSpecRef inref;
	PGPOptionListRef opts;
	FILELIST *FileCurrent;
	PGPError err;

	err=kPGPError_NoErr;

	context=ms->context;

	FileCurrent=ms->ListHead;

	while(!(ms->ws.CancelPending)&&(FileCurrent!=0)&&(IsntPGPError(err)))
	{    
		if(FileCurrent->IsDirectory)
		{
			FileCurrent=FileCurrent->next;
			continue;
		}

		PGPNewFileSpecFromFullPath (context, 
			FileCurrent->name, &inref);

		PGPBuildOptionList(context,&opts,
			PGPOInputFile(context,inref),
			PGPOLastOption(context));

		err=GenericAddKey(ms,opts,FileCurrent->name);

		PGPFreeOptionList(opts);

		PGPFreeFileSpec( inref );
	
		FileCurrent=FileCurrent->next;				
	}

	return err;
}

BOOL AddKeyFileList(HWND hwnd,void *PGPsc,void *PGPtls,FILELIST *ListHead)
{
	PGPContextRef context;
	PGPtlsContextRef tls;
	MYSTATE *ms;
	PGPError err;

	err=kPGPError_NoErr;

	context=(PGPContextRef)PGPsc;
	tls=(PGPtlsContextRef)PGPtls;

	if(IsPGPError(PGPclEvalExpired(hwnd, PGPCL_ALLEXPIRED)))
		return FALSE;

	ms=(MYSTATE *)malloc(sizeof(MYSTATE));

	if(ms)
	{
		memset(ms, 0x00, sizeof(MYSTATE) );

		ms->context=context;
		ms->tlsContext=tls;
		ms->ListHead=ListHead;
		ms->Operation=MS_ADDKEYFILELIST;

		if(OpenRings(hwnd,context,&(ms->KeySet)))
		{
			err=DoWork(hwnd,ms);

			if(!(ms->FoundPGPData))
				PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_NOPGPKEYSINFILE,
					MB_OK|MB_ICONEXCLAMATION);	
	
			PGPFreeKeySet(ms->KeySet);
		}
		free(ms);
	}

	FreeFileList(ListHead);

	if(IsPGPError(err))
		return FALSE;

	return TRUE;
}

PGPError AddKeyClipboardStub (MYSTATE *ms) 
{
	PGPOptionListRef opts;
	PGPContextRef context;
	char StrRes[100];
	PGPError err;

	err=kPGPError_NoErr;

	context=ms->context;

	PGPBuildOptionList(context,&opts,
		PGPOInputBuffer(context,ms->pInput,ms->dwInputSize),
		PGPOLastOption(context));

	LoadString (g_hinst, IDS_CLIPBOARD, StrRes, sizeof(StrRes));

	err=GenericAddKey(ms,opts,StrRes);

	PGPFreeOptionList(opts);

	return err;
}

BOOL AddKeyClipboard(HWND hwnd,void *PGPsc,void *PGPtls)
{
	PGPContextRef context;
	char *pInput;
	DWORD dwInputSize;
	UINT ClipboardFormat;
	MYSTATE *ms;
	PGPtlsContextRef tls;
	PGPError err;

	// Check for files copied into clipboard from explorer
	if(OpenClipboard(hwnd)) 
	{
		if(IsClipboardFormatAvailable(CF_HDROP))
		{
			FILELIST *ListHead;
			HDROP hDrop;

			hDrop=(HDROP)GetClipboardData(CF_HDROP);
			ListHead=HDropToFileList(hDrop);

			if(ListHead!=0)
			{
				CloseClipboard();

				return AddKeyFileList(hwnd,
					PGPsc,PGPtls,ListHead);
			}
		}
		CloseClipboard();
	}

	err=kPGPError_NoErr;

	context=(PGPContextRef)PGPsc;
	tls=(PGPtlsContextRef)PGPtls;

	if(IsPGPError(PGPclEvalExpired(hwnd, PGPCL_ALLEXPIRED)))
		return FALSE;

	pInput=RetrieveClipboardData(hwnd, &ClipboardFormat, 
			                     &dwInputSize);

	if(!pInput)
	{
		PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_NOCLIPBOARDCONTENTS,
					MB_OK|MB_ICONSTOP);
	}
	else
	{
		if((ClipboardFormat != CF_TEXT)||(*pInput==0))
		{
			PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_NOCLIPBOARDTEXT,
				MB_OK|MB_ICONSTOP);
		}
		else
		{
			ms=(MYSTATE *)malloc(sizeof(MYSTATE));

			if(ms)
			{
				memset(ms, 0x00, sizeof(MYSTATE) );

				ms->context=context;
				ms->tlsContext=tls;
				ms->pInput=pInput;
				ms->dwInputSize=dwInputSize;
				ms->Operation=MS_ADDKEYCLIPBOARD;

				if(OpenRings(hwnd,context,&(ms->KeySet)))
				{
					err=DoWork(hwnd,ms);

// If no PGP data in clipboard, warn....
					if(!(ms->FoundPGPData))
						PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_NOPGPKEYSINCLIPBOARD,
							MB_OK|MB_ICONEXCLAMATION);

					PGPFreeKeySet(ms->KeySet);
				}
				free(ms);
			}
		}
		memset(pInput,0x00,dwInputSize);
		free(pInput);
	}

	if(IsPGPError(err))
		return FALSE;

	return TRUE;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
