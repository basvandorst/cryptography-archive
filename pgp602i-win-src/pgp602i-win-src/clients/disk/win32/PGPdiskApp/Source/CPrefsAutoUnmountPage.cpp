//////////////////////////////////////////////////////////////////////////////
// CPrefsAutoUnmountPage.cpp
//
// Implementation of class CPrefsAutoUnmountPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CPrefsAutoUnmountPage.cpp,v 1.1.2.11 1998/07/31 21:36:48 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"

#include "Required.h"
#include "CPrefsAutoUnmountPage.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_CHECK_SLEEP,		IDH_PGPDISKAPP_PREFSAUTOSLEEPCHECK, 
	IDC_CHECK_PREVENT,		IDH_PGPDISKAPP_PREFSPREVENTSLEEPCHECK, 
	IDC_CHECK_UNMOUNT,		IDH_PGPDISKAPP_PREFSAUTOUNMOUNTCHECK, 
	IDC_EDIT_SPIN,			IDH_PGPDISKAPP_PREFSAUTOUNMOUNTSPIN, 
	IDC_EDIT_UNMOUNT,		IDH_PGPDISKAPP_PREFSAUTOUNMOUNTEDIT, 
	IDC_MINUTETEXT,			IDH_PGPDISKAPP_PREFSAUTOUNMOUNTCHECK, 
    0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CPrefsAutoUnmountPage, CPropertyPage)
	//{{AFX_MSG_MAP(CPrefsAutoUnmountPage)
	ON_BN_CLICKED(IDC_CHECK_SLEEP, OnCheckSleep)
	ON_BN_CLICKED(IDC_CHECK_UNMOUNT, OnCheckAutoUnmount)
	ON_EN_CHANGE(IDC_EDIT_UNMOUNT, OnChangeTimeout)
	ON_WM_HELPINFO()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_CHECK_PREVENT, OnCheckPrevent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////
// CPrefsAutoUnmountPage public custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CPrefsAutoUnmountPage, CPropertyPage)

// The CPrefsAutoUnmountPage default constructor.

CPrefsAutoUnmountPage::CPrefsAutoUnmountPage()
	 : CPropertyPage(CPrefsAutoUnmountPage::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAutoUnmountPage)
	mTimeoutValue = 0;
	mSleepValue = FALSE;
	mAutoUnmountValue = FALSE;
	mPreventSleepValue = FALSE;
	//}}AFX_DATA_INIT

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CPrefsAutoUnmountPage destructor.

CPrefsAutoUnmountPage::~CPrefsAutoUnmountPage()
{
}

// We override OnSetActive to handle some dialog control initialization.

BOOL 
CPrefsAutoUnmountPage::OnSetActive() 
{
	// Disable sleep options on NT4.
	if (IsWinNT4CompatibleMachine() && !IsWinNT5CompatibleMachine())
	{
		mCheckSleepButton.EnableWindow(FALSE);
		mCheckPreventButton.EnableWindow(FALSE);
	}
	else
	{
		// Disable the prevent sleep box if unmount on sleep is unchecked.
		mCheckPreventButton.EnableWindow(mSleepValue);
	}

	// Set the range of the spin control.
	mMinutesSpin.SetRange(1, kMaxUnmountTimeout);
	
	return CPropertyPage::OnSetActive();
}


////////////////////////////////////////////////////////////////////////////
// CPrefsAutoUnmountPage protected custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////////

// ValidateTimeoutValue checks if the user has entered an invalid timeout
// value, and if so, prompts the user to re-enter. Returns TRUE if a valid
// timeout value was detected, FALSE otherwise.

PGPBoolean 
CPrefsAutoUnmountPage::ValidateTimeoutValue()
{
	CString	text;

	// Verify that the user entered an acceptable timeout value.
	UpdateData(TRUE);

	if ((mTimeoutValue > 0) && (mTimeoutValue <= kMaxUnmountTimeout))
	{
		return TRUE;
	}
	else
	{
		if (mTimeoutValue > kMaxUnmountTimeout)
		{
			mTimeoutValue = kMaxUnmountTimeout;
			UpdateData(FALSE);

			ReportError(kPGDMajorError_TimeoutValueTooBig);
		}
		else
		{
			mTimeoutValue = 1;
			UpdateData(FALSE);

			ReportError(kPGDMajorError_TimeoutValueTooSmall);
		}

		mAutoUnmountValue = 1;

		try
		{
			// Highlight the offending numeral.
			mTimeoutBox.SetFocus();
			mTimeoutBox.GetWindowText(text);
			mTimeoutBox.SetSel(0, text.GetLength(), FALSE);
		}
		catch (CMemoryException *ex)
		{
			ex->Delete();
		}

		return FALSE;
	}
}

// The CPrefsAutoUnmountPage data exchange function.

void 
CPrefsAutoUnmountPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CString		text;
		PGPBoolean	skipDDX	= FALSE;

		CPropertyPage::DoDataExchange(pDX);

		// Our goal is to coax MFC into accepting NULL fields for our timeout
		// box.

		if (mTimeoutBox.GetSafeHwnd())
			mTimeoutBox.GetWindowText(text);

		// Don't overwrite a null field with a 0.
		if (!pDX->m_bSaveAndValidate && text.IsEmpty() && 
			(mTimeoutValue == 0))
		{
			skipDDX = TRUE;
		}

		// Note the way the second 'if' statement ends within the
		// AFX_DATA_MAP. This is done so our variable will still appear in
		// the ClassWizard, but allows us to skip the DDX_Text when we want
		// to.

		if (pDX->m_bSaveAndValidate && text.IsEmpty())
		{
			skipDDX = TRUE;
			mTimeoutValue = 0;
		}

		if (!skipDDX)
		//{{AFX_DATA_MAP(CPrefsAutoUnmountPage)
			DDX_Text(pDX, IDC_EDIT_UNMOUNT, mTimeoutValue);
		DDX_Control(pDX, IDC_CHECK_SLEEP, mCheckSleepButton);
		DDX_Control(pDX, IDC_CHECK_PREVENT, mCheckPreventButton);
		DDX_Control(pDX, IDC_EDIT_SPIN, mMinutesSpin);
		DDX_Control(pDX, IDC_MINUTETEXT, mMinuteText);
		DDX_Control(pDX, IDC_EDIT_UNMOUNT, mTimeoutBox);
		DDX_Check(pDX, IDC_CHECK_SLEEP, mSleepValue);
		DDX_Check(pDX, IDC_CHECK_UNMOUNT, mAutoUnmountValue);
		DDX_Check(pDX, IDC_CHECK_PREVENT, mPreventSleepValue);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnCheckSleep is overridden so we can enable the 'Apply' button and enable
// or disable the prevent sleep checkbox as appropriate.

void 
CPrefsAutoUnmountPage::OnCheckSleep() 
{
	SetModified(TRUE);
	UpdateData(TRUE);

	mCheckPreventButton.EnableWindow(mSleepValue);
}

// OnCheckAutoUnmount is overridden so we can enable the 'Apply' button.

void 
CPrefsAutoUnmountPage::OnCheckAutoUnmount() 
{
	UpdateData(TRUE);

	if (ValidateTimeoutValue())
	{
		if (mAutoUnmountValue == 1)
		{
			mTimeoutBox.EnableWindow(TRUE);
			mMinutesSpin.EnableWindow(TRUE);
		}
		else
		{
			mTimeoutBox.EnableWindow(FALSE);
			mMinutesSpin.EnableWindow(FALSE);
		}

		SetModified(TRUE);
	}
}

// OnChangeTimeout is overridden so we can enable the 'Apply' button.

void 
CPrefsAutoUnmountPage::OnChangeTimeout() 
{
	SetModified(TRUE);	
}

// OnCheckPrevent is overridden so we can enable the 'Apply' button.

void 
CPrefsAutoUnmountPage::OnCheckPrevent() 
{
	SetModified(TRUE);	
}


///////////////////////////////////////////////////////////
// CPrefsAutoUnmountPage protected default message handlers
///////////////////////////////////////////////////////////

// OnApply is overridden to apply the changes the user made to the application,
// but we don't save them to the registry.

BOOL 
CPrefsAutoUnmountPage::OnApply() 
{
	DualErr	derr;

	UpdateData(TRUE);

	// If the user entered an invalid timeout value, make him change it.
	if (ValidateTimeoutValue())
	{
		// Update the application's variables.
		App->mAutoUnmount			= mAutoUnmountValue;
		App->mUnmountOnSleep		= mSleepValue;
		App->mNoSleepIfUnmountFail	= mPreventSleepValue;
		App->mUnmountTimeout		= mTimeoutValue;

		// Tell the driver.
		App->SetDriverPrefsFromApp();

		return CPropertyPage::OnApply();
	}
	else
	{
		return FALSE;
	}
}

// OnHelpInfo handles context-sensitive help.

BOOL 
CPrefsAutoUnmountPage::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// OnInitDialog is overwritten to perform property page initialization.

BOOL 
CPrefsAutoUnmountPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// Initialize the three check boxes.
	mSleepValue			= App->mUnmountOnSleep;
	mAutoUnmountValue	= App->mAutoUnmount;
	mPreventSleepValue	= App->mNoSleepIfUnmountFail;

	// Fill in the edit box with the timeout value.
	mTimeoutValue = App->mUnmountTimeout;

	UpdateData(FALSE);

	if (mAutoUnmountValue == 1)
	{
		mTimeoutBox.EnableWindow(TRUE);
		mMinutesSpin.EnableWindow(TRUE);
	}
	else
	{
		mTimeoutBox.EnableWindow(FALSE);
		mMinutesSpin.EnableWindow(FALSE);
	}

	return TRUE;
}

// OnKillActive is overridden to perform data validation.

BOOL 
CPrefsAutoUnmountPage::OnKillActive() 
{
	// If the user entered an invalid timeout value, make him change it.
	if (!ValidateTimeoutValue())
		return FALSE;
	else
		return CPropertyPage::OnKillActive();
}

// OnLButtonDown is called when the user presses the left mouse button. We
// see if he is clicking in the 'minutes of inactivity' text and then alter
// the appropriate check box.

void 
CPrefsAutoUnmountPage::OnLButtonDown(UINT nFlags, CPoint point) 
{
	PGPBoolean pointInMinuteText;

	pointInMinuteText = ChildWindowFromPoint(point)->GetSafeHwnd() ==
		mMinuteText.GetSafeHwnd();

	if (pointInMinuteText)
	{
		UpdateData(TRUE);
		mAutoUnmountValue = !mAutoUnmountValue;
		UpdateData(FALSE);
	}

	CPropertyPage::OnLButtonDown(nFlags, point);
}

// OnOK is overridden to save the new value to the registry. It is always
// called right after OnApply.

void 
CPrefsAutoUnmountPage::OnOK() 
{
	DualErr	derr;

	// Update the registry.
	derr = App->SetRegistryPrefs();

	if (derr.IsError())
	{
		ReportError(kPGDMajorError_PreferencesAccessFailed, derr);
	}
	else
	{
		App->UpdatePGPdiskResident();
	}

	CPropertyPage::OnOK();
}
