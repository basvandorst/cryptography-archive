/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CMacFSWiper.cp,v 1.10.2.1.4.1 1998/11/12 03:19:22 heller Exp $____________________________________________________________________________*/#include <Errors.h>#include "MacFiles.h"#include "pgpMacMemory.h"#include "MacStrings.h"#include "CMacFSWiper.h"#define MIN(a, b )		( (a) <= (b) ? (a) : (b) )	OSStatusCMacFSWiper::FSWipeInternal(	short					refNum,	UInt32					wipeLength,	PGPUInt32				numPasses,	PGPUInt32 				numPatterns,	const PGPWipingPattern 	patternList[],	void 					*wipeBuffer,	PGPSize 				wipeBufferLength){	AssertFileRefNumIsValid( refNum, "FSWipeInternal" );	pgpAssertAddrValid( patternList, PGPWipingPattern );	pgpAssert( wipeBufferLength != 0 );	pgpAssert( wipeBufferLength % sizeof( PGPWipingPattern ) == 0 );		OSStatus	err	= noErr;		if ( wipeLength != 0 )	{		PGPUInt32	passIndex;				(void) FlushFile( refNum );				for( passIndex = 0; passIndex < numPasses; passIndex++ )		{			const PGPWipingPattern	*curPattern;			PGPUInt32				bytesRemaining;						curPattern = &patternList[ passIndex % numPatterns ];						pgpCopyPattern( &curPattern->bytes[0], sizeof( *curPattern ),					wipeBuffer, wipeBufferLength );								SetFPos( refNum, fsFromStart, 0 );					bytesRemaining = wipeLength;			while( bytesRemaining != 0 )			{				long	count	= MIN( wipeBufferLength, bytesRemaining );								err	= FSWriteUncached( refNum, &count, wipeBuffer );				if ( IsErr( err ) )					break;								bytesRemaining -= count;								err	= Progress( mWipeSpec.name, count );				if ( IsErr( err ) )					break;			}						FlushFile( refNum );		}	}		return( err );}	OSStatusCMacFSWiper::FSWipe(	short					refNum,	PGPUInt32				numPasses,	PGPUInt32 				numPatterns,	const PGPWipingPattern 	patternList[],	void 					*wipeBuffer,	PGPSize 				wipeBufferLength){	OSStatus	err	= noErr;	long		originalEOF;	UInt32		allocBlockSize;		err	= GetVolumeBlockCounts( mWipeSpec.vRefNum, nil, nil, &allocBlockSize );	if ( IsntErr( err ) )		err	= GetEOF( refNum, &originalEOF );			if ( IsntErr( err ) && originalEOF != 0 )	{		UInt32	wipeLength;				// wipe length should round up to next allocation block		wipeLength = ( originalEOF + allocBlockSize - 1 ) / allocBlockSize *							allocBlockSize;				err	= FSWipeInternal( refNum, wipeLength, numPasses, numPatterns,					patternList, wipeBuffer, wipeBufferLength );				(void)SetEOF( refNum, originalEOF );	}		return( err );}	OSStatusCMacFSWiper::FSpWipeFork(	const FSSpec *			spec,	ForkKind				whichFork,	PGPUInt32				numPasses,	PGPUInt32 				numPatterns,	const PGPWipingPattern 	patternList[],	void 					*wipeBuffer,	PGPSize 				wipeBufferLength){	OSStatus	err	= noErr;	short		fileRef;		err	= FSpOpenFork( spec, fsRdWrPerm, &fileRef, whichFork );	if ( IsntErr( err ) )	{		err	= FSWipe( fileRef, numPasses, numPatterns, patternList, wipeBuffer,						 wipeBufferLength );				(void) SetEOF( fileRef, 0 );		(void) FSClose( fileRef );	}		return( err );}	OSStatusCMacFSWiper::FSpWipe(	const FSSpec *			spec,	PGPUInt32				numPasses,	PGPUInt32 				numPatterns,	const PGPWipingPattern 	patternList[],	void 					*wipeBuffer,	PGPSize 				wipeBufferLength){	OSStatus	err	= noErr;	CInfoPBRec	cpb;		err = FSpGetCatInfo( spec, &cpb );	if( IsntErr( err ) )	{		if( ( cpbFileAttributes( &cpb ) & kFileAttributeFileLockedMask ) != 0 )		{			err = fLckdErr;		}		else if( ( cpbFileAttributes( &cpb ) & kFileAttributeFileOpenMask ) == 0 )		{			Boolean	allocatedBuffer = FALSE;				if( IsNull( wipeBuffer ) ||				wipeBufferLength < sizeof( PGPWipingPattern ) )			{				UInt32	bufferSize = 512L * 1024L;								wipeBuffer = pgpNewPtrMost( bufferSize,								sizeof( PGPWipingPattern ),								kMacMemory_UseApplicationHeap |								kMacMemory_UseTempMem |								kMacMemory_PreferTempMem, &bufferSize );				if( IsntNull( wipeBuffer ) )				{					allocatedBuffer 	= TRUE;					wipeBufferLength	= bufferSize;				}				else				{					err = memFullErr;				}			}						if( IsntErr( err ) )			{				// The wiping buffer must be a multiple of the pattern size								wipeBufferLength = ( wipeBufferLength / 										sizeof( PGPWipingPattern ) ) *										sizeof( PGPWipingPattern );				err	= FSpWipeFork( spec, kDataFork, numPasses, numPatterns,								patternList, wipeBuffer, wipeBufferLength );				if ( IsntErr( err ) )				{					err	= FSpWipeFork( spec, kResourceFork, numPasses,								numPatterns, patternList, wipeBuffer,								 wipeBufferLength);				}			}						if( allocatedBuffer )				pgpFreeMac( wipeBuffer );		}		else		{			err = fBsyErr;		}	}		return( err );}	OSStatusCMacFSWiper::FSpWipeAndDelete(	const FSSpec 			*spec,	PGPUInt32				numPasses,	PGPUInt32 				numPatterns,	const PGPWipingPattern 	patternList[],	void 					*wipeBuffer,	PGPSize 				wipeBufferLength){	OSStatus	err; 		mTotalDone		= 0;	mWipeSpec		= *spec;		/* wipe the data */	err	= FSpWipe( spec, numPasses, numPatterns, patternList, wipeBuffer,					 wipeBufferLength );	if( IsntErr( err ) )	{		FSSpec		tempSpec = *spec;		CInfoPBRec	cpb;				/* Clear the catalog information on the file */				err = FSpGetCatInfo( &tempSpec, &cpb );		if( IsntErr( err ) )		{			pgpClearMemory( &cpb, sizeof( cpb ) );						err = FSpSetCatInfo( &tempSpec, &cpb );						FlushVol( NULL, tempSpec.vRefNum );		}				if( IsntErr( err ) )		{			uchar	bogusName[ kMaxHFSFileNameLength + 1 ];					/* wipe the name (50/50 chance of this doing what we want) */			pgpFillMemory( bogusName, sizeof( bogusName ), 'x' );			bogusName[0] = kMaxHFSFileNameLength;			if ( IsntErr( FSpRename( &tempSpec, bogusName ) ) )			{				CopyPString( bogusName, tempSpec.name );				FlushVol( NULL, tempSpec.vRefNum );			}						err	= FSpDelete( &tempSpec );		}	}		/* caller is responsible for flushing volume */		return( err );}CMacFSWiper::CMacFSWiper(){	MacDebug_FillWithGarbage( &mWipeSpec, sizeof( mWipeSpec ) );		mTotalDone = 0;}/*____________________________________________________________________________	To get progress while wiping, subclass and override Progress().____________________________________________________________________________*/	OSStatusCMacFSWiper::Progress(	ConstStr255Param	fileName,	PGPUInt64			additionalDone ){	(void)fileName;		mTotalDone	+= additionalDone;		return( noErr );}/*__Editor_settings____	Local Variables:	tab-width: 4	End:	vi: ts=4 sw=4	vim: si_____________________*/