//////////////////////////////////////////////////////////////////////////////
// CConfirmPassphraseDialog.h
//
// Declaration of class CConfirmPassphraseDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CConfirmPassphraseDialog.h,v 1.1.2.6 1998/07/06 08:56:43 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CConfirmPassphraseDialog_h	// [
#define Included_CConfirmPassphraseDialog_h

#include "CSecureEdit.h"
#include "Resource.h"


////////
// Types
////////

// ConfirmPassDialogType specifies the type of confirmation passphrase dialog
// being shown. This influences the wording of the dialog box message and the
// presence of certain options.

enum ConfirmPassDialogType {kCPDT_Add, kCPDT_Change};


/////////////////////////////////
// Class CConfirmPassphraseDialog
/////////////////////////////////

class CConfirmPassphraseDialog : public CDialog
{
public:
	CButton		mReadOnlyButton;
	DualErr		mInitErr;
	PGPBoolean	mReadOnlyValue;

			CConfirmPassphraseDialog(ConfirmPassDialogType dlgType);
			~CConfirmPassphraseDialog();

	DualErr	AskForPassphrase(LPCSTR path);
	void	DisableReadOnlyButton();
	void	SetReadOnlyValue(PGPBoolean readOnly);

	//{{AFX_DATA(CConfirmPassphraseDialog)
	enum { IDD = IDD_CONFIRMPASS1_DLG };
	CStatic	mMessageStatic;
	CStatic	mCapsLockStatic;
	CButton	mOKButton;
	CButton	mCancelButton;
	CSecureEdit	mPassphraseEdit;
	CSecureEdit	mConfirmationEdit;
	CProgressCtrl	mQualityBar;
	BOOL	mHideTypeValue;
	CString	mMessageValue;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CConfirmPassphraseDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	ConfirmPassDialogType	mConfirmPassDialogType;		// dialog type
	CSecureEdit				*mIHadFocus;				// edit that had focus
	CString					mPath;						// path of PGPdisk

	void SetCapsLockMessageState();

	//{{AFX_MSG(CConfirmPassphraseDialog)
	afx_msg void OnHideTypeCheck();
	afx_msg void OnChangePassphrase();
	afx_msg void OnKillFocusPassphrase();
	afx_msg void OnKillFocusConfirmation();
	virtual void OnOK();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnROCheck();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif /* ] Included_CConfirmPassphraseDialog_h */
