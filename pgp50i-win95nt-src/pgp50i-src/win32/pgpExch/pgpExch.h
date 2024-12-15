/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */

#ifndef __PGPEXCH_H__
#define __PGPEXCH_H__


// Get application-private GUIDs

#include "guids.h"


// Forward class declarations

class CMsgType;
class CWaitCursor;


// Prototypes


// In PRSHT.CPP
extern BOOL CALLBACK MainPropPageDlgProc(HWND, UINT, WPARAM, LPARAM);

// In ABOUT.CPP
extern BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

// In STDNOTE.CPP
extern HWND FindREOnNote(HWND hwndNote);

// In DEBUG.CPP
extern void DumpPropTag(ULONG);

// In ExchEncryptSign.cpp
extern UINT EncryptSignBuffer(HWND hwnd,void* pInput,DWORD dwInSize,
                              void *pRecDlgStruct,void** ppOutput,
                              DWORD* pOutSize,BOOL bEncrypt,BOOL bSign,
                              BOOL bAttachment,BOOL bUseMIME);

extern UINT EncryptSignFile(HWND hwnd,char *szInFile,void *pRecDlgStruct,
                            char **pszOutFile,BOOL bEncrypt,BOOL bSign,
                            BOOL bEncryptMIME,BOOL bSignMIME);

// In ExchDecryptVerify.cpp
extern UINT DecryptVerifyBuffer(HWND hwnd,void* pInput,DWORD dwInSize,
                                void* pMessage,DWORD dwMsgSize,
                                void** ppOutput,DWORD* pOutSize);

extern UINT DecryptVerifyFile(HWND hwnd,char *szInFile, char *szSigFile,
                              char **pszOutFile);

// In ExchAddKey.cpp
extern UINT AddKeyBuffer(HWND hwnd, void* pInput, DWORD dwInSize);

extern UINT AddKeyFile(HWND hwnd, char *szInFile);


// Lock-object, which puts up a wait cursor until the object dies.

class CWaitCursor
{
private:
    HCURSOR _hcurSave;

public:
    CWaitCursor() : _hcurSave(SetCursor(LoadCursor(NULL, IDC_WAIT))) {}
    ~CWaitCursor()  { SetCursor(_hcurSave); }
};


// The follow persistence class is in PRSHT.CPP

// Persistence structure for decryption

class CDecrypt
{
private:
    IMAPISession* _psess;
    BOOL _fInit;
    BOOL _fAutoDecrypt;
    BOOL _fAutoAddKey;

public:
    CDecrypt(IMAPISession*);
    ~CDecrypt();

    void Init()
        { if (!_fInit) Load(); }
    void Load();
    void Save();

    // Can take or drop session on the fly (needed because it can
    // outlive any particular callback).

    void TakeSession(IMAPISession*);
    void DropSession();

    // Unlike cf, uses accessor functions.

    BOOL WillAutoDecrypt() const
       { return _fAutoDecrypt; }
    BOOL WillAutoAddKey() const
       { return _fAutoAddKey; }
    void SetWillAutoDecrypt(int nVal)
       { _fAutoDecrypt = (BOOL)!!nVal; }
    void SetWillAutoAddKey(int nVal)
       { _fAutoAddKey = (BOOL)!!nVal; }
};



// In STDNOTE.CPP
// Heuristic class to guess the origin of current note

class CMsgType
{
private:
    UINT _cRecips;
    BOOL _fSubjSet;
    BOOL _fNew;
    BOOL _fIsReply;
    BOOL _fIsFwd;
    BOOL _fIsSubmitted;

public:
    CMsgType() : _cRecips(0), _fSubjSet(FALSE),
        _fNew(FALSE), _fIsReply(FALSE), _fIsFwd(FALSE),
        _fIsSubmitted(FALSE) { }
    HRESULT Read(IExchExtCallback* peecb);
    HRESULT Write(IExchExtCallback* peecb, BOOL fInSubmit);

    BOOL IsNew() const;
    BOOL IsForward() const;
    BOOL IsReply() const;
    BOOL WasSubmittedOnce() const;
};



#endif // __PGPEXCH_H__
