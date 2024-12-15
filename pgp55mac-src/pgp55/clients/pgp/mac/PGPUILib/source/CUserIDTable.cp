/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CUserIDTable.cp,v 1.38.4.3 1997/11/20 20:41:35 heller Exp $
____________________________________________________________________________*/

#include <Icons.h>
#include <LowMem.h>

#include <LComparator.h>
#include <LDragTask.h>
#include <LTableArrayStorage.h>
#include <LTableMonoGeometry.h>
#include <LTableMultiSelector.h>
#include <PP_KeyCodes.h>
#include <UDrawingState.h>
#include <UGAColorRamp.h>
#include <UGraphicsUtilities.h>
#include <UDrawingUtils.h>

#include "MacBasics.h"
#include "MacDebugLeaks.h"
#include "MacFiles.h"
#include "MacQuickdraw.h"
#include "MacStrings.h"
#include "pgpKeys.h"
#include "pgpMem.h"

#include "CUserIDTable.h"
#include "pgpUserInterface.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include <string.h>
#include <ctype.h>
#include "PGPUILibDialogs.h"
#include "PGPUILibUtils.h"
#include "PGPSharedEncryptDecrypt.h"
#include "pgpGroupsUtil.h"

const ResID kRSAUserIDIconResID 	= 4747;
const ResID kDSSDHUserIDIconResID 	= 4748;
const ResID kUnknownUserIDIconResID = 4749;

const ResID kValidKeyIconResID 		= 4750;
const ResID kInvalidKeyIconResID 	= 4751;
const ResID kAxiomaticKeyIconResID 	= 4752;
const ResID kLockedKeyIconResID 	= 4753;
const ResID kGroupIconResID			= 4756;

const ResID kBarberPatternResID 	= 4747;

const short	kBarHeight		= 10;
const short	kMaxValidity	= 3;

// Column and gutter widths, from right to left
const short	kKeySizeRightMargin		= 5;
const short	kKeySizeColumnWidth		= 70;
const short	kKeySizeLeftMargin		= 10;

const short	kValidityBarColumnWidth	= 40;
const short	kValidityBarLeftMargin	= 10;

const short kUserIDLeftMargin		= 3;

const short	kIconColumnWidth		= 16;
const short	kIconLeftMargin			= 3;
const short	kIconRightMargin		= 3;

PGPUInt32	CUserIDTable::mNextSerialNumber 	= 1;
Handle		PGPRecipientTableItem::sNameData	= NULL;

class CUserIDComparator : public LComparator
{
	virtual			~CUserIDComparator(void);
	virtual Int32	Compare(const void *inItemOne, const void *inItemTwo,
								Uint32 inSizeOne, Uint32 inSizeTwo) const;
	virtual	Int32	CompareToKey(const void *inItem, Uint32 inSize,
								const void *inKey) const;
};

CUserIDComparator::~CUserIDComparator(void)
{
}

	Int32
CUserIDComparator::Compare(
	const void*		inItemOne,
	const void*		inItemTwo,
	Uint32			inSizeOne,
	Uint32			inSizeTwo) const
{
	Int32						result;
	const PGPRecipientTableItem	*tableItemOne;
	const PGPRecipientTableItem	*tableItemTwo;
	
	tableItemOne = (PGPRecipientTableItem *) inItemOne;
	tableItemTwo = (PGPRecipientTableItem *) inItemTwo;

	pgpAssertAddrValid( tableItemOne, PGPRecipientTableItem );
	pgpAssertAddrValid( tableItemTwo, PGPRecipientTableItem );
	
#if PGP_DEBUG
	pgpAssert( inSizeOne == sizeof(PGPRecipientTableItem) );
	pgpAssert( inSizeTwo == sizeof(PGPRecipientTableItem) );
#else
	#pragma unused( inSizeOne, inSizeTwo )
#endif
	
	result = tableItemOne->sortGrouping - tableItemTwo->sortGrouping;
	if( result == 0 )
	{
		result = PGPCompareUserIDStrings( tableItemOne->GetNamePtr(),
					tableItemTwo->GetNamePtr() );
					
		if( result == 0 )
		{
			result = tableItemOne->serialNumber -
							tableItemTwo->serialNumber;
		}
	}
	
	return( result );
}

	Int32
CUserIDComparator::CompareToKey(
	const void*	inItem,
	Uint32		inSize,
	const void*	inKey) const
{	
	Int32						result;
	const PGPRecipientTableItem	*tableItem;
	const char *				searchUserID;
	
	(void) inSize;
	
	tableItem 		= (PGPRecipientTableItem *) inItem;
	searchUserID	= (const char *) inKey;

	pgpAssertAddrValid( tableItem, PGPRecipientTableItem );
	
	result = PGPCompareUserIDStrings( tableItem->GetNamePtr(),
				searchUserID );
	
	return( result );
}



CUserIDTable::CUserIDTable(LStream	*inStream)
		: LTableView(inStream),
		LDragAndDrop( UQDGlobals::GetCurrentPort(), this )
{
	
	mTableGeometry	= new LTableMonoGeometry(this, mFrameSize.width, 16);
	mTableSelector	= new LTableMultiSelector(this);
	
	// Note: We own this array and need to dispose of it ourselves.
	mStorageArray 	= new LArray( sizeof(PGPRecipientTableItem),
								new CUserIDComparator, TRUE );
	mTableStorage	= new LTableArrayStorage(this, mStorageArray);
	
	mDeleteSendsMessage		= FALSE;
	mBarberPixPat			= nil;
	mShowMarginalValidity	= FALSE;
	mMarginalIsInvalid		= FALSE;
	mNextSerialNumber		= 1;

	mShowIconColumn			= TRUE;
	mShowValidityColumn		= TRUE;
	mShowKeySizeColumn		= TRUE;
}

CUserIDTable::~CUserIDTable()
{
	delete mStorageArray;

	if( IsntNull( mBarberPixPat ) )
		DisposePixPat( mBarberPixPat );
}

CUserIDTable *CUserIDTable::CreateFromStream(LStream *inStream)
{
	CUserIDTable	*table;
	
	// LDropArea leaks like a sieve. Need to suspend leaks when we create one.
	
	MacLeaks_Suspend();
	table = new CUserIDTable(inStream);
	MacLeaks_Resume();
	
	return( table );
}

	void
CUserIDTable::FinishCreateSelf()
{
	mLastKeypress	= 0;
	mLastClickTime	= 0;
	mLastClickCell	= 0;

	mBarberPixPat = GetPixPat( kBarberPatternResID );
	pgpAssertAddrValid( mBarberPixPat, VoidAlign);
	
	SetCustomHilite( TRUE );
}

	void
CUserIDTable::DisallowSelections(void)
{
	delete mTableSelector;
	mTableSelector = NULL;
}

	void
CUserIDTable::SetDisplayColumns(
	Boolean	showIconColumn,
	Boolean	showValidityColumn,
	Boolean	showKeySizeColumn)
{
	mShowIconColumn 	= showIconColumn;
	mShowValidityColumn	= showValidityColumn;
	mShowKeySizeColumn	= showKeySizeColumn;
}

	void
CUserIDTable::PrepareToDraw(void)
{
	RGBColor	tempColor;
	
	tempColor = UGAColorRamp::GetBlackColor();
	RGBForeColor( &tempColor );

	tempColor = UGAColorRamp::GetColor( colorRamp_Gray1 );
	RGBBackColor( &tempColor );
}

	void
CUserIDTable::DrawValidity(
	const PGPRecipientTableItem	*tableItem,
	const Rect 					*columnRect,
	UInt16 						bitDepth)
{
	Rect		drawRect = *columnRect;
	RGBColor	tempColor;
	RGBColor	saveBackColor;
	
	GetBackColor( &saveBackColor );
	
	tempColor = UGAColorRamp::GetBlackColor();
	RGBForeColor( &tempColor );

	if( mShowMarginalValidity )
	{
		RGBColor	trustColor = { 52223, 52223, 65535 };
		
		drawRect.bottom = drawRect.top + kBarHeight;
		AlignRect( columnRect, &drawRect, kAlignAtVerticalCenter );

		FrameRect( &drawRect );
		InsetRect( &drawRect, 1, 1);
		
		if( tableItem->user.isAxiomaticKey && IsntNull( mBarberPixPat ))
		{
			PenPixPat( mBarberPixPat );
			PaintRect( &drawRect );
			PenNormal();
		}
		else
		{
			short 	width;
			short	barPercent;
			
			switch( tableItem->validity )
			{
				default:
				case kPGPValidity_Unknown:
				case kPGPValidity_Invalid:
					barPercent = 0;
					break;
					
				case kPGPValidity_Marginal:
					barPercent = 50;
					break;
					
				case kPGPValidity_Complete:
					barPercent = 100;
					break;
			}
			
			if( bitDepth >= 8 )
			{
				RGBForeColor( &trustColor );
				PaintRect( &drawRect );
			}
			
			width 			= drawRect.right - drawRect.left;
			drawRect.right 	-= (width * (100 - barPercent)) / 100;
			
			tempColor = UGAColorRamp::GetColor( colorRamp_Gray9 );
			RGBForeColor( &tempColor );
			PaintRect( &drawRect );
		}

	}
	else
	{
		ResID	iconResID;
		
		if( tableItem->user.isAxiomaticKey )
		{
			iconResID = kAxiomaticKeyIconResID;
		}
		else if( tableItem->validity == kPGPValidity_Complete )
		{
			iconResID = kValidKeyIconResID;
		}
		else if( tableItem->validity == kPGPValidity_Marginal )
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
		
		drawRect.bottom = drawRect.top + 12;
		drawRect.right 	= drawRect.left + 16;
		AlignRect( columnRect, &drawRect, kAlignAtAbsoluteCenter );
		
		tempColor = UGAColorRamp::GetWhiteColor();
		RGBBackColor( &tempColor );

		PlotIconID( &drawRect, kAlignNone,
				IsEnabled() ? kTransformNone : kTransformDisabled,
				iconResID );
	}
	
	tempColor = UGAColorRamp::GetBlackColor();
	RGBForeColor( &tempColor );
	
	RGBBackColor( &saveBackColor );
}

	void
CUserIDTable::GetCellColumnRects(
	const Rect	*cellRect,
	Rect		*iconColumnRect,
	Rect		*userIDColumnRect,
	Rect		*validityColumnRect,
	Rect		*keySizeColumnRect)
{
	// Calculate all of the column rectangles
	
	*keySizeColumnRect = *cellRect;
	
	if( mShowKeySizeColumn )
	{
		keySizeColumnRect->right = cellRect->right - kKeySizeRightMargin;
		keySizeColumnRect->left	= keySizeColumnRect->right -
								kKeySizeColumnWidth;
	}
	else
	{
		keySizeColumnRect->left = keySizeColumnRect->right;
	}
	
	*validityColumnRect = *cellRect;
	
	if( mShowValidityColumn )
	{
		validityColumnRect->right = keySizeColumnRect->left -
										kKeySizeLeftMargin;
		validityColumnRect->left = validityColumnRect->right -
										kValidityBarColumnWidth;
	}
	else
	{
		validityColumnRect->right 	= keySizeColumnRect->left;
		validityColumnRect->left 	= validityColumnRect->right;
	}
	
	*iconColumnRect = *cellRect;
	
	if( mShowIconColumn )
	{
		iconColumnRect->left 	= cellRect->left + kIconLeftMargin;
		iconColumnRect->right	= iconColumnRect->left + kIconColumnWidth;
	}
	else
	{
		iconColumnRect->right = iconColumnRect->left;
	}
	
	*userIDColumnRect			= *cellRect;
	userIDColumnRect->left 		= iconColumnRect->right + kIconRightMargin;
	userIDColumnRect->right 	=
			validityColumnRect->left - kValidityBarLeftMargin;
			
	if( ! mShowIconColumn )
	{
		userIDColumnRect->left = cellRect->left;
	}
	
	if( ( ! mShowValidityColumn ) && ( ! mShowKeySizeColumn ) )
	{
		userIDColumnRect->right = cellRect->right;
	}
}

	void
CUserIDTable::DrawCellContents(
	const STableCell	&inCell,
	const Rect			&inLocalRect,
	UInt32				inBitDepth)
{
	Rect				iconRect;
	Rect				keySizeRect;
	Rect				validityColumnRect;
	Rect				userIDRect;
	Rect				userIDColumnRect;
	PGPRecipientTableItem	tableItem;
	Str255				str;
	IconTransformType	iconTransform;
	short				textMode;
	short				textHeight;
	PGPUInt16			maxUserIDWidth;
	RGBColor			tempColor;
	
	GetCellData( inCell, &tableItem );
	PrepareToDraw();
	
	textHeight = 12;
	
	GetCellColumnRects( &inLocalRect, &iconRect,
		&userIDColumnRect, &validityColumnRect, &keySizeRect );
		
	// Adjust column rectangles to fit enclosed content.
	
	userIDRect			= userIDColumnRect;
	userIDRect.left		= userIDRect.left + kUserIDLeftMargin;
	userIDRect.bottom 	= userIDRect.top + textHeight;
	AlignRect( &inLocalRect, &userIDRect, kAlignAtVerticalCenter );

	if( IsEnabled() )
	{
		textMode 		= 0;
		iconTransform	= ttNone;
	}
	else
	{
		textMode		= grayishTextOr;
		iconTransform 	= ttDisabled;
	}
	
	if( tableItem.itemKind == kTableItemKindMissingKey )
	{
		textMode		= grayishTextOr;
		iconTransform	= ttDisabled;
	}
	
	// Draw Icon

	if( mShowIconColumn )
	{
		short	iconID;
		
		if( tableItem.itemKind == kTableItemKindKey )
		{
			if( tableItem.user.algorithm == kPGPPublicKeyAlgorithm_RSA )
			{
				iconID = kRSAUserIDIconResID;
			}
			else
			{
				iconID = kDSSDHUserIDIconResID;
			}
		}
		else if( tableItem.itemKind == kTableItemKindMissingKey )
		{
			iconID = kUnknownUserIDIconResID;
		}
		else if ( tableItem.itemKind == kTableItemKindGroup )
		{
			iconID = kGroupIconResID;
		}
		else
		{
			pgpDebugMsg( "DrawCellContents: Unknown item status" );
		}

		iconRect.bottom = iconRect.top + 12;
		AlignRect( &inLocalRect, &iconRect, kAlignAtVerticalCenter );
	
		PlotIconID( &iconRect, atNone, iconTransform, iconID );
	}
	
	// Draw User ID
	
	tempColor = UGAColorRamp::GetColor( colorRamp_Gray2 );
	RGBBackColor( &tempColor );
	
	EraseRect( &userIDColumnRect );
	
	TextSize(9);
	TextFont(1);
	TextMode( textMode );

	if ( TableItemKindIsKey( tableItem.itemKind ) )
	{
		if( tableItem.user.isDefaultKey )
		{
			TextFace( bold );
		}
		else
		{
			TextFace( 0 );
		}
		
		if( tableItem.locked )
		{
			Rect	lockedIconRect;
			
			lockedIconRect 			= userIDRect;
			lockedIconRect.left 	= lockedIconRect.right - 16;
			lockedIconRect.bottom 	= lockedIconRect.top + 12;
		
			AlignRect( &userIDRect, &lockedIconRect, kAlignAtCenterRight );
			PlotIconID( &lockedIconRect, atNone, iconTransform,
						kLockedKeyIconResID );
						
			userIDRect.right -= 16;
		}
	}
	else if ( tableItem.itemKind == kTableItemKindGroup )
	{
		TextFace( 0 );
	}
	
	MoveTo( userIDRect.left, userIDRect.bottom - 2 );
	CToPString( tableItem.GetNamePtr(), str );
	
	maxUserIDWidth = userIDRect.right - userIDRect.left;
	if( StringWidth( str ) > maxUserIDWidth )
	{
		TextFace( condense );
		
		if( StringWidth( str ) > maxUserIDWidth )
		{
			// will not fit condensed. Resort to the default face and
			// truncate in the middle
			TextFace( 0 );
			TruncString( maxUserIDWidth, str, smTruncMiddle );
		}
	}
	
	DrawString( str );

	TextFace( 0 );
	
	tempColor = UGAColorRamp::GetColor( colorRamp_Gray1 );
	RGBBackColor( &tempColor );
	
	if( mShowValidityColumn )
	{
		DrawValidity( &tableItem, &validityColumnRect, inBitDepth );
	}
	
	if( mShowKeySizeColumn )
	{
		keySizeRect.bottom = keySizeRect.top + textHeight;
		AlignRect( &inLocalRect, &keySizeRect, kAlignAtVerticalCenter );

		if( tableItem.itemKind == kTableItemKindKey ||
			tableItem.itemKind == kTableItemKindGroup )
		{
			PGPBoolean		drawIt	= FALSE;
			Str63			theString;
			
			// Draw key size
			if ( tableItem.itemKind == kTableItemKindKey )
			{
				if( tableItem.user.keyBits != 0 )
				{
					drawIt = TRUE;
					
					if( tableItem.user.subKeyBits != 0 )
					{
						Str32	tempStr;
					
						GetIndString( theString,
								kPGPLibDialogsStringListResID,
								kDSSDHKeySizeFormatStrIndex );
								
						NumToString( tableItem.user.subKeyBits, tempStr );
						PrintPString( theString, theString, tempStr);

						NumToString( tableItem.user.keyBits, tempStr );
						PrintPString( theString, theString, tempStr);
					}
					else
					{
						NumToString( tableItem.user.keyBits, theString );
					}
				}
			}
			else
			{
				Str32	tempStr;
				
				pgpAssert( tableItem.itemKind == kTableItemKindGroup );
				
				drawIt = TRUE;
				
				if( tableItem.group.numMissingKeys != 0 )
				{
					GetIndString( theString, kPGPLibDialogsStringListResID,
								kNumMissingGroupKeysFormatStrIndex );
								
					NumToString( tableItem.group.numKeys -
								tableItem.group.numMissingKeys, tempStr );
					PrintPString( theString, theString, tempStr );
				}
				else
				{
					GetIndString( theString, kPGPLibDialogsStringListResID,
								kNumGroupKeysFormatStrIndex );
				}
				
				NumToString( tableItem.group.numKeys, tempStr );
				PrintPString( theString, theString, tempStr );
			}
			
			if ( drawIt )
			{
				MoveTo( keySizeRect.left, keySizeRect.bottom - 2 );
				DrawString( theString );
			}
		}
	}
	
	TextMode( srcOr );
	
	ForeColor( whiteColor );
	MoveTo( inLocalRect.left, inLocalRect.bottom - 1 );
	LineTo( inLocalRect.right, inLocalRect.bottom - 1 );
	ForeColor( blackColor );
}

	void
CUserIDTable::DrawCell(
	const STableCell	&inCell,
	const Rect			&inLocalRect)
{
	StDeviceLoop 	devLoop( inLocalRect );		
	SInt16			depth;

	while( devLoop.NextDepth( depth ) )
	{
		DrawCellContents( inCell, inLocalRect, depth );
	}
}

	void
CUserIDTable::ClassifyItems(
	PGPUInt32	*inUsers,
	PGPUInt32	*inGroups,
	PGPUInt32	*inIncompleteGroups,
	PGPUInt32	*inMissingUsers,
	PGPUInt32	*inMissingGroups,
	PGPValidity	*inMinValidity,
	PGPBoolean	*inHaveNonDefaultUsers)
{
	PGPUInt32	numUsers;
	PGPUInt32	numGroups;
	PGPUInt32	numIncompleteGroups;
	PGPUInt32	numMissingUsers;
	PGPUInt32	numMissingGroups;
	PGPValidity	minValidity;
	PGPBoolean	haveNonDefaultUsers;
	STableCell	curCell(0,0);
	
	AssertAddrNullOrValid( inUsers, PGPUInt32,
		"ClassifyItems" );
	AssertAddrNullOrValid( inGroups, PGPUInt32,
		"ClassifyItems" );
	AssertAddrNullOrValid( inIncompleteGroups, PGPUInt32,
		"ClassifyItems" );
	AssertAddrNullOrValid( inMissingUsers, PGPUInt32,
		"ClassifyItems" );
	AssertAddrNullOrValid( inMinValidity, PGPValidity,
		"ClassifyItems" );
	AssertAddrNullOrValid( inHaveNonDefaultUsers, PGPBoolean,
		"ClassifyItems" );
	
	numUsers			= 0;
	numGroups			= 0;
	numIncompleteGroups	= 0;
	numMissingUsers		= 0;
	numMissingGroups	= 0;
	minValidity			= kPGPValidity_Complete;
	haveNonDefaultUsers	= FALSE;
	
	while( GetNextCell( curCell ) )
	{
		PGPRecipientTableItem	tableItem;
		
		GetCellData( curCell, &tableItem );
		
		if( tableItem.itemKind == kTableItemKindKey )
		{
			numUsers += 1;
			
			if( ! tableItem.user.isAxiomaticKey )
			{
				if( tableItem.validity < minValidity )
				{
					minValidity = tableItem.validity;
				}
			}
			
			if( ! tableItem.user.isDefaultKey )
				haveNonDefaultUsers = TRUE;
		}
		else if( tableItem.itemKind == kTableItemKindGroup )
		{
			numGroups += 1;
			
			if( tableItem.validity < minValidity )
			{
				minValidity = tableItem.validity;
			}

			if( tableItem.group.numMissingKeys > 0 )
			{
				numIncompleteGroups += 1;
				
				if( tableItem.group.numMissingKeys ==
						tableItem.group.numKeys )
				{
					++numMissingGroups;
				}
			}
		}
		else if( tableItem.itemKind == kTableItemKindMissingKey )
		{
			numMissingUsers += 1;
		}
		else
		{
			pgpDebugMsg( "CUserIDTable::ClassifyItems: Unknown item kind" );
		}
	}
	
	if( numUsers + numGroups + numMissingUsers == 0 )
		minValidity	= kPGPValidity_Unknown;
		
	if( IsntNull( inUsers ) )
		*inUsers = numUsers;

	if( IsntNull( inGroups ) )
		*inGroups = numGroups;

	if( IsntNull( inIncompleteGroups ) )
		*inIncompleteGroups = numIncompleteGroups;

	if( IsntNull( inMissingUsers ) )
		*inMissingUsers = numMissingUsers;

	if( IsntNull( inMissingGroups ) )
		*inMissingGroups = numMissingGroups;

	if( IsntNull( inMinValidity ) )
		*inMinValidity = minValidity;
		
	if( IsntNull( inHaveNonDefaultUsers ) )
		*inHaveNonDefaultUsers = haveNonDefaultUsers;
}

	void
CUserIDTable::DragSelectedCells(const EventRecord& inMacEvent)
{
	RgnHandle	dragRgn;
	RgnHandle	tempRgn;
	Point		localPt;
	Point		globalPt;
	
	// This routine does not actually add the real cell data to the
	// drag. Instead, a token value (the pane ID) is added to the drag which
	// is checked for by the reveiving pane. If the token is presnet and the
	// destination pane is not the source pane, a message is broadcast to the
	// PGPRecipientGrafPort to move the selected cells to the receiving pane.
	
	FocusDraw();
	
	dragRgn = NewRgn();
	tempRgn	= NewRgn();
	
	GetHiliteRgn( dragRgn );
	CopyRgn( dragRgn, tempRgn );
	InsetRgn( tempRgn, 1, 1 );
	DiffRgn( dragRgn, tempRgn, dragRgn );
	
	// Need to convert the reqion to global coordinates.
	localPt.v = (**dragRgn).rgnBBox.top;
	localPt.h = (**dragRgn).rgnBBox.left;
	
	globalPt = localPt;
	LocalToGlobal( &globalPt );
	
	OffsetRgn( dragRgn, globalPt.h - localPt.h, globalPt.v - localPt.v );
	
	LDragTask	theDragTask( inMacEvent, dragRgn, 1, 'User', &mPaneID,
						sizeof( mPaneID ), flavorSenderOnly );

	// Invalidate LView's focus cache. The port may have changed during the
	// drag.
	LView::OutOfFocus( nil );

	DisposeRgn( dragRgn );
	DisposeRgn( tempRgn );
}

	void
CUserIDTable::ClickCell(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
{
	if((inMouseDown.macEvent.when - mLastClickTime > LMGetDoubleTime()) ||
		(mLastClickCell != inCell))
	{
		mLastClickTime = inMouseDown.macEvent.when;
		mLastClickCell = inCell;
		
		if( DragAndDropIsPresent() &&
			WaitMouseMoved( inMouseDown.macEvent.where ) &&
			CellIsSelected( inCell ) )
		{
			DragSelectedCells( inMouseDown.macEvent );
		}
	}
	else
	{
		mLastClickTime = 0;
		BroadcastMessage( 'Send', this);
	}
}

	Boolean
CUserIDTable::ItemIsAcceptable(
	DragReference	inDragRef,
	ItemReference	inItemRef )
{
	FlavorFlags	theFlags;
	
	return IsEnabled() && IsntErr( GetFlavorFlags( inDragRef,
		inItemRef, 'User', &theFlags ) );
}

	void
CUserIDTable::UnhiliteDropArea(
	DragReference	inDragRef)
{
	PrepareToDraw();
	LDragAndDrop::UnhiliteDropArea( inDragRef );
}

	void
CUserIDTable::HiliteDropArea(
	DragReference	inDragRef)
{
	PrepareToDraw();
	LDragAndDrop::HiliteDropArea( inDragRef );
}

	void
CUserIDTable::ReceiveDragItem(
	DragReference	inDragRef,
	DragAttributes	inDragAttrs,
	ItemReference	inItemRef,
	Rect			&inItemBounds )
{
#pragma unused( inDragAttrs, inItemBounds )

	PaneIDT		sourcePaneID;
	Size		theDataSize = sizeof( sourcePaneID );
	OSErr		err;
	
	err = GetFlavorData( inDragRef, inItemRef,
		'User', &sourcePaneID, &theDataSize, 0 );
	if( IsntErr( err ) && sourcePaneID != mPaneID )
	{
		// Unhilite manually because the result of BroadcastMessage may
		// display a dialog which will mess up the hilighting.
		
		FocusDropArea();
		mIsHilited = false;
		UnhiliteDropArea( inDragRef );
		
		BroadcastMessage( 'Recv', this );
	}
}

	void
CUserIDTable::HiliteCellActively(
	const STableCell	&inCell,
	Boolean				inHilite)
{
	Rect	cellFrame;
	
	#pragma unused( inHilite )
	
	if( GetLocalCellRect( inCell, cellFrame ) &&
		FocusExposed() )
	{
		StDeviceLoop	devLoop( cellFrame );		
		SInt16			depth;

		while( devLoop.NextDepth( depth ) )
		{
			StColorPenState saveColorPen;   // Preserve color & pen state
			Rect			iconColumnRect;
			Rect			userIDColumnRect;
			Rect			validityColumnRect;
			Rect			keySizeColumnRect;

			StColorPenState::Normalize();
			
			if( depth >= 8 )
			{
				RGBColor	tempColor;
				
				tempColor = UGAColorRamp::GetBlackColor();
				RGBForeColor( &tempColor );

				tempColor = UGAColorRamp::GetColor( colorRamp_Gray2 );
				RGBBackColor( &tempColor );
			}
			else
			{
				ForeColor( blackColor );
				BackColor( whiteColor );
			}
			
			GetCellColumnRects( &cellFrame, &iconColumnRect,
				&userIDColumnRect, &validityColumnRect,
				&keySizeColumnRect );
		
	        UDrawingUtils::SetHiliteModeOn();
			InvertRect( &userIDColumnRect );
		}
	}
}

	void
CUserIDTable::HiliteCellInactively(
	const STableCell	&inCell,
	Boolean			 	inHilite)
{
	Rect	cellFrame;
	
	#pragma unused( inHilite )

	if( GetLocalCellRect( inCell, cellFrame ) &&
		FocusExposed() )
	{
		StDeviceLoop	devLoop( cellFrame );		
		SInt16			depth;

		while( devLoop.NextDepth( depth ) )
		{
	        StColorPenState saveColorPen;   // Preserve color & pen state
			Rect			iconColumnRect;
			Rect			userIDColumnRect;
			Rect			validityColumnRect;
			Rect			keySizeColumnRect;
	        
	        StColorPenState::Normalize();
	        
	        if( depth >= 8 )
	        {
	        	PrepareToDraw();
			}
			else
			{
				ForeColor( blackColor );
				BackColor( whiteColor );
			}
			
			GetCellColumnRects( &cellFrame, &iconColumnRect,
				&userIDColumnRect, &validityColumnRect,
				&keySizeColumnRect );
		
			PenMode(srcXor);
			UDrawingUtils::SetHiliteModeOn();
			FrameRect( &userIDColumnRect );
		}
	}
}

	Boolean
CUserIDTable::HandleKeyPress(const EventRecord&	inKeyEvent)
{
	Boolean		keyHandled	= TRUE;
	Int16		theKey		= inKeyEvent.message & charCodeMask,
				charInx;
	
	if( inKeyEvent.modifiers & cmdKey )
	{
		keyHandled = LCommander::HandleKeyPress( inKeyEvent );
	}
	else if( IsEnabled() )
	{
		switch(theKey)
		{
			default:
			{
				if(inKeyEvent.when - kMaxKeypressInterval <= mLastKeypress)
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
				break;
			}
			
			case char_LeftArrow:
			case char_RightArrow:
				break;
				
			case char_UpArrow:
			{
				STableCell	firstSelectedCell(0, 0);
				
				if( GetNextSelectedCell( firstSelectedCell ) )
				{
					UnselectAllCells();

					if( firstSelectedCell.row != 1 )
					{
						firstSelectedCell.row -= 1;
					}
				}
				else
				{
					// There are no cells selected. Select the first cell
					firstSelectedCell.SetCell( 1, 1 );
				}
				
				ScrollCellIntoFrame( firstSelectedCell );
				SelectCell( firstSelectedCell );
				break;
			}
			
			case char_DownArrow:
			{
				STableCell	lastSelectedCell(0, 0);
				STableCell	curSelectedCell(0, 0);
				TableIndexT	numRows;
				TableIndexT	numColumns;
				
				while( GetNextSelectedCell( curSelectedCell ) )
				{
					lastSelectedCell = curSelectedCell;
				}
				
				UnselectAllCells();
				GetTableSize( numRows, numColumns );
				
				if( lastSelectedCell.row == 0 )
				{
					// No row selected. Select the last data row.
					
					lastSelectedCell.SetCell( numRows, 1 );
				}
				else if( lastSelectedCell.row == numRows )
				{
					// Last row is already selected. Select again
				}
				else
				{
					++lastSelectedCell.row;
				}
				
				ScrollCellIntoFrame( lastSelectedCell );
				SelectCell( lastSelectedCell );
				break;
			}
			
			case char_Home:
				// Using min_Int32 here causes overflow problems in
				// ScrollPinnedImageBy(). Use a "large" negative arbritray
				// number instead.
				ScrollPinnedImageBy( 0, -100000, TRUE );
				break;
				
			case char_End:
				// Using max_Int32 here causes overflow problems in
				// ScrollPinnedImageBy(). Use a "large" arbritray number
				// instead.
				ScrollPinnedImageBy( 0, 100000, TRUE );
				break;

			case char_PageUp:
			{
				Rect	frameRect;
				
				CalcLocalFrameRect( frameRect );
				ScrollPinnedImageBy( 0, -UGraphicsUtilities::RectHeight(
							frameRect ), TRUE );
				break;
			}

			case char_PageDown:
			{
				Rect	frameRect;
				
				CalcLocalFrameRect( frameRect );
				ScrollPinnedImageBy( 0, UGraphicsUtilities::RectHeight(
							frameRect ), TRUE );
				break;
			}
				
			case char_Return:
				if( GetFirstSelectedCell().row > 0 )
				{
					BroadcastMessage( 'Send', this);
				}
				break;
				
			case char_Backspace:
			{
				if( mDeleteSendsMessage && GetFirstSelectedCell().row > 0 )
				{
					BroadcastMessage( 'Send', this);
				}
				
				break;
			}
			
			case char_Tab:
			case char_Enter:
			case char_Escape:
				keyHandled = LCommander::HandleKeyPress(inKeyEvent);
				break;
		}
	}
	
	return keyHandled;
}

	void
CUserIDTable::SelectSearchString()
{
	STableCell				cell;
	TableIndexT				rows, cols;
	
	FocusDraw();
	PrepareToDraw();
	
	UnselectAllCells();

	GetTableSize(rows, cols);

	cell.col = 1;
	cell.row = mStorageArray->FetchInsertIndexOfKey( mSearchString );
	
	if( cell.row <= 0 || cell.row > rows )
		cell.row = rows;
		
	if( cell.row > 0 )
	{
		ScrollCellIntoFrame( cell );
		SelectCell( cell );
	}
}

	void
CUserIDTable::ClickSelf(
	const SMouseDownEvent &inMouseDown)
{
	PrepareToDraw();
	SwitchTarget( this );

	LTableView::ClickSelf(inMouseDown);
}

	void
CUserIDTable::DrawSelf()
{
	Rect 		frame;
	RGBColor	tempColor;
	
	tempColor = UGAColorRamp::GetColor( colorRamp_Gray1 );
	RGBForeColor( &tempColor );
	
	tempColor = UGAColorRamp::GetWhiteColor();
	RGBBackColor( &tempColor );
	
	CalcLocalFrameRect( frame );
	{
		StDeviceLoop	devLoop( frame );		
		SInt16			depth;

		while( devLoop.NextDepth( depth ) )
		{
			if( depth > 1 )
				PaintRect( &frame );
			else
				EraseRect( &frame );
		}
	}
	
	LTableView::DrawSelf();
}


	static PGPBoolean
sRemoveAngleBrackets(
	const char *	in,
	char *			out )
{
	PGPUInt32	length;
	PGPBoolean	haveBrackets	= FALSE;
	
	/* remove brackets if present */
	length			= strlen( in );
	haveBrackets	= in[ 0 ] == '<' && in[ length - 1 ] == '>';
	
	if ( haveBrackets )
	{
		CopyCString( in + 1, out );
		out[ length -1 - 1 ]	= '\0';
	}
	else
	{
		CopyCString( in, out );
	}
	return( haveBrackets );
}

	Boolean
CUserIDTable::FindUserID(
	const char	*userID,
	TableIndexT	startingRow,
	TableIndexT *foundRow)
{
	TableIndexT		numRows;
	TableIndexT		numCols;
	Boolean			foundIt	= FALSE;
	STableCell		cell;
	
	pgpAssertAddrValid( userID, char );
	pgpAssertAddrValid( foundRow, TableIndexT );
	
	*foundRow = 0;
	
	if( startingRow < 1 )
		startingRow = 1;
		
	GetTableSize( numRows, numCols );
	
	cell.col = 1;
	
	for( cell.row = startingRow; cell.row <= numRows; ++cell.row )
	{
		PGPRecipientTableItem	tableItem;
		
		GetCellData( cell, &tableItem );
		
		if ( tableItem.itemKind == kTableItemKindKey )
		{
			if ( FindSubStringNoCase( tableItem.GetNamePtr(), userID) )
			{
				foundIt		= TRUE;
				*foundRow 	= cell.row;
				break;
			}
		}
		else if ( tableItem.itemKind == kTableItemKindGroup )
		{
			char	tempStr[ 256 ];
			
			/* try without brackets.  Groups do not generally
			have angle brackets around them */
			(void) sRemoveAngleBrackets( userID, tempStr );
			
			if ( FindSubStringNoCase( tableItem.GetNamePtr(),
						tempStr) )
			{
				foundIt		= TRUE;
				*foundRow 	= cell.row;
				break;
			}
		}
	}
	
	return( foundIt );
}

	Boolean
CUserIDTable::HaveKey(
	PGPKeyRef	searchKey,
	UInt32 		*row)
{
	TableIndexT		numRows;
	TableIndexT		numCols;
	Boolean			foundIt	= FALSE;
	
	pgpAssertAddrValid( searchKey, VoidAlign );
	pgpAssertAddrValid( row, UInt32 );
	
	STableCell	cell;
	
	// search all rows to find the desired key
	cell.row	= 1;
	cell.col	= 1;
	GetTableSize( numRows, numCols );
	
	for( cell.row = 1; cell.row <= numRows; ++cell.row )
	{
		PGPRecipientTableItem	tableItem;
		
		GetCellData( cell, &tableItem );
		
		if( tableItem.itemKind == kTableItemKindKey &&
				tableItem.user.keyRef == searchKey )
		{
			// Check for a better match i.e. prefer primary user ID
			// over others
			
			if( foundIt )
			{
				if( tableItem.user.isPrimaryUserID )
				{
					*row = cell.row;
					break;
				}
			}
			else
			{
				foundIt = TRUE;
				*row 	= cell.row;
			}
		}
	}
	
	return( foundIt );
}

	void
CUserIDTable::GetCellData(
	const STableCell	&inCell,
	PGPRecipientTableItem 	*outTableItem) const
{
	UInt32	tableItemSize;
	
	pgpAssertAddrValid( outTableItem, PGPRecipientTableItem );

	tableItemSize = sizeof( *outTableItem );
	
	GetCellData( inCell, outTableItem, tableItemSize );
}

	void
CUserIDTable::GetCellData(
	const STableCell	&inCell,
	void				*outDataPtr,
	Uint32				&ioDataSize) const
{
	// This declaration is needed to pacify the Metrowerks compiler.
	LTableView::GetCellData( inCell, outDataPtr, ioDataSize );
}

	void
CUserIDTable::SetCellData(
	const STableCell			&inCell,
	const PGPRecipientTableItem 	*inTableItem)
{
	UInt32	tableItemSize;
	
	pgpAssertAddrValid( inTableItem, PGPRecipientTableItem );

	tableItemSize = sizeof( *inTableItem );
	
	SetCellData( inCell, inTableItem, tableItemSize );
}

	void
CUserIDTable::SetCellData(
	const STableCell	&inCell,
	const void			*inDataPtr,
	Uint32				inDataSize)
{
	// This declaration is needed to pacify the Metrowerks compiler.
	LTableView::SetCellData( inCell, inDataPtr, inDataSize );
}

	void
CUserIDTable::AddTableItem(
	PGPRecipientTableItem 	*inTableItem,
	Boolean					inRefresh)
{
	pgpAssertAddrValid( inTableItem, PGPRecipientTableItem );

	if( inTableItem->serialNumber == 0)
	{
		inTableItem->serialNumber = mNextSerialNumber++;
	}

	InsertRows( 1, 100000, inTableItem, sizeof( *inTableItem ), inRefresh );
}

	PGPError
CUserIDTable::AddMissingUserID(
	const char 	*userID,
	Boolean 	showAtTopOfList)
{
	PGPRecipientTableItem	tableItem;

	pgpClearMemory( &tableItem, sizeof( tableItem ) );
	
	tableItem.SetName( userID );
	
	tableItem.itemKind = kTableItemKindMissingKey;
	
	if( showAtTopOfList )
		tableItem.sortGrouping = -1;
		
	AddTableItem( &tableItem, TRUE );
	
	return( kPGPError_NoErr );
}

	void
CUserIDTable::SetPreferences(
	Boolean showMarginalValidity,
	Boolean	marginalIsInvalid)
{
	mShowMarginalValidity 	= showMarginalValidity;
	mMarginalIsInvalid		= marginalIsInvalid;
}

	PGPError
CUserIDTable::AddGroup(
	PGPKeySetRef	keySet,
	PGPGroupSetRef	set,
	PGPGroupID		id)
{
	PGPError				err = kPGPError_NoErr;
	PGPRecipientTableItem	tableItem;
	PGPGroupInfo			groupInfo;
	
	pgpClearMemory( &tableItem, sizeof( tableItem ) );
	tableItem.itemKind	= kTableItemKindGroup;
	tableItem.group.set	= set;
	tableItem.group.id	= id;
	
	err = PGPCountGroupItems( set, id,
			TRUE, &tableItem.group.numKeys, NULL );
	pgpAssertNoErr( err );
	
	if( tableItem.group.numKeys > 0 )
	{
		err	= PGPGetGroupInfo( set, id, &groupInfo );
		pgpAssertNoErr( err );
		tableItem.SetName( groupInfo.name );
		
		err	= PGPGetGroupLowestValidity( set, id, keySet,
				&tableItem.validity, &tableItem.group.numMissingKeys );
		pgpAssertNoErr( err );
		
		AddTableItem( &tableItem, FALSE );
	}
								
	return( err );
}

	void
CUserIDTable::UpdateGroupValidityAndKeyCounts(PGPKeySetRef allKeys)
{
	STableCell	cell(0, 0);
	PGPError	err = kPGPError_NoErr;
	
	while( GetNextCell( cell ) )
	{
		PGPRecipientTableItem	tableItem;
		
		GetCellData( cell, &tableItem );
		
		if( tableItem.itemKind == kTableItemKindGroup )
		{
			err	= PGPGetGroupLowestValidity( tableItem.group.set,
					tableItem.group.id, allKeys, &tableItem.validity,
					&tableItem.group.numMissingKeys );
			pgpAssertNoErr( err );

			SetCellData( cell, &tableItem );
		}
	}
}

	PGPError
CUserIDTable::CreateKeySet(
	PGPKeySetRef	allKeys,
	Boolean			markedOnly,
	PGPKeySetRef 	*outSet)
{	
	PGPError		err		= kPGPError_NoErr;
	PGPKeySetRef	keySet	= kInvalidPGPKeySetRef;
	
	PGPValidatePtr( outSet );
	*outSet	= NULL;
	
	err = PGPNewKeySet( PGPGetKeySetContext( allKeys ), &keySet );
	if( IsntPGPError( err ) )
	{
		TableIndexT		rows;
		TableIndexT		cols;
		
		GetTableSize( rows, cols );
		
		if( rows != 0 )
		{
			STableCell		cell(0, 1);
			
			for( cell.row = 1; cell.row <= rows; cell.row++ )
			{
				PGPRecipientTableItem	tableItem;
				PGPKeySetRef			additionalKeys;
				
				additionalKeys = kInvalidPGPKeySetRef;
				
				GetCellData( cell, &tableItem );
				
				if( markedOnly && ! tableItem.marked )
					continue;
					
				if( tableItem.itemKind == kTableItemKindKey )
				{
					err = PGPNewSingletonKeySet( tableItem.user.keyRef,
								&additionalKeys );
				}
				else if ( tableItem.itemKind == kTableItemKindGroup )
				{
					PGPUInt32	numMissing;
					
					err	= PGPNewKeySetFromGroup( tableItem.group.set,
							tableItem.group.id, allKeys,
							&additionalKeys, &numMissing );
				}
				
				if ( IsPGPError( err ) )
					break;
				
				if( PGPKeySetRefIsValid( additionalKeys ) )
				{
					err = PGPAddKeys( additionalKeys, keySet );
							
					(void) PGPFreeKeySet( additionalKeys );
				}
			}
		}

		if( IsntPGPError( err ) )
			err = PGPCommitKeyRingChanges( keySet );
			
		if( IsPGPError( err ) )
		{
			PGPFreeKeySet( keySet );
			keySet = kInvalidPGPKeySetRef;
		}
	}
	
	*outSet	= keySet;
	
	return( err );
}

	PGPError
CUserIDTable::AddKey(
	PGPKeyRef	theKey,
	Boolean		isDefaultKey,
	Boolean		addDefaultUserOnly)
{
	PGPError				err = kPGPError_NoErr;
	PGPUserIDRef			primaryUserID;
	PGPRecipientTableItem	tableItem;
	PGPInt32				algorithm;

	pgpClearMemory( &tableItem, sizeof( tableItem ) );
	
	tableItem.user.keyRef 		= theKey;
	tableItem.user.isDefaultKey	= isDefaultKey;
	tableItem.itemKind			= kTableItemKindKey;

	err	= PGPCountAdditionalRecipientRequests( theKey,
				&tableItem.user.numADKeys );
	
	if( IsPGPError( PGPGetPrimaryUserID( theKey, &primaryUserID ) ) )
	{
		primaryUserID = kInvalidPGPUserIDRef;
	}
	
	err = PGPGetKeyNumber( theKey, kPGPKeyPropAlgID, &algorithm );

	tableItem.user.algorithm = (PGPPublicKeyAlgorithm)algorithm;
	
	if( IsntPGPError( err ) )
	{
		PGPBoolean	isAxiomaticKey;
		
		err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsAxiomatic,
							&isAxiomaticKey );
							
		tableItem.user.isAxiomaticKey = isAxiomaticKey;
	}
	
	if( IsntPGPError( err ) )
	{
		PGPInt32	keyBits;
		
		err = PGPGetKeyNumber( theKey, kPGPKeyPropBits, &keyBits);
		
		tableItem.user.keyBits = keyBits;
	}
		
	if( IsntPGPError( err ) )
	{
		PGPBoolean	isSecretKey;
		
		err = PGPGetKeyBoolean( theKey, kPGPKeyPropIsSecret, &isSecretKey );
		
		tableItem.user.isSecretKey = isSecretKey;
	}	

	if( IsntPGPError( err ) )
	{
		PGPKeySetRef	keySet;
		
		// Create a singleton key set so we can iterate over the key
		// to get subkeys and user id info.
		
		err = PGPNewSingletonKeySet( theKey, &keySet );
		if( IsntPGPError( err ) )
		{
			PGPKeyListRef	keyList;
			
			err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keyList );
			if( IsntPGPError( err ) )
			{
				PGPKeyIterRef	iterator;
				
				err = PGPNewKeyIter( keyList, &iterator );
				if( IsntPGPError( err ) )
				{
					PGPKeyRef	iterKey;
					
					// Iterate our one-key set to the first key
					
					(void) PGPKeyIterNext( iterator, &iterKey );
					
					if( tableItem.user.algorithm ==
							kPGPPublicKeyAlgorithm_DSA )
					{
						PGPSubKeyRef	subKey;

						// Get the subkey to determine the
						// encryption key bits.
							
						err = PGPKeyIterNextSubKey( iterator, &subKey );
						if( IsntPGPError( err ) )
						{
							PGPInt32	subKeyBits;
							
							err = PGPGetSubKeyNumber( subKey,
										kPGPKeyPropBits,
										&subKeyBits );
										
							tableItem.user.subKeyBits = subKeyBits;
						}
					}
					
					if( IsntPGPError( err ) )
					{
						PGPUserIDRef	curUserID;
						
						err = PGPKeyIterNextUserID( iterator,
									&curUserID );
						while( IsntPGPError( err ) )
						{
							PGPSize		bufferSize;
							char		tempName[ 256 ];
							Boolean		addUser = TRUE;
							
							tableItem.user.isPrimaryUserID = 
								( curUserID == primaryUserID );
							
							if( addDefaultUserOnly &&
								! tableItem.user.isPrimaryUserID )
							{
								addUser = FALSE;
							}
							
							if( addUser )
							{
								err = PGPGetUserIDStringBuffer(
											curUserID,
											kPGPUserIDPropName, 
											sizeof( tempName ),
											tempName,
											&bufferSize );
								if( IsntPGPError( err ) ||
									err == kPGPError_BufferTooSmall )
								{
									PGPInt32	userValidity;
									
									tableItem.SetName( tempName );
									
									err = PGPGetUserIDNumber(
												curUserID,
												kPGPUserIDPropValidity,
												&userValidity );
									if( IsntPGPError( err ) )
									{
										tableItem.validity =
											(PGPValidity)userValidity;
										
										AddTableItem( &tableItem, FALSE );
									}
								}
							}
							
							err = PGPKeyIterNextUserID( iterator,
										&curUserID );
						}
					
						if( err == kPGPError_EndOfIteration )
							err = kPGPError_NoErr;
					}
					
					PGPFreeKeyIter( iterator );
				}
				
				PGPFreeKeyList( keyList );
			}
			
			PGPFreeKeySet( keySet );
		}
	}
	
	return( err );
}

	void
CUserIDTable::DisableSelf(void)
{
	LTableView::DisableSelf();
	
	UnselectAllCells();
	Draw( NULL );
}

	void
CUserIDTable::EnableSelf(void)
{
	LTableView::EnableSelf();
	
	Draw( NULL );
}

	void
CUserIDTable::DontBeTarget(void)
{
	UnselectAllCells();
}

	void
CUserIDTable::MarkSelected(void)
{
	STableCell	cell(0, 0);

	while( GetNextCell( cell ) )
	{
		PGPRecipientTableItem	tableItem;
		
		GetCellData( cell, &tableItem );
			tableItem.marked = CellIsSelected( cell );
		SetCellData( cell, &tableItem );
	}
}

	void
CUserIDTable::MarkAll(void)
{
	STableCell	cell(0, 0);

	while( GetNextCell( cell ) )
	{
		PGPRecipientTableItem	tableItem;
		
		GetCellData( cell, &tableItem );
			tableItem.marked = TRUE;
		SetCellData( cell, &tableItem );
	}
}

	void
CUserIDTable::UnmarkAll(void)
{
	STableCell	cell(0, 0);

	while( GetNextCell( cell ) )
	{
		PGPRecipientTableItem	tableItem;
		
		GetCellData( cell, &tableItem );
			tableItem.marked = FALSE;
		SetCellData( cell, &tableItem );
	}
}

	void
CUserIDTable::MarkOneRow(TableIndexT rowIndex)
{
	PGPRecipientTableItem	tableItem;
	STableCell				cell(rowIndex, 1);
	
	GetCellData( cell, &tableItem );
		tableItem.marked = TRUE;
	SetCellData( cell, &tableItem );
}

	Boolean
CUserIDTable::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = FALSE;
	
	switch( inCommand )
	{
		case msg_TabSelect:
			cmdHandled = TRUE;
			break;
		
		default:
			LCommander::ObeyCommand( inCommand, ioParam );
			break;
	}
	
	return cmdHandled;
}

	void
CUserIDTable::ResizeFrameBy(
	Int16		inWidthDelta,
	Int16		inHeightDelta,
	Boolean		inRefresh)
{
	LTableView::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);

	SetColWidth( mFrameSize.width, 1, 1 );
}



