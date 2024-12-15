/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CPGPLibGrafPortView.cp,v 1.8 1997/10/30 20:53:01 heller Exp $
____________________________________________________________________________*/

#include <Windows.h>

#include <LArrayIterator.h>
#include <LGAPushButton.h>
#include <PP_KeyCodes.h>
#include <PP_Messages.h>
#include <UDrawingUtils.h>
#include <UGAColorRamp.h>

#include "CPGPLibGrafPortView.h"
#include "pgpMem.h"

const PaneIDT	kOKButtonPaneID				= 'bOK ';
const PaneIDT	kCancelButtonPaneID			= 'bCAN';

typedef struct SavedWindowBitsInfo
{
	SavedWindowBitsInfo	*next;
	WindowRef			windowRef;
	ushort				bitDepth;
	Rect				bitsRect;	// In window coordinates
	GWorldPtr			gWorld;

} SavedWindowBitsInfo;


static CPGPLibGrafPortView	*sViewList 				= NULL;

CPGPLibGrafPortView::CPGPLibGrafPortView(LStream *inStream)
		: LGrafPortView(inStream)
{
	mDismissMessage 			= msg_Nothing;
	mInvalidWindowList			= new LArray( sizeof( WindowRef ) );
	mWindowIsMoveable			= FALSE;
	mWindowIsResizeable			= FALSE;
	mSaveRestoreWindowBits		= FALSE;
	mMinMaxSize.top				= 0;
	mMinMaxSize.left			= 0;
	mMinMaxSize.bottom			= 32767;
	mMinMaxSize.right			= 32767;
	mSavedWindowBitsList		= NULL;
	mMoveOnlyUserZoom			= FALSE;
	
	/* Link this view into the global view list */
	mNextView = sViewList;
	sViewList = this;
}

CPGPLibGrafPortView::~CPGPLibGrafPortView(void)
{
	CPGPLibGrafPortView	*curView;
	CPGPLibGrafPortView	*prevView;
	
	if( IsntNull( mInvalidWindowList ) )
	{
		WindowRef		windowRef;
		GrafPtr			savePort;	
		LArrayIterator	iterator( *mInvalidWindowList );
		
		GetPort( &savePort );
		
		while( iterator.Next( &windowRef ) )
		{
			SetPort( (GrafPtr) GetWindowPort( windowRef ) );
			InvalRect( &GetWindowPort( windowRef )->portRect );
		}
		
		SetPort( savePort );
	}

	delete( mInvalidWindowList );
	
	// Fix PowerPlant bug:
	DeleteAllSubPanes();
	
	/* Remove this view from the global view list */
	curView 	= sViewList;
	prevView	= NULL;
	
	while( IsntNull( curView ) )
	{
		if( curView == this )
		{
			if( IsntNull( prevView ) )
			{
				prevView->mNextView = mNextView;
			}
			else
			{
				sViewList = mNextView;
			}
			
			break;
		}
		
		prevView 	= curView;
		curView 	= curView->mNextView;
	}
	
	if( IsntNull( mSavedWindowBitsList ) )
	{
		SavedWindowBitsInfo	*curInfo;
		
		curInfo = mSavedWindowBitsList;
		while( IsntNull( curInfo ) )
		{
			SavedWindowBitsInfo	*nextInfo = curInfo->next;
			
			if( IsntNull( curInfo->gWorld ) )
				DisposeGWorld( curInfo->gWorld );
				
			pgpFree( curInfo );
			
			curInfo = nextInfo;
		}
	}
}

	void
CPGPLibGrafPortView::FinishCreateSelf(void)
{
	LGrafPortView::FinishCreateSelf();
	
	mOKButton = (LGAPushButton *) FindPaneByID( kOKButtonPaneID );
	if( IsntNull( mOKButton ) )
	{
		mOKButton->SetDefaultButton( TRUE );
		mOKButton->AddListener( this );
	}

	mCancelButton = (LGAPushButton *) FindPaneByID( kCancelButtonPaneID );
	if( IsntNull( mCancelButton ) )
	{
		mCancelButton->AddListener(this);
	}
	
	mSaveRestoreWindowBits = mWindowIsMoveable || mWindowIsResizeable;
	if( mSaveRestoreWindowBits )
	{
		// This only works if the window is not visible at creation time.
		pgpAssert( ! IsWindowVisible( GetMacPort() ) );
		
		SaveAllWindowBits();
	}

	CalcPortFrameRect( mUserBounds );	
	PortToGlobalPoint( topLeft( mUserBounds ) );
	PortToGlobalPoint( botRight(mUserBounds ) );

	mStandardSize.width 	= mMinMaxSize.right;
	mStandardSize.height 	= mMinMaxSize.bottom;
}

	void
CPGPLibGrafPortView::SaveWindowBits(WindowRef theWindow)
{
	GrafPtr		savePort;
	Rect		visRect;
	RGBColor	saveBackColor;
	RGBColor	saveForeColor;
	
	GetPort( &savePort );
	SetPort( theWindow );
	
	GetBackColor( &saveBackColor );
	GetForeColor( &saveForeColor );
	
	BackColor( whiteColor );
	ForeColor( blackColor );
		
	visRect = (**theWindow->visRgn).rgnBBox;
	
	{
		StDeviceLoop	devLoop( visRect );		
		SInt16			depth;
		
		while( devLoop.NextDepth( depth ) )
		{
			SavedWindowBitsInfo	*info;
			
			info = (SavedWindowBitsInfo *) pgpAlloc( sizeof( *info ) );
			if( IsntNull( info ) )
			{
				QDErr	qdErr;
				Boolean	addToList = FALSE;
				
				// Clipping region is setup for the part of this window at
				// this depth. Copy away the bits.
				
				pgpClearMemory( info, sizeof( *info ) );
				
				info->windowRef	= theWindow;
				info->bitDepth 	= depth;
				info->bitsRect	= (**theWindow->clipRgn).rgnBBox;

				qdErr = NewGWorld( &info->gWorld, depth, &info->bitsRect,
							NULL, devLoop.GetCurrentDevice(),
							pixPurge | noNewDevice | useTempMem );
				if( qdErr == noErr )
				{
					if( LockPixels( info->gWorld->portPixMap ) )
					{
						CopyBits( 	&theWindow->portBits,
									&((GrafPtr) info->gWorld)->portBits,
									&info->bitsRect, &info->bitsRect,
									srcCopy, NULL );
						
						UnlockPixels( info->gWorld->portPixMap );
						
						addToList = TRUE;
					}
				}
				
				if( addToList )
				{
					info->next 				= mSavedWindowBitsList;
					mSavedWindowBitsList	= info;
				}
				else
				{
					pgpFree( info );
					info = NULL;
				}
			}
		}
	}
	
	RGBBackColor( &saveBackColor );
	RGBForeColor( &saveForeColor );

	SetPort( savePort );
}

	void
CPGPLibGrafPortView::SaveAllWindowBits(void)
{
	WindowRef	theWindow;
	
	// Walk the window list of the current app and remember contents
	// of the visible windows.

	theWindow = LMGetWindowList();
	while( IsntNull( theWindow ) )
	{
		if( IsWindowVisible( theWindow ) &&
			theWindow != GetMacPort() &&
			IsntNull( theWindow->visRgn ) &&
			! EmptyRgn( theWindow->visRgn ) )
		{
			SaveWindowBits( theWindow );
		}
		
		theWindow = (WindowRef) ((WindowPeek) theWindow)->nextWindow;
	}
}

	void
CPGPLibGrafPortView::RestoreWindowBits(WindowRef theWindow)
{
	if( mSaveRestoreWindowBits )
	{
		CGrafPtr	savePort;
		GDHandle	saveGDevice;
		Rect		visRect;
		RGBColor	saveBackColor;
		RGBColor	saveForeColor;
		
		GetGWorld( &savePort, &saveGDevice );
		SetPort( theWindow );
		
		GetBackColor( &saveBackColor );
		GetForeColor( &saveForeColor );
		
		BackColor( whiteColor );
		ForeColor( blackColor );
		
		visRect = (**theWindow->visRgn).rgnBBox;
		{
			StDeviceLoop	devLoop( visRect );		
			SInt16			depth;
			
			while( devLoop.NextDepth( depth ) )
			{
				SavedWindowBitsInfo	*info;
				
				// Find an info record for this window at this depth
				
				info = mSavedWindowBitsList;
				while( IsntNull( info ) )
				{
					if( info->windowRef == theWindow &&
						info->bitDepth == depth )
					{
						break;
					}
					
					info = info->next;
				}
				
				if( IsntNull( info ) )
				{
					if( LockPixels( info->gWorld->portPixMap ) )
					{
						CopyBits( 	&((GrafPtr) info->gWorld)->portBits,
									&theWindow->portBits, 
									&info->bitsRect, &info->bitsRect,
									srcCopy, NULL );
						
						UnlockPixels( info->gWorld->portPixMap );
					}
				}
			}
		}

		RGBBackColor( &saveBackColor );
		RGBForeColor( &saveForeColor );
		
		SetGWorld( savePort, saveGDevice );
	}
}

	void
CPGPLibGrafPortView::ListenToMessage(MessageT inMessage, void *ioParam)
{
	#pragma unused( ioParam )
	
	switch( inMessage )
	{
		case msg_OK:
		case msg_Cancel:
			SetDismissMessage( inMessage );
			break;
	}
}

	Boolean
CPGPLibGrafPortView::HandleKeyPress(const EventRecord&	inKeyEvent)
{
	Boolean		keyHandled = TRUE;
	Int16		theKey = inKeyEvent.message & charCodeMask;
	
	if( (inKeyEvent.modifiers & cmdKey) != 0 )
	{
		// Handle Cmd-. to abort the dialog.
		switch( theKey )
		{
			case '.':
			{
				if( IsntNull( mCancelButton ) )
				{
					mCancelButton->SimulateHotSpotClick( kControlButtonPart );
					SetDismissMessage( msg_Cancel );
				}
				
				break;
			}
				
			default:
				keyHandled = LGrafPortView::HandleKeyPress( inKeyEvent );
				break;
		}
	}
	else
	{
		switch( theKey )
		{
			case char_Return:
			case char_Enter:
			{
				if( IsntNull( mOKButton ) && mOKButton->IsEnabled() )
				{
					mOKButton->SimulateHotSpotClick( kControlButtonPart );
					SetDismissMessage( msg_OK );
				}
				break;
			}
				
			case char_Escape:
			{
				if( IsntNull( mCancelButton ) )
				{
					mCancelButton->SimulateHotSpotClick( kControlButtonPart );
					SetDismissMessage( msg_Cancel );
				}
				break;
			}
			
			default:
				keyHandled = LGrafPortView::HandleKeyPress( inKeyEvent );
				break;
		}
	}
	
	return keyHandled;
}

	void
CPGPLibGrafPortView::GlobalToPortPoint(Point &ioPoint) const
{
	// This is stolen from LWindow. Apparently the CodeWarrior folks never
	// tried to do dragging from a GrafPortView.
	
	// Windows have a reference to the pixel image of the main screen--
	// portBits for B&W and portPixMap for color windows. The bounds
	// of the pixel image specify the alignment of the *local* Window
	// coordinates with *global* coordinates:
	//		localPt = globalPt + topLeft(imageBounds)
	//
	// To convert from *local* to *port* coordinates, we offset by
	// the top left of the Window's port rectangle:
	//		portPt = localPt - topLeft(portRect)
	//
	// Therefore,
	//		portPt = globalPt + topLeft(imageBounds) - topLeft(portRect)
	//
	// Note: We don't use the Toolbox routine GlobalToLocal because
	// that routine depends on the current port and the current port
	// origin. To use GlobalToLocal we would have to do the following:
	//		GrafPtr savePort;
	//		GetPort(&savePort);
	//		Point	saveOrigin = topLeft(mMacWindowP->portRect);
	//		SetPort(mMacWindowP);
	//		SetOrigin(0,0);
	//		GlobalToLocal(ioPoint);
	//		SetOrigin(saveOrigin.h, saveOrigin.v);
	//		SetPort(savePort);
	// The equation above avoids all this saving/setting/restoring
	// of the current port and port origin.

									// Assume a B&W Window
	Point	localOffset = topLeft(mGrafPtr->portBits.bounds);
	if (UDrawingUtils::IsColorGrafPort(mGrafPtr))
	{
									// Nope, it's a color Window
		CGrafPtr	colorPortP = (CGrafPtr) mGrafPtr;
		localOffset = topLeft((**(colorPortP->portPixMap)).bounds);
	}
	
	ioPoint.h += (localOffset.h - mGrafPtr->portRect.left);
	ioPoint.v += (localOffset.v - mGrafPtr->portRect.top);
}

	void
CPGPLibGrafPortView::PortToGlobalPoint(Point &ioPoint) const
{
									// Assume a B&W Window
	Point	localOffset = topLeft(mGrafPtr->portBits.bounds);
	if (UDrawingUtils::IsColorGrafPort(mGrafPtr)) {
									// Nope, it's a color Window
		CGrafPtr	colorPortP = (CGrafPtr) mGrafPtr;
		localOffset = topLeft((**(colorPortP->portPixMap)).bounds);
	}
	
	ioPoint.h -= (localOffset.h - mGrafPtr->portRect.left);
	ioPoint.v -= (localOffset.v - mGrafPtr->portRect.top);
}

	void
CPGPLibGrafPortView::DoIdle(const EventRecord &inMacEvent)
{
	Point	portMouse;
	Rect	viewRect;
	
	LGrafPortView::DoIdle( inMacEvent );
	
	portMouse = inMacEvent.where;
	GlobalToPortPoint( portMouse );
	CalcLocalFrameRect( viewRect );
	
	if( PtInRect( portMouse, &viewRect) )
	{
		AdjustCursor( portMouse, inMacEvent );
	}
}

	void
CPGPLibGrafPortView::RememberInvalidWindow(WindowRef theWindow)
{
	if( IsntNull( theWindow ) && IsntNull( mInvalidWindowList ) )
	{
		if( mInvalidWindowList->FetchIndexOf( &theWindow ) ==
					LArray::index_Bad )
		{
			mInvalidWindowList->InsertItemsAt( 1, max_Int32, &theWindow );
		}
	}
}

	void
CPGPLibGrafPortView::Draw(RgnHandle	inSuperDrawRgnH)
{
	FocusDraw();
	ApplyForeAndBackColors();
	
	LGrafPortView::Draw(inSuperDrawRgnH);

	if( mWindowIsResizeable )
	{
		Rect	sizeBox;
		
		sizeBox 		= GetMacPort()->portRect;
		sizeBox.left 	= sizeBox.right - 15;
		sizeBox.top 	= sizeBox.bottom - 15;
		
		/* Draw the window grow icon without the scroll bar borders */
		
		StClipRgnState	saveClip( sizeBox );
		DrawGrowIcon( GetMacPort() );
	}
}

	void
CPGPLibGrafPortView::UpdatePort(void)
{
	FocusDraw();
	ApplyForeAndBackColors();
	
	LGrafPortView::UpdatePort();
}

	DialogRef
CPGPLibGrafPortView::GetDialog(void)
{
	return( (DialogRef) GetUserCon() );
}

	CPGPLibGrafPortView *
CPGPLibGrafPortView::CreateDialog(ResIDT dialogResID)
{
	DialogRef			theDialog;
	CPGPLibGrafPortView	*dialogObj;
	
	dialogObj = NULL;
	
	FlushEvents( everyEvent, 0 );
	
	theDialog = GetNewDialog( dialogResID, NULL, (WindowRef) -1L );
	if( IsntNull( theDialog ) )
	{		
		SetPort( theDialog );
		
		dialogObj = (CPGPLibGrafPortView *)
					LGrafPortView::CreateGrafPortView( dialogResID, NULL );
		if( IsntNull( dialogObj ) )
		{	
			RGBColor	backColor = UGAColorRamp::GetColor( colorRamp_Gray2 );
			RGBColor	foreColor = UGAColorRamp::GetBlackColor();
			
			SetWRefCon( theDialog, (long) dialogObj );
			dialogObj->SetUserCon( (SInt32) theDialog );
			
			LPane::SetDefaultView( dialogObj );
			dialogObj->SetForeAndBackColors( &foreColor, &backColor );
		}
		else
		{
			DisposeDialog( theDialog );
		}
	}
	
	return( dialogObj );
}

	void
CPGPLibGrafPortView::ResizeWindowTo(
	Int16	newWidth,
	Int16	newHeight)
{
	SDimension16	frameSize;
	
	SizeWindow( GetMacPort(), newWidth, newHeight, TRUE );
	ResizeFrameTo( newWidth, newHeight, TRUE);

	GetFrameSize( frameSize );
	ResizeImageTo( frameSize.width, frameSize.height, FALSE );

	CalcPortFrameRect( mUserBounds );	
	PortToGlobalPoint( topLeft( mUserBounds ) );
	PortToGlobalPoint( botRight(mUserBounds ) );
	mMoveOnlyUserZoom = false;
}

	Boolean
CPGPLibGrafPortView::ClickInGrow(
	const EventRecord &inMacEvent)
{
	Boolean	handled = FALSE;
	
	if( mWindowIsResizeable )
	{
		long	result;
		
		result = GrowWindow( GetMacPort(), inMacEvent.where, &mMinMaxSize );
		if( result != 0 )
		{
			ResizeWindowTo( LoWord( result ), HiWord( result ) );
		}
		
		handled = TRUE;
	}
	
	return( handled );
}

	Boolean
CPGPLibGrafPortView::ClickInDrag(
	const EventRecord &inMacEvent)
{
	Boolean	handled = FALSE;
	
	if( mWindowIsMoveable )
	{
		Rect	limitRect;
		
		limitRect = (**GetGrayRgn()).rgnBBox;
		
		DragWindow( GetMacPort(), inMacEvent.where, &limitRect );
		
		CalcPortFrameRect( mUserBounds );	
		PortToGlobalPoint( topLeft( mUserBounds ) );
		PortToGlobalPoint( botRight(mUserBounds ) );
		mMoveOnlyUserZoom = FALSE;
		
		handled = TRUE;
	}
	
	return( handled );
}

// From PowerPlant:
	void
CPGPLibGrafPortView::CalcStandardBoundsForScreen(
	const Rect	&inScreenBounds,
	Rect		&outStdBounds) const
{
									// Structure and Content regions are
									//   in global coordinates
	Rect	strucRect = UWindows::GetWindowStructureRect( GetMacPort() );
	Rect	contRect = UWindows::GetWindowContentRect( GetMacPort() );
	
									// Structure can be (and usually is)
									//   larger than Content
	Rect	border;
	border.left = contRect.left - strucRect.left;
	border.right = strucRect.right - contRect.right;
	border.top = contRect.top - strucRect.top;
	border.bottom = strucRect.bottom - contRect.bottom;
						 
						 			// Don't zoom too close to edge of screen
	Int16	screenWidth = inScreenBounds.right - inScreenBounds.left - 4;
	Int16	screenHeight = inScreenBounds.bottom - inScreenBounds.top - 4;
	
									// Standard dimensions are the minimum
									//   of mStandardSize and the size of
									//   the screen
	Int16	stdWidth = mStandardSize.width;
	if (stdWidth > screenWidth - (border.left + border.right)) {
		stdWidth = screenWidth - (border.left + border.right);
	}
	
	Int16	stdHeight = mStandardSize.height;
	if (stdHeight > screenHeight - (border.top + border.bottom)) {
		stdHeight = screenHeight - (border.top + border.bottom);
	}

									// Standard position is the point closest
									//   to the current position at which
									//   the Window will be all on screen
									
									// Move window horizontally so that left
									//   or right edge of Struction region is
									//   2 pixels from the edge of the screen
	Int16	stdLeft = contRect.left;
	if (stdLeft < inScreenBounds.left + border.left + 2) {
		stdLeft = inScreenBounds.left + border.left + 2;
	} else if (stdLeft > inScreenBounds.right - stdWidth - border.right - 2) {
		stdLeft = inScreenBounds.right - stdWidth - border.right - 2;
	}
	
									// Move window vertically so that top
									//   or bottom edge of Struction region is
									//   2 pixels from the edge of the screen
	Int16	stdTop = contRect.top;
	if (stdTop < inScreenBounds.top + border.top + 2) {
		stdTop = inScreenBounds.top + border.top + 2;
	} else if (stdTop > inScreenBounds.bottom - stdHeight - border.bottom - 2) {
		stdTop = inScreenBounds.bottom - stdHeight - border.bottom - 2;
	}
	
	outStdBounds.left = stdLeft;
	outStdBounds.right = stdLeft + stdWidth;
	outStdBounds.top = stdTop;
	outStdBounds.bottom = stdTop + stdHeight;
}

// From PowerPlant:

	Boolean
CPGPLibGrafPortView::CalcStandardBounds(Rect &outStdBounds) const
{
									// Find GDevice containing largest
									//   portion of Window
	GDHandle	dominantDevice = UWindows::FindDominantDevice(
							UWindows::GetWindowStructureRect( GetMacPort()));

	if (dominantDevice == nil) {	// Window is offscreen, so use the
									//   main scren
		dominantDevice = ::GetMainDevice();
	}
	Rect		screenRect = (**dominantDevice).gdRect;
	
									// Must compensate for MenuBar on the
									//   main screen
	if (dominantDevice == ::GetMainDevice()) {
		screenRect.top += GetMBarHeight();
	}
	
	CalcStandardBoundsForScreen(screenRect, outStdBounds);
	
	Rect		contentRect = UWindows::GetWindowContentRect( GetMacPort() );
	return ::EqualRect(&outStdBounds, &contentRect);
}

// From PowerPlant:

	void
CPGPLibGrafPortView::DoSetZoom(Boolean inZoomToStdState)
{
	Rect	currBounds = UWindows::GetWindowContentRect( GetMacPort() );
	Rect	zoomBounds;

	if (inZoomToStdState) {			// Zoom to Standard state
		if (CalcStandardBounds(zoomBounds)) {
			return;					// Already at Standard state
		}
		
	} else {						// Zoom to User state
		zoomBounds = mUserBounds;
		
		if (mMoveOnlyUserZoom) {	// Special case for zooming a Window
									//   that is at standard size, but
									//   is partially offscreen
			zoomBounds.right = zoomBounds.left +
								(currBounds.right - currBounds.left);
			zoomBounds.bottom = zoomBounds.top +
								(currBounds.bottom - currBounds.top);
		}
	}
	
	Int16	zoomWidth = zoomBounds.right - zoomBounds.left;
	Int16	zoomHeight = zoomBounds.bottom - zoomBounds.top;
	mMoveOnlyUserZoom = false;
	
		// To avoid unnecessary redraws, we check to see if the
		// current and zoom states are either the same size
		// or at the same location
		
	if ( ((currBounds.right - currBounds.left) == zoomWidth) &&
		 ((currBounds.bottom - currBounds.top) == zoomHeight) ) {
									// Same size, just move
		::MoveWindow( GetMacPort() , zoomBounds.left, zoomBounds.top, false);
		mMoveOnlyUserZoom = true;
	
	} else if (::EqualPt(topLeft(currBounds), topLeft(zoomBounds))) {
									// Same location, just resize
		ResizeWindowTo( zoomWidth, zoomHeight );
		
	} else {						// Different size and location
									// Zoom appropriately
		FocusDraw();
		ApplyForeAndBackColors();
		::EraseRect(&GetMacPort()->portRect);
		if (inZoomToStdState) {
			SetWindowStandardState(GetMacPort(), &zoomBounds);
			::ZoomWindow(GetMacPort(), inZoomOut, false);
		} else {
			SetWindowStandardState(GetMacPort(), &currBounds);
			SetWindowUserState(GetMacPort(), &zoomBounds);
			::ZoomWindow(GetMacPort(), inZoomIn, false);
		}
		ResizeFrameTo(zoomWidth, zoomHeight, false);
	}
}

	Boolean
CPGPLibGrafPortView::ClickInZoom(
	const EventRecord	&inMacEvent,
	Int16				inZoomDirection)
{
	Boolean	handled = FALSE;
	
	if( mWindowIsResizeable )
	{
		if( TrackBox( GetMacPort(), inMacEvent.where, inZoomDirection ) )
		{
			Rect	stdBounds;
			Boolean	zoomToStdState = ! CalcStandardBounds( stdBounds );

			DoSetZoom( zoomToStdState );
			
			handled = TRUE;
		}
	}
	
	return( handled );
}

	pascal Boolean
CPGPLibGrafPortView::ModalFilterProc(
	DialogPtr 	theDialog,
	EventRecord *theEvent,
	short 		*itemHit)
{
	Boolean				handled;
	CPGPLibGrafPortView *grafPortObj;
	MessageT			dismissMessage;
	
	handled 	= FALSE;
	*itemHit	= 0;
	grafPortObj	= (CPGPLibGrafPortView *) GetWRefCon( theDialog );
	pgpAssertAddrValid( grafPortObj, VoidAlign );
	
	LPeriodical::DevoteTimeToRepeaters( *theEvent );

	switch( theEvent->what )
	{
		case mouseDown:
		{
			WindowPtr	macWindowP;
			Int16		thePart;
			
			thePart = FindWindow( theEvent->where, &macWindowP );
			if( macWindowP == grafPortObj->GetMacPort() )
			{
				switch( thePart )
				{
					case inContent:
					{
						SetPort( grafPortObj->GetMacPort() );
						grafPortObj->ClickInContent( *theEvent );
						
						handled = TRUE;
						
						break;
					}
					
					case inDrag:
					{
						handled = grafPortObj->ClickInDrag( *theEvent );
						break;
					}
					
					case inGrow:
					{
						handled = grafPortObj->ClickInGrow( *theEvent );
						break;
					}
					
					case inZoomIn:
					case inZoomOut:
					{
						handled = grafPortObj->ClickInZoom( *theEvent,
										thePart );
						break;
					}

				}
			}
			
			break;
		}
		
		case autoKey:
		case keyDown:
		{
			LCommander *theTarget;
			
			theTarget = LCommander::GetTarget();
			if( IsntNull( theTarget ) )
			{
				theTarget->ProcessKeyPress( *theEvent );
				handled = TRUE;
			}
			
			break;
		}
		
		case updateEvt:
		{
			WindowPtr	theWindow = (WindowPtr) theEvent->message;
			
			if( theWindow == grafPortObj->GetDialog() )
			{
				// Fix for a bug? with ModalDialog. If a library dialog is
				// shown while the process is in the background,
				// then we never get an activate event and this everything
				// draws disabled. Explicitly call Activate() in this case.
				
				if( IsWindowHilited( theWindow ) && ! grafPortObj->IsActive())
				{
					grafPortObj->Activate();
				}
				
				grafPortObj->UpdatePort();
			}
			else
			{
				CPGPLibGrafPortView	*targetObj;

				/* Determine if the window is one of ours */
				
				targetObj = sViewList;
				while( IsntNull( targetObj ) )
				{
					if( targetObj->GetDialog() == theWindow )
						break;
						
					targetObj = targetObj->mNextView;
				}
			
				if( IsntNull( targetObj ) )
				{
					targetObj->UpdatePort();
				}
				else
				{
					// Eat the update event for another window so we do not keep
					// getting it. Restore the bits if we can. Remember the window
					// so we can invalidate it later.
					
					grafPortObj->RememberInvalidWindow( theWindow );
					
					SetPort( theWindow );
					BeginUpdate( theWindow );
					
					grafPortObj->RestoreWindowBits( theWindow );
					
					EndUpdate( theWindow );
					SetPort( grafPortObj->GetMacPort() );
				}
			}
			
			handled = TRUE;
			
			break;
		}
		
		case activateEvt:
		{
			CPGPLibGrafPortView	*targetObj;

			/* Determine if the window is one of ours */
			
			targetObj = sViewList;
			while( IsntNull( targetObj ) )
			{
				if( targetObj->GetDialog() == (WindowPtr) theEvent->message )
					break;
					
				targetObj = targetObj->mNextView;
			}

			if( IsntNull( targetObj ) )
			{
				if( ( theEvent->modifiers & activeFlag ) != 0 )
				{
					targetObj->Activate();
				}
				else
				{
					targetObj->Deactivate();
				}
				
				handled = TRUE;
			}

			break;
		}
		
		case nullEvent:
		{
			// Fix for a bug? with ModalDialog. If a library dialog is
			// shown while the process is in the background,
			// then we never get an activate event and this everything draws
			// disabled. Explicitly call Activate() if we see ourselves come
			// to the front.
			
			if( IsWindowHilited( grafPortObj->GetMacPort() ) &&
				! grafPortObj->IsActive() )
			{
				grafPortObj->Activate();
			}

			grafPortObj->DoIdle( *theEvent );
			break;
		}
	}
	
	dismissMessage = grafPortObj->GetDismissMessage();
	if( dismissMessage != msg_Nothing )
	{
		// User has hit an item which dismisses the dialog. Set itemHit to 1
		// to force ModalDialog to exit.
		
		*itemHit 	= 1;
		handled		= TRUE;
	}
		
	return( handled );
}


