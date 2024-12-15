/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpGroupsUtil.c,v 1.4 1999/03/10 02:51:21 heller Exp $
____________________________________________________________________________*/
#include <string.h>

#include "pgpMem.h"

#include "pgpContext.h"
#include "pgpGroups.h"
#include "pgpGroupsPriv.h"
#include "pgpKeys.h"
#include "pgpEncode.h"
#include "pgpStrings.h"


#include "pgpErrors.h"

#if PGP_MACINTOSH
#include "MacStrings.h"
#endif



/*____________________________________________________________________________
	Caller must call PGPFreeKey() on resulting key
____________________________________________________________________________*/

	static PGPError
sGroupItemToKey(
	PGPGroupItem const *	item,
	PGPKeySetRef			keySet,
	PGPKeyRef *				outKey )
{
	PGPError		err	= kPGPError_NoErr;
	PGPContextRef	context;
	PGPKeyRef		key	= kInvalidPGPKeyRef;
	
	context	= PGPGetKeySetContext( keySet );
	
	/* get the key ID into a PGPsdk form */
	err	= PGPGetKeyByKeyID( keySet, &item->u.key.keyID,
			item->u.key.algorithm, &key );
	
	*outKey	= key;
	
	pgpAssert( ( IsntPGPError( err ) && IsntNull( *outKey ) ) ||
		( IsPGPError( err ) && IsNull( *outKey ) ));
	return( err );
}




	static PGPValidity
sMinValidity(
	PGPValidity	validity1,
	PGPValidity	validity2 )
{
	PGPValidity	validity;
	
	if ( validity1 == kPGPValidity_Unknown ||
		validity2 == kPGPValidity_Unknown )
	{
		/* lowest we can get */
		validity	= kPGPValidity_Invalid;
		return( validity );
	}
	
	/* asserts above guarantee we can use a simple MIN here */
	pgpAssert( kPGPValidity_Invalid < kPGPValidity_Marginal );
	pgpAssert( kPGPValidity_Marginal < kPGPValidity_Complete );
	if ( validity1 < validity2 )
		validity	= validity1;
	else
		validity	= validity2;
	
	return( validity );
}


/*____________________________________________________________________________
	Return the lowest validity of any item in the group
	keyset should contain all keys available
	It is not an error if keys can't be found; you may want to check
	the not found count.
	
	The lowest validity is kPGPValidity_Invalid and kPGPValidity_Unknown
	is never returned.
____________________________________________________________________________*/
	PGPError
PGPGetGroupLowestValidity(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPKeySetRef	keySet,
	PGPValidity *	lowestValidityOut,
	PGPUInt32 *		numKeysNotFound )
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroupItemIterRef	iter;
	
	if ( IsntNull( lowestValidityOut ) )
		*lowestValidityOut	= kPGPValidity_Invalid;
	if ( IsntNull( numKeysNotFound ) )
		*numKeysNotFound	= 0;
	PGPValidatePtr( lowestValidityOut );
	PGPValidatePtr( numKeysNotFound );
		
	PGPValidateGroupSet( set );
	PGPValidateParam( IsntNull( keySet ) );
	
	err	= PGPNewGroupItemIter( set, id,
			kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Keys,
			&iter );
	if ( IsntPGPError( err )  )
	{
		PGPGroupItem	item;
		PGPContextRef	context;
		PGPValidity		lowestValidity	= kPGPValidity_Complete;
		PGPBoolean		examinedAKey	= FALSE;
		
		context	= PGPGetKeySetContext( keySet );
		
		/* 'lowestValidity' can only decrease while looping */
		while( IsntPGPError( err = PGPGroupItemIterNext( iter, &item ) ) )
		{
			PGPKeyRef	key;
			
			pgpAssert( item.type == kPGPGroupItem_KeyID );
			
			err	= sGroupItemToKey( &item, keySet, &key );
			if ( IsntPGPError( err) )
			{
				PGPValidity		validity;
				
				examinedAKey	= TRUE;
			
				err	= PGPGetPrimaryUserIDValidity( key, &validity );
				
				lowestValidity	= sMinValidity( validity, lowestValidity );
				if ( lowestValidity == kPGPValidity_Invalid &&
					IsNull( numKeysNotFound ))
				{
					/* it doesn't get any lower than kPGPValidity_Invalid and
					   the caller does not want the number of missing keys */
					break;
				}
				
			}
			else
			{
				if ( err == kPGPError_ItemNotFound )
				{
					/* just can't find the key; that's OK */
					err	= kPGPError_NoErr;
					*numKeysNotFound	+= 1;
				}
			}
			
			if ( IsPGPError( err ) )
				break;
		}
		if ( err == kPGPError_EndOfIteration )
			err	= kPGPError_NoErr;
		
		/* if we examined keys, use the validity we found, otherwise
			consider it invalid */
		if ( examinedAKey )
			*lowestValidityOut	= lowestValidity;
		else
			*lowestValidityOut	= kPGPValidity_Invalid;
			
		PGPFreeGroupItemIter( iter );
	}
	
	return( err );
}



/*____________________________________________________________________________
	All all the keys in the group (and its subgroups) to the keyset
____________________________________________________________________________*/
	PGPError
PGPNewKeySetFromGroup(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPKeySetRef	masterSet,		/* resolve key IDs using the set */
	PGPKeySetRef *	outKeys,
	PGPUInt32 *		numKeysNotFound
	)
{
	PGPError		err	= kPGPError_NoErr;
	PGPGroupItemIterRef	iter;
	PGPKeySetRef	workingSet	= kInvalidPGPKeySetRef;
	
	if ( IsntNull( numKeysNotFound ) )
		*numKeysNotFound	= 0;
	if ( IsntNull( outKeys ) )
		*outKeys	= kInvalidPGPKeySetRef;
	PGPValidatePtr( numKeysNotFound );
	PGPValidatePtr( outKeys );
	PGPValidateGroupSet( set );
	PGPValidatePtr( masterSet );
	
	err	= PGPNewEmptyKeySet( masterSet, &workingSet );
	if ( IsntPGPError( err ) )
	{
		err	= PGPNewGroupItemIter( set, id,
				kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Keys,
				&iter );
	}
	if ( IsntPGPError( err )  )
	{
		PGPGroupItem	item;
		
		while( IsntPGPError( err = PGPGroupItemIterNext( iter, &item ) ) )
		{
			PGPKeyRef	key;
			
			pgpAssert( item.type == kPGPGroupItem_KeyID );
			
			err	= sGroupItemToKey( &item, masterSet, &key );
			if ( IsntPGPError( err ) )
			{
				PGPKeySetRef	singleKeySet;
				
				err	= PGPNewSingletonKeySet( key, &singleKeySet );
				if ( IsntPGPError( err ) )
				{
					PGPKeySetRef	oldKeys	= workingSet;
					
					err = PGPUnionKeySets( workingSet,
							singleKeySet, &workingSet );
							
					(void) PGPFreeKeySet( singleKeySet );
					(void) PGPFreeKeySet( oldKeys );
				}
				
				
				if ( IsPGPError( err ) )
					break;
			}
			else if ( err == kPGPError_ItemNotFound )
			{
				*numKeysNotFound	+= 1;
			}
			else
			{
				break;
			}
		}
		if ( err == kPGPError_EndOfIteration )
			err	= kPGPError_NoErr;
			
		PGPFreeGroupItemIter( iter );
	}
	
	if ( IsPGPError( err ) && PGPKeySetRefIsValid( workingSet ) )
	{
		(void) PGPFreeKeySet( workingSet );
		workingSet	= kInvalidPGPKeySetRef;
	}
	
	*outKeys	= workingSet;
	
	return( err );
}



typedef struct
{
	PGPKeySetRef	keySet;
	PGPGroupSetRef	set;
} MySortInfo;

	static PGPInt32
sCompareGroupItems(
	PGPGroupItem *	item1,
	PGPGroupItem *	item2,
	void *			userValue )
{
	PGPKeyRef		key1;
	PGPKeyRef		key2;
	PGPError		err1;
	PGPError		err2;
	MySortInfo const *	info	= (MySortInfo const *)userValue;
	PGPKeySetRef	keySet	= info->keySet;
	PGPInt32		result;
	
	pgpAssert( IsntNull( info ) );
		
	keySet	= info->keySet;
	
	if ( item1->type == kPGPGroupItem_Group ||
			item2->type == kPGPGroupItem_Group )
	{
		PGPGroupInfo	groupInfo1;
		PGPGroupInfo	groupInfo2;
		
		/* for now, always make groups come before keys */
		if ( item1->type == kPGPGroupItem_Group &&
			item2->type == kPGPGroupItem_KeyID )
		{
			return( -1 );
		}
		else if ( item1->type == kPGPGroupItem_KeyID &&
			item2->type == kPGPGroupItem_Group )
		{
			return( 1 );
		}
		/* both are groups */
		PGPGetGroupInfo( info->set, item1->u.group.id, &groupInfo1 );
		PGPGetGroupInfo( info->set, item2->u.group.id, &groupInfo2 );
		return( pgpCompareStringsIgnoreCase( groupInfo1.name,
					groupInfo2.name ) );
	}
	
	err1	= sGroupItemToKey( item1, keySet, &key1 );
	err2	= sGroupItemToKey( item2, keySet, &key2 );
	
	/*
		Rules:
		errors on both: consider the same
		error on one: it's always less than
	*/
	if ( IsPGPError( err1 ) && IsPGPError( err2 ) )
	{
		/* if we got errors on both, consider them the same */
		result	= 0;
	}
	else if ( IsPGPError( err1 ) )
	{
		result	= -1;
	}
	else if ( IsPGPError( err2 ) )
	{
		result	= 1;
	}
	else
	{
		result	= PGPCompareKeys( key1, key2, kPGPUserIDOrdering );
	}
	
	
	return( result );
}


	PGPError
PGPSortGroupSetStd(
	PGPGroupSetRef	set,
	PGPKeySetRef	keySet )
{
	PGPError	err	= kPGPError_NoErr;
	MySortInfo	info;
	
	info.set	= set;
	info.keySet	= keySet;
	
	err	= PGPSortGroupSet( set, sCompareGroupItems, &info );
	
	return( err );
}

	PGPError
PGPNewFlattenedGroupFromGroup(
	PGPGroupSetRef	sourceSet,
	PGPGroupID		sourceID,
	PGPGroupSetRef	destSet,
	PGPGroupID		*destID)
{
	PGPError	err;

	PGPValidatePtr( destID );
	*destID	= kPGPInvalidGroupID;
	PGPValidateGroupSet( sourceSet );
	PGPValidateGroupSet( destSet );
	PGPValidateParam( sourceSet != destSet );

	err = PGPNewGroup( destSet, "", "", destID );
	if( IsntPGPError( err ) )
	{
		PGPGroupItemIterRef	iterator;
		
		err	= PGPNewGroupItemIter( sourceSet, sourceID,
					kPGPGroupIterFlags_Recursive | kPGPGroupIterFlags_Keys,
					&iterator );
		if ( IsntPGPError( err )  )
		{
			PGPGroupItem	item;
			
			err = PGPGroupItemIterNext( iterator, &item );
			while( IsntPGPError( err ) )
			{
				err = PGPAddItemToGroup( destSet, &item, *destID );
				if( err == kPGPError_ItemAlreadyExists )
					err = kPGPError_NoErr;
				
				if( IsPGPError( err ) )
					break;
						
				err = PGPGroupItemIterNext( iterator, &item );
			}
			
			if ( err == kPGPError_EndOfIteration )
				err	= kPGPError_NoErr;
				
			PGPFreeGroupItemIter( iterator );
		}
		
		if( IsPGPError( err ) )
		{
			(void) PGPDeleteGroup( destSet, *destID );
			*destID = kPGPInvalidGroupID;
		}
	}
	
	return( err );
}



#if PGP_DEBUG /* [ */

	static PGPError
sAddAllKeysToGroup(
	PGPGroupSetRef	set,
	PGPGroupID		id,
	PGPKeySetRef	keySet )
{
	PGPError		err	= kPGPError_NoErr;
	PGPKeyListRef	keyList;
	
	err = PGPOrderKeySet( keySet, kPGPAnyOrdering, &keyList );
	if( IsntPGPError( err ) )
	{
		PGPKeyIterRef	iter;
		
		err = PGPNewKeyIter( keyList, &iter );
		if( IsntPGPError( err ) )
		{
			PGPKeyRef		key;
			
			while( IsntPGPError( err = PGPKeyIterNext( iter, &key ) ) )
			{
				PGPKeyID	keyID;
				
				err	= PGPGetKeyIDFromKey( key, &keyID );
				if ( IsntPGPError( err ) )
				{
					PGPGroupItem	item;
					PGPInt32		algorithm;
					
					item.type				= kPGPGroupItem_KeyID;
					item.u.key.keyID		= keyID;
					PGPGetKeyNumber( key, kPGPKeyPropAlgID, &algorithm);
					item.u.key.algorithm	= (PGPPublicKeyAlgorithm) algorithm;
						
					err	= PGPAddItemToGroup( set, &item, id);
					
					if ( IsPGPError( err ) )
						break;
				}
			}
			
			if( err == kPGPError_EndOfIteration )
				err = kPGPError_NoErr;
				
			(void) PGPFreeKeyIter( iter );
		}
		(void) PGPFreeKeyList( keyList );
	}
	return( err );
}


	static PGPError
sTestGroupValidity( PGPKeySetRef	keySet )
{
	PGPValidity		validity;
	PGPUInt32		numNotFound;
	PGPError		err	= kPGPError_NoErr;
	PGPGroupSetRef	set;
	PGPContextRef	memoryMgr	= NULL;
	
	err	= PGPNewContext( kPGPsdkAPIVersion, &memoryMgr );
	if ( IsntPGPError( err ) )
	{
		err	= PGPNewGroupSet( memoryMgr, &set );
		if ( IsntPGPError( err ) )
		{
			PGPGroupID	id;
			
			err	= PGPNewGroup( set, "test group", "", &id );
			if ( IsntPGPError( err ) )
				err	= sAddAllKeysToGroup( set, id, keySet );
			
			if ( IsntPGPError( err ) )
			{
				err	= PGPGetGroupLowestValidity( set, id, 
							keySet, &validity, &numNotFound );
			}
			
			PGPFreeGroupSet( set );
		}

		PGPFreeContext( memoryMgr );
	}
	
	return( err );
}


	PGPError
TestGroupsUtil()
{
	PGPContextRef	context;
	PGPError		err	= kPGPError_NoErr;
	
	err	= PGPNewContext( kPGPsdkAPIVersion, &context );
	if ( IsntPGPError( err ) )
	{
		PGPKeySetRef	keySet;
		
		err	= PGPOpenDefaultKeyRings( context, 0, &keySet );
		if ( IsntPGPError( err ) )
		{
			err	= sTestGroupValidity( keySet );
			
			PGPFreeKeySet( keySet );
		}
		PGPFreeContext( context );
	}
	
	pgpAssertNoErr( err );
	return( err );
}

#endif /* ] */



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
