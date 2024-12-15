/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	

	$Id: remove95.c,v 1.7.4.1 1999/06/14 05:26:54 philipn Exp $
____________________________________________________________________________*/
#pragma message( "Compiling " __FILE__ ) 
#pragma message( "Last modified on " __TIMESTAMP__ ) 

#include <windows.h>
#include "Define.h"
#include "globals.h"
#include "lister.h"

/*
 *  Removes any existings PGPnet network components 	
 */
BOOL Remove95(char *Key, int Flag)
{
	HKEY	hRootKey;
	HKEY	hDeleteKey;
	HKEY	hAdapterlist;
	UINT	nValues;
	UINT	nSubkey;
	DWORD	nMaxValueNameLen;
	DWORD	nMaxDataNameLen;
	DWORD	nValueNameLen;
	DWORD	nSubkeyNameLen;
	DWORD	dwKeyIndex;
	LPSTR	pValueBuffer;
	BYTE	DataBuffer[256];
	BYTE	pValue[256];
	LPSTR	pSubKeyValueBuffer;
//	LPSTR	CurrentSecuredCard;
	LPSTR	CurrentSecuredCardBindings;
	LPSTR	CurrentSecuredCardBindingsAfterFirstSlash;
	DWORD	dwValueIndex;
	DWORD	dwBufSize;
	DWORD	nDataNameLen;
	DWORD	dwType;
	DWORD	dwSize;
	UINT	RC;
	BOOL	bDelete;


	/*copy real cards old bindings back*/
	if ((RegOpenKey(HKEY_LOCAL_MACHINE, PATHTOADAPTERLIST, &hAdapterlist)
					== ERROR_SUCCESS) && (Flag))
	{
//		CurrentSecuredCard = malloc(nSubkeyNameLen + 1);
//		ZeroMemory(CurrentSecuredCard, (nSubkeyNameLen + 1));
		dwSize = sizeof(pValue);
		RegQueryValueEx(hAdapterlist, "FullSecuredCard", 0, &dwType, pValue, &dwSize);
		dwSize = sizeof(pValue);
		RegQueryValueEx(hAdapterlist, pValue, 0, &dwType, pValue, &dwSize);
		/*open key of the current secured card*/
		RegCloseKey(hAdapterlist);
		

		/*pvalue has the path to the secured card*/
		CurrentSecuredCardBindings = malloc(MAX_KEY_LENGTH);

		ZeroMemory(CurrentSecuredCardBindings, MAX_KEY_LENGTH);

		strcpy(CurrentSecuredCardBindings, "HKEY_LOCAL_MACHINE");
		strcat(CurrentSecuredCardBindings, "\\");
		strcat(CurrentSecuredCardBindings, pValue);
		strcat(CurrentSecuredCardBindings, "\\Bindings");

		CopyRegistryKeyValues(FULLBACKUP95BINDINGSKEY, 
							CurrentSecuredCardBindings);
	
		CurrentSecuredCardBindingsAfterFirstSlash = GetTxtAfterFirstSlash(CurrentSecuredCardBindings);
		
		RegDeleteKey(HKEY_LOCAL_MACHINE, BACKUP95BINDINGSKEY);
		if (RegOpenKey(HKEY_LOCAL_MACHINE, CurrentSecuredCardBindingsAfterFirstSlash, &hDeleteKey)
					== ERROR_SUCCESS)
		{
			RegDeleteValue(hDeleteKey, "PGPMAC\\0000");
			/*Temp fix for addnew hardware wizard
			popping up problem*/
			RegDeleteValue(hDeleteKey, "PGPMAC\\000");
			RegDeleteValue(hDeleteKey, "PGPMAC\\0001");
			RegDeleteValue(hDeleteKey, "PGPMAC\\0002");
			RegDeleteValue(hDeleteKey, "PGPMAC\\0003");
			RegDeleteValue(hDeleteKey, "PGPMAC\\0004");
			RegDeleteValue(hDeleteKey, "PGPMAC\\0005");
			RegCloseKey(hDeleteKey);
		}

//		free(CurrentSecuredCard);
		free(CurrentSecuredCardBindings);
		free(CurrentSecuredCardBindingsAfterFirstSlash);
		RegCloseKey(hAdapterlist);
	}
	

	RegDeleteKey(HKEY_LOCAL_MACHINE, "Enum\\Network\\PGPMAC");

	if (RegOpenKey(HKEY_LOCAL_MACHINE, Key, &hRootKey) == ERROR_SUCCESS)
	{
		/*Determine number of keys to enumerate*/
		RegQueryInfoKey(hRootKey, NULL, NULL, NULL, &nSubkey, 
					&nSubkeyNameLen, NULL, NULL, NULL,
					NULL, NULL, NULL);
		/*Retrieve Registry values*/
		pSubKeyValueBuffer = malloc(nSubkeyNameLen + 1);
		for (dwKeyIndex = 0; dwKeyIndex < nSubkey; dwKeyIndex++)
		{
			bDelete = FALSE;
			ZeroMemory(pSubKeyValueBuffer,(nSubkeyNameLen + 1)); 
			dwBufSize = (nSubkeyNameLen +1);
			if ((RC = RegEnumKey(hRootKey, dwKeyIndex, pSubKeyValueBuffer, dwBufSize)) 
								== ERROR_SUCCESS)
			{
				dwBufSize = (nSubkeyNameLen +1);
				if (RegOpenKey(hRootKey, pSubKeyValueBuffer, &hDeleteKey)
								== ERROR_SUCCESS)
				{
 					/*Determine number of values to enumerate*/
					RC = RegQueryInfoKey(hDeleteKey, NULL, NULL, NULL, NULL, 
										NULL, NULL, &nValues, &nMaxValueNameLen,
										&nMaxDataNameLen, NULL, NULL);

					/*Retrieve Registry values*/
					pValueBuffer = malloc(nMaxValueNameLen + 1);

					for (dwValueIndex = 0; dwValueIndex < nValues; dwValueIndex++)
					{
						nValueNameLen = (nMaxValueNameLen + 1);
						nDataNameLen = (nMaxDataNameLen + 1);
						ZeroMemory(pValueBuffer,(nMaxValueNameLen + 1));
						ZeroMemory(DataBuffer,(nMaxDataNameLen + 1));
						RegEnumValue(hDeleteKey, dwValueIndex, pValueBuffer, &nValueNameLen,
									NULL, NULL, DataBuffer, &nDataNameLen);
						if((strcmp (pValueBuffer, "PGPnetDelFlag") == 0)&&
							(strcmp (DataBuffer, "PGPnetDelFlag") == 0))
						{
							bDelete = TRUE;
							dwValueIndex = nValues;
						}/*if*/

					}/*for*/
					RegCloseKey(hDeleteKey);
					if (bDelete)
					{
						if(RegDeleteKey( hRootKey, pSubKeyValueBuffer) == ERROR_SUCCESS)
							dwKeyIndex = 0;
					}
					free(pValueBuffer);
				}/*if*/
			}/*if*/
		}/*for*/
		RegDeleteKey(HKEY_LOCAL_MACHINE, "Enum\\Root\\PGPnet");
		free(pSubKeyValueBuffer);
		RegCloseKey(hRootKey);
	}/*if*/
return TRUE;
}/*end*/
