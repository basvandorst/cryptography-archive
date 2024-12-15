/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: MacFiles.h,v 1.23 1997/10/22 01:07:17 heller Exp $
____________________________________________________________________________*/

#pragma once

#include <Files.h>

#include "pgpBase.h"
#include "pgpTypes.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


typedef enum
{
	kDataFork = 0,
	kResourceFork
	
} ForkKind;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

/* A block in an extent is always 512 bytes */

typedef struct DiskExtent
{
	PGPUInt32	diskBlockIndex;
	PGPUInt32	numBlocks;
	
} DiskExtent;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif


#define kDiskBlockSize				512UL
#define	kMaxHFSFileNameLength		31

#define	kHFSVolumeSignature			0x4244	/* 'BD' */
#define	kHFSPlusVolumeSignature		0x482B	/* 'H+' */

#define	kFileAttributeDataForkOpenMask		0x08
#define	kFileAttributeResourceForkOpenMask	0x04
#define	kFileAttributeFileOpenMask	\
			( kFileAttributeDataForkOpenMask |	\
			 kFileAttributeResourceForkOpenMask | 0x80 )
											  
#define	kFileAttributeFileLockedMask		0x01


#define	kVolumeAttributeSoftwareLockMask	0x8000
#define	kVolumeAttributeHardwareLockMask	0x0080
#define	kVolumeAttributeLockMask	( kVolumeAttributeSoftwareLockMask | \
									kVolumeAttributeHardwareLockMask )
											  
											  
#define cpbFileAttributes(cpb)		( (cpb)->hFileInfo.ioFlAttrib )
#define cpbFolderAttributes(cpb)	( (cpb)->hFileInfo.ioFlAttrib )
#define cpbAttributes(cpb)			( (cpb)->hFileInfo.ioFlAttrib )
#define cpbFileType(cpb)			( (cpb)->hFileInfo.ioFlFndrInfo.fdType )
#define cpbFileCreator(cpb)			( (cpb)->hFileInfo.ioFlFndrInfo.fdCreator )
#define cpbFileFlags(cpb)			( (cpb)->hFileInfo.ioFlFndrInfo.fdFlags )
#define cpbCreationDate(cpb)		( (cpb)->hFileInfo.ioFlCrDat )
#define cpbModificationDate(cpb)	( (cpb)->hFileInfo.ioFlMdDat )
#define cpbDataForkSize(cpb)		( (cpb)->hFileInfo.ioFlLgLen )
#define cpbResForkSize(cpb)			( (cpb)->hFileInfo.ioFlRLgLen )
#define cpbFInfo(cpb)				( (cpb)->hFileInfo.ioFlFndrInfo )

#define	cpbIsFile(cpb)				\
	( ( cpbAttributes( cpb ) & ioDirMask ) == 0 )
#define	cpbIsFolder(cpb)			\
	( ( cpbAttributes( cpb ) & ioDirMask ) != 0 )

#define cpbIsOpen(cpb)				\
	( ( cpbAttributes( cpb ) & kFileAttributeFileOpenMask ) != 0 )
#define cpbIsLocked(cpb)			\
	( ( cpbFileAttributes( cpb ) & kFileAttributeFileLockedMask ) != 0 )



OSStatus	FSpGetCatInfo(const FSSpec *itemSpec, CInfoPBRec *itemCatalogInfo);
OSStatus	FSpSetCatInfo(const FSSpec *itemSpec, CInfoPBRec *itemCatalogInfo);
Boolean		FSpEqual(const FSSpec *spec1, const FSSpec *spec2);
Boolean		FSpExists( const FSSpec *spec );
OSStatus	FlushFile(short fileRefNum);


OSStatus	GetVolumeBlockCounts(short vRefNum, UInt32 *freeBlocks,
				UInt32 *totalBlocks, UInt32 *blockSize);
						
void		GetAllVRefNums( short * vRefNums,
				UInt16 maxVRefNums, UInt16 * numVRefNums );
void		FlushAllVolumes( void );

OSStatus	FSReadUncached( short fileRefNum, long *count, void *buffer );
OSStatus	FSWriteUncached( short fileRefNum,
				long *count, const void *buffer );

OSStatus	FSReadAtOffset(short fileRefNum,
				UInt32 readOffset, UInt32 bytesToRead,
				void *buffer, UInt32 *bytesRead, Boolean restoreFilePos);
OSStatus	FSWriteAtOffset(short fileRefNum,
				UInt32 writeOffset, UInt32 bytesToWrite,
				const void *buffer, UInt32 *bytesWritten,
				Boolean restoreFilePos);
						
OSErr		GetVolumeName( short vRefNum, StringPtr name );
OSErr		GetVolumeDates( short vRefNum,
				long * creationDate, long * modificationDate );

OSErr		GetDiskExtentsForFork(short fileRefNum, DiskExtent **extentsListPtr,
				PGPUInt32 *numExtents);
void		FreeDiskExtentList(DiskExtent *theList);
	
struct FCBRec;

const struct FCBRec *	GetFCBForFile( short fileRefNum );
const VCB *		GetVCBForFile( short fileRefNum );
OSStatus		GetVolumeDriveNumberAndRefNum( short vRefNum,
					short *driveNumber, short *driverRefNum );
const VCB *		GetVCBForVolume( short vRefNum );
void			GetDriveNumberAndRefNumForVCB( const VCB *vcb,
					short *driveNumber, short *driverRefNum );

short			GetVolumeForFile( short fileRefNum );

/* direct VCB access...avoid if possible */
inline UInt32	GetVCBAllocBlockSize( const VCB *vcb )
					{ return( vcb->vcbAlBlkSiz ); }
UInt32			AllocBlockNumberToDiskBlockNumber( const VCB *vcb,
					UInt32 allocBlockIndex );

/* use of these routines is encouraged over direct VCB access */
UInt32			GetVolumeFreeAllocBlocks( short vRefNum );
UInt32			GetVolumeAllocBlockSize( short vRefNum );
OSStatus		GetVolumeDriveNumberAndRefNum( short vRefNum,
					short *driveNumber, short *driverRefNum );

Boolean		FSSpecIsValid( const FSSpec *spec );

OSStatus	FSGetUniqueName(short vRefNum,
				long dirID, ConstStr255Param baseName, StringPtr uniqueName);
						
OSStatus	FSpGetUniqueSpec(const FSSpec *spec, FSSpec *outSpec);

OSStatus	FSpGetFullPath(const FSSpec *fileSpec, StringPtr path);


OSStatus	GetSpecFromRefNum( short refNum, FSSpec *spec);
OSStatus	GetSpecFromFCBTable( short refNum, FSSpec *spec);
OSStatus	FSpFindOpenForkRef(FSSpec * inSpec,
				Boolean inResFork, SInt16 * outRefNum);


OSStatus	FSpDeleteResourceFork( const FSSpec *spec );

OSStatus	FSpOpenFork( const FSSpec *spec, SInt8 permission,
				short * fileRef, ForkKind whichFork );


OSStatus	FSpCopyFork( const FSSpec *	fromSpec,
				const FSSpec * toSpec, ForkKind forkKind );
				
OSStatus	FSCopyBytes( short fromRef, UInt32 fromOffset,
				short toRef, UInt32 toOffset, UInt32 numBytes );
				
OSStatus	FSpCopyFiles( const FSSpec *fromSpec, const FSSpec *toSpec );


OSErr		FSWriteString( short fileRef, ConstStr255Param	string);


OSErr		FSpCreateAndOpenFile( const FSSpec *spec,
				OSType creator, OSType type,
				SInt8 script, ForkKind, SInt8 perm, short *fileRef );

OSErr		FSpReadBytes( const FSSpec *spec, void * buffer,
				UInt32 requestCount, UInt32 *actualCount );

OSErr		FSpGetForkSizes( const FSSpec *spec,
				UInt32 *dataForkSize, UInt32 *resForkSize );

OSStatus	FindPGPPreferencesFolder(short vRefNum,
				short *foundVRefNum, long *foundDirID);


OSErr		FSpChangeFileType( const FSSpec *spec, OSType newType );
OSStatus	VolumeIsRemote(short vRefNum, Boolean *isRemote);

OSErr		FSWipeBytes( short fileRefNum, UInt32 startOffset,
				UInt32 numBytes, uchar wipeChar );

OSErr		FSpGetFileVersion( const FSSpec *spec, UInt16 *version );

OSStatus	FSReadFileUsingDriver(short fileRefNum,
				UInt32 readOffset, UInt32 bytesToRead,
				void *buffer, UInt32 *bytesRead);
OSStatus	FSWriteFileUsingDriver(short fileRefNum,
				UInt32 readOffset, UInt32 bytesToWrite,
				const void *buffer, UInt32 *callerBytesWritten);
						

Boolean		FSIsTrashFolder( short vRefNum, long dirID );

OSStatus	FSSpecContainsFSSpec(const FSSpec *containingSpec,
				const FSSpec *childSpec, Boolean *isContained);

OSStatus	DirContainsFSSpec(short containingVRefNum,
				long containingDirID, const FSSpec *childSpec,
				Boolean *isContained);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


					





