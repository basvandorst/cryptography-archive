/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	

	$Id: regsearch.c,v 1.6 1999/02/24 12:46:23 philipn Exp $
____________________________________________________________________________*/
#pragma message( "Compiling " __FILE__ ) 
#pragma message( "Last modified on " __TIMESTAMP__ ) 

#include "Lister.h"

BOOL RegistrySearch(char *szName, char *szValue, 
					char *szSubKey, RegSearchState *pState)
{
	DWORD dwSize;
	DWORD dwType;
	DWORD dwDataSize;
	char szKeyName[256];
	char szValueName[256];
	char szTempKey[256];
	char *szPtr;
	BYTE data[256];
	FILETIME lastTime;

	if (pState->nextState)
	{
		pState->bMatch = 
			RegistrySearch(szName, szValue, szSubKey, pState->nextState);

		if (!(pState->bMatch))
		{
			szPtr = strrchr(szSubKey, '\\');
			if (szPtr != NULL)
				*szPtr = 0;
			else
				szSubKey[0] = 0;

			RegCloseKey(pState->nextState->hKey);
			free(pState->nextState);
			pState->nextState = NULL;
		}
	}
	else if (pState->bMatch)
		pState->bMatch = FALSE;

	dwSize = 256;
	dwDataSize = 256;
	while (!(pState->bMatch) && 
			(RegEnumValue(pState->hKey, pState->dwValueIndex, szValueName, 
			&dwSize, NULL, &dwType, data, &dwDataSize) == ERROR_SUCCESS))
	{
		if (dwType == REG_SZ)
		{
			if (!_stricmp(szValueName, szName) && 
				!_stricmp(szValue, data))
			{
				pState->bMatch = TRUE;
			}
		}
		
		pState->dwValueIndex++;
		dwSize = 256;
		dwDataSize = 256;
	}

	if (!(pState->bMatch))
		pState->nextState = (RegSearchState *) malloc(sizeof(RegSearchState));

	dwSize = 256;
	while (!(pState->bMatch) && 
			(RegEnumKeyEx(pState->hKey, pState->dwKeyIndex, szKeyName, 
			&dwSize, NULL, NULL, NULL, &lastTime) == ERROR_SUCCESS))
	{
		pState->nextState->nextState = NULL;
		pState->nextState->dwValueIndex = 0;
		pState->nextState->dwKeyIndex = 0;
		pState->nextState->bMatch = FALSE;

		strcpy(szTempKey, szSubKey);
		if (szSubKey[0] != 0)
			strcat(szTempKey, "\\");
		strcat(szTempKey, szKeyName);

		if (RegOpenKeyEx(pState->hKey, szKeyName, 0, KEY_READ, 
			&(pState->nextState->hKey)) == ERROR_SUCCESS)
		{
			pState->bMatch = RegistrySearch(szName, szValue, szTempKey, 
								pState->nextState);

			if (pState->bMatch)
				strcpy(szSubKey, szTempKey);
			else
			{
				RegCloseKey(pState->nextState->hKey);
				pState->nextState->hKey = NULL;
			}
		}

		pState->dwKeyIndex++;
		dwSize = 256;
	}

	if (!(pState->bMatch))
	{
		free(pState->nextState);
		pState->nextState = NULL;
	}

	return pState->bMatch;
}


void FreeRegSearchState(RegSearchState *pState)
{
	while (pState->nextState != NULL)
	{
		FreeRegSearchState(pState->nextState);
		if (pState->nextState->hKey != NULL)
			RegCloseKey(pState->nextState->hKey);
		free(pState->nextState);
	}

	return;
}
