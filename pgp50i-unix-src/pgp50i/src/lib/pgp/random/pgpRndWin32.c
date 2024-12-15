/*
* pgpRndWin32.c -- Get high-resolution timing information to seed the random
* number generator. Win32 version.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRndWin32.c,v 1.2.2.1 1997/06/07 09:51:36 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <windows.h>

#include "pgpUsuals.h"
#include "pgpRndom.h"
#include "pgpRnd.h"

/*
* Add as much timing-dependent random noise as possible
* to the randPool. Typically, this involves reading the most
* accurate system clocks available.
*
* Returns the number of ticks that have passed since the last call,
* for entropy estimation purposes.
*/
word32
ranGetEntropy(struct PgpRandomContext const *rc)
	{
			word32 delta;
			static WORD oldms;
			static DWORD oldlow;
			LARGE_INTEGER newperf;
			SYSTEMTIME stime;

if (QueryPerformanceCounter(&newperf)) {
	pgpRandomAddBytes(rc, (byte *)&newperf, sizeof(newperf));
	delta = newperf.LowPart - oldlow;
	oldlow = newperf.LowPart;
	} else {
			GetSystemTime(&stime);
			pgpRandomAddBytes(rc, (byte *)&stime, sizeof(stime));
			delta = stime.wMilliseconds - oldms;
			oldms = stime.wMilliseconds;
			delta /= 10;	 /* crude resolution */
	}

return delta;
}
