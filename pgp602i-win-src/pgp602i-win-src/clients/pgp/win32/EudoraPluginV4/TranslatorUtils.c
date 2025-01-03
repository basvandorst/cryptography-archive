/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: TranslatorUtils.c,v 1.2.8.1 1998/11/12 03:11:51 heller Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <time.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpHash.h"

// Eudora Headers
#include "emssdk/ems-win.h"

// Project Headers
#include "MapFile.h"
#include "BlockUtils.h"
#include "TranslatorUtils.h"


long 
ExtractAddressesFromMailHeader(	emsHeaderDataP header, 
								char*** addresses)
{
	emsAddressP	to = header->to;
	emsAddressP	cc = header->cc;
	emsAddressP	bcc = header->bcc;
	long numRecipients = 0;
	long index = 0;

	assert(header);
	assert(addresses);

	*addresses = NULL;

	// first count all the addresses
	while( to && to->address )
	{
		numRecipients++;
		to = to->next;
	}

	while( cc && cc->address )
	{
		numRecipients++;
		cc = cc->next;
	}

	while( bcc && bcc->address )
	{
		numRecipients++;
		bcc = bcc->next;
	}

	if(numRecipients)
	{
		to = header->to;
		cc = header->cc;
		bcc = header->bcc;

		// allocate address array
		*addresses = (char**) malloc(numRecipients * sizeof(char*) );

		if(*addresses)
		{
			// fill the array
			while( to && to->address )
			{
				(*addresses)[index] = 
								(char*) malloc(strlen(to->address) + 1); 

				if((*addresses)[index])
				{
					strcpy((*addresses)[index++], to->address);
				}

				to = to->next;
			}

			while( cc && cc->address )
			{
				(*addresses)[index] = 
								(char*) malloc(strlen(cc->address) + 1); 

				if((*addresses)[index])
				{
					strcpy((*addresses)[index++], cc->address);
				}

				cc = cc->next;
			}

			while( bcc && bcc->address )
			{
				(*addresses)[index] = 
								(char*) malloc(strlen(bcc->address) + 1); 

				if((*addresses)[index])
				{
					strcpy((*addresses)[index++], bcc->address);
				}

				bcc = bcc->next;
			}
		}

		assert(index == numRecipients);
	}

	return numRecipients;
}

void 
FreeRecipientList(char** Recipients, 
				  unsigned long  numRecipients)
{
	unsigned long index = 0;

	//assert(Recipients);

	if(Recipients)
	{
		for(index = 0; index < numRecipients; index++)
		{
			if( Recipients[index] )
			{
				// free each string
				free(Recipients[index]);
			}
		}

		free(Recipients);
	}
}

void StripMIMEHeader(const char* file)
{
	HANDLE	hTempFile = NULL;
	char szTempFile[MAX_PATH];
	BOOL success = FALSE;

	assert(file);

	strcpy(szTempFile, file);
	strcat(szTempFile, ".tmp");

	hTempFile = CreateFile(	szTempFile,
							GENERIC_WRITE|GENERIC_READ,
							0, 
							NULL, 
							CREATE_ALWAYS, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL);

	// did we successfully open the file
	if(	( INVALID_HANDLE_VALUE != hTempFile ) )
	{
		char* input = NULL;
		DWORD size = 0;

		input = MapFile(file, &size);

		if(input)
		{
			int count = 3;
			char* pointer = input;

			while(count && *pointer)
			{
				pointer = strstr(pointer, "\r\n");
				pointer	+= 2;
				count--;
			}

			success = WriteFile(hTempFile, 
								pointer, 
								strlen(pointer), 
								&size, 
								NULL);

			free(input);
		}

		// clean up
		CloseHandle(hTempFile);

		if(success)
		{
			CopyFile(szTempFile, file, FALSE); 
		}

		DeleteFile(szTempFile);
	}

}


