/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	

	$Id: PGPwdNT.c,v 1.3 1999/05/11 06:57:14 wjb Exp $
____________________________________________________________________________*/
#include "ntddk.h"
#include "PGPwdNT.h"
#include "..\include\PGPwd.h"

//----------------------------------------------------------------------
//                         FORWARD DEFINES
//---------------------------------------------------------------------- 

//
// Undocumented ntoskrnl variable
//
extern PSHORT           NtBuildNumber;

//
// File name information 
//
typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

NTSTATUS PGPWDNTDispatch( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );

//
// These are prototypes for PGPWDNT's Fast I/O hooks. The originals 
// prototypes can be found in NTDDK.H
// 
BOOLEAN  PGPWDNTFastIoCheckifPossible( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, 
                                        IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN BOOLEAN CheckForReadOperation,
                                        OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoRead( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, 
                             IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, OUT PVOID Buffer,
                             OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoWrite( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, 
                              IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN PVOID Buffer,
                              OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoQueryBasicInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, 
                                       OUT PFILE_BASIC_INFORMATION Buffer,
                                       OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoQueryStandardInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, 
                                          OUT PFILE_STANDARD_INFORMATION Buffer,
                                          OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoLock( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset,
                             IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key,
                             BOOLEAN FailImmediately, BOOLEAN ExclusiveLock,
                             OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoUnlockSingle( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset,
                                     IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key,
                                     OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoUnlockAll( IN PFILE_OBJECT FileObject, PEPROCESS ProcessId,
                                  OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoUnlockAllByKey( IN PFILE_OBJECT FileObject, PEPROCESS ProcessId, ULONG Key,
                                       OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoDeviceControl( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait,
                                      IN PVOID InputBuffer, IN ULONG InputBufferLength, 
                                      OUT PVOID OutbufBuffer, IN ULONG OutputBufferLength, IN ULONG IoControlCode,
                                      OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject );
VOID     PGPWDNTFastIoAcquireFile( PFILE_OBJECT FileObject );
VOID     PGPWDNTFastIoReleaseFile( PFILE_OBJECT FileObject );
VOID     PGPWDNTFastIoDetachDevice( PDEVICE_OBJECT SourceDevice, PDEVICE_OBJECT TargetDevice );

//
// These are new NT 4.0 Fast I/O calls
//
BOOLEAN  PGPWDNTFastIoQueryNetworkOpenInfo(IN PFILE_OBJECT FileObject,
                                            IN BOOLEAN Wait, OUT struct _FILE_NETWORK_OPEN_INFORMATION *Buffer,
                                            OUT struct _IO_STATUS_BLOCK *IoStatus, IN PDEVICE_OBJECT DeviceObject );
NTSTATUS PGPWDNTFastIoAcquireForModWrite( IN PFILE_OBJECT FileObject,
                                           IN PLARGE_INTEGER EndingOffset, OUT struct _ERESOURCE **ResourceToRelease,
                                           IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoMdlRead( IN PFILE_OBJECT FileObject,
                                IN PLARGE_INTEGER FileOffset, IN ULONG Length,
                                IN ULONG LockKey, OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus,
                                IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoMdlReadComplete( IN PFILE_OBJECT FileObject,
                                        IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoPrepareMdlWrite( IN PFILE_OBJECT FileObject,
                                        IN PLARGE_INTEGER FileOffset, IN ULONG Length, IN ULONG LockKey,
                                        OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus,
                                        IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoMdlWriteComplete( IN PFILE_OBJECT FileObject,
                                         IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain,
                                         IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoReadCompressed( IN PFILE_OBJECT FileObject,
                                       IN PLARGE_INTEGER FileOffset, IN ULONG Length,
                                       IN ULONG LockKey, OUT PVOID Buffer, OUT PMDL *MdlChain,
                                       OUT PIO_STATUS_BLOCK IoStatus,
                                       OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
                                       IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoWriteCompressed( IN PFILE_OBJECT FileObject,
                                        IN PLARGE_INTEGER FileOffset, IN ULONG Length,
                                        IN ULONG LockKey, IN PVOID Buffer, OUT PMDL *MdlChain,
                                        OUT PIO_STATUS_BLOCK IoStatus,
                                        IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
                                        IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoMdlReadCompleteCompressed( IN PFILE_OBJECT FileObject,
                                                  IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoMdlWriteCompleteCompressed( IN PFILE_OBJECT FileObject,
                                                   IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain,
                                                   IN PDEVICE_OBJECT DeviceObject );
BOOLEAN  PGPWDNTFastIoQueryOpen( IN struct _IRP *Irp,
                                  OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
                                  IN PDEVICE_OBJECT DeviceObject );
NTSTATUS PGPWDNTFastIoReleaseForModWrite( IN PFILE_OBJECT FileObject,
                                           IN struct _ERESOURCE *ResourceToRelease, IN PDEVICE_OBJECT DeviceObject );
NTSTATUS PGPWDNTFastIoAcquireForCcFlush( IN PFILE_OBJECT FileObject,
                                          IN PDEVICE_OBJECT DeviceObject );
NTSTATUS PGPWDNTFastIoReleaseForCcFlush( IN PFILE_OBJECT FileObject,
                                          IN PDEVICE_OBJECT DeviceObject );

//----------------------------------------------------------------------
//                         GLOBALS
//---------------------------------------------------------------------- 

//
// This is PGPWDNT's user-inteface device object. It is addressed
// by calls from the GUI including CreateFile and DeviceIoControl
//
PDEVICE_OBJECT      GUIDevice;

//
// Mutex that is shared with GUI to synchronize access to the
// PGPWDNT bins
//
PKMUTEX            PGPWDNTMutex = NULL;

//
// Table of our hook devices for each drive letter. This makes it
// easy to look up the device object that was created to hook a 
// particular drive.
//
PDEVICE_OBJECT      LDriveDevices[26];

//
// System process
// 
PEPROCESS           SystemProcess;

ULONG tc_callback=0;
PKTHREAD tc_thread=0;

//
// This PGPWDNT's Fast I/O dispatch table. Note that NT assumes that
// file system drivers support some Fast I/O calls, so this table must
// be present for an file system filter driver
//
FAST_IO_DISPATCH    FastIOHook = {
    sizeof(FAST_IO_DISPATCH), 
    PGPWDNTFastIoCheckifPossible,
    PGPWDNTFastIoRead,
    PGPWDNTFastIoWrite,
    PGPWDNTFastIoQueryBasicInfo,
    PGPWDNTFastIoQueryStandardInfo,
    PGPWDNTFastIoLock,
    PGPWDNTFastIoUnlockSingle,
    PGPWDNTFastIoUnlockAll,
    PGPWDNTFastIoUnlockAllByKey,
    PGPWDNTFastIoDeviceControl,
    PGPWDNTFastIoAcquireFile,
    PGPWDNTFastIoReleaseFile,
    PGPWDNTFastIoDetachDevice,

    //
    // new for NT 4.0
    //
    PGPWDNTFastIoQueryNetworkOpenInfo,
    PGPWDNTFastIoAcquireForModWrite,
    PGPWDNTFastIoMdlRead,
    PGPWDNTFastIoMdlReadComplete,
    PGPWDNTFastIoPrepareMdlWrite,
    PGPWDNTFastIoMdlWriteComplete,
    PGPWDNTFastIoReadCompressed,
    PGPWDNTFastIoWriteCompressed,
    PGPWDNTFastIoMdlReadCompleteCompressed,
    PGPWDNTFastIoMdlWriteCompleteCompressed,
    PGPWDNTFastIoQueryOpen,
    PGPWDNTFastIoReleaseForModWrite,
    PGPWDNTFastIoAcquireForCcFlush,
    PGPWDNTFastIoReleaseForCcFlush
};


//----------------------------------------------------------------------
//                F A S T I O   R O U T I N E S
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//
// PGPWDNTFastIoCheckIfPossible
//
//----------------------------------------------------------------------
BOOLEAN  PGPWDNTFastIoCheckifPossible( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, 
                                        IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN BOOLEAN CheckForReadOperation,
                                        OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN         retval = FALSE;
    PHOOK_EXTENSION hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoCheckIfPossible ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoCheckIfPossible( 
            FileObject, FileOffset, Length,
            Wait, LockKey, CheckForReadOperation, IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
// 
// PGPWDNTFastIoRead
//
//----------------------------------------------------------------------
BOOLEAN  PGPWDNTFastIoRead( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, 
                             IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, OUT PVOID Buffer,
                             OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoRead ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoRead( 
            FileObject, FileOffset, Length,
            Wait, LockKey, Buffer, IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoWrite
//
//----------------------------------------------------------------------
BOOLEAN  PGPWDNTFastIoWrite( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset, 
                              IN ULONG Length, IN BOOLEAN Wait, IN ULONG LockKey, IN PVOID Buffer,
                              OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN              retval = FALSE;
    PHOOK_EXTENSION      hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoWrite ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoWrite( 
            FileObject, FileOffset, Length, Wait, LockKey, 
            Buffer, IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoQueryBasicinfo
//
//----------------------------------------------------------------------
BOOLEAN  PGPWDNTFastIoQueryBasicInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, 
                                       OUT PFILE_BASIC_INFORMATION Buffer,
                                       OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoQueryBasicInfo ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoQueryBasicInfo( 
            FileObject, Wait, Buffer, IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoQueryStandardInfo
//
//----------------------------------------------------------------------
BOOLEAN  PGPWDNTFastIoQueryStandardInfo( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait, 
                                          OUT PFILE_STANDARD_INFORMATION Buffer,
                                          OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoQueryStandardInfo ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoQueryStandardInfo( 
            FileObject, Wait, Buffer, IoStatus, hookExt->FileSystem );

    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoLock
//
//----------------------------------------------------------------------
BOOLEAN  PGPWDNTFastIoLock( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset,
                             IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key,
                             BOOLEAN FailImmediately, BOOLEAN ExclusiveLock,
                             OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;   

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoLock ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoLock( 
            FileObject, FileOffset, Length, ProcessId, Key, FailImmediately, 
            ExclusiveLock, IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoUnlockSingle
//
//----------------------------------------------------------------------
BOOLEAN  PGPWDNTFastIoUnlockSingle( IN PFILE_OBJECT FileObject, IN PLARGE_INTEGER FileOffset,
                                     IN PLARGE_INTEGER Length, PEPROCESS ProcessId, ULONG Key,
                                     OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;   
    
    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoUnlockSingle ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoUnlockSingle(
            FileObject, FileOffset, Length, ProcessId, Key, 
            IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoUnlockAll
//
//----------------------------------------------------------------------
BOOLEAN  PGPWDNTFastIoUnlockAll( IN PFILE_OBJECT FileObject, PEPROCESS ProcessId,
                                  OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if ( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoUnlockAll ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoUnlockAll( 
            FileObject, ProcessId, IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoUnlockAllByKey
//
//----------------------------------------------------------------------    
BOOLEAN  PGPWDNTFastIoUnlockAllByKey( IN PFILE_OBJECT FileObject, PEPROCESS ProcessId, ULONG Key,
                                       OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoUnlockAllByKey ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoUnlockAllByKey( 
            FileObject, ProcessId, Key, IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoQueryNetworkOpenInfo
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoQueryNetworkOpenInfo(IN PFILE_OBJECT FileObject,
                                           IN BOOLEAN Wait,
                                           OUT struct _FILE_NETWORK_OPEN_INFORMATION *Buffer,
                                           OUT PIO_STATUS_BLOCK IoStatus,
                                           IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoQueryNetworkOpenInfo ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoQueryNetworkOpenInfo( 
            FileObject, Wait, Buffer, IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoAcquireForModWrite
//
//----------------------------------------------------------------------    
NTSTATUS PGPWDNTFastIoAcquireForModWrite( IN PFILE_OBJECT FileObject,
                                           IN PLARGE_INTEGER EndingOffset,
                                           OUT struct _ERESOURCE **ResourceToRelease,
                                           IN PDEVICE_OBJECT DeviceObject )
{
    NTSTATUS            retval = STATUS_NOT_SUPPORTED;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->AcquireForModWrite ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->AcquireForModWrite( 
            FileObject, EndingOffset, ResourceToRelease, hookExt->FileSystem );

    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoMdlRead
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoMdlRead( IN PFILE_OBJECT FileObject,
                               IN PLARGE_INTEGER FileOffset, IN ULONG Length,
                               IN ULONG LockKey, OUT PMDL *MdlChain,
                               OUT PIO_STATUS_BLOCK IoStatus,
                               IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->MdlRead ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->MdlRead( 
            FileObject, FileOffset, Length, LockKey, MdlChain, 
            IoStatus, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoMdlReadComplete
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoMdlReadComplete( IN PFILE_OBJECT FileObject,
                                       IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->MdlReadComplete ) {

        retval = (BOOLEAN) hookExt->FileSystem->DriverObject->FastIoDispatch->MdlReadComplete( FileObject, 
                                                                                               MdlChain, hookExt->FileSystem );

    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoPrepareMdlWrite
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoPrepareMdlWrite( IN PFILE_OBJECT FileObject,
                                       IN PLARGE_INTEGER FileOffset, IN ULONG Length,
                                       IN ULONG LockKey, OUT PMDL *MdlChain,
                                       OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;
    IoStatus->Status      = STATUS_NOT_SUPPORTED;
    IoStatus->Information = 0;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->PrepareMdlWrite ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->PrepareMdlWrite( 
            FileObject, FileOffset, Length, LockKey, MdlChain, IoStatus, 
            hookExt->FileSystem );
    } 
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoMdlWriteComplete
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoMdlWriteComplete( IN PFILE_OBJECT FileObject,
                                        IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain, 
                                        IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->MdlWriteComplete ) { 

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->MdlWriteComplete( 
            FileObject, FileOffset, MdlChain, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoReadCompressed
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoReadCompressed( IN PFILE_OBJECT FileObject,
                                      IN PLARGE_INTEGER FileOffset, IN ULONG Length,
                                      IN ULONG LockKey, OUT PVOID Buffer,
                                      OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus,
                                      OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
                                      IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoReadCompressed ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoReadCompressed( 
            FileObject, FileOffset, Length, LockKey, Buffer, MdlChain, IoStatus,
            CompressedDataInfo, CompressedDataInfoLength, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoWriteCompressed
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoWriteCompressed( IN PFILE_OBJECT FileObject,
                                       IN PLARGE_INTEGER FileOffset, IN ULONG Length,
                                       IN ULONG LockKey, OUT PVOID Buffer,
                                       OUT PMDL *MdlChain, OUT PIO_STATUS_BLOCK IoStatus,
                                       OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
                                       IN ULONG CompressedDataInfoLength, IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoWriteCompressed ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoWriteCompressed( 
            FileObject, FileOffset, Length, LockKey, Buffer, MdlChain, IoStatus,
            CompressedDataInfo, CompressedDataInfoLength, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoMdlReadCompleteCompressed
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoMdlReadCompleteCompressed( IN PFILE_OBJECT FileObject,
                                                 IN PMDL MdlChain, IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->MdlReadCompleteCompressed ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->MdlReadCompleteCompressed( 
            FileObject, MdlChain, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoMdlWriteCompleteCompressed
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoMdlWriteCompleteCompressed( IN PFILE_OBJECT FileObject,
                                                  IN PLARGE_INTEGER FileOffset, IN PMDL MdlChain, 
                                                  IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension; 

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->MdlWriteCompleteCompressed ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->MdlWriteCompleteCompressed( 
            FileObject, FileOffset, MdlChain, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoQueryOpen
//
// This call actually passes an IRP! 
//
//----------------------------------------------------------------------    
BOOLEAN PGPWDNTFastIoQueryOpen( IN PIRP Irp,
                                 OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
                                 IN PDEVICE_OBJECT DeviceObject )
{
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;
    PFILE_OBJECT        fileobject;
    PIO_STACK_LOCATION  currentIrpStack;
    PIO_STACK_LOCATION  nextIrpStack;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoQueryOpen ) {

        currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
        nextIrpStack    = IoGetNextIrpStackLocation(Irp);
        fileobject      = currentIrpStack->FileObject;

        //
        // copy parameters down to next level in the stack
        //
        *nextIrpStack = *currentIrpStack;
        nextIrpStack->DeviceObject = hookExt->FileSystem;
        IoSetNextIrpStackLocation( Irp );

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoQueryOpen( 
            Irp, NetworkInformation, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoReleaseForModWrite
//
//----------------------------------------------------------------------    
NTSTATUS PGPWDNTFastIoReleaseForModWrite( IN PFILE_OBJECT FileObject,
                                           IN struct _ERESOURCE *ResourceToRelease,
                                           IN PDEVICE_OBJECT DeviceObject )
{
    NTSTATUS            retval = STATUS_NOT_SUPPORTED;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->ReleaseForModWrite ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->ReleaseForModWrite( 
            FileObject,  ResourceToRelease, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoAcquireForCcFlush
//
//----------------------------------------------------------------------    
NTSTATUS PGPWDNTFastIoAcquireForCcFlush( IN PFILE_OBJECT FileObject,
                                          IN PDEVICE_OBJECT DeviceObject )
{
    NTSTATUS            retval = STATUS_NOT_SUPPORTED;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->AcquireForCcFlush ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->AcquireForCcFlush( 
            FileObject, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoReleaseForCcFlush
//
//----------------------------------------------------------------------    
NTSTATUS PGPWDNTFastIoReleaseForCcFlush( IN PFILE_OBJECT FileObject,
                                          IN PDEVICE_OBJECT DeviceObject )
{
    NTSTATUS            retval = STATUS_NOT_SUPPORTED;
    PHOOK_EXTENSION     hookExt;

    hookExt = DeviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->ReleaseForCcFlush ) {

        retval = hookExt->FileSystem->DriverObject->FastIoDispatch->ReleaseForCcFlush( 
            FileObject, hookExt->FileSystem );
    }
    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoDeviceControl
//
//----------------------------------------------------------------------
BOOLEAN  PGPWDNTFastIoDeviceControl( IN PFILE_OBJECT FileObject, IN BOOLEAN Wait,
                                      IN PVOID InputBuffer, IN ULONG InputBufferLength, 
                                      OUT PVOID OutputBuffer, IN ULONG OutputBufferLength, IN ULONG IoControlCode,
                                      OUT PIO_STATUS_BLOCK IoStatus, IN PDEVICE_OBJECT DeviceObject ) {
    BOOLEAN             retval = FALSE;
    PHOOK_EXTENSION     hookExt;
  
    if ( DeviceObject == GUIDevice )  {

        //
        // Its a message from our GUI!
        //
        IoStatus->Status      = STATUS_SUCCESS; 
        IoStatus->Information = 0;      

        switch ( IoControlCode ) {

		case PGPWDNT_REGISTER_CB:
		{
			struct  trap_criteria *trap_cond;

			// get current thread
			tc_thread=KeGetCurrentThread();

			// Get the callback
			trap_cond=(struct trap_criteria *)InputBuffer;
			tc_callback=trap_cond->tc_callback;

			trap_cond->tc_callback=666;

			break;
		}

		case PGPWDNT_MAP_MEM :
		{
			struct trap_record *tr;
//			PMDL Mdl;
//			char *usFileName;

			tr=(struct trap_record *)InputBuffer;	
//			Mdl=(PMDL)tr->keFileName;
/*
			Mdl = IoAllocateMdl(tr->keFileName,1024,0,FALSE,NULL);
			MmBuildMdlForNonPagedPool(Mdl);
			MmProbeAndLockPages(Mdl, KernelMode,IoReadAccess);*/
//			usFileName = (char *)(((ULONG)MmMapLockedPages(Mdl, UserMode)) 
//                               | MmGetMdlByteOffset(Mdl));

			strcpy(tr->tr_file1,tr->keFileName);//usFileName); //usFileName);

//			MmUnmapLockedPages(usFileName,Mdl);
//			MmUnlockPages(Mdl);
//			IoFreeMdl(Mdl);
//			ExFreePool(tr->keFileName);
			break;
		}

		case PGPWDNT_REC_DONE :
		{
//			struct trap_record *tr;

//			tr=(struct trap_record *)InputBuffer;

		/*
			VMMLISTNODE *rec_buffer;

			rec_buffer=(VMMLISTNODE *)InputBuffer;

			IFSMgr_Wakeup((unsigned long)&(log_flag));

			List_Deallocate(list_handle,*rec_buffer);*/
			break;
		}
 
        default:

            //
            // Unknown control
            // 
            DbgPrint (("PGPWDNT: unknown IRP_MJ_DEVICE_CONTROL\n"));
            IoStatus->Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        retval = TRUE;

    } else {

        //
        // Its a call for a file system, so pass it through
        //
        hookExt = DeviceObject->DeviceExtension;

        if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoDeviceControl ) {
        
            retval = hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoDeviceControl( 
                FileObject, Wait, InputBuffer, InputBufferLength, OutputBuffer, 
                OutputBufferLength, IoControlCode, IoStatus, 
                hookExt->FileSystem );
        }
    }

    return retval;
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoAcquireFile
//
//----------------------------------------------------------------------
VOID PGPWDNTFastIoAcquireFile( PFILE_OBJECT FileObject ) {
    PDEVICE_OBJECT      deviceObject;
    PHOOK_EXTENSION     hookExt;

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    hookExt = deviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->AcquireFileForNtCreateSection ) {

        hookExt->FileSystem->DriverObject->FastIoDispatch->AcquireFileForNtCreateSection( 
            FileObject );
    }
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoReleaseFile
//
//----------------------------------------------------------------------
VOID PGPWDNTFastIoReleaseFile( PFILE_OBJECT FileObject ) {
    PDEVICE_OBJECT      deviceObject;
    PHOOK_EXTENSION     hookExt;

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    hookExt = deviceObject->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->ReleaseFileForNtCreateSection ) {

        hookExt->FileSystem->DriverObject->FastIoDispatch->ReleaseFileForNtCreateSection( FileObject );

    }
}


//----------------------------------------------------------------------
//
// PGPWDNTFastIoDetachDevice
//
// We get this call when a device that we have hooked is being deleted.
// This happens when, for example, a floppy is formatted. We have
// to detach from it and delete our device. We should notify the GUI
// that the hook state has changed, but its not worth the trouble.
//
//----------------------------------------------------------------------
VOID PGPWDNTFastIoDetachDevice( PDEVICE_OBJECT SourceDevice, PDEVICE_OBJECT TargetDevice ) {
    PHOOK_EXTENSION     hookExt;
    ULONG               i;

    //
    // See if a device (like a floppy) is being removed out from under us. If so,
    // we have to detach from it before it disappears  
    //
    for( i = 2; i < 26; i++ ) {

        if( SourceDevice == LDriveDevices[i] ) {

            //
            // We've hooked it, so we must detach
            //
            hookExt = SourceDevice->DeviceExtension;

            DbgPrint(("PGPWDNT: Detaching from drive: %c\n", 
                      hookExt->LogicalDrive ));

            IoDetachDevice( TargetDevice );
            IoDeleteDevice( SourceDevice );

            LDriveDevices[i] = NULL;

            return;
        }
    }

    //
    // Now we can pass the call through
    //
    hookExt = SourceDevice->DeviceExtension;

    if( hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoDetachDevice ) {

        hookExt->FileSystem->DriverObject->FastIoDispatch->FastIoDetachDevice( 
            SourceDevice, TargetDevice );

    }
}

//----------------------------------------------------------------------
//           F I L E   U N D L E T E   R O U T I N E S
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//
// PGPWDNTCheckFileForPGPWDNT
//
// See if this file is marked for delete. If so, get its name 
// and see if this is a file that should be protected.
//
//----------------------------------------------------------------------
BOOLEAN PGPWDNTCheckFileForPGPWDNT( PDEVICE_OBJECT DeviceObject, PIRP Irp,
                                      PPGPWDNT_COMPLETE_CONTEXT CompleteContext )
{
    PIO_STACK_LOCATION      currentIrpStack = IoGetCurrentIrpStackLocation(Irp);    
    PHOOK_EXTENSION         hookExt;
    PCHAR                   fileNameBuffer;
	UNICODE_STRING          fileNameUnicodeString;
	int i,length;

	// We have nothing to do anyway....
	if(tc_callback==0)
		return FALSE;

    //
    // Get the hook extension
    //
    hookExt = DeviceObject->DeviceExtension;

    //
    // First, make sure its not a directory
    //
	if( PGPWDNTIsDirectory( hookExt->FileSystem, currentIrpStack->FileObject ) ) {
  
    return FALSE;
	}

    //
    // Get the name of the file. 
    //
    fileNameBuffer = ExAllocatePool( NonPagedPool, 1024 );
    if( !PGPWDNTGetFileName( hookExt->IsNTFS, hookExt->FileSystem, currentIrpStack->FileObject,
                              fileNameBuffer, 1024)) 
	{
        //
        // Couldn't get the name for some reason
        //
        ExFreePool( fileNameBuffer );
        return FALSE;
    }

    fileNameUnicodeString.Length = (USHORT) ((PFILE_NAME_INFORMATION) fileNameBuffer)->FileNameLength + 12;
    fileNameUnicodeString.MaximumLength = fileNameUnicodeString.Length;
    fileNameUnicodeString.Buffer = ExAllocatePool( PagedPool, fileNameUnicodeString.Length+2+12);
    swprintf( fileNameUnicodeString.Buffer, L"%C:", hookExt->LogicalDrive );
    RtlCopyMemory( &fileNameUnicodeString.Buffer[2], ((PFILE_NAME_INFORMATION) fileNameBuffer)->FileName, 
                   fileNameUnicodeString.Length);
    fileNameUnicodeString.Buffer[fileNameUnicodeString.Length/2] = 0;

    //
    // Determine the length of the name
    //
	i=0;
	length=0;

    while( fileNameUnicodeString.Buffer[i++] ) length++;

    //
    // First, make the ascii name
    //
    for( i = 0; i < length; i++ ) {
      
        fileNameBuffer[i] = (CHAR) fileNameUnicodeString.Buffer[i];
    }
    fileNameBuffer[i] = 0;

	ExFreePool( fileNameUnicodeString.Buffer );
/*
	if(_strnicmp( fileNameBuffer, "aaaaaa", 6))
	{
        //
        // Its already been wiped by us. Go ahead and let OS delete it.
        //
        DbgPrint(("PGPWDNT: its already been wiped\n"));
        ExFreePool( fileNameBuffer );
        return FALSE;
    }
*/
    //
    // Set up the completion context, where the rest of the PGPWDNT will take place
    //
    CompleteContext->FileName  = fileNameBuffer;
    return TRUE;
}

/* The APC structure is defined in ntddk.h */

/* this is KERNEL_ROUTINE for our APC; in particular, it gets called when the APC
is being delivered. Usually one of predefined useful routines in the kernel are
used for this purpose, but we can't use any of them as none of them are exported. */

void MyRoutine(struct _KAPC *Apc,
               PKNORMAL_ROUTINE norm_routine,
               void *context,
               void *SysArg1,
               void *SysArg2)
{
    ExFreePool(Apc);
    return;
}

/* pointer to the APC we will create */

static struct _KAPC *apc;

/* KeInitializeApc() and KeInsertQueueApc() are the two functions needed to
send an APC; they're both exported but not prototyped in the DDK, so we
prototype them here. */

void KeInitializeApc(struct _KAPC *Apc, PKTHREAD thread,
                     unsigned char state_index,
                     PKKERNEL_ROUTINE ker_routine,
                     PKRUNDOWN_ROUTINE rd_routine,
                     PKNORMAL_ROUTINE nor_routine,
                     unsigned char mode,
                     void *context);
                            

void KeInsertQueueApc(struct _KAPC *APC,
                      void *SysArg1,
                      void *SysArg2,
                      unsigned char arg4);

/* call this function when you need to send a user-mode APC to 
the current thread. addr must be linear address of your user-mode
function to call:

void MyApcRoutine(ULONG arg1, ULONG arg2, ULONG arg3);
...
SendAddrToTheDriverUsingIoctl((ULONG)MyApcRoutine);

you should send it to the driver using your custom IOCTL.
arg1, arg2, arg3 are arbitrary ulong's which are passed to the function
residing at addr; this function should be prototyped as receiving three
parameters and returning void. */

void SendAPC(ULONG addr, PKTHREAD thread, ULONG arg1, ULONG arg2, ULONG arg3) 
{
/* this is self-explanatory */

    apc=ExAllocatePool(NonPagedPool, sizeof(struct _KAPC));

/* Initialize the user-mode APC */

    KeInitializeApc(apc, thread, 0,
                    (PKKERNEL_ROUTINE)&MyRoutine, 0,
                    (PKNORMAL_ROUTINE)addr, 1, (PVOID)arg1);

/* Insert it to the queue of the target thread */

    KeInsertQueueApc(apc, (PVOID)arg2, (PVOID)arg3, 0);

/* Mark the current thread as alertable to force it to deliver the APC on
the next return to the user-mode. NOTE: severely undocumented code here!
*/

    *((unsigned char *)thread+0x4a)=1; 
}

//----------------------------------------------------------------------
// 
// PGPWDNTworkitem
//
// Called by the system, this is initiated in the completion callback.
// We use it to ensure we''re running at passive level when we do
// an PGPWDNT
//
//----------------------------------------------------------------------
VOID PGPWDNTWorkItem( PVOID Context )
{
    BOOLEAN             mutexGrabbed = FALSE;
//	PMDL Mdl;

    //
    // Grab our mutex
    //
    if( PGPWDNTMutex ) {

        KeWaitForSingleObject( PGPWDNTMutex, Executive, KernelMode,
                               FALSE, NULL );
        mutexGrabbed = TRUE;
        DbgPrint(("Got mutex\n"));
    }

//	Mdl = IoAllocateMdl((void *)((PPGPWDNT_COMPLETE_CONTEXT)Context)->FileName,
//		1024,0,FALSE,NULL);
//	MmBuildMdlForNonPagedPool(Mdl);
//	MmProbeAndLockPages(Mdl, KernelMode,IoReadAccess);

    //
    // Call the PGPWDNT routine
    //
	SendAPC(tc_callback, tc_thread,
		(ULONG)((PPGPWDNT_COMPLETE_CONTEXT)Context)->FileName, 0, 0); 

    //
    // Release Mutex
    //
    if( mutexGrabbed ) {

        DbgPrint(("Releasing mutex\n"));        
        KeReleaseMutex( PGPWDNTMutex, FALSE );
    }

    //
    // Complete the IRP
    //
    ((PPGPWDNT_COMPLETE_CONTEXT) Context)->Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest( ((PPGPWDNT_COMPLETE_CONTEXT) Context)->Irp, 
                       IO_NO_INCREMENT );

    //
    // Free memory
    //
//    ExFreePool( ((PPGPWDNT_COMPLETE_CONTEXT) Context)->FileName);
      ExFreePool( Context );
}

//----------------------------------------------------------------------
//     D I S P A T C H   A N D   H O O K   E N T R Y   P O I N T S
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// 
// PGPWDNTHookDone
//
// Gets control on the way back from a delete operation. At this point
// the file cannot be opened by anything else, so we can move it into
// the recycle bin.
//
//----------------------------------------------------------------------
NTSTATUS PGPWDNTHookDone( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp,
                           IN PVOID Context )
{
    if( NT_SUCCESS(Irp->IoStatus.Status)) {

        ExInitializeWorkItem( &((PPGPWDNT_COMPLETE_CONTEXT) Context)->WorkItem, 
                              PGPWDNTWorkItem, Context );
        ExQueueWorkItem( &((PPGPWDNT_COMPLETE_CONTEXT) Context)->WorkItem, 
                         CriticalWorkQueue );

        //
        // We have to complete the IRP later
        //
        return STATUS_MORE_PROCESSING_REQUIRED;

    } else {

        //
        // Now we have to mark Irp as pending if necessary
        //
        if( Irp->PendingReturned ) {

            IoMarkIrpPending( Irp );
        }

        // 
        // Free the completion context
        //
        ExFreePool( ((PPGPWDNT_COMPLETE_CONTEXT) Context)->FileName );
        ExFreePool( Context );
        return Irp->IoStatus.Status;
    }
}


//----------------------------------------------------------------------
//
// PGPWDNTHookRoutine
//
// This routine is the main hook routine where we figure out what
// calls are being sent to the file system.
//
//----------------------------------------------------------------------
NTSTATUS PGPWDNTHookRoutine( PDEVICE_OBJECT HookDevice, IN PIRP Irp )
{
    PIO_STACK_LOCATION  currentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION  nextIrpStack    = IoGetNextIrpStackLocation(Irp);
    PFILE_OBJECT        fileObject;
    PHOOK_EXTENSION     hookExt;
    PPGPWDNT_COMPLETE_CONTEXT completeContext;

    //
    // Extract the file object from the IRP
    //
    fileObject    = currentIrpStack->FileObject;

    //
    // Point at the device extension, which contains information on which
    // file system this IRP is headed for
    //
    hookExt = HookDevice->DeviceExtension;

    //
    // Copy parameters down to next level in the stack for the driver below us
    //
    *nextIrpStack = *currentIrpStack;

    //
    // Determine what function we're dealing with
    //
    switch( currentIrpStack->MajorFunction ) {

    case IRP_MJ_CLEANUP:

        //
        // If the file will be deleted, see if we have to PGPWDNT it
        //
        completeContext = ExAllocatePool( NonPagedPool, sizeof( PGPWDNT_COMPLETE_CONTEXT));
        if( fileObject->DeletePending && (SystemProcess != PsGetCurrentProcess()) &&
            PGPWDNTCheckFileForPGPWDNT( HookDevice, Irp, completeContext )) {

            //
            // PGPWDNT the file
            // Not yet!
//            PGPWDNTSetDispositionFile( hookExt->FileSystem, currentIrpStack->FileObject,
//                                      FALSE );

            //
            // Finish setting up the completion context
            //
            completeContext->DeviceObject = HookDevice;
            completeContext->Irp = Irp;
            completeContext->Token = PsReferencePrimaryToken(PsGetCurrentProcess());
            if( !completeContext->Token ) {

                //
                // Couldn't reference the token(?)
                //
                DbgPrint(("PGPWDNT: Could not reference the process token.\n"));
                ExFreePool( completeContext );
                
            } else {

                //
                // Set the completion routine where we finish the job
                //
                DbgPrint(("PGPWDNT: going to move this to the recycle bin\n"));

                IoSetCompletionRoutine( Irp, PGPWDNTHookDone, (PVOID) completeContext, TRUE, TRUE, TRUE );  
                
                IoMarkIrpPending( Irp );
                IoCallDriver( hookExt->FileSystem, Irp );

                //
                // Return this because we're going to stall the IRP in the 
                // completion routine
                //
                return STATUS_PENDING;
            }

        } else {
 
            ExFreePool( completeContext );
        }
        break;

    default:

        //
        // Register a never-called completion routine just so that the IRP stack
        // doesn't have a bogus completion routine in it.
        //
        IoSetCompletionRoutine( Irp, PGPWDNTHookDone, 0, FALSE, FALSE, FALSE );
        break;
    }

    //
    // Return the results of the call to the caller
    //
    return IoCallDriver( hookExt->FileSystem, Irp );
}



//----------------------------------------------------------------------
//
// PGPWDNTDeviceRoutine
//
// In this routine we handle requests to our own device. The only 
// requests we care about handling explicitely are IOCTL commands that
// we will get from the GUI. We also expect to get Create and Close 
// commands when the GUI opens and closes communications with us.
//
//----------------------------------------------------------------------
NTSTATUS PGPWDNTDeviceRoutine( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
    PIO_STACK_LOCATION  irpStack;
    PVOID               inputBuffer;
    PVOID               outputBuffer;
    ULONG               inputBufferLength;
    ULONG               outputBufferLength;
    ULONG               ioControlCode;

    //
    // Go ahead and set the request up as successful
    //
    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    //
    // Get a pointer to the current location in the Irp. This is where
    // the function codes and parameters are located.
    //
    irpStack = IoGetCurrentIrpStackLocation (Irp);

    //
    // Get the pointer to the input/output buffer and its length
    //
    inputBuffer     = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength   = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBuffer    = Irp->AssociatedIrp.SystemBuffer;
    outputBufferLength  = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    ioControlCode   = irpStack->Parameters.DeviceIoControl.IoControlCode;

    switch (irpStack->MajorFunction) {
    case IRP_MJ_CREATE:

        DbgPrint(("PGPWDNT: IRP_MJ_CREATE\n"));
        break;

    case IRP_MJ_CLOSE:

        DbgPrint(("PGPWDNT: IRP_MJ_CLOSE\n"));
        break;
    }

    //
    // Complete the IRP
    //
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;   
}


//----------------------------------------------------------------------
//
// PGPWDNTDispatch
//
// Based on which device the Irp is destined for we call either the
// filesystem filter function, or our own device handling routine.
//
//----------------------------------------------------------------------
NTSTATUS PGPWDNTDispatch( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
    //
    // Determine if its a request from the GUI to us, or one that is
    // directed at a file system driver that we've hooked
    //
    if ( GUIDevice != DeviceObject ) {

        return( PGPWDNTHookRoutine( DeviceObject, Irp ));

    } else {

        return( PGPWDNTDeviceRoutine( DeviceObject, Irp ));

    }
}

//----------------------------------------------------------------------
//   D R I V E R - E N T R Y   A N D   H O O K   R O U T I N E S
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//
// HookDrive
//
// Hook the drive specified by determining which device object to 
// attach to. The algorithm used here is similar to the one used
// internally by NT to determine which device object a file system request
// is directed at.
//
//----------------------------------------------------------------------
BOOLEAN HookDrive( IN USHORT Drive, IN PDRIVER_OBJECT DriverObject, 
                   IN HANDLE VolumeHandle )
{
    PDEVICE_OBJECT      fileSysDevice;
    PDEVICE_OBJECT      hookDevice;
    NTSTATUS            ntStatus;
    PFILE_OBJECT        fileObject;
    IO_STATUS_BLOCK  	ioStatus;    
    PHOOK_EXTENSION     hookExtension;
    UCHAR               fsAttribInformation[256];
 
    //
    // Got the file handle, so now look-up the file-object it refers to
    //
    ntStatus = ObReferenceObjectByHandle( VolumeHandle, FILE_READ_DATA, 
                                          NULL, KernelMode, &fileObject, NULL );
    if( !NT_SUCCESS( ntStatus )) {

        DbgPrint(("PGPWDNT: Could not get fileobject from handle: %c\n", 'A'+Drive ));
        return FALSE;
    }

    //  
    // Next, find out what device is associated with the file object by getting its related
    // device object
    //
    fileSysDevice = IoGetRelatedDeviceObject( fileObject );

    if ( ! fileSysDevice ) {

        DbgPrint(("PGPWDNT: Could not get related device object: %c\n", 'A'+Drive ));
        ObDereferenceObject( fileObject );
        return FALSE;
    }

    //
    // The file system's device hasn't been hooked already, so make a hooking device
    //  object that will be attached to it.
    //
    ntStatus = IoCreateDevice( DriverObject,
                               sizeof(HOOK_EXTENSION),
                               NULL,
                               fileSysDevice->DeviceType,
                               0,
                               FALSE,
                               &hookDevice );

    if ( !NT_SUCCESS(ntStatus) ) {

        DbgPrint(("PGPWDNT: failed to create associated device: %c\n", 'A'+Drive ));   
        ObDereferenceObject( fileObject );
        return FALSE;
    }

    //
    // Clear the device's init flag as per NT DDK KB article on creating device 
    // objects from a dispatch routine
    //
    hookDevice->Flags &= ~DO_DEVICE_INITIALIZING;

    //
    // Setup the device extensions. The drive letter and file system object are stored
    // in the extension.
    //
    hookExtension = hookDevice->DeviceExtension;
    hookExtension->LogicalDrive = 'A'+Drive;
    hookExtension->FileSystem   = fileSysDevice;
    hookExtension->IsNTFS       = FALSE;
    hookExtension->IsAnsi       = FALSE;

    //
    // Determine which file system this is. If this fails, assume its NOT NTFS
    //
    ntStatus = ZwQueryVolumeInformationFile( VolumeHandle, &ioStatus, fsAttribInformation,
                                             sizeof(fsAttribInformation), FileFsAttributeInformation );
    if( !NT_SUCCESS( ntStatus )) {

        DbgPrint(("PGPWDNT: Could not get volume attributes: %x\n", ntStatus));

    } else if( RtlCompareMemory( ((PFILE_FS_ATTRIBUTE_INFORMATION)fsAttribInformation)->FileSystemName, L"NTFS", 8 ) == 8) {

        hookExtension->IsNTFS = TRUE;
        DbgPrint(("  => Its an NTFS volume\n"));
    } 

    //
    // Finally, attach to the device. The second we're successfully attached, we may 
    // start receiving IRPs targetted at the device we've hooked.
    //
    ntStatus = IoAttachDeviceByPointer( hookDevice, fileSysDevice );

    if ( !NT_SUCCESS(ntStatus) )  {

        //
        // Couldn't attach for some reason
        //
        DbgPrint(("PGPWDNT: Connect with Filesystem failed: %c (%x) =>%x\n", 
                  'A'+Drive, fileSysDevice, ntStatus ));

        //
        // Derefence the object and get out
        //
        ObDereferenceObject( fileObject );
        return FALSE;

    } 

    //
    // Note which device this is
    //
    LDriveDevices[Drive] = hookDevice;

    //
    // Dereference the object
    //
    ObDereferenceObject( fileObject );

    return TRUE;
}


//----------------------------------------------------------------------
//
// DriverEntry
//
// Installable driver initialization. Here we just set ourselves up.
//
//----------------------------------------------------------------------
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath )
{
    NTSTATUS                ntStatus;
    WCHAR                   deviceNameBuffer[]  = L"\\Device\\PGPWDNT";
    UNICODE_STRING          deviceNameUnicodeString;
    WCHAR                   deviceLinkBuffer[]  = L"\\DosDevices\\PGPWDNT";
    UNICODE_STRING          deviceLinkUnicodeString;
    WCHAR                   volumeBuffer[] = L"\\DosDevices\\X:\\";
    UNICODE_STRING          volumeBufferUnicodeString;
    USHORT                  drive;
    HANDLE 		            volumeHandle;   
    OBJECT_ATTRIBUTES       objectAttributes;
    IO_STATUS_BLOCK  	    ioStatus;
    BOOLEAN                 errorMode;
    FILE_FS_DEVICE_INFORMATION fsDeviceInformation;
 
    DbgPrint (("PGPWDNT.SYS: entering DriverEntry\n"));

    //
    // If not NT 4.0 Final Release, shorten the Fast I/O table so that PGPWDNT 
    // will work on the Betas and Release Candidates
    //
    if( *NtBuildNumber < NT4FINAL ) {

        FastIOHook.SizeOfFastIoDispatch = (ULONG) &FastIOHook.FastIoQueryNetworkOpenInfo - 
            (ULONG) &FastIOHook;
    } 

    //    
    // Setup the device name
    //    
    RtlInitUnicodeString (&deviceNameUnicodeString,
                          deviceNameBuffer );

    //
    // Create the device used for GUI communications
    //
    ntStatus = IoCreateDevice ( DriverObject,
                                0,
                                &deviceNameUnicodeString,
                                FILE_DEVICE_WD,
                                0,
                                TRUE,
                                &GUIDevice );

    //
    // If successful, make a symbolic link that allows for the device
    // object's access from Win32 programs
    //
    if (NT_SUCCESS(ntStatus)) {

        //
        // Create a symbolic link that the GUI can specify to gain access
        // to this driver/device
        //
        RtlInitUnicodeString (&deviceLinkUnicodeString,
                              deviceLinkBuffer );
        ntStatus = IoCreateSymbolicLink (&deviceLinkUnicodeString,
                                         &deviceNameUnicodeString );
        if (!NT_SUCCESS(ntStatus)) {

            DbgPrint (("PGPWDNT.SYS: IoCreateSymbolicLink failed\n"));
            // FIX
            ntStatus = STATUS_SUCCESS;
        }

        //
        // Create dispatch points for all routines that must be handled. 
        // All entry points are registered since we might filter a
        // file system that processes all of them.
        //
        DriverObject->MajorFunction[IRP_MJ_READ]            = 
            DriverObject->MajorFunction[IRP_MJ_CLOSE]           =
            DriverObject->MajorFunction[IRP_MJ_WRITE]           =
            DriverObject->MajorFunction[IRP_MJ_CREATE]          =
            DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]   =
            DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION]   =
            DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] =
            DriverObject->MajorFunction[IRP_MJ_QUERY_EA]        =
            DriverObject->MajorFunction[IRP_MJ_SET_EA]          =
            DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = 
            DriverObject->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION] =
            DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]   =
            DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] =
            DriverObject->MajorFunction[IRP_MJ_QUERY_SECURITY]  =
            DriverObject->MajorFunction[IRP_MJ_SET_SECURITY]    =
            DriverObject->MajorFunction[IRP_MJ_SHUTDOWN]        =
            DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL]    =
            DriverObject->MajorFunction[IRP_MJ_CLEANUP]         =
            DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = PGPWDNTDispatch;

        //
        // Set up the Fast I/O dispatch table
        //
        DriverObject->FastIoDispatch = &FastIOHook;
    }

    //
    // If something went wrong, cleanup the device object and don't load
    //
    if (!NT_SUCCESS(ntStatus)) {

        DbgPrint(("PGPWDNT: Failed to create our device!\n"));

        if( GUIDevice ) {

            IoDeleteDevice( GUIDevice );
        }
        return ntStatus;
    }

    //
    // Disable hard errors so that we don't pop-up when touching removable media
    //
    errorMode = IoSetThreadHardErrorMode( FALSE );

    //
    // Need to check each drive to see if its a local hard disk
    //
    RtlInitUnicodeString (&volumeBufferUnicodeString,
                          volumeBuffer );
    for(drive = 2; drive < 26; drive++ ) {

        LDriveDevices[drive] = NULL;

        volumeBufferUnicodeString.Buffer[12] = drive+'A';

        InitializeObjectAttributes( &objectAttributes, &volumeBufferUnicodeString, 
                                    OBJ_CASE_INSENSITIVE, NULL, NULL );

        ntStatus = ZwCreateFile( &volumeHandle, FILE_ANY_ACCESS, 
                                 &objectAttributes, &ioStatus, NULL, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, 
                                 FILE_OPEN, 
                                 FILE_SYNCHRONOUS_IO_NONALERT|FILE_OPEN_FOR_BACKUP_INTENT|FILE_DIRECTORY_FILE, 
                                 NULL, 0 );
        if( !NT_SUCCESS( ntStatus ) ) {

            DbgPrint(("PGPWDNT: Could not open drive %c: %x\n", 'A'+drive, ntStatus ));

        } else {

            //
            // Opened the volume. Now go and query it to see if its a local drive
            //
            ntStatus = ZwQueryVolumeInformationFile( volumeHandle, &ioStatus, &fsDeviceInformation,
                                                     sizeof(fsDeviceInformation), FileFsDeviceInformation );

            if( NT_SUCCESS( ntStatus )) {

                //
                // Alright, see if its the correct type 
                //
                if( fsDeviceInformation.DeviceType == FILE_DEVICE_DISK && !(fsDeviceInformation.Characteristics & 
                                                                            (FILE_REMOVABLE_MEDIA|FILE_READ_ONLY_DEVICE|FILE_FLOPPY_DISKETTE|FILE_WRITE_ONCE_MEDIA|
                                                                             FILE_REMOTE_DEVICE|FILE_VIRTUAL_VOLUME))) {
	                     
                    DbgPrint(("Volume %C must be filtered\n", 'A'+drive ));

                    //
                    // It is, so hook it.
                    //
                    HookDrive( drive, DriverObject, volumeHandle );
                }

                ZwClose( volumeHandle );
            }
        }
    }

    //
    // Save away system process
    //
    SystemProcess = PsGetCurrentProcess();

    IoSetThreadHardErrorMode( errorMode );

    return STATUS_SUCCESS;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
