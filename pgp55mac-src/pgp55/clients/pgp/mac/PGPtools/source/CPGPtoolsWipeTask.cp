/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <UDesktop.h>

#include "MacMemory.h"
#include "MacStrings.h"
#include "MacFiles.h"
#include "CMacFSWiper.h"

#include "pgpUtilities.h"

#include "PGPtoolsUtils.h"
#include "PGPtoolsResources.h"

#include "CPGPtools.h"
#include "CPGPtoolsTaskProgressDialog.h"
#include "CPGPtoolsWipeTask.h"

	inline UInt32
RoundUp(
	UInt32	num,
	UInt32	factor )
{
	return( ( (num + (factor -1)) / factor ) * factor );
}


/* scale byte counts to avoid overflow on really large wipes */
#define kProgressScaleFactor 	1024UL
	UInt32
CPGPtoolsWipeTask::GetScaledProgress( PGPUInt64 num )
{
	if ( num > mTotalBytesToWipe )
		num	= mTotalBytesToWipe;
	
	if ( mTotalBytesToWipe <= MAX_PGPUInt32 )
	{
		return( (UInt32)num );
	}
	
	/* scale by factor of one million */
	return( (UInt32)(num / (1UL << 20)) );
}

	
	
CPGPtoolsWipeTask::CPGPtoolsWipeTask( const FSSpec *spec ) 
		: CPGPtoolsTask( kPGPtoolsWipeOperation )
{
	AssertSpecIsValid( spec, "CPGPtoolsWipeTask::CPGPtoolsWipeTask" );
	
	mWipeSpec	= *spec;
	mProgress	= NULL;
}

CPGPtoolsWipeTask::~CPGPtoolsWipeTask(void)
{
}



	OSStatus
CPGPtoolsWipeTask::CalcProgressBytes(
	PGPContextRef		context,
	ByteCount *			progressBytes)
{
	OSStatus	err;
	
	(void) context;
	
	err	= Prescan();
	if ( IsntErr( err ) )
	{
		*progressBytes	= GetScaledProgress( mTotalBytesToWipe );
	}
	
	return( err  );
}


	void
CPGPtoolsWipeTask::GetTaskItemName(StringPtr name)
{
	CopyPString( mWipeSpec.name, name );
}


	CToolsError
CPGPtoolsWipeTask::Wipe(
	CPGPtoolsTaskProgressDialog *	progressDialogObj )
{
	CToolsError	err;
	
	mProgress	= progressDialogObj;
	mProgress->SetProgressOperation( kPGPtoolsWipeOperation );
	
	err	= WipeSpec( &mWipeSpec );
	
	if( ShouldReportError( err ) )
	{
		err = ReportError( err, kErrorWhileWipingStrIndex );
	}
	
	return( err );
}


	PGPUInt64
CPGPtoolsWipeTask::CalcFileSize( const CInfoPBRec *cpb )
{
	PGPUInt64	resSize		= cpbResForkSize( cpb );
	PGPUInt64	dataSize	= cpbDataForkSize( cpb );
	UInt32	allocBlockSize	= mAllocBlockSize;
	
	resSize		= RoundUp( resSize, allocBlockSize );
	dataSize	= RoundUp( dataSize, allocBlockSize );
	
	return( resSize + dataSize );
}

/*____________________________________________________________________________
	Add a file's info to our prescan info.
____________________________________________________________________________*/
	void
CPGPtoolsWipeTask::PrescanFile( const CInfoPBRec *	cpb)
{
	mTotalBytesToWipe	+= CalcFileSize( cpb );
	
	++mTotalFilesToWipe;
	
	if ( cpbIsLocked( cpb ) )
		++mNumLockedFiles;
		
	if ( cpbIsOpen( cpb ) )
		++mNumOpenFiles;
}


/*____________________________________________________________________________
	Recursively scan the spec and its subfolders, though spec could be a file.
____________________________________________________________________________*/
	OSStatus
CPGPtoolsWipeTask::PrescanFolder( 
	short		vRefNum,
	long		dirID )
{
	CInfoPBRec	cpb;
	Str63		name;
	OSStatus	err;
	
	cpb.hFileInfo.ioVRefNum		= vRefNum;
	cpb.hFileInfo.ioDirID		= dirID;
	cpb.hFileInfo.ioNamePtr		= name;
	cpb.hFileInfo.ioFDirIndex	= 1;
	while ( TRUE )
	{
		err	= PBGetCatInfoSync( &cpb );
		if ( IsErr( err ) )
		{
			err	= noErr;
			break;
		}
		
		if ( cpbIsFolder( &cpb ) )
		{
			err	= PrescanFolder( vRefNum, cpb.dirInfo.ioDrDirID );
		}
		else
		{
			PrescanFile( &cpb );
		}
		
		if ( IsErr( err ) )
			break;
		
		cpb.dirInfo.ioFDirIndex	+= 1;
		cpb.hFileInfo.ioDirID	= dirID;
	}
	
	return( err );
}


	OSStatus
CPGPtoolsWipeTask::Prescan(  )
{
	OSStatus	err;
	CInfoPBRec	cpb;
	
	mTotalBytesToWipe	= 0;
	mTotalFilesToWipe	= 0;
	mNumLockedFiles		= 0;
	mNumOpenFiles		= 0;
	mCurFileBytesWiped	= 0;
	
	mAllocBlockSize	= GetVolumeAllocBlockSize( mWipeSpec.vRefNum );

	err	= FSpGetCatInfo( &mWipeSpec, &cpb );
	if ( IsntErr( err ) )
	{
		if ( cpbIsFile( &cpb ) )
		{
			PrescanFile( &cpb );
		}
		else if( cpb.dirInfo.ioDrDirID != fsRtDirID )
		{
			err	= PrescanFolder( mWipeSpec.vRefNum,
						cpb.dirInfo.ioDrDirID );
		}
	}
	
	return( err );
}



	OSStatus
CPGPtoolsWipeTask::WipeHook(
	ConstStringPtr	fileName,
	PGPUInt64		additionalDone)
{
	OSStatus	err;
	
	/* we have our own totals across multiple files */
	mCurFileBytesWiped		+= additionalDone;
	
	(void)fileName;
	err	= DoProgress( mProgress,
			GetScaledProgress( mCurFileBytesWiped ),
			GetScaledProgress( mTotalBytesToWipe) );
	
	return( err );
}



class CMyWiper : public CMacFSWiper
{
	CPGPtoolsWipeTask *	mTask;
	
public:
	CMyWiper( CPGPtoolsWipeTask *task )
	{
		mTask	= task;
	}
	
		virtual OSStatus
	Progress(  ConstStringPtr fileName,
				PGPUInt64 additionalDone )
	{
		OSStatus	err;
		
		err	= inherited::Progress( fileName, additionalDone );
		if ( IsntErr( err ) )
			err	= mTask->WipeHook( fileName, additionalDone );
		return( err );
	}
};



	OSStatus
CPGPtoolsWipeTask::WipeFile( const FSSpec *spec )
{
	OSStatus		err;
	CMyWiper		wiper( this );
	
	/* this takes care of accumulating each files progress count */
	mProgress->SetNewProgressItem( spec->name );
	mCurFileBytesWiped	= 0; 
	
	/* wipe the data */
	err	= wiper.FSpWipeAndDelete( spec );
	
	return( err );
}


	OSStatus
CPGPtoolsWipeTask::WipeFolder(
	short		vRefNum,
	long		dirID )
{
	CInfoPBRec	cpb;
	FSSpec		spec;
	OSStatus	err;
	
	spec.vRefNum	= vRefNum;
	spec.parID		= dirID;
	
	cpb.hFileInfo.ioVRefNum		= vRefNum;
	cpb.hFileInfo.ioDirID		= dirID;
	cpb.hFileInfo.ioNamePtr		= spec.name;
	cpb.hFileInfo.ioFDirIndex	= 1;
	while ( TRUE )
	{
		err	= PBGetCatInfoSync( &cpb );
		if ( IsErr( err ) )
		{
			err	= noErr;
			break;
		}
		
		if ( cpbIsFolder( &cpb ) )
		{
			err	= WipeFolder( vRefNum, cpb.dirInfo.ioDrDirID );
		}
		else
		{
			err	= WipeFile( &spec );
		}
		
		if ( IsErr( err ) )
			break;
		
		/* leave ioFDirIndex at 1; we are continually deleting the first */
		cpb.hFileInfo.ioDirID	= dirID;
	}

	/* erase the folder itself */
	if ( IsntErr( err ) )
	{
		Boolean		wipeTheFolder	= TRUE;
		
		err	= FSMakeFSSpec( vRefNum, dirID, "\p", &spec );
		if ( IsntErr( err ) )
		{
			/* if it's the trash folder itself, don't delete it */
			wipeTheFolder	= ! FSIsTrashFolder( vRefNum, dirID );
		}
		
		if ( IsntErr( err ) && wipeTheFolder )
		{
			uchar	tempName[ kMaxHFSFileNameLength + 1];
			
			/* first, erase the folder name */
			pgpFillMemory( tempName, sizeof( tempName ), 'x' );
			tempName[ 0 ]	= kMaxHFSFileNameLength;
			
			err	= FSpRename( &spec, tempName );
			if ( IsntErr( err ) )
			{
				/* terminat the folder */
				CopyPString( tempName, spec.name );
				err	= FSpDelete( &spec );
			}
		}
	}
	
	FlushVol( nil, vRefNum );
	
	return( err );
}


	CToolsError
CPGPtoolsWipeTask::WipeSpec( const FSSpec *spec )
{
	CToolsError	err;
	CInfoPBRec	cpb;
	
	err.err	= FSpGetCatInfo( spec, &cpb );
	if ( err.IsntError() )
	{
		if ( cpbIsFile( &cpb ) )
		{
			err.err	= WipeFile( spec );
		}
		else if( cpb.dirInfo.ioDrDirID != fsRtDirID )
		{
			err.err	= WipeFolder( spec->vRefNum,
						cpb.dirInfo.ioDrDirID );
		}
	}
	
	return( err );
}











