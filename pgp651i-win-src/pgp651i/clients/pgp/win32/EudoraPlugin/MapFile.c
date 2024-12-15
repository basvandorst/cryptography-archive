/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
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
	/*else
	{
		LPVOID lpMessageBuffer;

		FormatMessage(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER |
		  FORMAT_MESSAGE_FROM_SYSTEM,
		  NULL,
		  GetLastError(),
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (LPTSTR) &lpMessageBuffer,
		  0,
		  NULL );

		// now display this string
 		MessageBox(NULL, (char*)lpMessageBuffer, Path, MB_OK|MB_ICONWARNING);

		// Free the buffer allocated by the system
		LocalFree( lpMessageBuffer );

	}*/

	return pBeginning;
}

