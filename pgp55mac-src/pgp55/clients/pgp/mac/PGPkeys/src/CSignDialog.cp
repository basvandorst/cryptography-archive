/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CSignDialog.cp,v 1.27.4.1 1997/11/06 12:01:47 wprice Exp $
____________________________________________________________________________*/
#include "CSignDialog.h"

#include "CKeyTable.h"
#include "CKeyInfoWindow.h"
#include "CWarningAlert.h"
#include "ResourceConstants.h"
#include "GetPassphrase.h"
#include "CPGPKeys.h"
#include "CSecureMemory.h"

#include "pgpUserInterface.h"

#include "pgpMem.h"
#include "pgpErrors.h"
#include "MacStrings.h"
#include "pgpClientPrefs.h"
#include <string.h>

const Int16		kUserIDColumnWidth		=	220;
const Int16		kFingerprintColumnWidth	=	280;
const Int16		kUserIDStringLength		=	256;

enum	{
			kStringListID			= 1019,
			kAdvancedOptionsStringID= 1,
			kFewerOptionsStringID,
			kNameColumnStringID,
			kFingerprintColumnStringID,
			kDuplicateCertID,
			kEnterPassphrasePromptStringID
		};

class CUserIDList	:	public LTableView
{
public:
	enum { class_ID = 'uidL' };
	static CUserIDList*	CreateFromStream(LStream *inStream);
	
							CUserIDList(LStream *inStream);
	void					DrawSelf();
	void					DrawCell(
								const STableCell	&inCell,
								const Rect			&inLocalRect);
	void					HiliteCellActively(
								const STableCell	&inCell,
								Boolean				inHilite);
	void					HiliteCellInactively(
								const STableCell	&inCell,
								Boolean			 	inHilite);
	void					ClickSelf(
								const SMouseDownEvent &inMouseDown);
};


class	CUserIDHeaders	:	public LPane
{
public:
	enum { class_ID = 'uidH' };

						CUserIDHeaders(LStream *inStream);
	virtual				~CUserIDHeaders();
	void				DrawSelf();
};

CSignDialog::CSignDialog()
{
	RegisterClass_(CUserIDList);
	RegisterClass_(CUserIDHeaders);
}

CSignDialog::CSignDialog(LStream *inStream)
	: LGADialogBox(inStream)
{
	RegisterClass_(CUserIDList);
	RegisterClass_(CUserIDHeaders);
}

CSignDialog::~CSignDialog()
{
	if(IsntNull(mUserIDs))
		pgpFree(mUserIDs);
}

	void
CSignDialog::FinishCreateSelf()
{
	LGADialogBox::FinishCreateSelf();
	
	mUserIDs = NULL;
	mAdvancedMode = FALSE;
	mExportCheckbox = (LGACheckbox *) FindPaneByID(kExportCheckbox);
	pgpAssertAddrValid(mExportCheckbox,	LGACheckbox);
	mAdvancedButton = (LGAPushButton *) FindPaneByID(kAdvancedButton);
	pgpAssertAddrValid(mAdvancedButton,	LGAPushButton);
	mAdvancedButton->AddListener(this);
	mSignButton = (LGAPushButton *) FindPaneByID(kSignButton);
	pgpAssertAddrValid(mSignButton,	LGAPushButton);
	mCancelButton = (LGAPushButton *) FindPaneByID(kCancelButton);
	pgpAssertAddrValid(mCancelButton,	LGAPushButton);
	mSignTypesGroup = (LGAPrimaryGroup *) FindPaneByID(kSignTypesGroup);
	pgpAssertAddrValid(mSignTypesGroup,	LGAPrimaryGroup);
	mSignN0Radio = (LGARadioButton *) FindPaneByID(kSignTypeN0Radio);
	pgpAssertAddrValid(mSignN0Radio,	LGARadioButton);
	mSignE0Radio = (LGARadioButton *) FindPaneByID(kSignTypeE0Radio);
	pgpAssertAddrValid(mSignE0Radio,	LGARadioButton);
	mSignE1Radio = (LGARadioButton *) FindPaneByID(kSignTypeE1Radio);
	pgpAssertAddrValid(mSignE1Radio,	LGARadioButton);
	mSignN2Radio = (LGARadioButton *) FindPaneByID(kSignTypeN2Radio);
	pgpAssertAddrValid(mSignN2Radio,	LGARadioButton);
	mScroller = (LActiveScroller *) FindPaneByID(kUserIDScroller);
	pgpAssertAddrValid(mScroller,	LActiveScroller);
}

	void
CSignDialog::SetUserIDs(PGPKeyRef		signer,
						KeyTableRef		*userIDs,
						Int32			numUserIDs)
{
	CUserIDList *uidList;
	
	mSigner = signer;
	mUserIDs = userIDs;
	mNumUserIDs = numUserIDs;
	uidList = (CUserIDList *)FindPaneByID(kUserIDList);
	uidList->InsertCols(1, 1, NULL, 0, TRUE);
	for(Int16 inx=0;inx<numUserIDs;inx++)
	{
		uidList->InsertRows(1, 32767,
			&userIDs[inx],
			sizeof(KeyTableRef),
			true);
	}
#if (PGP_BUSINESS_SECURITY == 0)
	mAdvancedButton->Disable();
#endif
	Show();
}

	void
CSignDialog::DrawSelf()
{
	Rect	frame;
	
	LGADialogBox::DrawSelf();
	FocusDraw();
	mScroller->CalcPortFrameRect(frame);
	::InsetRect(&frame, -1, -1);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
	::MoveTo(frame.left, frame.bottom-1);
	::LineTo(frame.left, frame.top - 16);
	::LineTo(frame.right - 1, frame.top - 16);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::MoveTo(frame.left+1, frame.bottom - 1);
	::LineTo(frame.right - 1, frame.bottom - 1);
	::LineTo(frame.right - 1, frame.top - 15);	
}

	void
CSignDialog::ListenToMessage(	MessageT inMessage,
								void */*ioParam*/)
{
	switch(inMessage)
	{
		case kSignButton:
			Hide();
			{
				PGPError	err;
				Boolean		dupeWarning = FALSE,
							exportable,
							success = FALSE;
				PGPBoolean	ksSync;
				char		promptCStr[256];
				Str255		promptStr;
				PGPUInt32	trustLevel;
				PGPGetPassphraseSettings	signDialogFlags;
				CSecureCString256			passphrase;
				
				if(mAdvancedMode)
				{
					if(mSignN0Radio->GetValue())
					{
						trustLevel = 0;
						exportable = FALSE;
					}
					else if(mSignE0Radio->GetValue())
					{
						trustLevel = 0;
						exportable = TRUE;
					}
					else if(mSignE1Radio->GetValue())
					{
						trustLevel = 1;
						exportable = TRUE;
					}
					else if(mSignN2Radio->GetValue())
					{
						trustLevel = 2;
						exportable = FALSE;
					}
				}
				else
				{
					trustLevel = 0;
					if(mExportCheckbox->GetValue())
						exportable = TRUE;
					else
						exportable = FALSE;
				}
				err = PGPGetPrefBoolean(gPrefRef,
							kPGPPrefKeyServerSyncOnKeySign, &ksSync);
				pgpAssertNoErr(err);
				
				::GetIndString(	promptStr,
								kStringListID,
								kEnterPassphrasePromptStringID);
				PToCString(promptStr, promptCStr);
				UDesktop::Deactivate();
				err = PGPGetSigningPassphraseDialog
						(	gPGPContext,
							CPGPKeys::TheApp()->GetKeySet(),
							promptCStr,
							kPGPGetPassphraseOptionsHideFileOptions,
							0,
							mSigner,
							passphrase,
							&signDialogFlags,
							&mSigner );
				UDesktop::Activate();
				
				if(IsntPGPError(err))
				{
					Int32			uIndex;
					PGPUserIDRef	userID;
					
					for(uIndex = mNumUserIDs - 1; uIndex >= 0; uIndex--) 
					{
						if(mUserIDs[uIndex].type == kKey)
						{
							err = PGPGetPrimaryUserID(mUserIDs[uIndex].u.key,
														&userID);
							pgpAssertNoErr(err);
						}
						else if(mUserIDs[uIndex].type == kUserID)
							userID = mUserIDs[uIndex].u.user;
					certifyAgain:
						CPGPKeys::TheApp()->GetMinimumRandomData();
						err = PGPSignUserID(userID, mSigner,
							PGPOPassphrase(gPGPContext, passphrase),
							PGPOExpiration(gPGPContext, 0),
							PGPOExportable(gPGPContext, exportable),
							PGPOSigTrust(gPGPContext, trustLevel,
												kPGPKeyTrust_Complete),
							PGPOLastOption(gPGPContext));
						if(err == kPGPError_DuplicateCert)
						{
							if(!dupeWarning)
							{
								CWarningAlert::Display(kWACautionAlertType,
												kWAOKStyle,
												kStringListID,
												kDuplicateCertID);
								dupeWarning = TRUE;
							}
						}
						else
						{
							pgpAssertNoErr(err);
							if(IsntPGPError(err))
								success = TRUE;
							if(ksSync && exportable)
							{
								err = PGPSendKeyToServer(
									gPGPContext, mUserIDs[uIndex].ownerKey, 0);
								if(IsPGPError(err) &&
									(err != kPGPError_UserAbort))
									ReportPGPError(err);
							}
						}
					}
					if(success)
					{
						CPGPKeys::TheApp()->CommitDefaultKeyrings();
						BroadcastMessage(kKeyTableResyncMessage, NULL);
					}
				}
			}
		case kCancelButton:
			delete this;
			break;
		case kAdvancedButton:
			{
				Rect		windBounds;
				Str255		str;
				
				if(mAdvancedMode)
				{
					mSignTypesGroup->Hide();
					mExportCheckbox->Show();
					GetMinMaxSize(windBounds);
					ResizeWindowTo(windBounds.right, windBounds.top);
					GetIndString(	str,
									kStringListID,
									kAdvancedOptionsStringID);
				}
				else
				{
					mExportCheckbox->Hide();
					mSignTypesGroup->Show();
					CalcStandardBounds(windBounds);
					DoSetBounds(windBounds);
					GetIndString(	str,
									kStringListID,
									kFewerOptionsStringID);
				}
				mAdvancedButton->SetDescriptor(str);
				mAdvancedMode = !mAdvancedMode;
			}
			break;
	}
}

	void
CSignDialog::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	#pragma unused( inCommand, outUsesMark, outMark, outName )
	
	outEnabled = FALSE;
}

CUserIDList*
CUserIDList::CreateFromStream(
	LStream	*inStream)
{
	return (new CUserIDList(inStream));
}


CUserIDList::CUserIDList(
	LStream	*inStream)
		: LTableView(inStream)
{
	mTableGeometry = new LTableMonoGeometry(this, mFrameSize.width, 18);
	mTableSelector = new LTableSingleSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(KeyTableRef));
}

	void
CUserIDList::DrawSelf()
{
	Rect frame;

	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
	::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
	CalcLocalFrameRect(frame);
	{
		StDeviceLoop	devLoop(frame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth))
		{
			if(depth > 1)
				::PaintRect(&frame);
			else
				::EraseRect(&frame);
		}
	}
	LTableView::DrawSelf();
}

	void
CUserIDList::DrawCell(
	const STableCell	&inCell,
	const Rect			&inLocalRect)
{
	StDeviceLoop	devLoop(inLocalRect);		
	SInt16			depth;
	Uint32			dataSize = sizeof(KeyTableRef);
	PGPError		err;
	KeyTableRef		keyRef;
	PGPUserIDRef	userID;
	Str255			str;
	size_t			len;
	Rect			sortColumnRect;
	PGPInt32		algorithm;

	while(devLoop.NextDepth(depth))
	{
		if(depth >= 8)
		{
			// Draw MacOS8-like background for cells, selected column
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
			::PaintRect(&inLocalRect);
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
			sortColumnRect = inLocalRect;
			sortColumnRect.right = inLocalRect.left + kUserIDColumnWidth;
			::PaintRect(&sortColumnRect);
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 1);
			::LineTo(inLocalRect.right - 1, inLocalRect.bottom - 1);
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
		}
		else
		{
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
			::EraseRect(&inLocalRect);
		}
		GetCellData(inCell, &keyRef, dataSize);
		if(keyRef.type == kKey)
		{
			err = PGPGetPrimaryUserID(keyRef.u.key, &userID);
			pgpAssertNoErr(err);
		}
		else if(keyRef.type == kUserID)
			userID = keyRef.u.user;
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
		::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
		err = PGPGetKeyNumber(keyRef.ownerKey, kPGPKeyPropAlgID,
								&algorithm);
		pgpAssertNoErr(err);
		CKeyTable::DrawIcon((algorithm == kPGPPublicKeyAlgorithm_DSA) ?
						kDHUserIDIconID : kRSAUserIDIconID, inLocalRect, -1);
		err = PGPGetUserIDStringBuffer(userID, kPGPUserIDPropName,
				sizeof(Str255) - 1, (char *)&str[1], &len);
		pgpAssertNoErr(err);
		str[0] = len;
		::TextSize(9);
		::TextFont(geneva);
		::TextFace(0);
		::TruncString(	kUserIDColumnWidth - 22, str, truncMiddle);
		::MoveTo(inLocalRect.left + 22, inLocalRect.bottom - 5);
		::DrawString(str);
		::MoveTo(	inLocalRect.left + kUserIDColumnWidth + 4,
					inLocalRect.bottom -5);
		CKeyInfoWindow::GetFingerprintString(str, keyRef.ownerKey);
		::DrawString(str);
	}
}

	void
CUserIDList::HiliteCellActively(
	const STableCell	&inCell,
	Boolean				inHilite)
{
	#pragma unused( inCell, inHilite )
}

	void
CUserIDList::HiliteCellInactively(
	const STableCell	&inCell,
	Boolean			 	inHilite)
{
	#pragma unused( inCell, inHilite )
}

	void
CUserIDList::ClickSelf(
	const SMouseDownEvent &inMouseDown)
{
	#pragma unused( inMouseDown )
}

CUserIDHeaders::CUserIDHeaders(LStream *inStream)
		: LPane(inStream)
{
}

CUserIDHeaders::~CUserIDHeaders()
{
}

	void
CUserIDHeaders::DrawSelf()
{
	Rect				frame;
	Str255				title;
	
	FocusDraw();
	CalcLocalFrameRect(frame);
	ApplyForeAndBackColors();
	::FrameRect(&frame);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::MoveTo(frame.left+1, frame.bottom-1);
	::LineTo(frame.left+1, frame.top+1);
	::LineTo(frame.right-2, frame.top+1);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
	::MoveTo(frame.left+2, frame.bottom-1);
	::LineTo(frame.right-2, frame.bottom-1);
	::LineTo(frame.right-2, frame.top+2);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
	::TextFont(geneva);
	::TextSize(9);
	::TextFace(bold);
		
	::GetIndString(title, kStringListID, kNameColumnStringID);
	::MoveTo(frame.left + 4, frame.top + 12);
	::DrawString(title);
	::GetIndString(title, kStringListID, kFingerprintColumnStringID);
	::MoveTo(frame.left + kUserIDColumnWidth + 4, frame.top + 12);
	::DrawString(title);
}

