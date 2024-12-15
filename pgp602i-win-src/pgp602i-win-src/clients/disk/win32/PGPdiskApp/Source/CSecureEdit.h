//////////////////////////////////////////////////////////////////////////////
// CSecureEdit.h
//
// Declaration of class CSecureEdit.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSecureEdit.h,v 1.1.2.5 1998/07/06 08:57:21 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CSecureEdit_h	// [
#define Included_CSecureEdit_h

#include "DualErr.h"
#include "SecureMemory.h"
#include "SecureString.h"

#include "Resource.h"


////////////////////
// Class CSecureEdit
////////////////////

class CSecureEdit : public CEdit
{
public:
	DualErr			mInitErr;
	SecureString	*mContents;					// string contents

				CSecureEdit();
	virtual		~CSecureEdit();

	void		ClearEditContents();

	PGPBoolean	SetHideTypingPref(PGPBoolean newHideValue);
	PGPUInt32	SetMaxSizeContents(PGPUInt32 maxSize);

	//{{AFX_VIRTUAL(CSecureEdit)
	//}}AFX_VIRTUAL

protected:
	PGPBoolean		mHideTyping;				// hide keystrokes?
	SecureMemory	mLockedMem;					// our locked memory

	PGPUInt8		mSpacesPerChar[kMaxStringSize];		// spaces per char
	PGPInt32		mCurCharPos;				// caret at which character?
	PGPInt32		mCurNumSpaces;				// current number spaces
	PGPUInt32		mMaxSizeContents;			// max characters

	void	FillEditBoxWithContents();
	void	FillEditBoxWithSpaces();

	void	NotifyParentOfEditChange();

	//{{AFX_MSG(CSecureEdit)
	afx_msg void DoNothing(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CSecureEdit_h
