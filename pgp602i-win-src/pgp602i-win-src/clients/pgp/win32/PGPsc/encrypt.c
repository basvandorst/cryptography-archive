/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: encrypt.c,v 1.119.2.2.2.1 1998/11/12 03:13:34 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

BOOL GetCommentLine(PGPContextRef context,
					char *comment,PGPSize size,PGPUInt32 *PrefAlg,
					PGPCipherAlgorithm *pAllowedAlgs, int *pNumAlgs)
{
	PGPPrefRef	ClientPrefRef=NULL;
	PGPPrefRef	AdminPrefRef=NULL;
	PGPError	err;
	BOOL RetVal;
	PGPMemoryMgrRef memMgr;

	memMgr=PGPGetContextMemoryMgr(context);

	RetVal=TRUE;

	*comment=0;
	*PrefAlg=kPGPCipherAlgorithm_CAST5;

#if PGP_BUSINESS_SECURITY || PGP_ADMIN_BUILD
#if PGP_ADMIN_BUILD				
	err = PGPclOpenAdminPrefs (memMgr,&AdminPrefRef, TRUE);
#else
	err = PGPclOpenAdminPrefs (memMgr,&AdminPrefRef, FALSE);
#endif	// PGP_BUSINESS_SECURITY_ADMIN
	if (IsPGPError (err)) 
		return FALSE;	// error-out

	err=PGPGetPrefStringBuffer(AdminPrefRef,
	   kPGPPrefComments,
	   size,
	   comment);

	PGPclCloseAdminPrefs (AdminPrefRef, FALSE);
#endif	// PGP_BUSINESS_SECURITY

	err=PGPclOpenClientPrefs (memMgr,&ClientPrefRef);
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
		PGPclCloseClientPrefs (ClientPrefRef, FALSE);
	}
	
	return RetVal;
}

PGPError GenericEncSign(MYSTATE *ms,
					PGPOptionListRef opts,
					char *OperationTarget)
{
	PGPError err;
	int UserCancel;
	PGPBoolean DataIsASCII;
	PGPUInt32 numKeys;
	PRECIPIENTDIALOGSTRUCT prds;
	PGPContextRef	context;
	char StrRes[500];
	PGPBoolean FYEO;

	err=kPGPError_NoErr;	
	
	context=ms->context;

	prds=ms->prds;

	// If this is first operation, put up recipient/sign dialog

	if(ms->numDone==0)
	{
		FreePhrases(ms);

		if(ms->Encrypt)
		{
			int RecipientReturn;

			LoadString (g_hinst, IDS_PGPKEYSELDLG, StrRes, sizeof(StrRes));

			prds->Version=CurrentPGPrecipVersion;
			prds->hwndParent=ms->hwndWorking;
			prds->szTitle=StrRes;
			prds->Context=ms->context;
			prds->tlsContext=ms->tlsContext;
			prds->OriginalKeySetRef=ms->KeySet;

			prds->dwDisableFlags|=PGPCL_DISABLE_FYEO;

			// FYEO only for clipboard or text files
			if((ms->Operation==MS_ENCRYPTCLIPBOARD) ||
				(FileHasThisExtension(OperationTarget,"txt")))
			{
				prds->dwDisableFlags&=(~PGPCL_DISABLE_FYEO);
			}

			RecipientReturn = PGPclRecipientDialog(prds);

			if(!RecipientReturn)
				return kPGPError_UserAbort;

			if(prds->dwOptions & PGPCL_PASSONLY)
			{
				LoadString (g_hinst, IDS_CONVPASSPROMPT, StrRes, sizeof(StrRes));

				UserCancel = PGPclGetPhrase (context,
					ms->KeySet,
					ms->hwndWorking,
					StrRes,
					&(ms->ConvPassPhrase), 
					NULL,
					NULL, 
					0,
					NULL,
					NULL,
					PGPCL_ENCRYPTION,
					NULL,NULL,
					1,0,ms->tlsContext,NULL);

				if(UserCancel)
					return kPGPError_UserAbort;
			}

		}

		if(ms->Sign)
		{
			DWORD *lpOptions;
			PGPError err;

			lpOptions=&(prds->dwOptions);

			if (ms->Encrypt || (ms->Operation==MS_ENCRYPTCLIPBOARD))
				lpOptions=0;

			LoadString (g_hinst, IDS_PASSPROMPTABOVEKEY, StrRes, sizeof(StrRes));

			err=PGPclGetCachedSigningPhrase (context,
				ms->tlsContext,
				ms->hwndWorking, 
				StrRes,
				FALSE, // ForceEntry
				&ms->PassPhrase,
				ms->KeySet,
				&(ms->SignKey),
				NULL,
				lpOptions,
				prds->dwFlags,
				&ms->PassKey,
				&ms->PassKeyLen,
				NULL);
			
			if(err != kPGPError_NoErr)
			{
				PGPclEncDecErrorBox(ms->hwndWorking,err);
				return kPGPError_UserAbort; // UserCancel 
			}
		}
	} // END of first operation recipient/sign dialog stuff

	// if encrypting, throw in recipients
	if(ms->Encrypt)
	{
		if(prds->dwOptions & PGPCL_PASSONLY)
		{
			int ConvPassLen;

			ConvPassLen = strlen(ms->ConvPassPhrase);

			PGPAppendOptionList(opts,
				PGPOConventionalEncrypt(context, 
					PGPOPassphraseBuffer(context,ms->ConvPassPhrase,
						ConvPassLen),
						PGPOLastOption(context) ),
				PGPOCipherAlgorithm(context,ms->PrefAlg),
				PGPOLastOption(context) );
		}
		else
		{
			PGPAppendOptionList(opts,
				PGPOEncryptToKeySet(context,prds->SelectedKeySetRef),
				PGPOLastOption(context) );
		}
	}

	if(ms->Sign)  // If signing, throw in a passphrase and key
	{
		// Use passkey if available
		if(ms->PassKey)
		{
			PGPAppendOptionList(opts,
				PGPOSignWithKey(context,ms->SignKey,
					PGPOPasskeyBuffer(context,
						ms->PassKey,ms->PassKeyLen),
					PGPOLastOption(context) ),
				PGPOLastOption(context) );
		}
		else
		{
			PGPAppendOptionList(opts,
				PGPOSignWithKey(context,ms->SignKey,
					PGPOLastOption(context) ),
				PGPOLastOption(context) );
		}
	}

	if((prds->dwOptions & PGPCL_DETACHEDSIG) &&
		!(ms->Operation==MS_ENCRYPTCLIPBOARD) && 
		(ms->Sign) && !(ms->Encrypt))
	{
		PGPAppendOptionList(opts,
			PGPODetachedSig(context,
				PGPOLastOption(context)), 
			PGPOLastOption(context));
	}
	else
		prds->dwOptions&=(~PGPCL_DETACHEDSIG);

	DataIsASCII=(ms->Operation==MS_ENCRYPTCLIPBOARD);

	// If we have text out arguments....
	if(prds->dwOptions & PGPCL_ASCIIARMOR)
	{
		if((ms->Operation==MS_ENCRYPTCLIPBOARD) && 
			(ms->Sign) && !(ms->Encrypt) && 
			((prds->dwOptions & PGPCL_DETACHEDSIG)==0)) 
		{ // Clearsign!
			PGPAppendOptionList(opts,
				PGPOClearSign(context, TRUE),
				PGPOLastOption(context));
		}
		else if(!(ms->Operation==MS_ENCRYPTCLIPBOARD) && 
			(ms->Sign) && !(ms->Encrypt) && 
			((prds->dwOptions & PGPCL_DETACHEDSIG)==0)&&
		  FileHasThisExtension(OperationTarget,"txt")) 
		{ // Clearsign a .txt file
			DataIsASCII=TRUE; // Set to ASCII input

			PGPAppendOptionList(opts,
				PGPOClearSign(context, TRUE),
				PGPOLastOption(context));
		} 
		else // Just Ascii Armor*/
		{
			PGPAppendOptionList(opts,
				PGPOArmorOutput(context, TRUE),
				PGPOLastOption(context));
		}
	}

	if(*(ms->comment)!=0)
	{
		PGPAppendOptionList(opts,
			PGPOCommentString(context,(ms->comment)),
			PGPOLastOption(context));
	}

	ms->fileName=OperationTarget;

	LoadString (g_hinst, IDS_ENCODINGFILE, StrRes, sizeof(StrRes));
	sprintf(ms->ws.TitleText,StrRes,JustFile(ms->fileName));

	SendMessage(ms->hwndWorking, PGPM_PROGRESS_UPDATE,
		(WPARAM)PGPSC_WORK_NEWFILE,0);

	if(!(ms->Operation==MS_ENCRYPTCLIPBOARD))
	{
		char outname[MAX_PATH];

		ms->fileRef=0;
		strcpy(outname,ms->fileName);

		AlterEncryptedFileName(outname,prds->dwOptions);

		LoadString (g_hinst, IDS_ENCFILENAMEPROMPT, StrRes, sizeof(StrRes));

		UserCancel=SaveOutputFile(context,
			ms->hwndWorking, 
			StrRes,
			outname,
			&(ms->fileRef),
			FALSE);

		if(UserCancel)
			return kPGPError_UserAbort;

		PGPAppendOptionList(opts,
			PGPOOutputFile(context,ms->fileRef),
			PGPOLastOption(context));
	}

	// Assume False for For Your Eyes Only
	FYEO=FALSE;

	// These are the only situations we can have FYEO
	if((ms->Operation==MS_ENCRYPTCLIPBOARD) ||
		(FileHasThisExtension(OperationTarget,"txt")))
	{
		// If the recipient dialog has selected it then we're OK
		if((prds->dwOptions & PGPCL_FYEO)==PGPCL_FYEO)
		{
			FYEO=TRUE;
		}
	}

	err = PGPEncode(context,
		opts,
		PGPOPreferredAlgorithms(context,ms->allowedAlgs,ms->nNumAlgs),
		PGPOSendNullEvents(context,75),
		PGPOEventHandler(context,myEvents,ms),
		PGPOVersionString(context,pgpVersionHeaderString),
		PGPOAskUserForEntropy(context, TRUE),
		PGPODataIsASCII(context,DataIsASCII),
		PGPOForYourEyesOnly(context,FYEO),
		PGPOLastOption(context));

	WorkingCallback (ms->hwndWorking, 
		ms->ws.total,ms->ws.total);

	if(PGPRefIsValid(prds->AddedKeys))
	{
		(void)PGPCountKeys( prds->AddedKeys, &numKeys );
		if ( numKeys > 0) 
		{	
			PGPclQueryAddKeys (context,ms->tlsContext,ms->hwndWorking,
				prds->AddedKeys,NULL);
		}
		PGPFreeKeySet (prds->AddedKeys);
	}

	return err;
}

PGPError EncryptFileListStub (MYSTATE *ms) 
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

// We'll ask for output file in event handler later...
		PGPNewFileSpecFromFullPath( context,
			FileCurrent->name, &inref);

		PGPBuildOptionList(context,&opts,
			PGPOInputFile(context,inref),
			PGPOLastOption(context));
	
		err=GenericEncSign(ms,opts,FileCurrent->name);

		PGPFreeOptionList(opts);

		PGPFreeFileSpec( inref );

		if(ms->fileRef)
			PGPFreeFileSpec(ms->fileRef);

		if((IsntPGPError(err))&&(ms->prds->dwOptions & PGPCL_WIPEORIG))
		{
			char TempStart[40];
			char TempEnd[40];
			UINT WipeReturn;

			strcpy(TempStart,ms->ws.StartText);
			strcpy(TempEnd,ms->ws.EndText);

			strcpy(ms->ws.StartText,"");
			strcpy(ms->ws.EndText,"");

			WipeReturn=InternalFileWipe(ms->context,
				ms->hwndWorking,
				FileCurrent->name);

			if(WipeReturn!=WIPE_OK)
			{
				if (WipeError(ms->hwndWorking,
					FileCurrent,WipeReturn))
				{
					err=kPGPError_UnknownError;
				}
			}

			strcpy(ms->ws.StartText,TempStart);
			strcpy(ms->ws.EndText,TempEnd);
		}

		ms->numDone++;
		FileCurrent=FileCurrent->next;				
	}

	return err;
}

BOOL EncryptFileList(HWND hwnd,
					 void *PGPsc,
					 void *PGPtls,
					 FILELIST *ListHead,
					 BOOL bEncrypt, 
					 BOOL bSign)

{
	PGPContextRef context;
	PRECIPIENTDIALOGSTRUCT prds;
	MYSTATE *ms;
	char comment[80];
	PGPUInt32 PrefAlg;
	PGPCipherAlgorithm allowedAlgs[3];
	int nNumAlgs;
	PGPtlsContextRef tls;
	PGPError err;
	char StrRes[500];

	err=kPGPError_NoErr;

	context=(PGPContextRef)PGPsc;
	tls=(PGPtlsContextRef)PGPtls;

	if(IsPGPError(PGPclEvalExpired(hwnd, PGPCL_ENCRYPTSIGNEXPIRED)))
		return FALSE;

	if(!GetCommentLine(context,comment,sizeof(comment),&PrefAlg,
		allowedAlgs, &nNumAlgs))
		return FALSE;

	prds=(PRECIPIENTDIALOGSTRUCT)
		malloc(sizeof(RECIPIENTDIALOGSTRUCT));

	if(prds)
	{
		memset(prds,0x00,sizeof(RECIPIENTDIALOGSTRUCT));

		LoadString (g_hinst, IDS_PGPKEYSELDLG, StrRes, sizeof(StrRes));

		prds->szTitle=StrRes;
		prds->dwDisableFlags=PGPCL_DISABLE_AUTOMODE;
		prds->dwOptions=PGPCL_DETACHEDSIG;

		ms=(MYSTATE *)malloc(sizeof(MYSTATE));

		if(ms)
		{
			memset(ms, 0x00, sizeof(MYSTATE) );

			LoadString (g_hinst, IDS_STARTINGWORK, ms->ws.StartText, sizeof(ms->ws.StartText));
			LoadString (g_hinst, IDS_FINISHINGUP, ms->ws.EndText, sizeof(ms->ws.EndText));
			ms->context=context;
			ms->tlsContext=tls;
			ms->ListHead=ListHead;
			ms->Operation=MS_ENCRYPTFILELIST;
			ms->prds=prds;
			ms->Encrypt=bEncrypt; 
			ms->Sign=bSign;
			ms->comment=comment;
			ms->PrefAlg=PrefAlg;
			ms->allowedAlgs=allowedAlgs; 
			ms->nNumAlgs=nNumAlgs;

			if(OpenRings(hwnd,context,&(ms->KeySet)))
			{	
				err=DoWork(hwnd,ms);

				if(prds->SelectedKeySetRef)
					PGPFreeKeySet(prds->SelectedKeySetRef);

				FreePhrases(ms);

				PGPFreeKeySet(ms->KeySet);
			}
			free(ms);
		}
		free(prds);
	}
	
	FreeFileList(ListHead);

	if(IsPGPError(err))
		return FALSE;

	return TRUE;
}

BOOL WeNeedToWordWrap(PGPContextRef context,
					  unsigned short* WordWrapThreshold)
{
	PGPPrefRef	PrefRefClient=NULL;
	PGPBoolean	bWrapReturn=TRUE;
	PGPUInt32	WrapWidth=72;
	PGPMemoryMgrRef memMgr;

	memMgr=PGPGetContextMemoryMgr(context);

	PGPclOpenClientPrefs (memMgr,&PrefRefClient);
	PGPGetPrefBoolean (PrefRefClient,
		kPGPPrefWordWrapEnable, &bWrapReturn);
	PGPGetPrefNumber (PrefRefClient, 
		kPGPPrefWordWrapWidth, &WrapWidth);
	PGPclCloseClientPrefs (PrefRefClient, FALSE);

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


BOOL WrapIfNecessary(PGPContextRef context,
					 char **pszClipText,DWORD *dwClipSize)
{
	unsigned short WordWrapThreshold;
	char* szWrappedText;
	DWORD dwWrappedSize;

	if(WeNeedToWordWrap(context,&WordWrapThreshold))
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

PGPError EncryptClipboardStub (MYSTATE *ms) 
{
	PGPOptionListRef opts;
	PGPContextRef context;
	PGPError err;
	char StrRes[100];

	err=kPGPError_NoErr;

	context=ms->context;

	PGPBuildOptionList(context,&opts,
		PGPOInputBuffer(context,ms->pInput,ms->dwInputSize),
		PGPOAllocatedOutputBuffer(context,
			&(ms->pOutput),MAX_BUFFER_SIZE ,&(ms->dwOutputSize)),
		PGPOLastOption(context));

	LoadString (g_hinst, IDS_CLIPBOARD, StrRes, sizeof(StrRes));

	err=GenericEncSign(ms,opts,StrRes);

	PGPFreeOptionList(opts);

	return err;
}

BOOL EncryptClipboard(HWND hwnd,void *PGPsc,void *PGPtls,BOOL Encrypt,BOOL Sign)
{
	PGPContextRef context;
	PRECIPIENTDIALOGSTRUCT prds;
	char *pInput;
	DWORD dwInputSize;
	UINT ClipboardFormat;
	char comment[80];
	PGPUInt32 PrefAlg;
	PGPCipherAlgorithm allowedAlgs[3];
	int nNumAlgs;
	MYSTATE *ms;
	PGPtlsContextRef tls;
	PGPError err;
	char StrRes[500];

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

				return EncryptFileList(hwnd,
					 PGPsc,PGPtls,ListHead,
					 Encrypt,Sign);
			}
		}
		CloseClipboard();
	}

	err=kPGPError_NoErr;

	context=(PGPContextRef)PGPsc;
	tls=(PGPtlsContextRef)PGPtls;

	if(IsPGPError(PGPclEvalExpired(hwnd, PGPCL_ENCRYPTSIGNEXPIRED)))
		return FALSE;

	if(!GetCommentLine(context,comment,sizeof(comment),&PrefAlg,
		allowedAlgs, &nNumAlgs))
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
			WrapIfNecessary(context,&pInput,&dwInputSize);

			prds=(PRECIPIENTDIALOGSTRUCT)
				malloc(sizeof(RECIPIENTDIALOGSTRUCT));

			if(prds)
			{
				memset(prds,0x00,sizeof(RECIPIENTDIALOGSTRUCT));

				LoadString (g_hinst, IDS_PGPKEYSELDLG, StrRes, sizeof(StrRes));

				prds->szTitle=StrRes;
				prds->dwOptions=PGPCL_ASCIIARMOR;
				prds->dwDisableFlags=PGPCL_DISABLE_ASCIIARMOR |
					PGPCL_DISABLE_WIPEORIG |
					PGPCL_DISABLE_AUTOMODE;

				ms=(MYSTATE *)malloc(sizeof(MYSTATE));
	
				if(ms)
				{
					memset(ms, 0x00, sizeof(MYSTATE) );

					LoadString (g_hinst, IDS_STARTINGWORK, ms->ws.StartText, sizeof(ms->ws.StartText));
					LoadString (g_hinst, IDS_FINISHINGUP, ms->ws.EndText, sizeof(ms->ws.EndText));
					ms->context=context;
					ms->tlsContext=tls;
					ms->pInput=pInput;
					ms->dwInputSize=dwInputSize;
					ms->Operation=MS_ENCRYPTCLIPBOARD;
					ms->prds=prds;
					ms->Encrypt=Encrypt; 
					ms->Sign=Sign;
					ms->comment=comment;
					ms->PrefAlg=PrefAlg;
					ms->allowedAlgs=allowedAlgs; 
					ms->nNumAlgs=nNumAlgs;

					if(OpenRings(hwnd,context,&(ms->KeySet)))
					{	
						err=DoWork(hwnd,ms);

						if(prds->SelectedKeySetRef)
							PGPFreeKeySet(prds->SelectedKeySetRef);

						FreePhrases(ms);
				
						if((ms->pOutput!=0) && (IsntPGPError(err)))
						{
							StoreClipboardData(hwnd, ms->pOutput, ms->dwOutputSize);
							memset(ms->pOutput,0x00,ms->dwOutputSize);
							PGPFreeData(ms->pOutput); // Since auto alloced by CDK
						}

						PGPFreeKeySet(ms->KeySet);
					}
					free(ms);
				}
				free(prds);
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
