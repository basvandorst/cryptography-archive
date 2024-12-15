/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CUserIDTable.cp,v 1.19 1999/03/10 02:41:33 heller Exp $____________________________________________________________________________*/#include <ctype.h>#include <Icons.h>#include <LowMem.h>#include <string.h>#include <LComparator.h>#include <LDragTask.h>#include <LTableArrayStorage.h>#include <LTableMonoGeometry.h>#include <LTableMultiSelector.h>#include <PP_KeyCodes.h>#include <PP_Messages.h>#include <UDrawingState.h>#include <UGAColorRamp.h>#include <UGraphicUtils.h>#include <UDrawingUtils.h>#include "MacBasics.h"#include "MacDebugLeaks.h"#include "MacFiles.h"#include "MacQuickdraw.h"#include "MacStrings.h"#include "pgpErrors.h"#include "pgpKeys.h"#include "pgpMem.h"#include "pgpUserInterface.h"#include "pgpRecipientDialogCommon.h"#include "CUserIDTable.h"#include "PGPsdkUILibDialogs.h"#include "PGPsdkUILibUtils.h"#include "pgpGroups.h"const ResID kRSAUserIDIconResID 	= 3002;const ResID kDSSDHUserIDIconResID 	= 3020;const ResID kUnknownUserIDIconResID = 4752;const ResID kValidKeyIconResID 		= 3014;const ResID kInvalidKeyIconResID 	= 3015;const ResID kAxiomaticKeyIconResID 	= 3017;const ResID kLockedKeyIconResID 	= 4753;const ResID kGroupIconResID			= 3026;const ResID kBarberPatternResID 	= 4747;const short	kBarHeight		= 10;const short	kMaxValidity	= 3;// Column and gutter widths, from right to leftconst short	kKeySizeRightMargin		= 5;const short	kKeySizeColumnWidth		= 70;const short	kKeySizeLeftMargin		= 10;const short	kValidityBarColumnWidth	= 40;const short	kValidityBarLeftMargin	= 10;const short kUserIDLeftMargin		= 3;const short	kIconColumnWidth		= 16;const short	kIconLeftMargin			= 3;const short	kIconRightMargin		= 3;class CUserIDComparator : public LComparator{	virtual			~CUserIDComparator(void);	virtual Int32	Compare(const void *inItemOne, const void *inItemTwo,								Uint32 inSizeOne, Uint32 inSizeTwo) const;	virtual	Int32	CompareToKey(const void *inItem, Uint32 inSize,								const void *inKey) const;};CUserIDComparator::~CUserIDComparator(void){}	Int32CUserIDComparator::Compare(	const void*		inItemOne,	const void*		inItemTwo,	Uint32			inSizeOne,	Uint32			inSizeTwo) const{	Int32					result;	const PGPRecipientUser	*tableItemOne;	const PGPRecipientUser	*tableItemTwo;		tableItemOne = *(PGPRecipientUser **) inItemOne;	tableItemTwo = *(PGPRecipientUser **) inItemTwo;	pgpAssertAddrValid( tableItemOne, PGPRecipientUser );	pgpAssertAddrValid( tableItemTwo, PGPRecipientUser );	#if PGP_DEBUG	pgpAssert( inSizeOne == sizeof(PGPRecipientUser *) );	pgpAssert( inSizeTwo == sizeof(PGPRecipientUser *) );#else	#pragma unused( inSizeOne, inSizeTwo )#endif		result = PGPCompareUserIDStrings(					PGPGetRecipientUserNamePtr( tableItemOne ),					PGPGetRecipientUserNamePtr( tableItemTwo ) );					if( result == 0 )	{		// Same User ID text. Use the pointer values to force		// a consistent sorting order.		result = (Uint32) tableItemOne - (Uint32) tableItemTwo;	}		return( result );}	Int32CUserIDComparator::CompareToKey(	const void*	inItem,	Uint32		inSize,	const void*	inKey) const{		Int32					result;	const PGPRecipientUser	*tableItem;	const char *			searchUserID;		(void) inSize;		tableItem 		= *(PGPRecipientUser **) inItem;	searchUserID	= (const char *) inKey;	pgpAssertAddrValid( tableItem, PGPRecipientUser );		result = PGPCompareUserIDStrings(					PGPGetRecipientUserNamePtr( tableItem ), searchUserID );		return( result );}CUserIDTable::CUserIDTable(LStream	*inStream)		: LTableView(inStream),		LDragAndDrop( UQDGlobals::GetCurrentPort(), this ){		mTableGeometry	= new LTableMonoGeometry(this, mFrameSize.width, 18);	mTableSelector	= new LTableMultiSelector(this);		// Note: We own this array and need to dispose of it ourselves.	mStorageArray 	= new LArray( sizeof(PGPRecipientUser *),								new CUserIDComparator, TRUE );	mTableStorage	= new LTableArrayStorage(this, mStorageArray);		mDeleteSendsMessage		= FALSE;	mBarberPixPat			= nil;	mShowMarginalValidity	= FALSE;	mMarginalIsInvalid		= FALSE;}CUserIDTable::~CUserIDTable(){	delete mStorageArray;	if( IsntNull( mBarberPixPat ) )		DisposePixPat( mBarberPixPat );}CUserIDTable *CUserIDTable::CreateFromStream(LStream *inStream){	CUserIDTable	*table;		// LDropArea leaks like a sieve. Need to suspend leaks when we create one.		MacLeaks_Suspend();	table = new CUserIDTable(inStream);	MacLeaks_Resume();		return( table );}	voidCUserIDTable::FinishCreateSelf(){	mLastKeypress	= 0;	mLastClickTime	= 0;	mLastClickCell	= 0;	mBarberPixPat = GetPixPat( kBarberPatternResID );	pgpAssertAddrValid( mBarberPixPat, VoidAlign);		SetCustomHilite( TRUE );}	voidCUserIDTable::DisallowSelections(void){	delete mTableSelector;	mTableSelector = NULL;}	voidCUserIDTable::ApplyForeAndBackColors(void) const{	RGBColor	tempColor;		tempColor = UGAColorRamp::GetBlackColor();	RGBForeColor( &tempColor );	tempColor = UGAColorRamp::GetColor( colorRamp_Gray1 );	RGBBackColor( &tempColor );}	voidCUserIDTable::DrawValidity(	const PGPRecipientUser	*tableItem,	const Rect 				*columnRect,	UInt16 					bitDepth){	Rect		drawRect = *columnRect;	RGBColor	tempColor;	RGBColor	saveBackColor;		GetBackColor( &saveBackColor );		tempColor = UGAColorRamp::GetBlackColor();	RGBForeColor( &tempColor );	if( mShowMarginalValidity )	{		RGBColor	trustColor = { 52223, 52223, 65535 };				drawRect.bottom = drawRect.top + kBarHeight;		AlignRect( columnRect, &drawRect, kAlignAtVerticalCenter );		FrameRect( &drawRect );		InsetRect( &drawRect, 1, 1);				if( tableItem->kind == kPGPRecipientUserKind_Key &&			IsntNull( tableItem->userInfo.key ) &&			tableItem->userInfo.key->isAxiomaticKey &&			IsntNull( mBarberPixPat ) )		{			PenPixPat( mBarberPixPat );			PaintRect( &drawRect );			PenNormal();		}		else		{			short 	width;			short	barPercent;						switch( tableItem->validity )			{				default:				case kPGPValidity_Unknown:				case kPGPValidity_Invalid:					barPercent = 0;					break;									case kPGPValidity_Marginal:					barPercent = 50;					break;									case kPGPValidity_Complete:					barPercent = 100;					break;			}						if( bitDepth >= 8 )			{				RGBForeColor( &trustColor );				PaintRect( &drawRect );			}						width 			= drawRect.right - drawRect.left;			drawRect.right 	-= (width * (100 - barPercent)) / 100;						tempColor = UGAColorRamp::GetColor( colorRamp_Gray9 );			RGBForeColor( &tempColor );			PaintRect( &drawRect );		}	}	else	{		ResID	iconResID;				if( tableItem->kind == kPGPRecipientUserKind_Key &&			IsntNull( tableItem->userInfo.key ) &&			tableItem->userInfo.key->isAxiomaticKey )		{			iconResID = kAxiomaticKeyIconResID;		}		else if( tableItem->validity == kPGPValidity_Complete )		{			iconResID = kValidKeyIconResID;		}		else if( tableItem->validity == kPGPValidity_Marginal )		{			if( mMarginalIsInvalid )			{				iconResID = kInvalidKeyIconResID;			}			else			{				iconResID = kValidKeyIconResID;			}		}		else		{			iconResID = kInvalidKeyIconResID;		}				drawRect.bottom = drawRect.top + 16;		drawRect.right 	= drawRect.left + 16;		AlignRect( columnRect, &drawRect, kAlignAtAbsoluteCenter );				tempColor = UGAColorRamp::GetWhiteColor();		RGBBackColor( &tempColor );		PlotIconID( &drawRect, kAlignNone,				IsEnabled() ? kTransformNone : kTransformDisabled,				iconResID );	}		tempColor = UGAColorRamp::GetBlackColor();	RGBForeColor( &tempColor );		RGBBackColor( &saveBackColor );}	voidCUserIDTable::GetCellColumnRects(	const Rect	*cellRect,	Rect		*iconColumnRect,	Rect		*userIDColumnRect,	Rect		*validityColumnRect,	Rect		*keySizeColumnRect){	// Calculate all of the column rectangles		*keySizeColumnRect 			= *cellRect;	keySizeColumnRect->right 	= cellRect->right - kKeySizeRightMargin;	keySizeColumnRect->left		= keySizeColumnRect->right -										kKeySizeColumnWidth;		*validityColumnRect 		= *cellRect;	validityColumnRect->right 	= keySizeColumnRect->left -										kKeySizeLeftMargin;	validityColumnRect->left 	= validityColumnRect->right -										kValidityBarColumnWidth;		*iconColumnRect 			= *cellRect;	iconColumnRect->left 		= cellRect->left + kIconLeftMargin;	iconColumnRect->right		= iconColumnRect->left + kIconColumnWidth;		*userIDColumnRect			= *cellRect;	userIDColumnRect->left 		= iconColumnRect->right + kIconRightMargin;	userIDColumnRect->right 	= validityColumnRect->left -										kValidityBarLeftMargin;}	voidCUserIDTable::DrawCellContents(	const STableCell	&inCell,	const Rect			&inLocalRect,	UInt32				inBitDepth){	Rect				iconRect;	Rect				keySizeRect;	Rect				validityColumnRect;	Rect				userIDRect;	Rect				userIDColumnRect;	PGPRecipientUser	*tableItem;	Str255				str;	IconTransformType	iconTransform;	short				textMode;	short				textHeight;	PGPUInt16			maxUserIDWidth;	RGBColor			tempColor;	short				iconID;		GetCellUser( inCell, &tableItem );		ApplyForeAndBackColors();		textHeight = 12;		GetCellColumnRects( &inLocalRect, &iconRect,		&userIDColumnRect, &validityColumnRect, &keySizeRect );			// Adjust column rectangles to fit enclosed content.		userIDRect			= userIDColumnRect;	userIDRect.left		= userIDRect.left + kUserIDLeftMargin;	userIDRect.bottom 	= userIDRect.top + textHeight;	AlignRect( &inLocalRect, &userIDRect, kAlignAtVerticalCenter );	if( IsEnabled() )	{		textMode 		= 0;		iconTransform	= ttNone;	}	else	{		textMode		= grayishTextOr;		iconTransform 	= ttDisabled;	}		if( tableItem->kind == kPGPRecipientUserKind_MissingRecipient )	{		textMode		= grayishTextOr;		iconTransform	= ttDisabled;	}		// Draw Icon			if( tableItem->kind == kPGPRecipientUserKind_Key )	{		if( tableItem->userInfo.key->algorithm == kPGPPublicKeyAlgorithm_RSA )		{			iconID = kRSAUserIDIconResID;		}		else		{			iconID = kDSSDHUserIDIconResID;		}	}	else if( tableItem->kind == kPGPRecipientUserKind_MissingRecipient )	{		iconID = kUnknownUserIDIconResID;	}	else if ( tableItem->kind == kPGPRecipientUserKind_Group )	{		iconID = kGroupIconResID;	}	else	{		pgpDebugMsg( "DrawCellContents: Unknown item status" );	}	iconRect.bottom = iconRect.top + 16;	AlignRect( &inLocalRect, &iconRect, kAlignAtVerticalCenter );	PlotIconID( &iconRect, atNone, iconTransform, iconID );		// Draw User ID		tempColor = UGAColorRamp::GetColor( colorRamp_Gray2 );	RGBBackColor( &tempColor );		EraseRect( &userIDColumnRect );		TextSize(9);	TextFont(1);	TextMode( textMode );	if( tableItem->kind == kPGPRecipientUserKind_Key )	{		if( tableItem->userInfo.key->isDefaultKey )		{			TextFace( bold );		}		else		{			TextFace( 0 );		}	}	else if ( tableItem->kind == kPGPRecipientUserKind_MissingRecipient ||				tableItem->kind == kPGPRecipientUserKind_Group )	{		TextFace( 0 );	}	if( tableItem->kind == kPGPRecipientUserKind_Key ||		tableItem->kind == kPGPRecipientUserKind_MissingRecipient )	{		if( tableItem->lockRefCount != 0 )		{			Rect	lockedIconRect;						lockedIconRect 			= userIDRect;			lockedIconRect.left 	= lockedIconRect.right - 16;			lockedIconRect.bottom 	= lockedIconRect.top + 16;					AlignRect( &userIDRect, &lockedIconRect, kAlignAtCenterRight );			PlotIconID( &lockedIconRect, atNone, iconTransform,						kLockedKeyIconResID );									userIDRect.right -= 16;		}	}		MoveTo( userIDRect.left, userIDRect.bottom - 2 );	CToPString( PGPGetRecipientUserNamePtr( tableItem ), str );		maxUserIDWidth = userIDRect.right - userIDRect.left;	if( StringWidth( str ) > maxUserIDWidth )	{		TextFace( condense );				if( StringWidth( str ) > maxUserIDWidth )		{			// will not fit condensed. Resort to the default face and			// truncate in the middle			TextFace( 0 );			TruncString( maxUserIDWidth, str, smTruncMiddle );		}	}		DrawString( str );	TextFace( 0 );		tempColor = UGAColorRamp::GetColor( colorRamp_Gray1 );	RGBBackColor( &tempColor );		DrawValidity( tableItem, &validityColumnRect, inBitDepth );		keySizeRect.bottom = keySizeRect.top + textHeight;	AlignRect( &inLocalRect, &keySizeRect, kAlignAtVerticalCenter );	if( tableItem->kind == kPGPRecipientUserKind_Key ||		tableItem->kind == kPGPRecipientUserKind_Group )	{		PGPBoolean		drawIt	= FALSE;		Str63			theString;				// Draw key size		if ( tableItem->kind == kPGPRecipientUserKind_Key )		{			if( tableItem->userInfo.key->keyBits != 0 )			{				drawIt = TRUE;								if( tableItem->userInfo.key->subKeyBits != 0 )				{					Str32	tempStr;									GetIndString( theString,							kPGPLibDialogsStringListResID,							kDSSDHKeySizeFormatStrIndex );												NumToString( tableItem->userInfo.key->subKeyBits, tempStr );					PrintPString( theString, theString, tempStr);					NumToString( tableItem->userInfo.key->keyBits, tempStr );					PrintPString( theString, theString, tempStr);				}				else				{					NumToString( tableItem->userInfo.key->keyBits, theString );				}			}		}		else		{			Str32	tempStr;						pgpAssert( tableItem->kind == kPGPRecipientUserKind_Group );						drawIt = TRUE;						if( tableItem->groupInfo.numMissingKeys != 0 )			{				GetIndString( theString, kPGPLibDialogsStringListResID,							kNumMissingGroupKeysFormatStrIndex );											NumToString( tableItem->groupInfo.numKeys -							tableItem->groupInfo.numMissingKeys, tempStr );				PrintPString( theString, theString, tempStr );			}			else			{				GetIndString( theString, kPGPLibDialogsStringListResID,							kNumGroupKeysFormatStrIndex );			}						NumToString( tableItem->groupInfo.numKeys, tempStr );			PrintPString( theString, theString, tempStr );		}				if ( drawIt )		{			MoveTo( keySizeRect.left, keySizeRect.bottom - 2 );			DrawString( theString );		}	}		TextMode( srcOr );		ForeColor( whiteColor );	MoveTo( inLocalRect.left, inLocalRect.bottom - 1 );	LineTo( inLocalRect.right, inLocalRect.bottom - 1 );	ForeColor( blackColor );}	voidCUserIDTable::DrawCell(	const STableCell	&inCell,	const Rect			&inLocalRect){	StDeviceLoop 	devLoop( inLocalRect );			SInt16			depth;	while( devLoop.NextDepth( depth ) )	{		DrawCellContents( inCell, inLocalRect, depth );	}}	voidCUserIDTable::DragSelectedCells(const EventRecord& inMacEvent){	RgnHandle	dragRgn;	RgnHandle	tempRgn;	Point		localPt;	Point		globalPt;		// This routine does not actually add the real cell data to the	// drag. Instead, a token value (the pane ID) is added to the drag which	// is checked for by the reveiving pane. If the token is presnet and the	// destination pane is not the source pane, a message is broadcast to the	// PGPRecipientGrafPort to move the selected cells to the receiving pane.		FocusDraw();	ApplyForeAndBackColors();		dragRgn = NewRgn();	tempRgn	= NewRgn();		GetHiliteRgn( dragRgn );	CopyRgn( dragRgn, tempRgn );	InsetRgn( tempRgn, 1, 1 );	DiffRgn( dragRgn, tempRgn, dragRgn );		// Need to convert the reqion to global coordinates.	localPt.v = (**dragRgn).rgnBBox.top;	localPt.h = (**dragRgn).rgnBBox.left;		globalPt = localPt;	LocalToGlobal( &globalPt );		OffsetRgn( dragRgn, globalPt.h - localPt.h, globalPt.v - localPt.v );		LDragTask	theDragTask( inMacEvent, dragRgn, 1, 'User', &mPaneID,						sizeof( mPaneID ), flavorSenderOnly );	// Invalidate LView's focus cache. The port may have changed during the	// drag.	LView::OutOfFocus( nil );	DisposeRgn( dragRgn );	DisposeRgn( tempRgn );}	voidCUserIDTable::ClickCell(	const STableCell&		inCell,	const SMouseDownEvent&	inMouseDown){	if((inMouseDown.macEvent.when - mLastClickTime > LMGetDoubleTime()) ||		(mLastClickCell != inCell))	{		mLastClickTime = inMouseDown.macEvent.when;		mLastClickCell = inCell;				if( DragAndDropIsPresent() &&			WaitMouseMoved( inMouseDown.macEvent.where ) &&			CellIsSelected( inCell ) )		{			DragSelectedCells( inMouseDown.macEvent );		}	}	else	{		mLastClickTime = 0;		BroadcastMessage( 'Send', this);	}}	BooleanCUserIDTable::ItemIsAcceptable(	DragReference	inDragRef,	ItemReference	inItemRef ){	FlavorFlags	theFlags;		return IsEnabled() && IsntErr( GetFlavorFlags( inDragRef,		inItemRef, 'User', &theFlags ) );}	voidCUserIDTable::UnhiliteDropArea(	DragReference	inDragRef){	ApplyForeAndBackColors();	LDragAndDrop::UnhiliteDropArea( inDragRef );}	voidCUserIDTable::ReceiveDragItem(	DragReference	inDragRef,	DragAttributes	inDragAttrs,	ItemReference	inItemRef,	Rect			&inItemBounds ){#pragma unused( inDragAttrs, inItemBounds )	PaneIDT		sourcePaneID;	Size		theDataSize = sizeof( sourcePaneID );	OSErr		err;		err = GetFlavorData( inDragRef, inItemRef,		'User', &sourcePaneID, &theDataSize, 0 );	if( IsntErr( err ) && sourcePaneID != mPaneID )	{		// Unhilite manually because the result of BroadcastMessage may		// display a dialog which will mess up the hilighting.				FocusDropArea();		mIsHilited = false;		UnhiliteDropArea( inDragRef );				BroadcastMessage( 'Recv', this );	}}	voidCUserIDTable::HiliteCellActively(	const STableCell	&inCell,	Boolean				inHilite){	Rect	cellFrame;		#pragma unused( inHilite )		if( GetLocalCellRect( inCell, cellFrame ) &&		FocusExposed() )	{		StDeviceLoop	devLoop( cellFrame );				SInt16			depth;		while( devLoop.NextDepth( depth ) )		{			StColorPenState saveColorPen;   // Preserve color & pen state			Rect			iconColumnRect;			Rect			userIDColumnRect;			Rect			validityColumnRect;			Rect			keySizeColumnRect;			StColorPenState::Normalize();						if( depth >= 8 )			{				RGBColor	tempColor;				tempColor = UGAColorRamp::GetBlackColor();				RGBForeColor( &tempColor );				tempColor = UGAColorRamp::GetColor( colorRamp_Gray2 );				RGBBackColor( &tempColor );			}			else			{				ForeColor( blackColor );				BackColor( whiteColor );			}						GetCellColumnRects( &cellFrame, &iconColumnRect,				&userIDColumnRect, &validityColumnRect,				&keySizeColumnRect );			        UDrawingUtils::SetHiliteModeOn();			InvertRect( &userIDColumnRect );		}	}}	voidCUserIDTable::HiliteCellInactively(	const STableCell	&inCell,	Boolean			 	inHilite){	Rect	cellFrame;		#pragma unused( inHilite )	if( GetLocalCellRect( inCell, cellFrame ) &&		FocusExposed() )	{		StDeviceLoop	devLoop( cellFrame );				SInt16			depth;		while( devLoop.NextDepth( depth ) )		{	        StColorPenState saveColorPen;   // Preserve color & pen state			Rect			iconColumnRect;			Rect			userIDColumnRect;			Rect			validityColumnRect;			Rect			keySizeColumnRect;	        	        StColorPenState::Normalize();	        	        if( depth >= 8 )	        {	        	ApplyForeAndBackColors();			}			else			{				ForeColor( blackColor );				BackColor( whiteColor );			}						GetCellColumnRects( &cellFrame, &iconColumnRect,				&userIDColumnRect, &validityColumnRect,				&keySizeColumnRect );					PenMode(srcXor);			UDrawingUtils::SetHiliteModeOn();			FrameRect( &userIDColumnRect );		}	}}	BooleanCUserIDTable::HandleKeyPress(const EventRecord&	inKeyEvent){	Boolean		keyHandled	= TRUE;	Int16		theKey		= inKeyEvent.message & charCodeMask,				charInx;		if( inKeyEvent.modifiers & cmdKey )	{		keyHandled = LCommander::HandleKeyPress( inKeyEvent );	}	else if( IsEnabled() )	{		switch(theKey)		{			default:			{				if(inKeyEvent.when - kMaxKeypressInterval <= mLastKeypress)				{					if((charInx = strlen(mSearchString)) <								kMaxSearchStringSize - 1)					{						mSearchString[charInx]		= theKey;						mSearchString[charInx+1]	= 0;					}				}				else				{					mSearchString[0] = theKey;					mSearchString[1] = 0;				}				mLastKeypress = inKeyEvent.when;				SelectSearchString();				break;			}						case char_LeftArrow:			case char_RightArrow:				break;							case char_UpArrow:			{				STableCell	firstSelectedCell(0, 0);								if( GetNextSelectedCell( firstSelectedCell ) )				{					UnselectAllCells();					if( firstSelectedCell.row != 1 )					{						firstSelectedCell.row -= 1;					}				}				else				{					// There are no cells selected. Select the first cell					firstSelectedCell.SetCell( 1, 1 );				}								ScrollCellIntoFrame( firstSelectedCell );				SelectCell( firstSelectedCell );				break;			}						case char_DownArrow:			{				STableCell	lastSelectedCell(0, 0);				STableCell	curSelectedCell(0, 0);				TableIndexT	numRows;				TableIndexT	numColumns;								while( GetNextSelectedCell( curSelectedCell ) )				{					lastSelectedCell = curSelectedCell;				}								UnselectAllCells();				GetTableSize( numRows, numColumns );								if( lastSelectedCell.row == 0 )				{					// No row selected. Select the last data row.										lastSelectedCell.SetCell( numRows, 1 );				}				else if( lastSelectedCell.row == numRows )				{					// Last row is already selected. Select again				}				else				{					++lastSelectedCell.row;				}								ScrollCellIntoFrame( lastSelectedCell );				SelectCell( lastSelectedCell );				break;			}						case char_Home:				// Using min_Int32 here causes overflow problems in				// ScrollPinnedImageBy(). Use a "large" negative arbritray				// number instead.				ScrollPinnedImageBy( 0, -100000, TRUE );				break;							case char_End:				// Using max_Int32 here causes overflow problems in				// ScrollPinnedImageBy(). Use a "large" arbritray number				// instead.				ScrollPinnedImageBy( 0, 100000, TRUE );				break;			case char_PageUp:			{				Rect	frameRect;								CalcLocalFrameRect( frameRect );				ScrollPinnedImageBy( 0, -UGraphicUtils::RectHeight(							frameRect ), TRUE );				break;			}			case char_PageDown:			{				Rect	frameRect;								CalcLocalFrameRect( frameRect );				ScrollPinnedImageBy( 0, UGraphicUtils::RectHeight(							frameRect ), TRUE );				break;			}							case char_Return:				if( GetFirstSelectedCell().row > 0 )				{					BroadcastMessage( 'Send', this);				}				break;							case char_Backspace:			{				if( mDeleteSendsMessage && GetFirstSelectedCell().row > 0 )				{					BroadcastMessage( 'Send', this);				}								break;			}						case char_Tab:			case char_Enter:			case char_Escape:				keyHandled = LCommander::HandleKeyPress(inKeyEvent);				break;		}	}		return keyHandled;}	voidCUserIDTable::SelectSearchString(){	STableCell				cell;	TableIndexT				rows, cols;		FocusDraw();	ApplyForeAndBackColors();		UnselectAllCells();	GetTableSize(rows, cols);	cell.col = 1;	cell.row = mStorageArray->FetchInsertIndexOfKey( mSearchString );		if( cell.row <= 0 || cell.row > rows )		cell.row = rows;			if( cell.row > 0 )	{		ScrollCellIntoFrame( cell );		SelectCell( cell );	}}	voidCUserIDTable::ClickSelf(	const SMouseDownEvent &inMouseDown){	if( IsntNull( mTableSelector ) )	{		ApplyForeAndBackColors();		SwitchTarget( this );		LTableView::ClickSelf(inMouseDown);	}}	voidCUserIDTable::DrawSelf(){	Rect 		frame;	RGBColor	tempColor;		tempColor = UGAColorRamp::GetColor( colorRamp_Gray1 );	RGBForeColor( &tempColor );		tempColor = UGAColorRamp::GetWhiteColor();	RGBBackColor( &tempColor );		CalcLocalFrameRect( frame );	{		StDeviceLoop	devLoop( frame );				SInt16			depth;		while( devLoop.NextDepth( depth ) )		{			if( depth > 1 )				PaintRect( &frame );			else				EraseRect( &frame );		}	}		LTableView::DrawSelf();}	static PGPBooleansRemoveAngleBrackets(	const char *	in,	char *			out ){	PGPUInt32	length;	PGPBoolean	haveBrackets	= FALSE;		/* remove brackets if present */	length			= strlen( in );	haveBrackets	= in[ 0 ] == '<' && in[ length - 1 ] == '>';		if ( haveBrackets )	{		CopyCString( in + 1, out );		out[ length -1 - 1 ]	= '\0';	}	else	{		CopyCString( in, out );	}	return( haveBrackets );}	voidCUserIDTable::SetPreferences(	Boolean showMarginalValidity,	Boolean	marginalIsInvalid){	mShowMarginalValidity 	= showMarginalValidity;	mMarginalIsInvalid		= marginalIsInvalid;}	voidCUserIDTable::GetCellUser(	const STableCell	&inCell,	PGPRecipientUser	**tableItemPtr) const{	Uint32	dataSize;		dataSize = sizeof( *tableItemPtr );	GetCellData( inCell, (void *) tableItemPtr, dataSize );		pgpAssert( dataSize == sizeof( *tableItemPtr ) );}	voidCUserIDTable::SetCellUser(	const STableCell		&inCell,	const PGPRecipientUser	*tableItem){	SetCellData( inCell, &tableItem, sizeof( &tableItem ) );}		voidCUserIDTable::DisableSelf(void){	LTableView::DisableSelf();		UnselectAllCells();	Draw( NULL );}	voidCUserIDTable::EnableSelf(void){	LTableView::EnableSelf();		Draw( NULL );}	voidCUserIDTable::DontBeTarget(void){	UnselectAllCells();}	BooleanCUserIDTable::AllowBeTarget(LCommander *inNewTarget){	Boolean	allowSwitch;		allowSwitch = LCommander::AllowBeTarget( inNewTarget );	if( IsNull( mTableSelector ) )	{		allowSwitch = FALSE;	}	return( allowSwitch );}	BooleanCUserIDTable::ObeyCommand(	CommandT	inCommand,	void*		ioParam){	Boolean		cmdHandled = FALSE;		switch( inCommand )	{		case msg_TabSelect:			cmdHandled = TRUE;			break;				default:			LCommander::ObeyCommand( inCommand, ioParam );			break;	}		return cmdHandled;}	voidCUserIDTable::ResizeFrameBy(	Int16		inWidthDelta,	Int16		inHeightDelta,	Boolean		inRefresh){	LTableView::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);	SetColWidth( mFrameSize.width, 1, 1 );}