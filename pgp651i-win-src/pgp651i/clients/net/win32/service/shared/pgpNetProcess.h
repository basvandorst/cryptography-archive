/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Portions Copyright (c) 1994-1996 Microsoft Corporation. 
	All rights reserved.

	$Id: pgpNetProcess.h,v 1.1 1999/01/07 23:03:52 elowe Exp $
____________________________________________________________________________*/

#ifndef _Included_pgpNetProcess_h
#define _Included_pgpNetProcess_h

#define TITLE_SIZE          64
#define PROCESS_SIZE        16

#include "pgpTypes.h"

//
// task list structure
//
typedef struct _TASK_LIST {
    PGPUInt32	dwProcessId;
    char		ProcessName[PROCESS_SIZE];
} TASK_LIST, *PTASK_LIST;

PGPUInt32	GetTaskList(PTASK_LIST pTask, PGPUInt32 dwNumTasks);

#endif // _Included_pgpNetProcess_h