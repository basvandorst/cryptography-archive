/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: PGPSelectiveImportDialog.cp,v 1.15.8.1 1997/12/05 22:13:34 mhw Exp $
____________________________________________________________________________*/

#include <LGAPushButton.h>
#include <LTableMonoGeometry.h>
#include <LTableArrayStorage.h>
#include <LTableMultiSelector.h>
#include <UGAColorRamp.h>

#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpKeys.h"
#include "pgpUtilities.h"
#include "pgpUserInterface.h"
#include "PGPOpenPrefs.h"
#include "pgpClientPrefs.h"

#include "pflPrefTypes.h"
#include "pflContext.h"

#include "MacErrors.h"
#include "MacCursors.h"
#include "MacQuickdraw.h"
#include "MacStrings.h"

#include "UPGPException.h"
#include "StSaveHeapZone.h"
#include "StSaveCurResFile.h"
#include "CString.h"

#include "PGPUILibUtils.h"
#include "PGPUILibDialogs.h"

#include "PGPSelectiveImportDialog.h"


const ResIDT	Txtr_Geneva9				=	4748;

const ResIDT	ppob_SelectiveImportID		=	4753;

const PaneIDT	button_Import				=	1001;
const PaneIDT	button_Cancel				=	1002;
const PaneIDT	caption_Prompt				=	1003;
const PaneIDT	table_UserIDs				=	1004;
const PaneIDT	button_SelectAll			=	1005;
const PaneIDT	button_SelectNone			=	1006;

const MessageT	msg_UserIDs					=	table_UserIDs;
const MessageT	msg_SelectAll				=	button_SelectAll;
const MessageT	msg_SelectNone				=	button_SelectNone;

const SInt16	kUserIDColumnWidth			=	232;
const SInt16	kValidityColumnLeft			=	241;
const SInt16	kValidityColumnWidth		=	50;
const SInt16	kValidityColumnIconIndent	=	14;
const SInt16	kCreationColumnLeft			=	310;
const SInt16	kCreationColumnWidth		=	65;
const SInt16	kSizeColumnLeft				=	384;
const SInt16	kSizeColumnWidth			=	75;
const SInt16	kIconWidth					=	16;
const SInt16	kIconHeight					=	12;
const SInt16	kIconIndent					=	2;
const SInt16	kUserIDIndent				=	4;
const SInt16	kVerticalIndent				=	3;

const ResIDT	icmx_ValidKey	 			=	4750;
const ResIDT	icmx_InvalidKey 			=	4751;
const ResIDT	icmx_AxiomaticKey			=	4752;
const ResIDT	icmx_LockedKey				=	4753;

const ResIDT	acur_BeachBall				=	4747;

const ResIDT	ppat_BarberPattern	 		=	4747;

const SInt16	kBarHeight					=	10;
const SInt16	kMaxValidity				=	3;

const ResIDT	icmx_DHKey					=	4754;
const ResIDT	icmx_RSAKey					=	4755;

const ResIDT	icmx_RevokedDHKey			=	4758;
const ResIDT	icmx_RevokedRSAKey			=	4761;

const ResIDT	icmx_SecretDHKey			=	4759;
const ResIDT	icmx_SecretRSAKey			=	4762;

const ResIDT	icmx_ExpiredDHKey			=	4757;
const ResIDT	icmx_ExpiredRSAKey			=	4760;


	PGPError
PGPSelectiveImportDialog(
	PGPContextRef	context,
	const char *	prompt,
	PGPKeySetRef	fromSet,							
	PGPKeySetRef	validitySet,
	PGPKeySetRef	*selectedSet)					
{
	PGPError	result = kPGPError_NoErr;
	
	PGPValidatePtr( selectedSet );
	*selectedSet	= kInvalidPGPKeySetRef;
	PGPValidateParam( IsntNull( context ) );
	PGPValidateParam( IsntNull( fromSet ) );
	PGPValidateParam( IsntNull( validitySet ) );
	
	try {
		StSaveHeapZone				savedZone(::SystemZone());
		StSaveCurResFile			saveResFile;
		LFile						thePlugFile;
		FSSpec						theLibrarySpec;
		CSelectiveImportDialog *	theDialog = nil;
		OSStatus					err;
		
		// Open our resource fork
		err = PGPGetUILibFSSpec(&theLibrarySpec);
		PGPThrowIfOSErr_(err);
		thePlugFile.SetSpecifier(theLibrarySpec);
		thePlugFile.OpenResourceFork(fsRdPerm);
		
		// Run our model dialog
		InitializePowerPlant();
		theDialog = (CSelectiveImportDialog *)
					CModalDialogGrafPortView::CreateCModalDialogGrafPortView(
							ppob_SelectiveImportID,
							nil);
		try {
			theDialog->Init(	context,
								prompt,
								fromSet,
								validitySet,
								selectedSet);
			theDialog->Show();
			theDialog->Run();
			delete theDialog;
		}
		
		catch (...) {
			delete theDialog;
			throw;
		}
		
		thePlugFile.CloseResourceFork();
	}
	
	
	catch (CComboError & comboError) {
		if (comboError.HaveNonPGPError()) {
			result = MacErrorToPGPError(comboError.err);
		} else {
			result = comboError.pgpErr;
		}
	}
	
	catch (...) {
		result = kPGPError_UnknownError;
	}
	
	return result;
}



CSelectiveImportDialog::CSelectiveImportDialog(
	LStream *	inStream)
		: CModalDialogGrafPortView(inStream)
{
}



CSelectiveImportDialog::~CSelectiveImportDialog()
{
}



	void
CSelectiveImportDialog::Init(
	PGPContextRef	inContext,
	const char *	inPrompt,
	PGPKeySetRef	inFromSet,
	PGPKeySetRef	inValiditySet,
	PGPKeySetRef *	outSelectedSet)
{
	if (inPrompt != nil) {
		FindPaneByID(caption_Prompt)->SetDescriptor(CString(inPrompt));
	}
	
	mUserIDTable->SetTableInfo(	inContext,
							inFromSet,
							inValiditySet,
							outSelectedSet);
	mUserIDTable->SelectAllCells();
	AdjustButtons();
}



	void
CSelectiveImportDialog::FinishCreateSelf()
{
	LGAPushButton *	button = (LGAPushButton *) FindPaneByID(button_Import);
	
	button->SetDefaultButton(true);
	button->AddListener(this);
	
	button = (LGAPushButton *) FindPaneByID(button_Cancel);
	button->AddListener(this);

	button = (LGAPushButton *) FindPaneByID(button_SelectAll);
	button->AddListener(this);

	button = (LGAPushButton *) FindPaneByID(button_SelectNone);
	button->AddListener(this);
	
	mUserIDTable = (CSelectiveImportTable *) FindPaneByID(table_UserIDs);
	pgpAssert( IsntNull( mUserIDTable ) );
	
	mUserIDTable->AddListener(this);
}



	Boolean
CSelectiveImportDialog::HandleKeyPress(
	const EventRecord &	inKeyEvent)
{
	StColorPortState	theSavePort(UQDGlobals::GetCurrentPort());
	Boolean				keyHandled = false;
	LControl *			keyButton = nil;
	
	OutOfFocus(nil);
	switch (inKeyEvent.message & charCodeMask) {
		case char_Enter:
		case char_Return:
		{
			keyButton = (LControl *) FindPaneByID(button_Import);
		}
		break;
		
			
		case char_Escape:
		{
			if ((inKeyEvent.message & keyCodeMask) == vkey_Escape) {
				keyButton =  (LControl*) FindPaneByID(button_Cancel);
			}
		}
		break;


		default:
		{
			if (UKeyFilters::IsCmdPeriod(inKeyEvent)) {
				keyButton =  (LControl *) FindPaneByID(button_Cancel);
			} else {
				keyHandled = CModalDialogGrafPortView::HandleKeyPress(
									inKeyEvent);
			}
		}
		break;
	}
			
	if (keyButton != nil) {
		keyButton->SimulateHotSpotClick(kControlButtonPart);
		keyHandled = true;
	}
	
	return keyHandled;
}

	void
CSelectiveImportDialog::AdjustButtons(void)
{
	TableIndexT	numRows;
	TableIndexT	numColumns;
	STableCell	theCell(0, 0);
	TableIndexT	numSelectedRows = 0;
	
	mUserIDTable->GetTableSize( numRows, numColumns );
	
	while( mUserIDTable->GetNextSelectedCell( theCell ) )
		++numSelectedRows;
		
	if ( numSelectedRows == 0 )
	{
		FindPaneByID(button_Import)->Disable();
		FindPaneByID(button_SelectNone)->Disable();
	}
	else
	{
		FindPaneByID(button_Import)->Enable();
		FindPaneByID(button_SelectNone)->Enable();
	}
	
	if ( numSelectedRows == numRows )
	{
		FindPaneByID(button_SelectAll)->Disable();
	}
	else
	{
		FindPaneByID(button_SelectAll)->Enable();
	}
}


	void
CSelectiveImportDialog::ListenToMessage(
	MessageT	inMessage,
	void *		ioParam)
{
	(void) ioParam;
	
	switch (inMessage) {
		case msg_OK:
		{
			mUserIDTable->ImportKeys();
			mExitDialog = true;
		}
		break;
		
		
		case msg_Cancel:
		{
			PGPThrowPGPErr_(kPGPError_UserAbort);
		}
		break;
		
		
		case msg_UserIDs:
		{
			AdjustButtons();
		}
		break;
		
		
		case msg_SelectAll:
		{
			mUserIDTable->SelectAllCells();
			AdjustButtons();
		}
		break;

		case msg_SelectNone:
		{
			mUserIDTable->UnselectAllCells();
			AdjustButtons();
		}
		break;
	}
}



CSelectiveImportTable::CSelectiveImportTable(
	LStream *	inStream)
		: LTableView(inStream),
		mFromSet(kInvalidPGPKeySetRef),
		mBarberPixPat(nil),
		mKeyIter(kInvalidPGPKeyIterRef),
		mSelectedSet(nil)
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
	SetTableSelector(new LTableMultiSelector(this));
	InsertCols(1, 0);
	SetScrollUnit(scrollUnit);
	mCustomHilite = true;
}



CSelectiveImportTable::~CSelectiveImportTable()
{
	if (mKeyIter != kInvalidPGPKeyIterRef) {
		PGPFreeKeyIter(mKeyIter);
	}
	if (mBarberPixPat != nil) {
		::DisposePixPat(mBarberPixPat);
	}
}
	


	void
CSelectiveImportTable::SetTableInfo(
	PGPContextRef	inContext,
	PGPKeySetRef	inFromSet,
	PGPKeySetRef	inValiditySet,
	PGPKeySetRef *	outSelectedSet)
{
	PGPError			pgpErr;
	OSStatus			err;
	PGPKeyListRef		keyList = kInvalidPGPKeyListRef;
	PGPKeyRef			key;
	PGPKeySetRef		combinedSet = kInvalidPGPKeySetRef;
	AnimatedCursorRef	cursorRef = nil;
	PFLContextRef		pflContext = kInvalidPFLContextRef;
	PGPPrefRef			prefRef = kInvalidPGPPrefRef;
	PGPUInt32			numKeys;

	try {
		// Make sure that there are some keys to show
		pgpErr = PGPCountKeys(inFromSet, &numKeys);
		PGPThrowIfPGPErr_(pgpErr);
		if (numKeys == 0) {
			PGPThrowPGPErr_(kPGPError_BadParams);
		}
		
		// Save off our sets
		mSelectedSet = outSelectedSet;
		mFromSet = inFromSet;
		
		// Get the marginal validity pref
		pgpErr = PFLNewContext(&pflContext);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPOpenClientPrefs(	pflContext,
										&prefRef);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPGetPrefBoolean(	prefRef,
									kPGPPrefDisplayMarginalValidity,
									&mShowMarginalValidity);
		PGPThrowIfPGPErr_(pgpErr);

		pgpErr = PGPGetPrefBoolean(	prefRef,
									kPGPPrefMarginalIsInvalid,
									&mMarginalIsInvalid);
		PGPThrowIfPGPErr_(pgpErr);

		PGPClosePrefFile(prefRef);
		PFLFreeContext(pflContext);
		prefRef = kInvalidPGPPrefRef;
		pflContext = kInvalidPFLContextRef;
		
		// Get the pixpat
		mBarberPixPat = ::GetPixPat(ppat_BarberPattern);
	
		// Add keys to table
		pgpErr = PGPOrderKeySet(	inFromSet,
									kPGPUserIDOrdering,
									&keyList);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPNewKeyIter(	keyList,
								&mKeyIter);
		PGPThrowIfPGPErr_(pgpErr);
		while ((PGPKeyIterNext(mKeyIter, &key) == kPGPError_NoErr)
		&& (key != kInvalidPGPKeyRef)) {
			InsertRows(	1,
						LArray::index_Last,
						&key);
		}
		PGPFreeKeyList(keyList);
		keyList = kInvalidPGPKeyListRef;
		
		// Propagate validity
		err = Get1AnimatedCursor(acur_BeachBall, &cursorRef);
		PGPThrowIfOSErr_(err);
		pgpErr = PGPCheckKeyRingSigs(	inFromSet,
												inValiditySet,
												true,
												EventHandler,
												cursorRef);
		PGPThrowIfPGPErr_(pgpErr);
		DisposeAnimatedCursor(cursorRef);
		cursorRef = nil;
		::InitCursor();
		pgpErr = PGPNewKeySet(	inContext,
								&combinedSet);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPAddKeys(	inFromSet,
								combinedSet);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPAddKeys(	inValiditySet,
								combinedSet);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPPropagateTrust(combinedSet);
		PGPThrowIfPGPErr_(pgpErr);
		PGPFreeKeySet(combinedSet);
		combinedSet = kInvalidPGPKeySetRef;
	}
	
	catch (...) {
		if (prefRef != kInvalidPGPPrefRef) {
			PGPClosePrefFile(prefRef);
		}
		if (pflContext != kInvalidPFLContextRef) {
			PFLFreeContext(pflContext);
		}
		if (keyList != kInvalidPGPKeyListRef) {
			PGPFreeKeyList(keyList);
		}
		if (cursorRef != nil) {
			DisposeAnimatedCursor(cursorRef);
		}
		if (combinedSet != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(combinedSet);
		}
		throw;
	}
}



	void
CSelectiveImportTable::ImportKeys()
{
	STableCell		theCell(0, 0);
	PGPKeyRef		key;
	UInt32			len = sizeof(key);
	PGPError		pgpErr;
	PGPKeySetRef	singleKeySet = kInvalidPGPKeySetRef;
	PGPKeySetRef	tempSet;

	try {
		pgpErr = PGPNewEmptyKeySet(mFromSet, mSelectedSet);
		PGPThrowIfPGPErr_(pgpErr);
		while (GetNextSelectedCell(theCell)) {
			tempSet = *mSelectedSet;
			GetCellData(	theCell,
							&key,
							len);
			pgpErr = PGPNewSingletonKeySet(key, &singleKeySet);
			PGPThrowIfPGPErr_(pgpErr);
			pgpErr = PGPUnionKeySets(	tempSet,
										singleKeySet,
										mSelectedSet);
			PGPThrowIfPGPErr_(pgpErr);
			PGPFreeKeySet(singleKeySet);
			PGPFreeKeySet(tempSet);
			singleKeySet = kInvalidPGPKeySetRef;
		}
	}
	
	catch (...) {
		if (singleKeySet != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(singleKeySet);
		}
		if (*mSelectedSet != kInvalidPGPKeySetRef) {
			PGPFreeKeySet(*mSelectedSet);
		}
		throw;
	}
}



	void
CSelectiveImportTable::ClickSelf(
	const SMouseDownEvent &	inMouseDown)
{
	LTableView::ClickSelf(inMouseDown);
	BroadcastMessage(GetPaneID());
}



	void
CSelectiveImportTable::DrawSelf()
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
CSelectiveImportTable::HiliteCellActively(
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
CSelectiveImportTable::HiliteCellInactively(
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
CSelectiveImportTable::DrawCell(
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
		ResID			iconID;
		PGPBoolean		secretKey;
		PGPBoolean		expiredKey;
		PGPBoolean		revokedKey;
		
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

		pgpErr = PGPGetKeyBoolean(	key,
									kPGPKeyPropIsSecret,
									&secretKey);
		PGPThrowIfPGPErr_(pgpErr);

		pgpErr = PGPGetKeyBoolean(	key,
									kPGPKeyPropIsRevoked,
									&revokedKey);
		PGPThrowIfPGPErr_(pgpErr);

		pgpErr = PGPGetKeyBoolean(	key,
									kPGPKeyPropIsExpired,
									&expiredKey);
		PGPThrowIfPGPErr_(pgpErr);
		
		if( algorithm == kPGPPublicKeyAlgorithm_DSA )
		{
			if( revokedKey )
				iconID = icmx_RevokedDHKey;
			else if( expiredKey )
				iconID = icmx_ExpiredDHKey;
			else if( secretKey )
				iconID = icmx_SecretDHKey;
			else
				iconID = icmx_DHKey;
		}
		else
		{
			if( revokedKey )
				iconID = icmx_RevokedRSAKey;
			else if( expiredKey )
				iconID = icmx_ExpiredRSAKey;
			else if( secretKey )
				iconID = icmx_SecretRSAKey;
			else
				iconID = icmx_RSAKey;
		}
		
		::PlotIconID(	&drawRect,
						atNone,
						ttNone,
						iconID);
								
		// Draw the UserID
		::SetRect(	&drawRect,
					iconRight + kUserIDIndent,
					inLocalRect.top + kVerticalIndent,
					kUserIDColumnWidth,
					inLocalRect.bottom - kVerticalIndent);
		len = sizeof(string) - 1;
		pgpErr = PGPGetPrimaryUserIDNameBuffer(key,
			sizeof( string ) - 1, (char *)&string[1], &len);
		if(pgpErr != kPGPError_BufferTooSmall)
		{
			PGPThrowIfPGPErr_(pgpErr);
		}
		string[0] = len;
		::TruncString(	drawRect.right - drawRect.left,
						string,
						truncMiddle);
		UTextDrawing::DrawWithJustification(	(Ptr) &string[1],
												string[0],
												drawRect,
												just);
								
		// Adjust the background color for last columns
		if (depth >= 8) {
			::RGBBackColor(&UGAColorRamp::GetColor(colorRamp_Gray1));
		}
		
		// Draw the validity
		RGBColor	tempColor;
		RGBColor	saveBackColor;
		Rect		validityBarRect;
		PGPBoolean	isAxiomatic;
		PGPInt32	validity;
				
		pgpErr = PGPGetKeyBoolean(	key,
									kPGPKeyPropIsAxiomatic,
									&isAxiomatic);
		PGPThrowIfPGPErr_(pgpErr);
		pgpErr = PGPGetKeyNumber(	key,
									kPGPKeyPropValidity,
									&validity);
		PGPThrowIfPGPErr_(pgpErr);
		::SetRect(	&drawRect,
					kValidityColumnLeft,
					inLocalRect.top + kVerticalIndent,
					kValidityColumnLeft + kValidityColumnWidth,
					inLocalRect.bottom - kVerticalIndent);
		::GetBackColor(&saveBackColor);
		tempColor = UGAColorRamp::GetBlackColor();
		::RGBForeColor(&tempColor);

		if (mShowMarginalValidity) {
			RGBColor	trustColor = { 52223, 52223, 65535 };
			
			::SetRect(	&validityBarRect,
						drawRect.left,
						drawRect.top,
						drawRect.right,
						drawRect.top + kBarHeight);
			AlignRect(	&drawRect,
						&validityBarRect,
						kAlignAtVerticalCenter);
			::FrameRect(&validityBarRect);
			::InsetRect(	&validityBarRect,
							1,
							1);

			if (isAxiomatic && (mBarberPixPat != nil)) {
				::PenPixPat(mBarberPixPat);
				::PaintRect(&validityBarRect);
				::PenNormal();
			} else {
				SInt16 		width;
				SInt16		barPercent;
				switch (validity) {
					default:
					case kPGPValidity_Unknown:
					case kPGPValidity_Invalid:
					{
						barPercent = 0;
					}
					break;
					
						
					case kPGPValidity_Marginal:
					{
						barPercent = 50;
					}
					break;
					
						
					case kPGPValidity_Complete:
					{
						barPercent = 100;
					}
					break;
				}
				
				if (depth >= 8) {
					::RGBForeColor(&trustColor);
					::PaintRect(&validityBarRect);
				}
				width = validityBarRect.right - validityBarRect.left;
				validityBarRect.right -= (width * (100 - barPercent)) / 100;
				tempColor = UGAColorRamp::GetColor(colorRamp_Gray9);
				::RGBForeColor(&tempColor);
				::PaintRect(&validityBarRect);
			}

		} else {
			ResIDT	iconResID;
			
			if (isAxiomatic)
			{
				iconResID = icmx_AxiomaticKey;
			}
			else if (validity == kPGPValidity_Complete)
			{
				iconResID = icmx_ValidKey;
			}
			else if (validity == kPGPValidity_Marginal)
			{
				if( mMarginalIsInvalid )
				{
					iconResID = icmx_InvalidKey;
				}
				else
				{
					iconResID = icmx_ValidKey;
				}
			}
			else
			{
				iconResID = icmx_InvalidKey;
			}
			
			::SetRect(	&validityBarRect,
						drawRect.left + kValidityColumnIconIndent,
						drawRect.top,
						drawRect.left + kIconWidth
								+ kValidityColumnIconIndent,
						drawRect.top + kIconHeight);
			AlignRect(	&drawRect,
						&validityBarRect,
						kAlignAtVerticalCenter);
			tempColor = UGAColorRamp::GetWhiteColor();
			::RGBBackColor(&tempColor);
			::PlotIconID(	&validityBarRect,
							kAlignNone,
							IsEnabled() ? kTransformNone : kTransformDisabled,
							iconResID);
		}
		tempColor = UGAColorRamp::GetBlackColor();
		::RGBForeColor(&tempColor);
		::RGBBackColor(&saveBackColor);

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
		if (algorithm == kPGPPublicKeyAlgorithm_DSA)
		{
			PGPInt32	bits2;
			Str31		string2;
			
			GetIndString( string, kPGPLibDialogsStringListResID,
					kDSSDHKeySizeFormatStrIndex );
			
			PGPKeyIterSeek(mKeyIter, key);
			pgpErr = PGPKeyIterNextSubKey(mKeyIter, &subKey);
			if (IsntPGPError(pgpErr) && PGPSubKeyRefIsValid(subKey)) {
				
				pgpErr = PGPGetSubKeyNumber(	subKey,
												kPGPKeyPropBits,
												&bits2);
				PGPThrowIfPGPErr_(pgpErr);
			}
			
			::NumToString(bits2, string2);
			PrintPString( string, string, string2 );

			::NumToString(bits, string2);
			PrintPString( string, string, string2 );
		}
		else
		{
			::NumToString(bits, string);
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



	PGPError
CSelectiveImportTable::EventHandler(
	PGPContextRef	context,
	PGPEvent *		event,
	PGPUserValue	userValue)
{
	(void) context;
	(void) event;
	
	AnimateCursor((AnimatedCursorRef) userValue);
	return kPGPError_NoErr;
}
