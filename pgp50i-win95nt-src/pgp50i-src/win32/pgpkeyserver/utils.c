/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpkeyserversupport.h"
#include "..\include\pgpcomdlg.h"

#include "keyserver.h"
#include "resource.h"

KSERR UrlEncode(char **Dest, char *Source)
{
	KSERR ReturnCode = SUCCESS;
	char *pSource, *pDest;

	/*Absolute worst case scenario is three times the source size.  Rather
	 *than getting too precise, we'll allocate that much initially, and then
	 *realloc it down to actuality later.
	 */

	assert(Source);
	assert(Dest);

	if(Source && Dest)
	{
		if(*Dest = malloc((sizeof(char)) * ((strlen(Source) * 3) + 1)))
		{
			pSource = Source;
			pDest = *Dest;

			while(pSource && *pSource)
			{
			    /*Zeroth case:  it's an alphabetic or numeric character*/
				if(!isalpha(*pSource) &&
				    !isdigit(*pSource) &&
				   *pSource != '-')
				{
					/*First case:  Turn spaces into pluses.*/
					if(*pSource == ' ')
					{
						*pDest = '+';
					}
					else
					{
						/*This is overkill, but works for our purposes*/
						*pDest = '%';
						++pDest;
						sprintf(pDest, "%02X", *pSource);
						++pDest;
					}
				}
				else
					*pDest = *pSource;


				++pDest;
				++pSource;
			}

			*pDest = '\0';

#if 0
			if(!(*Dest = realloc(*Dest, sizeof(char) * (strlen(*Dest) + 1))))
			{
				ReturnCode = ERR_MALLOC_FAILURE;
			}
#endif
		}
		else
			ReturnCode = ERR_MALLOC_FAILURE;
	}
	else
		ReturnCode = ERR_BAD_PARAM;

	return(ReturnCode);
}

/*Stolen from PBJ*/
void ConvertKeyIDToString (LPSTR sz)
{
	INT i;
	ULONG ulSecond;

	// convert second half of keyid to little endian
	ulSecond = 0;
	for (i=0; i<4; i++)
	{
		ulSecond <<= 8;
		ulSecond |= ((ULONG)sz[i+4] & 0xFF);
	}
	// convert to string
	wsprintf (sz, "0x%08lX", ulSecond);
}

unsigned long AddKeySetWithConfirm(HWND hwnd,
								   PGPKeySet *pSourceKeySet,
								   PGPKeySet *pDestKeySet,
								   unsigned long *NumKeysFound)
{
	unsigned long ReturnCode = ERR_KEYRING_FAILURE;
	char ConfirmationBuffer[1024], TempBuffer[1024], Title[1024];
	long NumKeys;
	PGPError error;

	assert(NumKeysFound);

	/*We assume NumKeysFound is set to zero on the first call*/
	NumKeys = pgpCountKeys(pSourceKeySet);
	*NumKeysFound += NumKeys;
			
	LoadString(g_hinst, IDS_SERVER_OK_TITLE, Title, sizeof(Title));
	if(NumKeys == 1)
	{
		LoadString(g_hinst, IDS_ONE_NEW_KEY_GOTTEN, ConfirmationBuffer,
					sizeof(ConfirmationBuffer));
	}
	else
	{
		LoadString(g_hinst, IDS_MANY_NEW_KEYS_GOTTEN, TempBuffer,
					sizeof(TempBuffer));
		sprintf(ConfirmationBuffer, TempBuffer, NumKeys);
	}

#if 1	//>>> pre-existing code	
	if(MessageBox(hwnd, ConfirmationBuffer, Title, MB_YESNO | MB_ICONQUESTION)
			== IDYES)
	{
		error = pgpAddKeys(pDestKeySet, pSourceKeySet);
		if(error == PGPERR_OK)
		{
			error = pgpCommitKeyRingChanges(pDestKeySet);
			
			if(error == PGPERR_OK)
			{
				ReturnCode = SUCCESS;
			}
		}
	}
	else
		ReturnCode = SUCCESS;
#else	//>>> PGPcomdlgQueryAddKey changes ...
	error = PGPcomdlgQueryAddKeys(hwnd, pDestKeySet, pSourceKeySet);
	if(error == PGPERR_OK)
	{
		error = pgpCommitKeyRingChanges(pDestKeySet);
		
		if(error == PGPERR_OK)
		{
			ReturnCode = SUCCESS;
		}
	}
#endif	//<<< PGPcomdlgQueryAddKey changes

	return(ReturnCode);
}
