/*
 * Get high-resolution timing information to seed the random number generator.
 * Mac version.  (The complicated one.)
 *
 * $Id: pgpRndMac.c,v 1.15 1999/04/21 18:35:38 heller Exp $
 */

#include "pgpConfig.h"

#include <stdlib.h>	/* For qsort() */

#include <Timer.h>

#include "pgpMem.h"
#include "pgpRnd.h"
#include "pgpRandomPoolPriv.h"
#include "pgpUsuals.h"

#define MINTICK 0

/* Number of deltas to try (at least 5, preferably odd) */
#define kNumDeltas 15	

typedef UnsignedWide timetype;

/* Function needed for qsort() */
static int
ranCompare(void const *p1, void const *p2)
{
	return *(PGPUInt32 const *)p1 > *(PGPUInt32 const *)p2 ?  1 :
	       *(PGPUInt32 const *)p1 < *(PGPUInt32 const *)p2 ? -1 : 0;
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

#if TARGET_CPU_PPC
/* Sample the timebase register */

static asm void GetClock(register timetype *t)
{
	mftb	r7, 268
	stw		r7, 4(t)
	mftb	r7, 269
	stw		r7, 0(t)
	blr
}

#else

static void GetClock(register timetype *t)
{
	Microseconds( t );
}

#endif

/*
 * Find the resolution of the high-resolution clock by sampling successive
 * values until a tick boundary, at which point the delta is entered into
 * a table.  An average near the median of the table is taken and returned
 * as the system tick size to eliminate outliers due to descheduling (high)
 * or tv0 not being the "zero" time in a given tick (low).
 *
 * Some trickery is needed to defeat the habit systems have of always
 * incrementing the microseconds field from gettimeofday() results so that
 * no two calls return the same value.  Thus, a "tick boundary" is assumed
 * when successive calls return a difference of more than MINTICK ticks.
 * (For gettimeofday(), this is set to 2 us.)  This catches cases where at
 * most one other task reads the clock between successive reads by this task.
 * More tasks in between are rare enough that they'll get cut off by the
 * median filter.
 *
 * When a tick boundary is found, the *first* time read during the previous
 * tick (tv0) is subtracted from the new time to get microseconds per tick.
 *
 * Suns have a 1 us timer, and as of SunOS 4.1, they return that timer, but
 * there is ~50 us of system-call overhead to get it, so this overestimates
 * the tick size considerably.  On SunOS 5.x/Solaris, the overhead has been
 * cut to about 2.5 us, so the measured time alternates between 2 and 3 us.
 * Some better algorithms will be required for future machines that really
 * do achieve 1 us granularity.
 *
 * Current best idea: discard all this hair and use Ueli Maurer's entropy
 * estimation scheme.  Assign each input event (delta) a sequence number.
 * 16 bits should be more than adequate.  Make a table of the last time
 * (by sequence number) each possibe input event occurred.  For practical
 * implementation, hash the event to a fixed-size code and consider two
 * events identical if they have the same hash code.  This will only ever
 * underestimate entropy.  Then use the number of bits in the difference
 * between the current sequence number and the previous one as the entropy
 * estimate.
 *
 * If it's desirable to use longer contexts, Maurer's original technique
 * just groups events into non-overlapping pairs and uses the technique on
 * the pairs.  If you want to increment the entropy numbers on each keystroke
 * for user-interface niceness, you can do the operation each time, but you
 * have to halve the sequence number difference before starting, and then you
 * have to halve the number of bits of entropy computed because you're adding
 * them twice.
 *
 * You can put the even and odd events into separate tables to close Maurer's
 * model exactly, or you can just dump them into the same table, which will
 * be more conservative.
 */
static PGPUInt32
ranTickSize(void)
{
	int i = 0, j = 0;
	PGPUInt32	diff, d[kNumDeltas];
	timetype	tv0, tv1, tv2;
	
	/*
	 * TODO Get some per-run data to seed the RNG with.
	 * pid, ppid, etc.
	 */
	GetClock(&tv0);
	tv1 = tv0;
	do {
		GetClock(&tv2);
		diff = tickdiff(tv2, tv1);
		if (diff > MINTICK) {
			d[i++] = diff;
			tv0 = tv2;
			j = 0;
		} else if (++j >= 4096)	/* Always getting <= MINTICK units */
			return MINTICK + !MINTICK;
		tv1 = tv2;
	} while (i < kNumDeltas);

	/* Return average of middle 5 values (rounding up) */
	qsort(d, kNumDeltas, sizeof(d[0]), ranCompare);
	diff = (d[kNumDeltas / 2 - 2] + d[kNumDeltas / 2 - 1] +
			d[kNumDeltas / 2] + d[kNumDeltas / 2 + 1] +
			d[kNumDeltas/2 + 2] + 4) / 5;
#if NOISEDEBUG
	fprintf(stderr, "Tick size is %u\n", diff);
#endif
	return diff;
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
	timetype t;
	static PGPUInt32 ticksize = 0;
	static timetype prevt;

	GetClock(&t);
	pgpRandomAddBytes(rc, (PGPByte const *)&t, sizeof(t));

	if (!ticksize)
	{
		ticksize = ranTickSize();
		prevt = t;
	}
	delta = (PGPUInt32)(tickdiff(t, prevt) / ticksize);
	prevt = t;

	return delta;
}

/*
 * pgpRandomCollectOsData
 *	Add random process and thread performance data noise
 *	to the random pool.
 */
	PGPBoolean
pgpRandomCollectOsData(PGPRandomContext const *rc)
{
	long		gestaltResult;
	timetype 	t;
	const VCB	*curVCB;
	
	GetClock(&t);
	pgpRandomAddBytes(rc, (PGPByte const *)&t, sizeof(t));
	
	if( Gestalt( gestaltLowMemorySize, &gestaltResult ) == noErr )
	{
		/* Add all of lowmem (except first byte) into pool */
		pgpRandomAddBytes( rc, (PGPByte *) 1, gestaltResult - 1 );
	}
	
	/* Add all VCB's into the pool */
	curVCB = (const VCB *) GetVCBQHdr()->qHead;
	while( IsntNull( curVCB ) )
	{
		pgpRandomAddBytes( rc, (PGPByte *) curVCB, sizeof( *curVCB ) );
		
		curVCB = (const VCB *) curVCB->qLink;
	}
	
	return TRUE;
}