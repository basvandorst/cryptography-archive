/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "..\include\config.h"
#include "..\include\spgp.h"
extern "C" {
#include "..\include\PGPRecip.h"
#include "..\include\pgpkeydb.h"
#include "..\include\PGPcomdlg.h"
#include "..\include\pgpphrase.h"
}
#include "..\include\pgpFileRef.h"
#include "..\include\pgpmem.h"

#include "define.h"
#include "ShellEncryptSign.h"
#include "working.h"
#include "resource.h"

extern HINSTANCE g_hmodThisDll;

UINT APIENTRY OpenFileNameHookProc (HWND hdlg, UINT uiMsg, WPARAM wParam,
									LPARAM lParam);

void GetFileRefs(char *InputFile,
				 char *OutputFile,
				 PGPFileRef **InputFileRef,
				 PGPFileRef **OutputFileRef);
BOOL SaveOutputFile(HWND hwnd,
				 char *InputFile,
				 char *TempFile,
				 char *OutputFile,
				 UINT Actions,
				 BOOL DidSign);
BOOL GetRecipientsInfo(HWND hwnd,
					   RECIPIENTDIALOGSTRUCT *prds,
					   DWORD PGPFlags,
					   DWORD DisableFlags);
BOOL GetSignPhrase(HWND hwnd,
				   char **EnteredOrCachedPhrase,
				   unsigned long *PassPhraseLen,
				   unsigned short *PassPhraseCount,
				   char *szSignKey,
				   UINT *Actions,
				   UINT uPhraseFlags);

UINT ShellEncryptSign(HWND hwnd,
					  char* szInputFile,
					  char* szOutputFileArg,
					  BOOL Encrypt,
					  BOOL Sign,
					  BOOL bDefaultASCIIArmor,
					  BOOL bAllowDetachedSig,
					  BOOL bAllowASCIIArmor,
					  PRECIPIENTDIALOGSTRUCT pRecipientDialogStruct,
					  BOOL IsFirstCallWithThisRecipientDialogStruct,
					  char **PassPhrase)
{
	UINT ReturnValue = STATUS_FAILURE, LibReturn = 0;
	char szOutputFile[MAX_PATH] = "\0", DefaultExtension[4] = "\0",
		*pOutputFile = NULL, *p;
	PGPFileRef *InputFileRef = NULL, *OutputFileRef = NULL;
	HWND hWorkingDlg = NULL;
	BOOL SaveFileNameOK = TRUE;
	char SignKey[KEYIDLENGTH + 3] = "\0";	// = {0x02, 0x00};
	BOOL UserCancel = FALSE, IsText = FALSE;
	UINT Actions = 0;
	unsigned short PassPhraseCount = 0, RetryCount = 0;
	unsigned long PassPhraseLen;

	assert(szInputFile);
	assert(pRecipientDialogStruct);

	// Check for expiration
	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
	{
		PGPcomdlgErrorMessage(PGPCOMDLG_EXPIRED);
		return STATUS_FAILURE;
	}

	p = strrchr(szInputFile, '.');
	if(p)
	{
		if(stricmp(p, ".txt") == 0)
			IsText = TRUE;
	}

	if(IsFirstCallWithThisRecipientDialogStruct && Encrypt)
	{
		DWORD PGPFlags = PGP_ENCRYPT, DisableFlags = 0;

		if(bDefaultASCIIArmor)
			PGPFlags |= PGP_ASCII_ARMOR;

		if(!bAllowASCIIArmor)
			DisableFlags |= DISABLE_ASCII_ARMOR;

		UserCancel = GetRecipientsInfo(hwnd, pRecipientDialogStruct, PGPFlags,
										DisableFlags);
	}

	if(!UserCancel)
	{
		GetFileRefs(szInputFile,
					szOutputFile,
					&InputFileRef,
					&OutputFileRef);

		do
		{
			if(Sign)
			{
				if(!Encrypt)
				{
					UserCancel = GetSignPhrase(hwnd,
											   PassPhrase,
											   &PassPhraseLen,
											   &PassPhraseCount,
											   SignKey,
											   &Actions,
											   0);
				}
				else
				{
					BOOL uPhraseFlags = 0;
					if (pRecipientDialogStruct->dwPGPFlags &
							PGP_ATLEASTONERSAKEY)
					{
						uPhraseFlags |= PGPCOMDLG_RSAENCRYPT;
					}
										
					UserCancel = GetSignPhrase(hwnd,
											   PassPhrase,
											   &PassPhraseLen,
											   &PassPhraseCount,
											   SignKey,
											   NULL,
											   uPhraseFlags);
					if ((pRecipientDialogStruct->dwPGPFlags & PGP_ASCII_ARMOR)
								== PGP_ASCII_ARMOR)
					{
						Actions |= PGPCOMDLG_ASCIIARMOR;
					}
				}
			}
			else
			{
				if(*PassPhrase)
				{
					PGPFreePhrase(*PassPhrase);
					*PassPhrase = NULL;
				}

				PassPhraseLen = 0;
				*PassPhrase = NULL;
				if ((pRecipientDialogStruct->dwPGPFlags & PGP_ASCII_ARMOR)
							== PGP_ASCII_ARMOR)
				{
					Actions |= PGPCOMDLG_ASCIIARMOR;
				}
			}

			if(!UserCancel)
			{
				hWorkingDlg = WorkingDlgProcThread(g_hmodThisDll,
												   hwnd,
												   "Encrypting",
												   szInputFile);

				if(Encrypt)
				{
					LibReturn = SimplePGPEncryptFileX(hwnd,
							InputFileRef,
							OutputFileRef,
							Sign,
							FALSE,			// Wipe It?
							(pRecipientDialogStruct->dwPGPFlags
									& PGP_ASCII_ARMOR) == PGP_ASCII_ARMOR,
							IsText,			// Text Mode = On
							FALSE,			// Not IDEA only
							TRUE,			// Use untrusted keys
							FALSE,			// Not for your eyes only
							pRecipientDialogStruct->szRecipients,
							SignKey,
							strlen(SignKey),
							*PassPhrase,
							PassPhraseLen,
							NULL, //IDEA password
							0,
							NULL,
							NULL,
							FALSE, //Not local Encode??
							(SPGPProgressCallBack) WorkingCallback,
							hWorkingDlg);


				}
				else
				{
					LibReturn = SimplePGPSignFileX(	
							hwnd,			// handle
							InputFileRef,
							OutputFileRef,
							(pRecipientDialogStruct->dwPGPFlags
										& PGP_ASCII_ARMOR) == PGP_ASCII_ARMOR,
							IsText,			// TextMode = On
							(Actions & PGPCOMDLG_DETACHEDSIG)
													== PGPCOMDLG_DETACHEDSIG,
							(Actions & PGPCOMDLG_ASCIIARMOR)
													== PGPCOMDLG_ASCIIARMOR,
							SignKey,		// Signer Id (char*)
							strlen(SignKey),	// strlen(Signer Id)
							*PassPhrase,
							PassPhraseLen,
							NULL,
							FALSE,
							(SPGPProgressCallBack) WorkingCallback,
							NULL);

				}

				PostMessage(hWorkingDlg, WM_QUIT, 0, 0);
			}

			switch(LibReturn)
			{
				case 0:
					ReturnValue = STATUS_SUCCESS;
					break;

				case SIMPLEPGP_RNGNOTSEEDED:
					if(PGPcomdlgRandom (hwnd, SimplePGPRandomNeeded(hwnd))
							== PGPCOMDLG_CANCEL)
						UserCancel = TRUE;
					break;

				case SIMPLEPGP_BADKEYPASSPHRASE:
					break;  /*Do nothing*/

				default:
					PGPcomdlgErrorMessage(LibReturn);
					++RetryCount;
					break;

			}
		}while((LibReturn == SIMPLEPGP_BADKEYPASSPHRASE ||
				LibReturn == SIMPLEPGP_RNGNOTSEEDED) &&
				!UserCancel &&
				RetryCount < MAX_RETRIES);

		pgpFreeFileRef(InputFileRef);
		pgpFreeFileRef(OutputFileRef);

		if(!UserCancel)
		{
			UserCancel = SaveOutputFile(hwnd,
										szInputFile,
										szOutputFile,
										szOutputFileArg,
										Actions,
										Sign);
		}
	}

	return ReturnValue;
}

BOOL GetRecipientsInfo(HWND hwnd,
					   RECIPIENTDIALOGSTRUCT *prds,
					   DWORD PGPFlags,
					   DWORD DisableFlags)
{
	BOOL UserCancel = FALSE;
	UINT RecipientReturn = RCPT_DLG_ERROR_SUCCESS;

	assert(prds);

	do
	{
		memset(prds, '\0', sizeof(RECIPIENTDIALOGSTRUCT) );

		prds->sVersion = RCPT_DLG_VERSION;
		prds->hwndParent = hwnd;
		prds->szTitle = "PGP - Key Selection Dialog";
		prds->dwPGPFlags = PGPFlags;
		prds->dwDisableFlags = DisableFlags;

		RecipientReturn = PGPRecipientDialog(prds);

		if(RecipientReturn == RCPT_DLG_ERROR_SUCCESS && !prds->dwNumRecipients)
			MessageBox(NULL,	"In order to Encrypt you must choose\n"
								"at least one recipient.", "No Recipients",
								MB_OK|MB_ICONWARNING);
	}while(RecipientReturn == RCPT_DLG_ERROR_SUCCESS &&
		   !prds->dwNumRecipients);

	if(RecipientReturn != RCPT_DLG_ERROR_SUCCESS)
		UserCancel = TRUE;

	return(UserCancel);
}

BOOL GetSignPhrase(HWND hwnd,
				   char **EnteredOrCachedPhrase,
				   unsigned long *PassPhraseLen,
				   unsigned short *PassPhraseCount,
				   char *szSignKey,
				   UINT *Actions,
				   UINT uFlags)
{
	BOOL UserCancel = FALSE;
	BOOL ForceEntry = FALSE;
	char PhraseTitle[256];

	assert(EnteredOrCachedPhrase);
	assert(PassPhraseLen);
	assert(szSignKey);

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
		LoadString(g_hmodThisDll, IDS_ENTER_PHRASE, PhraseTitle,
					sizeof(PhraseTitle));

	if(PGPGetSignCachedPhrase (hwnd,
							   PhraseTitle,
							   ForceEntry,
							   EnteredOrCachedPhrase,
							   szSignKey,
							   NULL,
							   Actions,
							   uFlags) != PGPPHRASE_OK)
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

void GetFileRefs(char *InputFile,
				 char *OutputFile,
				 PGPFileRef **InputFileRef,
				 PGPFileRef **OutputFileRef)
{
	char szTempPath[MAX_PATH];

	assert(InputFile);
	assert(OutputFile);
	assert(InputFileRef);
	assert(OutputFileRef);

	GetTempPath (sizeof(szTempPath), szTempPath);
	if (!GetTempFileName (szTempPath, "PGP", 0, OutputFile))
	{
		strcpy(OutputFile, "\\pgpfoo");
	}

	*InputFileRef = pgpNewFileRefFromFullPath(InputFile);
	*OutputFileRef = pgpNewFileRefFromFullPath(OutputFile);
	assert(*OutputFileRef);
}

UINT APIENTRY OpenFileNameHookProc (HWND hdlg, UINT uiMsg, WPARAM wParam,
									LPARAM lParam) {
	switch (uiMsg) {
	case WM_INITDIALOG :
		SetForegroundWindow (GetParent (hdlg));
		break;
	}
	return 0;
}

BOOL MyCopyFile (char *szDes, char* szSrc) {
	if (!CopyFile (szDes, szSrc, FALSE))
	{
		MessageBox (NULL,
			"The selected output file could not be written.\n"
			"The drive may be full or write-protected.",
			"PGP File Write Error",
			MB_OK | MB_ICONHAND | MB_SETFOREGROUND);
		return FALSE;
	}
	return TRUE;
}

BOOL SaveOutputFile(HWND hwnd,
				 char *InputFile,
				 char *TempFile,
				 char *OutputFile,
				 UINT Actions,
				 BOOL DidSign)
{
	char *p;
	int FileStart = 0, FileExtensionStart = 0;
	char FinalFile[_MAX_PATH] = "\0";
	BOOL UserCancel = FALSE;

	assert(InputFile);
	assert(TempFile);

	if(!OutputFile || (OutputFile && !*OutputFile))
	{
		char Title[64] = "Save Encrypted File As";
		char DefaultExtension[_MAX_PATH] = ".pgp";

		if(DidSign)
			strcpy(Title, "Save Signed File As");

		strcpy(FinalFile, InputFile);

		if((Actions & PGPCOMDLG_DETACHEDSIG) == PGPCOMDLG_DETACHEDSIG)
		{
			strcpy(DefaultExtension, ".sig");
			strcpy(Title, "Save Seperate Signature File As");
		}
		else
		{
			if((Actions & PGPCOMDLG_ASCIIARMOR) == PGPCOMDLG_ASCIIARMOR)
			{
				strcpy(DefaultExtension, ".asc");
			}
		}
	
		strcat(FinalFile, DefaultExtension);

		do {

			if((p = strrchr(FinalFile, '\\')))
				FileStart = p - FinalFile + 1;

			if((p = strrchr(FinalFile, '.')))
				FileExtensionStart = p - FinalFile + 1;

			OPENFILENAME SaveFileName = {
							sizeof(SaveFileName),
							hwnd,
							NULL,
							"PGP Files (*.asc;*.pgp)\0*.asc;*.pgp\0"
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
							OFN_NOREADONLYRETURN | OFN_NOCHANGEDIR,
							FileStart,
							FileExtensionStart,
							DefaultExtension,
							NULL,
							OpenFileNameHookProc,
							NULL};

			UserCancel = !GetSaveFileName(&SaveFileName);

		}
		while (!UserCancel && !MyCopyFile(TempFile, FinalFile));
	}
	else
	{
		strcpy(FinalFile, OutputFile);
		MyCopyFile(TempFile, FinalFile);
	}

	_unlink(TempFile);

	return(UserCancel);
}
