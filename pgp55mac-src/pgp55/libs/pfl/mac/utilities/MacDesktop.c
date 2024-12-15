/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDesktop.c,v 1.6 1997/08/24 09:55:47 jason Exp $
____________________________________________________________________________*/

#include "MacDesktop.h"
#include "MacFiles.h"
#include "MacProcesses.h"
#include "pgpMem.h"

/* applicationType is usually 'APPL',
but could be different. Use '****' to wildcard*/
	Boolean
FindApplicationOnVolume(
	short 	vRefNum,
	OSType 	applicationType,	
	OSType 	applicationCreator,
	FSSpec 	*appSpec)
{
	DTPBRec		dtpb;
	OSStatus	status = noErr;
	Str255		name;
	
	/* Note: If multiple copies are present,
	this function will return the application
		 with the latest creation date. */
	
	pgpAssertAddrValid( appSpec, FSSpec );
	
	pgpClearMemory( &dtpb, sizeof( dtpb ) );
	
	name[0] = 0;
	
	dtpb.ioNamePtr	= name;
	dtpb.ioVRefNum	= vRefNum;
	
	status = PBDTGetPath( &dtpb );
	if( IsntErr( status ) )
	{
		short	dtRefNum;
		UInt32	bestCreationDate = 0;
		FSSpec	bestAppSpec;
		UInt32	dtIndex;
		
		dtRefNum 			= dtpb.ioDTRefNum;
		dtIndex				= 1;
		bestCreationDate 	= 0;
		
		while( TRUE )
		{
			dtpb.ioCompletion	= nil;
			dtpb.ioNamePtr		= name;
			dtpb.ioDTRefNum		= dtRefNum;
			dtpb.ioIndex		= dtIndex;
			dtpb.ioFileCreator	= applicationCreator;
			
			if( IsErr( PBDTGetAPPLSync( &dtpb ) ) )
				break;
				
			if( dtpb.ioTagInfo > bestCreationDate )
			{
				FSSpec	checkSpec;
				
				/* check that the file truly exists and is the correct type */
				if( IsntErr( FSMakeFSSpec( vRefNum,
							dtpb.ioAPPLParID, name, &checkSpec ) ) )
				{
					CInfoPBRec	cpb;
					
					if( IsntErr( FSpGetCatInfo( &checkSpec, &cpb ) ) )
					{
						if( cpbIsFile( &cpb ) &&
							cpbFileCreator( &cpb ) == applicationCreator &&
							( cpbFileType( &cpb ) == applicationType ||
								applicationType == '****' ) )
						{
							bestCreationDate 	= dtpb.ioTagInfo;
							bestAppSpec			= checkSpec;
						}
					}
				}
			}
				
			++dtIndex;
		}
		
		if( bestCreationDate != 0 )
		{
			*appSpec 	= bestAppSpec;
			status		= noErr;
		}
		else
		{
			status = fnfErr;
		}
	}
	
	return( IsntErr( status ) );
}

	static Boolean
RealFindApplication(
	OSType 	applicationType,
	OSType 	applicationCreator,
	Boolean	doLocalVolumes,
	FSSpec 	*appSpec)
{
	Boolean			foundApp;
	HParamBlockRec	pb;
	short			volIndex;
	Str255			volumeName;
	OSStatus		err	= noErr;
	
	pgpClearMemory( &pb, sizeof( pb ) );
	
	volIndex = 1;
	foundApp = FALSE;
	
	while( TRUE )
	{
		pb.volumeParam.ioNamePtr	= volumeName;
		pb.volumeParam.ioVolIndex	= volIndex;
		 
		if( IsntErr( PBHGetVInfoSync( &pb ) ) )
		{
			Boolean	isRemote;

			err	= VolumeIsRemote( pb.volumeParam.ioVRefNum, &isRemote );
			if( IsntErr( err ) )
			{
				if( ( isRemote && ! doLocalVolumes ) ||
					( doLocalVolumes && ! isRemote ) )
				{
					foundApp = FindApplicationOnVolume(
									pb.volumeParam.ioVRefNum,
									applicationType, applicationCreator,
									appSpec );
					if( foundApp )
					{
						break;
					}
				}
			}
		}
		else
		{
			/* No more volumes */
			break;
		}
		
		++volIndex;
	}
	
	return( foundApp );
}

	Boolean
FindApplication(
	OSType 				applicationType,	/* **** wildcards */
	OSType 				applicationCreator,
	FindAppSearchType	searchType,
	FSSpec 				*appSpec)
{
	Boolean		foundApp = FALSE;

	/* Always try local volumes first. */
	if( searchType == kFindAppOnLocalVolumes ||
		searchType == kFindAppOnAllVolumes )
	{
		foundApp = RealFindApplication( applicationType,
			applicationCreator, TRUE, appSpec );
	}
	
	if( ! foundApp )
	{
		if( searchType == kFindAppOnRemoteVolumes ||
				searchType == kFindAppOnAllVolumes )
		{
			foundApp = RealFindApplication( applicationType,
				applicationCreator, FALSE, appSpec );
		}
	}
	
	return( foundApp );
}


	OSStatus
BringAppToFront(
	OSType				type,
	OSType				creator,
	FindAppSearchType	searchType,
	Boolean				launch,
	AppParametersPtr	params,
	Boolean				*wasLaunched)
{
	ProcessSerialNumber	psn;
	OSStatus			err = fnfErr;

	if ( wasLaunched != nil )
	{
		*wasLaunched = false;
	}
	if( FindProcess(type, creator, &psn, nil, nil, nil ))
	{
		err = SetFrontProcess( &psn );
	}
	else if ( launch )
	{
		FSSpec	appSpec;
		
		if( FindApplication(type, creator, searchType, &appSpec) )
		{
			LaunchParamBlockRec	launchParams = {	0,
									0,
									extendedBlock,
									extendedBlockLen,
									0,
									launchContinue | launchNoFileFlags,
									0,
									0,
									0,
									0,
									0,
									nil };
									
			launchParams.launchAppSpec = &appSpec;
			launchParams.launchAppParameters = params;
			
			err = LaunchApplication(&launchParams);
			if( (wasLaunched != nil) && (err == noErr) )
			{
				*wasLaunched = true;
			}
		}
	}
	
	return err;
}