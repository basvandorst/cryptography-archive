/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
____________________________________________________________________________*/
#include "pgpMem.h"
#include "MacBasics.h"
#include "MacEnvirons.h"

#include "MacStrings.h"
#include "MacTraps.h"

	UInt16
GetSystemVersion( void )
	{
	static UInt16	sSystemVersion	 = 0;
	
	if ( sSystemVersion == 0 )
		{
		long	gestaltResult;
		
		if( TrapAvailable( _Gestalt ) &&
			Gestalt( gestaltSystemVersion, &gestaltResult ) == noErr )
			{
			sSystemVersion	= (UInt16)gestaltResult;
			}
		else
			{
			pgpDebugPStr( "\pGetSystemVersion: "
			"gestaltSystemVersion undefined" );
			sSystemVersion	= 0x0600;
			}
		}
	
	return( sSystemVersion );
	}



	Boolean
VirtualMemoryIsOn( void )
	{
	static Boolean	sVirtualMemoryIsOn = 99;
	
	if ( sVirtualMemoryIsOn == 99 )
		{
		long	gestaltResult;
		OSErr	err	= noErr;
	
		err	= Gestalt( gestaltVMAttr, &gestaltResult );
		if ( IsntErr( err ) )
			{
			sVirtualMemoryIsOn	=
				(gestaltResult & (1UL << gestaltVMPresent)) != 0;
			}
		else
			{
			sVirtualMemoryIsOn	= FALSE;
			}
		}
		
	return( sVirtualMemoryIsOn );
	}

	void
GetMachineOwnerName(StringPtr owner)
{
	short	saveResFile;
	uchar	**ownerNameRes;
	
	pgpAssertAddrValid( owner, uchar );

	owner[0] = 0;
	
	saveResFile = CurResFile();
	UseResFile( 2 );
	
	ownerNameRes = (uchar **) Get1Resource( 'STR ', -16096 );
	if( IsntNull( ownerNameRes ) && IsntNull( *ownerNameRes ) )
	{
		CopyPString( *ownerNameRes, owner );
	}
	
	UseResFile( saveResFile );
}


#if ! powerc

	Boolean
RunningOnPowerPC( void )
	{
	long		gestaltResult;
	Boolean		onPowerPC	= false;
	
	if ( Gestalt( gestaltSysArchitecture, &gestaltResult ) == noErr )
		{
		onPowerPC	= (gestaltResult == gestaltPowerPC);
		}
	return( onPowerPC );
	}

#endif
	

	void
InitMacToolbox(void)
{
	UnloadScrap();
	
	InitGraf( &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();
	MaxApplZone();
}

