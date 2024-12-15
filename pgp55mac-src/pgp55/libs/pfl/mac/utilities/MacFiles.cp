/*____________________________________________________________________________
	MacFiles.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacFiles.cp,v 1.23 1997/10/22 01:07:16 heller Exp $
____________________________________________________________________________*/
#include <FSM.h>
#include <Folders.h>
#include <TextUtils.h>

#include "MacFiles.h"
#include "MacMemory.h"
#include "MacStrings.h"
#include "UInt64.h"
#include "MacBTree.h"
#include "CWrappers.h"
#include "MacDriverUtils.h"
#include "MacDebug.h"

#define MIN(a, b )		( (a) <= (b) ? (a) : (b) )


const short	kCacheDisable	= 32;


				
/*____________________________________________________________________________
	Get the catalog information for itemSpec in itemCatalogInfo
____________________________________________________________________________*/
	OSStatus
FSpGetCatInfo(const FSSpec *itemSpec, CInfoPBRec *itemCatalogInfo)
{
	OSStatus	status;
	
	AssertSpecIsValid( itemSpec, "FSpGetCatInfo" );
	pgpAssertAddrValid( itemCatalogInfo, CInfoPBRec );
	
	pgpClearMemory( itemCatalogInfo, sizeof( *itemCatalogInfo ) );
	
	itemCatalogInfo->hFileInfo.ioVRefNum	= itemSpec->vRefNum;
	itemCatalogInfo->hFileInfo.ioDirID		= itemSpec->parID;
	itemCatalogInfo->hFileInfo.ioNamePtr	= (uchar *) itemSpec->name;
	
	status = PBGetCatInfoSync( itemCatalogInfo );
	
	return( status );
}

/*____________________________________________________________________________
	Set the catalog information for itemSpec to itemCatalogInfo
____________________________________________________________________________*/

	OSStatus
FSpSetCatInfo(const FSSpec *itemSpec, CInfoPBRec *itemCatalogInfo)
{
	OSStatus	status;
	
	AssertSpecIsValid( itemSpec, "FSpSetCatInfo" );
	pgpAssertAddrValid( itemCatalogInfo, CInfoPBRec );
	
	itemCatalogInfo->hFileInfo.ioVRefNum	= itemSpec->vRefNum;
	itemCatalogInfo->hFileInfo.ioDirID		= itemSpec->parID;
	itemCatalogInfo->hFileInfo.ioNamePtr	= (uchar *) itemSpec->name;
	
	status = PBSetCatInfoSync( itemCatalogInfo );
	
	return( status );
}



	Boolean
FSpExists( const FSSpec *spec )
{
	CInfoPBRec	cpb;
	
	return( FSpGetCatInfo( spec, &cpb ) == noErr );
}


	void
GetAllVRefNums(
	short *		vRefNums,
	UInt16		maxVRefNums,
	UInt16 *	numVRefNums )
{
	UInt16	volIndex	= 1;
	OSErr	err	= noErr;
	
	*numVRefNums	= 0;
	
	while ( *numVRefNums < maxVRefNums )
	{
		HParamBlockRec	pb;
		
		pb.volumeParam.ioNamePtr	= nil;
		pb.volumeParam.ioVRefNum	= 0;
		pb.volumeParam.ioVolIndex	= volIndex;
		err	= PBHGetVInfoSync( &pb );
		if ( IsErr( err ) )
			break;
		
		vRefNums[ volIndex - 1 ]	= pb.volumeParam.ioVRefNum;
		
		*numVRefNums	+= 1;
		
		++volIndex;
	}
}



	void
FlushAllVolumes( void )
{
	enum { kMaxVRefNums	 = 200 };
	short	vRefNums[ kMaxVRefNums ];
	UInt16	numVRefNums	= 0;
	
	GetAllVRefNums( vRefNums, kMaxVRefNums, &numVRefNums );
	
	for( UInt16 index = 0; index < numVRefNums; ++index )
	{
		FlushVol( nil, vRefNums[ index ] );
	}
}



/*____________________________________________________________________________
	GetVolumeBlockCounts: Get the total and free block counts for a volume.
	All output parameters are optional.
____________________________________________________________________________*/

	OSStatus
GetVolumeBlockCounts(
	short 	vRefNum,
	UInt32 	*freeBlocks,
	UInt32 	*totalBlocks,
	UInt32 	*blockSize)
{
	OSStatus		status;
	HParamBlockRec	pb;
	Str255			volumeName;
	
	AssertAddrNullOrValid( freeBlocks, UInt32, "GetVolumeBlockCounts" );
	AssertAddrNullOrValid( totalBlocks, UInt32, "GetVolumeBlockCounts" );
	AssertAddrNullOrValid( blockSize, UInt32, "GetVolumeBlockCounts" );
	
	pgpClearMemory( &pb, sizeof( pb ) );
	
	pb.volumeParam.ioVRefNum	= vRefNum;
	pb.volumeParam.ioNamePtr	= volumeName;
	
	status = PBHGetVInfoSync( &pb );
	if( IsntErr( status ) )
	{
		if( freeBlocks != nil )
			*freeBlocks = pb.volumeParam.ioVFrBlk;
			
		if( totalBlocks != nil )
			*totalBlocks = pb.volumeParam.ioVNmAlBlks;
			
		if( blockSize != nil )
			*blockSize = pb.volumeParam.ioVAlBlkSiz;
	}
	
	return( status );
}







	const FCBRec *
GetFCBForFile( short fileRefNum )
	{
	return( (const FCBRec *)(LMGetFCBSPtr() + fileRefNum) );
	}
	
	
	const VCB *
GetVCBForFile( short fileRefNum )
	{
	const FCBRec *	fcb	= GetFCBForFile( fileRefNum);
	const VCB *		vcb	 = nil;
	
	if ( IsntNull( fcb ) )
		vcb	= fcb->fcbVPtr;
		
	return( vcb );
	}
	

	
	UInt32
GetVolumeFreeAllocBlocks( short vRefNum )
	{
	const VCB *	vcb	= GetVCBForVolume( vRefNum );
	pgpAssert( IsntNull( vcb ) );
	
	return( vcb->vcbFreeBks );
	}
	
	
	
	OSStatus
GetVolumeDriveNumberAndRefNum(
	short		vRefNum,
	short *		driveNumber,
	short *		driverRefNum )
	{
	const VCB *	vcb	= GetVCBForVolume( vRefNum );
	pgpAssert( IsntNull( vcb ) );
	OSStatus	err	= noErr;
	
	if ( IsntNull( vcb ) )
		{
		GetDriveNumberAndRefNumForVCB( vcb, driveNumber, driverRefNum );
		err	= noErr;
		}
	else
		{
		err	= nsvErr;
		}
	
	return( err );
	}
	
	
	
	UInt32
GetVolumeAllocBlockSize( short vRefNum )
	{
	const VCB *	vcb	= GetVCBForVolume( vRefNum );
	pgpAssert( IsntNull( vcb ) );
	
	return( vcb->vcbAlBlkSiz );
	}
	
	

	short
GetVolumeForFile( short fileRefNum )
	{
	const VCB *	vcb	= GetVCBForFile( fileRefNum );
	short		vRefNum	= 0;
	
	if ( IsntNull( vcb ) )
		{
		vRefNum	= vcb->vcbVRefNum;
		}
		
		
	return( vRefNum );
	}
	
	

/*____________________________________________________________________________
	Convert an allocation block number into a disk block number.
	
	Allocation blocks start at 'vcb->alBlkStart' disk blocks into the drive.
____________________________________________________________________________*/
	UInt32
AllocBlockNumberToDiskBlockNumber(
	const VCB *		vcb,
	UInt32			allocBlockNumber )
	{
	pgpAssertAddrValid( vcb, VCB );
	
	UInt32	diskBlockNumber;
	UInt32	numDiskBlocksPerAllocBlock;
	
	numDiskBlocksPerAllocBlock	= GetVCBAllocBlockSize( vcb ) / kDiskBlockSize;
	
	diskBlockNumber	=
		vcb->vcbAlBlSt + allocBlockNumber * numDiskBlocksPerAllocBlock;

	return( diskBlockNumber );
	}


typedef struct HFSExtent
{
	UInt16	startBlock;
	UInt16	numBlocks;
	
} HFSExtent;


	static OSErr
GetHFSExtentsForFork(
	short				fileRefNum,
	HFSExtent **		extentsListPtr,
	PGPUInt32 *			numExtentsPtr)
{
	const FCBRec *		fcb	= nil;
	const VCB *			vcb = nil;
	const HFSExtent *	fcbExtents;
	const UInt16		kNumFCBExtents	= 3;
	UInt32				numFileAllocBlocks;
	UInt32				numFoundAllocBlocks;
	OSErr				err	= noErr;
	UInt32				allocationBlockSize;
	UInt32				numExtents;
	HFSExtent			**extentList;
	
	AssertFileRefNumIsValid( fileRefNum, "GetHFSExtentsForFile" );
	pgpAssertAddrValid( numExtentsPtr, UInt32 );
	
	numExtents			= 0;
	fcb					= GetFCBForFile( fileRefNum );
	vcb					= GetVCBForFile( fileRefNum );
	allocationBlockSize	= vcb->vcbAlBlkSiz;
	
	// Use a handle here so we can dynamically grow the list
	extentList = (HFSExtent **) NewHandle( 0 );
	if( IsNull( extentList ) )
		return( memFullErr );
	
	FlushFile( fileRefNum );
	
	// The physical length should always be a multiple of the
	// allocation block size, but it doesn't hurt to compensate
	numFileAllocBlocks =
		( fcb->fcbPLen + allocationBlockSize - 1 ) / allocationBlockSize;
	pgpAssert( numFileAllocBlocks * allocationBlockSize == fcb->fcbPLen );
	
	fcbExtents			= (const HFSExtent *) &fcb->fcbExtRec[0];
	numFoundAllocBlocks = 0;
	
	for( UInt32 index = 0; index < kNumFCBExtents; ++index )
	{
		const HFSExtent *extent;
		
		extent = &fcbExtents[ index ];
		if( extent->numBlocks != 0 )
		{
			err = PtrAndHand( extent, (Handle) extentList, sizeof( *extent ));
			if( IsErr( err ) )
				break;
						
			++numExtents;
		
			numFoundAllocBlocks += extent->numBlocks;
		}
	}
		
	if( IsntErr( err ) && numFoundAllocBlocks < numFileAllocBlocks )
	{
		CMacExtentsBTree	bTreeObj;
		
		err = bTreeObj.OpenVolume( vcb->vcbVRefNum );
		if( IsntErr( err ) )
		{
			while( IsntErr( err ) )
			{
				ExtentDataRecord	extentRecord;

				err = bTreeObj.GetExtent( fcb->fcbFlNm,
							kBTreeDataForkRecordType,
							numFoundAllocBlocks, &extentRecord );
				if( IsntErr( err ) )
				{
					for( UInt32 index = 0; index < kNumFCBExtents; ++index )
					{
						const HFSExtent *extent;
						
						extent = (HFSExtent *) &extentRecord.extents[index];
						if( extent->numBlocks != 0 )
						{
							err = PtrAndHand( extent,
								(Handle) extentList, sizeof( *extent ) );
							if( IsErr( err ) )
								break;
										
							++numExtents;
						
							numFoundAllocBlocks += extent->numBlocks;
						}
					}
				}
			}
		}
			
		if( err == kBTreeRecordNotFoundError )
			err = noErr;
	}
	
	if( IsntErr( err ) )
	{
		if( numFoundAllocBlocks != numFileAllocBlocks )
		{
			// Didn't get all of the extents.
			pgpDebugFmtMsg( ( pgpaFmtPrefix,
				"GetHFSExtentsForFile:"
				"Missing or extra extents found=%lu num=%lu",
				numFoundAllocBlocks, numFileAllocBlocks)   );
			err = eofErr;
		}
		else
		{
			HFSExtent	*extents;
			
			extents = (HFSExtent *) NewPtr( numExtents * sizeof( *extents ) );
			if ( IsntNull( extents ) )
			{
				// Copy the data from our handle to an allocated pointer
				BlockMoveData( *extentList,
					extents, numExtents * sizeof( *extents ) );
				
				*extentsListPtr	= extents;
				*numExtentsPtr 	= numExtents;
			}
			else
			{
				err = memFullErr;
			}
		}
	}	
		
Exit:

	DisposeHandle( (Handle) extentList );
	
	return( err );
}

	static void
FreeHFSExtentList(HFSExtent *theList)
{
	if( IsntNull( theList ) )
		DisposePtr( (Ptr) theList );
}

#if 0	/* KEEP! [ */

/*
** The following code was provided to us by Apple. We store the Lg2Phys
** function as object code only because we want to call it from PPC code.
*/

/* Structure for parameters passed on the stack to Lg2Phys function: */

struct Lg2PhysStack
{
	long	returnAddress;
	long	registers[6];
	long	fileRefNum;
	long	filePosition;
	long	reqCount;
	long	volOffset;
	long	contiguousBytes;
};

/* Some system private low memory locations: */

enum
{
	FCBSPtr		= 0x034E,
	FSBusy		= 0x0360,
	HFSStkTop	= 0x036A,
	FSCallAsync	= 0x0342,
	JLg2Phys	= 0x06FC
};


static asm OSErr Lg2Phys(
	long	fileRefNum,
	long	filePosition,
	long	reqCount,
	long *	volOffset,
	long *	contiguousBytes)
{
	movem.l	d3-d6/a2/a6,-(sp)							// save registers
	
	move.l	struct(Lg2PhysStack.fileRefNum)(sp),d1		// d1 = fileRefNum
	move.l	struct(Lg2PhysStack.reqCount)(sp),d4		// d4 = reqCount
	move.l	struct(Lg2PhysStack.filePosition)(sp),d5	// d5 = filePosition
	
GrabFSLoop:
	bset	#0,FSBusy						// grab control of the File Manager
	bne.s	GrabFSLoop
	
	move.l	HFSStkTop,a6					// use the File Manager's stack
											// (an a6 stack)
	
	move.l	FCBSPtr,a1						// a1 = pointer to FCB table
	move.l	struct(FCBRec.fcbVPtr)(a1,d1.w),a2	// a2 = get VCB pointer
	
	clr.b	FSCallAsync						// tell the File Manager it's
											// doing a synchronous operation
	
	movea.l	JLg2Phys,a0						// let the FS do the mapping
	jsr		(a0)
	
	bclr	#0,FSBusy						// all done with the File Manager
	
	movea.l	struct(Lg2PhysStack.volOffset)(sp),a0
	move.l	d3,(a0)							// return volOffset from d3
	movea.l	struct(Lg2PhysStack.contiguousBytes)(sp),a0
	move.l	d6,(a0)							// return contiguousBytes from d6

	movem.l	(sp)+,d3-d6/a2/a6				// restore registers
	
	tst.w	d0								// set up condition codes
	rts										// and return
}

#endif	/* ] */

typedef OSErr	(*Lg2PhysProc)(long fileRefNum, long filePosition,
					long reqCount, long *volOffset, long *contiguousBytes);

static UInt16 gLg2PhysProcData[] =
{	
	0x48E7, 0x1E22,				// MOVEM.L   D3-D6/A2/A6,-(A7)
	0x222F, 0x001C,				// MOVE.L    $001C(A7),D1
	0x282F, 0x0024,				// MOVE.L    $0024(A7),D4
	0x2A2F, 0x0020,				// MOVE.L    $0020(A7),D5
	0x08F8, 0x0000, 0x0360,		// BSET      #$00,$0360
	0x66F8,						// BNE.S     *-$0006
	0x2C78, 0x036A,				// MOVEA.L   $036A,A6
	0x2278, 0x034E,				// MOVEA.L   $034E,A1
	0x2471, 0x1014,				// MOVEA.L   $14(A1,D1.W),A2
	0x4238, 0x0342,				// CLR.B     $0342
	0x2078, 0x06FC,				// MOVEA.L   $06FC,A0
	0x4E90,						// JSR       (A0)
	0x08B8, 0x0000, 0x0360,		// BCLR      #$00,$0360
	0x206F, 0x0028,				// MOVEA.L   $0028(A7),A0
	0x2083,						// MOVE.L    D3,(A0)
	0x206F, 0x002C,				// MOVEA.L   $002C(A7),A0
	0x2086,						// MOVE.L    D6,(A0)
	0x4CDF, 0x4478,				// MOVEM.L   (A7)+,D3-D6/A2/A6
	0x4A40,						// TST.W     D0
	0x4E75						// RTS
};

#if GENERATING68K	/* [ */

	static OSErr
CallLg2Phys(
	long	fileRefNum,
	long	filePosition,
	long 	reqCount,
	long 	*volOffset,
	long 	*contiguousBytes)
{
	static Boolean	inited = FALSE;
	Lg2PhysProc		procPtr;
	OSErr			err;
	
	if( ! inited )
	{
		inited = TRUE;
		
		FlushInstructionCache();
		FlushDataCache();
	}

	procPtr = (Lg2PhysProc) &gLg2PhysProcData[0];
	
	err = (*procPtr)( fileRefNum, filePosition, reqCount,
			volOffset, contiguousBytes );
	
	return( err );
}

#else	/* ][ */

/* Note: FSM.h defines a conflicting UPP to call Lg2Phys. Do not use it */

enum {
	kLg2PhysProcInfo = kCStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(long)))
};

	static OSErr
CallLg2Phys(
	long	fileRefNum,
	long	filePosition,
	long 	reqCount,
	long 	*volOffset,
	long 	*contiguousBytes)
{
	OSErr	err;
	
	err = CallUniversalProc( (UniversalProcPtr) &gLg2PhysProcData[0],
				kLg2PhysProcInfo, fileRefNum, filePosition, reqCount,
				volOffset, contiguousBytes );

	return( err );
}

#endif

/*
** Because of a requirement for PGPDisk, GetDiskExtentsForFork always
** allocates the list in the System Heap. This is an implementation
** detail. Clients should call FreeDiskExtentList to dispose of the list
*/

	static OSErr
GetLg2PhysExtentsForFork(
	short		fileRefNum,
	DiskExtent 	**extentsListPtr,
	PGPUInt32 	*numExtents)
{
	OSErr		err	= noErr;
	DiskExtent	**extentsHandle;	

	pgpAssertAddrValid( extentsListPtr, DiskExtent * );
	pgpAssertAddrValid( numExtents, PGPUInt32 );
	
	*extentsListPtr = NULL;
	*numExtents		= 0;
	
	extentsHandle = (DiskExtent **) NewHandle( 0 );
	if ( IsntNull( extentsHandle ) )
	{
		SInt32	bytesRemaining;
		
		err = GetEOF( fileRefNum, &bytesRemaining );
		if( IsntErr( err ) )
		{
			SInt32			filePosition;
			
			// Lg2Phys requires bytesRemaining up to nearest 512.
			bytesRemaining 	= ( ( bytesRemaining + kDiskBlockSize - 1 ) /
								kDiskBlockSize ) * kDiskBlockSize; 
			filePosition 	= 0;
			
			while( bytesRemaining > 0 && IsntErr( err ) )
			{
				SInt32		contiguousBytes;
				DiskExtent	curExtent;
				
				err = CallLg2Phys( fileRefNum, filePosition, bytesRemaining,
							(long *) &curExtent.diskBlockIndex,
							&contiguousBytes );
				if( IsntErr( err ) )
				{
					curExtent.numBlocks = contiguousBytes / kDiskBlockSize;
					
					err = PtrAndHand( &curExtent, (Handle) extentsHandle,
								sizeof( curExtent ) );
					if( IsntErr( err ) )
					{
						*numExtents	+= 1;
					}
				}

				bytesRemaining 	-= contiguousBytes;
				filePosition 	+= contiguousBytes;
			}
		}
		
		if( IsntErr( err ) && *numExtents > 0 )
		{
			UInt32	extentListSize;
			
			extentListSize = ( *numExtents * sizeof( DiskExtent ) );
			
			*extentsListPtr = (DiskExtent *) NewPtrSys( extentListSize );
			if( IsntNull( *extentsListPtr ) )
			{
				pgpCopyMemory( *extentsHandle, *extentsListPtr,
							extentListSize );
			}
			else
			{
				err = memFullErr;
			}
		}
		
		DisposeHandle( (Handle) extentsHandle );
	}
	else
	{
		err	= memFullErr;
	}
		
	return( err );
}

	static OSErr
ConvertHFSExtentsToDiskExtents(
	const HFSExtent *	hfsExtents,
	UInt32				numHFSExtents,
	UInt32				allocBlockStart,
	UInt32				allocBlockSize,
	DiskExtent **		diskExtentsListPtr)
{
	OSErr		err	= noErr;
	DiskExtent *diskExtents;	

	pgpAssertAddrValid( hfsExtents, HFSExtent );
	pgpAssertAddrValid( diskExtentsListPtr, DiskExtent * );
	
	diskExtents	= (DiskExtent *)NewPtrSysClear( numHFSExtents *
						sizeof( diskExtents[ 0 ] ) );
	if ( IsntNull( diskExtents ) )
	{
		const UInt32	kNumDiskBlocksPerAllocBlock	= allocBlockSize /
								kDiskBlockSize;
		DiskExtent		*curDiskExtent;
		const HFSExtent *curHFSExtent;
		const HFSExtent *lastHFSExtent;
		
		curDiskExtent 	= &diskExtents[0];
		curHFSExtent 	= &hfsExtents[0];
		lastHFSExtent 	= curHFSExtent + numHFSExtents - 1;
		
		while( curHFSExtent <= lastHFSExtent )
		{
			UInt32	numDiskBlocksInThisExtent;
			
			numDiskBlocksInThisExtent = curHFSExtent->numBlocks *
							kNumDiskBlocksPerAllocBlock;

			curDiskExtent->numBlocks		= numDiskBlocksInThisExtent;
			curDiskExtent->diskBlockIndex	= allocBlockStart +
												curHFSExtent->startBlock *
												kNumDiskBlocksPerAllocBlock;
			++curDiskExtent;
			++curHFSExtent;
		}
	}
	else
	{
		err	= memFullErr;
	}
		
	*diskExtentsListPtr	= diskExtents;

	return( err );
}

	OSErr
GetDiskExtentsForFork(
	short		fileRefNum,
	DiskExtent 	**extentsListPtr,
	PGPUInt32 	*numExtents)
{
	OSErr	err;
	
	err = GetLg2PhysExtentsForFork( fileRefNum, extentsListPtr, numExtents );
	if( IsErr( err ) )
	{
		const VCB	*vcb;
		
		// Revert to the old way of getting the extents if the volume
		// is HFS only
		pgpDebugMsg( "Lg2Phys failed!" );
		
		vcb	= GetVCBForFile( fileRefNum );
		pgpAssert( IsntNull( vcb ) );
		
		if( vcb->vcbSigWord == kHFSVolumeSignature )
		{
			HFSExtent	*hfsExtents;
		
			err	= GetHFSExtentsForFork( fileRefNum, &hfsExtents,
						numExtents);
			if( IsntErr( err ) )
			{
				err	= ConvertHFSExtentsToDiskExtents( hfsExtents,
							*numExtents, vcb->vcbAlBlSt,
							vcb->vcbAlBlkSiz, extentsListPtr);
				
				FreeHFSExtentList( hfsExtents );
			}
		}
	}
	
	return( err );
}

	void
FreeDiskExtentList(DiskExtent *theList)
{
	if( IsntNull( theList ) )
		DisposePtr( (Ptr) theList );
}

	OSErr
GetVolumeName(
	short		vRefNum,
	StringPtr	name )
	{
	HParamBlockRec	pb;
	OSErr			err	= noErr;
	
	pb.volumeParam.ioVRefNum	= vRefNum;
	pb.volumeParam.ioVolIndex	= 0;
	pb.volumeParam.ioNamePtr	= name;
	err	= PBHGetVInfoSync( &pb );
	
	return( err );
	}


	OSErr
GetVolumeDates(
	short		vRefNum,
	long *		creationDate,
	long *		modificationDate )
	{
	HParamBlockRec	pb;
	OSErr			err	= noErr;
	Str255			name;
	
	pb.volumeParam.ioVRefNum	= vRefNum;
	pb.volumeParam.ioVolIndex	= 0;
	pb.volumeParam.ioNamePtr	= name;
	err	= PBHGetVInfoSync( &pb );
	if ( IsntPGPError( err ) )
	{
		if ( IsntNull( creationDate ) )
			*creationDate	= pb.volumeParam.ioVCrDate;
			
		if ( IsntNull( modificationDate ) )
			*modificationDate	= pb.volumeParam.ioVLsMod;
	}
	
	return( err );
	}

	


/*____________________________________________________________________________
	Read specified number of bytes at specified offset in the file. The file
	position is preserved. bytesRead is optional
____________________________________________________________________________*/

	OSStatus
FSReadAtOffset(
	short 	fileRefNum,
	UInt32	writeOffset,
	UInt32 	bytesToRead,
	void	*buffer,
	UInt32	*bytesRead,
	Boolean	restoreFilePos )
	{
	OSStatus	status;
	UInt32		saveFilePos;
	
	AssertFileRefNumIsValid( fileRefNum, "FSReadAtOffset" );
	pgpAssertAddrValid( buffer, uchar );
	AssertAddrNullOrValid( bytesRead, UInt32, "FSReadAtOffset" );
	
	status = GetFPos( fileRefNum, (long *) &saveFilePos );
	if( IsntErr( status ) )
		{
		status = SetFPos( fileRefNum, fsFromStart, writeOffset );
		if( IsntErr( status ) )
			{
			UInt32	readCount = bytesToRead;
			
			status = FSRead( fileRefNum, (long *) &readCount, buffer );
			
			if( IsntNull( bytesRead ) )
				*bytesRead = readCount;
			
			if ( restoreFilePos )
				{
				(void) SetFPos( fileRefNum, fsFromStart, saveFilePos );
				}
			}
		}
		
	return( status );
	}

/*____________________________________________________________________________
	Write specified number of bytes at specified offset in the file. The file
	position is preserved. bytesWritten is optional
____________________________________________________________________________*/

	OSStatus
FSWriteAtOffset(
	short 		fileRefNum,
	UInt32		writeOffset,
	UInt32 		bytesToWrite,
	const void	*buffer,
	UInt32		*bytesWritten,
	Boolean		restoreFilePos)
	{
	OSStatus	status;
	UInt32		saveFilePos;
	
	AssertFileRefNumIsValid( fileRefNum, "FSWriteAtOffset" );
	pgpAssertAddrValid( buffer, uchar );
	AssertAddrNullOrValid( bytesWritten, UInt32, "FSWriteAtOffset" );
	
	status = GetFPos( fileRefNum, (long *) &saveFilePos );
	if( IsntErr( status ) )
		{
		status = SetFPos( fileRefNum, fsFromStart, writeOffset );
		if( IsntErr( status ) )
			{
			UInt32	writeCount = bytesToWrite;
			
			status = FSWrite( fileRefNum, (long *) &writeCount, buffer );
			
			if( IsntNull( bytesWritten ) )
				*bytesWritten = writeCount;
				
			if ( restoreFilePos )
				{
				(void) SetFPos( fileRefNum, fsFromStart, saveFilePos );
				}
			}
		}
		
	return( status );
	}




	Boolean
FSpEqual(const FSSpec *spec1, const FSSpec *spec2)
{
	AssertSpecIsValid( spec1, "FSpEqual" );
	AssertSpecIsValid( spec2, "FSpEqual" );
	
	if( spec1->parID == spec2->parID )
	{
		if( spec1->vRefNum == spec2->vRefNum )
		{
			if( EqualString( spec1->name, spec2->name, FALSE, TRUE ) )
			{
				return( TRUE );
			}
		}
	}
	
	return( FALSE );
}


	OSStatus
FlushFile(short fileRefNum)
{
	ParamBlockRec	pb;
	OSStatus		status;
	
	AssertFileRefNumIsValid( fileRefNum, "FlushFile" );
	
	pb.ioParam.ioCompletion	= nil;
	pb.ioParam.ioNamePtr	= nil;
	pb.ioParam.ioRefNum		= fileRefNum;
	
	status = PBFlushFileSync( &pb );

	return( status );
}




	OSStatus
GetSpecFromRefNum(
	short		refNum,
	FSSpec *	spec)
	{
	OSStatus	err	= noErr;
	FCBPBRec	pb;
	
	pb.ioFCBIndx	= 0;
	pb.ioRefNum		= refNum;
	pb.ioNamePtr	= spec->name;
	err	= PBGetFCBInfoSync( &pb );
	if ( IsntErr( err ) )
		{
		spec->vRefNum	= pb.ioFCBVRefNum;
		spec->parID		= pb.ioFCBParID;
		}
	
	return( err );
	}


	OSStatus
GetSpecFromFCBTable(
	short		refNum,
	FSSpec *	spec)
	{
	OSStatus	err	= noErr;
	
	const FCBRec *	fcb	= (const FCBRec *)( (Ptr)LMGetFCBSPtr() + refNum );
	
	spec->vRefNum	= fcb->fcbVPtr->vcbVRefNum;
	spec->parID		= fcb->fcbDirID;
	CopyPString( fcb->fcbCName, spec->name );
	
	return( err );
	}


/*____________________________________________________________________________
	Find the refnum for open file.
____________________________________________________________________________*/

OSStatus
FSpFindOpenForkRef(
	FSSpec *	inSpec,
	Boolean		inResFork,
	SInt16 *	outRefNum)
{
	OSErr		result = noErr;
	FCBPBRec	paramBlock;
	FSSpec		testSpec;
	Str32		fileName;
	
	pgpClearMemory(&paramBlock, sizeof(paramBlock));
	paramBlock.ioVRefNum = inSpec->vRefNum;
	paramBlock.ioFCBIndx = 1;
	paramBlock.ioNamePtr = fileName;
	
	for (;;) {
		result = ::PBGetFCBInfoSync(&paramBlock);
		if (result != noErr) {
			break;
		}
		
		if ((inResFork && (paramBlock.ioFCBFlags & 0x0200))
		|| (!inResFork && !(paramBlock.ioFCBFlags & 0x0200))) {
			result = ::FSMakeFSSpec(	paramBlock.ioVRefNum,
										paramBlock.ioFCBParID,
										paramBlock.ioNamePtr,
										&testSpec);
			if ((result != noErr) || FSpEqual(inSpec, &testSpec)) {
				break;
			}
		}
		paramBlock.ioFCBIndx++;
	}
	
	if (result == noErr) {
		*outRefNum = paramBlock.ioRefNum;
	} else {
		*outRefNum = -1;
	}
	
	return result;
}



	void
GetDriveNumberAndRefNumForVCB(
	const VCB *	vcb,
	short *		driveNumber,
	short *		driverRefNum )
	{
	pgpAssertAddrValid( vcb, VCB );
	
	*driveNumber	= vcb->vcbDrvNum;
	*driverRefNum	= vcb->vcbDRefNum;
	}





	const VCB *
GetVCBForVolume( short vRefNum )
	{
	pgpAssert( vRefNum < 0 );
	
	const VCB *	vcb	= (const VCB *)GetVCBQHdr()->qHead;
	
	while ( IsntNull( vcb ) )
		{
		if ( vcb->vcbVRefNum == vRefNum )
			break;
			
		vcb	= (const VCB *)vcb->qLink;
		}
	
	pgpAssert( IsntNull( vcb ) );
	
	return( vcb );
	}


	OSStatus
FSGetUniqueName(
	short 				vRefNum,
	long 				dirID,
	ConstStr255Param 	baseName,
	StringPtr 			uniqueName)
{
	FSSpec		fileSpec;
	OSStatus	status 		= noErr;
	
	pgpAssertAddrValid( baseName, uchar );
	pgpAssertAddrValid( uniqueName, uchar );
	
	fileSpec.vRefNum	= vRefNum;
	fileSpec.parID		= dirID;
	CopyPString( baseName, fileSpec.name );
	if ( FSpExists( &fileSpec ) )
	{
		UInt32		uniqueID 	= 1;
	
		while( TRUE )
		{
			Str32		uniqueIDStr;
			CInfoPBRec	cpb;
			
			CopyPString( baseName, fileSpec.name );
			NumToString( uniqueID, uniqueIDStr );
			
			if( fileSpec.name[0] > kMaxHFSFileNameLength - uniqueIDStr[0] - 1)
				fileSpec.name[0] = kMaxHFSFileNameLength - uniqueIDStr[0] - 1;
				
			AppendPString( "\p ", fileSpec.name );
			AppendPString( uniqueIDStr, fileSpec.name );
		
			status = FSpGetCatInfo( &fileSpec, &cpb );
			if( IsErr( status ) )
			{
				if( status == fnfErr )
					status = noErr;
					
				break;
			}
			
			++uniqueID;
		}
	}
	
	CopyPString( fileSpec.name, uniqueName );
	
	return( status );
}


	OSStatus
FSpGetUniqueSpec(
	const FSSpec *	spec,
	FSSpec *		outSpec )
{
	OSStatus	err;
	
	outSpec->vRefNum	= spec->vRefNum;
	outSpec->parID		= spec->parID;
	
	err	= FSGetUniqueName( spec->vRefNum, spec->parID,
			spec->name, outSpec->name );
	
	return( err );
}



	static OSStatus
GetDirFullPath(short vRefNum, long callerDirID, StringPtr path)
{
	OSStatus	status;
	CInfoPBRec	cpb;
	long		dirIDList[40];
	long		dirIDIndex;
	long		dirID;

	pgpAssertAddrValid( path, uchar );
	
	path[0] 	= 0;
	dirID		= callerDirID;
	dirIDIndex 	= 0;
		
	// Build up a list of directory ID's from the file spec to the root.
	
	do
	{
		Str255	dirName;
		
		dirIDList[dirIDIndex++] = dirID;
	
		cpb.dirInfo.ioVRefNum 	= vRefNum;
		cpb.dirInfo.ioDrDirID 	= dirID;
		cpb.dirInfo.ioNamePtr	= dirName;
		cpb.dirInfo.ioFDirIndex	= -1;
		
		status = PBGetCatInfoSync( &cpb );
		if( IsErr( status ) )
			break;
		
		dirID = cpb.dirInfo.ioDrParID;
		
	} while( dirID != fsRtParID );

	if( IsntErr( status ) )
	{
		--dirIDIndex;
		
		// Use the list of directory ids to get the path items.
		
		while( dirIDIndex >= 0 )
		{
			Str255	dirName;

			cpb.dirInfo.ioVRefNum 	= vRefNum;
			cpb.dirInfo.ioDrDirID 	= dirIDList[dirIDIndex];
			cpb.dirInfo.ioNamePtr	= dirName;
			cpb.dirInfo.ioFDirIndex	= -1;
			
			status = PBGetCatInfoSync( &cpb );
			if( IsErr( status ) )
				break;
			
			AppendPString( dirName, path );
			AppendPString( "\p:", path );
			
			--dirIDIndex;
		}
	}

	return( status );
}

	OSStatus
FSpGetFullPath(const FSSpec *fileSpec, StringPtr path)
{
	OSStatus	status;

	AssertSpecIsValid( fileSpec, "FSpGetPath" );
	pgpAssertAddrValid( path, uchar );
	
	status = GetDirFullPath( fileSpec->vRefNum, fileSpec->parID, path );
	if( IsntErr( status ) )
	{
		CInfoPBRec	cpb;

		// Append the file name to the directory path
		
		AppendPString( fileSpec->name, path );
		
		if( IsntErr( FSpGetCatInfo( fileSpec, &cpb ) ) )
		{
			if( cpbIsFolder( &cpb ) )
			{
				// If the item exists and is a folder, append a ":"
				AppendPString( "\p:", path );
			}
		}
	}

	return( status );
}



	OSStatus
FSpDeleteResourceFork( const FSSpec *spec )
{
	OSErr	err	= noErr;
	short	fileRef;
	
	AssertSpecIsValid( spec, "DeleteResourceFork" );
	err	= FSpOpenRF( spec, fsRdWrPerm, &fileRef );
	if ( IsntErr( err ) )
	{
		err	= SetEOF( fileRef, 0 );
		FSClose( fileRef );
	}
	
	AssertNoErr( err, "DeleteResourceFork" );
	return( err );
}



/*____________________________________________________________________________
	Copy the specified number of bytes at the specified offset to the output
	file at the specified offset.
____________________________________________________________________________*/
	OSStatus
FSCopyBytes(
	short		fromRef,
	UInt32		fromOffset,
	short		toRef,
	UInt32		toOffset,
	const UInt32	requestCount )
{
	OSStatus		err	= noErr;
	const UInt32	kMinBufferSize = kDiskBlockSize;
	const UInt32	kMaxBuffer	= kDiskBlockSize * 1024UL;
	void *			buffer	= nil;
	UInt32			bufferSize;
	
	AssertFileRefNumIsValid( fromRef, "CopyData" );
	AssertFileRefNumIsValid( toRef, "CopyData" );
	pgpAssert( (fromOffset & 0x80000000) == 0 &&
		(toOffset & 0x80000000) == 0 && (requestCount & 0x80000000) == 0 );
	
	if ( requestCount != 0 )
	{
		buffer	= pgpNewPtrMost( MIN( requestCount, kMaxBuffer ),
					kMinBufferSize,
					kMacMemory_UseCurrentHeap | kMacMemory_UseTempMem,
					&bufferSize );
	}
	
	if ( IsntNull( buffer ) )
	{
		// read whole blocks, for efficiency,
		// unless we have a buffer same size as whole request
		if ( bufferSize != requestCount )
		{
			pgpAssert( bufferSize >= kDiskBlockSize );
			bufferSize	= ( bufferSize / kDiskBlockSize ) * kDiskBlockSize;
		}
		
		err	= SetFPos( fromRef, fsFromStart, fromOffset );
		if ( IsntErr( err ) )
		{
			err	= SetFPos( toRef, fsFromStart, toOffset );
		}
		
		if ( IsntErr( err ) )
		{
			UInt32	remaining	= requestCount;
			
			while ( remaining != 0 )
			{
				long	count;
				
				count	= MIN( bufferSize, remaining );
				
				err	= FSRead( fromRef, &count, buffer );
				if ( IsntErr( err ) )
				{
					err	= FSWrite( toRef, &count, buffer );
				}
				if ( IsErr( err ) )
					break;
				
				remaining	-= count;
			}
		}
	
		pgpFreeMac( buffer );
	}
	
	return( err );
}



	OSStatus
FSpOpenFork(
	const FSSpec *	spec,
	SInt8			permission,
	short *			fileRef,
	ForkKind		whichFork )
{
	OSStatus	err	= noErr;
	
	if ( whichFork == kDataFork )
		err	= FSpOpenDF( spec, permission, fileRef );
	else
		err	= FSpOpenRF( spec, permission, fileRef );
	
	return( err );
}


	OSStatus
FSpCopyFork(
	const FSSpec *	fromSpec,
	const FSSpec *	toSpec,
	ForkKind		forkKind )
{
	OSStatus	err	= noErr;
	short		fromRef;
	
	err	= FSpOpenFork( fromSpec, fsRdPerm, &fromRef, forkKind );
	if ( IsntErr( err ) )
	{
		short	toRef;
		
		err	= FSpOpenFork( toSpec, fsRdWrPerm, &toRef, forkKind );
		if ( IsntErr( err ) )
		{
			long	eof;
			
			err	= GetEOF( fromRef, &eof );
			if ( IsntErr( err ) )
			{
				err	= SetEOF( toRef, 0 );
			}
			
			if ( IsntErr( err ) && eof != 0 )
			{
				err	= FSCopyBytes( fromRef, 0, toRef, 0, eof );
			}
			
			FSClose( toRef );
		}
	
		FSClose( fromRef );
	}
	
	return( err );
}


	OSStatus
FSpCopyFiles(
	const FSSpec *	fromSpec,
	const FSSpec *	toSpec )
{
	OSStatus	err	= noErr;
	CInfoPBRec	cpb;
	
	(void)FSpDelete( toSpec );
	
	err	= FSpGetCatInfo( fromSpec, &cpb );
	if ( IsntErr( err ) )
	{
		err	= FSpCreate( toSpec,
			cpbFileCreator( &cpb ), cpbFileType( &cpb ), smSystemScript );
	}

	if ( IsntErr( err ) )
	{
		err	= FSpCopyFork( fromSpec, toSpec, kDataFork );
		if ( IsntErr( err ) )
		{
			err	= FSpCopyFork( fromSpec, toSpec, kResourceFork );
		}
	}
	
	
	if ( IsntErr( err ) )
	{
		err	= FSpGetCatInfo( fromSpec, &cpb );
		if ( IsntErr( err ) )
		{
			/* if the files are in the same directory, make sure they
			don't have the same icon position */
			if ( fromSpec->vRefNum == toSpec->vRefNum &&
					fromSpec->parID == toSpec->parID )
			{
				cpbFInfo( &cpb ).fdLocation.v	+= 32;
				cpbFInfo( &cpb ).fdLocation.h	+= 32;
			}
			
			err	= FSpSetCatInfo( toSpec, &cpb );
		}
	}
	
	return( err );
}







	OSErr
FSWriteString(
	short				fileRef,
	ConstStr255Param	string)
{
	OSErr	err	= noErr;
	
	long	count	= StrLength( string );
	
	err	= FSWrite( fileRef, &count, &string[ 1 ] );
	
	return( err );
}



	OSErr
FSpCreateAndOpenFile(
	const FSSpec *	spec,
	OSType			creator,
	OSType			type,
	SInt8			script,
	ForkKind		forkKind,
	SInt8			perm,
	short *			fileRef )
{
	OSErr	err	= noErr;
	
	err	= FSpCreate( spec, creator, type, script );
	if ( IsntErr( err ) )
	{
		err	= FSpOpenFork( spec, perm, fileRef, forkKind );
	}
	
	return( err );
}



	OSErr
FSpReadBytes(
	const FSSpec *	spec,
	void *			buffer,
	UInt32			requestCount,
	UInt32 *			actualCountPtr )
{
	OSErr	err	= noErr;
	short	fileRef;
	
	AssertSpecIsValid( spec, "FSpReadBytes" );
	
	MacDebug_FillWithGarbage( buffer, requestCount );
	
	err	= FSpOpenDF( spec, fsRdPerm, &fileRef );
	if ( IsntErr( err ) )
	{
		long		eof;
		long		count;
		
		GetEOF( fileRef, &eof );
		
		count	= MIN( eof, requestCount );
		
		err	= FSRead( fileRef, &count, buffer );
		
		if ( IsntNull( actualCountPtr ) )
		{
			*actualCountPtr	= count;
		}
		
		FSClose( fileRef );
	}
	
	AssertNoErr( err, "ReadBytesFromFile" );
	return( err );
}


	Boolean
FSSpecIsValid( const FSSpec *spec )
{
	Boolean		isValid	= FALSE;
	
	isValid	= IsntNull( spec );
	if ( isValid ) 
	{
		const uchar	length	= StrLength( spec->name );
		
		isValid	= spec->vRefNum < 0 && spec->parID != 0 &&
					length >= 1 && length <= 31;
	}
	
	return( isValid );
}

	OSErr
FSpGetForkSizes(
	const FSSpec *	spec,
	UInt32 *			dataForkSize,	// may be nil
	UInt32 *			resForkSize		// may be nil
	)
{
	OSErr		err	= noErr;
	CInfoPBRec	cpb;
	
	err	= FSpGetCatInfo( spec, &cpb );
	if ( IsntErr( err ) )
	{
		if ( IsntNull( dataForkSize ) )
			*dataForkSize	= cpbDataForkSize( &cpb );
			
		if ( IsntNull( resForkSize ) )
			*resForkSize	= cpbResForkSize( &cpb );
	}
	
	return( err );
}




// Called just like FindFolder()
// except the folder is always created if not present.

	OSStatus
FindPGPPreferencesFolder(
	short 	vRefNum,
	short 	*foundVRefNum,
	long 	*foundDirID)
{
	OSStatus	err;
	FSSpec		folderSpec;
	
	pgpAssertAddrValid( foundVRefNum, short );
	pgpAssertAddrValid( foundDirID, long );
	
	err = FindFolder( vRefNum, kPreferencesFolderType, kCreateFolder,
				&folderSpec.vRefNum, &folderSpec.parID );
	if( IsntErr( err ) )
	{
		CInfoPBRec	cpb;
		
		*foundVRefNum = folderSpec.vRefNum;
		
		// This string should _not_ be internationalized.
		CopyPString( "\pPretty Good Preferences", folderSpec.name );
		
		err = FSpGetCatInfo( &folderSpec, &cpb );
		if( IsntErr( err ) )
		{
			if( cpbIsFolder( &cpb ) )
			{
				*foundDirID = cpb.dirInfo.ioDrDirID;
			}
			else
			{
				pgpDebugMsg( "Pretty Good Preferences is a file!" );
				err = dupFNErr;
			}
		}
		else
		{
			err = FSpDirCreate( &folderSpec, smSystemScript, foundDirID );
		}
	}

	return( err );
}



	OSErr
FSpChangeFileType(
	const FSSpec *	spec,
	OSType			newType )
{
	OSErr		err	= noErr;
	CInfoPBRec	cpb;
	
	err	= FSpGetCatInfo( spec, &cpb );
	if ( IsntErr( err ) )
	{
		if ( cpbIsFile( &cpb ) )
		{
			cpbFileType( &cpb )	= newType;
			
			err	= FSpSetCatInfo( spec, &cpb );
		}
		else
		{
			err	= notAFileErr;
		}
	}
	
	return( err );
}



	OSErr
FSWipeBytes(
	short	fileRef,
	UInt32	startOffset,
	UInt32	numBytes,
	uchar	wipeChar )
{
	OSStatus	err	= noErr;
	
	err	= SetFPos( fileRef, fsFromStart, startOffset );
	if ( IsntErr( err ) )
	{
		void *		buffer	= nil;
		UInt32		bufferSize;
		uchar		emergencyBuffer[ kDiskBlockSize ];
		Boolean		disposeBuffer	= FALSE;
		UInt32		remaining;
		
		buffer	= pgpNewPtrMost( numBytes, MIN( kDiskBlockSize, numBytes ),
					kMacMemory_PreferTempMem |
					kMacMemory_UseApplicationHeap, &bufferSize );
		if ( IsntNull( buffer ) )
		{
			disposeBuffer	= TRUE;
		}
		else
		{
			buffer		= &emergencyBuffer[ 0 ];
			bufferSize	= sizeof( emergencyBuffer );
		}
		
		
		pgpFillMemory( buffer, bufferSize, wipeChar );
		
		remaining	= numBytes;
		while( remaining != 0 )
		{
			long	count;
			
			count	= MIN( remaining, bufferSize );
			err	= FSWrite( fileRef, &count, buffer );
			if ( IsErr( err ) )
				break;
				
			remaining	-= count;
		}
		
		if ( disposeBuffer )
		{
			pgpFreeMac( buffer );
		}
	}
	
	return( err );
}



	OSErr
FSpGetFileVersion(
	const FSSpec *	spec,
	UInt16 *		version )
{
	OSStatus		err	= noErr;
	CSaveResFile	saveRes;
	CSaveResLoad	saveResLoad;
	short			fileRef;
	
	*version	= 0;
	
	SetResLoad( FALSE );
	fileRef	= FSpOpenResFile( spec, fsRdPerm );
	SetResLoad( TRUE );
	if ( IsntErr( err ) )
	{
		Handle	versResource;
		
		versResource	= Get1Resource( 'vers', 1 );
		if ( IsntNull( versResource ) )
		{
			*version	= **(const UInt16 **)versResource;
		}
		else
		{
			err	= resNotFound;
		}
		
		CloseResFile( fileRef );
	}
	
	
	return( err );
}

	OSStatus
VolumeIsRemote(short vRefNum, Boolean *isRemote)
{
	HParamBlockRec			pb;
	GetVolParmsInfoBuffer	parmsBuffer;
	Str255					volumeName;
	OSStatus				status;
	
	pgpAssertAddrValid( isRemote, Boolean );
	
	pgpClearMemory( &pb, sizeof( pb ) );
	pgpClearMemory( &parmsBuffer, sizeof( parmsBuffer ) );
	
	volumeName[0] = 0;
	
	parmsBuffer.vMVersion = 2;	// See IM-Files, Page 2-109
	
	pb.volumeParam.ioNamePtr	= &volumeName[0];
	pb.volumeParam.ioVRefNum	= vRefNum;
	pb.ioParam.ioBuffer			= (Ptr) &parmsBuffer;
	pb.ioParam.ioReqCount		= sizeof( parmsBuffer );
	
	status = PBHGetVolParmsSync( &pb );
	
	*isRemote = ( parmsBuffer.vMServerAdr != 0 );
	
	return( status );
}



	static OSStatus
ReadWriteFileBlocksUsingDriver(
	short 				fileRefNum,
	Boolean				doRead,
	const DiskExtent	*extentList,
	UInt32				numExtents,
	UInt32				startBlock,
	UInt32 				blocksToReadWrite,
	void				*buffer,
	UInt32				*blocksReadWritten)
{
	OSStatus	status = noErr;
	
	pgpAssertAddrValid( extentList, DiskExtent );
	AssertFileRefNumIsValid( fileRefNum, "ReadFileBlocksUsingDriver" );
	pgpAssertAddrValid( buffer, VoidAlign );
	pgpAssertAddrValid( blocksReadWritten, UInt32 );
	
	*blocksReadWritten = 0;
	
	if( blocksToReadWrite != 0 )
	{
		const VCB	*vcb;
		UInt32		extentIndex;
		uchar		*curBuffer;
		UInt32		curFileBlock;
		UInt32		blocksRemaining;
		
		vcb 			= GetVCBForFile( fileRefNum );
		curFileBlock	= 0;
		curBuffer		= (uchar *) buffer;
		blocksRemaining	= blocksToReadWrite;
		
		for( extentIndex = 0; extentIndex < numExtents; extentIndex++ )
		{
			const DiskExtent	*curExtent;
			
			curExtent = &extentList[extentIndex];
			
			if( curFileBlock + curExtent->numBlocks > startBlock )
			{
				UInt32	readWriteStartBlock;
				UInt32	numReadWrittenBlocks;
				UInt32	bytesReadWritten;
				UInt32	numBlocksReadWritten;
				
				readWriteStartBlock 	= curExtent->diskBlockIndex;
				numReadWrittenBlocks	= curExtent->numBlocks;

				if( curFileBlock < startBlock )
				{
					// File offset starts in this extent.
					readWriteStartBlock 	+= ( startBlock - curFileBlock );
					numReadWrittenBlocks	-= ( startBlock - curFileBlock );
				}
				
				if( numReadWrittenBlocks > blocksRemaining )
					numReadWrittenBlocks = blocksRemaining;
					
				if( NeedWidePositioning( readWriteStartBlock,
						numReadWrittenBlocks ) )
				{
					XIOParam	pb;
					
					pgpClearMemory( &pb, sizeof( pb ) );
				
					pb.ioVRefNum	= vcb->vcbDrvNum;
					pb.ioRefNum		= vcb->vcbDRefNum;
					pb.ioBuffer		= (Ptr) curBuffer;
					pb.ioReqCount	= numReadWrittenBlocks * kDiskBlockSize;
					pb.ioWPosOffset	= UInt64( 0, readWriteStartBlock ) << 9;
					pb.ioPosMode	= fsFromStart | kWidePosOffsetMask;
					
					if( doRead ) 
					{
						status = PBReadSync( (ParamBlockRec *) &pb );
					}
					else
					{
						status = PBWriteSync( (ParamBlockRec *) &pb );
					}
					
					bytesReadWritten = pb.ioActCount;
				}
				else
				{
					ParamBlockRec	pb;
					
					pgpClearMemory( &pb, sizeof( pb ) );
				
					pb.ioParam.ioVRefNum	= vcb->vcbDrvNum;
					pb.ioParam.ioRefNum		= vcb->vcbDRefNum;
					pb.ioParam.ioBuffer		= (Ptr) curBuffer;
					pb.ioParam.ioReqCount	=
						numReadWrittenBlocks * kDiskBlockSize;
					pb.ioParam.ioPosOffset	=
						readWriteStartBlock * kDiskBlockSize;
					pb.ioParam.ioPosMode	= fsFromStart;
					
					if( doRead )
					{
						status = PBReadSync( &pb );
					}
					else
					{
						status = PBWriteSync( &pb );
					}
					
					bytesReadWritten = pb.ioParam.ioActCount;
				}
				
				numBlocksReadWritten	= bytesReadWritten / kDiskBlockSize;
				*blocksReadWritten 		+= numBlocksReadWritten;
				blocksRemaining			-= numBlocksReadWritten;
				curBuffer 				+= bytesReadWritten;
				
				if( IsErr( status ) || blocksRemaining == 0 )
					break;
			}
			
			curFileBlock += curExtent->numBlocks;
		}
	}
	
	if( IsntErr( status ) && *blocksReadWritten != blocksToReadWrite )
	{
		status = eofErr;
	}
	
	return( status );
}

	OSStatus
FSReadFileUsingDriver(
	short 	fileRefNum,
	UInt32	readOffset,
	UInt32 	bytesToRead,
	void	*buffer,
	UInt32	*callerBytesRead)
{
	OSStatus	status = noErr;
	UInt32		bytesRead = 0;
	
	AssertFileRefNumIsValid( fileRefNum, "FSReadFileUsingDriver" );
	pgpAssertAddrValid( buffer, VoidAlign );
	AssertAddrNullOrValid( callerBytesRead, UInt32, "FSReadFileUsingDriver" );
	
	if( bytesToRead != 0 )
	{
		DiskExtent	*extentsList;
		PGPUInt32	numExtents;
		
		status = GetDiskExtentsForFork( fileRefNum, &extentsList, &numExtents );
		if( IsntErr( status ) )
		{
			uchar	*curBuffer;
			UInt32	curFileBlock;
			UInt32	blocksRead;
			UInt32	partialBytes;
			UInt32	bytesRemaining;
			uchar	tempBuffer[kDiskBlockSize];
			
			curBuffer 		= (uchar *) buffer;
			curFileBlock	= readOffset / kDiskBlockSize;
			bytesRemaining	= bytesToRead;
			
			// Do initial partial block, if any
		
			partialBytes = readOffset % kDiskBlockSize;
			if( partialBytes != 0 )
			{
				status = ReadWriteFileBlocksUsingDriver( fileRefNum, TRUE,
							extentsList, numExtents, curFileBlock, 1,
							tempBuffer, &blocksRead );
				if( IsntErr( status ) )
				{
					UInt32	bytesToCopy;
					UInt32	blockOffset;
					
					bytesToCopy = kDiskBlockSize - partialBytes;
					blockOffset = partialBytes;
					
					BlockMoveData( &tempBuffer[partialBytes],
						curBuffer, bytesToCopy );
					
					curBuffer 		+= bytesToCopy;
					curFileBlock	+= 1;
					bytesRemaining	-= bytesToCopy;
					bytesRead		+= bytesToCopy;
				}
			}
			
			// Do remaining whole blocks
			if( IsntErr( status ) && bytesRemaining > 0 )
		 	{
				UInt32	wholeBlocks;
				
				wholeBlocks = bytesRemaining / kDiskBlockSize;
				if( wholeBlocks > 0 )
				{
					status = ReadWriteFileBlocksUsingDriver( fileRefNum,
								TRUE,
								extentsList, numExtents, curFileBlock,
								wholeBlocks, curBuffer, &blocksRead );
					if( IsntErr( status ) )
					{
						UInt32	numBytesRead;
						
						numBytesRead	= blocksRead * kDiskBlockSize;
						curBuffer 		+= numBytesRead;
						bytesRemaining 	-= numBytesRead;
						bytesRead		+= numBytesRead;
						curFileBlock	+= blocksRead;
					}
				}
			}
			
			// Do last partial block, if any
			if( IsntErr( status ) && bytesRemaining > 0 )
		 	{
				pgpAssert( bytesRemaining < kDiskBlockSize );
				
				status = ReadWriteFileBlocksUsingDriver(
							fileRefNum, TRUE, extentsList,
							numExtents, curFileBlock, 1, tempBuffer,
							&blocksRead );
				if( IsntErr( status ) )
				{
					BlockMoveData( &tempBuffer[0], curBuffer, bytesRemaining);
					
					bytesRead += bytesRemaining;
				}
			}
		
			FreeDiskExtentList( extentsList );
		}
	}
	
	if( IsntNull( callerBytesRead ) )
		*callerBytesRead = bytesRead;
		
	pgpAssert( bytesRead == bytesToRead );
	
	return( status );
}



	OSStatus
FSWriteFileUsingDriver(
	short 		fileRefNum,
	UInt32		writeOffset,
	UInt32 		bytesToWrite,
	const void	*buffer,
	UInt32		*callerBytesWritten)
{
	OSStatus	status = noErr;
	UInt32		bytesWritten = 0;
	
	AssertFileRefNumIsValid( fileRefNum, "FSWriteFileUsingDriver" );
	pgpAssertAddrValid( buffer, VoidAlign );
	AssertAddrNullOrValid( callerBytesWritten, UInt32,
		"FSWriteFileUsingDriver" );
	
	if( bytesToWrite != 0 )
	{
		DiskExtent	*extentsList;
		PGPUInt32	numExtents;
		
		status = GetDiskExtentsForFork( fileRefNum, &extentsList, &numExtents);
		if( IsntErr( status ) )
		{
			const uchar	*curBuffer;
			UInt32		curFileBlock;
			UInt32		blocksWritten;
			UInt32		partialBytes;
			UInt32		bytesRemaining;
			uchar		tempBuffer[kDiskBlockSize];
			
			curBuffer 		= (const uchar *) buffer;
			curFileBlock	= writeOffset / kDiskBlockSize;
			bytesRemaining	= bytesToWrite;
			
			// Do initial partial block, if any
			partialBytes = writeOffset % kDiskBlockSize;
			if( partialBytes != 0 )
			{
				UInt32	blocksRead;
				
				status = ReadWriteFileBlocksUsingDriver( fileRefNum, TRUE,
							extentsList, numExtents,
							curFileBlock, 1, tempBuffer,
							&blocksRead );
				if( IsntErr( status ) )
				{
					UInt32	bytesToCopy;
					UInt32	blockOffset;
					
					bytesToCopy = kDiskBlockSize - partialBytes;
					blockOffset = partialBytes;
					
					BlockMoveData( curBuffer,
						&tempBuffer[partialBytes], bytesToCopy );
					
					status = ReadWriteFileBlocksUsingDriver( fileRefNum,
							FALSE,
							extentsList, numExtents, curFileBlock, 1,
							tempBuffer, &blocksWritten );
					if( IsntErr( status ) )
					{
						curBuffer 		+= bytesToCopy;
						curFileBlock	+= 1;
						bytesRemaining	-= bytesToCopy;
						bytesWritten	+= bytesToCopy;
					}
				}
			}
			
			// Do remaining whole blocks
			if( IsntErr( status ) && bytesRemaining > 0 )
		 	{
				UInt32	wholeBlocks;
				
				wholeBlocks = bytesRemaining / kDiskBlockSize;
				if( wholeBlocks > 0 )
				{
					status = ReadWriteFileBlocksUsingDriver( fileRefNum,
							FALSE, extentsList, numExtents, curFileBlock,
							wholeBlocks, (uchar *)curBuffer, &blocksWritten );
					if( IsntErr( status ) )
					{
						UInt32	numBytesWritten;
						
						numBytesWritten	= blocksWritten * kDiskBlockSize;
						curBuffer 		+= numBytesWritten;
						bytesRemaining 	-= numBytesWritten;
						bytesWritten	+= numBytesWritten;
						curFileBlock	+= blocksWritten;
					}
				}
			}
			
			// Do last partial block, if any
			if( IsntErr( status ) && bytesRemaining > 0 )
		 	{
				pgpAssert( bytesRemaining < kDiskBlockSize );
				
				status = ReadWriteFileBlocksUsingDriver( fileRefNum,
							TRUE, extentsList,
							numExtents, curFileBlock,
							1, tempBuffer, &blocksWritten );
				if( IsntErr( status ) )
				{
					BlockMoveData( curBuffer, &tempBuffer[0], bytesRemaining);
					
					status = ReadWriteFileBlocksUsingDriver( fileRefNum,
							FALSE, extentsList,
							numExtents, curFileBlock, 1, tempBuffer,
							&blocksWritten );

					bytesWritten += bytesRemaining;
				}
			}
		
			FreeDiskExtentList( extentsList );
		}
	}
	
	if( IsntNull( callerBytesWritten ) )
		*callerBytesWritten = bytesWritten;
		
	pgpAssert( bytesWritten == bytesToWrite );
	
	return( status );
}



	OSStatus
FSReadUncached(
	short			fileRefNum,
	long *			count,
	void *			buffer )
{
	ParamBlockRec	pb;
	OSErr			err	= noErr;
	
	pb.ioParam.ioRefNum		= fileRefNum;
	pb.ioParam.ioBuffer		= (Ptr)buffer;
	pb.ioParam.ioReqCount	= *count;
	pb.ioParam.ioPosMode	= fsFromMark | kCacheDisable;
	pb.ioParam.ioPosOffset	= 0;
	
	err	= PBReadSync( &pb );
	
	return( err );
}


	OSStatus
FSWriteUncached(
	short			fileRefNum,
	long *			count,
	const void *	buffer )
{
	ParamBlockRec	pb;
	OSErr			err	= noErr;
	
	pb.ioParam.ioRefNum		= fileRefNum;
	pb.ioParam.ioBuffer		= (Ptr)buffer;
	pb.ioParam.ioReqCount	= *count;
	pb.ioParam.ioPosMode	= fsFromMark | kCacheDisable;
	pb.ioParam.ioPosOffset	= 0;
	
	err	= PBWriteSync( &pb );
	
	return( err );
}


	Boolean
FSIsTrashFolder(
	short	vRefNum,
	long	dirID )
{
	short		trashVRefNum;
	long		trashDirID;
	
	(void)FindFolder( vRefNum,
		kTrashFolderType, FALSE, &trashVRefNum, &trashDirID);
	
	return( dirID == trashDirID );
}

	OSStatus
DirContainsFSSpec(
	short 			containingVRefNum,
	long			containingDirID,
	const FSSpec 	*childSpec,
	Boolean			*isContained)
{
	OSStatus	macErr;
	CInfoPBRec	cpb;

	pgpAssertAddrValid( isContained, Boolean );

	*isContained = FALSE;
	
	macErr = FSpGetCatInfo( childSpec, &cpb );
	if( IsntErr( macErr ) )
	{
		if( containingVRefNum == cpb.hFileInfo.ioVRefNum )
		{
			// Iterate through childs parents until we reaching the
			// containing dirID or the root.
			
			while( TRUE )
			{
				if( cpb.hFileInfo.ioFlParID == containingDirID )
				{
					*isContained = TRUE;
					break;
				}
				else if( cpb.hFileInfo.ioFlParID == fsRtParID )
				{
					break;
				}
				else
				{
					Str255	dirName;
			
					cpb.dirInfo.ioVRefNum 	= containingVRefNum;
					cpb.dirInfo.ioDrDirID 	= cpb.hFileInfo.ioFlParID;
					cpb.dirInfo.ioNamePtr	= dirName;
					cpb.dirInfo.ioFDirIndex	= -1;
					
					macErr = PBGetCatInfoSync( &cpb );
					if( IsErr( macErr ) )
						break;
				}
			}
		}
	}
	
	return( macErr );
}

	OSStatus
FSSpecContainsFSSpec(
	const FSSpec 	*containingSpec,
	const FSSpec 	*childSpec,
	Boolean			*isContained)
{
	OSStatus	macErr;
	CInfoPBRec	cpb;
	
	AssertSpecIsValid( containingSpec, "FSSpecContainsFSSpec" );
	AssertSpecIsValid( childSpec, "FSSpecContainsFSSpec" );
	pgpAssertAddrValid( isContained, Boolean );
	
	*isContained = FALSE;

	macErr = FSpGetCatInfo( containingSpec, &cpb );
	if( IsntErr( macErr ) && cpbIsFolder( &cpb ) )
	{
		macErr = DirContainsFSSpec( cpb.dirInfo.ioVRefNum,
					cpb.dirInfo.ioDrDirID, childSpec,
					isContained );
	}
	
	return( macErr );
}

	
























