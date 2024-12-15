/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacInternet.c,v 1.4 1997/09/18 01:34:48 lloyd Exp $
____________________________________________________________________________*/

#include <AppleEvents.h>

#include "MacDesktop.h"
#include "MacFiles.h"
#include "MacInternet.h"
#include "MacProcesses.h"
#include "pgpMem.h"

enum
{
	kOpenURLEventClass	= 'WWW!',		/* These are Mac "standards" */
	kOpenURLEventID		= 'OURL'
};

static OSType	sBrowserCreators[] = {	'MOSS',		/* Netscape */
										'MSIE' };	/* Internet Explorer */

	static OSStatus
SendURLToProcess(
	ConstStr255Param 	theURL,
	ProcessSerialNumber	appPSN)
{
	AEDesc		addressDesc;
	OSStatus	status;
	
	status = AECreateDesc( typeProcessSerialNumber, &appPSN, sizeof( appPSN ),
									&addressDesc );
	if( IsntErr( status ) )
	{
		AppleEvent	theAppleEvent;
		
		status = AECreateAppleEvent( kOpenURLEventClass,
					kOpenURLEventID, &addressDesc,
					kAutoGenerateReturnID, kAnyTransactionID, &theAppleEvent );
		if( IsntErr( status ) )
		{
			status = AEPutParamPtr( &theAppleEvent, keyDirectObject, typeChar,
								&theURL[1], theURL[0] );
			if( IsntErr( status ) )
			{
				AppleEvent	replyEvent;
				
				status = AESend( &theAppleEvent,
						&replyEvent, kAENoReply | kAECanInteract,
						kAENormalPriority, kAEDefaultTimeout, nil, nil );
				if( IsntErr( status ) )
				{
					SetFrontProcess( &appPSN );
				}
			}
			
			(void) AEDisposeDesc( &theAppleEvent );
		}
		
		(void) AEDisposeDesc( &addressDesc );
	}

	return( status );
}

	static OSStatus
SendURLToActiveBrowser(ConstStr255Param theURL)
{
	UInt32				browserIndex;
	UInt32				numBrowsers;
	ProcessSerialNumber	browserPSN;
	OSStatus			status;
	
	pgpAssertAddrValid( theURL, uchar );

	numBrowsers 	= sizeof( sBrowserCreators )/sizeof( sBrowserCreators[0] );
	status			= fnfErr;
		
	for( browserIndex = 0; browserIndex < numBrowsers; browserIndex++ )
	{
		if( FindProcess( 'APPL', sBrowserCreators[browserIndex], &browserPSN,
				nil, nil, nil ) )
		{
			status = SendURLToProcess( theURL, browserPSN );
			break;
		}
	}
	
	return( status );
}

	static OSStatus
SendURLToInactiveBrowser(ConstStr255Param theURL)
{
	UInt32		browserIndex;
	UInt32		numBrowsers;
	Boolean		foundBrowser;
	OSStatus	status;
	FSSpec		browserSpec;
	OSType		browserCreator;
	
	pgpAssertAddrValid( theURL, uchar );

	numBrowsers = sizeof( sBrowserCreators ) / sizeof( sBrowserCreators[0] );
	foundBrowser 	= FALSE;
	status 			= fnfErr;
	
	/* Try the full browser list on local volumes only,
		then retry the list for all volumes (local and remote).
		This means well find a locat Internet Explorer before
		a remote Navigator. */
	for( browserIndex = 0; browserIndex < numBrowsers; browserIndex++ )
	{
		if( FindApplication( 'APPL', sBrowserCreators[browserIndex],
						kFindAppOnLocalVolumes, &browserSpec ) )
		{
			foundBrowser 	= TRUE;
			browserCreator	= sBrowserCreators[browserIndex];
			break;
		}
	}

	if( ! foundBrowser )
	{
		for( browserIndex = 0; browserIndex < numBrowsers; browserIndex++ )
		{
			if( FindApplication( 'APPL', sBrowserCreators[browserIndex],
							kFindAppOnRemoteVolumes, &browserSpec ) )
			{
				foundBrowser 	= TRUE;
				browserCreator	= sBrowserCreators[browserIndex];
				break;
			}
		}
	}
	
	if( foundBrowser )
	{
		LaunchParamBlockRec	launchPB;

		pgpClearMemory( &launchPB, sizeof( launchPB ) );
	
		launchPB.launchBlockID			= extendedBlock;
		launchPB.launchEPBLength		= extendedBlockLen;
		launchPB.launchFileFlags		= 0;
		launchPB.launchControlFlags		= launchContinue | launchNoFileFlags;
		launchPB.launchAppSpec			= &browserSpec;
		launchPB.launchAppParameters	= nil;
		
		status = LaunchApplication( &launchPB );
		if( IsntErr( status ) )
		{
			status = SendURLToProcess( theURL, launchPB.launchProcessSN );
		}
	}
	
	return( status );
}

	OSStatus
OpenURL(ConstStr255Param theURL)
{
	OSStatus	status;
	
	pgpAssertAddrValid( theURL, uchar );
	
	status = SendURLToActiveBrowser( theURL );
	if( IsErr( status ) )
	{
		status = SendURLToInactiveBrowser( theURL );
	}

	return( status );
}