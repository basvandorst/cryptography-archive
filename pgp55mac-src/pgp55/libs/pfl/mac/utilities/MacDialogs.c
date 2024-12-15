/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDialogs.c,v 1.7 1997/09/18 01:34:42 lloyd Exp $
____________________________________________________________________________*/
#include "pgpMem.h"

#include "MacDialogs.h"

#define AssertDialogRefIsValid(theDialog)	\
	pgpAssertAddrValid( (theDialog), DialogRecord )

#define	RectWidth(x)	((UInt16) ((x)->right - (x)->left))
#define	RectHeight(x)	((UInt16) ((x)->bottom - (x)->top))


	void
SetUserItemProc(DialogRef theDialog, short theItem, UserItemUPP theProc)
{
	short	itemType;
	Handle	itemData;
	Rect	itemRect;
	
	AssertDialogRefIsValid( theDialog );
	AssertUPPIsNullOrValid( theProc, nil );

	GetDialogItem( theDialog, theItem, &itemType, &itemData, &itemRect );
	pgpAssertMsg( (itemType & ~itemDisable) == userItem,
		"SetUserItemProc(): Not a user item" );
	SetDialogItem( theDialog, theItem, itemType, (Handle) theProc, &itemRect );
}


	Handle
GetIndDialogItem(DialogRef theDialog, short theItem)
{
	short	itemType;
	Handle	itemData;
	Rect	itemRect;
	
	AssertDialogRefIsValid( theDialog );

	itemData = nil;
	GetDialogItem(theDialog, theItem, &itemType, &itemData, &itemRect);
	
	return ( itemData );
}


	void
GetDialogItemRect(DialogRef theDialog, short theItem, Rect *itemRect)
{
	Handle	itemData;
	short	itemType;
	
	AssertDialogRefIsValid( theDialog );
	pgpAssertAddrValid( itemRect, Rect );

	pgpAssertMsg( theItem > 0 && theItem <= CountDITL( theDialog ),
		"GetDialogItemRect(): Invalid item");
	
	GetDialogItem( theDialog, theItem, &itemType, &itemData, itemRect );
}


	void
SetDialogItemRect(DialogRef theDialog, short theItem, const Rect *newItemRect)
{
	Handle	itemData;
	short	itemType;
	Rect	oldItemRect;
	GrafPtr	savePort;
	
	AssertDialogRefIsValid( theDialog );
	pgpAssertAddrValid( newItemRect, Rect );

	GetPort( &savePort );
	SetGrafPortOfDialog( theDialog );
	
	GetDialogItem( theDialog, theItem, &itemType, &itemData, &oldItemRect );
	
	if( ! EqualRect( &oldItemRect, newItemRect ) )
	{
		InvalRect( &oldItemRect );
			SetDialogItem( theDialog, theItem,
				itemType, itemData, newItemRect );
		InvalRect( newItemRect );	
	}
	
	if( ( itemType & ctrlItem ) != 0 ) 
	{
		ControlHandle	theControl = (ControlHandle) itemData;
		
		MoveControl( theControl, newItemRect->left, newItemRect->top );
		SizeControl( theControl,
			RectWidth( newItemRect ), RectHeight( newItemRect ) );
	}
	
	SetPort( savePort );
}


	short
GetDialogItemType(DialogRef theDialog, short theItem)
{
	Handle	itemData;
	short	itemType;
	Rect	itemRect;
	
	AssertDialogRefIsValid( theDialog );
	pgpAssertMsg( theItem > 0 && theItem <= CountDITL( theDialog ),
		"GetDialogItemType(): Invalid item");
	
	GetDialogItem( theDialog, theItem, &itemType, &itemData, &itemRect );
	
	return( itemType & ~itemDisable );
}



	void
MoveDialogItem(DialogRef theDialog, short theItem, short hPos, short vPos)
{
	Rect	oldItemRect;
	Rect	newItemRect;
	
	AssertDialogRefIsValid( theDialog );

	GetDialogItemRect( theDialog, theItem, &oldItemRect );
	
	newItemRect.left	= hPos;
	newItemRect.top		= vPos;
	newItemRect.right	= hPos + RectWidth( &oldItemRect );
	newItemRect.bottom	= vPos + RectHeight( &oldItemRect );
	
	SetDialogItemRect( theDialog, theItem, &newItemRect );
}


	void
OffsetDialogItem(
	DialogRef	theDialog,
	short		theItem,
	short		hOffset,
	short		vOffset)
{
	Rect	itemRect;
	
	AssertDialogRefIsValid( theDialog );

	GetDialogItemRect( theDialog, theItem, &itemRect );
	MoveDialogItem( theDialog, theItem,
		itemRect.left + hOffset, itemRect.top + vOffset );
}


	void
InvalDialogItem(DialogRef theDialog, short theItem, short inset)
{
	GrafPtr	savePort;
	Rect	itemRect;
	
	AssertDialogRefIsValid( theDialog );

	GetPort( &savePort );
	SetGrafPortOfDialog( theDialog );
	
	GetDialogItemRect( theDialog, theItem, &itemRect );
	InsetRect( &itemRect, inset, inset );
	InvalRect( &itemRect );
	
	SetPort( savePort );
}


	void
EraseDialogItem(DialogRef theDialog, short theItem, short inset)
{
	GrafPtr	savePort;
	Rect	itemRect;
	
	AssertDialogRefIsValid( theDialog );

	GetPort( &savePort );
	SetGrafPortOfDialog( theDialog );
	
	GetDialogItemRect( theDialog, theItem, &itemRect );
	InsetRect( &itemRect, inset, inset );
	EraseRect( &itemRect );
	
	SetPort( savePort );
}


	void
ShowHideDialogItem(DialogRef theDialog, short theItem, Boolean show)
{
	AssertDialogRefIsValid( theDialog );

	if( show )
	{
		ShowDialogItem( theDialog, theItem );
	}
	else
	{
		HideDialogItem( theDialog, theItem );
	}
}


	ControlHandle
GetDialogControlItem(DialogRef theDialog, short theItem)
{
	short	itemType;
	Handle	controlHandle = nil;
	Rect	itemRect;
	
	AssertDialogRefIsValid( theDialog );

	GetDialogItem(theDialog, theItem, &itemType, &controlHandle, &itemRect);
	if( (itemType & ctrlItem) == 0 )
	{
		pgpDebugMsg( "GetDialogControlItem(): item not a control");
		controlHandle = nil;
	}
	else
	{
		AssertHandleIsValid( controlHandle, nil );
	}
	
	return ( (ControlHandle) controlHandle );
}


	short
GetDialogControlValue(DialogRef theDialog, short theItem)
{
	return( GetControlValue( GetDialogControlItem( theDialog, theItem ) ) );
}


	void
SetDialogControlValue(DialogRef theDialog, short theItem, short value)
{
	SetControlValue( GetDialogControlItem( theDialog, theItem ), value );
}


	Boolean
ToggleDialogCheckbox(DialogRef theDialog, short theItem)
{
	short	newValue;
	
	newValue = ( GetDialogControlValue( theDialog, theItem ) == 0 ? 1 : 0 );
	SetDialogControlValue( theDialog, theItem, newValue );
	
	return( newValue == 1 );
}


	void
GetIndDialogItemText(DialogRef theDialog, short theItem, uchar *itemText)
{
	Handle	itemHandle;
	short	itemType;
	Rect	itemRect;
	
	AssertDialogRefIsValid( theDialog );
	pgpAssertAddrValid( itemText, uchar );

	GetDialogItem( theDialog, theItem, &itemType, &itemHandle, &itemRect );
	
	if( ( itemType & ctrlItem ) != 0 )
	{
		GetControlTitle( (ControlHandle) itemHandle, itemText );
	}
	else if( ( itemType & ( statText | editText ) ) != 0 )
	{
		GetDialogItemText( itemHandle, itemText );
	}
	else
	{
		pgpDebugMsg("GetIndDialogItemText(): Item type has no text" );
	}	
}


	void
SetIndDialogItemText(
	DialogRef theDialog,
	short theItem, const uchar *itemText)
{
	Handle	itemHandle;
	short	itemType;
	Rect	itemRect;
	
	AssertDialogRefIsValid( theDialog );
	pgpAssertAddrValid( itemText, uchar );

	GetDialogItem( theDialog, theItem, &itemType, &itemHandle, &itemRect );
	
	if( ( itemType & ctrlItem ) != 0 )
	{
		SetControlTitle( (ControlHandle) itemHandle, itemText );
	}
	else if( ( itemType & ( statText | editText ) ) != 0 )
	{
		SetDialogItemText( itemHandle, itemText );
	}
	else
	{
		pgpDebugMsg("SetIndDialogItemText(): Item type has no text" );
	}	
}

