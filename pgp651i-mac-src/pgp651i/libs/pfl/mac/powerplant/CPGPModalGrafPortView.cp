/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		This class implements a PowerPlant hierarchy inside a ModalDialog.	Note that the modal dialog can be moveable and/or resizeable.		$Id: CPGPModalGrafPortView.cp,v 1.21 1999/04/23 17:24:48 heller Exp $____________________________________________________________________________*/#include <LowMem.h>#include <ToolUtils.h>#include <Windows.h>#include <Balloons.h>#include <ctype.h>#include <LArrayIterator.h>#include <LPeriodical.h>#include <LPushButton.h>#include <PP_KeyCodes.h>#include <PP_Messages.h>#include <UDrawingUtils.h>#include <UGAColorRamp.h>#include <UWindows.h>#include "MacEnvirons.h"#include "MacEvents.h"#include "MacStrings.h"#include "MacMenus.h"#include "CPGPModalGrafPortView.h"#include "pgpMem.h"const PaneIDT	kOKButtonPaneID				= 'bOK ';const PaneIDT	kCancelButtonPaneID			= 'bCAN';class	StUnhiliteMenu {public:						StUnhiliteMenu() { }						~StUnhiliteMenu() { ::HiliteMenu(0); }};struct EnabledItem {	MenuHandle	menu;	SInt16		item;};typedef struct SavedWindowBitsInfo{	SavedWindowBitsInfo	*next;	WindowRef			windowRef;	ushort				bitDepth;	Rect				bitsRect;	// In window coordinates	GWorldPtr			gWorld;} SavedWindowBitsInfo;static CPGPModalGrafPortView	*sViewList 				= NULL;CPGPModalGrafPortView::CPGPModalGrafPortView(LStream *inStream)		: LGrafPortView(inStream), LListener(){	mDismissMessage 			= msg_Nothing;	mInvalidWindowList			= new LArray( sizeof( WindowRef ) );	mWindowIsMoveable			= FALSE;	mWindowIsResizeable			= FALSE;	mSaveRestoreWindowBits		= FALSE;	mMinMaxSize.top				= 0;	mMinMaxSize.left			= 0;	mMinMaxSize.bottom			= 32767;	mMinMaxSize.right			= 32767;	mSavedWindowBitsList		= NULL;	mMoveOnlyUserZoom			= FALSE;	mLastIdleTicks				= 0;	mDialogError				= kPGPError_NoErr;	mEnabledMenuItems			= new LArray( sizeof( EnabledItem ) );	mEditMenu					= nil;	mCutItem					= -1;	mCopyItem					= -1;	mPasteItem					= -1;	mSelectAllItem				= -1;	mModalFilterProc			= NewModalFilterProc( ModalFilterProcHandler );		SaveAndDisableEnabledMenus();			/* Link this view into the global view list */	mNextView = sViewList;	sViewList = this;}CPGPModalGrafPortView::~CPGPModalGrafPortView(void){	CPGPModalGrafPortView	*curView;	CPGPModalGrafPortView	*prevView;		if( IsntNull( mInvalidWindowList ) )	{		WindowRef		windowRef;		GrafPtr			savePort;			LArrayIterator	iterator( *mInvalidWindowList );				GetPort( &savePort );				while( iterator.Next( &windowRef ) )		{			SetPort( (GrafPtr) GetWindowPort( windowRef ) );			InvalRect( &GetWindowPort( windowRef )->portRect );		}				SetPort( savePort );	}	delete( mInvalidWindowList );		if( IsntNull( mModalFilterProc ) )		DisposeRoutineDescriptor( mModalFilterProc );		// Fix PowerPlant bug:	DeleteAllSubPanes();		/* Remove this view from the global view list */	curView 	= sViewList;	prevView	= NULL;		while( IsntNull( curView ) )	{		if( curView == this )		{			if( IsntNull( prevView ) )			{				prevView->mNextView = mNextView;			}			else			{				sViewList = mNextView;			}						break;		}				prevView 	= curView;		curView 	= curView->mNextView;	}		if( IsntNull( mSavedWindowBitsList ) )	{		SavedWindowBitsInfo	*curInfo;				curInfo = mSavedWindowBitsList;		while( IsntNull( curInfo ) )		{			SavedWindowBitsInfo	*nextInfo = curInfo->next;						if( IsntNull( curInfo->gWorld ) )				DisposeGWorld( curInfo->gWorld );							pgpFree( curInfo );						curInfo = nextInfo;		}	}		if ( IsntNull (mEnabledMenuItems ) )	{		RestoreEnabledMenus();	}	delete ( mEnabledMenuItems );}	voidCPGPModalGrafPortView::FinishCreateSelf(void){	LGrafPortView::FinishCreateSelf();	mOKButton = (LPushButton *) FindPaneByID( kOKButtonPaneID );	if( IsntNull( mOKButton ) )	{		mOKButton->SetDefaultButton( TRUE );		mOKButton->AddListener( this );	}	mCancelButton = (LPushButton *) FindPaneByID( kCancelButtonPaneID );	if( IsntNull( mCancelButton ) )	{		mCancelButton->AddListener(this);	}		mSaveRestoreWindowBits = mWindowIsMoveable || mWindowIsResizeable;	if( mSaveRestoreWindowBits )	{		// This only works if the window is not visible at creation time.		pgpAssert( ! IsWindowVisible( GetMacPort() ) );				SaveAllWindowBits();	}	CalcPortFrameRect( mUserBounds );		PortToGlobalPoint( topLeft( mUserBounds ) );	PortToGlobalPoint( botRight(mUserBounds ) );	mStandardSize.width 	= mMinMaxSize.right;	mStandardSize.height 	= mMinMaxSize.bottom;		if( HaveAppearanceMgr() )	{		SetThemeWindowBackground( GetMacPort(),				kThemeActiveDialogBackgroundBrush, FALSE );	}}	voidCPGPModalGrafPortView::GetForeAndBackColors(	RGBColor	*outForeColor,	RGBColor	*outBackColor) const{	if( IsntNull( outForeColor ) )	{		*outForeColor = mForeColor;	}		if( IsntNull( outBackColor ) )	{		*outBackColor = mBackColor;	}}	voidCPGPModalGrafPortView::ApplyForeAndBackColors(void) const{	if( HaveAppearanceMgr() )	{		SetThemeWindowBackground( GetMacPort(),				kThemeActiveDialogBackgroundBrush, FALSE );	}	else	{		LGrafPortView::ApplyForeAndBackColors();	}}	voidCPGPModalGrafPortView::SaveWindowBits(WindowRef theWindow){	Rect				visRect;	StColorPortState	portState( theWindow );		portState.Normalize();	SetOrigin( 0, 0 );		visRect = (**theWindow->visRgn).rgnBBox;		{		StDeviceLoop	devLoop( visRect );				SInt16			depth;				while( devLoop.NextDepth( depth ) )		{			SavedWindowBitsInfo	*info;						info = (SavedWindowBitsInfo *) pgpAlloc( sizeof( *info ) );			if( IsntNull( info ) )			{				QDErr	qdErr;				Boolean	addToList = FALSE;								// Clipping region is setup for the part of this window at				// this depth. Copy away the bits.								pgpClearMemory( info, sizeof( *info ) );								info->windowRef	= theWindow;				info->bitDepth 	= depth;				info->bitsRect	= (**theWindow->clipRgn).rgnBBox;				qdErr = NewGWorld( &info->gWorld, depth, &info->bitsRect,							NULL, devLoop.GetCurrentDevice(),							pixPurge | noNewDevice | useTempMem );				if( qdErr == noErr )				{					if( LockPixels( info->gWorld->portPixMap ) )					{						CopyBits( 	&theWindow->portBits,									&((GrafPtr) info->gWorld)->portBits,									&info->bitsRect, &info->bitsRect,									srcCopy, NULL );												UnlockPixels( info->gWorld->portPixMap );												addToList = TRUE;					}				}								if( addToList )				{					info->next 				= mSavedWindowBitsList;					mSavedWindowBitsList	= info;				}				else				{					pgpFree( info );					info = NULL;				}			}		}	}}	voidCPGPModalGrafPortView::SaveAllWindowBits(void){	WindowRef	theWindow;		// Walk the window list of the current app and remember contents	// of the visible windows.	theWindow = LMGetWindowList();	while( IsntNull( theWindow ) )	{		if( IsWindowVisible( theWindow ) &&			theWindow != GetMacPort() &&			IsntNull( theWindow->visRgn ) &&			! EmptyRgn( theWindow->visRgn ) )		{			SaveWindowBits( theWindow );		}				theWindow = (WindowRef) ((WindowPeek) theWindow)->nextWindow;	}}	voidCPGPModalGrafPortView::RestoreWindowBits(WindowRef theWindow){	if( mSaveRestoreWindowBits )	{		CGrafPtr	savePort;		GDHandle	saveGDevice;		Rect		visRect;				GetGWorld( &savePort, &saveGDevice );		visRect = (**theWindow->visRgn).rgnBBox;		{			StDeviceLoop	devLoop( visRect );					SInt16			depth;						while( devLoop.NextDepth( depth ) )			{				SavedWindowBitsInfo	*info;								// Find an info record for this window at this depth								info = mSavedWindowBitsList;				while( IsntNull( info ) )				{					if( info->windowRef == theWindow &&						info->bitDepth == depth )					{						break;					}										info = info->next;				}								if( IsntNull( info ) )				{					if( LockPixels( info->gWorld->portPixMap ) )					{						CopyBits( 	&((GrafPtr) info->gWorld)->portBits,									&theWindow->portBits, 									&info->bitsRect, &info->bitsRect,									srcCopy, NULL );												UnlockPixels( info->gWorld->portPixMap );					}				}			}		}		SetGWorld( savePort, saveGDevice );	}}	voidCPGPModalGrafPortView::ListenToMessage(MessageT inMessage, void *ioParam){	#pragma unused( ioParam )		switch( inMessage )	{		case msg_OK:		case msg_Cancel:			SetDismissMessage( inMessage );			break;	}}	BooleanCPGPModalGrafPortView::HandleKeyPress(const EventRecord&	inKeyEvent){	Boolean		keyHandled = TRUE;	Int16		theKey = inKeyEvent.message & charCodeMask;		if( (inKeyEvent.modifiers & cmdKey) != 0 )	{		// Handle Cmd-. to abort the dialog.		switch( theKey )		{			case '.':			{				if( IsntNull( mCancelButton ) )				{					mCancelButton->SimulateHotSpotClick( kControlButtonPart );					SetDismissMessage( msg_Cancel );				}								break;			}							default:				keyHandled = LGrafPortView::HandleKeyPress( inKeyEvent );				break;		}	}	else	{		switch( theKey )		{			case char_Return:			case char_Enter:			{				if( IsntNull( mOKButton ) && mOKButton->IsEnabled() )				{					mOKButton->SimulateHotSpotClick( kControlButtonPart );					SetDismissMessage( msg_OK );				}				break;			}							case char_Escape:			{				if( IsntNull( mCancelButton ) )				{					mCancelButton->SimulateHotSpotClick( kControlButtonPart );					SetDismissMessage( msg_Cancel );				}				break;			}						default:				keyHandled = LGrafPortView::HandleKeyPress( inKeyEvent );				break;		}	}		return keyHandled;}	voidCPGPModalGrafPortView::GlobalToPortPoint(Point &ioPoint) const{	// This is stolen from LWindow. Apparently the CodeWarrior folks never	// tried to do dragging from a GrafPortView.		// Windows have a reference to the pixel image of the main screen--	// portBits for B&W and portPixMap for color windows. The bounds	// of the pixel image specify the alignment of the *local* Window	// coordinates with *global* coordinates:	//		localPt = globalPt + topLeft(imageBounds)	//	// To convert from *local* to *port* coordinates, we offset by	// the top left of the Window's port rectangle:	//		portPt = localPt - topLeft(portRect)	//	// Therefore,	//		portPt = globalPt + topLeft(imageBounds) - topLeft(portRect)	//	// Note: We don't use the Toolbox routine GlobalToLocal because	// that routine depends on the current port and the current port	// origin. To use GlobalToLocal we would have to do the following:	//		GrafPtr savePort;	//		GetPort(&savePort);	//		Point	saveOrigin = topLeft(mMacWindowP->portRect);	//		SetPort(mMacWindowP);	//		SetOrigin(0,0);	//		GlobalToLocal(ioPoint);	//		SetOrigin(saveOrigin.h, saveOrigin.v);	//		SetPort(savePort);	// The equation above avoids all this saving/setting/restoring	// of the current port and port origin.									// Assume a B&W Window	Point	localOffset = topLeft(mGrafPtr->portBits.bounds);	if (UDrawingUtils::IsColorGrafPort(mGrafPtr))	{									// Nope, it's a color Window		CGrafPtr	colorPortP = (CGrafPtr) mGrafPtr;		localOffset = topLeft((**(colorPortP->portPixMap)).bounds);	}		ioPoint.h += (localOffset.h - mGrafPtr->portRect.left);	ioPoint.v += (localOffset.v - mGrafPtr->portRect.top);}	voidCPGPModalGrafPortView::PortToGlobalPoint(Point &ioPoint) const{									// Assume a B&W Window	Point	localOffset = topLeft(mGrafPtr->portBits.bounds);	if (UDrawingUtils::IsColorGrafPort(mGrafPtr)) {									// Nope, it's a color Window		CGrafPtr	colorPortP = (CGrafPtr) mGrafPtr;		localOffset = topLeft((**(colorPortP->portPixMap)).bounds);	}		ioPoint.h -= (localOffset.h - mGrafPtr->portRect.left);	ioPoint.v -= (localOffset.v - mGrafPtr->portRect.top);}	voidCPGPModalGrafPortView::DoIdle(const EventRecord &inMacEvent){	UInt32	ticks = TickCount();		if( mLastIdleTicks < ticks )	{		Point	portMouse;		Rect	viewRect;				LGrafPortView::DoIdle( inMacEvent );				portMouse = inMacEvent.where;		GlobalToPortPoint( portMouse );		CalcLocalFrameRect( viewRect );				if( PtInRect( portMouse, &viewRect) )		{			AdjustCursor( portMouse, inMacEvent );		}				mLastIdleTicks = ticks;	}}	voidCPGPModalGrafPortView::RememberInvalidWindow(WindowRef theWindow){	if( IsntNull( theWindow ) && IsntNull( mInvalidWindowList ) )	{		if( mInvalidWindowList->FetchIndexOf( &theWindow ) ==					LArray::index_Bad )		{			mInvalidWindowList->InsertItemsAt( 1, max_Int32, &theWindow );		}	}}	voidCPGPModalGrafPortView::Draw(RgnHandle	inSuperDrawRgnH){	FocusDraw();	ApplyForeAndBackColors();	LGrafPortView::Draw(inSuperDrawRgnH);	if( mWindowIsResizeable )	{		Rect	sizeBox;				sizeBox 		= GetMacPort()->portRect;		sizeBox.left 	= sizeBox.right - 15;		sizeBox.top 	= sizeBox.bottom - 15;				/* Draw the window grow icon without the scroll bar borders */				StClipRgnState	saveClip( sizeBox );		DrawGrowIcon( GetMacPort() );	}}	voidCPGPModalGrafPortView::UpdatePort(void){	FocusDraw();	ApplyForeAndBackColors();		LGrafPortView::UpdatePort();}	DialogRefCPGPModalGrafPortView::GetDialog(void){	return( (DialogRef) GetUserCon() );}	DialogRefCPGPModalGrafPortView::CreateDialog(ResIDT dialogResID){	DialogRef		theDialog;	MenuHandle		appleMenu;	Boolean			appleMenuEnabled = FALSE;		FlushEvents( everyEvent, 0 );	// Take control of menu handling	appleMenu = GetIndMenuInMenuBar(0);	if( IsntNull( appleMenu ) )	{		appleMenuEnabled = IsMenuItemEnabled(appleMenu, 0);		::HiliteMenu(0);		::DisableItem( appleMenu, 0 );	}		theDialog = GetNewDialog( dialogResID, NULL, (WindowRef) -1L );	// Reenable apple menu for the moment	if (appleMenuEnabled)	{		::EnableItem( appleMenu, 0 );	}		if( IsntNull( theDialog ) )	{				CPGPModalGrafPortView	*dialogObj;		RGBColor				backColor;		RGBColor				foreColor;		SetPort( theDialog );		/*		** LGrafPortView::CreateGrafPortView() preserves the background		** color of the window, but it calls LGrafPortView::FinishCreate()		** while the background color is whacked. So, grab the color here		** and pass to the dialog object after calling CreateGrafPortView		*/				GetForeColor( &foreColor );		GetBackColor( &backColor );		dialogObj = (CPGPModalGrafPortView *)					LGrafPortView::CreateGrafPortView( dialogResID, NULL );		if( IsntNull( dialogObj ) )		{			dialogObj->SetForeAndBackColors( &foreColor, &backColor );						SetWRefCon( theDialog, (long) dialogObj );			dialogObj->SetUserCon( (SInt32) theDialog );						LPane::SetDefaultView( dialogObj );		}		else		{			DisposeDialog( theDialog );			theDialog = NULL;		}	}		return( theDialog );}	voidCPGPModalGrafPortView::ResizeWindowTo(	Int16	newWidth,	Int16	newHeight){	SDimension16	frameSize;		SizeWindow( GetMacPort(), newWidth, newHeight, TRUE );	ResizeFrameTo( newWidth, newHeight, TRUE);	GetFrameSize( frameSize );	ResizeImageTo( frameSize.width, frameSize.height, FALSE );	CalcPortFrameRect( mUserBounds );		PortToGlobalPoint( topLeft( mUserBounds ) );	PortToGlobalPoint( botRight(mUserBounds ) );	mMoveOnlyUserZoom = false;}	BooleanCPGPModalGrafPortView::ClickInGrow(	const EventRecord &inMacEvent){	Boolean	handled = FALSE;		if( mWindowIsResizeable )	{		long	result;				result = GrowWindow( GetMacPort(), inMacEvent.where, &mMinMaxSize );		if( result != 0 )		{			ResizeWindowTo( LoWord( result ), HiWord( result ) );		}				handled = TRUE;	}		return( handled );}	BooleanCPGPModalGrafPortView::ClickInDrag(	const EventRecord &inMacEvent){	Boolean	handled = FALSE;		if( mWindowIsMoveable )	{		Rect	limitRect;				limitRect = (**GetGrayRgn()).rgnBBox;				DragWindow( GetMacPort(), inMacEvent.where, &limitRect );				CalcPortFrameRect( mUserBounds );			PortToGlobalPoint( topLeft( mUserBounds ) );		PortToGlobalPoint( botRight(mUserBounds ) );		mMoveOnlyUserZoom = FALSE;				handled = TRUE;	}		return( handled );}// From PowerPlant:	voidCPGPModalGrafPortView::CalcStandardBoundsForScreen(	const Rect	&inScreenBounds,	Rect		&outStdBounds) const{									// Structure and Content regions are									//   in global coordinates	Rect	strucRect = UWindows::GetWindowStructureRect( GetMacPort() );	Rect	contRect = UWindows::GetWindowContentRect( GetMacPort() );										// Structure can be (and usually is)									//   larger than Content	Rect	border;	border.left = contRect.left - strucRect.left;	border.right = strucRect.right - contRect.right;	border.top = contRect.top - strucRect.top;	border.bottom = strucRect.bottom - contRect.bottom;						 						 			// Don't zoom too close to edge of screen	Int16	screenWidth = inScreenBounds.right - inScreenBounds.left - 4;	Int16	screenHeight = inScreenBounds.bottom - inScreenBounds.top - 4;										// Standard dimensions are the minimum									//   of mStandardSize and the size of									//   the screen	Int16	stdWidth = mStandardSize.width;	if (stdWidth > screenWidth - (border.left + border.right)) {		stdWidth = screenWidth - (border.left + border.right);	}		Int16	stdHeight = mStandardSize.height;	if (stdHeight > screenHeight - (border.top + border.bottom)) {		stdHeight = screenHeight - (border.top + border.bottom);	}									// Standard position is the point closest									//   to the current position at which									//   the Window will be all on screen																		// Move window horizontally so that left									//   or right edge of Struction region is									//   2 pixels from the edge of the screen	Int16	stdLeft = contRect.left;	if (stdLeft < inScreenBounds.left + border.left + 2) {		stdLeft = inScreenBounds.left + border.left + 2;	} else if (stdLeft > inScreenBounds.right - stdWidth - border.right - 2) {		stdLeft = inScreenBounds.right - stdWidth - border.right - 2;	}										// Move window vertically so that top									//   or bottom edge of Struction region is									//   2 pixels from the edge of the screen	Int16	stdTop = contRect.top;	if (stdTop < inScreenBounds.top + border.top + 2) {		stdTop = inScreenBounds.top + border.top + 2;	} else if (stdTop > inScreenBounds.bottom - stdHeight - border.bottom - 2) {		stdTop = inScreenBounds.bottom - stdHeight - border.bottom - 2;	}		outStdBounds.left = stdLeft;	outStdBounds.right = stdLeft + stdWidth;	outStdBounds.top = stdTop;	outStdBounds.bottom = stdTop + stdHeight;}// From PowerPlant:	BooleanCPGPModalGrafPortView::CalcStandardBounds(Rect &outStdBounds) const{									// Find GDevice containing largest									//   portion of Window	GDHandle	dominantDevice = UWindows::FindDominantDevice(							UWindows::GetWindowStructureRect( GetMacPort()));	if (dominantDevice == nil) {	// Window is offscreen, so use the									//   main scren		dominantDevice = ::GetMainDevice();	}	Rect		screenRect = (**dominantDevice).gdRect;										// Must compensate for MenuBar on the									//   main screen	if (dominantDevice == ::GetMainDevice()) {		screenRect.top += GetMBarHeight();	}		CalcStandardBoundsForScreen(screenRect, outStdBounds);		Rect		contentRect = UWindows::GetWindowContentRect( GetMacPort() );	return ::EqualRect(&outStdBounds, &contentRect);}// From PowerPlant:	voidCPGPModalGrafPortView::DoSetZoom(Boolean inZoomToStdState){	Rect	currBounds = UWindows::GetWindowContentRect( GetMacPort() );	Rect	zoomBounds;	if (inZoomToStdState) {			// Zoom to Standard state		if (CalcStandardBounds(zoomBounds)) {			return;					// Already at Standard state		}			} else {						// Zoom to User state		zoomBounds = mUserBounds;				if (mMoveOnlyUserZoom) {	// Special case for zooming a Window									//   that is at standard size, but									//   is partially offscreen			zoomBounds.right = zoomBounds.left +								(currBounds.right - currBounds.left);			zoomBounds.bottom = zoomBounds.top +								(currBounds.bottom - currBounds.top);		}	}		Int16	zoomWidth = zoomBounds.right - zoomBounds.left;	Int16	zoomHeight = zoomBounds.bottom - zoomBounds.top;	mMoveOnlyUserZoom = false;			// To avoid unnecessary redraws, we check to see if the		// current and zoom states are either the same size		// or at the same location			if ( ((currBounds.right - currBounds.left) == zoomWidth) &&		 ((currBounds.bottom - currBounds.top) == zoomHeight) ) {									// Same size, just move		::MoveWindow( GetMacPort() , zoomBounds.left, zoomBounds.top, false);		mMoveOnlyUserZoom = true;		} else if (::EqualPt(topLeft(currBounds), topLeft(zoomBounds))) {									// Same location, just resize		ResizeWindowTo( zoomWidth, zoomHeight );			} else {						// Different size and location									// Zoom appropriately		FocusDraw();		ApplyForeAndBackColors();		::EraseRect(&GetMacPort()->portRect);		if (inZoomToStdState) {			SetWindowStandardState(GetMacPort(), &zoomBounds);			::ZoomWindow(GetMacPort(), inZoomOut, false);		} else {			SetWindowStandardState(GetMacPort(), &currBounds);			SetWindowUserState(GetMacPort(), &zoomBounds);			::ZoomWindow(GetMacPort(), inZoomIn, false);		}		ResizeFrameTo(zoomWidth, zoomHeight, false);	}}	BooleanCPGPModalGrafPortView::ClickInZoom(	const EventRecord	&inMacEvent,	Int16				inZoomDirection){	Boolean	handled = FALSE;		if( mWindowIsResizeable )	{		if( TrackBox( GetMacPort(), inMacEvent.where, inZoomDirection ) )		{			Rect	stdBounds;			Boolean	zoomToStdState = ! CalcStandardBounds( stdBounds );			DoSetZoom( zoomToStdState );						handled = TRUE;		}	}		return( handled );}	pascal BooleanCPGPModalGrafPortView::ModalFilterProcHandler(	DialogPtr 	theDialog,	EventRecord *theEvent,	short 		*itemHit){	Boolean					handled = FALSE;	CPGPModalGrafPortView 	*grafPortObj;		*itemHit = 0;		LPeriodical::DevoteTimeToRepeaters( *theEvent );	grafPortObj	= GetWindowGrafPortView( theDialog );	switch( theEvent->what )	{		case updateEvt:		{			WindowPtr				theWindow = (WindowPtr) theEvent->message;			CPGPModalGrafPortView	*updateObj;						/* Determine if the window is one of ours */						handled = TRUE;			updateObj = GetWindowGrafPortView( theWindow );			if( IsntNull( updateObj ) )			{				updateObj->UpdatePort();			}			else if( IsntNull( grafPortObj ) )			{				// Eat the update event for another window so we do not keep				// getting it. Restore the bits if we can. Remember the				// window so we can invalidate it later.								StColorPortState	portState( theWindow );								portState.Normalize();				SetOrigin( 0, 0 );					grafPortObj->RememberInvalidWindow( theWindow );								BeginUpdate( theWindow );					grafPortObj->RestoreWindowBits( theWindow );				EndUpdate( theWindow );			}			else			{				handled = FALSE;			}						break;		}					case activateEvt:		{			WindowPtr				theWindow = (WindowPtr) theEvent->message;			CPGPModalGrafPortView	*activateObj;			/* Determine if the window is one of ours */						activateObj = GetWindowGrafPortView( theWindow );			if( IsntNull( activateObj ) )			{				if( ( theEvent->modifiers & activeFlag ) != 0 )				{					if( HaveAppearanceMgr() )					{						OSErr			err;						ControlHandle	rootControl = NULL;												/* The root control may have become disabled if another window						** was displayed in front of ours. Reenable the root control.						*/												err = GetRootControl( theWindow, &rootControl );						if( IsntErr( err ) && rootControl != NULL )						{							if( ! IsControlActive( rootControl ) )							{								ActivateControl( rootControl );								activateObj->Refresh();							}						}					}										activateObj->Activate();				}				else				{					activateObj->Deactivate();				}								handled = TRUE;			}			break;		}				case autoKey:		case keyDown:		{			if( IsntNull( grafPortObj ) )			{				handled = grafPortObj->ModalFilterProc( theDialog, theEvent,								itemHit );			}			else			{				handled = StdFilterProc( theDialog, theEvent, itemHit );			}					break;		}				default:		{			if( IsntNull( grafPortObj ) )			{				handled = grafPortObj->ModalFilterProc( theDialog, theEvent,								itemHit );			}						break;		}	}				return( handled );}	CPGPModalGrafPortView *CPGPModalGrafPortView::GetWindowGrafPortView(WindowPtr theWindow){	CPGPModalGrafPortView	*targetObj = NULL;	targetObj = sViewList;	while( IsntNull( targetObj ) )	{		if( targetObj->GetDialog() == theWindow )			break;					targetObj = targetObj->mNextView;	}	return( targetObj );}	BooleanCPGPModalGrafPortView::ModalFilterProc(	DialogPtr 	theDialog,	EventRecord *theEvent,	short 		*itemHit){	Boolean		handled;	MessageT	dismissMessage;		(void) theDialog;		handled = FALSE;		switch( theEvent->what )	{		case mouseDown:		{			WindowPtr	macWindowP;			Int16		thePart;						thePart = FindWindow( theEvent->where, &macWindowP );			if( macWindowP == GetMacPort() )			{				switch( thePart )				{					case inContent:					{						SetPort( GetMacPort() );						ClickInContent( *theEvent );												handled = TRUE;												break;					}										case inDrag:					{						handled = ClickInDrag( *theEvent );						break;					}										case inGrow:					{						handled = ClickInGrow( *theEvent );						break;					}										case inZoomIn:					case inZoomOut:					{						handled = ClickInZoom( *theEvent,										thePart );						break;					}				}			}			else if (thePart == inMenuBar)			{				SetPort( GetMacPort() );				ClickMenuBar(*theEvent);				handled = TRUE;			}						break;		}				case autoKey:		case keyDown:		{			LCommander *	theTarget;			CommandT		theCommand = cmd_Nothing;					theTarget = LCommander::GetTarget();						if( IsntNull( theTarget ) )			{				SInt32	menuChoice;									if ((theEvent->modifiers & cmdKey) != 0)				{					menuChoice = ::MenuKey((char) (theEvent->message										& charCodeMask));															if (HiWord(menuChoice) != 0)					{						theCommand = FindCommand(HiWord(menuChoice),										LoWord(menuChoice));					}									}				if (theCommand != cmd_Nothing)				{					StUnhiliteMenu unhiliter;										theTarget->ProcessCommand(theCommand, &menuChoice);				} 				else				{										theTarget->ProcessKeyPress( *theEvent );				}				handled = TRUE;			}			break;		}				case nullEvent:		{			// Fix for a bug? with ModalDialog. If a library dialog is			// shown while the process is in the background,			// then we never get an activate event and this everything draws			// disabled. Explicitly call Activate() if we see ourselves come			// to the front.						if( IsWindowHilited( GetMacPort() ) &&				! IsActive() )			{				Activate();			}			DoIdle( *theEvent );			break;		}	}		dismissMessage = GetDismissMessage();	if( dismissMessage != msg_Nothing )	{		// User has hit an item which dismisses the dialog. Set itemHit to 1		// to force ModalDialog to exit.				*itemHit 	= 1;		handled		= TRUE;	}			return( handled );}	MessageTCPGPModalGrafPortView::HandleMessage(MessageT theMessage){	return( theMessage );}	PGPErrorCPGPModalGrafPortView::DoDialog(void){	PGPError		err = kPGPError_NoErr;		DialogRef		theDialog;	MessageT		message;	theDialog = GetDialog();	pgpAssert( IsntNull( theDialog ) );		if( ! IsWindowVisible( theDialog ) )	{		InitCursor();		ShowWindow( theDialog );		SelectWindow( theDialog );	}		message = msg_Nothing;	while( message == msg_Nothing )	{		short	itemHit = 0;		while( itemHit == 0 )		{			ModalDialog( mModalFilterProc, &itemHit );			UpdateMenus();		}		message = HandleMessage( GetDismissMessage() );		if( message == msg_Cancel )		{			err = kPGPError_UserAbort;		}		else if( message != msg_Nothing )		{			err = mDialogError;		}				SetDismissMessage( message );	}				return( err );}		voidCPGPModalGrafPortView::SaveAndDisableEnabledMenus(){	UInt16		numMenus = GetNumMenusInMenuBar();	MenuHandle	helpMenu = nil;	MenuHandle	currentMenu;	SInt16		menuItemCount;	SInt16		cmdKey;				::HMGetHelpMenuHandle(&helpMenu);		// Walk through all menus	for (UInt16 i = 0; i < numMenus; i++) {		currentMenu = GetIndMenuInMenuBar(i);				// Look for edit menu items		if (mEditMenu == nil) {			menuItemCount = ::CountMItems(currentMenu);			for (SInt16 x = 1; x <= menuItemCount; x++) {				::GetItemCmd(currentMenu, x, &cmdKey);				switch (toupper(cmdKey)) {					case 'X':					case 'C':					case 'V':					case 'A':					{						mEditMenu = currentMenu;						switch (toupper(cmdKey)) {							case 'X':							{								mCutItem = x;							}							break;														case 'C':							{									mCopyItem = x;							}							break;														case 'V':							{								mPasteItem = x;							}							break;														case 'A':							{								mSelectAllItem = x;							}							break;						}					}					break;				}			}		}				// Save enabled ones		if (IsMenuItemEnabled(currentMenu, 0) && (currentMenu != helpMenu))	{			EnabledItem	enabledItem = {currentMenu, 0};						mEnabledMenuItems->InsertItemsAt(	1,												LArray::index_Last,												&enabledItem);			::DisableItem(currentMenu, 0);		}	}		// Save and disable edit menu	if (mEditMenu != nil) {		menuItemCount = ::CountMItems(mEditMenu);		for (SInt16 x = 1; x <= menuItemCount; x++) {			if (IsMenuItemEnabled(mEditMenu, x)) {				EnabledItem enabledItem = {mEditMenu, x};				mEnabledMenuItems->InsertItemsAt(	1,													LArray::index_Last,													&enabledItem);				::DisableItem(mEditMenu, x);			}		}	}}		voidCPGPModalGrafPortView::RestoreEnabledMenus(){	LArrayIterator	iterator(*mEnabledMenuItems);	EnabledItem		enabledItem;		while(iterator.Next(&enabledItem)) {		::EnableItem(enabledItem.menu, enabledItem.item);	}}	voidCPGPModalGrafPortView::UpdateMenus(){	// By default we handle cut, copy and paste in the same fashion	// as the Menu Manager and Dialog Manager according to	// IM: Mac Toolbox Essentials 3-85	LCommander *	theTarget = LCommander::GetTarget();	Boolean			isEnabled;	Boolean			usesMark;	// unused	Char16			mark;		// unused	Str255			itemName;	// unused	Boolean			editMenuEnabled;		if (mEditMenu != nil) {		editMenuEnabled = IsMenuItemEnabled(mEditMenu, 0);		::DisableItem(mEditMenu, 0);		if (theTarget != nil) {			if (mCutItem > 0) {				isEnabled = false;				usesMark = false;				itemName[0] = 0;				theTarget->ProcessCommandStatus(cmd_Cut, isEnabled, usesMark,								mark, itemName);				if (isEnabled) {					::EnableItem(mEditMenu, mCutItem);					::EnableItem(mEditMenu, 0);				} else {					::DisableItem(mEditMenu, mCutItem);				}			}			if (mCopyItem > 0) {				isEnabled = false;				usesMark = false;				itemName[0] = 0;				theTarget->ProcessCommandStatus(cmd_Copy, isEnabled, usesMark,								mark, itemName);				if (isEnabled) {					::EnableItem(mEditMenu, mCopyItem);					::EnableItem(mEditMenu, 0);				} else {					::DisableItem(mEditMenu, mCopyItem);				}			}			if (mPasteItem > 0) {				isEnabled = false;				usesMark = false;				itemName[0] = 0;				theTarget->ProcessCommandStatus(cmd_Paste, isEnabled, usesMark,								mark, itemName);				if (isEnabled) {					::EnableItem(mEditMenu, mPasteItem);					::EnableItem(mEditMenu, 0);				} else {					::DisableItem(mEditMenu, mPasteItem);				}			}			if (mSelectAllItem > 0) {				isEnabled = false;				usesMark = false;				itemName[0] = 0;				theTarget->ProcessCommandStatus(cmd_SelectAll, isEnabled,							usesMark, mark, itemName);				if (isEnabled) {					::EnableItem(mEditMenu, mSelectAllItem);					::EnableItem(mEditMenu, 0);				} else {					::DisableItem(mEditMenu, mSelectAllItem);				}			}		}		if (IsMenuItemEnabled(mEditMenu, 0) != editMenuEnabled) {			::DrawMenuBar();		}	}}	voidCPGPModalGrafPortView::ClickMenuBar(	const EventRecord &	inMacEvent){	StUnhiliteMenu	unhiliter;	SInt32			menuChoice = ::MenuSelect(inMacEvent.where);	CommandT		menuCmd = cmd_Nothing;		if (HiWord(menuChoice) != 0) {		menuCmd = FindCommand(HiWord(menuChoice), LoWord(menuChoice));	}		if ((menuCmd != cmd_Nothing) && (LCommander::GetTarget() != nil)) {		LCommander::GetTarget()->ProcessCommand(menuCmd, &menuChoice);	}}	CommandTCPGPModalGrafPortView::FindCommand(	ResIDT	menuID,	SInt16	item){	CommandT	theCommand = cmd_Nothing;		if (mEditMenu == ::GetMenuHandle(menuID)) {		if (item == mCutItem) {			theCommand = cmd_Cut;		} else if (item == mCopyItem) {			theCommand = cmd_Copy;		} else if (item == mPasteItem) {			theCommand = cmd_Paste;		} else if (item == mSelectAllItem) {			theCommand = cmd_SelectAll;		}	}		return theCommand;}