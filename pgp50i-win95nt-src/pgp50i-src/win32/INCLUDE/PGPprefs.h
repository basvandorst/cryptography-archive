//: PGPprefs.h - include for PGP preferences dialog DLL
/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */

// return codes
#define PGPPREF_CANCEL				0x0000
#define PGPPREF_OK					0x0001
#define PGPPREF_RINGFILESMODIFIED	0x0002


//-------------------------------------------------|
//	PGPprefsPropertySheet -
//	post PGP preferences property sheets.
//
//	This function posts the "PGP Preferences" property
//	sheet dialog boxes.  Appropriate calls are made into
//	the pgpkeydb library to set the preferences.  Thus,
//	this function assumes that library is up and running
//	and that pgpLibInit has already been called.
//
//	Entry parameters :
//		hwnd		- handle of parent window
//		startsheet	- zero-based index of property sheet
//                    page to display initially
//
//	This function returns either PGPPREF_CANCEL (if the user
//  presses the cancel button) or a bitwise OR of the above-
//  defined constants:
//
//		PGPPREF_OK					- user pressed OK button
//		PGPPREF_RINGFILESMODIFIED	- the user changed the
//									  the keyring file settings	
//
//

__declspec(dllimport)UINT PGPprefsPropertySheet (HWND hwnd, int startsheet);
