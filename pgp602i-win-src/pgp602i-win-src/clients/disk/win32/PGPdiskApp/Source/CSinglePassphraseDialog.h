//////////////////////////////////////////////////////////////////////////////
// CSinglePassphraseDialog.h
//
// Declaration of class CSinglePassphraseDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSinglePassphraseDialog.h,v 1.1.2.5.2.2 1998/10/28 00:13:31 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CSinglePassphraseDialog_h	// [
#define Included_CSinglePassphraseDialog_h

#include "DualErr.h"
#include "PGPdiskFileFormat.h"

#include "CDriveLetterCombo.h"
#include "CSecureEdit.h"
#include "Resource.h"


////////
// Types
////////

// SinglePassDialogType specifies the type of single passphrase dialog being
// shown. This influences the wording of the dialog box message and the
// presence of certain options.

enum SinglePassDialogType {kSPDT_Change, kSPDT_Master, kSPDT_Mount, 
	kSPDT_PublicKey, kSPDT_Remove};


////////////////////////////////
// Class CSinglePassphraseDialog
////////////////////////////////

// CSinglePassphraseDialog handles all single-passphrase dialog boxes.

class CSinglePassphraseDialog : public CDialog
{

public:
	CButton				mReadOnlyButton;	// MFC object for read-only button
	CDriveLetterCombo	mDriveCombo;		// MFC object for drive combo
	DualErr				mInitErr;
	PGPBoolean			mReadOnlyValue;		// read-only button checked?

			CSinglePassphraseDialog(SinglePassDialogType dlgType);
			~CSinglePassphraseDialog();

	DualErr	AskForPassphrase(LPCSTR path, PGPBoolean forceReadOnly = FALSE, 
		PGPBoolean needMaster = FALSE);

	void	SetReadOnlyValue(PGPBoolean readOnly);
	void	DisableReadOnlyButton();

	//{{AFX_DATA(CSinglePassphraseDialog)
	enum { IDD = IDD_SINGLEPASS1_DLG };
	CStatic	mMessageStatic;
	CStatic	mCapsLockStatic;
	CButton	mOKButton;
	CButton	mCancelButton;
	CSecureEdit	mPassphraseEdit;
	BOOL	mHideTypeValue;
	CString	mMessageValue;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CSinglePassphraseDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	CString					mPath;						// path of PGPdisk
	SinglePassDialogType	mSinglePassDialogType;		// dialog type
	PGPBoolean				mNeedMaster;				// need master?

	void	SetCapsLockMessageState();
	DualErr TellUserAboutUnpurgablePass();

	//{{AFX_MSG(CSinglePassphraseDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnHideTypeCheck();
	virtual void OnOK();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnROCheck();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// Included_CSinglePassphraseDialog_h
