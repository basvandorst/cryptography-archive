/*
 * MacEvents.c -- macintosh event handling utilities
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 * Portions of this code are (C) Copyright 1995-1996 by David K. Heller
 *
 */

#include "MacEvents.h"

#define		kCommandKeyCode		((short) 0x37)
#define		kShiftKeyCode		((short) 0x38)
#define		kCapsLockKeyCode	((short) 0x39)
#define		kOptionKeyCode		((short) 0x3A)
#define		kControlKeyCode		((short) 0x3B)

#define		KeyIsDown(theKeys, keyCode)		\
	((((const uchar *)(theKeys))[(keyCode) >> 3]) & (1 << ((keyCode) & 7)))

/*____________________________________________________________________________
__
	Return the state of all keyboard modifier keys
______________________________________________________________________________
*/

	UInt16
GetAllModifiers(void)
{
	UInt16	modifiers = 0;
	KeyMap	theKeys;

	GetKeys( theKeys );

	if( KeyIsDown( theKeys, kCommandKeyCode ) )
		modifiers |= cmdKey;

	if( KeyIsDown( theKeys, kShiftKeyCode ) )
		modifiers |= shiftKey;

	if( KeyIsDown( theKeys, kCapsLockKeyCode ) )
		modifiers |= alphaLock;

	if( KeyIsDown( theKeys, kOptionKeyCode ) )
		modifiers |= optionKey;

	if( KeyIsDown( theKeys, kControlKeyCode ) )
		modifiers |= controlKey;

	return( modifiers );
}

/*____________________________________________________________________________
__
	Return the state of all keyboard modifier keys except the caps lock key
______________________________________________________________________________
*/

	UInt16
GetModifiers(void)
{
	return( GetAllModifiers() & ~alphaLock );
}

/*____________________________________________________________________________
__
	Return TRUE if the caps lock key is down
______________________________________________________________________________
*/

	Boolean
CapsLockIsDown(void)
{
	return( ( GetAllModifiers() & alphaLock ) != 0 );
}


