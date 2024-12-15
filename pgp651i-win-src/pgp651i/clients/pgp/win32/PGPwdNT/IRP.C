/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	

	$Id: IRP.C,v 1.3 1999/05/11 06:57:13 wjb Exp $
____________________________________________________________________________*/
#include "ntddk.h"
#include "PGPwdNT.h"

//----------------------------------------------------------------------
//             I R P    R E L A T E D   F U N C T I O N S 
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//
// PGPWDNTIoComplete
//
// This routine is used to handle I/O (read OR write) completion.
//
//----------------------------------------------------------------------
static NTSTATUS PGPWDNTIoComplete(PDEVICE_OBJECT DeviceObject,
				    PIRP Irp,
				    PVOID Context)
{
    //
    // Copy the status information back into the "user" IOSB.
    //
    *Irp->UserIosb = Irp->IoStatus;

    if( !NT_SUCCESS(Irp->IoStatus.Status) ) {

        DbgPrint(("   ERROR ON IRP: %x\n", Irp->IoStatus.Status ));
    }
    
    //
    // Set the user event - wakes up the mainline code doing this.
    //
    KeSetEvent(Irp->UserEvent, 0, FALSE);
    
    //
    // Free the IRP now that we are done with it.
    //
    IoFreeIrp(Irp);
    
    //
    // We return STATUS_MORE_PROCESSING_REQUIRED because this "magic" return value
    // tells the I/O Manager that additional processing will be done by this driver
    // to the IRP - in fact, it might (as it is in this case) already BE done - and
    // the IRP cannot be completed.
    //
    return STATUS_MORE_PROCESSING_REQUIRED;
}


//----------------------------------------------------------------------
//
// PGPWDNTIsDirectory
//
// Returns TRUE if its a directory or we can't tell, FALSE if its a file.
//
//----------------------------------------------------------------------
BOOLEAN PGPWDNTIsDirectory(PDEVICE_OBJECT DeviceObject, 
                            PFILE_OBJECT FileObject )
{
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIO_STACK_LOCATION ioStackLocation;
    FILE_STANDARD_INFORMATION fileInfo;

    //
    // First, start by initializing the event
    //
    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    //
    // Allocate an irp for this request.  This could also come from a 
    // private pool, for instance.
    //
    irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (!irp) {

        //
        // Failure!
        //
        return FALSE;
    }

    irp->AssociatedIrp.SystemBuffer = &fileInfo;

    irp->UserEvent = &event;

    irp->UserIosb = &IoStatusBlock;

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

    irp->Tail.Overlay.OriginalFileObject = FileObject;

    irp->RequestorMode = KernelMode;
  
    irp->Flags = 0;

    //
    // Set up the I/O stack location.
    //
    ioStackLocation = IoGetNextIrpStackLocation(irp);

    ioStackLocation->MajorFunction = IRP_MJ_QUERY_INFORMATION;

    ioStackLocation->DeviceObject = DeviceObject;

    ioStackLocation->FileObject = FileObject;

    ioStackLocation->Parameters.QueryVolume.Length = sizeof(FILE_STANDARD_INFORMATION);
    
    ioStackLocation->Parameters.QueryVolume.FsInformationClass = FileStandardInformation;

    //
    // Set the completion routine.
    //
    IoSetCompletionRoutine(irp, PGPWDNTIoComplete, 0, TRUE, TRUE, TRUE);

    //
    // Send the request to the lower layer driver.
    //
    (void) IoCallDriver(DeviceObject, irp);

    //
    // Wait for the I/O
    //
    KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, 0);

    //
    // Return whether its a directory or not
    //
    if( !NT_SUCCESS( irp->IoStatus.Status ) || fileInfo.Directory) {

        return TRUE;

    } else {

        return FALSE; 
    }
}


//----------------------------------------------------------------------
//
// PGPWDNTGetFileName
//
// This function retrieves the "standard" information for the
// underlying file system, asking for the filename in particular.
//
//----------------------------------------------------------------------
BOOLEAN PGPWDNTGetFileName(BOOLEAN IsNTFS,
                            PDEVICE_OBJECT DeviceObject, 
                            PFILE_OBJECT FileObject,
                            PUCHAR FileName, ULONG FileNameLength )
{
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIO_STACK_LOCATION ioStackLocation;
    PVOID fsContext2;

    DbgPrint(("Getting file name for %x\n", FileObject));

    //
    // Initialize the event
    //
    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    //
    // Allocate an irp for this request.  This could also come from a 
    // private pool, for instance.
    //
    irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (!irp) {

        //
        // Failure!
        //
        return FALSE;
    }
  
    //
    // Zap Fscontext2 (the CCB) so that NTFS will give us the long name
    //
    if( IsNTFS ) {

        fsContext2 = FileObject->FsContext2;
  
        FileObject->FsContext2 = NULL;
    }

    irp->AssociatedIrp.SystemBuffer = FileName;

    irp->UserEvent = &event;

    irp->UserIosb = &IoStatusBlock;

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

    irp->Tail.Overlay.OriginalFileObject = FileObject;

    irp->RequestorMode = KernelMode;
  
    irp->Flags = 0;

    //
    // Set up the I/O stack location.
    //
    ioStackLocation = IoGetNextIrpStackLocation(irp);

    ioStackLocation->MajorFunction = IRP_MJ_QUERY_INFORMATION;

    ioStackLocation->DeviceObject = DeviceObject;

    ioStackLocation->FileObject = FileObject;

    ioStackLocation->Parameters.QueryFile.Length = FileNameLength;
    
    ioStackLocation->Parameters.QueryFile.FileInformationClass = FileNameInformation;

    //
    // Set the completion routine.
    //
    IoSetCompletionRoutine(irp, PGPWDNTIoComplete, 0, TRUE, TRUE, TRUE);

    //
    // Send it to the FSD
    //
    (void) IoCallDriver(DeviceObject, irp);

    //
    // Wait for the I/O
    //
    KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, 0);

    //
    // Restore the fscontext
    //
    if( IsNTFS ) {

        FileObject->FsContext2 = fsContext2;
    }

    //
    // Done!
    //
    return( NT_SUCCESS( irp->IoStatus.Status ));
}

//----------------------------------------------------------------------
//
// PGPWDNTSetDispositionFile
//
// Changes the delete status on a file
//
//----------------------------------------------------------------------
BOOLEAN PGPWDNTSetDispositionFile(PDEVICE_OBJECT DeviceObject, 
			           PFILE_OBJECT FileObject, BOOLEAN Delete )
{
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK IoStatusBlock;
    PIO_STACK_LOCATION ioStackLocation;
    FILE_DISPOSITION_INFORMATION disposition;

    //
    // Change the delete status
    //
    disposition.DeleteFile = Delete;

    //
    // Initialize the event
    //
    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    //
    // Allocate an irp for this request.  This could also come from a 
    // private pool, for instance.
    //
    irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (!irp) {

        //
        // Failure!
        //
        return FALSE;
    }

    irp->AssociatedIrp.SystemBuffer = &disposition;

    irp->UserEvent = &event;

    irp->UserIosb = &IoStatusBlock;

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

    irp->Tail.Overlay.OriginalFileObject = FileObject;

    irp->RequestorMode = KernelMode;
  
    irp->Flags = 0;

    //
    // Set up the I/O stack location.
    //
    ioStackLocation = IoGetNextIrpStackLocation(irp);

    ioStackLocation->MajorFunction = IRP_MJ_SET_INFORMATION;

    ioStackLocation->DeviceObject = DeviceObject;

    ioStackLocation->FileObject = FileObject;

    ioStackLocation->Parameters.SetFile.FileInformationClass = FileDispositionInformation;

    //
    // Set the completion routine.
    //
    IoSetCompletionRoutine(irp, PGPWDNTIoComplete, 0, TRUE, TRUE, TRUE);

    //
    // Send it to the FSD
    //
    (void) IoCallDriver(DeviceObject, irp);

    //
    // Wait for the I/O
    //
    KeWaitForSingleObject(&event, Executive, KernelMode, TRUE, 0);

    //
    // Done!
    //
    return TRUE;

}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
