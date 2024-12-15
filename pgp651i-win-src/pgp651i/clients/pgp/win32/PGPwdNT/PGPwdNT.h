/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.
	
	

	$Id: PGPwdNT.h,v 1.3 1999/05/11 06:57:14 wjb Exp $
____________________________________________________________________________*/

int __cdecl swprintf( WCHAR *, const WCHAR *, ... );

typedef struct _FILE_FS_ATTRIBUTE_INFORMATION {
    ULONG FileSystemAttributes;
    LONG MaximumComponentNameLength;
    ULONG FileSystemNameLength;
    WCHAR FileSystemName[1];
} FILE_FS_ATTRIBUTE_INFORMATION, *PFILE_FS_ATTRIBUTE_INFORMATION;

NTSTATUS NTAPI ZwQueryVolumeInformationFile( HANDLE FileHandle,
					     PIO_STATUS_BLOCK IoStatusBlock,
					     PVOID FsInformation,
					     ULONG Length,
					     FS_INFORMATION_CLASS FsInformationClass );

HANDLE PsReferencePrimaryToken(PEPROCESS Process );


//----------------------------------------------------------------------
//                           TYPEDEFS
//----------------------------------------------------------------------

// 
// Max path len
//
#define PATH_LEN	260

//
// Structure for device specific data that keeps track of what
// drive and what filesystem device are hooked 
//
typedef struct {
   PDEVICE_OBJECT   FileSystem;
   UCHAR            LogicalDrive;
   BOOLEAN          IsNTFS;
   BOOLEAN          IsAnsi;
   ERESOURCE        PGPWDNTLock;
} HOOK_EXTENSION, *PHOOK_EXTENSION;         

// 
// This is passed to the CLOSE completion routine for an PGPWDNT
//
typedef struct {
   PIRP                       Irp;
   WORK_QUEUE_ITEM            WorkItem;
   PVOID                      Token;
   PDEVICE_OBJECT             DeviceObject;
   char *		              FileName;
} PGPWDNT_COMPLETE_CONTEXT, *PPGPWDNT_COMPLETE_CONTEXT;

//----------------------------------------------------------------------
//                           DEFINES
//----------------------------------------------------------------------

//
// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
//
#define FILE_DEVICE_PGPWDNT	0x00008320

// 
// NT Final Build number
//
#define NT4FINAL        1381

//
// Print macro that only turns on when debugging is on
//
#if DBG
#define DbgPrint(arg) DbgPrint arg
#else
#define DbgPrint(arg) 
#endif

//----------------------------------------------------------------------
//                         FORWARDS
//----------------------------------------------------------------------

BOOLEAN PGPWDNTGetFileName(BOOLEAN IsNTFS, PDEVICE_OBJECT DeviceObject, 
                            PFILE_OBJECT FileObject,
                            PUCHAR FileName, ULONG FileNameLength );
BOOLEAN PGPWDNTSetDispositionFile(PDEVICE_OBJECT DeviceObject, 
							PFILE_OBJECT FileObject, BOOLEAN Delete );
BOOLEAN PGPWDNTIsDirectory(PDEVICE_OBJECT DeviceObject, 
                            PFILE_OBJECT FileObject );


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

