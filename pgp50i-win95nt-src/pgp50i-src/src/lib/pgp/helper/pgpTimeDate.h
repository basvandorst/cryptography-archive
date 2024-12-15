/*
* pgpTimeDate.h -- functions to convert between a word32 timestamp and
* various time and date values (uncluding ISO string
* represenations.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpTimeDate.h,v 1.4.2.1 1997/06/07 09:50:14 mhw Exp $
*/

#ifndef PGPTIMEDATE_H
#define PGPTIMEDATE_H

#include <time.h>

#include "pgpUsuals.h"
#include "pgpTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef word32	PGPTime;

	time_t PGPExport			pgpFromPGPTime(PGPTime theTime);
	PGPTime PGPExport			pgpToPGPTime(time_t theTime);

	PGPTime PGPExport			pgpGetTime(void);
struct tm PGPExport *	pgpLocalTime(PGPTime const *theTime);
	char PGPExport *			pgpCTime(PGPTime const *theTime);

	#if MACINTOSH	/* [ */
	ulong PGPExport			pgpTimeToMacTime(PGPTime theTime);
	PGPTime PGPExport			pgpTimeFromMacTime(ulong theTime);
	#endif	 /* ] MACINTOSH */

/* Convert between number of days since 1970 Jan 1 and year/month/day */
unsigned PGPExport pgpDateFromYMD (int y, int m, int d);
void PGPExport pgpDateToYMD (unsigned days, int *year, int *month, int *day);

/*
* Return the current timestamp. tzFix is +/- #hours to off from GMT
* if local conversions don't work properly. (c.f. TZFIX
* configuration variable)
*/
word32 PGPExport pgpTimeStamp (int tzFix);

/* Create strings for the Date or Time-and-date */
#define PGPDATESTRINGLEN 10
int PGPExport pgpDateString (word32 tstamp, char buf[PGPDATESTRINGLEN+1]);
#define PGPTIMESTRINGLEN 20
int PGPExport pgpTimeString (word32 tstamp, char buf[PGPTIMESTRINGLEN+1]);

#ifdef __cplusplus
}
#endif

#endif /* PGPTIMEDATE_H */
