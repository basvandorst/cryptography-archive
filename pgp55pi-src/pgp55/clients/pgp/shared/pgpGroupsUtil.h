/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpGroupsUtil.h,v 1.9 1997/09/18 02:27:48 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpGroupsUtil_h	/* [ */
#define Included_pgpGroupsUtil_h

#include "pgpGroups.h"


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


#include "pgpEncode.h"

/*____________________________________________________________________________
	Return the lowest validity of any item in the group
	keyset should contain all keys available
	It is not an error if keys can't be found; you may want to check
	the not found count.
	
	The lowest validity is kPGPValidity_Invalid and kPGPValidity_Unknown
	is never returned.
____________________________________________________________________________*/
PGPError	PGPGetGroupLowestValidity( PGPGroupSetRef set, PGPGroupID id,
				PGPKeySetRef keySet, PGPValidity * lowestValidity,
				PGPUInt32 * numKeysNotFound);



/*____________________________________________________________________________
	All all the keys in the group (and its subgroups) to the keyset
____________________________________________________________________________*/
PGPError	PGPNewKeySetFromGroup( PGPGroupSetRef set, PGPGroupID id,
				PGPKeySetRef masterSet, PGPKeySetRef * resultSet,
				PGPUInt32 * numKeysNotFound);

/*____________________________________________________________________________
	Create a simple, flattened group of unique key IDs from the source group.
	Note that sourceSet and destSet must be different.
____________________________________________________________________________*/
PGPError	PGPNewFlattenedGroupFromGroup(PGPGroupSetRef sourceSet,
				PGPGroupID sourceID, PGPGroupSetRef destSet,
				PGPGroupID *destID);

/* do the standard sorting */
PGPError	PGPSortGroupSetStd( PGPGroupSetRef set,
				PGPKeySetRef keys );
				
#if PGP_DEBUG /* [ */

PGPError	TestGroupsUtil( void );

#endif /* ] */



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpGroupsUtil_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
