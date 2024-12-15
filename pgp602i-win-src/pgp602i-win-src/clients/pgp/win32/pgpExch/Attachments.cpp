/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	


	$Id: Attachments.cpp,v 1.15.2.4.2.1 1998/11/12 03:13:53 heller Exp $


____________________________________________________________________________*/
#include "stdinc.h"
#include "Exchange.h"
#include "EncryptSign.h"
#include "DecryptVerify.h"
#include "resource.h"
#include "UIutils.h"

#include "pgpConfig.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "pgpMem.h"


STDMETHODIMP CExtImpl::OnReadPattFromSzFile(IAttach* lpAtt, 
											LPTSTR lpszFile, 
											ULONG ulFlags)
{
	_bHaveAttachments = TRUE;
	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnWritePattToSzFile(IAttach* lpAtt, 
										   LPTSTR lpszFile, 
										   ULONG ulFlags)
{
	return S_FALSE;
}


STDMETHODIMP CExtImpl::QueryDisallowOpenPatt(IAttach* lpAtt)
{
	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnOpenPatt(IAttach* lpAtt)
{
	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnOpenSzFile(LPTSTR lpszFile, ULONG ulFlags)
{
	char *szExtension;
	char *szOutFile = NULL;
	HINSTANCE hInst;
	int nError;
	char szName[256];
	char szFile[256];
	BOOL bDummy;

	if (_context != EECONTEXT_READNOTEMESSAGE)
		return S_FALSE;

	szExtension = strrchr(lpszFile, '.');
	if (!szExtension)
		return S_FALSE;
	if (stricmp(szExtension, ".asc") && stricmp(szExtension, ".pgp"))
		return S_FALSE;

	UIGetString(szName, sizeof(szName), IDS_LOGNAME);
	UIGetString(szFile, sizeof(szFile), IDS_DLL);

	nError = DecryptVerifyFile(UIGetInstance(), NULL, _pgpContext, 
				_tlsContext, szName, szFile, lpszFile, FALSE, TRUE, 
				&szOutFile, NULL, NULL, &bDummy);

	if (nError == kPGPError_NoErr)
	{
		hInst = ShellExecute(NULL, "open", szOutFile, NULL, NULL, 0);

		if (szOutFile != NULL)
			PGPFreeData(szOutFile);

		return S_OK;
	}

	return S_FALSE;
}


int CExtImpl::EncryptSignAttachment(HWND hwnd,
									IMessage *psmg,
									UINT nOutSize,
									SRowSet *prAttach,
									RECIPIENTDIALOGSTRUCT *prds,
									PGPOptionListRef *pSignOptions)
{
	ULONG ulRow;
	ULONG ulNum;
	SizedSPropTagArray(6, tagaAttach) = {
		6, {PR_ATTACH_METHOD, PR_ATTACH_TAG, PR_RENDERING_POSITION, 
			PR_ATTACH_FILENAME, PR_ATTACH_LONG_FILENAME, 
			PR_DISPLAY_NAME}};
	enum {ivalAttachMethod=0, ivalAttachTag, ivalRenderingPos, ivalFilename, 
			ivalLongFilename, ivalDisplayName};
	SPropValue *pval;
	ULONG ulNumVals;
	LPATTACH pAttach = 0;
	IStream *pstrmAttach;
	STATSTG AttStreamStats;
	DWORD dwAttSize;
	UINT nAttOutSize;
	char *pAttachText = NULL;
	char *pOutput = NULL;
	char *szFileName;
	char *szLongFileName;
	char *szExtension;
	int nError = 0;
	char szName[256];
	char szFile[256];
	
	UIGetString(szName, sizeof(szName), IDS_LOGNAME);
	UIGetString(szFile, sizeof(szFile), IDS_DLL);

	for (ulRow=0; ulRow<prAttach->cRows; ulRow++)
	{
		ulNum = prAttach->aRow[ulRow].lpProps[0].Value.l;
		psmg->OpenAttach(ulNum, NULL, MAPI_MODIFY, &pAttach);
		pAttach->GetProps((SPropTagArray *)&tagaAttach, 0, 
			&ulNumVals, &pval);
		switch (pval[ivalAttachMethod].Value.l)
		{
		case ATTACH_BY_VALUE:
			// Don't sign attachments unless we're also encrypting them
			if (_bEncrypt)
			{
				pAttach->OpenProperty(PR_ATTACH_DATA_BIN, &IID_IStream, 0, 
					MAPI_MODIFY, (LPUNKNOWN *) &pstrmAttach);
				
				pstrmAttach->Stat(&AttStreamStats, STATFLAG_NONAME);
				dwAttSize = AttStreamStats.cbSize.LowPart;
				
				pAttachText = (char *) calloc(dwAttSize+1, sizeof(char));
				pstrmAttach->Read(pAttachText, dwAttSize, &dwAttSize);
				pAttachText[dwAttSize] = 0;
				nError = EncryptSignBuffer(UIGetInstance(), hwnd, 
					_pgpContext, _tlsContext, szName, szFile,
					pAttachText, dwAttSize, prds, NULL, pSignOptions, 
					(VOID ** )&pOutput, &nAttOutSize, _bEncrypt, _bSign, 
					TRUE);
				
				if (!nError)
				{
					ULONG ulAttOutSize = nAttOutSize;
					LARGE_INTEGER li = {0,0};
					ULARGE_INTEGER uli = {ulAttOutSize, 0};
					ULONG ulWritten;
					ULARGE_INTEGER uliSeek;
					
					pstrmAttach->Seek(li, STREAM_SEEK_SET, &uliSeek);
					pstrmAttach->Write(pOutput, ulAttOutSize, &ulWritten);
					pstrmAttach->SetSize(uli);
					pstrmAttach->Commit(STGC_DEFAULT);
					pstrmAttach->Release();
					PGPFreeData(pOutput);
				}
				else
					pstrmAttach->Release();
				
				if (pval[ivalRenderingPos].Value.l > -1)
					pval[ivalRenderingPos].Value.l = nOutSize + ulRow;
				
				szFileName = (char *) calloc(MAX_PATH, sizeof(char));
				szLongFileName = (char *) calloc(MAX_PATH, sizeof(char));
				strcpy(szFileName, pval[ivalFilename].Value.lpszA);
				strcpy(szLongFileName, pval[ivalLongFilename].Value.lpszA);
				szExtension = strrchr(szFileName, '.');
				if (szExtension)
					*szExtension = '\0';
				
				strcat(szFileName, ".asc");
				strcpy(pval[ivalFilename].Value.lpszA, szFileName);
				strcat(szLongFileName, ".asc");
				MAPIFreeBuffer(pval[ivalLongFilename].Value.lpszA);
				MAPIAllocateBuffer(MAX_PATH, 
					(void **) &(pval[ivalLongFilename].Value.lpszA));
				MAPIFreeBuffer(pval[ivalDisplayName].Value.lpszA);
				MAPIAllocateBuffer(MAX_PATH, 
					(void **) &(pval[ivalDisplayName].Value.lpszA));
				strcpy(pval[ivalLongFilename].Value.lpszA, szLongFileName);
				strcpy(pval[ivalDisplayName].Value.lpszA, szLongFileName);
				if (!nError)
					pAttach->SetProps(6, pval, NULL);
				free(szFileName);
				free(szLongFileName);
			}
			else
			{
				if (pval[ivalRenderingPos].Value.l > -1)
					pval[ivalRenderingPos].Value.l = nOutSize + ulRow;
				pAttach->SetProps(6, pval, NULL);
			}
			pAttach->SaveChanges(0);
			break;
			
		case ATTACH_BY_REFERENCE:
			break;
			
		case ATTACH_BY_REF_RESOLVE:
			break;
			
		case ATTACH_BY_REF_ONLY:
			break;
			
		case ATTACH_EMBEDDED_MSG:
			{
				IMessage *pmsg = 0;

				pAttach->OpenProperty(PR_ATTACH_DATA_OBJ, &IID_IMessage, 0, 
					MAPI_MODIFY, (LPUNKNOWN *) &pmsg);
				nError = EncryptSignMessage(hwnd, pmsg, prds, pSignOptions);
				pmsg->SaveChanges(0);
				pmsg->Release();
			}
			pAttach->SaveChanges(0);
			break;
			
		case ATTACH_OLE:
			{
				if (pval[ivalRenderingPos].Value.l > -1)
					pval[ivalRenderingPos].Value.l = nOutSize + ulRow;

				pAttach->SetProps(6, pval, NULL);
			}
			pAttach->SaveChanges(0);
			break;
			
		default:
			break;
		}
		MAPIFreeBuffer(pval);
		if (pAttachText)
		{
			free(pAttachText);
			pAttachText = NULL;
		}
		pAttach->Release();
		
		if (nError)
			break;
	}

	return nError;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
