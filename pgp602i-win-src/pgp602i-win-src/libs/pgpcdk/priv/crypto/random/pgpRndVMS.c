/*
 * Get environmental noise to seed the random number generator.
 * VMS version.
 *
 * This hasn't been examined in a long time (December, 1995).
 * Could someone who knows VMS please have a look at it and determine
 * if this is still good?
 *
 * $Id: pgpRndVMS.c,v 1.6 1997/06/25 17:58:55 lloyd Exp $
 */

#include "pgpConfig.h"

#include "pgpRnd.h"
#include "pgpRndom.h"
#include "pgpUsuals.h"

/*
 * Add as much environmentally-derived random noise as possible
 * to the randPool.  Typically, this involves reading the most
 * accurate system clocks available.
 *
 * Returns the number of ticks that have passed since the last call,
 * for entropy estimation purposes.
 */
PGPUInt32
ranGetEntropy(PGPRandomContext const *rc)
{
	PGPUInt32 delta;
	PGPUInt32 d1;	/* MSW of difference */
	PGPUInt32 t[2];	/* little-endian 64-bit timer */
	static PGPUInt32 prevt[2];

	SYS$GETTIM(t);	/* VMS hardware clock increments by 100000 per tick */
	pgpRandomAddBytes(rc, (PGPByte const *)t, sizeof(t));
	/* Get difference in d1 and delta, and old time in prevt */
	d1 = t[1] - prevt[1] + (t[0] < prevt[0]);
	prevt[1] = t[1];
	delta = t[0] - prevt[0];
	prevt[0] = t[0];
	
	/* Now, divide the 64-bit value by 100000 = 2^5 * 5^5 = 32 * 3125 */
	/* Divide value, MSW in d1 and LSW in delta, by 32 */
	delta >>= 5;
	delta |= d1 << (32-5);
	d1 >>= 5;
	/*
	 * Divide by 3125.  This fits into 16 bits, so the following
	 * code is possible.  2^32 = 3125 * 1374389 + 1671.
	 *
	 * This code has confused people reading it, so here's a detailed
	 * explanation.  First, since we only want a 32-bit result,
	 * reduce the input mod 3125 * 2^32 before starting.  This
	 * amounts to reducing the most significant word mod 3125 and
	 * leaving the least-significant word alone.
	 *
	 * Then, using / for mathematical (real, not integer) division, we
	 * want to compute floor((d1 * 2^32 + d0) / 3125), which I'll denote
	 * using the old [ ] syntax for floor, so it's
	 *   [ (d1 * 2^32 + d0) / 3125 ]
	 * = [ (d1 * (3125 * 1374389 + 1671) + d0) / 3125 ]
	 * = [ d1 * 1374389 + (d1 * 1671 + d0) / 3125 ]
	 * = d1 * 137438 + [ (d1 * 1671 + d0) / 3125 ]
	 * = d1 * 137438 + [ d0 / 3125 ] + [ (d1 * 1671 + d0 % 3125) / 3125 ]
	 *
	 * The C / operator, applied to integers, performs [ a / b ], so
	 * this can be implemented in C, and since d1 < 3125 (by the first
	 * modulo operation), d1 * 1671 + d0 % 3125 < 3125 * 1672, which
	 * is 5225000, less than 2^32, so it all fits into 32 bits.
	 */
	d1 %= 3125;	/* Ignore overflow past 32 bits */
	delta = delta/3125 + d1*1374389 + (delta%3125 + d1*1671) / 3125;

	return delta;
}
