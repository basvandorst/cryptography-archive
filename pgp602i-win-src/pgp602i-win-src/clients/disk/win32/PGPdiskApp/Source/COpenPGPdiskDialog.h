//////////////////////////////////////////////////////////////////////////////
// COpenPGPdiskDialog.h
//
// Declaration of class COpenPGPdiskDialog.
//////////////////////////////////////////////////////////////////////////////

// $Id: COpenPGPdiskDialog.h,v 1.1.2.5 1998/07/06 08:57:01 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_COpenPGPdiskDialog_h	// [
#define Included_COpenPGPdiskDialog_h

#include "DualErr.h"


////////
// Types
////////

// OpenDialogType specifies the type of Open dialog being created. kODT_Mount
// is for mounts, kODT_SelectPGPdisk is for choosing a PGPdisk for passphrase
// additions/changes/removals. For now, the only difference between the types
// is in the title of the dialog box.

enum OpenDialogType {kODT_Mount, kODT_Select};


///////////////////////////
// Class COpenPGPdiskDialog
///////////////////////////

class COpenPGPdiskDialog : public CFileDialog
{
	DECLARE_DYNAMIC(COpenPGPdiskDialog)

public:
			COpenPGPdiskDialog(OpenDialogType dlgType, CWnd *pParent = NULL);

	DualErr	AskForPath(CString *path);

	//{{AFX_DATA(COpenPGPdiskDialog)
	//}}AFX_DATA

	//{{AFX_VIRTUAL(COpenPGPdiskDialog)
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnFileNameOK();
	//}}AFX_VIRTUAL

protected:
	OpenDialogType mOpenDialogType;		// open dialog type

	void	UpdateOpenDialogPrefs();
	void	OnInitDone();

	//{{AFX_MSG(COpenPGPdiskDialog)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_COpenPGPdiskDialog_h
