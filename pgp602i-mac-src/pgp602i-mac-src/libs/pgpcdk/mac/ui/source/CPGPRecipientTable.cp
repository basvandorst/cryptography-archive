/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CPGPRecipientTable.cp,v 1.4.10.1 1998/11/12 03:20:45 heller Exp $____________________________________________________________________________*/#include <LComparator.h>#include <LTableArrayStorage.h>#include <LTableMonoGeometry.h>#include <UDrawingUtils.h>#include <UGAColorRamp.h>#include "MacQuickdraw.h"#include "MacStrings.h"#include "pgpKeys.h"#include "pgpMem.h"#include "CPGPRecipientTable.h"const ResID kRSAUserIDIconResID 	= 3002;const ResID kDSSDHUserIDIconResID 	= 3020;const ResID kUnknownUserIDIconResID = 4752;const short kUserIDLeftMargin		= 3;const short kUserIDRightMargin		= 3;const short	kIconColumnWidth		= 16;const short	kIconLeftMargin			= 3;const short	kIconRightMargin		= 3;class CRecipientComparator : public LComparator{	virtual			~CRecipientComparator(void);	virtual Int32	Compare(const void *inItemOne, const void *inItemTwo,								Uint32 inSizeOne, Uint32 inSizeTwo) const;};CRecipientComparator::~CRecipientComparator(void){}	Int32CRecipientComparator::Compare(	const void*		inItemOne,	const void*		inItemTwo,	Uint32			inSizeOne,	Uint32			inSizeTwo) const{	Int32						result;	const RecipientTableItem	*tableItemOne;	const RecipientTableItem	*tableItemTwo;		tableItemOne = (RecipientTableItem *) inItemOne;	tableItemTwo = (RecipientTableItem *) inItemTwo;	pgpAssertAddrValid( tableItemOne, RecipientTableItem );	pgpAssertAddrValid( tableItemTwo, RecipientTableItem );	#if PGP_DEBUG	pgpAssert( inSizeOne == sizeof(RecipientTableItem) );	pgpAssert( inSizeTwo == sizeof(RecipientTableItem) );#else	#pragma unused( inSizeOne, inSizeTwo )#endif		result = tableItemOne->sortGrouping - tableItemTwo->sortGrouping;	if( result == 0 )	{		result = PGPCompareUserIDStrings( tableItemOne->description,					tableItemTwo->description );	}		return( result );}CPGPRecipientTable::CPGPRecipientTable(LStream	*inStream)		: LTableView( inStream ){	mTableGeometry	= new LTableMonoGeometry(this, mFrameSize.width, 18);	mTableSelector	= NULL;		// Note: We own this array and need to dispose of it ourselves.	mStorageArray 	= new LArray( sizeof(RecipientTableItem),								new CRecipientComparator, TRUE );	mTableStorage	= new LTableArrayStorage(this, mStorageArray);}CPGPRecipientTable::~CPGPRecipientTable(){	delete mStorageArray;}	voidCPGPRecipientTable::PrepareToDraw(void){	RGBColor	tempColor;		tempColor = UGAColorRamp::GetBlackColor();	RGBForeColor( &tempColor );	tempColor = UGAColorRamp::GetColor( colorRamp_Gray1 );	RGBBackColor( &tempColor );}	voidCPGPRecipientTable::GetCellColumnRects(	const Rect	*cellRect,	Rect		*iconColumnRect,	Rect		*userIDColumnRect){	// Calculate all of the column rectangles		*iconColumnRect 		= *cellRect;	iconColumnRect->left 	= cellRect->left + kIconLeftMargin;	iconColumnRect->right	= iconColumnRect->left + kIconColumnWidth;		*userIDColumnRect		= *cellRect;	userIDColumnRect->left 	= iconColumnRect->right + kIconRightMargin;}	voidCPGPRecipientTable::DrawCellContents(	const STableCell	&inCell,	const Rect			&inLocalRect){	Rect				iconRect;	Rect				userIDRect;	Rect				userIDColumnRect;	RecipientTableItem	tableItem;	Str255				str;	IconTransformType	iconTransform;	short				textMode;	short				textHeight;	PGPUInt16			maxUserIDWidth;	RGBColor			tempColor;	short				iconID;	Uint32				dataSize;		dataSize = sizeof( tableItem );	GetCellData( inCell, &tableItem, dataSize );	PrepareToDraw();		textHeight = 12;		GetCellColumnRects( &inLocalRect, &iconRect, &userIDColumnRect );			// Adjust column rectangles to fit enclosed content.		userIDRect			= userIDColumnRect;	userIDRect.left		= userIDRect.left + kUserIDLeftMargin;	userIDRect.right	= userIDRect.right - kUserIDRightMargin;	userIDRect.bottom 	= userIDRect.top + textHeight;		AlignRect( &inLocalRect, &userIDRect, kAlignAtVerticalCenter );	if( IsEnabled() )	{		textMode 		= 0;		iconTransform	= ttNone;	}	else	{		textMode		= grayishTextOr;		iconTransform 	= ttDisabled;	}		if( tableItem.itemKind == kTableItemKindMissingKey )	{		textMode		= grayishTextOr;		iconTransform	= ttDisabled;	}		// Draw Icon	if( tableItem.itemKind == kTableItemKindKey )	{		if( tableItem.algorithm == kPGPPublicKeyAlgorithm_RSA )		{			iconID = kRSAUserIDIconResID;		}		else		{			iconID = kDSSDHUserIDIconResID;		}	}	else if( tableItem.itemKind == kTableItemKindMissingKey )	{		iconID = kUnknownUserIDIconResID;	}	else	{		pgpDebugMsg( "DrawCellContents: Unknown item status" );	}	iconRect.bottom = iconRect.top + 16;	AlignRect( &inLocalRect, &iconRect, kAlignAtVerticalCenter );	PlotIconID( &iconRect, atNone, iconTransform, iconID );	// Draw User ID		tempColor = UGAColorRamp::GetColor( colorRamp_Gray2 );	RGBBackColor( &tempColor );		EraseRect( &userIDColumnRect );		TextSize(9);	TextFont(1);	TextMode( textMode );	MoveTo( userIDRect.left, userIDRect.bottom - 2 );	CToPString( tableItem.description, str );		maxUserIDWidth = userIDRect.right - userIDRect.left;	if( StringWidth( str ) > maxUserIDWidth )	{		TextFace( condense );				if( StringWidth( str ) > maxUserIDWidth )		{			// will not fit condensed. Resort to the default face and			// truncate in the middle			TextFace( 0 );			TruncString( maxUserIDWidth, str, smTruncMiddle );		}	}		DrawString( str );	TextFace( 0 );		tempColor = UGAColorRamp::GetColor( colorRamp_Gray1 );	RGBBackColor( &tempColor );			TextMode( srcOr );		ForeColor( whiteColor );	MoveTo( inLocalRect.left, inLocalRect.bottom - 1 );	LineTo( inLocalRect.right, inLocalRect.bottom - 1 );	ForeColor( blackColor );}	voidCPGPRecipientTable::DrawCell(	const STableCell	&inCell,	const Rect			&inLocalRect){	StDeviceLoop 	devLoop( inLocalRect );			SInt16			depth;	while( devLoop.NextDepth( depth ) )	{		DrawCellContents( inCell, inLocalRect );	}}	voidCPGPRecipientTable::DrawSelf(){	Rect 		frame;	RGBColor	tempColor;		tempColor = UGAColorRamp::GetColor( colorRamp_Gray1 );	RGBForeColor( &tempColor );		tempColor = UGAColorRamp::GetWhiteColor();	RGBBackColor( &tempColor );		CalcLocalFrameRect( frame );	{		StDeviceLoop	devLoop( frame );				SInt16			depth;		while( devLoop.NextDepth( depth ) )		{			if( depth > 1 )				PaintRect( &frame );			else				EraseRect( &frame );		}	}		LTableView::DrawSelf();}	voidCPGPRecipientTable::AddTableItem(	RecipientTableItem 	*inTableItem,	Boolean				inRefresh){	pgpAssertAddrValid( inTableItem, RecipientTableItem );	InsertRows( 1, 100000, inTableItem, sizeof( *inTableItem ), inRefresh );}	PGPErrorCPGPRecipientTable::AddMissingUserID(const char *userID){	RecipientTableItem	tableItem;	pgpClearMemory( &tableItem, sizeof( tableItem ) );		CopyCString( userID, tableItem.description );		tableItem.itemKind 				= kTableItemKindMissingKey;	tableItem.sortGrouping 			= -1;	/* Force to top of list */	tableItem.isMissingRecipients	= TRUE;		AddTableItem( &tableItem, TRUE );		return( kPGPError_NoErr );}	PGPErrorCPGPRecipientTable::AddKey(PGPKeyRef theKey){	PGPError				err = kPGPError_NoErr;	RecipientTableItem	tableItem;	PGPInt32				algorithm;	PGPSize					stringSize;		pgpClearMemory( &tableItem, sizeof( tableItem ) );		tableItem.keyRef 		= theKey;	tableItem.itemKind		= kTableItemKindKey;	err = PGPGetPrimaryUserIDNameBuffer( theKey, 255,				tableItem.description, &stringSize );	if( IsntPGPError( err ) )	{		err = PGPGetKeyNumber( theKey, kPGPKeyPropAlgID, &algorithm );		tableItem.algorithm = (PGPPublicKeyAlgorithm)algorithm;	}		if( IsntPGPError( err ) )	{		AddTableItem( &tableItem, TRUE );	}		return( err );}	voidCPGPRecipientTable::ClickCell(	const STableCell&		inCell,	const SMouseDownEvent&	inMouseDown){	(void) inMouseDown;		if ( GetClickCount() == 2)	{		RecipientTableItem	tableItem;		Uint32				dataSize = sizeof( tableItem );				GetCellData( inCell, &tableItem, dataSize );		if( tableItem.isMissingRecipients )			BroadcastMessage( kUpdateMissingRecipientsMsg, NULL );			}}