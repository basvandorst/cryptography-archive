//////////////////////////////////////////////////////////////////////////////
// CDriveLetterCombo.h
//
// Declaration of class CDriveLetterCombo.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDriveLetterCombo.h,v 1.1.2.3 1998/07/06 08:56:56 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDriveLetterCombo_h	// [
#define Included_CDriveLetterCombo_h

#include "Resource.h"


//////////////////////////
// Class CDriveLetterCombo
//////////////////////////

class CDriveLetterCombo : public CComboBox
{
public:
	PGPUInt8 mDriveNumber;		// drive number currently selected

			CDriveLetterCombo();
	virtual	~CDriveLetterCombo();

	void	InitDriveLetters();
	void	SetDriveLetter(PGPUInt8 drive);

	//{{AFX_VIRTUAL(CDriveLetterCombo)
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CDriveLetterCombo)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSelEndOk();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDriveLetterCombo_h
