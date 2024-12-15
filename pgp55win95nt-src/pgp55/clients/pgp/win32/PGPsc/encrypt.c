/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: encrypt.c,v 1.87 1997/10/13 16:40:51 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

BOOL GetCommentLine(char *comment,PGPSize size,PGPUInt32 *PrefAlg,
					PGPCipherAlgorithm *pAllowedAlgs, int *pNumAlgs)
{
	PGPPrefRef	ClientPrefRef=NULL;
	PGPPrefRef	AdminPrefRef=NULL;
	PGPError	err;
	BOOL RetVal;

	RetVal=TRUE;

	*comment=0;
	*PrefAlg=kPGPCipherAlgorithm_CAST5;

#if PGP_BUSINESS_SECURITY || PGP_ADMIN_BUILD
#if PGP_ADMIN_BUILD				
	err = PGPcomdlgOpenAdminPrefs (&AdminPrefRef, TRUE);
#else
	err = PGPcomdlgOpenAdminPrefs (&AdminPrefRef, FALSE);
#endif	// PGP_BUSINESS_SECURITY_ADMIN
	if (IsPGPError (err)) 
		return FALSE;	// error-out

	err=PGPGetPrefStringBuffer(AdminPrefRef,
	   kPGPPrefComments,
	   size,
	   comment);

	PGPcomdlgCloseAdminPrefs (AdminPrefRef, FALSE);
#endif	// PGP_BUSINESS_SECURITY

	err=PGPcomdlgOpenClientPrefs (&ClientPrefRef);
	if(IsntPGPError(err))
	{
		PGPCipherAlgorithm *pData;
		PGPSize	dataLength;
		int i;

		PGPGetPrefNumber(ClientPrefRef,
			kPGPPrefPreferredAlgorithm, 
			PrefAlg);

		err = PGPGetPrefData(ClientPrefRef,
			kPGPPrefAllowedAlgorithmsList, 
			&dataLength, (void **) &pData);

		if (IsntPGPError(err))
		{
			*pNumAlgs = dataLength / sizeof(PGPCipherAlgorithm);
			for (i=0; i<*pNumAlgs; i++)
				pAllowedAlgs[i] = pData[i];

			PGPDisposePrefData(ClientPrefRef, (void *) pData);
		}
		else
			*pNumAlgs = 0;

		if(*comment==0)
		{
			PGPGetPrefStringBuffer(ClientPrefRef,
				kPGPPrefComment,
				size,
				comment);
		}
		PGPcomdlgCloseClientPrefs (ClientPrefRef, FALSE);
	}
	
	return RetVal;
}

UINT GenericEncSign(
					PRECIPIENTDIALOGSTRUCT prds,
					MYSTATE *myState,
					PGPOptionListRef opts,
					BOOL Encrypt, 
					BOOL Sign,
					PGPKeyRef *SignKey,
					char *OperationTarget,
					BOOL ClipboardOperation,
					BOOL FirstOperation,
					char *comment,
					PGPUInt32 PrefAlg,
					PGPCipherAlgorithm *allowedAlgs, 
					int nNumAlgs)

{
	PGPError err;
	int UserCancel;
	int ReturnValue=TRUE;

	// If this is first operation, put up recipient/sign dialog

	ProcessWorkingDlg(myState->hwndWorking);

	if(FirstOperation)
	{
		if(Encrypt)
		{
			int RecipientReturn;

			RecipientReturn = PGPRecipientDialog(prds);

			if(!RecipientReturn)
				return FALSE;

			if(prds->dwOptions & PGPCOMDLG_PASSONLY)
			{
				UserCancel = PGPcomdlgGetPhrase (prds->Context,
					prds->hwndParent,
					"Enter conventional encryption passphrase :",
					&(myState->ConvPassPhrase), 
					NULL,
					NULL, 
					0,
					NULL,
					NULL,
					PGPCOMDLG_ENCRYPTION);

				if(UserCancel)
					return FALSE;
			}

		}

		if(Sign)
		{
			DWORD *lpOptions;
			PGPError err;

			lpOptions=&(prds->dwOptions);

			if (Encrypt || ClipboardOperation)
				lpOptions=0;

			err=PGPGetSignCachedPhrase (prds->Context,
				prds->hwndParent, 
				"Enter passphrase for the above key :",
				FALSE, // ForceEntry
				&myState->PassPhrase,
				prds->OriginalKeySetRef,
				SignKey,
				NULL,
				lpOptions,
				prds->dwFlags);
			
			if(err != kPGPError_NoErr)
			{
				PGPcomdlgErrorMessage (err);
				return FALSE; // UserCancel 
			}
		}

	} // END of first operation recipient/sign dialog stuff

	// if encrypting, throw in recipients
	if(Encrypt)
	{
		if(prds->dwOptions & PGPCOMDLG_PASSONLY)
		{
			int ConvPassLen;

			ConvPassLen = strlen(myState->ConvPassPhrase);

			PGPAppendOptionList(prds->Context,opts,
				PGPOConventionalEncrypt(prds->Context, 
					PGPOPassphraseBuffer(prds->Context,myState->ConvPassPhrase,
						ConvPassLen),
						PGPOLastOption(prds->Context) ),
				PGPOCipherAlgorithm(prds->Context,PrefAlg),
				PGPOLastOption(prds->Context) );
		}
		else
		{
			PGPAppendOptionList(prds->Context,opts,
				PGPOEncryptToKeySet(prds->Context,prds->SelectedKeySetRef),
				PGPOLastOption(prds->Context) );
		}
	}

	if(Sign)  // If signing, throw in a passphrase and key
	{
		PGPAppendOptionList(prds->Context,opts,
			PGPOSignWithKey(prds->Context,*SignKey,
				PGPOPassphraseBuffer(prds->Context,myState->PassPhrase,
					strlen(myState->PassPhrase)),
					PGPOLastOption(prds->Context) ),
			PGPOLastOption(prds->Context) );
	}

	if((prds->dwOptions & PGPCOMDLG_DETACHEDSIG) &&
		!ClipboardOperation && Sign && !Encrypt)
	{
		PGPAppendOptionList(prds->Context,opts,
			PGPODetachedSig(prds->Context,
				PGPOLastOption(prds->Context)), 
			PGPOLastOption(prds->Context));
	}
	else
		prds->dwOptions&=(~PGPCOMDLG_DETACHEDSIG);

	// If we have text out arguments....

	if(prds->dwOptions & PGPCOMDLG_ASCIIARMOR)
	{
		if(ClipboardOperation && Sign && !Encrypt && 
		  ((prds->dwOptions & PGPCOMDLG_DETACHEDSIG)==0)) 
		{ // Clearsign!
			PGPAppendOptionList(prds->Context,opts,
				PGPOClearSign(prds->Context, TRUE),
				PGPOLastOption(prds->Context));
		}
		else // Just Ascii Armor*/
		{
			PGPAppendOptionList(prds->Context,opts,
				PGPOArmorOutput(prds->Context, TRUE),
				PGPOLastOption(prds->Context));
		}
	}

	if(*comment!=0)
	{
		PGPAppendOptionList(prds->Context,opts,
			PGPOCommentString(prds->Context,comment),
			PGPOLastOption(prds->Context));
	}

	myState->hwnd=prds->hwndParent;
	myState->fileName=OperationTarget;
	myState->Action="Encoding";
	myState->OriginalKeySetRef=prds->OriginalKeySetRef;
	myState->SignKey=SignKey;

	sprintf(myState->ws.TitleText,"%s %s",
		myState->Action,JustFile(myState->fileName));

	SendMessage(myState->hwndWorking, PGPM_PROGRESS_UPDATE,
		(WPARAM)PGPSC_WORK_NEWFILE,0);

	ProcessWorkingDlg(myState->hwndWorking);

	if(!ClipboardOperation)
	{
		char outname[MAX_PATH];

		myState->fileRef=0;
		strcpy(outname,myState->fileName);

		AlterEncryptedFileName(outname,prds->dwOptions);

		UserCancel=SaveOutputFile(prds->Context,
			prds->hwndParent, 
			"Enter filename for encrypted file",
			outname,
			&(myState->fileRef),
			FALSE);

		ProcessWorkingDlg(myState->hwndWorking);

		if(UserCancel)
			return FALSE;

		PGPAppendOptionList(prds->Context,opts,
			PGPOOutputFile(prds->Context,myState->fileRef),
			PGPOLastOption(prds->Context));
	}

	err = PGPEncode(prds->Context,
		opts,
		PGPOPreferredAlgorithms(prds->Context,allowedAlgs,nNumAlgs),
		PGPOSendNullEvents(prds->Context,75),
		PGPOEventHandler(prds->Context,myEvents,myState),
		PGPOVersionString(prds->Context,pgpVersionHeaderString),
		PGPOAskUserForEntropy(prds->Context, TRUE),
		PGPODataIsASCII(prds->Context,(PGPBoolean)ClipboardOperation),
		PGPOLastOption(prds->Context));

	WorkingCallback (myState->hwndWorking, 
		myState->ws.total,myState->ws.total);

	ProcessWorkingDlg(myState->hwndWorking);

	if(IsPGPError(err))
	{
		return FALSE;
	}

	return TRUE;
}

UINT EncryptFileList(HWND hwnd,
					 void *PGPsc,
					 FILELIST *ListHead,
					 BOOL bEncrypt, 
					 BOOL bSign)

{
	PGPOptionListRef opts;
	PGPContextRef context;
	PGPFileSpecRef inref;
	int Success;
	FILELIST *FileCurrent;
	PRECIPIENTDIALOGSTRUCT prds;
	BOOL ReturnCode;
	PGPKeyRef SignKey;
	int NumDone;
	MYSTATE *myState;
	char comment[80];
	PGPUInt32 PrefAlg;
	PGPCipherAlgorithm allowedAlgs[3];
	int	nNumAlgs;

	context=(PGPContextRef)PGPsc;

	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
		return FALSE;

	if(!GetCommentLine(comment,sizeof(comment),&PrefAlg,
		allowedAlgs, &nNumAlgs))
		return FALSE;

	Success=FALSE;

	SignKey=0;

	prds=(PRECIPIENTDIALOGSTRUCT)
		malloc(sizeof(RECIPIENTDIALOGSTRUCT));
	memset(prds,0x00,sizeof(RECIPIENTDIALOGSTRUCT));

	myState=(MYSTATE *)malloc(sizeof(MYSTATE));
	memset(myState, 0x00, sizeof(MYSTATE) );

	if(OpenRings(hwnd,context,&(prds->OriginalKeySetRef)))
	{
		prds->Version=CurrentPGPrecipVersion;
		prds->hwndParent=hwnd;
		prds->szTitle="PGP - Key Selection Dialog";
		prds->Context=context;
		prds->dwDisableFlags=PGPCOMDLG_DISABLE_AUTOMODE;
		prds->dwOptions=PGPCOMDLG_DETACHEDSIG;
	
		FileCurrent=ListHead;
		ReturnCode=TRUE;
		NumDone=0;

		strcpy(myState->ws.StartText,"Starting work...");
		strcpy(myState->ws.EndText,"Finishing up...");

		myState->hwndWorking=CreateDialogParam(g_hinst, 
			MAKEINTRESOURCE(IDD_WORKING), 
			hwnd,
			(DLGPROC)WorkingDlgProc,
			(LPARAM)&(myState->ws));

		while((FileCurrent!=0)&&(ReturnCode==TRUE))
		{   
			if(FileCurrent->IsDirectory)
			{
				FileCurrent=FileCurrent->next;
				continue;
			}

			ReturnCode=FALSE;

// We'll ask for output file in generic later...
			PGPNewFileSpecFromFullPath( context,
				FileCurrent->name, &inref);

			PGPBuildOptionList(prds->Context,&opts,
				PGPOInputFile(prds->Context,inref),
				PGPOLastOption(prds->Context));
	
			Success=GenericEncSign(
				prds,
				myState,
				opts,
				bEncrypt, 
				bSign,
				&SignKey,
				FileCurrent->name,
				FALSE,
				NumDone==0,
				comment,
				PrefAlg,
				allowedAlgs, 
				nNumAlgs);

			PGPFreeOptionList(opts);

			PGPFreeFileSpec( inref );
			if(myState->fileRef)
				PGPFreeFileSpec(myState->fileRef);

			if((Success)&&(prds->dwOptions & PGPCOMDLG_WIPEORIG))
			{
				char TempStart[40];
				char TempEnd[40];
				UINT WipeReturn;

				strcpy(TempStart,myState->ws.StartText);
				strcpy(TempEnd,myState->ws.EndText);

				strcpy(myState->ws.StartText,"");
				strcpy(myState->ws.EndText,"");

				WipeReturn=InternalFileWipe(myState->hwndWorking,
					FileCurrent->name);

				if(WipeReturn!=WIPE_OK)
				{
					if (WipeError(myState->hwndWorking,
						FileCurrent->name,WipeReturn))
					{
						ReturnCode=FALSE;
					}
				}

				strcpy(myState->ws.StartText,TempStart);
				strcpy(myState->ws.EndText,TempEnd);
			}

			ReturnCode=Success;

			NumDone++;			
			FileCurrent=FileCurrent->next;				
		}

		DestroyWindow(myState->hwndWorking);
		ProcessWorkingDlg(myState->hwndWorking);

		if(prds->SelectedKeySetRef)
			PGPFreeKeySet(prds->SelectedKeySetRef);

		if(prds->OriginalKeySetRef)
			PGPFreeKeySet(prds->OriginalKeySetRef);

		if(myState->PassPhrase)
			PGPFreePhrase (myState->PassPhrase);
			
		if(myState->ConvPassPhrase)
			PGPcomdlgFreePhrase(myState->ConvPassPhrase);
	}
	free(myState);
	free(prds);

	FreeFileList(ListHead);

	return Success;
}

BOOL WeNeedToWordWrap(unsigned short* WordWrapThreshold)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPBoolean	bWrapReturn=TRUE;
	PGPUInt32	WrapWidth=72;

	PGPcomdlgOpenClientPrefs (&PrefRefClient);
	PGPGetPrefBoolean (PrefRefClient,
		kPGPPrefWordWrapEnable, &bWrapReturn);
	PGPGetPrefNumber (PrefRefClient, 
		kPGPPrefWordWrapWidth, &WrapWidth);
	PGPcomdlgCloseClientPrefs (PrefRefClient, FALSE);

	*WordWrapThreshold=WrapWidth;

	return bWrapReturn;
}

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


BOOL WrapIfNecessary(char **pszClipText,DWORD *dwClipSize)
{
	unsigned short WordWrapThreshold;
	char* szWrappedText;
	DWORD dwWrappedSize;

	if(WeNeedToWordWrap(&WordWrapThreshold))
	{
		if(TRUE == WrapBuffer(&szWrappedText, 
			*pszClipText,WordWrapThreshold))
		{	
			dwWrappedSize = strlen(szWrappedText);

			memset(*pszClipText,0x00,*dwClipSize);
			free(*pszClipText);
			*pszClipText=szWrappedText;

			*dwClipSize=dwWrappedSize;
		}
	}

	return TRUE;
}

unsigned long DoEncrypt(HWND hwnd,void *PGPsc,BOOL Encrypt,BOOL Sign)
{
	PGPOptionListRef opts;
	PGPContextRef context;
	int Success;
	PRECIPIENTDIALOGSTRUCT prds;
	PGPKeyRef SignKey;
	MYSTATE *myState;
	char *pInput;
	DWORD dwInputSize;
	char *pOutput;
	DWORD dwOutputSize;
	UINT ClipboardFormat;
	char comment[80];
	PGPUInt32 PrefAlg;
	PGPCipherAlgorithm allowedAlgs[3];
	int	nNumAlgs;

	context=(PGPContextRef)PGPsc;

	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
		return FALSE;

	if(!GetCommentLine(comment,sizeof(comment),&PrefAlg,
		allowedAlgs, &nNumAlgs))
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
			WrapIfNecessary(&pInput,&dwInputSize);

			SignKey=0;

			prds=(PRECIPIENTDIALOGSTRUCT)
				malloc(sizeof(RECIPIENTDIALOGSTRUCT));
			memset(prds,0x00,sizeof(RECIPIENTDIALOGSTRUCT));

			myState=(MYSTATE *)malloc(sizeof(MYSTATE));
			memset(myState, 0x00, sizeof(MYSTATE) );

			if(OpenRings(hwnd,context,&(prds->OriginalKeySetRef)))
			{
				prds->Version=CurrentPGPrecipVersion;
				prds->hwndParent=hwnd;
				prds->szTitle="PGP - Key Selection Dialog";
				prds->Context=context;
				prds->dwOptions=PGPCOMDLG_ASCIIARMOR;
				prds->dwDisableFlags=PGPCOMDLG_DISABLE_ASCIIARMOR |
					PGPCOMDLG_DISABLE_WIPEORIG |
					PGPCOMDLG_DISABLE_AUTOMODE;

				strcpy(myState->ws.StartText,"Starting work...");
				strcpy(myState->ws.EndText,"Finishing up...");

				myState->hwndWorking=CreateDialogParam(g_hinst, 
					MAKEINTRESOURCE(IDD_WORKING), 
					hwnd,
					(DLGPROC)WorkingDlgProc,
					(LPARAM)&(myState->ws));

				pOutput=0;

				PGPBuildOptionList(prds->Context,&opts,
					PGPOInputBuffer(prds->Context,pInput,dwInputSize),
					PGPOAllocatedOutputBuffer(prds->Context,
						&pOutput,MAX_BUFFER_SIZE ,&dwOutputSize),
					PGPOLastOption(prds->Context));

				Success=GenericEncSign(
					prds,
					myState,
					opts,
					Encrypt, 
					Sign,
					&SignKey,
					"Clipboard",
					TRUE,
					0==0,
					comment,
					PrefAlg,
					allowedAlgs, 
					nNumAlgs);

				PGPFreeOptionList(opts);

				DestroyWindow(myState->hwndWorking);
				ProcessWorkingDlg(myState->hwndWorking);

				if(prds->SelectedKeySetRef)
					PGPFreeKeySet(prds->SelectedKeySetRef);

				if(prds->OriginalKeySetRef)
					PGPFreeKeySet(prds->OriginalKeySetRef);

				if(myState->PassPhrase)
					PGPFreePhrase (myState->PassPhrase);

				if(myState->ConvPassPhrase)
					PGPcomdlgFreePhrase(myState->ConvPassPhrase);
				
				if((pOutput!=0) && (Success))
				{
					StoreClipboardData(hwnd, pOutput, dwOutputSize);
					memset(pOutput,0x00,dwOutputSize);
					PGPFreeData(pOutput); // Since auto alloced by CDK
				}
			}

			free(myState);
			free(prds);
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
