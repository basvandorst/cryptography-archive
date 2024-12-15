//////////////////////////////////////////////////////////////////////////////
// CSavePGPdiskDialog.h
//
// Declaration of class CSavePGPdiskDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSavePGPdiskDialog.h,v 1.5 1998/12/14 18:58:17 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CSavePGPdiskDialog_h	// [
#define Included_CSavePGPdiskDialog_h


////////
// Types
////////

// SaveDialogType specifies the type of Save dialog being created. So far
// there is only one type, kSDT_Create, for PGPdisk creation.

enum SaveDialogType	{kSDT_Create};


///////////////////////////
// Class CSavePGPdiskDialog
///////////////////////////

class CSavePGPdiskDialog : public CFileDialog
{
	DECLARE_DYNAMIC(CSavePGPdiskDialog)

public:
			CSavePGPdiskDialog(SaveDialogType dlgType, CWnd *pParent = NULL);

	DualErr	AskForPath(CString *path, LPCSTR defaultPath = NULL);

	//{{AFX_DATA(CSavePGPdiskDialog)
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CSavePGPdiskDialog)
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnFileNameOK();
	//}}AFX_VIRTUAL

protected:
	SaveDialogType mSaveDialogType;		// save dialog type

	void	GetInitialPGPdiskName(LPCSTR initialDir, CString *initialName);
	void	UpdateSaveDialogPrefs();
	void	OnInitDone();

	//{{AFX_MSG(CSavePGPdiskDialog)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CSavePGPdiskDialog_h
