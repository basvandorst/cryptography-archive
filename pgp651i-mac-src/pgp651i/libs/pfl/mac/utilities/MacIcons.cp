/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.____________________________________________________________________________*/#include <Icons.h>#include "pgpMem.h"#include "MacIcons.h"	OSErrDuplicateIconSuite(	Handle		inFromSuite,	Handle *	outToSuite){	OSErr 		err = noErr;	static const OSType sIconTypes[] =		{		kLarge1BitMask,		kLarge4BitData,		kLarge8BitData,		kSmall1BitMask,		kSmall4BitData,		kSmall8BitData,		kMini1BitMask,		kMini4BitData,		kMini8BitData		};	const PGPUInt16	kNumTypes	=		sizeof( sIconTypes ) / sizeof( sIconTypes[ 0 ] );	*outToSuite = nil;		// Create new icon suite	// *** WE SUSPEND AND RESUME LEAK CHECKING HERE BECAUSE OUR LEAKS CODE DOESN'T HANDLE	// *** CROSSING LIB BOUNDARIES AND THIS IS EXPORTED FROM CLIENTLIBpgpLeaksSuspend();	err = NewIconSuite(outToSuite);pgpLeaksResume();	if ( IsntErr( err ) )	{		PGPUInt16 index;				// Duplicate icons and add them to the new suite		for ( index = 0; index < kNumTypes; ++index )		{			Handle 		theDataH;				if ( GetIconFromSuite( &theDataH,					inFromSuite, sIconTypes[ index ]) == noErr )			{				if ( IsntNull( theDataH ) )				{pgpLeaksSuspend();					err	= HandToHand( &theDataH );pgpLeaksResume();					if ( IsErr( err ) )						break;					AddIconToSuite( theDataH,						*outToSuite, sIconTypes[ index ]);				}			}		}	}		if ( IsntErr( err ) )	{		SetSuiteLabel( *outToSuite, GetSuiteLabel(inFromSuite) );	}	else	{		if ( IsntNull( *outToSuite ) )		{			DisposeIconSuite( *outToSuite, TRUE );		}	}		return err;}