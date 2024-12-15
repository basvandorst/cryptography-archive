/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyTable.cp,v 1.87.4.1.4.1 1997/12/05 22:14:05 mhw Exp $
____________________________________________________________________________*/
#include "CKeyTable.h"
#include "CKeyTableLabels.h"
#include "CKeyView.h"
#include "CSearchWindow.h"
#include "CSignDialog.h"
#include "CKeyInfoWindow.h"
#include "CKeyDragTask.h"
#include "CPGPKeys.h"
#include "GetPassphrase.h"
#include "CWarningAlert.h"
#include "CPGPStDialogHandler.h"
#include "CPGPPreferenceDialog.h"
#include "ResourceConstants.h"
#include "MacStrings.h"
#include "MacDialogs.h"
#include "UDragAndDropUtils.h"
#include "ChoiceDialogBox.h"
#include "CKeyGenWizardDialog.h"
#include "CSecureMemory.h"

#include "pgpUserInterface.h"

// CDK Includes
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpFeatures.h"
#include "pgpUtilities.h"
#include "pgpClientPrefs.h"
#include "pgpAdminPrefs.h"
#include "pgpVersionHeader.h"
#include "pgpSDKPrefs.h"

#include <string.h>

const Int16		kDefaultColumnWidth		=	16;
const Int16		kRowHeight				=	18;
const Int16		kBarHeight				=	10;
const Int16 	kBarRightBorder			=	10;
const Int16	 	kBarLeftBorder			=	4;
const Int16		kMaxTrust				=	3;
const Int16		kMaxValidity			=	2;
const Int32		kMaxKeyBuffer			=	MAX_PGPInt32;
const ResIDT	kBarberIconID			=	4000;
const ResIDT	kExportCustomPutDialog	=	1004;
const Int16		kUserIDStringLength		=	256;

const Int16		kUserValueMarked		=	1;
const Int16		kUserValueUnmarked		=	0;

const ResIDT	dialog_AddUserID		= 138;
const ResIDT	dialog_KeyGenWizard		= 150;

const UInt32	kDefaultKeyBufferLength	= 32767;

static char		hexDigit[] = "0123456789ABCDEF";

NMRec				sImportNotification;
ProcessSerialNumber	sThisProcess;
Str255				sNotifyString;

KeyTableColumnInfo	CKeyTable::kDefaultColumnInfo	=
	{
#if PGP_BUSINESS_SECURITY
		4,
		kUserIDColumnID,
		kValidityColumnID,
		kKeySizeColumnID,
		kDescriptionColumnID,
		0,0,0,0,0,
#else
		5,
		kUserIDColumnID,
		kValidityColumnID,
		kTrustColumnID,
		kKeySizeColumnID,
		kDescriptionColumnID,
		0,0,0,0,
#endif
		
		286, 53, 53, 81, 71, 71, 71, 35, 100
	};

Int16 MatchUserIDs(char *uid1, char *uid2);

// The following small class fixes a PowerPlant bug in
// LTableArrayStorage.  The PowerPlant class does not
// properly support LHierarchyTable

class CHierarchyTableArrayStorage	:	public LTableArrayStorage
{
public:
						CHierarchyTableArrayStorage(
								LTableView			*inTableView,
								Uint32				inDataSize);
	virtual void		InsertCols(
								Uint32				inHowMany,
								TableIndexT			inAfterCol,
								const void			*inDataPtr,
								Uint32				inDataSize);
	virtual void		RemoveCols(
								Uint32				inHowMany,
								TableIndexT			inFromCol);
};

CHierarchyTableArrayStorage::CHierarchyTableArrayStorage(
	LTableView			*inTableView,
	Uint32				inDataSize )
		:	LTableArrayStorage(inTableView, inDataSize)
{
}

	void
CHierarchyTableArrayStorage::RemoveCols(
	Uint32		inHowMany,
	TableIndexT	inFromCol)
{
	TableIndexT			rows,
						cols;
	LHierarchyTable		*hTable;
	
	hTable = (LHierarchyTable *)mTableView;
	hTable->GetWideOpenTableSize(rows, cols);
	
	STableCell	theCell(0, inFromCol);
	
	for (theCell.row = 1; theCell.row <= rows; theCell.row++) {
		TableIndexT	startIndex;
		mTableView->CellToIndex(theCell, startIndex);
		mDataArray->RemoveItemsAt(inHowMany, startIndex);
	}
}

	void
CHierarchyTableArrayStorage::InsertCols(
	Uint32		inHowMany,
	TableIndexT	inAfterCol,
	const void	*inDataPtr,
	Uint32		inDataSize)
{
	TableIndexT			rows,
						cols;
	LHierarchyTable		*hTable;
	
	hTable = (LHierarchyTable *)mTableView;
	hTable->GetWideOpenTableSize(rows, cols);
	
	STableCell	theCell(0, inAfterCol + 1);
	
	for (theCell.row = 1; theCell.row <= rows; theCell.row++) {
		TableIndexT	startIndex;
		mTableView->CellToIndex(theCell, startIndex);
		mDataArray->InsertItemsAt(inHowMany, startIndex, inDataPtr,
								inDataSize);
	}
}

#pragma mark --- Initialization ---

CKeyTable::CKeyTable(
	LStream	*inStream)
		:	LHierarchyTable(inStream),
			LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
{
	mTableGeometry	= new LTableMultiGeometry(this,	kDefaultColumnWidth,
													kRowHeight);
	mTableSelector	= new LTableMultiSelector(this);
	mTableStorage	= new CHierarchyTableArrayStorage(this, (ulong)0);
	mLastKeypress	= 0;
	mKeySet			= NULL;
	mKeyList		= NULL;
	mKeyLabels		= NULL;
	mDefaultKey		= NULL;
	mImports		= NULL;
	mDefaultRing	= FALSE;
	mMutableRing	= FALSE;
	mNotified		= FALSE;
	mSendingDrag	= FALSE;
	mTargetKeyServer= NULL;
}

void
CKeyTable::FinishCreateSelf()
{
	PGPError			err;
	PGPSize				len;
	KeyTableColumnInfo	*prefColumnInfo;
	
	::GetCurrentProcess(&sThisProcess);
	
	mBarberPixPat = ::GetPixPat(kBarberIconID);
	pgpAssertAddrValid(mBarberPixPat, VoidAlign);
	
	// Load Prefs for column display
	len = sizeof(KeyTableColumnInfo);
	err = PGPGetPrefData(gPrefRef, kPGPPrefPGPkeysMacColumnData,
							&len, &prefColumnInfo);
	if(IsPGPError(err))
	{
		::BlockMoveData(&kDefaultColumnInfo, &mColumnInfo,
							sizeof(KeyTableColumnInfo));
	}
	else
	{
		::BlockMoveData(prefColumnInfo, &mColumnInfo,
							sizeof(KeyTableColumnInfo));
		PGPDisposePrefData(gPrefRef, prefColumnInfo);
	}
	SetupColumns();
	
	mKeyIter = NULL;
	
	SwitchTarget(this);
	
	BuildServerSubmenu();
}

CKeyTable::~CKeyTable()
{
	Reset1();
	::DisposePixPat(mBarberPixPat);
	SaveColumnInfo();
}

	void
CKeyTable::SaveColumnInfo()
{
	PGPError err;
	
	err = PGPSetPrefData(gPrefRef, kPGPPrefPGPkeysMacColumnData,
							sizeof(KeyTableColumnInfo), &mColumnInfo);
	pgpAssertNoErr(err);
}

	void
CKeyTable::ListenToMessage(	MessageT inMessage,
								void */*ioParam*/)
{
	switch(inMessage)
	{
		case kKeyTableRedrawMessage:
			RedrawTable();
			break;
		case kKeyTableResyncMessage:
			ResyncTable(FALSE, TRUE);
			RedrawTable();
			break;
		case kRebuildServersMessage:
			BuildServerSubmenu();
			break;
	}
}

	void
CKeyTable::SetupColumns()
{
	Int16		columnIndex;

	RemoveAllCols(false);
	InsertCols(mColumnInfo.numActiveColumns, 1, NULL, 0, false);
	for(columnIndex = 0;columnIndex < mColumnInfo.numActiveColumns;
		columnIndex++)
	{
		SetColWidth(
			mColumnInfo.columnWidths[mColumnInfo.columns[columnIndex]],
			columnIndex + 1, columnIndex + 1);
	}
}	

	void
CKeyTable::GetKeyColumnInfo(KeyTableColumnInfo *columnInfo)
{
	::BlockMoveData(&mColumnInfo, columnInfo, sizeof(KeyTableColumnInfo));
}

	Boolean
CKeyTable::ColumnVisible(Int16	columnID)
{
	Int16		columnIndex;

	for(columnIndex = 0;columnIndex < mColumnInfo.numActiveColumns;
		columnIndex++)
	{
		if(mColumnInfo.columns[columnIndex] == columnID)
			return true;
	}
	return false;
}

	void
CKeyTable::MoveColumn(Int16 oldCol, Int16 newCol)
{
	Int16 colType;
	
	colType = mColumnInfo.columns[oldCol];
	if(newCol < oldCol)
		newCol++;
	if(oldCol < mColumnInfo.numActiveColumns - 1)
		::BlockMoveData(&mColumnInfo.columns[oldCol + 1],
						&mColumnInfo.columns[oldCol],
						sizeof(Int16) *
						(mColumnInfo.numActiveColumns - oldCol - 1));
	if(newCol < mColumnInfo.numActiveColumns - 1)
		::BlockMoveData(&mColumnInfo.columns[newCol],
					&mColumnInfo.columns[newCol + 1],
					sizeof(Int16) *
					(mColumnInfo.numActiveColumns - newCol - 1));
	mColumnInfo.columns[newCol] = colType;
	RemoveCols(1, oldCol + 1, false);
	InsertCols(1, newCol, NULL, 0, false);
	SetColWidth(mColumnInfo.columnWidths[colType], newCol + 1, newCol + 1);
	Refresh();
}

	void
CKeyTable::Reset()
{
	PGPError err;
	
	if(PGPRefIsValid(mKeySet))
	{
		if(mKeyIter)
			PGPFreeKeyIter(mKeyIter);
		err = PGPNewKeyIter(mKeyList, &mKeyIter);
		pgpAssertNoErr(err);
		pgpAssertAddrValid(mKeyIter, VoidAlign);
		
		mDefaultKey = NULL;
	}
}

	void
CKeyTable::Reset1()
{
	PGPError err;
	
	CKeyInfoWindow::CloseAll();
	if(PGPRefIsValid(mKeyIter))
	{
		err = PGPFreeKeyIter(mKeyIter);
		pgpAssertNoErr(err);
	}
	mKeyIter = kPGPInvalidRef;
	if(PGPRefIsValid(mKeyList))
	{
		err = PGPFreeKeyList(mKeyList);
		pgpAssertNoErr(err);
	}
	mKeyList = kPGPInvalidRef;
	CPGPKeys::TheApp()->SetKeyList(mKeyList);
	if(PGPRefIsValid(mKeySet))
	{
		if(mDefaultRing)
			CPGPKeys::TheApp()->CommitDefaultKeyrings();
		else if(mMutableRing)
		{
			err = PGPCommitKeyRingChanges(mKeySet);
			if(IsPGPError(err))
				ReportPGPError(err);
		}
		PGPFreeKeySet(mKeySet);
	}
	mKeySet = kPGPInvalidRef;
}

	void
CKeyTable::Empty()
{
	ClearTable();
	Reset();
	Refresh();
}

	void
CKeyTable::CommitKeySet()
{
	PGPError	err;
	
	if(mDefaultRing)
		CPGPKeys::TheApp()->CommitDefaultKeyrings();
	else if(mMutableRing)
	{
		err = PGPCommitKeyRingChanges(mKeySet);
		if(IsPGPError(err))
			ReportPGPError(err);
	}
}

	void
CKeyTable::SetKeyDBInfo(	PGPKeySetRef	keySet,
							Boolean			writable,
							Boolean			defaultRings )
{
	PGPBoolean	generatedKey;
	Boolean		pairFound,
				allowKG = TRUE;
	PGPError	err;

	mKeySet			= keySet;
	mMutableRing	= writable;
	mDefaultRing	= defaultRings;

	err = PGPOrderKeySet(mKeySet, kPGPUserIDOrdering, &mKeyList);
	pgpAssert(IsntPGPError(err) && PGPRefIsValid(mKeyList));
	CPGPKeys::TheApp()->SetKeyList(mKeyList);

	ClearTable();
	Reset();
	
	pairFound = ResyncTable(TRUE, TRUE);
	
	if(defaultRings)
	{
#if PGP_BUSINESS_SECURITY
		PGPSize			defaultKeysSize;
		char			*defaultKeys;
		PGPKeySetRef	defaultKeysSet = kPGPInvalidRef;
		
		err = PGPGetPrefData(gAdminPrefRef, kPGPPrefDefaultKeys,
								&defaultKeysSize, &defaultKeys);
		if(IsntPGPError(err))
		{
			if(defaultKeysSize > 10)
			{
				err = PGPImportKeySet(gPGPContext, &defaultKeysSet,
						PGPOInputBuffer(gPGPContext, defaultKeys,
										defaultKeysSize),
						PGPOLastOption(gPGPContext));
				if(IsntPGPError(err) && PGPRefIsValid(defaultKeysSet))
				{
					err = PGPAddKeys(defaultKeysSet, mKeySet);
					pgpAssertNoErr(err);
					CommitKeySet();
					PGPFreeKeySet(defaultKeysSet);
					pairFound = ResyncTable(TRUE, TRUE);
				}
				err = PGPSetPrefString(gAdminPrefRef,
										kPGPPrefDefaultKeys, "");
			}
			PGPDisposePrefData(gPrefRef, defaultKeys);
		}

		err = PGPGetPrefBoolean(gAdminPrefRef, kPGPPrefAllowKeyGen,
								&allowKG);
		pgpAssertNoErr(err);
#endif
		
		err = PGPGetPrefBoolean(gPrefRef, kPGPPrefFirstKeyGenerated,
								&generatedKey);
		pgpAssertNoErr(err);
		if(!generatedKey && allowKG)
		{
			err = PGPSetPrefBoolean(gPrefRef, kPGPPrefFirstKeyGenerated,
									TRUE);
			pgpAssertNoErr(err);
			
			if(!pairFound)
			{
				CKeyGenWizardDialog *kgw;
				
				kgw = (CKeyGenWizardDialog *)
					LWindow::CreateWindow(dialog_KeyGenWizard,
											CPGPKeys::TheApp());
				kgw->SetKeyTable(this);
			}
		}
	}
}

#pragma mark --- Drawing ---

	void
CKeyTable::DrawCell(
	const STableCell	&inCell,
	const Rect			&inLocalRect)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);
	Rect		frame;

	CalcLocalFrameRect( frame );
	{
		StDeviceLoop	devLoop(frame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth))
		{
			STableCell		woCell(woRow, 1);
			Uint32			dataSize = sizeof(KeyTableRef);
			KeyTableRef		keyRef;
			
			GetCellData(woCell, &keyRef, dataSize);
			::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			::TextMode(srcOr);
			switch(mColumnInfo.columns[inCell.col-1])
			{
				case kUserIDColumnID:
					DrawUserIDColumnCell(&keyRef, inLocalRect, depth);
					DrawDropFlag(inCell, woRow);
					break;
				case kValidityColumnID:
					DrawValidityColumnCell(&keyRef, inLocalRect, depth);
					break;
				case kTrustColumnID:
					DrawTrustColumnCell(&keyRef, inLocalRect, depth);
					break;
				case kKeySizeColumnID:
					DrawSizeColumnCell(&keyRef, inLocalRect, depth);
					break;
				case kKeyIDColumnID:
					DrawKeyIDColumnCell(&keyRef, inLocalRect, depth);
					break;
				case kCreationColumnID:
					DrawCreationColumnCell(&keyRef, inLocalRect, depth);
					break;
				case kExpirationColumnID:
					DrawExpirationColumnCell(&keyRef, inLocalRect, depth);
					break;
				case kMRKColumnID:
					DrawMRKColumnCell(&keyRef, inLocalRect, depth);
					break;
				case kDescriptionColumnID:
					DrawDescriptionColumnCell(&keyRef, inLocalRect, depth);
					break;
			}
			if(depth >= 8)
				RefreshBorder(inLocalRect);
		}
	}
}

	void
CKeyTable::RefreshBorder(Rect cellRect)
{
	Rect frame;
	
	CalcLocalFrameRect(frame);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::MoveTo(frame.left, cellRect.bottom - 1);
	::LineTo(frame.right - 1, cellRect.bottom - 1);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
}

	void
CKeyTable::DrawUserIDColumnCell(KeyTableRef *ktr, Rect cellRect, Int16 depth)
{
	PGPError	err;
	Rect		sortColumnRect;
	char		cstr[kUserIDStringLength];
	Str255		pstr;
	PGPSize		len;

	// Draw Background
	if(depth >= 8)
	{
		// Draw MacOS8-like background for cells, selected column
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
		::PaintRect(&cellRect);
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
		sortColumnRect = cellRect;
		sortColumnRect.left = cellRect.left + kDropFlagSlop;
		sortColumnRect.right = cellRect.right;
		::PaintRect(&sortColumnRect);
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
	}
	else
	{
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
		::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
		::EraseRect(&cellRect);
	}
	// Draw Info
	switch(ktr->type)
	{
		case kKey:
			{
				ResIDT		iconID;
				PGPBoolean	expired,
							revoked,
							disabled,
							secret;
				PGPInt32	algorithm;
				PGPKeyRef	key;
				
	
				key = ktr->u.key;
				err = PGPGetKeyBoolean(	key, kPGPKeyPropIsExpired,
										&expired);
				pgpAssertNoErr(err);
				err = PGPGetKeyBoolean(	key, kPGPKeyPropIsRevoked,
										&revoked);
				pgpAssertNoErr(err);
				err = PGPGetKeyBoolean(	key, kPGPKeyPropIsDisabled,
										&disabled);
				pgpAssertNoErr(err);
				err = PGPGetKeyBoolean(key, kPGPKeyPropIsSecret,
										&secret);
				pgpAssertNoErr(err);
				err = PGPGetKeyNumber(key, kPGPKeyPropAlgID,
										&algorithm);
				pgpAssertNoErr(err);
				if(revoked)
					iconID = (algorithm == kPGPPublicKeyAlgorithm_DSA) ?	
								kDSARevokedKeyID : kRSARevokedKeyID;
				else if(expired)
					iconID = (algorithm == kPGPPublicKeyAlgorithm_DSA) ?	
								kDSAExpiredKeyID : kRSAExpiredKeyID;
				else if(disabled)
				{
					if(algorithm == kPGPPublicKeyAlgorithm_DSA)
						iconID = kDSADisabledKeyID;
					else
						iconID = kRSADisabledKeyID;
				}
				else if(secret)
				{
					if(algorithm == kPGPPublicKeyAlgorithm_DSA)
						iconID = kDSAKeyPairIconID;
					else
						iconID = kRSAKeyPairIconID;
				}
				else if(algorithm == kPGPPublicKeyAlgorithm_DSA)
					iconID = kDSAKeyIconID;
				else
					iconID = kRSAKeyIconID;
				DrawIcon(iconID, cellRect, 1);
				
				len = kUserIDStringLength;
				err = PGPGetPrimaryUserIDNameBuffer(key,
					kUserIDStringLength, cstr, &len);
				if(err != kPGPError_BufferTooSmall)
					pgpAssertNoErr(err);
				CToPString(cstr, pstr);
				::TextFont(geneva);
				::TextSize(9);
				::TextFace(0);
				if(mDefaultKey == key)
					::TextFace(bold);
				else if(disabled)
					::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray9));
#if NO_RSA_OPERATIONS
				if(algorithm == kPGPPublicKeyAlgorithm_RSA)
					::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray9));
#endif
				::TruncString(	mColumnInfo.columnWidths[kUserIDColumnID] -
								kDropFlagSlop - kLeftIndent -
								kIconWidth - kLeftBorder,
								pstr, truncMiddle);
				::MoveTo(cellRect.left + kDropFlagSlop + kLeftIndent +
								kIconWidth + kLeftBorder, cellRect.bottom -
								kBottomBorder);
				::DrawString(pstr);
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));	
			}
			break;
		case kUserID:
			{
				PGPUserIDRef	user;
				PGPInt32		algorithm;

				user = ktr->u.user;
				err = PGPGetKeyNumber(ktr->ownerKey, kPGPKeyPropAlgID,
										&algorithm);
				pgpAssertNoErr(err);
				DrawIcon((algorithm == kPGPPublicKeyAlgorithm_DSA) ?
						kDHUserIDIconID : kRSAUserIDIconID, cellRect, 2);
				err = PGPGetUserIDStringBuffer(user, kPGPUserIDPropName,
							kUserIDStringLength, cstr, &len);
				pgpAssertNoErr(err);
				CToPString(cstr, pstr);
				::TextFont(geneva);
				::TextSize(9);
				::TextFace(0);
				::TruncString(	mColumnInfo.columnWidths[kUserIDColumnID] -
								kDropFlagSlop - kLeftIndent -
								kLevelIndent - kIconWidth - kLeftBorder,
								pstr, truncMiddle);
				::MoveTo(cellRect.left + kDropFlagSlop + kLeftIndent +
								kLevelIndent + kIconWidth + kLeftBorder,
								cellRect.bottom - kBottomBorder);
				::DrawString(pstr);
			}
			break;
		case kSignature:
			{
				Str255		str2;
				size_t		len;
				PGPKeyRef	certKey;
				PGPBoolean	revoked,
							verified,
							tried,
							notCorrupt,
							exportable;
				PGPInt32	trustLevel;
				ResIDT		iconID;
				PGPSigRef	sig;

				sig = ktr->u.sig;
				err = PGPGetSigBoolean(sig, kPGPSigPropIsRevoked,
										&revoked);
				pgpAssertNoErr(err);
				err = PGPGetSigBoolean(sig, kPGPSigPropIsVerified,
										&verified);
				pgpAssertNoErr(err);
				err = PGPGetSigBoolean(sig, kPGPSigPropIsTried,
										&tried);
				pgpAssertNoErr(err);
				err = PGPGetSigBoolean(sig, kPGPSigPropIsNotCorrupt,
										&notCorrupt);
				pgpAssertNoErr(err);
				err = PGPGetSigBoolean(sig, kPGPSigPropIsExportable,
										&exportable);
				pgpAssertNoErr(err);
				err = PGPGetSigNumber(sig, kPGPSigPropTrustLevel,
										&trustLevel);
				pgpAssertNoErr(err);
				if(revoked)
					iconID = 	kRevokedSigID;
				else if(verified)
				{
					if(exportable)
					{
						if(trustLevel == 1)
							iconID =	kExportMetaSigIconID;
						else
							iconID =	kExportSignatureIconID;
					}
					else
					{
						if(trustLevel == 2)
							iconID =	kNoExportMetaSigIconID;
						else
							iconID =	kNoExportSignatureIconID;
					}
				}
				else if(tried)
					iconID =	kBadSignatureID;
				else if(notCorrupt)
					iconID =	kExportSignatureIconID;
				else
					iconID =	kBadSignatureID;
				DrawIcon(iconID, cellRect, 3);
				err = PGPGetSigCertifierKey(sig, mKeySet, &certKey);
				if(!PGPRefIsValid(certKey) && !mDefaultRing)
				{
					err = PGPGetSigCertifierKey(sig,
							CPGPKeys::TheApp()->GetKeySet(),
							&certKey);
				}
				if(IsntPGPError(err) && PGPRefIsValid(certKey))
				{
					err = PGPGetPrimaryUserIDNameBuffer(certKey,
						kUserIDStringLength, cstr, &len);
					pgpAssertNoErr(err);
					CToPString(cstr, pstr);
					::TextFace(0);
				}
				else
				{
					PGPKeyID		keyID;
					char			keyIDString[ kPGPMaxKeyIDStringSize ];
					
					::GetIndString(	pstr,
									kStringListID,
									kSigUserUnavailableID);
					err = PGPGetKeyIDOfCertifier(sig, &keyID);
					pgpAssertNoErr(err);
					err = PGPGetKeyIDString( &keyID,
											kPGPKeyIDString_Abbreviated,
											keyIDString);
					pgpAssertNoErr(err);
					CToPString(keyIDString, str2);
					if(str2[0] > 10)
						str2[0] = 10;
					AppendPString(str2, pstr);
					::TextFace(italic);
				}
				::TextFont(geneva);
				::TextSize(9);
				::TruncString(	mColumnInfo.columnWidths[kUserIDColumnID] -
								kDropFlagSlop - kLeftIndent -
								kLevelIndent * 2 - kIconWidth - kLeftBorder,
								pstr, truncMiddle);
				::MoveTo(cellRect.left + kDropFlagSlop + kLeftIndent +
								kLevelIndent * 2 + kIconWidth + kLeftBorder,
								cellRect.bottom - kBottomBorder);
				::DrawString(pstr);
			}
			break;
	}
	::TextFace(0);
}

	void
CKeyTable::DrawValidityColumnCell(	KeyTableRef	*ktr,
									Rect		cellRect,
									Int16		depth)
{
	PGPError	err;
	PGPValidity	validity	= kPGPValidity_Unknown;
	PGPBoolean	axiomatic;
	
	switch(ktr->type)
	{
		PGPInt32	myValidityNumber	= 0;
		
		case kKey:
		case kUserID:
			if(ktr->type == kKey)
			{
				PGPKeyRef	key;
	
				key = ktr->u.key;
				err = PGPGetPrimaryUserIDValidity(key, &validity);
				pgpAssertNoErr(err);
				err = PGPGetKeyBoolean(key, kPGPKeyPropIsAxiomatic,
									&axiomatic);
				pgpAssertNoErr(err);
			}
			else
			{
				PGPUserIDRef	user;
				PGPInt32		number;

				user = ktr->u.user;
				err = PGPGetUserIDNumber(user, kPGPUserIDPropValidity,
										&number);
				validity	= (PGPValidity)number;
				pgpAssertNoErr(err);
				err = PGPGetKeyBoolean(ktr->ownerKey,
									kPGPKeyPropIsAxiomatic,
									&axiomatic);
				pgpAssertNoErr(err);
			}
			switch(validity)
			{
				default:
				case kPGPValidity_Unknown:
				case kPGPValidity_Invalid:
					myValidityNumber = 0;
					break;
				case kPGPValidity_Marginal:
					myValidityNumber = 1;
					break;
				case kPGPValidity_Complete:
					myValidityNumber = 2;
					break;
			}
			DrawValidity(axiomatic ? (kMaxValidity + 1) : myValidityNumber,
							cellRect, depth);
			break;
		case kSignature:
			break;
	}
}

	void
CKeyTable::DrawTrustColumnCell(		KeyTableRef	*ktr,
									Rect		cellRect,
									Int16		depth)
{
	PGPError err;
	
	switch(ktr->type)
	{
		case kKey:
			{
				PGPKeyRef	key;
				PGPInt32	trust;
	
				key = ktr->u.key;
				err = PGPGetKeyNumber(key, kPGPKeyPropTrust, &trust);
				pgpAssertNoErr(err);
				switch(trust)
				{
					default:
					case kPGPKeyTrust_Undefined:
					case kPGPKeyTrust_Unknown:
					case kPGPKeyTrust_Never:
						trust = 0;
						break;
					case kPGPKeyTrust_Marginal:
						trust = 1;
						break;
					case kPGPKeyTrust_Complete:
						trust = 2;
						break;
					case kPGPKeyTrust_Ultimate:
						trust = 3;
						break;
				}
				DrawTrust(trust, cellRect, depth);
			}
			break;
		case kUserID:
		case kSignature:
			break;
	}
}

	void
CKeyTable::DrawSizeColumnCell(	KeyTableRef	*ktr,
									Rect		cellRect,
									Int16		depth)
{
	PGPError err;
	
	switch(ktr->type)
	{
		case kKey:
			{
				PGPKeyRef		key;
				PGPSubKeyRef	subKey;
				PGPInt32		bits,
								algorithm;
				Str255			str,
								str2;
	
				key = ktr->u.key;
				err = PGPGetKeyNumber(key, kPGPKeyPropBits, &bits);
				pgpAssertNoErr(err);
				err = PGPGetKeyNumber(key, kPGPKeyPropAlgID, &algorithm);
				pgpAssertNoErr(err);
				::NumToString(bits, str);
				if(algorithm == kPGPPublicKeyAlgorithm_DSA)
				{
					PGPKeyIterSeek(mKeyIter, key);
					err = PGPKeyIterNextSubKey(mKeyIter, &subKey);
					if(IsntPGPError(err) && IsntNull(subKey))
					{
						err = PGPGetSubKeyNumber(subKey, kPGPKeyPropBits,
													&bits);
						pgpAssertNoErr(err);
						::NumToString(bits, str2);
						AppendPString("\p/", str2);
						AppendPString(str, str2);
						CopyPString(str2, str);
					}
				}
				::TextFont(geneva);
				::TextSize(9);
				::TextFace(0);
				::MoveTo(	cellRect.left + kLeftIndent, 
							cellRect.bottom - kBottomBorder);
				::DrawString(str);
			}
			break;
		case kUserID:
		case kSignature:
			break;
	}
}

	void
CKeyTable::DrawKeyIDColumnCell(		KeyTableRef	*ktr,
									Rect		cellRect,
									Int16		depth)
{
	switch(ktr->type)
	{
		case kKey:
			{
				PGPError	err;
				PGPKeyRef	key;
				Str255		pstr;
				PGPKeyID	keyID;
				char		keyIDString[ kPGPMaxKeyIDStringSize ];
	
				key = ktr->u.key;
				err = PGPGetKeyIDFromKey(key, &keyID );
				pgpAssertNoErr(err);
				err = PGPGetKeyIDString( &keyID, kPGPKeyIDString_Abbreviated,
										keyIDString);
				pgpAssertNoErr(err);
				CToPString(keyIDString, pstr);
				//if(pstr[0] > 10)
				//	pstr[0] = 10;
				::TextFont(geneva);
				::TextSize(9);
				::TextFace(0);
				::MoveTo(	cellRect.left + kLeftIndent,
							cellRect.bottom - kBottomBorder);
				::DrawString(pstr);
			}
			break;
		case kUserID:
		case kSignature:
			break;
	}
}

	void
CKeyTable::DrawCreationColumnCell(	KeyTableRef	*ktr,
									Rect		cellRect,
									Int16		depth)
{
	PGPError	err;
	PGPTime		creation;
	Str255		str;
	
	::TextFont(geneva);
	::TextSize(9);
	::TextFace(0);
	switch(ktr->type)
	{
		case kKey:
			{
				PGPKeyRef	key;
	
				key = ktr->u.key;
				err = PGPGetKeyTime(key, kPGPKeyPropCreation, &creation);
				pgpAssertNoErr(err);
				creation = PGPTimeToMacTime(creation);
				::DateString(creation, shortDate, str, NULL);
				::MoveTo(	cellRect.left + kLeftIndent,
							cellRect.bottom - kBottomBorder);
				::DrawString(str);
			}
			break;
		case kUserID:
			break;
		case kSignature:
			{
				PGPSigRef	sig;

				sig = ktr->u.sig;
				err = PGPGetSigTime(sig, kPGPSigPropCreation, &creation);
				pgpAssertNoErr(err);
				creation = PGPTimeToMacTime(creation);
				::DateString(creation, shortDate, str, NULL);
				::MoveTo(	cellRect.left + kLeftIndent,
							cellRect.bottom - kBottomBorder);
				::DrawString(str);
			}
			break;
	}
}

	void
CKeyTable::DrawExpirationColumnCell(	KeyTableRef	*ktr,
										Rect		cellRect,
										Int16		depth)
{
	PGPError	err;
	PGPTime		expiration;
	Str255		str;

	switch(ktr->type)
	{
		case kKey:
			{
				PGPKeyRef	key;
	
				key = ktr->u.key;
				err = PGPGetKeyTime(key, kPGPKeyPropExpiration, &expiration);
				pgpAssertNoErr(err);
				if(IsntPGPError(err) && expiration)
				{
					expiration = PGPTimeToMacTime(expiration);
					::DateString(expiration, shortDate, str, NULL);
				}
				else
					::GetIndString(	str, kStringListID, kNeverExpiresID);
				::TextFont(geneva);
				::TextSize(9);
				::TextFace(0);
				::MoveTo(	cellRect.left + kLeftIndent,
							cellRect.bottom - kBottomBorder);
				::DrawString(str);
			}
			break;
		case kUserID:
		case kSignature:
			break;
	}
}

	void
CKeyTable::DrawMRKColumnCell(		KeyTableRef	*ktr,
									Rect		cellRect,
									Int16		depth)
{
	switch(ktr->type)
	{
		case kKey:
			{
				PGPKeyRef		key,
								mrKey;
				PGPByte			mrClass;
				PGPError		err;
	
				key = ktr->u.key;
				err = PGPGetIndexedAdditionalRecipientRequest(	key,
												mKeySet, 0,
												&mrKey, &mrClass);
				//pgpAssertNoErr(err);
				if((IsntPGPError(err) && PGPRefIsValid(mrKey)) ||
					err == kPGPError_AdditionalRecipientRequestKeyNotFound)
					DrawIcon(kMRKActiveIconID, cellRect, -1);
				else
					DrawIcon(kKeyAttrOffIconID, cellRect, -1);
			}
			break;
		case kUserID:
		case kSignature:
			break;
	}
}

	void
CKeyTable::DrawDescriptionColumnCell(	KeyTableRef	*ktr,
										Rect		cellRect,
										Int16		depth)
{
	Str255 str;
	
	switch(ktr->type)
	{
		case kKey:
			{
				PGPKeyRef	key;
				PGPInt32	algorithm;
				PGPBoolean	secret;
				PGPError	err;
	
				key = ktr->u.key;
				err = PGPGetKeyBoolean(key, kPGPKeyPropIsSecret,
										&secret);
				pgpAssertNoErr(err);
				err = PGPGetKeyNumber(key, kPGPKeyPropAlgID,
										&algorithm);
				pgpAssertNoErr(err);
				if(secret)
				{
					if(algorithm == kPGPPublicKeyAlgorithm_DSA)
					{
						::GetIndString(	str,
										kKeyDescriptionsStringListID,
										kDHDSSKeyPairDescriptionID);
					}
					else
					{
						::GetIndString(	str,
										kKeyDescriptionsStringListID,
										kRSAKeyPairDescriptionID);
					}
				}
				else
				{
					if(algorithm == kPGPPublicKeyAlgorithm_DSA)
					{
						::GetIndString(	str,
										kKeyDescriptionsStringListID,
										kDHDSSKeyDescriptionID);
					}
					else
					{
						::GetIndString(	str,
										kKeyDescriptionsStringListID,
										kRSAKeyDescriptionID);
					}
				}
			}
			break;
		case kUserID:
			::GetIndString(	str,
							kKeyDescriptionsStringListID,
							kUserIDDescriptionID);
			break;
		case kSignature:
			::GetIndString(	str,
							kKeyDescriptionsStringListID,
							kSignatureDescriptionID);
			break;
	}
	::TextFont(geneva);
	::TextSize(9);
	::TextFace(0);
	::MoveTo(	cellRect.left + kLeftIndent,
				cellRect.bottom - kBottomBorder);
	::DrawString(str);
}

	void
CKeyTable::DrawTrust(Int32 trustValue, Rect cellRect, Int16 depth)
{
	RGBColor	trustColor = { 52223, 52223, 65535 };
	Int16		width;
	
	width = mColumnInfo.columnWidths[kTrustColumnID] -
				kBarRightBorder - kBarLeftBorder;
	cellRect.left 	+=	kBarLeftBorder;
	cellRect.right	= 	cellRect.left + width;
	cellRect.bottom	=	cellRect.bottom - kBottomBorder;
	cellRect.top	=	cellRect.bottom - kBarHeight;
	
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
	::FrameRect(&cellRect);
	::InsetRect(&cellRect, 1, 1);
	if(trustValue >= kMaxTrust)
	{
		::PenPixPat(mBarberPixPat);
		::PaintRect(&cellRect);
		::PenNormal();
	}
	else
	{
		if(depth >= 8)
		{
			::RGBForeColor(&trustColor);
			::PaintRect(&cellRect);
		}
		
		cellRect.right -= width * ((kMaxTrust - 1) - trustValue) /
									(kMaxTrust - 1);
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray9));
		::PaintRect(&cellRect);
	}
	ApplyForeAndBackColors();
}

	void
CKeyTable::DrawValidity(Int32 trustValue, Rect cellRect, Int16 depth)
{
	RGBColor	trustColor = { 52223, 52223, 65535 };
	Int16		width;
	PGPBoolean	showMarginal;
	PGPError	err;
	
	err = PGPGetPrefBoolean(gPrefRef,
							kPGPPrefDisplayMarginalValidity, &showMarginal);
	pgpAssertNoErr(err);
	if(!showMarginal)
	{
		ResIDT		iconID;

		if(trustValue > kMaxValidity)
		{
			iconID = kAxiomaticIconID;
		}
		else if(trustValue > 1)
		{
			iconID = kValidKeyIconID;
		}
		else if(trustValue == 1)
		{
			PGPBoolean	marginalIsInvalid;
		
			err = PGPGetPrefBoolean(gPrefRef,
							kPGPPrefMarginalIsInvalid,
							&marginalIsInvalid);
			pgpAssertNoErr(err);
			
			if( marginalIsInvalid )
			{
				iconID = kKeyAttrOffIconID;
			}
			else
			{
				iconID = kValidKeyIconID;
			}
		}
		else
		{
			iconID = kKeyAttrOffIconID;
		}
		
		DrawIcon(iconID, cellRect, -2);
	}
	else
	{
		width = mColumnInfo.columnWidths[kValidityColumnID] - kBarRightBorder
				- kBarLeftBorder;
		cellRect.left 	+=	kBarLeftBorder;
		cellRect.right	= 	cellRect.left + width;
		cellRect.bottom	=	cellRect.bottom - kBottomBorder;
		cellRect.top	=	cellRect.bottom - kBarHeight;
		
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
		::FrameRect(&cellRect);
		::InsetRect(&cellRect, 1, 1);
		if(trustValue > kMaxValidity)
		{
			::PenPixPat(mBarberPixPat);
			::PaintRect(&cellRect);
			::PenNormal();
		}
		else
		{
			if(depth >= 8)
			{
				::RGBForeColor(&trustColor);
				::PaintRect(&cellRect);
			}
			
			cellRect.right -= width * (kMaxValidity - trustValue) /
					kMaxValidity;
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray9));
			::PaintRect(&cellRect);
		}
		ApplyForeAndBackColors();
	}
}

	void
CKeyTable::DrawIcon(
	ResIDT				iconID,
	Rect				cellRect,
	UInt32				indentLevel)
{
	Rect	iconRect;

	switch(indentLevel)
	{
		case -2:
			iconRect.left 	= 	cellRect.left + 10;
			break;
		case -1:
			iconRect.left 	= 	cellRect.left + kLeftIndent;
			break;
		case 0:
			iconRect.left 	= 	cellRect.left + kDropFlagSlop;
			break;
		case 1:
			iconRect.left 	= 	cellRect.left + kDropFlagSlop + kLeftIndent;
			break;
		case 2:
			iconRect.left 	= 	cellRect.left + kDropFlagSlop + kLeftIndent
									+ kLevelIndent;
			break;
		case 3:
			iconRect.left 	= 	cellRect.left + kDropFlagSlop + kLeftIndent
									+ kLevelIndent * 2;
			break;
		default:
			pgpAssert(0);
			break;
	}
			
	iconRect.top	=	cellRect.top + 2;
	iconRect.right 	= 	iconRect.left + kIconWidth;
	iconRect.bottom = 	iconRect.top + kIconHeight;
	
	::PlotIconID(&iconRect, atNone, ttNone, iconID);
}

	void
CKeyTable::RedrawTable()
{
	Rect frame;
	
	CalcPortFrameRect(frame);
	InvalPortRect(&frame);
}

	void
CKeyTable::HiliteCellActively(
	const STableCell	&inCell,
	Boolean				/*inHilite*/)
{
	Rect	cellFrame;
	
	if(inCell.col != 1)
		return;
	if(GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		StDeviceLoop	devLoop(cellFrame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth))
		{
			StColorPenState saveColorPen;   // Preserve color & pen state
			StColorPenState::Normalize();
			if(depth >= 8)
			{
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			else
			{
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			cellFrame.left += kDropFlagSlop;
			cellFrame.right = cellFrame.left +
					mColumnInfo.columnWidths[kUserIDColumnID];
	        UDrawingUtils::SetHiliteModeOn();
			::InvertRect(&cellFrame);
		}
	}
}

	void
CKeyTable::HiliteCellInactively(
	const STableCell	&inCell,
	Boolean			 	/*inHilite*/)
{
	Rect	cellFrame;
	
	if(inCell.col != 1)
		return;
	if(GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		StDeviceLoop	devLoop(cellFrame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth))
		{
	        StColorPenState saveColorPen;   // Preserve color & pen state
	        StColorPenState::Normalize();
	        if(depth >= 8)
	        {
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			else
			{
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			::PenMode(srcXor);
			cellFrame.left += kDropFlagSlop;
			cellFrame.right = cellFrame.left +
					mColumnInfo.columnWidths[kUserIDColumnID];
			UDrawingUtils::SetHiliteModeOn();
			::FrameRect(&cellFrame);
		}
	}
}

	void
CKeyTable::DrawSelf()
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

#pragma mark --- Table Management ---

	void
CKeyTable::UnmarkKey(PGPKeyRef inKey)
{
	PGPKeyRef		key;
	PGPUserIDRef	user;
	PGPSigRef		sig;
	PGPError		err;

	PGPKeyIterRewind(mKeyIter);
	while(IsntPGPError(err = PGPKeyIterNext(mKeyIter, &key)) && IsntNull(key))
	{
		if(key == inKey)
		{
			err = PGPSetKeyUserVal(key, kUserValueUnmarked);
			pgpAssert(!err);
			while(IsntPGPError(err = PGPKeyIterNextUserID(mKeyIter, &user)) &&
					IsntNull(user))
			{
				err = PGPSetUserIDUserVal(user, kUserValueUnmarked);
				pgpAssert(!err);
				while(IsntPGPError(
					err = PGPKeyIterNextUIDSig(mKeyIter, &sig)) &&
					IsntNull(sig))
				{
					err = PGPSetSigUserVal(sig, kUserValueUnmarked);
					pgpAssert(!err);
				}
			}
			break;
		}
	}
}

	void
CKeyTable::RemoveKeyFromTable(PGPKeyRef inKey)
{
	KeyTableRef		tableRef;
	STableCell		cell(1,1);
	TableIndexT		rows,
					cols,
					rowIndex;
	Uint32			dataSize = sizeof(KeyTableRef);
	
	GetWideOpenTableSize(rows, cols);
	for(rowIndex = 1;rowIndex < rows;rowIndex++)
	{
		cell.row = rowIndex;
		GetCellData(cell, &tableRef, dataSize);
		if((tableRef.type == kKey) && (tableRef.u.key == inKey))
		{
			RemoveRows(1, rowIndex, true);
			break;
		}
	}
}

	void
CKeyTable::TerminateKey(PGPKeyRef inKey)
{
	KeyTableRef		tableRef;
	STableCell		cell(1,1);
	TableIndexT		rows,
					cols,
					rowIndex;
	Uint32			dataSize = sizeof(KeyTableRef);
	
	GetWideOpenTableSize(rows, cols);
	for(rowIndex = 1;rowIndex < rows;rowIndex++)
	{
		cell.row = rowIndex;
		GetCellData(cell, &tableRef, dataSize);
		if((tableRef.type == kKey) && (tableRef.u.key == inKey))
		{
			tableRef.terminated = TRUE;
			SetCellData(cell, &tableRef, dataSize);
			break;
		}
	}
}

	void
CKeyTable::RemoveTerminatedKeys()
{
	KeyTableRef		tableRef;
	STableCell		cell(1,1);
	TableIndexT		rows,
					cols,
					rowIndex;
	Uint32			dataSize = sizeof(KeyTableRef);
	
	GetWideOpenTableSize(rows, cols);
	for(rowIndex = rows;rowIndex > 0;rowIndex--)
	{
		cell.row = rowIndex;
		GetCellData(cell, &tableRef, dataSize);
		if((tableRef.type == kKey) && tableRef.terminated)
			RemoveRows(1, rowIndex, true);
	}
}

	void
CKeyTable::ClearTable()
{
	RemoveAllRows(false);
}

/*
ResyncTable

This is a meaty function.  It runs through the KeyDB list of keys and checks
each key, userID, and signature to see whether its userVal has been set to
non-zero.  If it hasn't, then the function assumes that entry is not yet in
the table and adds it.  It also sets its userVal to 1 so that it wont be
added again later.  This function is used in many places, for example when
signing key(s), this function is called to merge all the new signature(s)
into the displayed list.

Returns TRUE if at least one key pair was found while iterating over the
list
*/

	Boolean
CKeyTable::ResyncTable(Boolean collapse, Boolean onlyUnmarked)
{
	PGPKeyRef 		key;
	PGPUserIDRef	user;
	PGPSigRef		sig;
	PGPError		err;
	KeyTableRef		tableRef;
	Int32			userVal;
	TableIndexT		uidSubRow,
					sigSubRow,
					lastKeyRow,
					lastRow;
	Int16			uidIndex,
					sigIndex;
	Boolean			pairFound = FALSE;
	PGPBoolean		secret;
	
	lastRow = lastKeyRow = 0;
	if(mDefaultRing)
	{
		PGPGetDefaultPrivateKey(mKeySet, &mDefaultKey);
	}
	PGPKeyIterRewind(mKeyIter);
	while(IsntPGPError(err = PGPKeyIterNext(mKeyIter, &key)) && IsntNull(key))
	{
		err = PGPGetKeyBoolean(key, kPGPKeyPropIsSecret, &secret);
		pgpAssert(!err);
		if(secret)
			pairFound = TRUE;
		err = PGPGetKeyUserVal(key, (PGPUserValue *)&userVal);
		pgpAssert(!err);
		++lastRow;
		if(userVal != kUserValueMarked || !onlyUnmarked)
		{
			tableRef.type		=	kKey;
			tableRef.ownerKey	=	key;
			tableRef.terminated =	FALSE;
			tableRef.u.key		=	key;
			lastKeyRow = InsertSiblingRows(1, lastKeyRow, &tableRef,
									sizeof(KeyTableRef), TRUE, TRUE);
			err = PGPSetKeyUserVal(key, (PGPUserValue)kUserValueMarked);
			pgpAssertNoErr(err);
		}
		else if(!lastKeyRow)
			lastKeyRow = 1;
		if(lastRow > 1)
			lastKeyRow = lastRow;
		for(uidIndex = 1;IsntPGPError(err = PGPKeyIterNextUserID(mKeyIter,
							&user)) && IsntNull(user);uidIndex++)
		{
			err = PGPKeyIterNextUIDSig(mKeyIter, &sig);
			err = PGPGetUserIDUserVal(user, (PGPUserValue *)&userVal);
			pgpAssert(!err);
			++lastRow;
			if(userVal != kUserValueMarked || !onlyUnmarked)
			{
				tableRef.type		=	kUserID;
				tableRef.terminated =	FALSE;
				tableRef.ownerKey	=	key;
				tableRef.u.user		=	user;
				if(uidIndex == 1)	// First user ID
				{
					InsertChildRows(1, lastKeyRow, &tableRef,
										sizeof(KeyTableRef), TRUE, TRUE);
					uidSubRow = lastKeyRow + 1;
				}
				else
					uidSubRow = InsertSiblingRows(1, uidSubRow, &tableRef,
										sizeof(KeyTableRef), TRUE, TRUE);
				err = PGPSetUserIDUserVal(user,
										(PGPUserValue)kUserValueMarked);
				pgpAssert(!err);
			}
			else
			{
				if(uidIndex == 1)
					uidSubRow = lastKeyRow + 1;
				else
					uidSubRow = lastRow;
			}
			for(sigIndex = 1; (sigIndex == 1 && IsntNull(sig)) ||
				IsntPGPError(err = PGPKeyIterNextUIDSig(mKeyIter, &sig)) &&
				IsntNull(sig);sigIndex++)
			{
				err = PGPGetSigUserVal(sig, (PGPUserValue *)&userVal);
				pgpAssert(!err);
				++lastRow;
				if(userVal != kUserValueMarked || !onlyUnmarked)
				{
					tableRef.type		=	kSignature;
					tableRef.ownerKey	=	key;
					tableRef.terminated =	FALSE;
					tableRef.u.sig		=	sig;
					if(sigIndex == 1)	// First signature
					{
						InsertChildRows(1, uidSubRow, &tableRef,
										sizeof(KeyTableRef), FALSE, TRUE);
						sigSubRow = uidSubRow + 1;
					}
					else
						sigSubRow = InsertSiblingRows(1, sigSubRow,
										&tableRef, sizeof(KeyTableRef),
										FALSE, TRUE);
					err = PGPSetSigUserVal(sig,
											(PGPUserValue)kUserValueMarked);
					pgpAssert(!err);
				}
				else
				{
					if(sigIndex == 1)
						sigSubRow = uidSubRow + 1;
					else
						sigSubRow = lastRow;
				}
			}
		}
	}
	if(collapse)
		CollapseKeys(TRUE);
	return pairFound;
}

	void
CKeyTable::Sign()
{
	TableIndexT		numRows, numCols;
	PGPError		err;
	KeyTableRef		*userIDs;
	Int32			numUserIDs;
	PGPBoolean		ksSync;
		
	GetTableSize(numRows, numCols);
	STableCell	topCell(1, 1);
	STableCell 	botCell(numRows, 1);
	STableCell	cell;				// Loop thru all cells
	
	err = PGPGetPrefBoolean(gPrefRef,
				kPGPPrefKeyServerSyncOnKeySign, &ksSync);
	pgpAssertNoErr(err);
	numUserIDs = 0;
	userIDs = (KeyTableRef	*)pgpAlloc(sizeof(KeyTableRef));
	for(cell = botCell; cell.row >= topCell.row; cell.row--) 
	{
		if(CellIsSelected(cell)) 
		{
			TableIndexT		inWideOpenRow =
							mCollapsableTree->GetWideOpenIndex(cell.row);
			STableCell		woCell(inWideOpenRow, 1);
			KeyTableRef		keyRef;
			Uint32			dataSize = sizeof(KeyTableRef);
			
			GetCellData(woCell, &keyRef, dataSize);
			if(keyRef.type == kSignature)
				continue;
			err = pgpRealloc(	&userIDs,
								(numUserIDs + 1) * sizeof(KeyTableRef));
			pgpAssertNoErr(err);
			if(IsntPGPError(err))
			{
				userIDs[numUserIDs] = keyRef;
				numUserIDs ++;
				if(ksSync)
					GetCellOnServer(cell);
			}
		}
	}
	if(numUserIDs > 0)
	{
		CSignDialog *mSignDialog;
		
		mSignDialog = (CSignDialog *)
			LWindow::CreateWindow(kSignDialogResID, CPGPKeys::TheApp());
		mSignDialog->AddListener(this);
		mSignDialog->SetUserIDs(mDefaultKey, userIDs, numUserIDs);
		if(ksSync)
		{
			ResyncTable(FALSE, TRUE);
			RedrawTable();
		}
	}
	else
		pgpFree(userIDs);
}

	Boolean
CKeyTable::Delete(STableCell& inCell)
{
	TableIndexT		inWideOpenRow =
						mCollapsableTree->GetWideOpenIndex(inCell.row);
	STableCell		woCell(inWideOpenRow, 1);
	KeyTableRef		keyRef;
	Uint32			dataSize = sizeof(KeyTableRef);
	Boolean			result = TRUE;
	PGPError		err;
	
	GetCellData(woCell, &keyRef, dataSize);

	switch(keyRef.type)
	{
		case kKey:
		{
			PGPKeySetRef		singleKeySet;
			
			err = PGPNewSingletonKeySet(keyRef.u.key, &singleKeySet);
			if(IsntPGPError(err) && IsntNull(singleKeySet))
			{
				err = PGPRemoveKeys( singleKeySet, mKeySet );
				pgpAssertNoErr(err);
				PGPFreeKeySet(singleKeySet);
				if(IsntPGPError(err))
				{
					RemoveRows(1, inWideOpenRow, true);
					if(mDefaultKey == keyRef.u.key)
						mDefaultKey = NULL;
					if(mDefaultRing)
						CPGPKeys::TheApp()->InvalidateCaches();
				}
			}
			break;
		}
		case kUserID:
			// This procedure could resort the table, so we need to
			// remove the whole key from the table and resync with
			// the sorted iterator
			
			UnmarkKey(keyRef.ownerKey);
			TerminateKey(keyRef.ownerKey);
			
			err = PGPRemoveUserID(keyRef.u.user);
			if(err != kPGPError_BadParams)	// tried to delete primary user id
				pgpAssertNoErr(err);
			break;
		case kSignature:
			err = PGPRemoveSig(keyRef.u.sig);
			pgpAssertNoErr(err);
			if(IsntPGPError(err))
				RemoveRows(1, inWideOpenRow, true);
			break;
	}
	return result;
}

	Boolean
CKeyTable::ShowKeyInfo(STableCell& inCell)
{
	TableIndexT		inWideOpenRow =
						mCollapsableTree->GetWideOpenIndex(inCell.row);
	STableCell		woCell(inWideOpenRow, 1);
	KeyTableRef		keyRef;
	PGPError		err;
	PGPKeyRef		certifierKey;
	Uint32			dataSize = sizeof(KeyTableRef);
	Boolean			result = TRUE;
	
	GetCellData(woCell, &keyRef, dataSize);
	if(keyRef.type == kKey)
	{
		if(!CKeyInfoWindow::OpenKeyInfo(keyRef.u.key, this))
			result = FALSE;	// Too many key windows, break out
	}
	else if(keyRef.type == kUserID)
	{
		if(!CKeyInfoWindow::OpenKeyInfo(keyRef.ownerKey, this))
			result = FALSE;	// Too many key windows, break out
	}
	else if(keyRef.type == kSignature)
	{
		CKeyTable	*parentTable = this;
		
		err = PGPGetSigCertifierKey(keyRef.u.sig, mKeySet, &certifierKey);
		if(!mDefaultRing && IsPGPError(err))
		{
			err = PGPGetSigCertifierKey(keyRef.u.sig, 
					CPGPKeys::TheApp()->GetKeySet(),
					&certifierKey);
			if(IsntPGPError(err))
				parentTable =
					CPGPKeys::TheApp()->GetDefaultKeyView()->GetKeyTable();
		}
		if(!PGPRefIsValid(certifierKey))
			CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
							kStringListID, kCertUnavailableID);
		else if(!err && !CKeyInfoWindow::OpenKeyInfo(certifierKey, parentTable))
			result = FALSE;	// Too many key windows, break out
	}
	
	return result;
}

	Boolean
CKeyTable::Revoke(STableCell& inCell)
{
	TableIndexT			inWideOpenRow =
							mCollapsableTree->GetWideOpenIndex(inCell.row);
	STableCell			woCell(inWideOpenRow, 1);
	KeyTableRef			keyRef;
	Uint32				dataSize = sizeof(KeyTableRef);
	PGPError			err;
	CSecureCString256	passphrase;
	char				userID[kUserIDStringLength];
	Boolean				result = TRUE;
	PGPBoolean			needsPassphrase;
	PGPBoolean			ksSync;
	PGPBoolean			secret;
	size_t				len;
	
	err = PGPGetPrefBoolean(gPrefRef,
				kPGPPrefKeyServerSyncOnRevocation, &ksSync);
	pgpAssertNoErr(err);
	GetCellData(woCell, &keyRef, dataSize);
	if(keyRef.type == kKey)
	{
		err = PGPGetKeyBoolean(keyRef.u.key, kPGPKeyPropIsSecret, &secret);
		pgpAssert(!err);
		if(secret)
		{
			if(CWarningAlert::Display(kWACautionAlertType, kWAOKCancelStyle,
							kStringListID, kRevokeConfirmID) == msg_OK)
			{
				if(ksSync)
					GetCellOnServer(inCell);
				err = PGPGetKeyBoolean(keyRef.u.key,
						kPGPKeyPropNeedsPassphrase, &needsPassphrase);
				pgpAssertNoErr(err);
				do
				{
					if(err == kPGPError_BadPassphrase)
						CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
										kStringListID, kBadPassphraseID);
					err = noErr;
					err = PGPGetPrimaryUserIDNameBuffer(keyRef.u.key,
								kUserIDStringLength -1, userID, &len);
					pgpAssert(!err);
					userID[len] = 0;
					if(!needsPassphrase || DoPassphraseDialog(TRUE, userID,
							passphrase))
					{
						CPGPKeys::TheApp()->GetMinimumRandomData();
						err = PGPRevokeKey( keyRef.u.key,
								PGPOPassphrase( gPGPContext, passphrase ),
								PGPOLastOption( gPGPContext ) );
					}
				} while(err == kPGPError_BadPassphrase);
				pgpAssertNoErr(err);
				if(ksSync)
					SendKeyToServer(inCell);
			}
		}
		else
		{
			CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
							kStringListID, kRevokeOnlyPairsID);
		}
	}
	else if(keyRef.type == kSignature)
	{
		PGPKeyRef	certKey;
		
		err = PGPGetSigCertifierKey(keyRef.u.sig, mKeySet, &certKey);
		pgpAssertNoErr(err);
		if(PGPRefIsValid(certKey))
		{
			if(ksSync)
				GetKeyOnServer(certKey);
			err = PGPGetKeyBoolean(certKey, kPGPKeyPropNeedsPassphrase,
									&needsPassphrase);
			pgpAssertNoErr(err);
			
			do
			{
				if(err == kPGPError_BadPassphrase)
					CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
									kStringListID, kBadPassphraseID);
				err = noErr;
				err = PGPGetPrimaryUserIDNameBuffer(certKey,
							kUserIDStringLength - 1, userID, &len);
				pgpAssertNoErr(err);
				userID[len] = 0;
				if(!needsPassphrase || DoPassphraseDialog(TRUE, userID,
						passphrase))
				{
					CPGPKeys::TheApp()->GetMinimumRandomData();
					err = PGPRevokeSig( keyRef.u.sig, mKeySet,
							PGPOPassphrase( gPGPContext, passphrase),
							PGPOLastOption( gPGPContext ) );
				}
			} while(err == kPGPError_BadPassphrase);
			pgpAssertNoErr(err);
			if(ksSync)
			{
				err = PGPSendKeyToServer(gPGPContext, certKey, 0);
				if(IsPGPError(err) && (err != kPGPError_UserAbort))
					ReportPGPError(err);
			}
		}
	}
	return result;
}

	Boolean
CKeyTable::SendKeyToServer(STableCell& inCell)
{
	TableIndexT		inWideOpenRow =
						mCollapsableTree->GetWideOpenIndex(inCell.row);
	STableCell		woCell(inWideOpenRow, 1);
	KeyTableRef		tableRef;
	UInt32			dataSize = sizeof(KeyTableRef);
	PGPError		err;

	GetCellData(woCell, &tableRef, dataSize);
	if(tableRef.type == kKey)
	{
		err = PGPSendKeyToServer(gPGPContext, tableRef.u.key, 0,
								mTargetKeyServer);
		if(IsPGPError(err) && (err != kPGPError_UserAbort))
			ReportPGPError(err);
	}
	mTargetKeyServer = NULL;
	return IsntPGPError(err);
}

	Boolean
CKeyTable::GetKeyOnServer(PGPKeyRef key)
{
	PGPKeyID		keyID;
	PGPKeySetRef	keySet;
	PGPError		err;

	err = PGPGetKeyIDFromKey(key, &keyID);
	pgpAssertNoErr(err);
	if(IsntPGPError(err) )
	{
		err = PGPGetKeyFromServer(gPGPContext, &keyID, key, NULL,
									0, &keySet);
		if(IsPGPError(err) && (err != kPGPError_UserAbort))
			ReportPGPError(err);
	}
	if(PGPRefIsValid(keySet))
	{
		err = PGPAddKeys(keySet, mKeySet);
		pgpAssertNoErr(err);
		CommitKeySet();
		err = PGPFreeKeySet(keySet);
		pgpAssertNoErr(err);
	}
	return IsntPGPError(err);
}

	Boolean
CKeyTable::GetCellOnServer(STableCell& inCell)
{
	TableIndexT		inWideOpenRow =
						mCollapsableTree->GetWideOpenIndex(inCell.row);
	STableCell		woCell(inWideOpenRow, 1);
	KeyTableRef		tableRef;
	Uint32			dataSize = sizeof(KeyTableRef);
	PGPKeyID		keyID;
	PGPKeySetRef	keySet;
	PGPKeyRef		key;
	PGPError		err;
	
	keySet = NULL;
	GetCellData(woCell, &tableRef, dataSize);
	if(tableRef.type == kKey)
	{
		err = PGPGetKeyIDFromKey(tableRef.u.key, &keyID);
		pgpAssertNoErr(err);
		if(IsntPGPError(err) )
		{
			err = PGPGetKeyFromServer(
						gPGPContext, &keyID, tableRef.u.key, NULL, 0, &keySet);
			if(IsPGPError(err) && (err != kPGPError_UserAbort))
				ReportPGPError(err);
		}
	}
	else if(tableRef.type == kSignature)
	{
		err = PGPGetKeyIDOfCertifier(tableRef.u.sig,
										&keyID);
		pgpAssertNoErr(err);
		if(IsntPGPError(err) )
		{
			err = PGPGetSigCertifierKey(tableRef.u.sig,
										mKeySet,
										&key);
			// intentionally ignore error here, key may not be on the ring
			err = PGPGetKeyFromServer(
						gPGPContext, &keyID, key, NULL, 0, &keySet);
			if(IsPGPError(err) && (err != kPGPError_UserAbort))
				ReportPGPError(err);
		}
	}
	if(PGPRefIsValid(keySet))
	{
		err = PGPAddKeys(keySet, mKeySet);
		pgpAssertNoErr(err);
		CommitKeySet();
		err = PGPFreeKeySet(keySet);
		pgpAssertNoErr(err);
	}
	return IsntPGPError(err);
}

	UInt32
CKeyTable::SelectionAction(Boolean (CKeyTable::*function)(STableCell&))
{	
	UInt32	cellsPerformedCount = 0;
	
	TableIndexT	numRows, numCols;
	GetTableSize(numRows, numCols);
	
	STableCell	topCell(1, 1);
	STableCell 	botCell(numRows, 1);
	STableCell	cell;
	
	for(cell = botCell; cell.row >= topCell.row; cell.row--) 
	{
		if(CellIsSelected(cell)) 
		{
			if((this->*function)(cell) == true)
				cellsPerformedCount++;
			else
				goto error;
		}
	}
error:
	return cellsPerformedCount;
}

	void
CKeyTable::ClickCell(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
{
	if(GetClickCount() == 2)
		SelectionAction(&CKeyTable::ShowKeyInfo);
	else if (LDropArea::DragAndDropIsPresent() && 
						::WaitMouseMoved(inMouseDown.macEvent.where))
	{
		mSendingDrag = TRUE;
		CKeyDragTask	drag(*this, inMouseDown.macEvent);
		drag.DoDrag();
		if(UDragAndDropUtils::DroppedInTrash(drag.GetDragReference()) &&
			mMutableRing)
		{
			if(UDragAndDropUtils::CheckForOptionKey(drag.GetDragReference()))
				LCommander::GetTarget()->ProcessCommand(cmd_OptionClear);		
			else
				LCommander::GetTarget()->ProcessCommand(cmd_Clear);
		}
		mSendingDrag = FALSE;
	}
	else if(inCell.col == 1)
	{
		if(!IsTarget())
			SwitchTarget(this);
		LHierarchyTable::ClickCell(inCell, inMouseDown);
	}
}

	void
CKeyTable::ClickValidityCell(
	const STableCell&		inCell)
{
	STableCell	selCell;
	SPoint32	imagePt;
	Rect		cellRect;
	Point		curMouse;
	Boolean		pressed = FALSE;
	PGPBoolean	showMarginal,
				axiomatic;
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);
	STableCell	woCell(woRow, 1);
	Uint32		dataSize = sizeof(KeyTableRef);
	KeyTableRef	keyRef;
	PGPError	err;
			
	GetCellData(woCell, &keyRef, dataSize);
	err = PGPGetPrefBoolean(gPrefRef,
		kPGPPrefDisplayMarginalValidity, &showMarginal);
	pgpAssertNoErr(err);
	if(!showMarginal && (keyRef.type != kSignature))
	{
		err = PGPGetKeyBoolean(keyRef.ownerKey, kPGPKeyPropIsAxiomatic,
							&axiomatic);
		pgpAssertNoErr(err);
		FocusDraw();
		GetLocalCellRect(inCell, cellRect);
		while(::StillDown())
		{
			::GetMouse(&curMouse);
			LocalToImagePoint(curMouse, imagePt);
			if(GetCellHitBy(imagePt, selCell) &&
				(selCell == inCell))
			{
				if(!pressed)
					DrawIcon(axiomatic? kPressedAxiomaticIconID : 
										kPressedValidityIconID,
										cellRect, -2);
				pressed = TRUE;
			}
			else if(pressed)
			{
				DrawCell(inCell, cellRect);
				pressed = FALSE;
			}
		}
		if(pressed)
		{
			DrawCell(inCell, cellRect);
			selCell = inCell;
			selCell.col = 1;
			if (!CellIsSelected(selCell)) {
				UnselectAllCells();
				SelectCell(selCell);
			}
			Sign();
		}
	}
}


// The following function is copied verbatim from LHierarchyTable,
// and is modified here to fix a stupid bug in PowerPlant that
// puts a drop flag in every cell in the row.

	void
CKeyTable::ClickSelf(
	const SMouseDownEvent	&inMouseDown)
{
	STableCell	hitCell;
	SPoint32	imagePt;
	
	LocalToImagePoint(inMouseDown.whereLocal, imagePt);
	
	if (GetCellHitBy(imagePt, hitCell)) {
										// Click is inside hitCell
										// Check if click is inside DropFlag
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(hitCell.row);
		Rect	flagRect;
		CalcCellFlagRect(hitCell, flagRect);
		
		if ((hitCell.col == 1) &&
			mCollapsableTree->IsCollapsable(woRow) &&
			::PtInRect(inMouseDown.whereLocal, &flagRect))
		{
										// Click is inside DropFlag
			FocusDraw();
			Boolean	expanded = mCollapsableTree->IsExpanded(woRow);
			if (LDropFlag::TrackClick(flagRect, inMouseDown.whereLocal,
									expanded)) {
										// Mouse released inside DropFlag
										//   so toggle the Row
				if (inMouseDown.macEvent.modifiers & optionKey) {
										// OptionKey down means to do
										//   a deep collapse/expand
					if (expanded) {
						DeepCollapseRow(woRow);
					} else {
						DeepExpandRow(woRow);
					}
				
				} else {				// Shallow collapse/expand
					if (expanded) {
						CollapseRow(woRow);
					} else {
						ExpandRow(woRow);
					}
				}
			}
	
		}
		else
		{
			if(hitCell.col == 1)
			{
				if (ClickSelect(hitCell, inMouseDown))
					ClickCell(hitCell, inMouseDown);
			}
			else
			{
				if(hitCell.col <= mColumnInfo.numActiveColumns &&
					mColumnInfo.columns[hitCell.col - 1] == kValidityColumnID)
				{
					ClickValidityCell(hitCell);
				}
			}
		}
	} else {							// Click is outside of any Cell
		UnselectAllCells();
	}
}

	void
CKeyTable::Click(
	SMouseDownEvent	&inMouseDown)
{
									// Check if a SubPane of this View
									//   is hit
	LPane	*clickedPane = FindSubPaneHitBy(inMouseDown.wherePort.h,
											inMouseDown.wherePort.v);
											
	if (clickedPane != nil) {		// SubPane is hit, let it respond to
									//   the Click
		clickedPane->Click(inMouseDown);
		
	} else {						// No SubPane hit. Inherited function
		PortToLocalPoint(inMouseDown.whereLocal);
		UpdateClickCount(inMouseDown);
		
		if (ExecuteAttachments(msg_Click, &inMouseDown)) {
			ClickSelf(inMouseDown);
		}
	}
}

	Boolean
CKeyTable::SelectionExists()
{
	if(mTableSelector->GetFirstSelectedRow() > 0)
		return TRUE;
	else
		return FALSE;
}

	void
CKeyTable::CollapseKeys(Boolean all)
{
	TableIndexT	rows,
				cols,
				rowIndex;
	STableCell	cell(1, 1);
	Boolean		selectedOnly;
	
	if(all)
		selectedOnly = FALSE;
	else
		selectedOnly = SelectionExists();
	GetWideOpenTableSize(rows, cols);
	for(rowIndex = rows;rowIndex > 0;rowIndex--)
	{
		cell.row = GetExposedIndex(rowIndex);
		if(!selectedOnly || CellIsSelected(cell))
			if(IsCollapsable(rowIndex))
				DeepCollapseRow(rowIndex);
	}
	cell.row = cell.col = 1;
	ScrollCellIntoFrame(cell);
	RedrawTable();
}

	void
CKeyTable::ExpandKeys()
{
	TableIndexT	rows,
				cols,
				rowIndex;
	STableCell	cell(1, 1);
	Boolean		selectedOnly;
	
	selectedOnly = SelectionExists();
	GetWideOpenTableSize(rows, cols);
	for(rowIndex = rows;rowIndex > 0;rowIndex--)
	{
		cell.row = GetExposedIndex(rowIndex);
		if(!selectedOnly || CellIsSelected(cell))
			if(IsCollapsable(rowIndex))
				DeepExpandRow(rowIndex);
	}
	RedrawTable();
}

	Boolean
CKeyTable::AskUserForUserID(Str255 userIDStr)
{
	CPGPStDialogHandler addUserDialog(dialog_AddUserID, CPGPKeys::TheApp());
	LWindow				*window;
	MessageT			dialogMessage;
	Str255				stringValue;
	
	window = addUserDialog.GetDialog();
tryAgain:
	do
	{
		dialogMessage = addUserDialog.DoDialog();
	} while(!dialogMessage);
	if(dialogMessage == 'okay')
	{
		((LGAEditField *)window->FindPaneByID('eNam'))->
									GetDescriptor(stringValue);

		if (stringValue[ 0 ] == '\0')
		{
			CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
							kStringListID, kBadNameID);
			goto tryAgain;
		}
		else
			CopyPString(stringValue, userIDStr);

		((LGAEditField *)window->FindPaneByID('eEma'))->
									GetDescriptor(stringValue);
		if (stringValue[ 0 ] != '\0')
		{
			AppendPString("\p <", userIDStr);
			AppendPString(stringValue, userIDStr);
			AppendPString("\p>", userIDStr);
		}
		return TRUE;
	}
	return FALSE;
}

	void
CKeyTable::AddUserID()
{
	PGPError		err;
	STableCell		cell,
					wideCell;
	Uint32			dataSize = sizeof(KeyTableRef),
					len;
	KeyTableRef		keyRef;
	PGPBoolean		secret;
	PGPBoolean		needsPass;
	Boolean			firstPass = TRUE;
	
	cell = GetFirstSelectedCell();
	wideCell.col = 1;
	wideCell.row = mCollapsableTree->GetWideOpenIndex(cell.row);
	GetCellData(wideCell, &keyRef, dataSize);
	secret = false;
	if(keyRef.type == kKey)
	{
		err = PGPGetKeyBoolean(keyRef.u.key, kPGPKeyPropIsSecret, &secret);
		pgpAssert(!err);
	}
	if((keyRef.type == kKey) && secret)
	{
		Str255				userIDStr;
		CSecureCString256	passphrase;
		char				oldUserID[kUserIDStringLength];
		PGPBoolean			ksSync;
				
		err = PGPGetPrefBoolean(gPrefRef,
					kPGPPrefKeyServerSyncOnAddName, &ksSync);
		pgpAssertNoErr(err);
		if(ksSync)
		{
			GetCellOnServer(cell);
			ResyncTable(FALSE, TRUE);
		}
		passphrase[0]=0;
		userIDStr[0]=0;
		if(AskUserForUserID(userIDStr))
		{
			err = PGPGetKeyBoolean(keyRef.u.key, kPGPKeyPropNeedsPassphrase,
									&needsPass);
			pgpAssert(!err);
			if(needsPass)
			{
				err = PGPGetPrimaryUserIDNameBuffer(keyRef.u.key,
						kUserIDStringLength -1, oldUserID, &len);
				pgpAssert(!err);
				oldUserID[len] = 0;
askPass:
				if(DoPassphraseDialog(firstPass, oldUserID, passphrase))
					needsPass = FALSE;
				firstPass = FALSE;
			}
			if(!needsPass)
			{
				char	cStringUsrID[ 256 ];

				PToCString( userIDStr, cStringUsrID );
				
				CPGPKeys::TheApp()->GetMinimumRandomData();
				err = PGPAddUserID(keyRef.u.key, cStringUsrID,
					PGPOPassphrase( gPGPContext, passphrase),
					PGPOLastOption( gPGPContext ));
					
				if(err == kPGPError_BadPassphrase)
				{
					needsPass = TRUE;
					goto askPass;
				}
				else if(err == kPGPError_DuplicateUserID)
				{
					CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
									kStringListID, kDupeUserID);
				}
				else
					pgpAssert(!err);
				if(!err)
				{
					UnmarkKey(keyRef.u.key);
					RemoveKeyFromTable(keyRef.u.key);
					ResyncTable(FALSE, TRUE);
					CommitKeySet();
					pgpAssertNoErr(err);
				}
			}
			if(ksSync)
			{
				err = PGPSendKeyToServer(gPGPContext, keyRef.u.key, 0);
				if(IsPGPError(err) && (err != kPGPError_UserAbort))
					ReportPGPError(err);
			}
		}
	}
	else
		CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
						kStringListID, kAddNameOnlyPairsID);
}

	Boolean
CKeyTable::SetDefault()
{
	PGPError		err;
	STableCell		cell;
	Uint32			dataSize = sizeof(KeyTableRef);
	KeyTableRef		keyRef;
	PGPBoolean		secret;
	
	cell = GetFirstSelectedCell();
	cell.col = 1;
	cell.row = mCollapsableTree->GetWideOpenIndex(cell.row);
	GetCellData(cell, &keyRef, dataSize);
	secret = false;
	if(keyRef.type == kKey)
	{
		err = PGPGetKeyBoolean(keyRef.u.key, kPGPKeyPropIsSecret, &secret);
		pgpAssertNoErr(err);
	}
	if((keyRef.type == kKey) && secret)
	{
		PGPSetDefaultPrivateKey(keyRef.u.key);
		mDefaultKey = keyRef.u.key;
		PGPsdkSavePrefs(gPGPContext);
		RedrawTable();
		return TRUE;
	}
	else if(keyRef.type == kUserID)
	{
		err = PGPSetPrimaryUserID(keyRef.u.user);
		pgpAssertNoErr(err);
		CommitKeySet();
		UnmarkKey(keyRef.ownerKey);
		RemoveKeyFromTable(keyRef.ownerKey);
		ResyncTable(FALSE, TRUE);
		RedrawTable();
		return TRUE;
	}
	else
		return FALSE;
}

	Boolean
CKeyTable::WeAreFront()
{
	ProcessSerialNumber	process;
	Boolean				weFront = FALSE;

	::GetFrontProcess(&process);
	::SameProcess(&process, &sThisProcess, &weFront);
	
	return weFront;
}

	void
CKeyTable::ImportHandler()
{
	PGPKeySetRef		newKeySet,
						selectedKeySet;
	PGPFileSpecRef		pgpFileRef;
	PGPUInt32			keyCount;
	Str255				str;
	PGPError			err;
	KeyImportStruct		*savedKis;
	Boolean				abort = FALSE;
	
	while(IsntNull(mImports))
	{
		if(mImports->savedImportIsFile)
		{
			err = PGPNewFileSpecFromFSSpec(gPGPContext,
									&mImports->savedImportSpec, &pgpFileRef);
			pgpAssertNoErr(err);
			if(IsntPGPError(err))
			{
				err = PGPImportKeySet( gPGPContext, &newKeySet,
							PGPOInputFile( gPGPContext, pgpFileRef ),
							PGPOLastOption( gPGPContext ) );
				PGPFreeFileSpec(pgpFileRef);
			}
		}
		else
		{
			err = PGPImportKeySet( gPGPContext, &newKeySet,
					PGPOInputBuffer( gPGPContext,
							mImports->savedImportBuffer,
							mImports->savedImportBufferSize ),
					PGPOLastOption( gPGPContext ) );
			pgpFree(mImports->savedImportBuffer);
		}
		if(IsntPGPError(err) && PGPRefIsValid(newKeySet))
		{
			err = PGPCountKeys(newKeySet, &keyCount);
			if(IsntPGPError(err) && keyCount > 0)
			{
				if(!mImports->bypassImportDialog)
				{
					char	cstring[128];
					
					::GetIndString(	str,
									kStringListID,
									kSpecifyKeysToImportID);
					PToCString(str, cstring);
					UDesktop::Deactivate();
					err = PGPSelectiveImportDialog(gPGPContext, cstring,
													newKeySet, mKeySet,
													&selectedKeySet);
					UDesktop::Activate();
				}
				else
					selectedKeySet = newKeySet;
				if(IsntPGPError(err) && PGPRefIsValid(selectedKeySet))
				{
					err = PGPAddKeys(selectedKeySet, mKeySet);
					pgpAssertNoErr(err);
					CommitKeySet();
					if(!mImports->bypassImportDialog)
						PGPFreeKeySet(selectedKeySet);
				}
				else if(err == kPGPError_UserAbort)
				{
					abort = TRUE;
				}
				else
					ReportPGPError(err);
			}
			else
				CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
							kStringListID, kNoKeysFoundID);
			PGPFreeKeySet(newKeySet);
		}
		savedKis = mImports;
		mImports = mImports->next;
		pgpFree(savedKis);
		if(abort)
		{
			// user has aborted import, dispose all queued imports
			while(IsntNull(mImports))
			{
				if(!mImports->savedImportIsFile)
					pgpFree(mImports->savedImportBuffer);
				savedKis = mImports;
				mImports = mImports->next;
				pgpFree(savedKis);
			}
		}
	}
	ResyncTable(FALSE, TRUE);
	RedrawTable();
	if(mDefaultRing)
		CPGPKeys::TheApp()->InvalidateCaches();
}

	void
CKeyTable::SpendTime(const EventRecord		&/*inMacEvent*/)
{
	if(!WeAreFront())
		return;
	else if(mBlockDialogDelays > 0)
	{
		mBlockDialogDelays--;
		return;
	}
	if(mNotified)
	{
		::NMRemove(&sImportNotification);
		mNotified = FALSE;
	}
	StopIdling();
	ImportHandler();
}

	void
CKeyTable::ImportKeysFromFile(FSSpec *keyFile, Boolean noDialog)
{
	KeyImportStruct	*kis;
	
	pgpAssertAddrValid(keyFile, FSSpec);
	
	kis = (KeyImportStruct	*)pgpAlloc(sizeof(KeyImportStruct));
	if(IsntNull(kis))
	{
		kis->savedImportIsFile = TRUE;
		kis->bypassImportDialog = noDialog;
		::BlockMoveData(keyFile, &kis->savedImportSpec, sizeof(FSSpec));
		kis->next = mImports;
		mImports = kis;
		StartIdling();
		StepForward();
	}
}

	void
CKeyTable::ImportKeysFromHandle(Handle data)
{
	KeyImportStruct	*kis;
	
	if(IsntNull(data))
	{
		kis = (KeyImportStruct	*)pgpAlloc(sizeof(KeyImportStruct));
		if(IsntNull(kis))
		{
			long	importSize;
			
			importSize = GetHandleSize(data);
			kis->savedImportIsFile = FALSE;
			kis->bypassImportDialog = TRUE;
			kis->savedImportBuffer = (PGPByte *)pgpAlloc(importSize);
			if(IsntNull(kis->savedImportBuffer))
			{
				::BlockMoveData(*data, kis->savedImportBuffer, importSize);
				kis->savedImportBufferSize = importSize;
				kis->next = mImports;
				mImports = kis;
				ImportHandler();
			}
			else
				// ***** out of memory
				pgpFree(kis);
		}
	}
}

	void
CKeyTable::ImportKeysFromKeySet(PGPKeySetRef keySet)
{
	if(PGPRefIsValid(keySet))
	{
		PGPError	err;
		
		err = PGPAddKeys(keySet, mKeySet);
		pgpAssertNoErr(err);
		CommitKeySet();
		PGPFreeKeySet(keySet);
		ResyncTable(FALSE, TRUE);
		RedrawTable();
		if(mDefaultRing)
			CPGPKeys::TheApp()->InvalidateCaches();
	}
}

	void
CKeyTable::CopyKeys()
{
	TableIndexT	numRows, numCols;
	GetTableSize(numRows, numCols);
	
	STableCell		topCell(1, 1);
	STableCell 		botCell(numRows, 1);
	STableCell		cell;
	
	PGPKeySetRef	keysToExport;
	PGPError		err;
	Int16			keyCount = 0;
	
	::ZeroScrap();
	err = PGPNewKeySet(gPGPContext, &keysToExport);
	pgpAssertNoErr(err);
	pgpAssertAddrValid(keysToExport, VoidAlign);
	for(cell = botCell; cell.row >= topCell.row; cell.row--) 
	{
		if(CellIsSelected(cell)) 
		{
			TableIndexT		inWideOpenRow = mCollapsableTree->
								GetWideOpenIndex(cell.row);
			STableCell		woCell(inWideOpenRow, cell.col);
			KeyTableRef		keyRef;
			Uint32			dataSize = sizeof(KeyTableRef);
			PGPKeySetRef	singleKeySet;
			
			GetCellData(woCell, &keyRef, dataSize);
			if(keyRef.type == kKey)
			{
				err = PGPNewSingletonKeySet(keyRef.u.key, &singleKeySet);
				pgpAssertNoErr(err);
				pgpAssertAddrValid(singleKeySet, VoidAlign);
				err = PGPAddKeys(singleKeySet, keysToExport);
				pgpAssertNoErr(err);
				err = PGPCommitKeyRingChanges(keysToExport);
				if(IsPGPError(err))
					ReportPGPError(err);
				PGPFreeKeySet(singleKeySet);
				keyCount++;
			}
		}
	}
	if((keyCount > 0) )
	{
		void			*keyBuffer;
		UInt32			keyBufferLen;
		char			comment[256];
		
		comment[0] = 0;
#if PGP_BUSINESS_SECURITY
		PGPGetPrefStringBuffer(
				gAdminPrefRef,
				kPGPPrefComments,
				sizeof(comment), comment);
#endif	// PGP_BUSINESS_SECURITY
		if(comment[0] == 0)
			PGPGetPrefStringBuffer(
					gPrefRef,
					kPGPPrefComment,
					sizeof( comment ), comment);
		
		/* let CDK export to a buffer it creates itself */
		err = PGPExportKeySet(keysToExport,
			PGPOAllocatedOutputBuffer( gPGPContext,
				&keyBuffer, MAX_PGPUInt32, &keyBufferLen ),
			PGPOVersionString(gPGPContext, pgpVersionHeaderString),
			PGPOCommentString(gPGPContext, comment),
			PGPOLastOption( gPGPContext ));
		pgpAssert(!err);
		if( IsntPGPError( err ) )
		{
			::PutScrap(keyBufferLen, 'TEXT', keyBuffer);
			
			PGPFreeData( keyBuffer );
		}
	}
	PGPFreeKeySet(keysToExport);
}

	void
CKeyTable::ToggleColumn(Int16 columnID)
{
	Int16	columnIndex,
			oldColumn;
	Boolean	found = false;
	
	for(columnIndex = 0; columnIndex < mColumnInfo.numActiveColumns;
		columnIndex++)
	{
		if(mColumnInfo.columns[columnIndex] == columnID)
		{
			oldColumn = columnIndex;
			found = true;
			break;
		}
	}
	
	if(found)
	{
		found = false;
		for(columnIndex = 0; columnIndex < mColumnInfo.numActiveColumns;
			columnIndex++)
		{
			if(found)
				mColumnInfo.columns[columnIndex - 1] =
					mColumnInfo.columns[columnIndex];
			else if(mColumnInfo.columns[columnIndex] == columnID)
				found = true;
		}
		mColumnInfo.numActiveColumns --;
		RemoveCols(1, oldColumn + 1, false);
	}
	else
	{
		mColumnInfo.columns[mColumnInfo.numActiveColumns] =
			columnID;
		InsertCols(1, mColumnInfo.numActiveColumns, NULL, 0, false);
		mColumnInfo.numActiveColumns ++;
		SetColWidth(mColumnInfo.columnWidths[columnID],
			mColumnInfo.numActiveColumns, mColumnInfo.numActiveColumns);
	}
	RedrawTable();
	mKeyLabels->Refresh();
	SaveColumnInfo();
}

typedef struct SFInfo
{
	StandardFileReply			sfReply;
	Boolean						exportPrivate;
} SFInfo;

enum
{
	kExportPrivateCheckboxItem	= 13
};

	static pascal short
ExportDialogHook(short item, DialogPtr dialog, SFInfo *sfInfo)
{
	pgpAssertAddrValid( dialog, DialogRecord );
	pgpAssertAddrValid( sfInfo, SFInfo );

	switch( item )
	{
		case kExportPrivateCheckboxItem:
		{
			sfInfo->exportPrivate = ToggleDialogCheckbox( dialog,
										kExportPrivateCheckboxItem );
			break;
		}
	}
	return item;
}

	void
CKeyTable::ExportSelected()
{
	PGPError			err;
	TableIndexT			numRows, numCols;
	DlgHookYDUPP		cbHook;
	static Point		where={0,0};
	GetTableSize(numRows, numCols);
	
	STableCell	topCell(1, 1);
	STableCell 	botCell(numRows, 1);
	STableCell	cell;
	
	SFInfo				sfInfo;
	Str255				exportPrompt,
						exportFilename;
	
	::GetIndString(exportPrompt, kStringListID, kExportKeysPromptID);
	::GetIndString(exportFilename, kStringListID, kExportKeysDefaultNameID);
	
	cbHook 		= NewDlgHookYDProc( ExportDialogHook );
	sfInfo.exportPrivate = FALSE;
	
	UDesktop::Deactivate();
	CustomPutFile(exportPrompt, exportFilename, &sfInfo.sfReply,
					kExportCustomPutDialog, where, cbHook, NULL, NULL,
					NULL, &sfInfo);
	UDesktop::Activate();

	DisposeRoutineDescriptor( cbHook );
	
	
	if(sfInfo.sfReply.sfGood)
	{
		PGPFileSpecRef		pgpFileRef = kPGPInvalidRef;
		PGPKeySetRef		keysToExport = kPGPInvalidRef;
		Int32				keysExported = 0;
		
		err = PGPNewFileSpecFromFSSpec(gPGPContext, &sfInfo.sfReply.sfFile,
										&pgpFileRef);
		pgpAssertNoErr(err);
		if(PGPRefIsValid(pgpFileRef))
		{
			err = PGPNewEmptyKeySet(mKeySet, &keysToExport);
			pgpAssertNoErr(err);
			if(PGPRefIsValid(keysToExport))
			{
				for(cell = botCell; cell.row >= topCell.row; cell.row--) 
				{
					if(CellIsSelected(cell)) 
					{
						TableIndexT		inWideOpenRow =
											GetWideOpenIndex(cell.row);
						STableCell		woCell(inWideOpenRow, 1);
						KeyTableRef		keyRef;
						Uint32			dataSize = sizeof(KeyTableRef);
						PGPKeySetRef	oldKeys = keysToExport;
						PGPKeySetRef	oneKey;
						
						GetCellData(woCell, &keyRef, dataSize);
						if(keyRef.type == kKey)
						{
							err = PGPNewSingletonKeySet(keyRef.u.key, &oneKey);
							pgpAssertNoErr(err);
							err = PGPUnionKeySets(oldKeys, oneKey,
													&keysToExport);
							pgpAssertNoErr(err);
							PGPFreeKeySet(oldKeys);
							PGPFreeKeySet(oneKey);
							keysExported++;
						}
					}
				}
				if(keysExported > 0)
				{
					char comment[256];
					
					comment[0] = 0;
	#if PGP_BUSINESS_SECURITY
					PGPGetPrefStringBuffer(
							gAdminPrefRef,
							kPGPPrefComments,
							sizeof(comment), comment);
	#endif	// PGP_BUSINESS_SECURITY
					if(comment[0] == 0)
						PGPGetPrefStringBuffer(
								gPrefRef,
								kPGPPrefComment,
								sizeof( comment ), comment);
					err = PGPExportKeySet( keysToExport,
						PGPOOutputFile(gPGPContext, pgpFileRef),
						PGPOExportPrivateKeys(gPGPContext,
											sfInfo.exportPrivate),
						PGPOVersionString(gPGPContext,
											pgpVersionHeaderString),
						PGPOCommentString(gPGPContext, comment),
						PGPOLastOption(gPGPContext));
					pgpAssertNoErr(err);
				}
				else
					CWarningAlert::Display(kWANoteAlertType, kWAOKStyle,
									kStringListID, kNoKeyToExportID);
				PGPFreeKeySet(keysToExport);
			}
			PGPFreeFileSpec(pgpFileRef);
		}
	}
}

	void
CKeyTable::BuildServerSubmenu()
{
	MenuHandle			serverMenu;
	Int16				numItems;
	PGPKeyServerEntry	*ksEntries;
	PGPUInt32			ksCount;
	PGPError			err;
	
	serverMenu = ::GetMenuHandle(kSendToServerMENUID);
	if(IsntNull(serverMenu))
	{
		numItems = ::CountMItems(serverMenu);
		while(numItems > 2)
			::DeleteMenuItem(serverMenu, numItems--);
		err = PGPGetKeyServerPrefs(gPrefRef, &ksEntries, &ksCount);
		pgpAssertNoErr(err);
		if(IsntPGPError(err))
		{
			for(Int16 ksInx = 0; ksInx < ksCount; ksInx++)
			{
				if(IsKeyServerListed(ksEntries[ksInx].flags))
				{
					Str255	menuStr;
					
					CToPString(ksEntries[ksInx].serverURL, menuStr);
					::InsertMenuItem(serverMenu, "\pA", 0x6FFF);
					::SetMenuItemText(serverMenu, ++numItems, menuStr);
				}
			}
			PGPDisposePrefData(gPrefRef, ksEntries);
		}
	}
}

	Boolean
CKeyTable::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true;
	ResIDT		theMenuID;
	Int16		theMenuItem;
	PGPError	err;

	if(IsSyntheticCommand(inCommand, theMenuID, theMenuItem))
	{
		switch(theMenuID)
		{
			case kSendToServerMENUID:
				if(gServerCallsPresent)
				{
					PGPKeyServerEntry	*ksEntries;
					PGPUInt32			ksCount;
					
					err = PGPGetKeyServerPrefs(gPrefRef, &ksEntries,
												&ksCount);
					pgpAssertNoErr(err);
					if(IsntPGPError(err))
					{
						theMenuItem -= 3;
						if(theMenuItem < ksCount)
						{
							UInt32	successfulSends;
							Str255	numSendsString;
							
							mTargetKeyServer = &ksEntries[theMenuItem];
							successfulSends = 
								SelectionAction(&CKeyTable::SendKeyToServer);
							if(successfulSends > 0)
							{
								NumToString(successfulSends, numSendsString);
								CWarningAlert::Display(kWANoteAlertType,
										kWAOKStyle, kStringListID,
										kServerSendSuccessful,
										numSendsString);
							}
							PGPDisposePrefData(gPrefRef, ksEntries);
						}
					}
				}
				break;
			default:
				cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
				break;
		}
	}
	else switch (inCommand) 
	{
		case cmd_ViewValidity:
			ToggleColumn(kValidityColumnID);
			break;
		case cmd_ViewTrust:
			ToggleColumn(kTrustColumnID);
			break;
		case cmd_ViewSize:
			ToggleColumn(kKeySizeColumnID);
			break;
		case cmd_ViewKeyID:
			ToggleColumn(kKeyIDColumnID);
			break;
		case cmd_ViewCreationDate:
			ToggleColumn(kCreationColumnID);
			break;
		case cmd_ViewExpirationDate:
			ToggleColumn(kExpirationColumnID);
			break;
		case cmd_ViewMRK:
			ToggleColumn(kMRKColumnID);
			break;
		case cmd_ViewDescription:
			ToggleColumn(kDescriptionColumnID);
			break;
		case cmd_Copy:
			CopyKeys();
			break;
		case cmd_Paste:
		{
			Handle	pastedTextH;
			Int32	offset,
					length;
			
			length = ::GetScrap(nil, 'TEXT', &offset);
			pastedTextH = ::NewHandle(length);
			HNoPurge(pastedTextH);
			if((length = ::GetScrap(pastedTextH, 'TEXT', &offset)) > 0)
			{
				PGPKeySetRef	newKeySet;
				uchar			*pastedChars;
				
				HLock(pastedTextH);
				pastedChars = *(uchar **)pastedTextH;
				
				err = PGPImportKeySet( gPGPContext, &newKeySet,
						PGPOInputBuffer( gPGPContext, pastedChars, length ),
						PGPOLastOption( gPGPContext ) );
						
				if(IsntPGPError(err) && IsntNull(newKeySet))
				{
					err = PGPAddKeys(newKeySet, mKeySet);
					pgpAssertNoErr(err);
					CommitKeySet();
					PGPFreeKeySet(newKeySet);
					ResyncTable(FALSE, TRUE);
					if(mDefaultRing)
						CPGPKeys::TheApp()->InvalidateCaches();
				}
			}
			::DisposeHandle(pastedTextH);
			break;
		}
		case cmd_NewKey:
			CKeyGenWizardDialog *kgw;
			
			kgw = (CKeyGenWizardDialog *)
				LWindow::CreateWindow(dialog_KeyGenWizard,
										CPGPKeys::TheApp());
			kgw->SetKeyTable(this);
			break;
		case cmd_OptionClear:
			SelectionAction(&CKeyTable::Delete);
			RemoveTerminatedKeys();
			ResyncTable(FALSE, TRUE);
			RedrawTable();
			CommitKeySet();
			break;
		case cmd_Clear:
			{
				Str255	question;
				GetIndString(	question,
								kStringListID,
								kClearQuestion);
				if(CWarningAlert::Display(kWACautionAlertType,
								kWAOKCancelStyle, kStringListID,
								kClearQuestion) == msg_OK)
				{
					SelectionAction(&CKeyTable::Delete);
					RemoveTerminatedKeys();
					ResyncTable(FALSE, TRUE);
					RedrawTable();
					CommitKeySet();
				}
			}
			break;
		case cmd_Revoke:
			SelectionAction(&CKeyTable::Revoke);
			ResyncTable(FALSE, TRUE);
			RedrawTable();
			CommitKeySet();
			break;
		case cmd_CollapseAll:
			CollapseKeys(FALSE);
			break;
		case cmd_ExpandAll:
			ExpandKeys();
			break;
		case cmd_SelectAll:
			UnselectAllCells();
			SelectAllCells();
			break;
		case cmd_SetDefault:
			if(!SetDefault())
				CWarningAlert::Display(kWACautionAlertType, kWAOKStyle,
								kStringListID, kCantDefaultStringID);
			break;
		case cmd_Info:
			SelectionAction(&CKeyTable::ShowKeyInfo);
			break;
		case cmd_Sign1:
			Sign();
			break;
		case cmd_AddName:
			AddUserID();
			RedrawTable();
			break;
		case cmd_KSGetKeys:
			if(gServerCallsPresent)
			{
				if(SelectionAction(&CKeyTable::GetCellOnServer))
				{
					ResyncTable(FALSE, TRUE);
					RedrawTable();
				}
			}
			break;
		case cmd_KSSendKeyToDomain:
			if(gServerCallsPresent)
			{
				UInt32	successfulSends;
				Str255	numSendsString;
				
				successfulSends = 
					SelectionAction(&CKeyTable::SendKeyToServer);
				if(successfulSends > 0)
				{
					NumToString(successfulSends, numSendsString);
					CWarningAlert::Display(kWANoteAlertType,
							kWAOKStyle, kStringListID,
							kServerSendSuccessful,
							numSendsString);
				}
			}
			break;
		case cmd_ImportKeys:
			{
				StandardFileReply	reply;
				SFTypeList			typeList;
				
				StandardGetFile(NULL, -1, typeList, &reply);
				if(reply.sfGood)
					ImportKeysFromFile(&reply.sfFile, FALSE);
			}
			break;
		case cmd_ExportKeys:
			ExportSelected();
			break;
		default:
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

	void
CKeyTable::GetColumnCheck(	Int16		columnID,
							Boolean		&outEnabled,
							Boolean		&outUsesMark,
							Char16		&outMark)
{
	if(ColumnVisible(columnID))
	{
		outUsesMark = true;
		outMark = checkMark;
	}
	else
	{
		outUsesMark = true;
		outMark = ' ';
	}
	outEnabled = true;
}

	void
CKeyTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	PGPError	err;
	PGPBoolean	secret,
				expired,
				revoked;
				
	switch(inCommand) 
	{
		case cmd_ViewValidity:
			GetColumnCheck(kValidityColumnID,
							outEnabled, outUsesMark, outMark);
			break;
		case cmd_ViewTrust:
			GetColumnCheck(kTrustColumnID,
							outEnabled, outUsesMark, outMark);
			break;
		case cmd_ViewSize:
			GetColumnCheck(kKeySizeColumnID,
							outEnabled, outUsesMark, outMark);
			break;
		case cmd_ViewKeyID:
			GetColumnCheck(kKeyIDColumnID, outEnabled, outUsesMark, outMark);
			break;
		case cmd_ViewCreationDate:
			GetColumnCheck(kCreationColumnID,
							outEnabled, outUsesMark, outMark);
			break;
		case cmd_ViewExpirationDate:
			GetColumnCheck(kExpirationColumnID,
							outEnabled, outUsesMark, outMark);
			break;
		case cmd_ViewMRK:
			GetColumnCheck(kMRKColumnID, outEnabled, outUsesMark, outMark);
			break;
		case cmd_ViewDescription:
			GetColumnCheck(kDescriptionColumnID,
							outEnabled, outUsesMark, outMark);
			break;
		case cmd_Paste:
			{	// Check if TEXT is in the Scrap
				Int32	offset;
				outEnabled = (::GetScrap(nil, 'TEXT', &offset) > 0);
			}
			break;
		case cmd_KSSendKeys:
		case cmd_KSSendKeyToDomain:
		case cmd_KSGetKeys:
			if(SelectionExists() && gServerCallsPresent)
				outEnabled = TRUE;
			break;
		case cmd_AddName:
		case cmd_Revoke:
			if(!mMutableRing || !mDefaultRing)
			{
				outEnabled = FALSE;
				break;
			}
		case cmd_SetDefault:
			{
				STableCell	cell,
							ocell;
				KeyTableRef	tableRef;
				UInt32		dataSize = sizeof(KeyTableRef);
				
				if(inCommand == cmd_SetDefault)
					GetIndString(	outName,
									kStringListID,
									kSetDefaultMenuItemID);
				if(SelectionExists())
				{
					ocell = cell = GetFirstSelectedCell();
					ocell.col = cell.col = 1;
					cell.row = GetWideOpenIndex(cell.row);
					GetCellData(cell, &tableRef, dataSize);
					if(inCommand == cmd_Revoke)
					{
						if(tableRef.type == kKey)
						{
							err = PGPGetKeyBoolean(tableRef.u.key,
									kPGPKeyPropIsSecret, &secret);
							pgpAssert(!err);
							err = PGPGetKeyBoolean(tableRef.u.key,
									kPGPKeyPropIsExpired, &expired);
							pgpAssert(!err);
							err = PGPGetKeyBoolean(tableRef.u.key,
									kPGPKeyPropIsRevoked, &revoked);
							pgpAssert(!err);
							if(secret && !expired && !revoked)
								outEnabled = TRUE;
						}
						else if(tableRef.type == kSignature)
						{
							// Could make sure the certifying key is a
							// secret key here
							outEnabled = TRUE;
						}
					}
					else
					{
						if(tableRef.type == kKey)
						{
							err = PGPGetKeyBoolean(tableRef.u.key,
									kPGPKeyPropIsSecret, &secret);
							pgpAssert(!err);
							err = PGPGetKeyBoolean(tableRef.u.key,
									kPGPKeyPropIsExpired, &expired);
							pgpAssert(!err);
							err = PGPGetKeyBoolean(tableRef.u.key,
									kPGPKeyPropIsRevoked, &revoked);
							pgpAssert(!err);
							if(secret && !expired && !revoked)
								outEnabled = TRUE;
						}
						else if((tableRef.type == kUserID) &&
								(inCommand == cmd_SetDefault))
						{
							outEnabled = TRUE;
							GetIndString(	outName,
											kStringListID,
											kSetPrimaryNameMenuItemID);
						}
					}
					if(GetNextSelectedCell(ocell))
						outEnabled = FALSE;
				}
			}
			break;
		case cmd_Clear:
		case cmd_OptionClear:
			if(!mMutableRing)
				break;
		case cmd_Info:	
		case cmd_Copy:
		case cmd_ExportKeys:
		case cmd_Sign1:
			outEnabled = SelectionExists();
			break;
		case cmd_CollapseAll:
			outEnabled = true;
			if(SelectionExists())
				GetIndString(	outName,
								kStringListID,
								kCollapseSelectionID);
			else
				GetIndString(	outName,
								kStringListID,
								kCollapseAllID);
			break;
		case cmd_ExpandAll:
			outEnabled = true;
			if(SelectionExists())
				GetIndString(	outName,
								kStringListID,
								kExpandSelectionID);
			else
				GetIndString(	outName,
								kStringListID,
								kExpandAllID);
			break;
		case cmd_NewKey:
#if	PGP_BUSINESS_SECURITY
			PGPBoolean		allowKG;
			
			err = PGPGetPrefBoolean(gAdminPrefRef, kPGPPrefAllowKeyGen,
									&allowKG);
			pgpAssertNoErr(err);
			if(!allowKG)
				break;
#endif
		case cmd_ImportKeys:	
			if(!mMutableRing || !mDefaultRing)
				break;
		case cmd_SelectAll:
			outEnabled = true;
			break;
		default:
			LCommander::FindCommandStatus(inCommand, outEnabled,
										outUsesMark, outMark, outName);
			break;
	}
}

	Boolean
CKeyTable::HandleKeyPress(const EventRecord&	inKeyEvent)
{
	Boolean		keyHandled	= true;
	Int16		theKey		= inKeyEvent.message & charCodeMask,
				charInx;
	STableCell	cell;
	TableIndexT	rows, cols;
	
	if(inKeyEvent.modifiers & cmdKey)
		keyHandled = LCommander::HandleKeyPress(inKeyEvent);
	else
	{
		SetUpdateCommandStatus(TRUE);
		switch(theKey)
		{
			default:
				if(((theKey >= ' ') && (theKey <= 'Z')) ||
					((theKey >= 'a') && (theKey <= 'z')))
				{
					if(inKeyEvent.when - kMaxKeypressInterval <=
						mLastKeypress)
					{
						if((charInx = strlen(mSearchString)) <
							kMaxSearchStringSize - 1)
						{
							mSearchString[charInx]		= theKey;
							mSearchString[charInx+1]	= 0;
						}
					}
					else
					{
						mSearchString[0] = theKey;
						mSearchString[1] = 0;
					}
					mLastKeypress = inKeyEvent.when;
					SelectSearchString();
				}
				else
					keyHandled = LCommander::HandleKeyPress(inKeyEvent);
				break;
			case char_UpArrow:
				cell = GetFirstSelectedCell();
				cell.col = 1;
				if(cell.row > 1)
					cell.row--;
				if(cell.row < 1)
					cell.row = 1;
				UnselectAllCells();
				ScrollCellIntoFrame(cell);
				SelectCell(cell);
				break;
			case char_DownArrow:
				GetTableSize(rows, cols);
				cell = GetFirstSelectedCell();
				cell.col = 1;
				cell.row++;
				if(cell.row > rows)
					cell.row = rows;
				UnselectAllCells();
				ScrollCellIntoFrame(cell);
				SelectCell(cell);
				break;
			case char_LeftArrow:
			case char_RightArrow:
				break;
			case char_Home:
				cell.row = cell.col = 1;
				ScrollCellIntoFrame(cell);
				break;
			case char_End:
				GetTableSize(rows, cols);
				cell.col = 1;
				cell.row = rows;
				ScrollCellIntoFrame(cell);
				break;
			case char_PageUp:
			{
				Rect	frameRect;
				
				CalcLocalFrameRect(frameRect);
				ScrollPinnedImageBy( 0,
					-UGraphicsUtilities::RectHeight(frameRect), true );
				break;
			}
			case char_PageDown:
			{
				Rect	frameRect;
				
				CalcLocalFrameRect(frameRect);
				ScrollPinnedImageBy( 0,
					UGraphicsUtilities::RectHeight(frameRect), true );
				break;
			}
			case char_Tab:
			case char_Return:
			case char_Enter:
				keyHandled = LCommander::HandleKeyPress(inKeyEvent);
				break;
			case char_FwdDelete:
			case char_Backspace:
				if(inKeyEvent.modifiers & optionKey)
					LCommander::GetTarget()->ProcessCommand(cmd_OptionClear);
				else
					LCommander::GetTarget()->ProcessCommand(cmd_Clear);
				break;
		}
	}
	return keyHandled;
}

	Int16
MatchUserIDs(char *uid1, char *uid2)
{
	Int16	matchChars = 0;
	char	a, b;
	
	while(*uid1 && *uid2)
	{
		a = *uid1++;	b = *uid2++;
		if((a >= 'a') && (a <= 'z'))
			a -= ('a' - 'A');
		if((b >= 'a') && (b <= 'z'))
			b -= ('a' - 'A');
		if(a == b)
			matchChars++;
		else
			break;
	}
	return matchChars;
}

	void
CKeyTable::SelectSearchString()
{
	STableCell	cell;
	TableIndexT	rows,
				cols,
				row,
				lastMarkerRow=0;
	KeyTableRef	tableRef;
	Uint32		dataSize = sizeof(KeyTableRef);
	PGPError	err;
	UInt32		len;
	char		userIDStr[kUserIDStringLength];
	Int16		searchScore,
				bestSearchScore = 0;
	
	UnselectAllCells();
	GetWideOpenTableSize(rows, cols);
	cell.col = 1;
	for(row=1;row<=rows;row++)
	{
		cell.row = row;
		GetCellData(	cell,
						&tableRef,
						dataSize);
		if(tableRef.type == kKey)
		{
			err = PGPGetPrimaryUserIDNameBuffer(tableRef.u.key,
						kUserIDStringLength - 1, userIDStr, &len);
			pgpAssertNoErr(err);
			userIDStr[len] = 0;
			searchScore = MatchUserIDs(userIDStr, mSearchString);
			if(searchScore > bestSearchScore)
			{
				bestSearchScore = searchScore;
				lastMarkerRow = row;	
			}
		}
	}
	cell.row = lastMarkerRow;
	if(!lastMarkerRow)
		mSearchString[0] = 0;
	else if(rows)
	{
		cell.row = GetExposedIndex(cell.row);
		ScrollCellIntoFrame(cell);
		SelectCell(cell);
	}
}

	void
CKeyTable::ScrollImageBy(	Int32		inLeftDelta,
							Int32		inTopDelta,
							Boolean		inRefresh)
{
	if(inLeftDelta != 0)
	{
		mKeyLabels->NotifyScrolled(inLeftDelta);
		if(inRefresh)
			mKeyLabels->Refresh();
	}
	LHierarchyTable::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

#pragma mark --- Drag and Drop ---

	void		
CKeyTable::GetDragHiliteRgn(RgnHandle	ioHiliteRgn)
{
	STableCell	cell;				// Loop thru all cells
	RgnHandle	cellRgn = ::NewRgn();

	cell.col = 1;
	for(cell.row = 0; cell.row <= mRows; cell.row++) 
	{
		if(CellIsSelected(cell))
		{
			Rect	cellRect, invertFrame;
			GetLocalCellRect(cell, cellRect);
			::InsetRect(&cellRect, 1, 1);
			cellRect.left = kDropFlagSlop;

	        invertFrame = cellRect;
	        invertFrame.right = invertFrame.left + 
	        				mColumnInfo.columnWidths[kUserIDColumnID];
			LocalToPortPoint(topLeft(invertFrame));
			LocalToPortPoint(botRight(invertFrame));
			PortToGlobalPoint(topLeft(invertFrame));
			PortToGlobalPoint(botRight(invertFrame));
			::RectRgn(cellRgn, &invertFrame);
			::UnionRgn(ioHiliteRgn, cellRgn, ioHiliteRgn);
		}
	}
	::DisposeRgn(cellRgn);
	
	RgnHandle	innerRgn = ::NewRgn();	// Carve out interior of region so
	::CopyRgn(ioHiliteRgn, innerRgn);	// that it's just a one-pixel thick
	::InsetRgn(innerRgn, 1, 1);			// outline of the item rectangle
	::DiffRgn(ioHiliteRgn, innerRgn, ioHiliteRgn);
	::DisposeRgn(innerRgn);
}


	void	
CKeyTable::AddFlavors(CKeyDragTask	*dragTask)
{
	pgpAssertAddrValid(dragTask, VoidAlign);
	mDragTask = dragTask;
	SelectionAction(&CKeyTable::AddDrag);
}

	Boolean	
CKeyTable::GetTextBlock(ItemReference	item, 
						void			**outData,
						Int32			*len)
{
	TableIndexT		inWideOpenRow =
						mCollapsableTree->GetWideOpenIndex(item);
	STableCell		woCell(inWideOpenRow, 1);
	KeyTableRef		keyRef;
	Uint32			dataSize = sizeof(KeyTableRef);
	PGPKeySetRef	singleKeySet;
	PGPError		err;

	GetCellData(woCell, &keyRef, dataSize);
	
	// This cell has been dragged.  We will now
	// create a text buffer of the ASCII armored
	// version of the key to hand back.
	
	*outData = NULL;
	if(keyRef.type == kKey)
	{
		err = PGPNewSingletonKeySet(keyRef.u.key, &singleKeySet);
		if(IsntPGPError(err) && IsntNull(singleKeySet))
		{
			PGPByte	*keyBuf;
			PGPSize	keyBufLen;
			char	comment[256];
							
			comment[0] = 0;
#if PGP_BUSINESS_SECURITY
			PGPGetPrefStringBuffer(
					gAdminPrefRef,
					kPGPPrefComments,
					sizeof(comment), comment);
#endif	// PGP_BUSINESS_SECURITY
			if(comment[0] == 0)
				PGPGetPrefStringBuffer(
						gPrefRef,
						kPGPPrefComment,
						sizeof(comment), comment);
			err = PGPExportKeySet( singleKeySet,
					PGPOAllocatedOutputBuffer(gPGPContext, &keyBuf,
									kMaxKeyBuffer, &keyBufLen),
					PGPOCommentString(gPGPContext, comment),
					PGPOVersionString(gPGPContext, pgpVersionHeaderString),
					PGPOLastOption(gPGPContext));
			pgpAssertNoErr(err);
			PGPFreeKeySet(singleKeySet);
			if(IsntPGPError(err))
			{
				*outData = (void *)keyBuf;
				*len = keyBufLen;
				return TRUE;
			}
			else
				ReportPGPError(err);
		}
	}
	return FALSE;
}

	Boolean
CKeyTable::GetHFS(	ItemReference	item,
					DragReference	dragRef,
					FSSpec			*spec)
{
	OSErr		theErr;
	AEDesc		droppedLocation;
	CInfoPBRec	catInfo;
	
	pgpAssertAddrValid(spec, FSSpec);
	theErr = ::GetDropLocation(dragRef, &droppedLocation);
	pgpAssert(theErr == noErr);
	if(theErr == noErr)
	{
		Boolean	isChanged;

		pgpAssert(droppedLocation.descriptorType == typeAlias);
		StHandleLocker	lock(droppedLocation.dataHandle);
		theErr = ::ResolveAlias(	nil, 
									AliasHandle(droppedLocation.dataHandle),
									spec,
									&isChanged);
		
		pgpAssert(theErr == noErr);
	}
	
	if (theErr == noErr)
	{
		catInfo.hFileInfo.ioCompletion 	= nil;
		catInfo.hFileInfo.ioNamePtr 	= spec->name;
		catInfo.hFileInfo.ioVRefNum		= spec->vRefNum;
		catInfo.hFileInfo.ioFDirIndex	= 0;
		catInfo.hFileInfo.ioDirID		= spec->parID;
		
		theErr = PBGetCatInfoSync(&catInfo);
		

		spec->parID 						= catInfo.hFileInfo.ioDirID;
	
		pgpAssert(theErr == noErr);
	}

	if (theErr == noErr)
	{
		TableIndexT		inWideOpenRow =
							mCollapsableTree->GetWideOpenIndex(item);
		STableCell		woCell(inWideOpenRow, 1);
		KeyTableRef		keyRef;
		Uint32			dataSize = sizeof(KeyTableRef),
						len;
		uchar			userIDStr[kUserIDStringLength];
		PGPKeySetRef	singleKeySet;
		PGPError		err;
		
		GetCellData(woCell, &keyRef, dataSize);
		// This cell has been dragged to the Finder.  We will now
		// create a file of the ASCII armored version of the key.
		
		if(keyRef.type == kKey)
		{
			err = PGPNewSingletonKeySet(keyRef.u.key, &singleKeySet);
			if(IsntPGPError(err) && IsntNull(singleKeySet))
			{
				err = PGPGetPrimaryUserIDNameBuffer(keyRef.u.key,
						kUserIDStringLength - 1, (char *)userIDStr, &len);
				pgpAssert(!err);
				userIDStr[len] = 0;
				if(len > 31)
				{
					userIDStr[30] = '\311';
					userIDStr[31] = 0;
				}
				pgpAssert(0);
				// Temporarily commented out following line
				//err = pgpExportKeyFile(singleKeySet, userIDStr);
				pgpAssert(!err);
				//pgpFixBeforeShip("handles dupe filenames??");
				PGPFreeKeySet(singleKeySet);
				return TRUE;
			}
		}
	}
	
	return theErr == noErr;
}

	Boolean	
CKeyTable::AddDrag(STableCell& aCell)
{
	pgpAssertAddrValid(mDragTask, VoidAlign);
	mDragTask->AddThisFlavor(aCell.row);
	return true;
}

	void
CKeyTable::FocusDropArea()
{
	OutOfFocus(nil);
	FocusDraw();
}

	void
CKeyTable::HiliteDropArea(
	DragReference	inDragRef)
{
	Rect		dropRect;
	RgnHandle	dropRgn;
	
	CalcLocalFrameRect(dropRect);
	/*::PenSize(2,2);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::FrameRect(&dropRect);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
	::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
	::PenSize(1,1);*/
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
	::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
	dropRgn = ::NewRgn();
	::RectRgn(dropRgn, &dropRect);
	::ShowDragHilite(inDragRef, dropRgn, true);
	::DisposeRgn(dropRgn);
}

	void
CKeyTable::UnhiliteDropArea(
	DragReference	inDragRef)
{
	Rect		dropRect;
	//RgnHandle	dropRgn;

	CalcPortFrameRect(dropRect);
	::HideDragHilite(inDragRef);
	/*dropRgn = ::NewRgn();
	::OpenRgn();
	::PenSize(2,2);
	::FrameRect(&dropRect);
	::CloseRgn(dropRgn);
	InvalPortRgn(dropRgn);
	::DisposeRgn(dropRgn);*/
}

	Boolean
CKeyTable::ItemIsAcceptable(DragReference inDragRef, ItemReference inItemRef)
{
	Boolean			isAcceptable = FALSE;
	ushort			numFlavors;
	OSStatus		status;
	
	if(!mMutableRing || mSendingDrag)
		return false;
	status = CountDragItemFlavors(inDragRef, inItemRef, &numFlavors);
	if(IsntErr(status))
	{
		for(ushort flavorIndex = 1; flavorIndex <= numFlavors; flavorIndex++)
		{
			FlavorType	flavorType;
			
			status = GetFlavorType(inDragRef, inItemRef, flavorIndex,
									&flavorType);
			if(IsntErr(status))
			{
				if(flavorType == 'TEXT')
				{
					isAcceptable = TRUE;
				}
				else if(flavorType == flavorTypeHFS)
				{
					HFSFlavor	flavorData;
					ByteCount	dataSize;
					ByteCount	dataOffset;
					
					dataSize 	= sizeof(flavorData);
					dataOffset	= 0;
					
					status = GetFlavorData(inDragRef, inItemRef, flavorType,
									&flavorData, (Int32 *) &dataSize,
									dataOffset);
					if(IsntErr(status))
					{
						// Check for dragged folders and/or disks
						
						if(flavorData.fileType != 'disk' &&
							flavorData.fileType != 'fold')
						{
							isAcceptable = true;
						}
					}
				}
				else if(flavorType == flavorTypePromiseHFS)
				{
					// We will accept promised flavors as well.
					// These come from products
					// like Stuffit Deluxe, which have no existing
					// files but are willing
					// to create them on demand if we so desire.
					
					PromiseHFSFlavor	flavorData;
					ByteCount			dataSize;
					ByteCount			dataOffset;
					
					dataSize 	= sizeof(flavorData);
					dataOffset	= 0;
					
					status = GetFlavorData(inDragRef, inItemRef, flavorType,
									&flavorData, (Int32 *) &dataSize,
									dataOffset);
					if(IsntErr(status))
					{
						// Check for dragged folders and/or disks
						
						if(flavorData.fileType != 'disk' &&
							flavorData.fileType != 'fold')
						{
							isAcceptable = true;
						}
					}
					
				}
				if(isAcceptable)
					break;
			}
		}
	}
	return(isAcceptable);
}

	void
CKeyTable::StepForward()
{
	if(!WeAreFront())
	{
		if(!mNotified)
		{
			Handle iconSuite;
			
			::GetIndString(	sNotifyString,
							kStringListID,
							kStepForwardID);
			::GetIconSuite(&iconSuite, 128, svAllSmallData);
			sImportNotification.qType = nmType;
			sImportNotification.nmMark = 1;
			sImportNotification.nmIcon = iconSuite;
			sImportNotification.nmSound = NULL;
			sImportNotification.nmStr = sNotifyString;
			sImportNotification.nmResp = NULL;
			sImportNotification.nmRefCon = 0;
			::NMInstall(&sImportNotification);
			mBlockDialogDelays = 3;
			mNotified = TRUE;
		}
	}
	else
	{
		mBlockDialogDelays = 0;
		mNotified = FALSE;
	}
}

	void
CKeyTable::ReceiveDragItem(
	DragReference	inDragRef,
	DragAttributes	inDragAttrs,
	ItemReference	inItemRef,
	Rect			&/*inItemBounds*/)	// In Local coordinates
{
	Boolean			received = FALSE,
					localSend;
	ushort			numFlavors;
	OSStatus		status;
	
	localSend = (inDragAttrs & dragInsideSenderApplication);
	
	status = CountDragItemFlavors(inDragRef, inItemRef, &numFlavors);
	if(IsntErr(status))
	{
		for(ushort flavorIndex = 1; flavorIndex <= numFlavors; flavorIndex++)
		{
			FlavorType	flavorType;
			
			status = GetFlavorType(inDragRef, inItemRef, flavorIndex,
									&flavorType);
			if(IsntErr(status))
			{
				if(flavorType == 'TEXT')
				{
					Size			droppedSize,
									expectedSize;
					PGPByte			*droppedBuf;
					
					status = GetFlavorDataSize(inDragRef, inItemRef,
												flavorType, &expectedSize);
					if(IsntErr(status) && IsntNull(droppedBuf =
								(PGPByte *)pgpAlloc(expectedSize)))
					{
						droppedSize = expectedSize;
						status = GetFlavorData(inDragRef, inItemRef,
								flavorType, droppedBuf, &droppedSize, 0);
						pgpAssertNoErr(status);
						pgpAssert(droppedSize == expectedSize);
						if(IsntErr(status))
						{
							KeyImportStruct *kis;
							
							kis = (KeyImportStruct *)
									pgpAlloc(sizeof(KeyImportStruct));
							if(IsntNull(kis))
							{
								kis->savedImportIsFile = FALSE;
								kis->bypassImportDialog = localSend;
								kis->savedImportBuffer = droppedBuf;
								kis->savedImportBufferSize = droppedSize;
								kis->next = mImports;
								mImports = kis;
								StartIdling();
								StepForward();
							}
						}
						else
							pgpFree(droppedBuf);
					}
					received = TRUE;
				}
				else if(flavorType == flavorTypeHFS)
				{
					HFSFlavor	flavorData;
					ByteCount	dataSize;
					
					dataSize 	= sizeof(flavorData);
					
					status = GetFlavorData(inDragRef, inItemRef, flavorType,
							&flavorData, (Int32 *) &dataSize, 0);
					if(IsntErr(status))
					{
						ImportKeysFromFile(&flavorData.fileSpec, localSend);
						received = TRUE;
					}
				}
				else if(flavorType == flavorTypePromiseHFS)
				{
					PromiseHFSFlavor	flavorData;
					ByteCount			dataSize;
					
					dataSize 	= sizeof(flavorData);
					
					status = GetFlavorData(inDragRef, inItemRef, flavorType,
									&flavorData, (Int32 *) &dataSize, 0);
					if(IsntErr(status))
					{
						SysBeep(1);
					//pgpFixBeforeShip("how do we get the FSSpec here...");
					// apparently we never get this message anyway....
						received = TRUE;
					}
					
				}
				if(received)
					break;
			}
		}
	}
}

