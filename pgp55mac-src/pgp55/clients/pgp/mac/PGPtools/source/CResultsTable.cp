/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <LTableArrayStorage.h>
#include <LTableMultiGeometry.h>
#include <UGAColorRamp.h>
#include <UGraphicsUtilities.h>
#include <UTextTraits.h>

#include "MacFiles.h"
#include "MacQuickdraw.h"
#include "MacStrings.h"
#include "pflContext.h"
#include "pflPrefTypes.h"
#include "pgpClientPrefs.h"
#include "pgpMem.h"
#include "pgpOpenPrefs.h"
#include "pgpUtilities.h"

#include "PGPtoolsResources.h"

#include "CResultsTable.h"
#include "CResultsWindow.h"

const ResIDT	kSummaryItemTextTraitsResID		= 133;
const ResIDT	kSingleItemTextTraitsResID		= 134;

const ResIDT	kSignatureBadIconResID			= 3000;
const ResIDT	kSignatureValidIconResID		= 3001;
const ResID 	kValidKeyIconResID 				= 3100;
const ResID 	kInvalidKeyIconResID 			= 3101;

const short		kDropFlagMargin			= 20;
const short		kValidityBarHeight		= 10;

typedef enum ResultItemType
{
	kResultTypeSummaryItem	= 1,
	kResultTypeSingleItem
	
} ResultItemType;

typedef struct ResultsSummaryItem
{
	UInt32		numItems;
	UInt32		dateAndTime;
	
} ResultsSummaryItem;

typedef struct ResultsSingleItem
{
	uchar			itemName[64];
	uchar			statusText[128];
	PGPValidity		validity;
	PGPTime			signingDate;
	Boolean			validSignature;
	Boolean			badSignature;
	ResID			iconResID;
	
} ResultsSingleItem;

typedef struct ResultsTableItem
{
	ResultItemType	itemType;
	
	union
	{
		ResultsSummaryItem	summary;
		ResultsSingleItem	single;
	};
		
} ResultsTableItem;

CResultsTable::CResultsTable(LStream *inStream)
	: LHierarchyTable( inStream )
{
	PFLContextRef	pflContext;
	PGPError		err;
	
	mTableGeometry	= new LTableMultiGeometry( this, 484, 18 );
	mTableSelector	= nil;	// Don't allow delections in this table.
	mTableStorage	= new LTableArrayStorage(this, sizeof( ResultsTableItem));
	
	// Get marginal validity setting
	err	= PFLNewContext( &pflContext );
	if ( IsntPGPError( err ) )
	{
		PGPPrefRef		prefRef;
		
		err = PGPOpenClientPrefs( pflContext, &prefRef );
		if( IsntPGPError( err ) )
		{
			(void) PGPGetPrefBoolean( prefRef,
						kPGPPrefDisplayMarginalValidity,
						&mShowMarginalValidity );

			(void) PGPGetPrefBoolean( prefRef,
						kPGPPrefMarginalIsInvalid,
						&mMarginalIsInvalid );

			(void) PGPClosePrefFile( prefRef );
		}
		
		PFLFreeContext( pflContext );
	}
}

CResultsTable::~CResultsTable(void)
{
	// LTableView will take care of mTableGeometry, mTableSelector,
	// and mTableStorage
}

	void
CResultsTable::FinishCreateSelf(void)
{
	LHierarchyTable::FinishCreateSelf();
	
	InsertCols( 1, 1, nil, 0, FALSE );
	
	UTextTraits::SetPortTextTraits( kSummaryItemTextTraitsResID );
	GetFontInfo( &mSummaryItemFontInfo );

	UTextTraits::SetPortTextTraits( kSingleItemTextTraitsResID );
	GetFontInfo( &mSingleItemFontInfo );
}

	TableIndexT
CResultsTable::AddSummaryRow(void)
{
	TableIndexT			lastWideOpenRow;
	TableIndexT			parentRow;
	ResultsTableItem	tableItem;
	TableIndexT			newRowIndex;
	
	lastWideOpenRow = mCollapsableTree->CountNodes();
	
	parentRow = GetParentIndex( lastWideOpenRow );
	if( parentRow == 0 )
		parentRow = lastWideOpenRow;
		
	tableItem.itemType 			= kResultTypeSummaryItem;
	tableItem.summary.numItems	= 0;
	
	GetDateTime( &tableItem.summary.dateAndTime );
	
	newRowIndex = InsertSiblingRows( 1, parentRow, &tableItem,
							sizeof( tableItem ), TRUE, Refresh_Yes );
			
	return( newRowIndex );
}

	void
CResultsTable::IncrementVerifiedCount(void)
{
	ResultsTableItem	tableItem;
	STableCell			wideOpenCell;
	UInt32				dataSize;
	TableIndexT			parentRow;
	TableIndexT			numRows;
	TableIndexT			numColumns;
	
	// Get the parent row
	GetWideOpenTableSize( numRows, numColumns );
	parentRow = GetParentIndex( numRows );
	if( parentRow == 0 )
		parentRow = numRows;
		
	// Update the parent item count
	
	wideOpenCell.SetCell( parentRow, 1 );
	dataSize = sizeof( tableItem );
	GetCellData( wideOpenCell, &tableItem, dataSize );
	
	pgpAssert( tableItem.itemType == kResultTypeSummaryItem );
	tableItem.summary.numItems += 1;
	SetCellData( wideOpenCell, &tableItem, sizeof( tableItem ) );
	RefreshCell( wideOpenCell );
}

	static short
GetSignatureStatusStrIndex(
	PGPtoolsSignatureData			*signatureData,
	const PGPSharedKeyProperties	*keyProperties)
{
	short		strIndex;
	
	if ( signatureData->data.checked )
	{
		strIndex = kVerifiedSignatureStrIndex;

		if ( keyProperties->isRevoked )
		{
			strIndex = kVerifiedRevokedStrIndex;
		}
		else if ( keyProperties->isExpired )
		{
			strIndex = kVerifiedExpiredStrIndex;
		}
		else if ( keyProperties->isDisabled )
		{
			strIndex = kVerifiedDisabledStrIndex;
		}
		else if ( keyProperties->isCorrupt )
		{
			strIndex = kVerifiedCorrputStrIndex;
		}
	}
	else if( signatureData->signingKeyIDStr[0] != 0 )
	{
		strIndex = kCouldNotVerifyKeyIDStrIndex;
	}
	else
	{
		strIndex = kCouldNotVerifyStrIndex;
	}
	
	
	return( strIndex );
}

	void
CResultsTable::AddProcessedItemRow(
	ConstStr255Param		itemName,
	PGPtoolsSignatureData	*signatureData)
{
	ResultsTableItem	tableItem;
	STableCell			wideOpenCell;
	UInt32				numChildren;
	TableIndexT			newRowIndex;
	TableIndexT			parentRow;
	TableIndexT			numRows;
	TableIndexT			numColumns;
	PGPError			err = kPGPError_NoErr;
	Str255				statusText;
	char				userID[256];
	short				statusTextStrIndex;
	PGPSharedKeyProperties	keyProperties;
	
	pgpAssertAddrValid( itemName, uchar );
	pgpAssertAddrValid( signatureData, PGPEventSignatureData );
	
	IncrementVerifiedCount();
	// Get the parent row
	GetWideOpenTableSize( numRows, numColumns );
	parentRow = GetParentIndex( numRows );
	if( parentRow == 0 )
		parentRow = numRows;
		
	pgpClearMemory( &tableItem, sizeof( tableItem ) );
	pgpClearMemory( &keyProperties, sizeof( keyProperties ) );

	if( PGPKeyRefIsValid( signatureData->data.signingKey ) )
	{
		// Get the primary user ID and key properties for the signing key:
		userID[0] = 0;
		
		err	= PGPGetPrimaryUserIDNameBuffer(
					signatureData->data.signingKey,
					sizeof( userID ), userID, NULL );
		pgpAssertNoErr( err );
		
		err	= PGPSharedGetKeyProperties( signatureData->data.signingKey,
					userID, &keyProperties );
		pgpAssertNoErr( err );
	}
	
	statusTextStrIndex = GetSignatureStatusStrIndex( signatureData,
							&keyProperties );
		
	tableItem.itemType 				= kResultTypeSingleItem;
	tableItem.single.validity		= keyProperties.userValidity;
	tableItem.single.signingDate	= signatureData->data.creationTime;
	tableItem.single.validSignature	= ( signatureData->data.checked &&
										signatureData->data.verified );
	tableItem.single.badSignature	= ( signatureData->data.checked &&
										! signatureData->data.verified );
	
	if( tableItem.single.validSignature &&
		signatureData->haveResourceFork &&
		signatureData->didNotVerifyResourceFork )
	{
		Str255	formatStr;
		
		GetIndString( formatStr, kResultsTableStringListResID,
					kDataForkOnlyFormatStrIndex );
		PrintPString( formatStr, formatStr, itemName );
		CopyPStringMax( formatStr, tableItem.single.itemName,
				sizeof( tableItem.single.itemName ) - 1 );
	}
	else
	{
		CopyPStringMax( itemName, tableItem.single.itemName,
				sizeof( tableItem.single.itemName ) - 1 );
	}
		
	GetIndString( statusText, kResultsTableStringListResID,
				statusTextStrIndex );
	
	if( tableItem.single.validSignature )
	{
		tableItem.single.iconResID = kSignatureValidIconResID;
	}
	else if( tableItem.single.badSignature )
	{
		tableItem.single.iconResID = kSignatureBadIconResID;
	}

	if( statusTextStrIndex == kCouldNotVerifyKeyIDStrIndex )
	{
		Str255	keyIDStr;
		
		CToPString( signatureData->signingKeyIDStr, keyIDStr );
		PrintPString( statusText, statusText, keyIDStr );
	}
	else if( statusTextStrIndex != kCouldNotVerifyStrIndex )
	{
		Str255	pSignerID;
		
		CToPString( userID, pSignerID );
		PrintPString( statusText, statusText, pSignerID );
	}
	
	CopyPStringMax( statusText, tableItem.single.statusText,
				sizeof( tableItem.single.statusText ) - 1 );
	
	// We do not call AddLastChildRow() because it does not return the index
	// of the newly added row, which we need to set the row height.
	
	numChildren = CountAllDescendents( parentRow );
	if( numChildren == 0 )
	{
		InsertChildRows( 1, parentRow, &tableItem,
							sizeof( tableItem ), FALSE, Refresh_Yes );
		newRowIndex = parentRow + 1;
	}
	else
	{
		InsertSiblingRows( 1, parentRow + numChildren, &tableItem,
							sizeof( tableItem ), FALSE, Refresh_Yes );
		newRowIndex = parentRow + numChildren + 1;
	}

	STableCell	newCell( newRowIndex, 1 );
	
	ScrollCellIntoFrame( newCell );
	
	UpdatePort();
}

	void
CResultsTable::DrawSelf(void)
{
	Rect frame;

	CalcLocalFrameRect( frame );
	
	{
		StDeviceLoop	devLoop( frame );		
		SInt16			depth;

		while( devLoop.NextDepth( depth ) )
		{
			switch( depth )
			{
				case 1:
				case 4:	
					// Nothing to do. Table is B/W.
					break;
					
				default:
					RGBForeColor( &UGAColorRamp::GetColor( colorRamp_Gray1 ) );
					PaintRect( &frame );
					break;
			}

			LHierarchyTable::DrawSelf();
		}
	}
}

	void
CResultsTable::DrawCell(
	const STableCell	&inCell,
	const Rect			&inLocalRect)
{
	TableIndexT			wideOpenRow;
	UInt32				dataSize;
	ResultsTableItem	item;
	STableCell			wideOpenCell;
	RGBColor			saveBackColor;
	
	wideOpenRow = mCollapsableTree->GetWideOpenIndex( inCell.row );
	
	FocusDraw();
	
	GetBackColor( &saveBackColor );
	
	// Draw Copland-like background for cells
	RGBForeColor( &UGAColorRamp::GetWhiteColor() );
	MoveTo( inLocalRect.left, inLocalRect.bottom - 1 );
	LineTo( inLocalRect.right - 1, inLocalRect.bottom - 1 );
	RGBForeColor( &UGAColorRamp::GetBlackColor() );
	
	DrawDropFlag( inCell, wideOpenRow );
	
	wideOpenCell.SetCell( wideOpenRow, inCell.col );
	dataSize = sizeof( item );
	
	GetCellData( wideOpenCell, &item, dataSize );
	
	switch( item.itemType )
	{
		case kResultTypeSummaryItem:
			DrawSummaryCell( &item.summary, &inLocalRect );
			break;
			
		case kResultTypeSingleItem:
			DrawSingleItemCell( &item.single, &inLocalRect );
			break;
	}
	
	RGBBackColor( &saveBackColor );
}
	
	void
CResultsTable::DrawSummaryCell(
	const ResultsSummaryItem	*data,
	const Rect					*cellRect)
{
	Rect	textRect;
	Str255	theText;
	Str255	tempStr;
	short	strIndex;
	
	UTextTraits::SetPortTextTraits( kSummaryItemTextTraitsResID );
	
	if( data->numItems == 1 )
	{
		strIndex = kVerifiedOneItemStrIndex;
	}
	else
	{
		strIndex = kVerifiedNItemsStrIndex;
	}
	
	GetIndString( theText, kResultsTableStringListResID, strIndex );
	
	NumToString( data->numItems, tempStr );
	PrintPString( theText, theText, tempStr );
	
	IUDateString( data->dateAndTime, shortDate, tempStr );
	PrintPString( theText, theText, tempStr );

	IUTimeString( data->dateAndTime, FALSE, tempStr );
	PrintPString( theText, theText, tempStr );
	
	textRect 		= *cellRect;
	textRect.left 	+= kDropFlagMargin;
	textRect.bottom	= textRect.top + mSummaryItemFontInfo.ascent +
						mSummaryItemFontInfo.descent +
						mSummaryItemFontInfo.leading;
	
	AlignRect( cellRect, &textRect, kAlignAtVerticalCenter );
	
	MoveTo( textRect.left, textRect.bottom - mSummaryItemFontInfo.descent );
	DrawString( theText );
}

	void
CResultsTable::DrawValidity(
	PGPValidity		validity,
	const Rect		*columnRect)
{
	Rect	validityRect = *columnRect;

	if( mShowMarginalValidity )
	{
		ushort		barPercent;
		RGBColor	validityColor = { 0xCCCC, 0xCCCC, 0xFFFF };
		
		// Draw the validity
		
		RGBForeColor( &UGAColorRamp::GetBlackColor() );
		FrameRect( &validityRect );
		InsetRect( &validityRect, 1, 1 );

		RGBForeColor( &validityColor );
		PaintRect( &validityRect );
		
		switch( validity )
		{
			case kPGPValidity_Complete:
				barPercent = 100;
				break;
				
			case kPGPValidity_Marginal:
				barPercent = 50;
				break;

			default:
				barPercent = 0;
				break;
		}
				
		validityRect.right = validityRect.left +
					(( UGraphicsUtilities::RectWidth( validityRect ) * 
					barPercent ) / 100 );
					
		RGBForeColor( &UGAColorRamp::GetColor( colorRamp_Gray9 ) );
		PaintRect( &validityRect );
	}
	else
	{
		ResID		iconResID;
		RGBColor	tempColor;
		
		if( validity == kPGPValidity_Complete )
		{
			iconResID = kValidKeyIconResID;
		}
		else if( validity == kPGPValidity_Marginal )
		{
			if( mMarginalIsInvalid )
			{
				iconResID = kInvalidKeyIconResID;
			}
			else
			{
				iconResID = kValidKeyIconResID;
			}
		}
		else
		{
			iconResID = kInvalidKeyIconResID;
		}
		
		validityRect.bottom = validityRect.top + 12;
		validityRect.right 	= validityRect.left + 16;
		AlignRect( columnRect, &validityRect, kAlignAtAbsoluteCenter );
		
		tempColor = UGAColorRamp::GetWhiteColor();
		RGBBackColor( &tempColor );

		PlotIconID( &validityRect, kAlignNone,
				IsEnabled() ? kTransformNone : kTransformDisabled,
				iconResID );
	}
}

	void
CResultsTable::DrawSingleItemCell(
	const ResultsSingleItem		*data,
	const Rect					*cellRect)
{
	Rect		textRect;
	Str255		tempStr;
	Rect		iconRect;
	Rect		nameRect;
	Rect		statusRect;
	Rect		validityRect;
	Rect		signingDateRect;
	ushort		maxTextWidth;
	ushort		textBaseLine;
	
	UTextTraits::SetPortTextTraits( kSingleItemTextTraitsResID );

	// Align the text vertically within the cell.
	textRect 		= *cellRect;
	textRect.bottom	= textRect.top + mSingleItemFontInfo.ascent +
						mSingleItemFontInfo.descent +
						mSingleItemFontInfo.leading;
	
	AlignRect( cellRect, &textRect, kAlignAtVerticalCenter );
	textBaseLine = textRect.bottom - mSingleItemFontInfo.descent;
	
	CResultsWindow::CalcColumnBounds( &textRect, &iconRect, &nameRect,
			&statusRect, &validityRect, &signingDateRect );
	
	if( data->iconResID != 0 )
	{
		// This code assumes that mini icons are 12x16. 
		iconRect.bottom = iconRect.top + 12;

		AlignRect( cellRect, &iconRect, kAlignAtVerticalCenter );
		PlotIconID( &iconRect, atNone, ttNone, data->iconResID );
	}
	
	// Draw the item name
	CopyPString( data->itemName, tempStr );
	
	maxTextWidth = UGraphicsUtilities::RectWidth( nameRect );
	if( StringWidth( tempStr ) > maxTextWidth )
	{
		TruncString( maxTextWidth, tempStr, smTruncMiddle );
	}
	
	MoveTo( nameRect.left, textBaseLine );
	DrawString( tempStr );
	
	// Draw the signer ID or status
	
	CopyPString( data->statusText, tempStr );

	maxTextWidth = UGraphicsUtilities::RectWidth( statusRect );
	if( StringWidth( tempStr ) > maxTextWidth )
	{
		TruncString( maxTextWidth, tempStr, smTruncMiddle );
	}
	
	MoveTo( statusRect.left, textBaseLine );
	DrawString( tempStr );

	UTextTraits::SetPortTextTraits( kSingleItemTextTraitsResID );
	
	if( data->validSignature || data->badSignature )
	{
		validityRect.bottom = validityRect.top + kValidityBarHeight;
		AlignRect( cellRect, &validityRect, kAlignAtVerticalCenter );
		
		DrawValidity( data->validity, &validityRect );
	}

	RGBForeColor( &UGAColorRamp::GetBlackColor() );

	// Draw the signing date
	if( data->badSignature )
	{
		GetIndString( tempStr, kResultsTableStringListResID,
				kBadSignatureStrIndex );
	}
	else
	{
		IUDateString( PGPTimeToMacTime( data->signingDate ), shortDate,
					tempStr );
	}
	
	maxTextWidth = UGraphicsUtilities::RectWidth( signingDateRect );
	if( StringWidth( tempStr ) > maxTextWidth )
	{
		TruncString( maxTextWidth, tempStr, smTruncEnd );
	}

	MoveTo( signingDateRect.left, textBaseLine );
	DrawString( tempStr );
}

	void
CResultsTable::AdaptToSuperFrameSize(
	Int32	inSurrWidthDelta,
	Int32	inSurrHeightDelta,
	Boolean	inRefresh)
{
	// Resize the single column to fit to the width of the window.

	LHierarchyTable::AdaptToSuperFrameSize( inSurrWidthDelta,
				inSurrHeightDelta, inRefresh );
				
	SetColWidth( GetColWidth( 1 ) + inSurrWidthDelta, 1, 1 );
}

#if 0	// KEEP!

	TableIndexT
CResultsTable::InsertSiblingRows(
	Uint32		inHowMany,
	TableIndexT	inAfterRow,				// WideOpen Index
	const void	*inDataPtr,
	Uint32		inDataSize,
	Boolean		inCollapsable,
	Boolean		inRefresh)
{
	// This is an almost identical copy of LHierarchyTable::InsertSiblingRows,
	// with a bug fixed. Basically, if you're inserting a sibling row which
	// is separated from it sibling by the expanded sibling's children, then
	// the data gets inserted at the correct offset, however the geometry and
	// selector arrays are updated as if these intervening exposed children
	// were not present.
	
	#if __PowerPlant__	!= 0x01608000
	#error !! check this version against LHierarchyTable::InsertSiblingRows()
	to see if bug has been fixed.
	#endif
	
	TableIndexT		lastWideOpen = mCollapsableTree->CountNodes();
	if (inAfterRow > lastWideOpen) {	// If inAfterIndex is too big,
		inAfterRow = lastWideOpen;		//   insert after last row
	}

		// Inserts Rows into CollapsableTree and store the data
		// for the new Rows

	TableIndexT	insertAtRow = mCollapsableTree->InsertSiblingNodes(inHowMany,
								inAfterRow, inCollapsable);
	
	if (mTableStorage != nil) {
		mTableStorage->InsertRows(inHowMany, insertAtRow - 1,
							inDataPtr, inDataSize);
	}
	
		// If inserted rows are exposed, we need to adjust the
		// visual display, which involves the Table Geometry,
		// Selection, and Image size.
	
	TableIndexT	exSibling = mCollapsableTree->GetExposedIndex(inAfterRow);
	if ((exSibling > 0) || (inAfterRow == 0))
	{
		TableIndexT		exRowIndex;
		
		mRows += inHowMany;
		
		exRowIndex = mCollapsableTree->GetExposedIndex(insertAtRow);
		mTableGeometry->InsertRows(inHowMany, exRowIndex);
		
		if (mTableSelector != nil) {
			mTableSelector->InsertRows(inHowMany, exRowIndex);
		}
		
		AdjustImageSize(false);
		
		if (inRefresh) {
			RefreshRowsBelow(inAfterRow);
		}
	}
	
	return insertAtRow;
}

#endif	// KEEP!