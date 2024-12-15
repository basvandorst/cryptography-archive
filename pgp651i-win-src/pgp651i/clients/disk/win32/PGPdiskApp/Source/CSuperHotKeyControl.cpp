//////////////////////////////////////////////////////////////////////////////
// CSuperHotKeyControl.cpp
//
// Implementation of class CSuperHotKeyControl.
//////////////////////////////////////////////////////////////////////////////

// $Id: CSuperHotKeyControl.cpp,v 1.5 1998/12/15 01:25:04 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "PGPdiskPrefs.h"
#include "UtilityFunctions.h"

#include "CSuperHotKeyControl.h"


////////////
// Constants
////////////

// Flags for GetKeyNameText.
const PGPUInt32	kExtendedKeyFlag	= 1 << 24;		// handle extended keys
const PGPUInt32	kDontCareFlag		= 1 << 25;		// who cares left/right


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CSuperHotKeyControl, CEdit)
	//{{AFX_MSG_MAP(CSuperHotKeyControl)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////
// CSuperHotKeyControl public custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////

// The CSuperHotKeyControl default constructor.

CSuperHotKeyControl::CSuperHotKeyControl()
{
	mPrimaryVKey = NULL;
	mModKeyState = NULL;
}

// The CSuperHotKeyControl default destructor.

CSuperHotKeyControl::~CSuperHotKeyControl()
{
}

// GetHotKey returns the current hotkey. The low-order byte is the virtual key
// code, the high-order byte is the modifier flags.

PGPUInt16 
CSuperHotKeyControl::GetHotKey()
{
	return MakeWord(mPrimaryVKey, mModKeyState);
}

// SetHotKey sets the current hotkey. The low-order byte is the virtual key
// code, the high-order byte is the modifier flags.

void 
CSuperHotKeyControl::SetHotKey(PGPUInt16 code)
{
	mPrimaryVKey = GetLowByte(code);
	mModKeyState = GetHighByte(code);

	UpdateHotKeyDisplay();
}

// PreTranslateMessage is overridden so we can get at the key-code messages
// at the earliest possible moment.

BOOL 
CSuperHotKeyControl::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		ProcessKeyDown((PGPUInt8) pMsg->wParam, pMsg->lParam);
		return TRUE;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		return TRUE;
	}

	return CEdit::PreTranslateMessage(pMsg);
}


//////////////////////////////////////////////////////////////////////////
// CSuperHotKeyControl protected custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////

// ProcessKeyDown performs hot-key processing relating to key presses.

void 
CSuperHotKeyControl::ProcessKeyDown(PGPUInt8 vKey, PGPUInt32 wmKeyData) 
{
	PGPUInt8 primaryVKey, modKeyState;

	primaryVKey = vKey & 0x7F;
	modKeyState = NULL;

	switch (primaryVKey)
	{
	case VK_CONTROL:	// we take care of modifier keys separately
	case VK_MENU:
	case VK_SHIFT:
		break;

	case VK_CANCEL:		// these keys don't work for whatever reason
	case VK_PAUSE:
	case VK_SCROLL:
		break;

	case VK_F1:			// don't do F1 since it shows help
		break;

	default:			// all other keys we can handle
		if (wmKeyData & kExtendedKeyFlag)
			modKeyState |= kSHK_Extended;

		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			modKeyState |= kSHK_Control;

		if (GetAsyncKeyState(VK_MENU) & 0x8000)
			modKeyState |= kSHK_Alt;

		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
			modKeyState |= kSHK_Shift;

		mPrimaryVKey = primaryVKey;
		mModKeyState = modKeyState;

		UpdateHotKeyDisplay();

		break;
	}
}

// UpdateHotKeyDisplay is the heart of this control. It updates the edit box
// display and the mCurrentCombo member variable according to the current
// hotkey.

void 
CSuperHotKeyControl::UpdateHotKeyDisplay()
{
	try
	{
		CString		keyText, temp;
		PGPUInt8	scanCode;
		PGPUInt32	keyNameData;

		if (mModKeyState & kSHK_Control)
		{
			keyText += "Ctrl+";
		}

		if (mModKeyState & kSHK_Alt)
		{
			keyText += "Alt+";
		}

		if (mModKeyState & kSHK_Shift)
		{
			keyText += "Shift+";
		}

		// Gather data about the key.
		scanCode	= MapVirtualKey(mPrimaryVKey, 0);
		keyNameData = MakeLong(0, scanCode) | kDontCareFlag;

		if (mModKeyState & kSHK_Extended)
			keyNameData |= kExtendedKeyFlag;

		// Get the name of the key.
		GetKeyNameText(keyNameData, temp.GetBuffer(kMaxStringSize), 
			kMaxStringSize);
		temp.ReleaseBuffer();

		keyText += temp;
		SetWindowText(keyText);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}


/////////////////////////////////////////////////
// CSuperHotKeyControl protected default handlers
/////////////////////////////////////////////////

// OnMouseMove is called when the mouse moves within the control.

void 
CSuperHotKeyControl::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Disallow selections.
}
