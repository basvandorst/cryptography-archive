/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: MacMenus.c,v 1.5.12.1 1998/11/12 03:19:41 heller Exp $____________________________________________________________________________*/#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */#include "pgpDebug.h"#include "pgpMem.h"#include "MacMenus.h"	static ushort_GetNumMenusInMenuBar( const MenuList ** menuBar ){	ushort				numMenus	= 0;	char *				firstMenu = (char *) &((**menuBar).menus[0]);	char *				lastMenu = (char *) *menuBar + (**menuBar).lastMenu											+ sizeof(MenuRec);		numMenus	= (lastMenu - firstMenu) / sizeof( (**menuBar).menus[ 0 ] );		return( numMenus );}	ushortGetNumMenusInMenuBar( void ){	const MenuList **	menuBar	= NULL;	ushort				numMenus	= 0;		menuBar	= (MenuList **)GetMenuBar();	pgpAssert( IsntNull( menuBar ) );	if ( IsntNull( menuBar ) )	{		numMenus	= _GetNumMenusInMenuBar( menuBar );		DisposeHandle( (Handle)menuBar );	}		return( numMenus );}	MenuHandleGetIndMenuInMenuBar( ushort index ){	const MenuList **	menuBar	= NULL;	ushort				numMenus	= 0;	MenuHandle			menu	= NULL;		menuBar	= (MenuList **)GetMenuBar();	pgpAssert( IsntNull( menuBar ) );	if ( IsntNull( menuBar ) )	{		if ( index >= _GetNumMenusInMenuBar( menuBar ) )		{			menu	= NULL;		}		else		{			menu	= (MenuHandle)((**menuBar).menus[ index ].menuRef);		}				DisposeHandle( (Handle)menuBar );	}		return( menu );}/*__Editor_settings____	Local Variables:	tab-width: 4	End:	vi: ts=4 sw=4	vim: si_____________________*/