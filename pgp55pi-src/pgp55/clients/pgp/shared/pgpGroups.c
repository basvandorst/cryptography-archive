/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Core groups-handling code.  Does not, and should not have any
	connection to PGPsdk constructs.  Code of that nature belongs
	in pgpGroupsUtil.c.

	$Id: pgpGroups.c,v 1.32 1997/09/18 02:27:38 heller Exp $
____________________________________________________________________________*/

#include <string.h>

#include "pgpPFLErrors.h"
#include "pflContext.h"
#include "pgpFileSpec.h"
#include "pgpMem.h"
#include "pgpFileUtilities.h"
#include "pgpEndianConversion.h"

#include "pgpGroups.h"
#include "pgpStrings.h"

#if PGP_MACINTOSH

#include "MacStrings.h"

/* because we include sort code twice */
#pragma once off
#endif

/*____________________________________________________________________________
	Notes:
	
	All data structures are in-memory representations.  On disk storage is
	completely different, including ommitting fields, changing the
	endian-ness, etc.
	
	Data structure explanation:
	
	A "group set" is a data structure which contains groups and all the stuff
	they reference.  A group belongs to one and only one set, and whenever
	an operation is performed, the set is required.  Groups are only
	referred to by a PGPGroupID and thus can freely be rearranged internally.
	
	A "group" contains other groups and/or a key ids, in no particular order.
	Each group has an ID, which is unique to the PGPGroupSet which contains
	it.  The key ids are exported key data from the PGPsdk and the data
	is never interpreted, only stored.
	
	These data structures are implemented using arrays. A few notes on this:
	
	1. the goal was rapid, bug-free implementation.  An assumption was made
	that creating large numbers of groups would occur via user-interface
	interaction and that the speed of adding them was not important. In fact,
	adding n groups is an O( n^2 ) operation.  This sounds bad, but in
	practice is irrelevant.  The same holds true for items within a group.
	So DO NOT change the simple implementation unless there is a proven
	reason that speed is a problem in the UI.
	
	2. Reading groups in and writing them out is O( n ).  It can't get any
	faster in the big O sense.
	
	3. PGPGroupIDs are used heavily.  This is perhaps one relevant place
	to optimize, but again, this shouldn't be done unless there is a
	proven issue.  Currently, a linear search is used; a binary search
	could be used, but the reality is that even with a few thousand
	groups (highly unlikely), the speed would probably still be
	reasonable.  The correct way to optimize would be to keep the groups
	sorted by group ID and them implement a binary search.
____________________________________________________________________________*/


typedef struct PGPGroup
{
	PGPGroupID			id;
	PGPGroupName		name;
	PGPGroupDescription	description;
	
	/* array of group IDs */
	PGPUInt32			numItems;
	PGPGroupItem *		itemsArray;
	
	/* these items are not stored to disk: */
	PGPGroupSetRef		set;
	PGPUserValue		userValue;
} PGPGroup;


#define kPGPGroupSetMagic	0x47524F55	/* 'GROU */
struct PGPGroupSet
{
	PGPUInt32		magic;
	PGPUInt32		version;
	PGPUInt32		nextAvailGroupID;
	
	char			pad[ 128 ];
	
	/* groupArray is sorted by ascending group ID */
	PGPUInt32		numGroups;
	PGPGroup *		groupArray;
	
	PFLContextRef	context;
	PGPUInt32		lastAccessIndex;
	PGPBoolean		hasBeenModified;
	
};
typedef struct PGPGroupSet	PGPGroupSet;


	
	PGPBoolean
PGPGroupSetIsValid( PGPGroupSetRef set )
{
	PGPBoolean	isValid;
	
	isValid	= IsntNull( set ) &&
				set->magic == kPGPGroupSetMagic;
	
	return( isValid );
}


static PGPError	sSortGroups( PGPGroupSetRef set );


typedef PGPUInt16	CRC16;

	static PGPUInt16
sCalcCRC16Continue(
	PGPUInt16		startValue,
	const void *	data,
	PGPInt32		len)
{
	const char *	dp	= (const char *)data;
	PGPUInt16		crc	= startValue;
	
	while(len--)
	{
		PGPInt16		i;
	
		crc ^= (PGPUInt16)(*dp++) << 8;
		for (i = 0; i < 8; ++i)
		{
			if (crc & 0x8000)
				crc = (crc << 1) ^ 0x1021;
			else
				crc <<= 1;
		}
	}
	return(crc);
}


#define sChecksumBytes( checksum, bytes, numBytes ) \
			sCalcCRC16Continue( checksum, bytes, numBytes )

	static CRC16
sChecksumUInt16(
	CRC16	checksum,
	PGPUInt16		number )
{
	PGPByte	storage[ sizeof( PGPUInt16 ) ];
	
	PGPUInt16ToStorage( number, storage );
	checksum	= sChecksumBytes( checksum, storage, sizeof( PGPUInt16 ) );
	return( checksum );
}

	static CRC16
sChecksumUInt32(
	CRC16	checksum,
	PGPUInt32		number )
{
	PGPByte	storage[ sizeof( PGPUInt32 ) ];
	
	PGPUInt32ToStorage( number, storage );
	checksum	= sChecksumBytes( checksum, storage, sizeof( PGPUInt32 ) );
	return( checksum );
}


	static CRC16
sComputeGroupChecksum( PGPGroup const * group)
{
	CRC16		checksum	= 0;
	
	/* these assumptions are made when writing out the data */
	pgpAssert( sizeof( group->id ) == sizeof( PGPUInt32 ) &&
		sizeof( group->numItems ) == sizeof( PGPUInt32 ) );
		
	checksum	= sChecksumUInt32( checksum, (PGPUInt32)group->id );
	
	checksum	= sChecksumBytes( checksum,
					group->name, sizeof( group->name ) );
					
	checksum	= sChecksumBytes( checksum,
					group->description, sizeof( group->description ));
					
	checksum	= sChecksumUInt32( checksum, group->numItems );
	
	return( checksum );
}

	static CRC16
sComputeGroupSetChecksum( PGPGroupSet const * set)
{
	CRC16		checksum	= 0;
	
	/* these assumptions are made when writing out the data */
	pgpAssert( sizeof( set->magic ) == sizeof( PGPUInt32 ) &&
		sizeof( set->version ) == sizeof( PGPUInt32 ) &&
		sizeof( set->nextAvailGroupID ) == sizeof( PGPUInt32 ) &&
		sizeof( set->numGroups ) == sizeof( PGPUInt32 ) );
		
	checksum	= sChecksumUInt32( checksum, set->magic );
	checksum	= sChecksumUInt32( checksum, set->version );
	checksum	= sChecksumUInt32( checksum, set->nextAvailGroupID );
	checksum	= sChecksumUInt32( checksum, set->numGroups );
	
	return( checksum );
}


	static CRC16
sComputeGroupItemChecksum( PGPGroupItem const * item)
{
	CRC16		checksum	= 0;
	
	/* these assumptions are made when writing out the data */
	pgpAssert( sizeof( item->type ) == sizeof( PGPUInt32 ) &&
		sizeof( item->u.group.id ) == sizeof( PGPUInt32 ) &&
		sizeof( item->u.key.algorithm ) == sizeof( PGPUInt32 ) &&
		sizeof( item->u.key.length ) == 1 );
		
	checksum	= sChecksumUInt32( checksum, item->type );
	if ( item->type == kPGPGroupItem_Group )
	{
		checksum	= sChecksumUInt32( checksum, item->u.group.id );
	}
	else if ( item->type == kPGPGroupItem_KeyID )
	{
		checksum	= sChecksumUInt32( checksum, item->u.key.algorithm );
		checksum	= sChecksumBytes( checksum, &item->u.key.length, 1 );
		checksum	= sChecksumBytes( checksum,
							item->u.key.exportedKeyID,
							item->u.key.length );
	}
	
	return( checksum );
}




	static PGPError
sReadUInt32FromIO(
	PGPIORef	io,
	PGPUInt32 *	number)
{
	PGPByte		endianBuf[ sizeof( PGPUInt32 ) ];
	PGPError	err;
	
	err	= PGPIORead( io, sizeof( endianBuf ), endianBuf, NULL );
	
	*number	= PGPStorageToUInt32( endianBuf );
	
	return( err );
}


	static PGPError
sReadUInt16FromIO(
	PGPIORef	io,
	PGPUInt16 *	number)
{
	PGPByte		endianBuf[ sizeof( PGPUInt16 ) ];
	PGPError	err;
	
	err	= PGPIORead( io, sizeof( endianBuf ), endianBuf, NULL );
	
	*number	= PGPStorageToUInt16( endianBuf );
	
	return( err );
}

	static PGPError
sWriteUInt32ToIO(
	PGPUInt32	number,
	PGPIORef	io )
{
	PGPByte		endianBuf[ sizeof( PGPUInt32 ) ];
	
	PGPUInt32ToStorage( number, endianBuf );
	return( PGPIOWrite( io, sizeof( endianBuf ), endianBuf ) );
}


	static PGPError
sWriteUInt16ToIO(
	PGPUInt16	number,
	PGPIORef	io )
{
	PGPByte		endianBuf[ sizeof( PGPUInt16 ) ];
	
	PGPUInt16ToStorage( number, endianBuf );
	return( PGPIOWrite( io, sizeof( endianBuf ), endianBuf ) );
}



#define sWriteChecksumToIO( checksum, io ) \
	sWriteUInt16ToIO( checksum, io )

	static PGPError
sVerifyChecksumFromIO(
	CRC16	checksum,
	PGPIORef		io )
{
	CRC16	ioChecksum;
	PGPError		err	= kPGPError_NoErr;
		
	err	= sReadUInt16FromIO( io, &ioChecksum );
	if ( IsntPGPError( err ) && checksum != ioChecksum )
	{
		err	= kPGPError_CorruptData;
	}
	return( err );
}


	static void
sInitGroup(
	PGPGroupSetRef			set,
	PGPGroupID				id,
	const char *			name,
	const char *			description,
	PGPGroup *				group)
{
	pgpClearMemory( group, sizeof( *group ) );
	group->id			= id;
	group->itemsArray	= NULL;
	group->numItems		= 0;
	group->set			= set;
	if ( IsntNull( name ) )
	{
		strcpy( group->name, name );
	}
	if ( IsntNull( description ) )
	{
		strcpy( group->description, description );
	}
}


	static void
sDestroyGroup(
	PGPGroup *				group)
{
	if ( IsntNull( group->itemsArray ) )
	{
		pgpAssert( IsntNull( group->set ) );
		PFLContextMemFree( group->set->context, group->itemsArray );
		group->itemsArray	= NULL;
	}
}


	static PGPError
sNewGroupSet(
	PFLContextRef			context,
	PGPGroupSetRef *		outSet )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroupSet *	set	= NULL;
	
	set	= (PGPGroupSet *)PFLContextMemAlloc( context,
			sizeof( *set ), kPFLMemoryFlags_Clear );
	if ( IsntNull( set ) )
	{
		set->magic				= kPGPGroupSetMagic;
		set->context			= context;
		set->nextAvailGroupID	= ((PGPUInt32)kPGPInvalidGroupID) + 1;
		set->groupArray			= NULL;
		set->numGroups			= 0;
		set->hasBeenModified	= TRUE;
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	*outSet	= set;
	return( err );
}


	static PGPBoolean
sSearchForID(
	PGPGroup const *	firstGroup,
	PGPGroup const *	lastGroup,
	PGPGroupID			id,
	PGPUInt32 *			outIndex )
{
	PGPGroup const *	curGroup;
	PGPBoolean			foundIt	= FALSE;
	
	/* we could use a sentinel value, but if we're that concerned about
	speed we should implement a binary search */
	curGroup	= firstGroup;
	while ( curGroup <= lastGroup )
	{
		if ( curGroup->id == id )
		{
			foundIt	= TRUE;
			*outIndex	= curGroup - firstGroup;
			break;
		}
		++curGroup;
	}
	
	return( foundIt );
}

/*____________________________________________________________________________
	Maybe to be implemented later as a binary search.  But that requires
	making sure all the items are sorted and is probably not worth it.
	
	The algorithm here works well for sequential, forward access.
____________________________________________________________________________*/
	static PGPError
sGetGroupIndexByID(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPUInt32 *		outIndex )
{
	PGPError	err	= kPGPError_ItemNotFound;
	PGPGroup const *	firstGroup;
	PGPGroup const *	lastGroup;
	PGPGroup const *	curGroup;
	PGPBoolean			foundIt	= FALSE;
	PGPUInt32			foundIndex	= 0;
	
	*outIndex	= 0;
	if ( set->numGroups == 0 )
		return( kPGPError_ItemNotFound );

	firstGroup	= &set->groupArray[ 0 ];
	lastGroup	= firstGroup + (set->numGroups - 1 );
	
	if ( set->lastAccessIndex < set->numGroups )
	{
		/* search from last access forward */
		curGroup	= &set->groupArray[ set->lastAccessIndex ];
		if ( sSearchForID( curGroup, lastGroup, id, &foundIndex ) )
		{
			foundIt		= TRUE;
			foundIndex	+= set->lastAccessIndex;
		}
		
		/* search items preceeding last access */
		if ( ( ! foundIt ) && set->lastAccessIndex != 0 )
		{
			foundIt	= sSearchForID( firstGroup,
						curGroup - 1, id, &foundIndex );
		}
	}
	else
	{
		/* search entire array from beginning */
		foundIt	= sSearchForID( firstGroup, lastGroup, id, &foundIndex );
	}
	
	if ( foundIt)
	{
		*outIndex	= foundIndex;
		set->lastAccessIndex	= foundIndex;
		err	= kPGPError_NoErr;
	}
	else
	{
		err	= kPGPError_ItemNotFound;
	}

	return( err );
}



/*____________________________________________________________________________
	to be implemented later as a binary search
____________________________________________________________________________*/
	static PGPError
sFindGroupByID(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPGroup **		outGroup )
{
	PGPError	err	= kPGPError_NoErr;
	PGPUInt32	index;
	
	err	= sGetGroupIndexByID( set, id, &index );
	if ( IsntPGPError( err ) )
	{
		*outGroup	= &set->groupArray[ index ];
	}

	return( err );
}




	PFLContextRef
PGPGetGroupSetContext( PGPGroupSetRef set )
{
	pgpAssert( PGPGroupSetIsValid( set ) );
	if ( ! PGPGroupSetIsValid( set ) )
		return( NULL );
	return( set->context );
}



/*____________________________________________________________________________
	create a new, empty groups collection
____________________________________________________________________________*/
	PGPError
PGPNewGroupSet(
	PFLContextRef		context,
	PGPGroupSetRef *	outRef )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PFLValidateContext( context );
	
	err	= sNewGroupSet( context, outRef );
	
	return( err );
}


	PGPError
PGPCountGroupsInSet(
	PGPGroupSetRef	set,
	PGPUInt32 *			numGroups)
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( numGroups );
	*numGroups	= 0;
	PGPValidateGroupSet( set );
	
	*numGroups	= set->numGroups;
	
	return( err );
}


	PGPError
PGPGetIndGroupID(
	PGPGroupSetRef	set,
	PGPUInt32			index,
	PGPGroupID *		id)
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( id );
	*id	= kPGPInvalidGroupID;
	PGPValidateGroupSet( set );
	PGPValidateParam( index < set->numGroups );
	
	*id	= set->groupArray[ index ].id;
	
	return( err );
}









	static PGPError
sReadGroupSetHeaderFromIO(
	PGPIORef		io,
	PGPGroupSet *	set )
{
	PGPError	err	= kPGPError_NoErr;
	
	err	= sReadUInt32FromIO( io, &set->magic);
	if ( IsntPGPError( err ) )
	{
		if ( set->magic != kPGPGroupSetMagic )
			err	= kPGPError_BadParams;
		else
			err	= sReadUInt32FromIO( io, &set->numGroups );
	}
	if ( IsntPGPError( err ) )
	{
		err	= sReadUInt32FromIO( io, &set->version );
	}
	if ( IsntPGPError( err ) )
	{
		err	= sReadUInt32FromIO( io, &set->nextAvailGroupID );
	}
	if ( IsntPGPError( err ) )
	{
		err	= PGPIORead( io, sizeof( set->pad ), set->pad, NULL );
	}
	
	if ( IsntPGPError( err ) )
	{
		err	= sVerifyChecksumFromIO( sComputeGroupSetChecksum( set ), io );
	}
	
	return( err );
}



	static PGPError
sReadGroupItemFromIO(
	PGPIORef			io,
	PGPGroupItem *		item )
{
	PGPError		err	= kPGPError_NoErr;
	PGPUInt32		temp;
	
	err	= sReadUInt32FromIO( io, &temp );
	if ( IsntPGPError( err ) )
	{
		item->type	= (PGPGroupItemType)temp;
		
		if ( item->type == kPGPGroupItem_Group )
		{
			err	= sReadUInt32FromIO( io, &temp );
			item->u.group.id	= (PGPGroupID)temp;
			pgpAssert( item->u.group.id != kPGPInvalidGroupID );
		}
		else if ( item->type == kPGPGroupItem_KeyID )
		{
			err	= sReadUInt32FromIO( io, &item->u.key.algorithm );
			if ( IsntPGPError( err ) )
				err	= PGPIORead( io, 1, &item->u.key.length, NULL);
			if ( IsntPGPError( err ) )
			{
				err	= PGPIORead( io,
					item->u.key.length,
					item->u.key.exportedKeyID, NULL);
			}
		}
	}
	
	if ( IsntPGPError( err ) )
	{
		err	= sVerifyChecksumFromIO( sComputeGroupItemChecksum( item ), io );
	}
	
	return( err );
}


	static PGPError
sReadGroupFromIO(
	PFLContextRef	context,
	PGPIORef		io,
	PGPGroup *		group )
{
	PGPError		err	= kPGPError_NoErr;
	PGPUInt32		index;
	PGPUInt32		temp;
	
	err	= sReadUInt32FromIO( io, &temp );
	if ( IsntPGPError( err ) )
	{
		group->id	= (PGPGroupID)temp;
		err	= PGPIORead( io, sizeof( group->name ), group->name, NULL);
	}
	if ( IsntPGPError( err ) )
	{
		err	= PGPIORead( io,
				sizeof( group->description ), group->description, NULL);
	}
	if ( IsntPGPError( err ) )
	{
		err	= sReadUInt32FromIO( io, &group->numItems);
	}
	
	if ( IsntPGPError( err ) )
	{
		err	= sVerifyChecksumFromIO( sComputeGroupChecksum( group ), io );
	}
	
	if ( IsntPGPError( err ) )
	{
		if ( group->numItems != 0 )
		{
			group->itemsArray	= (PGPGroupItem *)PFLContextMemAlloc( context,
					( group->numItems * sizeof(group->itemsArray[ 0 ]) ),
					kPFLMemoryFlags_Clear );
			if ( IsNull( group->itemsArray ) )
			{
				err	= kPGPError_OutOfMemory;
			}
		}
		
		if ( IsntPGPError( err ) )
		{
			/* now read all the items in */
			for ( index = 0; index < group->numItems; ++index )
			{
				err	= sReadGroupItemFromIO( io, &group->itemsArray[ index ] );
				if ( IsPGPError( err ) )
					break;
			}
		}
	}
	
	pgpAssertNoErr( err );
	return( err );
}



	static PGPError
sReadGroupSetFromIO(
	PGPIORef		io,
	PGPGroupSetRef	set )
{
	PGPError		err	= kPGPError_NoErr;
	
	err	= sReadGroupSetHeaderFromIO( io, set );
	if ( IsntPGPError( err )  )
	{
		PGPUInt32	groupIndex;
		
		/* allocate the groups */
		if ( set->numGroups != 0 )
		{
			set->groupArray	= (PGPGroup *)PFLContextMemAlloc( set->context,
					( set->numGroups * sizeof( set->groupArray[ 0 ] ) ),
					kPFLMemoryFlags_Clear );
			if ( IsNull( set->groupArray ) )
				err	= kPGPError_OutOfMemory;
		}
		
		if ( IsntPGPError( err ) )
		{
			for( groupIndex = 0; groupIndex < set->numGroups; ++groupIndex )
			{
				PGPGroup *	group;
				
				group	= &set->groupArray[ groupIndex ];
				sInitGroup( set, kPGPInvalidGroupID, NULL, NULL, group );
				err		= sReadGroupFromIO( set->context, io, group );
				if ( IsPGPError( err ) )
					break;
			}
		}
	}
	
	
	return( err );
}


	static PGPError
sReadGroupSetFromFile(
	PFLFileSpecRef	file,
	PGPGroupSet *	set )
{
	PGPError		err	= kPGPError_NoErr;
	PGPFileIORef	io	= NULL;
	PFLFileSpecRef	tempSpec	= NULL;
	PGPBoolean		fileExists	= TRUE;
	
	PGPValidateGroupSet( set );
	PFLValidateFileSpec( file );
	PGPValidateParam( set->hasBeenModified );
	
	/* open it, and write everything out */
	err	= PGPOpenFileSpec( file,
			kPFLFileOpenFlags_ReadOnly, &io );
	if ( IsntPGPError( err ) )
	{
		err	= sReadGroupSetFromIO( (PGPIORef)io, set);
		PGPFreeIO( (PGPIORef)io );
		
		/* we just read it in and there are no changes */
		set->hasBeenModified	= FALSE;
	}
	
	return( err );
}





	PGPError
PGPNewGroupSetFromFile(
	PFLFileSpecRef		file,
	PGPGroupSetRef *	outSet )
{
	PGPError		err	= kPGPError_NoErr;
	PFLContextRef	context	= NULL;
	PGPGroupSet *	set	= NULL;
	PGPBoolean		exists	= FALSE;
	
	PGPValidatePtr( outSet );
	*outSet	= NULL;
	PFLValidateFileSpec( file );
	
	err	= PFLFileSpecExists( file, &exists );
	if ( IsPGPError( err ) )
		return( err );
	if ( ! exists )
		return( kPGPError_FileNotFound );
	
	context	= PFLGetFileSpecContext( file );
	
	err	= sNewGroupSet( context, &set );
	if ( IsntPGPError( err ) )
	{
		err	= sReadGroupSetFromFile( file, set );
	}
	
	*outSet	= set;
	return( err );
}


#if PGP_MACINTOSH

	PGPError
PGPNewGroupSetFromFSSpec(
	PFLContextRef		context,
	const FSSpec *		spec,
	PGPGroupSetRef *	outSet )
{
	PGPError		err;
	PFLFileSpecRef	fileSpec;
	
	PGPValidatePtr( outSet );
	*outSet	= NULL;
	PGPValidatePtr( spec );
	
	err	= PFLNewFileSpecFromFSSpec( context, spec, &fileSpec );
	if ( IsntPGPError( err ) )
	{
		err	= PGPNewGroupSetFromFile( fileSpec, outSet );
		PFLFreeFileSpec( fileSpec );
	}
	
	return( err );
}

#endif




	static PGPError
sWriteGroupSetHeaderToIO(
	PGPGroupSet const *	set,
	PGPIORef			io )
{
	PGPError		err	= kPGPError_NoErr;
	
	err	= sWriteUInt32ToIO( set->magic, io);
	if ( IsntPGPError( err ) )
	{
		err	= sWriteUInt32ToIO( set->numGroups, io );
	}
	if ( IsntPGPError( err ) )
	{
		err	= sWriteUInt32ToIO( set->version, io );
	}
	if ( IsntPGPError( err ) )
	{
		err	= sWriteUInt32ToIO( set->nextAvailGroupID, io );
	}
	if ( IsntPGPError( err ) )
	{
		err	= PGPIOWrite( io, sizeof( set->pad ), set->pad );
	}
	
	if ( IsntPGPError( err ) )
	{
		err	= sWriteChecksumToIO( sComputeGroupSetChecksum( set ), io );
	}
	
	return( err );
}



	static PGPError
sWriteGroupItemToIO(
	PGPGroupItem const *	item,
	PGPIORef				io )
{
	PGPError		err	= kPGPError_NoErr;
	
	err	= sWriteUInt32ToIO( item->type, io);
	if ( IsntPGPError( err ) )
	{
		if ( item->type == kPGPGroupItem_Group )
		{
			err	= sWriteUInt32ToIO( (PGPUInt32)item->u.group.id, io);
		}
		else if ( item->type == kPGPGroupItem_KeyID )
		{
			err	= sWriteUInt32ToIO( item->u.key.algorithm, io );
			if ( IsntPGPError( err ) )
				err	= PGPIOWrite( io, 1, &item->u.key.length);
			if ( IsntPGPError( err ) )
			{
				err	= PGPIOWrite( io, item->u.key.length,
							item->u.key.exportedKeyID );
			}
		}
	}
	
	if ( IsntPGPError( err ) )
	{
		err	= sWriteChecksumToIO( sComputeGroupItemChecksum( item ), io );
	}
	
	return( err );
}


	static PGPError
sWriteGroupToIO(
	PGPGroup const *	group,
	PGPIORef			io )
{
	PGPError		err	= kPGPError_NoErr;
	
	err	= sWriteUInt32ToIO( (PGPUInt32)group->id, io );
	if ( IsntPGPError( err ) )
	{
		err	= PGPIOWrite( io, sizeof( group->name ), group->name );
	}
	if ( IsntPGPError( err ) )
	{
		err	= PGPIOWrite( io,
			sizeof( group->description ), group->description );
	}
	if ( IsntPGPError( err ) )
	{
		err	= sWriteUInt32ToIO( group->numItems, io);
	}
	
	if ( IsntPGPError( err ) )
	{
		err	= sWriteChecksumToIO( sComputeGroupChecksum( group ), io );
	}
	
	if ( IsntPGPError( err ) )
	{
		PGPUInt32		index;
	
		/* now write all the items out */
		for ( index = 0; index < group->numItems; ++index )
		{
			err	= sWriteGroupItemToIO( &group->itemsArray[ index ], io );
			if ( IsPGPError( err ) )
				break;
		}
	}
	
	return( err );
}



	static PGPError
sWriteGroupSetToIO(
	PGPGroupSetRef	set,
	PGPIORef		io )
{
	PGPError		err	= kPGPError_NoErr;
	
	err	= sWriteGroupSetHeaderToIO( set, io );
	if ( IsntPGPError( err ) && IsntNull( set->groupArray ) )
	{
		PGPUInt32	groupIndex;
		
		for( groupIndex = 0; groupIndex < set->numGroups; ++groupIndex )
		{
			PGPGroup *	group;
			
			group	= &set->groupArray[ groupIndex ];
			err	= sWriteGroupToIO( group, io );
			if ( IsPGPError( err ) )
				break;
		}
	}
	
	return( err );
}


	PGPError
PGPSaveGroupSetToFile(
	PGPGroupSetRef	set,
	PFLFileSpecRef	file )
{
	PGPError		err	= kPGPError_NoErr;
	PGPFileIORef	io	= NULL;
	PFLFileSpecRef	tempSpec	= NULL;
	PGPBoolean		fileExists	= TRUE;
	
	PGPValidateGroupSet( set );
	PFLValidateFileSpec( file );
	
	err	= PFLFileSpecExists( file, &fileExists );
	
	/* get a temp file to work with */
	if ( IsntPGPError( err ) )
		err	= PFLFileSpecGetUniqueSpec( file, &tempSpec );
		
	if ( IsntPGPError( err ) )
	{
		/* create the temp file */
		err	= PFLFileSpecCreate( tempSpec );
		if ( IsntPGPError( err ) )
		{
			/* open it, and write everything out */
			err	= PGPOpenFileSpec( tempSpec,
					kPFLFileOpenFlags_ReadWrite, &io );
			if ( IsntPGPError( err ) )
			{
				err	= sWriteGroupSetToIO( set, (PGPIORef)io);
				PGPFreeIO( (PGPIORef)io );
			}
		}
		
		/* delete the original and rename the temp */
		if ( IsntPGPError( err ) && fileExists )
		{
			char	name[ 512 ];
			
			/* get original file name */
			err	= PFLGetFileSpecNameBuffer( file,
					sizeof( name ), name );
						
			if ( IsntPGPError( err ) )
			{
				/* delete the original */
				err	= PFLFileSpecDelete( file );
			}
				
			if ( IsntPGPError( err ) )
			{
				/* rename temp to original */
				err	= PFLFileSpecRename( tempSpec, name );
			}

		}
		
		if ( IsntPGPError( err ) )
		{
			/* safely up to date on disk... */
			set->hasBeenModified	= FALSE;
		}
		
		PFLFreeFileSpec( tempSpec );
	}
	
	return( err );
}


	static void
sDeleteGroupsArray( PGPGroupSetRef set )
{
	PFLContextRef	context	= set->context;
	
	if ( IsntNull( set->groupArray ) )
	{
		PGPUInt32		groupIndex;
	
		for( groupIndex = 0; groupIndex < set->numGroups; ++groupIndex )
		{
			PGPGroup *	group;
			
			group	= &set->groupArray[ groupIndex ];
			
			sDestroyGroup( group );
		}
		
		PFLContextMemFree( context, set->groupArray );
		set->groupArray	= NULL;
	}
}

	PGPError
PGPFreeGroupSet( PGPGroupSetRef set )
{
	PGPError		err	= kPGPError_NoErr;
	PFLContextRef	context	= set->context;
	
	PGPValidateGroupSet( set );
	
	sDeleteGroupsArray( set );
	PFLContextMemFree( context, set );
	
	return( err );
}


	PGPBoolean
PGPGroupSetNeedsCommit( PGPGroupSetRef set )
{
	pgpAssert( PGPGroupSetIsValid( set ) );
	if ( IsNull( set ) )
		return( FALSE );
		
	return( set->hasBeenModified );
}



	static PGPGroupID
sGetNextAvailGroupID(PGPGroupSetRef	set)
{
	PGPGroupID	id;
	
	set->nextAvailGroupID	+= 1;
	id	= (PGPGroupID)set->nextAvailGroupID;
	return( id );
}


	static int
sCompareGroupsProc(
	PGPGroup *	in1,
	PGPGroup *	in2,
	void *		userValue )
{
	const PGPGroup *	group1	= (const PGPGroup *)in1;
	const PGPGroup *	group2	= (const PGPGroup *)in2;
	int					result;
	
	(void)userValue;
	result	= pgpCompareStringsIgnoreCase( group1->name, group2->name );
	
	return( result );
}


#define InsertionSortName	sInsertionSortGroups
#define InsertionSortItem	PGPGroup
#include "pgpInsertionSort.h"
#undef InsertionSortName
#undef InsertionSortItem

	static PGPError
sSortGroups( PGPGroupSetRef set )
{	
	sInsertionSortGroups( set->groupArray,
		set->numGroups, sCompareGroupsProc, 0 );
		
	return( kPGPError_NoErr );
}



#define InsertionSortName	sInsertionSortGroupItems
#define InsertionSortItem	PGPGroupItem
#include "pgpInsertionSort.h"
#undef InsertionSortName
#undef InsertionSortItem



	PGPError
PGPSortGroupItems(
	PGPGroupSetRef			set,
	PGPGroupID				id,
	PGPGroupItemCompareProc	compareProc,
	PGPUserValue			userValue )
{
	PGPError	err	= kPGPError_NoErr;
	PGPGroup *	group;
	
	err	= sFindGroupByID( set, id, &group );
	if ( IsntPGPError( err ) )
	{
		sInsertionSortGroupItems( group->itemsArray,
			group->numItems, compareProc, userValue );
	}
		
	return( err );
}


	PGPError
PGPSortGroupSet(
	PGPGroupSetRef			set,
	PGPGroupItemCompareProc	compareProc,
	PGPUserValue			userValue )
{
	PGPUInt32	index;
	PGPError	err	= kPGPError_NoErr;
	
	sSortGroups( set );
	
	for( index = 0; index < set->numGroups; ++index )
	{
		err	= PGPSortGroupItems( set,
			set->groupArray[ index ].id, compareProc, userValue );
		if ( IsPGPError( err ) )
			break;
	}
	return( err );
}

/*____________________________________________________________________________
	Add a group to the set.  Caller has initialized the group.
____________________________________________________________________________*/
	static PGPError
sAddGroup(
	PGPGroupSetRef		set,
	PGPGroup const *	group )
{
	PGPError		err	= kPGPError_NoErr;
	PFLContextRef	context	= set->context;
	PGPSize			newSize;
	
	newSize	= ( set->numGroups + 1 ) * sizeof( *group );
	err	= PFLContextMemRealloc( context, &set->groupArray,
			newSize, 0);

	if ( IsntPGPError( err ) )
	{
		(set->groupArray)[ set->numGroups ]	= *group;
		set->numGroups++;
	}
	
	return( err );
}


	PGPError
PGPNewGroup(
	PGPGroupSetRef			set,
	const char *			name,
	const char *			description,
	PGPGroupID *			idOut )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroup		newGroup;
	PGPGroupID		newID;
	PGPGroup *		dummyGroup;
	
	PGPValidatePtr( idOut );
	*idOut	= kPGPInvalidGroupID;
	PGPValidatePtr( name );
	PGPValidatePtr( description );
	PGPValidateParam( strlen( name ) <= kPGPMaxGroupNameLength );
	PGPValidateParam( strlen( description ) <= kPGPMaxGroupDescriptionLength );
	PGPValidateGroupSet( set );
	
	do
	{
		newID	= sGetNextAvailGroupID( set );
	}
	while ( IsntPGPError( err = sFindGroupByID( set, newID, &dummyGroup ) ));
	pgpAssert( newID != kPGPInvalidGroupID );
	
	sInitGroup( set, newID, name, description, &newGroup );
	err	= sAddGroup( set, &newGroup );
	if ( IsntPGPError( err ) )
	{
		*idOut	= newID;
		set->hasBeenModified	= TRUE;
	}
	
	return( err );
}



/*____________________________________________________________________________
	Eliminate any reference to the group with ID 'id' from 'group'.
____________________________________________________________________________*/
	static PGPError
sEliminateGroupFromGroup(
	PGPGroup *		group,
	PGPGroupID		id )
{
	PGPUInt32	index;
	PGPError	err	= kPGPError_NoErr;
	
	pgpAssert( IsntNull( group->set ) );
	for (index = 0; index < group->numItems; ++index )
	{
		PGPGroupItem const *	item;
		
		item	= &group->itemsArray[ index ];
		if (item->type == kPGPGroupItem_Group &&
			item->u.group.id == id )
		{
			err	= PGPDeleteIndItemFromGroup( group->set, group->id, index );
			/* it can only be in there once so we're done */
			break;
		}
	}
	return( err );
}


/*____________________________________________________________________________
	Eliminate references to this group from everything.
____________________________________________________________________________*/
	static PGPError
sEliminateReferencesToGroup(
	PGPGroupSetRef	set,
	PGPGroupID		id )
{
	PGPUInt32	index;
	PGPError	err	= kPGPError_NoErr;
	
	for (index = 0; index < set->numGroups; ++index )
	{
		PGPGroup *	group;
	
		group	= &set->groupArray[ index ];
		err	= sEliminateGroupFromGroup( group, id );
		if ( IsPGPError( err ) )
			break;
	}
	return( err );
}



/*____________________________________________________________________________
	Determine if an item is already a member of the group
____________________________________________________________________________*/
	static PGPBoolean
sIsGroupMember(
	PGPGroup const *		group,
	PGPGroupItem const *	item,
	PGPUInt32 *				outIndex )
{
	PGPBoolean		isMember	= FALSE;
	PGPUInt32		index;
	
	*outIndex	= ~(PGPUInt32)0;
	for( index = 0; index < group->numItems; ++index )
	{
		PGPGroupItem const *	indItem;
		
		indItem	= &group->itemsArray[ index ];
		if ( indItem->type == item->type )
		{
			if ( indItem->type == kPGPGroupItem_Group )
			{
				if ( indItem->u.group.id == item->u.group.id )
				{
					*outIndex	= index;
					isMember	= TRUE;
					break;
				}
			}
			else if ( indItem->type == kPGPGroupItem_KeyID )
			{
				if ( indItem->u.key.length == item->u.key.length &&
					pgpMemoryEqual( indItem->u.key.exportedKeyID,
						item->u.key.exportedKeyID, item->u.key.length )
					)
				{
					*outIndex	= index;
					isMember	= TRUE;
					break;
				}
			}
		}
	}
	
	return( isMember );
}


/*____________________________________________________________________________
	Delete a group.  Since other groups may refer to it, search for and
	eliminate all references to it.
____________________________________________________________________________*/
	PGPError
PGPDeleteGroup(
	PGPGroupSetRef		set,
	PGPGroupID			id )
{
	PGPError		err	= kPGPError_NoErr;
	PGPUInt32		index;
	
	PGPValidateGroupSet( set );
	
	/* find the group in our array */
	err	= sGetGroupIndexByID( set, id, &index );
	if ( IsntPGPError( err ) )
	{
		PGPGroup *	group;
		PGPUInt32	bytesAfter;
		
		set->hasBeenModified	= TRUE;
		
		/* destroy the group, the remove it from the array */
		group		= &set->groupArray[ index ];
		
		sDestroyGroup( group );
		/* shift bytes down */
		bytesAfter	= ( set->numGroups - index - 1 ) * sizeof( *group );
		if ( bytesAfter != 0 )
		{
			pgpCopyMemory( group + 1, group, bytesAfter);
		}
		set->numGroups	-= 1;
		/* we don't shrink the array */
	}
	
	/* now eliminate this group from all other groups (yuck) */
	sEliminateReferencesToGroup( set, id );
	
	return( err );
}


/*____________________________________________________________________________
	Delete an item from a group.  The item may be of any valid kind.
	
	Afterwards, there will be one less item in the group.
____________________________________________________________________________*/
	PGPError
PGPDeleteIndItemFromGroup(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPUInt32		index )
{
	PGPGroup *	group	= NULL;
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidateGroupSet( set );
	
	err	= sFindGroupByID( set, id, &group );
	if ( IsntPGPError( err ) )
	{
		PGPSize			bytesAfter;
		PGPGroupItem *	item;
		
		PGPValidateParam( index < group->numItems );
	
		set->hasBeenModified	= TRUE;
		
		item	= &group->itemsArray[ index ];
		bytesAfter	= ( group->numItems - index - 1 ) * sizeof( *item );
		if ( bytesAfter != 0 )
		{
			pgpCopyMemory( item + 1, item, bytesAfter );
		}
		
		group->numItems	-= 1;
		
		/* don't bother shrinking, but if nothing left, get rid of it */
		if ( group->numItems == 0 )
		{
			PFLContextMemFree( group->set->context, group->itemsArray );
			group->itemsArray	= NULL;
		}
		
	}
	return( err );
}



	PGPError
PGPDeleteItemFromGroup(
	PGPGroupSetRef			set,
	PGPGroupID				id,
	PGPGroupItem const *	item )
{
	PGPGroup *	group	= NULL;
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidateGroupSet( set );
	PGPValidatePtr( item );

	err	= sFindGroupByID( set, id, &group );
	if ( IsntPGPError( err ) )
	{
		PGPUInt32	index;
		
		err	= sIsGroupMember( group, item, &index );
		if ( IsntPGPError( err ) )
		{
			err	= PGPDeleteIndItemFromGroup( set, id, index );
		}
	}
	return( err );
}


	PGPError
PGPGetGroupInfo(
	PGPGroupSetRef		set,
	PGPGroupID			id,
	PGPGroupInfo *		info )
{
	PGPError			err		= kPGPError_NoErr;
	PGPGroup const *	group	= NULL;
	
	PGPValidatePtr( info );
	pgpClearMemory( info, sizeof( *info ) );
	PGPValidateGroupSet( set );
	
	err	= sFindGroupByID( set, id, (PGPGroup **)&group );
	if ( IsntPGPError( err ) )
	{
		info->id		= id;
		strcpy( info->name, group->name );
		strcpy( info->description, group->description );
		info->userValue	= group->userValue;
	}
	
	return( err );
}


	PGPError
PGPSetGroupName(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	const char *	name )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroup *		group	= NULL;
	
	PGPValidatePtr( name );
	PGPValidateGroupSet( set );
	PGPValidateParam( strlen( name ) <= kPGPMaxGroupNameLength );
	
	err	= sFindGroupByID( set, id, &group );
	if ( IsntPGPError( err ) )
	{
		strcpy( group->name, name );
		set->hasBeenModified	= TRUE;
	}
	
	return( err );
}


	PGPError
PGPSetGroupDescription(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	const char *	description )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroup *		group	= NULL;
	
	PGPValidatePtr( description );
	PGPValidateParam( strlen( description ) <= kPGPMaxGroupDescriptionLength );
	PGPValidateGroupSet( set );
	
	err	= sFindGroupByID( set, id, &group );
	if ( IsntPGPError( err ) )
	{
		strcpy( group->description, description );
		set->hasBeenModified	= TRUE;
	}
	
	return( err );
}


	PGPError
PGPSetGroupUserValue(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPUserValue	userValue )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroup *		group	= NULL;
	
	PGPValidateGroupSet( set );
	
	err	= sFindGroupByID( set, id, &group );
	if ( IsntPGPError( err ) )
	{
		group->userValue	= userValue;
	}
	
	return( err );
}





/*____________________________________________________________________________
	Return TRUE if group1 contains group 2, either directly or in a subgroup.
____________________________________________________________________________*/
	static PGPBoolean
sGroupContainsGroup(
	PGPGroupSetRef	set,
	PGPGroupID		mainGroupID,
	PGPGroupID		subGroup )
{
	PGPError			err	= kPGPError_NoErr;
	PGPGroup  *			mainGroup	= NULL;
	PGPBoolean			containsGroup	= FALSE;
	PGPUInt32			index;
	
	err	= sFindGroupByID( set, mainGroupID, &mainGroup );
	pgpAssertNoErr( err );
	if ( IsPGPError( err ) )
		return( FALSE );
	if ( mainGroupID == subGroup )
	{
		/* can't add a group to itself */
		return( TRUE );
	}
	
	for( index = 0; index < mainGroup->numItems; ++index )
	{
		PGPGroupItem const *	item;
		
		item	= &mainGroup->itemsArray[ index ];
		if ( item->type == kPGPGroupItem_Group )
		{
			if ( item->u.group.id == subGroup )
			{
				containsGroup	= TRUE;
				break;
			}
			else if ( sGroupContainsGroup( set, item->u.group.id, subGroup ) )
			{
				containsGroup	= TRUE;
				break;
			}
		}
	}
	
	return( containsGroup );
}


/*____________________________________________________________________________
	Add an item to the group.  Caller must completely fill in the item
	before adding.
____________________________________________________________________________*/
	PGPError
PGPAddItemToGroup(
	PGPGroupSetRef			set,
	PGPGroupItem const *	item,
	PGPGroupID				id )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroup *		group	= NULL;
	PGPUInt32		index;
	
	PGPValidateGroupSet( set );
	PGPValidatePtr( item );
	PGPValidateParam( item->type == kPGPGroupItem_KeyID ||
		item->type == kPGPGroupItem_Group );

	/* verify that the group being added to exists */
	err	= sFindGroupByID( set, id, &group );
	if ( IsPGPError( err ) )
		return( err );
	
	if ( item->type == kPGPGroupItem_Group )
	{
		/* verify that if the group being added that exists */
		PGPGroup *	itemGroup;
		err	= sFindGroupByID( set, item->u.group.id, &itemGroup );
		if ( IsPGPError( err ) )
			return( err );
	}
	else
	{
		/* verify that the key data is reasonable */
		PGPSize	itemLength	= item->u.key.length;
		PGPValidateParam( itemLength >= 4 &&
			itemLength <= sizeof( item->u.key.exportedKeyID ) );
	}
	
	/* verify that the group doesn't already contain this item */
	if ( sIsGroupMember( group, item, &index )  )
		return( kPGPError_ItemAlreadyExists );
	
	/* verify that a cycle won't be created by adding */
	if ( item->type == kPGPGroupItem_Group &&
		sGroupContainsGroup( set, item->u.group.id, id ) )
	{
		return( kPGPError_BadParams );
	}
	
	/* OK, all verification is now done */
	
	if ( IsntPGPError( err ) )
	{
		PFLContextRef	context	= set->context;
		PGPSize			newSize;
		
		newSize	= ( group->numItems + 1 ) * sizeof( PGPGroupItem );
		/* add the new item */
		err	= PFLContextMemRealloc( context, &group->itemsArray,
				newSize, 0);
		
		if ( IsntPGPError( err ) )
		{
			(group->itemsArray)[ group->numItems ]	= *item;
			group->numItems++;
			set->hasBeenModified	= TRUE;
		}
	}
	
	return( err );
}
				

/*____________________________________________________________________________
	This can't use an iterator, because it is used to create an iterator.
____________________________________________________________________________*/
	PGPError
PGPCountGroupItems(
	PGPGroupSetRef		set,
	PGPGroupID 			id,
	PGPBoolean			recursive,
	PGPUInt32 *			outNumKeys,
	PGPUInt32 *			outTotal )
{
	PGPError	err	= kPGPError_NoErr;
	PGPGroup *	group	= NULL;
	PGPUInt32	numItems	= 0;
	PGPUInt32	numKeys		= 0;
	
	if ( IsntNull( outNumKeys ) )
		*outNumKeys	= 0;
	if ( IsntNull( outTotal ) )
		*outTotal	= 0;
	PGPValidateGroupSet( set );
	
	err	= sFindGroupByID( set, id, &group);
	if ( IsntPGPError( err ) )
	{
		PGPUInt32	index;
			
		/* account for all items in this group right here */
		numItems	+= group->numItems;
		
		for (index = 0; index < group->numItems; ++index )
		{
			PGPGroupItem const *item;
			
			item	= &group->itemsArray[ index ];
			if ( item->type == kPGPGroupItem_KeyID )
			{
				++numKeys;
			}
			else if ( recursive && item->type == kPGPGroupItem_Group )
			{
				PGPUInt32	tempTotal;
				PGPUInt32	tempKeys;
				
				err	= PGPCountGroupItems( set, item->u.group.id, TRUE,
					&tempKeys, &tempTotal );
				if ( IsPGPError( err ) )
					break;
				numKeys		+= tempKeys;
				numItems	+= tempTotal;
			}
		}
	}
	
	if ( IsntPGPError( err ) )
	{
		if ( IsntNull( outTotal ) )
			*outTotal	= numItems;
			
		if ( IsntNull( outNumKeys ) )
			*outNumKeys	= numKeys;
	}
	
	return( err );
}




	PGPError
PGPGetIndGroupItem(
	PGPGroupSetRef	set,
	PGPGroupID			id,
	PGPUInt32			index,
	PGPGroupItem *		item )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroup *		group	= NULL;
	
	PGPValidatePtr( item );
	pgpClearMemory( item, sizeof( *item ) );
	PGPValidateGroupSet( set );
	
	err	= sFindGroupByID( set, id, &group);
	if ( IsntPGPError( err ) )
	{
		if ( index < group->numItems )
		{
			*item	= group->itemsArray[ index ];
		}
		else
		{
			err	= kPGPError_BadParams;
		}
	}
	
	return( err );
}


	PGPError
PGPSetIndGroupItemUserValue(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPUInt32		index,
	PGPUserValue	userValue )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroup *		group	= NULL;
	
	PGPValidateGroupSet( set );
	
	err	= sFindGroupByID( set, id, &group);
	if ( IsntPGPError( err ) )
	{
		if ( index < group->numItems )
		{
			group->itemsArray[ index ].userValue	= userValue;
		}
		else
		{
			err	= kPGPError_BadParams;
		}
	}
	
	return( err );
}

/*____________________________________________________________________________
	An iterator is implemented by creating an array of pointers to the
	elements to be iterated.  This uses a modest amount of memory relative
	to the overall structure, and results in very fast iteration.
	
	Its drawbacks include the inability to change the overall structure
	while iterating (at least in this implementation).
____________________________________________________________________________*/

typedef struct PGPGroupIter	PGPGroupIter;
struct PGPGroupIter
{
	PGPGroupSetRef			set;
	const PGPGroupItem  **	items;
	#define kBeforeFirstItemIndex	0xFFFFFFFF
	PGPUInt32				curIndex;
	PGPUInt32				numItems;
};


/*____________________________________________________________________________
	Fill the iter->items array with pointers to all items in the group.
____________________________________________________________________________*/
	static PGPUInt32
sCollectPointersForIter(
	PGPGroupIter *	iter,
	PGPGroupID		id,
	PGPBoolean		recursive,
	PGPFlags		flags,
	PGPUInt32		startSlot )
{
	PGPUInt32			nextSlot	= startSlot;
	PGPUInt32			index;
	PGPGroup const *	group	= NULL;
	PGPError			err	= kPGPError_NoErr;
	
	pgpAssert( IsntNull( iter ) );
	
	if ( startSlot >= iter->numItems )
	{
		/* if we've found all the items we could be pointing after 
		the end of the array, but we shouldn't find any more items
		of the desired kind, so no point in continuing */
		return( startSlot );
	}
	
	err	= sFindGroupByID( iter->set, id, (PGPGroup **)&group );
	pgpAssertNoErr( err );
	
	for( index = 0; index < group->numItems; ++index )
	{
		const PGPGroupItem *	item;
		
		item	= &group->itemsArray[ index ];
		if ( item->type == kPGPGroupItem_KeyID )
		{
			if ( ( flags & kPGPGroupIterFlags_Keys) != 0 )
			{
				iter->items[ nextSlot ]	= item;
				++nextSlot;
			}
		}
		else
		{
			pgpAssert( item->type == kPGPGroupItem_Group );
			if ( ( flags & kPGPGroupIterFlags_Groups) != 0 )
			{
				iter->items[ nextSlot ]	= item;
				++nextSlot;
			}
		
			if ( recursive )
			{
				nextSlot	= sCollectPointersForIter( iter,
								item->u.group.id, TRUE, flags, nextSlot );
			}
		}
	}
	
	return( nextSlot );
}



	static PGPError
sInitIter(
	PGPGroupSetRef		set,
	PGPGroupID			id,
	PGPFlags			flags,
	PGPGroupIter *		iter )
{
	PGPError	err	= kPGPError_NoErr;
	PGPUInt32	numKeys;
	PGPUInt32	numItems;
	Boolean		recursive;
	PGPGroupID	actualID	= id;
	
	recursive	= (flags & kPGPGroupIterFlags_Recursive) != 0;
	
	pgpClearMemory( iter, sizeof( *iter ) );
	
	if ( IsntPGPError( err ) )
	{
		iter->set		= set;
		iter->items		= NULL;
		iter->curIndex	= kBeforeFirstItemIndex;
		err	= PGPCountGroupItems( set, actualID,
				recursive, &numKeys, &numItems );
		if ( IsntPGPError( err ) )
		{
			iter->numItems	= 0;
			if ( ( flags & kPGPGroupIterFlags_Keys) != 0 )
				iter->numItems	+= numKeys;
			if ( ( flags & kPGPGroupIterFlags_Groups) != 0 )
				iter->numItems	+= (numItems - numKeys);
			
			/* create an array of pointers to the items */
			iter->items	= PFLContextMemAlloc( set->context,
					iter->numItems * sizeof( iter->items[ 0 ] ), 0);
			if ( IsNull( iter->items ) )
				err	= kPGPError_OutOfMemory;
		}
		
		if ( IsntPGPError( err ) )
		{
			PGPUInt32	nextSlot;
			
			/* collect all the pointers we need */
			nextSlot	= sCollectPointersForIter( iter,
							actualID, recursive, flags, 0);
			pgpAssert( nextSlot == iter->numItems ||
					nextSlot == iter->numItems + 1);
		}
	}
	
	return( err );
}


	PGPError
PGPNewGroupItemIter(
	PGPGroupSetRef			set,
	PGPGroupID				id,
	PGPFlags				flags,
	PGPGroupItemIterRef *	iterOut )
{
	PGPError			err	= kPGPError_NoErr;
	PGPGroupIter *		iter	= NULL;
	
	PGPValidatePtr( iterOut );
	*iterOut	= NULL;
	PGPValidateGroupSet( set );
	PGPValidateParam( (flags & kPGPGroupIterFlags_AllItems) != 0 );
	PGPValidateParam( (flags & ~ kPGPGroupIterFlags_AllRecursive) == 0 );
	
	iter	= PFLContextMemAlloc( set->context,
			sizeof( *iter ), kPFLMemoryFlags_Clear );
	if ( IsNull( iter ) )
		err	= kPGPError_OutOfMemory;
	if ( IsntPGPError( err ) )
	{
		err	= sInitIter( set, id, flags, iter );
		if ( IsPGPError( err ) )
		{
			PFLContextMemFree( set->context, iter );
			iter	= NULL;
		}
	}
	
	*iterOut	= iter;
	return( err );
}


	PGPError
PGPFreeGroupItemIter( PGPGroupItemIterRef iter )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidateParam( IsntNull( iter ) );
	if ( IsntNull( iter->items ) )
	{
		err	= PFLContextMemFree( iter->set->context,
				(void *)iter->items );
		iter->items	= NULL;
	}
	
	if ( IsntPGPError( err ) )
	{
		err	= PFLContextMemFree( iter->set->context, iter );
	}
	
	return( err );
}




/*____________________________________________________________________________
	Returns kPGPError_EndOfIteration when done
____________________________________________________________________________*/
	PGPError
PGPGroupItemIterNext(
	PGPGroupItemIterRef	iter,
	PGPGroupItem *		item )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( item );
	PGPValidateParam( IsntNull( iter ) );
	PGPValidateGroupSet( iter->set );
	
	pgpDebugWhackMemory( item, sizeof(*item) );
	
	if ( iter->curIndex == iter->numItems )
	{
		/* already at end of list */
		err	= kPGPError_EndOfIteration;
	}
	else
	{
		if ( iter->curIndex == kBeforeFirstItemIndex )
			iter->curIndex	= 0;
		else
			iter->curIndex	+= 1;
			
		if ( iter->curIndex == iter->numItems )
			err	= kPGPError_EndOfIteration;
		else
			*item	= *(iter->items[ iter->curIndex ]);
	}
	
	
	return( err );
}





	static PGPError
sFindGroupByName(
	PGPGroupSetRef	set,
	const char *	name,
	PGPGroupID *	outID )
{
	PGPUInt32	index;
	PGPError	err	= kPGPError_ItemNotFound;
	
	*outID	= kPGPInvalidGroupID;
	
	for( index = 0; index < set->numGroups; ++index )
	{
		PGPGroup const *	group;
		
		group	= &set->groupArray[ index ];
		if ( pgpCompareStringsIgnoreCase( group->name, name ) == 0 )
		{
			*outID	= group->id;
			err	= kPGPError_NoErr;
			break;
		}
	}
	
	return( err );
}



/*____________________________________________________________________________
	Recursively add a group into the destination set, merging all its
	entries that don't exist in the destination.
____________________________________________________________________________*/
	static PGPError
sAddGroupIntoDifferentSet(
	PGPGroup const *	srcGroup,
	PGPGroupSetRef		destSet,
	PGPGroupID *		resultID )
{
	PGPError	err		= kPGPError_NoErr;
	PGPGroupID	destID	= kPGPInvalidGroupID;
	
	/* first make sure the group exists in destination set */
	if ( sFindGroupByName( destSet,
			srcGroup->name, &destID ) == kPGPError_ItemNotFound )
	{
		err	= PGPNewGroup( destSet,
				srcGroup->name, srcGroup->description, &destID);
	}
	if ( IsntPGPError( err ) )
	{
		PGPUInt32	itemIndex;
	
		/* loop over all items in srcGroup */
		for( itemIndex = 0; itemIndex < srcGroup->numItems; ++itemIndex )
		{
			PGPGroupItem const *	item;
			
			item	= &srcGroup->itemsArray[ itemIndex ];
			if ( item->type == kPGPGroupItem_Group )
			{
				/* recursively add subgroup */
				PGPGroup *		subGroup;
				PGPGroupID		newID;
				PGPGroupItem	newItem;
				
				err	= sFindGroupByID( srcGroup->set,
							item->u.group.id, &subGroup );
				if ( IsPGPError( err ) )
					break;
					
				err	= sAddGroupIntoDifferentSet( subGroup, destSet, &newID );
				if ( IsPGPError( err ) )
					break;
					
				/* now add the newly created group to this group */
				newItem.type		= kPGPGroupItem_Group;
				newItem.userValue	= 0;
				newItem.u.group.id	= newID;
				item				= &newItem;
			}
			
			/* add the key or group to dest set if it doesn't already exist */
			err	= PGPAddItemToGroup( destSet, item, destID );
			if ( err == kPGPError_ItemAlreadyExists )
				err	= kPGPError_NoErr;
				
			if ( IsPGPError( err ) )
				break;
		}
	}
	
	*resultID	= destID;
	return( err );
}


/*____________________________________________________________________________
	Merge a group from one set into another.  If a group with the same name
	exists, items found in the source set are all added (if not already there)
	into the destination set.
	
	Recursively adds everything.
____________________________________________________________________________*/
	PGPError
PGPMergeGroupIntoDifferentSet(
	PGPGroupSetRef		fromSet,
	PGPGroupID			fromID,
	PGPGroupSetRef		toSet )
{
	PGPError			err	= kPGPError_NoErr;
	PGPGroup const *	group	= NULL;
	
	PGPValidateGroupSet( fromSet );
	PGPValidateGroupSet( toSet );
	
	err	= sFindGroupByID( fromSet, fromID, (PGPGroup **)&group );
	if ( IsntPGPError( err ) )
	{
		PGPGroupID	newID;
		
		err	= sAddGroupIntoDifferentSet( group, toSet, &newID );
	}
	
	return( err );
}


	PGPError
PGPMergeGroupSets(
	PGPGroupSetRef	fromSet,
	PGPGroupSetRef	intoSet )
{
	PGPUInt32	index;
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidateGroupSet( fromSet );
	PGPValidateGroupSet( intoSet );
	
	for( index = 0; index < fromSet->numGroups; ++index )
	{
		PGPGroupID	fromID;
		
		err	= PGPGetIndGroupID( fromSet, index, &fromID );
		if ( IsPGPError( err ) )
			break;
		
		err	= PGPMergeGroupIntoDifferentSet( fromSet, fromID, intoSet );
		if ( IsPGPError( err ) )
			break;
	}
	
	return( err );
}






#if PGP_DEBUG /* [ */
#if PGP_MACINTOSH

#include "MacStrings.h"

	static PGPError
sTestCreate(
	PFLContextRef		context,
	PGPGroupSetRef *	outSet,
	PGPGroupID *		outMaster )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroupSetRef	set	= NULL;
	
	*outSet		= NULL;
	*outMaster	= kPGPInvalidGroupID;
			
	err	= PGPNewGroupSet( context, &set );
	if ( IsntPGPError( err ) )
	{
		PGPUInt32		index;
		PGPUInt32		kNumTestGroups	= 500;
		PGPBoolean		needsCommit;
		PGPGroupItem	item;
		PGPGroupID		masterID;
		PGPUInt32		numKeys;
		PGPUInt32		numItems;
		
		*outSet	= set;
		
		needsCommit	= PGPGroupSetNeedsCommit( set );
		pgpAssert( needsCommit );
			
		for( index = 0; index < kNumTestGroups; ++index )
		{
			PGPGroupID	newID;
			Str255		pStr;
			char		name[ 256 ];
			
			NumToString( index, pStr );
			PToCString( pStr, name );
			
			err	= PGPNewGroup( set, name, "test group", &newID );
			if ( IsPGPError( err ))
				break;
				
			/* add a key to the gruop */
			pgpClearMemory( &item, sizeof( item ));
			item.type	= kPGPGroupItem_KeyID;
			item.u.key.length	= 9;	/* whatever */
			err	= PGPAddItemToGroup( set, &item, newID );
			if ( IsPGPError( err ))
				break;
		}
		
		/* add all existing groups to the master group */
		if ( IsntPGPError( err ) )
		{
			PGPUInt32	numGroups;
			
			err	= PGPCountGroupsInSet( set, &numGroups );
			pgpAssertNoErr( err );
			
			if ( IsntPGPError( err ) )
			{
				err	= PGPNewGroup( set, "master group",
							"contains all the other groups",
							&masterID);
				pgpAssertNoErr( err );
			}
		
			if ( IsntPGPError( err ) )
			{
				*outMaster	= masterID;
				
				for( index = 0; index < numGroups; ++index )
				{
					PGPGroupID	id;
					
					err	= PGPGetIndGroupID( set, index, &id);
					pgpAssertNoErr( err );
					if ( IsPGPError( err ))
						break;
						
					item.type		= kPGPGroupItem_Group;
					item.u.group.id	= id;
					err	= PGPAddItemToGroup( set, &item, masterID );
					pgpAssertNoErr( err );
					if ( IsPGPError( err ))
						break;
				}
			
				if ( IsntPGPError( err ) )
				{
					err	= PGPCountGroupItems( set, masterID, TRUE,
							&numKeys, &numItems );
					pgpAssertNoErr( err );
					pgpAssert( numItems - numKeys == numGroups );
				}
			}
		}
		
		if ( IsPGPError( err ) )
		{
			PGPFreeGroupSet( set );
			*outSet	= NULL;
		}
	}
	return( err );
}


	static PGPError
sIteratorTest(
	PGPGroupSetRef		set,
	PGPGroupID			id )
{
	PGPError			err	= kPGPError_NoErr;
	PGPGroupItemIterRef	iter;
	PGPGroupItem		item;
	
	/* test iterators */
	err	= PGPNewGroupItemIter( set, id, kPGPGroupIterFlags_AllItems, &iter );
	pgpAssertNoErr( err );
	while ( IsntPGPError( err = PGPGroupItemIterNext( iter, &item ) ) )
	{
		/* just iterate through all */
	}
	pgpAssert( err == kPGPError_EndOfIteration );
	err	= PGPFreeGroupItemIter( iter );
	pgpAssertNoErr( err );

	/* now do it recursively */
	err	= PGPNewGroupItemIter( set, id,
				kPGPGroupIterFlags_AllRecursive, &iter);
	pgpAssertNoErr( err );
	while ( IsntPGPError( err = PGPGroupItemIterNext( iter, &item ) ) )
	{
		/* just iterate through all */
	}
	pgpAssert( err == kPGPError_EndOfIteration );
	err	= PGPFreeGroupItemIter( iter );
	pgpAssertNoErr( err );
	
	return( err );
}


	static PGPError
sSaveRestoreTest( PGPGroupSetRef set )
{
	FSSpec			fsSpec;
	PFLFileSpecRef	fileSpec	= NULL;
	PGPError		err	= kPGPError_NoErr;

#if PGP_MACINTOSH
	pgpAssert( PGPGroupSetNeedsCommit( set ) );
	FSMakeFSSpec( 0, 0, "\pRAM Disk:test", &fsSpec );
	err	= PFLNewFileSpecFromFSSpec( PGPGetGroupSetContext( set ),
			&fsSpec, &fileSpec );
	pgpAssertNoErr( err );
#else
	err	= kPGPError_FileNotFound;
#endif

	if ( IsntPGPError( err ) )
		err	= PGPSaveGroupSetToFile( set, fileSpec );
	pgpAssertNoErr( err );
	if ( IsntPGPError( err ) )
	{
		PGPGroupSetRef	tempSet;
		
		err	= PGPNewGroupSetFromFile( fileSpec, &tempSet );
		pgpAssertNoErr( err );
		
		err	= PGPFreeGroupSet( tempSet );
		pgpAssertNoErr( err );
	}

	PFLFreeFileSpec( fileSpec );
	
	return( err );
}

	static PGPError
sMergeTest( PGPGroupSetRef set )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroupSetRef	toSet;
	PGPUInt32		numGroups;
	PGPUInt32		index;
	
	/* copy the entire group into another group */
	err	= PGPCountGroupsInSet( set, &numGroups );
	pgpAssertNoErr( err );
	if ( IsntPGPError( err ) )
	{
		err	= PGPNewGroupSet( PGPGetGroupSetContext(set), &toSet );
		pgpAssertNoErr( err );
		if ( IsntPGPError( err ) )
		{
			for( index = 0; index < numGroups; ++index )
			{
				PGPGroupID	id;
				
				err	= PGPGetIndGroupID( set, index, &id );
				pgpAssertNoErr( err );
				if ( IsPGPError( err ) )
					break;
				
				err	= PGPMergeGroupIntoDifferentSet( set, id, toSet );
				pgpAssertNoErr( err );
				if ( IsPGPError( err ) )
					break;
			}
			err	= PGPFreeGroupSet( toSet );
			pgpAssertNoErr( err );
		}
	}
	
	return( err );
}


	static PGPError
sDeleteTest( PGPGroupSetRef set )
{
	PGPError	err	= kPGPError_NoErr;
	PGPUInt32	index;
	PGPUInt32	numGroups;
	
	err	= PGPCountGroupsInSet( set, &numGroups );
	
	for( index = 0; index < numGroups; ++index )
	{
		PGPGroupID	id;
		
		err	= PGPGetIndGroupID( set, 0, &id );
		pgpAssertNoErr( err );
		if ( IsntPGPError( err ) )
		{
			err	= PGPDeleteGroup( set, id );
			pgpAssertNoErr( err );
		}
	}
	return( err );
}


	static PGPError
sMiscTest( PGPGroupSetRef set )
{
	PGPError	err	= kPGPError_NoErr;
	PGPGroupID	id;
	PGPUInt32	numGroups;
	
	err	= PGPCountGroupsInSet( set, &numGroups );
	pgpAssertNoErr( err );
	if ( IsntPGPError( err ) )
	{
		PGPUInt32		index;
		PGPGroupInfo	info;
		
		for( index = 0; index < numGroups; ++index )
		{
			err	= PGPGetIndGroupID( set, 0, &id );
			if (IsntPGPError( err ) )
			{
				err	= PGPGetGroupInfo( set, id, &info );
				pgpAssertNoErr( err );
			}
		}
	}
	
	return( err );
}


	void
TestGroups()
{
	PGPGroupSetRef	set;
	PGPError		err;
	PFLContextRef	context;
	
	err	= PFLNewContext( &context );
	if ( IsntPGPError( err ) )
	{
		PGPGroupID			masterID;
			
		err	= sTestCreate( context, &set, &masterID );
		if ( IsntPGPError( err ) )
		{
			err	= sIteratorTest( set, masterID );
			pgpAssertNoErr( err );
			
			if (IsntPGPError( err ) )
			{
				err	= sMiscTest( set );
				pgpAssertNoErr( err );
			}
			
			if (IsntPGPError( err ) )
			{
				err	= sSaveRestoreTest( set );
				pgpAssertNoErr( err );
			}
			
			if (IsntPGPError( err ) )
			{
				err	= sMergeTest( set );
				pgpAssertNoErr( err );
			}
			
			if (IsntPGPError( err ) )
			{
				err	= sDeleteTest( set );
				pgpAssertNoErr( err );
			}


			err	= PGPFreeGroupSet( set );
			pgpAssertNoErr( err );
		}
		PFLFreeContext( context);
	}
	pgpAssertNoErr( err );
}

#endif
#endif /* ] */



















/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
