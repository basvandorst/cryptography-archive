/*
* pgpRndMSDOS.c -- Get high-resolution timing information to seed the
*                  random number generator. MS-DOS version.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRndMSDOS.c,v 1.1.2.1 1997/06/07 09:51:34 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <time.h>		/* For time measurement code */

#if __BORLANDC__
#ifndef far
#define far __far /* Borland C++ 3.1's <dos.h> kacks in ANSI mode. Ugh! */
#endif
#endif

#include <dos.h>		/* for enable() and disable() */
#include <conio.h>		/* for inp() and outp() */

#include "pgpRndom.h"
#include "pgpUsuals.h"
#include "pgpRnd.h"

/*
* This code gets as much information as possible out of 8253/8254 timer 0,
* which ticks every .84 microseconds. There are three cases:
* 1) Original 8253. 15 bits available, as the low bit is unused.
* 2) 8254, in mode 3. The 16th bit is available from the status register.
* 3) 8254, in mode 2. All 16 bits of the counters are available.
* (This is not documented anywhere, but I've seen it!)
*
* This code repeatedly tries to latch the status (ignored by an 8253) and
* sees if it looks like xx1101x0. If not, it's definitely not an 8254.
* Repeat this a few times to make sure it is an 8254.
*/
static int
has8254(void)
	{
			int i, s1, s2;

			for (i = 0; i < 5; i++) {
					_disable();
					outp(0x43, 0xe2);		/* Latch status for timer 0 */
					s1 = inp(0x40);			/* If 8253, read timer low byte */
					outp(0x43, 0xe2);		/* Latch status for timer 0 */
					s2 = inp(0x40);			/* If 8253, read timer high byte */
					_enable();
					if ((s1 & 0x3d) != 0x34 || (s2 & 0x3d) != 0x34)
						return 0;	/* Ignoring status latch; 8253 */
			}
			return 1;	/* Status reads as expected; 8254 */
	}

static unsigned
read8254(void)
{
	unsigned status, count;

	_disable();
	outp(0x43, 0xc2);		/* Latch status and count for timer 0 */
	status = inp(0x40);
	count = inp(0x40);
	count |= inp(0x40) << 8;
	_enable();
/* The timer is usually in mode 3, but some motherboards use mode 2. */
if (status & 2)
	count = count>>1 | (status & 0x80)<<8;

return count;
}

static unsigned
read8253(void)
{
			unsigned count;

			_disable();
			outp(0x43, 0x00);	/* Latch count for timer 0 */
			count = (inp(0x40) & 0xff);
			count |= (inp(0x40) & 0xff) << 8;
			_enable();

			return count >> 1;
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
			static unsigned deltamask = 0;
			static unsigned prevt;
			unsigned t;
			time_t tnow;
			clock_t cnow;

if (deltamask == 0)
	deltamask = has8254() ? 0xffff : 0x7fff;
t = (deltamask & 0x8000) ? read8254() : read8253();
pgpRandomAddBytes(rc, (byte const *)&t, sizeof(t));
delta = deltamask & (t - prevt);
prevt = t;

/* Add more-significant time components. */
			cnow = clock();
			pgpRandomAddBytes(rc, (byte *)&cnow, sizeof(cnow));
			tnow = time((time_t *)0);
			pgpRandomAddBytes(rc, (byte *)&tnow, sizeof(tnow));

			return delta;
	}
