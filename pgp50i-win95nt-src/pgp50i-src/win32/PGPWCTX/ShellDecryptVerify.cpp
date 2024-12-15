/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <assert.h>

#define PGPLIB

#include "..\include\config.h"
#include "..\include\spgp.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpphrase.h"
extern "C" {
#include "..\include\PGPRecip.h"
#include "..\include\PGPcomdlg.h"
}
#include "..\include\pgpmem.h"
#include "..\include\errs.h"

#include "define.h"
#include "ResultsDlg.h"
#include "working.h"
#include "resource.h"
#include "shelladdkey.h"

extern HINSTANCE g_hmodThisDll;

UINT APIENTRY OpenFileNameHookProc (HWND hdlg, UINT uiMsg, WPARAM wParam,
									LPARAM lParam);

void GetFileRefs(char *InputFile,
				 char *OutputFile,
				 PGPFileRef **InputFileRef,
				 PGPFileRef **OutputFileRef);

BOOL GetOriginalFileRef(HWND hwnd,
						char *InputFile,
						PGPFileRef **OriginalFileRef);

BOOL  SaveDecryptedOutputFile(HWND hwnd,
							  char *InputFile,
							  char *OutputFile,
							  char *TempFile,
							  BOOL IsEncrypted);

BOOL GetDecryptPhrase(HWND hwnd,
					   char **EnteredOrCachedPhrase,
					   unsigned long *PassPhraseLen,
					   unsigned short *PassPhraseCount,
					   char *KeyForPhrase);

extern "C" UINT PGPExport ShellDecryptVerify(HWND hwnd,
						char* InputFile,
						BOOL JustVerify,
						BOOL PrimarilyAddKeys,
						char **PassPhrase,
						BOOL IsFirstCallWithThisPassphrase)
{
	char TempFile[MAX_PATH], status[32] = "Decrypting/Verifying";
	PGPFileRef *InputFileRef = NULL, *OutputFileRef = NULL,
				*OriginalFileRef = NULL;
	BOOL IsDetachedSig = FALSE, IsEncrypted = FALSE, UserCancel = FALSE;
	BOOL IsConventional = FALSE;
	Boolean IsEyesOnly = FALSE;
	UINT LibReturn = PGPERR_OK, ReturnValue = STATUS_FAILURE;
	INT SignatureStatus = 0;
	unsigned short PassPhraseCount = 0, RetryCount = 0;
	unsigned long PassPhraseLen = 0;
	HWND hWorkingDlg;
	char Signer[1024], SignatureDate[1024], SignatureResultBuffer[1024];
	char *pPhraseToUse;
	UINT AddKeyReturn;

	assert(InputFile);
	assert(PassPhrase);

	// Check for expiration
	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED))
	{
		PGPcomdlgErrorMessage(PGPCOMDLG_EXPIRED);
		return STATUS_FAILURE;
	}

	if(!*PassPhrase)
		pPhraseToUse = "\0";
	else
		pPhraseToUse = *PassPhrase;


	AddKeyReturn = ShellAddKey(hwnd, InputFile, PrimarilyAddKeys);

	GetFileRefs(InputFile,
				TempFile,
				&InputFileRef,
				&OutputFileRef);

	do
	{
		/*First time through, we assume this is a non-detached signature, and
		 *that they don't want to JustVerify.  For program simplicity, we
		 *actually do write out the verified sig file; if they select
		 *JustVerify, we just don't pop the save-as dialog, and merely delete
		 *the verified sig when we're done.
		 */

		if(IsDetachedSig)
		{
			UserCancel = GetOriginalFileRef(hwnd,
											InputFile,
											&OriginalFileRef);
			strcpy(status, "Verifying");
		}

		if(IsEncrypted && !UserCancel)
		{
			/*If it IsEncrypted (this is known because a previous call to
			 *verify has said "hey, this is the wrong passphrase!"), then we
			 *need to ask the user for her phrase:
			 */
			if (IsConventional)
				UserCancel = GetDecryptPhrase(hwnd,
										   PassPhrase,
										  &PassPhraseLen,
										  &PassPhraseCount,
										   " decryption :");
			else
				UserCancel = GetDecryptPhrase(hwnd,
										  PassPhrase,
										  &PassPhraseLen,
										  &PassPhraseCount,
										   " your private key :");
			pPhraseToUse = *PassPhrase;
			strcpy(status, "Decrypting");
		}

		if(!UserCancel)
		{
			hWorkingDlg = WorkingDlgProcThread(g_hmodThisDll,
											    hwnd,
											    status,
											    InputFile);
			if(!IsDetachedSig)
			{
				LibReturn = SimplePGPReceiveFileX(hwnd,
									    InputFileRef,
									    OutputFileRef,	 	 	
									    pPhraseToUse,
									    PassPhraseLen,
									    (int *) &SignatureStatus,
									    NULL,
									    0,
									    Signer,
									    sizeof(Signer),
									    (UCHAR*)SignatureDate,
									    sizeof(SignatureDate),
									    &IsEyesOnly,
									    NULL,
									    NULL,
									    FALSE,
									    (SPGPProgressCallBack) WorkingCallback,
									    hWorkingDlg);
			}
			else
			{
				LibReturn  = SimplePGPVerifyDetachedSignatureFileX(hwnd,
									    OriginalFileRef,
									    InputFileRef,
									   &SignatureStatus,
									    NULL,
									   0,
									    Signer,
									    sizeof(Signer),
									    (UCHAR*) SignatureDate,
									    sizeof(SignatureDate),
									    NULL,
									    FALSE,
									    (SPGPProgressCallBack) WorkingCallback,
									    hWorkingDlg);
			}

			PostMessage(hWorkingDlg, WM_QUIT, 0, 0);

			switch(LibReturn)
			{
				case PGPERR_OK:
					if(SignatureStatus != KERNEL_EXIT_OK_NOSIG &&
					   SignatureStatus != KERNEL_EXIT_OK)
					{
						if(PGPcomdlgTranslateSignatureResult(
											 SignatureStatus,
											 Signer,
											 SignatureDate,
											 SignatureResultBuffer,
											 sizeof(SignatureResultBuffer)))
						{
							MessageBox(hwnd,
									    SignatureResultBuffer,
									    "Signature Result",
									    MB_OK | MB_ICONINFORMATION);
						}
					}
					ReturnValue = STATUS_SUCCESS;
					break;

				case SIMPLEPGP_BADKEYPASSPHRASE:
					IsEncrypted = TRUE;
					IsConventional = FALSE;
					break;

				case SIMPLEPGP_BADCONVENTIONALPASSPHRASE:
					IsEncrypted = TRUE;
					IsConventional = TRUE;
					break;

				case SIMPLEPGP_DETACHEDSIGNATUREFOUND:
					IsDetachedSig = TRUE;
					break;

				case SIMPLEPGP_NOPGPDATA:
				case SIMPLEPGPRECEIVEFILE_ISKEY:
					if(!PrimarilyAddKeys && (AddKeyReturn == STATUS_FAILURE))
						PGPcomdlgErrorMessage(LibReturn);
					else
						UserCancel = TRUE; /*Fall out of the loop*/
					break;

				default:
					if(!PrimarilyAddKeys && (AddKeyReturn == STATUS_FAILURE))
					{
						PGPcomdlgErrorMessage(LibReturn);
						++RetryCount;
					}
					else
						UserCancel = TRUE; /*Fall out of the loop*/
					break;
			}
		}
	}while((LibReturn == SIMPLEPGP_DETACHEDSIGNATUREFOUND ||
		    LibReturn == SIMPLEPGP_BADKEYPASSPHRASE ||
			LibReturn == SIMPLEPGP_BADCONVENTIONALPASSPHRASE) &&
			!UserCancel &&
			RetryCount < MAX_RETRIES);

	if(InputFileRef)
		pgpFreeFileRef(InputFileRef);

	if(OutputFileRef)
		pgpFreeFileRef(OutputFileRef);

	if(OriginalFileRef)
		pgpFreeFileRef(OriginalFileRef);

	if(LibReturn == PGPERR_OK && (!UserCancel && !JustVerify && !IsDetachedSig))
	{
		UserCancel = SaveDecryptedOutputFile(hwnd,
											 InputFile,
											 NULL,
											 TempFile,
											 IsEncrypted);
	}
	else
		_unlink(TempFile);

	return(ReturnValue);
}

/*Returns TRUE if the user hit cancel*/
BOOL GetDecryptPhrase(HWND hwnd,
					  char **EnteredOrCachedPhrase,
					  unsigned long *PassPhraseLen,
					  unsigned short *PassPhraseCount,
					  char *KeyForPhrase)
{
	BOOL UserCancel = FALSE;
	BOOL ForceEntry = FALSE;
	char PhraseTitle[256];

	assert(EnteredOrCachedPhrase);
	assert(PassPhraseLen);

	if(*EnteredOrCachedPhrase)
	{
		PGPFreePhrase(*EnteredOrCachedPhrase);
		*EnteredOrCachedPhrase = NULL;
	}

	if(PassPhraseCount && *PassPhraseCount > 0)
	{
		LoadString(g_hmodThisDll, IDS_WRONG_PHRASE, PhraseTitle,
					sizeof(PhraseTitle));
		ForceEntry = TRUE;
	}
	else
	{
		LoadString(g_hmodThisDll, IDS_ENTER_PHRASE, PhraseTitle,
					sizeof(PhraseTitle));
		lstrcat (PhraseTitle, KeyForPhrase);
	}


	if(PGPcomdlgGetPhrase(hwnd,
						  PhraseTitle,
						  EnteredOrCachedPhrase,
						  NULL, NULL, NULL, NULL, 0) != PGPPHRASE_OK)
	{
		UserCancel = TRUE;
	}
	else
	{
		*PassPhraseLen = (unsigned long) strlen(*EnteredOrCachedPhrase);
	}
	if(PassPhraseCount)
		++(*PassPhraseCount);

	return(UserCancel);
}

BOOL  SaveDecryptedOutputFile(HWND hwnd,
							  char *InputFile,
							  char *OutputFile,
							  char *TempFile,
							  BOOL IsEncrypted)
{
	char *p;
	int FileStart = 0, FileExtensionStart = 0;
	char FinalFile[_MAX_PATH] = "\0";
	BOOL UserCancel = FALSE;

	assert(InputFile);
	assert(TempFile);

	if(!OutputFile || (OutputFile && !*OutputFile))
	{
		char Title[64] = "Save Verified File As";
		char DefaultExtension[_MAX_PATH] = "\0";

		if(IsEncrypted)
			strcpy(Title, "Save Decrypted File As");

		strcpy(FinalFile, InputFile);

		p = strrchr(FinalFile, '.');
		if(p)
		{
			*p = '\0';
		}

		if((p = strrchr(FinalFile, '\\')))
			FileStart = p - FinalFile + 1;

		if((p = strrchr(FinalFile, '.')))
			FileExtensionStart = p - FinalFile + 1;

		OPENFILENAME SaveFileName = {
							sizeof(SaveFileName),
							hwnd,
							NULL,
							"All Files (*.*)\0*.*\0\0",
							NULL,
							0,
							1,
							FinalFile,
							MAX_PATH,
							NULL,
							0,
							NULL,
							Title,
							OFN_EXPLORER | OFN_OVERWRITEPROMPT |
							OFN_HIDEREADONLY | OFN_ENABLEHOOK |
							OFN_NOREADONLYRETURN,
							FileStart,
							FileExtensionStart,
							DefaultExtension,
							NULL,
							OpenFileNameHookProc,
							NULL};

		UserCancel = !GetSaveFileName(&SaveFileName);
	}
	else
	{
		strcpy(FinalFile, OutputFile);
	}

	if(!UserCancel)
	{
		CopyFile(TempFile, FinalFile, FALSE);
		_unlink(TempFile);
	}
	else
	{
		_unlink(TempFile);
	}

	return(UserCancel);
}

BOOL GetOriginalFileRef(HWND hwnd,
						char *InputFile,
						PGPFileRef **OriginalFileRef)
{
	char OriginalFile[MAX_PATH + 1], *p;
	char *pDefaultExtension;
	unsigned short FileStart, FileExtensionStart;
	BOOL UserCancel = FALSE;

	assert(InputFile);
	assert(OriginalFileRef);

	strcpy(OriginalFile, InputFile);

	if((p = strrchr(OriginalFile, '\\')))
		FileStart = p - OriginalFile + 1;
	else
		FileStart = 0;

	// Get rid of the .sig extension

	if((p = strrchr(OriginalFile, '.')))
	{
		*p = '\0';
	}

	// Don't add any default extensions to the file

	pDefaultExtension=0;
	FileExtensionStart=strlen(OriginalFile);

	OPENFILENAME OriginalFileName = {
						sizeof(OriginalFileName),
						hwnd,
						NULL,
						"All Files (*.*)\0*.*\0\0",
						NULL,
						0,
						1,
						OriginalFile,
						MAX_PATH,
						NULL,
						0,
						NULL,
						"Select Signed File",
						OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY |
							OFN_FILEMUSTEXIST | OFN_EXPLORER |
							OFN_PATHMUSTEXIST | OFN_ENABLEHOOK,
						FileStart,
						FileExtensionStart,
						pDefaultExtension,
						NULL,
						OpenFileNameHookProc,
						NULL};


	if(GetOpenFileName(&OriginalFileName))
	{
		*OriginalFileRef = pgpNewFileRefFromFullPath(OriginalFile);
	}
	else
		UserCancel = TRUE;

	return(UserCancel);
}
