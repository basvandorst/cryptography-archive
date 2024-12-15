/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	


	$Id: Main.cpp,v 1.31.2.3 1999/06/04 17:02:38 dgal Exp $



____________________________________________________________________________*/
#include "stdinc.h"
#include "Exchange.h"
#include "Outlook.h"
#include "Recipients.h"
#include "resource.h"
#include "UIutils.h"
#include "pgpUtilities.h"
#include "pgpKeys.h"
#include "PGPcl.h"

// The version that went out with Win95 is 410; with Exchange Server, 837.
// (RC1 used 611; RC2 used 736.3.)
// The Windows 95 Messaging Update is 839.
// 
// Exchange 5.0 comes back as 4.0 build 1458
// Outlook 98 comes back as 4.0 build 1573

#define LAST_BUILD_SUPPORTED	1573
#define OUTLOOK98_BUILD			1573

// Other values returned by IExchExtCallback::GetVersion
// Microsoft Exchange 4.0 is 0x01??0400

#define CURRENT_PRODUCT_CODE  0x01000000
#define LAST_MAJVER_SUPPORTED 0x00000400
#define LAST_MINVER_SUPPORTED 0x00000000

extern "C" IExchExt* CALLBACK ExchEntryPoint();

static BOOL IsExchExtWithinOutlook(IExchExtCallback* peecb);

static void TurnOffWarning(ULONG ulMajVer, ULONG ulMinVer, 
						   ULONG ulBuildMinVer);

static void SetNewerOK(ULONG ulMajVer, 
					   ULONG ulMinVer, 
					   ULONG ulBuildMinVer, 
					   BOOL bOK);

static BOOL IsWarningOff(ULONG ulMajVer, 
						 ULONG ulMinVer, 
						 ULONG ulBuildMinVer);

static BOOL IsNewerOK(ULONG ulMajVer, 
					  ULONG ulMinVer, 
					  ULONG ulBuildMinVer);

static BOOL DoIExist(BOOL bOutlook);
static void SetNoLoad(void);
static BOOL ShouldILoad(void);
static HWND CreateHiddenWindow(void);


LRESULT CALLBACK HiddenWindowProc(HWND hwnd, 
								  UINT msg, 
								  WPARAM wParam, 
								  LPARAM lParam);

// Global variables

PGPContextRef _pgpContext = NULL;		// PGP context of current instance
PGPtlsContextRef _tlsContext = NULL;	// TLS context of current instance
PGPMemoryMgrRef _memoryMgr = NULL;		// Memory manager of PGP context
PGPError _errContext = kPGPError_NoErr;	// Error code if context failed
HWND _hwndHidden = NULL;				// Window for catching messages
UINT _nPurgeCacheMsg = 0;				// Purge Passphrase Cache message
LPSRowSet _pGALRows;					// Data extracted from GAL Table


// DLL entry point

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID)
{
 	if (DLL_PROCESS_ATTACH == fdwReason)
 	{
		UISetInstance(hinstDll);
 	}

 	if (DLL_PROCESS_DETACH == fdwReason)
 	{
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
	_bHaveAttachments = FALSE;
	_szAttachment = NULL;
}


CExtImpl::~CExtImpl()
{
	if (_hmenuPGP)
		DestroyMenu(_hmenuPGP);

	if (_szAttachment != NULL)
		WipeMessageAttachment();

	if (!_fLoaded)
	{
		if (_hwndHidden)
		{
			DestroyWindow(_hwndHidden);
			_hwndHidden = NULL;
		}
		
		if (_pGALRows != NULL)
		{
			FreeProws(_pGALRows);
			_pGALRows = NULL;
		}
		
		if (_tlsContext)
		{
			PGPFreeTLSContext(_tlsContext);
			_tlsContext = NULL;
		}

		if (_pgpContext && IsntPGPError(_errContext))
		{
			PGPclCloseLibrary();
			PGPFreeContext(_pgpContext);
			_pgpContext = NULL;
			_memoryMgr = NULL;
		}
	}
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
    ULONG ulBuildVer;
	ULONG ulProductVer;
	ULONG ulMajVer;
	ULONG ulMinVer;
	ULONG ulBuildMinVer;
	BOOL fMinorBuildOk;
    HRESULT hr;
	HWND hwnd;
	PGPError err;

	peecb->GetWindow(&hwnd);
	if (!hwnd)
		hwnd = GetTopWindow(NULL);

	hr = peecb->GetVersion(&ulProductVer, EECBGV_GETVIRTUALVERSION);
	if (SUCCEEDED(hr))
		hr = peecb->GetVersion(&ulBuildVer, EECBGV_GETBUILDVERSION);
	if (FAILED(hr))
	{
		UIDisplayStringID(hwnd, IDS_E_NOVERSION);
		SetNoLoad();
		return S_FALSE;
	}
		
	ulMajVer = ulProductVer & EECBGV_VERSION_MAJOR_MASK;
	ulMinVer = ulProductVer & EECBGV_VERSION_MINOR_MASK;
	ulBuildMinVer = ulBuildVer & EECBGV_BUILDVERSION_MINOR_MASK;
		
	// Check to see if we're running Outlook97
	_fOutlook = IsExchExtWithinOutlook(peecb);

	// Check to see if we're running Outlook98
	if (ulBuildMinVer == OUTLOOK98_BUILD)
		_fOutlook98 = TRUE;
	else
		_fOutlook98 = FALSE;

	_fLoaded = DoIExist(_fOutlook);
	if ((!_fLoaded) || (_pgpContext == NULL))
		_errContext = PGPNewContext(kPGPsdkAPIVersion, &_pgpContext);

	if (!ShouldILoad())
		return S_FALSE;

    _context = eecontext;

	// Check for SDK expiration or other SDK initialization failure

	if (IsPGPError(_errContext))
	{
		if (_errContext == kPGPError_FeatureNotAvailable)
			UIDisplayStringID(hwnd, IDS_E_EXPIRED);
		else
			PGPclErrorBox(hwnd, _errContext);

		SetNoLoad();
		return S_FALSE;
	}

	// Initialize common library

	if (!_fLoaded)
	{
		err = PGPclInitLibrary(_pgpContext);
		if (IsPGPError(err))
		{
			PGPclErrorBox(hwnd, err);
			SetNoLoad();
			return S_FALSE;
		}
	}

	// Check for beta/demo expiration

	if (PGPclIsExpired(hwnd) != kPGPError_NoErr)
	{
		SetNoLoad();
		return S_FALSE;
	}

	if (!_fLoaded)
	{
		// Register the passphrase cache purge message

		_nPurgeCacheMsg = RegisterWindowMessage(PURGEPASSPHRASECACEHMSG);

		// Create a hidden window to catch messages

		_hwndHidden = CreateHiddenWindow();

		_memoryMgr = PGPGetContextMemoryMgr(_pgpContext);
		PGPNewTLSContext(_pgpContext, &_tlsContext);

		_pGALRows = NULL;

		// Ensure that this is the right version on first load
		
		if ((CURRENT_PRODUCT_CODE != 
			(ulProductVer & EECBGV_VERSION_PRODUCT_MASK))   
			||
			(EECBGV_BUILDVERSION_MAJOR != 
			(ulBuildVer & EECBGV_BUILDVERSION_MAJOR_MASK)))
		{
			// The first time, explain why we aren't loading.
			// Subsequently, remain silent.
			
			if (!IsWarningOff(ulMajVer, ulMinVer, ulBuildMinVer))
			{
				TurnOffWarning(ulMajVer, ulMinVer, ulBuildMinVer);
				UIDisplayStringID(hwnd, IDS_E_INCOMPATIBLE_VERSION);
			}
			SetNoLoad();
			return S_FALSE;
		}

		if ((LAST_MAJVER_SUPPORTED < (ulMajVer)) ||
			(LAST_MINVER_SUPPORTED < (ulMinVer)) ||
			(LAST_BUILD_SUPPORTED  < (ulBuildMinVer)))
		{
			// Warn the user of a newer version of Exchange.
			// If the user loads the plug-in anyway, don't warn anymore,
			// otherwise, warn every time.
			if (!IsWarningOff(ulMajVer, ulMinVer, ulBuildMinVer))
			{
				fMinorBuildOk =	
					(UIAskYesNoStringID(hwnd, IDS_Q_LATER_BUILD) == IDYES);
				if (fMinorBuildOk)
					TurnOffWarning(ulMajVer, ulMinVer, ulBuildMinVer);
			}
			else
				fMinorBuildOk = IsNewerOK(ulMajVer, ulMinVer, ulBuildMinVer);
			
			if (!fMinorBuildOk)
			{
				SetNoLoad();
				return S_FALSE;
			}
			else
				SetNewerOK(ulMajVer, ulMinVer, ulBuildMinVer, TRUE);
		}
	}

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


BOOL IsExchExtWithinOutlook(IExchExtCallback* peecb)
{
	IOutlookGetObjectForExchExtCallback* po = NULL;

	HRESULT hr = peecb->
					QueryInterface(IID_IOutlookGetObjectForExchExtCallback, 
						(void**)&po);
	if (po)
		po->Release();

	return (SUCCEEDED(hr));
}


void TurnOffWarning(ULONG ulMajVer, ULONG ulMinVer, ULONG ulBuildMinVer)
{
	HKEY hkey;
	char szRegKey[255];
	char szVersion[255];
	DWORD dwDummy;
	DWORD dwValue;

	UIGetString(szRegKey, 254, IDS_WARNING_REGKEY);
	wsprintf(szVersion, "Maj %ld Min %ld Build %ld", ulMajVer, ulMinVer, 
		ulBuildMinVer);
	strcat(szRegKey, szVersion);

	RegCreateKeyEx(HKEY_CURRENT_USER, szRegKey, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDummy);

	dwValue = 1;
	RegSetValueEx(hkey, "WarningOff", 0, REG_DWORD, (BYTE *) &dwValue, 
		sizeof(DWORD));
	RegCloseKey(hkey);
	return;
}


void SetNewerOK(ULONG ulMajVer, 
				ULONG ulMinVer, 
				ULONG ulBuildMinVer, 
				BOOL bOK)
{
	HKEY hkey;
	char szRegKey[255];
	char szVersion[255];
	DWORD dwDummy;
	DWORD dwValue;

	UIGetString(szRegKey, 254, IDS_WARNING_REGKEY);
	wsprintf(szVersion, "Maj %ld Min %ld Build %ld", ulMajVer, ulMinVer, 
		ulBuildMinVer);
	strcat(szRegKey, szVersion);

	RegCreateKeyEx(HKEY_CURRENT_USER, szRegKey, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDummy);

	dwValue = bOK;
	RegSetValueEx(hkey, "NewerOK", 0, REG_DWORD, (BYTE *) &dwValue, 
		sizeof(DWORD));
	RegCloseKey(hkey);
	return;
}


BOOL IsWarningOff(ULONG ulMajVer, ULONG ulMinVer, ULONG ulBuildMinVer)
{
	HKEY hkey;
	char szRegKey[255];
	char szVersion[255];
	DWORD dwDummy;
	DWORD dwValue;
	DWORD dwType;
	DWORD dwSize=4;

	UIGetString(szRegKey, 254, IDS_WARNING_REGKEY);
	wsprintf(szVersion, "Maj %ld Min %ld Build %ld", ulMajVer, ulMinVer, 
		ulBuildMinVer);
	strcat(szRegKey, szVersion);

	RegCreateKeyEx(HKEY_CURRENT_USER, szRegKey, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDummy);

	if (RegQueryValueEx(hkey, "WarningOff", 0, &dwType, (BYTE *) &dwValue, 
		&dwSize) != ERROR_SUCCESS)
	{
		dwValue = 0;
		RegSetValueEx(hkey, "WarningOff", 0, REG_DWORD, (BYTE *) &dwValue, 
			sizeof(DWORD));
	}

	RegCloseKey(hkey);
	return dwValue;
}


BOOL IsNewerOK(ULONG ulMajVer, ULONG ulMinVer, ULONG ulBuildMinVer)
{
	HKEY hkey;
	char szRegKey[255];
	char szVersion[255];
	DWORD dwDummy;
	DWORD dwValue;
	DWORD dwType;
	DWORD dwSize=4;

	UIGetString(szRegKey, 254, IDS_WARNING_REGKEY);
	wsprintf(szVersion, "Maj %ld Min %ld Build %ld", ulMajVer, ulMinVer, 
		ulBuildMinVer);
	strcat(szRegKey, szVersion);

	RegCreateKeyEx(HKEY_CURRENT_USER, szRegKey, 0, NULL, 
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDummy);

	if (RegQueryValueEx(hkey, "NewerOK", 0, &dwType, (BYTE *) &dwValue, 
		&dwSize) != ERROR_SUCCESS)
	{
		dwValue = 0;
		RegSetValueEx(hkey, "NewerOK", 0, REG_DWORD, (BYTE *) &dwValue, 
			sizeof(DWORD));
	}

	RegCloseKey(hkey);
	return dwValue;
}


BOOL DoIExist(BOOL bOutlook)
{
    HANDLE hSem;

    // Create or open a named semaphore

	if (bOutlook)
	    hSem = CreateSemaphore (NULL, 0, 1, "pgpOutlookInstSem");
	else
		hSem = CreateSemaphore (NULL, 0, 1, "pgpExchangeInstSem");

    // Close handle and return TRUE if existing semaphore was opened.
    if ((hSem != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)) 
	{
        CloseHandle(hSem);
        return TRUE;
	}

	return FALSE;
}


void SetNoLoad(void)
{
    HANDLE hSem;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, "pgpExchLoadSem");

    if ((hSem != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS)) 
        CloseHandle(hSem);

	return;
}


BOOL ShouldILoad(void)
{
    HANDLE hSem;
	BOOL bShouldLoad = TRUE;

    // Create or open a named semaphore. 
    hSem = CreateSemaphore (NULL, 0, 1, "pgpExchLoadSem");

    if (hSem != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS) 
			bShouldLoad = FALSE;
		else
			bShouldLoad = TRUE;

		CloseHandle(hSem);
	}

	return bShouldLoad;
}


HWND CreateHiddenWindow(void)
{
	HWND hwnd;
	WNDCLASS wc;

	// Register the Window Class

	wc.style			= 0;
	wc.lpfnWndProc		= HiddenWindowProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= UIGetInstance();
	wc.hIcon			= 0;
	wc.hCursor			= 0;
	wc.hbrBackground	= 0;
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= "pgpExch Hidden Window";

	RegisterClass(&wc);

	hwnd = CreateWindow("pgpExch Hidden Window", "pgpExch Hidden Window",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, NULL, NULL, UIGetInstance(), NULL);

	return hwnd;
}


LRESULT CALLBACK HiddenWindowProc(HWND hwnd, 
								  UINT msg, 
								  WPARAM wParam, 
								  LPARAM lParam)
{
	if (msg == WM_CREATE)
	{
		ShowWindow(hwnd, SW_HIDE);
		return 0;
	}

	if (msg == _nPurgeCacheMsg)
	{
		PGPclPurgeCachedPassphrase(wParam);
		return TRUE;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
