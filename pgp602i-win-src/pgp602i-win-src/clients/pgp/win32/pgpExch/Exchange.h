/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	


	$Id: Exchange.h,v 1.18.8.1 1998/11/12 03:13:55 heller Exp $



____________________________________________________________________________*/
#ifndef Included_Exchange_h	/* [ */
#define Included_Exchange_h

#include "stdinc.h"
#include "pgpUtilities.h"
#include "pgpEncode.h"
#include "pgpTLS.h"
#include "pgpcl.h"

// Global variables

extern PGPContextRef _pgpContext;		// PGP context of current instance
extern PGPtlsContextRef _tlsContext;	// TLS context of current instance
extern PGPMemoryMgrRef _memoryMgr;		// Memory manager of PGP context
extern PGPError _errContext;			// Error code if context failed


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
	BOOL	_fOutlook;			// Set if Outlook is being used
	BOOL	_fOutlook98;		// Set if Outlook98 is being used
	BOOL	_fLoaded;			// Set if the plug-in DLL was already loaded
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
	UINT	_itbmPgpKeys;

	// Button ID's

	UINT	_itbbSign;
	UINT	_itbbSignNow;
	UINT	_itbbEncrypt;
	UINT	_itbbEncryptNow;
	UINT	_itbbEncryptSign;
	UINT	_itbbDecrypt;
	UINT	_itbbPgpKeys;

	// Toolbar handles

	HWND	_hwndSendToolbar;
	HWND	_hwndReadToolbar;

	// Message flags

	BOOL	_bSign;
	BOOL	_bEncrypt;
	BOOL	_bHaveAttachments;

	// Set in either or both of InitMenu and OnReadComplete. 

	CMsgType _msgtype;			// Type of message, analyzed at note-read 
								// time.

int EncryptSignMessage(HWND hwnd,
					   IMessage *pmsg,
					   RECIPIENTDIALOGSTRUCT *prds,
					   PGPOptionListRef *pSignOptions);

int EncryptSignAttachment(HWND hwnd,
						  IMessage *pmsg, 
						  UINT nOutSize,
						  SRowSet *prAttach,
						  RECIPIENTDIALOGSTRUCT *prds,
						  PGPOptionListRef *pSignOptions);

BOOL EncryptSignMessageWindow(HWND hwndMain,
							  IExchExtCallback *pmecb,
							  BOOL bEncrypt,
							  BOOL bSign,
							  RECIPIENTDIALOGSTRUCT *prds);
};


// In MsgProps.cpp
// Persistence structure for decryption

class CDecrypt
{
private:
	IMAPISession* _psess;
	BOOL _fInit;

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
};



// Lock-object, which puts up a wait cursor until the object dies.

class CWaitCursor
{
private:
	HCURSOR _hcurSave;

public:
	CWaitCursor() : _hcurSave(SetCursor(LoadCursor(NULL, IDC_WAIT))) {}
	~CWaitCursor()  { SetCursor(_hcurSave); }
};

#endif /* ] Included_Exchange_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
