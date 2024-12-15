/*____________________________________________________________________________
	PFLContext.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Contains the definition of the PFLContext data structure and
	prototypes for creating and using it.

	$Id: pflContext.h,v 1.13 1997/09/11 05:38:26 lloyd Exp $
____________________________________________________________________________*/

#ifndef Included_PFLContext_h	/* [ */
#define Included_PFLContext_h


#include "pgpBase.h"
#include "pflTypes.h"


#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif


enum 
{
	kPFLMemoryFlags_Clear = 1
};

typedef PGPFlags	PFLMemoryFlags;

typedef	void	*(*PFLMemoryAllocationProc)( PFLContextRef context,
						PGPSize allocationSize, PFLMemoryFlags flags,
						PGPUserValue userValue );

typedef	PGPError (*PFLMemoryReallocationProc)( PFLContextRef context,
						void **allocation, PGPSize newAllocationSize,
						PFLMemoryFlags flags, PGPUserValue userValue );

typedef	PGPError (*PFLMemoryDeallocationProc)( PFLContextRef context,
						void *allocation, PGPUserValue userValue );

typedef struct PFLNewContextStruct
{
	PFLMemoryAllocationProc		allocProc;
	PFLMemoryReallocationProc	reallocProc;
	PFLMemoryDeallocationProc	deallocProc;
	PGPUserValue					allocUserValue;
	PGPUInt32						pad[ 4 ];
} PFLNewContextStruct;

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/*____________________________________________________________________________
	Context routines
____________________________________________________________________________*/
PGPBoolean	PFLContextIsValid( PFLContextRef context );
#define PFLValidateContext( context )	\
		PGPValidateParam( PFLContextIsValid( context ) )

PGPError	PFLNewContext( PFLContextRef *newContext );

#define kDefaultContextSize		0
/* pass kDefaultContextSize to PFLNewContextCustom() if you want std size */
/* size is intended to allow you to extend the structure for your own stuff */
PGPError	PFLNewContextCustom( PGPSize contextSize,
				PFLNewContextStruct const * custom,
				PFLContextRef *newContext );

PGPError	PFLFreeContext( PFLContextRef context );

PGPError	PFLGetContextUserValue( PFLContextRef context,
					PGPUserValue *userValue );
PGPError	PFLSetContextUserValue( PFLContextRef context,
				PGPUserValue userValue );

/* memory allocation and deallocation */
 void *  PFLContextMemAlloc( PFLContextRef context,
				PGPSize allocationSize, PFLMemoryFlags flags );
					
 void * PFLContextMemAllocCopy( PFLContextRef context,
				PGPSize allocationSize, PFLMemoryFlags flags,
				const void *dataToCopy);
					
PGPError  	PFLContextMemRealloc( PFLContextRef context,
					void **allocation, PGPSize newAllocationSize,
					PFLMemoryFlags flags );
PGPError 	PFLContextMemFree( PFLContextRef context,
					void *allocation );



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS



#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif


#endif /* ] Included_PFLContext_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
