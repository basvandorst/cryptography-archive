/*
 * pgpTimeDate.c -- conversions between a PGPUInt32 timestamp and year,
 * month, day, and time.  A bunch of convenience functions.
 *
 * $Id: pgpTimeDate.c,v 1.19 1998/04/24 21:01:33 heller Exp $
 */

#include <string.h>
#include "pgpConfig.h"

#include <stdio.h>
#include <time.h>

#include "pgpTimeDate.h"
#include "pgpUsuals.h"
#include "pgpTypes.h"
#include "pgpMem.h"

static PGPBoolean	offsetsComputed = FALSE;

#define kSecondsPerDay	(24UL * 60UL * 60UL )

/*
 * Offset in seconds from UTC
 */
static long		tzDiff;

/*
 * Compute local time_t representation of
 * Midnight Jan 1 1970 local time.
 */
static time_t		stdEpochLocal;
typedef struct tm	PGPtm;

	static void
pgpComputeOffsets(void)
{
	static	PGPtm	stdEpochStruct = { 0, 0, 0, 1, 0, 70, 0, 0, 0 };
	time_t			testTime = 10000000;
	PGPtm *			utcTime;
	PGPtm			timeStruct;
	
	if (!offsetsComputed)
	{
		stdEpochLocal = mktime(&stdEpochStruct);
		
		utcTime = gmtime(&testTime);
		if (utcTime == NULL)
		{
			/*
			 * Time zone information not available,
			 * so just pretend we're in UTC.
			 */
			tzDiff = 0;
		}
		else
		{
			pgpCopyMemory(utcTime, &timeStruct, sizeof(timeStruct));
			tzDiff = testTime - mktime(&timeStruct);
		}
		offsetsComputed = TRUE;
	}
}


	static time_t
pgpStdEpoch(void)
{
	pgpComputeOffsets();
	return stdEpochLocal + tzDiff;
}


	PGPtm *
pgpLocalTime(
	PGPTime const *	theTime)
{
	time_t		stdTime = PGPGetStdTimeFromPGPTime(*theTime);
	
	return localtime(&stdTime);
}

	char *
pgpCTime(
	PGPTime const *	theTime)
{
	time_t		stdTime = PGPGetStdTimeFromPGPTime(*theTime);
	
	return ctime(&stdTime);
}






/* System clock must be broken if it isn't past this date: */
#define REASONABLE_DATE ((unsigned long) 0x27804180L)  /* 91 Jan 01 00:00:00 */

/*
 * Convert a year-month-day to a number of days since Jan 1, 1970
 * Month and day are 1-based.
 *
 * To compute the day of the week, note that Jan 1, 1970 is a Thursday.
 */
unsigned
pgpDateFromYMD (int y, int m, int d)
{
	unsigned t;
	static const unsigned startday[12] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
	/*	 J   F   M   A    M    J    J    A    S    O    N    D */

	/* Number of 4-years since 1968 */
	t = (y - 1968)/4;
	/* Number of days in the 4-years */
	t *= 365*4+1;
	/* Add days in years since */
	t += 365 * (y%4);
	/* Add leap day unless before march in year 0 of cycle */
	if (y % 4 != 0 || m > 2)
		t++;
	/* Adjust to 1970 base */
	t -= 365*2+1;
	/* Add month and day offset */
	t += startday[m-1] + d-1;

	return t;
}

/*
 * Given timestamp as days elapsed since 1970 Jan 1,
 * returns year (1970-2106), month (1-12), and day (1-31).
 * Not valid for dates after 2100 Feb 28 (no leap day that year).
 */
void
pgpDateToYMD (unsigned days, int *year, int *month, int *day)
{
	int y, m;
	static const PGPByte mdays[12] =	/* March..February */
		{31,30,31,30,31,31,30,31,30,31,31,29};

	days += 365+365-31-28;	/* Days since Fri 1 Mar 1968 */

	/* Locate to within a leap-year cycle */
	y = days / 1461;
	days %= 1461;
	y *= 4;

	/* Find the year within the cycle */
	if (days >= 730) {
		y += 2;
		days -= 730;
	}
	if (days >= 365) {
		y++;
		days -= 365;
	}

	*year = y;

	/* days is now in the range [0..365] */
	/* Compute month, based on March=0 */
	m = 0;
	while (days >= mdays[m])
		days -= mdays[m++];

	/* Now, fix the month so March = 2, with carry into the year */
	m += 2;
	if (m >= 12) {	/* January or February of the next year */
		m -= 12;
		y++;
	}

	/* Save the final (1-based) results */
	*year = y + 1968;
	*month = m+1;
	*day = days+1;
}



	void
PGPGetYMDFromPGPTime(
	PGPTime		theTime,
	PGPUInt16 *	yearPtr,
	PGPUInt16 *	monthPtr,
	PGPUInt16 *	dayPtr )
{
	int		year;
	int		month;
	int		day;
	
	pgpDateToYMD( theTime / kSecondsPerDay, &year, &month, &day );
	
	if ( IsntNull( yearPtr ) )
		*yearPtr	= year;
		
	if ( IsntNull( monthPtr ) )
		*monthPtr	= month;
		
	if ( IsntNull( dayPtr ) )
		*dayPtr		= day;
}


						
						
	PGPTime
pgpTimeStamp (int tzFix)
{
	return (PGPUInt32)(PGPGetTime() + (60 * 60 * tzFix));
}

/* XXX: The routine below fails on Mac's when the timezone isn't set */
#if 0	/* [ */
/*
 * It's a bit annoying that we have to go through these shenanigans
 * to get seconds since Jan 1, 1970 0:00:00 GMT (okay, okay, UTC),
 * when time() on half the platforms in the world returns this value
 * already, but some platforms (e.g. Microsoft C, version 7.0 - but
 * not earlier versions!) return different values.
 *
 * So this uses the ANSI C functions.  tm_yday could be used to reduce the
 * work, but this is hardly a time-critical function and tm_yday is
 * infrequently used, so it might be buggy.
 *
 * ANSI allows the time() and gmtime() to fail, so this returns 0 in that
 * case.
 */
PGPTime
pgpTimeStamp (PGPInt32 tzFix)
{
	time_t const t = time((time_t *)0);
	PGPtm const *g;
	unsigned days;

	if (t == (time_t)-1)
		return 0;
 	g = gmtime(&t);
	if (!g)
		return 0;

	/* tm_year is 1900-based, tm_mon is 0-based, tm_day is 1-based */
	/* Some systems do tm_year oddly, though... */
	days = pgpDateFromYMD(g->tm_year + (g->tm_year > 1900 ? 0 : 1900),
	                      g->tm_mon+1, g->tm_mday);
	return g->tm_sec+60*(g->tm_min+60*(g->tm_hour+tzFix+24*(PGPUInt32)days));
}
#endif	/* ] 0 */

/*
 * Create a date string, given a 32-bit timestamp.  Returns characters
 * printed.  Format: "yyyy-mm-dd"
 *                    1234567890
 * This format is specified by some international ISO standard, but I
 * forget which one.
 */
int
pgpDateString (PGPTime tstamp, char buf[PGPDATESTRINGLEN+1])
{
	int month, day, year;

	if (tstamp == 0) {
		pgpFillMemory (buf, 10, ' ');
		buf[10] = '\0';
		return 10;
	}

	pgpDateToYMD((unsigned)(tstamp/86400), &year, &month, &day);
	sprintf (buf, "%4d-%02d-%02d", year, month, day);
	return PGPDATESTRINGLEN;
}

/*
 * Create a "date and time" string, given a 32-bit timestamp.
 * Returns number of characters printed.
 * Format: "yyyy-mm-dd hh:mm GMT"
 *          12345678901234567890
 */
int
pgpTimeString (PGPTime tstamp, char buf[PGPTIMESTRINGLEN+1])
{
	unsigned min;

	pgpDateString (tstamp, buf);
	tstamp /= 60;	/* Minutes since 00:00 Jan 1, 1970 */
	min = (unsigned)(tstamp % 1440);	/* Minutes since 0:00 today */
	sprintf (buf+PGPDATESTRINGLEN, " %02d:%02d GMT", min/60, min%60);
	return PGPTIMESTRINGLEN;
}


#if PGP_MACINTOSH	/* [ */

	static long
pgpTZDiff(void)
{
	pgpComputeOffsets();
	return tzDiff;
}



/*
 * Number of seconds between Midnight Jan 1, 1904 (Mac time base)
 * and Midnight Jan 1, 1970 (Un*x time base)
 */
#define kMacTimeDiff	2082844800

	PGPUInt32
PGPTimeToMacTime(PGPTime theTime)
{
	return theTime + kMacTimeDiff + pgpTZDiff();
}

	PGPTime
PGPTimeFromMacTime(PGPUInt32 theTime)
{
	return theTime - kMacTimeDiff - pgpTZDiff();
}

#endif	/* ] PGP_MACINTOSH */



	time_t
PGPGetStdTimeFromPGPTime(
	PGPTime			theTime)
{
	return theTime + pgpStdEpoch();
}



	PGPTime
PGPGetPGPTimeFromStdTime( time_t	theTime)
{
	return theTime - pgpStdEpoch();
}


	PGPTime
PGPGetTime(void)
{
	return PGPGetPGPTimeFromStdTime(time((time_t *) NULL));
}



#ifdef TESTMAIN
#include <stdlib.h>

int
main(void)
{
	unsigned i, t;
	int y, m, d;

	puts("Starting test...");
	for (i = 0; i < 65535; i++) {
		if (i % 1000 == 0)
			printf("% 5u\r", i);
		pgpDateToYMD(i, &y, &m, &d);
		t = pgpDateFromYMD(y, m, d);
		if (i != t) {
			printf("\ni = %u != t = %u   y/m/d = %04d/%02d/%02d\n",
				i, t, y, m, d);
		}
	}
	puts("\nDone!");
	return 0;
}

#endif