/*
* pgpRndMac.c -- Get high-resolution timing information to seed the random
* number generator. Mac version. (The complicated one.)
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRndMac.c,v 1.1.2.1 1997/06/07 09:51:34 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>	 /* For qsort() */

#include <Timer.h>

#include "pgpRnd.h"
#include "pgpRndom.h"
#include "pgpUsuals.h"

#define MINTICK 0
#define N 15	/* Number of deltas to try (at least 5, preferably odd) */


typedef UnsignedWide timetype;

/* Function needed for qsort() */
static int
ranCompare(void const *p1, void const *p2)
{
	return *(unsigned const *)p1 > *(unsigned const *)p2 ? 1 :
	*(unsigned const *)p1 < *(unsigned const *)p2 ? -1 : 0;
}

/*
* XXX: Not necessarily accurate!
*
* This returns t1 - t2 if it fits in an unsigned long, or else it returns 0
* if negative or ULONG_MAX if out of range.
*/
static unsigned long
tickdiff(UnsignedWide const t1, UnsignedWide const t2)
	{
			if (t1.hi == t2.hi)
			{
					if (t1.lo >= t2.lo)
						return t1.lo - t2.lo;
					else
						return 0;
			}
			else if (t1.hi == t2.hi + 1)
			{
					if (t1.lo < t2.lo)
						return t1.lo - t2.lo;	/* This underflows, */
							 		 /* and that's the intent */
				else
						return ULONG_MAX;
			}
else if (t1.hi < t2.hi)
			return 0;
	else
			return ULONG_MAX;
}

/*
* Find the resolution of the high-resolution clock by sampling successive
* values until a tick boundary, at which point the delta is entered into
* a table. An average near the median of the table is taken and returned
* as the system tick size to eliminate outliers due to descheduling (high)
* or tv0 not being the "zero" time in a given tick (low).
*
* Some trickery is needed to defeat the habit systems have of always
* incrementing the microseconds field from gettimeofday() results so that
* no two calls return the same value. Thus, a "tick boundary" is assumed
* when successive calls return a difference of more than MINTICK ticks.
* (For gettimeofday(), this is set to 2 us.) This catches cases where at
* most one other task reads the clock between successive reads by this task.
* More tasks in between are rare enough that they'll get cut off by the
* median filter.
*
* When a tick boundary is found, the *first* time read during the previous
* tick (tv0) is subtracted from the new time to get microseconds per tick.
*
* Suns have a 1 us timer, and as of SunOS 4.1, they return that timer, but
* there is ~50 us of system-call overhead to get it, so this overestimates
* the tick size considerably. On SunOS 5.x/Solaris, the overhead has been
* cut to about 2.5 us, so the measured time alternates between 2 and 3 us.
* Some better algorithms will be required for future machines that really
* do achieve 1 us granularity.
*
* Current best idea: discard all this hair and use Ueli Maurer's entropy
* estimation scheme. Assign each input event (delta) a sequence number.
* 16 bits should be more than adequate. Make a table of the last time
* (by sequence number) each possibe input event occurred. For practical
* implementation, hash the event to a fixed-size code and consider two
* events identical if they have the same hash code. This will only ever
* underestimate entropy. Then use the number of bits in the difference
* between the current sequence number and the previous one as the entropy
* estimate.
*
* If it's desirable to use longer contexts, Maurer's original technique
* just groups events into non-overlapping pairs and uses the technique on
* the pairs. If you want to increment the entropy numbers on each keystroke
* for user-interface niceness, you can do the operation each time, but you
* have to halve the sequence number difference before starting, and then you
* have to halve the number of bits of entropy computed because you're adding
* them twice.
*
* You can put the even and odd events into separate tables to close Maurer's
* model exactly, or you can just dump them into the same table, which will
* be more conservative.
*/
static unsigned long
ranTickSize(void)
{
int i = 0, j = 0;
unsigned long diff, d[N];
timetype tv0, tv1, tv2;
	
			/*
			* TODO Get some per-run data to seed the RNG with.
			* pid, ppid, etc.
			*/
			Microseconds(&tv0);
			tv1 = tv0;
			do {
					Microseconds(&tv2);
					diff = tickdiff(tv2, tv1);
					if (diff > MINTICK) {
						d[i++] = diff;
						tv0 = tv2;
						j = 0;
					} else if (++j >= 4096)	/* Always getting <= MINTICK units */
						return MINTICK + !MINTICK;
					tv1 = tv2;
			} while (i < N);

			/* Return average of middle 5 values (rounding up) */
			qsort(d, N, sizeof(d[0]), ranCompare);
			diff = (d[N/2-2]+d[N/2-1]+d[N/2]+d[N/2+1]+d[N/2+2]+4)/5;
#if NOISEDEBUG
			fprintf(stderr, "Tick size is %u\n", diff);
#endif
			return diff;
	}

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
			timetype t;
			static unsigned long ticksize = 0;
			static timetype prevt;

			Microseconds(&t);
			pgpRandomAddBytes(rc, (byte const *)&t, sizeof(t));

			if (!ticksize)
			{
				 ticksize = ranTickSize();
				 prevt = t;
			}
			delta = (word32)(tickdiff(t, prevt) / ticksize);
			prevt = t;

return delta;
}