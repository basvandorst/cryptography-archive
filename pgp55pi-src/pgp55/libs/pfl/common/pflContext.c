/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.

	$Id: pflContext.c,v 1.15 1997/09/18 01:34:09 lloyd Exp $
____________________________________________________________________________*/
#include <stdio.h>

#include "pgpPFLErrors.h"
#include "pgpMem.h"

#include "pflContextPriv.h"




/*____________________________________________________________________________
	This is the default PFL memory allocator. Note that context will be
	NULL when allocating the PFLContext structure itself.
____________________________________________________________________________*/

	static void *
pflDefaultMemoryAllocationProc(
	PFLContextRef	context,
	PGPSize 		allocationSize,
	PFLMemoryFlags	flags,
	PGPUserValue 	userValue)
{
	(void) context;
	(void) userValue;
	(void) flags;

	return( pgpAlloc( allocationSize ) );
}

/*____________________________________________________________________________
	This is the default PFL memory reallocator. 
____________________________________________________________________________*/

	static PGPError
pflDefaultMemoryReallocationProc(
	PFLContextRef	context,
	void			**allocation,
	PGPSize 		newAllocationSize,
	PFLMemoryFlags	flags,
	PGPUserValue 	userValue)
{
	PGPError	err;
	
	(void) context;
	(void) userValue;
	(void) flags;

	err = pgpRealloc( allocation, newAllocationSize );
	
	return( err );
}

/*____________________________________________________________________________
	This is the default PFL memory deallocator. Note that context will
	be NULL when deallocating the PFLContext structure itself.
____________________________________________________________________________*/

	static PGPError
pflDefaultMemoryDeallocationProc(
	PFLContextRef	context,
	void *			allocation,
	PGPUserValue 	userValue)
{
	(void) context;
	(void) userValue;

	pgpFree( allocation );
	
	return( kPGPError_NoErr );
}

/*____________________________________________________________________________
	This function validates a context data structure.
____________________________________________________________________________*/

	PGPBoolean
PFLContextIsValid(PFLContextRef context)
{
	return( context != NULL &&
			context->magic == kPFLContextMagic );
}



/*____________________________________________________________________________
	Allocate a new PFLContext using custom memory allocators.
	This function es exported for the client library and should never be
	called by the shared library.
____________________________________________________________________________*/

	PGPError
PFLNewContextCustom(
	PGPSize							contextSize,
	PFLNewContextStruct const *		custom,
	PFLContextRef 					*newContext)
{
	PGPError		err = kPGPError_NoErr;
	PFLContextRef	context	= NULL;
	
	PGPValidatePtr( newContext );
	*newContext	= NULL;
	PGPValidatePtr( custom );
	PGPValidatePtr( custom->allocProc );
	PGPValidatePtr( custom->reallocProc );
	PGPValidatePtr( custom->deallocProc );
	
	if ( contextSize == kDefaultContextSize )
		contextSize	= sizeof( PFLContext );
	PGPValidateParam( contextSize >= sizeof( PFLContext ) );

	/* NOTE: do not depend on kPFLMemoryFlags_Clear here;
	 allocator is not required to support it */
	context = (PFLContext *) (*(custom->allocProc))( NULL,
				contextSize, 0, custom->allocUserValue );
	if( IsntNull( context ) )
	{
		pgpClearMemory( context, contextSize );
		context->magic				= kPFLContextMagic;
		context->allocProc		= custom->allocProc;
		context->reallocProc	= custom->reallocProc;
		context->deallocProc	= custom->deallocProc;
		context->allocUserValue	= custom->allocUserValue;
	}
	else
	{
		err = kPGPError_OutOfMemory;
	}
	
	*newContext	= context;
	
	return( err );
}


/*____________________________________________________________________________
____________________________________________________________________________*/

	PGPError
PFLNewContext( PFLContextRef *	newContext )
{
	PFLNewContextStruct		custom;
	
	pgpClearMemory( &custom, sizeof( custom ) );
	
	custom.allocProc		= pflDefaultMemoryAllocationProc;
	custom.reallocProc		= pflDefaultMemoryReallocationProc;
	custom.deallocProc		= pflDefaultMemoryDeallocationProc;
	custom.allocUserValue	= NULL;
	
	return( PFLNewContextCustom( kDefaultContextSize, &custom, newContext ) );
}



/*____________________________________________________________________________
	Delete an existing PFLContext and all resources associated with it.
____________________________________________________________________________*/
	PGPError 
PFLFreeContext(PFLContextRef context)
{
	PGPError	err = kPGPError_NoErr;
	
	pgpAssert( PFLContextIsValid( context ) );
	
	if( PFLContextIsValid( context ) )
	{
		(*context->deallocProc)( NULL,
			context, context->allocUserValue );
	}
	else
	{
		err = kPGPError_BadParams;
	}
	
	return( err );
}


	PGPError 
PFLGetContextUserValue(
	PFLContextRef 	context,
	PGPUserValue 	*userValue)
{
	PGPError	err = kPGPError_NoErr;
	
	pgpAssert( PFLContextIsValid( context ) );
	pgpAssertAddrValid( userValue, PGPUserValue );
	
	if( PFLContextIsValid( context ) &&
		IsntNull( userValue ) )
	{
		*userValue = context->userValue;
	}
	else
	{
		err = kPGPError_BadParams;
	}
	
	return( err );
}

	PGPError 
PFLSetContextUserValue(
	PFLContextRef 	context,
	PGPUserValue 	userValue)
{
	PGPError	err = kPGPError_NoErr;
	
	pgpAssert( PFLContextIsValid( context ) );
	
	if( PFLContextIsValid( context ) )
	{
		context->userValue = userValue;
	}
	else
	{
		err = kPGPError_BadParams;
	}
	
	return( err );
}


#ifndef PFL_CONTEXT_USE_HEADER
#define PFL_CONTEXT_USE_HEADER		PGP_DEBUG
#endif

#if PFL_CONTEXT_USE_HEADER	/* [ */

/* when debugging, we add a header to verify the legitimacy of the block */
typedef struct ContextMemAllocHeader
{
	#define kContextMemAllocMagic	((PGPUInt32)0xFEEDDEAF)	
	PGPUInt32			magic;
	PFLContextRef	context;
} ContextMemAllocHeader;
#define HeaderToUserBlock( hdr )	\
	( (void *)(((char *)hdr) + sizeof( ContextMemAllocHeader )) )
#define UserBlockToHeader( user )	\
	( (ContextMemAllocHeader *)\
		(((char *)user) - sizeof( ContextMemAllocHeader )) )

#define AssertBlockValid( block ) \
	{\
	ContextMemAllocHeader *	header_	= UserBlockToHeader( block );\
	pgpAssert( header_->magic == kContextMemAllocMagic );\
	}

#else 	/* ] PFL_CONTEXT_USE_HEADER [ */

#define AssertBlockValid( block ) /* nothing */

#endif	/* ] PFL_CONTEXT_USE_HEADER */



/*____________________________________________________________________________
	Allocate a block of memory using the allocator stored in a PFLContext.
____________________________________________________________________________*/

	void *
PFLContextMemAlloc(
	PFLContextRef 	context,
	PGPSize 		requestSize,
	PFLMemoryFlags	flags)
{
	void	*allocation		= NULL;
	PGPSize	allocationSize	= requestSize;
	
#if PFL_CONTEXT_USE_HEADER
	allocationSize	+= sizeof( ContextMemAllocHeader );
#endif
	
	pgpAssert( PFLContextIsValid( context ) );
	
	if( PFLContextIsValid( context ) )
	{
		allocation = (*context->allocProc)( context,
						allocationSize, flags,
						context->allocUserValue );
	}
	
	if ( IsntNull( allocation ) && (flags & kPFLMemoryFlags_Clear ) != 0 )
	{
		pgpClearMemory( allocation, allocationSize );
	}
	
#if PFL_CONTEXT_USE_HEADER
	if ( IsntNull( allocation ) )
	{
		ContextMemAllocHeader *	header	= NULL;
	
		header	= (ContextMemAllocHeader *)allocation;
		header->magic	= kContextMemAllocMagic;
		header->context	= context;
		allocation		= HeaderToUserBlock( allocation );
		AssertBlockValid( allocation );
	}
#endif

	return( allocation );
}

	void *
PFLContextMemAllocCopy(
	PFLContextRef 	context,
	PGPSize 		requestSize,
	PFLMemoryFlags	flags,
	const void *	dataToCopy )
{
	void * result	= NULL;
	
	/* note that clear flag is unneeded since we'll copy all the data */
	result	= PFLContextMemAlloc( context, requestSize,
				flags & ~kPFLMemoryFlags_Clear );
	if ( IsntNull( result ) )
	{
		pgpCopyMemory( dataToCopy, result, requestSize );
	}
	return( result );
}


/*____________________________________________________________________________
	Allocate a block of memory using the allocator stored in a PFLContext.
____________________________________________________________________________*/
	PGPError
PFLContextMemRealloc(
	PFLContextRef 	context,
	void			**allocation,
	PGPSize 		requestSize,
	PFLMemoryFlags	flags)
{
	PGPError		err			= kPGPError_NoErr;
	
	PFLValidateContext( context );
	PGPValidatePtr( allocation );
	/* can't support clear because we don't know the size of the original */
	PGPValidateParam( ( flags & kPFLMemoryFlags_Clear) == 0 );

	/* allow a NULL input */
	if ( IsntNull( *allocation ) )
	{
#if PFL_CONTEXT_USE_HEADER /* [ */
		void *		temp	= UserBlockToHeader( *allocation );
		
		AssertBlockValid( *allocation );
		err = (*(context->reallocProc))( context,
					&temp,
					sizeof( ContextMemAllocHeader ) + requestSize,
					flags, context->allocUserValue );
		if ( IsntPGPError( err ) )
		{
			ContextMemAllocHeader *header;
			
			header	= (ContextMemAllocHeader *)temp;
			*allocation	= HeaderToUserBlock( temp );
			AssertBlockValid( *allocation );
		}
	#else  /* ] PFL_CONTEXT_USE_HEADER [ */
		err = (*(context->reallocProc))( context,
					allocation, requestSize, flags,
					context->allocUserValue );
	#endif  /* ] PFL_CONTEXT_USE_HEADER */
	}
	else
	{
		/* allocate a brand-new block */
		*allocation	= PFLContextMemAlloc( context, requestSize, flags );
		if ( IsNull( *allocation ) )
			err	= kPGPError_OutOfMemory;
	}
	
	return( err );
}

/*____________________________________________________________________________
	Free a block of memory using the deallocator stored in a PFLContext.
____________________________________________________________________________*/

	PGPError
PFLContextMemFree(
	PFLContextRef 	context,
	void * 			allocation)
{
	PGPError	err	= kPGPError_NoErr;
	
	PFLValidateContext( context );
	PGPValidatePtr( allocation );
#if PFL_CONTEXT_USE_HEADER
	PGPValidateParam( UserBlockToHeader( allocation )->magic ==
		kContextMemAllocMagic );
#endif
	
	if( PFLContextIsValid( context ) )
	{
	#if PFL_CONTEXT_USE_HEADER
		err	= (*(context->deallocProc))(
			context, UserBlockToHeader( allocation ),
			context->allocUserValue );
	#else
		err	= (*(context->deallocProc))(
			context, allocation, context->allocUserValue );
	#endif
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
