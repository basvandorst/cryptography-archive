/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	pgpMemLock.c - main C routines for PGPmemlock VxD (Win95/98)
	

	$Id: pgpMemLock.c,v 1.2 1999/04/09 19:14:48 pbj Exp $
____________________________________________________________________________*/
/*____________________________________________________________________________

	This virtual device driver (VxD) is used to lock memory pages into RAM 
	so that they will not get copied to the system page file.  This 
	functionality is desired when allocating buffers that will contain
	sensitive information (e.g. passwords) which could present a security
	hazard if copied to the page file.

    Usage :

	1) memory allocation.  Memory that is to be locked should be allocated
	directly from the OS using the Win32 function VirtualAlloc :

		PVOID	pMem;
		ULONG	ulNumBytes;

		ulNumBytes = number_of_bytes_to_allocate;

		pMem = VirtualAlloc ( NULL, ulNumBytes, MEM_COMMIT, PAGE_READWRITE );


	Notes: 
	NULL			- the first parameter to VirtualAlloc must be NULL
	ulNumBytes		- the actual allocation is rounded up by VirtualAlloc
					  to the next page boundary
	MEM_COMMIT		- this flag causes a region of physical memory to be
					  committed for the allocated block of virtual memory
	PAGE_READWRITE	- this allows both read and write access to the memory


	2) memory locking.  Lock the memory by passing the address and size of
	the allocated memory to the driver :

		PGPMEMLOCKSTRUCT	mls;
		DWORD				dw;
		BOOL				bDIOreturn;
	 
		mls.pMem = pMem;
		mls.ulNumBytes = ulNumBytes;
		bDIOreturn = DeviceIoControl(	hPGPMemLockDriver, 
										IOCTL_PGPMEMLOCK_LOCK_MEMORY, 
										&mls, 
										sizeof( mls ), 
										&mls, 
										sizeof( mls ), 
										&dw, 
										NULL );
		if ( !bDIOreturn )
		{
			// this is an error communicating with the driver
		}

		else 
		{
			if ( mls.ulError != 0 )
			{
				// this is an error internal to the driver
			}
			else 
			{
				// successfully locked!
			}
		}


	3) unlocking the memory.  To unlock the memory, the procedure is 
	similar :

		PGPMEMLOCKSTRUCT	mls;
		DWORD				dw;
		BOOL				bDIOreturn;
	 
		mls.pMem = pMem;
		mls.ulNumBytes = ulNumBytes;
		bDIOreturn = DeviceIoControl(	hPGPMemLockDriver, 
										IOCTL_PGPMEMLOCK_UNLOCK_MEMORY, 
										&mls, 
										sizeof( mls ), 
										&mls, 
										sizeof( mls ), 
										&dw, 
										NULL );
		if ( !bDIOreturn )
		{
			// this is an error communicating with the driver
		}

		else 
		{
			if ( mls.ulError != 0 )
			{
				// this is an error internal to the driver
			}
			else 
			{
				// successfully unlocked!
			}
		}


	4) freeing the memory.  Use VirtualFree to free the memory :

		PVOID	pMem;

		VirtualFree( pMem, 0, MEM_RELEASE );
	
____________________________________________________________________________*/


#define WANTVXDWRAPS
#include <basedef.h>
#include <vmm.h>
#include <vtd.h>
#include <vwin32.h>
#include <vxdwraps.h>

#include <debug.h>

#include <winerror.h>
#include <memory.h>

#define PGP_WIN32	1
#include "pgpMemLockWin32.h"

// instruct compiler to use inline version of function
#pragma intrinsic(memset)

// constants
//	number of list items that can be held in each internal heap block
#define NUMITEMSPERBLOCK	64

// virtual memory paging in Win32 uses 4K byte pages (2^12) 
#define WIN32PAGESIZE		12	

// macros
#if DEBUG
#define PGPmlPrintf(arg) Debug_Printf arg
#else
#define PGPmlPrintf(arg)
#endif

#define PGPmlEnterCriticalSection() _EnterMutex (g_mutexHeap, \
			BLOCK_THREAD_IDLE|BLOCK_SVC_INTS)
#define PGPmlLeaveCriticalSection() _LeaveMutex (g_mutexHeap)

// typedefs
//	elements of list used to keep track of locked blocks
typedef struct _MEMLOCKITEM {
	struct _MEMLOCKITEM* next;
	HANDLE		hProcess;
	DWORD		dwHandle;
	ULONG		ulPage;
	ULONG		ulNumPages;
} PGPMEMLOCKITEM, *PPGPMEMLOCKITEM;

//	block of such items
typedef struct _MEMLOCKBLOCK {
	struct _MEMLOCKBLOCK*	nextBlock;
	PGPMEMLOCKITEM			item[NUMITEMSPERBLOCK];
} PGPMEMLOCKBLOCK, *PPGPMEMLOCKBLOCK;

// global data 
//	pointer to local heap
static PPGPMEMLOCKBLOCK	g_pHeapBlocks;

//	pointer to head of free list
static PPGPMEMLOCKITEM	g_pFreeHeapItems;

//	pointer to head of used list
static PPGPMEMLOCKITEM	g_pLockedList;

//  handle of mutex used to protect the heap linked-list code
static PVMMMUTEX		g_mutexHeap;


//----------------------------------------------------|
//  internal item allocation routines
//

VOID
PGPmlHeapBlockInit (PPGPMEMLOCKBLOCK pmlb)
{
	PPGPMEMLOCKITEM	pmli	= NULL;
	ULONG			i		= 0;

	pmlb->nextBlock = NULL;

	pmli = &(pmlb->item[0]);

	for (i=1; i<NUMITEMSPERBLOCK; i++)
	{
		pmli->next = &(pmlb->item[i]);
		pmli = pmli->next;
	}
	pmli->next = NULL;
}


BOOL
PGPmlHeapInit ()
{

	g_pHeapBlocks = _HeapAllocate (sizeof(PGPMEMLOCKBLOCK), 
								   HEAPZEROINIT|HEAPLOCKEDIFDP);

	if (g_pHeapBlocks) 
	{
		PGPmlHeapBlockInit (g_pHeapBlocks);
		g_pFreeHeapItems = &(g_pHeapBlocks->item[0]);
		return TRUE;
	}
	else return FALSE;
}


BOOL
PGPmlHeapDestroy ()
{
	PPGPMEMLOCKBLOCK	pmlb		= NULL;
	PPGPMEMLOCKBLOCK	pmlbNext	= NULL;

	pmlb = g_pHeapBlocks;
	
	while (pmlb) 
	{
		pmlbNext = pmlb->nextBlock;
		_HeapFree (pmlb, 0);
		pmlb = pmlbNext;
	}

	g_pFreeHeapItems = NULL;

	return TRUE;
}


PPGPMEMLOCKITEM
PGPmlAllocateItem ()
{
	PPGPMEMLOCKITEM		pmli		= NULL;
	PPGPMEMLOCKBLOCK	pmlb		= NULL;
	PPGPMEMLOCKBLOCK	pmlbNew		= NULL;

	// there is an item available
	if (g_pFreeHeapItems)
	{
		// remove item from free list
		pmli = g_pFreeHeapItems;
		g_pFreeHeapItems = g_pFreeHeapItems->next;

		return pmli;
	}

	// free list is empty, allocate more memory
	pmlbNew = _HeapAllocate (sizeof(PGPMEMLOCKBLOCK), 
							 HEAPZEROINIT|HEAPLOCKEDIFDP);
	if (pmlbNew) 
	{
		// append block to linked list of blocks
		pmlb = g_pHeapBlocks;
		while (pmlb->nextBlock) pmlb = pmlb->nextBlock;
		pmlb->nextBlock = pmlbNew;
		pmlbNew->nextBlock = NULL;

		// link new items into a list
		PGPmlHeapBlockInit (pmlbNew);

		// first item will be returned to caller
		pmli = &(pmlbNew->item[0]);

		// add rest of new items to free list
		g_pFreeHeapItems = &(pmlbNew->item[1]);

		return pmli;
	}

	// couldn't get an item
	return NULL;
}


VOID
PGPmlFreeItem (PPGPMEMLOCKITEM pmli)
{
	// add item to list of free items
	pmli->next = g_pFreeHeapItems;
	g_pFreeHeapItems = pmli;
}


//----------------------------------------------------|
//  wipe the specified memory block
//
//	Arguments:
//		ulPage			- initial page being wiped
//		ulNumPages		- number of pages in block being wiped
//

VOID
PGPmlWipeBlock (
	ULONG	ulPage,
	ULONG	ulNumPages)
{
	PVOID	pmem;
	ULONG	ulBytes;

	pmem = (PVOID)(ulPage << WIN32PAGESIZE);
	ulBytes = ulNumPages << WIN32PAGESIZE;

	memset (pmem, 0, ulBytes);
}


//----------------------------------------------------|
//  add the specified memory block to list of locked blocks
//

BOOL
PGPmlAddBlockToList (
	PDIOCPARAMETERS p,
	ULONG			ulPage,
	ULONG			ulNumPages)
{
	HANDLE			hProc		= NULL;
	DWORD			dwHandle	= NULL;
	PPGPMEMLOCKITEM	pmli		= NULL;

	// allocate new linked-list item
	pmli = PGPmlAllocateItem (); 
	if (!pmli) return FALSE;

	// get process handle of current process
	VxDCall (VWIN32_GetCurrentProcessHandle)
	_asm mov hProc, eax

	// get handle as used by calling app
	dwHandle = p->hDevice;

	// fill in new list element
	pmli->next = g_pLockedList;
	pmli->hProcess = hProc;
	pmli->dwHandle = dwHandle;
	pmli->ulPage = ulPage;
	pmli->ulNumPages = ulNumPages;

	// insert element at head of list
	g_pLockedList = pmli;

	return TRUE;
}


//----------------------------------------------------|
//  remove the specified memory block from list of locked blocks
//

BOOL
PGPmlRemoveBlockFromList (
	PDIOCPARAMETERS p,
	ULONG			ulPage,
	ULONG			ulNumPages)
{
	HANDLE			hProc		= NULL;
	DWORD			dwHandle	= NULL;
	PPGPMEMLOCKITEM	pmli		= NULL;
	PPGPMEMLOCKITEM	pPrev		= NULL;

	// get process handle of current process
	VxDCall (VWIN32_GetCurrentProcessHandle)
	_asm mov hProc, eax

	// get handle as used by calling app
	dwHandle = p->hDevice;

	// search list for specified block
	pPrev = NULL;
	pmli = g_pLockedList;
	while (pmli) 
	{
		if ((pmli->hProcess == hProc) &&
			(pmli->dwHandle == dwHandle))
		{
			if ((pmli->ulPage == ulPage)  && 
				(pmli->ulNumPages == ulNumPages))
			{
				if (pPrev)
				{
					pPrev->next = pmli->next;
				}
				else 
				{
					g_pLockedList = pmli->next;
				}
				PGPmlWipeBlock (ulPage, ulNumPages);
				PGPmlFreeItem (pmli);
				return TRUE;
			}
		}
		pPrev = pmli;
		pmli = pmli->next;
	}

	// didn't find specified block, return error
	return FALSE;
}


//----------------------------------------------------|
//  unlock and remove the memory blocks belonging to this handle
//

BOOL
PGPmlUnlockHandleBlocks (PDIOCPARAMETERS p)
{
	HANDLE			hProc		= NULL;
	DWORD			dwHandle	= NULL;
	PPGPMEMLOCKITEM	pmli		= NULL;
	PPGPMEMLOCKITEM	pPrev		= NULL;
	PPGPMEMLOCKITEM	pNext		= NULL;

	// get process handle of current process
	VxDCall (VWIN32_GetCurrentProcessHandle)
	_asm mov hProc, eax

	// get handle as used by calling app
	dwHandle = p->hDevice;

	// search list for block belonging to this process
	pPrev = NULL;
	pmli = g_pLockedList;
	while (pmli) 
	{
		if ((pmli->hProcess == hProc) &&
			(pmli->dwHandle == dwHandle))
		{
			if (pPrev)
			{
				pPrev->next = pmli->next;
			}
			else 
			{
				g_pLockedList = pmli->next;
			}
			pNext = pmli->next;
			PGPmlWipeBlock (pmli->ulPage, pmli->ulNumPages);
			_LinPageUnLock (pmli->ulPage, pmli->ulNumPages, 0);
			PGPmlFreeItem (pmli);
			pmli = pNext;
		}
		else 
		{
			pPrev = pmli;
			pmli = pmli->next;
		}
	}

	return TRUE;
}


//----------------------------------------------------|
//  unlock and remove all blocks in the list
//

BOOL
PGPmlUnlockAllBlocks ()
{
	PPGPMEMLOCKITEM	pmli	= NULL;
	PPGPMEMLOCKITEM	pNext	= NULL;

	pmli = g_pLockedList;
	while (pmli) 
	{
		pNext = pmli->next;
		PGPmlWipeBlock (pmli->ulPage, pmli->ulNumPages);
		_LinPageUnLock (pmli->ulPage, pmli->ulNumPages, 0);
		PGPmlFreeItem (pmli);
		pmli = pNext;
	}

	g_pLockedList = NULL;

	return TRUE;
}


//----------------------------------------------------|
//  lock the page(s) specified by structure

DWORD 
PGPmlLockPage (PDIOCPARAMETERS p) 
{
	PPGPMEMLOCKSTRUCT	pplsIn		= (PPGPMEMLOCKSTRUCT)(p->lpvInBuffer);
	PPGPMEMLOCKSTRUCT	pplsOut		= (PPGPMEMLOCKSTRUCT)(p->lpvOutBuffer);
	ULONG				ulPage		= 0;
	ULONG				ulNumPages	= 0;
	ULONG				ulError		= 0;

	PGPmlPrintf (("PGPmemlock: LockPage.\n"));

	// check buffer pointers
	if ((!pplsIn) || (!pplsOut))
	{
		return ERROR_INVALID_PARAMETER;
	}

	// check buffer lengths
	if ((p->cbInBuffer < sizeof(PGPMEMLOCKSTRUCT)) ||
		(p->cbOutBuffer < sizeof(PGPMEMLOCKSTRUCT)))
	{
		return ERROR_INVALID_PARAMETER;
	}

	// assume no error
	pplsOut->pMem = pplsIn->pMem;
	pplsOut->ulNumBytes = pplsIn->ulNumBytes;
	pplsOut->ulError = 0;

	// calculate page numbers
	ulPage = ((ULONG)(pplsIn->pMem)) >> WIN32PAGESIZE;
	ulNumPages = (((pplsIn->ulNumBytes)-1) >> WIN32PAGESIZE ) +1;
	PGPmlPrintf (("PGPmemlock: locking page(s) %x - %x.\n", 
					ulPage, ulPage+ulNumPages-1));

	// add the block to the list of locked pages
	if (PGPmlAddBlockToList (p, ulPage, ulNumPages))
	{
		// actually lock the page(s)
		ulError = _LinPageLock (ulPage, ulNumPages, 0);
		if (ulError == 0)	// error
		{
			PGPmlPrintf (("PGPmemlock: Err: _LinPageLock error.\n"));
			pplsOut->ulError = kPGPMemLockError_LockError;
			PGPmlRemoveBlockFromList (p, ulPage, ulNumPages);
		}
		else 
		{
			PGPmlPrintf (("PGPmemlock: successfully locked memory.\n"));
		}
	}
	else 
	{
		PGPmlPrintf (("PGPmemlock: Err: PGPmlAddBlockToList error.\n"));
		pplsOut->ulError = kPGPMemLockError_ListError;
	}

	return 0;
}   


//----------------------------------------------------|
//  unlock the page(s) specified by structure

DWORD 
PGPmlUnlockPage (PDIOCPARAMETERS p) 
{
	PPGPMEMLOCKSTRUCT	pplsIn		= (PPGPMEMLOCKSTRUCT)(p->lpvInBuffer);
	PPGPMEMLOCKSTRUCT	pplsOut		= (PPGPMEMLOCKSTRUCT)(p->lpvOutBuffer);
	ULONG				ulPage;
	ULONG				ulNumPages;
	ULONG				ulError;

	PGPmlPrintf (("PGPmemlock: UnlockPage.\n"));

	// check buffer pointers
	if ((!pplsIn) || (!pplsOut))
	{
		return ERROR_INVALID_PARAMETER;
	}

	// check buffer lengths
	if ((p->cbInBuffer < sizeof(PGPMEMLOCKSTRUCT)) ||
		(p->cbOutBuffer < sizeof(PGPMEMLOCKSTRUCT)))
	{
		return ERROR_INVALID_PARAMETER;
	}

	// assume no error
	pplsOut->pMem = pplsIn->pMem;
	pplsOut->ulNumBytes = pplsIn->ulNumBytes;
	pplsOut->ulError = 0;

	// calculate page numbers
	ulPage = ((ULONG)(pplsIn->pMem)) >> WIN32PAGESIZE;
	ulNumPages = (((pplsIn->ulNumBytes)-1) >> WIN32PAGESIZE ) +1;
	PGPmlPrintf (("PGPmemlock: unlocking page(s) %x - %x.\n", 
					ulPage, ulPage+ulNumPages-1));

	// remove the pages from the list of locked pages
	if (PGPmlRemoveBlockFromList (p, ulPage, ulNumPages))
	{
		// actually unlock the page(s)
		ulError = _LinPageUnLock (ulPage, ulNumPages, 0);
		if (ulError == 0)		// error
		{
			PGPmlPrintf (("PGPmemlock: Err: _LinPageUnLock error.\n"));
			pplsOut->ulError = kPGPMemLockError_UnlockError;
		}
		else 
		{
			PGPmlPrintf (("PGPmemlock: successfully unlocked memory.\n"));
		}
	}
	else 
	{
		PGPmlPrintf (("PGPmemlock: Err: PGPmlRemoveBlockFromList error.\n"));
		pplsOut->ulError = kPGPMemLockError_ListError;
	}

	return 0; 
}   


//----------------------------------------------------|
//  called upon initialization of VxD

BOOL 
OnSysDynamicDeviceInit () 
{ 
	PGPmlPrintf (("PGPmemlock: OnSysDynamicDeviceInit.\n"));

	// initialize critical section mutex
	g_mutexHeap = 
		_CreateMutex (CRITICAL_SECTION_BOOST, MUTEX_MUST_COMPLETE);
	if (g_mutexHeap == NULL)
		return FALSE;	// failure

	// initialize heap
	PGPmlEnterCriticalSection ();
	g_pLockedList = NULL;
	PGPmlHeapInit ();
	PGPmlLeaveCriticalSection ();

	return TRUE;    	// success
}


//----------------------------------------------------|
//  called upon shutdown of VxD

BOOL 
OnSysDynamicDeviceExit () 
{

	PGPmlPrintf (("PGPmemlock: OnSysDynamicDeviceExit.\n"));

	PGPmlEnterCriticalSection ();
	PGPmlUnlockAllBlocks ();
	PGPmlHeapDestroy ();
	PGPmlLeaveCriticalSection ();

	_DestroyMutex (g_mutexHeap);

    return TRUE;    // success
}   


//----------------------------------------------------|
//  called upon as result of call to DeviceIoControl

DWORD 
OnDeviceIoControl (PDIOCPARAMETERS p) 
{
	DWORD	dwReturn;

    switch (p->dwIoControlCode) {
    case DIOC_GETVERSION :
        dwReturn = 0;	// no error
		break;

	case IOCTL_PGPMEMLOCK_LOCK_MEMORY :
		PGPmlEnterCriticalSection ();
		dwReturn = PGPmlLockPage (p);
		PGPmlLeaveCriticalSection ();
		break;

	case IOCTL_PGPMEMLOCK_UNLOCK_MEMORY :
		PGPmlEnterCriticalSection ();
		dwReturn = PGPmlUnlockPage (p);
		PGPmlLeaveCriticalSection ();
		break;

	case DIOC_CLOSEHANDLE :
		PGPmlPrintf (("PGPmemlock: DIOC_CLOSEHANDLE.\n"));
		PGPmlEnterCriticalSection ();
		PGPmlUnlockHandleBlocks (p);
		PGPmlLeaveCriticalSection ();
		dwReturn = 0;	// no error
		break;

    default :
		PGPmlPrintf (("PGPmemlock: Err: unknown IoControlCode: %X.\n",
						p->dwIoControlCode));
		dwReturn = ERROR_BAD_COMMAND;
    }

	return dwReturn;
}

