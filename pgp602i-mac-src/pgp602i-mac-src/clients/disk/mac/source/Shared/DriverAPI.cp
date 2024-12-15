/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: DriverAPI.cp,v 1.4.8.1 1998/11/12 03:06:04 heller Exp $____________________________________________________________________________*/#include "MacFiles.h"#include "MacMemory.h"#include "pgpMem.h"#include "MacStrings.h"#include "MacDriverUtils.h"#include "DriverAPI.h"	static voidPGPDiskInitCSParam(	ParamBlockRec *	pb,	const void *	bytes,	ushort			numBytes )	{	DriverAPIStruct *	api	= (DriverAPIStruct *)&pb->cntrlParam.csParam[ 0 ];		if ( numBytes <= sizeof( api->data ) + sizeof( api->extra ) )		{		api->magic	= DriverAPIStruct::kMagicIn;					if ( numBytes != 0 )			{			pgpAssert( sizeof( *api ) <= sizeof( pb->cntrlParam.csParam ) );			BlockMoveData( bytes, &api->data, numBytes );			}		}	else		{		pgpDebugPStr( "\pPGPDiskInitCSParam: illegal length" );		}	}enum CallType	{ kStatus, kControl };	static OSErrPGPDiskStatusOrControl(	ParamBlockRec *	pb,	const short		csCode,	const long		data,	const short		optionalDriverRefNum,	CallType		status)	{	OSErr	err	= paramErr;	short	driverRefNum	= optionalDriverRefNum;		if ( driverRefNum == 0 )		{		if ( ! DriverIsInstalled( kPGPDiskDriverName, &driverRefNum ) )			{			driverRefNum	= 0;			}		}		if ( driverRefNum != 0 )		{		pb->cntrlParam.ioCRefNum	= driverRefNum;		pb->cntrlParam.csCode		= csCode;		PGPDiskInitCSParam( pb, &data, sizeof( data ) );				if ( status == kStatus )			err	= PBStatusSync( pb );		else			err	= PBControlSync( pb );				if ( IsntErr( err ) )			{			// verify that driver understood our request			const DriverAPIStruct *	api	=					(const DriverAPIStruct *)&pb->cntrlParam.csParam[ 0 ];				if ( api->magic != DriverAPIStruct::kMagicOut )				{				err	= paramErr;				}			}		}		AssertNoErr( err, "PGPDiskStatusOrControl" );	return( err ); 	}			OSErrPGPDiskStatus(	ParamBlockRec *	pb,	const short		csCode,	const long		data,	const short		optionalDriverRefNum)	{	return( PGPDiskStatusOrControl( pb, csCode, data, optionalDriverRefNum,					kStatus ) );	}			OSErrPGPDiskControl(	ParamBlockRec *	pb,	const short		csCode,	const long		data,	const short		optionalDriverRefNum)	{	return( PGPDiskStatusOrControl( pb, csCode, data, optionalDriverRefNum,					kControl ) );	}#pragma mark -			BooleanDriverNameIsPGPDisk( ConstStr255Param	driverName )	{	Boolean	isPGPDiskDriverName	= false;		pgpAssert( pgpMemoryEqual( &kPGPDiskDriverNamePrefix[ 1 ],				&kPGPDiskDriverName[ 1 ], kPGPDiskDriverNamePrefix[ 0 ] ) );		// check the the driver name has the prefix 'kPGPDiskDriverNamePrefix'	if ( StrLength( driverName ) >= StrLength( kPGPDiskDriverNamePrefix ) &&		pgpMemoryEqual( &kPGPDiskDriverNamePrefix[ 1 ], &driverName[ 1 ],				StrLength( kPGPDiskDriverNamePrefix ) ) )		{		isPGPDiskDriverName	= TRUE;		}			return( isPGPDiskDriverName  );	} 				BooleanIsPGPDiskDriver( short driverRefNum )	{	Boolean	isPGPDiskDriver	= false;	OSErr	err	= noErr;	Str255	driverName;		err	= GetDriverNameUsingRefNum( driverRefNum, driverName );		if ( IsntErr( err ) && DriverNameIsPGPDisk( driverName ) )		{		PGPDriverInfoStruct 	info;		err	= GetPGPDiskDriverInfo( driverRefNum, &info );		isPGPDiskDriver	= ( err == noErr );		}			return( isPGPDiskDriver );	}	voidGetUniquePGPDiskDriverName( StringPtr newName )	{	ushort	suffixNumber	= 0;		CopyPString( kPGPDiskDriverName, newName );		do		{		Str255	tempStr;		short	refNum;				if ( ! DriverIsInstalled( newName, &refNum ) )			{			break;			}				CopyPString( kPGPDiskDriverNamePrefix, newName );		++suffixNumber;		NumToString( suffixNumber, tempStr );		AppendPString( tempStr, newName );				} while ( TRUE );	}	BooleanDriveIsPGPDisk( short	driveNumber )	{	pgpAssert( driveNumber > 0 );		short		driverRefNum;			GetDriverRefNumForDrive( driveNumber, &driverRefNum );			return( IsPGPDiskDriver( driverRefNum ) );	}	BooleanVolumeIsPGPDisk( short	vRefNum )	{	short		driveNumber;	short		driverRefNum;	OSStatus	err	= noErr;		err	= GetVolumeDriveNumberAndRefNum( vRefNum, &driveNumber,				&driverRefNum );		return( IsntErr( err ) && IsPGPDiskDriver( driverRefNum ) );	}	static OSErrSendIOOptionsToDriverForVolume(	short				vRefNum,	IOOptionsStruct	*	options)	{	pgpAssert( vRefNum < 0 );	OSErr	err	= noErr;		if ( VolumeIsPGPDisk( vRefNum ) )		{		short			driverRefNum;		OSErr			err	= noErr;		ParamBlockRec	pb;				GetVolumeDriveNumberAndRefNum( vRefNum, &options->driveNumber,				&driverRefNum );				err	= PGPDiskControl( &pb, kSetIOOptionsCC, (long)options,					driverRefNum);		AssertNoErr( err, "ChangeDriverMaxReadMaxBlocks" );		}			return( err );	}		OSErrChangeDriveTuner(	short						vRefNum,	const TunerStruct *			tuner,	IOOptionsStruct::TunerType	tunerType)	{	IOOptionsStruct	options;	OSErr			err	= noErr;		pgpClearMemory( &options, sizeof( options ) );		options.tunerType		= tunerType;	options.tuner			= *tuner;		err	= SendIOOptionsToDriverForVolume( vRefNum, &options );		return( err );	}		OSErrGetPGPDiskDriverInfo(	short					driverRefNum,	PGPDriverInfoStruct *	info )	{	OSErr			err;	ParamBlockRec	pb;		err	= PGPDiskStatus( &pb, kGetDriverInfoSC, (long)info, driverRefNum);		return( err );	}				OSErrGetPGPDriveInfo(	short					driveNumberOrVRefNum,	PGPDriveInfoStruct *	driveInfo )	{	OSErr			err	= noErr;	ParamBlockRec	pb;	short			driverRefNum;	short			driveNumber;		pgpAssert( driveNumberOrVRefNum != 0 );		if ( driveNumberOrVRefNum < 0 )		{		// it's a vRefNum		GetVolumeDriveNumberAndRefNum( driveNumberOrVRefNum, &driveNumber,				&driverRefNum );		}	else		{		// it's a drive number		driveNumber	= driveNumberOrVRefNum;		err	= GetDriverRefNumForDrive( driveNumber, &driverRefNum );		}		if ( IsntErr( err ) )		{		driveInfo->driveNumberIn	= driveNumber;				err	= PGPDiskStatus( &pb, kGetDriveInfoSC, (long)driveInfo,					driverRefNum);		}		return( err );	}		OSStatusChangeWriteBufferSize(	short	driverRefNum,	ulong	bufferSize )	{	OSStatus		err	= noErr;	ParamBlockRec	pb;		err	= PGPDiskControl( &pb, kChangeWriteBufferSizeCC, (long)bufferSize,				driverRefNum);		return( err );	}					OSStatusPGPDriverRemoveDrive(	short	driverRefNum,	short	driveNumber )	{	OSStatus		err	= noErr;	ParamBlockRec	pb;		err	= PGPDiskControl( &pb, kRemoveDriveCC, (long)driveNumber,				driverRefNum);		return( err );	}				OSStatusSendPreferencesToDriver(	short						driverRefNum,	const PGPDiskDriverPrefs 	*driverPrefs)	{	OSStatus		err	= noErr;	ParamBlockRec	pb;		pgpAssertAddrValid( driverPrefs, PGPDiskDriverPrefs );		err	= PGPDiskControl( &pb, kSetPrefsCC, (long) driverPrefs,				driverRefNum );		return( err );	}	OSStatusPGPDiskDriveUnmountedInProcess(	short				driverRefNum,	short				driveNumber,	ProcessSerialNumber	unmountingAppPSN)	{	OSStatus		err	= noErr;	ParamBlockRec	pb;	PGPDriveUnmountedStruct	info;		info.driveNumber		= driveNumber;	info.unmountingProcess	= unmountingAppPSN;		err	= PGPDiskControl( &pb, kDriveUnmountedCC, (long) &info,				driverRefNum );		return( err );	}	