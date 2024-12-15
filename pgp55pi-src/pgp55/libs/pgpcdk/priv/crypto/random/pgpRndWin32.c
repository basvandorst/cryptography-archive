/*
 * Get high-resolution timing information to seed the random number
 * generator.  Win32 version.
 *
 * $Id: pgpRndWin32.c,v 1.7 1997/06/25 17:58:56 lloyd Exp $
 */

#include "pgpConfig.h"

#include <windows.h>

#include "pgpRandomContext.h"
#include "pgpRnd.h"

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
	LARGE_INTEGER newperf;
	SYSTEMTIME stime;

	if (QueryPerformanceCounter(&newperf)) {
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
