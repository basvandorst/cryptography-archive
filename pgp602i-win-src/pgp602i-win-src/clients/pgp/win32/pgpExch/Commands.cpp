/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	


	$Id: Commands.cpp,v 1.34.2.2.2.1 1998/11/12 03:13:54 heller Exp $



____________________________________________________________________________*/
#include "stdinc.h"
#include "Exchange.h"
#include "resource.h"
#include "AddKey.h"
#include "EncryptSign.h"
#include "DecryptVerify.h"
#include "Prefs.h"
#include "Recipients.h"
#include "RichEdit_IO.h"
#include "BlockUtils.h"
#include "UIutils.h"

#include "pgpConfig.h"
#include "pgpOptionList.h"
#include "pgpUtilities.h"
#include "pgpSDKPrefs.h"
#include "pgpMem.h"
#include "PGPcl.h"
#include "PGPsc.h"

typedef struct {
		HWND hwndFound;
		int yMax;
		BOOL bInternetExplorer;
		BOOL bMicrosoftWord;
} FindStruct;

static HWND FindMessageWindow(IExchExtCallback* pmecb, FindStruct *fs);
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
			if (_fOutlook98)
				tbab.nID = IDB_T_ENCRYPT;
			else
				tbab.nID = IDB_ENCRYPT;
			_itbmEncrypt = SendMessage(hwndToolbar, TB_ADDBITMAP, 1, 
							(LPARAM)&tbab);

			if (_fOutlook98)
				tbab.nID = IDB_T_SIGN;
			else
				tbab.nID = IDB_SIGN;
			_itbmSign = SendMessage(hwndToolbar, TB_ADDBITMAP, 1, 
							(LPARAM)&tbab);
		}

		if (_context == EECONTEXT_READNOTEMESSAGE)
		{
			if (_fOutlook98)
				tbab.nID = IDB_T_DECRYPT;
			else
				tbab.nID = IDB_DECRYPT;
			_itbmDecrypt = SendMessage(hwndToolbar, TB_ADDBITMAP, 1, 
							(LPARAM)&tbab);
		}

		if (_fOutlook98)
			tbab.nID = IDB_T_PGPKEYS;
		else
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
	FindStruct fs;
	HWND hwndMain;
	HWND hwndToolbar;
	HRESULT hr;

	if ((EECONTEXT_READNOTEMESSAGE != _context) &&
		(EECONTEXT_SENDNOTEMESSAGE != _context))
		return;

	pmecb->GetWindow(&hwndMain);
	_hwndMessage = FindMessageWindow(pmecb, &fs);

	hr = pmecb->GetToolbar(EETBID_STANDARD, &hwndToolbar);
	if (hr != S_OK)
		return;

	if (fs.bInternetExplorer || fs.bMicrosoftWord)
	{
		if (_context == EECONTEXT_SENDNOTEMESSAGE)
		{
			DeleteMenu(_hmenuPGP, _cmdidEncryptNow, MF_BYCOMMAND);
			DeleteMenu(_hmenuPGP, _cmdidSignNow, MF_BYCOMMAND);
			DeleteMenu(_hmenuPGP, _cmdidEncryptSign, MF_BYCOMMAND);
			DrawMenuBar(hwndMain);
		}

		if (_context == EECONTEXT_READNOTEMESSAGE)
		{
			DeleteMenu(_hmenuPGP, _cmdidDecrypt, MF_BYCOMMAND);
			DrawMenuBar(hwndMain);

			SendMessage(hwndToolbar, TB_ENABLEBUTTON, 
				_cmdidDecrypt, MAKELONG(FALSE, 0));
		}
	}
	else
		if (_context == EECONTEXT_READNOTEMESSAGE)
		{
			char szCommand[256];
			char szMenu[256];
			MENUITEMINFO miiMenu;

			miiMenu.cbSize = sizeof(MENUITEMINFO);
			miiMenu.fMask = MIIM_TYPE;
			miiMenu.dwTypeData = szMenu;
			miiMenu.cch = 255;

			UIGetString(szCommand, sizeof(szCommand), IDS_DECRYPT_MENU);
			GetMenuItemInfo(_hmenuPGP, 0, TRUE, &miiMenu);
			if (strcmp(szCommand, szMenu))
			{
				strcpy(szMenu, szCommand);
				SetMenuItemInfo(_hmenuPGP, 0, TRUE, &miiMenu);
				DrawMenuBar(hwndMain);
			}

			SendMessage(hwndToolbar, TB_ENABLEBUTTON, 
				_cmdidDecrypt, MAKELONG(TRUE, 0));
		}


	if (!_fInitMenuOnce)
	{
		// Only set encrypt and sign flags once.
		
		_fInitMenuOnce = TRUE;
		
		_bEncrypt = ByDefaultEncrypt(_memoryMgr);
		_bSign = ByDefaultSign(_memoryMgr);
	}

	// Now to the real menu business

	if (_context == EECONTEXT_SENDNOTEMESSAGE)
	{
		HMENU hmenu;
		MENUITEMINFO miiEncrypt;
		MENUITEMINFO miiSign;
		
		hr = pmecb->GetMenu(&hmenu);
		if (FAILED(hr))
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
	HWND hwndMain;
	FindStruct fs;
	BOOL FYEO;

	if ((EECONTEXT_READNOTEMESSAGE != _context) &&
		(EECONTEXT_SENDNOTEMESSAGE != _context) &&
		(EECONTEXT_VIEWER != _context))
		return S_FALSE;

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

	pmecb->GetWindow(&hwndMain);

	if ((_context == EECONTEXT_SENDNOTEMESSAGE) || 
		(_context == EECONTEXT_READNOTEMESSAGE))
		_hwndMessage = FindMessageWindow(pmecb, &fs);

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
			RECIPIENTDIALOGSTRUCT *prds;
			PGPKeySetRef newKeySet = NULL;
			PGPError err;

			prds = (RECIPIENTDIALOGSTRUCT *) 
					calloc(sizeof(RECIPIENTDIALOGSTRUCT), 1);

			err = PGPsdkLoadDefaultPrefs(_pgpContext);
			if (IsPGPError(err))
			{
				UIDisplayErrorCode(__FILE__, __LINE__, NULL, err);
				return S_FALSE;
			}

			err = PGPOpenDefaultKeyRings(_pgpContext, (PGPKeyRingOpenFlags)0, 
					&(prds->OriginalKeySetRef));

			if (IsPGPError(err))
			{
				UIDisplayErrorCode(__FILE__, __LINE__, NULL, err);
				return S_FALSE;
			}

			PGPNewKeySet(_pgpContext, &newKeySet);
			PGPAddKeys(prds->OriginalKeySetRef, newKeySet);
			PGPFreeKeySet(prds->OriginalKeySetRef);

			prds->OriginalKeySetRef = newKeySet;

			if (EncryptSignMessageWindow(hwndMain, pmecb, bEncrypt, bSign, 
				prds))
			{
				_bEncrypt = _bEncrypt && !bEncrypt;
				_bSign = _bSign && !bSign;
				InitMenu(pmecb);
			}

			FreeRecipients(prds);
	
			if (IsntNull(prds->OriginalKeySetRef))
			{
				PGPFreeKeySet(prds->OriginalKeySetRef);
				prds->OriginalKeySetRef = NULL;
			}

			free(prds);
		}

		return S_OK;
	}

	if ((cmdid == _cmdidDecrypt) && (_context == EECONTEXT_READNOTEMESSAGE))
	{
		if (_hwndMessage)
		{
			char *szInput;
			char *szOutput = NULL;
			long lLength;
			UINT nOutLength;
			BOOL bSelectedText = FALSE;
			CHARRANGE chRange = {0,0};
			PGPError nError = kPGPError_NoErr;
			char szName[256];
			char szFile[256];

			UIGetString(szName, sizeof(szName), IDS_LOGNAME);
			UIGetString(szFile, sizeof(szFile), IDS_DLL);

			// Determine if the user selected a particular piece of text
			SendMessage(_hwndMessage, EM_EXGETSEL, 
				(WPARAM)0, (LPARAM) &chRange);
			bSelectedText = chRange.cpMax - chRange.cpMin;
			
			szInput = GetRichEditContents(_hwndMessage, &lLength, 
												FALSE, bSelectedText);
			if (!szInput)
				return S_OK;

			chRange.cpMin = 0;
			chRange.cpMax = lLength;

			while (!isgraph(szInput[chRange.cpMax-3]))
				chRange.cpMax--;

			if (_fOutlook)
			{
				char *szCR;
				int nNumCR=0;
					
				szCR = strchr(szInput, '\r');
				while (szCR && (szCR < (szInput+lLength)))
				{
					nNumCR++;
					szCR = strchr(szCR+1, '\r');
				}

				chRange.cpMax -= nNumCR;
			}

			SendMessage(_hwndMessage, EM_EXSETSEL, (WPARAM)0, 
				(LPARAM) &chRange);

			nError = DecryptVerifyBuffer(UIGetInstance(), hwndMain, 
						_pgpContext, _tlsContext,
						szName, szFile, szInput, lLength,
						FALSE, (void **) &szOutput, &nOutLength, &FYEO);

			if (IsntPGPError(nError))
			{
				if ((nOutLength > 0) && (szOutput != NULL))
				{
					if(FYEO)
					{
						TempestViewer((void *)_pgpContext,hwndMain,
							szOutput,nOutLength);
					}
					else
					{
						if ((nOutLength != (UINT) lLength) &&
							(nOutLength != (UINT) (lLength+1)))
						{
							SetRichEditContents(_hwndMessage, szOutput, 
								FALSE, TRUE);
						}
					}

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

	if (cmdid == _cmdidPgpKeys)
	{
		char szPath[MAX_PATH];
		char szPGPkeys[MAX_PATH];

		PGPclGetPGPPath(szPath, MAX_PATH-1);
		UIGetString(szPGPkeys, sizeof(szPGPkeys), IDS_PGPKEYSEXE);
		strcat(szPath, szPGPkeys);

		// run it...
		WinExec(szPath, SW_SHOW);
		return S_OK;
	}

	if (cmdid == _cmdidPrefs)
	{
		PGPclPreferences(_pgpContext, hwndMain, 0, NULL);
		return S_OK;
	}

	if (cmdid == _cmdidHelp)
	{
		CHAR szHelpFile[MAX_PATH] = {0x00};
		char szHelpName[256];

		PGPclGetPGPPath(szHelpFile, MAX_PATH-1);
		UIGetString(szHelpName, sizeof(szHelpName), IDS_PGPHELP);
		strcat(szHelpFile, szHelpName);
		
		WinHelp(hwndMain, szHelpFile, HELP_FINDER, 0);
		return S_OK;
	}

	if (cmdid == _cmdidAbout)
	{
		PGPclHelpAbout(_pgpContext, hwndMain, NULL, NULL, NULL);
		return S_OK;
	}

	return S_FALSE;
}


BOOL CExtImpl::EncryptSignMessageWindow(HWND hwndMain,
										IExchExtCallback *pmecb,
										BOOL bEncrypt,
										BOOL bSign,
										RECIPIENTDIALOGSTRUCT *prds)
{
	char *szInput;
	char *szOutput = NULL;
	long lLength;
	UINT nOutLength;
	BOOL bSelectedText = FALSE;
	BOOL bReturn = FALSE;
	CHARRANGE chRange = {0,0};
	PGPOptionListRef signOptions = NULL;
	char szName[256];
	char szFile[256];
	PGPError nError = kPGPError_NoErr;

//	prds = (RECIPIENTDIALOGSTRUCT *) 
//			calloc(sizeof(RECIPIENTDIALOGSTRUCT), 1);

	if ((prds == NULL) && bEncrypt)
		return FALSE;

	UIGetString(szName, sizeof(szName), IDS_LOGNAME);
	UIGetString(szFile, sizeof(szFile), IDS_DLL);

	if (bEncrypt)
		nError = GetRecipients(pmecb, _pgpContext, _tlsContext, prds);
			
	if (IsPGPError(nError))
	{
		if (nError != kPGPError_UserAbort)
			UIDisplayErrorCode(__FILE__, __LINE__, NULL, nError);
		return FALSE;
	}

	// Determine if the user selected a particular piece of text
	SendMessage(_hwndMessage, EM_EXGETSEL, 
		(WPARAM)0, (LPARAM) &chRange);

	bSelectedText = chRange.cpMax - chRange.cpMin;
			
	szInput = GetRichEditContents(_hwndMessage, &lLength, 
									FALSE, bSelectedText);
	if (!szInput)
		return FALSE;

	lLength = strlen(szInput);
	nError = EncryptSignBuffer(UIGetInstance(), hwndMain, _pgpContext, 
				_tlsContext, szName, szFile, szInput, 
				lLength, prds, NULL, &signOptions, (void **) &szOutput, 
				&nOutLength, bEncrypt, bSign, FALSE);

	if (IsntPGPError(nError))
	{
		SetRichEditContents(_hwndMessage, szOutput, FALSE, 
			bSelectedText);
		PGPFreeData(szOutput);
		bReturn = TRUE;
	}

	if (signOptions != NULL)
	{
		PGPFreeOptionList(signOptions);
		signOptions = NULL;
	}

	memset(szInput, 0, lLength);
	HeapFree(GetProcessHeap (), 0, szInput);

	return bReturn;
}


HWND FindMessageWindow(IExchExtCallback* pmecb, FindStruct *fs)
{
	HWND hwndMain;
	HWND hwndSearch = NULL;
	HWND hwndFound = NULL;
	HWND hwndLast = NULL;

	fs->hwndFound = NULL;
	fs->yMax = 0;
	fs->bInternetExplorer = FALSE;
	fs->bMicrosoftWord = FALSE;

	pmecb->GetWindow(&hwndMain);
	EnumChildWindows(hwndMain, (WNDENUMPROC) ReportChildren, 
					(LPARAM) fs);

	hwndFound = fs->hwndFound;
	return hwndFound;
}


BOOL CALLBACK ReportChildren(HWND hwnd, LPARAM lParam)
{
	char szClassName[200];
	char szOldEditClass[256];
	char szNewEditClass[256];
	char szIEClass[256];
	char szWordClass[256];
	FindStruct *pfsRichEdit;
	RECT rc;

	UIGetString(szOldEditClass, sizeof(szOldEditClass), IDS_OLDEDITCTRLCLASS);
	UIGetString(szNewEditClass, sizeof(szNewEditClass), IDS_NEWEDITCTRLCLASS);
	UIGetString(szIEClass, sizeof(szIEClass), IDS_IE40CLASS);
	UIGetString(szWordClass, sizeof(szWordClass), IDS_WORDCLASS);

	pfsRichEdit = (FindStruct *) lParam;
	GetClassName(hwnd, szClassName, 199);

	if (!strcmp(szClassName, szOldEditClass) || 
		!strcmp(szClassName, szNewEditClass))
	{
		GetWindowRect(hwnd, &rc);
		if (rc.bottom > pfsRichEdit->yMax)
		{
			pfsRichEdit->yMax = rc.bottom;
			pfsRichEdit->hwndFound = hwnd;
		}
	}

	if (!strcmp(szClassName, szIEClass))
		pfsRichEdit->bInternetExplorer = TRUE;

	if (!strcmp(szClassName, szWordClass))
		pfsRichEdit->bMicrosoftWord = TRUE;

	return TRUE;
}



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
