/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDriverUtils.cp,v 1.7 1997/10/14 21:32:42 heller Exp $
____________________________________________________________________________*/

#include <FSM.h>

#include "pgpMem.h"
#include "MacDriverUtils.h"
#include "UInt64.h"
#include "MacStrings.h"





const UInt32	kDiskBlockSize	= 512L;
	
	


/*____________________________________________________________________________
	Allocates and switches to a larger unit table.  If the current unit table
	has less than kMinUnitNum entries, the unit table is increased to
	kMinUnitNum + 16.  If the current unit table has more than kMinUnitNum
	entries, the unit table is increased by 4 entries up to
	kMaxUnitTableEntries.
____________________________________________________________________________*/
	static OSErr
GrowUnitTable(void)
{
	OSErr	err = noErr;
	Ptr		oldUnitTableBase;
	short	oldUnitEntryCount;
	Ptr		newUnitTableBase;
	short	newUnitEntryCount;
	
	oldUnitTableBase = LMGetUTableBase();
	oldUnitEntryCount = LMGetUnitTableEntryCount();
	if (oldUnitEntryCount < kMinUnitNum)
		newUnitEntryCount = 64;
	else
		newUnitEntryCount = oldUnitEntryCount + 4;
		
	/* The unit table cannot grow past kMaxUnitTableEntries (128) */
	if(newUnitEntryCount <= kMaxUnitTableEntries)
	{
		/* Allocate the new unit table */
		newUnitTableBase = NewPtrSysClear(newUnitEntryCount * sizeof(long));
		if (newUnitTableBase != nil)
		{
			MacLeaks_IgnoreItem( newUnitTableBase );
			
	/*	Copy the old unit table into the new. Although the 
	**	"Driver Education" Tech Note says that you need to disable
	**	interrupts around this operations, you don't.
	**  You only need to
	**	perform the steps in the right order because anything that uses
	**	the unit table at interrupt time shouldn't be saving the UTableBase
	**	or UnitNtryCnt values between calls (if they do, they'll break even
	**	if we do disable interrupts around these steps).
	*/
		
			/* First, copy the current unit table into the new unit table */
			BlockMoveData(oldUnitTableBase,
				newUnitTableBase, oldUnitEntryCount * 4);
		
			/* Now, switch to the new unit table. After this step, the */
			/* Device Manager will be using the new unit table. */
			LMSetUTableBase(newUnitTableBase);
		
			/* Now, tell the system the table is larger. */
			LMSetUnitTableEntryCount(newUnitEntryCount);
		
		
			/* Everything is switched, so we can get
				rid of the old unit table */
			MacLeaks_RememberPtr( oldUnitTableBase,
				kMacLeaks_NewPtrSysClearAllocType, 0, "dummy", 0);
				
			/* Everything switched, so we can get rid of the old unit table */
			DisposePtr(oldUnitTableBase);
		}
		else
		{
			pgpDebugPStr("\pGrowUnitTable: "
			"New unit table could not be allocated");
			err = unitTblFullErr;
		}
		
	}
	else
	{
		pgpDebugPStr("\pGrowUnitTable: "
			"Unit table cannot grow past kMaxUnitTableEntries (128)");
		err = unitTblFullErr;
	}
	
	return (err);
}


/*____________________________________________________________________________
	Finds the first unused unit table entry >= kMinUnitNum. Returns 0 if no
	unused entry is found.
____________________________________________________________________________*/
	static Boolean
FindUnusedDriverRefNum( short *refNum )
{
	const AuxDCEPtr	*unitTable;
	short		unitEntryCount;
	short		unitNumber;
	Boolean		foundOne	= FALSE;

	*refNum	= 0;
	
	unitTable		= (const AuxDCEPtr *)LMGetUTableBase();
	unitEntryCount	= LMGetUnitTableEntryCount();
	
	unitNumber	= kMinUnitNum;
	while ( unitNumber < unitEntryCount )
	{
		if (  IsNull( unitTable[ unitNumber ] ))
		{
			// empty entry, calculate its driver reference number 
			*refNum	= -1 * (unitNumber +1);
			foundOne	= TRUE;
			break;
		}
		
		++unitNumber;
	}
	
	return( foundOne );
}



/*____________________________________________________________________________
	Gets a reference number for our driver to use.  Grows the unit table if
	if is less than kMinUnitNum or if there are no empty entries in the
	unit table.
____________________________________________________________________________*/
	OSErr
GetUnusedDriverRefNum(short *driverRefNum)
	{
	OSErr	err = noErr;
	
	pgpAssertAddrValid( driverRefNum, short );
	
	*driverRefNum = 0;
	
	/* See if we need to resize before we even look for an empty entry */
	if (LMGetUnitTableEntryCount() < kMinUnitNum)
		/* yep, so do it */
		err = GrowUnitTable();
	
	if (err == noErr)
		{
		Boolean	foundOne	= FALSE;
		
		foundOne	= FindUnusedDriverRefNum( driverRefNum );
		
		if ( ! foundOne )
			{
			err	= GrowUnitTable();
			
			if ( IsntErr( err ) )
				{
				foundOne	= FindUnusedDriverRefNum( driverRefNum );
				}
			}
			
		if ( ! foundOne )
			{
			err = unitTblFullErr;
			}
		}
	
	return ( err );
	}




/*____________________________________________________________________________
	Find the drive queue element for driveNum in the drive queue and Dequeue it.
	Return pointer to the drive queue element removed in *drivePtr.
____________________________________________________________________________*/

	OSErr
RemoveDriveQEl(
	short		driveNum,
	DrvQEl	**	drivePtr)
{
	QHdr *		driveQHdr;
	DrvQEl *	curDrive;
	Boolean		foundIt	= FALSE;
	OSErr		err	= nsDrvErr;

	pgpAssert( driveNum > 0 );
	pgpAssertAddrValid( drivePtr, DrvQEl *);
	
	*drivePtr	= nil;
	
	driveQHdr = GetDrvQHdr();
	
	curDrive = (DrvQEl *)driveQHdr->qHead;
	while ( IsntNull( curDrive ) )
		{
		if ( driveNum == curDrive->dQDrive )
			{
			foundIt	= TRUE;
			break;
			}
			
		curDrive = (DrvQEl *)( curDrive->qLink );
		}
	
	if ( foundIt )
		{
		*drivePtr	= curDrive;
		err			= Dequeue( (QElem *)curDrive, driveQHdr);
		}
	else
		{
		err	= nsDrvErr;
		}
	
	return( err );
}



	const VCB *
GetVCBForDrive( short driveNumber )
	{
	pgpAssert( driveNumber > 0 );
	
	const VCB *	vcb	= (const VCB *)GetVCBQHdr()->qHead;
	
	while ( IsntNull( vcb ) )
		{
		if ( vcb->vcbDrvNum == driveNumber )
			break;
			
		vcb	= (const VCB *)vcb->qLink;
		}
	
	return( vcb );
	}
	





	UInt32
GetPBStartBlock( const IOParam  *ioPB )
	{
	pgpAssertAddrValid( ioPB, IOParam );
	
	UInt32	startBlock;
	
	if ( PBUsesWidePositioning( ioPB ) )
		{
		pgpAssert( kDiskBlockSize == 512 );	// 2^9
		startBlock	= ( UInt64( ((XIOParam *)ioPB)->ioWPosOffset ) >> 9 ).lo;
		}
	else
		{
		startBlock	= ioPB->ioPosOffset / kDiskBlockSize;
		}
		
	return( startBlock );
	}


	UInt32
GetPBNumBlocks( const IOParam  *ioPB )
	{
	pgpAssertAddrValid( ioPB, IOParam );
	
	return( ioPB->ioReqCount / kDiskBlockSize );
	}


	UInt16
GetNumDCtlEntries( void )
	{
	return( LMGetUnitTableEntryCount() );
	}


	DCtlHandle
GetIndDCtlEntry( UInt16	unitNum)
	{
	const DCtlHandle *	unitTable;
	
	unitTable	= (const DCtlHandle *)LMGetUTableBase();
	
	return( unitTable[ unitNum ] );
	}


	DRVRHeader *
GetDriverHeaderPtrFromDCtlHdl( DCtlHandle	dCtl)
	{
	DRVRHeader *	drvrPtr;
	
	if ( ((**dCtl).dCtlFlags & dRAMBasedMask) != 0 )
		{
		drvrPtr	= (DRVRHeader *) *(Handle) ((**dCtl).dCtlDriver);
		}
	else
		{
		drvrPtr	= (DRVRHeader  *) ((*dCtl)->dCtlDriver);
		}
		
	return( drvrPtr );
	}
	
	
	void
GetDriverNameFromDCtlHdl(
	DCtlHandle	dCtl,
	StringPtr	name)
	{
	AssertHandleIsValid( dCtl, "GetDriverNameFromDCtlHdl");
	
	const DRVRHeader *	drvrPtr;
	
	drvrPtr	= GetDriverHeaderPtrFromDCtlHdl( dCtl );
	if ( IsntNull( drvrPtr ) )
		{
		CopyPString( drvrPtr->drvrName, name );
		}
	}



	Boolean
DriverIsInstalled(
	ConstStr255Param	name,
	short *				refNum)
{
	pgpAssertAddrValid( name, uchar );
	
	*refNum	= 0;
	
	UInt16 numDCtlEntries	= GetNumDCtlEntries();
	
	for( UInt16 unitNum = 0; unitNum < numDCtlEntries; unitNum++)
		{
		DCtlHandle		curDCtlHndl	= GetIndDCtlEntry( unitNum );
		
		if( IsntNull( curDCtlHndl ) && IsntNull( (**curDCtlHndl).dCtlDriver ))
			{
			Str255	driverName;
			
			GetDriverNameFromDCtlHdl( curDCtlHndl, driverName );
			
			if( EqualString( name, driverName, 0, 1) )
				{
				*refNum	= -( unitNum + 1 );
				return( TRUE );
				}
			}
		}
	
	return FALSE;
}



	OSErr
GetDriverNameUsingRefNum(
	const short	driverRefNum,
	StringPtr	driverName )
	{
	pgpAssert( driverRefNum < 0 );
	
	OSErr		err	= noErr;

	driverName[ 0 ]	= 0;
			
	if ( driverRefNum < 0 )
		{
		DCtlHandle	entry;
	
		entry	= GetIndDCtlEntry( DriverRefNumToUnitNum( driverRefNum ) );
		
		if ( IsntNull( entry ) )
			{
			GetDriverNameFromDCtlHdl( entry, driverName);
			}
		else
			{
			err		= paramErr;
			}
		}
	else
		{
		err	= paramErr;
		}
	
	return( err );
	}
	
	
	OSErr
SetDriverName(
	DRVRHeader *		header,
	ConstStr255Param	newName )
	{
	pgpAssertAddrValid( header, DRVRHeader );
	
	OSErr		err	= noErr;
	
	if ( IsntNull( header ) )
		{
		if ( StrLength( newName ) <= StrLength( header->drvrName ) )
			{
			CopyPString( newName, header->drvrName );
			}
		else
			{
			err	= paramErr;
			}
		}
	else
		{
		err		= paramErr;
		}
	
	return( err );
	}


	OSErr
SetDriverName(
	DCtlHandle			dCtlHdl,
	ConstStr255Param	newName )
	{
	AssertHandleIsValid( dCtlHdl, "SetDriverName" );
	
	OSErr		err	= noErr;
	
	if ( IsntNull( dCtlHdl ) )
		{
		DRVRHeader *	header	= GetDriverHeaderPtrFromDCtlHdl( dCtlHdl );
		
		err	= SetDriverName( header, newName );
		}
	else
		{
		err		= paramErr;
		}
	
	return( err );
	}
	
	
	
	
	OSErr
SetDriverName(
	short				driverRefNum,
	ConstStr255Param	newName )
	{
	pgpAssert( driverRefNum < 0 );
	OSErr		err	= noErr;
	DCtlHandle	entry;

	entry	= GetIndDCtlEntry( DriverRefNumToUnitNum( driverRefNum ) );
	
	err	= SetDriverName( entry, newName );
	
	return( err );
	}
	

	OSStatus
GetDriverRefNumForDrive(
	short	driveNumber,
	short *	driverRefNum)
	{
	OSStatus		err	= nsDrvErr;
	const DrvQEl	*driveQEl = nil;
	
	*driverRefNum	= 0;
	
	driveQEl = (const DrvQEl *)(GetDrvQHdr()->qHead);
	while( IsntNull( driveQEl ) )
		{
		if( driveQEl->dQDrive == driveNumber )
			{
			*driverRefNum	= driveQEl->dQRefNum;
			err	= noErr;
			break;
			}
			
		driveQEl	= (const DrvQEl *)(driveQEl->qLink);
		}
		
	return( err );
	}
	

/*____________________________________________________________________________
	Return TRUE if the proposed disk drive request needs
	to use wide positioning.
____________________________________________________________________________*/
	Boolean
NeedWidePositioning(
	UInt32	startBlock,
	UInt32	numBlocks )
	{
	// comparison should be strictly '>',
	// but who knows if this will derange some driver
	// when the sum equals 4GB
	return( startBlock + numBlocks >= kMaximumBlocksIn4GB );
	}


	const DrvQEl *
GetDriveQElem( short driveNumber )
	{
	const DrvQEl *	drvQElem	= (const DrvQEl *)GetDrvQHdr()->qHead;
	
	while ( IsntNull( drvQElem ) )
		{
		if ( drvQElem->dQDrive == driveNumber )
			{
			break;
			}
			
		drvQElem	= (const DrvQEl *)drvQElem->qLink;
		}
	
	return( drvQElem );
	}















