/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.

	Portions Copyright (c) 1994-1996 Microsoft Corporation. 
	All rights reserved.

	$Id: pgpNetProcess.cpp,v 1.1 1999/01/07 23:03:51 elowe Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <winperf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pgpTypes.h"

#include "pgpNetProcess.h"

// manafest constants
static const PGPInt32 INITIAL_SIZE		= 51200;
static const PGPInt32 EXTEND_SIZE 		= 25600;
static const char REGKEY_PERF[]	=	"software\\microsoft\\windows nt\\"
									"currentversion\\perflib";
static const char REGSUBKEY_COUNTERS[]	= "Counters";
static const char PROCESS_COUNTER[]		= "process";
static const char PROCESSID_COUNTER[]	= "id process";
static const char UNKNOWN_TASK[]		= "unknown";

//
// Routine Description:
//
//   Provides an API for getting a list of tasks running at the time of the
//   API call.  This function uses the registry performance data to get the
//   task list and is therefor straight WIN32 calls that anyone can call.
//
// Arguments:
//
//    dwNumTasks    - maximum number of tasks that the pTask array can hold
//
// Return Value:
//
//    Number of tasks placed into the pTask array.
//
PGPUInt32
GetTaskList(PTASK_LIST pTask, PGPUInt32 dwNumTasks)
{
	HKEY		hKeyNames;
    CHAR		szSubKey[1024];
    PGPUInt32	dwProcessIdTitle;
    PGPUInt32	dwProcessIdCounter;
    CHAR		szProcessName[MAX_PATH];
    PGPUInt32	dwLimit = dwNumTasks - 1;

    //
    // Look for the list of counters.  Always use the neutral
    // English version, regardless of the local language.  We
    // are looking for some particular keys, and we are always
    // going to do our looking in English.  We are not going
    // to show the user the counter names, so there is no need
    // to go find the corresponding name in the local language.
    //
    LANGID lid = MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL );
    sprintf(szSubKey, "%s\\%03x", REGKEY_PERF, lid );
    PGPUInt32 rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
								szSubKey,
								0,
								KEY_READ,
								&hKeyNames);
    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

    //
    // get the buffer size for the counter names
    //
	DWORD dwType, dwSize;
    rc = RegQueryValueEx(hKeyNames,
						 REGSUBKEY_COUNTERS,
						 NULL,
						 &dwType,
						 NULL,
						 &dwSize);

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

    //
    // allocate the counter names buffer
    //
    PGPByte * buf;
	buf = (LPBYTE) malloc(dwSize);
    if (buf == NULL) {
        goto exit;
    }
    memset(buf, 0, dwSize);

    //
    // read the counter names from the registry
    //
    rc = RegQueryValueEx(hKeyNames,
						 REGSUBKEY_COUNTERS,
						 NULL,
						 &dwType,
						 buf,
						 &dwSize);

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

    //
    // now loop thru the counter names looking for the following counters:
    //
    //      1.  "Process"           process name
    //      2.  "ID Process"        process id
    //
    // the buffer contains multiple null terminated strings and then
    // finally null terminated at the end.  the strings are in pairs of
    // counter number and counter name.
    //
    char *p, *p2;
	p = (char*)buf;
    while (*p) {
        if (p > (char*)buf) {
            for( p2=p-2; isdigit(*p2); p2--) ;
		}
        if (stricmp(p, PROCESS_COUNTER) == 0) {
            //
            // look backwards for the counter number
            //
            for( p2=p-2; isdigit(*p2); p2--) ;
            strcpy( szSubKey, p2+1 );
        }
        else if (stricmp(p, PROCESSID_COUNTER) == 0) {
			//
			// look backwards for the counter number
			//
			for( p2=p-2; isdigit(*p2); p2--) ;
			dwProcessIdTitle = atol( p2+1 );
        }
        //
        // next string
        //
        p += (strlen(p) + 1);
    }

    //
    // free the counter names buffer
    //
    free(buf);

    //
    // allocate the initial buffer for the performance data
    //
    dwSize = INITIAL_SIZE;
    buf = (PGPByte *)malloc( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );

    PPERF_DATA_BLOCK			pPerf;
    PPERF_OBJECT_TYPE			pObj;
    PPERF_INSTANCE_DEFINITION	pInst;
    PPERF_COUNTER_BLOCK			pCounter;
    PPERF_COUNTER_DEFINITION	pCounterDef;
	
    while (TRUE) {
		
        rc = RegQueryValueEx(HKEY_PERFORMANCE_DATA,
							 szSubKey,
							 NULL,
							 &dwType,
							 buf,
							 &dwSize);

        pPerf = (PPERF_DATA_BLOCK) buf;

        //
        // check for success and valid perf data block signature
        //
        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' ) {
            break;
        }
		
        //
        // if buffer is not big enough, reallocate and try again
        //
        if (rc == ERROR_MORE_DATA) {
            dwSize += EXTEND_SIZE;
            buf = (PGPByte *) realloc( buf, dwSize );
            memset(buf, 0, dwSize );
        }
        else {
            goto exit;
        }
    }

    //
    // set the perf_object_type pointer
    //
    pObj = (PPERF_OBJECT_TYPE) ((DWORD)pPerf + pPerf->HeaderLength);

    //
    // loop thru the performance counter definition records looking
    // for the process id counter and then save its offset
    //
    pCounterDef = (PPERF_COUNTER_DEFINITION)((DWORD)pObj+pObj->HeaderLength);
    PGPUInt32 i;
    for (i=0; i<(DWORD)pObj->NumCounters; i++) {
        if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle) {
            dwProcessIdCounter = pCounterDef->CounterOffset;
            break;
        }
        pCounterDef++;
    }
	
    dwNumTasks = min(dwLimit, (DWORD)pObj->NumInstances );
	
    pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD)pObj + pObj->DefinitionLength);
	
    //
    // loop thru the performance instance data extracting each process name
    // and process id
    //
    for (i=0; i<dwNumTasks; i++) {
        //
        // pointer to the process name
        //
        p = (LPSTR) ((DWORD)pInst + pInst->NameOffset);

        //
        // convert it to ascii
        //
        rc = WideCharToMultiByte(CP_ACP,
								 0,
								 (LPCWSTR)p,
								 -1,
								 szProcessName,
								 sizeof(szProcessName),
								 NULL,
								 NULL);

        if (!rc) {
            //
			// if we cant convert the string then use a default value
            //
            strcpy( pTask->ProcessName, UNKNOWN_TASK );
        }
		
        if (strlen(szProcessName)+4 <= sizeof(pTask->ProcessName)) {
            strcpy( pTask->ProcessName, szProcessName );
            strcat( pTask->ProcessName, ".exe" );
        }
		
        //
        // get the process id
        //
        pCounter = (PPERF_COUNTER_BLOCK) ((DWORD)pInst + pInst->ByteLength);
        pTask->dwProcessId = *((LPDWORD) ((DWORD)pCounter+dwProcessIdCounter));
        if (pTask->dwProcessId == 0) {
            pTask->dwProcessId = (DWORD)-2;
        }
		
        //
        // next process
        //
        pTask++;
        pInst = (PPERF_INSTANCE_DEFINITION)
			((DWORD)pCounter + pCounter->ByteLength);
    }
	
exit:
    if (buf) {
        free( buf );
    }
	
    RegCloseKey(hKeyNames);
    RegCloseKey(HKEY_PERFORMANCE_DATA);
	
    return dwNumTasks;
}
