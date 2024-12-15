/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */

#include "stdinc.h"
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pgpexch.h"
#include "ui.h"
#include "assert.h"
#include "resource.h"
#include "re_io.h"
#include "outlook.h"

#include "ExchKeyDB.h"

#include "config.h"
#include "pgpTypes.h"
#include "pgpKeyDB.h"
#include "pgpMem.h"
#include "PGPcomdlg.h"
#include "PGPRecip.h"
#include "PGPPhrase.h"
#include "spgp.h"
//#include "pgpVersion.h"

#include "BlockUtils.h"

// Used by assert() and other code of last resort

const char SzDefaultCaption[] = "PGP Plugin for Exchange";

// The version that went out with Win95 is 410; with Exchange Server, 837.
// (RC1 used 611; RC2 used 736.3.)
// The Windows 95 Messaging Update is 839.

#define LAST_BUILD_SUPPORTED 994

// Other values returned by IExchExtCallback::GetVersion
// Microsoft Exchange 4.0 is 0x01??0400

#define CURRENT_PRODUCT_CODE  0x01000000
#define LAST_MAJVER_SUPPORTED 0x00000400
#define LAST_MINVER_SUPPORTED 0x00000000

// Module local declarations

extern "C" IExchExt* CALLBACK ExchEntryPoint();
static HWND FindMessageWindow(IExchExtCallback* pmecb);
BOOL CALLBACK ReportChildren(HWND hwnd, LPARAM lParam);
static BOOL IsExchExtWithinOutlook(IExchExtCallback* peecb);
static void HexDump(HWND hwnd, char *szMessage, int nLength);

typedef struct {
		HWND hwndFound;
		int yMax;
} FindStruct;

class CExtImpl : public IExchExt,
	IExchExtPropertySheets, IExchExtMessageEvents,
	IExchExtCommands, IExchExtAttachedFileEvents
{
public:
    CExtImpl();
	~CExtImpl();

	// The methods of IUnknown

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    inline STDMETHODIMP_(ULONG) AddRef()
    	{ ++_cRef; return _cRef; }
    inline STDMETHODIMP_(ULONG) Release()
         { ULONG ulCount = --_cRef;
           if (!ulCount) { delete this; }
           return ulCount;
         }

	// The methods of IExchExt

    STDMETHODIMP Install(IExchExtCallback* pmecb,
                          ULONG mecontext, ULONG ulFlags);

    // The methods of IExchExtPropertySheets

    STDMETHODIMP_(ULONG) GetMaxPageCount(ULONG ulFlags);
    STDMETHODIMP GetPages(IExchExtCallback* peecb,
                          ULONG ulFlags, LPPROPSHEETPAGE ppsp,
						  ULONG * pcpsp);
    STDMETHODIMP_(VOID) FreePages(LPPROPSHEETPAGE ppsp, ULONG ulFlags,
                                             ULONG cpsp);

	// The methods of IExchExtMessageEvents

    STDMETHODIMP OnRead(IExchExtCallback* lpeecb);
    STDMETHODIMP OnReadComplete(IExchExtCallback* lpeecb, ULONG ulFlags);
    STDMETHODIMP OnWrite(IExchExtCallback* lpeecb);
    STDMETHODIMP OnWriteComplete(IExchExtCallback* lpeecb, ULONG ulFlags);
    STDMETHODIMP OnCheckNames(IExchExtCallback* lpeecb);
    STDMETHODIMP OnCheckNamesComplete(IExchExtCallback* lpeecb,
										ULONG ulFlags);
    STDMETHODIMP OnSubmit(IExchExtCallback* lpeecb);
    STDMETHODIMP_(VOID) OnSubmitComplete(IExchExtCallback* lpeecb,
											ULONG ulFlags);

	// The methods of IExchExtCommands

    STDMETHODIMP InstallCommands(IExchExtCallback* pmecb,
                                   HWND hwnd, HMENU hmenu,
                                   UINT * cmdidBase, LPTBENTRY lptbeArray,
                                   UINT ctbe, ULONG ulFlags);
    STDMETHODIMP DoCommand(IExchExtCallback* pmecb, UINT mni);
    STDMETHODIMP_(VOID) InitMenu(IExchExtCallback* pmecb);
    STDMETHODIMP Help(IExchExtCallback* pmecb, UINT mni);
    STDMETHODIMP QueryHelpText(UINT mni, ULONG ulFlags, LPTSTR sz, UINT cch);
    STDMETHODIMP QueryButtonInfo(ULONG tbid, UINT itbb, LPTBBUTTON ptbb,
                                LPTSTR lpsz, UINT cch, ULONG ulFlags);
    STDMETHODIMP ResetToolbar(ULONG tbid, ULONG ulFlags);

	// The methods of IExchExtAttachedFileEvents

	STDMETHODIMP OnReadPattFromSzFile(IAttach* lpAtt, LPTSTR lpszFile,
										ULONG ulFlags);
	STDMETHODIMP OnWritePattToSzFile(IAttach* lpAtt, LPTSTR lpszFile,
										ULONG ulFlags);
	STDMETHODIMP QueryDisallowOpenPatt(IAttach* lpAtt);
    STDMETHODIMP OnOpenPatt(IAttach* lpAtt);
	STDMETHODIMP OnOpenSzFile(LPTSTR lpszFile, ULONG ulFlags);

private:
    ULONG	_cRef;

	// Set in Install.

    UINT	_context;			// The context of the current instance

    BOOL	_fOldEEME;			// Set if old style IExchExtMessageEvents
	BOOL	_fOutlook;			// Set if Outlook97 is being used
	HWND	_hwndRE;			// The richedit control on a note form
	HWND	_hwndMessage;		// Window containing message

	// Set in OnSubmit and OnSubmitComplete.

    BOOL    _fInSubmitState;	// Set if in a submission.

	// Set on the first pass through InitMenu.

	BOOL	_fInitMenuOnce;		// Flag as to whether the above is
								// initialized yet.

	// Set in InstallCommands.

	// PGP menu on menu bar

	HMENU _hmenuPGP;

	// Menu and/or button command ID's

	UINT	_cmdidSign;
	UINT	_cmdidSignNow;
	UINT	_cmdidEncrypt;
	UINT	_cmdidEncryptNow;
	UINT	_cmdidEncryptSign;
	UINT	_cmdidDecrypt;
	UINT	_cmdidAddKey;
	UINT	_cmdidPgpKeys;
	UINT	_cmdidPrefs;
	UINT	_cmdidHelp;
	UINT	_cmdidAbout;

	// Button bitmap ID's

	UINT	_itbmSign;
	UINT	_itbmSignNow;
	UINT	_itbmEncrypt;
	UINT	_itbmEncryptNow;
	UINT	_itbmEncryptSign;
	UINT	_itbmDecrypt;
	UINT	_itbmAddKey;
	UINT	_itbmPgpKeys;

	// Button ID's

	UINT	_itbbSign;
	UINT	_itbbSignNow;
	UINT	_itbbEncrypt;
	UINT	_itbbEncryptNow;
	UINT	_itbbEncryptSign;
	UINT	_itbbDecrypt;
	UINT	_itbbAddKey;
	UINT	_itbbPgpKeys;

	// Toolbar handles

	HWND	_hwndSendToolbar;
	HWND	_hwndReadToolbar;

	// Message flags

	BOOL	_bSign;
	BOOL	_bEncrypt;
	BOOL	_bUseMIME;

	// Attachment variables

	UINT	_nAttachNum;
	char *	_szSig;

	// Set in either or both of InitMenu and OnReadComplete.

	CMsgType _msgtype;			// Type of message, analyzed at note-read time.

};


// Implementation follows

// DLL entry point

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID)
{
 	if (DLL_PROCESS_ATTACH == fdwReason)
 	{
		// Make sure the PGP plugin hasn't expired
		if (PGPcomdlgIsExpired(NULL))
			return FALSE;

		CUICommon::Init(hinstDll);
		pgpLibInit();
 	}

 	if (DLL_PROCESS_DETACH == fdwReason)
 	{
		while (PGPPurgeCachedPhrase() || PGPPurgeSignCachedPhrase())
		{
			Sleep(250);
		}
		pgpLibCleanup();
 	}

	return TRUE;
}


// Exchange client extension entry point

IExchExt* CALLBACK ExchEntryPoint()
{
	return new CExtImpl;
}


CExtImpl::CExtImpl()
	: _cRef(1), _context(0),
	  _fOldEEME(FALSE), _fInSubmitState(FALSE),
	  _fInitMenuOnce(FALSE),
	  _hwndRE(NULL), _msgtype()
{
	_hmenuPGP = NULL;
}


CExtImpl::~CExtImpl()
{
	if (_hmenuPGP)
		DestroyMenu(_hmenuPGP);
}


STDMETHODIMP CExtImpl::QueryInterface(REFIID riid, void** ppvObj)
{
    *ppvObj = NULL;

    HRESULT hr = S_OK;
	IUnknown* punk = NULL;

    if (( IID_IUnknown == riid) || ( IID_IExchExt == riid) )
    {
        punk = (IExchExt*)this;
    }
    else if (IID_IExchExtPropertySheets == riid)
    {
        punk = (IExchExtPropertySheets*)this;
    }
    else if (IID_IExchExtMessageEvents == riid)
    {
        punk = (IExchExtMessageEvents*)this;
     }
     else if (IID_IExchExtCommands == riid)
     {
         punk = (IExchExtCommands*)this;
     }
     else if (IID_IExchExtAttachedFileEvents == riid)
     {
         punk = (IExchExtAttachedFileEvents*)this;
     }
     else
         hr = E_NOINTERFACE;

	if (NULL != punk)
	{
		*ppvObj = punk;
		AddRef();
	}

     return hr;
}


STDMETHODIMP CExtImpl::Install(IExchExtCallback* peecb, ULONG eecontext,
							   ULONG ulFlags)
{
	static BOOL fWarnOnce = FALSE;
	static BOOL fMinorBuildOk = FALSE;

    ULONG ulBuildVer;
	ULONG ulProductVer;
    HRESULT hr;

    _context = eecontext;
	CUICommon ui;

    // Ensure that this is the right version.

	hr = peecb->GetVersion(&ulProductVer, EECBGV_GETVIRTUALVERSION);
	if (SUCCEEDED(hr))
		hr = peecb->GetVersion(&ulBuildVer, EECBGV_GETBUILDVERSION);
	if (FAILED(hr))
	{
		ui.Message(IDS_E_NOVERSION);
		return S_FALSE;
	}

    if ((CURRENT_PRODUCT_CODE !=
		(ulProductVer & EECBGV_VERSION_PRODUCT_MASK))
		||
		(EECBGV_BUILDVERSION_MAJOR !=
		(ulBuildVer & EECBGV_BUILDVERSION_MAJOR_MASK)))
	{
		// The first time, explain why we aren't loading.
		// Subsequently, remain silent.

		if (!fWarnOnce)
		{
			fWarnOnce = TRUE;
			ui.Message(IDS_E_INCOMPATIBLE_VERSION);
		}
         return S_FALSE;
	}
	if ((LAST_MAJVER_SUPPORTED <
		(ulProductVer & EECBGV_VERSION_MAJOR_MASK))
		 ||
		(LAST_MINVER_SUPPORTED <
		(ulProductVer & EECBGV_VERSION_MINOR_MASK))
		 ||
		(LAST_BUILD_SUPPORTED  <
		(ulBuildVer & EECBGV_BUILDVERSION_MINOR_MASK)) )
	{
		// The first time, make a note of it.
		// Subsequently, abide by the user's decision.

		if (!fWarnOnce)
		{
			fWarnOnce = TRUE;
			fMinorBuildOk =
				(ui.Query(IDS_Q_LATER_BUILD, MB_YESNO) == IDYES);
		}
		if (!fMinorBuildOk)
			return S_FALSE;
	}


	// Check to see if we're running Outlook97
	_fOutlook = IsExchExtWithinOutlook(peecb);

	BOOL fFindRE = FALSE; // Set if it needs to find the RE.

    switch (eecontext)
    {
	case EECONTEXT_SENDPOSTMESSAGE:
	case EECONTEXT_SENDNOTEMESSAGE:
	case EECONTEXT_READNOTEMESSAGE:
	case EECONTEXT_READPOSTMESSAGE:
//		fFindRE = TRUE;
		hr = S_OK;
		break;

    case EECONTEXT_PROPERTYSHEETS:
		// To get the property page
         hr = S_OK;
         break;

	case EECONTEXT_VIEWER:
		hr = S_OK;
		break;

	case EECONTEXT_READREPORTMESSAGE:
	case EECONTEXT_SENDRESENDMESSAGE:
     default:
         hr = S_FALSE;
         break;
     }

     // Make a note of pre-RC1 builds,
     // in which the IExchExtMessageEvent sequence differed on reply notes.

     _fOldEEME = ((EECBGV_BUILDVERSION_MINOR_MASK & ulBuildVer) < 611);

     return hr;
}


STDMETHODIMP_(ULONG) CExtImpl::GetMaxPageCount(ULONG ulFlags)
{
	// In a message, Exchange is calling the extension once for
	// propertysheets, and once for the note form.  Respond only
	// once.
	
	if (_context != EECONTEXT_PROPERTYSHEETS)
		return 0;

	// Note that "ulFlags" is actually an opcode, not a word of
	// flags.

	if (ulFlags == EEPS_TOOLSOPTIONS)
		return 1;

	// Ignore all other property sheets.

	return 0;
}


STDMETHODIMP CExtImpl::GetPages(IExchExtCallback* peecb,
	ULONG ulFlags, LPPROPSHEETPAGE ppsp, ULONG * pcpsp)
{
	if (_context != EECONTEXT_PROPERTYSHEETS)
		return S_FALSE;

	if (ulFlags != EEPS_TOOLSOPTIONS)
		return S_FALSE;

    ppsp[0].dwSize = sizeof (PROPSHEETPAGE);
    ppsp[0].dwFlags = PSP_DEFAULT|PSP_HASHELP;
    ppsp[0].hInstance = CUICommon::GetHinst();
    ppsp[0].pszTemplate = MAKEINTRESOURCE(IDD_MAINPROP);
    ppsp[0].hIcon = NULL;     // not used in this sample
    ppsp[0].pszTitle = NULL;  // not used in this sample
    ppsp[0].pfnDlgProc = (DLGPROC)MainPropPageDlgProc;

	// Give the property page a reference to the session to use.

	IMAPISession* psess;
	HRESULT hr = peecb->GetSession(&psess, NULL);
	if (FAILED(hr))
	{
		CUICommon().Message(IDS_E_NOSESSION);
		return S_FALSE;
	}

    ppsp[0].lParam = (DWORD)psess; // Inherits AddRef in GetSession.
    ppsp[0].pfnCallback = NULL;
    ppsp[0].pcRefParent = NULL; // Not used in this sample

    *pcpsp = 1; // Only a single page

    return S_OK;
}


STDMETHODIMP_(VOID) CExtImpl::FreePages(LPPROPSHEETPAGE ppsp,
										ULONG ulFlags, ULONG cpsp)
{
	if (_context != EECONTEXT_PROPERTYSHEETS)
		return;

	if (ulFlags != EEPS_TOOLSOPTIONS)
		return;

	// Release the session reference that the property page held.

	IMAPISession* psess = (IMAPISession*)ppsp[0].lParam;
	psess->Release();
}


STDMETHODIMP CExtImpl::OnRead(IExchExtCallback* peecb)
{
	HWND hwnd;
	TCHAR szTitle[200];

	peecb->GetWindow(&hwnd);
	if (!hwnd)
		hwnd = GetTopWindow(NULL);

	GetString(szTitle, sizeof(szTitle)/sizeof(TCHAR), IDS_TITLE);

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

/*			
				tbbNew[3].iBitmap = _itbmEncryptSign;
				tbbNew[3].idCommand = _cmdidEncryptSign;
				tbbNew[3].fsState = TBSTATE_ENABLED;
				tbbNew[3].fsStyle = TBSTYLE_BUTTON;
				tbbNew[3].dwData = 0;
				tbbNew[3].iString = -1;
*/
			
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
			
				tbbNew[2].iBitmap = _itbmAddKey;
				tbbNew[2].idCommand = _cmdidAddKey;
				tbbNew[2].fsState = TBSTATE_ENABLED;
				tbbNew[2].fsStyle = TBSTYLE_BUTTON;
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
			
				SendMessage(_hwndReadToolbar, TB_ADDBUTTONS, 5,
							(LPARAM) tbbNew);
			}

			IMAPISession* psess;

			_nAttachNum = 0;
			HRESULT hr = peecb->GetSession(&psess, NULL);
			if (FAILED(hr))
			{
				CUICommon().Message(IDS_E_NOSESSION);
				return S_FALSE;
			}
			
			CDecrypt decrypt(psess);
			decrypt.Init();
			
			if (decrypt.WillAutoDecrypt() || decrypt.WillAutoAddKey())
			{
				IMessage *pmsg = 0;
				STATSTG StreamStats;
				DWORD dwInSize;
				ULONG ulOutSize;
				char *pInput;
				char *pOutput = NULL;
				CUICommon ui(hwnd);
				int nError;
				
				CWaitCursor wait; // Mark busy
				
				hr = peecb->GetObject(NULL, (IMAPIProp**)&pmsg);
				if (FAILED(hr))
				{
					ui.Message(IDS_E_NOMESSAGE);
					return S_FALSE;
				}
				
				IStream *pstrmBody = 0;
				hr = pmsg->OpenProperty(PR_BODY, &IID_IStream, STGM_READWRITE,
					MAPI_MODIFY, (IUnknown**)&pstrmBody);
				if (FAILED(hr))
				{
					ui.ErrorMessage(pmsg, hr, IDS_E_NOBODY);
					pmsg->Release();
					psess->Release();
					return S_FALSE;
				}
				
				pstrmBody->Stat(&StreamStats, STATFLAG_NONAME);
				dwInSize = StreamStats.cbSize.LowPart;
				
				pInput = (char *) calloc(dwInSize+1, sizeof(char));
				if (!pInput)
				{
					ui.Message(IDS_E_NOMEMORY);
					pstrmBody->Release();
					pmsg->Release();
					psess->Release();
					return S_FALSE;
				}
				pstrmBody->Read(pInput, dwInSize, &dwInSize);
				pInput[dwInSize] = 0;
				
				if (decrypt.WillAutoAddKey())
					AddKeyBuffer(hwnd, pInput, dwInSize);

				if (decrypt.WillAutoDecrypt())
				{
					int nContents;

					// Check to see if there's anything to decrypt/verify
					nContents = SimplePGPAnalyzeBuffer(pInput, dwInSize);

					if (nContents == SIMPLEANALYZE_IOERROR)
					{
						char szError[200];
						char szTitle[200];

						GetString(szError, sizeof(szError), IDS_E_BADPGPBLOCK);
						GetString(szTitle, sizeof(szTitle), IDS_TITLE);
						MessageBox(hwnd, szError, szTitle, MB_ICONEXCLAMATION);

						free(pInput);
						pstrmBody->Release();
						pmsg->Release();
						psess->Release();
						return S_FALSE;
					}

					if ((nContents != SIMPLEANALYZE_ENCR) &&
						(nContents != SIMPLEANALYZE_SIGN))
					{
						free(pInput);
						pstrmBody->Release();
						pmsg->Release();
						psess->Release();
						return S_FALSE;
					}

					nError = DecryptVerifyBuffer(hwnd, pInput, dwInSize,
												NULL, 0, (VOID ** )&pOutput,
												&ulOutSize);
					if (!nError)
					{
						LARGE_INTEGER li = {0,0};
						ULARGE_INTEGER uli = {ulOutSize, 0};
						BOOL fPartied;
						char *szBuffer;
						ULONG ulBlockStart, ulBlockLength;
						
						// Alter only the block of encrypted/signed text
						szBuffer = (char *) calloc(dwInSize+ulOutSize+1,
													sizeof(char));
						if (FindEncryptedBlock(pInput, dwInSize,
												&ulBlockStart, &ulBlockLength))
						{
							strncpy(szBuffer, pInput, ulBlockStart);
							szBuffer[ulBlockStart] = 0;
							strcat(szBuffer, pOutput);
							strcat(szBuffer,
									&(pInput[ulBlockStart+ulBlockLength]));
						}
						else if (FindSignedBlock(pInput, dwInSize,
												&ulBlockStart, &ulBlockLength))
						{
							strncpy(szBuffer, pInput, ulBlockStart);
							szBuffer[ulBlockStart] = 0;
							strcat(szBuffer, pOutput);
							strcat(szBuffer,
									&(pInput[ulBlockStart+ulBlockLength]));
						}
						else
							strcpy(szBuffer, pOutput);

						if (nContents == SIMPLEANALYZE_ENCR)
						{
							uli.LowPart = strlen(szBuffer);
							pstrmBody->Seek(li, STREAM_SEEK_SET, NULL);
							pstrmBody->Write(szBuffer, strlen(szBuffer), NULL);
							pstrmBody->SetSize(uli);
							pstrmBody->Commit(STGC_DEFAULT);
							pstrmBody->Release();
							RTFSync(pmsg, RTF_SYNC_BODY_CHANGED, &fPartied);
						}

						memset(pOutput, 0, ulOutSize);
						memset(szBuffer, 0, dwInSize+ulOutSize+1);
						pgpFree(pOutput);
						free(szBuffer);
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

/*
	IMAPISession* psess;
	HRESULT hr = peecb->GetSession(&psess, NULL);
	if (FAILED(hr))
	{
		CUICommon().Message(IDS_E_NOSESSION);
		return S_FALSE;
	}

	CCharFormat cf(psess);
	psess->Release();
	cf.Load();
	SendMessage(_hwndRE, EM_SETCHARFORMAT, 0, (LPARAM)(void*)&cf);
*/

	// Can't yet detect whether it's a reply, since recipients
	// are unavailable.  (Pre 611)

	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnReadComplete(IExchExtCallback* peecb, ULONG ulFlags)
{
	HWND hwndMain;
	TCHAR szTitle[200];

	peecb->GetWindow(&hwndMain);
	if (!hwndMain)
		hwndMain = GetTopWindow(NULL);

	GetString(szTitle, sizeof(szTitle)/sizeof(TCHAR), IDS_TITLE);

	if (_context != EECONTEXT_READNOTEMESSAGE)
		return S_FALSE;

/*
	IMAPISession* psess;

	_nAttachNum = 0;
	HRESULT hr = peecb->GetSession(&psess, NULL);
	if (FAILED(hr))
	{
		CUICommon().Message(IDS_E_NOSESSION);
		return S_FALSE;
	}
			
	CDecrypt decrypt(psess);
	decrypt.Init();
			
	if (decrypt.WillAutoDecrypt() || decrypt.WillAutoAddKey())
	{
		_hwndMessage = FindMessageWindow(peecb);
		if (_hwndMessage)
		{
			char *szInput;
			char *szOutput = NULL;
			long lLength;
			ULONG ulOutLength;
			BOOL bSelectedText = FALSE;
			CHARRANGE chRange = {0,0};

			// Determine if the user selected a particular piece of text
			SendMessage(_hwndMessage,  EM_EXGETSEL, (WPARAM)0,
						(LPARAM) &chRange);
			bSelectedText = chRange.cpMax - chRange.cpMin;
			
			// If not, select all the text
			if(!bSelectedText)
			{
				RECT rect;
				
				GetClientRect(_hwndMessage, &rect);
				SendMessage(_hwndMessage, WM_LBUTTONDOWN, MK_LBUTTON,
							MAKELPARAM(rect.right - 5, rect.bottom -5 ));
				SendMessage(_hwndMessage, WM_LBUTTONUP, MK_LBUTTON,
							MAKELPARAM(rect.right - 5 , rect.bottom -5 ));
			}
			
			szInput = GetRichEditContents(_hwndMessage, &lLength,
											FALSE, bSelectedText);
			szInput[lLength-1] = 0;
			if (!DecryptVerifyBuffer(hwndMain, szInput, lLength, NULL, 0,
									(void **) &szOutput, &ulOutLength))
			{
				SetWindowText(_hwndMessage, szOutput);
				pgpFree(szOutput);
			}
			
			HeapFree(GetProcessHeap (), 0, szInput);
		}
	}

	psess->Release();
*/
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
	// The postwrite handler appends signatures onto outgoing messages.

	if (_context != EECONTEXT_SENDNOTEMESSAGE)
		return S_FALSE;

	if (ulFlags == EEME_COMPLETE_FAILED) // Nothing to unwind
		return S_FALSE;

	if (!_fInSubmitState)	// This is not a submission.
		return S_FALSE;
//	if (!_fUseSig)			// Suppressed on this message, or else not present
//		return S_FALSE;

	if (_bEncrypt || _bSign)
	{
		IMessage *pmsg = 0;
		STATSTG StreamStats;
		DWORD dwInSize;
		DWORD dwNumRecip = 0;
		ULONG ulOutSize;
		char *pInput;
		char *pOutput = NULL;
		PRECIPIENTDIALOGSTRUCT prds = NULL;
		HWND hwnd;
		LPMAPITABLE ptableAttach = 0;
		LPATTACH pAttach = 0;
		SizedSPropTagArray(1, tagaTable) = {
				1, {PR_ATTACH_NUM}};
		SRowSet *prAttach = 0;
		int nError=0;

		pecb->GetWindow(&hwnd);
		if (!hwnd)
			hwnd = GetTopWindow(NULL);
		CUICommon ui(hwnd);
		
		CWaitCursor wait; // Mark busy
		
		HRESULT hr = pecb->GetObject(NULL, (IMAPIProp**)&pmsg);
		if (FAILED(hr))
		{
			ui.Message(IDS_E_NOMESSAGE);
			return E_ABORT;
		}

		if (_bEncrypt)
		{
			LPADRLIST pAddrList;

			prds = (PRECIPIENTDIALOGSTRUCT) pgpAlloc(
									sizeof(RECIPIENTDIALOGSTRUCT));
			memset(prds, '\0', sizeof(RECIPIENTDIALOGSTRUCT) );

			prds->szRecipients = (char *) pgpAlloc(1);
			prds->szRecipients[0] = 0;

			pecb->GetRecipients(&pAddrList);
			if (pAddrList)
			{
				prds->dwNumRecipients = pAddrList->cEntries;
			}
			else
			{
				prds->dwNumRecipients = 0;
			}

			if (prds->dwNumRecipients)
			{	
				ULONG ulRow;
				int nLength=1;
				ULONG ulIndex;
				ULONG ulEmailAddr;
				ULONG ulDisplayName;
				ULONG ulType;
				int nChar;
				int nRecipLength;
				char *szRecip;
				ULONG ulNumRows;
					
				ulNumRows = prds->dwNumRecipients;
				for (ulRow=0; ulRow<ulNumRows; ulRow++)
				{
					ulEmailAddr = 65535;
					ulDisplayName = 65535;
					ulType = 65535;

					for (ulIndex=0; ulIndex<pAddrList->aEntries[ulRow].cValues;
							ulIndex++)
					{
						if (pAddrList->aEntries[ulRow].rgPropVals[ulIndex]
								.ulPropTag == PR_DISPLAY_NAME)
						{
							ulDisplayName = ulIndex;
						}

						if (pAddrList->aEntries[ulRow].rgPropVals[ulIndex]
								.ulPropTag == PR_EMAIL_ADDRESS)
						{
							ulEmailAddr = ulIndex;
						}

						if (pAddrList->aEntries[ulRow].rgPropVals[ulIndex]
								.ulPropTag == PR_OBJECT_TYPE)
						{
							ulType = ulIndex;
						}
					}

					if ((ulDisplayName == 65535) && (ulEmailAddr == 65535))
					{
						prds->dwNumRecipients--;
						continue;
					}

					if (ulEmailAddr != 65535)
					{
						if (pAddrList->
							aEntries[ulRow].rgPropVals[ulEmailAddr].
							Value.lpszA)
						{
							if (strlen(pAddrList->
								aEntries[ulRow].rgPropVals[ulEmailAddr].
								Value.lpszA))
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
						prds->dwNumRecipients--;
						continue;
					}

					// Now see if the recipient has any properties
					if (!(pAddrList->aEntries[ulRow].cValues) ||
						((pAddrList->aEntries[ulRow].cValues-1) < ulIndex))
					{
						prds->dwNumRecipients--;
						continue;
					}

					szRecip = pAddrList->
						aEntries[ulRow].rgPropVals[ulIndex].Value.lpszA;

					// Make sure the recipient e-mail address is not
					// a null string
					nRecipLength = strlen(szRecip);
					if (!nRecipLength)
					{
						prds->dwNumRecipients--;
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
						prds->dwNumRecipients--;
						continue;
					}

					nLength += nRecipLength+1;
					pgpRealloc((void **) &(prds->szRecipients), nLength+1);
					strcat(prds->szRecipients, szRecip);
					strcat(prds->szRecipients, "\n");
				}
			}
			
/*
			pmsg->GetRecipientTable(0, &ptableRecip);
			HrQueryAllRows(ptableRecip, (SPropTagArray *)&tagaRecip,
							NULL, NULL, 0, &prRecip);
			prds->dwNumRecipients = prRecip->cRows;
			if (prRecip->cRows)
			{	
				ULONG ulRow;
				int nLength=1;

				prds->szRecipients = (char *) pgpAlloc(1);
				prds->szRecipients[0] = 0;

				for (ulRow=0; ulRow<prRecip->cRows; ulRow++)
				{
					nLength += strlen(prRecip->aRow[ulRow].lpProps[0]
										.Value.lpszA)+1;
					pgpRealloc((void **) &(prds->szRecipients), nLength+1);
					strcat(prds->szRecipients,
							prRecip->aRow[ulRow].lpProps[0].Value.lpszA);
					strcat(prds->szRecipients, "\n");
				}
			}
			ptableRecip->Release();
*/
		}

		pmsg->GetAttachmentTable(0, &ptableAttach);
		HrQueryAllRows(ptableAttach, (SPropTagArray *)&tagaTable,
						NULL, NULL, 0, &prAttach);
		
		IStream *pstrmBody = 0;
		hr = pmsg->OpenProperty(PR_BODY, &IID_IStream, STGM_READWRITE,
			MAPI_MODIFY, (IUnknown**)&pstrmBody);
		if (FAILED(hr))
		{
			ui.ErrorMessage(pmsg, hr, IDS_E_NOBODY);
			pmsg->Release();
			return E_ABORT;
		}
		
		pstrmBody->Stat(&StreamStats, STATFLAG_NONAME);
		dwInSize = StreamStats.cbSize.LowPart;
		
		pInput = (char *) calloc(dwInSize+1, sizeof(char));
		if (!pInput)
		{
			ui.Message(IDS_E_NOMEMORY);
			pmsg->Release();
			return E_ABORT;
		}
		pstrmBody->Read(pInput, dwInSize, &dwInSize);
		pInput[dwInSize] = 0;

		nError = EncryptSignBuffer(hwnd, pInput, dwInSize, (void *)prds,
									(VOID ** )&pOutput, &ulOutSize,
									_bEncrypt, _bSign, FALSE, FALSE);
		if (!nError)
		{
			LARGE_INTEGER li = {0,0};
			ULARGE_INTEGER uli = {ulOutSize+prAttach->cRows, 0};
			BOOL fPartied;
			
			pstrmBody->Seek(li, STREAM_SEEK_SET, NULL);
			pstrmBody->Write(pOutput, ulOutSize, NULL);
			pstrmBody->SetSize(uli);
			pstrmBody->Commit(STGC_DEFAULT);
			pstrmBody->Release();
			RTFSync(pmsg, RTF_SYNC_BODY_CHANGED, &fPartied);
			memset(pOutput, 0, ulOutSize);
			pgpFree(pOutput);
		}
		else
			pstrmBody->Release();
		
		free(pInput);

		if (prAttach->cRows && !nError)
		{
			ULONG ulRow;
			ULONG ulNum;
			SizedSPropTagArray(5, tagaAttach) = {
				5, {PR_ATTACH_METHOD, PR_RENDERING_POSITION, PR_ATTACH_FILENAME,
					PR_ATTACH_LONG_FILENAME, PR_DISPLAY_NAME}};
			enum {ivalAttachMethod=0, ivalRenderingPos, ivalFilename,
					ivalLongFilename, ivalDisplayName};
			SPropValue *pval;
			ULONG ulNumVals;
			IStream *pstrmAttach;
			STATSTG AttStreamStats;
			DWORD dwAttSize;
			ULONG ulAttOutSize;
			char *pAttachText;
			char *szFileName;
			char *szLongFileName;
			char *szExtension;
			
			for (ulRow=0; ulRow<prAttach->cRows; ulRow++)
			{
				ulNum = prAttach->aRow[ulRow].lpProps[0].Value.l;
				pmsg->OpenAttach(ulNum, NULL, MAPI_MODIFY, &pAttach);
				pAttach->GetProps((SPropTagArray *)&tagaAttach, 0, &ulNumVals,
									&pval);
				switch (pval[ivalAttachMethod].Value.l)
				{
				case ATTACH_BY_VALUE:
					{
						pAttach->OpenProperty(PR_ATTACH_DATA_BIN,
											&IID_IStream, 0, MAPI_MODIFY,
											(LPUNKNOWN *) &pstrmAttach);
						
						pstrmAttach->Stat(&AttStreamStats, STATFLAG_NONAME);
						dwAttSize = AttStreamStats.cbSize.LowPart;
						
						pAttachText = (char *) calloc(dwAttSize+1,
														sizeof(char));
						pstrmAttach->Read(pAttachText, dwAttSize, &dwAttSize);
						pAttachText[dwAttSize] = 0;
						nError = EncryptSignBuffer(hwnd, pAttachText,
										dwAttSize, (void *)prds,
										(VOID ** )&pOutput, &ulAttOutSize,
										_bEncrypt, _bSign, TRUE, FALSE);
						if (!nError)
						{
							LARGE_INTEGER li = {0,0};
							ULARGE_INTEGER uli = {ulAttOutSize, 0};
			
							pstrmAttach->Seek(li, STREAM_SEEK_SET, NULL);
							pstrmAttach->Write(pOutput, ulAttOutSize, NULL);
							pstrmAttach->SetSize(uli);
							pstrmAttach->Commit(STGC_DEFAULT);
							pstrmAttach->Release();
							memset(pOutput, 0, ulAttOutSize);
							pgpFree(pOutput);
						}
						else
							pstrmAttach->Release();

						if (pval[ivalRenderingPos].Value.l > -1)
							pval[ivalRenderingPos].Value.l = ulOutSize + ulRow;

						szFileName = (char *) calloc(MAX_PATH, sizeof(char));
						szLongFileName = (char *) calloc(MAX_PATH,
														sizeof(char));
						strcpy(szFileName, pval[ivalFilename].Value.lpszA);
						strcpy(szLongFileName,
								pval[ivalLongFilename].Value.lpszA);
						szExtension = strrchr(szFileName, '.');
						if (szExtension)
							*szExtension = '\0';

						strcat(szFileName, ".asc");
						strcpy(pval[ivalFilename].Value.lpszA, szFileName);
						strcat(szLongFileName, ".asc");
						MAPIFreeBuffer(pval[ivalLongFilename].Value.lpszA);
						MAPIAllocateBuffer(MAX_PATH,
										(void **) &(pval[ivalLongFilename]
													.Value.lpszA));
						MAPIFreeBuffer(pval[ivalDisplayName].Value.lpszA);
						MAPIAllocateBuffer(MAX_PATH,
										(void **) &(pval[ivalDisplayName]
													.Value.lpszA));
						strcpy(pval[ivalLongFilename].Value.lpszA,
								szLongFileName);
						strcpy(pval[ivalDisplayName].Value.lpszA,
								szLongFileName);
						if (!nError)
							pAttach->SetProps(5, pval, NULL);
						free(szFileName);
						free(szLongFileName);
					}
					pAttach->SaveChanges(0);
					break;
					
				case ATTACH_BY_REFERENCE:
					MessageBox(NULL, "BY_REFERENCE", "ATTACH", MB_OK);
					break;
					
				case ATTACH_BY_REF_RESOLVE:
					MessageBox(NULL, "BY_REF_RESOLVE", "ATTACH", MB_OK);
					break;
					
				case ATTACH_BY_REF_ONLY:
					MessageBox(NULL, "BY_REF_ONLY", "ATTACH", MB_OK);
					break;
					
				case ATTACH_EMBEDDED_MSG:
					MessageBox(NULL, "EMBEDDED_MSG", "ATTACH", MB_OK);
					break;
					
				case ATTACH_OLE:
					MessageBox(NULL, "OLE", "ATTACH", MB_OK);
					break;
					
				default:
					MessageBox(NULL, "Unknown", "ATTACH", MB_OK);
				}
				MAPIFreeBuffer(pval);
				if (pAttachText)
					free(pAttachText);
				pAttach->Release();

				if (nError)
					break;
			}
		}
		ptableAttach->Release();

		if (prds)
		{
			if (prds->szRecipients)
				pgpFree(prds->szRecipients);

			pgpFree(prds);
		}
		pmsg->Release();

		if (nError)
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
	assert(S_OK == hr);
	if (S_OK != hr)
		return S_FALSE; // No such menu item?  Very bad.
	hr = pmecb->GetMenuPos(EECMDID_HelpAboutMicrosoftExchange, &hmenuHelp,
							NULL, NULL, 0);
	assert(S_OK == hr);
	if (S_OK != hr)
		return S_FALSE; // No such menu item?  Very bad.
	hr = pmecb->GetMenuPos(EECMDID_HelpMicrosoftExchangeHelpTopics,
							&hmenuHelpTopics, NULL, &ulAfterExchange, 0);
	assert(S_OK == hr);
	if (S_OK != hr)
		return S_FALSE; // No such menu item?  Very bad.

	TCHAR szCommand[80];
	
	_hmenuPGP = CreatePopupMenu();
	InsertMenu(hmenuTools, ulBeforeTools,
				MF_BYPOSITION | MF_STRING | MF_POPUP,
				(UINT) _hmenuPGP, "&PGP");

	if (_context == EECONTEXT_SENDNOTEMESSAGE)
	{
		GetString(szCommand, sizeof(szCommand)/sizeof(TCHAR), IDS_ENCRYPT_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidEncrypt = *pcmdidBase;
 		++(*pcmdidBase);

		GetString(szCommand, sizeof(szCommand)/sizeof(TCHAR), IDS_SIGN_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidSign = *pcmdidBase;
 		++(*pcmdidBase);

		AppendMenu(_hmenuPGP, MF_SEPARATOR, 0, NULL);

		FormatString(szCommand, sizeof(szCommand)/sizeof(TCHAR),
						IDS_ENCRYPTNOW_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidEncryptNow = *pcmdidBase;
 		++(*pcmdidBase);

		FormatString(szCommand, sizeof(szCommand)/sizeof(TCHAR),
					IDS_SIGNNOW_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidSignNow = *pcmdidBase;
 		++(*pcmdidBase);

		FormatString(szCommand, sizeof(szCommand)/sizeof(TCHAR),
						IDS_ENCRYPTSIGN_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidEncryptSign = *pcmdidBase;
 		++(*pcmdidBase);

		AppendMenu(_hmenuPGP, MF_SEPARATOR, 0, NULL);
	}

	if (_context == EECONTEXT_READNOTEMESSAGE)
	{
		GetString(szCommand, sizeof(szCommand)/sizeof(TCHAR), IDS_DECRYPT_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidDecrypt = *pcmdidBase;
 		++(*pcmdidBase);

		GetString(szCommand, sizeof(szCommand)/sizeof(TCHAR), IDS_ADDKEY_MENU);
		AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
		_cmdidAddKey = *pcmdidBase;
 		++(*pcmdidBase);

		AppendMenu(_hmenuPGP, MF_SEPARATOR, 0, NULL);
	}

	GetString(szCommand, sizeof(szCommand)/sizeof(TCHAR), IDS_PGPKEYS_MENU);
	AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
	_cmdidPgpKeys = *pcmdidBase;
	++(*pcmdidBase);

	GetString(szCommand, sizeof(szCommand)/sizeof(TCHAR), IDS_PGPPREFS_MENU);
	AppendMenu(_hmenuPGP, MF_STRING, *pcmdidBase, szCommand);
	_cmdidPrefs = *pcmdidBase;
	++(*pcmdidBase);
	
	GetString(szCommand, sizeof(szCommand)/sizeof(TCHAR), IDS_PGPHELP_MENU);
	InsertMenu(hmenuHelpTopics, ulAfterExchange,
				MF_BYPOSITION | MF_STRING, *pcmdidBase, szCommand);
	_cmdidHelp = *pcmdidBase;
	++(*pcmdidBase);
	
	GetString(szCommand, sizeof(szCommand)/sizeof(TCHAR), IDS_ABOUT_MENU);
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
//				_itbbEncryptSign = lptbeArray[tbindx].itbbBase++;
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

		tbab.hInst = CUICommon::GetHinst();
		if (_context == EECONTEXT_SENDNOTEMESSAGE)
		{
			tbab.nID = IDB_ENCRYPT;
			_itbmEncrypt = SendMessage(hwndToolbar, TB_ADDBITMAP, 1,
										(LPARAM)&tbab);

			tbab.nID = IDB_SIGN;
			_itbmSign = SendMessage(hwndToolbar, TB_ADDBITMAP, 1,
									(LPARAM)&tbab);

/*
			tbab.nID = IDB_ENCRYPTSIGN;
			_itbmEncryptSign = SendMessage(hwndToolbar, TB_ADDBITMAP, 1,
											(LPARAM)&tbab);
*/
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
	TCHAR szState[60];

	if ((itbb == _itbbEncrypt) && (_context == EECONTEXT_SENDNOTEMESSAGE))
	{
		ptbb->iBitmap = _itbmEncrypt;
		ptbb->idCommand = _cmdidEncrypt;
		ptbb->fsState = TBSTATE_ENABLED;
		ptbb->fsStyle = TBSTYLE_CHECK;
		ptbb->dwData = 0;
		ptbb->iString = -1;

//		if (_bEncrypt)
			FormatString(lpsz, cch, IDS_ENCRYPT_TOOLTIP, "Encrypt");
//		else
//			FormatString(lpsz, cch, IDS_ENCRYPT_TOOLTIP, "Do not encrypt");

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

//		if (_bSign)
			FormatString(lpsz, cch, IDS_SIGN_TOOLTIP, "Sign");
//		else
//			FormatString(lpsz, cch, IDS_SIGN_TOOLTIP, "Do not sign");

		hr = S_OK;
	}

/*
	if ((itbb == _itbbEncryptSign) && (_context == EECONTEXT_SENDNOTEMESSAGE))
	{
		ptbb->iBitmap = _itbmEncryptSign;
		ptbb->idCommand = _cmdidEncryptSign;
		ptbb->fsStyle = TBSTYLE_BUTTON;
		ptbb->fsState = TBSTATE_ENABLED;
		ptbb->dwData = 0;
		ptbb->iString = -1;
		FormatString(lpsz, cch, IDS_ENCRYPTSIGN_TOOLTIP);
		hr = S_OK;
	}
*/

	if (itbb == _itbbPgpKeys)
	{
		ptbb->iBitmap = _itbmPgpKeys;
		ptbb->idCommand = _cmdidPgpKeys;
		ptbb->fsState = TBSTATE_ENABLED;
		ptbb->fsStyle = TBSTYLE_BUTTON;
		ptbb->dwData = 0;
		ptbb->iString = -1;
		FormatString(lpsz, cch, IDS_PGPKEYS_TOOLTIP);
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
		FormatString(lpsz, cch, IDS_DECRYPT_TOOLTIP, szState);
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
		FormatString(lpsz, cch, IDS_ADDKEY_TOOLTIP, szState);
		hr = S_OK;
	}

	return hr;
}


STDMETHODIMP CExtImpl::ResetToolbar(ULONG tbid, ULONG ulFlags)
{
	// To implement this method,
	// the extension must cache the results of a prior call
	// to IExchExtCallback::GetToolbar.

	if ((EECONTEXT_READNOTEMESSAGE != _context) &&
		(EECONTEXT_SENDNOTEMESSAGE != _context))
		return S_FALSE;

	if (EETBID_STANDARD != tbid)
		return S_FALSE;

	return S_FALSE;

/*
	TBBUTTON tbbNew[5];

	tbbNew[0].iBitmap = 0;
	tbbNew[0].idCommand = 0;
	tbbNew[0].fsState = 0;
	tbbNew[0].fsStyle = TBSTYLE_SEP;
	tbbNew[0].dwData = 0;
	tbbNew[0].iString = -1;

	if (_context == EECONTEXT_SENDNOTEMESSAGE)
	{
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
		
//		tbbNew[3].iBitmap = _itbmEncryptSign;
//		tbbNew[3].idCommand = _cmdidEncryptSign;
//		tbbNew[3].fsState = TBSTATE_ENABLED;
//		tbbNew[3].fsStyle = TBSTYLE_BUTTON;
//		tbbNew[3].dwData = 0;
//		tbbNew[3].iString = -1;
			
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
		
		SendMessage(_hwndSendToolbar, TB_ADDBUTTONS, 5, (LPARAM) tbbNew);

	}

	if (_context == EECONTEXT_READNOTEMESSAGE)
	{
		tbbNew[1].iBitmap = _itbmDecrypt;
		tbbNew[1].idCommand = _cmdidDecrypt;
		tbbNew[1].fsState = TBSTATE_ENABLED;
		tbbNew[1].fsStyle = TBSTYLE_BUTTON;
		tbbNew[1].dwData = 0;
		tbbNew[1].iString = -1;

			
		tbbNew[2].iBitmap = _itbmAddKey;
		tbbNew[2].idCommand = _cmdidAddKey;
		tbbNew[2].fsState = TBSTATE_ENABLED;
		tbbNew[2].fsStyle = TBSTYLE_BUTTON;
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

		SendMessage(_hwndReadToolbar, TB_ADDBUTTONS, 5, (LPARAM) tbbNew);
	}

 	return S_OK;
*/
}


STDMETHODIMP CExtImpl::QueryHelpText(UINT cmdid, ULONG ulFlags,
                                           LPTSTR psz, UINT cch)
{
	if ((EECONTEXT_READNOTEMESSAGE != _context) &&
		(EECONTEXT_SENDNOTEMESSAGE != _context) &&
		(EECONTEXT_VIEWER != _context))
		return S_FALSE;

	TCHAR szState[60];
//	GetString(szStatus, sizeof(szStatus)/sizeof(TCHAR),
//		_fUseSig ? IDS_ENABLED : IDS_DISABLED);

	if (ulFlags == EECQHT_STATUS)
	{
		if ((cmdid == _cmdidEncrypt) && (_context == EECONTEXT_SENDNOTEMESSAGE))
		{
//			if (!_bEncrypt)
				wsprintf(szState, "Encrypt");
//			else
//				wsprintf(szState, "Do not encrypt");

			FormatString(psz, cch, IDS_ENCRYPT_STATUS, szState);
			return S_OK;
		}

		if ((cmdid == _cmdidSign) && (_context == EECONTEXT_SENDNOTEMESSAGE))
		{
//			if (!_bSign)
				wsprintf(szState, "Sign");
//			else
//				wsprintf(szState, "Do not sign");

			FormatString(psz, cch, IDS_SIGN_STATUS, szState);
			return S_OK;
		}

		if ((cmdid == _cmdidEncryptNow) &&
				(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_ENCRYPTNOW_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidSignNow) && (_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_SIGNNOW_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidEncryptSign) &&
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_ENCRYPTSIGN_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidDecrypt) && (_context == EECONTEXT_READNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_DECRYPT_STATUS);
			return S_OK;
		}

		if ((cmdid == _cmdidAddKey) && (_context == EECONTEXT_READNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_ADDKEY_STATUS);
			return S_OK;
		}

		if (cmdid == _cmdidPgpKeys)
		{
			FormatString(psz, cch, IDS_PGPKEYS_STATUS);
			return S_OK;
		}

		if (cmdid == _cmdidPrefs)
		{
			FormatString(psz, cch, IDS_PGPPREFS_STATUS);
			return S_OK;
		}

		if (cmdid == _cmdidHelp)
		{
			FormatString(psz, cch, IDS_PGPHELP_STATUS);
			return S_OK;
		}

		if (cmdid == _cmdidAbout)
		{
			FormatString(psz, cch, IDS_ABOUT_STATUS);
			return S_OK;
		}

	}
	else if (ulFlags == EECQHT_TOOLTIP)
	{
		if ((cmdid == _cmdidEncrypt) && (_context == EECONTEXT_SENDNOTEMESSAGE))
		{
//			if (_bEncrypt)
				wsprintf(szState, "Encrypt");
//			else
//				wsprintf(szState, "Do not encrypt");

			FormatString(psz, cch, IDS_ENCRYPT_TOOLTIP, szState);
			return S_OK;
		}

		if ((cmdid == _cmdidSign) && (_context == EECONTEXT_SENDNOTEMESSAGE))
		{
//			if (_bSign)
				wsprintf(szState, "Sign");
//			else
//				wsprintf(szState, "Do not sign");

			FormatString(psz, cch, IDS_SIGN_TOOLTIP, szState);
			return S_OK;
		}

		if ((cmdid == _cmdidEncryptNow) &&
				(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_ENCRYPTNOW_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidSignNow) && (_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_SIGNNOW_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidEncryptSign) &&
			(_context == EECONTEXT_SENDNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_ENCRYPTSIGN_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidDecrypt) && (_context == EECONTEXT_READNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_DECRYPT_TOOLTIP);
			return S_OK;
		}

		if ((cmdid == _cmdidAddKey) && (_context == EECONTEXT_READNOTEMESSAGE))
		{
			FormatString(psz, cch, IDS_ADDKEY_TOOLTIP);
			return S_OK;
		}

		if (cmdid == _cmdidPgpKeys)
		{
			FormatString(psz, cch, IDS_PGPKEYS_TOOLTIP);
			return S_OK;
		}
	}

	return S_FALSE;
}


STDMETHODIMP CExtImpl::Help(IExchExtCallback* pmecb, UINT cmdid)
{
	if ((EECONTEXT_READNOTEMESSAGE != _context) &&
		(EECONTEXT_SENDNOTEMESSAGE != _context) &&
		(EECONTEXT_VIEWER != _context))
		return S_FALSE;

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
			SendMessage(hwndToolbar, TB_CHECKBUTTON, _cmdidSign,
						MAKELONG(TRUE, 0));
		}
		else
		{
			miiSign.fState = MFS_UNCHECKED;
			miiSign.hbmpUnchecked = NULL;
			SendMessage(hwndToolbar, TB_CHECKBUTTON, _cmdidSign,
						MAKELONG(FALSE, 0));
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
		
		bEncrypt = (cmdid == _cmdidEncryptNow) || (cmdid == _cmdidEncryptSign);
		bSign = (cmdid == _cmdidSignNow) || (cmdid == _cmdidEncryptSign);

		if (_hwndMessage)
		{
			char *szInput;
			char *szOutput = NULL;
			long lLength;
			ULONG ulOutLength;
			BOOL bSelectedText = FALSE;
			CHARRANGE chRange = {0,0};
			PRECIPIENTDIALOGSTRUCT prds = NULL;
			int nError;

			if (bEncrypt)
			{
				LPADRLIST pAddrList;

				prds = (PRECIPIENTDIALOGSTRUCT) pgpAlloc(
									sizeof(RECIPIENTDIALOGSTRUCT));
				memset(prds, '\0', sizeof(RECIPIENTDIALOGSTRUCT) );

				prds->szRecipients = (char *) pgpAlloc(1);
				prds->szRecipients[0] = 0;

				pmecb->GetRecipients(&pAddrList);
				if (pAddrList)
				{
					prds->dwNumRecipients = pAddrList->cEntries;
				}
				else
				{
					prds->dwNumRecipients = 0;
				}

				if (prds->dwNumRecipients)
				{	
					ULONG ulRow;
					int nLength=1;
					ULONG ulIndex;
					ULONG ulEmailAddr;
					ULONG ulDisplayName;
					ULONG ulType;
					ULONG ulEntryID;
					int nChar;
					int nRecipLength;
					char *szRecip;
					ULONG ulNumRows;
					
					ulNumRows = prds->dwNumRecipients;
					for (ulRow=0; ulRow<ulNumRows; ulRow++)
					{
						ulEmailAddr = 65535;
						ulDisplayName = 65535;
						ulType = 65535;

						for (ulIndex=0;
								ulIndex<pAddrList->aEntries[ulRow].cValues;
								ulIndex++)
						{

							if (pAddrList->aEntries[ulRow].rgPropVals[ulIndex]
									.ulPropTag == PR_DISPLAY_NAME)
							{
								ulDisplayName = ulIndex;
							}
							if (pAddrList->aEntries[ulRow].rgPropVals[ulIndex]
									.ulPropTag == PR_EMAIL_ADDRESS)
							{
								ulEmailAddr = ulIndex;
							}
							if (pAddrList->aEntries[ulRow].rgPropVals[ulIndex]
									.ulPropTag == PR_OBJECT_TYPE)
							{
								ulType = ulIndex;
							}
							if (pAddrList->aEntries[ulRow].rgPropVals[ulIndex]
									.ulPropTag == PR_ENTRYID)
							{
								ulEntryID = ulIndex;
							}
						}

						if ((ulDisplayName == 65535) && (ulEmailAddr == 65535))
						{
							prds->dwNumRecipients--;
							continue;
						}

						if (ulEmailAddr != 65535)
						{
							if (pAddrList->
								aEntries[ulRow].rgPropVals[ulEmailAddr].
								Value.lpszA)
							{
								if (strlen(pAddrList->
									aEntries[ulRow].rgPropVals[ulEmailAddr].
									Value.lpszA))
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
							prds->dwNumRecipients--;
							continue;
						}

						// Now see if the recipient has any properties
						if (!(pAddrList->aEntries[ulRow].cValues) ||
							((pAddrList->aEntries[ulRow].cValues-1) < ulIndex))
						{
							prds->dwNumRecipients--;
							continue;
						}

						szRecip = pAddrList->
							aEntries[ulRow].rgPropVals[ulIndex].Value.lpszA;

						// Make sure the recipient e-mail address is not
						// a null string
						nRecipLength = strlen(szRecip);
						if (!nRecipLength)
						{
							prds->dwNumRecipients--;
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
							prds->dwNumRecipients--;
							continue;
						}

						nLength += nRecipLength+1;
						pgpRealloc((void **) &(prds->szRecipients), nLength+1);
						strcat(prds->szRecipients, szRecip);
						strcat(prds->szRecipients, "\n");
					}
				}
			}

			// Determine if the user selected a particular piece of text
			SendMessage(_hwndMessage,  EM_EXGETSEL, (WPARAM)0,
						(LPARAM) &chRange);
			bSelectedText = chRange.cpMax - chRange.cpMin;
			
/*
			// If not, select all the text
			if(!bSelectedText)
			{
				RECT rect;
				
				GetClientRect(_hwndMessage, &rect);
				SendMessage(_hwndMessage, WM_LBUTTONDOWN, MK_LBUTTON,
							MAKELPARAM(rect.right - 5, rect.bottom -5 ));
				SendMessage(_hwndMessage, WM_LBUTTONUP, MK_LBUTTON,
							MAKELPARAM(rect.right - 5 , rect.bottom -5 ));
			}
*/
			
			szInput = GetRichEditContents(_hwndMessage, &lLength,
											FALSE, bSelectedText);
			if (!szInput)
			{
				char szMessage[1024];
				char szTitle[255];

				GetString(szMessage, sizeof(szMessage), IDS_E_GETTEXT);
				GetString(szTitle, sizeof(szTitle), IDS_TITLE);
				MessageBox(hwndMain, szMessage, szTitle, MB_ICONEXCLAMATION);
				return S_OK;
			}

			szInput[lLength-1] = 0;
			lLength = strlen(szInput);
			nError = EncryptSignBuffer(hwndMain, szInput, lLength, (void *)prds,
										(void **) &szOutput, &ulOutLength,
										bEncrypt, bSign, FALSE, FALSE);
			if (!nError)
			{
				SetRichEditContents(_hwndMessage, szOutput, FALSE,
									bSelectedText);
				pgpFree(szOutput);
				_bEncrypt = _bEncrypt && !bEncrypt;
				_bSign = _bSign && !bSign;
				InitMenu(pmecb);
			}

			memset(szInput, 0, lLength);
			HeapFree(GetProcessHeap (), 0, szInput);

			if (prds)
			{
				if (prds->szRecipients)
					pgpFree(prds->szRecipients);

				pgpFree(prds);
			}
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
			ULONG ulOutLength;
			ULONG ulBlockStart;
			ULONG ulBlockLength;
			BOOL bSelectedText = FALSE;
			CHARRANGE chRange = {0,0};
			int nContents;

			// Determine if the user selected a particular piece of text
			SendMessage(_hwndMessage,  EM_EXGETSEL, (WPARAM)0,
						(LPARAM) &chRange);
			bSelectedText = chRange.cpMax - chRange.cpMin;
			
/*
			// If not, select all the text
			if(!bSelectedText)
			{
				RECT rect;
				
				GetClientRect(_hwndMessage, &rect);
				SendMessage(_hwndMessage, WM_LBUTTONDOWN, MK_LBUTTON,
							MAKELPARAM(rect.right - 5, rect.bottom -5 ));
				SendMessage(_hwndMessage, WM_LBUTTONUP, MK_LBUTTON,
							MAKELPARAM(rect.right - 5 , rect.bottom -5 ));
			}
*/
			szInput = GetRichEditContents(_hwndMessage, &lLength,
											FALSE, bSelectedText);
			if (!szInput)
			{
				char szMessage[1024];
				char szTitle[255];

				GetString(szMessage, sizeof(szMessage), IDS_E_GETTEXT);
				GetString(szTitle, sizeof(szTitle), IDS_TITLE);
				MessageBox(hwndMain, szMessage, szTitle, MB_ICONEXCLAMATION);
				return S_OK;
			}

			szInput[lLength-1] = 0;
			lLength = strlen(szInput);
			
			// Find the PGP block within the selected block and select it only

			if (FindEncryptedBlock(szInput, lLength, &ulBlockStart,
									&ulBlockLength))
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
					
					nNumCR = 1;
					while (szCR && (szCR <
										(szInput+ulBlockStart+ulBlockLength)))
					{
						nNumCR++;
						szCR = strchr(szCR+1, '\r');
					}
					ulBlockLength -= nNumCR;
				}

				chRange.cpMin = ulBlockStart;
				chRange.cpMax = ulBlockStart+ulBlockLength;
				SendMessage(_hwndMessage, EM_EXSETSEL, (WPARAM)0,
							(LPARAM) &chRange);
			}
			else if (FindSignedBlock(szInput, lLength, &ulBlockStart,
									&ulBlockLength))
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
					
					nNumCR = 1;
					while (szCR && (szCR <
										(szInput+ulBlockStart+ulBlockLength)))
					{
						nNumCR++;
						szCR = strchr(szCR+1, '\r');
					}
					ulBlockLength -= nNumCR;
				}

				chRange.cpMin = ulBlockStart;
				chRange.cpMax = ulBlockStart+ulBlockLength;
				SendMessage(_hwndMessage, EM_EXSETSEL, (WPARAM)0,
							(LPARAM) &chRange);
			}
			else
			{
				chRange.cpMin = 0;
				chRange.cpMax = 0;
				SendMessage(_hwndMessage, EM_EXSETSEL, (WPARAM)0,
							(LPARAM) &chRange);
			}

			nContents = SimplePGPAnalyzeBuffer(szInput, lLength);
			if (!DecryptVerifyBuffer(hwndMain, szInput, lLength, NULL, 0,
									(void **) &szOutput, &ulOutLength))
			{
//				SetRichEditContents(_hwndMessage, szOutput, FALSE, !_fOutlook);
				if (nContents == SIMPLEANALYZE_ENCR)
					SetRichEditContents(_hwndMessage, szOutput, FALSE, TRUE);
				memset(szOutput, 0, ulOutLength);
				pgpFree(szOutput);
			}
			
			chRange.cpMin = 0;
			chRange.cpMax = 0;
			SendMessage(_hwndMessage, EM_EXSETSEL, (WPARAM)0,
						(LPARAM) &chRange);
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

			// Determine if the user selected a particular piece of text
			SendMessage(_hwndMessage,  EM_EXGETSEL, (WPARAM)0,
						(LPARAM) &chRange);
			bSelectedText = chRange.cpMax - chRange.cpMin;
			
/*
			// If not, select all the text
			if(!bSelectedText)
			{
				RECT rect;
				
				GetClientRect(_hwndMessage, &rect);
				SendMessage(_hwndMessage, WM_LBUTTONDOWN, MK_LBUTTON,
							MAKELPARAM(rect.right - 5, rect.bottom -5 ));
				SendMessage(_hwndMessage, WM_LBUTTONUP, MK_LBUTTON,
							MAKELPARAM(rect.right - 5 , rect.bottom -5 ));
			}
*/
			
			szInput = GetRichEditContents(_hwndMessage, &lLength,
											FALSE, bSelectedText);
			if (!szInput)
			{
				char szMessage[1024];
				char szTitle[255];

				GetString(szMessage, sizeof(szMessage), IDS_E_GETTEXT);
				GetString(szTitle, sizeof(szTitle), IDS_TITLE);
				MessageBox(hwndMain, szMessage, szTitle, MB_ICONEXCLAMATION);
				return S_OK;
			}

			szInput[lLength-1] = 0;
			AddKeyBuffer(hwndMain, szInput, lLength);
			HeapFree(GetProcessHeap (), 0, szInput);
		}

		return S_OK;
	}

	if (cmdid == _cmdidPgpKeys)
	{
		HKEY hkey;
		char szKey[] = "Software\\Microsoft\\Windows\\"
						"CurrentVersion\\App Paths\\PGPKeys.exe";
		
		// where is PGPkeys?
		if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0,
											KEY_ALL_ACCESS,&hkey))
		{
			DWORD Size,Type;
			char szPath[MAX_PATH];
			
			Size = sizeof(szPath);
			if(RegQueryValueEx(hkey, NULL,NULL, &Type,(BYTE*)szPath, &Size)
				== ERROR_SUCCESS)
			{
				// '/s' keeps it from showing that damn splash screen
				strcat(szPath, " /s");
				// run it...
				WinExec(szPath, SW_SHOW);
			}
			else
			{
				MessageBox(hwndMain,
							"Unable to locate the PGPkeys Application",
							0, MB_OK);
			}
		}

		return S_OK;
	}

	if (cmdid == _cmdidPrefs)
	{
		PGPcomdlgPreferences(hwndMain, 0);
		return S_OK;
	}

	if (cmdid == _cmdidHelp)
	{
		CHAR szHelpFile[MAX_PATH] = {0x00};
		HKEY hKey;
		LONG lResult;
		CHAR* p;
		DWORD dwValueType, dwSize;
		
		lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			"Software\\Microsoft\\Windows\\CurrentVersion\\"
				"App Paths\\PGPKeys.exe",
			0,
			KEY_READ,
			&hKey);
		
		if (lResult == ERROR_SUCCESS)
		{
			dwSize = MAX_PATH;
			lResult = RegQueryValueEx (hKey, NULL, NULL, &dwValueType,
										(LPBYTE)szHelpFile, &dwSize);
			RegCloseKey (hKey);
			p = strrchr (szHelpFile, '\\');
			if (p)
			{
				p++;
				*p = '\0';
				strcat(szHelpFile, "PGP50.hlp");
			}
			else
			{	
				lstrcpy (szHelpFile, "");
			}
		}
		
		WinHelp(hwndMain, szHelpFile, HELP_FINDER, 0);
		return S_OK;
	}

	if (cmdid == _cmdidAbout)
	{
//		TCHAR szTitle[100];
//		TCHAR szVersion[50];
//		TCHAR szVerString[200];

//		GetString(szTitle, sizeof(szTitle)/sizeof(TCHAR), IDS_TITLE);
//		GetString(szVersion, sizeof(szVersion)/sizeof(TCHAR), IDS_VERSION);
//		wsprintf(szVerString, "%s %s", szTitle, szVersion);
//		strcpy(szVerString, PGPVERSIONSTRING);
		PGPcomdlgHelpAbout(hwndMain, NULL, NULL, NULL);

		return S_OK;
	}

	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnReadPattFromSzFile(IAttach* lpAtt,
											LPTSTR lpszFile, ULONG ulFlags)
{
/*
	char *szNewFile;
	char *szShortFileName;
	char *szExtension;
	char szDrive[_MAX_DRIVE];
	char szDir[_MAX_DIR];
	char szFileName[_MAX_FNAME];
	PRECIPIENTDIALOGSTRUCT prds;
	SizedSPropTagArray(5, tagaAttach) = {
		5, {PR_ATTACH_METHOD, PR_RENDERING_POSITION, PR_ATTACH_FILENAME,
			PR_ATTACH_LONG_FILENAME, PR_ATTACH_EXTENSION}};
	SPropValue *pval;
	ULONG ulNumVals;
	IStream *pstrmAttach;
	HANDLE hFile;
	char *szAttachText;
	DWORD dwAttSize;
	int nError;

	if (_context != EECONTEXT_SENDNOTEMESSAGE)
		return S_FALSE;
	if (!_bEncrypt && !_bSign)
		return S_FALSE;

	szNewFile = (char *) calloc(MAX_PATH, sizeof(char));
	strcpy(szNewFile, lpszFile);
	szExtension = strrchr(szNewFile, '.');
	if (szExtension)
		strcpy(szExtension, ".asc");
	else
		strcat(szNewFile, ".asc");

	prds = (PRECIPIENTDIALOGSTRUCT) pgpAlloc(sizeof(RECIPIENTDIALOGSTRUCT));
	memset(prds, '\0', sizeof(RECIPIENTDIALOGSTRUCT) );

	if (EncryptSignFile(_hwndMessage, lpszFile, prds, &szNewFile,
		_bEncrypt, _bSign, TRUE, FALSE))
	{
		pgpFree(prds);
		free(szNewFile);
		return S_FALSE;
	}
	
	MessageBox(_hwndMessage, "About to get props", "plugin", MB_OK);
	lpAtt->GetProps((SPropTagArray *)&tagaAttach, 0, &ulNumVals, &pval);

	switch (pval[0].Value.l)
	{
	case ATTACH_BY_VALUE:
		{
			MessageBox(_hwndMessage, "About to open stream", "plugin", MB_OK);
			lpAtt->OpenProperty(PR_ATTACH_DATA_BIN, &IID_IStream, 0,
				MAPI_MODIFY, (LPUNKNOWN *) &pstrmAttach);
			
			hFile = CreateFile(szNewFile, GENERIC_READ, 0, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL);
			dwAttSize = GetFileSize(hFile, NULL);

			szAttachText = (char *) calloc(dwAttSize+1, sizeof(char));
			if (ReadFile(hFile, szAttachText, dwAttSize, &dwAttSize, NULL))
			{
				LARGE_INTEGER li = {0,0};
				ULARGE_INTEGER uli = {dwAttSize, 0};
				
				MessageBox(_hwndMessage, "About to write stream", "plugin",
							MB_OK);
				pstrmAttach->Seek(li, STREAM_SEEK_SET, NULL);
				pstrmAttach->Write(szAttachText, dwAttSize, NULL);
				pstrmAttach->SetSize(uli);
				pstrmAttach->Commit(STGC_DEFAULT);
			}
			pstrmAttach->Release();
			CloseHandle(hFile);
		}
		break;
					
	case ATTACH_BY_REFERENCE:
		MessageBox(NULL, "BY_REFERENCE", "ATTACH", MB_OK);
		break;
					
	case ATTACH_BY_REF_RESOLVE:
		MessageBox(NULL, "BY_REF_RESOLVE", "ATTACH", MB_OK);
		break;
					
	case ATTACH_BY_REF_ONLY:
		MessageBox(NULL, "BY_REF_ONLY", "ATTACH", MB_OK);
		break;
					
	case ATTACH_EMBEDDED_MSG:
		MessageBox(NULL, "EMBEDDED_MSG", "ATTACH", MB_OK);
		break;
					
	case ATTACH_OLE:
		MessageBox(NULL, "OLE", "ATTACH", MB_OK);
		break;
					
	default:
		MessageBox(NULL, "Unknown", "ATTACH", MB_OK);
	}

	szExtension = (char *) calloc(_MAX_EXT, sizeof(char));
	szShortFileName = (char *) calloc(_MAX_FNAME + _MAX_EXT, sizeof(char));

	_splitpath(szNewFile, szDrive, szDir, szFileName, szExtension);
	wsprintf(szShortFileName, "%s.%s", szFileName, szExtension);
	pval[0].Value.l = ATTACH_BY_REFERENCE;
	pval[2].Value.lpszA = szShortFileName;
	pval[3].Value.lpszA = szNewFile;
	pval[4].Value.lpszA = szExtension;
	
	MessageBox(_hwndMessage, "About to set props", "plugin", MB_OK);
	lpAtt->SetProps(5, pval, NULL);
	lpAtt->SaveChanges(0);
	
	pgpFree(prds);
	MessageBox(_hwndMessage, "Returning...", "plugin", MB_OK);
	return S_OK;
*/
	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnWritePattToSzFile(IAttach* lpAtt,
										   LPTSTR lpszFile, ULONG ulFlags)
{
	return S_FALSE;
}


STDMETHODIMP CExtImpl::QueryDisallowOpenPatt(IAttach* lpAtt)
{
	MessageBox(NULL, "QueryDisallowOpenPatt", "PGP Plugin", MB_OK);
	return S_OK;
}


STDMETHODIMP CExtImpl::OnOpenPatt(IAttach* lpAtt)
{
	SPropTagArray sProps;
	LPSPropValue psPropVal;
	ULONG lCount;
	long lAttachMethod;
	HRESULT hr;

	sProps.cValues = 1;
	sProps.aulPropTag[0] = PR_ATTACH_METHOD;

	hr = lpAtt->GetProps(&sProps, 0, &lCount, &psPropVal);
	if (hr != S_OK)
	{
		MessageBox(NULL, "GetProps failed", "OnOpenPatt", MB_OK);
		return S_FALSE;
	}

	lAttachMethod = psPropVal->Value.l;

	switch (lAttachMethod)
	{
	case ATTACH_BY_VALUE:
		MessageBox(NULL, "BY_VALUE", "ATTACH", MB_OK);
		break;

	case ATTACH_BY_REFERENCE:
		MessageBox(NULL, "BY_REFERENCE", "ATTACH", MB_OK);
		break;

	case ATTACH_BY_REF_RESOLVE:
		MessageBox(NULL, "BY_REF_RESOLVE", "ATTACH", MB_OK);
		break;

	case ATTACH_BY_REF_ONLY:
		MessageBox(NULL, "BY_REF_ONLY", "ATTACH", MB_OK);
		break;

	case ATTACH_EMBEDDED_MSG:
		MessageBox(NULL, "EMBEDDED_MSG", "ATTACH", MB_OK);
		break;

	case ATTACH_OLE:
		MessageBox(NULL, "OLE", "ATTACH", MB_OK);
		break;

	default:
		MessageBox(NULL, "Unknown", "ATTACH", MB_OK);
	}

	MAPIFreeBuffer(psPropVal);
	return S_FALSE;
}


STDMETHODIMP CExtImpl::OnOpenSzFile(LPTSTR lpszFile, ULONG ulFlags)
{
//	char *szOutFile;
	char szTitle[100];
	char szSigMsg[1024];
	int nError;
	int nMsgType;

	if (_context != EECONTEXT_READNOTEMESSAGE)
		return S_FALSE;

	if (ulFlags != EEAFE_OPEN)
		return S_FALSE;

	GetString(szTitle, sizeof(szTitle), IDS_TITLE);
	GetString(szSigMsg, sizeof(szSigMsg), IDS_DETACHEDSIG);

	nMsgType = SimplePGPAnalyzeFile(lpszFile);
	switch (nMsgType)
	{
	case SIMPLEANALYZE_UNKNOWN:
		if (!_nAttachNum)
			return S_FALSE;
		nError = DecryptVerifyFile(_hwndMessage, lpszFile, _szSig, &lpszFile);
		free(_szSig);
		return S_FALSE;

	case SIMPLEANALYZE_ENCR:
	case SIMPLEANALYZE_SIGN:
/*
		nError = DecryptVerifyFile(_hwndMessage, lpszFile, NULL, &szOutFile);
		if (!nError)
			pgpFree(szOutFile);
*/
		if (_nAttachNum)
		{
			free(_szSig);
			_nAttachNum = 0;
		}
		return S_FALSE;

	case SIMPLEANALYZE_KEY:
		if (_nAttachNum)
		{
			free(_szSig);
			_nAttachNum = 0;
		}
		return S_FALSE;

	case SIMPLEANALYZE_IOERROR:
		return S_FALSE;

	case SIMPLEANALYZE_DETACHEDSIG:
		if (!_nAttachNum)
		{
			MessageBox(_hwndMessage, szSigMsg, szTitle, MB_OK);
			_nAttachNum++;
			_szSig = (char *) calloc(strlen(lpszFile)+1, sizeof(char));
			strcpy(_szSig, lpszFile);
			return S_OK;
		}
		else
		{
			_nAttachNum = 0;
			free(_szSig);
			return S_OK;
		}

	default:
		return S_FALSE;
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

/*	
	hwndSearch = FindWindowEx(hwndMain, NULL, "#32770", NULL);
	if (hwndSearch)
	{
		int yMax = 0;
		
		const HWND hwndDialog = hwndSearch;
		while ((hwndSearch = FindWindowEx(hwndDialog, hwndLast,
			"RICHEDIT", NULL)) != NULL)
		{
			RECT rc;
			::GetClientRect(hwndSearch, &rc);
			
			if (rc.bottom > yMax)
			{
				yMax = rc.bottom;
				hwndFound = hwndSearch;
			}
			hwndLast = hwndSearch;
		}
		
		if (!hwndFound)
		{
			hwndLast = NULL;
			
			MessageBox(hwndMain, "Looking for RichEdit20A", "plugin", MB_OK);
			while ((hwndSearch = FindWindowEx(hwndDialog, hwndLast,
				"RichEdit20A", NULL)) != NULL)
			{
				RECT rc;
				::GetClientRect(hwndSearch, &rc);
				
				if (rc.bottom > yMax)
				{
					yMax = rc.bottom;
					hwndFound = hwndSearch;
				}
				hwndLast = hwndSearch;
			}
		}
		
		if (!hwndFound)
			MessageBox(hwndMain, "Failed to find rich edit control", "plugin",
						MB_ICONEXCLAMATION);
	}
	else
		MessageBox(hwndMain, "Failed to find dialog box", "plugin",
					MB_ICONEXCLAMATION);
*/

	return hwndFound;
}


BOOL CALLBACK ReportChildren(HWND hwnd, LPARAM lParam)
{
	char szClassName[200];
	FindStruct *pfsRichEdit;
	RECT rc;

	pfsRichEdit = (FindStruct *) lParam;
	GetClassName(hwnd, szClassName, 199);
	if (!strcmp(szClassName, "RICHEDIT") || !strcmp(szClassName, "RichEdit20A"))
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


BOOL IsExchExtWithinOutlook(IExchExtCallback* peecb)
{
	IOutlookGetObjectForExchExtCallback* po = NULL;

	HRESULT hr = peecb->QueryInterface(IID_IOutlookGetObjectForExchExtCallback,
										(void**)&po);
	if (po)
		po->Release();

	return (SUCCEEDED(hr));
}


void HexDump(HWND hwnd, char *szMessage, int nLength)
{
	int nX;
	int nIndex;
	char *szText;

	szText = (char *) calloc(nLength * 3, sizeof(char));
	nIndex = 0;

	for (nX=0; nX<nLength; nX++)
	{
		wsprintf(&(szText[nIndex]), "%02X ", szMessage[nX]);
		nIndex += 3;
	}

	MessageBox(hwnd, szText, "Hex Dump", MB_OK);
	free(szText);
	return;
}
