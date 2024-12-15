/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <PP_Messages.h>
#include <UGraphicsUtilities.h>
#include <UTextTraits.h>

#include "MacQuickdraw.h"

#include "PGPtoolsResources.h"

#include "CResultsTable.h"
#include "CResultsWindow.h"

const PaneIDT	kResultsTablePaneID 			= 5002;
const ResIDT	kColumnTitlesTextTraitsResID	= 135;

const ushort	kColumnSpacingWidth			= 10;
const short		kSingleItemLeftMargin		= 20;
const short		kIconColumnWidth			= 16;
const ushort	kValidityColumnWidth		= 40;
const ushort	kSigningDateColumnWidth		= 70;
const ushort	kMaxNameColumnWidth			= 200;


static CResultsWindow	*gResultsWindow;

CResultsWindow::CResultsWindow(LStream *inStream)
	: CPGPtoolsWindow( inStream )
{
	pgpAssert( gResultsWindow == nil );
	
	gResultsWindow 		= this;
	mHaveSignatures		= FALSE;
	mPendingNewSummary	= FALSE;
}

CResultsWindow::~CResultsWindow(void)
{
}

	void
CResultsWindow::FinishCreateSelf(void)
{
	CPGPtoolsWindow::FinishCreateSelf();
	
	mResultsTable = (CResultsTable *) FindPaneByID( kResultsTablePaneID );
	pgpAssertAddrValid( mResultsTable, VoidAlign );

	UTextTraits::SetPortTextTraits( kColumnTitlesTextTraitsResID );
	GetFontInfo( &mTitlesFontInfo );
}

	void
CResultsWindow::AttemptClose(void)
{
	Hide();
}

	void
CResultsWindow::DoClose(void)
{
	Hide();
}

	void
CResultsWindow::DrawSelf(void)
{
	CPGPtoolsWindow::DrawSelf();
	
	DrawColumnTitles();
}

	void
CResultsWindow::DrawColumnTitles(void)
{
	Rect	textRect;
	Rect	titlesRect;
	Str255	tempStr;
	Rect	iconRect;
	Rect	nameRect;
	Rect	statusRect;
	Rect	validityRect;
	Rect	signingDateRect;
	ushort	textBaseLine;
	Rect	windowRect;

	FocusDraw();
	
	GetColumnTitlesRect( &titlesRect );
	CalcPortFrameRect( windowRect );
		
	windowRect.bottom = titlesRect.bottom;

	EraseRect( &windowRect );
	
	UTextTraits::SetPortTextTraits( kColumnTitlesTextTraitsResID );
	
	// Alight the text vertically within the cell.
	textRect 		= titlesRect;
	textRect.bottom	= textRect.top + mTitlesFontInfo.ascent +
						mTitlesFontInfo.descent +
						mTitlesFontInfo.leading;
	
	AlignRect( &titlesRect, &textRect, kAlignAtVerticalCenter );
	textBaseLine = textRect.bottom - mTitlesFontInfo.descent;
	
	CResultsWindow::CalcColumnBounds( &textRect, &iconRect, &nameRect,
			&statusRect, &validityRect, &signingDateRect );
	
	// Draw the titles
	GetIndString( tempStr, kResultsTableStringListResID,
				kNameColumnTitleStrIndex );
	MoveTo( nameRect.left, textBaseLine );
	DrawString( tempStr );

	GetIndString( tempStr, kResultsTableStringListResID,
				kStatusColumnTitleStrIndex );
	MoveTo( statusRect.left, textBaseLine );
	DrawString( tempStr );

	GetIndString( tempStr, kResultsTableStringListResID,
				kValidityColumnTitleStrIndex );
	MoveTo( validityRect.left, textBaseLine );
	DrawString( tempStr );

	GetIndString( tempStr, kResultsTableStringListResID,
				kSigningDateColumnTitleStrIndex );
	MoveTo( signingDateRect.left, textBaseLine );
	DrawString( tempStr );

}

	void
CResultsWindow::GetColumnTitlesRect(Rect *titlesRect)
{
	Rect	tableRect;
	
	CalcPortFrameRect( *titlesRect );
	mResultsTable->CalcPortFrameRect( tableRect );

	titlesRect->bottom 	= tableRect.top - 1;
	titlesRect->right	= tableRect.right;
}

	void
CResultsWindow::ResizeFrameBy(
	Int16		inWidthDelta,
	Int16		inHeightDelta,
	Boolean		inRefresh)
{
	// Resize the single column to fit to the width of the window.

	// Force redraw of the column titles.	
	
	CPGPtoolsWindow::ResizeFrameBy( inWidthDelta, inHeightDelta,
				inRefresh );	

	FocusDraw();
	
	if( inWidthDelta != 0 )
	{
		Rect	titleRect;
		Rect	windowRect;
		
		GetColumnTitlesRect( &titleRect );
		CalcPortFrameRect( windowRect );
		
		windowRect.bottom = titleRect.bottom;
		InvalRect( &windowRect );
	}
}

	void
CResultsWindow::NewSummary(void)
{
	gResultsWindow->mPendingNewSummary = TRUE;
}

	void
CResultsWindow::AddResult(
	ConstStr255Param		itemName,
	PGPtoolsSignatureData	*signatureData)
{
	SwitchTarget( this );
	Select();
	
	if( mPendingNewSummary )
	{
		mPendingNewSummary = FALSE;
		
		mResultsTable->AddSummaryRow();
	}
	
	mHaveSignatures = TRUE;
	
	mResultsTable->AddProcessedItemRow( itemName, signatureData );

	UpdatePort();
}

	void
CResultsWindow::NewResult(
	ConstStr255Param		itemName,
	PGPtoolsSignatureData	*signatureData)
{
	gResultsWindow->Show();
	gResultsWindow->AddResult( itemName, signatureData );
}

	void
CResultsWindow::IncrementCount(void)
{
	SwitchTarget( this );
	Select();
	Show();
	
	mResultsTable->IncrementVerifiedCount();

	UpdatePort();
}

	void
CResultsWindow::IncrementVerifiedCount(void)
{
	gResultsWindow->IncrementCount();
}

	void
CResultsWindow::CalcColumnBounds(
	const Rect	*cellRect,
	Rect		*iconRect,
	Rect		*nameRect,
	Rect		*statusRect,
	Rect		*validityRect,
	Rect		*signingDateRect)
{
	ushort	statusColumnWidth;
	ushort	nameColumnWidth;
	short	variableWidth;
	
	pgpAssertAddrValid( cellRect, Rect );
	pgpAssertAddrValid( iconRect, Rect );
	pgpAssertAddrValid( nameRect, Rect );
	pgpAssertAddrValid( statusRect, Rect );
	pgpAssertAddrValid( validityRect, Rect );
	pgpAssertAddrValid( signingDateRect, Rect );
	
	// Calculate the column widths. The signing date and validity columns
	// are fixed in width. The item name and status columns vary with
	// the status column getting 40% of the remaining space, up to a maximum.
	
	variableWidth = UGraphicsUtilities::RectWidth( (Rect &) *cellRect ) -
					kSigningDateColumnWidth - kValidityColumnWidth -
					kIconColumnWidth - kSingleItemLeftMargin -
					( 4 * kColumnSpacingWidth );
	pgpAssert( variableWidth > 0 );
					
	nameColumnWidth = variableWidth * 0.4;
	if( nameColumnWidth > kMaxNameColumnWidth )
		nameColumnWidth = kMaxNameColumnWidth;
		
	statusColumnWidth = variableWidth - nameColumnWidth;
	
	// Assumes we're using a 16 pixel wide icon
	*iconRect 		= *cellRect;
	iconRect->left 	+= kSingleItemLeftMargin;
	iconRect->right	= iconRect->left + kIconColumnWidth;
	
	*nameRect 		= *cellRect;
	nameRect->left	= iconRect->right + 5;
	nameRect->right	= nameRect->left + nameColumnWidth;

	*statusRect 		= *cellRect;
	statusRect->left	= nameRect->right + kColumnSpacingWidth;
	statusRect->right	= statusRect->left + statusColumnWidth;

	*validityRect 		= *cellRect;
	validityRect->left	= statusRect->right + kColumnSpacingWidth;
	validityRect->right	= validityRect->left + kValidityColumnWidth;

	*signingDateRect 		= *cellRect;
	signingDateRect->left	= validityRect->right + kColumnSpacingWidth;
	signingDateRect->right	= signingDateRect->left + kSigningDateColumnWidth;
}

	void
CResultsWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	switch (inCommand)
	{
		case cmd_Clear:
		{
			TableIndexT	numRows;
			TableIndexT	numColumns;
			
			mResultsTable->GetTableSize( numRows, numColumns );
			outEnabled = ( numRows > 0 );
			break;
		}
			
		default:
			inherited::FindCommandStatus(inCommand, outEnabled,
												outUsesMark, outMark, outName);
			break;
	}
}

	Boolean
CResultsWindow::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand)
	{
		case cmd_Clear:
		{
			TableIndexT	numRows;
			TableIndexT	numColumns;
			
			do
			{
				mResultsTable->GetTableSize( numRows, numColumns );
				if( numRows > 0 )
				{
					mResultsTable->RemoveRows( 1, 1, FALSE );
				}
			} while( numRows != 0 );
			
			mResultsTable->Refresh();
			
			break;
		}
			
		default:
			cmdHandled = inherited::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

