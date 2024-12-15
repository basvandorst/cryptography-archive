/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: decrypt.c,v 1.53 1997/10/09 00:44:20 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

UINT GenericDecVer(HWND hwnd,
				    MYSTATE *myState,
					PGPContextRef context,
					PGPKeySetRef OrigKeySet,
					PGPOptionListRef opts,
					char *OperationTarget,
					BOOL FirstOperation)
{
	PGPError err;
	PGPKeySetRef AddKeySet;
	int ReturnValue=TRUE;
	PGPUInt32	numKeys	= 0;
	
	ProcessWorkingDlg(myState->hwndWorking);

	if(myState->PassPhrase)
	{
		PGPAppendOptionList(context,opts,
				   PGPOPassphraseBuffer(context,myState->PassPhrase,
						strlen(myState->PassPhrase)),
				   PGPOLastOption(context) );
	}

	myState->hwnd=hwnd;
	myState->fileName=OperationTarget;
	myState->fileRef=0;
	myState->Action="Decoding";
	myState->PassCount=0;
	myState->RecipientKeySet=0;
	myState->RecipientKeyIDArray=0;
	myState->dwKeyIDCount=0;
	myState->FoundPGPData=FALSE;

	sprintf(myState->ws.TitleText,"%s %s",
		myState->Action,JustFile(myState->fileName));

	SendMessage(myState->hwndWorking, PGPM_PROGRESS_UPDATE,
		(WPARAM)PGPSC_WORK_NEWFILE,0);

	ProcessWorkingDlg(myState->hwndWorking);

	PGPNewKeySet( context, &AddKeySet);

	err = PGPDecode( context, 
					opts,
					PGPOKeySetRef(context,OrigKeySet),
					PGPOEventHandler(context,myEvents, myState),
					PGPOImportKeysTo(context,AddKeySet),
					PGPOSendNullEvents(context,75),
					PGPOLastOption(context) );

	WorkingCallback (myState->hwndWorking, 
		myState->ws.total,myState->ws.total);

	ProcessWorkingDlg(myState->hwndWorking);
	
	(void)PGPCountKeys( AddKeySet, &numKeys );
	if ( numKeys > 0) 
	{	
		if (OleInitialize (NULL) == NOERROR)
		{
			PGPkmQueryAddKeys (context,hwnd,AddKeySet,NULL);
			OleUninitialize();
		}
	}
	PGPFreeKeySet (AddKeySet);

	if(myState->RecipientKeySet)
		PGPFreeKeySet(myState->RecipientKeySet);

	if(myState->RecipientKeyIDArray)
		free(myState->RecipientKeyIDArray);

	ProcessWorkingDlg(myState->hwndWorking);

	if(IsPGPError(err))
	{
		return FALSE;
	}

	return TRUE;
}

UINT DecryptFileList(HWND hwnd,void *PGPsc,FILELIST *ListHead)
{
	PGPContextRef context;
	PGPFileSpecRef inref;
	PGPOptionListRef opts;
	BOOL ReturnCode;
	FILELIST *FileCurrent;
	PGPKeySetRef kset;
	int NumDone;
	MYSTATE *myState;

	context=(PGPContextRef)PGPsc;

	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED))
		return FALSE;

	myState=(MYSTATE *)malloc(sizeof(MYSTATE));
	memset(myState, 0x00, sizeof(MYSTATE) );

	if(OpenRings(hwnd,context,&kset))
	{
		strcpy(myState->ws.StartText,"Starting work...");
		strcpy(myState->ws.EndText,"Finishing up...");

		myState->hwndWorking=CreateDialogParam(g_hinst, 
			MAKEINTRESOURCE(IDD_WORKING), 
			hwnd,
			(DLGPROC)WorkingDlgProc,
			(LPARAM)&(myState->ws));

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

// events handler will ask for output file later...
			PGPNewFileSpecFromFullPath( context,
				FileCurrent->name, &inref);

			PGPBuildOptionList(context,&opts,
				PGPOInputFile(context,inref),
				PGPOLastOption(context) );

			ReturnCode=GenericDecVer(hwnd,
				myState,
				context,
				kset,
				opts,
				FileCurrent->name,
				NumDone==0);

			PGPFreeOptionList(opts);

			if((ReturnCode)&&(myState->fileRef))
			{
				PGPUInt32 macCreator,macTypeCode;
				PGPFileSpecRef deMacifiedFSpec;

				deMacifiedFSpec=0;

				PGPMacBinaryToLocal(myState->fileRef,
					&deMacifiedFSpec,
					&macCreator,&macTypeCode );

				if(deMacifiedFSpec)
					PGPFreeFileSpec(deMacifiedFSpec);
			}

			PGPFreeFileSpec(inref);
			if(myState->fileRef)
				PGPFreeFileSpec(myState->fileRef);

			NumDone++;			
			FileCurrent=FileCurrent->next;				
		}

		DestroyWindow(myState->hwndWorking);
		ProcessWorkingDlg(myState->hwndWorking);

		PGPFreeKeySet(kset);

		if(myState->PassPhrase)
			PGPFreePhrase (myState->PassPhrase);

// If we have one file, and no PGP data, warn....
		if(!(myState->FoundPGPData))
			if(ListHead!=0)
				if(ListHead->next==0)
					MessageBox(hwnd,"Found no PGP information in this file.",
						ListHead->name,
						MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND);	
	}
	
	free(myState);

	FreeFileList(ListHead);

	return ReturnCode;
}

unsigned long DoDecrypt(HWND hwnd,void *PGPsc)
{
	PGPContextRef context;
	PGPOptionListRef opts;
	int Success;
	PGPKeySetRef kset;
	MYSTATE *myState;
	char *pInput;
	DWORD dwInputSize;
	char *pOutput;
	DWORD dwOutputSize;
	UINT ClipboardFormat;

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
			pOutput=0;

			myState=(MYSTATE *)malloc(sizeof(MYSTATE));
			memset(myState, 0x00, sizeof(MYSTATE) );

			if(OpenRings(hwnd,context,&kset))
			{
				strcpy(myState->ws.StartText,"Starting work...");
				strcpy(myState->ws.EndText,"Finishing up...");

				myState->hwndWorking=CreateDialogParam(g_hinst, 
					MAKEINTRESOURCE(IDD_WORKING), 
					hwnd,
					(DLGPROC)WorkingDlgProc,
					(LPARAM)&(myState->ws));

				PGPBuildOptionList(context,&opts,
					PGPOInputBuffer(context,pInput, dwInputSize),
					PGPOAllocatedOutputBuffer(context,&pOutput, 
						MAX_BUFFER_SIZE ,&dwOutputSize),
					PGPOLastOption(context) );

				Success=GenericDecVer(hwnd,
					myState,
					context,
					kset,
					opts,
					"Clipboard",
					TRUE);

				DestroyWindow(myState->hwndWorking);
				ProcessWorkingDlg(myState->hwndWorking);

				PGPFreeOptionList(opts);

				PGPFreeKeySet(kset);

				if(myState->PassPhrase)
					PGPFreePhrase(myState->PassPhrase);

				if((Success) && (pOutput!=0))
				{
					TextViewer(hwnd,pOutput,dwOutputSize);
					memset(pOutput,0x00,dwOutputSize);
					PGPFreeData(pOutput); // Since auto alloced by CDK
				}
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
