/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	


	$Id: MessageEvents.cpp,v 1.26.2.2.2.1 1998/11/12 03:13:58 heller Exp $



____________________________________________________________________________*/
#include "stdinc.h"
#include "Exchange.h"
#include "EncryptSign.h"
#include "DecryptVerify.h"
#include "resource.h"
#include "BlockUtils.h"
#include "Recipients.h"
#include "Prefs.h"
#include "UIutils.h"

#include "pgpConfig.h"
#include "pgpOptionList.h"
#include "pgpUtilities.h"
#include "pgpSDKPrefs.h"
#include "pgpMem.h"

#include "PGPsc.h"

#if !defined(PR_BODY_HTML)
#define PR_BODY_HTML	0x1013001E
#endif

STDMETHODIMP CExtImpl::OnRead(IExchExtCallback* peecb)
{
	HWND hwnd;

	peecb->GetWindow(&hwnd);
	if (!hwnd)
		hwnd = GetTopWindow(NULL);

	switch (_context)
	{
	case EECONTEXT_SENDNOTEMESSAGE:
		{
			BOOL fAddButtons = TRUE;
			int nNumButtons;
			int nIndex;
			TBBUTTON tbbNew[5];
			TBBUTTON tbbCheck;
			
			// Check to make sure the buttons aren't already on the toolbar

			nNumButtons = SendMessage(_hwndSendToolbar, TB_BUTTONCOUNT, 0, 0);
			for (nIndex=0; nIndex<nNumButtons; nIndex++)
			{
				SendMessage(_hwndSendToolbar, TB_GETBUTTON, (WPARAM) nIndex, 
							(LPARAM) &tbbCheck);

				// If a button exists with the same bitmap ID and command ID
				// as the Encrypt button, odds are good that the button is
				// already on the toolbar
				if ((tbbCheck.iBitmap == (int)_itbmEncrypt) && 
					(tbbCheck.idCommand == (int)_cmdidEncrypt))
				{
					fAddButtons = FALSE;
					nIndex = nNumButtons;
				}
			}

			if (fAddButtons)
			{
				tbbNew[0].iBitmap = 0;
				tbbNew[0].idCommand = 0;
				tbbNew[0].fsState = 0;
				tbbNew[0].fsStyle = TBSTYLE_SEP;
				tbbNew[0].dwData = 0;
				tbbNew[0].iString = -1;
			
				tbbNew[1].iBitmap = _itbmEncrypt;
				tbbNew[1].idCommand = _cmdidEncrypt;
				tbbNew[1].fsState = TBSTATE_ENABLED;
				tbbNew[1].fsStyle = TBSTYLE_CHECK;
				tbbNew[1].dwData = 0;
				tbbNew[1].iString = -1;
			
				tbbNew[2].iBitmap = _itbmSign;
				tbbNew[2].idCommand = _cmdidSign;
				tbbNew[2].fsState = TBSTATE_ENABLED;
				tbbNew[2].fsStyle = TBSTYLE_CHECK;
				tbbNew[2].dwData = 0;
				tbbNew[2].iString = -1;

				tbbNew[3].iBitmap = 0;
				tbbNew[3].idCommand = 0;
				tbbNew[3].fsState = 0;
				tbbNew[3].fsStyle = TBSTYLE_SEP;
				tbbNew[3].dwData = 0;
				tbbNew[3].iString = -1;
			
				tbbNew[4].iBitmap = _itbmPgpKeys;
				tbbNew[4].idCommand = _cmdidPgpKeys;
				tbbNew[4].fsState = TBSTATE_ENABLED;
				tbbNew[4].fsStyle = TBSTYLE_BUTTON;
				tbbNew[4].dwData = 0;
				tbbNew[4].iString = -1;
			
				SendMessage(_hwndSendToolbar, TB_ADDBUTTONS, 5, 
					(LPARAM) tbbNew);
			}
		}
		break;

    case EECONTEXT_SENDPOSTMESSAGE:
		break;
    
	case EECONTEXT_READNOTEMESSAGE:
		{
			BOOL fAddButtons = TRUE;
			int nNumButtons;
			int nIndex;
			TBBUTTON tbbNew[5];
			TBBUTTON tbbCheck;
			BOOL FYEO;

			// Check to make sure the buttons aren't already on the toolbar

			nNumButtons = SendMessage(_hwndReadToolbar, TB_BUTTONCOUNT, 0, 0);
			for (nIndex=0; nIndex<nNumButtons; nIndex++)
			{
				SendMessage(_hwndReadToolbar, TB_GETBUTTON, 
							(WPARAM) nIndex, (LPARAM) &tbbCheck);

				// If a button exists with the same bitmap ID and command ID
				// as the Decrypt button, odds are good that the button is
				// already on the toolbar
				if ((tbbCheck.iBitmap == (int)_itbmDecrypt) && 
					(tbbCheck.idCommand == (int)_cmdidDecrypt))
				{
					fAddButtons = FALSE;
					nIndex = nNumButtons;
				}
			}

			if (fAddButtons)
			{
				tbbNew[0].iBitmap = 0;
				tbbNew[0].idCommand = 0;
				tbbNew[0].fsState = 0;
				tbbNew[0].fsStyle = TBSTYLE_SEP;
				tbbNew[0].dwData = 0;
				tbbNew[0].iString = -1;
			
				tbbNew[1].iBitmap = _itbmDecrypt;
				tbbNew[1].idCommand = _cmdidDecrypt;
				tbbNew[1].fsState = TBSTATE_ENABLED;
				tbbNew[1].fsStyle = TBSTYLE_BUTTON;
				tbbNew[1].dwData = 0;
				tbbNew[1].iString = -1;
			
				tbbNew[2].iBitmap = 0;
				tbbNew[2].idCommand = 0;
				tbbNew[2].fsState = 0;
				tbbNew[2].fsStyle = TBSTYLE_SEP;
				tbbNew[2].dwData = 0;
				tbbNew[2].iString = -1;
			
				tbbNew[3].iBitmap = _itbmPgpKeys;
				tbbNew[3].idCommand = _cmdidPgpKeys;
				tbbNew[3].fsState = TBSTATE_ENABLED;
				tbbNew[3].fsStyle = TBSTYLE_BUTTON;
				tbbNew[3].dwData = 0;
				tbbNew[3].iString = -1;
			
				SendMessage(_hwndReadToolbar, TB_ADDBUTTONS, 4, 
					(LPARAM) tbbNew);
			}

			IMAPISession* psess;

			HRESULT hr = peecb->GetSession(&psess, NULL);
			if (FAILED(hr))
			{
				UIDisplayStringID(hwnd, IDS_E_NOSESSION);
				return S_FALSE;
			}
			
			if (AutoDecrypt(_memoryMgr))
			{
				IMessage *pmsg = 0;
				STATSTG StreamStats;
				DWORD dwInSize;
				UINT nOutSize;
				char *pInput;
				char *pOutput = NULL;
				BOOL bGotHTML = FALSE;
				PGPError nError = kPGPError_NoErr;
				char szFile[256];
				char szName[256];

				UIGetString(szName, sizeof(szName), IDS_LOGNAME);
				UIGetString(szFile, sizeof(szFile), IDS_DLL);
				
				CWaitCursor wait; // Mark busy
				
				hr = peecb->GetObject(NULL, (IMAPIProp**)&pmsg);
				if (FAILED(hr))
				{
					UIDisplayStringID(hwnd, IDS_E_NOMESSAGE);
					return S_FALSE;
				}
				
				IStream *pstrmBody = 0;

				hr = pmsg->OpenProperty(PR_BODY_HTML, &IID_IStream, 
					STGM_READWRITE, MAPI_MODIFY, (IUnknown**)&pstrmBody);
				
				if (FAILED(hr))
				{
					hr = pmsg->OpenProperty(PR_BODY, &IID_IStream, 
						STGM_READWRITE, MAPI_MODIFY, (IUnknown**)&pstrmBody);
				}
				else
					bGotHTML = TRUE;

				if (FAILED(hr))
				{
					UIDisplayStringID(hwnd, IDS_E_NOBODY);
					pmsg->Release();
					psess->Release();
					return S_FALSE;
				}
				
				pstrmBody->Stat(&StreamStats, STATFLAG_NONAME);
				dwInSize = StreamStats.cbSize.LowPart;
				
				pInput = (char *) calloc(dwInSize+1, sizeof(char));
				if (!pInput)
				{
					UIDisplayStringID(hwnd, IDS_E_NOMEMORY);
					pstrmBody->Release();
					pmsg->Release();
					psess->Release();
					return S_FALSE;
				}
				pstrmBody->Read(pInput, dwInSize, &dwInSize);
				pInput[dwInSize] = 0;
				
				if (AutoDecrypt(_memoryMgr))
				{
					nError = DecryptVerifyBuffer(UIGetInstance(), hwnd, 
								_pgpContext, _tlsContext, szName, 
								szFile, pInput, dwInSize, 
								FALSE, (VOID ** )&pOutput, &nOutSize, &FYEO);

					if (IsntPGPError(nError) && (nOutSize > 0) && 
						(pOutput != NULL))
					{
						LARGE_INTEGER li = {0,0};
						ULARGE_INTEGER uli = {nOutSize, 0};
						BOOL fPartied;
						char *szBuffer = NULL;
						
						// Alter only the block of encrypted/signed text
						// if this is not HTML

						if (FYEO)
						{
							TempestViewer((void *)_pgpContext,hwnd,
								pOutput,nOutSize);
						}
						else
						{
							szBuffer = (char *) calloc(dwInSize+nOutSize+1, 
													sizeof(char));

							strcpy(szBuffer, pOutput);

							if (strlen(szBuffer) > 0)
							{
								uli.LowPart = strlen(szBuffer);
								pstrmBody->Seek(li, STREAM_SEEK_SET, NULL);
								pstrmBody->Write(szBuffer, strlen(szBuffer), 
											NULL);
								pstrmBody->SetSize(uli);
								pstrmBody->Commit(STGC_DEFAULT);
								pstrmBody->Release();
								RTFSync(pmsg, RTF_SYNC_BODY_CHANGED, 
									&fPartied);
							}
	
							free(szBuffer);
						}

						PGPFreeData(pOutput);
					}
					else
						pstrmBody->Release();
				}
				else
					pstrmBody->Release();

				free(pInput);
				pmsg->Release();
			}
			
			psess->Release();
		}
		break;

	case EECONTEXT_READPOSTMESSAGE:
		break;

	default:
		// This way, the function defends itself against unknown future
		// variants, as FindREOnNote is less robust than it might be.
		return S_FALSE;
	}

	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnReadComplete(IExchExtCallback* peecb, ULONG ulFlags)
{
	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnWrite(IExchExtCallback* pecb)
{
	// The prewrite handler stamps this message as having been processed.
	// This prevents the extension from munging the message body twice.

	if (_context != EECONTEXT_SENDNOTEMESSAGE)
		return S_FALSE;

	CWaitCursor wait; // Mark busy
	HRESULT hr = _msgtype.Write(pecb, _fInSubmitState);
	if (FAILED(hr))
		return hr;

	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnWriteComplete(IExchExtCallback* pecb, ULONG ulFlags)
{
	if (_context != EECONTEXT_SENDNOTEMESSAGE)
		return S_FALSE;

	if (ulFlags == EEME_COMPLETE_FAILED) // Nothing to unwind
		return S_FALSE;

	if (!_fInSubmitState)	// This is not a submission.
		return S_FALSE;

	if (_bEncrypt || _bSign)
	{
		IMessage *pmsg = 0;
		RECIPIENTDIALOGSTRUCT *prds;
		PGPOptionListRef signOptions = NULL;
		HWND hwnd;
		PGPError nError = kPGPError_NoErr;

		pecb->GetWindow(&hwnd);
		if (!hwnd)
			hwnd = GetTopWindow(NULL);
		
		CWaitCursor wait; // Mark busy
		
		HRESULT hr = pecb->GetObject(NULL, (IMAPIProp**)&pmsg);
		if (FAILED(hr))
		{
			UIDisplayStringID(hwnd, IDS_E_NOMESSAGE);
			return E_ABORT;
		}

		prds = (RECIPIENTDIALOGSTRUCT *) 
				calloc(sizeof(RECIPIENTDIALOGSTRUCT), 1);	

		nError = PGPsdkLoadDefaultPrefs(_pgpContext);
		if (IsPGPError(nError))
		{
			UIDisplayErrorCode(__FILE__, __LINE__, NULL, nError);
			return E_ABORT;
		}

		nError = PGPOpenDefaultKeyRings(_pgpContext, (PGPKeyRingOpenFlags)0, 
					&(prds->OriginalKeySetRef));

		if (IsPGPError(nError))
		{
			UIDisplayErrorCode(__FILE__, __LINE__, NULL, nError);
			return E_ABORT;
		}

		if (_bEncrypt)
			nError = GetRecipients(pecb, _pgpContext, _tlsContext, prds);

		if (IsPGPError(nError))
		{
			if (nError != kPGPError_UserAbort)
				UIDisplayErrorCode(__FILE__, __LINE__, NULL, 
					nError);
			return E_ABORT;
		}

		nError = EncryptSignMessage(hwnd, pmsg, prds, &signOptions);

		if (signOptions != NULL)
		{
			PGPFreeOptionList(signOptions);
			signOptions = NULL;
		}

		FreeRecipients(prds);

		if (IsntNull(prds->OriginalKeySetRef))
		{
			PGPFreeKeySet(prds->OriginalKeySetRef);
			prds->OriginalKeySetRef = NULL;
		}

		free(prds);
		pmsg->Release();

		if (IsPGPError(nError))
			return E_ABORT;
	}

	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnCheckNames(IExchExtCallback*)
{
	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnCheckNamesComplete(IExchExtCallback*, ULONG)
{
	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnSubmit(IExchExtCallback*)
{
	_fInSubmitState = TRUE;
	return S_FALSE;
}


STDMETHODIMP_(VOID) CExtImpl::OnSubmitComplete(IExchExtCallback*, ULONG)
{
	_fInSubmitState = FALSE;
	return;
}


int CExtImpl::EncryptSignMessage(HWND hwnd,
								 IMessage *pmsg,
								 RECIPIENTDIALOGSTRUCT *prds,
								 PGPOptionListRef *pSignOptions)
{
	IStream *pstrmBody = 0;
	STATSTG StreamStats;
	DWORD dwInSize;
	UINT nOutSize;
	BOOL bExchangeUser = FALSE;
	char *pInput;
	char *pOutput = NULL;
	LPMAPITABLE ptableAttach = 0;
	SizedSPropTagArray(1, tagaTable) = {
			1, {PR_ATTACH_NUM}};
	SRowSet *prAttach = 0;
	PGPError nError = kPGPError_NoErr;
	HRESULT hr;
	char szName[256];
	char szFile[256];

	UIGetString(szName, sizeof(szName), IDS_LOGNAME);
	UIGetString(szFile, sizeof(szFile), IDS_DLL);

	pmsg->GetAttachmentTable(0, &ptableAttach);
	HrQueryAllRows(ptableAttach, (SPropTagArray *)&tagaTable, 
					NULL, NULL, 0, &prAttach);

	hr = pmsg->OpenProperty(PR_BODY, &IID_IStream, STGM_READWRITE, 
		MAPI_MODIFY, (IUnknown**)&pstrmBody);
	if (FAILED(hr))
	{
		UIDisplayStringID(hwnd, IDS_E_NOBODY);
		ptableAttach->Release();
		return kPGPError_ItemNotFound;
	}
		
	pstrmBody->Stat(&StreamStats, STATFLAG_NONAME);
	dwInSize = StreamStats.cbSize.LowPart;
		
	pInput = (char *) calloc(dwInSize+1, sizeof(char));
	if (!pInput)
	{
		UIDisplayStringID(hwnd, IDS_E_NOMEMORY);
		pstrmBody->Release();
		ptableAttach->Release();
		return kPGPError_OutOfMemory;
	}
	pstrmBody->Read(pInput, dwInSize, &dwInSize);
	pInput[dwInSize] = 0;

	nError = EncryptSignBuffer(UIGetInstance(), hwnd, _pgpContext, 
				_tlsContext, szName, szFile, pInput, 
				dwInSize, prds, NULL, pSignOptions, (VOID ** )&pOutput, 
				&nOutSize, _bEncrypt, _bSign, FALSE);

	if ((dwInSize > 0) && IsntPGPError(nError))
	{
		LARGE_INTEGER li = {0,0};
		ULARGE_INTEGER uli = {1,0};
		BOOL fPartied;

		pstrmBody->Seek(li, STREAM_SEEK_SET, NULL);
		pstrmBody->Write("\0", 1, NULL);
		pstrmBody->SetSize(uli);
		pstrmBody->Commit(STGC_DEFAULT);
		pstrmBody->Release();
		RTFSync(pmsg, RTF_SYNC_BODY_CHANGED, &fPartied);

		pmsg->OpenProperty(PR_BODY, &IID_IStream, STGM_READWRITE, 
			MAPI_MODIFY, (IUnknown**)&pstrmBody);
	}

	if (IsntPGPError(nError))
	{
		LARGE_INTEGER li = {0,0};
		ULARGE_INTEGER uli = {nOutSize + prAttach->cRows, 0};
		BOOL fPartied;

		pstrmBody->Seek(li, STREAM_SEEK_SET, NULL);
		pstrmBody->Write(pOutput, nOutSize, NULL);
		pstrmBody->SetSize(uli);
		pstrmBody->Commit(STGC_DEFAULT);
		pstrmBody->Release();
		RTFSync(pmsg, RTF_SYNC_BODY_CHANGED, &fPartied);
		PGPFreeData(pOutput);
	}
	else
		pstrmBody->Release();
		
	free(pInput);

	hr = pmsg->OpenProperty(PR_BODY_HTML, &IID_IStream, STGM_READWRITE, 
		MAPI_MODIFY, (IUnknown**)&pstrmBody);

	if (FAILED(hr))
		goto NoBodyHTML;
	else
	{
		LARGE_INTEGER li = {0,0};
		ULARGE_INTEGER uli = {1,0};
		BOOL fPartied;

		pstrmBody->Seek(li, STREAM_SEEK_SET, NULL);
		pstrmBody->Write("\0", 1, NULL);
		pstrmBody->SetSize(uli);
		pstrmBody->Commit(STGC_DEFAULT);
		pstrmBody->Release();
		RTFSync(pmsg, RTF_SYNC_BODY_CHANGED, &fPartied);
	}

NoBodyHTML:
	if (prAttach->cRows && IsntPGPError(nError))
		nError = EncryptSignAttachment(hwnd, pmsg, nOutSize,
					prAttach, prds, pSignOptions);

	ptableAttach->Release();
	return nError;
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
