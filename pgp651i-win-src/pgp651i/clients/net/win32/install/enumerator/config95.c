/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	

	$Id: config95.c,v 1.11.4.2 1999/06/14 05:26:54 philipn Exp $
____________________________________________________________________________*/
#pragma message( "Compiling " __FILE__ ) 
#pragma message( "Last modified on " __TIMESTAMP__ ) 

#include <windows.h>
#include <stdio.h>
#include "Define.h"
#include "Lister.h"


char gNetSlot[5];
char gTransSlot[5];
char gEnumNetSlot[5];


/*
 *  Creates our "System\\CurrentControlSet\\Services\\Class\\NetTrans" Structure 	
 */
BOOL ConfigSystemNetTrans ()
{
	#define NetTransKey		"System\\CurrentControlSet\\Services\\Class\\NetTrans"
	#define Ndikey			"Ndi"
	#define Ndikeya			"Ndi\\Install"
	#define Ndikeyb			"Ndi\\Interfaces"
	#define Ndikeyc			"Ndi\\Remove"
	#define NDISkey			"NDIS"
	#define MAXKEYLENGTH	"1024"

	HKEY			hTransKey;
	HKEY			hNewKey;
	DWORD			dwIndex;
	LPSTR			SubKeyBuffer	= NULL;
	LPSTR			KeyBuffer		= NULL;
	DWORD			dwDisposition;
	char			Binary;

	dwIndex = 9999;
	/*Find an open position*/
	/*Assuming no forced order, we look for FIRST open position*/
	for(dwIndex = 9999; dwIndex > 0000; dwIndex --)
	{
		SubKeyBuffer = (char *)malloc (5);
		sprintf(SubKeyBuffer,"%04d", dwIndex);
		KeyBuffer = (char *)malloc (strlen(NetTransKey) +6);
		ZeroMemory (KeyBuffer, (strlen(NetTransKey) +6));
		strcat (KeyBuffer, NetTransKey);
		strcat (KeyBuffer, "\\");
		strcat (KeyBuffer, SubKeyBuffer);
		
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer, 0, KEY_READ, &hTransKey)
						== ERROR_FILE_NOT_FOUND)
			break;

		RegCloseKey(hTransKey);
		free(KeyBuffer);
		free(SubKeyBuffer);
	}/*for*/
	//RegCloseKey(hTransKey);
	free(KeyBuffer);
	strcpy (gTransSlot, SubKeyBuffer);

	/*Create the tree structure*/
	/*ROOT*/	
	KeyBuffer = (char *)malloc (1024);
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetTransKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegSetValueEx(hNewKey, "DeviceVxDs" , 0, REG_SZ, "PGPnet.vxd", 11);
	RegSetValueEx(hNewKey, "DevLoader" , 0, REG_SZ, "*ndis", 6);
	/*release date GET THIS FROM THE VXD*/
	RegSetValueEx(hNewKey, "DriverDate" , 0, REG_SZ, "06-10-99", 9);
	RegSetValueEx(hNewKey, "DeviceDesc" , 0, REG_SZ, "PGPnet Protocol driver", 24);
	//RegSetValueEx(hNewKey, "InfPath" , 0, REG_SZ, "PGPmact.inf", 12);
	RegSetValueEx(hNewKey, "PGPnetDelFlag" , 0, REG_SZ, "PGPnetDelFlag", 14);

	RegCloseKey(hNewKey);
 
	
	/*Create the tree structure*/
	/*Ndi*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetTransKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, Ndikey);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegSetValueEx(hNewKey, "DeviceID" , 0, REG_SZ, "PGPtrans", 9);
	RegSetValueEx(hNewKey, "HelpText" , 0, REG_SZ,
				"PGPnet Protocol driver. By Network Associates Inc.", 51);
	RegSetValueEx(hNewKey, "InfSection" , 0, REG_SZ, "PGPtrans.ndi", 13);
	//RegSetValueEx(hNewKey, "InstallInf" , 0, REG_SZ, "", 1);
	RegSetValueEx(hNewKey, "MaxInstance" , 0, REG_SZ, "0", 2);

	RegCloseKey(hNewKey);

	/*Create the tree structure*/
	/*Ndi\\Install*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetTransKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, Ndikeya);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegCloseKey(hNewKey);

	/*Create the tree structure*/
	/*Ndi\\Interfaces*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetTransKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, Ndikeyb);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegSetValueEx(hNewKey, "DefLower" , 0, REG_SZ, "ndis3", 6);
	RegSetValueEx(hNewKey, "Lower" , 0, REG_SZ, "ndis3", 6);
	RegSetValueEx(hNewKey, "LowerRange" , 0, REG_SZ, "ndis3", 6);
	RegSetValueEx(hNewKey, "Upper" , 0, REG_SZ, "", 1);

	RegCloseKey(hNewKey);

	/*Create the tree structure*/
	/*Ndi\\Remove*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetTransKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, Ndikeyc);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegCloseKey(hNewKey);

	/*Create the tree structure*/
	/*NDIS*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetTransKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, NDISkey);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	
	RegSetValueEx(hNewKey, "LogDriverName" , 0, REG_SZ, "PGPMAC", 6);
	Binary = 3;
	RegSetValueEx(hNewKey, "MajorNdisVersion" , 0, REG_BINARY, &Binary, 1);
	Binary = 0x0a;
	RegSetValueEx(hNewKey, "MinorNdisVersion", 0, REG_BINARY, &Binary, 1);

	RegCloseKey(hNewKey);
	free(KeyBuffer);
	free(SubKeyBuffer);
	return TRUE;
}/*end*/


/*
 *  Creates our "System\\CurrentControlSet\\Services\\Class\\Net" Structure 	
 */
BOOL ConfigSystemNet ()
{
	#define NetKey			"System\\CurrentControlSet\\Services\\Class\\Net"
	#define Ndikey			"Ndi"
	#define Ndikeya			"Ndi\\Install"
	#define Ndikeyb			"Ndi\\Interfaces"
	#define NDISkey			"NDIS"
	#define MAXKEYLENGTH	"1024"

	HKEY			hNetKey;
	HKEY			hNewKey;
	DWORD			dwIndex;
	LPSTR			SubKeyBuffer	= NULL;
	LPSTR			KeyBuffer		= NULL;
	DWORD			dwDisposition;
	char			Binary;

	/*Find an open position*/
	/*Assuming no forced order, we look for FIRST open position*/
	for(dwIndex = 9999; dwIndex > 0000; dwIndex --)
	{
		SubKeyBuffer = (char *)malloc (5);
		sprintf(SubKeyBuffer,"%04d", dwIndex);
		KeyBuffer = (char *)malloc (strlen(NetKey) +6);
		ZeroMemory (KeyBuffer, (strlen(NetKey) +6));
		strcat (KeyBuffer, NetKey);
		strcat (KeyBuffer, "\\");
		strcat (KeyBuffer, SubKeyBuffer);
		
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer, 0, KEY_READ, &hNetKey)
						== ERROR_FILE_NOT_FOUND)
			break;

		RegCloseKey(hNetKey);
		free(KeyBuffer);
		free(SubKeyBuffer);
	}/*for*/


	free(KeyBuffer);
	strcpy (gNetSlot, SubKeyBuffer);

	/*Create the tree structure*/
	/*ROOT*/	
	KeyBuffer = (char *)malloc (1024);
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegSetValueEx(hNewKey, "DeviceVxDs" , 0, REG_SZ, "PGPnet.vxd", 11);
	RegSetValueEx(hNewKey, "DevLoader" , 0, REG_SZ, "*ndis", 6);
	RegSetValueEx(hNewKey, "DeviceDesc" , 0, REG_SZ,
				"PGPnet Virtual Private Networking driver", 41);
	RegSetValueEx(hNewKey, "EnumPropPages" , 0, REG_SZ, "netdi.dll,EnumPropPages", 24);
	RegSetValueEx(hNewKey, "InfPath" , 0, REG_SZ, "Pgpmaca.inf", 12);
	RegSetValueEx(hNewKey, "PGPnetDelFlag" , 0, REG_SZ, "PGPnetDelFlag", 14);

	RegCloseKey(hNewKey);
 
	
	/*Create the tree structure*/
	/*Ndi*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, Ndikey);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegSetValueEx(hNewKey, "DeviceID" , 0, REG_SZ, "PGPmac", 7);
	RegSetValueEx(hNewKey, "HelpText" , 0, REG_SZ,
				"PGPnet Virtual Private Networking driver. By Network Associates Inc.", 69);
	RegSetValueEx(hNewKey, "InfSection" , 0, REG_SZ, "PGPmac.ndi", 11);
	RegSetValueEx(hNewKey, "InstallInf" , 0, REG_SZ, "", 1);

	RegCloseKey(hNewKey);

	/*Create the tree structure*/
	/*Ndi\\Install*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, Ndikeya);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegSetValueEx(hNewKey, "" , 0, REG_SZ, "PGPMAC.Install", 15);

	RegCloseKey(hNewKey);

	/*Create the tree structure*/
	/*Ndi\\Interfaces*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, Ndikeyb);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegSetValueEx(hNewKey, "DefLower" , 0, REG_SZ, "ethernet", 9);
	RegSetValueEx(hNewKey, "DefUpper" , 0, REG_SZ, "ndis3", 6);
	RegSetValueEx(hNewKey, "Lower" , 0, REG_SZ, "ethernet", 9);
	RegSetValueEx(hNewKey, "LowerRange" , 0, REG_SZ, "ethernet", 9);
	RegSetValueEx(hNewKey, "Upper" , 0, REG_SZ, "ndis3", 6);
	RegSetValueEx(hNewKey, "UpperRange" , 0, REG_SZ, "ndis3", 6);

	RegCloseKey(hNewKey);

	/*Create the tree structure*/
	/*NDIS*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, NDISkey);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);
	
	RegSetValueEx(hNewKey, "LogDriverName" , 0, REG_SZ, "PGPMAC", 7);

	Binary = 3;
	RegSetValueEx(hNewKey, "MajorNdisVersion" , 0, REG_BINARY, &Binary, 1);
	Binary = 0x0a;
	RegSetValueEx(hNewKey, "MinorNdisVersion", 0, REG_BINARY, &Binary, 1);

	RegCloseKey(hNewKey);
	free(KeyBuffer);
	free(SubKeyBuffer);
	#undef NetKey
	return TRUE;
}/*end*/


/*
 *  Creates our "Enum\\Root\\Net" Structure 	
 */
BOOL ConfigEnumNet ()
{
	#define EnumNetKey		"Enum\\Root\\PGPnet" 
	#define BindingsKey		"Bindings"
	#define MAXKEYLENGTH	"1024"


	HKEY			hEnumNetKey;
	HKEY			hNewKey;
	DWORD			dwIndex;
	LPSTR			SubKeyBuffer	= NULL;
	LPSTR			KeyBuffer		= NULL;
	LPSTR			Driver			= NULL;
	DWORD			dwDisposition;
	char			Binary [4];

	/*Find an open position*/
	/*Assuming no forced order, we look for FIRST open position*/
	for(dwIndex = 9999; dwIndex > 0000; dwIndex --)
	{
		SubKeyBuffer = (char *)malloc (5);
		sprintf(SubKeyBuffer,"%04d", dwIndex);
		KeyBuffer = (char *)malloc (strlen(EnumNetKey) +6);
		ZeroMemory (KeyBuffer, (strlen(EnumNetKey) +6));
		strcat (KeyBuffer, EnumNetKey);
		strcat (KeyBuffer, "\\");
		strcat (KeyBuffer, SubKeyBuffer);
		
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer, 0, KEY_READ, &hEnumNetKey)
						== ERROR_FILE_NOT_FOUND)
			break;

		RegCloseKey(hEnumNetKey);
		free(KeyBuffer);
		free(SubKeyBuffer);
	}/*for*/
	free(KeyBuffer);
	strcpy(gEnumNetSlot, SubKeyBuffer);

	/*Create the tree structure*/
	/*ROOT*/	
	KeyBuffer = (char *)malloc (1024);
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, EnumNetKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegSetValueEx(hNewKey, "Class" , 0, REG_SZ, "Net", 4);
	RegSetValueEx(hNewKey, "CompatibleIDs" , 0, REG_SZ, "*PGPMAC", 8);
	RegSetValueEx(hNewKey, "PGPnetDelFlag" , 0, REG_SZ, "PGPnetDelFlag", 14);

	Binary [0] = 0x10;
	Binary [1] = 0x0;
	Binary [2] = 0x0;
	Binary [3] = 0x0;
	RegSetValueEx(hNewKey, "ConfigFlags", 0, REG_BINARY, *&Binary, 4);

	RegSetValueEx(hNewKey, "DeviceDesc" , 0, REG_SZ,
				"PGPnet Virtual Private Networking driver", 41);

	Driver = (char *)malloc (10);
	ZeroMemory (Driver, (10));
	strcat(Driver, "Net\\");
	strcat(Driver, gNetSlot);
	RegSetValueEx(hNewKey, "Driver" , 0, REG_SZ, Driver, 10);
	RegSetValueEx(hNewKey, "Mfg" , 0, REG_SZ, "Network Associates Inc.", 24);

	RegCloseKey(hNewKey);
 
	
	/*Create the tree structure*/
	/*Bindings*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, EnumNetKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, BindingsKey);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegCloseKey(hNewKey);
	free(Driver);
	free(KeyBuffer);
	free(SubKeyBuffer);
	#undef EnumNetKey	
	return TRUE;
}/*end*/


/*
 *  Creates our "Enum\\Network\\PGPMAC" Structure 	
 */
BOOL ConfigEnumNetwork ()
{
	#define EnumTransKey	"Enum\\Network\\PGPMAC" 
	#define BindingsKey		"Bindings"
	#define MAXKEYLENGTH	"1024"


	HKEY			hNewKey;
	DWORD			dwIndex;
	LPSTR			SubKeyBuffer	= NULL;
	LPSTR			KeyBuffer		= NULL;
	LPSTR			Driver			= NULL;
	LPSTR			MasterCopy		= NULL;
	DWORD			dwDisposition;
	char			Binary[4];

	dwIndex = 0000;
	/*Find an open position*/
	SubKeyBuffer = (char *)malloc (5);
	sprintf(SubKeyBuffer,"%04d", dwIndex);

	/*Create the tree structure*/
	/*ROOT*/	
	KeyBuffer = (char *)malloc (1024);
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, EnumTransKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	
	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegSetValueEx(hNewKey, "Class" , 0, REG_SZ, "NetTrans", 9);
	RegSetValueEx(hNewKey, "CompatibleIDs" , 0, REG_SZ, "PGPMAC", 7);

	Binary [0] = 0x10;
	Binary [1] = 0x0;
	Binary [2] = 0x0;
	Binary [3] = 0x0;
	RegSetValueEx(hNewKey, "ConfigFlags", 0, REG_BINARY, *&Binary, 4);

	RegSetValueEx(hNewKey, "DeviceDesc" , 0, REG_SZ,
				"PGPnet Virtual Private Networking Protocol driver.", 51);
	
	Driver = (char *)malloc (14);
	ZeroMemory (Driver, (14));
	strcat(Driver, "NetTrans\\");
	strcat(Driver, gTransSlot);
	RegSetValueEx(hNewKey, "Driver" , 0, REG_SZ, Driver, 14);
	MasterCopy = (char *)malloc (26);
	ZeroMemory (MasterCopy, (26));
	strcat(MasterCopy, EnumTransKey);
	strcat(MasterCopy, "\\");
	strcat(MasterCopy, SubKeyBuffer);
	RegSetValueEx(hNewKey, "MasterCopy" , 0, REG_SZ, MasterCopy, 26);
	RegSetValueEx(hNewKey, "Mfg" , 0, REG_SZ, "Network Associates Inc.", 24);

	RegCloseKey(hNewKey);
 
	
	/*Create the tree structure*/
	/*Bindings*/
	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, EnumTransKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, SubKeyBuffer);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, BindingsKey);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE, KeyBuffer,
	0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey,
	&dwDisposition);

	RegCloseKey(hNewKey);
	free(Driver);
	free(MasterCopy);
	free(KeyBuffer);
	free(SubKeyBuffer);
	return TRUE;
}/*end*/


/*
 *  Sets the RealMac value inside HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\Class\Net
 */
BOOL SetRealMac (LPSTR RealMac)
{
	#define	DRIVER			"Driver"
	#define NetKey			"System\\CurrentControlSet\\Services\\Class\\Net"
	#define EnumNetKey		"Enum\\Root\\PGPnet" 

	HKEY			hNewKey;
	HKEY			hOurBindKey;
	HKEY			hTheirBindKey;
	HKEY			hTheirDriverKey;
	HKEY			hTheirOldBindings;
	unsigned long   MaxValueNameLen;
	unsigned long   MaxDataNameLen;
	unsigned long   ValueNameLen;
	LPBYTE			pData					= NULL;
	DWORD			dwSize;
	DWORD			dwType;
	DWORD			dwIndex;
	LPSTR			pszText					= NULL;
	LPSTR			KeyBuffer				= NULL;
	LPSTR			lpstrTheirBindings		= NULL;
	LPSTR			lpstrOurBindings		= NULL;
	LPSTR			lpstrTheirDriverEntryKey= NULL;
	LPSTR			lpstrTheirOldBindings	= NULL;
	LPSTR			pValueBuffer			= NULL;
	UINT			nValues;


	RegOpenKey(HKEY_LOCAL_MACHINE, PATHTOADAPTERLIST, &hNewKey);
	RegQueryInfoKey(hNewKey, NULL, NULL, NULL, NULL, 
				    NULL, NULL, NULL, &MaxValueNameLen,
				    &MaxDataNameLen, NULL, NULL);
	pData = (LPBYTE) calloc(sizeof(BYTE), MaxDataNameLen);
	dwSize = MaxDataNameLen;


	RegQueryValueEx(hNewKey, RealMac, 0,
				  &dwType, pData, &dwSize);
	RegCloseKey(hNewKey);
	
	/*allocate mem*/
	lpstrTheirBindings = (char *)malloc (1024);
	lpstrTheirDriverEntryKey = (char *)malloc (1024);
	lpstrTheirOldBindings = (char *)malloc (1024);
	/*clean mem*/
	ZeroMemory (lpstrTheirDriverEntryKey, (1024));
	ZeroMemory (lpstrTheirBindings, (1024));
	ZeroMemory (lpstrTheirOldBindings, (1024));

	/*build key to secured cards bindings and*/
	/*driver entry key*/
	/*pData is the subtree to the secured card where*/
	/*the bindings are*/
	strcat (lpstrTheirBindings, pData);

	strcpy (lpstrTheirDriverEntryKey, lpstrTheirBindings);

	strcpy (lpstrTheirOldBindings, BACKUP95BINDINGSKEY);

	strcat (lpstrTheirBindings, "\\");
	strcat (lpstrTheirBindings, "Bindings");

	/*build key to our bindings*/
	lpstrOurBindings = (char *)malloc (1024);
	ZeroMemory (lpstrOurBindings, (1024));

	strcat (lpstrOurBindings, EnumNetKey);
	strcat (lpstrOurBindings, "\\");
	strcat (lpstrOurBindings, gEnumNetSlot);
	strcat (lpstrOurBindings, "\\");
	strcat (lpstrOurBindings, "Bindings");

	/*move bindings to our card*/
	/*open both ours and their bind keys*/
	RegOpenKey(HKEY_LOCAL_MACHINE, lpstrOurBindings, &hOurBindKey);
	RegOpenKey(HKEY_LOCAL_MACHINE, lpstrTheirBindings, &hTheirBindKey);
	/*create key to hold moved bindings*/
	RegCreateKey(HKEY_LOCAL_MACHINE, lpstrTheirOldBindings, &hTheirOldBindings);
	
	/*Enumerate the values*/
	/*Determine number of keys to enumerate*/
	RegQueryInfoKey(hTheirBindKey, NULL, NULL, NULL, NULL, 
					NULL, NULL, &nValues, &MaxValueNameLen,
					NULL, NULL, NULL);

	/*Retrieve Registry values*/
	for (dwIndex = 0; dwIndex <= nValues; dwIndex++)
	{
		ValueNameLen = (MaxValueNameLen + 1);
		pValueBuffer = malloc(MaxValueNameLen + 1);
		ZeroMemory(pValueBuffer,(MaxValueNameLen + 1)); 
		RegEnumValue(hTheirBindKey, dwIndex, pValueBuffer, 
					&ValueNameLen, NULL, NULL, NULL, NULL); 

		/*Save off their old bindings if its not one of ours*/
		/*fix for addnew hardware wizard*/
		if  ((strstr(pValueBuffer, "PGPMAC") == NULL))
			RegSetValueEx(hTheirOldBindings, pValueBuffer , 0, REG_SZ, "", 1);
		/*if this is TCPIP copy to our card*/
		if (strstr(pValueBuffer, "MSTCP"))
		{
			/*set there mstcp to us*/
			RegSetValueEx(hOurBindKey, pValueBuffer , 0, REG_SZ, "", 1);
			/*add PGPMAC 0000 to them*/
			RegSetValueEx(hTheirBindKey, "PGPMAC\\0000" , 0, REG_SZ, "", 1);
			/*Temp fix for addnew hardware wizard
			popping up problem*/
			RegDeleteValue(hTheirBindKey, "PGPMAC\\000");
			RegDeleteValue(hTheirBindKey, "PGPMAC\\0001");
			RegDeleteValue(hTheirBindKey, "PGPMAC\\0002");
			RegDeleteValue(hTheirBindKey, "PGPMAC\\0003");
			RegDeleteValue(hTheirBindKey, "PGPMAC\\0004");
			RegDeleteValue(hTheirBindKey, "PGPMAC\\0005");
			/*remove there mstcp*/
			RegDeleteValue(hTheirBindKey, pValueBuffer); 
		}

		free(pValueBuffer);
	}/*for*/
	
	/*close bind keys*/
	if(hTheirOldBindings)
		RegCloseKey(hTheirOldBindings);
	if(hOurBindKey)
		RegCloseKey(hOurBindKey);
	if(hTheirBindKey)
		RegCloseKey(hTheirBindKey);

	/*Get the Real driver entry*/
	RegOpenKey(HKEY_LOCAL_MACHINE, lpstrTheirDriverEntryKey, &hTheirDriverKey);
	RegQueryValueEx(hTheirDriverKey, DRIVER, 0,
				  &dwType, pData, &dwSize);

	pszText = GetTxtAfterLastSlash(pData);
	KeyBuffer = (char *)malloc (1024);

	ZeroMemory (KeyBuffer, (1024));

	strcat (KeyBuffer, NetKey);
	strcat (KeyBuffer, "\\");
	strcat (KeyBuffer, gNetSlot);

	RegOpenKey(HKEY_LOCAL_MACHINE, KeyBuffer, &hNewKey);
	RegSetValueEx(hNewKey, "RealMac" , 0, REG_SZ, pszText, 24);
	
	RegCloseKey(hNewKey);
	RegCloseKey(hTheirDriverKey);
	free(lpstrTheirOldBindings);
	free(lpstrTheirDriverEntryKey);
	free(pszText);
	free(pData);
	free(KeyBuffer);
	free(lpstrOurBindings);
	free(lpstrTheirBindings);
	return TRUE;
}/*end*/


