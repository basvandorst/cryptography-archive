/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpGroups.h,v 1.22 1997/09/30 19:58:54 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpGroups_h	/* [ */
#define Included_pgpGroups_h

#include "pgpPFLConfig.h"

#include "pflContext.h"

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif



#define kPGPMaxGroupNameLength				63
#define kPGPMaxGroupDescriptionLength		63
typedef char	PGPGroupName[ kPGPMaxGroupNameLength + 1 ];
typedef char	PGPGroupDescription[ kPGPMaxGroupDescriptionLength + 1 ];


typedef struct PGPGroupSet *		PGPGroupSetRef;
typedef struct PGPGroupIter *		PGPGroupItemIterRef;

#define	kInvalidPGPGroupSetRef			((PGPGroupSetRef) NULL)
#define	kInvalidPGPGroupItemIterRef		((PGPGroupItemIterRef) NULL)

#define PGPGroupSetRefIsValid(ref)		((ref) != kInvalidPGPGroupSetRef)
#define PGPGroupItemIterRefIsValid(ref)	((ref) != kInvalidPGPGroupItemIterRef)

/* any type will do that is distinct */
typedef PGPUInt32				PGPGroupID;
#define kPGPInvalidGroupID		( (PGPGroupID)0 )

enum PGPGroupItemType_
{
	kPGPGroupItem_KeyID = 1,
	kPGPGroupItem_Group,
	
	PGP_ENUM_FORCE( PGPGroupItemType_)
};
PGPENUM_TYPEDEF( PGPGroupItemType_, PGPGroupItemType );

/*____________________________________________________________________________
	A run-time group item, used when iterating through a group.
	For client use; not necessarily the internal storage format.
	
	'userValue' is *not* saved to disk.
____________________________________________________________________________*/
typedef struct PGPGroupItem
{
	PGPGroupItemType	type;
	PGPUserValue		userValue;
	union
	{
		/* type selects which substructure */
		struct	/* if kGroupItem_Group */
		{
			PGPGroupID	id;				
		} group;
		
		struct	/* if kGroupItem_KeyID */
		{
			/* exported key IDs currently take 1 + 8 bytes */
			PGPUInt32	algorithm;
			PGPByte		length;		/* actual size of data */
			#define kPGPGroupItemPad		0
			PGPByte		exportedKeyID[ 8 + 1 + kPGPGroupItemPad ];
		} key;
	} u;
} PGPGroupItem;


/*____________________________________________________________________________
	Info obtained via PGPGetGroupInfo.
____________________________________________________________________________*/
typedef struct PGPGroupInfo
{
	PGPGroupID		id;
	PGPGroupName	name;
	PGPGroupName	description;
	PGPUserValue	userValue;
} PGPGroupInfo;


typedef PGPFlags	PGPGroupItemIterFlags;
/* flag (1UL << 0 ) is reserved */
#define kPGPGroupIterFlags_Recursive	(PGPFlags)(1UL << 1 )
#define kPGPGroupIterFlags_Keys			(PGPFlags)(1UL << 2 )
#define kPGPGroupIterFlags_Groups		(PGPFlags)(1UL << 3 )

#define kPGPGroupIterFlags_AllKeysRecursive			\
	( kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Keys )
	
#define kPGPGroupIterFlags_AllGroupsRecursive			\
	( kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Groups )
	
#define kPGPGroupIterFlags_AllItems			\
	( kPGPGroupIterFlags_Keys | kPGPGroupIterFlags_Groups )
	
#define kPGPGroupIterFlags_AllRecursive	\
	( kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_AllItems )
	
	

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif




PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/*____________________________________________________________________________
	Manipulating pgp group sets (PGPGroupSetRef)
____________________________________________________________________________*/
/* create a new, empty groups collection */
PGPError	PGPNewGroupSet( PFLContextRef context, PGPGroupSetRef *outRef );

/* file is *not* left open; all data is loaded into memory */
PGPError	PGPNewGroupSetFromFile( PFLFileSpecRef file,
				PGPGroupSetRef *outRef );
#if PGP_MACINTOSH
PGPError	PGPNewGroupSetFromFSSpec( PFLContextRef context,
				const FSSpec *spec, PGPGroupSetRef *outRef );
#endif

/* overwrites existing.  Don't bother unless PGPGroupSetNeedsCommit() */
PGPError	PGPSaveGroupSetToFile( PGPGroupSetRef set, PFLFileSpecRef file );

/* free all data structures; be sure to save first if you want */
PGPError	PGPFreeGroupSet( PGPGroupSetRef set );


/* has the group changed? */
PGPBoolean		PGPGroupSetNeedsCommit( PGPGroupSetRef set );

PFLContextRef	PGPGetGroupSetContext( PGPGroupSetRef set );

PGPBoolean		PGPGroupSetIsValid( PGPGroupSetRef set );
#define PGPValidateGroupSet( set )	\
		PGPValidateParam( PGPGroupSetIsValid( set ) );
		

		
/*____________________________________________________________________________
	Manipulating groups
	
	Groups are always referred to by IDs which remain valid until the set
	is disposed.
____________________________________________________________________________*/

/* initial parent ID is kPGPInvalidGroupID */
PGPError	PGPNewGroup( PGPGroupSetRef set,
				const char * name, const char *description, PGPGroupID *id );

PGPError	PGPCountGroupsInSet( PGPGroupSetRef set,
				PGPUInt32 *numGroups);
PGPError	PGPGetIndGroupID( PGPGroupSetRef set,
				PGPUInt32 index, PGPGroupID *id );

/* delete this group from the set */
/* All references to it are removed in all sets */
PGPError	PGPDeleteGroup( PGPGroupSetRef set, PGPGroupID id );

/* delete the indexed item from the group */
/* the item may be a group or a key */
PGPError	PGPDeleteIndItemFromGroup( PGPGroupSetRef set,
				PGPGroupID	id, PGPUInt32 item );

/* same as PGPDeleteIndItemFromGroup, but accepts an item */
PGPError	PGPDeleteItemFromGroup( PGPGroupSetRef set,
				PGPGroupID id, PGPGroupItem const *item );


PGPError	PGPGetGroupInfo( PGPGroupSetRef set,
				PGPGroupID id, PGPGroupInfo *info );
				
PGPError	PGPSetGroupName( PGPGroupSetRef set,
				PGPGroupID id, const char * name );
PGPError	PGPSetGroupUserValue( PGPGroupSetRef set,
				PGPGroupID id, PGPUserValue userValue );
PGPError	PGPSetGroupDescription( PGPGroupSetRef set,
				PGPGroupID id, const char * name );

/* 'item' specifies a group or a key id */
/* you must fill the item in completely */
PGPError	PGPAddItemToGroup( PGPGroupSetRef set,
				PGPGroupItem const *item, PGPGroupID group );


PGPError	PGPMergeGroupIntoDifferentSet( PGPGroupSetRef fromSet,
				PGPGroupID fromID, PGPGroupSetRef toSet );


PGPError	PGPMergeGroupSets( PGPGroupSetRef fromSet,
				PGPGroupSetRef intoSet );

/*____________________________________________________________________________
	Manipulating group items
____________________________________________________________________________*/

/* count how many items there are in a group */
/* totalItems includes keys and groups */
PGPError	PGPCountGroupItems( PGPGroupSetRef set,
					PGPGroupID id, PGPBoolean recursive,
					PGPUInt32 * numKeys,
					PGPUInt32 * totalItems );

/* non-recursive call; index only applies to group itself */
PGPError	PGPGetIndGroupItem( PGPGroupSetRef set,
				PGPGroupID id, PGPUInt32 index, PGPGroupItem * item );

/* use PGPGetIndGroupItem() if you want to get the user value */
PGPError	PGPSetIndGroupItemUserValue( PGPGroupSetRef set,
				PGPGroupID id, PGPUInt32 index, PGPUserValue userValue );


typedef int		(*PGPGroupItemCompareProc)( PGPGroupItem *,
					PGPGroupItem *, PGPUserValue userValue );

PGPError	PGPSortGroupItems( PGPGroupSetRef set, PGPGroupID id,
				PGPGroupItemCompareProc, PGPUserValue userValue );
				
PGPError	PGPSortGroupSet( PGPGroupSetRef set,
				PGPGroupItemCompareProc, PGPUserValue userValue );

/*____________________________________________________________________________
	PGPGroupItemIterRef--iterator through group items.
	
	Special note: this is not a full-fledged iterator.  You may *not* add
	or delete items while iterating and you may only move forward.  However,
	you may change the values of items.
____________________________________________________________________________*/

PGPError	PGPNewGroupItemIter( PGPGroupSetRef set, PGPGroupID id,
				PGPGroupItemIterFlags flags, PGPGroupItemIterRef *iter );
				
PGPError	PGPFreeGroupItemIter( PGPGroupItemIterRef iter );

/* returns kPGPError_EndOfIteration when done */
PGPError	PGPGroupItemIterNext( PGPGroupItemIterRef iter,
				PGPGroupItem * item );



#if PGP_DEBUG

#if PGP_MACINTOSH
void	TestGroups( void );
#endif

#endif



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpGroups_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/








