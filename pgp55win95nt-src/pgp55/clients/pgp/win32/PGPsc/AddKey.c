/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: AddKey.c,v 1.30 1997/09/21 14:19:02 wjb Exp $
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
	case kPGPEvent_AnalyzeEvent:
		{	PGPEventAnalyzeData *d = &event->data.analyzeData;

			if(d->sectionType!=kPGPAnalyze_Key)
				return kPGPError_SkipSection;
		}
		break;
	default:
		break;
	}
				
	return kPGPError_NoErr;
}

BOOL GenericAddKey(HWND hwnd,
				   PGPContextRef context,
				   char *FileName,
				   MYSTATE *myState,
				   PGPKeySetRef OrigKeySet,
				   PGPOptionListRef opts)
{
	BOOL Success;
	PGPKeySetRef AddKeySet;
	PGPError err;

	ProcessWorkingDlg(myState->hwndWorking);

	Success=FALSE;

	sprintf(myState->ws.TitleText,"Adding %s",JustFile(FileName));

	strcpy(myState->ws.StartText,"");
	strcpy(myState->ws.EndText,"Finished reading keys. Please wait.");

	SendMessage(myState->hwndWorking, PGPM_PROGRESS_UPDATE,
		(WPARAM)PGPSC_WORK_NEWFILE,0);

	ProcessWorkingDlg(myState->hwndWorking);

	err=PGPImportKeySet(context,&AddKeySet,
		opts,
		PGPOSendNullEvents(context,75),
		PGPOEventHandler(context,AddEvents,myState),
		PGPOLastOption(context));

	WorkingCallback (myState->hwndWorking, 
		myState->ws.total,myState->ws.total);

	ProcessWorkingDlg(myState->hwndWorking);

	if(IsntPGPError(err)) 
	{
		PGPUInt32	numKeys;

		Success=TRUE;

		PGPCountKeys( AddKeySet, &numKeys);
		if ( numKeys > 0) 
		{	
			if (OleInitialize (NULL) == NOERROR)
			{
				if (PGPkmQueryAddKeys (context, 
					hwnd,AddKeySet,NULL) != kPGPError_NoErr)
					Success=FALSE;
				OleUninitialize();
			}
		}
		PGPFreeKeySet (AddKeySet);
	}

	ProcessWorkingDlg(myState->hwndWorking);

	return Success;
}

UINT AddKeyFileList(HWND hwnd,void *PGPsc,FILELIST *ListHead)
{
	PGPOptionListRef opts;
	PGPKeySetRef OrigKeySetRef;
	PGPContextRef context;
	PGPFileSpecRef inref;
	int Success;
	FILELIST *FileCurrent;
	BOOL ReturnCode;
	int NumDone;
	MYSTATE *myState;

	context=(PGPContextRef)PGPsc;

	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED))
		return FALSE;

	Success=FALSE;

	myState=(MYSTATE *)malloc(sizeof(MYSTATE));
	memset(myState, 0x00, sizeof(MYSTATE) );

	if(OpenRings(hwnd,context,&OrigKeySetRef))
	{
		myState->hwndWorking=CreateDialogParam(g_hinst, 
			MAKEINTRESOURCE(IDD_WORKING), 
			hwnd,
			(DLGPROC)WorkingDlgProc,
			(LPARAM)&(myState->ws));

		ProcessWorkingDlg(myState->hwndWorking);

		ReturnCode=TRUE;
		FileCurrent=ListHead;
		NumDone=0;

		while((FileCurrent!=0)&&(ReturnCode==TRUE))
		{    
			if(FileCurrent->IsDirectory)
			{
				FileCurrent=FileCurrent->next;
				continue;
			}

			ReturnCode=FALSE;

			PGPNewFileSpecFromFullPath (context, 
				FileCurrent->name, &inref);

			PGPBuildOptionList(context,&opts,
				PGPOInputFile(context,inref),
				PGPOLastOption(context));

			Success=GenericAddKey(hwnd,
				context,
				FileCurrent->name,
				myState,
				OrigKeySetRef,
				opts);

			PGPFreeOptionList(opts);

			PGPFreeFileSpec( inref );

			ReturnCode=Success;

			NumDone++;			
			FileCurrent=FileCurrent->next;				
		}

		DestroyWindow(myState->hwndWorking);
		ProcessWorkingDlg(myState->hwndWorking);

		PGPFreeKeySet(OrigKeySetRef);
	}

	free(myState);

	FreeFileList(ListHead);

	return Success;
}

unsigned long DoAddKey(HWND hwnd,void *PGPsc)
{
	PGPOptionListRef opts;
	PGPContextRef context;
	unsigned long Success;
	char *pInput;
	DWORD dwInputSize;
	UINT ClipboardFormat;
	MYSTATE *myState;
	PGPKeySetRef OrigKeySetRef;

	context=(PGPContextRef)PGPsc;

	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED))
		return FALSE;

	Success=FALSE;

	pInput=RetrieveClipboardData(hwnd, &ClipboardFormat, 
			                     &dwInputSize);

	if(!pInput)
	{
		MessageBox(NULL,"Clipboard contents could not be obtained.",
			"Clipboard Error",MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
	}
	else
	{
		if((ClipboardFormat == CF_TEXT)&&(*pInput!=0))
		{
			myState=(MYSTATE *)malloc(sizeof(MYSTATE));
			memset(myState, 0x00, sizeof(MYSTATE) );

			if(OpenRings(hwnd,context,&OrigKeySetRef))
			{
				myState->hwndWorking=CreateDialogParam(g_hinst, 
					MAKEINTRESOURCE(IDD_WORKING), 
					hwnd,
					(DLGPROC)WorkingDlgProc,
					(LPARAM)&(myState->ws));

				ProcessWorkingDlg(myState->hwndWorking);

				PGPBuildOptionList(context,&opts,
					PGPOInputBuffer(context,pInput,dwInputSize),
					PGPOLastOption(context));

				Success=GenericAddKey(hwnd,
					context,
					"Clipboard",
					myState,
					OrigKeySetRef,
					opts);

				PGPFreeOptionList(opts);

				DestroyWindow(myState->hwndWorking);
				ProcessWorkingDlg(myState->hwndWorking);

				PGPFreeKeySet(OrigKeySetRef);
			}
			free(myState);
		}
		memset(pInput,0x00,dwInputSize);
		free(pInput);
	}

	return Success;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
