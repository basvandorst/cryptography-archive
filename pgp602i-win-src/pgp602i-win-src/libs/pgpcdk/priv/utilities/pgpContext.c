/*____________________________________________________________________________
	pgpContext.c
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.

	$Id: pgpContext.c,v 1.74.10.1 1998/11/12 03:23:04 heller Exp $
____________________________________________________________________________*/
#include <stdio.h>

#include "pgpErrors.h"
#include "pgpMem.h"

#include "pgpContext.h"
#include "pgpEnv.h"
#include "pgpRandomPoolPriv.h"
#include "pgpRandomContext.h"
#include "pgpRandomX9_17.h"
#include "pgpRnd.h"
#include "pgpRndSeed.h"
#include "pgpRngPub.h"
#include "pgpDEBUGStartup.h"
#include "pflPrefs.h"
#include "pgpUtilitiesPriv.h"
#include "pgpMacFileMapping.h"
#include "bn.h"
#include "pgpMemoryMgrPriv.h"
#include "pgpTimeBomb.h"


enum
{
	kPGPContextMagic	= 0x436e7478	/* 'Cntx' */
};

struct PGPContext
{
	PGPMemoryMgrRef		memoryMgr;
	
	PGPUInt32		magic;				/* Always kPGPContextMagic */
	PGPUInt32		clientAPIVersion;	/* version client compiled against */
	PGPUserValue	userValue;			/* Client storage */
	PGPKeySetRef	defaultKeySet;		/* Set by pgpOpenDefualtKeyRings() */
	
	PGPEnv		   *	pgpEnvironment;		/* State for within library */
	
	PGPRandomContext *	randomPoolContext;	/* ctxt around global rand pool*/
	PGPRandomContext *	randomPoolX9_17;	/* uses 'randomPoolContext' */
	PGPRandomContext *	dummyRandomContext;
	
	PGPBoolean			prefsChanged;		/* Have changed internal prefs */
	PGPPrefRef			prefsRef;
	
	RingPool	   *	ringPool;			/* Keyring data structures */
	
};

typedef struct PGPContext	PGPContext;



/*____________________________________________________________________________
	This function validates a context data structure.
____________________________________________________________________________*/

	PGPBoolean
pgpContextIsValid(PGPContextRef context)
{
	return( context != NULL &&
			context->magic == kPGPContextMagic );
}


/*____________________________________________________________________________
	Allocate a new PGPContext using custom memory allocators.
	This function es exported for the client library and should never be
	called by the shared library.
____________________________________________________________________________*/
	static PGPError
FinishInitContext( PGPContextRef  context )
{
	PGPError	err = kPGPError_NoErr;
		
	err = pgpenvCreate( context, &context->pgpEnvironment );
	if ( IsntPGPError( err ) )
	{
		context->ringPool = ringPoolCreate( context->pgpEnvironment );
		if ( IsNull( context->ringPool ) )
		{
			err	= kPGPError_OutOfMemory;
		}
	}
	
	/* only actually seeds it the first time */
	(void)pgpSeedGlobalRandomPool( context );
	
	return( err );
}

/*____________________________________________________________________________
	Allocate a new PGPContext using custom memory allocators.
	This function es exported for the client library and should never be
	called by the shared library.
____________________________________________________________________________*/

	PGPError
pgpNewContextCustomInternal(
	PGPUInt32					clientAPIVersion,
	PGPNewContextStruct const *	custom,
	PGPContextRef*				newContext)
{
	PGPError			err = kPGPError_NoErr;
	PGPContextRef		context	= NULL;

	PGPValidatePtr( newContext );
	*newContext	= NULL;
	PGPValidatePtr( custom );
	/*	Note: if the struct grows, then
		sizeof( PGPNewContextStruct ) will need to be a fixed number */ 
	PGPValidateParam( custom->sizeofStruct >= sizeof( PGPNewContextStruct ) );
	PGPValidateParam( PGPMemoryMgrIsValid( custom->memoryMgr ) );
	
	if ( ! pgpsdkIsInited() )
	{
		return( kPGPError_ImproperInitialization );
	}

	if ( PGPMajorVersion( clientAPIVersion ) >
			PGPMajorVersion( kPGPsdkAPIVersion ) ||
			PGPMajorVersion( clientAPIVersion ) == 0  )
	{
		return( kPGPError_IncompatibleAPI );
	}
	
#if PGP_TIME_BOMB
	if ( pgpTimeBombHasExpired() )
	{
		*newContext	= NULL;
		return( kPGPError_FeatureNotAvailable );
	}
#endif

	PGPDebugStartup();

	context	= (PGPContext *)PGPNewData( custom->memoryMgr,
				sizeof( PGPContext ), kPGPMemoryMgrFlags_Clear );
	
	if( IsntPGPError( err ) )
	{
		context->memoryMgr			= custom->memoryMgr;
		context->magic				= kPGPContextMagic;
		context->clientAPIVersion	= clientAPIVersion;
		context->defaultKeySet		= kInvalidPGPKeySetRef;
		
		err	= FinishInitContext( context );

		if( IsPGPError( err ) )
		{
			PGPFreeContext( context );
			context = kInvalidPGPContextRef;
		}
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}
	
	*newContext	= context;
	
	return( err );
}

/*____________________________________________________________________________
	Allocate a new PGPContext using the default PGPsdk memory allocators.
	This function es exported for the client library and should never be
	called by the shared library.
____________________________________________________________________________*/

	PGPError
pgpNewContextInternal(
	PGPUInt32 		clientAPIVersion,
	PGPContextRef *	newContext)
{
	PGPNewContextStruct		custom;
	PGPError				err	= kPGPError_NoErr;
	
	pgpAssert( IsntNull( newContext ) );
	*newContext	= NULL;
	
	pgpClearMemory( &custom, sizeof( custom ) );
	custom.sizeofStruct	= sizeof( custom );
	err	= PGPNewMemoryMgr( 0, &custom.memoryMgr );
	if ( IsntPGPError( err ) )
	{
		err	= pgpNewContextCustomInternal( clientAPIVersion,
				&custom, newContext);
	}
	
	return( err );
}




/*____________________________________________________________________________
	Allocate a new PGPContext using the default PGPsdk memory allocators.
____________________________________________________________________________*/

	PGPError 
PGPNewContext(
	PGPUInt32		clientAPIVersion,
	PGPContextRef *	newContext)
{
	PGPValidatePtr( newContext );
	*newContext	= NULL;
	
	return( pgpNewContextInternal( clientAPIVersion, newContext ) );
}


/*____________________________________________________________________________
	Allocate a new custom PGPContext.
____________________________________________________________________________*/
	PGPError
PGPNewContextCustom(
	PGPUInt32						sdkVersion,
	PGPNewContextStruct const *		custom,
	PGPContextRef 					*newContext)
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( newContext );
	*newContext	= kInvalidPGPContextRef;
	PGPValidatePtr( custom );
	
	err	= pgpNewContextCustomInternal( sdkVersion, custom, newContext );
	
	return( err );
}


	static void
sDestroyContext( PGPContextRef	context )
{
	if ( IsntNull( context->ringPool ) )
	{
		ringPoolDestroy ( context->ringPool );
		context->ringPool	= NULL;
	}
		
	if( IsntNull( context->pgpEnvironment ) )
	{
		pgpenvDestroy( context->pgpEnvironment );
		context->pgpEnvironment	= NULL;
	}
	
	if ( IsntNull( context->randomPoolContext ) )
	{
		pgpRandomDestroy( context->randomPoolContext );
		context->randomPoolContext	= NULL;
	}
	
	if ( IsntNull( context->randomPoolX9_17 ) )
	{
		pgpRandomDestroy( context->randomPoolX9_17 );
		context->randomPoolX9_17	= NULL;
	}
	
	if ( IsntNull( context->dummyRandomContext ) )
	{
		pgpRandomDestroy( context->dummyRandomContext );
		context->dummyRandomContext	= NULL;
	}
	
	if ( IsntNull( context->prefsRef ) )
	{
		PGPClosePrefFile( context->prefsRef );
	}
}

/*____________________________________________________________________________
	Delete an existing PGPContext and all resources associated with it.
____________________________________________________________________________*/

	PGPError 
PGPFreeContext(PGPContextRef context)
{
	PGPError		err = kPGPError_NoErr;
	PGPMemoryMgrRef	mgr;
	
	PGPValidateContext( context );
	
	/* Get current time into rand pool */
	PGPGlobalRandomPoolAddKeystroke( PGPGetTime() );
	err	= pgpSaveGlobalRandomPool( context );
	
	sDestroyContext( context );
	
	mgr	= context->memoryMgr;
	PGPFreeData( context );
	PGPFreeMemoryMgr( mgr );
	
	return( err );
}



/*____________________________________________________________________________
	Allocate a block of memory using the allocator stored in a PGPContext.
____________________________________________________________________________*/

	void *
pgpContextMemAlloc(
	PGPContextRef 		context,
	PGPSize 			requestSize,
	PGPMemoryMgrFlags	flags)
{
	void	*	allocation = NULL;
	PGPSize		allocationSize	= requestSize;
	
	pgpAssert( pgpContextIsValid( context ) );
	
	allocation = PGPNewData( context->memoryMgr,
					allocationSize, flags );
	
	return( allocation );
}

/*____________________________________________________________________________
	Same as pgpContextMemAlloc(), but copies data into newly allocated block.
____________________________________________________________________________*/

	void *
PGPContextMemAllocCopy(
	PGPContextRef 		context,
	PGPSize 			requestSize,
	PGPMemoryMgrFlags	flags,
	const void *		dataToCopy )
{
	void *	allocation;
	
	pgpAssert( pgpContextIsValid( context ) );
	
	allocation = PGPNewData( context->memoryMgr,
					requestSize, flags & ~kPGPMemoryMgrFlags_Clear );
	if ( IsntNull( allocation ) )
	{
		pgpCopyMemory( dataToCopy, allocation, requestSize );
	}
	
	return( allocation );
}


/*____________________________________________________________________________
	Allocate a block of memory using the allocator stored in a PGPContext.
____________________________________________________________________________*/

	PGPError
pgpContextMemRealloc(
	PGPContextRef 		context,
	void				**allocation,
	PGPSize 			requestSize,
	PGPMemoryMgrFlags	flags)
{
	PGPError	err	= kPGPError_NoErr;
	
	err = PGPReallocData( context->memoryMgr,
			allocation, requestSize, flags );
	return( err );
}

/*____________________________________________________________________________
	Free a block of memory using the deallocator stored in a PGPContext.
____________________________________________________________________________*/

	PGPError
pgpContextMemFree(
	PGPContextRef 	context,
	void * 			allocation)
{
	PGPError	err;
	
	PGPValidateContext( context );
	(void)context;
	
	err = PGPFreeData( allocation );
	
	return( err );
}
	
	PGPError 
PGPGetContextUserValue(
	PGPContextRef 	context,
	PGPUserValue 	*userValue)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidatePtr( userValue );
	*userValue	= NULL;
	PGPValidateContext( context );
	
	*userValue = context->userValue;
	
	return( err );
}


	PGPMemoryMgrRef
PGPGetContextMemoryMgr( PGPContextRef context )
{
	if ( ! pgpContextIsValid( context ) )
		return( NULL );
		
	return( context->memoryMgr );
}


	PGPError 
PGPSetContextUserValue(
	PGPContextRef 	context,
	PGPUserValue 	userValue)
{
	PGPError	err = kPGPError_NoErr;
	
	PGPValidateContext( context );
	
	context->userValue = userValue;
	
	return( err );
}

	PGPEnv *
pgpContextGetEnvironment(PGPContextRef context)
{
	pgpAssert( pgpContextIsValid( context ) );

	return( context->pgpEnvironment );
}


	static PGPError
sCreateRandomContext( 
	PGPContextRef			context,
	PGPRandomVTBL const *	vtbl,
	PGPRandomContext **		outRandom )
{
	PGPError			err	= kPGPError_NoErr;
	PGPRandomContext *	newRandom	= NULL;
	
	pgpAssert( pgpContextIsValid( context ) );
	
		
	newRandom	= (PGPRandomContext *)
		pgpContextMemAlloc( context, sizeof( *newRandom ),
		kPGPMemoryMgrFlags_Clear );
	if ( IsntNull( newRandom ) )
	{
		newRandom->context	= context;
		newRandom->vtbl		= vtbl;
		newRandom->priv		= NULL;
		newRandom->destroy	= NULL;
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}

	*outRandom	= newRandom;
	return( err );
}



	PGPRandomContext *
pgpContextGetGlobalPoolRandomContext(PGPContextRef context)
{
	pgpAssert( pgpContextIsValid( context ) );
	
	if ( IsNull( context->randomPoolContext ) )
	{
		PGPRandomContext *	randomContext	= NULL;
		
		(void)sCreateRandomContext( context,
			pgpGetGlobalRandomPoolVTBL(), &randomContext );
		context->randomPoolContext	= randomContext;
		
		if ( IsntNull( context->randomPoolContext ) )
		{
			/*
			* in case there is a problem in reading the random pool,
			* feed in some amount of "random" data to avoid a totally
			* predictible pool
			*/
			pgpRandomCollectEntropy( randomContext );
			pgpRandomCollectEntropy( randomContext );
			pgpRandomCollectEntropy( randomContext );
			pgpRandomCollectEntropy( randomContext );
		}
	}
	/* NOTE: someone must still call pgpSeedGlobalRandomPool() */

	return( context->randomPoolContext );
}



	PGPRandomContext *
pgpContextGetDummyRandomContext(PGPContextRef context)
{
	pgpAssert( pgpContextIsValid( context ) );
	
	if ( IsNull( context->dummyRandomContext ) )
	{
		(void)sCreateRandomContext( context,
			pgpGetGlobalDummyRandomPoolVTBL(),
			&context->dummyRandomContext );
	}

	return( context->dummyRandomContext );
}



	PGPRandomContext *
pgpContextGetX9_17RandomContext(PGPContextRef context)
{
	pgpAssert( pgpContextIsValid( context ) );
	
	if ( IsNull( context->randomPoolX9_17 ) )
	{
		context->randomPoolX9_17	= pgpRandomCreate( context );
	}

	return( context->randomPoolX9_17 );
}


/*____________________________________________________________________________
    Fill the user-specified buffer with random values from the global
    random pool.  If we were not able to initialize the random pool
    we return kPGPError_OutOfEntropy.
____________________________________________________________________________*/
	PGPError
PGPContextGetRandomBytes(
	PGPContextRef	context,
	void *			buf,
	PGPSize			len
	)
{
	PGPRandomContext *	randomContext;

	PGPValidatePtr( buf );
	PGPValidateParam( pgpContextIsValid( context ) );

	randomContext = pgpContextGetX9_17RandomContext( context );
	if( IsNull( randomContext )
		|| !PGPGlobalRandomPoolHasMinimumEntropy() )
	{
		return kPGPError_OutOfEntropy;
	}

	pgpRandomGetBytes( randomContext, buf, len );
	
	return kPGPError_NoErr;
}


	RingPool *
pgpContextGetRingPool(PGPContextRef context)
{
	pgpAssert( pgpContextIsValid( context ) );

	return( context->ringPool );
}

	void
pgpContextSetRingPool(
	PGPContextRef 		context,
	RingPool			*ringPool)
{
	pgpAssert( pgpContextIsValid( context ) );

	context->ringPool = ringPool;
}


	PGPKeySetRef
pgpContextGetDefaultKeySet(PGPContextRef context)
{
	pgpAssert( pgpContextIsValid( context ) );

	return( context->defaultKeySet );
}

/* We may retire the prefs stuff, but this is for transition at least */
	void
pgpContextSetPrefsChanged(
	PGPContextRef	context,
	PGPBoolean		prefsChanged
	)
{
	context->prefsChanged = prefsChanged;
}

	PGPBoolean
pgpContextArePrefsChanged(
	PGPContextRef	context
	)
{
	return context->prefsChanged;
}


	PGPPrefRef
pgpContextGetPrefs( PGPContextRef	context)
{
	return( context->prefsRef );
}

	void
pgpContextSetPrefs(
	PGPContextRef	context,
	PGPPrefRef		prefs )
{
	context->prefsRef = prefs;
}


static PGPUInt32	sInitedCount		= 0;
static PGPBoolean	sFreeMemoryMgrList	= FALSE;

	PGPError
PGPsdkInit( void )
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( sInitedCount == 0 )
	{
		pgpLeaksBeginSession( "PGPsdk" );
		
		if( IsNull( PGPGetDefaultMemoryMgr() ) )
		{
			PGPMemoryMgrRef	memoryMgr;
			
			sFreeMemoryMgrList = TRUE;
			
			err = PGPNewMemoryMgr( 0, &memoryMgr );
			pgpAssertNoErr( err );
			if( IsntPGPError( err ) )
			{
				err = PGPSetDefaultMemoryMgr( memoryMgr );
				pgpAssertNoErr( err );
			}
		}
		
		if( IsntPGPError( err ) )
		{
			err	= pgpInitMacBinaryMappings();
			pgpAssertNoErr( err );
		}
		
		if( IsntPGPError( err ) )
		{
			bnInit();
		}
	}
	
	if ( IsntPGPError( err ) )
	{
		++sInitedCount;
	}
	
	return( err );
}


	PGPBoolean
pgpsdkIsInited( void )
{
	return( sInitedCount != 0 );
}

	PGPError
pgpsdkCleanupForce( void )
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( sInitedCount != 0 )
	{
		sInitedCount	= 1;
	}
	
	err	= PGPsdkCleanup();
	return( err );
}

	PGPError
PGPsdkCleanup( void )
{
	PGPError	err	= kPGPError_NoErr;
	
	pgpAssert( sInitedCount != 0 );
	if ( sInitedCount != 0 )
	{
		--sInitedCount;
		if ( sInitedCount == 0 )
		{
			pgpDisposeMacBinaryMappings();
			
			if( sFreeMemoryMgrList )
				pgpFreeDefaultMemoryMgrList();
				
			pgpLeaksEndSession();
		}
	}
	else
	{
		err	= kPGPError_BadParams;
	}
	
	return( err );
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
