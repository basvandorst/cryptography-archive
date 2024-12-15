/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Recipients.cpp,v 1.21.2.5.2.1 1998/11/12 03:14:00 heller Exp $
____________________________________________________________________________*/
#include "stdinc.h"
#include "Recipients.h"
#include "UIutils.h"
#include "pgpKeys.h"
#include "PGPcl.h"
#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpSDKPrefs.h"
#include "pgpClientPrefs.h"
#include "pgpUtilities.h"
#include "resource.h"

#ifndef PR_EMS_AB_PROXY_ADDRESSES
#define PR_EMS_AB_PROXY_ADDRESSES	PROP_TAG( PT_MV_TSTRING,    0x800F)
#endif


static void ResolveEntries(LPADRBOOK pAddrBook, LPABCONT pContainer, 
						   ULONG ulNumRows, LPADRENTRY aEntries, 
						   RECIPIENTDIALOGSTRUCT *prds, DWORD *pdwNumRecip, 
						   DWORD *pdwRecipIndex);

BOOL GetSMTPAddress(ULONG ulNum, LPSTR *lppszProxy,
					RECIPIENTDIALOGSTRUCT *prds,
					DWORD *pdwRecipIndex, DWORD *pdwNumRecip,
					char *szAddress, BOOL *pbRecipAdded);


PGPError GetRecipients(IExchExtCallback *pmecb, 
					   PGPContextRef context,
					   PGPtlsContextRef tlsContext,
					   RECIPIENTDIALOGSTRUCT *prds)
{
	LPADRLIST		pAddrList		= NULL;
	LPADRBOOK		pAddrBook		= NULL;
	LPABCONT		pContainer		= NULL;
	LPENTRYID		pEntryID		= NULL;
	DWORD			dwNumRecip		= 0;
	ULONG			ulRow			= 0;
	ULONG			ulIndex			= 0;
	ULONG			ulEntrySize		= 0;
	char			szTitle[256];
	ULONG			ulNumRows		= 0;
	DWORD			dwRecipIndex	= 0;
	HWND			hwnd			= NULL;
	BOOL			bGotRecipients	= FALSE;
	PGPError		err				= kPGPError_NoErr;

	pgpAssert(pmecb);
	pgpAssert(prds);
	pgpAssert(prds->OriginalKeySetRef);
	pgpAssert(PGPRefIsValid(context));

	pmecb->GetWindow(&hwnd);
	if (!hwnd)
		hwnd = GetTopWindow(NULL);

	UIGetString(szTitle, sizeof(szTitle), IDS_RECIPIENTTITLE);

	prds->Version = CurrentPGPrecipVersion;
	prds->hwndParent = hwnd;
	prds->szTitle = szTitle;
	prds->Context = context;
	prds->tlsContext = tlsContext;
	prds->SelectedKeySetRef = NULL;
	prds->szRecipientArray = NULL;
	prds->dwOptions = PGPCL_ASCIIARMOR;
	prds->dwFlags = 0;
	prds->dwDisableFlags = PGPCL_DISABLE_ASCIIARMOR |
							PGPCL_DISABLE_WIPEORIG;
	prds->AddedKeys = NULL;

	pmecb->GetRecipients(&pAddrList);
	if (pAddrList)
	{
		LPSRowSet pSearch = NULL;
		ULONG ulMatch = ULONG_MAX;
		ULONG ulObjType = 0;
		SizedSPropTagArray(6, propTags) = {
			6, {PR_DISPLAY_NAME, PR_EMAIL_ADDRESS, PR_OBJECT_TYPE, PR_ENTRYID,
				PR_ADDRTYPE, PR_EMS_AB_PROXY_ADDRESSES}};

		pmecb->GetSession(NULL, &pAddrBook);
		pAddrBook->GetSearchPath(0, &pSearch);
		pAddrBook->PrepareRecips(0, (LPSPropTagArray) &propTags, pAddrList);

		for (ulRow=0; ulRow<pSearch->cRows; ulRow++)
		{
			for(ulIndex=0; ulIndex<pSearch->aRow[ulRow].cValues; ulIndex++)
			{
				switch (pSearch->aRow[ulRow].lpProps[ulIndex].ulPropTag)
				{
				case PR_DISPLAY_TYPE:
					if (pSearch->aRow[ulRow].lpProps[ulIndex].Value.l == 
						DT_GLOBAL)
						ulMatch = ulRow;
					break;

				case PR_ENTRYID:
					ulEntrySize = pSearch->aRow[ulRow].
									lpProps[ulIndex].Value.bin.cb;
				
					pEntryID = (LPENTRYID) pSearch->aRow[ulRow].
									lpProps[ulIndex].Value.bin.lpb;
					break;
				}
			}

			if (ulMatch != ULONG_MAX)
				ulRow = pSearch->cRows;
		}

		if (ulMatch != ULONG_MAX)
			pAddrBook->OpenEntry(ulEntrySize, pEntryID, &IID_IABContainer,
							0, &ulObjType, (IUnknown **) &pContainer);

		FreeProws(pSearch);
		dwNumRecip = pAddrList->cEntries;
	}
	else
	{
		prds->dwNumRecipients = 0;
		return err;
	}
				
	ulNumRows = dwNumRecip;
	prds->szRecipientArray = (char **) calloc(sizeof(char *), dwNumRecip);
	
	ResolveEntries(pAddrBook, pContainer, ulNumRows, 
		pAddrList->aEntries, prds, &dwNumRecip, &dwRecipIndex);

	prds->dwNumRecipients = dwNumRecip;

	if (pContainer != NULL)
		pContainer->Release();

	if (pAddrBook != NULL)
		pAddrBook->Release();

	// If shift is pressed, force the dialog to pop.
	if (GetAsyncKeyState( VK_SHIFT) & 0x8000)
		prds->dwDisableFlags|=PGPCL_DISABLE_AUTOMODE;

	bGotRecipients = PGPclRecipientDialog(prds);

	if (prds->AddedKeys != NULL)
	{
		PGPUInt32 numKeys;

		PGPCountKeys(prds->AddedKeys, &numKeys);
		if (numKeys > 0)
			PGPclQueryAddKeys(context, tlsContext, hwnd, 
				prds->AddedKeys, NULL);

		PGPFreeKeySet(prds->AddedKeys);
		prds->AddedKeys = NULL;
	}

	if (bGotRecipients != TRUE)
	{
		err = kPGPError_UserAbort;
		goto GetRecipientsError;
	}

	return err;

GetRecipientsError:

	FreeRecipients(prds);
	return err;
}


void FreeRecipients(RECIPIENTDIALOGSTRUCT *prds)
{
	DWORD dwIndex;

	if (prds == NULL)
		return;

	if (prds->szRecipientArray != NULL)
	{
		for (dwIndex=0; dwIndex<prds->dwNumRecipients; dwIndex++)
		{
			if ((prds->szRecipientArray)[dwIndex] != NULL)
			{
				free((prds->szRecipientArray)[dwIndex]);
				(prds->szRecipientArray)[dwIndex] = NULL;
			}
		}
		
		free(prds->szRecipientArray);
		prds->szRecipientArray = NULL;
	}

	if (IsntNull(prds->SelectedKeySetRef))
	{
		PGPFreeKeySet(prds->SelectedKeySetRef);
		prds->SelectedKeySetRef = NULL;
	}
	
	return;
}


static void ResolveEntries(LPADRBOOK pAddrBook, LPABCONT pContainer, 
						   ULONG ulNumRows, LPADRENTRY aEntries, 
						   RECIPIENTDIALOGSTRUCT *prds, DWORD *pdwNumRecip, 
						   DWORD *pdwRecipIndex)
{
	LPENTRYID		pEntryID		= NULL;
	ULONG			ulRow			= 0;
	ULONG			ulIndex			= 0;
	ULONG			ulEmailAddr		= 0;
	ULONG			ulDisplayName	= 0;
	ULONG			ulType			= 0;
	ULONG			ulEntryID		= 0;
	ULONG			ulAddrType		= 0;
	ULONG			ulNumProxies	= 0;
	ULONG			ulProxy			= 0;
	ULONG			ulEntrySize		= 0;
	int				nChar			= 0;
	int				nRecipLength	= 0;
	char *			szRecip			= NULL;
	char *			szAddrType		= NULL;
	char			szSMTP[1024];
	char **			pszProxy		= NULL;
	BOOL			bGotSMTP		= TRUE;
	BOOL			bRecipAdded		= FALSE;

	for (ulRow=0; ulRow<ulNumRows; ulRow++)
	{
		ulEmailAddr		= ULONG_MAX;
		ulDisplayName	= ULONG_MAX;
		ulType			= ULONG_MAX;
		ulEntryID		= ULONG_MAX;
		ulAddrType		= ULONG_MAX;
		ulProxy			= ULONG_MAX;
		
		for (	ulIndex=0;
				ulIndex<aEntries[ulRow].cValues; 
				ulIndex++)
		{
			switch (aEntries[ulRow].rgPropVals[ulIndex].ulPropTag)
			{
			case PR_DISPLAY_NAME:
				ulDisplayName = ulIndex;
				break;
			
			case PR_EMAIL_ADDRESS:
				ulEmailAddr = ulIndex;
				break;
			
			case PR_OBJECT_TYPE:
				ulType = ulIndex;
				break;

			case PR_ENTRYID:
				ulEntryID = ulIndex;
				break;

			case PR_ADDRTYPE:
				ulAddrType = ulIndex;
				break;

			case PR_EMS_AB_PROXY_ADDRESSES:
				ulProxy = ulIndex;
			}
		}

		if ((ulDisplayName == ULONG_MAX) && (ulEmailAddr == ULONG_MAX))
		{
			(*pdwNumRecip)--;
			continue;
		}
		
		if (ulEmailAddr != ULONG_MAX)
		{
			szRecip = aEntries[ulRow].rgPropVals[ulEmailAddr].Value.lpszA;

			if (szRecip)
			{
				if (strlen(szRecip))
				{
					ulIndex = ulEmailAddr;
				}
				else
					ulIndex = ulDisplayName;
			}
			else
				ulIndex = ulDisplayName;
		}
		else
			ulIndex = ulDisplayName;
		
		// Older versions of Exchange return an invalid number
		// of recipients, so we have to check each recipient
		// for validity
		
		// First, make sure there's a pointer to the recipient
		// entry
		if (!&(aEntries[ulRow]))
		{
			(*pdwNumRecip)--;
			continue;
		}
		
		// Now see if the recipient has any properties
		if (!(aEntries[ulRow].cValues) ||
			((aEntries[ulRow].cValues-1) < ulIndex))
		{
			(*pdwNumRecip)--;
			continue;
		}
		
		// Check to see if this is a distribution list. If so,
		// process it

		if (ulType != ULONG_MAX)
		{
			if (aEntries[ulRow].rgPropVals[ulType].Value.l ==
				MAPI_DISTLIST)
			{
				LPDISTLIST pDistList = NULL;
				LPMAPITABLE pTable = NULL;
				SizedSPropTagArray(6, tagaDistList) = {6, 
					{PR_DISPLAY_NAME, PR_EMAIL_ADDRESS, PR_ENTRYID, 
					PR_ADDRTYPE, PR_OBJECT_TYPE,
					PR_EMS_AB_PROXY_ADDRESSES}};
				SRowSet *pRows = NULL;
				ULONG ulObjType = 0;

				ulEntrySize = aEntries[ulRow].
								rgPropVals[ulEntryID].Value.bin.cb;
				
				pEntryID = (LPENTRYID) aEntries[ulRow].
								rgPropVals[ulEntryID].Value.bin.lpb;
				
				pAddrBook->OpenEntry(ulEntrySize, pEntryID, &IID_IDistList,
								0, &ulObjType, (IUnknown**)&pDistList);
				
				pDistList->GetContentsTable(0, &pTable);
				HrQueryAllRows(pTable, (SPropTagArray *)&tagaDistList, NULL, 
					NULL, 0, &pRows);
				
				(*pdwNumRecip) += pRows->cRows - 1;
				prds->szRecipientArray = (char **) 
											realloc(prds->szRecipientArray, 
											sizeof(char *) * (*pdwNumRecip));

				ResolveEntries(pAddrBook, pContainer, pRows->cRows, 
					(LPADRENTRY) pRows->aRow, prds, pdwNumRecip, 
					pdwRecipIndex);

				pTable->Release();
				pDistList->Release();
				continue;
			}
		}

		szRecip = aEntries[ulRow].rgPropVals[ulIndex].Value.lpszA;

		// Make sure e-mail address is an SMTP address. If not, try
		// to fetch the SMTP address

		bGotSMTP = FALSE;
		bRecipAdded = FALSE;
		
		if (ulAddrType != ULONG_MAX)
			szAddrType = aEntries[ulRow].
							rgPropVals[ulAddrType].Value.lpszA;
		else
		{
			(*pdwNumRecip)--;
			continue;
		}

		if (szAddrType)
		{
			if (strstr(szAddrType, "SMTP"))
				bGotSMTP = TRUE;
			else
			{
				SizedADRLIST(1, newAddress);
				newAddress.cEntries = 1;
				ulNumProxies = 0;
				pszProxy = NULL;

				if ((ulProxy == ULONG_MAX) && (pContainer != NULL))
				{
					FlagList flags;
					char *szName = NULL;
					char *szNameCopy = NULL;
					char *szFirstName = NULL;
					char *szLastName = NULL;
					char *szPutNull = NULL;
					HRESULT result;
					SizedSPropTagArray(2, propTags) = {
						2, {PR_DISPLAY_NAME, PR_EMS_AB_PROXY_ADDRESSES}};

					flags.cFlags = 1;
					flags.ulFlag[0] = MAPI_UNRESOLVED;

					newAddress.aEntries[0].ulReserved1 = 0;
					newAddress.aEntries[0].cValues = 2;
					MAPIAllocateBuffer(sizeof(SPropValue)*2, (void **)
						&(newAddress.aEntries[0].rgPropVals));

					newAddress.aEntries[0].rgPropVals[0].ulPropTag = 
						PR_DISPLAY_NAME;
					newAddress.aEntries[0].rgPropVals[1].ulPropTag = 
						PR_EMS_AB_PROXY_ADDRESSES;
					
					newAddress.aEntries[0].rgPropVals[1].
						Value.MVszA.cValues = 0;
					newAddress.aEntries[0].rgPropVals[1].
						Value.MVszA.lppszA = NULL;

					szName = aEntries[ulRow].
								rgPropVals[ulDisplayName].Value.lpszA;

					szPutNull = strstr(szName, " (E-mail)");
					if (szPutNull)
						*szPutNull = 0;

					MAPIAllocateBuffer(strlen(szName)+2, (void **)
						&(newAddress.aEntries[0].rgPropVals[0].Value.lpszA));

					strcpy(newAddress.aEntries[0].rgPropVals[0].Value.lpszA,
						szName);

					result = pContainer->ResolveNames(
								(LPSPropTagArray) &propTags, 0, 
								(LPADRLIST) &newAddress, 
								(FlagList *) &flags);

					if (flags.ulFlag[0] == MAPI_UNRESOLVED)
					{
						szNameCopy = (char *) calloc(strlen(szName)+1, 1);
						strcpy(szNameCopy, szName);

						szFirstName = strtok(szNameCopy, " ");
						if (szFirstName != NULL)
						{
							szLastName = szFirstName + strlen(szFirstName) 
											+ 1;

							szName = (char *) calloc(strlen(szFirstName)
												+ strlen(szLastName) + 3, 1);

							strcpy(szName, szLastName);
							strcat(szName, ", ");
							strcat(szName, szFirstName);

							strcpy(newAddress.aEntries[0].
								rgPropVals[0].Value.lpszA, szName);

							result = pContainer->ResolveNames(
										(LPSPropTagArray) &propTags, 0, 
										(LPADRLIST) &newAddress, 
										(FlagList *) &flags);

							free(szName);
						}

						free(szNameCopy);
					}

					ulNumProxies = newAddress.aEntries[0].
									rgPropVals[1].Value.MVszA.cValues;

					pszProxy = newAddress.aEntries[0].
								rgPropVals[1].Value.MVszA.lppszA;
				}
				else if (ulProxy != ULONG_MAX)
				{
					ulNumProxies = aEntries[ulRow].
									rgPropVals[ulProxy].Value.MVszA.cValues;

					pszProxy = aEntries[ulRow].
								rgPropVals[ulProxy].Value.MVszA.lppszA;
				}

				if (GetSMTPAddress(ulNumProxies, pszProxy, prds, 
						pdwRecipIndex, pdwNumRecip, szSMTP, &bRecipAdded))
				{
					szRecip = szSMTP;
					bGotSMTP = TRUE;
				}
				else
					bGotSMTP = FALSE;

				if (ulProxy == ULONG_MAX)
				{
					MAPIFreeBuffer(newAddress.aEntries[0].
						rgPropVals[0].Value.lpszA);

					MAPIFreeBuffer(newAddress.aEntries[0].rgPropVals);
				}
			}
		}

		if (!bGotSMTP && !bRecipAdded && (ulIndex == ulEmailAddr))
		{
			ulIndex = ulDisplayName;
			szRecip = 
				aEntries[ulRow].rgPropVals[ulIndex].Value.lpszA;
			bGotSMTP = TRUE;
		}

		if (bGotSMTP)
		{
			// Make sure the recipient e-mail address is not
			// a null string
			nRecipLength = strlen(szRecip);
			if (!nRecipLength)
			{
				(*pdwNumRecip)--;
				continue;
			}
			
			// Finally, make sure the address has at least one
			// printable non-space character in it
			for (nChar=0; nChar<nRecipLength; nChar++)
			{
				if (isgraph(szRecip[nChar]))
					break;
			}
			
			if (nChar >= nRecipLength)
			{
				(*pdwNumRecip)--;
				continue;
			}
			
			(prds->szRecipientArray)[*pdwRecipIndex] = 
				(char *) calloc(1, nRecipLength+1);
			
			strcpy((prds->szRecipientArray)[*pdwRecipIndex], szRecip);
			(*pdwRecipIndex)++;
		}
	}

	return;
}


BOOL GetSMTPAddress(ULONG ulNum, LPSTR *lppszProxy,
					RECIPIENTDIALOGSTRUCT *prds,
					DWORD *pdwRecipIndex, DWORD *pdwNumRecip,
					char *szAddress, BOOL *pbRecipAdded)
{
	BOOL			bMatch = FALSE;
	BOOL			bReturn = FALSE;
	BOOL			*pbSMTP = NULL;
	BOOL			bFoundSMTP = FALSE;
	const char *	szNewAddress = NULL;
	ULONG			ulIndex;
	PGPFilterRef	filter;
	PGPKeySetRef	userKeySet;
	PGPKeySetRef	matchKeySet;
	PGPUInt32		nNumKeys;
	DWORD			dwNumNew = 0;
	PGPError		err;

	userKeySet = prds->OriginalKeySetRef;
	*pbRecipAdded = FALSE;

	pbSMTP = (BOOL *) calloc(sizeof(BOOL), ulNum);

	for (ulIndex=0; ulIndex<ulNum; ulIndex++)
	{
		if (!_strnicmp("smtp", lppszProxy[ulIndex], 4))
		{
			pbSMTP[ulIndex] = TRUE;
			bFoundSMTP = TRUE;
		}
	}

	if (!bFoundSMTP)
	{
		if (pbSMTP != NULL)
			free(pbSMTP);

		return FALSE;
	}

	ulIndex = 0;
	while ((ulIndex < ulNum) && !bMatch)
	{
		if (pbSMTP[ulIndex])
		{
			dwNumNew++;
			szNewAddress = lppszProxy[ulIndex] + 5;

			PGPNewUserIDEmailFilter(PGPGetKeySetContext(userKeySet), 
				szNewAddress, kPGPMatchEqual, &filter);
			err = PGPFilterKeySet(userKeySet, filter, &matchKeySet);
			
			if (IsntNull(matchKeySet))
			{
				PGPCountKeys(matchKeySet, &nNumKeys);
				if (nNumKeys > 0)
					bMatch = TRUE;
			}
			
			PGPFreeKeySet(matchKeySet);
			PGPFreeFilter(filter);
		}

		ulIndex++;
	}
	
	if (bMatch)
	{
		strcpy(szAddress, szNewAddress);
		bReturn = TRUE;
	}

	if (!bMatch)
	{
		PGPPrefRef prefs = NULL;
		PGPBoolean sync = FALSE;
		PGPUInt32 numServers = 0;
		PGPKeySetRef tempSet = NULL;
		char szTempAddress[1024];

		err = PGPclOpenClientPrefs(PGPGetContextMemoryMgr(prds->Context),
					&prefs);

		if (IsPGPError(err))
			goto AddAllToRecipientDialog;

		PGPGetPrefBoolean(prefs, kPGPPrefKeyServerSyncUnknownKeys, &sync);
		PGPclCloseClientPrefs(prefs, FALSE);

		if (!sync)
			goto AddAllToRecipientDialog;

		ulIndex = 0;
		while ((ulIndex < ulNum) && !bMatch)
		{
			if (pbSMTP[ulIndex])
			{
				szNewAddress = lppszProxy[ulIndex] + 5;
				
				strcpy(szTempAddress, "<");
				strcat(szTempAddress, szNewAddress);
				strcat(szTempAddress, ">");
				
				err = PGPclSearchServerForUserID(prds->Context, 
						prds->tlsContext, prds->hwndParent, szTempAddress, 
						PGPCL_DEFAULTSERVER, prds->OriginalKeySetRef, 
						&tempSet);
				
				if(IsntPGPError(err) && PGPKeySetRefIsValid(tempSet))
				{
					PGPUInt32		numKeys;
					PGPKeySetRef	newKeySet = NULL;
					
					err = PGPCountKeys(tempSet, &numKeys);
					if(IsntPGPError(err) && (numKeys > 0))
					{
						PGPclQueryAddKeys(prds->Context, prds->tlsContext, 
							prds->hwndParent, tempSet, NULL);
						
						PGPAddKeys(tempSet, prds->OriginalKeySetRef);
						PGPCommitKeyRingChanges(prds->OriginalKeySetRef);
						
						(*pdwNumRecip)++;
						prds->szRecipientArray = 
							(char **) realloc(prds->szRecipientArray, 
							sizeof(char *) * (*pdwNumRecip));
						
						(prds->szRecipientArray)[*pdwRecipIndex] = 
							(char *) calloc(1, strlen(szNewAddress)+1);
						
						strcpy((prds->szRecipientArray)[*pdwRecipIndex], 
							szNewAddress);
						
						(*pdwRecipIndex)++;
						bMatch = TRUE;
						*pbRecipAdded = TRUE;
					}
					
					PGPFreeKeySet(tempSet);
				}
				
				if (bMatch)
					(*pdwNumRecip)--;
			}

			ulIndex++;
		}
	}


AddAllToRecipientDialog:

	if (!bMatch)
	{
		(*pdwNumRecip) += dwNumNew-1;
		prds->szRecipientArray = (char **) 
			realloc(prds->szRecipientArray, sizeof(char *) * (*pdwNumRecip));

		for (ulIndex=0; ulIndex<ulNum; ulIndex++)
		{
			if (pbSMTP[ulIndex])
			{
				szNewAddress = lppszProxy[ulIndex] + 5;
	
				(prds->szRecipientArray)[*pdwRecipIndex] = 
					(char *) calloc(1, strlen(szNewAddress)+1);
			
				strcpy((prds->szRecipientArray)[*pdwRecipIndex], 
					szNewAddress);
				(*pdwRecipIndex)++;

				*pbRecipAdded = TRUE;
			}
		}
	}

	if (pbSMTP != NULL)
		free(pbSMTP);

	return bReturn;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
