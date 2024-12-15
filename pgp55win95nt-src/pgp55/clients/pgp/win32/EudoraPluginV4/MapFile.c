/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	MapFile.c,v 1.3 1997/07/16 05:51:49 elrod Exp
____________________________________________________________________________*/

// System Headers
#include <windows.h>

char* MapFile(const char* Path, DWORD* FileSize)
{
	char* cp = NULL;
	char* pBeginning = NULL;
	HANDLE fh = NULL;
	DWORD Bytes;

	fh = CreateFile(Path, 
					GENERIC_READ, 
					FILE_SHARE_READ|FILE_SHARE_WRITE, 
					NULL, 
					OPEN_EXISTING, 
					FILE_ATTRIBUTE_NORMAL,
					NULL);

	if(fh != INVALID_HANDLE_VALUE)
	{
		*FileSize =  GetFileSize(fh, NULL);

		if(*FileSize > 0)
		{
			pBeginning = (char*) malloc (*FileSize + 1);

			if(!pBeginning)
			{
				return NULL;
			}

			cp = pBeginning;

			ReadFile(fh, cp, *FileSize, &Bytes, NULL);

			*(cp+(*FileSize)) = 0x00;	
		}
		else
		{
			pBeginning = (char*) malloc (1);

			if(pBeginning)
			{
				*pBeginning = 0x00;
			}
		}

		CloseHandle( fh );
	}

	return pBeginning;
}
