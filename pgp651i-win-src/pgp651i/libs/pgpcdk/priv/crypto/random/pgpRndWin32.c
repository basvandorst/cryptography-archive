/*
 * Get high-resolution timing information to seed the random number
 * generator.  Win32 version.
 *
 * $Id: pgpRndWin32.c,v 1.10 1999/05/13 23:00:55 cpeterson Exp $
 */

#include "pgpPFLErrors.h"
#include "pgpConfig.h"
#include "pgpRandomContext.h"
#include "pgpRnd.h"
#include "pgpMemoryMgr.h"

#include <windows.h>
#include <winperf.h>   // for Windows NT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>  // for Windows 95

/*
 * Defines for routines used to collect random performance
 * data from the system.
 */
#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         "software\\microsoft\\windows nt\\currentversion\\perflib"
#define REGSUBKEY_COUNTERS  "Counters"
struct perfdatahdr {
	int id;
	char name[80];
	struct perfdatahdr *next;
};
static struct perfdatahdr *hdrlist;
static char * sPerfCounterIdxToName();
static int sPerfCounterNameToIdx();
static void *sAllocPerfData_WinNT(unsigned long *);
static void *sAllocPerfData_Win95(unsigned long *);
static PGPBoolean sPerfAllocHdrList();
static void sPerfFreeHdrList();
PGPBoolean pgpRandomCollectOsData(PGPRandomContext const *);

// RDTSC instruction is not supported by Visual C++ inline assembler
// so we use this #define to fake it
#define rdtsc __asm __emit 0fh __asm __emit 031h

/*
 * Tests for presence of Pentium Time Stamp Counter high resolution
 * timer by executing the RDTSC instruction.  If the processor does
 * not support this instruction, an exception is generated and this
 * function returns FALSE.
 */

static BOOL
sTestForHiResTimer(VOID)
{
	BOOL	present = TRUE;

	__try
	{
		__asm 
		{
			rdtsc
		}
	}

	__except (EXCEPTION_EXECUTE_HANDLER) 
	{
		present = FALSE;
	}

	return present;
}

/*
 * Add as much timing-dependent random noise as possible
 * to the randPool.  Typically, this involves reading the most
 * accurate system clocks available.
 *
 * Returns the number of ticks that have passed since the last call,
 * for entropy estimation purposes.
 */

PGPUInt32
pgpRandomCollectEntropy(PGPRandomContext const *rc)
{
	PGPUInt32 delta;
	static WORD oldms;
	static DWORD oldlow;
	static BOOL tsctested = FALSE;
	static BOOL tscpresent = FALSE;
	DWORD tsclow;
	LARGE_INTEGER newperf;
	SYSTEMTIME stime;

	if (!tsctested) {
		tscpresent = sTestForHiResTimer ();
		tsctested = TRUE;
	}

	if (tscpresent) {
		__asm 
		{
			rdtsc
			mov		tsclow, eax
		}
		pgpRandomAddBytes(rc, (PGPByte *)&tsclow, sizeof(tsclow));
		delta = tsclow - oldlow;
		oldlow = tsclow;
	}
	else if (QueryPerformanceCounter(&newperf)) {
		pgpRandomAddBytes(rc, (PGPByte *)&newperf, sizeof(newperf));
		delta = newperf.LowPart - oldlow;
		oldlow = newperf.LowPart;
	} else {
		GetSystemTime(&stime);
		pgpRandomAddBytes(rc, (PGPByte *)&stime, sizeof(stime));
		delta = stime.wMilliseconds - oldms;
		oldms = stime.wMilliseconds;
		delta /= 10;	/* crude resolution */
	}

	return delta;
}

/*
 * pgpRandomCollectOsData()
 *	Add as much random performance data noise to the
 *	random pool as we can find.  WindowsNT and Windows95/98
 *	have different ways of getting per-process and per-thread
 *	information.
 */
PGPBoolean
pgpRandomCollectOsData(PGPRandomContext const *rc)
{
	void *perf_data;
	unsigned long amount;
	BOOL bIsWinNT;
	OSVERSIONINFO osid;
	LARGE_INTEGER newperf;
	SYSTEMTIME stime;
	PGPMemoryMgrRef memref;
	
	memref = PGPGetDefaultMemoryMgr();
	if (memref == kInvalidPGPMemoryMgrRef)
		return FALSE;

	if (QueryPerformanceCounter(&newperf))
		pgpRandomAddBytes(rc, (PGPByte *)&newperf, sizeof(newperf));

	GetSystemTime(&stime);
	pgpRandomAddBytes(rc, (PGPByte *)&stime, sizeof(stime));

	osid.dwOSVersionInfoSize = sizeof(osid);
	GetVersionEx(&osid);   
	bIsWinNT = (osid.dwPlatformId == VER_PLATFORM_WIN32_NT);

	if (bIsWinNT) {
		if (!sPerfAllocHdrList())
			return FALSE;
		perf_data = sAllocPerfData_WinNT(&amount);
		if (perf_data == NULL) {
			sPerfFreeHdrList();
			return FALSE;
		}
		pgpRandomAddBytes(rc, (PGPByte *)perf_data, amount);
		PGPFreeData(perf_data);
		sPerfFreeHdrList();
	}
	else {
		perf_data = sAllocPerfData_Win95(&amount);
		if (perf_data == NULL)
			return FALSE;
		pgpRandomAddBytes(rc, (PGPByte *)perf_data, amount);
		PGPFreeData(perf_data);
	}
	return TRUE;
}

/*
 * sAllocPerfData_WinNT
 *	Get the performance data in HKEY_PERFORMANCE_DATA corresponding
 *	to the "Global" counter subkey.  The returned buffer is allocated
 *	with malloc(), and free() needs to be called on it.
 */
static void *
sAllocPerfData_WinNT(unsigned long *amt)
{
	DWORD dwSize, dwType, rc;
	void *buf;
    char subkey_str[80];
	PERF_DATA_BLOCK *pPerf;
	PGPMemoryMgrRef memref;
	PGPError pgpErr;
	
	memref = PGPGetDefaultMemoryMgr();
	if (memref == kInvalidPGPMemoryMgrRef)
		return NULL;

	sprintf(subkey_str, "Global");

    /*
     * Allocate an initial buffer for the performance data,
	 * then repeat realloc() as we need more.
	 */
    dwSize = INITIAL_SIZE;
    buf = PGPNewData(memref, dwSize, kPGPMemoryMgrFlags_Clear  );
    if (buf == NULL)
        return NULL;

	for(;;) {
        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              subkey_str,
                              NULL,
                              &dwType,
                              buf,
                              &dwSize
                            );

        pPerf = (PPERF_DATA_BLOCK) buf;

        /*
         * Check for success and valid perf data block signature.
         */
        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' ) {
            break;
        }

        /*
         * If buffer is not big enough, reallocate and try again.
         */
        if (rc == ERROR_MORE_DATA) {
            dwSize += EXTEND_SIZE;
            pgpErr = PGPReallocData( memref, &buf, dwSize, kPGPMemoryMgrFlags_Clear );
			if (IsPGPError(pgpErr)) {
				return NULL;
			}
        }
        else {
			PGPFreeData(buf);
            return NULL;
        }
    }

	*amt = dwSize;
	return buf;
}

/*
 * sPerfAllocHdrList
 *	Create the linked list of performance counter names.
 *	This list is freed by calling sPerfFreeHdrList();
 */
static PGPBoolean
sPerfAllocHdrList()
{
    HKEY		hKeyNames;
    DWORD		dwType, dwSize, rc;
    LPBYTE		buf;
    CHAR		szSubKey[1024], *p;
    LANGID		lid;
	PGPMemoryMgrRef memref;
	
	memref = PGPGetDefaultMemoryMgr();
	if (memref == kInvalidPGPMemoryMgrRef)
		return FALSE;

	hdrlist = NULL;
	/*
	 * MSDN says english-neutral is okay for this.
	 */
    lid = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL );
    sprintf( szSubKey, "%s\\%03x", REGKEY_PERF, lid );
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       szSubKey,
                       0,
                       KEY_READ,
                       &hKeyNames
                     );
    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    /*
     * Get the buffer size for the counter names
     */
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    /*
     * Allocate the counter names buffer.
     */
    buf = PGPNewData(memref, dwSize, kPGPMemoryMgrFlags_Clear  );
    if (buf == NULL) {
        return FALSE;
    }

    /*
     * Read the counter names from the registry.
     */
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          buf,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
		PGPFreeData(buf);
        return FALSE;
    }

	/*
	 * Load up all the Counter Names into hdrlist
	 */
	p = buf;
	while (*p) {
		struct perfdatahdr *hdrptr;
		hdrptr = PGPNewData(memref, sizeof(*hdrptr), kPGPMemoryMgrFlags_Clear);
		if (hdrptr == NULL) break;
		hdrptr->next = hdrlist;
		hdrlist = hdrptr;

		hdrptr->id = atoi(p);
		while (*p) ++p;
		++p;

		strcpy(hdrptr->name, p);
		while (*p) ++p;
		++p;
	}

	PGPFreeData(buf);

	return TRUE;
}

/*
 * sPerfFreeHdrList
 *	Free the linked list of counter names.
 */
static void
sPerfFreeHdrList()
{
	struct perfdatahdr *hdrptr, *next;

	for (hdrptr = hdrlist; hdrptr; hdrptr = next) {
		next = hdrptr->next;
		PGPFreeData(hdrptr);
	}
	hdrlist = NULL;
}

static char *
sPerfCounterIdxToName(int num)
{
	struct perfdatahdr *hdrptr;

	for (hdrptr=hdrlist; hdrptr; hdrptr = hdrptr->next) {
		if (hdrptr->id == num)
			return hdrptr->name;
	}
	return NULL;
}

static int
sPerfCounterNameToIdx(char *name)
{
	struct perfdatahdr *hdrptr;

	for (hdrptr=hdrlist; hdrptr; hdrptr = hdrptr->next) {
		if (!strcmp(hdrptr->name, name))
			return hdrptr->id;
	}
	return -1;
}


/*
 * sAllocPerfData_Win95
 *	This code uses the CreateToolhelp32Snapshot API to get the
 *	current list of procs and threads, and various counters about each.
 *	This will compile on WinNT 4.0, but won't run on WinNT 4.0 because
 *	the snapshot routines don't exist on WinNT 4.0.
 */

typedef BOOL (WINAPI *PROCESSWALK)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *THREADWALK)(HANDLE hSnapshot, LPTHREADENTRY32 lppe);
typedef HANDLE (WINAPI *CREATESNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);

static void *
sAllocPerfData_Win95(unsigned long *amt)
{
	HANDLE hThreadSnap, hProcessSnap, hKernel;
	THREADENTRY32 te32;
	PROCESSENTRY32 pe32;
	THREADWALK pThread32First, pThread32Next;
	PROCESSWALK pProcess32First, pProcess32Next;
	CREATESNAPSHOT pCreateToolhelp32Snapshot;
	char *databuf;
	unsigned long databuf_len;
	PGPMemoryMgrRef memref;
	PGPError pgpErr;

	memref = PGPGetDefaultMemoryMgr();
	if (memref == kInvalidPGPMemoryMgrRef)
		return NULL;

	hKernel = GetModuleHandle("KERNEL32.DLL");
	if (!hKernel) return NULL;

	pCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");
	pThread32First = (THREADWALK)GetProcAddress(hKernel, "Thread32First");
	pThread32Next = (THREADWALK)GetProcAddress(hKernel, "Thread32Next");
	pProcess32First = (PROCESSWALK)GetProcAddress(hKernel, "Process32First");
	pProcess32Next = (PROCESSWALK)GetProcAddress(hKernel, "Process32Next");

	if (!pCreateToolhelp32Snapshot || !pThread32First || !pThread32Next 
			|| !pProcess32First || !pProcess32Next)
		return NULL;

	databuf_len = 4096;
	databuf = PGPNewData(memref, databuf_len, kPGPMemoryMgrFlags_Clear);
	if (!databuf) return NULL;
	*amt = 0;

	/*
	 * PROCESS Snapshot.
	 */
	hProcessSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == (HANDLE)-1) {
		PGPFreeData(databuf);
		return NULL;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (pProcess32First(hProcessSnap, &pe32)) {
		do {
			if (*amt + pe32.dwSize > databuf_len) {
				pgpErr = PGPReallocData(memref, &databuf, databuf_len += 4096, 0);
				if (IsPGPError(pgpErr))
					return NULL;
			}
			memcpy(databuf + *amt, &pe32, pe32.dwSize);
			*amt += pe32.dwSize;

			pe32.dwSize = sizeof(PROCESSENTRY32);
		} while (pProcess32Next(hProcessSnap, &pe32));
	}
	CloseHandle(hProcessSnap);

	/*
	 * THREAD Snapshot.
	 */
	hThreadSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == (HANDLE)-1)
		return FALSE;
	te32.dwSize = sizeof(THREADENTRY32);
	if (pThread32First(hThreadSnap, &te32)) {
		do {
			if (*amt + te32.dwSize > databuf_len) {
				pgpErr = PGPReallocData(memref, &databuf, databuf_len += 4096, 0);
				if (IsPGPError(pgpErr))
					return NULL;
			}
			memcpy(databuf + *amt, &te32, te32.dwSize);
			*amt += te32.dwSize;

			te32.dwSize = sizeof(THREADENTRY32);
		} while (pThread32Next(hThreadSnap, &te32));
	}
	CloseHandle(hThreadSnap);

	return databuf;
}
