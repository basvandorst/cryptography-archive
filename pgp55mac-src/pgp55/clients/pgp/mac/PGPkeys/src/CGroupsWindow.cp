/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGroupsWindow.cp,v 1.25 1997/11/03 23:30:43 heller Exp $
____________________________________________________________________________*/
#include "CGroupsWindow.h"

#include "CKeyTable.h"
#include "CKeyView.h"
#include "CPGPKeys.h"
#include "CKeyInfoWindow.h"
#include "ResourceConstants.h"

#include "CWarningAlert.h"
#include "MacFiles.h"
#include "MacStrings.h"
#include "CPGPStDialogHandler.h"

#include "pgpFileSpec.h"
#include "pgpMem.h"
#include "pgpKeys.h"
#include "pgpUtilities.h"
#include "pgpClientPrefs.h"
#include "pgpOpenPrefs.h"
#include "pgpUserInterface.h"


const Int16 		kDefaultColumnWidth			= 	16;
const Int16			kRowHeight					= 	18;
const ResIDT		kNewGroupDialogID			= 	155;
const Int16			kUserIDStringLength			= 	256;
const FlavorType	kGroupIDDragFlavor			= 	'gkID';
const FlavorType	kGroupItemDragFlavor		=	'giID';
const ResIDT		kGroupColumnTitleStringsID	= 	1021;
enum	{
			kStringListID		= 1020,
			kBadGroupInfoStringID = 1,
			kDeleteConfirmStringID,
			kUnknownKeyStringID,
			kExpandSelectionID,
			kCollapseSelectionID,
			kExpandAllID,
			kCollapseAllID
		};	

GroupTableColumnInfo	CGroupsTable::kDefaultColumnInfo	=
	{
		3,
		kAddressColumnID,
		kValidityColumnID,
		kDescriptionColumnID,
		
		286, 51, 384
	};
	

#pragma mark --- Window Functions ---

CGroupsWindow::CGroupsWindow()
{
}

CGroupsWindow::CGroupsWindow(LStream *inStream)
	:	LWindow(inStream)
{
	mGroupsRef		= kPGPInvalidRef;
	mGroupsFile		= kPGPInvalidRef;
}

CGroupsWindow::~CGroupsWindow()
{
	if(PGPRefIsValid(mGroupsRef) && PGPRefIsValid(mGroupsFile))
	{
		if(PGPGroupSetNeedsCommit(mGroupsRef))
			PGPSaveGroupSetToFile(mGroupsRef, mGroupsFile);
	}
	if(PGPRefIsValid(mGroupsRef))
		PGPFreeGroupSet(mGroupsRef);
	if(PGPRefIsValid(mGroupsFile))
		PFLFreeFileSpec(mGroupsFile);
}

	void
CGroupsWindow::FinishCreateSelf()
{
	LWindow::FinishCreateSelf();
	mBorderView = (LView *)FindPaneByID(kBorderViewMarker);
	mGroupsTable = (CGroupsTable *)FindPaneByID(CGroupsTable::class_ID);
	mGroupLabels = (CGroupTableLabels *)FindPaneByID(kGroupLabels);
	
	mGroupLabels->SetTable(mGroupsTable);
	mGroupsTable->SetLabels(mGroupLabels);
	
	{
		PGPError		err;
		FSSpec			fsSpec;
		PFLFileSpecRef	pflSpec;
		
		err = PGPGetGroupsFileFSSpec( &fsSpec );
		if( IsntPGPError( err ) )
		{
			err = PFLNewFileSpecFromFSSpec(gPFLContext, &fsSpec, &pflSpec);
			if(IsntPGPError(err))
			{
				PFLFileSpecMacMetaInfo metaInfo;
				
				pgpClearMemory(&metaInfo, sizeof(PFLFileSpecMacMetaInfo));
				metaInfo.fInfo.fdType = 'pgGR';
				metaInfo.fInfo.fdCreator = 'pgpK';
				PFLSetFileSpecMetaInfo(pflSpec, &metaInfo);
				mGroupsFile = pflSpec;
				err = PGPNewGroupSetFromFile(pflSpec, &mGroupsRef);

				if ( IsntPGPError( err ) )
				{
					err	= PGPSortGroupSetStd( mGroupsRef,
							CPGPKeys::TheApp()->GetKeySet() );
				}
			}
		}
		if(IsErr(err) && !PGPRefIsValid(mGroupsRef))
		{
			// We couldn't open the file, let's just try to make it work
			err = PGPNewGroupSet(gPFLContext, &mGroupsRef);
			pgpAssertNoErr(err);
		}
		if(IsErr(err))
			ReportPGPError(err);
		pgpAssertAddrValid(mGroupsRef, PGPGroupSetRef);
		mGroupsTable->SetGroupSet(mGroupsRef, mGroupsFile);
	}
	SwitchTarget(mGroupsTable);
	Show();
}

	void
CGroupsWindow::DrawSelf()
{
	Rect	frame;
	
	FocusDraw();
	CalcLocalFrameRect(frame);
	LWindow::DrawSelf();
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
	::MoveTo(frame.right - 1, frame.top + 1);
	::LineTo(frame.right - 1, frame.bottom - 16);
	::LineTo(frame.right - 16, frame.bottom - 16);
	::LineTo(frame.right - 16, frame.bottom - 1);
	::LineTo(frame.left + 1, frame.bottom - 1);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::MoveTo(frame.left, frame.bottom);
	::LineTo(frame.left, frame.top);
	::LineTo(frame.right, frame.top);
	
	mBorderView->CalcPortFrameRect(frame);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
	::MoveTo(frame.left, frame.bottom-1);
	::LineTo(frame.left, frame.top);
	::LineTo(frame.right-1, frame.top);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::MoveTo(frame.left+1, frame.bottom-1);
	::LineTo(frame.right - 16, frame.bottom-1);
	::LineTo(frame.right - 16, frame.bottom - 16);
	::LineTo(frame.right-1, frame.bottom - 16);
	::LineTo(frame.right-1, frame.top+1);
}

	void
CGroupsWindow::AttemptClose()
{
	BroadcastMessage(kGroupsWindowClosed, NULL);
	LWindow::AttemptClose();
}

	void
CGroupsWindow::ResizeFrameBy(
	Int16		inWidthDelta,
	Int16		inHeightDelta,
	Boolean		inRefresh)
{
	Rect	frame;

	LWindow::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	
	// following corrects a drawing problem with resizing that
	// was caused by our 3D border effect
	CalcPortFrameRect(frame);
	InvalPortRect(&frame);
}

	void
CGroupsWindow::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	}
}

	void
CGroupsWindow::InvalidateCaches()
{
	mGroupsTable->InvalidateCaches();
}


#pragma mark --- Table Functions ---

CGroupsTable::CGroupsTable(
	LStream	*inStream)
		:	LHierarchyTable(inStream),
			LDragAndDrop(UQDGlobals::GetCurrentPort(), this)
{
	mTableGeometry	= new LTableMultiGeometry(this,	kDefaultColumnWidth,
													kRowHeight);
	mTableSelector	= new LTableMultiSelector(this);
	mTableStorage	= new LTableArrayStorage(this, (ulong)0);
	::BlockMoveData(&kDefaultColumnInfo, &mColumnInfo,
						sizeof(GroupTableColumnInfo));
	mGroupsRef	= kPGPInvalidRef;
	mGroupsFile	= kPGPInvalidRef;
	mSendingDrag = FALSE;
}

void
CGroupsTable::FinishCreateSelf()
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

CGroupsTable::~CGroupsTable()
{
}

	void
CGroupsTable::SetGroupSet(PGPGroupSetRef groupRef, PFLFileSpecRef groupsFile)
{
	mGroupsRef	= groupRef;
	mGroupsFile	= groupsFile;
	ResyncTable(TRUE, FALSE);
	RedrawTable();
}

	void
CGroupsTable::InvalidateCaches()
{
	RebuildTable();
}

	void
CGroupsTable::RebuildTable()
{
	RemoveAllRows(false);
	ResortGroups();
	ResyncTable(TRUE, FALSE);
	RedrawTable();
}

	void
CGroupsTable::ResyncTable(Boolean collapse, Boolean onlyUnmarked)
{
	PGPUInt32			numGroups,
						groupIndex,
						itemIndex,
						numKeyEntries,
						numTotalEntries;
	PGPGroupID			groupID;
	PGPError			err;
	GroupTableEntry		groupEntry;
	PGPGroupInfo		groupInfo;
	PGPGroupItem		groupItem;
	TableIndexT			lastGroupRow,
						lastRow,
						itemSubRow;
	
	if(!onlyUnmarked)
		RemoveAllRows(false);
	lastRow = lastGroupRow = 0;
	err = PGPCountGroupsInSet(mGroupsRef, &numGroups);
	pgpAssertNoErr(err);
	for(groupIndex = 0; groupIndex < numGroups ; groupIndex++)
	{
		err = PGPGetIndGroupID(mGroupsRef, groupIndex, &groupID);
		pgpAssertNoErr(err);
		err = PGPGetGroupInfo(mGroupsRef, groupID, &groupInfo);
		pgpAssertNoErr(err);
		++lastRow;
		if(groupInfo.userValue == 0 || !onlyUnmarked)
		{
			groupEntry.isTopGroup = TRUE;
			groupEntry.u.groupID = groupID;
			lastGroupRow = InsertSiblingRows(1, lastGroupRow, &groupEntry,
								sizeof(GroupTableEntry),
								TRUE, TRUE);
			PGPSetGroupUserValue(mGroupsRef, groupID, (PGPUserValue)1);
		}
		else if(!lastGroupRow)
			lastGroupRow = 1;
		if(lastRow > 1)
			lastGroupRow = lastRow;
		err = PGPCountGroupItems(mGroupsRef, groupID, FALSE,
							&numKeyEntries, &numTotalEntries);
		pgpAssertNoErr(err);
		if(numTotalEntries > 0)
		{
			for(itemIndex = 0; itemIndex < numTotalEntries; itemIndex++)
			{
				err = PGPGetIndGroupItem(mGroupsRef, groupID,
											itemIndex, &groupItem);
				pgpAssertNoErr(err);
				lastRow++;
				if(groupItem.userValue == 0 || !onlyUnmarked)
				{
					groupEntry.isTopGroup = FALSE;
					groupEntry.u.item.ownerGroupID = groupID;
					groupEntry.u.item.cachedKey = kPGPInvalidRef;
					pgpCopyMemory(&groupItem, &groupEntry.u.item.groupItem,
									sizeof(PGPGroupItem));
									
					if(itemIndex == 0)
					{
						InsertChildRows(1, lastGroupRow, &groupEntry,
											sizeof(GroupTableEntry),
											FALSE, TRUE);
						itemSubRow = lastGroupRow + 1;
					}
					else
						itemSubRow = InsertSiblingRows(1, itemSubRow,
										&groupEntry, sizeof(GroupTableEntry),
										FALSE, TRUE);
					PGPSetIndGroupItemUserValue(mGroupsRef, groupID,
												itemIndex, (PGPUserValue)1);
				}
				else
				{
					if(itemIndex == 0)
						itemSubRow = lastGroupRow + 1;
					else
						itemSubRow = lastRow;
				}
			}
		}
	}
	if(collapse)
		CollapseGroups(FALSE);
}

	void
CGroupsTable::RedrawTable()
{
	Rect frame;
	
	CalcPortFrameRect(frame);
	InvalPortRect(&frame);
}

	void
CGroupsTable::CollapseGroups(Boolean all)
{
	TableIndexT	rows,
				cols,
				rowIndex;
	STableCell	cell(1, 1);
	Boolean		selectedOnly;
	Rect		frame;
	
	if(all)
		selectedOnly = FALSE;
	else
		selectedOnly = (mTableSelector->GetFirstSelectedRow() > 0);
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
	
	CalcPortFrameRect(frame);
	InvalPortRect(&frame);
}

	void
CGroupsTable::ExpandGroups()
{
	TableIndexT	rows,
				cols,
				rowIndex;
	STableCell	cell(1, 1);
	Boolean		selectedOnly;
	
	selectedOnly = (mTableSelector->GetFirstSelectedRow() > 0);
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

	void
CGroupsTable::DrawCell(
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
			STableCell			woCell(woRow, 1);
			Uint32				dataSize = sizeof(GroupTableEntry);
			GroupTableEntry		tableEntry;
			
			GetCellData(woCell, &tableEntry, dataSize);
			::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			::TextMode(srcOr);
			switch(mColumnInfo.columns[inCell.col-1])
			{
				case kAddressColumnID:
					DrawAddressColumnCell(&tableEntry, inLocalRect, depth);
					SetCellData(woCell, &tableEntry, dataSize);
					DrawDropFlag(inCell, woRow);
					break;
				case kValidityColumnID:
					DrawValidityColumnCell(&tableEntry, inLocalRect, depth);
					break;
				case kDescriptionColumnID:
					DrawDescriptionColumnCell(&tableEntry, inLocalRect, depth);
					break;
			}
			if(depth >= 8)
				RefreshBorder(inLocalRect);
		}
	}
}

	void
CGroupsTable::CacheKey(GroupTableEntry *tableEntry)
{
	PGPKeyID	keyID;
	PGPError	err;
	
	if(!PGPRefIsValid(tableEntry->u.item.cachedKey) &&
		tableEntry->u.item.groupItem.type == kPGPGroupItem_KeyID)
	{
		err = PGPImportKeyID(tableEntry->u.item.groupItem.u.key.exportedKeyID,
							&keyID);
		if(IsntPGPError(err))
		{
			err = PGPGetKeyByKeyID(CPGPKeys::TheApp()->GetKeySet(),
							&keyID, (PGPPublicKeyAlgorithm)
							tableEntry->u.item.groupItem.u.key.algorithm,
							&tableEntry->u.item.cachedKey);
		}
	}
}

	void
CGroupsTable::DrawAddressColumnCell(
	GroupTableEntry		*gtr,
	Rect				cellRect,
	Int16				depth)
{
	PGPError		err;
	Rect			sortColumnRect;
	Str255			pstr;
	ResIDT			iconID;
	Int16			slop;
	PGPGroupInfo	groupInfo;

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
	if(gtr->isTopGroup)
	{
		iconID = kDHGroupIconID;
		CKeyTable::DrawIcon(iconID, cellRect, 1);
		
		err = PGPGetGroupInfo(mGroupsRef, gtr->u.groupID, &groupInfo);
		pgpAssertNoErr(err);
		CToPString(groupInfo.name, pstr);
		::TextFont(geneva);
		::TextSize(9);
		::TextFace(0);
		::TruncString(	mColumnInfo.columnWidths[kAddressColumnID] -
						kDropFlagSlop - kLeftIndent -
						kIconWidth - kLeftBorder,
						pstr, truncMiddle);
		::MoveTo(cellRect.left + kDropFlagSlop + kLeftIndent +
						kIconWidth + kLeftBorder, cellRect.bottom -
						kBottomBorder);
		::DrawString(pstr);
	}
	else
	{
		pstr[0] = 0;
		::TextFace(0);
		if(gtr->u.item.groupItem.type == kPGPGroupItem_KeyID)
		{
			char		cstr[kUserIDStringLength];
			PGPSize		len;
			
			CacheKey(gtr);
			if(PGPRefIsValid(gtr->u.item.cachedKey))
			{
				PGPBoolean				expired,
										revoked,
										disabled,
										secret;
				PGPPublicKeyAlgorithm	algorithm;
				PGPKeyRef				key;
				
				key 		= gtr->u.item.cachedKey;
				algorithm 	= (PGPPublicKeyAlgorithm)
									gtr->u.item.groupItem.u.key.algorithm;
				
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

				if( revoked )
				{
					if(algorithm == kPGPPublicKeyAlgorithm_DSA)
						iconID = kDSARevokedKeyID;
					else
						iconID = kRSARevokedKeyID;
				}
				else if( expired )
				{
					if( algorithm == kPGPPublicKeyAlgorithm_DSA )
						iconID = kDSAExpiredKeyID;
					else
						iconID = kRSAExpiredKeyID;
				}
				else if( disabled )
				{
					if( algorithm == kPGPPublicKeyAlgorithm_DSA )
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
				else
				{
					if(algorithm == kPGPPublicKeyAlgorithm_DSA)
						iconID = kDSAKeyIconID;
					else
						iconID = kRSAKeyIconID;
				}
				
				len = kUserIDStringLength;
				err = PGPGetPrimaryUserIDNameBuffer( key,
					kUserIDStringLength, cstr, &len);
				if(err != kPGPError_BufferTooSmall)
					pgpAssertNoErr(err);
				CToPString(cstr, pstr);
			}
			else
			{
				PGPKeyID		keyID;
				char			keyIDString[ kPGPMaxKeyIDStringSize ];
				
				iconID = kDSADisabledKeyID;
				::GetIndString(pstr, kStringListID, kUnknownKeyStringID);
				err = PGPImportKeyID(gtr->u.item.groupItem.u.key.exportedKeyID,
							&keyID);
				pgpAssertNoErr(err);
				if(IsntPGPError(err))
				{
					err = PGPGetKeyIDString( &keyID,
											kPGPKeyIDString_Abbreviated,
											keyIDString);
					pgpAssertNoErr(err);
					if(IsntPGPError(err))
					{
						Str255		keyIDPString;
						
						CToPString(keyIDString, keyIDPString);
						AppendPString(keyIDPString, pstr);
					}
				}
				::TextFace(italic);
			}
		}
		else
		{
			iconID = kDHGroupIconID;
			err = PGPGetGroupInfo(mGroupsRef, gtr->u.item.groupItem.u.group.id,
							&groupInfo);
			pgpAssertNoErr(err);
			if(IsntPGPError(err))
				CToPString(groupInfo.name, pstr);
		}
		CKeyTable::DrawIcon(iconID, cellRect, 2);
		
		::TextFont(geneva);
		::TextSize(9);
		slop = kDropFlagSlop + kLeftIndent + kLevelIndent + kIconWidth +
				kLeftBorder;
		::TruncString(	mColumnInfo.columnWidths[kAddressColumnID] -
						slop, pstr, truncMiddle);
		::MoveTo(cellRect.left + slop, cellRect.bottom - kBottomBorder);
		::DrawString(pstr);
	}
	::TextFace(0);
}

	void
CGroupsTable::DrawValidity(Int32 trustValue, Rect cellRect, Int16 depth)
{
	RGBColor	trustColor = { 52223, 52223, 65535 };
	Int16		width;
	PGPBoolean	showMarginal;
	PGPError	err;
	
	const Int16		kMaxValidity 			=	2;
	const Int16 	kBarRightBorder			=	10;
	const Int16	 	kBarLeftBorder			=	4;
	const Int16		kBarHeight				=	10;
	
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

		CKeyTable::DrawIcon(iconID, cellRect, -2);
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
			trustValue = kMaxValidity;
		if(depth >= 8)
		{
			::RGBForeColor(&trustColor);
			::PaintRect(&cellRect);
		}
		
		cellRect.right -= width * (kMaxValidity - trustValue) /
				kMaxValidity;
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray9));
		::PaintRect(&cellRect);
		ApplyForeAndBackColors();
	}
}

	void
CGroupsTable::DrawValidityColumnCell(
	GroupTableEntry		*gtr,
	Rect				cellRect,
	Int16				depth)
{
	PGPError	err;
	PGPValidity	validity	= kPGPValidity_Unknown;
	PGPUInt32	notFound;
	PGPInt32	displayValidity	= 0;
	
	if(gtr->isTopGroup)
	{
		err = PGPGetGroupLowestValidity(mGroupsRef,
						gtr->u.groupID, CPGPKeys::TheApp()->GetKeySet(),
						&validity, &notFound);
		pgpAssertNoErr(err);
	}
	else
	{
		if(gtr->u.item.groupItem.type == kPGPGroupItem_KeyID)
		{
			CacheKey(gtr);
			if(PGPRefIsValid(gtr->u.item.cachedKey))
			{
				err = PGPGetPrimaryUserIDValidity(gtr->u.item.cachedKey,
													&validity);
				pgpAssertNoErr(err);
			}
		}
		else
		{
			err = PGPGetGroupLowestValidity(mGroupsRef,
							gtr->u.item.groupItem.u.group.id,
							CPGPKeys::TheApp()->GetKeySet(),
							&validity, &notFound);
			pgpAssertNoErr(err);
		}
	}
	switch(validity)
	{
		default:
		case kPGPValidity_Unknown:
		case kPGPValidity_Invalid:
			displayValidity = 0;
			break;
		case kPGPValidity_Marginal:
			displayValidity = 1;
			break;
		case kPGPValidity_Complete:
			displayValidity = 2;
			break;
	}
	DrawValidity( displayValidity, cellRect, depth);
}

	void
CGroupsTable::DrawDescriptionColumnCell(
	GroupTableEntry		*gtr,
	Rect				cellRect,
	Int16				depth)
{
	Str255			str;
	PGPError		err;
	PGPGroupInfo	groupInfo;
	
	if(gtr->isTopGroup)
	{
		err = PGPGetGroupInfo(mGroupsRef, gtr->u.groupID, &groupInfo);
		pgpAssertNoErr(err);
		CToPString(groupInfo.description, str);
		::TextFont(geneva);
		::TextSize(9);
		::TextFace(0);
		::MoveTo(	cellRect.left + kLeftIndent,
					cellRect.bottom - kBottomBorder);
		::DrawString(str);
	}
}

	void
CGroupsTable::UpdateValidityColumn()
{
	TableIndexT	numRows, numCols;
	GetTableSize(numRows, numCols);
	
	STableCell	topCell(1, 2);
	STableCell 	botCell(numRows, 2);
	STableCell	cell;
	
	FocusDraw();
	for(cell = botCell; cell.row >= topCell.row; cell.row--) 
	{
		Rect cellRect;
		if(GetLocalCellRect(cell, cellRect))
			DrawCell(cell, cellRect);
	}
}

	void
CGroupsTable::RefreshBorder(Rect cellRect)
{
	Rect frame;
	
	CalcLocalFrameRect(frame);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
	::MoveTo(frame.left, cellRect.bottom - 1);
	::LineTo(frame.right - 1, cellRect.bottom - 1);
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
}

	void
CGroupsTable::ShowInfo()
{
	TableIndexT	numRows, numCols;
	GetTableSize(numRows, numCols);
	
	STableCell	topCell(1, 1);
	STableCell 	botCell(numRows, 1);
	STableCell	cell;
	PGPError		err;
			
	for(cell = botCell; cell.row >= topCell.row; cell.row--) 
	{
		if(CellIsSelected(cell)) 
		{
			TableIndexT		inWideOpenRow =
							mCollapsableTree->GetWideOpenIndex(cell.row);
			STableCell		woCell(inWideOpenRow, 1);
			GroupTableEntry	tableEntry;
			Uint32			dataSize = sizeof(GroupTableEntry);
			
			GetCellData(woCell, &tableEntry, dataSize);
			if(tableEntry.isTopGroup ||
				tableEntry.u.item.groupItem.type == kPGPGroupItem_Group)
			{
				PGPGroupInfo	groupInfo;
				PGPGroupID		id;
				
				if(tableEntry.isTopGroup)
					id = tableEntry.u.groupID;
				else
					id = tableEntry.u.item.groupItem.u.group.id;
				err = PGPGetGroupInfo(mGroupsRef, id, &groupInfo);
				pgpAssertNoErr(err);
				if(IsntPGPError(err))
				{
					if(EditGroup(&groupInfo))
					{
						err = PGPSetGroupName(mGroupsRef,
											id, groupInfo.name);
						pgpAssertNoErr(err);
						err = PGPSetGroupDescription(mGroupsRef,
											id, groupInfo.description);
						pgpAssertNoErr(err);
						Draw(nil);
					}
				}
			}
			else
			{
				if(PGPRefIsValid(tableEntry.u.item.cachedKey))
				{
					if(!CKeyInfoWindow::OpenKeyInfo(
							tableEntry.u.item.cachedKey, 
							CPGPKeys::TheApp()->GetDefaultKeyView()->
							GetKeyTable()))
						break;	// Too many key windows, break out
				}
			}
		}
	}
}

	void
CGroupsTable::ClickCell(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
{
	if(GetClickCount() == 2)
	{
		ShowInfo();
	}
	else if (LDropArea::DragAndDropIsPresent() && 
			::WaitMouseMoved(inMouseDown.macEvent.where))
		StartDrag(inCell, inMouseDown);
	else if(inCell.col == 1)
		LHierarchyTable::ClickCell(inCell, inMouseDown);
}

	void
CGroupsTable::StartDrag(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
{
	Rect			cellRect;
	TableIndexT		inWideOpenRow =
					mCollapsableTree->GetWideOpenIndex(inCell.row);
	STableCell		woCell(inWideOpenRow, 1);
	GroupTableEntry	tableEntry;
	Uint32			dataSize = sizeof(GroupTableEntry);
	
	GetCellData(woCell, &tableEntry, dataSize);

	mSendingDrag = TRUE;
	
	// Setup the drag rect
	GetLocalCellRect(inCell, cellRect);
	::InsetRect(&cellRect, 1, 1);
	cellRect.left = kDropFlagSlop;
	cellRect.right = mColumnInfo.columnWidths[kAddressColumnID];
	if(tableEntry.isTopGroup)
	{
		LDragTask	drag(inMouseDown.macEvent, cellRect, 
							1, kGroupIDDragFlavor,
							&tableEntry.u.groupID,
							sizeof(PGPGroupID), flavorSenderOnly);
	}
	else
	{
		LDragTask	drag(inMouseDown.macEvent, cellRect, 
							1, kGroupItemDragFlavor,
							&tableEntry.u.item.groupItem,
							sizeof(PGPGroupItem), flavorSenderOnly);
	}
	
	mSendingDrag = FALSE;
}

	void
CGroupsTable::ClickSelf(
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
		}
	} else {							// Click is outside of any Cell
		UnselectAllCells();
	}
}

	void
CGroupsTable::HiliteCellActively(
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
					mColumnInfo.columnWidths[kAddressColumnID];
	        UDrawingUtils::SetHiliteModeOn();
			::InvertRect(&cellFrame);
		}
	}
}

	void
CGroupsTable::HiliteCellInactively(
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
					mColumnInfo.columnWidths[kAddressColumnID];
			UDrawingUtils::SetHiliteModeOn();
			::FrameRect(&cellFrame);
		}
	}
}

	void
CGroupsTable::HiliteCellDrag(
	const STableCell	&inCell)
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
					mColumnInfo.columnWidths[kAddressColumnID];
			cellFrame.bottom -= 1;
			::InvertRect(&cellFrame);
		}
	}
}

	void
CGroupsTable::DrawSelf()
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

	Boolean
CGroupsTable::EditGroup(PGPGroupInfo *groupInfo)
{
	CPGPStDialogHandler	newGroupDialog(kNewGroupDialogID, this);
	LWindow				*ngd;
	MessageT			dialogMessage;
	Str255				pstr;
	LGAEditField		*mAddressEditField,
						*mDescriptionEditField;
	Boolean				success = FALSE;

	const PaneIDT	kAddressEditField		= 'eAdd';
	const PaneIDT	kDescEditField			= 'eDes';
	
	ngd = newGroupDialog.GetDialog();
	mAddressEditField = (LGAEditField *)ngd->FindPaneByID(kAddressEditField);
	mDescriptionEditField =
		(LGAEditField *)ngd->FindPaneByID(kDescEditField);
	if(groupInfo->id != kPGPInvalidGroupID)
	{
		CToPString(groupInfo->name, pstr);
		mAddressEditField->SetDescriptor(pstr);
		CToPString(groupInfo->description, pstr);
		mDescriptionEditField->SetDescriptor(pstr);
	}
	ngd->Show();
badInput:
	do
	{
		dialogMessage = newGroupDialog.DoDialog();
	} while(dialogMessage != msg_OK &&
			dialogMessage != msg_Cancel);
	if(dialogMessage == msg_OK)
	{
		mAddressEditField->GetDescriptor(pstr);
		if(pstr[0] == '\0')
		{
			CWarningAlert::Display(kWAStopAlertType, kWAOKStyle,
						kStringListID, kBadGroupInfoStringID);
			goto badInput;
		}
		PToCString(pstr, groupInfo->name);
		mDescriptionEditField->GetDescriptor(pstr);
		PToCString(pstr, groupInfo->description);
		success = TRUE;
	}
	return success;
}


	void
CGroupsTable::DeleteSelection()
{	
	TableIndexT	numRows, numCols;
	GetTableSize(numRows, numCols);
	
	STableCell	topCell(1, 1);
	STableCell 	botCell(numRows, 1);
	STableCell	cell;
	
	for(cell = botCell; cell.row >= topCell.row; cell.row--) 
	{
		if(CellIsSelected(cell)) 
		{
			TableIndexT		inWideOpenRow =
								mCollapsableTree->GetWideOpenIndex(cell.row);
			STableCell		woCell(inWideOpenRow, 1);
			GroupTableEntry	tableEntry;
			PGPError		err;
			Uint32			dataSize = sizeof(GroupTableEntry);
			
			GetCellData(woCell, &tableEntry, dataSize);
			RemoveRows(1, inWideOpenRow, true);
			if(tableEntry.isTopGroup)
			{
				err = PGPDeleteGroup(mGroupsRef, tableEntry.u.groupID);
				pgpAssertNoErr(err);
			}
			else
			{
				PGPUInt32		numKeys,
								numTotal,
								itemIndex;
				PGPGroupItem	groupItem;
				
				err = PGPCountGroupItems(mGroupsRef,
							tableEntry.u.item.ownerGroupID,
							FALSE, &numKeys, &numTotal);
				pgpAssertNoErr(err);
				if(IsntPGPError(err) && numTotal > 0)
				{
					for(itemIndex = 0; itemIndex < numTotal; itemIndex++)
					{
						err = PGPGetIndGroupItem(mGroupsRef,
								tableEntry.u.item.ownerGroupID,
								itemIndex, &groupItem);
						pgpAssertNoErr(err);
						if(pgpMemoryEqual(&groupItem.u,
							&tableEntry.u.item.groupItem.u,
							sizeof(groupItem.u)))
						{
							err = PGPDeleteIndItemFromGroup(mGroupsRef,
										tableEntry.u.item.ownerGroupID,
										itemIndex);
							pgpAssertNoErr(err);
							break;
						}
					}
				}
			}
		}
	}
	UpdateValidityColumn();
	if(PGPGroupSetNeedsCommit(mGroupsRef))
		PGPSaveGroupSetToFile(mGroupsRef, mGroupsFile);
}

	void
CGroupsTable::UpdateFromServer()
{
	TableIndexT	numRows, numCols;
	GetTableSize(numRows, numCols);
	
	STableCell	topCell(1, 1);
	STableCell 	botCell(numRows, 1);
	STableCell	cell;
	
	for(cell = botCell; cell.row >= topCell.row; cell.row--) 
	{
		if(CellIsSelected(cell)) 
		{
			TableIndexT		inWideOpenRow =
								mCollapsableTree->GetWideOpenIndex(cell.row);
			STableCell		woCell(inWideOpenRow, 1);
			GroupTableEntry	tableEntry;
			Uint32			dataSize = sizeof(GroupTableEntry);
			PGPKeyID		keyID;
			PGPKeySetRef	keySet = kPGPInvalidRef;
			PGPError		err;
			
			keySet = NULL;
			GetCellData(woCell, &tableEntry, dataSize);
			if(tableEntry.isTopGroup)
			{
				err = PGPGetGroupFromServer(gPGPContext,
						mGroupsRef, tableEntry.u.groupID, NULL, 0, &keySet);
				if(IsPGPError(err) && (err != kPGPError_UserAbort))
					ReportPGPError(err);
			}
			else
			{
				err = PGPImportKeyID(
						tableEntry.u.item.groupItem.u.key.exportedKeyID,
							&keyID);
				if(IsntPGPError(err) )
				{
					err = PGPGetKeyFromServer(
								gPGPContext, &keyID, NULL, NULL, 0, &keySet);
					if(IsPGPError(err) && (err != kPGPError_UserAbort))
						ReportPGPError(err);
				}
			}
			if(PGPRefIsValid(keySet))
			{
				CPGPKeys::TheApp()->GetDefaultKeyView()->GetKeyTable()->
					ImportKeysFromKeySet(keySet);
			}
		}
	}
}

	void
CGroupsTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	switch(inCommand) 
	{
		case cmd_Clear:
		case cmd_OptionClear:
			if(mTableSelector->GetFirstSelectedRow() > 0)
				outEnabled = true;
			break;
		case cmd_SelectAll:
		case cmd_NewGroup:
			outEnabled = true;
			break;
		case cmd_CollapseAll:
			outEnabled = true;
			if(mTableSelector->GetFirstSelectedRow() > 0)
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
			if(mTableSelector->GetFirstSelectedRow() > 0)
				GetIndString(	outName,
								kStringListID,
								kExpandSelectionID);
			else
				GetIndString(	outName,
								kStringListID,
								kExpandAllID);
			break;
		case cmd_Info:
			if(mTableSelector->GetFirstSelectedRow() > 0)
				outEnabled = TRUE;
			break;
		case cmd_KSGetKeys:
			if(mTableSelector->GetFirstSelectedRow() > 0 &&
				gServerCallsPresent)
				outEnabled = TRUE;
			break;
		default:
			 LCommander::FindCommandStatus(inCommand, outEnabled,
										outUsesMark, outMark, outName);
			break;
	}
}

	Boolean
CGroupsTable::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true,
				bypass = FALSE;

	switch (inCommand) 
	{
		case cmd_OptionClear:
			bypass = TRUE;
		case cmd_Clear:
		{
			if(bypass || CWarningAlert::Display(kWACautionAlertType,
							kWAOKCancelStyle, kStringListID,
							kDeleteConfirmStringID) == msg_OK)
			{
				DeleteSelection();
			}
			break;
		}
		case cmd_NewGroup:
		{
			PGPGroupInfo	groupInfo;
			PGPError		err;
			
			pgpClearMemory(&groupInfo, sizeof(PGPGroupInfo));
			if(EditGroup(&groupInfo))
			{
				PGPGroupID		newGroupID = kPGPInvalidGroupID;
				
				err = PGPNewGroup(mGroupsRef, groupInfo.name,
							groupInfo.description, &newGroupID);
				pgpAssertNoErr(err);
				if(IsntPGPError(err))
				{
					if ( IsntPGPError( err ) )
					{
						err	= PGPSortGroupSetStd( mGroupsRef,
								CPGPKeys::TheApp()->GetKeySet() );
					}

					ResyncTable(FALSE, TRUE);
				}
			}
			break;
		}
		case cmd_CollapseAll:
			CollapseGroups(FALSE);
			break;
		case cmd_ExpandAll:
			ExpandGroups();
			break;
		case cmd_SelectAll:
			UnselectAllCells();
			SelectAllCells();
			break;
		case cmd_Info:
			ShowInfo();
			break;
		case cmd_KSGetKeys:
			UpdateFromServer();
			break;
		default:
			cmdHandled =  LCommander::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

	void
CGroupsTable::FocusDropArea()
{
	OutOfFocus(nil);
	FocusDraw();
}

	Boolean
CGroupsTable::ItemIsAcceptable(
	DragReference		inDragRef,
	ItemReference		inItemRef)
{
	Boolean			isAcceptable = FALSE;
	ushort			numFlavors;
	OSStatus		status;
	DragAttributes	attributes;
	
	GetDragAttributes(inDragRef, &attributes);
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
				switch(flavorType)
				{
					case 'TEXT':
					case kGroupIDDragFlavor:
					case kGroupItemDragFlavor:
						isAcceptable = TRUE;
						break;
					case flavorTypeHFS:
					{
						HFSFlavor	flavorData;
						ByteCount	dataSize;
						ByteCount	dataOffset;
						
						dataSize 	= sizeof(flavorData);
						dataOffset	= 0;
						
						status = GetFlavorData(inDragRef, inItemRef,
										flavorType, &flavorData,
										(Int32 *) &dataSize, dataOffset);
						if(IsntErr(status))
						{
							// Check for dragged folders and/or disks
							
							if(flavorData.fileType == 'pgGR' &&
								flavorData.fileCreator == 'pgpK')
							{
								isAcceptable = true;
							}
						}
						break;
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
CGroupsTable::ResortGroups()
{
	PGPError	err;
	
	err	= PGPSortGroupSetStd( mGroupsRef,
			CPGPKeys::TheApp()->GetKeySet() );
	pgpAssertNoErr(err);
}

	PGPError
CGroupsTable::AddKeyIDToGroup(
	PGPByte				*keyID,
	PGPSize				keyIDSize,
	PGPUInt32			algorithm,
	const STableCell	&groupCell)
{
	TableIndexT			woRow = GetWideOpenIndex(groupCell.row);
	STableCell			woCell(woRow, 1);
	Uint32				dataSize = sizeof(GroupTableEntry);
	GroupTableEntry		tableEntry;
	PGPError			err = kPGPError_UnknownError;
	
	GetCellData(woCell, &tableEntry, dataSize);
	if(tableEntry.isTopGroup)
	{
		PGPGroupItem	groupItem;
		
		groupItem.type = kPGPGroupItem_KeyID;
		groupItem.userValue = 0;
		groupItem.u.key.length = keyIDSize;
		groupItem.u.key.algorithm = algorithm;
		pgpCopyMemory(keyID, groupItem.u.key.exportedKeyID, keyIDSize);
		err = PGPAddItemToGroup(mGroupsRef, &groupItem, tableEntry.u.groupID);
		ResortGroups();
		ResyncTable(FALSE, TRUE);
		UpdateValidityColumn();
		if(PGPGroupSetNeedsCommit(mGroupsRef))
			PGPSaveGroupSetToFile(mGroupsRef, mGroupsFile);
	}
	return err;
}

	PGPError
CGroupsTable::AddGroupToGroup(
	PGPGroupID			groupID,
	const STableCell	&groupCell)
{
	TableIndexT			woRow = GetWideOpenIndex(groupCell.row);
	STableCell			woCell(woRow, 1);
	Uint32				dataSize = sizeof(GroupTableEntry);
	GroupTableEntry		tableEntry;
	PGPError			err = kPGPError_UnknownError;
	
	GetCellData(woCell, &tableEntry, dataSize);
	if(tableEntry.isTopGroup)
	{
		PGPGroupItem	groupItem;
		
		groupItem.type = kPGPGroupItem_Group;
		groupItem.userValue = 0;
		groupItem.u.group.id = groupID;
		err = PGPAddItemToGroup(mGroupsRef, &groupItem, tableEntry.u.groupID);
		ResortGroups();
		ResyncTable(FALSE, TRUE);
		UpdateValidityColumn();
		if(PGPGroupSetNeedsCommit(mGroupsRef))
			PGPSaveGroupSetToFile(mGroupsRef, mGroupsFile);
	}
	return err;
}

	PGPError
CGroupsTable::AddGroupItemToGroup(
	PGPGroupItem		*item,
	const STableCell	&groupCell)
{
	TableIndexT			woRow = GetWideOpenIndex(groupCell.row);
	STableCell			woCell(woRow, 1);
	Uint32				dataSize = sizeof(GroupTableEntry);
	GroupTableEntry		tableEntry;
	PGPError			err = kPGPError_UnknownError;
	
	GetCellData(woCell, &tableEntry, dataSize);
	if(tableEntry.isTopGroup)
	{
		item->userValue = 0;
		err = PGPAddItemToGroup(mGroupsRef, item, tableEntry.u.groupID);
		ResortGroups();
		ResyncTable(FALSE, TRUE);
		UpdateValidityColumn();
		if(PGPGroupSetNeedsCommit(mGroupsRef))
			PGPSaveGroupSetToFile(mGroupsRef, mGroupsFile);
	}
	return err;
}

	void
CGroupsTable::ReceiveDragItem(
	DragReference	inDragRef,
	DragAttributes	inDragAttrs,
	ItemReference	inItemRef,
	Rect			&/*inItemBounds*/)	// In Local coordinates
{
	Boolean			received = FALSE;
	ushort			numFlavors;
	OSStatus		status;
	Size			droppedSize,
					expectedSize;
	PGPError		err;
	
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
				status = GetFlavorDataSize(inDragRef, inItemRef,
											flavorType, &expectedSize);
				if(flavorType == 'TEXT')
				{
					PGPByte			*droppedBuf;
					
					if((mLastDragCell.row == 0) || (mLastDragCell.col != 1))
						return;
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
							PGPKeySetRef	newKeySet;
							
							err = PGPImportKeySet( gPGPContext, &newKeySet,
									PGPOInputBuffer( gPGPContext,
											droppedBuf, droppedSize ),
									PGPOLastOption( gPGPContext ) );
							pgpAssertNoErr(err);
							if(IsntPGPError(err) && PGPRefIsValid(newKeySet))
							{
								PGPKeyIterRef	keyIter;
								PGPKeyListRef	keyList;
								
								err = PGPOrderKeySet(newKeySet,
												kPGPAnyOrdering,
												&keyList);
								if(IsntPGPError(err))
								{
									err = PGPNewKeyIter(keyList, &keyIter);
									if(IsntPGPError(err))
									{
										PGPKeyRef	key;
										PGPKeyID	keyID;
										PGPSize		keyIDSize;
										PGPByte		keyIDExport[
											kPGPMaxExportedKeyIDSize ];
										
										while(IsntPGPError(
											PGPKeyIterNext(keyIter, &key))
											&& PGPRefIsValid(key))
										{
											err = PGPGetKeyIDFromKey(key,
													&keyID);
											if(IsntPGPError(err) )
											{
												err = PGPExportKeyID( &keyID,
														keyIDExport,
														&keyIDSize);
												if(IsntPGPError(err))
												{
													PGPInt32 algorithm;
													
													err = PGPGetKeyNumber(key,
														kPGPKeyPropAlgID, 
														&algorithm);
													pgpAssertNoErr(err);
													err = AddKeyIDToGroup(
														keyIDExport,
														keyIDSize,
														algorithm,
														mLastDragCell);
													if(IsntPGPError(err))
														received = TRUE;
												}
											}
										}
										PGPFreeKeyIter(keyIter);
									}
									PGPFreeKeyList(keyList);
								}
								PGPFreeKeySet(newKeySet);
							}
						}
						pgpFree(droppedBuf);
					}
				}
				else if(flavorType == kGroupIDDragFlavor)
				{
					PGPGroupID	groupID;
					
					if((mLastDragCell.row == 0) || (mLastDragCell.col != 1))
						return;
					if(IsntErr(status) && expectedSize == sizeof(PGPGroupID))
					{
						droppedSize = expectedSize;
						status = GetFlavorData(inDragRef, inItemRef,
								flavorType, &groupID, &droppedSize, 0);
						pgpAssertNoErr(status);
						if(IsntErr(status))
						{
							err = AddGroupToGroup(groupID, mLastDragCell);
							if(IsntPGPError(err))
								received = TRUE;
						}
					}
				}
				else if(flavorType == kGroupItemDragFlavor)
				{
					PGPGroupItem	item;
					
					if((mLastDragCell.row == 0) || (mLastDragCell.col != 1))
						return;
					if(IsntErr(status) && expectedSize == sizeof(PGPGroupItem))
					{
						droppedSize = expectedSize;
						status = GetFlavorData(inDragRef, inItemRef,
								flavorType, &item, &droppedSize, 0);
						pgpAssertNoErr(status);
						if(IsntErr(status))
						{
							err = AddGroupItemToGroup(&item, mLastDragCell);
							if(IsntPGPError(err))
								received = TRUE;
						}
					}
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
						PGPGroupSetRef	mergeGroupSet;
						
						err = PGPNewGroupSetFromFSSpec(gPFLContext,
							&flavorData.fileSpec, &mergeGroupSet);
						if(IsntPGPError(err))
						{
							err = PGPMergeGroupSets(mergeGroupSet, mGroupsRef);
							pgpAssertNoErr(err);
							PGPFreeGroupSet(mergeGroupSet);
							ResortGroups();
							ResyncTable(FALSE, TRUE);
							UpdateValidityColumn();
							if(PGPGroupSetNeedsCommit(mGroupsRef))
								PGPSaveGroupSetToFile(mGroupsRef, mGroupsFile);
							if(IsntPGPError(err))
								received = TRUE;
						}
					}
				}
				if(received)
					break;
			}
		}
	}
}

	void
CGroupsTable::HiliteDropArea(
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
CGroupsTable::UnhiliteDropArea(
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

	void
CGroupsTable::EnterDropArea(
	DragReference	inDragRef,
	Boolean			inDragHasLeftSender)
{
	LDragAndDrop::EnterDropArea(inDragRef, inDragHasLeftSender);
	UnselectAllCells();
	mLastDragCell.SetCell(0,0);
}

	void
CGroupsTable::LeaveDropArea(
	DragReference	inDragRef)
{
	if(mLastDragCell.row != 0)
		HiliteCellDrag(mLastDragCell);
	LDragAndDrop::LeaveDropArea(inDragRef);
}

	void
CGroupsTable::InsideDropArea(
	DragReference	inDragRef)
{
	Point		mousePos;
	SPoint32	imagePt;
	STableCell	overCell;
	Boolean		foundCell;
	Uint32		nestingLevel;
	
	if(IsntErr(::GetDragMouse(inDragRef, &mousePos, nil)))
	{
		::GlobalToLocal(&mousePos);
		LocalToImagePoint(mousePos, imagePt);
		foundCell = GetCellHitBy(imagePt, overCell);
		if(foundCell)
			nestingLevel = GetNestingLevel(GetWideOpenIndex(overCell.row));
		if(foundCell && (overCell.col == 1) && (nestingLevel == 0))
		{
			if(overCell != mLastDragCell)
			{
				if(mLastDragCell.row != 0)
					HiliteCellDrag(mLastDragCell);
				mLastDragCell = overCell;
				HiliteCellDrag(mLastDragCell);
			}
		}
		else
		{
			if(mLastDragCell.row != 0)
			{
				HiliteCellDrag(mLastDragCell);
				mLastDragCell.SetCell(0,0);
			}
		}
	}
}

	void
CGroupsTable::ScrollImageBy(	Int32		inLeftDelta,
							Int32		inTopDelta,
							Boolean		inRefresh)
{
	if(inLeftDelta != 0)
	{
		mLabels->NotifyScrolled(inLeftDelta);
		if(inRefresh)
			mLabels->Refresh();
	}
	LHierarchyTable::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

	void
CGroupsTable::GetColumnInfo(GroupTableColumnInfo *columnInfo)
{
	::BlockMoveData(&mColumnInfo, columnInfo, sizeof(GroupTableColumnInfo));
}

	Boolean
CGroupsTable::HandleKeyPress(const EventRecord&	inKeyEvent)
{
	Boolean		keyHandled	= true;
	Int16		theKey		= inKeyEvent.message & charCodeMask;
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

#pragma mark --- Label Functions ---

#define TYPEICONSLOP		16
#define LEFTMARGIN			4

CGroupTableLabels::CGroupTableLabels(LStream *inStream)
		: LPane(inStream)
{
	mTable			= nil;
	mScrollDelta	= 0;
}

CGroupTableLabels::~CGroupTableLabels()
{
}

	void
CGroupTableLabels::FinishCreateSelf()
{	
}

	void
CGroupTableLabels::DrawSelf()
{
	Rect					frame,
							borderClip,
							textClip,
							colFrame;
	short					columnIndex;
	Str255					title;
	GroupTableColumnInfo	columnInfo;
	
	FocusDraw();
	CalcLocalFrameRect(frame);
	if(mActive == triState_On)
	{
		::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray3));
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray12));
	}
	else
	{
		::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
		::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray5));
	}
	::EraseRect(&frame);
	::FrameRect(&frame);
	::TextFont(geneva);
	::TextSize(9);
	::TextFace(0);
	
	borderClip = textClip = frame;
	::InsetRect(&textClip, 2, 2);
	::InsetRect(&borderClip, 1, 1);
		
	mTable->GetColumnInfo(&columnInfo);
	for(columnIndex = 0;columnIndex<columnInfo.numActiveColumns;
		columnIndex++)
	{
		GetColumnRect(columnIndex, colFrame);
		{
			StClipRgnState		clip(borderClip);
			
			if(mActive == triState_On)
			{
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
				::MoveTo(colFrame.left+1, colFrame.bottom-1);
				::LineTo(colFrame.left+1, colFrame.top+1);
				::LineTo(colFrame.right-1, colFrame.top+1);
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
				::MoveTo(colFrame.left+2, colFrame.bottom-1);
				::LineTo(colFrame.right-1, colFrame.bottom-1);
				::LineTo(colFrame.right-1, colFrame.top+2);
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray12));
			}
			else
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray5));
			::MoveTo(colFrame.right, colFrame.bottom-1);
			::LineTo(colFrame.right, colFrame.top+1);
		}
		{
			StClipRgnState		clip(textClip);
			
			::GetIndString(title, kGroupColumnTitleStringsID, 1 + 
							columnInfo.columns[columnIndex]);
			::MoveTo(colFrame.left + LEFTMARGIN + (columnIndex == 0 ?
						kDropFlagSlop : 0), frame.top + 12);
			if(mActive == triState_On)
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			else
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray7));
			::DrawString(title);
		}
	}
}

	void
CGroupTableLabels::GetColumnRect(Int16 col, Rect &colRect)
{
	GroupTableColumnInfo	columnInfo;
	Int16					pos,
							colIndex;
	Rect					localRect;
	
	CalcLocalFrameRect(localRect);
	mTable->GetColumnInfo(&columnInfo);
	pos = 0;
	for(colIndex = 0; colIndex < columnInfo.numActiveColumns; colIndex++)
	{
		if(colIndex == col)
		{
			colRect.top		= localRect.top;
			colRect.bottom	= localRect.bottom - 1;
			colRect.left	= localRect.left + pos;
			colRect.right	= colRect.left +
					columnInfo.columnWidths[columnInfo.columns[colIndex]];
			::OffsetRect(&colRect, -mScrollDelta, 0);
			break;
		}
		pos += columnInfo.columnWidths[columnInfo.columns[colIndex]];
	}
}

	void
CGroupTableLabels::NotifyScrolled(Int32 delta)
{
	mScrollDelta += delta;
}

	void
CGroupTableLabels::ActivateSelf()
{
	if(mActive == triState_On)
		Refresh ();
}

	void
CGroupTableLabels::DeactivateSelf()
{
	if(mActive == triState_Off || mActive == triState_Latent)
		Refresh ();
}

