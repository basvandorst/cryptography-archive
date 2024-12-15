/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: PGPwd.h,v 1.2 1999/05/11 04:53:40 wjb Exp $
____________________________________________________________________________*/

// ********************** 95 DRIVER INFO

#define VXD_NAME		"\\\\.\\PGPwd9x.VXD"
#define VXD_REGISTER_CB 2       //register a callback and logging conditions
#define VXD_REC_DONE    3       //release the buffer VxD reported to us

// ********************** NT DRIVER INFO

#define FILE_DEVICE_WD	0x00008320

#ifndef CTL_CODE

typedef struct {
   void * Flink;
   void * Blink;
} WORK_LIST_ENTRY;

typedef struct _WORK_QUEUE_ITEM {
    WORK_LIST_ENTRY List;
    void * WorkerRoutine;
    void * Parameter;
} WORK_QUEUE_ITEM;

typedef struct {
   void *                       Irp;
   WORK_QUEUE_ITEM            WorkItem;
   void *                      Token;
   void *			        DeviceObject;
   char *		             FileName;
} PGPWDNT_COMPLETE_CONTEXT;

//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

//
// Define the method codes for how buffers are passed for I/O and FS controls
//

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

//
// Define the access check value for any access
//
//
// The FILE_READ_ACCESS and FILE_WRITE_ACCESS constants are also defined in
// ntioapi.h as FILE_READ_DATA and FILE_WRITE_DATA. The values for these
// constants *MUST* always be in sync.
//


#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe

#endif

//
// commands that the GUI can send the device driver
//
#define PGPWDNT_REC_DONE (ULONG) CTL_CODE( FILE_DEVICE_WD, 0x00, METHOD_NEITHER, FILE_ANY_ACCESS )
#define PGPWDNT_REGISTER_CB (ULONG) CTL_CODE( FILE_DEVICE_WD, 0x01, METHOD_NEITHER, FILE_ANY_ACCESS )
#define PGPWDNT_MAP_MEM (ULONG) CTL_CODE( FILE_DEVICE_WD, 0x02, METHOD_NEITHER, FILE_ANY_ACCESS )

// ********************** COMMON INFO

struct trap_criteria   
{
	unsigned long tc_callback;    //address to call back
};


struct trap_record     
{
	char tr_file1[300];
	void *keFileName;
};

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
