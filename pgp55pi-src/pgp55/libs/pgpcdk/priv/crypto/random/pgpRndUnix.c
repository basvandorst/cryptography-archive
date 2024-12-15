/*
 * Get high-resolution timing information to seed the random number generator.
 * Unix version.  (The complicated one.)
 *
 * $Id: pgpRndUnix.c,v 1.10 1997/08/30 03:08:08 hal Exp $
 */

#include "pgpConfig.h"

/*
 * This code uses five different timers, if available.  Each possibility
 * can be specifically enabled or disabled by predefining USE_XX to 1
 * or 0.  For some, the code attempts to detect availability automatically.
 * If the symbols HAVE_XX are defined, they are used.  If not, they are set
 * to reasonable defaults while other clues are checked.  The choices,
 * and the auto-detection methods used, are:
 * - gethrtime(), if HAVE_GETHRTIME is set to 1.
 * - clock_gettime(CLOCK_REALTIME,...), if CLOCK_REALTIME is in <time.h>
 * - gettimeofday(), assumed available unless HAVE_GETTIMEOFDAY=0
 * - getitimer(ITIMER_REAL,...), if ITIMER_REAL is defined in <sys/time.h>
 * - ftime(), assumed available unless HAVE_FTIME=0
 *
 * These are all accessed through the gettime(), timetype, and tickdiff()
 * macros.  The MINTICK constant is something to avoid the gettimeofday()
 * glitch wherein it increments the return value even if no tick has occurred.
 * When measuring the tick interval, if the difference between two successive
 * times is not at least MINTICK ticks, it is ignored.
 */

#ifndef TIME_WITH_SYS_TIME
#define TIME_WITH_SYS_TIME 1	/* Assume true if not told */
#endif

/*
 * Include <time.h> and <sys/time.h>
 */
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#if TIME_WITH_SYS_TIME
#include <time.h>
#endif
#else
#include <time.h>
#endif

#include <sys/types.h>
#include <sys/times.h>	/* for times() */
#include <stdlib.h>	/* For qsort() */

#include "pgpRnd.h"
#include "pgpRandomContext.h"
#include "pgpUsuals.h"

#ifndef USE_GETHRTIME
#define USE_GETHRTIME HAVE_GETHRTIME
#endif

#if USE_GETHRTIME
typedef hrtime_t timetype;
#define gettime(s) (*(s) = gethrtime())
#define tickdiff(s,t) ((s)-(t))
#define MINTICK 0

#else
#ifndef USE_CLOCK_GETTIME
#ifndef HAVE_CLOCK_GETTIME
#define HAVE_CLOCK_GETTIME 1
#endif
#if HAVE_CLOCK_GETTIME
#ifdef CLOCK_REALTIME
#define USE_CLOCK_GETTIME 1
#endif
#endif
#endif

#if USE_CLOCK_GETTIME

#define CHOICE_CLOCK_GETTIME 1
typedef struct timespec timetype;
#define gettime(s) (void)clock_gettime(CLOCK_REALTIME, s)
#define tickdiff(s,t) (((s).tv_sec-(t).tv_sec)*1000000000 + \
	(s).tv_nsec - (t).tv_nsec)
#ifndef HAVE_CLOCK_GETRES
#define HAVE_CLOCK_GETRES 1
#endif

#else

#ifndef USE_GETTIMEOFDAY
#ifndef HAVE_GETTIMEOFDAY
#define HAVE_GETTIMEOFDAY 1	/* Assume we have it... */
#endif
#define USE_GETTIMEOFDAY HAVE_GETTIMEOFDAY
#endif

#if USE_GETTIMEOFDAY
typedef struct timeval timetype;
#define gettime(s) (void)gettimeofday(s, (struct timezone *)0)
#define tickdiff(s,t) (((s).tv_sec-(t).tv_sec)*1000000+(s).tv_usec-(t).tv_usec)
#define MINTICK 1

#else

#ifndef USE_GETITIMER
#ifndef HAVE_GETITIMER
#define HAVE_GETITIMER 1
#endif
#ifndef HAVE_SETITIMER
#define HAVE_SETITIMER 1
#endif
#if HAVE_GETITIMER && HAVE_SETITIMER
#ifdef ITIMER_REAL
#define USE_GETITIMER 1
#endif
#endif
#endif

#if USE_GETITIMER
#define CHOICE_GETITIMER 1
#include <signal.h>	/* For signal(), SIGALRM, SIG_IGN  */
typedef struct itimerval timetype;
#define gettime(s) (void)getitimer(ITIMER_REAL, s)
/* Subtract *backwards* because timer is counting *down* */
#define tickdiff(s,t) (((t).it_value.tv_sec-(s).it_value.tv_sec)*1000000 + \
	(t).it_value.tv_usec - (s).it_value.tv_usec)
#define MINTICK 1

#else

#ifndef USE_FTIME
#ifndef HAVE_FTIME
#define HAVE_FTIME 1
#endif
#ifndef HAVE_SYS_TIMEB_H
#define HAVE_SYS_TIMEB_H 1
#endif
#define USE_FTIME HAVE_FTIME && HAVE_SYS_TIMEB_H
#endif

#if USE_FTIME
#include <sys/timeb.h>
typedef struct timeb timetype;
#define gettime(s) (void)ftime(s)
#define tickdiff(s,t) (((s).time-(t).time)*1000 + (s).millitm - (t).millitm)
#define MINTICK	0

#else

#error No clock available

#endif /* USE_FTIME */
#endif /* USE_GETITIMER */
#endif /* USE_GETTIMEOFDAY */
#endif /* USE_CLOCK_GETTIME */
#endif /* USE_GETHRTIME */

#if CHOICE_CLOCK_GETTIME && HAVE_CLOCK_GETRES

static unsigned
ranTickSize(void)
{
	struct timespec res;

	clock_getres(CLOCK_REALTIME, &res);
	return (unsigned)res.tv_nsec;
}

#else /* Normal clock resolution estimation */

#if NOISEDEBUG
#include <stdio.h>
#endif

#define N 15	/* Number of deltas to try (at least 5, preferably odd) */

/* Function needed for qsort() */
static int
ranCompare(void const *p1, void const *p2)
{
	return *(unsigned const *)p1 > *(unsigned const *)p2 ?  1 :
	       *(unsigned const *)p1 < *(unsigned const *)p2 ? -1 : 0;
}

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
static unsigned
ranTickSize(void)
{
	unsigned i = 0, j = 0,  diff, d[N];
	timetype tv0, tv1, tv2;
	/*
	 * TODO Get some per-run data to seed the RNG with.
	 * pid, ppid, etc.
	 */
	gettime(&tv0);
	tv1 = tv0;
	do {
		gettime(&tv2);
		diff = (unsigned)tickdiff(tv2, tv1);
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

#endif /* Clock resolution measurement selection */

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
	static unsigned ticksize = 0;
	static timetype prevt;

	gettime(&t);
#if CHOICE_GETITIMER
	/* If itimer isn't started, start it */
	if (t.it_value.tv_sec == 0 && t.it_value.tv_usec == 0) {
		/*
		 * start the timer - assume that PGP won't be running for
		 * more than 31 days...
		 */
		t.it_value.tv_sec = 1000000000;
		t.it_interval.tv_sec = 1000000000;
		t.it_interval.tv_usec = 0;
#ifdef SIGALRM
		signal(SIGALRM, SIG_IGN);	/* just in case.. */
#endif
		setitimer(ITIMER_REAL, &t, NULL);
		t.it_value.tv_sec = 0;
	}
	pgpRandomAddBytes(rc, (PGPByte const *)&t.it_value, sizeof(t.it_value));
#else
	pgpRandomAddBytes(rc, (PGPByte const *)&t, sizeof(t));
#endif

	if (!ticksize)
		ticksize = ranTickSize();
	delta = (PGPUInt32)(tickdiff(t, prevt) / ticksize);
	prevt = t;

	return delta;
}
