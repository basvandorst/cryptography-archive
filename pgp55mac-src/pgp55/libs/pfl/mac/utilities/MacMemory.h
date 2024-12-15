/*____________________________________________________________________________
____________________________________________________________________________*/
#pragma once

#include <stddef.h>
#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


#include "pgpMem.h"

#define kMacMemory_UseCurrentHeap		( 1UL << 0 )
#define kMacMemory_UseApplicationHeap	( 1UL << 1 )
#define kMacMemory_UseSystemHeap		( 1UL << 2 )
#define kMacMemory_UseTempMem			( 1UL << 3 )
#define kMacMemory_PreferTempMem		( 1UL << 4 )
#define kMacMemory_ClearBytes			( 1UL << 5 )

typedef PGPUInt16	MacMemoryAllocFlags;

Handle	pgpNewHandle( const size_t size, const MacMemoryAllocFlags flags );
void	pgpDisposeHandle( Handle theHandle );


void *	pgpAllocMac( const size_t size, const MacMemoryAllocFlags flags );
void	pgpFreeMac( void *memory );

UInt32	pgpMaxBlock( MacMemoryAllocFlags allocFlags );
void *	pgpNewPtrMost( UInt32 desiredSize, UInt32 minSize,
			MacMemoryAllocFlags allocFlags, UInt32 * actualSize );



	
#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS