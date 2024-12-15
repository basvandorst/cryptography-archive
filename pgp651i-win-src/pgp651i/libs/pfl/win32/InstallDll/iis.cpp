//____________________________________________________________________________
//	Copyright (C) 1998 Network Associates Inc. and affiliated companies.
//	All rights reserved.
//	
//	iis.c -- iis administration functions.
//  Author: Philip Nathan
//
//Id: iis.c,v 1.1 1999/02/10 00:06:08 philipn Exp $_______________________
//IIS

#define INITGUID 
#define STATUS_STRING_SIZE 2048

#include <stdio.h>
#include <windows.h>
#include "CrtVDir.h"
#include <iostream.h>
#include <winnt.h>
#include <ks.h>
#include "iadmw.h"							// COM Interface header 
#include "iiscnfg.h"						// MD_ & IIS_MD_ #defines 
#include "iwamreg.h"
#include "atlBase.h"						// ATL support for smart pointers

#include "install.h"
#include "prototype.h"


#if PGPCERTD
	//IIS admin 
	CComPtr <IMSAdminBase> g_spAdminBase;  
	CComPtr <IWamAdmin> g_spWamAdmin;  

	char*	Title = "IIS Configuration";
	int		g_nOperation;						// CD == 1, RD == 2, CA == 3, RA == 4
	DWORD	g_dwPermissions = 1, g_dwSite = 1;	// default to "read" permission and site 1
	DWORD	g_dwAuthorization = 5;				//default to MD_AUTH_ANONYMOUS | MD_AUTH_NT
	BOOL	g_bInProc = true;
	BOOL	g_CD = true;
	LPSTR	lpstrPath;
	char*	pName;


	int CreateIISVDir(int arg)
	{
		//getting path
		#define PATHTOCERTDKEY "SOFTWARE\\Network Associates\\PGP Certificate Server"
		LPSTR lpstrCertPath;
		LPSTR lpstrCgiPath;
		LPSTR lpstrDocPath;
		DWORD dwType;
		DWORD dwSize;
		LPBYTE pValue		= NULL;
		HKEY hMainKey		= HKEY_LOCAL_MACHINE;
		HKEY hOpenKey		= NULL;
		char* Name = "AppPath";	
		BOOL bNewDefaultFile = 0;
		char* NewDefaultFile = "";
		BOOL bSetCustomErr = 0;

		//Get usage
		if	((1 != arg)	&&
			(0 != arg))		
		{
			MessageBox (GetFocus(), "Usage error: use either CD or RD\n"
									"for CommandLine.\n\n"
									"1 = Create Directory\n"
									"0 = Remove Directory\n", Title, 0 | MB_ICONERROR);
			return 0;
		}

		if(1 == arg)
		{
			g_CD = true;
		}
		if(0 == arg)
		{
			g_CD = false;
		}


		CHAR szStatus[STATUS_STRING_SIZE];
		HRESULT hres = S_OK;

		// initialize COM
		hres = CoInitialize(NULL);

		if (FAILED(hres))
		{
			wsprintf(szStatus, "CoInitializeEx failed: %d (0x%08x)", hres, hres);
			//MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);
			return 0;
		}

		// get a pointer to the IIS Admin Base Object
		hres = CoCreateInstance(CLSID_MSAdminBase, NULL, CLSCTX_ALL, 
				IID_IMSAdminBase, (void **) &g_spAdminBase);  

		if (FAILED(hres))  
		{
			wsprintf(szStatus, "CoCreateInstance failed for CLSID_MSAdminBase: %d (0x%08x)", hres, hres);
			//MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);
			CoUninitialize();
			return 0;  
		}

		// get a pointer to the IWamAdmin Object
		hres = CoCreateInstance(CLSID_WamAdmin, NULL, CLSCTX_ALL, 
				IID_IWamAdmin, (void **) &g_spWamAdmin);  

		if (FAILED(hres))  
		{
			wsprintf(szStatus, "CoCreateInstance failed for CLSID_WamAdmin: %d (0x%08x)", hres, hres);
			//MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);
			g_spAdminBase.Release();
			CoUninitialize();
			return 0;  
		}

		if(g_CD)
		{
			if (RegOpenKeyEx(hMainKey, PATHTOCERTDKEY, 0, KEY_ALL_ACCESS, &hOpenKey)
							== ERROR_SUCCESS)
			{
				lpstrCertPath = (char*)malloc (_MAX_PATH);
				lpstrCgiPath = (char*)malloc (_MAX_PATH);
				lpstrDocPath = (char*)malloc (_MAX_PATH);

				dwSize = _MAX_PATH;
				pValue = (LPBYTE) calloc(sizeof(BYTE), (DWORD)dwSize);
				
				RegQueryValueEx(hOpenKey, Name, NULL, &dwType, pValue, &dwSize);
				RegCloseKey (hOpenKey);	
			}
			else
			{
				MessageBox (GetFocus(), "Unable to open certd key.", Title, 0 | MB_ICONERROR);
				return 0;
			}

			memcpy (lpstrCertPath, pValue, dwSize);
			memcpy (lpstrCgiPath, pValue, dwSize);
			memcpy (lpstrDocPath, pValue, dwSize);

			strcat (lpstrCertPath, "\\Web\\HTDOCS");
			strcat (lpstrCgiPath, "\\Web\\cgi-bin");
			strcat (lpstrDocPath, "\\Documentation");

			// create the virtual root
			//g_dwPermissions set to default 1
			g_dwAuthorization = 4;
			bSetCustomErr = 1;
			if(!CreateVirtualRoot("certserver", lpstrCertPath, g_dwPermissions, g_dwSite, szStatus
								,bNewDefaultFile, NewDefaultFile, bSetCustomErr))
				MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);

			// create the virtual root
			bSetCustomErr = 0;
			if(!CreateVirtualRoot("certserver/docs", lpstrDocPath, g_dwPermissions, g_dwSite, szStatus
								,bNewDefaultFile, NewDefaultFile, bSetCustomErr))
				MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);

			// create the virtual root
			//g_dwPermissions reset to 4 for execute permission
			g_dwPermissions = 4;
			g_dwAuthorization =4;
			if(!CreateVirtualRoot("certserver/cgi-bin", lpstrCgiPath, g_dwPermissions, g_dwSite, szStatus
								,bNewDefaultFile, NewDefaultFile, bSetCustomErr))
				MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);

			// create the virtual root
			g_dwAuthorization = 0;
			g_dwPermissions = 0;
			if(!CreateVirtualRoot("pks", lpstrCgiPath, g_dwPermissions, g_dwSite, szStatus
								,bNewDefaultFile, NewDefaultFile, bSetCustomErr))
				MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);

			// create the virtual root
			//g_dwPermissions reset to 4 for execute permission
			bNewDefaultFile = 1;
			NewDefaultFile = "Add.exe";
			g_dwPermissions = 4;
			g_dwAuthorization = 1;
			if(!CreateVirtualRoot("pks/Add", lpstrCgiPath, g_dwPermissions, g_dwSite, szStatus
								,bNewDefaultFile, NewDefaultFile, bSetCustomErr))
				MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);

			// create the virtual root
			bNewDefaultFile = 1;
			NewDefaultFile = "Lookup.exe";
			g_dwPermissions = 4;
			g_dwAuthorization = 1;
			if(!CreateVirtualRoot("pks/Lookup", lpstrCgiPath, g_dwPermissions, g_dwSite, szStatus
								,bNewDefaultFile, NewDefaultFile, bSetCustomErr))
				MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);

			//cleanup
			if (pValue)
				free(pValue);
			if (lpstrCertPath)
				free(lpstrCertPath);
			if (lpstrCgiPath)
				free(lpstrCgiPath);
			if (lpstrDocPath)
				free(lpstrDocPath);
		}
		else
		{
			// delete the virtual root
			//dont show err msg
			if(!DeleteVirtualRoot("certserver/cgi-bin", g_dwSite, szStatus))
				Sleep(1);
				//MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);
			// delete the virtual root
			if(!DeleteVirtualRoot("certserver/docs", g_dwSite, szStatus))
				Sleep(1);
				//MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);
			// delete the virtual root
			if(!DeleteVirtualRoot("certserver", g_dwSite, szStatus))
				Sleep(1);
				//MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);
			// delete the virtual root
			if(!DeleteVirtualRoot("pks/Lookup", g_dwSite, szStatus))
				Sleep(1);
				//MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);
			// delete the virtual root
			if(!DeleteVirtualRoot("pks/Add", g_dwSite, szStatus))
				Sleep(1);
				//MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);
			// delete the virtual root
			if(!DeleteVirtualRoot("pks", g_dwSite, szStatus))
				Sleep(1);
				//MessageBox (GetFocus(), szStatus, Title, 0 | MB_ICONERROR);
		}

		// this needs to be released before we uninitialize COM
		g_spAdminBase.Release();
		g_spWamAdmin.Release();

		CoUninitialize();
		return 0;
	}

	/*
	Function :  CreateVirtualRoot

	Description:

		Creates the specified virtual root

	Arguments:

		szName - Name of the virtual root to add
		szPhysicalPath - Physical path of the virtual root
		dwPermissions - Access permissions for the virtual root
		dwSite - The site to which the virtual root is to be added
		szStatus - The function can report error descriptions in this string

	Return Value:

		Returns TRUE if successfull; otherwise FALSE.
	*/
	BOOL CreateVirtualRoot(
		LPSTR szName,
		LPSTR szPhysicalPath,
		DWORD dwPermissions,
		DWORD dwSite,
		CHAR szStatus[STATUS_STRING_SIZE],
		BOOL bNewDefaultFile,
		char* NewDefaultFile,
		BOOL bSetCustomErr
		)
	{
		CHAR szMetaPath[MAX_PATH];
		BOOL bResult;

		// Create the metabase path

		wsprintf(szMetaPath, "/LM/W3SVC/%d/ROOT/%s", dwSite, szName);

		// Create a new key for the virtual directory

		bResult = WrAddKey(szMetaPath);

		if (!bResult)
		{
			wsprintf(
				szStatus,
				"CreateVirtualRoot: Error %d (0x%08x) creating key for virtual root",
				GetLastError(),
				GetLastError()
				);

			goto Failed;
		}

		// Set the key type for the virtual directory

		bResult = WrSetData(
			szMetaPath,         // metabase path
			MD_KEY_TYPE,        // identifier
			METADATA_INHERIT,   // attributes
			IIS_MD_UT_FILE,     // user type
			STRING_METADATA,    // data type
			0,                  // data size (not used for STRING_METADATA)
			"IIsWebVirtualDir"  // data
			);

		if (!bResult)
		{
			wsprintf(szStatus, "CreateVirtualRoot: Error %d (0x%08x)setting key type for virtual root",
				GetLastError(), GetLastError());

			goto Failed;
		}
            
		// Set the VRPath for the virtual directory

		bResult = WrSetData(
			szMetaPath,         // metabase path
			MD_VR_PATH,         // identifier
			METADATA_INHERIT,   // attributes
			IIS_MD_UT_FILE,     // user type
			STRING_METADATA,    // data type
			0,                  // data size (not used for STRING_METADATA)
			szPhysicalPath      // data
			);

		if (!bResult)
		{
			wsprintf(szStatus, "CreateVirtualRoot: Error %d (0x%08x) setting vrpath for virtual root",
					GetLastError(), GetLastError());

			goto Failed;
		}

		// Set the permissions for the virtual directory

		bResult = WrSetData(
			szMetaPath,         // metabase path
			MD_ACCESS_PERM,     // identifier
			METADATA_INHERIT,   // attributes
			IIS_MD_UT_FILE,     // user type
			DWORD_METADATA,     // data type
			0,                  // data size (not used for DWORD_METADATA)
			&dwPermissions      // data
			);



		if (!bResult)
		{
			wsprintf(szStatus, "CreateVirtualRoot: Error %d (0x%08x) setting permissions for virtual root",
					GetLastError(), GetLastError());

			goto Failed;
		}
		
		
		// Set the Authorization for the virtual directory
		bResult = WrSetData(
			szMetaPath,         // metabase path
			MD_AUTHORIZATION,   // identifier
			METADATA_INHERIT,   // attributes
			IIS_MD_UT_FILE,     // user type
			DWORD_METADATA,     // data type
			0,                  // data size (not used for DWORD_METADATA)
			&g_dwAuthorization	// data
			);

		if (!bResult)
		{
			wsprintf(szStatus, "CreateVirtualRoot: Error %d (0x%08x) setting Authorization for virtual root",
					GetLastError(), GetLastError());
		}

		if(bNewDefaultFile)
		{
			// Set a new default load file
			bResult = WrSetData(
				szMetaPath,					// metabase path
				MD_DEFAULT_LOAD_FILE,		// identifier
				METADATA_INHERIT,			// attributes
				IIS_MD_UT_FILE,				// user type
				STRING_METADATA,			// data type
				0,							// data size (not used for STRING_METADATA)
				NewDefaultFile				// data
				);

			if (!bResult)
			{
				wsprintf(szStatus, "CreateVirtualRoot: Error %d (0x%08x) setting new default file for virtual root",
						GetLastError(), GetLastError());
			}
		}
		
//DONT SET CUSTOM ERROR UNTIL PROPER WAY IS FOUND
//		if(bSetCustomErr)
//		{
//			LPSTR lpstrCustomError;
//			LPSTR StringsA [5] = {"404,", "*,", "FILE,", szPhysicalPath, "\\error_doc.html"};
//			int i = 0;
//			int iTotalLength = 0;
//		
//			lpstrCustomError = (char*)malloc(strlen(szPhysicalPath)+27/*total chars*/+5/*nulls*/);
//			strcpy(lpstrCustomError, StringsA[0]);
//			strcat(lpstrCustomError, StringsA[1]);
//			strcat(lpstrCustomError, StringsA[2]);
//			strcat(lpstrCustomError, StringsA[3]);
//			strcat(lpstrCustomError, StringsA[4]);
//			//add extra null to end
//			lpstrCustomError [strlen (lpstrCustomError) + 1] = '\0';
//
//			// Set new custom error
//			bResult = WrSetData(
//				szMetaPath,         // metabase path
//				MD_CUSTOM_ERROR,    // identifier
//				METADATA_INHERIT,   // attributes
//				IIS_MD_UT_SERVER,   // user type
//				STRING_METADATA,	// data type
//				0,
//				lpstrCustomError	// data
//				);
//
//			if (!bResult)
//			{
//				wsprintf(szStatus, "CreateVirtualRoot: Error %d (0x%08x) setting custom error for virtual root",
//						GetLastError(), GetLastError());
//			}
//		}

		// Commit the changes and return

		g_spAdminBase->SaveData();

		wsprintf(szStatus, "CreateVirtualRoot completed successfully.");
    
		return TRUE;

	Failed:

		return FALSE;
	}

	/*
	Function :  DeleteVirtualRoot

	Description:

		Deletes the specified virtual root

	Arguments:

		szName - Name of the virtual root to be deleted
		dwSite - The site from which the virtual root will be deleted
		szStatus - The function can report error descriptions in this string

	Return Value:

		Returns TRUE if successfull; otherwise FALSE.

	*/

	BOOL DeleteVirtualRoot(
		LPSTR szName,
		DWORD dwSite,
		CHAR szStatus[STATUS_STRING_SIZE]
		)
	{
		CHAR szMetaPath[MAX_PATH];
		CHAR szParent[MAX_PATH];
		CHAR szVDir[MAX_PATH];
		LPSTR szPtr;
		LPWSTR szwParent = NULL;
		LPWSTR szwVDir = NULL;
		METADATA_HANDLE hMetaData;
		BOOL fMetaData = FALSE;
		BOOL bRes;
		HRESULT hres;
		DWORD dwLastError;

		wsprintf(szMetaPath, "/LM/W3SVC/%d/ROOT/%s", dwSite, szName);

		strcpy(szParent, szMetaPath);

		szPtr = strrchr(szParent, '/');

		strcpy(szVDir, szPtr + 1);

		*szPtr = 0;

		szwParent = MakeUnicode(szParent, 1);
		szwVDir = MakeUnicode(szVDir, 1);

		if (!szwParent || !szwVDir)
		{
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);

			wsprintf(
				szStatus,
				"DeleteVirtualDirectory failed: %d",
				GetLastError()
				);

			goto Failed;
		}

		// Delete any applications on this directory

		bRes = DeleteApplication(
			szMetaPath,     // metabase path
			FALSE,          // not recoverable
			TRUE,           // recursive
			szStatus        // status string
			);

		if (!bRes)
			goto Failed;

		// Get a handle to the metabase

		hres = g_spAdminBase->OpenKey(
			METADATA_MASTER_ROOT_HANDLE,
			szwParent,
			METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE,
			60000,
			&hMetaData
			);

		if (FAILED(hres))
		{
			SetLastError(hres);

			wsprintf(
				szStatus,
				"DeleteVirtualRoot: Error %d (0x%08x) getting handle to metabase",
				hres,
				hres
				);

			goto Failed;
		}
		else
			fMetaData = TRUE;

		// Do the work

		hres = g_spAdminBase->DeleteKey(
			hMetaData,
			szwVDir
			);

		if (FAILED(hres))
		{
			SetLastError(hres);

			wsprintf(
				szStatus,
				"DeleteVirtualRoot: IMSAdminBase->DeleteKey returned %d (0x%08x)",
				hres,
				hres
				);

			goto Failed;
		}

		// Commit the changes

		g_spAdminBase->SaveData();

		// Clean up and return

		g_spAdminBase->CloseKey(hMetaData);

		LocalFree(szwParent);
		LocalFree(szwVDir);

		wsprintf(szStatus, "DeleteVirtualRoot completed successfully.");

		return TRUE;

	Failed:

		dwLastError = GetLastError();

		if (fMetaData)
			g_spAdminBase->CloseKey(hMetaData);

		LocalFree(szwParent);
		LocalFree(szwVDir);

		SetLastError(dwLastError);

		return FALSE;
	}

	/*
	Function :  DeleteApplication

	Description:

		Deletes the specified application

	Arguments:

		szMetaPath - The metabase path of the application to be deleted
		fRecoverable - If this flag is true, the app will be recoverable
		fRecursive - If this flag is true, all sub-applications will also be deleted
		szStatus - The function can report error descriptions in this string

	Return Value:

		Returns TRUE if successfull; otherwise FALSE.

	*/
	BOOL DeleteApplication(
		LPSTR szMetaPath,
		BOOL fRecoverable,
		BOOL fRecursive,
		CHAR szStatus[STATUS_STRING_SIZE]
		)
	{
		CHAR szFullMetaPath[MAX_PATH];
		LPWSTR szwMetaPath = NULL;
		HRESULT hres;
		DWORD dwLastError;

		// Sanitize the metabase path and get a unicode version
    
		if (*szMetaPath == '/')
			strcpy(szFullMetaPath, szMetaPath);
		else
			wsprintf(szFullMetaPath, "/LM/%s", szMetaPath);

		szwMetaPath = MakeUnicode(szFullMetaPath, 1);

		if (!szwMetaPath)
		{
			wsprintf(
				szStatus,
				"DeleteApplication failed: %d",
				GetLastError()
				);

			goto Failed;
		}

		// Do the work

		if (fRecoverable)
		{
			hres = g_spWamAdmin->AppDeleteRecoverable(szwMetaPath, fRecursive);

			if (FAILED(hres))
			{
				SetLastError(hres);

				wsprintf(
					szStatus,
					"DeleteApplication: IWamAdmin->AppDeleteRecoverable failed: %d (0x%08x)",
					hres,
					hres
					);

				goto Failed;
			}
		}
		else
		{
			hres = g_spWamAdmin->AppDelete(szwMetaPath, fRecursive);

			if (FAILED(hres))
			{
				SetLastError(hres);

				wsprintf(
					szStatus,
					"DeleteApplication: IWamAdmin->AppDelete failed: %d (0x%08x)",
					hres,
					hres
					);

				goto Failed;
			}
		}

		// Clean up

		LocalFree(szwMetaPath);

		wsprintf(szStatus, "DeleteApplication completed successfully.");

		return TRUE;

	Failed:

		dwLastError = GetLastError();
    
		LocalFree(szwMetaPath);

		SetLastError(dwLastError);
        
		return FALSE;
	}

	/*
	Function :  MakeUnicode

	Description:

		Returns a unicode version of the provided string

	Arguments:

		szString - The string to be translated

	Return Value:

		Returns a pointer to the unicode string if successful, NULL if not

	*/
	LPWSTR MakeUnicode(
		LPSTR szString,
		BOOL bRegularString
		)
	{
		LPWSTR szwRet;
		DWORD dwNumChars;
		int	NumChars;

		if (bRegularString)
			NumChars = -1;
		else
			NumChars = (strlen(szString) + 2);

		// Allocate buffer for the returned wide string

		dwNumChars = MultiByteToWideChar(
			CP_ACP,         // code page
			MB_PRECOMPOSED, // flags
			szString,       // ANSI source string
			-1,             // source string length
			NULL,           // destination buffer
			0               // size of destination buffer in chars
			);

		szwRet = (LPWSTR)LocalAlloc(LPTR, dwNumChars * sizeof(WCHAR));

		if (!szwRet)
			return NULL;

		// Do the conversion

		MultiByteToWideChar(
			CP_ACP,         // code page
			MB_PRECOMPOSED, // flags
			szString,       // ANSI source string
			NumChars,             // source string length
			szwRet,         // destination buffer
			dwNumChars      // size of destination buffer in chars
			);
   
		return szwRet;
	}

	/*
	Function :  WrAddKey

	Description:

		Creates the specified metabase path

	Arguments:

		szMetaPath - The metabase path to be created

	Return Value:

		Returns TRUE if successfull; otherwise FALSE.

	*/
	BOOL WrAddKey(
		LPSTR szMetaPath
		)
	{
		CHAR szParent[MAX_PATH];
		CHAR szDir[MAX_PATH];
		LPWSTR szwPath = NULL;
		LPWSTR szwNew = NULL;
		LPSTR szPtr;
		METADATA_HANDLE hMetaData;
		BOOL fMetaData = FALSE;
		HRESULT hres;
		DWORD dwLastError;

		// Parse the supplied metabase path into parent and new directory

		strcpy(szParent, szMetaPath);

		szPtr = strrchr(szParent, '/');

		if (!szPtr)
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			goto Failed;
		}

		*szPtr = 0;

		strcpy(szDir, szPtr + 1);

		// Open the metabase

		szwPath = MakeUnicode(szParent, 1);
		szwNew = MakeUnicode(szDir, 1);

		if (!szwPath || !szwNew)
		{
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			goto Failed;
		}

		hres = g_spAdminBase->OpenKey(
			METADATA_MASTER_ROOT_HANDLE,
			szwPath,
			METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE,
			60000,
			&hMetaData
			);

		if (FAILED(hres))
		{
			SetLastError(hres);
			goto Failed;
		}
		else
			fMetaData = TRUE;

		// Create the new key

		hres = g_spAdminBase->AddKey(
			hMetaData,
			szwNew
			);

		if (FAILED(hres))
		{
			SetLastError(hres);
			goto Failed;
		}

		// Clean up

		hres = g_spAdminBase->CloseKey(hMetaData);

		fMetaData = FALSE;

		if(szwNew)
		LocalFree(szwNew);

		if(szwPath)
			LocalFree(szwPath);

		return TRUE;

	Failed:

		dwLastError = GetLastError();

		if (fMetaData)
			g_spAdminBase->CloseKey(hMetaData);

		LocalFree(szwNew);
		LocalFree(szwPath);


		SetLastError(dwLastError);

		return FALSE;
	}

	/*
	Function :  WrSetData

	Description:

		Sets the specified data in the metabase

	Arguments:

		szMetaPath - The metabase path where the data will be set
		dwIdentifier - The metabase data identifier
		dwAttributes - The data attributes
		dwUserType - The metabase user type
		dwDataType - The type of data being set
		dwDataSize - The size of the data being set
		pData - The actual data

	Return Value:

		Returns TRUE if successfull; otherwise FALSE.

	*/
	BOOL WrSetData(
		LPSTR szMetaPath,
		DWORD dwIdentifier,
		DWORD dwAttributes,
		DWORD dwUserType,
		DWORD dwDataType,
		DWORD dwDataSize,
		LPVOID pData
		)
	{
		LPWSTR szwPath = NULL;
		LPWSTR szwValue = NULL;
		METADATA_RECORD mdRecord;
		METADATA_HANDLE hMetaData;
		BOOL fMetaData = FALSE;
		HRESULT hres;
		DWORD dwLastError;


		// Get a handle to the metabase location specified

		szwPath = MakeUnicode(szMetaPath, 1);

		if (!szwPath)
			goto Failed;

		hres = g_spAdminBase->OpenKey(
			METADATA_MASTER_ROOT_HANDLE,
			szwPath,
			METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE,
			60000,
			&hMetaData
			);

		if (FAILED(hres))
		{
			SetLastError(hres);
			goto Failed;
		}
		else
			fMetaData = TRUE;

		//
		// Populate the metadata record
		//

		mdRecord.dwMDIdentifier = dwIdentifier;
		mdRecord.dwMDAttributes = dwAttributes;
		mdRecord.dwMDUserType = dwUserType;
		mdRecord.dwMDDataType = dwDataType;
		mdRecord.dwMDDataTag = 0;

		switch (mdRecord.dwMDDataType)
		{
		case ALL_METADATA:
			SetLastError(ERROR_NOT_SUPPORTED);
			goto Failed;

		case BINARY_METADATA:
			mdRecord.dwMDDataLen = dwDataSize;
			mdRecord.pbMDData = (PBYTE)pData;
			break;

		case DWORD_METADATA:
			mdRecord.dwMDDataLen = sizeof(DWORD);
			mdRecord.pbMDData = (PBYTE)pData;
			break;

		case EXPANDSZ_METADATA:
			SetLastError(ERROR_NOT_SUPPORTED);
			goto Failed;

		case STRING_METADATA:
			szwValue = MakeUnicode((LPSTR)pData, 1);

			if (!szwValue)
				goto Failed;

			mdRecord.dwMDDataLen = sizeof(WCHAR) * (wcslen(szwValue) + 1);
			mdRecord.pbMDData = (PBYTE)szwValue;
			break;

		case MULTISZ_METADATA:
			szwValue = MakeUnicode((LPSTR)pData, 0);

			if (!szwValue)
				goto Failed;

			mdRecord.dwMDDataLen = sizeof(WCHAR) * (wcslen(szwValue) + 2);
			mdRecord.pbMDData = (PBYTE)szwValue;
			break;

		default:
			SetLastError(ERROR_INVALID_PARAMETER);
			goto Failed;
		}

		//
		// Do the work
		//

		hres = g_spAdminBase->SetData(
			hMetaData,
			L"/",
			&mdRecord
			);

		if (FAILED(hres))
		{
			SetLastError(hres);
			goto Failed;
		}

		//
		// Close the metabase
		//

		g_spAdminBase->CloseKey(hMetaData);
		fMetaData = FALSE;

		//
		// Clean up
		//

		LocalFree(szwPath);
		//LocalFree(szwValue);

		return TRUE;

	Failed:

		dwLastError = GetLastError();

		if (fMetaData)
			g_spAdminBase->CloseKey(hMetaData);

		LocalFree(szwPath);
		LocalFree(szwValue);

		SetLastError(dwLastError);

		return FALSE;
	}
#else
	int CreateIISVDir(int arg)
	{
		return 0;
	}
#endif