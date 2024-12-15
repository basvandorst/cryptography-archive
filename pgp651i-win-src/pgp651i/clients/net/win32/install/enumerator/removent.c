/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	

	$Id: removent.c,v 1.18 1999/03/30 23:12:38 philipn Exp $
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
BOOL RemoveNT()
{
	HKEY			hMainKey		= HKEY_LOCAL_MACHINE;
	HKEY			hOpenKey		= NULL;

	StopService ("PGPmac");
	StopService ("PGPMacMP");
	CleanRegNT ();

	if((bDelReg)||(bSansCmdLine))
	{
		PutSAinRunKey();
	}

	//Set PGPnetNTSafeToRebind entry
	if (RegOpenKeyEx(hMainKey, PATHTOPGPNETREGKEY, 0, KEY_ALL_ACCESS, &hOpenKey)
		== ERROR_SUCCESS)	
	{
		RegSetValueEx(hOpenKey, "PGPnetNTSafeToRebind" , 0, REG_SZ, "", 1);
		RegCloseKey (hOpenKey);
	}/*if*/
	
	if(bDelReg)
	{
		
		if (FindWindow("", "Bind review occured") == NULL)
			MessageBox (GetFocus(), BINDREVIEWOCCURED, "Bind review occured", 0 | MB_ICONINFORMATION);
		
	}/*if*/

	return TRUE;
}/*end*/


/*
 *  Code to stop a running service so it can be removed 	
 */
INT StopService (LPSTR ServiceName)
{
	SERVICE_STATUS STATUS;
	SC_HANDLE hSCManager;
	SC_HANDLE hService;
	int iReturn;

	hSCManager = NULL;
	hService = NULL;
	iReturn = FALSE;

	
	hSCManager = OpenSCManager	(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	hService = OpenService	(hSCManager, ServiceName,SERVICE_ALL_ACCESS | DELETE);

	iReturn = ControlService(hService, SERVICE_CONTROL_STOP, &STATUS);

	iReturn = DeleteService(hService);

	CloseServiceHandle (hService);
	CloseServiceHandle (hSCManager);

return TRUE;
}


/*
 *  Code to stop a running service so it can be removed 	
 */
BOOL CleanRegNT ()
{
	HKEY			hMainKey		= HKEY_LOCAL_MACHINE;
	HKEY			hNewKey			= NULL;
	HKEY			hDeleteKey		= NULL;
	HKEY			hNewKeyb		= NULL;
	LPBYTE			pData			= NULL;
	LPTSTR 			pSubkeyBuffer	= NULL;	
	LPTSTR			KeyTitle        = NULL;
	LPBYTE			pValue			= NULL;
	unsigned long	nSubkeyNameLen;
	unsigned long	dwBufSize;
	DWORD			nSubkeys;
	DWORD			dwIndex;
	DWORD			dwType;
	DWORD			dwSize;
	LONG			lResult			= ERROR_SUCCESS;

	/*Open key*/
   	if (RegOpenKeyEx(hMainKey, SERVICESSUBKEY, 0, KEY_ALL_ACCESS, &hNewKey)
					== ERROR_SUCCESS)
	{
		/*Determine number of keys to enumerate*/
		RegQueryInfoKey(hNewKey, NULL, NULL, NULL, &nSubkeys, 
						&nSubkeyNameLen, NULL, NULL, NULL,
						NULL, NULL, NULL);

		/*Retrieve Registry values*/
		pSubkeyBuffer = malloc(nSubkeyNameLen);
		for (dwIndex = 0; dwIndex < nSubkeys; dwIndex++)
		{
			dwBufSize = nSubkeyNameLen;
			lResult = RegEnumKeyEx(hNewKey, dwIndex, pSubkeyBuffer, &dwBufSize, 
							       NULL, NULL, NULL, NULL);
			//if the service key contains PGPmac
			if (strstr(pSubkeyBuffer, "PGPmac") ||
				strstr(pSubkeyBuffer, "PGPMac"))
			{
				if (RegOpenKeyEx(hNewKey, pSubkeyBuffer, 0, KEY_ALL_ACCESS, &hDeleteKey)
							== ERROR_SUCCESS)
				{  
					lResult = RegDeleteKey (hDeleteKey, "Enum");
					lResult = RegDeleteKey (hDeleteKey, "Linkage\\Disabled");
					lResult = RegDeleteKey (hDeleteKey, "Linkage");
					lResult = RegDeleteKey (hDeleteKey, "Security");
					lResult = RegDeleteKey (hDeleteKey, "Parameters\\Signature");
					lResult = RegDeleteKey (hDeleteKey, "Parameters\\Tcpip");
					lResult = RegDeleteKey (hDeleteKey, "Parameters");
					lResult = RegDeleteKey (hNewKey, pSubkeyBuffer);
				}/*if*/
			}/*if*/

		}/*for*/
		free(pSubkeyBuffer);
		RegCloseKey (hNewKey);
		RegCloseKey (hDeleteKey);
	}/*if*/

	/*Open key*/
   	if (RegOpenKeyEx(hMainKey, NETA_KEY, 0, KEY_ALL_ACCESS, &hNewKey)
					== ERROR_SUCCESS)
	{
		/*Determine number of keys to enumerate*/
		RegQueryInfoKey(hNewKey, NULL, NULL, NULL, &nSubkeys, 
						&nSubkeyNameLen, NULL, NULL, NULL,
						NULL, NULL, NULL);

		/*Retrieve Registry values*/
		pSubkeyBuffer = malloc(nSubkeyNameLen);
		for (dwIndex = 0; dwIndex < nSubkeys; dwIndex++)
		{
			dwBufSize = nSubkeyNameLen;
			lResult = RegEnumKeyEx(hNewKey, dwIndex, pSubkeyBuffer, &dwBufSize, 
							       NULL, NULL, NULL, NULL);
			//if the service key contains PGPmac
			if (strstr(pSubkeyBuffer, "PGPmac") ||
				strstr(pSubkeyBuffer, "PGPMac"))
			{
				if (RegOpenKeyEx(hNewKey, pSubkeyBuffer, 0, KEY_ALL_ACCESS, &hDeleteKey)
							== ERROR_SUCCESS)
				{
				lResult = RegDeleteKey (hDeleteKey,"CurrentVersion\\NetRules");
				lResult = RegDeleteKey (hDeleteKey,"CurrentVersion");
				lResult = RegDeleteKey (hNewKey, pSubkeyBuffer);
				}
			}/*if*/
		}/*for*/
		free(pSubkeyBuffer);
		RegCloseKey (hNewKey);
		RegCloseKey (hDeleteKey);
	}/*if*/

	/*Open key*/
   	if (RegOpenKeyEx(hMainKey, NETWORKCARDS_KEY, 0, KEY_ALL_ACCESS, &hNewKey)
					== ERROR_SUCCESS)
	{
		/*Determine number of keys to enumerate*/
		RegQueryInfoKey(hNewKey, NULL, NULL, NULL, &nSubkeys, 
						&nSubkeyNameLen, NULL, NULL, NULL,
						NULL, NULL, NULL);

		/*Retrieve Registry values*/
		pSubkeyBuffer = malloc(nSubkeyNameLen);
		dwSize = 1024;
		pValue = (LPBYTE) calloc(sizeof(BYTE), dwSize);
		for (dwIndex = 0; dwIndex < nSubkeys; dwIndex++)
		{
			dwBufSize = nSubkeyNameLen;
			lResult = RegEnumKeyEx(hNewKey, dwIndex, pSubkeyBuffer, &dwBufSize, 
							       NULL, NULL, NULL, NULL);
			if (RegOpenKeyEx(hNewKey, pSubkeyBuffer, 0, KEY_ALL_ACCESS, &hDeleteKey)
						== ERROR_SUCCESS)	
			{
				dwSize = 1024;
				lResult = RegQueryValueEx(hDeleteKey, "ServiceName", 0, &dwType, pValue, &dwSize);
				//if the service key contains PGPmac
				if (strstr(pValue, "PGPmac") ||
					strstr(pValue, "PGPMac"))
				{
					lResult = RegDeleteKey (hDeleteKey,"NetRules");
					lResult = RegDeleteKey (hNewKey, pSubkeyBuffer);
				}/*if*/
			}/*if*/
		RegCloseKey (hDeleteKey);
		}/*for*/
		free (pValue);
		free (pSubkeyBuffer);
		RegCloseKey (hNewKey);
	}/*if*/

return TRUE;
}


int CheckNTBindStatus()
{
	HKEY			hMainKey		= HKEY_LOCAL_MACHINE;
	HKEY			hOpenKey		= NULL;
	LPBYTE			pValue			= NULL;
	DWORD			dwType;
	DWORD			dwSize;

	if(bRunOnce)
		return 3;

	if (RegOpenKeyEx(hMainKey, RUNONCE, 0, KEY_ALL_ACCESS, &hOpenKey)
		== ERROR_SUCCESS)	
	{
		if (RegQueryValueEx(hOpenKey, "PGPnetNeedsReboot", 0, &dwType, pValue, &dwSize) == ERROR_SUCCESS)
		{
			RegCloseKey (hOpenKey);
			return 2;	
		}
		RegCloseKey (hOpenKey);
	}/*if*/
	
	if (RegOpenKeyEx(hMainKey, PATHTOPGPNETREGKEY, 0, KEY_ALL_ACCESS, &hOpenKey)
		== ERROR_SUCCESS)	
	{
		if (RegQueryValueEx(hOpenKey, "PGPnetNTSafeToRebind", 0, &dwType, pValue, &dwSize) == ERROR_SUCCESS)
		{
			RegCloseKey (hOpenKey);
			return 3;	
		}
		RegCloseKey (hOpenKey);
	}/*if*/
	return 1;
}/*end*/


//____________________________
//
//	add specified string to existing multistring value

BOOL
AddStringToMultiString (
		HKEY	hkey,
		LPSTR	szValue,
		LPSTR	szString)
{
	BOOL	bReturn			= FALSE;
	BYTE*	pBuffer;
	DWORD	dwSizeTotal;
	DWORD	dwSize;
	DWORD	dwType;
	DWORD	dwNewEntrySize;
	BYTE*	tmppBuffer;
	BYTE*	tmppBufferHead;
	unsigned int		index;
	int		count;

	// get type and length of existing string
	if (ERROR_SUCCESS == RegQueryValueEx (
							hkey,
							szValue,
							NULL,
							&dwType,
							NULL,
							&dwSize))
	{
		// verify type
		if (dwType == REG_MULTI_SZ)
		{
			// compute new length and allocate buffer
			dwNewEntrySize = lstrlen (szString)+1;
			dwSizeTotal = dwSize + lstrlen (szString) + 1;
			pBuffer = malloc (dwSizeTotal);
			ZeroMemory(pBuffer, dwSizeTotal);
			tmppBuffer = malloc (dwSizeTotal);
			ZeroMemory(tmppBuffer, dwSizeTotal);
			tmppBufferHead = tmppBuffer;

			if (pBuffer)
			{
				// get existing data and copy 
				RegQueryValueEx (
							hkey,
							szValue,
							NULL,
							&dwType,
							pBuffer, 
							&dwSize);

				// append new string
				lstrcpy (tmppBuffer, szString);
				//tmppBuffer += (strlen(szString)+1);
				count = 0;
				for (index = dwNewEntrySize; index <= dwSizeTotal; index++)
				{
					tmppBuffer[index] = pBuffer [count];
					count ++;
				}

				// write the new string back to the registry
				if (ERROR_SUCCESS == RegSetValueEx (
										hkey,
										szValue,
										0,
										REG_MULTI_SZ,
										tmppBufferHead,
										dwSizeTotal))
				{
					bReturn = TRUE;
				}
				free (pBuffer);
			}
		}
	}
	return bReturn;
}

 
//____________________________
//
//	delete all occurances of specified string from multistring value

BOOL
DeleteStringFromMultiString (
		HKEY	hkey,
		LPSTR	szValue,
		LPSTR	szString)
{
	BOOL	bReturn			= FALSE;
	BYTE*	pBuffer;
	BYTE*	pSrc;
	BYTE*	pDst;
	DWORD	dwSize;
	DWORD	dwType;

	// get type and length of existing string
	if (ERROR_SUCCESS == RegQueryValueEx (
							hkey,
							szValue,
							NULL,
							&dwType,
							NULL,
							&dwSize))
	{
		// verify type
		if (dwType == REG_MULTI_SZ)
		{
			// allocate buffer
			pBuffer = malloc (dwSize);
			if (pBuffer)
			{
				// get existing data
				RegQueryValueEx (
							hkey,
							szValue,
							NULL,
							&dwType,
							pBuffer,
							&dwSize);

				// kill all instances of specified string
				pSrc = pDst = pBuffer;
				while (*pSrc) 
				{
					if (!strstr (pSrc, szString))
					{
						dwSize = lstrlen (pSrc);

						lstrcpy (pDst, pSrc);

						pDst += lstrlen (pDst) +1;
						pSrc += dwSize +1;
					}
					else
					{
						pSrc += lstrlen (pSrc) +1;
					}
				}

				// add the final NULL
				*pDst = '\0';
				dwSize = pDst - pBuffer +1;

				// write the new string back to the registry
				if (ERROR_SUCCESS == RegSetValueEx (
										hkey,
										szValue,
										0,
										REG_MULTI_SZ,
										pBuffer,
										dwSize))
				{
					bReturn = TRUE;
				}

				free (pBuffer);
			}
		}
	}

	return bReturn;
}


BOOL AddInf ()
{
	HKEY		hMainKey	= HKEY_LOCAL_MACHINE;
	HKEY		hOpenKey	= NULL;
	LPSTR		KeyValue;
	LPSTR		szString;
	UINT		nBufferLengthMax = 1024;

	if (RegOpenKeyEx(hMainKey, NCPA, 0, KEY_ALL_ACCESS, &hOpenKey)
			== ERROR_SUCCESS)
	{
		KeyValue = malloc(15);
		ZeroMemory (KeyValue, 15);
		strcpy (KeyValue, "ReviewPrograms");

		szString = malloc(nBufferLengthMax);
		ZeroMemory (szString, nBufferLengthMax);

		GetCurrentDirectory(nBufferLengthMax, szString);
		strcat(szString, "\\PGPRebind.inf");
		//Get the short path (no spaces)
		GetShortPathName(szString, szString, nBufferLengthMax); 

		AddStringToMultiString (
		hOpenKey,
		KeyValue,
		szString);

		RegCloseKey (hOpenKey);
		free(szString);
		free(KeyValue);
	}/*if*/
	return TRUE;
}


BOOL RemoveInf ()
{
	HKEY		hMainKey	= HKEY_LOCAL_MACHINE;
	HKEY		hOpenKey	= NULL;
	LPSTR		KeyValue;
	LPSTR		szString;
	UINT		nBufferLengthMax = 1024;

	if (RegOpenKeyEx(hMainKey, NCPA, 0, KEY_ALL_ACCESS, &hOpenKey)
	== ERROR_SUCCESS)
	{
		KeyValue = malloc(15);
		ZeroMemory (KeyValue, 15);
		strcpy (KeyValue, "ReviewPrograms");

		szString = malloc(10);
		ZeroMemory (szString, 10);
		strcpy (szString, "PGP");

		DeleteStringFromMultiString(
		hOpenKey,
		KeyValue,
		szString);

		RegCloseKey (hOpenKey);
		free(szString);
		free(KeyValue);
	}/*if*/
	return TRUE;
}


//____________________________
//
//	Disable all protocols that bind to us except
//	the ones we use.

BOOL DisableExtraProtocols ()
{
	//#define SERVICESSUBKEY			"SYSTEM\\CurrentControlSet\\Services"

	HKEY		hMainKey	= HKEY_LOCAL_MACHINE;
	HKEY		hOpenKey	= NULL;
	/*these are service keys we use PLUS all PGPmac.... keys*/
	LPTSTR LinkedKeys[]				= {"RasArp",
									   "NetBT",
									   "LanmanServer",
									   "LanmanWorkstation",
									   "RemoteAccess",
									   "NetBIOS",
									   "Tcpip"};
	HKEY			hNewKey			= NULL;
	HKEY			hNewKeyb		= NULL;
	HKEY			hLinkKey		= NULL;
	HKEY			hDisableKey		= NULL;
	LPBYTE			pData			= NULL;
	LPBYTE			pOurBindStr		= NULL;
	LPTSTR 			pSubkeyBuffer	= NULL;	
	LPTSTR			KeyTitle        = NULL;
	LPTSTR			CurrentKey      = NULL;
	LPTSTR			DisableKey      = NULL;
	unsigned long	nSubkeyNameLen;
	unsigned long	dwBufSize;
	DWORD			nSubkeys;
	DWORD			dwIndex;
	DWORD			dwIndexb;
	DWORD			dwSize;
	DWORD			dwType;
	LONG			lResult			= ERROR_SUCCESS;
	BOOL			Pass;
	BOOL			bRC;
	BOOL			bregerr			= FALSE;
	BOOL			ItemDisabled	= FALSE;	


	/*get value like \Device\PGPMacMP4*/
	if (RegOpenKeyEx(hMainKey, PATHTOPGPNETREGKEY, 0, KEY_ALL_ACCESS, &hOpenKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx (hOpenKey, "BindString", NULL, &dwType, NULL, &dwSize)
						== ERROR_SUCCESS)
		{
			pOurBindStr = (LPBYTE) calloc(sizeof(BYTE), dwSize);
			ZeroMemory (pOurBindStr, dwSize);
			if (RegQueryValueEx (hOpenKey, "BindString", NULL, &dwType, pOurBindStr, &dwSize)
							== ERROR_SUCCESS)
				goto success;
			else
				bregerr = TRUE;
		}/*if*/
		else
			bregerr = TRUE;
success:
	RegCloseKey(hOpenKey);
	}/*if*/
	else
		bregerr = TRUE;
	
	if (bregerr)
	{
		MessageBox (GetFocus(), ERR_READREG, "Set Adapter", 0 | MB_ICONERROR);
		exit (1);
	}

	/*Open key*/
	if (RegOpenKeyEx(hMainKey, SERVICESSUBKEY, 0, KEY_ALL_ACCESS, &hOpenKey) == ERROR_SUCCESS)
	{
		/*Determine number of keys to enumerate*/
		RegQueryInfoKey(hOpenKey, NULL, NULL, NULL, &nSubkeys, 
						&nSubkeyNameLen, NULL, NULL, NULL,
						NULL, NULL, NULL);

		/*Retrieve Registry values*/
		pSubkeyBuffer = malloc(nSubkeyNameLen + 1);
		CurrentKey = malloc(1024);
		DisableKey = malloc(1024);
		for (dwIndex = 0; dwIndex < nSubkeys; dwIndex++)
		{
			dwBufSize = nSubkeyNameLen + 1;
			lResult = RegEnumKeyEx(hOpenKey, dwIndex, pSubkeyBuffer, &dwBufSize, 
							       NULL, NULL, NULL, NULL);

			Pass = TRUE;
			for (dwIndexb = 0; dwIndexb<=6; dwIndexb++)
			{
				/*check if key is one of the ones NOT to touch*/
				if ((strcmp (pSubkeyBuffer, LinkedKeys[dwIndexb])==0) ||
					(strstr(pSubkeyBuffer, "PGPMac") != NULL))
					Pass = FALSE;
					/*we now check if changes need to be made*/
			}/*for*/
			if (Pass)
			{
				ZeroMemory(CurrentKey, 1024);
				ZeroMemory(DisableKey, 1024);
				/*SERVICESSUBKEY contains the path to the top of the 
								services reg. key.
				pSubkeyBuffer contains the name of the current key
							to examine within the services tree.*/
				strcpy(CurrentKey, SERVICESSUBKEY);
				strcat(CurrentKey, "\\");
				strcat(CurrentKey, pSubkeyBuffer);
				strcat(CurrentKey, "\\Linkage");
				strcpy(DisableKey, CurrentKey);
				strcat(DisableKey, "\\Disabled");

				if (lResult = RegOpenKeyEx(hMainKey, CurrentKey, 0, KEY_ALL_ACCESS,
								&hLinkKey) == ERROR_SUCCESS)
				{
					if (RegQueryValueEx (hLinkKey, BIND, NULL, &dwType, NULL,	&dwSize)
										== ERROR_SUCCESS)
					{
						pData = (LPBYTE) calloc(sizeof(BYTE), dwSize);
						ZeroMemory (pData, dwSize);
						if (RegQueryValueEx (hLinkKey, BIND, NULL, &dwType, pData, &dwSize)
											== ERROR_SUCCESS)
							bRC = MultiStrStrCmp((char*)pData, (char*)pOurBindStr);
							
						if(bRC)
						{
							/*Add pOurBindStr to Linkage\\Disable Bind value
							of CurrentKey*/
							if (lResult = RegOpenKeyEx(hMainKey, DisableKey, 0, KEY_ALL_ACCESS, &hDisableKey)
											== ERROR_SUCCESS)	
							{
								AddStringToMultiString (hDisableKey,
														BIND,
														pOurBindStr);
								RegCloseKey (hDisableKey);
								ItemDisabled = TRUE;
							}/*if*/
						}/*if*/
						free(pData);
					}/*if*/
					RegCloseKey(hLinkKey);
				}/*if*/	
			}/*if*/
		}/*for*/
	free(CurrentKey);
	free(DisableKey);
	free(pSubkeyBuffer);
	free(pOurBindStr);
	RegCloseKey(hOpenKey);
	}/*if*/
	return ItemDisabled;
}


//____________________________
//
//	Find if stringb is in multistringa
//	

BOOL MultiStrStrCmp (LPSTR multistringa, LPSTR stringb)
{
	int	strlength;
	int	count;


compare:
	if (strstr(multistringa, stringb) == NULL)
	{
		strlength = strlen (multistringa);
		for (count = 0; count <= strlength; count++)
			*multistringa++;

		if (*multistringa == '\0')
			/*we are at the end of the multisting*/
			return FALSE;
		else
			goto compare;
	}
	else
		return TRUE;
}



/*if (iReturn == 0)
{
	char szText[256];
	wsprintf(szText, "Error code: %d", GetLastError());
	MessageBox(NULL, szText, "Delete Service", MB_OK);
}*/



