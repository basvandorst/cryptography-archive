/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <mapi.h>

#include "..\include\errs.h"
#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\PGPRecip.h"
#include "..\include\pgpphrase.h"
#include "..\include\pgpcomdlg.h"
#include "..\include\pgpmem.h"

#include "ShellEncryptSign.h"

#define DELIMITER   ";"

UINT ShellEncryptSend(HWND hWnd, HDROP hDrop, UINT NumFiles)
{
	unsigned short FileNum = 0;
	char FilePath[_MAX_PATH + 1], *pListOfFiles = NULL,
		OutputFile[_MAX_PATH + 1];
	UINT ReturnCode = SUCCESS;
	RECIPIENTDIALOGSTRUCT RecipientDialogStruct;
	char *Passphrase;

	assert(hDrop);

	// Check for expiration
	if (PGPcomdlgEvalExpired(hWnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
	{
		PGPcomdlgErrorMessage(PGPCOMDLG_EXPIRED);
		return SUCCESS;
	}

	for(FileNum = 0;
		FileNum < NumFiles && ReturnCode == SUCCESS;
		++FileNum)
	{
		if(DragQueryFile(hDrop,
						 FileNum,
						 FilePath,
						 sizeof(FilePath)))
		{
			tmpnam(OutputFile);
			if(!strrchr(OutputFile, '.'))
				strcat(OutputFile, ".");

			strcat(OutputFile, "asc");
			if((ReturnCode = ShellEncryptSign(hWnd,
									 	  FilePath,
										  OutputFile,
										  TRUE,
										  FALSE,
										  TRUE,
										  FALSE,
										  FALSE,
										  &RecipientDialogStruct,
										  (FileNum == 0),
										  &Passphrase)) == SUCCESS)
			{										
				/*If this is the first time through, Alloc and copy:*/
				if(!pListOfFiles)
				{
					if(pListOfFiles = (char *) pgpAlloc(sizeof(char)
												* (strlen(OutputFile) + 1)))
					{
						strcpy(pListOfFiles, OutputFile);
					}
					else
					{
						ReturnCode = ERR_NO_MEM;
					}
				}
				else
				/*Not the first file; reAlloc and stick the new after the
				 *old followed by a delimiter character
				 */
				{
					/*+2 is so that we have room for the NULL and the
					 *semicolon we're going to add as a delimiter between
					 *files.
					 */
					if((pgpRealloc((void **) &pListOfFiles,
									sizeof(char) *
									(strlen(OutputFile) + 2 +
										strlen(pListOfFiles)))))
					{
						strcat(pListOfFiles, DELIMITER);
						strcat(pListOfFiles, OutputFile);
					}
					else
					{
						ReturnCode = ERR_NO_MEM;
					}
				}
			}
		}
		else
		{
			ReturnCode = ERR_VALUE_NOT_FOUND;
		}
	}


	if(pListOfFiles)
	{
		if(ReturnCode == SUCCESS)
		{
			HINSTANCE hMAPIDLL = NULL;
			LPMAPILOGON pMAPILogon = NULL;
			LPMAPISENDDOCUMENTS pMAPISendDocuments = NULL;
			LPMAPILOGOFF pMAPILogoff = NULL;
			LHANDLE				lhMAPISession = 0;

			if((hMAPIDLL = LoadLibrary("MAPI32.DLL")))
			{
				if((pMAPILogon = (LPMAPILOGON) GetProcAddress(hMAPIDLL,
															"MAPILogon")) &&
				   (pMAPILogoff = (LPMAPILOGOFF) GetProcAddress(hMAPIDLL,
															"MAPILogoff")) &&
				   (pMAPISendDocuments = (LPMAPISENDDOCUMENTS) GetProcAddress(
											   hMAPIDLL, "MAPISendDocuments")))
				{
					if (pMAPILogon((ULONG)hWnd,	// main menu window
								 	NULL,	// no profile name
									NULL,	// no password
									MAPI_LOGON_UI,		// logon dialog box
									(ULONG)0,		// reserved
									&lhMAPISession)	// session handle
							== SUCCESS_SUCCESS)
					{
						pMAPISendDocuments((ULONG) hWnd,
										   DELIMITER,
										   pListOfFiles,
										   NULL,
										   (ULONG) 0);

						pMAPILogoff((ULONG) lhMAPISession, (ULONG) hWnd,
									(ULONG) 0, (ULONG) 0);
					}
				}
				else
				{
					ReturnCode = ERR_MAPI_DLL_FAILURE;
				}
				FreeLibrary(hMAPIDLL);
			}
			else
			{
				ReturnCode = ERR_MAPI_DLL_FAILURE;
			}
		}
		pgpFree(pListOfFiles);
	}

	if(Passphrase)
		PGPFreePhrase(Passphrase);
	return(ReturnCode);
}
