/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */

#ifndef __UI_H__
#define __UI_H__


// CUICommon is the catch-all user interface class.

class CUICommon
{
private:
    static HINSTANCE _hinst;

    static const TCHAR* _pszHelpFileForCallback;
    static void CALLBACK MessageBoxHelpCallback(LPHELPINFO);

protected:
    HWND _hwnd;
    BOOL _fWindowVisible;

    // Strings fetched from the resource file

    static TCHAR _szTitle[128];

 public:
    CUICommon() : _hwnd(NULL), _fWindowVisible(FALSE) {}
    CUICommon(HWND hwnd) : _hwnd(hwnd), _fWindowVisible(hwnd != NULL) {}

    static BOOL Init( HINSTANCE hinst );

    HWND GetHwndVisible() const;
    HWND GetHwnd() const
        { return _hwnd; }
    void GetRect(LPRECT prc) const
        { ::GetWindowRect(_hwnd, prc); }
    void Close();

    void Message(UINT ids,
        UINT nStyle = ( MB_SETFOREGROUND |
                        MB_TASKMODAL |
                        MB_ICONEXCLAMATION |
                        MB_OK)) const;
    void Message(const TCHAR* psz,
        UINT nStyle = ( MB_SETFOREGROUND |
                        MB_TASKMODAL |
                        MB_ICONEXCLAMATION |
                        MB_OK)) const;
    void MessageWithHelp(const TCHAR* psz, const TCHAR* pszHelpFile,
        ULONG ulHelpContext,
        UINT nStyle = ( MB_SETFOREGROUND |
                        MB_TASKMODAL |
                        MB_ICONEXCLAMATION |
                        MB_OK |
                        MB_HELP)) const;
    void ErrorMessage( IUnknown* punk, HRESULT hr,
        const TCHAR* pszFunction ) const;
    void ErrorMessage( IUnknown* punk, HRESULT hr, UINT ids ) const;
    int Query(UINT ids, UINT nStyle) const;

    static HINSTANCE GetHinst() { return _hinst; }
};

// Other utility functions

extern void GetString(TCHAR*, int, UINT);
extern void FormatString(TCHAR*, int, UINT, ...);


#endif // __UI_H__
