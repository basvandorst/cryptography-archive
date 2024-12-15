/*____________________________________________________________________________
	pgpHash.c
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpHash.c,v 1.32 1997/10/14 01:48:20 heller Exp $
____________________________________________________________________________*/
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpContext.h"

#include "pgpMD5.h"
#include "pgpSHA.h"
#include "pgpRIPEMD160.h"
#include "pgpSHADouble.h"

#include "pgpHash.h"
#include "pgpHashPriv.h"
#include "pgpUtilitiesPriv.h"



struct PGPHashContext
{
#define kHashContextMagic		0xABBADABA
	PGPUInt32			magic;
	PGPHashVTBL const *	vtbl;
	PGPContextRef		context;
	void *				hashData;
};

struct PGPHashList
{
	PGPUInt32		numHashes;
	PGPContextRef	context;
	PGPHashContext	hashes[ 1 ];	/* open ended */
};

static void	sDisposeHashData( PGPHashContextRef	ref );


#define CallInit(hc)	(hc)->vtbl->init((hc)->hashData)
#define CallUpdate(hc, buf, len) (hc)->vtbl->update((hc)->hashData, buf, len)
#define CallFinal(hc) (hc)->vtbl->final((hc)->hashData)


	static PGPBoolean
pgpHashContextIsValid( PGPConstHashContextRef ref)
{
	return( IsntNull( ref ) &&
			IsntNull( ref->hashData ) &&
			ref->magic == kHashContextMagic  );
}


#define pgpValidateHash( ref )		\
	PGPValidateParam( pgpHashContextIsValid( ref ) )
	
#define IsValidAlgorithm( alg )		\
	IsntNull( pgpHashByNumber( alg ) )


	static PGPError
sHashInit(
	PGPHashContextRef		ref,
	PGPContextRef			context,
	PGPHashAlgorithm		algorithm )
{
	PGPError				err	= kPGPError_NoErr;
	
	pgpClearMemory( ref, sizeof( *ref ) );
	ref->magic		= kHashContextMagic;
	ref->context	= context;
	ref->vtbl		= pgpHashByNumber( algorithm );
	pgpAssert( IsntNull( ref->vtbl ) );
	
	ref->hashData	= pgpContextMemAlloc( context,
		ref->vtbl->context_size, 0);
	if ( IsntNull( ref->hashData ) )
	{
		CallInit( ref );
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}


	static PGPError
sHashCreate(
	PGPContextRef			context,
	PGPHashAlgorithm		algorithm,
	PGPHashContextRef *		outRef )
{
	PGPHashContextRef		ref	= NULL;
	PGPError				err	= kPGPError_NoErr;
	
	*outRef	= NULL;
	
	ref	= (PGPHashContextRef)
		pgpContextMemAlloc( context, sizeof( *ref ),
			0 | kPGPMemoryFlags_Clear );
	
	if ( IsntNull( ref ) )
	{
		err	= sHashInit( ref, context, algorithm );
		
		if ( IsPGPError( err ) )
		{
			pgpContextMemFree( ref->context, ref );
			ref	= NULL;
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	*outRef	= ref;
	return( err );
}




	PGPError 
PGPNewHashContext(
	PGPContextRef		context,
	PGPHashAlgorithm	algorithm,
	PGPHashContextRef *	outRef )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateContext( context );
	PGPValidateParam( IsValidAlgorithm( algorithm ) );
	
	err	= sHashCreate( context, algorithm, outRef );
	
	return( err );
}



	PGPError 
PGPFreeHashContext( PGPHashContextRef ref )
{
	PGPError		err	= kPGPError_NoErr;
	PGPContextRef	cdkContext;
	
	pgpValidateHash( ref );

	cdkContext = ref->context;
	
	sDisposeHashData(ref);
	pgpClearMemory( ref, sizeof( *ref ) );
	pgpContextMemFree( cdkContext, ref );
	
	return( err );
}


	PGPError 
PGPCopyHashContext(
	PGPConstHashContextRef	ref,
	PGPHashContextRef *		outRef)
{
	PGPError			err	= kPGPError_NoErr;
	PGPHashContextRef	newRef	= NULL;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	pgpValidateHash( ref );
	
	err	= sHashCreate( ref->context, ref->vtbl->algorithm, &newRef );
	if ( IsntPGPError( err ) )
	{
		pgpCopyMemory( ref->hashData,
			newRef->hashData, ref->vtbl->context_size );
		
	}
	
	*outRef	= newRef;
	return( err );
}



	PGPError 
PGPResetHash( PGPHashContextRef ref )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateHash( ref );
	
	CallInit( ref );
	
	return( err );
}


	PGPError 
PGPContinueHash(
	PGPHashContextRef	ref,
	const void *		in,
	PGPSize			numBytes )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpValidateHash( ref );
	PGPValidatePtr( in );

	if ( numBytes != 0 )
	{
		CallUpdate( ref, in, numBytes );
	}
	
	return( err );
}


	PGPError 
PGPFinalizeHash(
	PGPHashContextRef	ref,
	void *				hashOut )
{
	PGPError		err	= kPGPError_NoErr;
	const void *	result;
	PGPSize			hashSize;
	
	pgpValidateHash( ref );
	PGPValidatePtr( hashOut );
	
	(void)PGPGetHashSize( ref, &hashSize);
	
	result	= CallFinal( ref );
	pgpCopyMemory( result, hashOut, hashSize );
	
	return( err );
}


	PGPError 
PGPGetHashSize(
	PGPConstHashContextRef	ref,
	PGPSize *				hashSize )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( hashSize );
	*hashSize	= 0;
	pgpValidateHash( ref );
	
	*hashSize	= ref->vtbl->hashsize;
	
	return( err );
}





	PGPHashContextRef
pgpHashCreate(
	PGPContextRef		context,
	PGPHashVTBL const *	vtbl)
{
	PGPError			err	= kPGPError_NoErr;
	PGPHashContextRef	newRef;
	
	pgpAssert( pgpContextIsValid( context ) );
	
	err	= PGPNewHashContext( context, vtbl->algorithm, &newRef );

	pgpAssert( ( IsntPGPError( err ) && IsntNull( newRef ) ) ||
		( IsPGPError( err ) && IsNull( newRef ) ) );
	
	return( newRef );
}




	static void
sDisposeHashData (PGPHashContextRef	ref)
{
	if ( pgpHashContextIsValid( ref ) )
	{
		pgpClearMemory (ref->hashData, ref->vtbl->context_size);
		pgpContextMemFree( ref->context, ref->hashData );
		ref->hashData	= NULL;
	}
}


	void const *
pgpHashFinal( PGPHashContextRef ref )
{
	pgpAssert( pgpHashContextIsValid( ref ) );
	
	return( CallFinal( ref ) );
}


	PGPHashContextRef
pgpHashCopy(PGPConstHashContextRef	ref)
{
	PGPHashContextRef	newRef;

	pgpAssert( pgpHashContextIsValid( ref ) );
	
	(void)PGPCopyHashContext( (PGPHashContextRef)ref, &newRef );
	
	return newRef;
}

	void
pgpHashCopyData(
	PGPHashContext const *	src,
	PGPHashContext *		dest )
{
	pgpAssert( pgpHashContextIsValid( src ) );
	pgpAssert(dest->vtbl == src->vtbl);
	
	pgpCopyMemory( src->hashData, dest->hashData, src->vtbl->context_size);
}


/* Access to all known hashes */
static PGPHashVTBL const * const sHashList[]  =
{
	&HashMD5,
	&HashSHA,
	&HashRIPEMD160,
	&HashSHADouble
};
#define kNumHashes	 ( sizeof( sHashList ) / sizeof( sHashList[ 0 ] ) )

	PGPHashVTBL const *
pgpHashByNumber (PGPHashAlgorithm	algorithm)
{
	const PGPHashVTBL *	vtbl	= NULL;
	PGPUInt32			algIndex;
	
	for( algIndex = 0; algIndex < kNumHashes; ++algIndex )
	{
		if ( sHashList[ algIndex ]->algorithm == algorithm )
		{
			vtbl	= sHashList[ algIndex ];
			break;
		}
	}
	
	return vtbl;
}

/*
 * Given a hash name, return the corresponding hash.
 */
	PGPHashVTBL const *
pgpHashByName (char const *name, size_t namelen)
{
	PGPUInt32	algIndex;

	for( algIndex = 0; algIndex < kNumHashes; ++algIndex )
	{
		PGPHashVTBL const *vtbl;
	
		vtbl = sHashList[ algIndex ];
		
		if ( pgpMemoryEqual (name, vtbl->name, namelen) && 
		    vtbl->name[ namelen ] == '\0')
		{
			return vtbl;
		}
	}
	return NULL;	/* Not found */
}



	PGPHashVTBL const  *
pgpHashGetVTBL( const PGPHashContext *	ref )
{
	pgpAssert( pgpHashContextIsValid( ref ) );
	
	return( ref->vtbl );
}

/*____________________________________________________________________________
	Given a list of hash identifiers, create a list of hash contexts.
	Ignores unknown algorithms.  Returns the number of PgpHashContexts
	created and stored in the "hashes" buffer, or an Error (and none created)
	on error.
	
	Note that the formal data type returned is an opaque 'PGPHashListRef',
	although the actual format of the list is just an array of PGPHashContext.
	The formal data type is used to preserve opacity of the PGPHashContext.
____________________________________________________________________________*/
	PGPError
pgpHashListCreate (
	PGPContextRef		context,
	void const *		bufParam,
	PGPHashListRef *	hashListPtr,
	PGPUInt32			numHashes )
{
	PGPInt32				numHashesCreated;
	PGPHashListRef			hashList;
	PGPError				err	= kPGPError_NoErr;
	PGPUInt32				listSize;
	const PGPByte *			buf;
	
	PGPValidatePtr( hashListPtr );
	*hashListPtr = NULL;
	PGPValidatePtr( bufParam );
	PGPValidateParam( numHashes != 0 );
	pgpValidatePGPContextRef( context );

	buf 		= (const PGPByte *) bufParam;
	listSize	= sizeof( *hashList ) +
		( numHashes -1 )  * sizeof( hashList->hashes[ 0 ] );
		
	hashList	= (PGPHashListRef)
		pgpContextMemAlloc( context, listSize,
		0 | kPGPMemoryFlags_Clear );
	
	if ( IsNull( hashList ) )
		return( kPGPError_OutOfMemory );
		
	pgpClearMemory( hashList, listSize );
	hashList->numHashes	= 0;
	hashList->context	= context;

	numHashesCreated = 0;
	while (numHashes--)
	{
		PGPHashAlgorithm		algorithm;
		PGPHashVTBL const *		vtbl;
		
		algorithm	= (PGPHashAlgorithm) ( *buf++ );
		
		vtbl	= pgpHashByNumber ( algorithm );
		if ( IsntNull( vtbl ) )
		{
			PGPHashContext *	curHash;
			
			curHash	= &hashList->hashes[ numHashesCreated ];
			
			err	= sHashInit( curHash, context, vtbl->algorithm );
			if ( IsPGPError( err ) )
			{
				while ( numHashesCreated-- )
				{
					sDisposeHashData( curHash );
				}
				
				pgpContextMemFree( context, hashList );
				hashList	= NULL;
				err	= kPGPError_OutOfMemory;
				break;
			}
			numHashesCreated++;
		}
	}

	hashList->numHashes	= numHashesCreated;
	
	*hashListPtr = hashList;
	
	return err;
}


	void
pgpHashListDestroy ( PGPHashListRef	hashList )
{
	PGPUInt32		hashIndex;
	
	pgpAssertAddrValid( hashList, PGPHashList );
	
	hashIndex	= hashList->numHashes;
	if ( hashIndex != 0 )
	{
		pgpAssert( pgpContextIsValid( hashList->context ) );
		
		while ( hashIndex--)
		{
			sDisposeHashData( &hashList->hashes[ hashIndex ] );
		}
		
		pgpContextMemFree( hashList->context, hashList );
	}
}


	PGPUInt32
pgpHashListGetSize( PGPHashListRef	list  )
{
	pgpAssertAddrValid( list, PGPHashList );
	return( list->numHashes );
}

/*____________________________________________________________________________
	pgpHashListGetIndHash() is made necessary by incestuous code that wants
	to be able to index over a struct.  Since we want to keep the structure
	of a PGPHashContext opaque, we need to provide this accessor.
____________________________________________________________________________*/

	PGPHashContext *
pgpHashListGetIndHash(
	PGPHashListRef	list,
	PGPUInt32		algIndex )
{
	pgpAssertAddrValid( list, PGPHashList );
	pgpAssert( algIndex < list->numHashes );
	
	if ( algIndex < list->numHashes )
		return( &list->hashes[ algIndex ] );
		
	return( NULL );
}


	PGPHashContext *
pgpHashListFind (
	PGPHashListRef		hashList,
	PGPHashVTBL const *	vtbl)
{
	PGPHashContext *	cur;
	PGPUInt32			remaining;
	
	pgpAssertAddrValid( hashList, PGPHashList );
	
	cur	= &hashList->hashes[ 0 ];
	remaining	= hashList->numHashes;
	while (remaining--)
	{
		if ( cur->vtbl == vtbl )
			return cur;
		cur++;
	}
	return NULL;
}






















/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
