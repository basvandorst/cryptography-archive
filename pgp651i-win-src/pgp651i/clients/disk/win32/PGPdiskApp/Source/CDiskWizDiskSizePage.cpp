//////////////////////////////////////////////////////////////////////////////
// CDiskWizDiskSizePage.cpp
//
// Implementation of class CDiskWizDiskSizePage
//////////////////////////////////////////////////////////////////////////////

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#include "StdAfx.h"
#include <math.h>

#include "PGPdiskPfl.h"
#include "PGPdiskPrefs.h"
#include "UtilityFunctions.h"

#include "CDiskWizardSheet.h"
#include "CDiskWizDiskSizePage.h"
#include "File.h"
#include "Globals.h"
#include "PGPdiskHelpIds.h"


////////////
// Constants
////////////

const PGPUInt32 kMaxSliderTicks = 40;

static LPCSTR kFreeSize1MsgString = 
	"You have %s of free space available, but the largest size PGPdisk you "
	"can create here will be slightly smaller.";

static LPCSTR kFreeSize2MsgString = 
	"You have %s of free space available, but the largest size PGPdisk you "
	"can create here will be %s.";

static LPCSTR kFreeSize3MsgString = 
	"PGPdisk was unable to determine the amount of available free space. "
	"The maximum possible size for a PGPdisk is %s.";


/////////////////////
// Context help array
/////////////////////

static PGPUInt32 HelpIds[] =
{
	IDC_SIZE_EDIT,			IDH_PGPDISKAPP_DISKWIZSIZESIZEEDIT, 
	IDC_SIZE_SPIN,			IDH_PGPDISKAPP_DISKWIZSIZESIZESPIN, 
	IDC_MB_CHECK,			IDH_PGPDISKAPP_DISKWIZSIZEMBCHECK, 
	IDC_KB_CHECK,			IDH_PGPDISKAPP_DISKWIZSIZEKBCHECK, 
	IDC_SIZE_SLIDER,		IDH_PGPDISKAPP_DISKWIZSIZESLIDER, 
	IDC_MINSIZE,			IDH_PGPDISKAPP_DISKWIZSIZEMINTEXT, 
	IDC_MAXSIZE,			IDH_PGPDISKAPP_DISKWIZSIZEMAXTEXT, 
    IDC_FREESIZE_TEXT,		((PGPUInt32) -1), 
	IDC_SIDEBAR,			((PGPUInt32) -1), 
	0,0 
};


///////////////////////////
// MFC specific definitions
///////////////////////////

BEGIN_MESSAGE_MAP(CDiskWizDiskSizePage, CPropertyPage)
	//{{AFX_MSG_MAP(CDiskWizDiskSizePage)
	ON_WM_HELPINFO()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_KB_CHECK, OnKbCheck)
	ON_BN_CLICKED(IDC_MB_CHECK, OnMbCheck)
	ON_EN_CHANGE(IDC_SIZE_EDIT, OnChangeSizeEdit)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


////////////////////////////////////////////////////////////////////////
// CDiskWizDiskSizePage public custom functions and non-default handlers
////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CDiskWizDiskSizePage, CPropertyPage)

// The CDiskWizDiskSizePage default constructor.

CDiskWizDiskSizePage::CDiskWizDiskSizePage()
	 : CPropertyPage(CDiskWizDiskSizePage::IDD)
{
	//{{AFX_DATA_INIT(CDiskWizDiskSizePage)
	mSizeValue = 0;
	mMaxSizeText = _T("");
	mMinSizeText = _T("");
	mFreeSizeText = _T("");
	//}}AFX_DATA_INIT

	mParentDiskWiz = NULL;

	// Exorcise the big help button.
	m_psp.dwFlags &= ~PSP_HASHELP;
}

// The CDiskWizDiskSizePage destructor.

CDiskWizDiskSizePage::~CDiskWizDiskSizePage()
{
}

// PreTranslateMessage sees all messages before MFC filters them. We switch
// the up and down arrows in the slider.

BOOL 
CDiskWizDiskSizePage::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (GetFocus()->GetSafeHwnd() == mSizeSlider.GetSafeHwnd())
		{
			PGPUInt8 vCode = pMsg->wParam & 0x7F;

			switch (vCode)
			{
			case VK_UP:
				pMsg->wParam = VK_DOWN;
				break;

			case VK_DOWN:
				pMsg->wParam = VK_UP;
				break;
			}
		}
		break;
	}

	return CPropertyPage::PreTranslateMessage(pMsg);
}


///////////////////////////////////////////////////////
// CDiskWizDiskSizePage public default message handlers
///////////////////////////////////////////////////////

// OnSetActive is called when this page is made active. We make sure the
// correct buttons are enabled/disabled.

BOOL 
CDiskWizDiskSizePage::OnSetActive() 
{
	pgpAssertAddrValid(mParentDiskWiz, CDiskWizardSheet);
	mParentDiskWiz->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	
	return CPropertyPage::OnSetActive();
}

// OnWizardNext is called when the 'Next' button is pressed. We perform
// data validation.

LRESULT 
CDiskWizDiskSizePage::OnWizardNext() 
{
	try
	{
		PGPBoolean	failThis	= FALSE;
		PGPUInt32	kbData;

		if (CPropertyPage::OnWizardNext() == -1)
			return -1;

		// Get the size value in kilobytes.
		if (mIsSizeInMb)
			kbData = mSizeValue*kKbPerMeg;
		else
			kbData = mSizeValue;

		// Verify that the space being asked is not too big or small.
		if (kbData < kMinPGPdiskKbSize)
		{
			SetPGPdiskSize(kMinPGPdiskKbSize);

			App->ReportError(kPGDMajorError_PGPdiskTooSmall);
			failThis = TRUE;
		}
		else if (kbData > kMaxPGPdiskKbSize)
		{
			SetPGPdiskSize(mLocalMaxKbSize);

			App->ReportError(kPGDMajorError_PGPdiskTooBig);
			failThis = TRUE;
		}
		else if (kbData > mLocalMaxKbSize)
		{
			SetPGPdiskSize(mLocalMaxKbSize);

			App->ReportError(kPGDMajorError_NotEnoughDiskSpace);
			failThis = TRUE;
		}

		// Handle special case where we can't be sure of the amount of free
		// space on the host drive.

		if (mAreWeFailingDiskFree && !failThis)
		{
			CString		dir;
			DualErr		dummyErr;
			PGPBoolean	isEnoughSpace;

			// Get the directory to test.
			dummyErr = App->GetDirectory(mParentDiskWiz->mPath, &dir);

			// Test if there really is enough space available. If we can't
			// tell then don't fail.

			if (dummyErr.IsntError())
			{
				dummyErr = App->BruteForceTestIfEnoughSpace(dir, 
					EstimateDiskSizeFromDataSize(kbData), &isEnoughSpace);
			}

			if (dummyErr.IsntError())
			{
				if (!isEnoughSpace)
				{
					App->ReportError(kPGDMajorError_NotEnoughDiskSpace);
					failThis = TRUE;
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

			mBlocksData = kbData*kBytesPerKb/kDefaultBlockSize;
			return 0;
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
		return 0;
	}
}



///////////////////////////////////////////////////////////////////////////
// CDiskWizDiskSizePage protected custom functions and non-default handlers
///////////////////////////////////////////////////////////////////////////

// ConvertControlsToKb prepares the controls to show KB sizes.

void 
CDiskWizDiskSizePage::ConvertControlsToKb()
{
	PGPUInt32 nMin, nMax, numTicks;

	mKbButton.SetCheck(1);
	mMbButton.SetCheck(0);

	mIsSizeInMb = FALSE;

	nMin	= kMinPGPdiskKbSize;
	nMax	= min(kKbPerMeg, mLocalMaxKbSize);

	// If current size is out of range, change it.
	UpdateData(TRUE);

	if (mSizeValue < nMin)
	{
		mSizeValue = nMin;
	}
	else if (mSizeValue >nMax)
	{
		mSizeValue = nMax;
	}

	UpdateData(FALSE);

	// Set the slider range.
	numTicks		= min(kMaxSliderTicks, nMax - nMin);
	mUnitsPerTick	= (double) (nMax - nMin)/numTicks;

	mSizeSlider.SetRange(0, numTicks);

	// Set the spinner range.
	mSizeSpin.SetRange(nMin, nMax);

	// Reposition the slider.
	UpdateSliderFromSize();

	// Update the min/max static fields.
	try
	{
		mMinSizeText.Format("%dK", nMin);
		mMaxSizeText.Format("%dK", nMax);
		UpdateData(FALSE);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// ConvertControlsToMb prepares the controls to show MB sizes.

void 
CDiskWizDiskSizePage::ConvertControlsToMb()
{
	PGPUInt32 nMin, nMax, numTicks;

	mKbButton.SetCheck(0);
	mMbButton.SetCheck(1);

	mIsSizeInMb = TRUE;

	nMin	= 1;
	nMax	= min(kMaxPGPdiskKbSize/kKbPerMeg, mLocalMaxKbSize/kKbPerMeg);

	// If current size is out of range, change it.
	UpdateData(TRUE);
	if (mSizeValue < nMin)
	{
		mSizeValue = nMin;
	}
	else if (mSizeValue >nMax)
	{
		mSizeValue = nMax;
	}
	UpdateData(FALSE);

	// Set the slider range.
	numTicks		= min(kMaxSliderTicks, nMax - nMin);
	mUnitsPerTick	= (double) (nMax - nMin)/numTicks;

	mSizeSlider.SetRange(0, numTicks);

	// Set the spinner range.
	mSizeSpin.SetRange(nMin, nMax);

	// Reposition the slider.
	UpdateSliderFromSize();

	// Update the min/max static fields.
	try
	{
		mMinSizeText.Format("%dMB", nMin);
		mMaxSizeText.Format("%dMB", nMax);
		UpdateData(FALSE);
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

}

// SetPGPdiskSize sets the size edit box and the slider to the given size.

void 
CDiskWizDiskSizePage::SetPGPdiskSize(PGPUInt32 kbSize)
{
	// Set size to legal value.
	if (kbSize < kMinPGPdiskKbSize)
	{
		kbSize = kMinPGPdiskKbSize;
	}
	else if (kbSize > mLocalMaxKbSize)
	{
		kbSize = mLocalMaxKbSize;
	}

	// More than or less than 1 meg in size?
	if (kbSize >= kKbPerMeg)
	{
		mSizeValue = kbSize/kKbPerMeg;
		UpdateData(FALSE);

		ConvertControlsToMb();
	}
	else
	{
		mSizeValue = kbSize;
		UpdateData(FALSE);

		ConvertControlsToKb();
	}
}

// GetPGPdiskSizeFromRegistry initializes the size from the value we
// previously stored in the registry.

void 
CDiskWizDiskSizePage::GetPGPdiskSizeFromRegistry()
{
	PGPdiskWin32Prefs	prefs;

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
		SetPGPdiskSize(prefs.lastCreateSize);
}

// SetPGPdiskSizeInRegistry stores the current PGPdisk size in the registry.

DualErr 
CDiskWizDiskSizePage::SetPGPdiskSizeInRegistry()
{
	DualErr				derr;
	PGPdiskWin32Prefs	prefs;
	PGPUInt32			kbSize;

	UpdateData(TRUE);

	kbSize = mSizeValue;

	if (mIsSizeInMb)
		kbSize *= kKbPerMeg;

	derr = GetPGPdiskWin32Prefs(prefs);

	if (derr.IsntError())
	{
		prefs.lastCreateSize = kbSize;
		derr = SetPGPdiskWin32Prefs(prefs);
	}

	return derr;
}

// UpdateSizeFromSlider updates the size edit box from the slider position.

void 
CDiskWizDiskSizePage::UpdateSizeFromSlider()
{
	PGPInt32 pos;
	
	pos = mSizeSlider.GetPos();

	mSizeValue	= (PGPUInt32) floor((double) pos*mUnitsPerTick + 0.5);
	mSizeValue	+= (mIsSizeInMb ? 1 : kMinPGPdiskKbSize);

	UpdateData(FALSE);
}

// UpdateSliderFromSize updates the slider position from the size edit box.

void 
CDiskWizDiskSizePage::UpdateSliderFromSize()
{
	PGPUInt32 temp;

	UpdateData(TRUE);

	temp	= mSizeValue;
	temp	-= (mIsSizeInMb ? 1 : kMinPGPdiskKbSize);

	mSizeSlider.SetPos((PGPUInt32) floor((double) temp/mUnitsPerTick + 0.5));
}

// The CDiskWizDiskSizePage data exchange function.

void 
CDiskWizDiskSizePage::DoDataExchange(CDataExchange *pDX)
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
		if (!pDX->m_bSaveAndValidate && text.IsEmpty && (mSizeValue == 0))
		{
			skipDDX = TRUE;
		}

		// Note the way the second if statement ends within the AFX_DATA_MAP.
		// This is done so our variable will still appear in the ClassWizard,
		// but allows us to skip the DDX_Text when we want to. Kewl!

		if (pDX->m_bSaveAndValidate && text.IsEmpty())
		{
			skipDDX = TRUE;
			mSizeValue = 0;
		}

		if (!skipDDX)		
		//{{AFX_DATA_MAP(CDiskWizDiskSizePage)
			DDX_Text(pDX, IDC_SIZE_EDIT, mSizeValue);
		DDX_Control(pDX, IDC_SIDEBAR, mSidebarGraphic);
		DDX_Control(pDX, IDC_SIZE_SPIN, mSizeSpin);
		DDX_Control(pDX, IDC_SIZE_EDIT, mSizeEditBox);
		DDX_Control(pDX, IDC_SIZE_SLIDER, mSizeSlider);
		DDX_Text(pDX, IDC_MAXSIZE, mMaxSizeText);
		DDX_Text(pDX, IDC_MINSIZE, mMinSizeText);
		DDX_Text(pDX, IDC_FREESIZE_TEXT, mFreeSizeText);
		//}}AFX_DATA_MAP
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}
}

// OnChangeSizeEdit is called when the user enters a new size in the size
// edit box. We update the slider bar accordingly.

void 
CDiskWizDiskSizePage::OnChangeSizeEdit() 
{
	if (mSizeEditBox.GetSafeHwnd())
		UpdateSliderFromSize();
}

// OnKbCheck is called when the kB button is clicked.

void 
CDiskWizDiskSizePage::OnKbCheck() 
{
	ConvertControlsToKb();	
}

// OnMbCheck is called when the MB button is clicked.

void 
CDiskWizDiskSizePage::OnMbCheck() 
{
	ConvertControlsToMb();	
}


//////////////////////////////////////////////////////////
// CDiskWizDiskSizePage protected default message handlers
//////////////////////////////////////////////////////////

// OnHelpInfo handles context-sensitive help.

BOOL 
CDiskWizDiskSizePage::OnHelpInfo(HELPINFO *pHelpInfo)
{
	if ((pHelpInfo->iContextType == HELPINFO_WINDOW) &&
		(pHelpInfo->iCtrlId != ((PGPUInt16) IDC_STATIC)))
	{
		::WinHelp((HWND) pHelpInfo->hItemHandle, App->m_pszHelpFilePath, 
			HELP_WM_HELP, (PGPUInt32) HelpIds);
	}

	return TRUE;
}

// OnHScroll is called when the user drags the slider thumb.

void 
CDiskWizDiskSizePage::OnHScroll(
	UINT		nSBCode, 
	UINT		nPos, 
	CScrollBar	*pScrollBar) 
{
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
	UpdateSizeFromSlider();
}

// OnInitDialog is overwritten to perform property page initialization.

BOOL 
CDiskWizDiskSizePage::OnInitDialog() 
{
	DualErr		dummyErr;
	File		existingFile;
	PGPUInt32	bytesFile;
	PGPUInt64	bytesFree;

	CPropertyPage::OnInitDialog();

	// This must be called first to make MFC happy.
	UpdateData(FALSE);

	// Classwizard won't subclasss our radio buttons (sniff).
	mKbButton.SubclassWindow(GetDlgItem(IDC_KB_CHECK)->GetSafeHwnd());
	mMbButton.SubclassWindow(GetDlgItem(IDC_MB_CHECK)->GetSafeHwnd());

	// How much space do we have available?
	dummyErr = App->HowMuchFreeSpace(mParentDiskWiz->mPath, &bytesFree);

	// Don't fail if the disk free call fails, but make note of it.
	if (dummyErr.IsntError())
	{
		mAreWeFailingDiskFree = FALSE;
		mKbFree = (PGPUInt32) (bytesFree / kBytesPerKb);
	}
	else
	{
		mAreWeFailingDiskFree = TRUE;
		mKbFree = kMaxPGPdiskKbSize*2;		// fake max amount of space
	}

	// If the pathname contains the name of an existing file, the user wants
	// to replace it, so add its size to the amount of free space.

	dummyErr = existingFile.mInitErr;

	if (dummyErr.IsntError())
	{
		dummyErr = existingFile.Open(mParentDiskWiz->mPath, 
			kOF_MustExist | kOF_ReadOnly);
	}

	if (dummyErr.IsntError())
	{
		existingFile.GetLength(&bytesFile);
	}

	if (dummyErr.IsntError())
	{
		mKbFree += bytesFile/kBytesPerKb;
	}

	if (existingFile.Opened())
	{
		existingFile.Close();
	}

	// Estimate roughly what size PGPdisk we can handle.
	mLocalMaxKbSize = (PGPUInt32) min(EstimateDataSizeFromDiskSize(mKbFree), 
		kMaxPGPdiskKbSize);

	// If this is less than one meg, disable the Mb button.
	if (mLocalMaxKbSize < kKbPerMeg)
		mMbButton.EnableWindow(FALSE);

	pgpAssert(mLocalMaxKbSize >= kMinPGPdiskKbSize);	// should've been checked

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
			mFreeSizeText.Format(kFreeSize3MsgString, maxPGPdiskStr);
		}
		else if (mLocalMaxKbSize == kMaxPGPdiskKbSize)
		{
			mFreeSizeText.Format(kFreeSize2MsgString, freeSpaceStr, 
				maxPGPdiskStr);
		}
		else
		{
			mFreeSizeText.Format(kFreeSize1MsgString, freeSpaceStr);
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
CDiskWizDiskSizePage::OnPaint() 
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

	oldBitmap = (HBITMAP) 
		::SelectObject(memDC, mParentDiskWiz->mSidebarBitmap);

	DC.BitBlt(sidebarRect.left, sidebarRect.top, sidebarRect.right, 
		sidebarRect.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(oldBitmap);
	memDC.DeleteDC();
}
