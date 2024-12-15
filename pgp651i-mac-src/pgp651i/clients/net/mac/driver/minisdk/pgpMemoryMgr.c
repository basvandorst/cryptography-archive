/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: pgpMemoryMgr.c,v 1.3 1999/03/10 02:53:17 heller Exp $____________________________________________________________________________*/#include "pgpPFLErrors.h"#include "pgpMem.h"//#include "pgpRMWOLock.h"#include "pgpMemoryMgrPriv.h"/*____________________________________________________________________________	This structure is prepended to every allocated block.		It is dynamically padded to keep it aligned to the alignment	requirement.  See kRealHeaderSize.____________________________________________________________________________*/typedef struct DataHeader{#define kHeaderMagic	((PGPUInt16)0xABCD)	PGPMemoryMgrRef	mgr;		PGPUInt32		secureAlloc		: 1;	PGPUInt32		nonPageable		: 1;	PGPUInt32		requestSize	: 30;	PGPUInt32		magic;	#define kDataHeaderMagic		0x42554746  /*'BUGF'*/} DataHeader;#define kHeaderAlign		16#define kOddHeaderBytes		( sizeof( DataHeader ) % kHeaderAlign )#define kNeededHeaderBytes	( (kHeaderAlign - kOddHeaderBytes) % kHeaderAlign)#define kRealHeaderSize		( sizeof( DataHeader ) + kNeededHeaderBytes )#define HeaderToUserBlock( hdr )	\	( (void *)( ((char *)(hdr)) + kRealHeaderSize) )#define UserBlockToHeader( user )	\	( (DataHeader *)(((char *)user) - kRealHeaderSize) )	static PGPBooleansDataHeaderIsValid( const DataHeader *header ){	return( IsntNull( header ) &&			PGPMemoryMgrIsValid( header->mgr ) &&			header->magic == kDataHeaderMagic );}struct PGPMemoryMgr{#define kPGPMemoryMgrMagic	0x4D4D6772	/* 'MMgr' */	PGPUInt32		magic;				/* Always kPGPMemoryMgrMagic */	PGPUserValue	customValue;		/* for custom memory mgrs */	PGPBoolean		isDefault;		PGPMemoryMgrAllocationProc		allocProc;	PGPMemoryMgrReallocationProc	reallocProc;	PGPMemoryMgrDeallocationProc	deallocProc;		PGPMemoryMgrSecureAllocationProc	secureAllocProc;	/* may be NULL */	PGPMemoryMgrSecureDeallocationProc	secureDeallocProc;	/* may be NULL */} ;	PGPBooleanPGPMemoryMgrIsValid( PGPMemoryMgrRef mgr ){	return( IsntNull( mgr ) &&			mgr->magic == kPGPMemoryMgrMagic );}	voidPGPCheckMemoryMgrIntegrity( PGPMemoryMgrRef mgr ){	(void)mgr;}	static voidsInitDataHeader(	PGPMemoryMgrRef	mgr,	PGPSize			requestSize,	DataHeader *	header ){	pgpClearMemory( header, sizeof(*header) );	header->mgr				= mgr;	header->secureAlloc		= FALSE;	header->nonPageable		= FALSE;	header->requestSize	= requestSize;		header->magic	= kDataHeaderMagic;}/*____________________________________________________________________________	Reallocate a secure block.		To preserve the semantics of the block not changing if it shrinks,	we take no action if the block is shrinking.  If it's growing, we	always discard it and create a new one.____________________________________________________________________________*/	static PGPErrorsSecureMemoryReallocProc(	PGPMemoryMgrRef		mgr,	void **				allocation,	PGPSize 			newAllocationSize,	PGPMemoryMgrFlags	flags,	PGPSize				existingSize ){	PGPError		err		= kPGPError_NoErr;		pgpAssert( IsntNull( *allocation ) );	#if 0	if ( newAllocationSize <= existingSize )	{		PGPUInt32	excessByteCount;		char *		dataToClear	= NULL;				/* leave existing size intact, but clear excess bytes now		to avoid leaving sensitive data in memory unnecessarily */		excessByteCount	= existingSize - newAllocationSize;		dataToClear	= ((char *)*allocation) + newAllocationSize;		pgpClearMemory( dataToClear, excessByteCount );				err	= kPGPError_NoErr;	}	else#endif	{	/* block is increasing in size */		PGPBoolean	isNonPageable;		void *		newBlock	= NULL;				newBlock	= (mgr->secureAllocProc)( mgr, mgr->customValue,			newAllocationSize, flags, &isNonPageable);		if ( IsntNull( newBlock ) )		{			DataHeader *	oldHeader	= (DataHeader *)*allocation;			pgpCopyMemory( *allocation, newBlock,				pgpMin( existingSize, newAllocationSize ) );						(mgr->secureDeallocProc)( mgr, mgr->customValue,				*allocation, existingSize,				(PGPBoolean)oldHeader->nonPageable );			*allocation	= newBlock;		}		else		{			err	= kPGPError_OutOfMemory;		}	}		return( err );}/*____________________________________________________________________________	Allocate a new Memory Mgr using custom memory allocators.____________________________________________________________________________*/	PGPErrorPGPNewMemoryMgrCustom(	PGPNewMemoryMgrStruct const *	custom,	PGPMemoryMgrRef *				newMgr ){	PGPError			err = kPGPError_NoErr;	PGPMemoryMgrRef		mgr	= NULL;	PGPUInt32			idx;		PGPValidatePtr( newMgr );	*newMgr	= NULL;	PGPValidatePtr( custom );	PGPValidatePtr( custom->allocProc );	PGPValidatePtr( custom->reallocProc );	PGPValidatePtr( custom->deallocProc );	PGPValidatePtr( custom->secureAllocProc );	PGPValidatePtr( custom->secureDeallocProc );	PGPValidateParam( custom->sizeofStruct >= sizeof( *custom ) );	PGPValidateParam( custom->reserved == NULL );		#define kNumPads	sizeof( custom->pad ) / sizeof( custom->pad[ 0 ] )	for( idx = 0; idx < kNumPads; ++idx )	{		PGPValidateParam( custom->pad[ idx ] == NULL );	}		/* NOTE: do not depend on kPGPMemoryMgrFlags_Clear here;	 allocator is not required to support it */	mgr = (PGPMemoryMgr *) (*(custom->allocProc))( NULL, custom->customValue,				sizeof( *mgr ), 0 );	if( IsntNull( mgr ) )	{		pgpClearMemory( mgr, sizeof( *mgr ) );		mgr->magic				= kPGPMemoryMgrMagic;		mgr->allocProc			= custom->allocProc;		mgr->reallocProc		= custom->reallocProc;		mgr->deallocProc		= custom->deallocProc;		mgr->secureAllocProc	= custom->secureAllocProc;		mgr->secureDeallocProc	= custom->secureDeallocProc;		mgr->customValue		= custom->customValue;		mgr->isDefault			= FALSE;			}	else	{		err = kPGPError_OutOfMemory;	}		*newMgr	= mgr;			return( err );}/*________________________________________________________________________________________________________________________________________________________*/	PGPErrorPGPNewMemoryMgr(	PGPFlags			reserved,	PGPMemoryMgrRef *	newMgr ){	PGPError	err	= kPGPError_NoErr;		PGPValidatePtr( newMgr );	*newMgr	= NULL;	PGPValidateParam( reserved == 0 );		err	= pgpCreateStandardMemoryMgr( newMgr );	if ( IsntPGPError( err ) )	{		(*newMgr)->isDefault	= TRUE;	}		return( err );}/*____________________________________________________________________________	Delete an existing PFLContext and all resources associated with it.____________________________________________________________________________*/	PGPErrorPGPFreeMemoryMgr(PGPMemoryMgrRef	mgr){	PGPError	err = kPGPError_NoErr;		PGPValidateParam( PGPMemoryMgrIsValid( mgr ) );	if( PGPMemoryMgrIsValid( mgr ) )	{		PGPUserValue	customValue	= mgr->customValue;		PGPBoolean		isDefault	= mgr->isDefault;				(*mgr->deallocProc)( NULL, customValue, mgr, sizeof( *mgr ));				if ( isDefault )		{			pgpDisposeStandardMemoryMgrUserValue( customValue );		}			}	else	{		err = kPGPError_BadParams;	}		return( err );}	PGPErrorPGPGetMemoryMgrCustomValue(	PGPMemoryMgrRef 	mgr,	PGPUserValue *		customValue){	PGPError	err = kPGPError_NoErr;		PGPValidatePtr( customValue );	*customValue	= NULL;	PGPValidateMemoryMgr( mgr );	/* illegal to change user value of default allocator */	PGPValidateParam( ! mgr->isDefault );		if( PGPMemoryMgrIsValid( mgr ) &&		IsntNull( customValue ) )	{		*customValue = mgr->customValue;	}	else	{		err = kPGPError_BadParams;	}		return( err );}	PGPErrorPGPSetMemoryMgrCustomValue(	PGPMemoryMgrRef 	mgr,	PGPUserValue 		customValue){	PGPError	err = kPGPError_NoErr;		PGPValidateMemoryMgr( mgr );	/* illegal to change user value of default allocator */	PGPValidateParam( ! mgr->isDefault );		if( PGPMemoryMgrIsValid( mgr ) )	{		mgr->customValue = customValue;	}	else	{		err = kPGPError_BadParams;	}		return( err );}/*____________________________________________________________________________	Allocate a block of memory using the allocator stored in a PFLContext.____________________________________________________________________________*/	void *PGPNewData(	PGPMemoryMgrRef 	mgr,	PGPSize 			requestSize,	PGPMemoryMgrFlags	flags){	void	*allocation		= NULL;	PGPSize	allocationSize	= requestSize;		allocationSize	+= kRealHeaderSize;		pgpAssert( PGPMemoryMgrIsValid( mgr ) );		if( PGPMemoryMgrIsValid( mgr ) )	{		allocation = (*mgr->allocProc)( mgr, mgr->customValue,						allocationSize, flags );	}		if ( IsntNull( allocation ) )	{		DataHeader *	header	= NULL;			if ( (flags & kPGPMemoryMgrFlags_Clear ) != 0 )		{			pgpClearMemory( allocation, allocationSize );		}				header	= (DataHeader *)allocation;		sInitDataHeader( mgr, requestSize, header );				allocation		= HeaderToUserBlock( allocation );	}	return( allocation );}/*____________________________________________________________________________	Allocate a block of memory (secure or non-secure).____________________________________________________________________________*/	PGPErrorPGPReallocData(	PGPMemoryMgrRef 	mgr,	void **				userPtr,	PGPSize 			requestSize,	PGPMemoryMgrFlags	flags){	PGPError		err			= kPGPError_NoErr;		PGPValidateParam( PGPMemoryMgrIsValid( mgr ) );	PGPValidatePtr( userPtr );	/* allow a NULL input  */	if ( IsntNull( *userPtr ) )	{		DataHeader *	header	= UserBlockToHeader( *userPtr );		PGPSize			oldRequestSize	= header->requestSize;				if ( header->secureAlloc )		{			err = sSecureMemoryReallocProc( mgr,						(void **) &header,						kRealHeaderSize + requestSize,						flags,						kRealHeaderSize + header->requestSize );					}		else		{			err = (*(mgr->reallocProc))( mgr, mgr->customValue,						(void **) &header,						kRealHeaderSize + requestSize,						flags,						kRealHeaderSize + header->requestSize );		}				if ( IsntPGPError( err ) )		{			*userPtr	= HeaderToUserBlock( header );			header->requestSize	= requestSize;		}						if ( IsntPGPError( err ) && requestSize > oldRequestSize )		{			if ( (flags & kPGPMemoryMgrFlags_Clear ) != 0 )			{				pgpClearMemory( ((char *)(*userPtr)) + oldRequestSize,					requestSize - oldRequestSize );			}		}	}	else	{		/* allocate a brand-new block */		*userPtr	= PGPNewData( mgr, requestSize, flags );		if ( IsNull( *userPtr ) )		{			err	= kPGPError_OutOfMemory;		}	}		return( err );}/*____________________________________________________________________________	Free a block of memory, whether secure or not.____________________________________________________________________________*/	PGPErrorPGPFreeData( void * allocation ){	PGPError		err	= kPGPError_NoErr;	PGPMemoryMgrRef	mgr	= NULL;	DataHeader *	header;	PGPSize			allocationSize;		PGPValidatePtr( allocation );	header	= UserBlockToHeader( allocation );	PGPValidateParam( sDataHeaderIsValid( header ) );		mgr				= header->mgr;	allocationSize	= kRealHeaderSize + header->requestSize;		if ( header->secureAlloc )	{		pgpClearMemory( allocation, header->requestSize );				err	= (*(mgr->secureDeallocProc))(					mgr, mgr->customValue,					header,					allocationSize,					(PGPBoolean)header->nonPageable );	}	else	{		err	= (*(mgr->deallocProc))(					mgr, mgr->customValue,					header,					allocationSize );	}		return( err );}	void *PGPNewSecureData(	PGPMemoryMgrRef 	mgr,	PGPSize 			requestSize,	PGPMemoryMgrFlags	flags ){	void *		allocation		= NULL;	PGPSize		allocationSize	= requestSize;	PGPBoolean	isNonPageable;		allocationSize	+= kRealHeaderSize;		if( PGPMemoryMgrIsValid( mgr ) )	{		allocation = (*mgr->secureAllocProc)( mgr, mgr->customValue,						allocationSize, flags, &isNonPageable );	}		if ( IsntNull( allocation ) )	{		DataHeader *	header	= NULL;			if ( (flags & kPGPMemoryMgrFlags_Clear ) != 0)		{			pgpClearMemory( allocation, allocationSize );		}				header	= (DataHeader *)allocation;		sInitDataHeader( mgr, requestSize, header );		header->secureAlloc	= TRUE;		header->nonPageable	= isNonPageable;				allocation		= HeaderToUserBlock( allocation );	}	return( allocation );}	PGPFlagsPGPGetMemoryMgrDataInfo( void *allocation ){	PGPFlags		flags	= 0;	DataHeader *	header;		if ( IsNull( allocation ) )		return( 0 );			header	= UserBlockToHeader( allocation );	if ( sDataHeaderIsValid( header ) )	{		flags	|= kPGPMemoryMgrBlockInfo_Valid;				if ( header->secureAlloc )			flags	|= kPGPMemoryMgrBlockInfo_Secure;					if ( header->nonPageable )			flags	|= kPGPMemoryMgrBlockInfo_NonPageable;	}	return( flags );}typedef struct MemoryMgrInfo{	struct MemoryMgrInfo	*next;	PGPMemoryMgrRef			mgr;} MemoryMgrInfo;/*static PGPMemoryMgrRef	sDefaultMemoryMgr 		= kInvalidPGPMemoryMgrRef;static MemoryMgrInfo	*sMemoryMgrList			= NULL;static PGPRMWOLock		sMemoryMgrListLock;	PGPMemoryMgrRefPGPGetDefaultMemoryMgr(void){	PGPMemoryMgrRef	mgr;		if( IsntNull( sMemoryMgrList ) )	{		/*		** If the list is non-null, then we have a mgr. We need to use a RMWO		** lock because this is a global list		*//*				PGPRMWOLockStartReading( &sMemoryMgrListLock );			mgr = sDefaultMemoryMgr;		PGPRMWOLockStopReading( &sMemoryMgrListLock );	}	else	{		mgr = kInvalidPGPMemoryMgrRef;	}		return( mgr );}	PGPErrorPGPSetDefaultMemoryMgr(PGPMemoryMgrRef memoryMgr){	PGPError	err 			= kPGPError_NoErr;	PGPBoolean	foundMemoryMgr	= FALSE;		PGPValidateParam( PGPMemoryMgrRefIsValid( memoryMgr ) );	if( IsNull( sMemoryMgrList ) )	{		/*		** If there is no list, initialize the RMWO lock. This presents		** a small window where the lock could get initialized twice,		** however this is not a call likely to introduce race conditions.		*//*			InitializePGPRMWOLock( &sMemoryMgrListLock );	}	PGPRMWOLockStartWriting( &sMemoryMgrListLock );	if( IsntNull( sMemoryMgrList ) )	{		MemoryMgrInfo	*cur;				/*		** Check to see if this memory mgr is already in our list and use		** it if it is		*//*				cur = sMemoryMgrList;		while( IsntNull( cur ) )		{			if( cur->mgr == memoryMgr )			{				foundMemoryMgr = TRUE;				break;			}						cur = cur->next;		}	}	if( ! foundMemoryMgr )	{		MemoryMgrInfo	*info;				info = (MemoryMgrInfo *) PGPNewData( memoryMgr, sizeof( *info ), 0 );		if( IsntNull( info ) )		{			info->mgr = memoryMgr;						info->next		= sMemoryMgrList;			sMemoryMgrList	= info;		}		else		{			err = kPGPError_OutOfMemory;		}	}	if( IsntPGPError( err ) )		sDefaultMemoryMgr = memoryMgr;			PGPRMWOLockStopWriting( &sMemoryMgrListLock );		return( err );}	voidpgpFreeDefaultMemoryMgrList(void){	if( IsntNull( sMemoryMgrList ) )	{		MemoryMgrInfo	*cur;		PGPRMWOLockStartWriting( &sMemoryMgrListLock );		cur = sMemoryMgrList;		while( IsntNull( cur ) )		{			MemoryMgrInfo	*next;			PGPMemoryMgrRef	mgr;						next 	= cur->next;			mgr		= cur->mgr;						PGPFreeData( cur );			PGPFreeMemoryMgr( mgr );						cur = next;		}				sMemoryMgrList = NULL;				PGPRMWOLockStopWriting( &sMemoryMgrListLock );		DeletePGPRMWOLock( &sMemoryMgrListLock );	}}*//*__Editor_settings____	Local Variables:	tab-width: 4	End:	vi: ts=4 sw=4	vim: si_____________________*/