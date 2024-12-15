/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Recipients.cpp,v 1.11.2.1.2.1 1998/11/12 03:12:47 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include "wab.h"
#include "UIutils.h"
#include "resource.h"
#include "Recipients.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "PGPcl.h"


enum {
    ieidPR_DISPLAY_NAME = 0,
    ieidPR_ENTRYID,
	ieidPR_OBJECT_TYPE,
    ieidMax
};
static const SizedSPropTagArray(ieidMax, ptaEid)=
{
    ieidMax,
    {
        PR_DISPLAY_NAME,
        PR_ENTRYID,
		PR_OBJECT_TYPE,
    }
};


enum {
    iemailPR_DISPLAY_NAME = 0,
    iemailPR_ENTRYID,
    iemailPR_EMAIL_ADDRESS,
    iemailPR_OBJECT_TYPE,
    iemailMax
};
static const SizedSPropTagArray(iemailMax, ptaEmail)=
{
    iemailMax,
    {
        PR_DISPLAY_NAME,
        PR_ENTRYID,
        PR_EMAIL_ADDRESS,
        PR_OBJECT_TYPE
    }
};

typedef struct _AdrBookTable
{
	char *szName;
	char *szEmail;
} AdrBookTable;


BOOL LoadWAB(HINSTANCE *phWAB, 
			 LPWABOPEN *ppWABOpen, 
			 LPADRBOOK *ppAdrBook, 
			 LPWABOBJECT *ppWABObject);

BOOL ReadWAB(LPADRBOOK pAdrBook, 
			 LPWABOBJECT pWABObject, 
			 AdrBookTable **ppTable,
			 UINT *pNumEntries);

void FreeProws(LPWABOBJECT pWABObject, LPSRowSet prows);

void FreeWAB(HINSTANCE hWAB, 
			 LPADRBOOK pAdrBook, 
			 LPWABOBJECT pWABObject,
			 AdrBookTable *pTable,
			 UINT nNumEntries);


PGPError GetRecipients(HWND hwnd,
					   char *szNames, 
					   PGPContextRef pgpContext, 
					   PGPtlsContextRef tlsContext,
					   RECIPIENTDIALOGSTRUCT *prds)
{
    HINSTANCE		hWAB = NULL;
    LPWABOPEN		pWABOpen = NULL;
    LPADRBOOK		pAdrBook = NULL; 
    LPWABOBJECT		pWABObject = NULL;
	AdrBookTable *	pTable = NULL;
	UINT			nNumEntries = 0;
	UINT			nIndex;
	DWORD			dwNumRecips = 0;
	BOOL			bGotRecipients = FALSE;
	BOOL			bAddressBook = FALSE;
	char			szTitle[256];
	char *			szName;
	char *			szEmail;
	char *			szNewRecipient;
	char **			szOldRecipients;
	char **			szRecipientArray = NULL;
	PGPKeySetRef	pubKeySet = NULL;
	PGPError		err = kPGPError_NoErr;

	if (LoadWAB(&hWAB, &pWABOpen, &pAdrBook, &pWABObject))
		if (ReadWAB(pAdrBook, pWABObject, &pTable, &nNumEntries))
			bAddressBook = TRUE;

	szName = strtok(szNames, ",;");
	while (szName)
	{
		szEmail = szName;

		if (bAddressBook)
		{
			for (nIndex=0; nIndex<nNumEntries; nIndex++)
			{
				if ((!_stricmp(szName, pTable[nIndex].szName)) &&
					(pTable[nIndex].szEmail != NULL))
				{
					szEmail = pTable[nIndex].szEmail;
					nIndex = nNumEntries;
				}
			}
		}

		szNewRecipient = (char *) calloc(sizeof(char), strlen(szEmail)+1);
		strcpy(szNewRecipient, szEmail);
		
		szOldRecipients = szRecipientArray;
		szRecipientArray = (char **) calloc(sizeof(char *), dwNumRecips+1);

		if (dwNumRecips > 0)
		{
			UINT i;

			for (i=0; i<dwNumRecips; i++)
				(szRecipientArray)[i] = szOldRecipients[i];
		}

		(szRecipientArray)[dwNumRecips] = szNewRecipient;
		dwNumRecips++;
		szName = strtok(NULL, ",;");
	}

	UIGetString(szTitle, sizeof(szTitle), IDS_RECIPIENTDIALOG);

	prds->Version = CurrentPGPrecipVersion;
	prds->hwndParent = hwnd;
	prds->szTitle = "Recipient Selection";
	prds->Context = pgpContext;
	prds->tlsContext = tlsContext;
	prds->SelectedKeySetRef = NULL;
	prds->szRecipientArray = szRecipientArray;
	prds->dwNumRecipients = dwNumRecips;
	prds->dwOptions = PGPCL_ASCIIARMOR;
	prds->dwFlags = 0;
	prds->dwDisableFlags = PGPCL_DISABLE_ASCIIARMOR |
							PGPCL_DISABLE_WIPEORIG;
	prds->AddedKeys = NULL;

	// If shift is pressed, force the dialog to pop.
	if (GetAsyncKeyState( VK_SHIFT) & 0x8000)
		prds->dwDisableFlags|=PGPCL_DISABLE_AUTOMODE;

	bGotRecipients = PGPclRecipientDialog(prds);

	if (prds->AddedKeys != NULL)
	{
		PGPUInt32 numKeys;

		PGPCountKeys(prds->AddedKeys, &numKeys);
		if (numKeys > 0)
			PGPclQueryAddKeys(pgpContext, tlsContext, hwnd, 
				prds->AddedKeys, NULL);

		PGPFreeKeySet(prds->AddedKeys);
		prds->AddedKeys = NULL;
	}

	if (bGotRecipients != TRUE)
	{
		err = kPGPError_UserAbort;
		goto GetRecipientsError;
	}

	FreeWAB(hWAB, pAdrBook, pWABObject, pTable, nNumEntries);
	return err;

GetRecipientsError:

	FreeRecipients(prds);
	FreeWAB(hWAB, pAdrBook, pWABObject, pTable, nNumEntries);
	return err;
}


void FreeRecipients(RECIPIENTDIALOGSTRUCT *prds)
{
	DWORD dwIndex;

	for (dwIndex=0; dwIndex<prds->dwNumRecipients; dwIndex++)
	{
		if ((prds->szRecipientArray)[dwIndex] != NULL)
		{
			free((prds->szRecipientArray)[dwIndex]);
			(prds->szRecipientArray)[dwIndex] = NULL;
		}
	}

	if (prds->SelectedKeySetRef != NULL)
	{
		PGPFreeKeySet(prds->SelectedKeySetRef);
		prds->SelectedKeySetRef = NULL;
	}
	
	return;
}


BOOL LoadWAB(HINSTANCE *phWAB, 
			 LPWABOPEN *ppWABOpen, 
			 LPADRBOOK *ppAdrBook, 
			 LPWABOBJECT *ppWABObject)
{
    HINSTANCE   hWAB = *phWAB;
    LPWABOPEN   pWABOpen = *ppWABOpen;
    LPADRBOOK   pAdrBook = *ppAdrBook; 
    LPWABOBJECT pWABObject = *ppWABObject;

	TCHAR	szWABDllPath[MAX_PATH];
	DWORD	dwType = 0;
	ULONG	cbData = sizeof(szWABDllPath);
	HKEY	hKey = NULL;
	
	*szWABDllPath = '\0';
	
	// First we look under the default WAB DLL path location in the
	// Registry. 
	// WAB_DLL_PATH_KEY is defined in wabapi.h
	//
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &hKey))
		RegQueryValueEx( hKey, "", NULL, &dwType, (LPBYTE) szWABDllPath, 
			&cbData);
	
	if(hKey) RegCloseKey(hKey);
	
	// if the Registry came up blank, we do a loadlibrary on the wab32.dll
	// WAB_DLL_NAME is defined in wabapi.h
	//
	hWAB = LoadLibrary((lstrlen(szWABDllPath)) 
					? szWABDllPath : WAB_DLL_NAME );

    if(hWAB)
    {
        // if we loaded the dll, get the entry point 
        //
        pWABOpen = (LPWABOPEN) GetProcAddress(hWAB, "WABOpen");

        if(pWABOpen)
        {
            HRESULT hr = E_FAIL;
            WAB_PARAM wp = {0};
            wp.cbSize = sizeof(WAB_PARAM);
            wp.szFileName = "";
        
            // if we choose not to pass in a WAB_PARAM object, 
            // the default WAB file will be opened up
            //
            hr = pWABOpen(ppAdrBook,ppWABObject,&wp,0);
        }
		else
			return FALSE;
    }
	else
		return FALSE;

	return TRUE;
}


BOOL ReadWAB(LPADRBOOK pAdrBook, 
			 LPWABOBJECT pWABObject, 
			 AdrBookTable **ppTable,
			 UINT *pNumEntries)
{
    ULONG ulObjType =   0;
	LPMAPITABLE lpAB =  NULL;
    LPTSTR * lppszArray=NULL;
    ULONG cRows =       0;
    LPSRowSet lpRow =   NULL;
	LPSRowSet lpRowAB = NULL;
    LPABCONT  lpContainer = NULL;
	int cNumRows = 0;
    int nRows=0;
	int nCount = 0;

    HRESULT hr = E_FAIL;

    ULONG lpcbEID;
	LPENTRYID lpEID = NULL;

    // Get the entryid of the root PAB container
    //
    hr = pAdrBook->GetPAB( &lpcbEID, &lpEID);

	ulObjType = 0;

    // Open the root PAB container
    // This is where all the WAB contents reside
    //
    hr = pAdrBook->OpenEntry(lpcbEID,
					    		(LPENTRYID)lpEID,
						    	NULL,
							    0,
							    &ulObjType,
							    (LPUNKNOWN *)&lpContainer);

	pWABObject->FreeBuffer(lpEID);

	lpEID = NULL;
	
    if(HR_FAILED(hr))
        goto exit;

    // Get a contents table of all the contents in the
    // WABs root container
    //
    hr = lpContainer->GetContentsTable( 0,
            							&lpAB);

    if(HR_FAILED(hr))
        goto exit;

    // Order the columns in the ContentsTable to conform to the
    // ones we want - which are mainly DisplayName, EntryID and
    // ObjectType
    // The table is gauranteed to set the columns in the order 
    // requested
    //
	hr =lpAB->SetColumns( (LPSPropTagArray)&ptaEid, 0 );

    if(HR_FAILED(hr))
        goto exit;


    // Reset to the beginning of the table
    //
	hr = lpAB->SeekRow( BOOKMARK_BEGINNING, 0, NULL );

    if(HR_FAILED(hr))
        goto exit;

    // Read all the rows of the table one by one
    //
	do {

		hr = lpAB->QueryRows(1,	0, &lpRowAB);

        if(HR_FAILED(hr))
            break;

        if(lpRowAB)
        {
            cNumRows = lpRowAB->cRows;

		    if (cNumRows)
		    {
                LPTSTR szName = lpRowAB->aRow[0].lpProps[ieidPR_DISPLAY_NAME].Value.lpszA;
				LPTSTR szEmail = NULL;
                LPENTRYID lpEID = (LPENTRYID) lpRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.lpb;
                ULONG cbEID = lpRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb;
				LPMAILUSER pMailUser = NULL;
				ULONG ulObjType = 0;
				ULONG ulValues;
				LPSPropValue lpPropArray;

				*ppTable = (AdrBookTable *) realloc(*ppTable, 
										sizeof(AdrBookTable) * (nCount+1));

				(*ppTable)[nCount].szName = 
										(char *) calloc(strlen(szName)+1, 1);
				strcpy((*ppTable)[nCount].szName, szName);

				if (lpRowAB->aRow[0].lpProps[ieidPR_OBJECT_TYPE].Value.l == 
					MAPI_MAILUSER)
				{
					pAdrBook->OpenEntry(cbEID,
								lpEID,
								NULL,         // interface
								0,            // flags
								&ulObjType,
								(LPUNKNOWN *)&pMailUser);

					if(pMailUser)
					{
						pMailUser->GetProps((LPSPropTagArray) &ptaEmail, 
									0, 
									&ulValues, 
									&lpPropArray);
						
						pMailUser->Release();
					}
					
					szEmail = lpPropArray[iemailPR_EMAIL_ADDRESS].Value.lpszA;
					
					(*ppTable)[nCount].szEmail = 
									(char *) calloc(strlen(szEmail)+1, 1);
					strcpy((*ppTable)[nCount].szEmail, szEmail);
	
					pWABObject->FreeBuffer(lpPropArray);
				}
				else
					(*ppTable)[nCount].szEmail = NULL;
				
				nCount++;
		    }
		    FreeProws(pWABObject, lpRowAB);		
        }

	}while ( SUCCEEDED(hr) && cNumRows && lpRowAB)  ;

exit:

	if ( lpContainer )
		lpContainer->Release();

	if ( lpAB )
		lpAB->Release();

	*pNumEntries = nCount;

	if (SUCCEEDED(hr))
		return TRUE;
	else
		return FALSE;
}


void FreeProws(LPWABOBJECT pWABObject, LPSRowSet prows)
{
	ULONG		irow;
	if (!prows)
		return;
	for (irow = 0; irow < prows->cRows; ++irow)
		pWABObject->FreeBuffer(prows->aRow[irow].lpProps);
	pWABObject->FreeBuffer(prows);
}


void FreeWAB(HINSTANCE hWAB, 
			 LPADRBOOK pAdrBook, 
			 LPWABOBJECT pWABObject,
			 AdrBookTable *pTable,
			 UINT nNumEntries)
{
	if (pTable && nNumEntries)
	{
		for (UINT i=0; i<nNumEntries; i++)
		{
			if (pTable[i].szName)
				free(pTable[i].szName);

			if (pTable[i].szEmail)
				free(pTable[i].szEmail);
		}

		free(pTable);
	}

	if(pAdrBook)
		pAdrBook->Release();
	
	if(pWABObject)
		pWABObject->Release();
	
	if(hWAB)
		FreeLibrary(hWAB);

	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
