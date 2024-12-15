/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Key filtering routines implementing the PGPFilterRef abstract data type

	$Id: pgpKeyFilter.c,v 1.77 1997/10/22 08:58:18 wprice Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"

#include <string.h>
#include <ctype.h>

#include "pgpDebug.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpKDBInt.h"
#include "pgpKeyFilterPriv.h"
#include "pgpKeyIDPriv.h"
#include "pgpTimeDate.h"
#include "pgpKeyIDPriv.h"


	PGPBoolean
PGPFilterIsValid( PGPFilterRef	filter )
{
	return( IsntNull( filter ) && filter->magic == kPGPFilterMagic );
}



#define PGPValidateMatchCriterion( m ) \
	pgpAssert( (m) == kPGPMatchEqual || (m) == kPGPMatchGreaterOrEqual || \
			   (m) == kPGPMatchLessOrEqual || (m) == kPGPMatchSubString )
	
	
/*
 * Stolen from pgpRngPub.c:
 *
 * Return pointer to first instance of (s1,l1) in (s0,l0),
 * ignoring case.  Uses a fairly simple-minded algorithm.
 * Search for the first char of s1 in s0, and when we have it,
 * scan for the rest.
 *
 * Is it worth mucking with Boyer-Moore or the like?
 */
static char const *
xmemimem(char const *s0, size_t l0, char const *s1, size_t l1)
{
	char c0, c1, c2;
	size_t l;

	/*
	 * The trivial cases - this means that NULL inputs are very legal
	 * if the correspinding lengths are zero.
	 */
	if (l0 < l1)
		return NULL;
	if (!l1)
		return s0;
	l0 -= l1;

	c1 = tolower((unsigned char)*s1);
	do {
		c0 = tolower((unsigned char)*s0);
		if (c0 == c1) {
			l = 0;
			do {
				if (++l == l1)
					return s0;
				c0 = tolower((unsigned char)s0[l]);
				c2 = tolower((unsigned char)s1[l]);
			} while (c0 == c2);
		}
		s0++;
	} while (l0--);
	return NULL;
}


	static PGPError
sAllocateFilter(
	PGPContextRef		context, 
	PGPFilterClass 		filterClass, 
	PGPFilterType		filterType,
	PGPMatchCriterion	match,
	PGPFilterRef *		outFilter)
{
	PGPFilterRef	newFilter	= NULL;
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter	= NULL;
	PGPValidateContext( context );
	PGPValidateParam( filterType < kPGPFilterTypeEnd );
	PGPValidateMatchCriterion( match );
	
	newFilter	= (PGPFilterRef)
		pgpContextMemAlloc( context, sizeof( PGPFilter),
		kPGPMemoryFlags_Clear);
	if( IsntNull( newFilter ) )
	{
		newFilter->magic		= kPGPFilterMagic;
		newFilter->context		= context;
		newFilter->refCount		= 1;
		newFilter->filterClass	= filterClass;
		newFilter->filterType	= filterType;
		newFilter->match		= match;
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}

	
	*outFilter = newFilter;
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPIncFilterRefCount( PGPFilterRef filter )
{
	PGPValidateFilter( filter );
	filter->refCount++;
	return kPGPError_NoErr;
}


	PGPError
PGPFreeFilter( PGPFilterRef filter )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidateFilter( filter );
	
	pgpAssert( filter->refCount >= 1 );
	
	filter->refCount--;
	if( filter->refCount == 0 ) 
	{
		PGPContextRef	context		= filter->context;
		void *			ptrToFree	= NULL;
		
		switch( filter->filterType) 
		{
		default:
			break;
		case kPGPFilterTypeKeyKeyID:
			break;
		case kPGPFilterTypeKeySubKeyID:
			break;
		case kPGPFilterTypeKeyFingerPrint:
			ptrToFree	= filter->value.keyFingerPrint.keyFingerPrintData;
			break;
		case kPGPFilterTypeUserIDEmail:
			ptrToFree	= filter->value.userIDEmail;
			break;
		case kPGPFilterTypeUserIDName:
			ptrToFree	= filter->value.userIDName;
			break;
		case kPGPFilterTypeUserIDString:
			ptrToFree	= filter->value.userIDString;
			break;
		case kPGPFilterTypeSigKeyID:
			break;
			
		case kPGPFilterTypeNot:
			PGPFreeFilter( filter->value.notFilter );
			break;
		case kPGPFilterTypeAnd:
			PGPFreeFilter( filter->value.andFilter.andFilter1 );
			PGPFreeFilter( filter->value.andFilter.andFilter2 );
			break;
		case kPGPFilterTypeOr:
			PGPFreeFilter( filter->value.orFilter.orFilter1 );
			PGPFreeFilter( filter->value.orFilter.orFilter2 );
			break;
		}
		
		if ( IsntNull( ptrToFree ) )
		{
			pgpContextMemFree( context, ptrToFree);
		}
		
		pgpContextMemFree( context, filter );
	}
	
	return err;
}


/*____________________________________________________________________________
	The passed in filter will be freed when the newly created filter is
	freed.  Therefore, if an error occurs, the passed filter must
	also be freed.  Caller should increment the passed-in filter 
	ref count if the filter should persist.
____________________________________________________________________________*/
	PGPError
PGPNegateFilter(
	PGPFilterRef	filter,
	PGPFilterRef *	outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	if ( IsntNull( outFilter ) )
	{
		*outFilter	= NULL;
		if ( ! PGPFilterIsValid( filter ) )
			err	= kPGPError_BadParams;
	}
	else
	{
		err	= kPGPError_BadParams;
	}
		
	if ( IsntPGPError( err ) )
	{
		err = sAllocateFilter( filter->context, filter->filterClass,
				kPGPFilterTypeNot, kPGPMatchDefault, outFilter);
		pgpAssertErrWithPtr( err, *outFilter );
								
		if ( IsntPGPError( err ) )
		{
			(*outFilter)->value.notFilter	= filter;
		}
	}
	
	/* careful to clean up in event of error */
	if ( IsPGPError( err ) )
	{
		if ( PGPFilterIsValid( filter ) )
		{
			PGPFreeFilter( filter );
			filter	= NULL;
		}
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


/*____________________________________________________________________________
	The passed in filters will be freed when the newly created filter is
	freed.  Therefore, if an error occurs, the passed filters must
	also be freed.  Caller should increment the passed-in filter 
	ref count if the filters should persist.
____________________________________________________________________________*/
	PGPError
PGPIntersectFilters(
	PGPFilterRef 	filter1,
	PGPFilterRef	filter2, 
	PGPFilterRef *	outFilter)
{
	PGPError			err	= kPGPError_NoErr;
		
	if ( IsntNull( outFilter ) )
		*outFilter	= NULL;
			
	if ( IsntNull( outFilter ) &&
		PGPFilterIsValid( filter1 ) &&
		PGPFilterIsValid( filter2 )  )
	{
		PGPFilterClass		filterClass;
	
		filterClass = filter1->filterClass & filter2->filterClass;
		if( filterClass == 0 )
			err	= kPGPError_InconsistentFilterClasses;
			
		if ( IsntPGPError( err ) )
		{
			err = sAllocateFilter( filter1->context, filterClass,
										kPGPFilterTypeAnd, kPGPMatchDefault, 
										outFilter);
			if( IsntPGPError( err ) )
			{
				(*outFilter)->value.andFilter.andFilter1 = filter1;
				(*outFilter)->value.andFilter.andFilter2 = filter2;
			}
		}
	}
	else
	{
		err	= kPGPError_BadParams;
	}
	
	
	/* careful to clean up in event of error */
	if ( IsPGPError( err ) )
	{
		if ( PGPFilterIsValid( filter1 ) )
			PGPFreeFilter( filter1 );
		if ( PGPFilterIsValid( filter2 ) )
			PGPFreeFilter( filter2 );
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


/*____________________________________________________________________________
	The passed in filters will be freed when the newly created filter is
	freed.  Therefore, if an error occurs, the passed filters must
	also be freed.  Caller should increment the passed-in filter 
	ref count if the filters should persist.
____________________________________________________________________________*/
	PGPError
PGPUnionFilters(
	PGPFilterRef	filter1,
	PGPFilterRef	filter2, 
	PGPFilterRef *	outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	if ( IsntNull( outFilter ) )
		*outFilter	= NULL;
		
	if ( IsntNull( outFilter ) &&
		PGPFilterIsValid( filter1 ) &&
		PGPFilterIsValid( filter2 )  )
	{
		PGPFilterClass	filterClass;
	
		filterClass	= filter1->filterClass & filter2->filterClass;
		if( filterClass == 0 )
			err	= kPGPError_InconsistentFilterClasses;
			
		if ( IsntPGPError( err ) )
		{
			err = sAllocateFilter( filter1->context, filterClass, 
					kPGPFilterTypeOr, kPGPMatchDefault,  outFilter);
			if( IsntPGPError( err ) )
			{
				(*outFilter)->value.orFilter.orFilter1 = filter1;
				(*outFilter)->value.orFilter.orFilter2 = filter2;
			}
		}
	}
	else
	{
		err	= kPGPError_BadParams;
	}
	
	/* careful to clean up in event of error */
	if ( IsPGPError( err ) )
	{
		if ( PGPFilterIsValid( filter1 ) )
			PGPFreeFilter( filter1 );
		if ( PGPFilterIsValid( filter2 ) )
			PGPFreeFilter( filter2 );
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewKeyIDFilter( 
	PGPContextRef		context,
	PGPKeyID const *	keyID,  
	PGPFilterRef *		outFilter)
{
	PGPError		err	= kPGPError_NoErr;
	PGPFilterRef	newFilter	= NULL;

	PGPValidatePtr( outFilter );
	*outFilter = NULL;
	PGPValidateContext( context );
	PGPValidateKeyID( keyID );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeyKeyID, kPGPMatchDefault, &newFilter);
	if( IsntPGPError( err ) )
	{
		newFilter->value.keyKeyID	= *keyID;
	}
	
	*outFilter = newFilter;
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
} 

	PGPError
PGPNewSubKeyIDFilter( 
	PGPContextRef		context,
	PGPKeyID const *	subKeyID,  
	PGPFilterRef *		outFilter)
{
	PGPError		err	= kPGPError_NoErr;
	PGPFilterRef	newFilter	= NULL;

	PGPValidatePtr( outFilter );
	*outFilter = NULL;
	PGPValidateContext( context );
	PGPValidateKeyID( subKeyID );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeySubKeyID, kPGPMatchDefault, &newFilter);
	if( IsntPGPError( err ) )
	{
		newFilter->value.keySubKeyID	= *subKeyID;
	}
	
	*outFilter = newFilter;
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewKeyEncryptAlgorithmFilter(
	PGPContextRef			context, 
	PGPPublicKeyAlgorithm	encryptAlgorithm, 
	PGPFilterRef *			outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter	= NULL;
	PGPValidateContext( context );
	PGPValidateParam( encryptAlgorithm >= kPGPPublicKeyAlgorithm_First &&
		encryptAlgorithm <= kPGPPublicKeyAlgorithm_Last	);
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeyEncryptAlgorithm,  kPGPMatchDefault, outFilter);
	if( IsntPGPError( err ) )
	{
		(*outFilter)->value.keyEncryptAlgorithm	= encryptAlgorithm;
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


/*____________________________________________________________________________
____________________________________________________________________________*/
	PGPError
PGPNewKeyFingerPrintFilter(
	PGPContextRef		context, 
	void const *		fingerPrint,  
	PGPSize				fingerPrintLength, 
	PGPFilterRef *		outFilter)
{
	PGPFilterRef	newFilter	= NULL;
	PGPError		err			= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter = NULL;
	PGPValidateContext( context );
	PGPValidatePtr( fingerPrint );
	PGPValidateParam( fingerPrintLength >= 1 );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeyFingerPrint,  kPGPMatchDefault, &newFilter);
	if( IsntPGPError( err ) )
	{
		PGPByte *		value		= NULL;
	
		value =( PGPByte *)
			pgpContextMemAlloc( context, fingerPrintLength, 0);
		if( IsntNull( value ) ) 
		{
			pgpCopyMemory( fingerPrint, value, fingerPrintLength);
			newFilter->value.keyFingerPrint.keyFingerPrintData		= value;
			newFilter->value.keyFingerPrint.keyFingerPrintLength	=
														fingerPrintLength;
		}
		else
		{
			err	= kPGPError_OutOfMemory;
			PGPFreeFilter( newFilter );
			newFilter	= NULL;
		}
	}
	
	
	*outFilter = newFilter;
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewKeyCreationTimeFilter(
	PGPContextRef		context,
	PGPTime				creationTime, 
	PGPMatchCriterion	match,
	PGPFilterRef *		outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter	= NULL;
	PGPValidateContext( context );
	PGPValidateMatchCriterion( match );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeyCreationTime, match, outFilter);
	if( IsntPGPError( err ) )
	{
		(*outFilter)->value.keyCreationTime = creationTime;
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewKeyExpirationTimeFilter(
	PGPContextRef		context,
	PGPTime				expirationTime, 
	PGPMatchCriterion	match,
	PGPFilterRef *		outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter = NULL;
	PGPValidateContext( context );
	PGPValidateMatchCriterion( match );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeyExpirationTime, match, outFilter);
								
	if( IsntPGPError( err ) )
	{
		(*outFilter)->value.keyExpirationTime	= expirationTime;
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewKeyRevokedFilter(
	PGPContextRef		context, 
	PGPBoolean			revoked, 
	PGPFilterRef *		outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter	= NULL;
	PGPValidateContext( context );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeyRevoked, kPGPMatchDefault, outFilter);
	if( IsntPGPError( err ) )
	{
		(*outFilter)->value.keyRevoked = revoked;
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}

	PGPError
PGPNewKeyDisabledFilter(
	PGPContextRef		context, 
	PGPBoolean			disabled, 
	PGPFilterRef *		outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter	= NULL;
	PGPValidateContext( context );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeyDisabled, kPGPMatchDefault, outFilter);
	if( IsntPGPError( err ) )
	{
		(*outFilter)->value.keyDisabled = disabled;
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewKeySigAlgorithmFilter(
	PGPContextRef			context,
	PGPPublicKeyAlgorithm	sigAlgorithm, 
	PGPFilterRef *			outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter	= NULL;
	PGPValidateContext( context );
	PGPValidateParam( sigAlgorithm >= kPGPPublicKeyAlgorithm_First &&
		sigAlgorithm <= kPGPPublicKeyAlgorithm_Last	);
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeySigAlgorithm, kPGPMatchDefault, outFilter);
	if( IsntPGPError( err ) )
	{
		(*outFilter)->value.keySigAlgorithm = sigAlgorithm;
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewKeyEncryptKeySizeFilter(
	PGPContextRef		context,
	PGPUInt32			keySize, 
	PGPMatchCriterion	match,
	PGPFilterRef *		outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter	= NULL;
	PGPValidateContext( context );
	PGPValidateMatchCriterion( match );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeyEncryptKeySize, match, outFilter);
	if( IsntPGPError( err ) )
	{
		(*outFilter)->value.keyEncryptKeySize = keySize;
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewKeySigKeySizeFilter(
	PGPContextRef		context,
	PGPUInt32			keySize, 
	PGPMatchCriterion	match,
	PGPFilterRef *		outFilter)
{
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter = NULL;
	PGPValidateContext( context );
	PGPValidateMatchCriterion( match );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeKeySigKeySize, match, outFilter);
	if( IsntPGPError( err ) )
	{
		(*outFilter)->value.keySigKeySize = keySize;
	}
	
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}




	PGPError
PGPNewUserIDStringFilter(
	PGPContextRef		context,
	char const *		userIDString, 
	PGPMatchCriterion	match,
	PGPFilterRef *		outFilter)
{
	PGPFilterRef	newFilter;
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter = NULL;
	PGPValidateContext( context );
	PGPValidateMatchCriterion( match );
	
	err = sAllocateFilter( context,  kPGPFilterClassDefault,
			/*	kPGPFilterClassKey | kPGPFilterClassDefault, */
			kPGPFilterTypeUserIDString, match, &newFilter);
	if( IsntPGPError( err ) )
	{
		char *			value;
		
		value = (char *)pgpContextMemAlloc( context,
						strlen( userIDString ) + 1,  0);
		if( IsntNull( value )) 
		{
			newFilter->value.userIDString = value;
			strcpy( value, userIDString);
		}
		else
		{
			PGPFreeFilter( newFilter );
			newFilter	= NULL;
			err	= kPGPError_OutOfMemory;
		}
	}
	
	*outFilter = newFilter;
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}

	PGPError
PGPNewUserIDEmailFilter(
	PGPContextRef		context,
	char const *		eMail, 
	PGPMatchCriterion	match,
	PGPFilterRef *		outFilter)
{
	PGPFilterRef	newFilter;
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter = NULL;
	PGPValidateContext( context );
	PGPValidateMatchCriterion( match );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeUserIDEmail, match, &newFilter);
	if( IsntPGPError( err ) )
	{
		char *			value;

		value = (char *)pgpContextMemAlloc( context, strlen( eMail ) + 1, 0);
		if( IsntNull( value ) )
		{
			strcpy( value, eMail);
			newFilter->value.userIDEmail = value;
		}
		else
		{
			PGPFreeFilter( newFilter );
			newFilter	= NULL;
			err	= kPGPError_OutOfMemory;
		}
	}
	
	*outFilter = newFilter;
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewUserIDNameFilter(
	PGPContextRef		context,
	char const *		name, 
	PGPMatchCriterion	match,
	PGPFilterRef *		outFilter)
{
	PGPFilterRef	newFilter;
	PGPError		err	= kPGPError_NoErr;
	char *			value;

	PGPValidatePtr( outFilter );
	*outFilter = NULL;
	PGPValidateContext( context );
	PGPValidateMatchCriterion( match );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeUserIDName, match, &newFilter);
	if( IsntPGPError( err ) )
	{
		value = (char *)pgpContextMemAlloc( context, strlen( name) + 1, 0);
		if( IsntNull( value ) ) 
		{
			strcpy( value, name);
			newFilter->value.userIDName	= value;
		}
		else
		{
			PGPFreeFilter( newFilter );
			newFilter	= NULL;
			err	= kPGPError_OutOfMemory;
		}
	}
	
	*outFilter	= newFilter;
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}


	PGPError
PGPNewSigKeyIDFilter(
	PGPContextRef		context,
	PGPKeyID const *	keyID,  
	PGPFilterRef *		outFilter)
{
	PGPFilterRef	newFilter;
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( outFilter );
	*outFilter = NULL;
	PGPValidateContext( context );
	
	err = sAllocateFilter( context, kPGPFilterClassDefault,
			kPGPFilterTypeSigKeyID, kPGPMatchDefault, &newFilter);
	if ( IsntPGPError( err ) )
	{
		newFilter->value.sigKeyID	= *keyID;
	}
	
	*outFilter = newFilter;
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}

#if NOT_YET		/* [ */

/*
 * Create a filtered KeySet based on the same KeyDB as the specified
 * original.  The original must be unfiltered.  The filtering type
 * is chosen automatically based on the string.
 *
 * Currently, a "0x" prefix looks up by keyID, otherwise
 * by userid name substring matching.
 */
	PGPError 
PGPNewKeyFilterFromStringQuery(
	PGPContextRef		context,
	char const *		query,
	PGPFilterRef *		outFilter)
{
	PGPError		err	= kPGPError_NoErr;
	PGPFilterRef	filter = kPGPInvalidRef;

	PGPValidatePtr( outFilter );
	PGPValidatePtr( query );
	*outFilter	= NULL;
	PGPValidateContext( context );
	
not yet

	if (query[0] == '0' && query[1] == 'x')
	{
		PGPKeyIDRef		keyID;

		err = PGPNewKeyIDFromString( context, query, &keyID );
		if ( IsntPGPError( err ) )
		{
			err = PGPNewKeyIDFilter( context, keyID, &filter );
			PGPFreeKeyID( keyID );
		}
	}
	else
	{
		err = PGPNewUserIDStringFilter( context, query,
										kPGPMatchSubString,
										&filter );
	}

	*outFilter	= filter;
	pgpAssertErrWithPtr( err, *outFilter );
	return err;
}

#endif	/* ] NOT_YET */

	static PGPBoolean
sComparisonMatchesCriterion(
	long				comparison,
	PGPMatchCriterion	criterion )
{
	switch (criterion)
	{
		case kPGPMatchLessOrEqual:
			return comparison <= 0;
		case kPGPMatchGreaterOrEqual:
			return comparison >= 0;
		case kPGPMatchEqual:
			return comparison == 0;
		default:
			pgpAssert(0);
			break;
	}
	return FALSE;
}

	PGPBoolean
pgpKeyObjMatchesFilter(
	PGPFilterRef		filter,
	RingSet const *		ringSet,
	RingObject *		keyObj)
{
	PGPContextRef		context = NULL;
	long				comparison;
	PGPBoolean			result = FALSE;
	PGPByte				pkAlg;

	if ( ! PGPFilterIsValid( filter ) )
		return( FALSE );
	context = filter->context;
	if ( ! pgpContextIsValid( context ) )
		return( FALSE );

	pgpAssert(ringSet != NULL);
	switch(filter->filterType) 
	{
		case kPGPFilterTypeKeyKeyID:
		{
			PGPKeyID			keyID;
	
			ringKeyID8(ringSet, keyObj, &pkAlg, &keyID);

			result	= pgpKeyIDsEqual( &keyID, &filter->value.keyKeyID );
			break;
		}

		case kPGPFilterTypeKeyFingerPrint:
		{
			PGPSize		length;
			PGPByte		buffer[32];

			ringKeyID8(ringSet, keyObj, &pkAlg, NULL);
			if (pkAlg == kPGPPublicKeyAlgorithm_RSA)
			{
				ringKeyFingerprint16(ringSet, keyObj, buffer);
				length = 16;
			}
			else
			{
				ringKeyFingerprint20(ringSet, keyObj, buffer);
				length = 20;
			}
			if (filter->value.keyFingerPrint.keyFingerPrintLength == length)
			{
				result = !memcmp(buffer,
							filter->value.keyFingerPrint.keyFingerPrintData,
							length);
			}
			break;
		}

		/* Time filters, which utilize shared locals */
		{
			PGPTime		keyTime;
			PGPTime		filterTime;

		case kPGPFilterTypeKeyCreationTime:
			keyTime = ringKeyCreation(ringSet, keyObj);
			filterTime = filter->value.keyCreationTime;
			goto compareTimes;

		case kPGPFilterTypeKeyExpirationTime:
			keyTime = ringKeyExpiration(ringSet, keyObj);
			filterTime = filter->value.keyExpirationTime;

			/* FALL THROUGH */
		compareTimes:
			if (keyTime == filterTime)
				result = TRUE;	/* All match criteria include equality */
			else if (filter->match == kPGPMatchEqual)
				result = FALSE;
			else
			{
				/*
				 * Now we know that the two times are unequal,
				 * and the match criterion is either <= or >=.
				 * So we'll just evaluate <=, and then invert
				 * the result if the criterion was >=.
				 */
				result = (keyTime <= filterTime);
				if (filter->filterType == kPGPFilterTypeKeyExpirationTime)
				{
					if (filterTime == kPGPExpirationTime_Never)
						result = TRUE;
					else if (keyTime == kPGPExpirationTime_Never)
						result = FALSE;
				}

				if (filter->match == kPGPMatchGreaterOrEqual)
					result = !result;
				else
					pgpAssert(filter->match == kPGPMatchLessOrEqual);
			}
			break;
		}

		case kPGPFilterTypeKeyRevoked:
			result = ((!filter->value.keyRevoked)
					  == (!ringKeyRevoked(ringSet, keyObj)));
			break;
		case kPGPFilterTypeKeyDisabled:
			result = ((!filter->value.keyDisabled)
					  == (!ringKeyDisabled(ringSet, keyObj)));
			break;

		case kPGPFilterTypeKeyEncryptAlgorithm:
		{
			RingIterator *	ringIter;
			RingObject *	subKeyObj;

			ringKeyID8(ringSet, keyObj, &pkAlg, NULL);
			if (pkAlg == kPGPPublicKeyAlgorithm_RSA)
			{
				result = (pkAlg == filter->value.keyEncryptAlgorithm);
			}
			else
			{
				ringIter = ringIterCreate(ringSet);
				if (ringIter != NULL)
				{
					ringIterSeekTo(ringIter, keyObj);
					while (ringIterNextObject(ringIter, 2) == 2)
					{
						subKeyObj = ringIterCurrentObject(ringIter, 2);
						if (ringObjectType(subKeyObj) == RINGTYPE_KEY)
						{
							ringKeyID8(ringSet, subKeyObj, &pkAlg, NULL);
							if (pkAlg == filter->value.keyEncryptAlgorithm)
							{
								result = TRUE;
								break;
							}
						}
					}
					ringIterDestroy(ringIter);
				}
			}
			break;
		}
		case kPGPFilterTypeKeySigAlgorithm:
			ringKeyID8(ringSet, keyObj, &pkAlg, NULL);
			result = (pkAlg == filter->value.keyEncryptAlgorithm);
			result = ((!filter->value.keySigAlgorithm)
					  == (!ringKeyRevoked(ringSet, keyObj)));
			break;
		case kPGPFilterTypeKeySubKeyID:
		{
			RingIterator *	ringIter;
			RingObject *	subKeyObj;

			ringIter = ringIterCreate(ringSet);
			if (ringIter != NULL)
			{
				ringIterSeekTo(ringIter, keyObj);
				while (ringIterNextObject(ringIter, 2) == 2)
				{
					subKeyObj = ringIterCurrentObject(ringIter, 2);
					if (ringObjectType(subKeyObj) == RINGTYPE_KEY)
					{
						PGPKeyID	keyID;
						
						ringKeyID8(ringSet, subKeyObj, NULL, &keyID);
						if ( pgpKeyIDsEqual( &keyID,
								&filter->value.keySubKeyID ) )
						{
							result = TRUE;
							break;
						}
					}
				}
				ringIterDestroy(ringIter);
			}
			break;
		}
		case kPGPFilterTypeSigKeyID:
		{
			RingIterator *	ringIter;
			RingObject *	sigObj;

			ringIter = ringIterCreate(ringSet);
			if (ringIter != NULL)
			{
				ringIterSeekTo(ringIter, keyObj);
				while (!result && ringIterNextObject(ringIter, 2) == 2)
				{
					while (ringIterNextObject(ringIter, 3) == 3)
					{
						sigObj = ringIterCurrentObject(ringIter, 3);
						if (ringObjectType(sigObj) == RINGTYPE_SIG)
						{
							PGPKeyID	keyID;
						
							ringSigID8(ringSet, sigObj, NULL, &keyID);
							if ( pgpKeyIDsEqual( &keyID,
									&filter->value.sigKeyID ))
							{
								result = TRUE;
								break;
							}
						}
					}
				}
				ringIterDestroy(ringIter);
			}
			break;
		}
		case kPGPFilterTypeKeyEncryptKeySize:
		{
			RingIterator *	ringIter;
			RingObject *	subKeyObj;

			ringKeyID8(ringSet, keyObj, &pkAlg, NULL);
			if (pkAlg == kPGPPublicKeyAlgorithm_RSA)
			{
				comparison = (ringKeyBits(ringSet, keyObj)
								  - filter->value.keyEncryptKeySize);
				result = sComparisonMatchesCriterion(comparison,
													 filter->match);
			}
			else
			{
				ringIter = ringIterCreate(ringSet);
				if (ringIter != NULL)
				{
					ringIterSeekTo(ringIter, keyObj);
					while (ringIterNextObject(ringIter, 2) == 2)
					{
						subKeyObj = ringIterCurrentObject(ringIter, 2);
						if (ringObjectType(subKeyObj) == RINGTYPE_KEY)
						{
							comparison = (ringKeyBits(ringSet, subKeyObj)
											- filter->value.keyEncryptKeySize);
							if (sComparisonMatchesCriterion(comparison,
															filter->match))
							{
								result = TRUE;
								break;
							}
						}
					}
					ringIterDestroy(ringIter);
				}
			}
			break;
		}
		case kPGPFilterTypeKeySigKeySize:
			comparison = (ringKeyBits(ringSet, keyObj)
							  - filter->value.keySigKeySize);
			result = sComparisonMatchesCriterion(comparison, filter->match);
			break;

		/* UserID filters, which utilize shared locals */
		{
			RingIterator *	ringIter;
			RingObject *	nameObj;
			char const *	nameStr;
			PGPSize			nameLength;
			char *			string;
			PGPSize			stringLength;
			char const *	p;

		case kPGPFilterTypeUserIDEmail:
			string = filter->value.userIDEmail;
			stringLength = strlen(string);
			goto stringFilter;

		case kPGPFilterTypeUserIDName:
			string = filter->value.userIDName;
			stringLength = strlen(string);
			goto stringFilter;
			
		case kPGPFilterTypeUserIDString:
			string = filter->value.userIDString; 
			stringLength = strlen(string);

			/* FALL THROUGH */
		stringFilter:
			ringIter = ringIterCreate(ringSet);
			if (ringIter != NULL)
			{
				ringIterSeekTo(ringIter, keyObj);
				while (ringIterNextObject(ringIter, 2) == 2)
				{
					nameObj = ringIterCurrentObject(ringIter, 2);
					if (ringObjectType(nameObj) == RINGTYPE_NAME)
					{
						nameStr = ringNameName(ringSet, nameObj, &nameLength);
						switch (filter->filterType)
						{
							case kPGPFilterTypeUserIDEmail:
								p = (char *)memchr(nameStr, '<', nameLength);
								if (p == NULL)
								{
									nameLength = 0;
									break;
								}
								p++;
								nameLength -= (p - nameStr);
								nameStr = p;
								p = (char *)memchr(nameStr, '>', nameLength);
								if (p != NULL)
									nameLength = p - nameStr;
								break;
							case kPGPFilterTypeUserIDName:
								p = (char *)memchr(nameStr, '<', nameLength);
								if (p == NULL)
									break;
								while (p > nameStr && p[-1] == ' ')
									p--;
								nameLength = p - nameStr;
								break;
							case kPGPFilterTypeUserIDString:
								break;
							default:
								/* This should never happen */
								pgpAssert(0);
								break;
						}
						if (filter->match == kPGPMatchEqual)
						{
							if (nameLength == stringLength &&
								memcmp(nameStr, string, nameLength) == 0)
							{
								result = TRUE;
								break;
							}
						}
						else if (filter->match == kPGPMatchSubString)
						{
							if (xmemimem(nameStr, nameLength,
										 string, stringLength))
							{
								result = TRUE;
								break;
							}
						}
					}
				}
				ringIterDestroy(ringIter);
			}
			break;
		}

		/* Compound filters */
		case kPGPFilterTypeNot:
			result = !pgpKeyObjMatchesFilter(filter->value.notFilter,
											 ringSet, keyObj);
			break;
		case kPGPFilterTypeAnd:
			result = pgpKeyObjMatchesFilter(filter->value.andFilter.andFilter1,
											ringSet, keyObj)
				  && pgpKeyObjMatchesFilter(filter->value.andFilter.andFilter2,
											ringSet, keyObj);
			break;
		case kPGPFilterTypeOr:
			result = pgpKeyObjMatchesFilter(filter->value.orFilter.orFilter1,
											ringSet, keyObj)
				  || pgpKeyObjMatchesFilter(filter->value.orFilter.orFilter2,
											ringSet, keyObj);
			break;
		default:
			pgpAssertMsg(FALSE, "Unimplemented filter type");
			break;
	}
	return result;
}

	PGPBoolean
pgpKeyMatchesFilter(
	PGPFilterRef		filter,
	PGPKeyRef			key)
{
	return pgpKeyObjMatchesFilter(filter, pgpKeyDBRingSet(key->keyDB),
								  key->key);
}

static PGPError
pgpEncodeSearchTerms( PGPContextRef context,
				char *var,
				char **newString )
{
	static char const badChars[]   = "*()";
	char		*ptrBad;
	char		*ptrGood;
	PGPInt16	i;

	*newString = (char*) pgpContextMemAlloc( context,
								2*strlen(var)+1,
								kPGPMemoryFlags_Clear );
	if ( *newString == NULL )
	{
		return kPGPError_OutOfMemory;
	}

	ptrBad  = var;
	ptrGood = *newString;

	while ( *ptrBad != '\0' ) 
	{
		for (i=0; badChars[i] != '\0'; i++ )
		{
			if ( *ptrBad == badChars[i] )
			{
				*ptrGood++ = '\\';
				*ptrGood++ = *ptrBad++;
				break;
			}
		}
		if (badChars[i] == '\0')
		{
			*ptrGood++ = *ptrBad++;
		}
	}
	*ptrGood = '\0';

	return kPGPError_NoErr;;
}


static PGPError
pgpGrowQueryString(PGPContextRef context,
			char **query, PGPUInt16 *maxsize, PGPUInt16 growthfactor)
{
	PGPError	err;

	if ( strlen(*query) + growthfactor + 1 > *maxsize )
	{
		if (growthfactor < 500)
		{
			growthfactor = 500;
		}
		err = pgpContextMemRealloc( context, (void **)query, 
							*maxsize + growthfactor + 1, 0 );
		if ( IsPGPError(err) )
		{
			return err;
		}
		*maxsize += 500;
	}
	return kPGPError_NoErr;
}

static PGPError
pgpBuildLDAPQuery( 
	PGPFilterRef filter, 
	PGPInt16 filterClass,
	PGPBoolean *disableVisited,
	char **query, 
	PGPUInt16 *maxsize  )
{
	PGPError	err			= kPGPError_NoErr;
	char		*newString  = NULL;
	char		buffer[500];

	switch (filter->filterType)
	{
		case kPGPFilterTypeAnd:
		{
			err = pgpGrowQueryString(filter->context, query, maxsize, 10);
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, "(&" );

			err = pgpBuildLDAPQuery(filter->value.andFilter.andFilter1,
								filterClass, disableVisited, query, maxsize );
			if (err != kPGPError_NoErr)
			{
				return err;
			}
			err = pgpBuildLDAPQuery(filter->value.andFilter.andFilter2,
								filterClass, disableVisited, query, maxsize );
			if (err != kPGPError_NoErr)
			{
				return err;
			}
			err = pgpGrowQueryString(filter->context, query, maxsize, 10);
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}

			strcat( *query, ")" );
			break;
		}

		case kPGPFilterTypeOr:
		{
			err = pgpGrowQueryString(filter->context, query, maxsize, 10);
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat(*query, "(|");

			err = pgpBuildLDAPQuery(filter->value.orFilter.orFilter1,
						filterClass, disableVisited, query, maxsize );
			if (err != kPGPError_NoErr)
			{
				return err;
			}
			err = pgpBuildLDAPQuery(filter->value.orFilter.orFilter2, 
						filterClass, disableVisited, query, maxsize );
			if (err != kPGPError_NoErr)
			{
				return err;
			}

			err = pgpGrowQueryString(filter->context, query, maxsize, 10);
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, ")");
			break;
		}

		case kPGPFilterTypeNot:
		{
			err = pgpGrowQueryString(filter->context, query, maxsize, 10);
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, "(!" );

			err = pgpBuildLDAPQuery(filter->value.notFilter, 
						filterClass, disableVisited, query, maxsize );
			if (err != kPGPError_NoErr)
			{
				return err;
			}

			err = pgpGrowQueryString(filter->context, query, maxsize, 10);
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, ")");
			break;
		}

		case kPGPFilterTypeKeyEncryptAlgorithm:
		{	
			*buffer = '\0';

			switch ( filter->value.keyEncryptAlgorithm )
			{
				case kPGPPublicKeyAlgorithm_RSA:
				case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
				case kPGPPublicKeyAlgorithm_RSASignOnly:
				{	
					strcpy( buffer, "(pgpKeyType=RSA)");
					break;
				}
				case kPGPPublicKeyAlgorithm_ElGamal:
				case kPGPPublicKeyAlgorithm_DSA:
				{
					strcpy( buffer, "(pgpKeyType=DSS/DH)");
					break;
				}

				default:
					return kPGPError_InvalidFilterParameter;
			}
			err = pgpGrowQueryString(filter->context, 
								query, maxsize, (PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;

		}
		case kPGPFilterTypeKeySigAlgorithm:
		{	
			*buffer = '\0';

			switch ( filter->value.keySigAlgorithm )
			{
				case kPGPPublicKeyAlgorithm_RSA:
				case kPGPPublicKeyAlgorithm_RSAEncryptOnly:
				case kPGPPublicKeyAlgorithm_RSASignOnly:
				{	
					strcpy( buffer, "(pgpKeyType=RSA)");
					break;
				}
				case kPGPPublicKeyAlgorithm_ElGamal:
				case kPGPPublicKeyAlgorithm_DSA:
				{
					strcpy( buffer, "(pgpKeyType=DSS/DH)");
					break;
				}

				default:
					return kPGPError_InvalidFilterParameter;
			}
			err = pgpGrowQueryString(filter->context, 
								query, maxsize, 
								(PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;
		}
			
		case kPGPFilterTypeKeyKeyID:
		{
			char		keyIDBuffer[ 128 ];
	
			*buffer = '\0';

			err = PGPGetKeyIDString( 
								&filter->value.keyKeyID,
								kPGPKeyIDString_Full,
								keyIDBuffer );
								
			if (IsPGPError(err))
			{
				return err;
			}
			if ( keyIDBuffer[0] == '0' && tolower(keyIDBuffer[1]) == 'x' ) {
				if (strlen(keyIDBuffer) == 10) {
					sprintf( buffer, "(pgpKeyID=%s)", &keyIDBuffer[2]);
				} 
				else 
				{
					sprintf( buffer, "(pgpCertID=%s)", &keyIDBuffer[2]);
				}
			} 
			else 
			{
				if (strlen(keyIDBuffer) == 8) {
					sprintf( buffer, "(pgpKeyID=%s)", keyIDBuffer);
				}
				else
				{
					sprintf( buffer, "(pgpCertID=%s)", keyIDBuffer);
				}
			}

			err = pgpGrowQueryString(filter->context, 
								query, maxsize, (PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;
		}

		case kPGPFilterTypeKeySubKeyID:
		{	
			char		keyIDBuffer[ 128 ];
			
			*buffer = '\0';

			err = PGPGetKeyIDString( 
								&filter->value.keySubKeyID,
								kPGPKeyIDString_Full,
								keyIDBuffer );
								
			if (IsPGPError(err))
			{
				return err;
			}
			if ( keyIDBuffer[0] == '0' && tolower(keyIDBuffer[1]) == 'x' ) {
				sprintf( buffer, "(pgpSubKeyID=%s)", &keyIDBuffer[2]);
			} 
			else 
			{
				sprintf( buffer, "(pgpSubKeyID=%s)", keyIDBuffer);
			}

			err = pgpGrowQueryString(filter->context, 
								query, maxsize, (PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;
		}

		case kPGPFilterTypeKeyCreationTime:
		{	
			/*
			 * format: YYYYMMDDHHMMSSZ
			 */

			PGPTime tempTime;

			*buffer = '\0';
			tempTime = filter->value.keyCreationTime;

			if (filter->value.keyCreationTime != 0) 
			{
				struct tm	*localTime;
				char		timeString[20];

				localTime = pgpLocalTime(&tempTime);

				if ( localTime == NULL )
				{
					return kPGPError_OutOfMemory;
				}

				sprintf( timeString, "%4d%02d%02d%02d%02d%02dZ",
							1900+localTime->tm_year, localTime->tm_mon+1, 
							localTime->tm_mday, localTime->tm_hour, 
							localTime->tm_min, localTime->tm_sec );

				switch ( filter->match )
				{
					case kPGPMatchEqual:

						sprintf( buffer, 
							"(pgpKeyCreateTime=%s)", timeString );
						break;

					case kPGPMatchGreaterOrEqual:

						sprintf( buffer, 
							"(|(pgpKeyCreateTime>=%s)(pgpKeyCreateTime=%s))", 
							timeString, timeString);
						break;

					case kPGPMatchLessOrEqual:

						sprintf( buffer, 
							"(|(pgpKeyCreateTime<=%s)(pgpKeyCreateTime=%s))", 
							timeString, timeString);
						break;

					default:
						return kPGPError_InvalidFilterParameter;
				}

				err = pgpGrowQueryString(filter->context, 
									query, maxsize, 
									(PGPUInt16) strlen(buffer));
				if ( IsPGPError(err) )
				{
					return kPGPError_OutOfMemory;
				}
			}
			strcat( *query, buffer );
			break;
		}

		case kPGPFilterTypeKeyExpirationTime:
		{	
			/*
			 * format: YYYYMMDDHHMMSSZ
			 */

			PGPTime		tempTime;

			*buffer = '\0';
			tempTime = filter->value.keyExpirationTime;

			if (filter->value.keyExpirationTime != 0) 
			{
				struct tm	*localTime;
				char		timeString[20];

				localTime = pgpLocalTime(&tempTime);

				if ( localTime == NULL )
				{
					return kPGPError_OutOfMemory;
				}

				sprintf( timeString, "%4d%02d%02d%02d%02d%02dZ",
							1900+localTime->tm_year, localTime->tm_mon+1, 
							localTime->tm_mday, localTime->tm_hour, 
							localTime->tm_min, localTime->tm_sec );

				switch ( filter->match )
				{
					case kPGPMatchEqual:

						sprintf( buffer, 
							"(pgpKeyExpireTime=%s)", timeString );
						break;

					case kPGPMatchGreaterOrEqual:

						sprintf( buffer, 
							"(|(pgpKeyExpireTime>=%s)(pgpKeyExpireTime=%s))",
							timeString, timeString);
						break;

					case kPGPMatchLessOrEqual:

						sprintf( buffer, 
							"(|(pgpKeyExpireTime<=%s)(pgpKeyExpireTime=%s))",
							timeString, timeString );
						break;

					default:
						return kPGPError_InvalidFilterParameter;
				}

				err = pgpGrowQueryString(filter->context, 
									query, maxsize, 
									(PGPUInt16) strlen(buffer));
				if ( IsPGPError(err) )
				{
					return kPGPError_OutOfMemory;
				}
			}
			strcat( *query, buffer );
			break;
		}

		case kPGPFilterTypeKeyRevoked:
		{
			sprintf( buffer, "(pgpRevoked=%d)", filter->value.keyRevoked);

			err = pgpGrowQueryString(filter->context, 
								query, maxsize, 
								(PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;
		}

		case kPGPFilterTypeKeyDisabled:
		{
			sprintf( buffer, "(pgpDisabled=%d)", filter->value.keyDisabled);

			err = pgpGrowQueryString(filter->context, 
								query, maxsize, 
								(PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			*disableVisited = TRUE;
			break;
		}

		case kPGPFilterTypeKeyEncryptKeySize:
		{	
			switch ( filter->match )
			{
				case kPGPMatchEqual:

					sprintf( buffer, "(pgpKeySize=%05d)",
						filter->value.keyEncryptKeySize );
					break;

				case kPGPMatchGreaterOrEqual:

					sprintf( buffer, 
						"(|(pgpKeySize>=%05d)(pgpKeySize=%05d))",
						filter->value.keyEncryptKeySize,
						filter->value.keyEncryptKeySize);
					break;

				case kPGPMatchLessOrEqual:

					sprintf( buffer, 
						"(|(pgpKeySize<=%05d)(pgpKeySize=%05d))",
						filter->value.keyEncryptKeySize,
						filter->value.keyEncryptKeySize);
					break;

				default:
					return kPGPError_InvalidFilterParameter;
			}

			err = pgpGrowQueryString(filter->context, 
								query, maxsize, 
								(PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;
		}

		case kPGPFilterTypeUserIDString:
		{	
			err = pgpEncodeSearchTerms( filter->context,
									filter->value.userIDString, &newString );

			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}

			if ( filter->match == kPGPMatchSubString )
			{
				sprintf( buffer, "(pgpUserID=*%s*)", newString );
			} 
			else
			{
				sprintf( buffer, "(pgpUserID=%s)", newString );
			}

			pgpContextMemFree( filter->context, newString);

			err = pgpGrowQueryString(filter->context, 
								query, maxsize, 
								(PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;
		}

		case kPGPFilterTypeUserIDName:
		{	
			err = pgpEncodeSearchTerms( filter->context,
									filter->value.userIDName, &newString );

			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}

			sprintf( buffer, "(pgpUserID=%s)", newString );

			pgpContextMemFree( filter->context, newString);

			err = pgpGrowQueryString(filter->context, 
								query, maxsize, 
								(PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;
		}

		case kPGPFilterTypeUserIDEmail:
		{	
			err = pgpEncodeSearchTerms( filter->context,
									filter->value.userIDEmail,
									&newString);

			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}

			if ( filter->match == kPGPMatchSubString )
			{

				sprintf( buffer, "(pgpUserID=*<*%s*>*)", newString );
			} 
			else 
			{
				/*
				 * assume EXACT email address matching
				 */

				sprintf( buffer, "(pgpUserID=*<%s>*)", newString );
			}

			pgpContextMemFree( filter->context, newString);
			err = pgpGrowQueryString(filter->context, 
								query, maxsize, 
								(PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;
		}

		case kPGPFilterTypeSigKeyID:
		{
			char	keyIDBuffer[ 128 ];
			
			/*
			 * This search supports ONLY the LONG 16 char
			 * keyid search
			 */

			*buffer = '\0';

			err = PGPGetKeyIDString( 
							&filter->value.sigKeyID,
							kPGPKeyIDString_Full,
							keyIDBuffer );
							

			if (err != kPGPError_NoErr)
			{
				return err;
			}

			if ( keyIDBuffer[0] == '0' && tolower(keyIDBuffer[1]) == 'x' ) {
				sprintf( buffer, "(pgpSignerID=%s)", &keyIDBuffer[2]);
			} 
			else 
			{
				sprintf( buffer, "(pgpSignerID=%s)", keyIDBuffer);
			}

			err = pgpGrowQueryString(filter->context, 
								query, maxsize, 
								(PGPUInt16) strlen(buffer));
			if ( IsPGPError(err) )
			{
				return kPGPError_OutOfMemory;
			}
			strcat( *query, buffer );
			break;
		}

		/*
		 * Searches which are not supported by LDAP
		 */

		case kPGPFilterTypeKeyFingerPrint:
		case kPGPFilterTypeKeySigKeySize:
		{
			return kPGPError_UnsupportedLDAPFilter;
		}

		default:
		{
			return kPGPError_UnknownFilterType;
		}
	}

	return(0);

}

PGPError 
PGPLDAPQueryFromFilter( 
	PGPFilterRef	filter,
	char			**queryOut )
{
	PGPUInt16	maxsize	= 1000;
	PGPUInt16	bufLength = 0;
	PGPError	err		= kPGPError_NoErr;
	PGPBoolean	disableVisited = FALSE;
	char		*querybuf;
	
	PGPValidatePtr( queryOut );
	*queryOut	= NULL;
	PGPValidateFilter( filter );

	querybuf = (char*) pgpContextMemAlloc( filter->context, 
								maxsize, kPGPMemoryFlags_Clear );
	if (querybuf == NULL)
	{
		return kPGPError_OutOfMemory;
	}
	*querybuf = '\0';

	err = pgpBuildLDAPQuery(filter, 
						kPGPFilterClassDefault, 
						&disableVisited, &querybuf, &maxsize);

	if ( IsntPGPError(err) )
	{
		/*
		 * allocate a buffer big enough to hold the query and potentially
		 * the extra pgpDisabled condition that is added at the end
		 */

		bufLength = strlen(querybuf);
		*queryOut = (char *)PGPNewData( filter->context, bufLength + 41);

		if ( *queryOut != NULL )
		{
			if (disableVisited == FALSE)
			{
				sprintf(*queryOut, "(&%s(pgpDisabled=0))", querybuf );
			}
			else
			{
				pgpCopyMemory( querybuf, *queryOut, bufLength);
				(*queryOut)[bufLength] = '\0';
			}
		}
		else
		{
			*queryOut = NULL;
			err = kPGPError_OutOfMemory;
		}
	}
	else 
	{
		*queryOut = NULL;
	}
	pgpContextMemFree( filter->context, querybuf );

	pgpAssertErrWithPtr( err, *queryOut );
	return err;
}

static 
PGPError
pgpUrlEncode( PGPContextRef context, char **Dest, char *Source )
{
    PGPError	err = kPGPError_NoErr;
    char		*pSource, *pDest;

    /*Absolute worst case scenario is three times the source size.  Rather
     *than getting too precise, we'll allocate that much initially, and then
     *realloc it down to actuality later.
     */

    pgpAssert( Source );
    pgpAssert( Dest );

    if ( Source && Dest ) 
	{
		*Dest = (char *)pgpContextMemAlloc( context, 
							(strlen(Source) * 3) + 1,
							kPGPMemoryFlags_Clear );
		if ( Dest != NULL )
		{
			pSource = Source;
			pDest = *Dest;
			
			while(pSource && *pSource) 
			{
				/*Zeroth case:  it's an alphabetic or numeric character*/
				if (!isalpha(*pSource) && 
				    !isdigit(*pSource) && 
				    *pSource != '-') 
				{
					/*First case:  Turn spaces into pluses.*/
					if(*pSource == ' ') 
					{
						*pDest = '+';
					}
					else 
					{
						/*This is overkill, but works for our purposes*/
						*pDest = '%';
						++pDest;
						sprintf(pDest, "%02X", *pSource);
						++pDest;
					}
				}
				else
				{
					*pDest = *pSource;
				}
				
				++pDest;
				++pSource;
			}

			*pDest = '\0';
			
		}
		else
		{
			err = kPGPError_OutOfMemory;
		}
	}
    else
	{
		err = kPGPError_BadParams;
	}
    return err;
}


PGPError 
PGPHKSQueryFromFilter( 
	PGPFilterRef	filter,
	char			**queryOut )
{
	PGPError	err				= kPGPError_NoErr;
	char		*encodedValue	= NULL;
	static const char  prefix[]			= "exact=off&search=";

	PGPValidatePtr( queryOut );
	*queryOut	= NULL;
	PGPValidateFilter( filter );

	/*
	 * HKS searches can only do 1 thing at a time
	 */

	switch ( filter->filterType )
	{
		case kPGPFilterTypeKeyKeyID:
		{
			char	keyIDBuffer[ 128 ];
			
			err = PGPGetKeyIDString( 
							&filter->value.sigKeyID,
							kPGPKeyIDString_Abbreviated,
							keyIDBuffer );
							

			if ( IsPGPError(err) )
			{
				return err;
			}
			*queryOut = (char*) PGPNewData( filter->context,
										strlen( keyIDBuffer ) +
										strlen( prefix ) +1 );
			if ( *queryOut == NULL )
			{
				return kPGPError_OutOfMemory;
			}
			sprintf( *queryOut, "%s%s", prefix, keyIDBuffer);
			break;
		}

		case kPGPFilterTypeUserIDString:
		{
			if ( filter->match == kPGPMatchSubString )
			{
				err = pgpUrlEncode( filter->context, &encodedValue, 
							filter->value.userIDString);

				if ( IsPGPError( err ) )
				{
					return kPGPError_UnsupportedHKPFilter;
				}

				*queryOut = (char*) PGPNewData( filter->context,
											strlen( encodedValue )+
											strlen( prefix ) +1 );
				if ( *queryOut == NULL )
				{
					pgpContextMemFree( filter->context, encodedValue );
					return kPGPError_OutOfMemory;
				}
				sprintf( *queryOut, "%s%s",  prefix, 
					encodedValue);

				pgpContextMemFree( filter->context, encodedValue );
			}
			else
			{
				return kPGPError_UnsupportedHKPFilter;
			}
			break;
		}

		case kPGPFilterTypeUserIDEmail:
		{
			if ( filter->match == kPGPMatchSubString )
			{
				err = pgpUrlEncode( filter->context, &encodedValue, 
							filter->value.userIDEmail);

				if ( IsPGPError( err ) )
				{
					return kPGPError_UnsupportedHKPFilter;
				}

				*queryOut = (char*) PGPNewData( filter->context,
											strlen( encodedValue )+
											strlen( prefix ) +1 );
				if ( *queryOut == NULL )
				{
					pgpContextMemFree( filter->context, encodedValue );
					return kPGPError_OutOfMemory;
				}
				sprintf( *queryOut, "%s%s",  prefix, 
					encodedValue);

				pgpContextMemFree( filter->context, encodedValue );
			}
			else
			{
				return kPGPError_UnsupportedHKPFilter;
			}
			break;
		}

		case kPGPFilterTypeUserIDName:
		{
			if ( filter->match == kPGPMatchSubString )
			{
				err = pgpUrlEncode( filter->context, &encodedValue, 
							filter->value.userIDName );

				if ( IsPGPError( err ) )
				{
					return kPGPError_UnsupportedHKPFilter;
				}

				*queryOut = (char*) PGPNewData( filter->context,
											strlen( encodedValue )+
											strlen( prefix ) +1 );
				if ( *queryOut == NULL )
				{
					pgpContextMemFree( filter->context, encodedValue );
					return kPGPError_OutOfMemory;
				}
				sprintf( *queryOut, "%s%s",  prefix, 
					encodedValue);

				pgpContextMemFree( filter->context, encodedValue );
			}
			else
			{
				return kPGPError_UnsupportedHKPFilter;
			}
			break;
		}

		/*
		 * Searches which are not supported by HKP
		 */

		case kPGPFilterTypeNot:
		case kPGPFilterTypeAnd:
		case kPGPFilterTypeOr:
		case kPGPFilterTypeKeySubKeyID:
		case kPGPFilterTypeKeyEncryptAlgorithm:
		case kPGPFilterTypeKeyFingerPrint:
		case kPGPFilterTypeKeyCreationTime:
		case kPGPFilterTypeKeyExpirationTime:
		case kPGPFilterTypeKeyRevoked:
		case kPGPFilterTypeKeyDisabled:
		case kPGPFilterTypeKeySigAlgorithm:
		case kPGPFilterTypeKeyEncryptKeySize:
		case kPGPFilterTypeKeySigKeySize:
		case kPGPFilterTypeSigKeyID:
		{
			return kPGPError_UnsupportedHKPFilter;
		}

		default:
		{
			return kPGPError_UnknownFilterType;
			break;
		}
	}
	
	pgpAssertErrWithPtr( err, *queryOut );
	return err;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
