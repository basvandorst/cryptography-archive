/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Recipients.cpp,v 1.8 1997/09/20 01:08:43 dgal Exp $
____________________________________________________________________________*/
#include "stdinc.h"
#include "Recipients.h"
#include "UIutils.h"
#include "pgpKeys.h"
#include "PGPcmdlg.h"
#include "PGPRecip.h"
#include "pgpDebug.h"
#include "pgpMem.h"
#include "pgpSDKPrefs.h"


PGPError GetRecipients(IExchExtCallback *pmecb, 
					   PGPContextRef context,
					   RECIPIENTDIALOGSTRUCT *prds)
{
	LPADRLIST		pAddrList		= NULL;
	PGPKeySetRef	pubKeySet		= NULL;
	DWORD			dwNumRecip		= 0;
	ULONG			ulRow			= 0;
	ULONG			ulIndex			= 0;
	ULONG			ulEmailAddr		= 0;
	ULONG			ulDisplayName	= 0;
	ULONG			ulType			= 0;
	ULONG			ulEntryID		= 0;
	int				nChar			= 0;
	int				nRecipLength	= 0;
	char *			szRecip			= NULL;
	ULONG			ulNumRows		= 0;
	DWORD			dwRecipIndex	= 0;
	char *			szNewRecipient	= NULL;
	char **			szOldRecipients	= NULL;
	HWND			hwnd			= NULL;
	BOOL			bGotRecipients	= FALSE;
	PGPError		err				= kPGPError_NoErr;

	pgpAssert(pmecb);
	pgpAssert(prds);
	pgpAssert(PGPRefIsValid(context));

	pmecb->GetWindow(&hwnd);
	if (!hwnd)
		hwnd = GetTopWindow(NULL);

	prds->szRecipientArray = NULL;

	pmecb->GetRecipients(&pAddrList);
	if (pAddrList)
	{
		dwNumRecip = pAddrList->cEntries;
	}
	else
	{
		prds->dwNumRecipients = 0;
		return err;
	}
				
	ulNumRows = dwNumRecip;
	
	for (ulRow=0; ulRow<ulNumRows; ulRow++)
	{
		ulEmailAddr		= 65535;
		ulDisplayName	= 65535;
		ulType			= 65535;
		ulEntryID		= 65535;
		szNewRecipient	= NULL;
		
		for (	ulIndex=0;
				ulIndex<pAddrList->aEntries[ulRow].cValues; 
				ulIndex++)
		{
			switch (pAddrList->aEntries[ulRow].rgPropVals[ulIndex].ulPropTag)
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
			}
		}

		if ((ulDisplayName == 65535) && (ulEmailAddr == 65535))
		{
			dwNumRecip--;
			continue;
		}
		
		if (ulEmailAddr != 65535)
		{
			szRecip = pAddrList->
						aEntries[ulRow].rgPropVals[ulEmailAddr].Value.lpszA;

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
		if (!&(pAddrList->aEntries[ulRow]))
		{
			dwNumRecip--;
			continue;
		}
		
		// Now see if the recipient has any properties
		if (!(pAddrList->aEntries[ulRow].cValues) ||
			((pAddrList->aEntries[ulRow].cValues-1) < ulIndex))
		{
			dwNumRecip--;
			continue;
		}
		
		szRecip = pAddrList->aEntries[ulRow].rgPropVals[ulIndex].Value.lpszA;
		
		// Make sure the recipient e-mail address is not
		// a null string
		nRecipLength = strlen(szRecip);
		if (!nRecipLength)
		{
			dwNumRecip--;
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
			dwNumRecip--;
			continue;
		}
		
		szNewRecipient = (char *) calloc(1, nRecipLength+1);
		strcpy(szNewRecipient, szRecip);
		
		szOldRecipients = prds->szRecipientArray;
		prds->szRecipientArray = (char **) calloc(1, dwRecipIndex+1);

		if (dwRecipIndex > 0)
		{
			UINT i;

			for (i=0; i<dwRecipIndex; i++)
				(prds->szRecipientArray)[i] = szOldRecipients[i];
		}

		(prds->szRecipientArray)[dwRecipIndex] = szNewRecipient;
		dwRecipIndex++;
	}

	prds->Version = CurrentPGPrecipVersion;
	prds->hwndParent = hwnd;
	prds->szTitle = NULL;
	prds->Context = context;
	prds->SelectedKeySetRef = NULL;
	prds->dwNumRecipients = dwNumRecip;
	prds->dwOptions = PGPCOMDLG_ASCIIARMOR;
	prds->dwFlags = 0;
	prds->dwDisableFlags = PGPCOMDLG_DISABLE_ASCIIARMOR |
							PGPCOMDLG_DISABLE_WIPEORIG;

	err = PGPsdkLoadDefaultPrefs(context);
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto GetRecipientsError;
	}

	err = PGPOpenDefaultKeyRings(context, (PGPKeyRingOpenFlags)0, &pubKeySet);
	if (IsPGPError(err))
	{
		UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", err);
		goto GetRecipientsError;
	}

	prds->OriginalKeySetRef = pubKeySet;

	bGotRecipients = PGPRecipientDialog(prds);
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

	for (dwIndex=0; dwIndex<prds->dwNumRecipients; dwIndex++)
	{
		if ((prds->szRecipientArray)[dwIndex] != NULL)
		{
			free((prds->szRecipientArray)[dwIndex]);
			(prds->szRecipientArray)[dwIndex] = NULL;
		}
	}

	if (IsntNull(prds->SelectedKeySetRef))
	{
		PGPFreeKeySet(prds->SelectedKeySetRef);
		prds->SelectedKeySetRef = NULL;
	}
	
	if (IsntNull(prds->OriginalKeySetRef))
	{
		PGPFreeKeySet(prds->OriginalKeySetRef);
		prds->OriginalKeySetRef = NULL;
	}
	
	return;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
