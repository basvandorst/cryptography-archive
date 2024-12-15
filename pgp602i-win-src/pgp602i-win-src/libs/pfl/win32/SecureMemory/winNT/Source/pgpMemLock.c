/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPmemlock.c - PGPmemlock kernel mode device driver for locking memory
	

	$Id: pgpMemLock.c,v 1.5 1998/08/11 13:43:55 pbj Exp $
____________________________________________________________________________*/
/*____________________________________________________________________________

	This kernel mode device driver is used to lock memory pages into RAM 
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


#include "ntddk.h"
#include "stdarg.h"

#define PGP_WIN32	1
#include "pgpMemLockWin32.h"

// constants
//	number of list items that can be held in internal heap
#define NUMITEMSPERBLOCK	64

// virtual memory paging in Win32 uses 4K byte pages (2^12) 
#define WIN32PAGESIZE		12	

// typedefs
//	elements of list used to keep track of locked blocks
typedef struct _MEMLOCKITEM {
	struct _MEMLOCKITEM* next;
	PEPROCESS		peProcess;
	PFILE_OBJECT	pfo;
	ULONG			ulPage;
	ULONG			ulNumPages;
	PMDL			pmdl;
} PGPMEMLOCKITEM, *PPGPMEMLOCKITEM;

//	block of such items
typedef struct _MEMLOCKBLOCK {
	struct _MEMLOCKBLOCK*	nextBlock;
	PGPMEMLOCKITEM			item[NUMITEMSPERBLOCK];
} PGPMEMLOCKBLOCK, *PPGPMEMLOCKBLOCK;

//	device object extension data structure
typedef struct _DEVEXTENSION {
	KSPIN_LOCK		slListLock;
	PPGPMEMLOCKITEM	pLockedList;
} DEVEXTENSION, *PDEVEXTENSION;


// macros
//	DbgPrint prints a message to the kernel debugging console
#if DBG
#define PGPmlPrint(arg) DbgPrint arg
#else
#define PGPmlPrint(arg)
#endif


// globals
//	pointer to local heap
static PPGPMEMLOCKBLOCK	g_pHeapBlocks;

//	pointer to head of free list
static PPGPMEMLOCKITEM	g_pFreeHeapItems;



//----------------------------------------------------|
//  internal item allocation routines
//

VOID
PGPmlHeapBlockInit (IN PPGPMEMLOCKBLOCK pmlb)
{
	ULONG			i		= 0;
	PPGPMEMLOCKITEM	pmli	= NULL;

	pmlb->nextBlock = NULL;

	pmli = &(pmlb->item[0]);

	for (i=1; i<NUMITEMSPERBLOCK; i++)
	{
		pmli->next = &(pmlb->item[i]);
		pmli = pmli->next;
	}
	pmli->next = NULL;
}

NTSTATUS
PGPmlHeapInit ()
{
	ULONG			i		= 0;
	PPGPMEMLOCKITEM	pmli	= NULL;

	g_pHeapBlocks = ExAllocatePool (NonPagedPool, 
								    sizeof(PGPMEMLOCKBLOCK));

	if (g_pHeapBlocks) 
	{
		PGPmlHeapBlockInit (g_pHeapBlocks);
		g_pFreeHeapItems = &(g_pHeapBlocks->item[0]);
		return STATUS_SUCCESS;
	}
	else return STATUS_NO_MEMORY;
}

NTSTATUS
PGPmlHeapDestroy ()
{
	PPGPMEMLOCKBLOCK	pmlb		= NULL;
	PPGPMEMLOCKBLOCK	pmlbNext	= NULL;

	pmlb = g_pHeapBlocks;
	
	while (pmlb) 
	{
		pmlbNext = pmlb->nextBlock;
		ExFreePool (pmlb);
		pmlb = pmlbNext;
	}

	g_pFreeHeapItems = NULL;

	return STATUS_SUCCESS;
}

PPGPMEMLOCKITEM
PGPmlAllocateItem ()
{
	PPGPMEMLOCKITEM		pmli		= NULL;
	PPGPMEMLOCKBLOCK	pmlb		= NULL;
	PPGPMEMLOCKBLOCK	pmlbNew		= NULL;
	ULONG				i			= 0;

	// there is an item available
	if (g_pFreeHeapItems)
	{
		// remove item from free list
		pmli = g_pFreeHeapItems;
		g_pFreeHeapItems = g_pFreeHeapItems->next;

		return pmli;
	}

	// free list is empty, allocate more memory
	pmlbNew = ExAllocatePool (NonPagedPool, 
							  sizeof(PGPMEMLOCKBLOCK));
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
PGPmlFreeItem (IN PPGPMEMLOCKITEM pmli)
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
	IN ULONG			ulPage,
	IN ULONG			ulNumPages)
{
	PVOID	pmem;
	ULONG	ulBytes;

	pmem = (PVOID)(ulPage << WIN32PAGESIZE);
	ulBytes = ulNumPages << WIN32PAGESIZE;

	memset (pmem, 0, ulBytes);
}


//----------------------------------------------------|
//  add the specified memory block to list of blocks
//
//	Arguments:
//		pDevO			- pointer to a device object
//		ulPage			- initial page being locked
//		ulNumPages		- number of pages in block being locked
//		pmdl			- pointer to MDL used to lock pages
//

NTSTATUS
PGPmlAddBlockToList (
    IN PDEVICE_OBJECT	pDevO,
    IN PIRP 			pIrp,
	IN ULONG			ulPage,
	IN ULONG			ulNumPages,
	IN PMDL				pmdl)
{
	PPGPMEMLOCKITEM		pmli		= NULL;
	PDEVEXTENSION		pde			= NULL;
	PEPROCESS			peProcess	= NULL;
	PFILE_OBJECT		pfo			= NULL;
	NTSTATUS			ntStatus	= STATUS_NO_MEMORY;
	KIRQL				irql;

	peProcess = PsGetCurrentProcess ();
	pfo = IoGetCurrentIrpStackLocation (pIrp)->FileObject;
	pde = (PDEVEXTENSION)(pDevO->DeviceExtension);

	KeAcquireSpinLock (&pde->slListLock, &irql);

	// allocate new item
	pmli = PGPmlAllocateItem ();
	if (pmli) 
	{
		// insert block information into item
		pmli->peProcess = peProcess;
		pmli->pfo = pfo;
		pmli->ulPage = ulPage; 
		pmli->ulNumPages = ulNumPages; 
		pmli->pmdl = pmdl;

		// insert item into list
		pmli->next = pde->pLockedList;
		pde->pLockedList = pmli;

		ntStatus = STATUS_SUCCESS;
	}

	KeReleaseSpinLock (&pde->slListLock, irql);

	return ntStatus;
}


//----------------------------------------------------|
//  remove the specified memory block from list of 
//	locked blocks and unlock the memory
//
//	Arguments:
//		pDevO			- pointer to a device object
//		ulPage			- initial page to unlock
//		ulNumPages		- number of pages in block to unlock
//		bUnlock			- TRUE=>unlock and remove from list
//						  FALSE=>remove from list only
//

NTSTATUS
PGPmlUnlockBlock (
    IN PDEVICE_OBJECT	pDevO,
    IN PIRP 			pIrp,
	IN ULONG			ulPage,
	IN ULONG			ulNumPages,
	IN ULONG			bUnlock)
{
	PDEVEXTENSION		pde			= NULL;
	PMDL				pmdl		= NULL;
	PEPROCESS			peProcess	= NULL;
	PFILE_OBJECT		pfo			= NULL;
	PPGPMEMLOCKITEM		pmli		= NULL;
	PPGPMEMLOCKITEM		pPrev		= NULL;
	NTSTATUS			ntStatus	= STATUS_NOT_LOCKED;
	KIRQL				irql;

	peProcess = PsGetCurrentProcess ();
	pfo = IoGetCurrentIrpStackLocation (pIrp)->FileObject;
	pde = (PDEVEXTENSION)(pDevO->DeviceExtension);

	KeAcquireSpinLock (&pde->slListLock, &irql);

	// search list for specified block
	pPrev = NULL;
	pmli = pde->pLockedList;
	while (pmli) 
	{
		if ((pmli->peProcess == peProcess) &&
			(pmli->pfo == pfo))
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
					pde->pLockedList = pmli->next;
				}
				pmdl = pmli->pmdl;
				PGPmlFreeItem (pmli);
				ntStatus = STATUS_SUCCESS;
				break;
			}
		}
		pPrev = pmli;
		pmli = pmli->next;
	}

	KeReleaseSpinLock (&pde->slListLock, irql);

	// unlock pages and discard MDL
	if (bUnlock && pmdl) 
	{
		PGPmlWipeBlock (ulPage, ulNumPages);
		MmUnlockPages(pmdl);
		IoFreeMdl(pmdl);
	}

	return ntStatus;
}


//----------------------------------------------------|
//  remove all of this handle's memory blocks from 
//	list of blocks and unlock the memory
//
//	Arguments:
//		pDevO				- pointer to a device object
//

VOID
PGPmlUnlockHandleBlocks (
    IN PDEVICE_OBJECT	pDevO,
    IN PIRP 			pIrp)
{
	PDEVEXTENSION		pde			= NULL;
	PEPROCESS			peProcess	= NULL;
	PFILE_OBJECT		pfo			= NULL;
	PPGPMEMLOCKITEM		pmli		= NULL;
	PPGPMEMLOCKITEM		pPrev		= NULL;
	PPGPMEMLOCKITEM		pNext		= NULL;
	KIRQL				irql;

	peProcess = PsGetCurrentProcess ();
	pfo = IoGetCurrentIrpStackLocation (pIrp)->FileObject;
	pde = (PDEVEXTENSION)(pDevO->DeviceExtension);

	// first wipe any blocks belonging to this handle
	pmli = pde->pLockedList;
	while (pmli) 
	{
		if ((pmli->peProcess == peProcess) &&
			(pmli->pfo == pfo))
		{
			PGPmlWipeBlock (pmli->ulPage, pmli->ulNumPages);
		}
		pmli = pmli->next;
	}

	KeAcquireSpinLock (&pde->slListLock, &irql);

	// search list for block belonging to this handle
	pPrev = NULL;
	pmli = pde->pLockedList;
	while (pmli) 
	{
		if ((pmli->peProcess == peProcess) &&
			(pmli->pfo == pfo))
		{
			if (pPrev)
			{
				pPrev->next = pmli->next;
			}
			else 
			{
				pde->pLockedList = pmli->next;
			}
			pNext = pmli->next;
			MmUnlockPages (pmli->pmdl);
			IoFreeMdl (pmli->pmdl);
			PGPmlFreeItem (pmli);
			pmli = pNext;
		}
		else 
		{
			pPrev = pmli;
			pmli = pmli->next;
		}
	}

	KeReleaseSpinLock (&pde->slListLock, irql);
}


//----------------------------------------------------|
//  remove all locked memory blocks from 
//	list of blocks and unlock the memory
//
//	Arguments:
//		pDevO				- pointer to a device object
//

VOID
PGPmlUnlockAllBlocks (
    IN PDEVICE_OBJECT	pDevO)
{
	PDEVEXTENSION		pde			= NULL;
	PPGPMEMLOCKITEM		pmli		= NULL;
	PPGPMEMLOCKITEM		pNext		= NULL;
	KIRQL				irql;

	pde = (PDEVEXTENSION)(pDevO->DeviceExtension);

	// first wipe all blocks 
	pmli = pde->pLockedList;
	while (pmli) 
	{
		PGPmlWipeBlock (pmli->ulPage, pmli->ulNumPages);
		pmli = pmli->next;
	}

	KeAcquireSpinLock (&pde->slListLock, &irql);

	// now unlock blocks and destroy list
	pmli = pde->pLockedList;
	while (pmli) 
	{
		pNext = pmli->next;
		MmUnlockPages (pmli->pmdl);
		IoFreeMdl (pmli->pmdl);
		PGPmlFreeItem (pmli);
		pmli = pNext;
	}

	pde->pLockedList = NULL;

	KeReleaseSpinLock (&pde->slListLock, irql);
}


//----------------------------------------------------|
//  lock the pages in the specified virtual address range
//
//	Arguments:
//		pDevO				- pointer to a device object
//		IoBuffer			- pointer to the I/O buffer
//		InputBufferLength	- input buffer length
//		OutputBufferLength	- output buffer length
//
//	Return Value:
//		STATUS_SUCCESS if sucessful, otherwise
//		STATUS_*
//

NTSTATUS
PGPmlLockTheMemory(
    IN PDEVICE_OBJECT pDevO,
	IN PIRP			  pIrp,
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength)
{
    PPGPMEMLOCKSTRUCT	ppmls			= (PPGPMEMLOCKSTRUCT) IoBuffer;
    NTSTATUS			ntStatus		= 0;
	PMDL				pmdl			= NULL;
	ULONG				ulPage			= 0;
	ULONG				ulNumPages		= 0;

	PGPmlPrint (("PGPmemlock: PGPmlLockTheMemory.\n"));

	if ((InputBufferLength < sizeof (PGPMEMLOCKSTRUCT)) ||
		(OutputBufferLength < sizeof (PGPMEMLOCKSTRUCT)) ||
		(IoBuffer == NULL))
    {
       PGPmlPrint (("PGPmemlock: Err: Invalid input parameters.\n"));
       ntStatus = STATUS_INVALID_PARAMETER;
       goto done;
    }

	// assume no error
	ppmls->ulError = 0;

	// allocate and initialize a Memory Descriptor List 
	pmdl = IoAllocateMdl(ppmls->pMem, ppmls->ulNumBytes, FALSE, FALSE, NULL); 
	if (pmdl == NULL) {
		PGPmlPrint (("PGPmemlock: Err: IoAllocateMdl failed.\n"));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		ppmls->ulError = kPGPMemLockError_AllocError;
		goto done;
	}

	// calculate page numbers
	ulPage = ((ULONG)(ppmls->pMem)) >> WIN32PAGESIZE;
	ulNumPages = (((ppmls->ulNumBytes)-1) >> WIN32PAGESIZE ) +1;
	PGPmlPrint (("PGPmemlock: locking page(s) %x - %x.\n", 
						ulPage, ulPage+ulNumPages-1));

	// add this block to the array of locked blocks
	ntStatus = PGPmlAddBlockToList (pDevO, pIrp, ulPage, ulNumPages, pmdl);
	if (ntStatus != STATUS_SUCCESS) 
	{
		// we couldn't allocate a new list item
		PGPmlPrint (("PGPmemlock: Err: PGPmlAddBlockToList failed.\n"));
		ppmls->ulError = kPGPMemLockError_ListError;

		// free the mdl we allocated,
		IoFreeMdl(pmdl);
		goto done;
	}

	// try to lock down the client's area
	__try {
		MmProbeAndLockPages(pmdl, UserMode, IoModifyAccess);
	} __except(EXCEPTION_EXECUTE_HANDLER) {
		// client must have passed a bad address, or it exceeded its quota
		PGPmlPrint (("PGPmemlock: Err: MmProbeAndLockPages failed.\n"));
		ntStatus = GetExceptionCode();
		ppmls->ulError = kPGPMemLockError_LockError;

		// remove block from array and free the MDL we allocated,
		PGPmlUnlockBlock (pDevO, pIrp, ulPage, ulNumPages, FALSE);
		IoFreeMdl(pmdl);
		goto done;
	}

done:
    return ntStatus;
}


//----------------------------------------------------|
//  unlock the pages in the specified virtual address range
//
//	Arguments:
//		pDevO				- pointer to a device object
//		IoBuffer			- pointer to the I/O buffer
//		InputBufferLength	- input buffer length
//		OutputBufferLength	- output buffer length
//
//	Return Value:
//		STATUS_SUCCESS if sucessful, otherwise
//		STATUS_* 
//

NTSTATUS
PGPmlUnlockTheMemory(
    IN PDEVICE_OBJECT	pDevO,
	IN PIRP				pIrp,
    IN OUT PVOID		IoBuffer,
    IN ULONG			InputBufferLength,
    IN ULONG			OutputBufferLength)
{
	PPGPMEMLOCKSTRUCT	ppmls			= (PPGPMEMLOCKSTRUCT) IoBuffer;
	NTSTATUS			ntStatus		= STATUS_SUCCESS;
	ULONG				ulPage			= 0;
	ULONG				ulNumPages		= 0;

	PGPmlPrint (("PGPmemlock: PGPmlUnlockTheMemory.\n"));

	if ((InputBufferLength < sizeof (PGPMEMLOCKSTRUCT)) ||
		(OutputBufferLength < sizeof (PGPMEMLOCKSTRUCT)) ||
		(IoBuffer == NULL))
	{
		PGPmlPrint (("PGPmemlock: Err: Invalid input parameters.\n"));
		ntStatus = STATUS_INVALID_PARAMETER;
    }
	else 
	{
		// assume no error
		ppmls->ulError = 0;

		// calculate page numbers
		ulPage = ((ULONG)(ppmls->pMem)) >> WIN32PAGESIZE;
		ulNumPages = (((ppmls->ulNumBytes)-1) >> WIN32PAGESIZE ) +1;
		PGPmlPrint (("PGPmemlock: unlocking page(s) %x - %x.\n", 
						ulPage, ulPage+ulNumPages-1));

		// unlock the pages 
		ntStatus = PGPmlUnlockBlock (pDevO, pIrp, ulPage, ulNumPages, TRUE);
		if (ntStatus != STATUS_SUCCESS) 
		{
			// we couldn't unlock the memory
			PGPmlPrint (("PGPmemlock: Err: PGPmlUnlockBlock failed.\n"));
			ppmls->ulError = kPGPMemLockError_UnlockError;
		}
	}

	return ntStatus;
}


//----------------------------------------------------|
//  delete the associated device
//
//	Arguments:
//		pDrvO		- pointer to a driver object
//

VOID
PGPmlUnload(
    IN PDRIVER_OBJECT pDrvO)
{ 
    WCHAR           deviceLinkBuffer[]  = L"\\DosDevices\\PGPMEMLOCK";
    UNICODE_STRING	deviceLinkUnicodeString;
	PDEVICE_OBJECT	pDevO				= NULL;

	// get first (and only) device object
	pDevO = pDrvO->DeviceObject;

	// Unlock all blocks referenced in list and free list
	if (KeGetCurrentIrql () < DISPATCH_LEVEL) 
	{
		PGPmlUnlockAllBlocks (pDevO);
	}

    // Delete the symbolic link
    RtlInitUnicodeString (&deviceLinkUnicodeString,
                          deviceLinkBuffer);
    IoDeleteSymbolicLink (&deviceLinkUnicodeString);

    // Delete the device object
    PGPmlPrint (("PGPmemlock: unloading.\n"));
    IoDeleteDevice (pDrvO->DeviceObject);

	// Close down our item allocation system
	PGPmlHeapDestroy (); 
}


//----------------------------------------------------|
//  process the IRPs sent to this device
//
//	Arguments:
//		pDevO		- pointer to a device object
//		pIrp		- pointer to an I/O Request Packet
//
//	Return Value:
//		STATUS_SUCCESS if successful,
//		STATUS_* otherwise
//

NTSTATUS
PGPmlDispatch(
    IN PDEVICE_OBJECT pDevO,
    IN PIRP pIrp)
{
    PIO_STACK_LOCATION pIrpStack			= NULL;
    PVOID              ioBuffer				= NULL;
    ULONG              inBufferLength		= 0;
    ULONG              outBufferLength		= 0;
    ULONG              ioControlCode		= 0;
    NTSTATUS           ntStatus				= STATUS_SUCCESS;

    // Init to default settings
    pIrp->IoStatus.Status      = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    // Get a pointer to the current location in the Irp. This is where
    //     the function codes and parameters are located.
    pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

    // Get the pointer to the input/output buffer and it's length
    ioBuffer        = 
				pIrp->AssociatedIrp.SystemBuffer;
    inBufferLength  = 
				pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
    outBufferLength = 
				pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (pIrpStack->MajorFunction)
    {
	// called when user app calls CreateFile
    case IRP_MJ_CREATE:
        PGPmlPrint (("PGPmemlock: IRP_MJ_CREATE\n"));
        break;

	// called when user app closes handle to driver --
	//	make sure all locked memory belonging to this handle gets unlocked
    case IRP_MJ_CLOSE:
		PGPmlPrint (("PGPmemlock: IRP_MJ_CLOSE\n"));
		if (KeGetCurrentIrql () < DISPATCH_LEVEL)
		{
			PGPmlUnlockHandleBlocks (pDevO, pIrp);
		}
		break;

	// called when user apps calls DeviceIoControl
    case IRP_MJ_DEVICE_CONTROL:
        ioControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;

        switch (ioControlCode)
        {
		// user app wants to lock some memory ...
        case IOCTL_PGPMEMLOCK_LOCK_MEMORY:

			// try locking it
            pIrp->IoStatus.Status = PGPmlLockTheMemory (pDevO,
														pIrp,
                                                        ioBuffer,
                                                        inBufferLength,
                                                        outBufferLength);

            if (NT_SUCCESS(pIrp->IoStatus.Status))
            {
                pIrp->IoStatus.Information = sizeof(PGPMEMLOCKSTRUCT);
                PGPmlPrint (("PGPmemlock: memory successfully locked.\n"));
            }
            else
            {
                PGPmlPrint (("PGPmemlock: Err: memory lock failed.\n"));
            }
            break;

		// user app wants to unlock some memory
        case IOCTL_PGPMEMLOCK_UNLOCK_MEMORY:

			// try unlocking it ...
            pIrp->IoStatus.Status = PGPmlUnlockTheMemory (pDevO,
														  pIrp,
                                         				  ioBuffer,
                                                          inBufferLength,
                                                          outBufferLength);

            if (NT_SUCCESS(pIrp->IoStatus.Status))
            {
               pIrp->IoStatus.Information = sizeof(PGPMEMLOCKSTRUCT);
               PGPmlPrint (("PGPmemlock: memory successfully unlocked.\n"));
            }
            else
            {
                PGPmlPrint (("PGPmemlock: Err: memory unlock failed.\n"));
            }
            break;

        default:
            pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            PGPmlPrint (
					("PGPmemlock: Err: unknown IRP_MJ_DEVICE_CONTROL.\n"));
            break;
        }

        break;
    }

    ntStatus = pIrp->IoStatus.Status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return ntStatus;
}


//----------------------------------------------------|
//  called upon initialization of driver
//
//	Routine Description:
//
//	Installable driver initialization entry point.
//	This entry point is called directly by the I/O system.
//
//	Arguments:
//		pDrvO		 -	pointer to the driver object
//		RegistryPath -	pointer to a unicode string representing the path
//						to driver-specific key in the registry
//
//	Return Value:
//		STATUS_SUCCESS if successful,
//		STATUS_* otherwise
//
	
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  pDrvO,
    IN PUNICODE_STRING RegistryPath)
{

    PDEVICE_OBJECT	pDevO				= NULL;
    WCHAR			deviceNameBuffer[]	= L"\\Device\\PGPmemlock";
    UNICODE_STRING	deviceNameUnicodeString;
    WCHAR			deviceLinkBuffer[]	= L"\\DosDevices\\PGPMEMLOCK";
    UNICODE_STRING	deviceLinkUnicodeString;
    NTSTATUS		ntStatus			= STATUS_SUCCESS;
	PDEVEXTENSION	pde					= NULL;

    PGPmlPrint (("PGPmemlock: entering DriverEntry.\n"));

	// Initialize internal list item allocation routines
	ntStatus = PGPmlHeapInit ();
	if (!NT_SUCCESS (ntStatus)) 
	{
		PGPmlPrint (("PGPmemlock: Err: PGPmlHeapInit failed.\n"));
		return ntStatus;
	}

    // Create an NON exclusive device object (multiple threads
    // can make requests to this device at a time)
    RtlInitUnicodeString (&deviceNameUnicodeString,
                          deviceNameBuffer);

	ntStatus = IoCreateDevice (pDrvO,
                               sizeof(DEVEXTENSION),
                               &deviceNameUnicodeString,
                               FILE_DEVICE_PGPMEMLOCK,
                               0,
                               FALSE,
                               &pDevO);

    if (NT_SUCCESS(ntStatus))
    {
        // Create dispatch points for device control, create, close.
        pDrvO->MajorFunction[IRP_MJ_CREATE]         =
        pDrvO->MajorFunction[IRP_MJ_CLOSE]          =
        pDrvO->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PGPmlDispatch;
        pDrvO->DriverUnload                         = PGPmlUnload;

        // Create a symbolic link, e.g. a name that a Win32 app can specify
        // to open the device
        RtlInitUnicodeString(&deviceLinkUnicodeString,
                             deviceLinkBuffer);
        ntStatus = IoCreateSymbolicLink (&deviceLinkUnicodeString,
                                         &deviceNameUnicodeString);

        if (!NT_SUCCESS(ntStatus))
        {
            // Symbolic link creation failed- note this & then delete the
            // device object (it's useless if a Win32 app can't get at it).
            PGPmlPrint (("PGPmemlock: Err: IoCreateSymbolicLink failed.\n"));
            IoDeleteDevice (pDevO);
        }
		else 
		{
			// initialize the device extension data structure
			pde = (PDEVEXTENSION)(pDevO->DeviceExtension);
			memset (pde, 0, sizeof(DEVEXTENSION));
			KeInitializeSpinLock (&pde->slListLock);
		}
    }
    else
    {
        PGPmlPrint (("PGPmemlock: Err: IoCreateDevice failed.\n"));
    }

    return ntStatus;
}


