/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Commands.cpp,v 1.13.4.1 1997/11/11 00:46:13 dgal Exp $
____________________________________________________________________________*/
#include "stdinc.h"
#include "Exchange.h"
#include "resource.h"
#include "pgpExch.h"
#include "ExchPrefs.h"
#include "Recipients.h"
#include "RichEdit_IO.h"
#include "BlockUtils.h"
#include "UIutils.h"

#include "pgpConfig.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "pgpMem.h"
#include "PGPcmdlg.h"
#include "PGPRecip.h"

typedef struct {
		HWND hwndFound;
		int yMax;
} FindStruct;

static HWND FindMessageWindow(IExchExtCallback* pmecb);
BOOL CALLBACK ReportChildren(HWND hwnd, LPARAM lParam);


STDMETHODIMP CExtImpl::InstallCommands(IExchExtCallback* pmecb, 
                            HWND hWnd, HMENU hMenu,
                            UINT * pcmdidBase, LPTBENTRY lptbeArray,
                            UINT ctbe, ULONG ulFlags)
{
	if ((EECONTEXT_READNOTEMESSAGE != _context) && 
		(EECONTEXT_SENDNOTEMESSAGE != _context) &&
		(EECONTEXT_VIEWER != _context))
		return S_FALSE;

	// First, the menu

	HMENU hmenuTools;
	HMENU hmenuHelp;
	HMENU hmenuHelpTopics;
	ULONG ulBeforeTools;
	ULONG ulAfterExchange;
	HRESULT hr = pmecb->GetMenuPos(EECMDID_Tools, &hmenuTools, 
									&ulBeforeTools, NULL, 0);
	if (S_OK != hr)
		return S_FALSE; // No such menu item?  Very bad.
	hr = pmecb->GetMenuPos(EECMDID_HelpAboutMicrosoftExchange, &hmenuHelp,
							NULL, NULL, 0);
	if (S_OK != hr)
		return S_FALSE; // No such menu item?  Very bad.
	hr = pmecb->GetMenuPos(EECMDID_HelpMicrosoftExchangeHelpTopics, 
							&hmenuHelpTopics, NULL, &ulAfterExchange, 0);
	if (S_OK != hr)
		return S_FALSE; // No such menu item?  Very bad.

	char szCommand[80];
	
	_hmenuPGP = CreatePopupMenu();
	InsertMenu(hmenuTools, ulBeforeTools, 
				MF_BYPOSITION | MF_STRING | MF_POPUP, 
				(UINT) _hmenuPGP, "&PGP");

	if (_context == EECONTEXT_SENDNOTEMESSAGE)
	{
		UIGetString(szCommand, sizeof(szCommand), IDS_ENCRYPT_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidEncrypt = *pcmdidBase;
 		++(*pcmdidBase);

		UIGetString(szCommand, sizeof(szCommand), IDS_SIGN_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidSign = *pcmdidBase;
 		++(*pcmdidBase);

		AppendMenu(_hmenuPGP, MF_SEPARATOR, 0, NULL);

		UIGetString(szCommand, sizeof(szCommand), IDS_ENCRYPTNOW_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidEncryptNow = *pcmdidBase;
 		++(*pcmdidBase);

		UIGetString(szCommand, sizeof(szCommand), IDS_SIGNNOW_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidSignNow = *pcmdidBase;
 		++(*pcmdidBase);

		UIGetString(szCommand, sizeof(szCommand), IDS_ENCRYPTSIGN_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidEncryptSign = *pcmdidBase;
 		++(*pcmdidBase);

		AppendMenu(_hmenuPGP, MF_SEPARATOR, 0, NULL);
	}

	if (_context == EECONTEXT_READNOTEMESSAGE)
	{
		UIGetString(szCommand, sizeof(szCommand), IDS_DECRYPT_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidDecrypt = *pcmdidBase;
 		++(*pcmdidBase);

		UIGetString(szCommand, sizeof(szCommand), IDS_ADDKEY_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidAddKey = *pcmdidBase;
 		++(*pcmdidBase);

		AppendMenu(_hmenuPGP, MF_SEPARATOR, 0, NULL);
	}

	UIGetString(szCommand, sizeof(szCommand), IDS_PGPKEYS_MENU);
	AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
	_cmdidPgpKeys = *pcmdidBase;
	++(*pcmdidBase);

	UIGetString(szCommand, sizeof(szCommand), IDS_PGPPREFS_MENU);
	AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
	_cmdidPrefs = *pcmdidBase;
	++(*pcmdidBase);
	
	UIGetString(szCommand, sizeof(szCommand), IDS_PGPHELP_MENU);
	InsertMenu(hmenuHelpTopics, ulAfterExchange, 
				MF_BYPOSITION | MF_STRING, *pcmdidBase, szCommand);
	_cmdidHelp = *pcmdidBase;
	++(*pcmdidBase);
	
	UIGetString(szCommand, sizeof(szCommand), IDS_ABOUT_MENU);
	AppendMenu(hmenuHelp, MF_STRING, *pcmdidBase, szCommand);
	_cmdidAbout = *pcmdidBase;
	++(*pcmdidBase);
	
	// Next, the toolbar

	int tbindx;
	HWND hwndToolbar = NULL;
	for (tbindx = ctbe-1; (int) tbindx > -1; --tbindx)
	{
		if (EETBID_STANDARD == lptbeArray[tbindx].tbid)
		{
			hwndToolbar = lptbeArray[tbindx].hwnd;
			if (_context == EECONTEXT_SENDNOTEMESSAGE)
			{
				_hwndSendToolbar = hwndToolbar;
				_itbbEncrypt = lptbeArray[tbindx].itbbBase++;
				_itbbSign = lptbeArray[tbindx].itbbBase++;
			}

			if (_context == EECONTEXT_READNOTEMESSAGE)
			{
				_hwndReadToolbar = hwndToolbar;
				_itbbDecrypt = lptbeArray[tbindx].itbbBase++;
				_itbbAddKey = lptbeArray[tbindx].itbbBase++;
			}
			
			_itbbPgpKeys = lptbeArray[tbindx].itbbBase++;
			break;
		}
	}

	if (hwndToolbar)
	{
		TBADDBITMAP tbab;

		tbab.hInst = UIGetInstance();
		if (_context == EECONTEXT_SENDNOTEMESSAGE)
		{
			tbab.nID = IDB_ENCRYPT;
			_itbmEncrypt = SendMessage(hwndToolbar, TB_ADDBITMAP, 1, 
							(LPARAM)&tbab);

			tbab.nID = IDB_SIGN;
			_itbmSign = SendMessage(hwndToolbar, TB_ADDBITMAP, 1, 
							(LPARAM)&tbab);
		}

		if (_context == EECONTEXT_READNOTEMESSAGE)
		{
			tbab.nID = IDB_DECRYPT;
			_itbmDecrypt = SendMessage(hwndToolbar, TB_ADDBITMAP, 1, 
							(LPARAM)&tbab);

			tbab.nID = IDB_ADDKEY;
			_itbmAddKey = SendMessage(hwndToolbar, TB_ADDBITMAP, 1, 
							(LPARAM)&tbab);
		}

		tbab.nID = IDB_PGPKEYS;
		_itbmPgpKeys = SendMessage(hwndToolbar, TB_ADDBITMAP, 1, 
						(LPARAM)&tbab);

	}

	return S_OK;
}


STDMETHODIMP CExtImpl::QueryButtonInfo (ULONG tbid, UINT itbb, 
                            LPTBBUTTON ptbb, LPTSTR lpsz, UINT cch, 
                            ULONG ulFlags)
{
	if ((EECONTEXT_READNOTEMESSAGE != _context) &&
		(EECONTEXT_SENDNOTEMESSAGE != _context) &&
		(EECONTEXT_VIEWER != _context))
		return S_FALSE;

	HRESULT hr = S_FALSE;

	if ((itbb == _itbbEncrypt) && (_context == EECONTEXT_SENDNOTEMESSAGE))
	{
		ptbb->iBitmap = _itbmEncrypt;
		ptbb->idCommand = _cmdidEncrypt;
		ptbb->fsState = TBSTATE_ENABLED;
		ptbb->fsStyle = TBSTYLE_CHECK;
		ptbb->dwData = 0;
		ptbb->iString = -1;

		UIGetString(lpsz, cch, IDS_ENCRYPT_TOOLTIP);
		hr = S_OK;
	}

	if ((itbb == _itbbSign) && (_context == EECONTEXT_SENDNOTEMESSAGE))
	{
		ptbb->iBitmap = _itbmSign;
		ptbb->idCommand = _cmdidSign;
		ptbb->fsState = TBSTATE_ENABLED;
		ptbb->fsStyle = TBSTYLE_CHECK;
		ptbb->dwData = 0;
		ptbb->iString = -1;

		UIGetString(lpsz, cch, IDS_SIGN_TOOLTIP);
		hr = S_OK;
	}

	if (itbb == _itbbPgpKeys)
	{
		ptbb->iBitmap = _itbmPgpKeys;
		ptbb->idCommand = _cmdidPgpKeys;
		ptbb->fsState = TBSTATE_ENABLED;
		ptbb->fsStyle = TBSTYLE_BUTTON;
		ptbb->dwData = 0;
		ptbb->iString = -1;

		UIGetString(lpsz, cch, IDS_PGPKEYS_TOOLTIP);
		hr = S_OK;
	}

	if ((itbb == _itbbDecrypt) && (_context == EECONTEXT_READNOTEMESSAGE))
	{
		ptbb->iBitmap = _itbmDecrypt;
		ptbb->idCommand = _cmdidDecrypt;
		ptbb->fsState = TBSTATE_ENABLED;
		ptbb->fsStyle = TBSTYLE_BUTTON;
		ptbb->dwData = 0;
		ptbb->iString = -1;

		UIGetString(lpsz, cch, IDS_DECRYPT_TOOLTIP);
		hr = S_OK;
	}


	if ((itbb == _itbbAddKey) && (_context == EECONTEXT_READNOTEMESSAGE))
	{
		ptbb->iBitmap = _itbmAddKey;
		ptbb->idCommand = _cmdidAddKey;
		ptbb->fsState = TBSTATE_ENABLED;
		ptbb->fsStyle = TBSTYLE_BUTTON;
		ptbb->dwData = 0;
		ptbb->iString = -1;

		UIGetString(lpsz, cch, IDS_ADDKEY_TOOLTIP);
		hr = S_OK;
	}

	return hr;
}


STDMETHODIMP CExtImpl::ResetToolbar(ULONG tbid, ULONG ulFlags)
{
	// To implement this method,
	// the extension must cache the results of a prior call
	// to IExchExtCallback::GetToolbar.

	return S_FALSE;
}


STDMETHODIMP CExtImpl::QueryHelpText(UINT cmdid, ULONG ulFlags, 
                                      LPTSTR psz, UINT cch)
{
	if ((EECONTEXT_READNOTEMESSAGE != _context) &&
		(EECONTEXT_SENDNOTEMESSAGE != _context) &&
		(EECONTEXT_VIEWER != _context))
		return S_FALSE;

	if (ulFlags == EECQHT_STATUS)
	{
		if ((cmdid == _cmdidEncrypt) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_ENCRYPT_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidSign) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_SIGN_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidEncryptNow) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_ENCRYPTNOW_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidSignNow) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_SIGNNOW_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidEncryptSign) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_ENCRYPTSIGN_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidDecrypt) && 
			(_context == EECONTEXT_READNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_DECRYPT_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidAddKey) && 
			(_context == EECONTEXT_READNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_ADDKEY_STATUS);
			return S_OK;
		}

		if (cmdid == _cmdidPgpKeys)
		{
			UIGetString(psz, cch, IDS_PGPKEYS_STATUS);
			return S_OK;
		}

		if (cmdid == _cmdidPrefs)
		{
			UIGetString(psz, cch, IDS_PGPPREFS_STATUS);
			return S_OK;
		}

		if (cmdid == _cmdidHelp)
		{
			UIGetString(psz, cch, IDS_PGPHELP_STATUS);
			return S_OK;
		}

		if (cmdid == _cmdidAbout)
		{
			UIGetString(psz, cch, IDS_ABOUT_STATUS);
			return S_OK;
		}

	}
	else if (ulFlags == EECQHT_TOOLTIP)
	{
		if ((cmdid == _cmdidEncrypt) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_ENCRYPT_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidSign) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_SIGN_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidEncryptNow) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_ENCRYPTNOW_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidSignNow) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_SIGNNOW_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidEncryptSign) && 
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_ENCRYPTSIGN_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidDecrypt) && 
			(_context == EECONTEXT_READNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_DECRYPT_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidAddKey) && 
			(_context == EECONTEXT_READNOTEMESSAGE))
		{
			UIGetString(psz, cch, IDS_ADDKEY_TOOLTIP);
			return S_OK;
		}

		if (cmdid == _cmdidPgpKeys)
		{
			UIGetString(psz, cch, IDS_PGPKEYS_TOOLTIP);
			return S_OK;
		}
	}

	return S_FALSE;
}


STDMETHODIMP CExtImpl::Help(IExchExtCallback* pmecb, UINT cmdid)
{
	return S_FALSE;
}


STDMETHODIMP_(VOID) CExtImpl::InitMenu(IExchExtCallback* pmecb)
{
	if ((EECONTEXT_READNOTEMESSAGE != _context) &&
		(EECONTEXT_SENDNOTEMESSAGE != _context))
		return;

	if (!_fInitMenuOnce)
	{
		// Only set encrypt and sign flags once.
		
		_fInitMenuOnce = TRUE;
		
		_bEncrypt = ByDefaultEncrypt();
		_bSign = ByDefaultSign();
		_bUseMIME = UsePGPMime();
	}

	// Now to the real menu business

	if (_context == EECONTEXT_SENDNOTEMESSAGE)
	{
		HMENU hmenu;
		MENUITEMINFO miiEncrypt;
		MENUITEMINFO miiSign;
		
		HRESULT hr = pmecb->GetMenu(&hmenu);
		if (FAILED(hr))
			return;

		HWND hwndToolbar;
		hr = pmecb->GetToolbar(EETBID_STANDARD, &hwndToolbar);
		if (hr != S_OK)
			return;

		miiEncrypt.cbSize = sizeof(MENUITEMINFO);
		miiEncrypt.fMask = MIIM_STATE | MIIM_CHECKMARKS;
		GetMenuItemInfo(hmenu, _cmdidEncrypt, FALSE, &miiEncrypt);
		if (_bEncrypt)
		{
			miiEncrypt.fState = MFS_CHECKED;
			miiEncrypt.hbmpChecked = NULL;
			SendMessage(hwndToolbar, TB_CHECKBUTTON, 
				_cmdidEncrypt, MAKELONG(TRUE, 0));
		}
		else
		{
			miiEncrypt.fState = MFS_UNCHECKED;
			miiEncrypt.hbmpUnchecked = NULL;
			SendMessage(hwndToolbar, TB_CHECKBUTTON, 
				_cmdidEncrypt, MAKELONG(FALSE, 0));
		}
		SetMenuItemInfo(hmenu, _cmdidEncrypt, FALSE, &miiEncrypt);

		miiSign.cbSize = sizeof(MENUITEMINFO);
		miiSign.fMask = MIIM_STATE | MIIM_CHECKMARKS;
		GetMenuItemInfo(hmenu, _cmdidSign, FALSE, &miiSign);
		if (_bSign)
		{
			miiSign.fState = MFS_CHECKED;
			miiSign.hbmpChecked = NULL;
			SendMessage(hwndToolbar, TB_CHECKBUTTON, 
				_cmdidSign, MAKELONG(TRUE, 0));
		}
		else
		{
			miiSign.fState = MFS_UNCHECKED;
			miiSign.hbmpUnchecked = NULL;
			SendMessage(hwndToolbar, TB_CHECKBUTTON, 
				_cmdidSign, MAKELONG(FALSE, 0));
		}
		SetMenuItemInfo(hmenu, _cmdidSign, FALSE, &miiSign);
	}
}


STDMETHODIMP CExtImpl::DoCommand(IExchExtCallback* pmecb, UINT cmdid)
{
	if ((EECONTEXT_READNOTEMESSAGE != _context) &&
		(EECONTEXT_SENDNOTEMESSAGE != _context) &&
		(EECONTEXT_VIEWER != _context))
		return S_FALSE;

	HWND hwndMain;
	pmecb->GetWindow(&hwndMain);

	_hwndMessage = FindMessageWindow(pmecb);

	if ((cmdid == _cmdidEncrypt) && (_context == EECONTEXT_SENDNOTEMESSAGE))
	{
		_bEncrypt = !_bEncrypt;
		InitMenu(pmecb);
		return S_OK;
	}

	if ((cmdid == _cmdidSign) && (_context == EECONTEXT_SENDNOTEMESSAGE))
	{
		_bSign = !_bSign;
		InitMenu(pmecb);
		return S_OK;
	}

	if (((cmdid == _cmdidEncryptNow) || (cmdid == _cmdidSignNow) || 
		(cmdid == _cmdidEncryptSign)) && 
		(_context == EECONTEXT_SENDNOTEMESSAGE))
	{
		BOOL bEncrypt;
		BOOL bSign;
		
		bEncrypt =	(cmdid == _cmdidEncryptNow) || 
					(cmdid == _cmdidEncryptSign);

		bSign = (cmdid == _cmdidSignNow) || 
				(cmdid == _cmdidEncryptSign); 

		if (_bHaveAttachments)
			if (!UIWarnUser(hwndMain, IDS_Q_ATTACHMENT, "Attachment"))
				return S_OK;

		if (_hwndMessage)
		{
			char *szInput;
			char *szOutput = NULL;
			long lLength;
			UINT nOutLength;
			BOOL bSelectedText = FALSE;
			CHARRANGE chRange = {0,0};
			RECIPIENTDIALOGSTRUCT *prds;
			PGPOptionListRef signOptions = NULL;
			PGPError nError = kPGPError_NoErr;

			prds = (RECIPIENTDIALOGSTRUCT *) 
					calloc(sizeof(RECIPIENTDIALOGSTRUCT), 1);

			if (bEncrypt)
				nError = GetRecipients(pmecb, _pgpContext, prds);
			
			if (IsPGPError(nError))
			{
				if (nError != kPGPError_UserAbort)
					UIDisplayErrorCode(__FILE__, __LINE__, "pgpExch.dll", 
						nError);
				return S_OK;
			}
			
			// Determine if the user selected a particular piece of text
			SendMessage(_hwndMessage, EM_EXGETSEL, 
				(WPARAM)0, (LPARAM) &chRange);

			bSelectedText = chRange.cpMax - chRange.cpMin;
			
			szInput = GetRichEditContents(_hwndMessage, &lLength, 
											FALSE, bSelectedText);
			if (!szInput)
				return S_OK;

			lLength = strlen(szInput);
			nError = EncryptSignBuffer(hwndMain, _pgpContext, szInput, 
						lLength, prds, &signOptions, (void **) &szOutput, 
						&nOutLength, bEncrypt, bSign, FALSE, FALSE);

			if (IsntPGPError(nError))
			{
				SetRichEditContents(_hwndMessage, szOutput, FALSE, 
					bSelectedText);
				PGPFreeData(szOutput);
				_bEncrypt = _bEncrypt && !bEncrypt;
				_bSign = _bSign && !bSign;
				InitMenu(pmecb);
			}

			if (signOptions != NULL)
			{
				PGPFreeOptionList(signOptions);
				signOptions = NULL;
			}

			memset(szInput, 0, lLength);
			HeapFree(GetProcessHeap (), 0, szInput);

			FreeRecipients(prds);
			free(prds);
		}

		return S_OK;
	}

	if ((cmdid == _cmdidDecrypt) && (_context == EECONTEXT_READNOTEMESSAGE))
	{
		if (_hwndMessage)
		{
			char *szInput;
			char *szTemp;
			char *szOutput = NULL;
			long lLength;
			UINT nOutLength;
			ULONG ulBlockStart;
			ULONG ulBlockLength;
			BOOL bSelectedText = FALSE;
			CHARRANGE chRange = {0,0};
			BOOL bFoundBlock;
			PGPError nError = kPGPError_NoErr;

			// Determine if the user selected a particular piece of text
			SendMessage(_hwndMessage, EM_EXGETSEL, 
				(WPARAM)0, (LPARAM) &chRange);
			bSelectedText = chRange.cpMax - chRange.cpMin;
			
			szInput = GetRichEditContents(_hwndMessage, &lLength, 
											FALSE, bSelectedText);
			if (!szInput)
				return S_OK;

			szTemp = (char *) pgpAlloc(lLength+2);
			strcpy(szTemp, szInput);
			strcat(szTemp, "\r\n");
			HeapFree(GetProcessHeap (), 0, szInput);
			szInput = szTemp;
			lLength += 2;

			// Find the PGP block within the selected block and select it only

			bFoundBlock = FindEncryptedBlock(szInput, lLength, 
							&ulBlockStart, &ulBlockLength);

			if (!bFoundBlock)
				bFoundBlock = FindSignedBlock(szInput, lLength, 
								&ulBlockStart, &ulBlockLength);
		
			if (bFoundBlock)
			{
				// Outlook counts CRLF as one character, while Exchange counts
				// it as two. Correct for this in Outlook by subtracting the
				// number of CR's from the BlockStart and BlockLength

				if (_fOutlook)
				{
					char *szCR;
					int nNumCR=0;
					
					szCR = strchr(szInput, '\r');
					while (szCR && (szCR < (szInput+ulBlockStart)))
					{
						nNumCR++;
						szCR = strchr(szCR+1, '\r');
					}
					ulBlockStart -= nNumCR;
					
					nNumCR = 0;
					while	(szCR && 
							(szCR < (szInput+ulBlockStart+ulBlockLength)))
					{
						nNumCR++;
						szCR = strchr(szCR+1, '\r');
					}
					ulBlockLength -= nNumCR;
				}

				chRange.cpMin = ulBlockStart;
				chRange.cpMax = ulBlockStart+ulBlockLength;
				SendMessage(_hwndMessage, EM_EXSETSEL, 
					(WPARAM)0, (LPARAM) &chRange);

				pgpFree(szInput);
				szInput = GetRichEditContents(_hwndMessage, &lLength, 
												FALSE, TRUE);
				if (!szInput)
					return S_OK;

				szInput[lLength-1] = 0;
				lLength = strlen(szInput);
			}
			else
			{
				chRange.cpMin = 0;
				chRange.cpMax = 0;
				SendMessage(_hwndMessage, EM_EXSETSEL, (WPARAM)0, 
					(LPARAM) &chRange);
			}

			nError = DecryptVerifyBuffer(hwndMain, _pgpContext, szInput, 
						lLength, NULL, 0, FALSE, 
						(void **) &szOutput, &nOutLength);

			if (IsntPGPError(nError))
			{
				if ((nOutLength > 0) && (szOutput != NULL))
				{
					SetRichEditContents(_hwndMessage, szOutput, FALSE, TRUE);
					PGPFreeData(szOutput);
				}
				else
					UIDisplayStringID(_hwndMessage, IDS_E_NOPGP);
			}
			
			chRange.cpMin = 0;
			chRange.cpMax = 0;
			SendMessage(_hwndMessage, EM_EXSETSEL, 
				(WPARAM)0, (LPARAM) &chRange);
			HeapFree(GetProcessHeap (), 0, szInput);
		}

		return S_OK;
	}

	if ((cmdid == _cmdidAddKey) && (_context == EECONTEXT_READNOTEMESSAGE))
	{
		if (_hwndMessage)
		{
			char *szInput;
			long lLength;
			BOOL bSelectedText = FALSE;
			CHARRANGE chRange = {0,0};
			PGPError nError = kPGPError_NoErr;

			// Determine if the user selected a particular piece of text
			SendMessage(_hwndMessage, EM_EXGETSEL, 
				(WPARAM) 0,(LPARAM) &chRange);
			bSelectedText = chRange.cpMax - chRange.cpMin;
			
			szInput = GetRichEditContents(_hwndMessage, &lLength, 
											FALSE, bSelectedText);
			if (!szInput)
				return S_OK;

			szInput[lLength-1] = 0;
			nError = AddKeyBuffer(hwndMain, _pgpContext, szInput, lLength, 
						FALSE);
			HeapFree(GetProcessHeap (), 0, szInput);
		}

		return S_OK;
	}

	if (cmdid == _cmdidPgpKeys)
	{
		char szPath[MAX_PATH];

		PGPcomdlgGetPGPPath(szPath, MAX_PATH-1);
		strcat(szPath, "PGPkeys /s");

		// run it...
		WinExec(szPath, SW_SHOW);
		return S_OK;
	}

	if (cmdid == _cmdidPrefs)
	{
		PGPcomdlgPreferences(_pgpContext, hwndMain, 0);
		return S_OK;
	}

	if (cmdid == _cmdidHelp)
	{
		CHAR szHelpFile[MAX_PATH] = {0x00};

		PGPcomdlgGetPGPPath(szHelpFile, MAX_PATH-1);
		strcat(szHelpFile, "PGP55.hlp");
		
		WinHelp(hwndMain, szHelpFile, HELP_FINDER, 0);
		return S_OK;
	}

	if (cmdid == _cmdidAbout)
	{
		PGPcomdlgHelpAbout(hwndMain, NULL, NULL, NULL);
		return S_OK;
	}

	return S_FALSE;
}


HWND FindMessageWindow(IExchExtCallback* pmecb)
{
	HWND hwndMain;
	HWND hwndSearch = NULL;
	HWND hwndFound = NULL;
	HWND hwndLast = NULL;
	FindStruct fsRichEdit;

	fsRichEdit.hwndFound = NULL;
	fsRichEdit.yMax = 0;

	pmecb->GetWindow(&hwndMain);
	EnumChildWindows(hwndMain, (WNDENUMPROC) ReportChildren, 
					(LPARAM) &fsRichEdit);
	hwndFound = fsRichEdit.hwndFound;

	return hwndFound;
}


BOOL CALLBACK ReportChildren(HWND hwnd, LPARAM lParam)
{
	char szClassName[200];
	FindStruct *pfsRichEdit;
	RECT rc;

	pfsRichEdit = (FindStruct *) lParam;
	GetClassName(hwnd, szClassName, 199);
	if (!strcmp(szClassName, "RICHEDIT") || 
		!strcmp(szClassName, "RichEdit20A"))
	{
		GetClientRect(hwnd, &rc);
		if (rc.bottom > pfsRichEdit->yMax)
		{
			pfsRichEdit->yMax = rc.bottom;
			pfsRichEdit->hwndFound = hwnd;
		}
	}

	return TRUE;
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
