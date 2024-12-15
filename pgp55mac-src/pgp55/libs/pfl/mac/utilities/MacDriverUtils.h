/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: MacDriverUtils.h,v 1.9 1997/09/18 01:34:43 lloyd Exp $
____________________________________________________________________________*/

#pragma once

#include <Devices.h>

/* A DrvQEl with flags */
#pragma options align=mac68k
typedef struct FlagsDrvQEl
{
	long			flags;
	DrvQEl			el;
} FlagsDrvQEl;
#pragma options align=reset

// flags for FlagsDrvQEl.flags
const long	kDriveLockedMask			= 0x80000000;	// see Files 2-85
const long	kDiskInDriveMask			= 0x00010000;	// see Files 2-85
const long	kDriveIsNotEjectableMask	= 0x00080000;	// see Files 2-85
const long	kDriveWantsEjectCallMask	= 0x00400000;	// see Files 2-85



const UInt16	kMaxUnitTableEntries	= 128;
const UInt16	kMinUnitNum = 48;

const OSErr	kAsyncIOInProgress	= 1;

struct FCBRec;
struct VCB;

const VCB *		GetVCBForDrive( short driveNumber );

OSStatus		GetDriverRefNumForDrive( short driveNumber,
					short *driverRefNum );

const DrvQEl *	GetDriveQElem( short driveNumber );


const short	kWidePosOffsetMask	= ( (short)1 << kWidePosOffsetBit );

Boolean			NeedWidePositioning( UInt32 startBlock,	UInt32 numBlocks );

inline Boolean	PBUsesWidePositioning( const IOParam *pb )
					{ return( ( pb->ioPosMode & kWidePosOffsetMask ) != 0 ); }
					
inline Boolean	PBUsesWidePositioning( const ParamBlockRec *pb )
					{ return( PBUsesWidePositioning( &pb->ioParam) ); }


UInt32			GetPBStartBlock( const IOParam  *ioPB );
inline UInt32	GetPBStartBlock( const ParamBlockRec  *pb )
{
	return( GetPBStartBlock( &pb->ioParam ) );
}

UInt32			GetPBNumBlocks( const IOParam  *ioPB );
inline UInt32	GetPBNumBlocks( const ParamBlockRec  *pb )
{
	return( GetPBNumBlocks( &pb->ioParam ) );
}


inline Boolean	PBIsAsync( const IOParam *pb )
{
	return( ( pb->ioTrap & 0x0400) != 0 );
}

inline Boolean	PBIsSync( const IOParam *pb )
{
	return( ( pb->ioTrap & 0x0400) == 0 );
}

inline Boolean	PBIsAsync( const ParamBlockRec *pb )
{
	return( PBIsAsync( &pb->ioParam ) );
}

inline Boolean	PBIsSync( const ParamBlockRec *pb )
{
	return( PBIsSync( &pb->ioParam ) );
}


OSErr		GetDriverNameUsingRefNum( short driverRefNum, StringPtr name);

OSErr		SetDriverName( DRVRHeader * header, ConstStr255Param name);
OSErr		SetDriverName( DCtlHandle, ConstStr255Param name);
OSErr		SetDriverName( short driverRefNum, ConstStr255Param name);

Boolean	DriverIsInstalled( ConstStr255Param name, short * refNum);


void			GetDriverNameFromDCtlHdl( DCtlHandle dCtl, StringPtr name);
DRVRHeader *	GetDriverHeaderPtrFromDCtlHdl( DCtlHandle dCtl );

inline short	DriverRefNumToUnitNum( short driverRefNum )
{
	return( -1 * ( driverRefNum + 1 ) );
}
					
DCtlHandle		GetIndDCtlEntry( UInt16	unitNum);
UInt16			GetNumDCtlEntries( void );


OSErr		GetUnusedDriverRefNum (short *drvrRefNum);

OSErr		RemoveDriveQEl( short driveNumber, DrvQEl	**	drivePtr);

