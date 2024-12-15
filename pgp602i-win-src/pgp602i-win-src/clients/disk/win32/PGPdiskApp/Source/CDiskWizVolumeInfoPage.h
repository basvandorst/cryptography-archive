//////////////////////////////////////////////////////////////////////////////
// CDiskWizVolumeInfoPage.h
//
// Declaration of class CDiskWizVolumeInfoPage.
//////////////////////////////////////////////////////////////////////////////

// $Id: CDiskWizVolumeInfoPage.h,v 1.1.2.8 1998/07/31 21:36:47 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_CDiskWizVolumeInfoPage_h	// [
#define Included_CDiskWizVolumeInfoPage_h

#include "PGPdiskRegistry.h"

#include "CDriveLetterCombo.h"
#include "Resource.h"


////////
// Types
////////

class CDiskWizardSheet;


///////////////////////////////
// Class CDiskWizVolumeInfoPage
///////////////////////////////

class CDiskWizVolumeInfoPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDiskWizVolumeInfoPage)

public:
	CDiskWizardSheet	*mParentDiskWiz;	// pointer to parent sheet
	PGPUInt64			mBlocksDisk;		// user wants disk this big

	CDiskWizVolumeInfoPage();
	~CDiskWizVolumeInfoPage();

	//{{AFX_DATA(CDiskWizVolumeInfoPage)
	enum { IDD = IDD_DISKWIZ_VOLINFO };
	CDriveLetterCombo	mDriveCombo;
	CStatic	mSidebarGraphic;
	CSpinButtonCtrl	mSizeSpin;
	CEdit	mSizeEditBox;
	UINT	mSizeValue;
	CString	mFreeSizeText;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CDiskWizVolumeInfoPage)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	//}}AFX_VIRTUAL

protected:
	CButton				mKbButton;			// KB radio button
	CButton				mMbButton;			// MB radio button
	CButton				mGbButton;			// GB radio button

	PGPUInt32			mLocalMaxKbSize;	// max size PGPdisk we can make
	PGPUInt32			mLocalMinKbSize;	// min size PGPdisk we can make
	PGPUInt32			mKbFree;			// KB free on PGPdisk host

	PGPBoolean			mAreWeFailingDiskFree;	// disk free calls failing?
	PGPdiskCreateScale	mSizeScale;			// scale of displayed size

	void		ConvertControlsAux(PGPUInt32 nMin, PGPUInt32 nMax, 
					PGPUInt32 kbPerUnit);

	void		ConvertControlsToKb();
	void		ConvertControlsToMb();
	void		ConvertControlsToGb();

	PGPUInt32	GetSizeValueInKb();
	void		SetPGPdiskSize(PGPUInt32 kbSize, 
					PGPdiskCreateScale scale = kLCS_InvalidScale);

	void		GetPGPdiskSizeFromRegistry();
	DualErr		SetPGPdiskSizeInRegistry();

	//{{AFX_MSG(CDiskWizVolumeInfoPage)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO *pHelpInfo);
	afx_msg void OnKbCheck();
	afx_msg void OnMbCheck();
	afx_msg void OnPaint();
	afx_msg void OnGbCheck();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif	// ] Included_CDiskWizVolumeInfoPage_h
