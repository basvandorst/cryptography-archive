/*____________________________________________________________________________
	MacTraps.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Portions of this code are (C) Copyright 1995-1996 by
	David K. Heller and are provided to PGP without restriction.
	

	$Id: MacTraps.c,v 1.4.10.1 1997/12/07 04:27:14 mhw Exp $
____________________________________________________________________________*/

#include <Traps.h>

#include "MacTraps.h"

#define kToolTrapMask 0x0800

	short
NumToolboxTraps(void)
{
	if( NGetTrapAddress( _InitGraf, ToolTrap ) ==
			NGetTrapAddress( 0xAA6E, ToolTrap ) )
	{
		return( 0x0200 );
	}
	else
	{
		return( 0x0400 );
	}
}

	TrapType
GetTrapType(short theTrap)
{
	if( (theTrap & kToolTrapMask) != 0 )
	{
		return ( ToolTrap );
	}
	else
	{
		return ( OSTrap );
	}
}

	Boolean
TrapAvailable(short theTrap)
{
	TrapType	trapType;

	trapType = GetTrapType(theTrap);
	if(trapType == ToolTrap)
	{
		theTrap = theTrap & 0x07FF;
	}
	
	if ( theTrap >= NumToolboxTraps() )
	{
		theTrap = _Unimplemented;
	}
	
	return ( NGetTrapAddress( theTrap, trapType ) !=
		NGetTrapAddress( _Unimplemented, ToolTrap ) );
}

	Boolean
PFLPatchTrap(short trapNum, UniversalProcPtr newAddr, UniversalProcPtr *oldAddr)
{
	TrapType			trapType;
	
	trapType	= GetTrapType( trapNum );
	*oldAddr	= NGetTrapAddress( trapNum, trapType );
	newAddr		= (UniversalProcPtr) StripAddress( newAddr );
	
	if ( newAddr != *oldAddr )
	{
		NSetTrapAddress( newAddr, trapNum, trapType );
		return( TRUE );
	}
	
	return( FALSE );
}
