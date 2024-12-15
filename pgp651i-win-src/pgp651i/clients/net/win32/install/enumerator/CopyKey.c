/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	

	$Id: CopyKey.c,v 1.2 1999/02/24 12:46:22 philipn Exp $
____________________________________________________________________________*/
#pragma message( "Compiling " __FILE__ ) 
#pragma message( "Last modified on " __TIMESTAMP__ ) 

#include <windows.h>

/*
 *  extract root key and subkey from string	(this is called by CopyRegistryKeyValues)
 */
static HKEY sParseRegKeyString (LPSTR pszKey, LPSTR* ppszSubKey)
{
	CHAR*	p;
	UINT	u;
	HKEY	hkey;

	/*validate parameters*/
	if (!pszKey)
		return NULL;

	if (!ppszSubKey)
		return NULL;

	/*initialize subkey string pointer*/
	*ppszSubKey = NULL;

	p = strchr (pszKey, '\\');
	if (!p) 
		return NULL;

	/*calculate length of root level key*/
	u = p - pszKey -1;

	hkey = NULL;
	if (strncmp (pszKey, "HKEY_CLASSES_ROOT", u) == 0)
		hkey = HKEY_CLASSES_ROOT;

	else if (strncmp (pszKey, "HKEY_CURRENT_USER", u) == 0)
		hkey = HKEY_CURRENT_USER;

	else if (strncmp (pszKey, "HKEY_LOCAL_MACHINE", u) == 0)
		hkey = HKEY_LOCAL_MACHINE;

	else if (strncmp (pszKey, "HKEY_USERS", u) == 0)
		hkey = HKEY_USERS;

	else if (strncmp (pszKey, "HKEY_PERFORMANCE_DATA", u) == 0)
		hkey = HKEY_PERFORMANCE_DATA;

#if(WINVER >= 0x0400)
	else if (strncmp (pszKey, "HKEY_CURRENT_CONFIG", u) == 0)
		hkey = HKEY_CURRENT_CONFIG;

	else if (strncmp (pszKey, "HKEY_DYN_DATA", u) == 0)
		hkey = HKEY_DYN_DATA;
#endif
	if (hkey != NULL)
		*ppszSubKey = p+1;

	return hkey;
}/*end*/


/*
 *  copy values of one key to another key
 */
BOOL CopyRegistryKeyValues (LPSTR pszSrcKey,	LPSTR pszDstKey)
{
	HKEY	hkeySrcRoot;
	HKEY	hkeyDstRoot;

	HKEY	hkeySrc;
	HKEY	hkeyDst;

	LPSTR	pszSrcSubKey;
	LPSTR	pszDstSubKey;

	LRESULT	lResultSrc;
	LRESULT	lResultDst;
	DWORD	dw;
	INT		index;

	CHAR	szValueName[256];
	UINT	uValueNameLength;
	DWORD	dwType;
	DWORD	dwSize;
	LPBYTE	pbData;


	/*get the keys and subkeys*/
	hkeySrcRoot = sParseRegKeyString (pszSrcKey, &pszSrcSubKey);
	if (!hkeySrcRoot)
		return FALSE;

	hkeyDstRoot = sParseRegKeyString (pszDstKey, &pszDstSubKey);
	if (!hkeyDstRoot)
		return FALSE;

	/*open handles to the keys*/
	lResultSrc = RegOpenKeyEx (hkeySrcRoot, pszSrcSubKey, 0,
										KEY_ALL_ACCESS, &hkeySrc);
	if (lResultSrc != ERROR_SUCCESS)
		return FALSE;

	lResultDst = RegCreateKeyEx (hkeyDstRoot, pszDstSubKey, 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyDst, &dw);
	if (lResultDst != ERROR_SUCCESS)
	{
		RegCloseKey (hkeySrc);
		return FALSE;
	}

	/*iterate the values*/
	index = 0;
	do 
	{
		uValueNameLength = sizeof(szValueName);
		lResultSrc = RegEnumValue (hkeySrc, index, 
						szValueName, &uValueNameLength,
						NULL, &dwType, NULL, &dwSize);

		if (lResultSrc == ERROR_SUCCESS)
		{
			uValueNameLength = sizeof(szValueName);
			pbData = malloc (dwSize);
			lResultSrc = RegEnumValue (hkeySrc, index, 
						szValueName, &uValueNameLength,
						NULL, &dwType, pbData, &dwSize);

			if (lResultSrc == ERROR_SUCCESS)
			{
				lResultDst = RegSetValueEx (hkeyDst, szValueName,
							0, dwType, pbData, dwSize);
			}/*if*/
			free (pbData);
		}/*if*/

		index++;
	} while ((lResultSrc == ERROR_SUCCESS) && 
			 (lResultDst == ERROR_SUCCESS));

	RegCloseKey (hkeySrc);
	RegCloseKey (hkeyDst);

	return ((lResultSrc == ERROR_NO_MORE_ITEMS) &&
			(lResultDst == ERROR_SUCCESS));
}/*end*/

