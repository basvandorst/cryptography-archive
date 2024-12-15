/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <assert.h>

#include "..\include\config.h"
#include "..\include\spgp.h"
extern "C" {
#include "..\include\pgpkeydb.h"
#include "..\include\pgpcomdlg.h"
}

#include "define.h"
#include "working.h"
#include "resultsdlg.h"

UINT APIENTRY OpenFileNameHookProc (HWND hdlg, UINT uiMsg, WPARAM wParam,
									LPARAM lParam);

UINT ShellVerifySeperateSig(HWND hwnd,
							char *szFilePath,
							HMODULE hmodThisDll)
{
	UINT ReturnCode = STATUS_FAILURE, VerifyResult = 0;
	int SignatureStatus = 0;
	char OriginalFile[_MAX_PATH], *p, DefaultExtension[_MAX_PATH] = ".txt";
	int FileStart, FileExtensionStart;
	HWND hWorkingDlg = NULL;

	assert(szFilePath);
	assert(hmodThisDll);

	// Check for expiration
	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED))
	{
		PGPcomdlgErrorMessage(PGPCOMDLG_EXPIRED);
		return STATUS_FAILURE;
	}

	strcpy(OriginalFile, szFilePath);

	if((p = strrchr(OriginalFile, '.')))
	{
		*p = '\0';
		if((p = strrchr(OriginalFile, '.')))
		{
			strcpy(DefaultExtension, p);
		}
	}

	if((p = strrchr(OriginalFile, '\\')))
		FileStart = p - OriginalFile + 1;

	if((p = strrchr(OriginalFile, '.')))
		FileExtensionStart = p - OriginalFile + 1;

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
						DefaultExtension,
						NULL,
						OpenFileNameHookProc,
						NULL};

	if(GetOpenFileName(&OriginalFileName))
	{
		PGPFileRef *OriginalFileRef = NULL, *SignatureFileRef = NULL;
		int SignatureSTatus = 0;
		char Signer[1024], SignatureDate[1024], SignatureResultBuffer[1024];

		if((OriginalFileRef = pgpNewFileRefFromFullPath(OriginalFile)))
		{
			if((SignatureFileRef = pgpNewFileRefFromFullPath(szFilePath)))
			{
				hWorkingDlg = WorkingDlgProcThread(hmodThisDll,
												   hwnd,
												   "Verifying Signature",
												   szFilePath);

				VerifyResult = SimplePGPVerifyDetachedSignatureFileX(hwnd,
									  OriginalFileRef,
									  SignatureFileRef,
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

				PostMessage(hWorkingDlg, WM_QUIT, 0, 0);

				if(VerifyResult == KERNEL_EXIT_OK)
				{
					ReturnCode = STATUS_SUCCESS;

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
				else
					PGPcomdlgErrorMessage(VerifyResult);
			}
		}
	}
	else
		ReturnCode = STATUS_FAILURE;

	return(ReturnCode);
}
