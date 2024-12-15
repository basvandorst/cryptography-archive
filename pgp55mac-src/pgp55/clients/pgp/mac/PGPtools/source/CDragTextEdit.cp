/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <LDragAndDrop.h>
#include <LDragTask.h>

#include "pgpMem.h"

#include "CDragTextEdit.h"

CDragTextEdit::CDragTextEdit(LStream *inStream)
		: LTextEdit( inStream )
{
	if( IsntNull( GetMacTEH() ) && LDragAndDrop::DragAndDropIsPresent() )
	{
		TEFeatureFlag( teFOutlineHilite, teBitSet, GetMacTEH() );
	}
}

CDragTextEdit::~CDragTextEdit(void)
{
}

	Boolean
CDragTextEdit::PortPtInTextSelection(Point inPortPt)
{
	Boolean	inSelection = FALSE;
	
	FocusDraw();
	
	if( IsntNull( GetMacTEH() ) && LDragAndDrop::DragAndDropIsPresent() )
	{
		RgnHandle	hiliteRgn;
		
		hiliteRgn = NewRgn();
		if( IsntNull( hiliteRgn ) )
		{
			if( IsntErr( TEGetHiliteRgn( hiliteRgn, GetMacTEH() ) ) )
			{
				if( ! EmptyRgn( hiliteRgn ) )
				{
					if( PtInRgn( inPortPt, hiliteRgn ) )
					{
						inSelection = TRUE;
					}
				}
			}
			
			DisposeRgn( hiliteRgn );
		}
	}
	
	return( inSelection );
}

	void
CDragTextEdit::AdjustCursorSelf(
	Point				inPortPt,
	const EventRecord&	inMacEvent)
{
	if( PortPtInTextSelection( inPortPt ) )
	{
		InitCursor();
	}
	else
	{
		LTextEdit::AdjustCursorSelf( inPortPt, inMacEvent );
	}
}

	void
CDragTextEdit::Click(SMouseDownEvent &inMouseDown)
{
	Boolean	handled = FALSE;
	
	if( PortPtInTextSelection( inMouseDown.whereLocal ) )
	{
		if( WaitMouseMoved( inMouseDown.macEvent.where ) )
		{
			RgnHandle	dragRgn;
			
			dragRgn = NewRgn();
			if( IsntNull( dragRgn ) )
			{
				RgnHandle	tempRgn;
				
				tempRgn = NewRgn();
				if( IsntNull( tempRgn ) )
				{
					Point		localTopLeft;			
					Handle		theText;
					SInt8		saveTextState;
					char		*selectedDataStart;
					Size		selectedDataLength;
					TERec		**textRec;
					
					// Get outline drag rgn
					TEGetHiliteRgn( dragRgn, GetMacTEH() );
					CopyRgn( dragRgn, tempRgn );
					InsetRgn( tempRgn, 1, 1 );
					DiffRgn( dragRgn, tempRgn, dragRgn );
					DisposeRgn( tempRgn );

					// Convert the region to global coordinates.
					localTopLeft.v = 0;
					localTopLeft.h = 0;
					
					LocalToGlobal( &localTopLeft );
					OffsetRgn( dragRgn, localTopLeft.h, localTopLeft.v );
					
					theText = GetTextHandle();
					textRec = GetMacTEH();
					
					saveTextState = HGetState( theText );
					HLock( theText );
					
					selectedDataStart 	= *theText + (**textRec).selStart;
					selectedDataLength	= (**textRec).selEnd -
								(**textRec).selStart;
					
					LDragTask	dragTask( inMouseDown.macEvent,
											dragRgn,
											1L,
											'TEXT',
											selectedDataStart,
											selectedDataLength,
											0L );

					HSetState( theText, saveTextState );
			
					handled = TRUE;
				}
				
				DisposeRgn( dragRgn );
			}
		}
	}
	
	if( ! handled )
	{
		LTextEdit::Click( inMouseDown );
	}
}
