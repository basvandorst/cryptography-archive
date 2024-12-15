//////////////////////////////////////////////////////////////////////////////
// CUnmountPGPdiskDialog.h
//
// Declaration of class CUnmountPGPdiskDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CUnmountPGPdiskDialog.h,v 1.4 1998/12/14 18:58:35 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CUnmountPGPdiskDialog_h	// [
#define Included_CUnmountPGPdiskDialog_h

#include "DualErr.h"

#include "Resource.h"


//////////////////////////////
// Class CUnmountPGPdiskDialog
//////////////////////////////

// CUnmountPGPdiskDialog handles all single-passphrase dialog boxes.

class CUnmountPGPdiskDialog : public CDialog
{

public:
	PGPUInt8 mDriveToUnmount;		// specifies the PGPdisk to unmount

			CUnmountPGPdiskDialog(CWnd* pParent = NULL);
			~CUnmountPGPdiskDialog();

	DualErr	AskForDriveLetter();

	//{{AFX_DATA(CUnmountPGPdiskDialog)
	enum { IDD = IDD_UNMOUNT_DLG };
	CComboBox	mDriveComboBox;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CUnmountPGPdiskDialog)
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CUnmountPGPdiskDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnUnmount();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CUnmountPGPdiskDialog_h
