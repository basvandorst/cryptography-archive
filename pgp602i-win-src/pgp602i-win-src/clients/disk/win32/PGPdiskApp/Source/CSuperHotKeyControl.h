//////////////////////////////////////////////////////////////////////////////
// CSuperHotKeyControl.h
//
// Declaration of class CSuperHotKeyControl.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSuperHotKeyControl.h,v 1.1.2.4 1998/07/06 08:57:25 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CSuperHotKeyControl_h	// [
#define Included_CSuperHotKeyControl_h


////////////////////////////
// Class CSuperHotKeyControl 
////////////////////////////

// A CSuperHotKeyControl is like a CHotKeyCtrl except it handles many more key
// combinations.

class CSuperHotKeyControl : public CEdit
{
public:
				CSuperHotKeyControl();
	virtual		~CSuperHotKeyControl();

	PGPUInt16	GetHotKey();
	void		SetHotKey(PGPUInt16 code);

	//{{AFX_VIRTUAL(CSuperHotKeyControl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
	PGPUInt8	mPrimaryVKey;		// the primary (non-mod) key
	PGPUInt8	mModKeyState;		// state of alt, control, shift

	void	ProcessKeyDown(PGPUInt8 vKey, PGPUInt32 wmKeyData);
	void	UpdateHotKeyDisplay();

	//{{AFX_MSG(CSuperHotKeyControl)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CSuperHotKeyControl_h
