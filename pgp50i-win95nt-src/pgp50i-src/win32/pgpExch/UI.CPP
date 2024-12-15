/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */

#include "stdinc.h"
#include "pgpexch.h"
#include "ui.h"
#include "assert.h"
#include "resource.h"


// Set this if a message box is up.

static BOOL FMBoxUp = FALSE;

// Used by MessageBoxHelpCallback

const TCHAR* CUICommon::_pszHelpFileForCallback = NULL;

HINSTANCE CUICommon::_hinst = NULL;

TCHAR CUICommon::_szTitle[128];


BOOL CUICommon::Init( HINSTANCE hinst )
{
    _hinst = hinst;

    ::LoadString(hinst, IDS_TITLE, _szTitle, sizeof(_szTitle));

    return TRUE;
}


void CUICommon::ErrorMessage( IUnknown* punk, HRESULT hr,
                             const TCHAR* pszFunction ) const
{
    // The error code MAPI_E_USER_CANCEL indicates that the user
    // terminated the current action.  There's no need to present
    // an error message in this case.

    if ( MAPI_E_USER_CANCEL == hr )
        return;

    assert( S_OK != hr && S_FALSE != hr ); // Oops!

    // Cast the LPUNKNOWN object to one that contains GetLastError.
    // Since this call is "guaranteed" to be in the same location on
    // all objects, this is a "safe" assumption.  Ahem.

    MAPIERROR* perr = NULL;
    ((IMAPIProp*)punk)->GetLastError( hr, fMapiUnicode, &perr );

    // Render MAPIERROR information into temporary buffers.

    ULONG ulLowLevelError = 0;
    const TCHAR* pszError = NULL;
    const TCHAR* pszComponent = NULL;

    if (perr != NULL)
    {
        if (perr->lpszError)
            pszError = perr->lpszError;
        if (perr->lpszComponent)
            pszComponent = perr->lpszComponent;
        if (perr->ulLowLevelError)
            ulLowLevelError = perr->ulLowLevelError;
    }

    TCHAR szNoErr[128];
    TCHAR szNoInfo[128];

    if (pszFunction == NULL)
    {
        GetString(szNoErr, sizeof(szNoErr)/sizeof(TCHAR), IDS_NO_ERR);
        pszFunction = szNoErr;
    }
    if (pszError == NULL)
    {
        GetString(szNoInfo, sizeof(szNoInfo)/sizeof(TCHAR), IDS_NO_INFO);
        pszError = szNoInfo;
    }

    TCHAR szNumbers[80];
    if (pszComponent == NULL)
    {
        if (ulLowLevelError == 0L)
            wsprintf(szNumbers, "[%08X]", (ULONG)hr);
        else
            wsprintf(szNumbers, "%u - [%08X]", ulLowLevelError, (ULONG)hr);
    }
    else
    {
        if (ulLowLevelError == 0L)
            wsprintf(szNumbers, "%s - [%08X]", pszComponent, (ULONG)hr);
        else
            wsprintf(szNumbers, "%50s - %u - [%08X]",
                pszComponent, ulLowLevelError, (ULONG)hr);
    }

    TCHAR szBuffer[1024];
    wsprintf(szBuffer, "%s\r\n%s\r\n%s", pszFunction, pszError, szNumbers);

    // See if help is available for this error and component.

    TCHAR szHelpFile[_MAX_PATH];
    BOOL fHelpAvailable = (pszComponent && perr->ulContext);
    if (fHelpAvailable)
    {
        int cch = ::GetPrivateProfileString("Help File Mappings",
            pszComponent,
            "", szHelpFile, _MAX_PATH, "mapisvc.inf");
        if (0 == cch)
            fHelpAvailable = FALSE;
    }

    // Display the error, with help if available.

    if (!fHelpAvailable)
    {
        Message(szBuffer);
    }
    else
    {
        MessageWithHelp(szBuffer,
            szHelpFile,
            perr->ulContext);
    }

    MAPIFreeBuffer(perr);
}


void CALLBACK CUICommon::MessageBoxHelpCallback(LPHELPINFO lpHelpInfo)
{
    assert(_pszHelpFileForCallback && *_pszHelpFileForCallback);

    WinHelp(NULL, _pszHelpFileForCallback, HELP_CONTEXT,
        lpHelpInfo->dwContextId);
}


void CUICommon::MessageWithHelp(const TCHAR* pszMessage,
                                const TCHAR* pszHelpFile,
                                ULONG ulHelpContext,
                                UINT nStyle) const
{
    MSGBOXPARAMS mbp = {0};

    mbp.cbSize = sizeof(MSGBOXPARAMS);
    mbp.hwndOwner = GetHwndVisible();
    mbp.hInstance = GetHinst();
    mbp.lpszText = pszMessage;
    mbp.lpszCaption = _szTitle;
    mbp.dwStyle = nStyle | MB_HELP;
    mbp.lpszIcon = NULL;
    mbp.dwContextHelpId = ulHelpContext;
    mbp.lpfnMsgBoxCallback = MessageBoxHelpCallback;
    mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

    _pszHelpFileForCallback = pszHelpFile;
    ::MessageBoxIndirect(&mbp);
    _pszHelpFileForCallback = NULL;
}


void CUICommon::ErrorMessage( IUnknown* punk, HRESULT hr, UINT ids ) const
{
    TCHAR szMessage[256];

    ::LoadString(GetHinst(), ids, szMessage, 255);
    ErrorMessage(punk, hr, szMessage);
}


HWND CUICommon::GetHwndVisible() const
{
    if (_fWindowVisible)
        return _hwnd;
    else
       return ::GetDesktopWindow();
}


void CUICommon::Message( const TCHAR* pszFunction, UINT nStyle ) const
{
    BOOL fOldMBUp = ::FMBoxUp;
    ::FMBoxUp = TRUE;

    ::MessageBox(GetHwndVisible(), pszFunction, _szTitle, nStyle);

    ::FMBoxUp = fOldMBUp;
}


void CUICommon::Message(UINT idString, UINT nStyle) const
{
    TCHAR szMessage[256];

    ::LoadString(GetHinst(), idString, szMessage, 255);
    Message(szMessage, nStyle);
}


int CUICommon::Query(UINT idString, UINT nStyle) const
{
    BOOL fOldMBUp = ::FMBoxUp;
    ::FMBoxUp = TRUE;

    TCHAR szMessage[256];

    ::LoadString(GetHinst(), idString, szMessage, 255);
    int nRet = ::MessageBox(GetHwndVisible(), szMessage, _szTitle,
        (MB_SETFOREGROUND | MB_TASKMODAL | MB_ICONQUESTION | nStyle));

    ::FMBoxUp = fOldMBUp;
    return nRet;
}


void CUICommon::Close()
{
    ::PostMessage(_hwnd, WM_CLOSE, 0, 0L);
}


void GetString(TCHAR* szOut, int cch, UINT ids)
{
    ::LoadString(CUICommon::GetHinst(), ids, szOut, cch);
}


void FormatString(TCHAR* szOut, int cch, UINT ids, ...)
{
    va_list va;
    va_start(va, ids);

    TCHAR szBuffer[256];
    ::LoadString(CUICommon::GetHinst(), ids, szBuffer, 255);
    ::FormatMessage(FORMAT_MESSAGE_FROM_STRING,
        szBuffer, 0, 0, szOut, cch, &va);

    va_end(va);
}
