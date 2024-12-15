//////////////////////////////////////////////////////////////////////////////
// CDiskWizVolumeInfoPage.cpp
//
// Implementation of class CDiskWizVolumeInfoPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizVolumeInfoPage.cpp,v 1.9 1999/05/24 23:41:05 heller Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"
#include <math.h>

#include "Required.h"
#include "PGPdiskPrefs.h"
#include "StringAssociation.h"
#include "UtilityFunctions.h"

#include "CDiskWizardSheet.h"
#include "CDiskWizVolumeInfoPage.h"
#include "File.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_SIZE_EDIT,			IDH_PGPDISKAPP_DISKWIZVOLSIZEEDIT, 
	IDC_SIZE_SPIN,			IDH_PGPDISKAPP_DISKWIZVOLSIZESPIN, 
	IDC_DRIVE_COMBO,		IDH_PGPDISKAPP_DISKWIZVOLDRIVECOMBO, 
	IDC_GB_CHECK,			IDH_PGPDISKAPP_DISKWIZVOLGBCHECK, 
	IDC_MB_CHECK,			IDH_PGPDISKAPP_DISKWIZVOLMBCHECK, 
	IDC_KB_CHECK,			IDH_PGPDISKAPP_DISKWIZVOLKBCHECK, 
    IDC_FREESIZE_TEXT,		((PGPUInt32) -1), 
	IDC_SIDEBAR,			((PGPUInt32) -1), 
	0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CDiskWizVolumeInfoPage, CPropertyPage)
	//{{AFX_MSG_MAP(CDiskWizVolumeInfoPage)
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_KB_CHECK, OnKbCheck)
	ON_BN_CLICKED(IDC_MB_CHECK, OnMbCheck)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_GB_CHECK, OnGbCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////
// Constants
////////////

const double	kSizeIsCloseRatio	= 0.95;


//////////////////////////////////////////////////////////////////////////
// CDiskWizVolumeInfoPage public custom functions and non-default handlers
//////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDiskWizVolumeInfoPage, CPropertyPage)

// The CDiskWizVolumeInfoPage default constructor.

CDiskWizVolumeInfoPage::CDiskWizVolumeInfoPage()
	 : CPropertyPage(CDiskWizVolumeInfoPage::IDD)
{
	//{{AFX_DATA_INIT(CDiskWizVolumeInfoPage)
	mSizeValue = 0;
	mFreeSizeText = _T("");
	//}}AFX_DATA_INIT

	mParentDiskWiz = NULL;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CDiskWizVolumeInfoPage destructor.

CDiskWizVolumeInfoPage::~CDiskWizVolumeInfoPage()
{
}


/////////////////////////////////////////////////////////
// CDiskWizVolumeInfoPage public default message handlers
/////////////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CDiskWizVolumeInfoPage::OnSetActive() 
{
	pgpAssertAddrValid(mParentDiskWiz, CDiskWizardSheet);
	mParentDiskWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

// OnWizardNext is called when the 'Next' button is pressed. We perform
// data validation.

LRESULT 
CDiskWizVolumeInfoPage::OnWizardNext() 
{
	try
	{
		PGPBoolean	failThis	= FALSE;
		PGPUInt32	kbDisk;

		if (CPropertyPage::OnWizardNext() == -1)
			return -1;

		// Get the size value in kilobytes.
		kbDisk = GetSizeValueInKb();

		// Verify that the space being asked is not too big or small.
		if (kbDisk < mLocalMinKbSize)
		{
			failThis = TRUE;
			SetPGPdiskSize(mLocalMinKbSize);

			ReportError(kPGDMajorError_PGPdiskTooSmall);
		}
		else if (kbDisk > App->mPlatformMaxKbSize)
		{
			failThis = TRUE;
			SetPGPdiskSize(mLocalMaxKbSize);

			ReportError(kPGDMajorError_PGPdiskTooBig);
		}
		else if (kbDisk > mLocalMaxKbSize)
		{
			failThis = TRUE;
			SetPGPdiskSize(mLocalMaxKbSize);

			ReportError(kPGDMajorError_NotEnoughDiskSpace);
		}

		// Handle special case where we can't be sure of the amount of free
		// space on the host drive.

		if (mAreWeFailingDiskFree && !failThis)
		{
			CString		dir;
			DualErr		dummyErr;
			PGPBoolean	isEnoughSpace;

			// Get the directory to test.
			dummyErr = GetDirectory(mParentDiskWiz->mPath, &dir);

			// Test if there really is enough space available. If we can't
			// tell then don't fail.

			if (dummyErr.IsntError())
			{
				dummyErr = App->BruteForceTestIfEnoughSpace(dir, kbDisk, 
					&isEnoughSpace);
			}

			if (dummyErr.IsntError())
			{
				if (!isEnoughSpace)
				{
					failThis = TRUE;
					ReportError(kPGDMajorError_NotEnoughDiskSpace);
				}
			}
		}

		if (failThis)
		{
			CString text;

			// Point out the incorrect size entered by the user.
			mSizeEditBox.SetFocus();
			mSizeEditBox.GetWindowText(text);
			mSizeEditBox.SetSel(0, text.GetLength(), FALSE);

			return -1;
		}
		else
		{
			// Remember disk size.
			SetPGPdiskSizeInRegistry();

			mBlocksDisk = ((PGPUInt64) kbDisk * kBytesPerKb) / 
				kDefaultBlockSize;

			return 0;
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		return 0;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDiskWizVolumeInfoPage protected custom functions and non-default handlers
/////////////////////////////////////////////////////////////////////////////

// ConvertControlsAux is a utility function for the 'ConvertControls'
// functions.

void 
CDiskWizVolumeInfoPage::ConvertControlsAux(
	PGPUInt32	nMin, 
	PGPUInt32	nMax, 
	PGPUInt32	kbPerUnit)
{
	// If current size is out of range, change it.
	UpdateData(TRUE);

	if (mSizeValue < nMin)
	{
		mSizeValue = nMin;
	}
	else if (mSizeValue > nMax)
	{
		mSizeValue = nMax;
	}

	// Set ranges of controls.
	mSizeSpin.SetRange(nMin, nMax);

	UpdateData(FALSE);
}

// ConvertControlsToKb converts the dialog's controls to KB.

void 
CDiskWizVolumeInfoPage::ConvertControlsToKb()
{
	mSizeScale = kLCS_KbScale;

	mKbButton.SetCheck(1);
	mMbButton.SetCheck(0);
	mGbButton.SetCheck(0);

	ConvertControlsAux(mLocalMinKbSize, min(kKbPerMeg*2, mLocalMaxKbSize), 1);
}

// ConvertControlsToMb converts the dialog's controls to MB.

void 
CDiskWizVolumeInfoPage::ConvertControlsToMb()
{
	mSizeScale = kLCS_MbScale;

	mKbButton.SetCheck(0);
	mMbButton.SetCheck(1);
	mGbButton.SetCheck(0);

	// Round up if closer than 5%.
	PGPUInt32	mbMax	= mLocalMaxKbSize/kKbPerMeg;

	if (mLocalMaxKbSize%kKbPerMeg > kSizeIsCloseRatio*kKbPerMeg)
		mbMax++;

	ConvertControlsAux(1, min(kMbPerGig*2, mbMax), kKbPerMeg);
}

// ConvertControlsToGb converts the dialog's controls to GB.

void 
CDiskWizVolumeInfoPage::ConvertControlsToGb()
{
	mSizeScale = kLCS_GbScale;

	mKbButton.SetCheck(0);
	mMbButton.SetCheck(0);
	mGbButton.SetCheck(1);

	// Round up if closer than 5%.
	PGPUInt32	gbMax	= mLocalMaxKbSize/kKbPerGig;

	if (mLocalMaxKbSize%kKbPerGig > kSizeIsCloseRatio*kKbPerGig)
		gbMax++;

	ConvertControlsAux(1, gbMax, kKbPerGig);
}

// GetSizeValueInKb returns the current size entered in KB.

PGPUInt32 
CDiskWizVolumeInfoPage::GetSizeValueInKb()
{
	UpdateData(TRUE);

	switch (mSizeScale)
	{
	case kLCS_KbScale:
		return mSizeValue;

	case kLCS_MbScale:
		
		if ((mLocalMaxKbSize%kKbPerMeg > kSizeIsCloseRatio*kKbPerMeg) &&
			(mSizeValue == mLocalMaxKbSize/kKbPerMeg + 1))
		{
			return mLocalMaxKbSize;
		}
		else
		{
			return mSizeValue*kKbPerMeg;
		}

	case kLCS_GbScale:

		if ((mLocalMaxKbSize%kKbPerGig > kSizeIsCloseRatio*kKbPerGig) &&
			(mSizeValue == mLocalMaxKbSize/kKbPerGig + 1))
		{
			return mLocalMaxKbSize;
		}
		else
		{
			return mSizeValue*kKbPerGig;
		}

	default:
		pgpAssert(FALSE);
		return 0;
	}
}

// SetPGPdiskSize sets the size edit box and the slider to the given size.

void 
CDiskWizVolumeInfoPage::SetPGPdiskSize(
	PGPUInt32			kbSize, 
	PGPdiskCreateScale	scale)
{
	if (scale == kLCS_InvalidScale)
		scale = mSizeScale;

	// Set size to legal value.
	if (kbSize < mLocalMinKbSize)
	{
		kbSize = mLocalMinKbSize;
	}
	else if (kbSize > mLocalMaxKbSize)
	{
		kbSize = mLocalMaxKbSize;
	}

	// Set scale to legal value.
	if (mLocalMaxKbSize < kSizeIsCloseRatio*kKbPerMeg)
	{
		scale = kLCS_KbScale;
	}
	else if (mLocalMaxKbSize < kSizeIsCloseRatio*kKbPerGig)
	{
		if (scale == kLCS_GbScale)
			scale = kLCS_MbScale;
	}

	// What scale do we use?
	switch (scale)
	{
	case kLCS_KbScale:

		mSizeValue = kbSize;
		UpdateData(FALSE);
		ConvertControlsToKb();
		break;

	case kLCS_MbScale:

		mSizeValue = kbSize/kKbPerMeg;
		UpdateData(FALSE);
		ConvertControlsToMb();
		break;

	case kLCS_GbScale:

		mSizeValue = kbSize/kKbPerGig;
		UpdateData(FALSE);
		ConvertControlsToGb();
		break;

	default:
		pgpAssert(FALSE);
		break;
	}
}

// GetPGPdiskSizeFromRegistry initializes the size from the value we
// previously stored in the registry.

void 
CDiskWizVolumeInfoPage::GetPGPdiskSizeFromRegistry()
{
	PGPdiskCreateScale	createScale;
	PGPdiskWin32Prefs	prefs;
	PGPUInt32			kbSize;

	if (GetPGPdiskWin32Prefs(prefs).IsntError())
	{
		kbSize = prefs.lastCreateSize;
		createScale = prefs.lastCreateScale;
		
		SetPGPdiskSize(kbSize, createScale);
	}
}

// SetPGPdiskSizeInRegistry stores the current PGPdisk size in the registry.

DualErr 
CDiskWizVolumeInfoPage::SetPGPdiskSizeInRegistry()
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
	{
		prefs.lastCreateSize = GetSizeValueInKb();
		prefs.lastCreateScale = mSizeScale;

		derr = SetPGPdiskWin32Prefs(prefs);
	}

	return derr;
}

// The CDiskWizVolumeInfoPage data exchange function.

void 
CDiskWizVolumeInfoPage::DoDataExchange(CDataExchange *pDX)
{
	try
	{
		CString		text;
		PGPBoolean	skipDDX	= FALSE;

		CPropertyPage::DoDataExchange(pDX);

		// Our goal is to coax MFC into accepting NULL fields for our size
		// box.

		if (mSizeEditBox.GetSafeHwnd())
			mSizeEditBox.GetWindowText(text);

		// Don't overwrite a null field with a 0.
		if (!pDX->m_bSaveAndValidate && text.IsEmpty() && (mSizeValue == 0))
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
			mSizeValue = 0;
		}

		if (!skipDDX)		
		//{{AFX_DATA_MAP(CDiskWizVolumeInfoPage)
			DDX_Text(pDX, IDC_SIZE_EDIT, mSizeValue);
		DDX_Control(pDX, IDC_DRIVE_COMBO, mDriveCombo);
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		DDX_Control(pDX, IDC_SIZE_SPIN, mSizeSpin);
		DDX_Control(pDX, IDC_SIZE_EDIT, mSizeEditBox);
		DDX_Text(pDX, IDC_FREESIZE_TEXT, mFreeSizeText);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnKbCheck is called when the kB button is clicked.

void 
CDiskWizVolumeInfoPage::OnKbCheck() 
{
	ConvertControlsToKb();	
}

// OnMbCheck is called when the MB button is clicked.

void 
CDiskWizVolumeInfoPage::OnMbCheck() 
{
	ConvertControlsToMb();	
}

// OnMbCheck is called when the GB button is clicked.

void 
CDiskWizVolumeInfoPage::OnGbCheck() 
{
	ConvertControlsToGb();	
}


////////////////////////////////////////////////////////////
// CDiskWizVolumeInfoPage protected default message handlers
////////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CDiskWizVolumeInfoPage::OnHelpInfo(HELPINFO *pHelpInfo)
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
CDiskWizVolumeInfoPage::OnInitDialog() 
{
	DualErr		dummyErr;
	File		existingFile;
	PGPUInt64	bytesFile, bytesFree;

	CPropertyPage::OnInitDialog();

	// This must be called first to make MFC happy.
	UpdateData(FALSE);

	// Classwizard won't subclasss our radio buttons (sniff).
	mKbButton.SubclassWindow(GetDlgItem(IDC_KB_CHECK)->GetSafeHwnd());
	mMbButton.SubclassWindow(GetDlgItem(IDC_MB_CHECK)->GetSafeHwnd());
	mGbButton.SubclassWindow(GetDlgItem(IDC_GB_CHECK)->GetSafeHwnd());

	// Any drive letters free?
	if (!App->AreAnyDriveLettersFree())
	{
		mParentDiskWiz->mDerr = 
			DualErr(kPGDMinorError_NoDriveLettersFree);
		mParentDiskWiz->PressButton(PSBTN_CANCEL);	// NOT EndDialog (bug)

		return FALSE;
	}

	// Initialize the combo box.
	mDriveCombo.InitDriveLetters();

	// How much space do we have available?
	dummyErr = App->HowMuchFreeSpace(mParentDiskWiz->mPath, &bytesFree);

	// Don't fail if the disk free call fails, but make note of it.
	if (dummyErr.IsntError())
	{
		mAreWeFailingDiskFree = FALSE;
		mKbFree = (PGPUInt32) (bytesFree / kBytesPerKb) - 10;
	}
	else
	{
		mAreWeFailingDiskFree = TRUE;
		mKbFree = App->mPlatformMaxKbSize*2;	// fake max amount of space
	}

	// If the pathname contains the name of an existing file, the user wants
	// to replace it, so add its size to the amount of free space.

	dummyErr = existingFile.Open(mParentDiskWiz->mPath, 
		kOF_MustExist | kOF_ReadOnly);

	if (dummyErr.IsntError())
	{
		existingFile.GetLength(&bytesFile);
	}

	if (dummyErr.IsntError())
	{
		mKbFree += (PGPUInt32) (bytesFile/kBytesPerKb);
	}

	if (existingFile.Opened())
		existingFile.Close();

	// Calculate max and min PGPdisk sizes we can create.
	mLocalMaxKbSize	= min(mKbFree, App->mPlatformMaxKbSize);
	mLocalMinKbSize	= min(mKbFree, App->mPlatformMinKbSize);

	// If max size is less than one meg, disable the Mb button.
	if (mLocalMaxKbSize < kSizeIsCloseRatio*kKbPerMeg)
		mMbButton.EnableWindow(FALSE);

	// If max size is less than one gig, disable the Gb button.
	if (mLocalMaxKbSize < kSizeIsCloseRatio*kKbPerGig)
		mGbButton.EnableWindow(FALSE);

	pgpAssert(mLocalMaxKbSize >= mLocalMinKbSize);

	// Set PGPdisk size to what we remember it as in the registry.
	GetPGPdiskSizeFromRegistry();

	try
	{
		CString freeSpaceStr, maxPGPdiskStr;

		FormatSizeString(mKbFree, freeSpaceStr.GetBuffer(kMaxStringSize), 
			kMaxStringSize);

		freeSpaceStr.ReleaseBuffer();

		FormatSizeString(mLocalMaxKbSize, 
			maxPGPdiskStr.GetBuffer(kMaxStringSize), kMaxStringSize);

		maxPGPdiskStr.ReleaseBuffer();
 
		// Update the contents of the free size message shown to the user.
		if (mAreWeFailingDiskFree)
		{
			mFreeSizeText.Format(GetCommonString(kPGPdiskFreeSize3MsgString), 
				maxPGPdiskStr);
		}
		else if (mLocalMaxKbSize == App->mPlatformMaxKbSize)
		{
			mFreeSizeText.Format(GetCommonString(kPGPdiskFreeSize2MsgString), 
				freeSpaceStr, maxPGPdiskStr);
		}
		else
		{
			mFreeSizeText.Format(GetCommonString(kPGPdiskFreeSize1MsgString), 
				freeSpaceStr);
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	UpdateData(FALSE);

	return TRUE;
}

// We override OnPaint to draw the sidebar graphic at the correct depth.

void 
CDiskWizVolumeInfoPage::OnPaint() 
{
	CDC			memDC;
	CPaintDC	DC(this);
	HBITMAP		oldBitmap;
	RECT		sidebarRect;

	DC.SelectPalette(&mParentDiskWiz->mSidebarPalette, FALSE);
	DC.RealizePalette();

	memDC.CreateCompatibleDC(&DC);
	memDC.SelectPalette(&mParentDiskWiz->mSidebarPalette, FALSE);
	memDC.RealizePalette();

	mSidebarGraphic.GetWindowRect(&sidebarRect);
	ScreenToClient(&sidebarRect);
	
	// MFC bug - CDC::SelectObject returns incorrect pointers/handles so don't
	// fool with it.

	oldBitmap = (HBITMAP) SelectObject(memDC, mParentDiskWiz->mSidebarBitmap);

	DC.BitBlt(sidebarRect.left, sidebarRect.top, sidebarRect.right, 
		sidebarRect.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(oldBitmap);
	memDC.DeleteDC();
}
