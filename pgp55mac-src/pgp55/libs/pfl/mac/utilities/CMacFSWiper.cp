/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CMacFSWiper.cp,v 1.4 1997/09/18 01:34:38 lloyd Exp $
____________________________________________________________________________*/
#include "MacFiles.h"
#include "MacMemory.h"
#include "MacStrings.h"
#include "CMacFSWiper.h"



#define MIN(a, b )		( (a) <= (b) ? (a) : (b) )

			
				
	OSStatus
CMacFSWiper::FSWipeInternal(
	short					refNum,
	UInt32					wipeLength,
	const void *			pattern,
	UInt32					patternLength)
{
	AssertFileRefNumIsValid( refNum, "FSWipeInternal" );
	pgpAssert( patternLength != 0 );
	pgpAssertAddrValid( pattern, char );
	
	OSStatus	err	= noErr;
	void *		wipeBuffer;
	UInt32		wipeBufferLength;
	
	if ( wipeLength != 0 )
	{
		SetFPos( refNum, fsFromStart, 0 );
		
		// wipe buffer must be at least as long as the pattern or
		// the wipe length in order
		// to satisfy the semantics; not acceptable to not use the
		// whole pattern
		wipeBufferLength	= MIN( wipeLength, 512UL * 1024UL );
		wipeBuffer	= pgpNewPtrMost( wipeBufferLength,
						MIN( patternLength, wipeLength ),
						kMacMemory_PreferTempMem | kMacMemory_UseCurrentHeap,
						&wipeBufferLength );
		if ( IsntNull( wipeBuffer ) )
		{
			UInt32	remaining	= wipeLength;
			
			pgpCopyPattern( pattern, patternLength,
				wipeBuffer, wipeBufferLength );
			
			while ( remaining != 0 )
			{
				long	count	= MIN( wipeBufferLength, remaining );
				
				err	= FSWriteUncached( refNum, &count, wipeBuffer );
				if ( IsErr( err ) )
					break;
				
				remaining	-= count;
				
				err	= Progress( mWipeSpec.name, count );
				if ( IsErr( err ) )
					break;
			}
			
			pgpFreeMac( wipeBuffer );

			FlushFile( refNum );
		}
		else
		{
			err	= memFullErr;
		}
	}
	
	return( err );
}



	OSStatus
CMacFSWiper::FSWipe(
	short					refNum,
	const void *			pattern,
	UInt32					patternLength)
{
	OSStatus	err	= noErr;
	long		originalEOF;
	FSSpec		spec;
	UInt32	allocBlockSize;
	
	err	= GetSpecFromRefNum( refNum, &spec );
	if ( IsntErr( err ) )
		err	= GetVolumeBlockCounts( spec.vRefNum, nil, nil, &allocBlockSize );
	if ( IsntErr( err ) )
		err	= GetEOF( refNum, &originalEOF );
		
	if ( IsntErr( err ) && originalEOF != 0 )
	{
		UInt32	wipeLength;
		
		// wipe length should round up to next allocation block
		wipeLength	= ( originalEOF / allocBlockSize ) * allocBlockSize;
		if ( ( originalEOF % allocBlockSize ) != 0 )
		{
			wipeLength	+= allocBlockSize;
		}
		
		err	= FSWipeInternal( refNum, wipeLength, pattern,  patternLength );
		
		(void)SetEOF( refNum, originalEOF );
	}
	
	return( err );
}



	OSStatus
CMacFSWiper::FSpWipeFork(
	const FSSpec *			spec,
	ForkKind				whichFork,
	const void *			pattern,
	UInt32					patternLength)
{
	OSStatus	err	= noErr;
	short		fileRef;
	
	err	= FSpOpenFork( spec, fsRdWrPerm, &fileRef, whichFork );
	if ( IsntErr( err ) )
	{
		err	= FSWipe( fileRef, pattern, patternLength );
		
		(void)FSClose( fileRef );
	}
	return( err );
}


	OSStatus
CMacFSWiper::FSpWipe(
	const FSSpec *			spec,
	const void *			pattern,
	UInt32					patternLength )
{
	OSStatus	err	= noErr;
	CInfoPBRec	cpb;
	
	err = FSpGetCatInfo( spec, &cpb );
	if( IsntErr( err ) )
	{
		if( ( cpbFileAttributes( &cpb ) & kFileAttributeFileOpenMask ) == 0 )
		{
			err	= FSpWipeFork( spec, kDataFork, pattern, patternLength );
			if ( IsntErr( err ) )
			{
				err	= FSpWipeFork( spec, kResourceFork,
						pattern, patternLength );
			}
		}
		else
		{
			err = fBsyErr;
		}
	}
	
	return( err );
}


	OSStatus
CMacFSWiper::FSpWipeAndDelete( const FSSpec *	spec)
{
	OSStatus		err; 
	char			pattern[ 256 ];
	
	mTotalDone	= 0;
	mWipeSpec	= *spec;
	
	for( UInt32 index = 0; index < sizeof( pattern ); ++index )
	{
		pattern[ index ]	= index;
	}
	
	/* wipe the data */
	err	= FSpWipe( spec, pattern, sizeof( pattern ) );
	if ( IsntErr( err ) )
	{
		uchar	bogusName[ kMaxHFSFileNameLength + 1 ];
	
		/* wipe the name (50/50 chance of this doing what we want) */
		pgpFillMemory( bogusName, '-', sizeof( bogusName ) );
		bogusName[ 0 ]	= kMaxHFSFileNameLength;
		err	= FSpRename( spec, bogusName );
		if ( IsntErr( err ) )
		{
			FSSpec	tempSpec;
			
			tempSpec	= *spec;
			CopyPString( bogusName, tempSpec.name );
			/* terminate the file */
			err	= FSpDelete( &tempSpec );
		}
	}
	
	/* caller is responsible for flushing volume */
	
	return( err );
}



CMacFSWiper::CMacFSWiper()
{
	MacDebug_FillWithGarbage( &mWipeSpec, sizeof( mWipeSpec ) );
	mTotalDone	= 0;
}

/*____________________________________________________________________________
	To get progress while wiping, subclass and override Progress().
____________________________________________________________________________*/
	OSStatus
CMacFSWiper::Progress(
	ConstStringPtr	fileName,
	PGPUInt64		additionalDone )
{
	(void)fileName;
	
	mTotalDone	+= additionalDone;
	
	return( noErr );
}




























/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/