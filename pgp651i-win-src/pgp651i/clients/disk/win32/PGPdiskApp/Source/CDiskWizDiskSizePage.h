//////////////////////////////////////////////////////////////////////////////
// CDiskWizDiskSizePage.h
//
// Declaration of class CDiskWizDiskSizePage
//////////////////////////////////////////////////////////////////////////////

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDiskWizDiskSizePage_h	// [
#define Included_CDiskWizDiskSizePage_h

#include <Afxcmn.h>

#include "PGPdiskPfl.h"
#include "Resource.h"


////////
// Types
////////

class CDiskWizardSheet;


/////////////////////////////
// Class CDiskWizDiskSizePage
/////////////////////////////

class CDiskWizDiskSizePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDiskWizDiskSizePage)

public:
	CDiskWizardSheet	*mParentDiskWiz;	// pointer to parent sheet
	PGPUInt32			mBlocksData;		// user wants disk this big

	CDiskWizDiskSizePage();
	~CDiskWizDiskSizePage();

	//{{AFX_DATA(CDiskWizDiskSizePage)
	enum { IDD = IDD_DISKWIZ_DISKSIZE };
	CStatic	mSidebarGraphic;
	CSpinButtonCtrl	mSizeSpin;
	CEdit	mSizeEditBox;
	CSliderCtrl	mSizeSlider;
	UINT	mSizeValue;
	CString	mMaxSizeText;
	CString	mMinSizeText;
	CString	mFreeSizeText;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CDiskWizDiskSizePage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	CButton		mKbButton;			// kB radio button
	CButton		mMbButton;			// MB radio button

	double		mUnitsPerTick;		// kB or MB per slider tick

	PGPBoolean	mIsSizeInMb;		// is size field in MB?
	PGPBoolean	mAreWeFailingDiskFree;		// are disk free calls failing?
	PGPUInt32	mKbFree;			// kB free on PGPdisk host
	PGPUInt32	mLocalMaxKbSize;	// based on mKbFree

	void	ConvertControlsToKb();
	void	ConvertControlsToMb();

	void	SetPGPdiskSize(PGPUInt32 size);

	void	GetPGPdiskSizeFromRegistry();
	DualErr	SetPGPdiskSizeInRegistry();

	void	UpdateSizeFromSlider();
	void	UpdateSliderFromSize();

	//{{AFX_MSG(CDiskWizDiskSizePage)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKbCheck();
	afx_msg void OnMbCheck();
	afx_msg void OnChangeSizeEdit();
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDiskWizDiskSizePage_h
