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

#include "config.h"
#include "pgpKeyDB.h"
#include "PGPcomdlg.h"

struct SPropPageAux
{
    BOOL fDirty;
    CDecrypt decrypt;

    SPropPageAux(IMAPISession* psess)
        : fDirty(FALSE), decrypt(psess) {}
};


// Callback for the property sheet page in Tools - Options

BOOL CALLBACK MainPropPageDlgProc(HWND hdlg, UINT nMsg,
                                  WPARAM wParam, LPARAM lParam)
{
    BOOL fMsgResult;

    switch (nMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE* ppsp = (PROPSHEETPAGE*)lParam;
            SPropPageAux *paux = new SPropPageAux(
                                            (IMAPISession*)(ppsp->lParam));
            paux->decrypt.Init();
            SetWindowLong(hdlg, GWL_USERDATA, (DWORD)paux);

            CheckDlgButton(hdlg, IDC_DECRYPT, paux->decrypt.WillAutoDecrypt());
            CheckDlgButton(hdlg, IDC_ADDKEY, paux->decrypt.WillAutoAddKey());
            return TRUE;
        }
        break;

    case WM_DESTROY:
        {
            SPropPageAux *paux =
                (SPropPageAux *)GetWindowLong(hdlg, GWL_USERDATA);
            delete paux;
            SetWindowLong(hdlg, GWL_USERDATA, 0L);
        }
        return TRUE;

    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDC_ABOUT:
                {
                    TCHAR szTitle[100];
                    TCHAR szVersion[50];
                    TCHAR szVerString[200];

                    GetString(szTitle, sizeof(szTitle)/sizeof(TCHAR),
                                IDS_TITLE);
                    GetString(szVersion, sizeof(szVersion)/sizeof(TCHAR),
                                IDS_VERSION);
                    wsprintf(szVerString, "%s %s", szTitle, szVersion);
                    PGPcomdlgHelpAbout(hdlg, NULL, NULL, NULL);
                }
                return TRUE;

            case IDC_PGPPREFS:
                {
                    PGPcomdlgPreferences(hdlg, 0);
                }
                return TRUE;

            case IDC_DECRYPT:
                {
                    const BOOL fCheck = IsDlgButtonChecked(hdlg, IDC_DECRYPT);
                    SPropPageAux *paux =
                        (SPropPageAux *)GetWindowLong(hdlg, GWL_USERDATA);
                    paux->fDirty = TRUE;
                    paux->decrypt.SetWillAutoDecrypt(fCheck);
                    PropSheet_Changed(GetParent(hdlg), hdlg);
                }
                return TRUE;

            case IDC_ADDKEY:
                {
                    const BOOL fCheck = IsDlgButtonChecked(hdlg, IDC_ADDKEY);
                    SPropPageAux *paux =
                        (SPropPageAux *)GetWindowLong(hdlg, GWL_USERDATA);
                    paux->fDirty = TRUE;
                    paux->decrypt.SetWillAutoAddKey(fCheck);
                    PropSheet_Changed(GetParent(hdlg), hdlg);
                }
                return TRUE;
            }
        }
        break;

    case WM_NOTIFY:
        {
            switch ( ((LPNMHDR) lParam)->code)
            {
            case PSN_KILLACTIVE:
                fMsgResult = FALSE;  // Allow this page to receive PSN_APPLY.
                break;

            case PSN_SETACTIVE:

                //  Fill the controls in the page with information...

                fMsgResult = FALSE;  // Accepts activation
                break;

            case PSN_APPLY:
                {
                    SPropPageAux *paux =
                        (SPropPageAux *)GetWindowLong(hdlg, GWL_USERDATA);
                    if (paux->fDirty)
                    {
                        paux->decrypt.Save();
                        paux->fDirty = FALSE;
                    }
                }
                fMsgResult = PSNRET_NOERROR;
                break;

            case PSN_HELP:
                // Fake an About command
                PostMessage(hdlg, WM_COMMAND, IDC_ABOUT, 0);
                fMsgResult = TRUE;    // Doesn't matter on this notification
                break;

            case PSN_QUERYCANCEL:
                {
                    SPropPageAux *paux =
                        (SPropPageAux *)GetWindowLong(hdlg, GWL_USERDATA);
                    if (paux->fDirty)
                    {
                        CUICommon ui(hdlg);
                        fMsgResult = (ui.Query(IDS_Q_DISCARDCHANGES, MB_YESNO)
                                        == IDNO);

                       // Return TRUE (i.e., IDNO) to cancel.
                       // Return FALSE (i.e., IDYES) to proceed,
                       //   discarding changes.
                    }
                    else
                    {
                        fMsgResult = FALSE;
                    }
                }
                break;

            default:
                fMsgResult = FALSE;
                break;
            } // Switch

            SetWindowLong(hdlg, DWL_MSGRESULT, fMsgResult);
            break;
        } // Case WM_NOTIFY

    default:
        fMsgResult = FALSE;
        break;
    } // Switch

    return fMsgResult;
    }


#define PR_PROFILE_DECRYPT    PROP_TAG(PT_BOOLEAN, 0x7C00)
#define PR_PROFILE_ADDKEY     PROP_TAG(PT_BOOLEAN, 0x7C01)


CDecrypt::CDecrypt(IMAPISession* psess)
    : _psess(psess), _fInit(FALSE), _fAutoDecrypt(FALSE), _fAutoAddKey(FALSE)
{
    if (psess)
        psess->AddRef();
}


CDecrypt::~CDecrypt()
{
    if (_psess)
        _psess->Release();
}


void CDecrypt::TakeSession(IMAPISession* psess)
{
    if (_psess)
        _psess->Release();
        _psess = psess;
    if (psess)
        psess->AddRef();
}


void CDecrypt::DropSession()
{
    if (_psess)
        _psess->Release();
    _psess = NULL;
}


void CDecrypt::Load()
{
    // If we hit any exceptions, initialize as empty anyway.

    _fInit = TRUE;

    assert(_psess != NULL);

    IProfSect* pprof;
    HRESULT hr = _psess->OpenProfileSection(
       (LPMAPIUID)&GUID_PGPPlugin, NULL, 0, &pprof);
    if (FAILED(hr)) // Assume uninitialized
       return;

    enum {ivalDecrypt=0, ivalAddKey, cvalTotal};
    SizedSPropTagArray(cvalTotal, taga) =
        { cvalTotal,
        { PR_PROFILE_DECRYPT, PR_PROFILE_ADDKEY }};
    SPropValue* pval;
    ULONG cval = cvalTotal;
    hr = pprof->GetProps((SPropTagArray*)&taga, 0, &cval, &pval);
    if (FAILED(hr))
    {
        CUICommon().Message( IDS_E_INITPROFILE );
        pprof->Release();
        return;
    }
    if (hr == MAPI_W_ERRORS_RETURNED)
    {
        MAPIFreeBuffer(pval);
        CUICommon().Message( IDS_E_INITPROFILE );
        pprof->Release();
        return;
    }

    _fAutoDecrypt = pval[ivalDecrypt].Value.b;
    _fAutoAddKey = pval[ivalAddKey].Value.b;

    MAPIFreeBuffer(pval);
    pprof->Release();

}


void CDecrypt::Save()
{
    assert(_psess != NULL);

    IProfSect* pprof;
    HRESULT hr = _psess->OpenProfileSection(
        (LPMAPIUID)&GUID_PGPPlugin, NULL, MAPI_MODIFY, &pprof);
    if (SUCCEEDED(hr))
    {
        SPropValue val;
        val.ulPropTag = PR_PROFILE_DECRYPT;
        val.Value.b = _fAutoDecrypt;

        hr = HrSetOneProp(pprof, &val);
    }
    if (SUCCEEDED(hr))
    {
        SPropValue val;
        val.ulPropTag = PR_PROFILE_ADDKEY;
        val.Value.b = _fAutoAddKey;

        hr = HrSetOneProp(pprof, &val);
    }
    if (SUCCEEDED(hr))
        hr = pprof->SaveChanges(0);
    if (pprof)
       pprof->Release();

     if (FAILED(hr))
     {
        CUICommon().Message( IDS_E_SAVEPROFILE );
     }

  }
