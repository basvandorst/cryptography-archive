/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacMenus.c,v 1.3 1997/08/25 04:31:58 lloyd Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	/* or pgpConfig.h in the CDK */

#include "pgpDebug.h"
#include "pgpMem.h"
#include "MacMenus.h"


	static ushort
_GetNumMenusInMenuBar( const MenuList ** menuBar )
{
	ushort				numMenus	= 0;
	UInt32				listSize;
	
	listSize	= GetHandleSize( (Handle)menuBar );
	pgpAssert( sizeof( MenuList ) == 12 );
	/* assume array is declared as [1] */
	listSize	-= sizeof( MenuList ) - sizeof( MenuRec );
	
	numMenus	= listSize / sizeof( (**menuBar).menus[ 0 ] );
	
	return( numMenus );
}

	ushort
GetNumMenusInMenuBar( void )
{
	const MenuList **	menuBar	= NULL;
	ushort				numMenus	= 0;
	
	menuBar	= (MenuList **)GetMenuBar();
	pgpAssert( IsntNull( menuBar ) );
	if ( IsntNull( menuBar ) )
	{
		numMenus	= _GetNumMenusInMenuBar( menuBar );
		DisposeHandle( (Handle)menuBar );
	}
	
	return( numMenus );
}


	MenuHandle
GetIndMenuInMenuBar( ushort index )
{
	const MenuList **	menuBar	= NULL;
	ushort				numMenus	= 0;
	MenuHandle			menu	= NULL;
	
	menuBar	= (MenuList **)GetMenuBar();
	pgpAssert( IsntNull( menuBar ) );
	if ( IsntNull( menuBar ) )
	{
		if ( index >= _GetNumMenusInMenuBar( menuBar ) )
		{
			pgpDebugMsg( "illegal menu index" );
			menu	= NULL;
		}
		else
		{
			menu	= (MenuHandle)((**menuBar).menus[ index ].menuRef);
		}
		
		DisposeHandle( (Handle)menuBar );
	}
	
	return( menu );
}






























/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
