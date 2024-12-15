//////////////////////////////////////////////////////////////////////////////
// CSecureEdit.cpp
//
// Implementation of class CSecureEdit.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSecureEdit.cpp,v 1.1.2.6 1998/07/28 23:46:37 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"
#include <new.h>

#include "Required.h"
#include "UtilityFunctions.h"

#include "CSecureEdit.h"
#include "Globals.h"


////////////
// Constants
////////////

// Hide typing by default?

const PGPBoolean kDefaultCSecureEditHideTyping = TRUE;


///////////////////////////
// MFC specific definitions
///////////////////////////

// These defines are needed because ClassWizard is intelligent enough to
// realize I'm mapping to the same function but too dumb to outsmart the
// preprocessor.

#define	DontClear	DoNothing
#define	DontCopy	DoNothing
#define	DontCut		DoNothing
#define	DontPaste	DoNothing
#define	DontUndo	DoNothing

BEGIN_MESSAGE_MAP(CSecureEdit, CEdit)
	//{{AFX_MSG_MAP(CSecureEdit)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_CLEAR, DontClear)
	ON_MESSAGE(WM_COPY, DontCopy)
	ON_MESSAGE(WM_CUT, DontCut)
	ON_MESSAGE(WM_PASTE, DontPaste)
	ON_MESSAGE(WM_UNDO, DontUndo)
	ON_CONTROL_REFLECT(EN_SETFOCUS, OnSetFocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CSecureEdit public custom functions and non-default message handlers
///////////////////////////////////////////////////////////////////////

// The CSecureEdit default constructor.

CSecureEdit::CSecureEdit() : mLockedMem(sizeof(SecureString))
{
	mCurCharPos			= 0;
	mCurNumSpaces		= 0;
	mHideTyping			= kDefaultCSecureEditHideTyping;
	mMaxSizeContents	= kMaxStringSize;

	// Pick from 1-3 random spaces to be used for each character.
	srand((PGPUInt32) PGPdiskGetTicks());

	for (PGPUInt32 i=0; i<kMaxStringSize; i++)
	{
		PGPUInt32 num = rand();		// rand roolz!

		if (num < RAND_MAX/3)
			mSpacesPerChar[i] = 1;
		else if (num < RAND_MAX*2/3)
			mSpacesPerChar[i] = 2;
		else
			mSpacesPerChar[i] = 3;
	}

	// Make sure our locked memory initialized correctly.
	mInitErr = mLockedMem.mInitErr;

	// Construct the secure string object.
	if (mInitErr.IsntError())
	{
		mContents = new (mLockedMem.GetPtr()) SecureString;		// <new.h>
		mContents->ClearString();
	}
}

// The CSecureEdit destructor destructs the SecureString we constructed in our
// locked memory.

CSecureEdit::~CSecureEdit()
{
	if (mContents)
		mContents->~SecureString();		// we used placement new
}

// ClearEditContents erases the contents of the edit box.

void 
CSecureEdit::ClearEditContents()
{
	mContents->ClearString();
	SetWindowText(kEmptyString);

	mCurCharPos = 0;

	// Notify parent of the edit box change.
	WPARAM wParam = (WPARAM) MakeLong(GetDlgCtrlID(), EN_CHANGE);
	LPARAM lParam = (LPARAM) GetSafeHwnd();

	GetParent()->PostMessage(WM_COMMAND, wParam, lParam);
}

// SetHideTypingPref shows or hides the text the user is typing. It returns
// the previous value of the preference.

PGPBoolean 
CSecureEdit::SetHideTypingPref(PGPBoolean newHideValue)
{
	CString		text;
	PGPBoolean	oldHideValue	= mHideTyping;

	mHideTyping = newHideValue;

	try
	{
		// If hiding the text, replace it with all spaces. If showing the
		// text, extract the contents from mContents and set the edit box.

		if (!oldHideValue && newHideValue)
		{
			FillEditBoxWithSpaces();
		}
		else if (oldHideValue && !newHideValue)
		{
			FillEditBoxWithContents();
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	return oldHideValue;
}

// SetMaxSizeContents sets the maximum size of the edit box. It returns the
// value of the old size. Nothing is changed if there is already this much
// or more text in the edit box.

PGPUInt32 
CSecureEdit::SetMaxSizeContents(PGPUInt32 maxSize)
{
	PGPUInt32 oldSize = mMaxSizeContents;

	if (maxSize > mContents->GetLength())
		mMaxSizeContents = maxSize;

	return oldSize;
}

//////////////////////////////////////////////////////////////////////////
// CSecureEdit protected custom functions and non-default message handlers
//////////////////////////////////////////////////////////////////////////

// FillEditBoxWithContents fills the edit box with the actual string
// contents.

void 
CSecureEdit::FillEditBoxWithContents()
{
	pgpAssert(!mHideTyping);

	try
	{
		CString text;

		// Get real contents.
		mContents->GetString(text.GetBuffer(kMaxStringSize), 
			kMaxStringSize);
		text.ReleaseBuffer();
		
		// Put the contents in the edit box.
		SetWindowText(text);
		SetSel(mCurCharPos, mCurCharPos);

		// Notify parent of the edit box change.
		NotifyParentOfEditChange();
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// FillEditBoxWithSpaces fills the edit box with predetermined amount of
// spaces instead of characters.

void 
CSecureEdit::FillEditBoxWithSpaces()
{
	pgpAssert(mHideTyping);

	try
	{
		CString spaces;

		mCurNumSpaces = 0;

		// Construct string of spaces.
		for (PGPUInt32 i = 0; i < mContents->GetLength(); i++)
		{
			mCurNumSpaces += mSpacesPerChar[i];
		}

		pgpFillMemory(spaces.GetBuffer(mCurNumSpaces), mCurNumSpaces, ' ');
		spaces.ReleaseBuffer(mCurNumSpaces);

		// Put the spaces in the edit box.
		SetWindowText(spaces);
		SetSel(mCurNumSpaces, mCurNumSpaces);

		// Notify parent of the edit box change.
		NotifyParentOfEditChange();
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// NotifyParentOfEditChange sends a message to the parent dialog telling it
// that the edit box contents have changed.

void 
CSecureEdit::NotifyParentOfEditChange()
{
	CWnd	*pParent;
	LPARAM	lParam;
	WPARAM	wParam;

	wParam = (WPARAM) MakeLong(GetDlgCtrlID(), EN_CHANGE);
	lParam = (LPARAM) GetSafeHwnd();

	pParent = GetParent();
	pgpAssert(pParent);

	if (pParent)
		pParent->PostMessage(WM_COMMAND, wParam, lParam);
}


/////////////////////////////////////////////////
// CSecureEdit protected default message handlers
/////////////////////////////////////////////////

// DoNothing is a dummy message handler used to prevent certain "unsafe"
// messages from reaching the framework.

void 
CSecureEdit::DoNothing(WPARAM wParam, LPARAM lParam)
{
}

// OnChar is overriden so we can implement our hidden typing routine.
// Instead of placing the character in the edit box, we place a space
// there instead and update only the secure mContents variable with the
// new character.

void 
CSecureEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	PGPBoolean isPrintable;

	isPrintable = (nChar >=32);

	if (isPrintable)	// printable character?
	{
		if (mContents->GetLength() == mMaxSizeContents)	// overflow?
		{
			MessageBeep(MB_ICONHAND);
		}
		else
		{
			// Insert the new character in the contents.
			mContents->InsertAt(mCurCharPos++, nChar);

			// If hiding typing, fill the edit box with spaces, else show the
			// real contents.

			if (mHideTyping)
				FillEditBoxWithSpaces();
			else
				FillEditBoxWithContents();
		}
	}
}

// OnLButtonDown is overidden so we can stop the user from making a selection
// or changing the position of the caret by clicking.

void 
CSecureEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Set us to have the focus!
	SetFocus();
}

// OnLButtonDblClk is overidden so we can stop the user from making a
// selection.

void 
CSecureEdit::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// Set us to have the focus!
	SetFocus();

	return;
}

// OnKeyDown is overriden so we can handle character deletion and selection.

void 
CSecureEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
	case VK_DELETE:
	case VK_HOME:
	case VK_END:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
	case VK_NEXT:
	case VK_PRIOR:
		// Disallow caret movement.
		break;

	case VK_BACK:
		// Delete the current character.
		if (mCurCharPos > 0)
		{
			// Remove the character from the contents.
			mContents->RemoveAt(--mCurCharPos);

			// If hiding typing, fill the edit box with spaces, else show the
			// real contents.

			if (mHideTyping)
				FillEditBoxWithSpaces();
			else
				FillEditBoxWithContents();
		}
		break;

	default:
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}

// OnMouseMove is overidden so we can stop the user from making a selection.

void 
CSecureEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	return;
}

// OnSetFocus is called when we are recieving the focus. Stop any text
// selections from being made.

void 
CSecureEdit::OnSetFocus() 
{
	SetSel(-1, -1);	
}
