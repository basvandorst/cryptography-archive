/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CKeyTable.cp,v 1.17.8.1 1997/12/05 22:13:38 mhw Exp $
____________________________________________________________________________*/

#include <LTableMonoGeometry.h>
#include <LTableSingleSelector.h>
#include <LTableMultiSelector.h>
#include <LTableArrayStorage.h>
#include <UGAColorRamp.h>
#include <UGraphicsUtilities.h>

#include "pgpUtilities.h"
#include "MacStrings.h"

#include "CString.h"

#include "PGPadminGlobals.h"
#include "CKeyTable.h"


const SInt16	kUserIDColumnWidth		=	186;
const SInt16	kCreationColumnLeft		=	192;
const SInt16	kCreationColumnWidth	=	65;
const SInt16	kSizeColumnLeft			=	266;
const SInt16	kSizeColumnWidth		=	75;
const SInt16	kIconWidth				=	16;
const SInt16	kIconHeight				=	12;
const SInt16	kIconIndent				=	2;
const SInt16	kUserIDIndent			=	4;
const SInt16	kVerticalIndent			=	3;

const ResIDT	Txtr_Geneva9			=	130;

const ResIDT	icmx_DHKey				=	1000;
const ResIDT	icmx_RSAKey				=	1001;


extern	PGPContextRef	gPGPContext;



class CKeyIDComparator	:	public LComparator {
	virtual Int32		Compare(
								const void*			inItemOne,
								const void* 		inItemTwo,
								Uint32				inSizeOne,
								Uint32				inSizeTwo) const;
};


CKeyTable::CKeyTable(
	LStream *	inStream)
		: LTableView(inStream), mFilterRef(kPGPInvalidRef),
			mKeyIterRef(kPGPInvalidRef), mKeyListRef(kPGPInvalidRef),
			mDisplayList(false), mSelectAllEnabled(false)
{
	FontInfo	fontInfo;
	SPoint32	scrollUnit;
	
	UTextTraits::SetPortTextTraits(Txtr_Geneva9);
	::GetFontInfo(&fontInfo);
	scrollUnit.v = fontInfo.ascent + fontInfo.leading + fontInfo.descent
					+ (kVerticalIndent * 2);
	scrollUnit.h = 1;
	SetTableGeometry(new LTableMonoGeometry(	this,
												mFrameSize.width,
												scrollUnit.v));
	SetTableStorage(new LTableArrayStorage(	this,
											sizeof(PGPKeyRef)));
	InsertCols(1, 0);
	SetScrollUnit(scrollUnit);
	mCustomHilite = true;
}



CKeyTable::~CKeyTable()
{
	if (mFilterRef != kPGPInvalidRef) {
		PGPFreeFilter(mFilterRef);
	}
	FreeAllStoredKeys();
}



	void
CKeyTable::SetTableInfo(
	PGPFilterRef	inFilter,
	Boolean			inMultiSelector)
{
	if (mFilterRef != kPGPInvalidRef) {
		PGPFreeFilter(mFilterRef);
	}
	mFilterRef = inFilter;
	
	delete mTableSelector;
	if (inMultiSelector) {
		SetTableSelector(new LTableMultiSelector(this));
		mSelectAllEnabled = true;
	} else {
		SetTableSelector(new LTableSingleSelector(this));
	}
}



	void
CKeyTable::SetTableInfo(
	PGPKeySetRef	inKeySet)
{
	PGPError	pgpErr;
	
	// Remove all current rows
	if (mFilterRef != kPGPInvalidRef) {
		PGPFreeFilter(mFilterRef);
		mFilterRef = kPGPInvalidRef;
	}
	FreeAllStoredKeys();
	RemoveAllRows(false);
	
	delete mTableSelector;
	SetTableSelector(nil);
	
	pgpErr = PGPOrderKeySet(	inKeySet,
								kPGPUserIDOrdering,
								&mKeyListRef);
	PGPThrowIfPGPErr_(pgpErr);
	pgpErr = PGPNewKeyIter(	mKeyListRef,
							&mKeyIterRef);
	PGPThrowIfPGPErr_(pgpErr);

	PGPKeyRef		key;

	while ((PGPKeyIterNext(mKeyIterRef, &key) == kPGPError_NoErr)
	&& (key != kPGPInvalidRef)) {
		// Add the keyref to the table
		InsertRows(	1,
					LArray::index_Last,
					&key);					
	}

	mDisplayList = true;
}




	void
CKeyTable::GetSelectedKeys(
	Handle outHandle)
{
	STableCell		theCell(0, 0);
	PGPKeyRef		key;
	UInt32			len = sizeof(key);
	PGPError		pgpErr;
	PGPKeySetRef	singleKeySetRef = kPGPInvalidRef;
	UInt32			keyBufferLen;
	void *			keyBuffer = nil;
	
	try {
		::SetHandleSize(outHandle, 0);
		while (GetNextSelectedCell(theCell)) {
			GetCellData(	theCell,
							&key,
							len);
			pgpErr = PGPNewSingletonKeySet(key, &singleKeySetRef);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPExportKeySet(singleKeySetRef,
						PGPOAllocatedOutputBuffer(	gPGPContext,
													&keyBuffer,
													::MaxBlock(),
													&keyBufferLen),
						PGPOLastOption(gPGPContext));
			PGPThrowIfPGPErr_(pgpErr);
			::PtrAndHand(keyBuffer, outHandle, keyBufferLen);
			PGPThrowIfMemError_();
			PGPFreeKeySet(singleKeySetRef);
			singleKeySetRef = kPGPInvalidRef;
			PGPFreeData(keyBuffer);
			keyBuffer = nil;
		}
	}
	
	catch (...) {
		if (singleKeySetRef != kPGPInvalidRef) {
			PGPFreeKeySet(singleKeySetRef);
		}
		if (keyBuffer != nil) {
			PGPFreeData(keyBuffer);
		}
		throw;
	}
}



	void
CKeyTable::GetSelectedKeys(
	PGPKeySetRef *	outKeys)
{
	STableCell		theCell(0, 0);
	PGPKeyRef		key;
	UInt32			len = sizeof(key);
	PGPError		pgpErr;
	PGPKeySetRef	singleKeySet = kPGPInvalidRef;

	try {
		pgpErr = PGPNewKeySet(PGPGetKeyListContext(mKeyListRef), outKeys);
		while (GetNextSelectedCell(theCell)) {
			GetCellData(	theCell,
							&key,
							len);
			pgpErr = PGPNewSingletonKeySet(key, &singleKeySet);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPAddKeys(	singleKeySet,
									*outKeys);
			PGPThrowIfPGPErr_(pgpErr);
			PGPFreeKeySet(singleKeySet);
			singleKeySet = kPGPInvalidRef;
		}
	}

	catch (...) {
		if (singleKeySet != kPGPInvalidRef) {
			PGPFreeKeySet(singleKeySet);
		}
		throw;
	}
}



	void
CKeyTable::GetFirstSelectedKeyID(
	PGPKeyID *		outKeyID,
	SInt32 *		outAlgorithm)
{
	STableCell		theCell(0, 0);
	PGPKeyRef		key;
	UInt32			len = sizeof(key);
	PGPError		pgpErr;

	try {
		if (GetNextSelectedCell(theCell)) {
			GetCellData(	theCell,
							&key,
							len);
			pgpErr = PGPGetKeyIDFromKey(key, outKeyID);
			PGPThrowIfPGPErr_(pgpErr);
			if (outAlgorithm != nil) {
				pgpErr = PGPGetKeyNumber(	key,
											kPGPKeyPropAlgID,
											(PGPInt32 *) outAlgorithm);
				PGPThrowIfPGPErr_(pgpErr);
			}
		} else {
		}
	}
	
	catch (...) {
		throw;
	}
}



	void
CKeyTable::GetFirstSelectedUserID(
	CString &	outUserID)
{
	STableCell		theCell(0, 0);
	PGPKeyRef		key;
	UInt32			len = sizeof(key);
	PGPError		pgpErr;

	if (GetNextSelectedCell(theCell)) {
		GetCellData(	theCell,
						&key,
						len);
		pgpErr = PGPGetPrimaryUserIDNameBuffer(key,
			outUserID.GetBufferSize(), outUserID, &len);
		if (pgpErr != kPGPError_BufferTooSmall) {
			PGPThrowIfPGPErr_(pgpErr);
		}
	}
}




	void
CKeyTable::ShowSelf()
{
	if (! mDisplayList) {
		STableCell		theCell(0, 0);
		PGPKeyID		keyIDRef;
		PGPKeyRef		key;
		PGPKeySetRef	keySet = kPGPInvalidRef;
		PGPKeySetRef	filteredSet = kPGPInvalidRef;
		PGPError		pgpErr;
		LArray			selectedKeys(	sizeof(PGPKeyID),
										new CKeyIDComparator,
										true);
		UInt32			len = sizeof(key);
		
		// Save the currently selected keys
		while (GetNextSelectedCell(theCell)) {
			GetCellData(	theCell,
							&key,
							len);
			pgpErr = PGPGetKeyIDFromKey(key, &keyIDRef);
			PGPThrowIfPGPErr_(pgpErr);
			selectedKeys.InsertItemsAt(	1,
										LArray::index_Last,
										&keyIDRef);
		}
		
		// Remove all current rows
		FreeAllStoredKeys();
		RemoveAllRows(false);
		
		try {
			// Get new list of keys
			pgpErr = PGPOpenDefaultKeyRings(	gPGPContext,
												0,
												&keySet);
			PGPThrowIfPGPErr_(pgpErr);
			if (mFilterRef != kPGPInvalidRef) {
				pgpErr = PGPFilterKeySet(	keySet,
											mFilterRef,
											&filteredSet);
				PGPThrowIfPGPErr_(pgpErr);
				pgpErr = PGPOrderKeySet(	filteredSet,
											kPGPUserIDOrdering,
											&mKeyListRef);
			} else {
				pgpErr = PGPOrderKeySet(	keySet,
											kPGPUserIDOrdering,
											&mKeyListRef);
			}		
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPNewKeyIter(	mKeyListRef,
									&mKeyIterRef);
			PGPThrowIfPGPErr_(pgpErr);
			while ((PGPKeyIterNext(mKeyIterRef, &key) == kPGPError_NoErr)
			&& (key != kPGPInvalidRef)) {
				// Add the keyref to the table
				InsertRows(	1,
							LArray::index_Last,
							&key);
							
				// Check to see if we need to select the cell
				pgpErr = PGPGetKeyIDFromKey(key, &keyIDRef);
				PGPThrowIfPGPErr_(pgpErr);
				if (selectedKeys.FetchIndexOf(&keyIDRef)
				!= LArray::index_Bad) {
					STableCell	selectedCell;
					
					GetTableSize(selectedCell.row, selectedCell.col);
					SelectCell(selectedCell);
				}
			}
		}
		
		catch (...) {
		}
		
		// Free the key ids
		LArrayIterator	iterator(selectedKeys);
		
		if (filteredSet != kPGPInvalidRef) {
			PGPFreeKeySet(filteredSet);
		}
		if (keySet != kPGPInvalidRef) {
			PGPFreeKeySet(keySet);
		}
	}
	
	LTableView::Show();
	
	SwitchTarget( this );
}



	void
CKeyTable::ClickSelf(
	const SMouseDownEvent &	inMouseDown)
{
	LTableView::ClickSelf(inMouseDown);
	BroadcastMessage(GetPaneID());
}



	void
CKeyTable::DrawSelf()
{
	Rect	frame;
	
	::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
	::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
	CalcLocalFrameRect(frame);
	{
		StDeviceLoop	devLoop(frame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth)) {
			if(depth >= 8) {
				::PaintRect(&frame);
			} else {
				::EraseRect(&frame);
			}
		}
	}
	LTableView::DrawSelf();
}



	void
CKeyTable::DrawCell(
	const STableCell &	inCell,
	const Rect &		inLocalRect)
{
	Rect			theRect = inLocalRect;
	StDeviceLoop	devLoop(inLocalRect);
	SInt16			depth;
	
	while (devLoop.NextDepth(depth)) {
		if (depth >= 8) {
			theRect.right = theRect.left + kUserIDColumnWidth;
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
			::PaintRect(&theRect);
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
			theRect.left += kUserIDColumnWidth;
			theRect.right = inLocalRect.right;
			::PaintRect(&theRect);
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_White));
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 1);
			::LineTo(inLocalRect.right - 1, inLocalRect.bottom - 1);
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
		} else {
			::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
			::EraseRect(&inLocalRect);
		}
		
		// Draw the contents
		PGPError		pgpErr;
		PGPKeyRef		key;
		PGPInt32		algorithm;
		SInt16			iconTop = inLocalRect.top + (GetRowHeight(inCell.row)
									- kIconHeight) / 2;
		SInt16			iconRight = inLocalRect.left + kIconIndent
										+ kIconWidth;
		Rect			drawRect = {iconTop, inLocalRect.left + kIconIndent,
									iconTop + kIconHeight, iconRight};
		SInt16			just;
		Str255			string;
		size_t			len;
		PGPTime			creation;
		PGPInt32		bits;
		PGPSubKeyRef	subKey;		
	
		len = sizeof(key);
		GetCellData(	inCell,
						&key,
						len);
		just = UTextTraits::SetPortTextTraits(Txtr_Geneva9);
		if (depth >= 8) {
			::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
		}
		
		// Plot the icon
		pgpErr = PGPGetKeyNumber(	key,
									kPGPKeyPropAlgID,
									&algorithm);
		PGPThrowIfPGPErr_(pgpErr);
		::PlotIconID(	&drawRect,
						atNone,
						ttNone,
						(algorithm == kPGPPublicKeyAlgorithm_DSA) ?	
								icmx_DHKey : icmx_RSAKey);
								
		// Draw the UserID
		::SetRect(	&drawRect,
					iconRight + kUserIDIndent,
					inLocalRect.top + kVerticalIndent,
					kUserIDColumnWidth,
					inLocalRect.bottom - kVerticalIndent);
		len = sizeof(string) - 1;
		pgpErr = PGPGetPrimaryUserIDNameBuffer(key,
			sizeof( string ) - 1, (char *)&string[1], &len);
		PGPThrowIfPGPErr_(pgpErr);
		string[0] = len;
		::TruncString(	drawRect.right - drawRect.left,
						string,
						truncMiddle);
		UTextDrawing::DrawWithJustification(	(Ptr) &string[1],
												string[0],
												drawRect,
												just);
								
		// Adjust the background color for last two columns
		if (depth >= 8) {
			::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
		}
												
		// Draw the creation date
		pgpErr = PGPGetKeyTime(	key,
								kPGPKeyPropCreation,
								&creation);
		PGPThrowIfPGPErr_(pgpErr);
		creation = PGPTimeToMacTime(creation);
		::DateString(creation, shortDate, string, nil);
		::SetRect(	&drawRect,
					kCreationColumnLeft,
					inLocalRect.top + kVerticalIndent,
					kCreationColumnLeft + kCreationColumnWidth,
					inLocalRect.bottom - kVerticalIndent);
		UTextDrawing::DrawWithJustification(	(Ptr) &string[1],
												string[0],
												drawRect,
												just);

		// Draw the size
		pgpErr = PGPGetKeyNumber(	key,
									kPGPKeyPropBits,
									&bits);
		PGPThrowIfPGPErr_(pgpErr);
		::NumToString(bits, string);
		if (algorithm == kPGPPublicKeyAlgorithm_DSA) {
			PGPKeyIterSeek(mKeyIterRef, key);
			pgpErr = PGPKeyIterNextSubKey(mKeyIterRef, &subKey);
			if (IsntPGPError(pgpErr) && PGPRefIsValid(subKey)) {
				Str31	string2;
				
				pgpErr = PGPGetSubKeyNumber(	subKey,
												kPGPKeyPropBits,
												&bits);
				PGPThrowIfPGPErr_(pgpErr);
				::NumToString(bits, string2);
				AppendPString(	CString(	STRx_AdminStrings,
											kKeySizeSeparatorID),
								string2);
				AppendPString(string, string2);
				CopyPString(string2, string);
			}
		}
		::SetRect(	&drawRect,
					kSizeColumnLeft,
					inLocalRect.top + kVerticalIndent,
					kSizeColumnLeft + kSizeColumnWidth,
					inLocalRect.bottom - kVerticalIndent);
		UTextDrawing::DrawWithJustification(	(Ptr) &string[1],
												string[0],
												drawRect,
												just);
	}
}



	void
CKeyTable::HiliteCellActively(
	const STableCell	&inCell,
	Boolean				/*inHilite*/)
{
	Rect	cellFrame;
	
	if(GetLocalCellRect(inCell, cellFrame) && FocusExposed()) {
		StDeviceLoop	devLoop(cellFrame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth)) {
			StColorPenState saveColorPen;   // Preserve color & pen state
			
			StColorPenState::Normalize();
			if(depth >= 8) {
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			} else {
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			cellFrame.right = cellFrame.left + kUserIDColumnWidth;
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
	
	if(GetLocalCellRect(inCell, cellFrame) && FocusExposed()) {
		StDeviceLoop	devLoop(cellFrame);		
		SInt16			depth;

		while(devLoop.NextDepth(depth)) {
	        StColorPenState saveColorPen;   // Preserve color & pen state
	        
	        StColorPenState::Normalize();
	        if(depth >= 8) {
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray2));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			} else {
				::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_White));
				::RGBForeColor(&UGAColorRamp::GetColor(colorRamp_Black));
			}
			::PenMode(srcXor);
			cellFrame.right = cellFrame.left + kUserIDColumnWidth;
			cellFrame.bottom -= 1;
			UDrawingUtils::SetHiliteModeOn();
			::FrameRect(&cellFrame);
		}
	}
}



	void
CKeyTable::FreeAllStoredKeys()
{
	STableCell	theCell(0, 0);
	PGPKeyRef	key;
	UInt32		len = sizeof(key);
	
	while (GetNextCell(theCell)) {
		GetCellData(	theCell,
						&key,
						len);
	}
	if (mKeyIterRef != kPGPInvalidRef) {
		PGPFreeKeyIter(mKeyIterRef);
		mKeyIterRef = kPGPInvalidRef;
	}
	if (mKeyListRef != kPGPInvalidRef) {
		PGPFreeKeyList(mKeyListRef);
		mKeyListRef = kPGPInvalidRef;
	}
}

	void
CKeyTable::FindCommandStatus(
	CommandT			inCommand,
	Boolean				&outEnabled,
	Boolean				&outUsesMark,
	Char16				&outMark,
	Str255				outName)
{
	switch (inCommand) {
		case cmd_SelectAll:
		{
			outEnabled = mSelectAllEnabled;
		}
		break;
		
		default:
		{
			LCommander::FindCommandStatus(inCommand, outEnabled, outUsesMark,
					outMark, outName);
		}
		break;
	}
}



	Boolean
CKeyTable::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand) {
		case cmd_SelectAll:
		{
			SelectAllCells();
		}
		break;
		
		default:
		{
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
		}
		break;
	}
	
	return cmdHandled;
}



	Boolean
CKeyTable::HandleKeyPress(const EventRecord& inKeyEvent)
{
	Boolean		keyHandled	= TRUE;
	if( inKeyEvent.modifiers & cmdKey )
	{
		keyHandled = LCommander::HandleKeyPress(inKeyEvent);
	}
	else
	{
		Int16		theKey = inKeyEvent.message & charCodeMask;
		STableCell	cell;
		TableIndexT	rows, cols;
		Boolean		changedSelection = FALSE;

		switch(theKey)
		{
			case char_UpArrow:
			{
				cell = GetFirstSelectedCell();
				cell.col = 1;
				if(cell.row > 1)
					cell.row--;
				if(cell.row < 1)
					cell.row = 1;
				UnselectAllCells();
				ScrollCellIntoFrame(cell);
				SelectCell(cell);
				changedSelection = TRUE;
				break;
			}
			
			case char_DownArrow:
			{
				GetTableSize(rows, cols);
				cell = GetFirstSelectedCell();
				cell.col = 1;
				cell.row++;
				if(cell.row > rows)
					cell.row = rows;
				UnselectAllCells();
				ScrollCellIntoFrame(cell);
				SelectCell(cell);
				changedSelection = TRUE;
				break;
			}
				
			case char_LeftArrow:
			case char_RightArrow:
				changedSelection = FALSE;
				break;
				
			case char_Home:
			{
				cell.row = cell.col = 1;
				ScrollCellIntoFrame(cell);
				break;
			}
			
			case char_End:
			{
				GetTableSize(rows, cols);
				cell.col = 1;
				cell.row = rows;
				ScrollCellIntoFrame(cell);
				break;
			}
			
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
			
			default:
				keyHandled = LCommander::HandleKeyPress(inKeyEvent);
				break;
		}
		
		if( changedSelection )
		{
			BroadcastMessage( GetPaneID() );
		}
	}
	
	return keyHandled;
}

	Int32
CKeyIDComparator::Compare(
	const void *	inItemOne,
	const void * 	inItemTwo,
	UInt32			inSizeOne,
	UInt32			inSizeTwo) const
{
	return PGPCompareKeyIDs(	(PGPKeyID const *) inItemOne,
								(PGPKeyID const *) inItemTwo);
}
