/*____________________________________________________________________________
	MacDebugLeaks.cp
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MacDebugLeaks.cp,v 1.9 1997/09/18 01:34:24 lloyd Exp $
____________________________________________________________________________*/

#include "MacDebugLeaks.h"
#include "MacDebugTrapsOff.h"

#include "pgpMem.h"
#include "pgpLeaks.h"
#include <string.h>

#if ! USE_PGP_LEAKS
	#undef USE_MAC_DEBUG_LEAKS
#endif

#if USE_MAC_DEBUG_LEAKS	// [


extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_OperatorDeleteDeallocType,
		"operator delete" );

extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_DisposeHandleDeallocType,
		"DisposeHandle" );

extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_DisposePtrDeallocType,
		"DisposePtr" );

extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_DisposeRgnDeallocType,
		"DisposeRgn" );

extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_DisposeIconSuiteDeallocType,
		"DisposeIconSuite" );

extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_DisposeAEDescDeallocType,
		"DisposeAEDesc" );

extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_DisposeWindowDeallocType,
		"DisposeWindow" );

extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_DisposeDialogDeallocType,
		"DisposeDialog" );

extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_CloseWindowDeallocType,
		"CloseWindow" );

extern DECLARE_LEAKDEALLOCTYPE( kMacLeaks_CloseDialogDeallocType,
		"CloseDialog" );



extern DECLARE_LEAKALLOCTYPE( kMacLeaks_OperatorNewAllocType,
		"operator new",			kMacLeaks_OperatorDeleteDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewHandleAllocType,
		"NewHandle",			kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewHandleClearAllocType,
		"NewHandleClear",		kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewHandleSysAllocType,
		"NewHandleSys",			kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewHandleSysClearAllocType,
		"NewHandleSysClear",	kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_PtrToHandAllocType,
		"PtrToHand",			kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_HandToHandAllocType,
		"HandToHand",			kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_TempNewHandleAllocType,
		"TempNewHandle",		kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_DetachResourceAllocType,
		"DetachResource",		kMacLeaks_DisposeHandleDeallocType );


extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewRgnAllocType,
		"NewRgn",		kMacLeaks_DisposeRgnDeallocType );


extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewMenuAllocType,
		"NewMenu",		kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_GetMenuBarAllocType,
		"GetMenuBar",	kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_GetNewMBarAllocType,
		"GetNewMBar",	kMacLeaks_DisposeHandleDeallocType );


extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewIconSuiteAllocType,
		"NewIconSuite",	kMacLeaks_DisposeIconSuiteDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_GetIconSuiteAllocType,
		"GetIconSuite",	kMacLeaks_DisposeIconSuiteDeallocType );



extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewAliasAllocType,
		"NewAlias",	kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewAliasMinimalAllocType,
		"NewAliasMinimal",	kMacLeaks_DisposeHandleDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewAliasMinimalFromFullPathAllocType,
		"NewAliasMinimalFromFullPath",	kMacLeaks_DisposeHandleDeallocType );


extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewPtrAllocType,
		"NewPtr",	kMacLeaks_DisposePtrDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewPtrClearAllocType,
		"NewPtrClear", kMacLeaks_DisposePtrDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewPtrSysAllocType,
		"NewPtrSys",		kMacLeaks_DisposePtrDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewPtrSysClearAllocType,
		"NewPtrSysClear",	kMacLeaks_DisposePtrDeallocType );



extern DECLARE_LEAKALLOCTYPE( kMacLeaks_GetNewWindowAllocType,
		"GetNewWindow",		kMacLeaks_DisposeWindowDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewWindowAllocType,
		"NewWindow",		kMacLeaks_DisposeWindowDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_GetNewCWindowAllocType,
		"GetNewCWindow",	kMacLeaks_DisposeWindowDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewCWindowAllocType,
		"NewCWindow",		kMacLeaks_DisposeWindowDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewDialogAllocType,
		"NewDialog",		kMacLeaks_DisposeDialogDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_GetNewDialogAllocType,
		"GetNewDialog",		kMacLeaks_DisposeDialogDeallocType );


extern DECLARE_LEAKALLOCTYPE( kMacLeaks_GetNewWindowStorageAllocType,
		"GetNewWindow",	kMacLeaks_CloseWindowDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewWindowStorageAllocType,
		"NewWindow",	kMacLeaks_CloseWindowDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_GetNewCWindowStorageAllocType,
		"GetNewCWindow", kMacLeaks_CloseWindowDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewCWindowStorageAllocType,
		"NewCWindow",	kMacLeaks_CloseWindowDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_NewDialogStorageAllocType,
		"NewDialog",	kMacLeaks_CloseDialogDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_GetNewDialogStorageAllocType,
		"GetNewDialog",	kMacLeaks_CloseDialogDeallocType );



extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AECreateDescAllocType,
		"AECreateDesc",		kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AECoercePtrAllocType,
		"AECoercePtr",		kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AECoerceDescAllocType,
		"AECoerceDesc", 	kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AEDuplicateDescAllocType,
		"AEDuplicateDesc",	kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AECreateListAllocType,
		"AECreateList",		kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AEGetNthDescAllocType,
		"AEGetNthDesc",		kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AEGetAttributeDescAllocType,
		"AEGetAttributeDesc",	kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AECreateAppleEventAllocType,
		"AECreateAppleEvent",	kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AEGetTheCurrentEventAllocType,
		"AEGetTheCurrentEvent",	kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_AEResolveAllocType,
		"AEResolve", kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_CreateOffsetDescriptorAllocType,
		"CreateOffsetDescriptor",	kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_CreateCompDescriptorAllocType,
		"CreateCompDescriptor",		kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_CreateLogicalDescriptorAllocType,
		"CreateLogicalDescriptor",	kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_CreateObjSpecifierAllocType,
		"CreateObjSpecifier",		kMacLeaks_DisposeAEDescDeallocType );

extern DECLARE_LEAKALLOCTYPE( kMacLeaks_CreateRangeDescriptorAllocType,
		"CreateRangeDescriptor",	kMacLeaks_DisposeAEDescDeallocType );



/*____________________________________________________________________________
	Maintain a list of leaks items.
	Each allocated Handle, Ptr, etc needs to be
	tracked with with a separately allocated leak item.  The strategy here is
	to allocate arrays of leaks items and link them together into a free list.
____________________________________________________________________________*/

typedef struct MacLeakItem
	{
	LeakItem		leak;
	MacLeakItem *	next;	// used while in free list
	} MacLeakItem;
inline MacLeakItem *	PGPLeakToMacLeak( const LeakItem *pgpLeak )
							{ return( (MacLeakItem *)pgpLeak ); }

static MacLeakItem *	sFreeLeakList = nil;


static void	CreateMoreLeakItems( void );


	static MacLeakItem *
GetNextAvailLeakItem( void )
	{
	MacLeakItem	*	nextAvail	= nil;
	
	if ( IsNull( sFreeLeakList ) )
		{
		CreateMoreLeakItems();
		}
	
	nextAvail		= sFreeLeakList;
	if ( IsntNull( nextAvail ) )
		{
		sFreeLeakList	= nextAvail->next;
		}
	
	return( nextAvail );
	}



	static void
ReturnLeakItemToFreeList( MacLeakItem *leak )
	{
	MacDebug_FillWithGarbage( leak, sizeof( *leak ) );
	
	leak->next		= sFreeLeakList;
	sFreeLeakList	= leak;
	}


	
/*____________________________________________________________________________
	Allocate another large chunk of leak items and link them together
	into the free list
____________________________________________________________________________*/
	static void
CreateMoreLeakItems( void )
	{
	MacLeakItem *	moreLeaks		 = nil;
	const UInt32		kLeaksPerChunk	= 128;
	
	moreLeaks	= (MacLeakItem *)
		NewPtrClear( kLeaksPerChunk * sizeof( MacLeakItem ) );
	
	if ( IsntNull( moreLeaks ) )
		{
		MacLeakItem *	curLeak;
		MacLeakItem *	lastLeak;
		
		// link all leak items together in a linked list
		curLeak		= moreLeaks;
		lastLeak	= moreLeaks + ( kLeaksPerChunk - 1);
		while ( curLeak <= lastLeak )
			{
			MacLeakItem *	nextLeak;
			
			nextLeak		= curLeak + 1;
			curLeak->next	= nextLeak;
			curLeak			= nextLeak;
			}
		
		// add this new list to the main list
		lastLeak->next	= sFreeLeakList;
		sFreeLeakList	= moreLeaks;
		}
	}



#pragma mark -

/*____________________________________________________________________________
	Note:
		Some of the routines in this file would normally be called only from
	DebugTraps, usually not from general program code.
____________________________________________________________________________*/

#if PRAGMA_MARK_SUPPORTED
#pragma mark --- Rare Public Use ---
#endif

static UInt16	sSuspendCount	= 0;
	void
MacLeaks_Suspend( void )
	{
	++sSuspendCount;
	
	pgpLeaksSuspend();
	}


	void
MacLeaks_Resume( void )
	{
	pgpAssertMsg( sSuspendCount >= 1,
		"MacLeaks_Resume: not currently suspended" );
	--sSuspendCount;
	
	pgpLeaksResume();
	}


	static inline Boolean
LeaksSuspended( void )
	{
	return( sSuspendCount != 0 );
	}


	static void
IgnoreIfSuspended( LeakItem *	leak )
	{
	if ( LeaksSuspended() )
		{
		pgpLeaksIgnoreItem( leak );
		}
	}


// we can avoid using ANSI if we write this routine here
	static Boolean
CStringsEqual(
	const char *	str1,
	const char *	str2)
	{
	Boolean			stringsEqual	= FALSE;
	register char	theChar;
	
	while ( ( theChar = *str1++ ) == *str2++ )
		{
		if ( theChar == '\0' )
			{
			stringsEqual	= TRUE;
			break;
			}
		}
		
	return( stringsEqual );
	}

/*____________________________________________________________________________
	Ignore an leak if it happens to be in a certain file and allocated with
	a certain allocator.
	
	This is ugly, but necessary to deal with PowerPlant staticallly
	allocated items that are never disposed of, without modifying PowerPlant.
____________________________________________________________________________*/
	static void
PossibleIgnoreFromFile(
	LeakItem *		leak,
	const char *	srcFile,
	UInt32			lineNumber)
	{	
	const UInt32	kMaxLine	= 0xFFFFFFFF;
	typedef struct IgnoreDesc
		{
		const char *	srcFile;
		const char *	allocator;
		const UInt32		startLine;
		const UInt32		endLine;
		} IgnoreDesc;
	
	// good for CodeWarrior release 10
	static IgnoreDesc sIgnoreListCW10[] =
		{
		{ "LGrowZone.cp", "NewHandle", 0, kMaxLine},
		{ "UScreenPort.cp", "NewPtr", 0, kMaxLine},
		};
	const UInt16 kNumIgnores	=
		sizeof( sIgnoreListCW10 ) / sizeof( sIgnoreListCW10[ 0 ] );
	const IgnoreDesc *	desc;
	
	desc	= &sIgnoreListCW10[ 0 ];
	for( UInt16 index = 0; index < kNumIgnores; ++index, ++desc)
		{
		if ( CStringsEqual( desc->srcFile, srcFile ) )
			{
			if ( CStringsEqual( desc->allocator, leak->allocType->name ) )
				{
				if ( lineNumber >= desc->startLine &&
				lineNumber <= desc->endLine )
					{
					pgpLeaksIgnoreItem( leak );
					}
				break;
				}
			}
		}
	}


/*____________________________________________________________________________
	Remember an item of any kind.
____________________________________________________________________________*/
	static void
RememberHandleOrPtr(
	const void *			theItem,
	LeakAllocType *			allocType,
	UInt32					size,
	const char *			srcFile,
	long					srcLine)
	{
	pgpAssertAddrValid( theItem, VoidAlign );
	pgpAssertAddrValid( allocType, LeakAllocType );
	pgpAssertAddrValid( srcFile, char );
	
	MacLeakItem *leak;
	
	leak	= GetNextAvailLeakItem();
	pgpAssertMsg( IsntNull( leak ),
		"RememberHandleOrPtr: no leak items available");
	
	if ( IsntNull( leak ) )
		{
		pgpLeaksRememberItem( &leak->leak, allocType,
				theItem, size, srcFile, srcLine);
				
		IgnoreIfSuspended( &leak->leak );
		PossibleIgnoreFromFile( &leak->leak, srcFile, srcLine);
		}
	
	}
	
	

/*____________________________________________________________________________
	Forget an item; it has been disposed of properly.
____________________________________________________________________________*/
	static void
ForgetItem(
	const void	*		handleOrPtr,
	LeakDeallocType *	deallocType)
	{
	pgpAssertAddrValid( handleOrPtr, VoidAlign );
	pgpAssertAddrValid( deallocType, LeakDeallocType );
	
	LeakItem *		pgpLeak	= pgpLeaksFindItem( handleOrPtr );
	
	if ( IsntNull( pgpLeak ) )
		{
		MacLeakItem *	macLeak	= PGPLeakToMacLeak( pgpLeak );
		
		pgpLeaksForgetItem( pgpLeak, deallocType );
		
		ReturnLeakItemToFreeList( macLeak );
		}
	}
	
	
/*____________________________________________________________________________
	A Handle has been allocated. Remember it.
____________________________________________________________________________*/
	void
MacLeaks_RememberHandle(
	const void *			theHandle,
	LeakAllocType *			howAllocated,
	const char *			srcFileNameCString,
	long					lineNumber)
	{
	AssertHandleIsValid( theHandle, "MacLeaks_RememberHandle" );
	
	RememberHandleOrPtr( theHandle, howAllocated,
			GetHandleSize( (Handle)theHandle ),
			srcFileNameCString, lineNumber);
	}



/*____________________________________________________________________________
	A Handle has been deallocated. Forget it.
____________________________________________________________________________*/
	void
MacLeaks_ForgetHandle(
	const void *		theHandle,
	LeakDeallocType *	deallocType )
	{
	AssertHandleIsValid( theHandle, "MacLeaks_RememberHandle" );
	
	ForgetItem( theHandle, deallocType);
	}



/*____________________________________________________________________________
	A memory manager 'Ptr' has been allocated. Remember it.
____________________________________________________________________________*/
	void
MacLeaks_RememberPtr(
	const void *			thePtr,
	LeakAllocType *			howAllocated,
	UInt32					size,
	const char *			srcFileNameCString,
	long					lineNumber)
	{
	// Ptr types always aligned to 4 PGPByte boundaries;
	// check for that instead of VoidAlign
	pgpAssertAddrValid( thePtr, long);
	
	RememberHandleOrPtr( thePtr, howAllocated, size,
		srcFileNameCString, lineNumber);
	}


/*____________________________________________________________________________
	A Ptr has been deallocated. Forget it.
____________________________________________________________________________*/
	void
MacLeaks_ForgetPtr(
	const void *		thePtr,
	LeakDeallocType *	deallocType)
	{
	// Ptr types always aligned to 4 PGPByte boundaries;
	// check for that instead of VoidAlign
	pgpAssertAddrValid( thePtr, long);
	
	ForgetItem( thePtr, deallocType);
	}



/*____________________________________________________________________________
	A AEDesc has been allocated. Remember it.
____________________________________________________________________________*/
	void
MacLeaks_RememberAEDesc(
	const AEDesc *			theAEDesc,
	LeakAllocType *			howAllocated,
	const char *			srcFileNameCString,
	long					lineNumber)
	{
	pgpAssertAddrValidMsg( theAEDesc, AEDesc, "MacLeaks_RememberAEDesc");

	if ( theAEDesc->descriptorType != typeNull &&
		 IsntNull( theAEDesc->dataHandle ))
		{
		RememberHandleOrPtr( theAEDesc->dataHandle,
			howAllocated, 0, srcFileNameCString, lineNumber);
		}
	}



/*____________________________________________________________________________
	A AEDesc has been deallocated. Forget it.
____________________________________________________________________________*/
	void
MacLeaks_ForgetAEDesc(
	const AEDesc	*	theAEDesc,
	LeakDeallocType *	deallocType)
	{
	pgpAssertAddrValidMsg( theAEDesc, AEDesc, "MacLeaks_ForgetAEDesc");

	if ( theAEDesc->descriptorType != typeNull &&
		 IsntNull( theAEDesc->dataHandle ) &&
		 MacLeaks_ItemIsRemembered( theAEDesc->dataHandle ))
		{
		ForgetItem( theAEDesc->dataHandle, deallocType);
		}
	}



/*____________________________________________________________________________
	An object has been allocated. Remember it.
	
	Override operator new() so you can remember objects.
____________________________________________________________________________*/
	void
MacLeaks_RememberObject(
	const void	*	object,
	UInt32			size,
	const char *	srcFile,
	long			lineNumber)
	{
	MacLeakItem *leak;
	
	leak	= GetNextAvailLeakItem();
	pgpAssertMsg( IsntNull( leak ),
		"MacLeaks_RememberObject: no leak items available");
	
	if ( IsntNull( leak ) )
		{
		// file, line not available for original caller, so 
		pgpLeaksRememberItem( &leak->leak, kMacLeaks_OperatorNewAllocType,
				object, size, srcFile, lineNumber );
	
		IgnoreIfSuspended( &leak->leak );
		}
	}


/*____________________________________________________________________________
	An object has been deallocated. Forget it.
	
	Note: you will need to override operator delete() so
	you can call this routine.
____________________________________________________________________________*/
	void
MacLeaks_ForgetObject( const void *object )
	{
	ForgetItem( object, kMacLeaks_OperatorDeleteDeallocType);
	}
	
	

/*____________________________________________________________________________
	Return TRUE if an item is remembered.
____________________________________________________________________________*/
	Boolean
MacLeaks_ItemIsRemembered( const void	*handleOrPtr )
	{
	return( IsntNull( pgpLeaksFindItem( handleOrPtr ) ) );
	}


/*____________________________________________________________________________
	Ignore this item (Ptr, Handle or object).  It will not be reported as a 
	leak.
	However, the item should remain in the list of remembered items until it
	has been disposed. This allows stricter assertions in some cases
	(e.g. an attempt to dispose of it twice).
____________________________________________________________________________*/
	void
MacLeaks_IgnoreItem( const void *mem)
	{
	pgpAssertAddrValidMsg( mem, VoidAlign, "MacLeaks_IgnoreItem" );
	
	pgpLeaksIgnoreItem( pgpLeaksFindItem( mem ) );
	}



#endif // ] USE_MAC_DEBUG_LEAKS







