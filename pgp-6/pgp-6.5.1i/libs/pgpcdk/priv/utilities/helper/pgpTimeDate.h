/*
 * pgpTimeDate.h -- functions to convert between a PGPUInt32 timestamp and
 * various time and date values (uncluding ISO string represenations
 *
 * $Id: pgpTimeDate.h,v 1.14 1999/02/11 01:15:29 hal Exp $
 */

#ifndef Included_pgpTimeDate_h
#define Included_pgpTimeDate_h

#include <time.h>

#include "pgpUsuals.h"
#include "pgpTypes.h"

PGP_BEGIN_C_DECLARATIONS

struct tm  *	pgpLocalTime(PGPTime const *theTime);
char  *			pgpCTime(PGPTime const *theTime);

#include "pgpUtilities.h"

/* Convert between number of days since 1970 Jan 1 and year/month/day */
unsigned	pgpDateFromYMD (int y, int m, int d);
void		pgpDateToYMD (unsigned days, int *year, int *month, int *day);

/*
 * Return the current timestamp.  tzFix is +/- #hours to off from GMT
 * if local conversions don't work properly.  (c.f. TZFIX
 * configuration variable)
 */
PGPTime		pgpTimeStamp (PGPInt32 tzFix);

/* Create strings for the Date or Time-and-date */
#define PGPDATESTRINGLEN 10
int			pgpDateString (PGPTime tstamp, char buf[PGPDATESTRINGLEN+1]);

#define PGPTIMESTRINGLEN 20
int			pgpTimeString (PGPTime tstamp, char buf[PGPTIMESTRINGLEN+1]);

#define PGPX509TIMELEN 15
int			pgpTimeToX509Time (PGPTime tstamp, char buf[PGPX509TIMELEN+1]);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpTimeDate_h */
