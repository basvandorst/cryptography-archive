/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacMenus.h,v 1.2 1997/08/21 22:24:03 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_MacMenus_h	/* [ */
#define Included_MacMenus_h

#include "Menus.h"

#pragma options align=mac68k
typedef struct MenuRec {
	MenuRef		menuRef;
	SInt16		menuLeft;
} MenuRec;

typedef struct MenuList {
	SInt16	lastMenu;
	SInt16	lastRight;
	SInt16	notUsed;
	MenuRec	menus[ 1 ];	/* dynamic */
} MenuList;
#pragma options align=reset


PGP_BEGIN_C_DECLARATIONS


ushort		GetNumMenusInMenuBar( void );
MenuHandle	GetIndMenuInMenuBar( ushort index );


PGP_END_C_DECLARATIONS

#endif /* ] Included_MacMenus_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
